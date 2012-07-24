#
#

TARGET	= fastboot

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0	= ../src
INC1	= ../../inc
INC		= -I$(INC0) -I$(INC1)
SRC		= ../src
VPATH	= $(SRC):$(INC0):$(INC1)

SCATER	= $(TARGET).lds

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR	= ../../via/
OPT		= $(VIA_DIR)loader_opt.mk     \
		  $(VIA_DIR)loader_link_opt.mk\

include $(OPT)


#----------------------------------------------------------------
# object
#----------------------------------------------------------------

OBJS	= fb_common.o\
		  fb_dev_mgmr.o\
		  fb_adapt_emmc.o\
		  fb_adapt_usb.o\
		  fb_main.o\
		  fb_flash.o\
		  fb_comm.o\
		  fb_analy.o\
		  fb_boot.o\
		  sparse_crc32.o\
		  simg2img.o
		  

LIBSDIR	= ../../libs
LIBS	= $(LIBSDIR)/FlashAccessApi.a\
          $(LIBSDIR)/common.a\
          $(LIBSDIR)/std.a\
          $(LIBSDIR)/usb_fb.a\
          $(LIBSDIR)/hw_init.a\
          $(LIBSDIR)/serial.a

.PHONY : all allclean clean

all: $(TARGET).bin
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)         :$(TARGET).mk $(OPT)
fb_boot.o		:
fb_analy.o		:string.h fb_common.h fb_analy.h
fb_comm.o		:string.h fb_common.h fb_comm.h fb_dev_mgmr.h
fb_flash.o		:fb_common.h fb_flash.h fb_dev_mgmr.h
fb_main.o		:string.h fb_common.h fb_comm.h fb_analy.h fb_flash.h\
				 fb_dev_mgmr.h flash_api.h usb_api.h common.h com_type.h com_api.h 
fb_adapt_usb.o	:fb_common.h fb_dev_mgmr.h usb_api.h fb_adapt_usb.h
fb_adapt_emmc.o	:fb_common.h fb_dev_mgmr.h flash_api.h fb_adapt_emmc.h
fb_dev_mgmr.o	:fb_common.h fb_dev_mgmr.h flash_api.h fb_adapt_emmc.h fb_adapt_usb.h
fb_common.o		:string.h fb_common.h
sparse_crc32.o	:ext4_utils.h
simg2img.o		:ext4_utils.h sparse_format.h sparse_crc32.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
include $(VIA_DIR)loader_bin.mk
