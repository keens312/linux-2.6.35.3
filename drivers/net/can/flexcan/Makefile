obj-$(CONFIG_CAN_FLEXCAN)	+= flexcan.o

ifeq ($(CONFIG_CONVERT_TO_SCR_CAN),y)
	flexcan-y := dev.o drv.o mbm.o
else
	flexcan-y := dev.o drv.zy mbm.o
endif
