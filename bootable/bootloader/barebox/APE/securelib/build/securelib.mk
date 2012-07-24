#
#

#----------------------------------------------------------------
# target
#----------------------------------------------------------------
TARGET=securelib
ULIB = $(TARGET).a

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0    = ../src
INC1    = ../../inc
INC2    = ../inc
INC     = -I$(INC0) -I$(INC1) -I$(INC2)
VPATH   = $(INC0):$(INC1):$(INC2)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../via
OPT     = $(VIA_DIR)/loader_opt.mk

include $(OPT)
#----------------------------------------------------------------
# object
#----------------------------------------------------------------
OBJS        = hw_arm.o sec_bridge_tz.o sec_dispatch.o malloc.o hw_l2.o sec_msg.o security.o

.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)             : $(TARGET).mk $(OPT)
hw_arm.o:
sec_bridge_tz.o  :
sec_dispatch.o	: sec_dispatch.h libc.h
malloc.o	: malloc.h
hw_l2.o	: hw_arm.h
sec_msg.o	: sec_msg.h
security.o	: security.h


#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------

include $(VIA_DIR)/loader_lib.mk
