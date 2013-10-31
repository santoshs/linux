#
# Makefile for the linux kernel.
#

# Common objects
# obj-y				:= timer.o console.o clock.o pm_runtime.o
obj-y				:= timer.o console.o clock.o cmt.o serial.o
#obj-y                           += tz_comm/
# CPU objects
obj-$(CONFIG_ARCH_SH7372)	+= setup-sh7372.o clock-sh7372.o intc-sh7372.o
obj-$(CONFIG_ARCH_SH73A0)	+= setup-sh73a0.o clock-sh73a0.o intc-sh73a0.o
obj-$(CONFIG_ARCH_R8A7373)	+= setup-r8a7373.o clock-r8a7373.o intc-r8a7373.o sbsc.o
obj-$(CONFIG_ARCH_R8A73A4)	+= setup-r8a73a4.o clock-r8a73a4.o
obj-$(CONFIG_ARCH_R8A7740)	+= setup-r8a7740.o clock-r8a7740.o intc-r8a7740.o
obj-$(CONFIG_ARCH_R8A7778)	+= setup-r8a7778.o clock-r8a7778.o
obj-$(CONFIG_ARCH_R8A7779)	+= setup-r8a7779.o clock-r8a7779.o intc-r8a7779.o
obj-$(CONFIG_ARCH_R8A7790)	+= setup-r8a7790.o clock-r8a7790.o
obj-$(CONFIG_ARCH_EMEV2)	+= setup-emev2.o clock-emev2.o

# SMP objects
smp-y				:= platsmp.o headsmp.o
smp-$(CONFIG_ARCH_SH73A0)	+= smp-sh73a0.o headsmp-scu.o
smp-$(CONFIG_ARCH_R8A7373)	+= smp-r8a7373.o headsmp-scu.o
smp-$(CONFIG_ARCH_R8A7779)	+= smp-r8a7779.o headsmp-scu.o
smp-$(CONFIG_ARCH_EMEV2)	+= smp-emev2.o headsmp-scu.o

# IRQ objects
obj-$(CONFIG_ARCH_SH7372)	+= entry-intc.o
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
obj-$(CONFIG_ARCH_SHMOBILE)	+= pm-rmobile.o
obj-y						+= poweroff.o
obj-$(CONFIG_PDC) 			+= powerdomain.o
obj-$(CONFIG_CPU_FREQ) 		+= cpufreq.o clocksuspend.o

ccflags-$(CONFIG_PM_TEST)	:= -DDEBUG
obj-$(CONFIG_ARCH_SH7372)	+= pm-sh7372.o sleep-sh7372.o
obj-$(CONFIG_ARCH_R8A7373)	+= pm-r8a7373.o sleep-r8a7373.o
obj-$(CONFIG_ARCH_R8A7740)	+= pm-r8a7740.o
obj-$(CONFIG_ARCH_R8A7779)	+= pm-r8a7779.o
obj-$(CONFIG_ARCH_SH73A0)	+= pm-sh73a0.o
# boot stop sysfs
obj-$(CONFIG_PM_BOOT_SYSFS)	+= pm_boot_sysfs.o

# Board objects
obj-$(CONFIG_MACH_AP4EVB)	+= board-ap4evb.o
obj-$(CONFIG_MACH_AG5EVM)	+= board-ag5evm.o
obj-$(CONFIG_MACH_APE6EVM)	+= board-ape6evm.o
obj-$(CONFIG_MACH_MACKEREL)	+= board-mackerel.o
obj-$(CONFIG_MACH_KOTA2)	+= board-kota2.o
obj-$(CONFIG_MACH_BONITO)	+= board-bonito.o
obj-$(CONFIG_MACH_BOCKW)	+= board-bockw.o
obj-$(CONFIG_MACH_MARZEN)	+= board-marzen.o
obj-$(CONFIG_MACH_MARZEN_REFERENCE)	+= board-marzen-reference.o
obj-$(CONFIG_MACH_LAGER)	+= board-lager.o
obj-$(CONFIG_MACH_ARMADILLO800EVA)	+= board-armadillo800eva.o
obj-$(CONFIG_MACH_KZM9D)	+= board-kzm9d.o
obj-$(CONFIG_MACH_KZM9G)	+= board-kzm9g.o
obj-$(CONFIG_MACH_KZM9G_REFERENCE)	+= board-kzm9g-reference.o
obj-$(CONFIG_MACH_LOGANLTE)	+= board-loganlte.o
obj-$(CONFIG_MACH_AMETHYST)	+= board-amethyst.o

obj-$(CONFIG_ARCH_R8A7373)	+= setup-u2stm.o
obj-$(CONFIG_MACH_U2USB)	+= setup-u2usb.o

# BT object for renesas reference implementation
obj-$(CONFIG_RENESAS_BT)	+= dev-renesas-bt.o

# BT object
obj-$(CONFIG_BT_BCM4330)		+= board-bcm4334-bt.o
obj-$(CONFIG_BT_BCM4334)		+= board-bcm4334-bt.o
obj-$(CONFIG_BT_BCM43241)		+= board-bcm4334-bt.o
obj-$(CONFIG_BCM4334_BT)		+= board-bcm4334-bt.o

# Framework support
obj-$(CONFIG_SMP)		+= $(smp-y)

# LED support
#obj-y	+= led-control.o
#obj-y	+= flash-control.o

# RAM Defragmentation
obj-$(CONFIG_ARCH_R8A7373)	+= ramdefrag/

# crashlog
obj-y += crashlog.o

obj-$(CONFIG_SEC_DEBUG)			+= sec_debug.o sec_gaf.o sec_getlog.o sec-common.o sec-reboot.o
obj-$(CONFIG_SEC_DEBUG_INFORM)	+= sec_debug_inform.o

obj-$(CONFIG_U2_TOUCHPANEL) += dev-touchpanel.o
obj-$(CONFIG_U2_TOUCHKEY) += setup-u2touchkey.o

#touchscreen BCM15xx
#obj-$(CONFIG_TOUCHSCREEN_BCMTCH15XXX) += dev-touchpanel-bcmtch15xxx.o
obj-y += dev-touchpanel-bcmtch15xxx.o

# touchscreen
obj-$(CONFIG_TOUCHSCREEN_ATMEL_MXT) += setup-u2mxt224.o

# vibrator
obj-y +=  setup-u2vibrator.o

# charging
ifeq ($(CONFIG_SEC_CHARGING_FEATURE),y)
obj-$(CONFIG_MACH_LOGANLTE)	+= board-loganlte-spa.o
obj-$(CONFIG_MACH_AMETHYST)	+= board-amethyst-spa.o
endif

# common timer setup
obj-$(CONFIG_U2_TIMERS)	+= setup-u2timers.o
# sdhi
obj-$(CONFIG_U2_SDHI)	+= setup-u2sdhi.o

# gpio_key and keysc
obj-$(CONFIG_U2_GPIOKEY)	+= setup-u2gpio_key.o
# WiFi objects
obj-$(CONFIG_ARCH_SHMOBILE)	+= dev-wifi.o

# gpio
obj-y						+= gpio.o

# setup-d2153
obj-$(CONFIG_MFD_D2153)	+= setup-d2153.o
ifeq ($(CONFIG_REGULATOR_D2153),y)
obj-$(CONFIG_MACH_LOGANLTE) += loganlte-ldoconfig.o
obj-$(CONFIG_MACH_AMETHYST) += loganlte-ldoconfig.o
endif

#setup-audio
obj-$(CONFIG_SND_SOC_SH4_FSI)	+= setup-u2audio.o

# GPS
obj-$(CONFIG_GPS_CSR_GSD5T) += dev-gps.o
obj-$(CONFIG_GPS_BCM4752) += dev-gps.o

# NFC
obj-$(CONFIG_BCMI2CNFC)+= dev-nfc.o
obj-$(CONFIG_PN547_NFC)+= dev-nfc.o

# NFC for renesas reference implementation
obj-$(CONFIG_NFC_PN547)+= dev-nfc.o

# MHL
# obj-$(CONFIG_SAMSUNG_MHL)	+= dev-mhl.o
obj-$(CONFIG_SAMSUNG_MHL)	+= dev-edid.o
obj-$(CONFIG_SAMSUNG_MHL)	+= dev-mhl-sii8332.o

# Sensor
obj-$(CONFIG_AMETHYST_SENSOR)	+= dev-sensor.o
obj-$(CONFIG_SAMSUNG_SENSOR)   += dev-sensor.o

# rcu
obj-$(CONFIG_VIDEO_SH_MOBILE_RCU) += setup-u2rcu.o

# csi2
obj-$(CONFIG_VIDEO_SH_MOBILE_CSI2) += setup-u2csi2.o

# ion
obj-$(CONFIG_ION) += setup-u2ion.o

# camera
obj-$(CONFIG_SOC_CAMERA) += setup-u2camera.o

# Ramdump
obj-$(CONFIG_SH_RAMDUMP)	+= ramdump.o

# Samsung features
include arch/arm/mach-shmobile/Makefile.sec

