# flash writer program

TARGET = flashwriter

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0   = ../src
INC1   = ../../inc
INC    = -I$(INC0) -I$(INC1)
SRC    = ../src
VPATH  = $(SRC):$(INC0):$(INC1)

ifeq ($(DUPDATER),UPDATER)
SCATER = updater.lds
else
SCATER = $(TARGET).lds
endif

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

OBJS    = flw_main.o\
          flw_write.o\
          flw_boot.o\
          flw_read.o\
          flw_write_protect.o\
          flw_clear_protect.o

LIBSDIR = ../../libs

ifeq ($(FW_LCD_ENABLE),TRUE)
LIBS    = $(LIBSDIR)/FlashAccessApi.a\
          $(LIBSDIR)/common.a\
          $(LIBSDIR)/std.a\
          $(LIBSDIR)/usb.a\
          $(LIBSDIR)/lcd.a\
          $(LIBSDIR)/hw_init.a\
          $(LIBSDIR)/serial.a\
		  $(LIBSDIR)/tmu.a
else
LIBS    = $(LIBSDIR)/FlashAccessApi.a\
          $(LIBSDIR)/common.a\
          $(LIBSDIR)/std.a\
          $(LIBSDIR)/usb.a\
          $(LIBSDIR)/hw_init.a\
          $(LIBSDIR)/serial.a\
		  $(LIBSDIR)/tmu.a
endif

ifeq ($(INTEGRITY_CHECK_ENABLE),TRUE)
LIBS +=  $(LIBSDIR)/securelib.a
endif

.PHONY : all allclean clean

all: $(TARGET).bin
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)         :$(TARGET).mk $(OPT)
flw_boot.o	:
flw_main.o	:flw_main.h\
                 string.h\
                 flash_api.h\
                 usb_api.h\
                 common.h\
                 com_type.h\
                 com_api.h\
                 lcd_api.h
flw_write.o	:flw_main.h\
                 string.h\
                 flash_api.h\
                 usb_api.h\
                 common.h\
                 com_type.h\
                 com_api.h\
				 lcd_api.h
flw_read.o	:flw_main.h\
                 usb_api.h\
                 common.h\
                 com_type.h\
                 com_api.h
flw_write_protect.o	:protect_option.h\
                 flw_main.h\
                 string.h\
                 flash_api.h\
                 usb_api.h\
                 common.h\
                 com_type.h\
                 com_api.h
flw_clear_protect.o	:protect_option.h\
                 flw_main.h\
                 string.h\
                 flash_api.h\
                 usb_api.h\
                 common.h\
                 com_type.h\
                 com_api.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
include $(VIA_DIR)/loader_bin.mk
