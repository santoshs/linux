EXTRA_CFLAGS += -D__PRINT_VCD__
#EXTRA_CFLAGS += -D__VCD_MERAM_ENABLE__
EXTRA_CFLAGS += -D__VCD_POWERDOMAIN_ENABLE__
EXTRA_CFLAGS += -D__DEBUG__
#EXTRA_CFLAGS += -D__VCD_SPUV_FW_FROM_SD_ENABLE__
#EXTRA_CFLAGS +=  -D__VCD_TTY_ENABLE__


ifneq ($(KERNELRELEASE),)
# call from kernel build system

vcd-objs	:= ./interface/vcd.o
vcd-objs	+= ./control/vcd_ctrl.o
vcd-objs	+= ./control/vcd_ctrl_func.o
vcd-objs	+= ./spuv/vcd_spuv.o
vcd-objs	+= ./spuv/vcd_spuv_func.o

obj-y		+= vcd.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

