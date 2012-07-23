# lcd.mk


#----------------------------------------------------------------
# target
#----------------------------------------------------------------
TARGET=lcd
ULIB = $(TARGET).a

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0    = ../src
INC1    = ../inc
INC2    = ../../../inc
INC     = -I$(INC0) -I$(INC1) -I$(INC2)
VPATH   = $(INC0):$(INC1):$(INC2)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../../via
OPT     = $(VIA_DIR)/loader_opt.mk

include $(OPT)
#----------------------------------------------------------------
# object
#----------------------------------------------------------------

OBJS    = lcd_api.o lcd_print.o lcd_pic_draw.o lcd_font_en.o lcd_font_num.o lcd_font_sign.o lcd_drv.o i2c_drv.o ledcnt_drv.o mipi_dsi_drv.o lcd_panel_drv.o port_io_drv.o

.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)             : $(TARGET).mk $(OPT)
lcd_api.o               : com_type.h common.h string.h lcd_common.h lcd_api.h lcd_drv.h ledcnt_drv.h timer_drv.h lcd_panel_drv.h lcd.h
lcd_print.o         : com_type.h string.h lcd_api.h lcd.h lcd_font_en.h lcd_font_num.h lcd_font_sign.h
lcd_pic_draw.o    : com_type.h string.h lcd_api.h lcd.h
lcd_font_en.o       : lcd_font_en.h
lcd_font_num.o      : lcd_font_num.h
lcd_font_sign.o     : lcd_font_sign.h
lcd_drv.o           : lcd_common.h mipi_dsi_drv.h lcd_panel_drv.h timer_drv.h ledcnt_drv.h i2c_drv.h lcd_drv.h
i2c_drv.o           : lcd_common.h timer_drv.h i2c_drv.h
ledcnt_drv.o        : lcd_common.h timer_drv.h i2c_drv.h ledcnt_drv.h
mipi_dsi_drv.o      : lcd_common.h timer_drv.h mipi_dsi_drv.h
lcd_panel_drv.o     : lcd_common.h timer_drv.h mipi_dsi_drv.h ledcnt_drv.h
port_io_drv.o       : port_io_drv.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------

include $(VIA_DIR)/loader_lib.mk
