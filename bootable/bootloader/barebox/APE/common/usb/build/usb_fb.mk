#  usb.mk


#----------------------------------------------------------------
# target
#----------------------------------------------------------------
TARGET=usb_fb
ULIB = $(TARGET).a

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0   = ../inc
INC1   = ../inc/api
INC2   = ../inc/hal
INC3   = ../inc/usb_spec
INC4   = ../src/usbdrv/dev_stack/usbhs
INC5   = ../src/usbdrv/dev_class/comm_class/comm_acm
INC6   = ../../../inc
INC    = -I$(INC0) -I$(INC1) -I$(INC2) -I$(INC3) -I$(INC4) -I$(INC5) -I$(INC6)
SRC    = ../src
#####USBDEVICE#####
SRCUSBCM  = ../src/usbdrv/common/os_dep
SRCUSBHS  = ../src/usbdrv/dev_stack/usbhs
SRCVCOM	  = ../src/usbdrv/dev_class/comm_class/comm_acm

VPATH = $(INC0):$(INC1):$(INC2):$(INC3):$(INC4):$(INC5):$(INC6):$(SRC):$(SRCUSBCM):$(SRCUSBHS):$(SRCVCOM)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../../via
OPT     = $(VIA_DIR)/loader_opt.mk

include $(OPT)
#----------------------------------------------------------------
# object
#----------------------------------------------------------------
#####USBDEVICE#####
USBOBJ	=	dev_api.o \
			hiae_dep_lib.o \
			usbfunc_hs.o \
			usbf_api.o \
			c_global.o \
			c_dataio.o \
			c_intrn.o \
			c_lib596.o \
			c_libassp.o \
			c_usbint.o \
			c_usbsig.o \
			p_changeep.o \
			p_classvendor.o \
			p_controlrw.o \
			p_intrn.o \
			p_lib596.o \
			p_libassp.o \
			p_status.o \
			p_stdreqget.o \
			p_stdreqset.o \
			p_usbint.o \
			p_usbsig.o

### USER object ###
USROBJ	=	usb_init.o \
			usb_open.o \
			usb_close.o \
			usb_check.o \
			usb_receive.o \
			usb_send.o \
			usb_phy_init.o	\
			usb_phy_read.o	\
			usb_phy_write.o	\
			usb_common.o \
			usb_descriptor_fb.o \
			comm_acm_dev_vcom.o \
			comm_acm_dev_api.o \
			comm_acm_dev_mcpc.o \
			comm_acm_dev_tools.o \
			usb_workmem.o \
			string.o

OBJS	=	$(USROBJ) \
			$(USBOBJ)

.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)             : $(TARGET).mk $(OPT)
dev_api.o           : usbd_def.h usbd_types.h usbd_system.h usb.h dev_api.h dev_globals.h \
                      usbfunc_hs.h usb_download.h
hiae_dep_lib.o      : types.h usbd_def.h usbd_system.h
usbfunc_hs.o        : usbd_def.h usbd_types.h usbd_system.h usb.h dev_api.h dev_globals.h usbfunc_hs.h \
                      usbf_api.h c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h
usbf_api.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h usbf_api.h \
                      c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_global.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h p_def_ep.h p_descrip.h \
                      c_def596.h usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_dataio.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h usbf_api.h \
                      c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_intrn.o           : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_lib596.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_libassp.o         : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_usbint.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
c_usbsig.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_changeep.o        : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_classvendor.o     : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_controlrw.o       : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_intrn.o           : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_lib596.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_libassp.o         : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_status.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_stdreqget.o       : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_stdreqset.o       : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_usbint.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h
p_usbsig.o          : c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h c_def596.h \
                      usbf_api.h c_typedef.h c_macusr.h c_libassp.h c_extern.h

usb_init.o          : usb_api.h cpu_register.h com_type.h
usb_phy_init.o      : usb_api.h usb_private.h cpu_register.h com_type.h
usb_phy_read.o      : usb_api.h usb_private.h cpu_register.h com_type.h
usb_phy_write.o     : usb_api.h usb_private.h cpu_register.h com_type.h
usb_open.o          : usb_api.h usb_private.h usb_download.h comm_acm_dev_api.h comm_acm_dev_vcom.h \
                      usbf_api.h c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h com_type.h error_base.h
usb_close.o         : usb_api.h usb_private.h com_type.h
usb_check.o         : usb_api.h usb_private.h usb_download.h comm_acm_dev_vcom.h com_type.h
usb_receive.o       : usb_api.h usb_private.h usb_download.h comm_acm_dev_vcom.h com_type.h
usb_send.o          : usb_api.h usb_private.h usb_download.h comm_acm_dev_vcom.h com_type.h
usb_common.o        : usb_api.h usb_private.h usb_download.h comm_acm_dev_api.h usbf_api.h \
                      c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h com_type.h error_base.h
usb_descriptor_fb.o    : 
comm_acm_dev_vcom.o : usbd_def.h usbd_types.h usb_download.h usbd_system.h usb.h \
                      usb_comm.h dev_api.h comm_acm_dev_api.h comm_acm_dev_globals.h \
                      comm_acm_dev_vcom.h usb_module_control.h error_base.h
comm_acm_dev_api.o  : usbd_def.h usbd_types.h usbd_system.h usb.h usb_comm.h \
                      dev_api.h comm_acm_dev_api.h usb_download.h comm_acm_dev_vcom.h \
                      comm_acm_dev_globals.h usb_module_control.h error_base.h
comm_acm_dev_mcpc.o : usbd_def.h usbd_types.h usbd_system.h usb.h usb_comm.h \
                      dev_api.h comm_acm_dev_api.h comm_acm_dev_globals.h error_base.h
comm_acm_dev_tools.o: usbd_def.h usbd_types.h usbd_system.h usb.h usb_comm.h dev_api.h \
                      comm_acm_dev_api.h comm_acm_dev_globals.h usb_download.h comm_acm_dev_vcom.h \
                      usbf_api.h c_typedef.h c_defusr.h c_macusr.h c_libassp.h c_extern.h error_base.h
usb_workmem.o       : types.h usbd_def.h usbd_types.h usb_download.h usbd_system.h \
                      usb.h usb_comm.h dev_api.h comm_acm_dev_api.h comm_acm_dev_vcom.h error_base.h
string.o            :

#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------
# Exception for fastboot only

%.o : %.s
	$(AS) $(ASFLAGS) $< -o $(@F)

%.o : %.c
	$(CC) $(CCFLAGS) -c -o $(@F) $<

$(ULIB): $(OBJS)
	$(AR) $(AROPT) $(ULIB) $(OBJS)
	cp ./$(ULIB) $(VIA_DIR)/../libs/

clean:
	$(RM) usb_descriptor_fb.o
	
allclean:
	$(RM) $(VIA_DIR)/../libs/$(ULIB)
	$(RM) $(ULIB)
	$(RM) usb_descriptor_fb.o