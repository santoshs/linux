# high_temp program

TARGET = high_temp

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0   = ../src
INC1   = ../../inc
INC    = -I$(INC0) -I$(INC1)
SRC    = ../src
VPATH  = $(SRC):$(INC0):$(INC1)

SCATER = $(TARGET).lds

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../via
OPT	= $(VIA_DIR)/loader_opt.mk       \
	  $(VIA_DIR)/loader_link_opt.mk\

include $(OPT)


#----------------------------------------------------------------
# object
#----------------------------------------------------------------

OBJS    = 	high_temp.o \
			high_temp_main.o

LIBSDIR = ../../libs
LIBS    = $(LIBSDIR)/common.a\
          $(LIBSDIR)/std.a\
          $(LIBSDIR)/lcd.a\
          $(LIBSDIR)/hw_init.a\
          $(LIBSDIR)/serial.a\
          $(LIBSDIR)/ths.a\
          $(LIBSDIR)/pmic.a\
		  $(LIBSDIR)/tmu.a


.PHONY : all allclean clean

all: $(TARGET).bin
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)         :$(TARGET).mk $(OPT)
high_temp.o	:
high_temp_main.o :high_temp.h\
                 string.h\
                 flash_api.h\
                 usb_api.h\
                 common.h\
                 com_type.h\
                 com_api.h\
                 lcd_api.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
include $(VIA_DIR)/loader_bin.mk
