# ramset program

TARGET = ramset

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0   = ../src
INC1   = ../inc
INC2   = ../../inc
INC    = -I$(INC0) -I$(INC1) -I$(INC2)
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

OBJS    = 	ramset.o \
			ramset_entry.o

LIBSDIR = ../../libs
LIBS    = $(LIBSDIR)/common.a\
          $(LIBSDIR)/hw_init.a
          

.PHONY : all allclean clean

all: $(TARGET).bin
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)         :$(TARGET).mk $(OPT)
ramset_entry.o	:
ramset.o 		:ramset_ram.h
                 

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
include $(VIA_DIR)/loader_bin.mk
