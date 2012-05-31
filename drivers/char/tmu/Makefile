#
# obj-m   := tmu_driver.o
#

TARGET:= tmu_driver.ko

DEBFLAGS = -O2
EXTRA_CFLAGS += $(DEBFLAGS)

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all: ${TARGET}

tmu_driver.ko: tmu_driver.c
	$(MAKE) -C $(KERNELDIR) M=$(PWD) V=1 LDDINCDIR=$(PWD)/../include
clean:
	make -C $(KERNELDIR) M=$(PWD) V=1 clean

obj-m:= tmu_driver.o

clean-files := *.o *.ko *.mod.[co] *~

