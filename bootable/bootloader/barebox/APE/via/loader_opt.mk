#----------------------------------------------------------------
# command
#----------------------------------------------------------------
RM = rm
COPY = cp

#----------------------------------------------------------------
# command
#----------------------------------------------------------------
CC = arm-eabi-gcc
AS = arm-eabi-as
AR = arm-eabi-ar

#----------------------------------------------------------------
# compile option
#----------------------------------------------------------------
CCOPT = -g -O1 -fno-common -ffixed-r8 -msoft-float -fno-builtin -ffreestanding -nostdinc -isystem -I$(pwd)/lib/gcc/arm-eabi/4.4.3/include -pipe -marm  -mabi=aapcs-linux -mno-thumb-interwork -Wall -Wstrict-prototypes -fno-stack-protector -mcpu=cortex-a9

ASOPT = -g -mcpu=cortex-a9

AROPT = crv

#----------------------------------------------------------------
# define option
#----------------------------------------------------------------
ifeq ($(DUPDATER),UPDATER)
DEFOPT0 = -D__UPDATER__
else
DEFOPT0 = 
endif

ifeq ($(RL_LCD_ENABLE),TRUE)
DEFOPT1 = -D__RL_LCD_ENABLE__
else
DEFOPT1 = 
endif

ifeq ($(FW_LCD_ENABLE),TRUE)
DEFOPT2 = -D__FW_LCD_ENABLE__
else
DEFOPT2 = 
endif

ifeq ($(INTEGRITY_CHECK_ENABLE),TRUE)
DEFOPT3 = -D__INTEGRITY_CHECK_ENABLE__ -DLOADER -DTRUSTZONE
else
DEFOPT3 = 
endif

ifeq ($(LCD_CONFIG),OPT_qHD)
DEFOPT4 = -D__OPT_qHD__
else
DEFOPT4 = 
endif

ifeq ($(BOOTLOG_ENABLE),TRUE)
DEFOPT5 = -D__BOOTLOG_ENABLE__ 
else
DEFOPT5 = 
endif

DEFOPT = $(DEFOPT0) $(DEFOPT1) $(DEFOPT2) $(DEFOPT3) $(DEFOPT4) $(DEFOPT5) 
#----------------------------------------------------------------
# Build options.
#----------------------------------------------------------------
CCFLAGS		= $(CCOPT) $(INC) $(DEFOPT)
ASFLAGS		= $(ASOPT) $(INC)
