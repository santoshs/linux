# hw_init.mk


#----------------------------------------------------------------
# target
#----------------------------------------------------------------
TARGET=hw_init
ULIB = $(TARGET).a

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0    = ../src
INC1    = ../../../inc
INC     = -I$(INC0) -I$(INC1)
VPATH   = $(INC0):$(INC1)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../../via
OPT     = $(VIA_DIR)/loader_opt.mk

include $(OPT)
#----------------------------------------------------------------
# object
#----------------------------------------------------------------
OBJS        = cpg.o gpio.o i2c.o i2cm.o sbsc.o sysc.o boot_init.o

.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)             : $(TARGET).mk $(OPT)
cpg.o        : com_type.h compile_option.h common.h cpg.h
gpio.o       : com_type.h compile_option.h gpio.h 
i2c.o        : com_type.h i2c.h gpio.h cpg.h 
i2cm.o       : com_type.h i2cm.h gpio.h cpg.h log_output.h
sbsc.o       : com_type.h common.h sbsc.h cpg.h
sysc.o       : com_type.h common.h sbsc.h cpg.h sysc.h
boot_init.o  : com_type.h common.h cpg.h gpio.h i2c.h sbsc.h boot_init.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------

include $(VIA_DIR)/loader_lib.mk
