/*
 *
 * lenovo sw jixj 2014.9.10 add for hardware id
 *
 */
#include <mach/lenovo_hw_id.h>
#include <linux/device.h>

extern char temp_command_line[1024];
#define HWID_KEYWORD "hwid="

HW_ID_TYPE_ENUM get_hardware_id(void)
{
    int hwid = -1;
    char hwid_str[10];
    
    char *start = strstr(temp_command_line, HWID_KEYWORD);
    if(start != NULL) {
        start += strlen(HWID_KEYWORD);
        char *end = strchr(start, ' ');
        strncpy(hwid_str, start, end -start);
        //hwid = atoi(hwid_str);
        sscanf(hwid_str, "%d", &hwid);
    } else {
        printk(KERN_ERR"cmdline not hwid\n");
    }
    printk("hwid=%d\n", hwid);
    return hwid;
}

EXPORT_SYMBOL(get_hardware_id) ;

