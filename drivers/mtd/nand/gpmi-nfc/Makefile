obj-$(CONFIG_MTD_NAND_GPMI_NFC) += gpmi-nfc.o

ifeq ($(CONFIG_CONVERT_TO_SCR_NANDFLASH),y)
	gpmi-nfc-objs += gpmi-nfc-main.o
else
	gpmi-nfc-objs += gpmi-nfc-main.zy
endif

gpmi-nfc-objs += gpmi-nfc-event-reporting.o
gpmi-nfc-objs += gpmi-nfc-hal-common.o
gpmi-nfc-objs += gpmi-nfc-hal-v0.o
gpmi-nfc-objs += gpmi-nfc-hal-v1.o
gpmi-nfc-objs += gpmi-nfc-hal-v2.o
gpmi-nfc-objs += gpmi-nfc-rom-common.o
gpmi-nfc-objs += gpmi-nfc-rom-v0.o
gpmi-nfc-objs += gpmi-nfc-rom-v1.o
gpmi-nfc-objs += gpmi-nfc-mil.o
