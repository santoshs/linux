obj-m   := sndptest.o

sndptest-objs := sndptest_if.o sndptest_core.o

KDIR    := /lib/modules/$(shell uname -r)/build
PWD     := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f *.o
	rm -f *.ko
	rm -f *.mod.c
	rm -f *.cmd
	rm -f *.symvers
	rm -f *.order
	rm -f *.BAK
	rm -f *~
