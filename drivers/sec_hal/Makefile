# /* ************************************************************************* **
# **                               Renesas                                     **
# ** ************************************************************************* */
#
# /* *************************** COPYRIGHT INFORMATION *********************** **
# ** This program contains proprietary information that is a trade secret of   **
# ** Renesas and also is protected as an unpublished work under                **
# ** applicable Copyright laws. Recipient is to retain this program in         **
# ** confidence and is not permitted to use or make copies thereof other than  **
# ** as permitted in a written agreement with Renesas.                         **
# **                                                                           **
# ** Copyright (C) 2012 Renesas Electronics Corp.                              **
# ** All rights reserved.                                                      **
# ** ************************************************************************* */
ifneq ($(KERNELRELEASE),)

NAME                      := sec_hal
INCLUDES                   = -I$(src) -I$(src)/exports -I$(src)/rt/inc -I$(src)/sdtoc/inc

obj-$(CONFIG_ARM_SEC_HAL)  = $(NAME).o
ccflags-y                 += $(INCLUDES) -DPUBLIC_ENVIRONMENT -DSEC_HAL_TRACE_TO_KERNEL
asflags-y                 += $(INCLUDES)
$(NAME)-y                 := sec_hal_dev.o
# legacy code ported from previous implementation.
$(NAME)-y                 += rt/src/sec_hal_rt.o
$(NAME)-y                 += rt/src/sec_msg.o
$(NAME)-y                 += rt/src/sec_hal_pm.o

ifeq ($(CONFIG_ARM_SEC_HAL_TEST_DISPATCHER),y) # select test dispatcher.
$(NAME)-y                 += rt/test/src/sec_hal_test_dispatcher.o
else # select real dispatcher.
# ARM specific implementation for entering and exiting TZ.
$(NAME)-y                 += rt/src/sec_dispatch.o
$(NAME)-y                 += rt/src/sec_bridge_tz.o
endif

ifeq ($(CONFIG_ARM_SEC_HAL_SDTOC),y) # toc read feature
$(NAME)-y                 += sdtoc/src/sec_hal_sdtoc.o
endif

else

KERNELDIR                := /lib/modules/$(shell uname -r)/build
PWD                      := $(shell pwd)


default all modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean


endif

