#######################################################
###	Proximity and ambient light driver Makefile #######
#######################################################


ifeq ($(CONFIG_PROXIMITY_POLLING),y)
	EXTRA_CFLAGS	+= -DPROXIMITY_POLLING
endif
obj-$(CONFIG_PROXIMITY) += bh1771glc.o

