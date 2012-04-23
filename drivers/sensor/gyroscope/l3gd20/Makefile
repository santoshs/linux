########################################
###	l3gd20 driver Makefile #######
########################################


ifeq ($(CONFIG_L3GD20_POLLING),y)
	EXTRA_CFLAGS	+= -DL3GD20_POLLING
endif
obj-$(CONFIG_GYROSCOPE) += l3gd20.o

