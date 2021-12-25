/*
* Copyright (C) 2012 lenovo, Inc.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <linux/irq.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ctype.h>
#include <linux/uaccess.h> /* sys_sync */
#include <linux/rtc.h> /* sys_sync */
#include <linux/err.h>
//#include <asm/gpio.h>


#if 0
//acpuclk_get_rate doesn't work because acpuclk_data is no longer available in krait
//yangjq, Add for acpuclk_get_rate()
#include "acpuclock.h"
#endif

#define private_attr(_name) \
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = 0644,			\
	},					\
	.show	= _name##_show,			\
	.store	= _name##_store,		\
}

extern unsigned int pm_log_get_cpu_freq(int cpu);
extern int wakelock_dump_info(char* buf);
static ssize_t pm_status_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	char *s = buf;
	unsigned long rate; // khz
	int cpu;

	// show CPU clocks
	for (cpu = 0; cpu < nr_cpu_ids; cpu++) {
		s += sprintf(s, "APPS[%d]:", cpu);
		if (cpu_online(cpu)) {
#if 0
			//acpuclk_get_rate doesn't work because acpuclk_data is no longer available in krait
			rate = acpuclk_get_rate(cpu); // khz
			s += sprintf(s, "(%3lu MHz); \n", rate / 1000);
#else
			//Call acpu_clk_get_rate added in clock-krait-8974.c
			rate = pm_log_get_cpu_freq(cpu); // hz
			s += sprintf(s, "(%3lu MHz); \n", rate);
#endif
		} else {
			s += sprintf(s, "sleep; \n");
		}
	}

	s += wakelock_dump_info(s);
	
	return (s - buf);
}

static ssize_t pm_status_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t n)
{
	printk(KERN_ERR "%s: no support yet.\n", __func__);

	return -EPERM;
}

static unsigned pm_wakeup_fetched = true;
static ssize_t pm_wakeup_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	char *s = buf;

	if (!pm_wakeup_fetched) {
		pm_wakeup_fetched = true;
		s += sprintf(s, "true");
	} else
		s += sprintf(s, "false");
	
	return (s - buf);
}

static ssize_t pm_wakeup_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t n)
{
	printk(KERN_ERR "%s: no support yet.\n", __func__);

	return -EPERM;
}

private_attr(pm_status);
private_attr(pm_wakeup);

static struct attribute *g_private_attr[] = {
	&pm_status_attr.attr,
	&pm_wakeup_attr.attr,
	NULL,
};

static struct attribute_group private_attr_group = {
	.attrs = g_private_attr,
};

static struct kobject *sysfs_private_kobj;

#define SLEEP_LOG
#ifdef SLEEP_LOG
#define WRITE_SLEEP_LOG
#define MAX_WAKEUP_IRQ 8
#define WAKEUP_REASON_BUF_LEN 256

enum {
	DEBUG_SLEEP_LOG = 1U << 0,
	DEBUG_WRITE_LOG = 1U << 1,
	DEBUG_WAKEUP_IRQ = 1U << 2,
	DEBUG_RPM_SPM_LOG = 1U << 3,
	DEBUG_RPM_CXO_LOG = 1U << 4,
	DEBUG_ADSP_CXO_LOG = 1U << 5,
	DEBUG_MODEM_CXO_LOG = 1U << 6,
	DEBUG_WCNSS_CXO_LOG = 1U << 7,
};
static int debug_mask;// = DEBUG_WRITE_LOG;
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

struct sleep_log_t {
	char time[18];
	long timesec;
	unsigned int log;
	int wakeup_irq[MAX_WAKEUP_IRQ];
	int wakeup_gpio;
       char wakeup_reason_buf[WAKEUP_REASON_BUF_LEN];
//31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
//bit1-0=00 :try to sleep; bit 1-0 = 01 : leave from sleep    ;bit1-0=10:fail to sleep
//bit31-bit24 : return value
};

#define TRY_TO_SLEEP  (0)
#define LEAVE_FORM_SLEEP  (1)
#define FAIL_TO_SLEEP  (2)

#define SLEEP_LOG_LENGTH 80

struct sleep_log_t sleep_log_array[SLEEP_LOG_LENGTH];
int sleep_log_pointer = 0;
int sleep_log_count = 0;
int enter_times = 0;

static int irq_wakeup_saved = MAX_WAKEUP_IRQ;
static int irq_wakeup_irq[MAX_WAKEUP_IRQ];
static int irq_wakeup_gpio;

static char wakeup_reason[WAKEUP_REASON_BUF_LEN] = { 0 };
static int wakeup_reason_buf_len = 0;

char sleep_log_name[60];
struct file *sleep_log_file = NULL;

#ifdef WRITE_SLEEP_LOG
static int sleep_log_write(void)
{
	char buf[256];
	char *p, *p0;
	int i, j, pos;
	mm_segment_t old_fs;
	p = buf;
	p0 = p;

	if (sleep_log_file == NULL)
		sleep_log_file = filp_open(sleep_log_name, O_RDWR | O_APPEND | O_CREAT,
				0644);
	if (IS_ERR(sleep_log_file)) {
		printk("error occured while opening file %s, exiting...\n",
				sleep_log_name);
		return 0;
	}

	if (sleep_log_count > 1) {
		for (i = 0; i < 2; i++) {
			if (sleep_log_pointer == 0)
				pos = SLEEP_LOG_LENGTH - 2 + i;
			else
				pos = sleep_log_pointer - 2 + i;
			switch (sleep_log_array[pos].log & 0xF) {
			case TRY_TO_SLEEP:
				p += sprintf(p, ">[%ld]%s\n", sleep_log_array[pos].timesec,
						sleep_log_array[pos].time);
				break;
			case LEAVE_FORM_SLEEP:
				p += sprintf(p, "<[%ld]%s(",
						sleep_log_array[pos].timesec,
						sleep_log_array[pos].time);
				for (j = 0; j < MAX_WAKEUP_IRQ && sleep_log_array[pos].wakeup_irq[j]; j++)
					p += sprintf(p, " %d", sleep_log_array[pos].wakeup_irq[j]);

				if (sleep_log_array[pos].wakeup_gpio)
					p += sprintf(p, ", gpio %d", sleep_log_array[pos].wakeup_gpio);

                            if (strlen(sleep_log_array[pos].wakeup_reason_buf))
                                   p += sprintf(p, ",%s", sleep_log_array[pos].wakeup_reason_buf);
                            
				p += sprintf(p, ")\n");
				break;
			case FAIL_TO_SLEEP:
				p += sprintf(p, "^[%ld]%s(%d)\n", sleep_log_array[pos].timesec,
						sleep_log_array[pos].time,
						(char) (sleep_log_array[pos].log >> 24));
				break;
			}
		}
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	sleep_log_file->f_op->write(sleep_log_file, p0, p - p0,
			&sleep_log_file->f_pos);
	set_fs(old_fs);

	if (sleep_log_file != NULL) {
		filp_close(sleep_log_file, NULL);
		sleep_log_file = NULL;
	}
	return 0;
}
#else //WRITE_SLEEP_LOG
static int sleep_log_write(void)
{
	return 0;
}
#endif //WRITE_SLEEP_LOG

static int save_irq_wakeup_internal(int irq)
{
	int i;
	int ret;

	ret = 0;
	if (irq_wakeup_saved < MAX_WAKEUP_IRQ) {
		for (i = 0; i < irq_wakeup_saved; i++) {
			if (irq == irq_wakeup_irq[i])
				break;
		}
		if (i == irq_wakeup_saved)
			ret = irq_wakeup_irq[irq_wakeup_saved++] = irq;
	}
	return ret;
}

int save_irq_wakeup_gpio(int irq, int gpio)
{
	struct irq_desc *desc;
	int ret;

	ret = 0;
	if (debug_mask & DEBUG_WAKEUP_IRQ) {
		desc = irq_to_desc(irq);
		if (desc != NULL) {
			if (irqd_is_wakeup_set(&desc->irq_data)) {
				ret = save_irq_wakeup_internal(irq);
				if (ret) {
					if (gpio != 0 && irq_wakeup_gpio == 0) {
						irq_wakeup_gpio = gpio;
						irq_wakeup_saved = MAX_WAKEUP_IRQ;
					}
#ifdef CONFIG_KALLSYMS
					printk("%s(), irq=%d, gpio=%d, %s, handler=(%pS)\n", __func__, irq, gpio, 
						desc->action && desc->action->name ? desc->action->name : "",
						desc->action ? (void *)desc->action->handler : 0);
#else
					printk("%s(), irq=%d, gpio=%d, %s, handler=0x%08x\n", __func__, irq, gpio, 
						desc->action && desc->action->name ? desc->action->name : "",
						desc->action ? (unsigned int)desc->action->handler : 0);
#endif
				}
			}
		}
	}

	return ret;
}

void pmlog_save_wake_reason(char *reason_buf, int len)
{
    
    BUG_ON(len >= WAKEUP_REASON_BUF_LEN);
    
    if (len > 0)
    {
        strcpy(wakeup_reason, reason_buf);
        wakeup_reason_buf_len = strlen(reason_buf);
    }
}


static void clear_irq_wakeup_saved(void)
{
	if (debug_mask & DEBUG_WAKEUP_IRQ) {
		memset(irq_wakeup_irq, 0, sizeof(irq_wakeup_irq));
		irq_wakeup_gpio = 0;
		irq_wakeup_saved = 0;

              memset(wakeup_reason, 0, sizeof(wakeup_reason));
              wakeup_reason_buf_len = 0;
	}
}

static void set_irq_wakeup_saved(void)
{
	if (debug_mask & DEBUG_WAKEUP_IRQ)
		irq_wakeup_saved = MAX_WAKEUP_IRQ;
}

void log_suspend_enter(void)
{
	//extern void smem_set_reserved(int index, int data);
	struct timespec ts_;
	struct rtc_time tm_;

	//Turn on/off the share memory flag to inform RPM to record spm logs
	//smem_set_reserved(6, debug_mask & DEBUG_WAKEUP_IRQ ? 1 : 0);
	//smem_set_reserved(6, debug_mask);

	if (debug_mask & DEBUG_SLEEP_LOG) {
		printk("%s(), APPS try to ENTER sleep mode>>>\n", __func__);

		getnstimeofday(&ts_);
		rtc_time_to_tm(ts_.tv_sec + 8 * 3600, &tm_);

		sprintf(sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].time,
				"%d-%02d-%02d %02d:%02d:%02d", tm_.tm_year + 1900, tm_.tm_mon + 1,
				tm_.tm_mday, tm_.tm_hour, tm_.tm_min, tm_.tm_sec);

		if (strlen(sleep_log_name) < 1) {
			sprintf(sleep_log_name,
					"/data/local/log/aplog/sleeplog%d%02d%02d_%02d%02d%02d.txt",
					tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday, tm_.tm_hour,
					tm_.tm_min, tm_.tm_sec);
			printk("%s(), sleep_log_name = %s \n", __func__, sleep_log_name);
		}

		sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].timesec = ts_.tv_sec;
		sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].log = TRY_TO_SLEEP;
		sleep_log_pointer++;
		sleep_log_count++;
		if (sleep_log_pointer == SLEEP_LOG_LENGTH)
			sleep_log_pointer = 0;
	}

	clear_irq_wakeup_saved();
	pm_wakeup_fetched = false;
}

void log_suspend_exit(int error)
{
#if 0
	extern int smem_get_reserved(int index);
#else
	//extern void msm_rpmstats_get_reverved(u32 reserved[][4]);
	//u32 reserved[4][4];
#endif
	struct timespec ts_;
	struct rtc_time tm_;
	//uint32_t smem_value;
	int i;

	if (debug_mask & DEBUG_SLEEP_LOG) {
		getnstimeofday(&ts_);
		rtc_time_to_tm(ts_.tv_sec + 8 * 3600, &tm_);
		sprintf(sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].time,
				"%d-%02d-%02d %02d:%02d:%02d", tm_.tm_year + 1900, tm_.tm_mon + 1,
				tm_.tm_mday, tm_.tm_hour, tm_.tm_min, tm_.tm_sec);

		sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].timesec = ts_.tv_sec;

		if (error == 0) {
			sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].log =
					LEAVE_FORM_SLEEP;
			for (i = 0; i < (irq_wakeup_gpio == 0 ? irq_wakeup_saved : 1); i++)
				sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].wakeup_irq[i] =
					irq_wakeup_irq[i];
			for (; i < MAX_WAKEUP_IRQ; i++)
				sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].wakeup_irq[i] = 0;
			sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].wakeup_gpio =
					irq_wakeup_gpio;

                     if (wakeup_reason_buf_len < WAKEUP_REASON_BUF_LEN && wakeup_reason_buf_len > 0)
                            strcpy(sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].wakeup_reason_buf, wakeup_reason);
		} else {
			printk("%s(), APPS FAIL to enter sleep^^^\n", __func__);

			sleep_log_array[sleep_log_pointer % SLEEP_LOG_LENGTH].log =
					FAIL_TO_SLEEP | (error << 24);
		}

		sleep_log_pointer++;
		sleep_log_count++;

		if (sleep_log_pointer == SLEEP_LOG_LENGTH)
			sleep_log_pointer = 0;

		if (debug_mask & DEBUG_WRITE_LOG) {
			enter_times++;
			if (enter_times < 5000)
				sleep_log_write();
		}
	}

	set_irq_wakeup_saved();
}
#else //SLEEP_LOG
void log_suspend_enter(void)
{
	clear_irq_wakeup_saved();
	pm_wakeup_fetched = false;
}

void log_suspend_exit(int error)
{
	set_irq_wakeup_saved();
}
#endif //SLEEP_LOG

static int __init sysfs_private_init(void)
{
	int result;

	printk("%s(), %d\n", __func__, __LINE__);

	sysfs_private_kobj = kobject_create_and_add("private", NULL);
	if (!sysfs_private_kobj)
		return -ENOMEM;

	result = sysfs_create_group(sysfs_private_kobj, &private_attr_group);
	printk("%s(), %d, result=%d\n", __func__, __LINE__, result);

#ifdef SLEEP_LOG
	strcpy (sleep_log_name, "");
	sleep_log_pointer = 0;
	sleep_log_count = 0;
	enter_times = 0;
#endif

	return result;
}

static void __exit sysfs_private_exit(void)
{
	printk("%s(), %d\n", __func__, __LINE__);
	sysfs_remove_group(sysfs_private_kobj, &private_attr_group);

	kobject_put(sysfs_private_kobj);
}

module_init(sysfs_private_init);
module_exit(sysfs_private_exit);
