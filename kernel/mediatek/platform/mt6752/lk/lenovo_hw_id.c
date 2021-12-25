/*
 *
 * lenovo sw jixj 2014.9.10 add for hardware id
 *
 */
#include <platform/lenovo_hw_id.h>
#include <platform/mt_gpio.h>

HW_ID_TYPE_ENUM get_hardware_id()
{
    unsigned char hwid0 = 0;
    unsigned char hwid1 = 0;
    unsigned char hwid2 = 0;
    int hwid = -1;
    
    hwid0 = mt_get_gpio_in(GPIO_HARDWARE_ID0);
    hwid1 = mt_get_gpio_in(GPIO_HARDWARE_ID1);
    hwid2 = mt_get_gpio_in(GPIO_HARDWARE_ID2);

    hwid = (hwid2 & 0x01)<<2 |(hwid1 & 0x01)<<1 | (hwid0 & 0x01); 
    return hwid;
}

