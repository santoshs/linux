# pmic.mk

#----------------------------------------------------------------
# target
#----------------------------------------------------------------
TARGET	= pmic
ULIB	= $(TARGET).a

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0	= ../src
INC1	= ../../../inc
INC		= -I$(INC0) -I$(INC1)
VPATH	= $(INC0):$(INC1):$(SRC)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR	= ../../../via
OPT		= $(VIA_DIR)/loader_opt.mk

include $(OPT)
#----------------------------------------------------------------
# object
#----------------------------------------------------------------
OBJS        = pmic_battery.o pmic_charger.o pmic_rtc.o pmic_tusb.o

LIBSDIR     = ../../libs
SEC_LIBSDIR = ../../../securelib/lib
ICE_LIBSDIR = ../../../ice_enable/lib

LIBS        = $(LIBSDIR)/i2c.a  			\
              $(LIBSDIR)/std.a             \
              $(LIBSDIR)/common.a          \
              $(LIBSDIR)/serial.a
              


.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)           :$(TARGET).mk $(OPT)
pmic_battery.o          :common.h pmic.h i2c.h string.h log_output.h
pmic_charger.o          :common.h pmic.h i2c.h string.h log_output.h
pmic_rtc.o          	:common.h pmic.h i2c.h string.h log_output.h
pmic_tusb.o          	:common.h pmic.h i2c.h string.h log_output.h cpg.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------

include $(VIA_DIR)/loader_lib.mk
