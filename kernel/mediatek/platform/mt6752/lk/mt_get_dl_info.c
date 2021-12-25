#include <printf.h>
#include <debug.h>
#include <malloc.h>
#include <string.h>
#include <platform/errno.h>
#include "platform/partition.h"

struct checksum_info {
    u32 image_index;
    u32 pc_checksum;
    u32 da_checksum;
    char checksum_status[8];
};

struct image_info {
    char image_name[16];
};

struct dl_status {
    char magic_num[24];
    char version[8];
    struct checksum_info cs_info[PART_MAX_COUNT];
    char ram_checksum[16];
    char download_status[16];
    struct image_info img_info[PART_MAX_COUNT];
};

/*lenovo-sw jixj 2013.10.11 add begin*/
typedef struct{
	char magic_num[32];
	u32 checksum[32];
	char download_status[16];
}PROG_STATUS_EMMC;
/*lenovo-sw jixj 2013.10.11 add end*/


#define DL_PASS         (0)
#define DL_FAIL         (1)
#define DL_NOT_FOUND    (2)
#define DL_INFO_SIZE    (2048)

int mmc_get_dl_info(void)
{
    struct dl_status download_info;
    u64 dl_addr;
    u8 *dl_buf;

    part_dev_t *dev;
    part_t *part;

    int i, err, loglevel;
    /*lenovo-sw jixj 2013.10.11 add begin*/
    PROG_STATUS_EMMC prog_info;
    /*lenovo-sw jixj 2013.10.11 add end*/

    dev = mt_part_get_device();
    if (!dev) {
        dprintf(CRITICAL, "[DL_INFO]fail to get device\n");
        err = -ENODEV;
        goto out;
    }

    part = get_part("flashinfo");
    if (!part) {
        dprintf(CRITICAL, "[DL_INFO]fail to find partition flashinfo\n");
        err = -ENODEV;
        goto out;
    }

    dl_addr = (u64)(part->start_sect + part->nr_sects) * 512ULL - DL_INFO_SIZE;
    dprintf(ALWAYS, "[DL_INFO]get dl info from 0x%llx\n", dl_addr);

    dl_buf = (u8 *)calloc(1, DL_INFO_SIZE);
    if (!dl_buf) {
        dprintf(CRITICAL, "[DL_INFO]fail to calloc buffer(count=%d)\n", DL_INFO_SIZE);
        err = -ENOMEM;
        goto fail_malloc;
    }

    err = dev->read(dev, dl_addr, dl_buf, DL_INFO_SIZE, part->part_id);
    if (err != DL_INFO_SIZE) {
        dprintf(CRITICAL, "[DL_INFO]fail to read data(%d)\n", err);
        err = -EIO;
        goto fail_read;
    }

    memcpy(&download_info, dl_buf, sizeof(download_info));

    /*lenovo-sw jixj 2013.10.11 add begin*/
    memcpy(&prog_info,dl_buf,sizeof(prog_info));
    if(!memcmp(prog_info.download_status,"PASS",4)) {
        return DL_PASS;
    }
    /*lenovo-sw jixj 2013.10.11 add end*/

    if (memcmp(download_info.magic_num, "DOWNLOAD INFORMATION!!", 22)) {
        dprintf(CRITICAL, "[DL_INFO]fail to find DL INFO magic\n");
        err = DL_NOT_FOUND;
        goto fail_read;
    }

    /*lenovo-sw chuyq modified 2014.8.02 begin:need checksum download*/
#ifndef LENOVO_UBOOT_DL_CHECK
    if (!memcmp(download_info.download_status, "DL_DONE", 7) ||
        !memcmp(download_info.download_status, "DL_CK_DONE", 10)) 
#else
	if (!memcmp(download_info.download_status,"DL_CK_DONE",10))
#endif
    /*lenovo-sw chuyq modified 2014.8.02 end*/
    {
        loglevel = INFO;
        loglevel = CRITICAL;
        err = DL_PASS;
    } else {
        loglevel = CRITICAL;
        err = DL_FAIL;
    }

    dprintf(loglevel, "[DL_INFO]version: %s\n", download_info.version);	
    dprintf(loglevel, "[DL_INFO]dl_status: %s\n", download_info.download_status);	
    dprintf(loglevel, "[DL_INFO]dram_checksum: %s\n", download_info.ram_checksum);
    for (i = 0; i < PART_MAX_COUNT; i++) {

        /*lenovo-sw jixj 2012.10.9 add begin*/
        #ifdef LENOVO_UBOOT_DL_CHECK
        download_info.cs_info[i].checksum_status[8] = '\0';
        #endif
        /*lenovo-sw jixj 2012.10.9 add end*/

        if (download_info.cs_info[i].image_index != 0) {
            dprintf(loglevel, "[DL_INFO]image:[%02d]%-12s, checksum: %s\n", 
                    download_info.cs_info[i].image_index,
                    download_info.img_info[i].image_name,
                    download_info.cs_info[i].checksum_status);
        }
    }

fail_read:
    free(dl_buf);
fail_malloc:
    //put_part(part);
out:
    return err;
}
