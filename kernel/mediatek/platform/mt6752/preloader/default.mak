###################################################################
# Default Internal Feautre
###################################################################
HW_INIT_ONLY :=

###################################################################
# Default Project Feautre  (cust_bldr.h)
###################################################################

CFG_BOOT_DEV :=BOOTDEV_SDMMC

CFG_FPGA_PLATFORM :=0
CFG_EVB_PLATFORM :=0
CFG_BATTERY_DETECT :=1
CFG_UART_TOOL_HANDSHAKE :=1
CFG_USB_TOOL_HANDSHAKE :=1
CFG_USB_DOWNLOAD :=1
CFG_PMT_SUPPORT :=0
CFG_LOG_BAUDRATE :=921600
CFG_META_BAUDRATE :=115200
CFG_UART_LOG :=UART1
CFG_UART_META :=UART1

#only enable in eng mode
ifeq ("$(TARGET_BUILD_VARIANT)","eng")
CFG_OUTPUT_PL_LOG_TO_UART1:=0
else
CFG_OUTPUT_PL_LOG_TO_UART1:=0
endif

#only enable in user mode
ifeq ("$(TARGET_BUILD_VARIANT)","eng")
CFG_PMIC_FULL_RESET_SUPPORT:=0
else
CFG_PMIC_FULL_RESET_SUPPORT:=1
endif

CFG_EMERGENCY_DL_SUPPORT :=1
CFG_EMERGENCY_DL_TIMEOUT_MS :=1000*30
CFG_EMERGENCY_MAX_TIMEOUT_MS :=1000*0x3fff
CFG_USBIF_COMPLIANCE :=0
CFG_MMC_ADDR_TRANS :=1
CFG_LEGACY_USB_DOWNLOAD :=0

CFG_USB_UART_SWITCH:=0

CFG_HW_WATCHDOG :=1
CFG_BOOT_ARGUMENT :=1
CFG_RAM_CONSOLE :=1
CFG_MTJTAG_SWITCH :=0
CFG_MDMETA_DETECT :=0
CFG_APWDT_DISABLE :=0
CFG_MDWDT_DISABLE :=0
CFG_SYS_STACK_SZ :=3*1024

CFG_WORLD_PHONE_SUPPORT :=1

ONEKEY_REBOOT_NORMAL_MODE_PL :=1
KPD_PMIC_LPRST_TD :=1


CFG_USB_AUTO_DETECT :=0
CFG_USB_AUTO_DETECT_TIMEOUT_MS :=1000*3

CFG_FEATURE_ENCODE :=v1

###################################################################
# image loading options
###################################################################

#For Normal Boot
CFG_LOAD_UBOOT :=1
#For Dummy AP
CFG_LOAD_MD_ROM :=0
CFG_LOAD_MD_RAMDISK :=0
CFG_LOAD_MD_DSP :=0
#For SLT and Dummy AP
CFG_LOAD_AP_ROM :=0

#For CTP
CFG_LOAD_CONN_SYS :=0
#For SLT
CFG_LOAD_SLT_MD :=0
CFG_LOAD_SLT_MD32 :=0

###################################################################
# Dummy Load address
# If image header have load address, we use address in image header
# If not, we use address here
###################################################################
#For Normal Boot
CFG_UBOOT_MEMADDR       :=0x41E00000
#For Dummy AP
CFG_MD1_ROM_MEMADDR     :=0x40000000
CFG_MD1_RAMDISK_MEMADDR :=0x41400000
CFG_MD2_ROM_MEMADDR     :=0x42000000
CFG_MD2_RAMDISK_MEMADDR :=0x43400000
CFG_MD_DSP_MEMADDR      :=0x44000000
#For SLT and Dummy AP
CFG_AP_ROM_MEMADDR      :=0x46000000
#For CTP
CFG_CONN_SYS_MEMADDR    :=0x45A00000
#For SLT
CFG_TDD_MD_ROM_MEMADDR   :=0x40000000
CFG_TDD_ONLY_ROM_MEMADDR :=0x41000000
CFG_FDD_MD_ROM_MEMADDR   :=0x42000000
CFG_2G_MD_ROM_MEMADDR    :=0x43000000
CFG_MD32P_ROM_MEMADDR    :=0x44000000
CFG_MD32D_ROM_MEMADDR    :=0x45002000
CFG_BOOTA64_MEMADDR 	 :=0x40000000
CFG_DTB_MEMADDR 	 :=0x40000300
CFG_IMAGE_AARCH64_MEMADDR :=0x40080000


#For ATF
#CFG_ATF_ROM_MEMADDR   :=0x00110000
CFG_ATF_ROM_MEMADDR   :=0x00110DC0
CFG_BOOTIMG_HEADER_MEMADDR   :=0x00110800

CFG_ATF_SUPPORT :=1
CFG_TEE_SUPPORT :=0
CFG_TRUSTONIC_TEE_SUPPORT :=0

#For SRAM Protection
CFG_NON_SECURE_SRAM_ADDR :=0x0012c000
CFG_NON_SECURE_SRAM_SIZE :=0x4000
