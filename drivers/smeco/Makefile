#
# SMeCo makefile to build SMC for Linux Kernel
#

SMECO_PATH := drivers/smeco
U_INCLUDE  := -I$(SMECO_PATH)
U_INCLUDE  += -I$(SMECO_PATH)/linux
U_INCLUDE  += -I$(SMECO_PATH)/config
E_CFLAGS   := -DSMECO_LINUX_KERNEL

  # Check if test modules included
ifeq ($(CONFIG_SMECO_TEST),)
  # Tests not included
E_CFLAGS   += -DSMCTEST=FALSE
else
U_INCLUDE  += -I$(SMECO_PATH)/test
E_CFLAGS   += -DSMCTEST=TRUE
endif

  #
  # SMC control configuration
  #
ifeq ($(CONFIG_SMECO_CONTROL),y)
E_CFLAGS   += -DSMC_CONTROL=TRUE
ifeq ($(CONFIG_SMECO_CONTROL_USE_PHONET),y)
E_CFLAGS   += -DSMC_CONTROL_USE_PHONET=TRUE
else
E_CFLAGS   += -DSMC_CONTROL_USE_PHONET=FALSE
endif
else
E_CFLAGS   += -DSMC_CONTROL=FALSE
E_CFLAGS   += -DSMC_CONTROL_USE_PHONET=FALSE
endif

  #
  # L2MUX configuration
  #
ifeq ($(CONFIG_SMECO_L2MUX),y)
E_CFLAGS   += -DSMC_L2MUX_IF=TRUE
else
E_CFLAGS   += -DSMC_L2MUX_IF=FALSE
endif

EXTRA_CFLAGS += -std=gnu99 $(U_INCLUDE) $(E_CFLAGS)

ifneq ($(KERNELRELEASE),)
  # Linux Kernel build

obj-$(CONFIG_SMECO) += smeco.o

smeco-objs := smc.o
smeco-objs += smc_conf.o
smeco-objs += smc_fifo.o
smeco-objs += smc_mdb.o
smeco-objs += smc_memory_mgmt.o
smeco-objs += ./linux/smc_impl_linux.o
smeco-objs += ./linux/smc_device_driver_linux.o

ifeq ($(CONFIG_SMECO_CONTROL),y)
smeco-objs += ./linux/smc_conf_control_linux.o
endif

ifeq ($(CONFIG_SMECO_L2MUX),y)
smeco-objs += ./linux/smc_conf_l2mux_linux.o
endif

ifeq ($(CONFIG_SMECO_TEST),)
  # Tests not included
else
  # Tests included
smeco-objs += ./test/smc_test.o
smeco-objs += ./test/smc_test_fifo.o
smeco-objs += ./test/smc_test_mdb.o
smeco-objs += ./test/smc_test_messaging.o
endif

  # Build the test module if selected
obj-$(CONFIG_SMECO_TEST)  += /linux/test/

else

    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD       := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) LDDINCDIR=$(PWD)


endif

clean:
	rm -rf *.o *.ko
