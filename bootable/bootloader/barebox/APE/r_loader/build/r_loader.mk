#
#

TARGET      = r_loader
SCATER      = $(TARGET).lds

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0        = ../src
INC1        = ../../inc
SEC_INC     = ../../../securelib/inc
ICE_INC     = ../../../ice_enable/inc
INC         = -I$(INC0) -I$(INC1) -I$(SEC_INC) -I$(ICE_INC)
SRC         = ../src
VPATH       = $(INC0):$(INC1):$(SEC_INC):$(ICE_INC):$(SRC)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------

VIA_DIR     = ../../via
OPT         = $(VIA_DIR)/loader_opt.mk      \
              $(VIA_DIR)/loader_link_opt.mk

include $(OPT)

#----------------------------------------------------------------
# object
#----------------------------------------------------------------
OBJS        = r_loader.o r_loader_boot.o r_loader_vectors.o r_loader_boot_log.o r_loader_boot_matrix.o r_loader_crash_log.o

LIBSDIR     = ../../libs
SEC_LIBSDIR = ../../../securelib/lib
ICE_LIBSDIR = ../../../ice_enable/lib

ifeq ($(RL_LCD_ENABLE),TRUE)
LIBS        = $(LIBSDIR)/FlashAccessApi.a  	\
              $(LIBSDIR)/hw_init.a         	\
              $(LIBSDIR)/std.a             	\
              $(LIBSDIR)/common.a       	\
              $(LIBSDIR)/lcd.a      	    \
              $(LIBSDIR)/serial.a			\
			  $(LIBSDIR)/tmu.a				\
			  $(LIBSDIR)/pmic.a 			\
			  $(LIBSDIR)/ths.a				\
			  $(LIBSDIR)/usb.a
else
LIBS        = $(LIBSDIR)/FlashAccessApi.a  	\
              $(LIBSDIR)/hw_init.a         	\
              $(LIBSDIR)/std.a             	\
              $(LIBSDIR)/common.a          	\
			  $(LIBSDIR)/tmu.a	          	\
              $(LIBSDIR)/serial.a			\
			  $(LIBSDIR)/pmic.a				\
			  $(LIBSDIR)/ths.a				\
			  $(LIBSDIR)/usb.a
endif
              
ifeq ($(INTEGRITY_CHECK_ENABLE),TRUE)
LIBS +=  $(LIBSDIR)/disk_drive.a $(LIBSDIR)/securelib.a
endif

.PHONY : all allclean clean
all: $(TARGET).bin
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)       			: $(TARGET).mk $(OPT)
r_loader.o    			: com_type.h r_loader.h flash_api.h common.h compile_option.h r_loader_crash_log.h sysc.h\
						  string.h cpg.h gpio.h i2c.h sbsc.h log_output.h lcd_api.h r_loader_boot_log.h r_loader_boot_matrix.h
r_loader_boot_log.o    	: r_loader.h pmic.h r_loader_boot_log.h log_output.h com_type.h usb_api.h ths_api.h r_loader_boot_matrix.h
r_loader_boot_matrix.o  : r_loader.h pmic.h log_output.h com_type.h usb_api.h ths_api.h r_loader_boot_matrix.h gpio.h
r_loader_crash_log.o	: r_loader_crash_log.h compile_option.h com_type.h r_loader_boot_log.h
r_loader_boot.o    		:
r_loader_vectors.o 		:

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
include $(VIA_DIR)/loader_bin.mk
