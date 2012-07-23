#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0   = ../inc
INC1   = ../../../../libs
INC2   = ../../../../inc
INC3   = ../../../emmc/inc
INC    = -I$(INC0) -I$(INC1) -I$(INC2) -I$(INC3)
SRC0   = ../src/driver
SRC1   = ../src/wrapper
SRC    = $(SRC0),$(SRC1)

VPATH = $(INC0):$(INC1):$(INC2):$(INC3):$(SRC0):$(SRC1)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../../../via
OPT		= $(VIA_DIR)/loader_opt.mk\
		  $(VIA_DIR)/loader_link_opt.mk

include $(OPT)

#----------------------------------------------------------------
# object
#----------------------------------------------------------------
ULIB = FlashAccessApi.a

### USER object ###
USROBJ	=	emmc_common.o\
            emmc_init.o\
            emmc_mount.o\
            emmc_read.o\
            emmc_write.o\
            emmc_erase.o\
            emmc_format.o\
            emmc_write_protect.o\
            emmc_clear_protect.o\
            flash_common.o\
            flash_init.o\
            flash_mount.o\
            flash_unmount.o\
            flash_read.o\
            flash_write.o\
            flash_erase.o\
            flash_format.o\
            flash_write_protect.o\
            flash_clear_protect.o

OBJS	=	$(USROBJ)

.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)         :emmc.mk $(OPT)
emmc_common.o   :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_init.o     :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_mount.o    :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_read.o     :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_write.o    :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_erase.o    :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_format.o   :com_type.h\
                 emmc.h\
                 emmc_private.h

emmc_write_protect.o :com_type.h\
                      emmc.h\
                      emmc_private.h\
                      emmc_protect_common.h

emmc_clear_protect.o :protect_option.h\
                      com_type.h\
                      emmc.h\
                      emmc_private.h\
                      emmc_protect_common.h

flash_common.o  :flash_api.h\
                 flash_private.h\
                 emmc.h

flash_init.o    :flash_api.h\
                 flash_private.h\
                 emmc.h

flash_mount.o   :flash_api.h\
                 flash_private.h\
                 emmc.h

flash_unmount.o :flash_api.h\
                 flash_private.h\
                 emmc.h

flash_read.o    :flash_api.h\
                 flash_private.h\
                 emmc.h

flash_write.o   :flash_api.h\
                 flash_private.h\
                 emmc.h
                 
flash_erase.o   :flash_api.h\
                 flash_private.h\
                 emmc.h
                 
flash_format.o  :flash_api.h\
                 flash_private.h\
                 emmc.h

flash_write_protect.o  :protect_option.h\
                        flash_api.h\
                        flash_private.h\
                        emmc.h

flash_clear_protect.o  :protect_option.h\
                        flash_api.h\
                        flash_private.h\
                        emmc.h

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------

include $(VIA_DIR)/loader_lib.mk
