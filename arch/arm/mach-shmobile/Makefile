#
# Makefile for the linux kernel.
#

# Common objects
# obj-y				:= timer.o console.o clock.o pm_runtime.o
obj-y				:= timer.o console.o clock.o
obj-$(CONFIG_SH_TIMER_CMT_ARM)	+= sh_cmt.o
obj-y                           += tz_comm/
# CPU objects
obj-$(CONFIG_ARCH_SH7367)	+= setup-sh7367.o clock-sh7367.o intc-sh7367.o
obj-$(CONFIG_ARCH_SH7377)	+= setup-sh7377.o clock-sh7377.o intc-sh7377.o
obj-$(CONFIG_ARCH_SH7372)	+= setup-sh7372.o clock-sh7372.o intc-sh7372.o
obj-$(CONFIG_ARCH_SH73A0)	+= setup-sh73a0.o clock-sh73a0.o intc-sh73a0.o
obj-$(CONFIG_ARCH_R8A7740)	+= setup-r8a7740.o clock-r8a7740.o intc-r8a7740.o
obj-$(CONFIG_ARCH_R8A7779)	+= setup-r8a7779.o clock-r8a7779.o intc-r8a7779.o
obj-$(CONFIG_ARCH_R8A7373)	+= setup-r8a7373.o clock-r8a7373.o intc-r8a7373.o sbsc.o

# SMP objects
smp-y				:= platsmp.o headsmp.o
smp-$(CONFIG_HOTPLUG_CPU)	+= hotplug.o
smp-$(CONFIG_ARCH_SH73A0)	+= smp-sh73a0.o
smp-$(CONFIG_ARCH_R8A7779)	+= smp-r8a7779.o
smp-$(CONFIG_ARCH_R8A7373)	+= smp-r8a7373.o

# Pinmux setup
pfc-y				:=
pfc-$(CONFIG_ARCH_SH7367)	+= pfc-sh7367.o
pfc-$(CONFIG_ARCH_SH7377)	+= pfc-sh7377.o
pfc-$(CONFIG_ARCH_SH7372)	+= pfc-sh7372.o
pfc-$(CONFIG_ARCH_SH73A0)	+= pfc-sh73a0.o
pfc-$(CONFIG_ARCH_R8A7740)	+= pfc-r8a7740.o
pfc-$(CONFIG_ARCH_R8A7779)	+= pfc-r8a7779.o
pfc-$(CONFIG_ARCH_R8A7373)	+= pfc-r8a7373.o

# IRQ objects
obj-$(CONFIG_ARCH_SH7367)	+= entry-intc.o
obj-$(CONFIG_ARCH_SH7377)	+= entry-intc.o
obj-$(CONFIG_ARCH_SH7372)	+= entry-intc.o
obj-$(CONFIG_ARCH_SH73A0)	+= entry-gic.o
obj-$(CONFIG_ARCH_R8A7373)	+= entry-gic.o
obj-$(CONFIG_ARCH_R8A7740)	+= entry-intc.o

# PM objects
ifeq ($(CONFIG_CPU_IDLE),y)
	ifeq ($(CONFIG_PM_HAS_SECURE),y)
	obj-$(CONFIG_CPU_IDLE)		+= pmCoreStandby_tz.o cpuidle.o pm.o
	else
	obj-$(CONFIG_CPU_IDLE)		+= pmCoreStandby.o cpuidle.o pm.o
	endif
else
	ifeq ($(CONFIG_PM_HAS_SECURE),y)
	obj-$(CONFIG_SUSPEND)		+= pmCoreStandby_tz.o pm.o
	else
	obj-$(CONFIG_SUSPEND)		+= pmCoreStandby.o pm.o
	endif
endif
obj-$(CONFIG_SUSPEND)		+= suspend.o

obj-$(CONFIG_ARCH_SH7372)	+= pm-sh7372.o sleep-sh7372.o
obj-y						+= poweroff.o
obj-$(CONFIG_PDC) 			+= powerdomain.o
obj-$(CONFIG_CPU_FREQ) 		+= cpufreq.o clocksuspend.o

ccflags-$(CONFIG_PM_TEST)	:= -DDEBUG
obj-$(CONFIG_ARCH_R8A7779)	+= pm-r8a7779.o
obj-$(CONFIG_ARCH_R8A7373)	+= pm-r8a7373.o sleep-r8a7373.o

# Board objects
obj-$(CONFIG_MACH_G3EVM)	+= board-g3evm.o
obj-$(CONFIG_MACH_G4EVM)	+= board-g4evm.o
obj-$(CONFIG_MACH_AP4EVB)	+= board-ap4evb.o
obj-$(CONFIG_MACH_AG5EVM)	+= board-ag5evm.o
obj-$(CONFIG_MACH_MACKEREL)	+= board-mackerel.o
# obj-$(CONFIG_RENESAS_WIFI)	+= board-u2evm-renesas-wifi.o
obj-$(CONFIG_MACH_KOTA2)	+= board-kota2.o
obj-$(CONFIG_MACH_BONITO)	+= board-bonito.o
obj-$(CONFIG_MACH_MARZEN)	+= board-marzen.o
obj-$(CONFIG_MACH_U2EVM)	+= board-u2evm.o
obj-$(CONFIG_MACH_U2USB)	+= setup-u2usb.o
# BT object
obj-$(CONFIG_RENESAS_BT)	+= board-u2evm-renesas-bt.o

# Framework support
obj-$(CONFIG_SMP)		+= $(smp-y)
obj-$(CONFIG_GENERIC_GPIO)	+= $(pfc-y)

# LED support
#obj-y	+= led-control.o
#obj-y	+= flash-control.o

# RAM Defragmentation
obj-$(CONFIG_ARCH_R8A73734)	+= ramdefrag/
# crashlog
obj-y += crashlog.o

obj-$(CONFIG_SEC_DEBUG)			+= sec_debug.o sec_gaf.o sec_getlog.o sec-common.o sec-reboot.o
obj-$(CONFIG_SEC_DEBUG_INFORM)	+= sec_debug_inform.o
# touchkey
obj-y += setup-u2touchkey.o
# touchscreen
obj-y += setup-u2mxt224.o
# vibrator
obj-$(CONFIG_VIBRATOR_ISA1000A)	+=  setup-u2vibrator.o
# charging
obj-$(CONFIG_SEC_CHARGING_FEATURE) += setup-u2smb328a.o
# current timer
obj-$(CONFIG_MACH_U2EVM)	+= setup-u2current_timer.o
# sdhi
obj-$(CONFIG_MACH_U2EVM)	+= setup-u2sdhi.o
# gpio_key and keysc
obj-$(CONFIG_MACH_U2EVM)	+= setup-u2gpio_key.o
#setup-u2tps80032
obj-$(CONFIG_MACH_U2EVM)	+= setup-u2tps80032.o
# WiFi objects
# obj-$(CONFIG_ARCH_SHMOBILE) +=  board-renesas_wifi.o
obj-$(CONFIG_RENESAS_WIFI) +=  board-renesas_wifi.o
# gpio
obj-$(CONFIG_GENERIC_GPIO)	+= gpio.o
