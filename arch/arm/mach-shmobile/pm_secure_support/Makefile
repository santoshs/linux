#
# Makefile for the linux kernel.
#

# PM with secure OS support
obj-$(CONFIG_SUSPEND)			+= suspend.o
obj-$(CONFIG_CPU_IDLE)			+= cpuidle.o pmCoreStandby.o
obj-$(CONFIG_PM_SECURE_HAL_DUMMY)	+= sec_hal.o

ccflags-$(CONFIG_PM_TEST)	:= -DDEBUG
