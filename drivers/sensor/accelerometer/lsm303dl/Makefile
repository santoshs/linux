########################################
###	lsm303dlhc driver Makefile #######
########################################

ifeq ($(CONFIG_LSM303DL_ACC_INT1),y)
	EXTRA_CFLAGS	+= -DLSM303DL_ACC_INT1
endif

ifeq ($(CONFIG_LSM303DL_ACC_INT2),y)
	EXTRA_CFLAGS	+= -DLSM303DL_ACC_INT2
endif

ifeq ($(CONFIG_LSM303DL_MAG_INT),y)
	EXTRA_CFLAGS	+= -DLSM303DL_MAG_INT
endif

obj-$(CONFIG_LSM303DL) 	+= lsm303dl_input.o lsm303dl_i2c_acc.o lsm303dl_i2c_mag.o