#
# Makefile for the linux kernel.
#

# Common objects
# obj-y				:= timer.o console.o clock.o pm_runtime.o
obj-y				:= timer.o console.o clock.o
obj-$(CONFIG_SH_TIMER_CMT_ARM)	+= sh_cmt.o

# CPU objects
obj-$(CONFIG_ARCH_SH7367)	+= setup-sh7367.o clock-sh7367.o intc-sh7367.o
obj-$(CONFIG_ARCH_SH7377)	+= setup-sh7377.o clock-sh7377.o intc-sh7377.o
obj-$(CONFIG_ARCH_SH7372)	+= setup-sh7372.o clock-sh7372.o intc-sh7372.o
obj-$(CONFIG_ARCH_SH73A0)	+= setup-sh73a0.o clock-sh73a0.o intc-sh73a0.o
obj-$(CONFIG_ARCH_R8A73734)	+= setup-r8a73734.o clock-r8a73734.o intc-r8a73734.o

# SMP objects
smp-y				:= platsmp.o headsmp.o
smp-$(CONFIG_HOTPLUG_CPU)	+= hotplug.o
smp-$(CONFIG_LOCAL_TIMERS)	+= localtimer.o
smp-$(CONFIG_ARCH_SH73A0)	+= smp-sh73a0.o
smp-$(CONFIG_ARCH_R8A73734)	+= smp-r8a73734.o

# Pinmux setup
pfc-y				:=
pfc-$(CONFIG_ARCH_SH7367)	+= pfc-sh7367.o
pfc-$(CONFIG_ARCH_SH7377)	+= pfc-sh7377.o
pfc-$(CONFIG_ARCH_SH7372)	+= pfc-sh7372.o
pfc-$(CONFIG_ARCH_SH73A0)	+= pfc-sh73a0.o
pfc-$(CONFIG_ARCH_R8A73734)	+= pfc-r8a73734.o

# IRQ objects
obj-$(CONFIG_ARCH_SH7367)	+= entry-intc.o
obj-$(CONFIG_ARCH_SH7377)	+= entry-intc.o
obj-$(CONFIG_ARCH_SH7372)	+= entry-intc.o
obj-$(CONFIG_ARCH_SH73A0)	+= entry-gic.o
obj-$(CONFIG_ARCH_R8A73734)	+= entry-gic.o

# PM objects
ifeq ($(CONFIG_PM_HAS_SECURE),y)
obj-y						+= pm_secure_support/
else
obj-$(CONFIG_SUSPEND)		+= suspend.o
obj-$(CONFIG_CPU_IDLE)		+= cpuidle.o pmCoreStandby.o
endif
obj-$(CONFIG_ARCH_SH7372)	+= pm-sh7372.o sleep-sh7372.o
obj-y						+= poweroff.o
obj-$(CONFIG_PDC) 			+= powerdomain.o
obj-$(CONFIG_CPU_FREQ) 		+= cpufreq.o clocksuspend.o

ccflags-$(CONFIG_PM_TEST)	:= -DDEBUG

# Board objects
obj-$(CONFIG_MACH_G3EVM)	+= board-g3evm.o
obj-$(CONFIG_MACH_G4EVM)	+= board-g4evm.o
obj-$(CONFIG_MACH_AP4EVB)	+= board-ap4evb.o
obj-$(CONFIG_MACH_AG5EVM)	+= board-ag5evm.o
obj-$(CONFIG_MACH_MACKEREL)	+= board-mackerel.o
obj-$(CONFIG_MACH_U2EVM)	+= board-u2evm.o

# Framework support
obj-$(CONFIG_SMP)		+= $(smp-y)
obj-$(CONFIG_GENERIC_GPIO)	+= $(pfc-y)

# LED support
#obj-y	+= led-control.o
#obj-y	+= flash-control.o

# RAM Defragmentation
obj-$(CONFIG_MACH_U2EVM)	+= ramdefrag/
