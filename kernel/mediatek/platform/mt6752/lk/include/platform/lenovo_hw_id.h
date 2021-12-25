/*
 *
 * lenovo sw jixj 2014.9.10 add for hardware id
 *
 */

#ifndef __LENOVO_HARDWARE_ID_H
#define __LENOVO_HARDWARE_ID_H
typedef enum {
    HWID_PRC_OPEN,
    HWID_CMCC,
    HWID_CU,
    HWID_CT,
    HWID_ROW1,
    HWID_ROW2,
    HWID_CN,
    HWID_APAC
}HW_ID_TYPE_ENUM;

extern HW_ID_TYPE_ENUM get_hardware_id(void);
#endif /*__LENOVO_HARDWARE_ID_H */

