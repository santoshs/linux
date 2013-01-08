/*
 * arch/arm/mach-shmobile/suspend.c
 *
 * Copyright (C) 2011 Magnus Damm
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>
#include <mach/pm.h>
#include <mach/io.h>
#include <linux/delay.h>
#include <linux/hwspinlock.h>
#include <mach/sbsc.h>
#include <linux/wakelock.h>
#ifndef CONFIG_PM_HAS_SECURE
#include "pm_ram0.h"
#else /*CONFIG_PM_HAS_SECURE*/
#include "pm_ram0_tz.h"
#endif /*CONFIG_PM_HAS_SECURE*/
#include "pmRegisterDef.h"

#include <mach/ram_defrag.h>

#define pm_writeb(v, a)			__raw_writeb(v, (void *__iomem)a)
#define pm_writew(v, a)			__raw_writew(v, (void *__iomem)a)
#define pm_writel(v, a)			__raw_writel(v, (void *__iomem)a)
#define pm_readb(a)				__raw_readb((void *__iomem)a)
#define pm_readw(a)				__raw_readw((void *__iomem)a)
#define pm_readl(a)				__raw_readl((void *__iomem)a)
#define DO_SAVE_REGS(array)		do_save_regs(array, ARRAY_SIZE(array))
#define DO_RESTORE_REGS(array)	do_restore_regs(array, ARRAY_SIZE(array))

#ifndef CONFIG_PM_HAS_SECURE
#define RAM_ARM_VECT                   ram1BasePhys
#else /*CONFIG_PM_HAS_SECURE*/
#define RAM_ARM_VECT                   ram0ArmVectorPhys
#endif /*CONFIG_PM_HAS_SECURE*/

#define PMDBG_PRFX				"PM-DBG: "

/* Enable/disable PASR for SDRAM */
#define PASR_SUPPORT

enum {
	IRQC_EVENTDETECTOR_BLK0 = 0,
	IRQC_EVENTDETECTOR_BLK1,
	IRQC_EVENTDETECTOR_BLK10,
	IRQC_EVENTDETECTOR_BLK11,
	IRQC_EVENTDETECTOR_BLK12,
	HSGPR,
	SYSGPR,
	HPB,
	SHWYSTATHS,
	SHWYSTATSY,
	SHWYSTATDM,
	SHBUF,
#ifdef CONFIG_PM_HAS_SECURE
	SBSC_SDCR0A_TZ,
	SBSC_SDWCRC0A_TZ,
	SBSC_SDWCRC1A_TZ,
	SBSC_SDWCR00A_TZ,
	SBSC_SDWCR01A_TZ,
	SBSC_SDWCR10A_TZ,
	SBSC_SDWCR11A_TZ,
	SBSC_SDWCRC2A_TZ,
#endif
};

static suspend_state_t shmobile_suspend_state;
static int not_core_shutdown;

static unsigned int save_sbar_val;

static int log_wakeupfactor;
module_param(log_wakeupfactor, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int es;

static char xtal1_log_out;

/*Change clocks function*/
unsigned int frqcrA_save;
unsigned int frqcrB_save;
unsigned int frqcrD_save;
unsigned int is_clock_updated;
#define CLOCK_SUSPEND		0
#define CLOCK_RESTORE		1
#define ZB3_CLK_SUSPEND		130000

#ifdef CONFIG_PM_DEBUG
/*
 * Dynamic on/off for System Suspend
 *   0: System Suspend is disable
 *   1: System Suspend in enable
 */
static int enable_module = 1;
static DEFINE_SPINLOCK(systemsuspend_lock);
#endif	/* CONFIG_PM_DEBUG */
struct base_map {
	unsigned long phys;	/* phys base  */
	int size;			/* remap size */
	unsigned long base;	/* virt base  */
};

struct reg_info {
	unsigned long *vbase;
	unsigned long offset;
	int size;
	int esrev;
	unsigned int val;
};

#define PM_SAVE_REG(type, of, sz, rev)	\
{								\
	.vbase  = &map[type].base,	\
	.offset = of,				\
	.size   = sz,				\
	.esrev  = rev,				\
}

static struct base_map map[] = {
	[IRQC_EVENTDETECTOR_BLK0] = {
		.phys = IRQC_EVENTDETECTOR_BLK0_BASE,
		.size = SZ_4K,
		.base = IRQC_EVENTDETECTOR_BLK0_BASE,
	},
	[IRQC_EVENTDETECTOR_BLK1] = {
		.phys = IRQC_EVENTDETECTOR_BLK1_BASE,
		.size = SZ_4K,
		.base = IRQC_EVENTDETECTOR_BLK1_BASE,
	},
	[IRQC_EVENTDETECTOR_BLK10] = {
		.phys = IRQC_EVENTDETECTOR_BLK10_BASE,
		.size = SZ_4K,
		.base = IRQC_EVENTDETECTOR_BLK10_BASE,
	},
	[IRQC_EVENTDETECTOR_BLK11] = {
		.phys = IRQC_EVENTDETECTOR_BLK11_BASE,
		.size = SZ_4K,
		.base = IRQC_EVENTDETECTOR_BLK11_BASE,
	},
	[IRQC_EVENTDETECTOR_BLK12] = {
		.phys = IRQC_EVENTDETECTOR_BLK12_BASE,
		.size = SZ_4K,
		.base = IRQC_EVENTDETECTOR_BLK12_BASE,
	},
	[HSGPR] = {
		.phys = HSGPR_BASE_PHYS,
		.size = SZ_4K,
		.base = 0x0,	/* Allocate at boot time */
	},
	[SYSGPR] = {
		.phys = SYSGPR_BASE_PHYS,
		.size = SZ_4K,
		.base = 0x0,	/* Allocate at boot time */
	},
	[HPB] = {
		.phys = HPB_BASE,
		.size = SZ_8K,
		.base = HPB_BASE,
	},
	[SHWYSTATHS] = {
		.phys = SHWYSTATHS_BASE,
		.size = SZ_4K,
		.base = SHWYSTATHS_BASE,
	},
	[SHWYSTATSY] = {
		.phys = SHWYSTATSY_BASE,
		.size = SZ_4K,
		.base = SHWYSTATSY_BASE,
	},
	[SHWYSTATDM] = {
		.phys = SHWYSTATDM_BASE,
		.size = SZ_4K,
		.base = 0x0,	/* Allocate at boot time */
	},
	[SHBUF] = {
		.phys = SHBUF_BASE,
		.size = SZ_4K,
		.base = SHBUF_BASE,
	},
#ifdef CONFIG_PM_HAS_SECURE
	[SBSC_SDCR0A_TZ] = {	/* for setclock */
		.phys = SBSC_SDCR0APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},

	[SBSC_SDWCRC0A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCRC0APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
	[SBSC_SDWCRC1A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCRC1APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
	[SBSC_SDWCR00A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCR00APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
	[SBSC_SDWCR01A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCR01APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
	[SBSC_SDWCR10A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCR10APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
	[SBSC_SDWCR11A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCR11APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
	[SBSC_SDWCRC2A_TZ] = {	/* for setclock */
		.phys = SBSC_SDWCRC2APhys,
		.size = SZ_4,	/* 4 bytes */
		.base = 0x0,	/* Allocate at boot time */
	},
#endif
};
static struct reg_info irqx_eventdetectors_regs[] = {
/* IRQC Event Detector Block_0  */
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_00,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_01,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_02,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_03,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_04,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_05,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_06,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_07,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_08,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_09,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_10,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_11,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_12,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_13,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_14,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_15,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_16,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_17,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_18,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_19,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_20,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_21,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_22,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_23,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_24,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_25,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_26,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_27,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_28,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_29,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_30,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK0, CONFIG_31,   32, ES_REV_ALL),
/* IRQC Event Detector Block_1  */
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_00,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_01,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_02,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_03,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_04,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_05,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_06,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_07,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_08,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_09,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_10,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_11,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_12,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_13,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_14,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_15,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_16,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_17,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_18,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_19,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_20,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_21,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_22,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_23,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_24,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_25,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_26,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_27,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_28,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_29,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_30,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK1, CONFIG_31,   32, ES_REV_ALL),
/* IRQC Event Detector Block_10  */
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_00,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_01,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_02,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_03,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_04,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_05,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_06,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_07,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_08,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_09,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_10,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_11,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_12,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_13,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_14,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_15,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_16,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_17,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_18,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_19,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_20,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_21,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_22,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_23,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_24,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_25,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_26,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_27,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_28,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_29,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_30,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK10, CONFIG_31,   32, ES_REV_ALL),
/* IRQC Event Detector Block_11  */
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_00,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_01,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_02,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_03,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_04,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_05,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_06,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_07,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_08,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_09,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_10,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_11,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_12,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_13,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_14,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_15,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_16,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_17,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_18,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_19,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_20,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_21,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_22,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_23,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_24,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_25,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_26,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_27,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_28,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_29,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_30,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK11, CONFIG_31,   32, ES_REV_ALL),
/* IRQC Event Detector Block_12  */
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_00,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_01,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_02,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_03,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_04,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_05,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_06,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_07,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_08,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_09,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_10,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_11,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_12,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_13,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_14,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_15,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_16,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_17,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_18,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_19,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_20,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_21,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_22,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_23,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_24,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_25,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_26,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_27,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_28,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_29,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_30,   32, ES_REV_ALL),
	PM_SAVE_REG(IRQC_EVENTDETECTOR_BLK12, CONFIG_31,   32, ES_REV_ALL),
};

static struct reg_info shwy_regs[] = {
/* SHBUF */
	PM_SAVE_REG(SHBUF, SHBMCTR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMAR,     32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBARCR11,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBARCR12,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMCTR2,   32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR00,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR00,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR00, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER00, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR01,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR01,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR01, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER01, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR02,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR02,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR02, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER02, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR03,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR03,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR03, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER03, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR04,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR04,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR04, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER04, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR05,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR05,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR05, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER05, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR06,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR06,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR06, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER06, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBADDR07,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBMSKR07,  32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBCHCTR07, 32, ES_REV_ALL),
	PM_SAVE_REG(SHBUF, SHBSIZER07, 32, ES_REV_ALL),
/* HS GPR */
	PM_SAVE_REG(HSGPR, HSPRPRICR,    32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR11,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR12,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR13,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR14,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR31,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR32,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR33,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRARCR34,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRERRMSK,   32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRPRICNT11, 32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRPRICNT12, 32, ES_REV_ALL),
	PM_SAVE_REG(HSGPR, HSPRPRICNT13, 32, ES_REV_ALL),
/* Sys GPR */
	PM_SAVE_REG(SYSGPR, SYPRPRICR,  32, ES_REV_ALL),
	PM_SAVE_REG(SYSGPR, SYPRARCR11, 32, ES_REV_ALL),
	PM_SAVE_REG(SYSGPR, SYPRARCR12, 32, ES_REV_ALL),
	PM_SAVE_REG(SYSGPR, SYPRARCR13, 32, ES_REV_ALL),
	PM_SAVE_REG(SYSGPR, SYPRERRMSK, 32, ES_REV_ALL),
/* HPB */
	PM_SAVE_REG(HPB, HPBCTRL1,      32, ES_REV_ALL),
	PM_SAVE_REG(HPB, HPBCTRL2,      32, ES_REV_ALL),
	PM_SAVE_REG(HPB, HPBCTRL4,      32, ES_REV_ALL),
	PM_SAVE_REG(HPB, HPBCTRL5,      32, ES_REV_ALL),
	PM_SAVE_REG(HPB, HPBCTRL7,      32, ES_REV_ALL),
	PM_SAVE_REG(HPB, OCPBRGWIN1,    32, ES_REV_ALL),
	PM_SAVE_REG(HPB, OCPBRGWIN2,    32, ES_REV_ALL),
	PM_SAVE_REG(HPB, OCPBRGWIN3,    32, ES_REV_ALL),
/* SHWYSTAT HS */
	PM_SAVE_REG(SHWYSTATHS, SHSTxCR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxIR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxDMR,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxCNT,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxTN,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxTR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxAM11,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxAM12,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxTM1,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxAM21,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxAM22,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxTM2,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxATRM1, 32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATHS, SHSTxATRM2, 32, ES_REV_ALL),
/* SHWYSTAT SYS */
	PM_SAVE_REG(SHWYSTATSY, SHSTxCR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxIR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxDMR,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxCNT,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxTN,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxTR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxAM11,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxAM12,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxTM1,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxAM21,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxAM22,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxTM2,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxATRM1, 32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATSY, SHSTxATRM2, 32, ES_REV_ALL),
};

struct reg_info shwystatdm_regs[] = {
/* SHWYSTAT DM */
	PM_SAVE_REG(SHWYSTATDM, SHSTxCR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxIR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxDMR,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxCNT,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxTN,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxTR,    32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxAM11,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxAM12,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxTM1,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxAM21,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxAM22,  32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxTM2,   32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxATRM1, 32, ES_REV_ALL),
	PM_SAVE_REG(SHWYSTATDM, SHSTxATRM2, 32, ES_REV_ALL),
};

/*
 * Helper functions for system suspend
 */
static void wakeups_factor(void)
{
	unsigned int dummy;

	/* clear */
	dummy = __raw_readl(WUPSFAC);

	if (log_wakeupfactor == 1) {
		pr_debug(PMDBG_PRFX "WakeUpFactor Value = 0x%08x\n", \
					dummy);
		pr_debug(PMDBG_PRFX "WakeUpS Mask Value = 0x%08x\n", \
					__raw_readl(WUPSMSK));

		/* For IRQ0,IRQ1 wakeup factors */
		if ((dummy & 0x40) != 0)
			pr_debug(PMDBG_PRFX " Wakeup by IRQ[31:0]: 0x%08x\n", \
					__raw_readl(ram0_ICSPISR0Phys));
		else if ((dummy & 0x80) != 0)
			pr_debug(PMDBG_PRFX " Wakeup by IRQ[63:32]: 0x%08x\n",\
					__raw_readl(ram0_ICSPISR1Phys));
		else
			pr_debug(PMDBG_PRFX "Not wakeup by IRQ wakeup factors.\n");
	}
}

/*
 * Helper functions for getting platform suspend state
 */
suspend_state_t get_shmobile_suspend_state(void)
{
	return shmobile_suspend_state;
}

/*
 * Helper functions for saving IP registers value
 */
static void do_save_regs(struct reg_info *regs, int count)
{
	struct reg_info *info = regs;
	int i;

	for (i = 0; i < count; i++, *info++) {
		if (!*info->vbase)
			continue;
		if ((info->esrev & es) == es) {
			switch (info->size) {
			case 8:
				info->val = pm_readb(*info->vbase + \
							info->offset);
				break;
			case 16:
				info->val = pm_readw(*info->vbase + \
							info->offset);
				break;
			case 32:
				info->val = pm_readl(*info->vbase + \
							info->offset);
				break;
			default:
				break;
			}
		}
	}
}

/*
 * Helper functions for restoring IP registers value
 */
static void do_restore_regs(struct reg_info *regs, int count)
{
	struct reg_info *info;
	int i;

	info = regs + count;

	for (i = count; i > 0; i--) {
		*info--;
		if (!*info->vbase)
			continue;
		if ((info->esrev & es) == es) {
			switch (info->size) {
			case 8:
				pm_writeb(info->val, (*info->vbase + \
						info->offset));
				break;
			case 16:
				pm_writew(info->val, (*info->vbase + \
						info->offset));
				break;
			case 32:
				pm_writel(info->val, (*info->vbase + \
						info->offset));
				break;
			default:
				break;
			}
		}
	}
}

static void irqx_eventdetectors_regs_save(void)
{
	DO_SAVE_REGS(irqx_eventdetectors_regs);
}

static void shwy_regs_save(void)
{
	DO_SAVE_REGS(shwy_regs);
}

void shwystatdm_regs_save(void)
{
	DO_SAVE_REGS(shwystatdm_regs);
}

static void irqx_eventdetectors_regs_restore(void)
{
	DO_RESTORE_REGS(irqx_eventdetectors_regs);
}

static void shwy_regs_restore(void)
{
	DO_RESTORE_REGS(shwy_regs);
}

void shwystatdm_regs_restore(void)
{
	DO_RESTORE_REGS(shwystatdm_regs);
}

/*
 * Helper functions for checking CPU status
 */
static int core_shutdown_status(unsigned int cpu)
{
	if (es == ES_REV_1_0)
		return (__raw_readl(CPG_SCPUSTR) >> (4 * cpu)) & 3;
	else
		return (__raw_readl(CPG_SCPUSTR) >> (4 * cpu)) & 3;
}

/*
 * System suspend callback functions' implementation
 */
static int shmobile_suspend_begin(suspend_state_t state)
{
	shmobile_suspend_state = state;
	if (get_shmobile_suspend_state() & PM_SUSPEND_MEM)
		is_suspend_request = 1;

#if 1
	int ret;
	/* set DFS mode */
	ret = suspend_cpufreq();
	if (ret != 0) {
		pr_debug(PMDBG_PRFX "%s: suspend_cpufreq() returns %d.\n", \
				__func__, ret);
	}
#endif
	return 0;
}

static void shmobile_suspend_end(void)
{
	shmobile_suspend_state = PM_SUSPEND_ON;
	is_suspend_request = 0;

	if (not_core_shutdown) {
		pr_debug(PMDBG_PRFX "%s: CPU0 waited until the CPU1 power down.\n", \
				__func__);
		not_core_shutdown = 0;
	}

#if 1
	int ret;
	ret = resume_cpufreq();
	if (ret != 0) {
		pr_debug(PMDBG_PRFX "%s: resume_cpufreq() returns %d.\n", \
				__func__, ret);
	}
#endif
}

static void do_iicdvm_setting(void)
{
	pr_debug("Setting IICDVM\n");
	__raw_writeb(0x80, ICCRDVM);
	__raw_writeb(0x00, ICTMC1DVM1);
	__raw_writeb(0x00, ICTMC2DVM1);
	__raw_writeb(0x01, ICTMCWDVM1);
	__raw_writeb(0x03, ICICDVM1);
	__raw_writeb(0x90, ICACEDVM1);
	__raw_writeb(0x00, ICIMSKDVM1);
	__raw_writeb(0x00, ICATFRDVM1);
	__raw_writeb(0x10, ICVCONDVM1);
	__raw_writeb(0x02, ICATSET1DVM1);

	pr_debug("Set IICDVM communication data\n");
	__raw_writeb(0xFE, ICATD00DVM12);
	__raw_writeb(0xFE, ICATD00DVM13);
	__raw_writeb(0xFF, ICATD01DVM12);
	__raw_writeb(0xFF, ICATD01DVM13);
	__raw_writeb(0xFF, ICATD02DVM12);
	__raw_writeb(0xFF, ICATD02DVM13);

	pr_debug("Set IICDVM start transmission\n\n");
	__raw_writeb(0x80, ICASTARTDVM1);
}

#define Enable_PM_Test_Mode  0

#if Enable_PM_Test_Mode
#define Enable_Change_Pin  0

typedef struct {
	unsigned long addr;
	unsigned long val;
	unsigned long size;
	char sz[50];
	unsigned long saved;
} port_ctrl_register_t;

static port_ctrl_register_t port_ctrl_register[] =
{
	{ __io(0xe6050000), 0x10,  8, " Port0  MSECURE "			, 0},
	{ __io(0xe6050001), 0xA0,  8, " Port1   "                  , 0},
	{ __io(0xe6050002), 0xA0,  8, " Port2   "                  , 0},
	{ __io(0xe6050003), 0x10,  8, " Port3  CAM_PWR_EN "        , 0},
	{ __io(0xe6050004), 0x00,  8, " Port4  CAM_PWR_SCL "       , 0},
	{ __io(0xe6050005), 0x00,  8, " Port5  CAM_PWR_SDA "       , 0},
	{ __io(0xe6050006), 0xE0,  8, " Port6  OLED_DET "          , 0},
	{ __io(0xe6050007), 0xA0,  8, " Port7  OLED_ID "           , 0},
	{ __io(0xe6050008), 0x10,  8, " Port8  MAIN_MIC_LDO_EN "   , 0},
	{ __io(0xe6050009), 0x10,  8, " Port9  SENSOR_LDO_EN "     , 0},
	{ __io(0xe605000A), 0x10,  8, " Port10  GPS_RST_N "        , 0},
	{ __io(0xe605000B), 0x10,  8, " Port11  GPS_EN "           , 0},
	{ __io(0xe605000C), 0x10,  8, " Port12  NFC_EN "           , 0},
	{ __io(0xe605000D), 0xA0,  8, " Port13  NFC_IRQ "          , 0},
	{ __io(0xe605000E), 0x00,  8, " Port14  MHL_SCL_1.8V "     , 0},
	{ __io(0xe605000F), 0x00,  8, " Port15  MHL_SDA_1.8V "     , 0},
	{ __io(0xe6050010), 0x10,  8, " Port16  CAM1_RST_N "       , 0},
	{ __io(0xe6050011), 0x10,  8, " Port17  SUB_MIC_LDO_EN "   , 0},
	{ __io(0xe6050012), 0xA0,  8, " Port18  UICC1_LDO_VSELL "  , 0},
	{ __io(0xe6050013), 0xA0,  8, " Port19  TP3008 "           , 0},
	{ __io(0xe6050014), 0x10,  8, " Port20  CAM0_RESET_N "     , 0},
	{ __io(0xe6050015), 0xA0,  8, " Port21   "                 , 0},
	{ __io(0xe6050016), 0xA0,  8, " Port22   "                 , 0},
	{ __io(0xe6050017), 0xA0,  8, " Port23   "                 , 0},
	{ __io(0xe6050018), 0xA0,  8, " Port24  EAR_SEND_END "     , 0},
	{ __io(0xe6050019), 0x10,  8, " Port25  VBUS_EN "          , 0},
	{ __io(0xe605001A), 0x00,  8, " Port26  TCKEY_SCL "        , 0},
	{ __io(0xe605001B), 0x00,  8, " Port27  TCKEY_SDA "        , 0},
	{ __io(0xe605001D), 0x10,  8, " Port29  TCKEY_LDO_EN "     , 0},
	{ __io(0xe605001E), 0x10,  8, " Port30  TSP_LDO_EN "       , 0},
	{ __io(0xe6050020), 0xa0,  8, " Port32  TSP_INT "          , 0},
	{ __io(0xe6050021), 0x10,  8, " Port33  LCD_TE "           , 0},
	{ __io(0xe6050023), 0xD0,  8, " Port35  PMIC_RST_N "       , 0},
	{ __io(0xe6050024), 0xA0,  8, " Port36  VIB_PWM1 "         , 0},
	{ __io(0xe6050025), 0x11,  8, " Port37  BT_RTS "           , 0},
	{ __io(0xe6050026), 0xE1,  8, " Port38  BT_CTS "           , 0},
	{ __io(0xe6050027), 0x10,  8, " Port39  RST_N_MHL "        , 0},
	{ __io(0xe6050050), 0x10,  8, " Port80  SPI_MOSI "         , 0},
	{ __io(0xe6050051), 0xE0,  8, " Port81  SPI_MISO "         , 0},
	{ __io(0xe6050052), 0x10,  8, " Port82  SPI_SCLK "         , 0},
	{ __io(0xe6050053), 0xA0,  8, " Port83  SPI_SSN "          , 0},
	{ __io(0xe6050054), 0x00,  8, " Port84  TSP_SCL "          , 0},
	{ __io(0xe6050055), 0x00,  8, " Port85  TSP_SDA "          , 0},
	{ __io(0xe6050056), 0x00,  8, " Port86  AF_SCL "           , 0},
	{ __io(0xe6050057), 0x00,  8, " Port87  AF_SDA "           , 0},
	{ __io(0xe6050058), 0xA0,  8, " Port88  TP_GPIO3001 "      , 0},
	{ __io(0xe6050059), 0x10,  8, " Port89  LCD_PWR_EN "       , 0},
	{ __io(0xe60520DB), 0x01,  8, " Port219  HDMI_CLKO "       , 0},
	{ __io(0xe605005A), 0xA0,  8, " Port90   "                 , 0},
	{ __io(0xe605005B), 0x10,  8, " Port91  CAM1_STBY "        , 0},
	{ __io(0xe605002c), 0xA0,  8, " Port44   "                 , 0},
	{ __io(0xe60520D7), 0x01,  8, " Port215  CAM0_MCLK "       , 0},
	{ __io(0xe60520D8), 0x01,  8, " Port216  CAM1_MCLK "       , 0},
	{ __io(0xe6052111), 0x00,  8, " Port273  NFC_SCL "         , 0},
	{ __io(0xe6052112), 0x00,  8, " Port274  NFC_SDA "         , 0},
	{ __io(0xe6052113), 0xA0,  8, " Port275  NC "              , 0},
	{ __io(0xe6052114), 0xA0,  8, " Port276  NC "              , 0},
	{ __io(0xe6052115), 0xA0,  8, " Port277  NC "              , 0},
	{ __io(0xe6052105), 0xA1,  8, " Port261  CODEC_I2S_DI "    , 0},
	{ __io(0xe6052107), 0x91,  8, " Port263  CODEC_I2S_SYNC "  , 0},
	{ __io(0xe6052108), 0x91,  8, " Port264  CODEC_I2S_CLK "   , 0},
	{ __io(0xe6052109), 0x91,  8, " Port265  CODEC_I2S_DO "    , 0},
	{ __io(0xe6052137), 0xA0,  8, " Port311  NC "              , 0},
	{ __io(0xe6052138), 0xA0,  8, " Port312  NC "              , 0},
	{ __io(0xe60520DA), 0x01,  8, " Port218  CODEC_MCLK "      , 0},
	{ __io(0xe6051089), 0x11,  8, " Port137  BT_TXD "          , 0},
	{ __io(0xe605108A), 0xE1,  8, " Port138  BT_RXD "          , 0},
	{ __io(0xe605108C), 0x11,  8, " Port140  GPS_TSYNC "       , 0},
	{ __io(0xe60520C6), 0xA0,  8, " Port198  LTE_WIFI_COEX(0) ", 0},
	{ __io(0xe60520C7), 0xA0,  8, " Port199  LTE_WIFI_COEX(1) ", 0},
	{ __io(0xe60520C8), 0xA0,  8, " Port200  LTE_WIFI_COEX(2) ", 0},
	{ __io(0xe60520C9), 0xA0,  8, " Port201  LTE_WIFI_COEX(3) ", 0},
	{ __io(0xe6052104), 0x10,  8, " Port260  WLAN_EN "         , 0},
	{ __io(0xe6052106), 0x10,  8, " Port262  BT_WAKEUP "       , 0},
	{ __io(0xe605210A), 0xA1,  8, " Port266  BT_I2S_DI "       , 0},
	{ __io(0xe605210B), 0x91,  8, " Port267  BT_I2S_SYNC "     , 0},
	{ __io(0xe605210C), 0x10,  8, " Port268  BT_EN "           , 0},
	{ __io(0xe605210D), 0x91,  8, " Port269  BT_I2S_CLK "      , 0},
	{ __io(0xe605210E), 0x91,  8, " Port270  BT_I2S_DO "       , 0},
	{ __io(0xe605210F), 0xA0,  8, " Port271  NC "              , 0},
	{ __io(0xe6052110), 0x20,  8, " Port272  BT_HOST_WAKEUP "  , 0},
	{ __io(0xe6052120), 0x01,  8, " Port288  WLAN_SDIO_CLK "   , 0},
	{ __io(0xe6052121), 0xc1,  8, " Port289  WLAN_SDIO_DAT0 "  , 0},
	{ __io(0xe6052122), 0xc1,  8, " Port290  WLAN_SDIO_DAT1 "  , 0},
	{ __io(0xe6052123), 0xc1,  8, " Port291  WLAN_SDIO_DAT2 "  , 0},
	{ __io(0xe6052124), 0xc1,  8, " Port292  WLAN_SDIO_DAT3 "  , 0},
	{ __io(0xe6052125), 0xc1,  8, " Port293  WLAN_SDIO_CMD "   , 0},
	{ __io(0xe6052126), 0xA0,  8, " Port294   "                , 0},
	{ __io(0xe6052127), 0xA0,  8, " Port295   "                , 0},
	{ __io(0xe6052128), 0xA0,  8, " Port296   "                , 0},
	{ __io(0xe6052129), 0xA0,  8, " Port297   "                , 0},
	{ __io(0xe605212A), 0xA0,  8, " Port298   "                , 0},
	{ __io(0xe605212B), 0xA0,  8, " Port299   "                , 0},
	{ __io(0xe605212C), 0x01,  8, " Port300  EMMC(0) "         , 0},
	{ __io(0xe605212D), 0x01,  8, " Port301  EMMC(1) "         , 0},
	{ __io(0xe605212E), 0x01,  8, " Port302  EMMC(2) "         , 0},
	{ __io(0xe605212F), 0x01,  8, " Port303  EMMC(3) "         , 0},
	{ __io(0xe6052130), 0x01,  8, " Port304  EMMC(4) "         , 0},
	{ __io(0xe6052131), 0x01,  8, " Port305  EMMC(5) "         , 0},
	{ __io(0xe6052132), 0x01,  8, " Port306  EMMC(6) "         , 0},
	{ __io(0xe6052133), 0x01,  8, " Port307  EMMC(7) "         , 0},
	{ __io(0xe6052134), 0x01,  8, " Port308  EMMC_CMD "        , 0},
	{ __io(0xe6052135), 0x01,  8, " Port309  EMMC_CLK "        , 0},
	{ __io(0xe6052136), 0x11,  8, " Port310  EMMC_RST_N "      , 0},
	{ __io(0xe605002D), 0xE0,  8, " Port45  HOME_KEY "         , 0},
	{ __io(0xe605002E), 0xE0,  8, " Port46  VOL_UP "           , 0},
	{ __io(0xe605002F), 0xE0,  8, " Port47  VOL_DN "           , 0},
	{ __io(0xe6050030), 0x10,  8, " Port48  ECLK_EN_GPS "      , 0},
	{ __io(0xe6050060), 0xE0,  8, " Port96  GPS_INT "          , 0},
	{ __io(0xe6050061), 0xE0,  8, " Port97  MUS_INT "          , 0},
	{ __io(0xe6050062), 0xA0,  8, " Port98  WLAN_OOB_IRQ "     , 0},
	{ __io(0xe6050063), 0x10,  8, " Port99  CAM_FLASH_SET "    , 0},
	{ __io(0xe6050064), 0x10,  8, " Port100  CAM_FLASH_EN "    , 0},
	{ __io(0xe6050065), 0x10,  8, " Port101  NFC_FIRMWARE "    , 0},
	{ __io(0xe6050066), 0x10,  8, " Port102  MHL_EN "          , 0},
	{ __io(0xe6050067), 0xE0,  8, " Port103  CHG_INT "         , 0},
	{ __io(0xe6050068), 0xA0,  8, " Port104  TCK_INT "         , 0},
	{ __io(0xe6050069), 0xE0,  8, " Port105  FG_INT "          , 0},
	{ __io(0xe605006B), 0xA0,  8, " Port107  GYRO_INT "        , 0},
	{ __io(0xe605006C), 0x20,  8, " Port108  PROXI_INT "       , 0},
	{ __io(0xe605006D), 0xE0,  8, " Port109  MHL_INT "         , 0},
	{ __io(0xe605006E), 0xA0,  8, " Port110  LTE_WIFI_COEX(4) ", 0},
	{ __io(0xe605108B), 0x11,  8, " Port139  DIG_RFEN "        , 0},
	{ __io(0xe6052140), 0xC1,  8, " Port320  SDHI0_DATA_0 "    , 0},
	{ __io(0xe6052141), 0xC1,  8, " Port321  SDHI0_DATA_1 "    , 0},
	{ __io(0xe6052142), 0xC1,  8, " Port322  SDHI0_DATA_2 "    , 0},
	{ __io(0xe6052143), 0xC1,  8, " Port323  SDHI0_DATA_3 "    , 0},
	{ __io(0xe6052144), 0xC1,  8, " Port324  SDHI0_CMD "       , 0},
	{ __io(0xe6052145), 0xC1,  8, " Port325  NC "              , 0},
	{ __io(0xe6052146), 0x01,  8, " Port326  SDHI0_CLK "       , 0},
	{ __io(0xe6052147), 0xE1,  8, " Port327  SDHI0_CD "        , 0},
	{ __io(0xe6050040), 0xA0,  8, " Port64  NC "               , 0},
	{ __io(0xe6050041), 0xA0,  8, " Port65  NC "               , 0},
	{ __io(0xe6050042), 0xA0,  8, " Port66  NC "               , 0},
	{ __io(0xe6050046), 0xA0,  8, " Port70  NC "               , 0},
	{ __io(0xe6050047), 0xA0,  8, " Port71  NC "               , 0},
	{ __io(0xe6051082), 0x90,  8, " Port130  ULPI_CS "         , 0},
	{ __io(0xe6051083), 0x90,  8, " Port131  ULPI_RST "        , 0},
	{ __io(0xe60520CB), 0x00,  8, " Port203  ULPID(0) "        , 0},
	{ __io(0xe60520CC), 0x00,  8, " Port204  ULPID(1) "        , 0},
	{ __io(0xe60520CD), 0x00,  8, " Port205  ULPID(2) "        , 0},
	{ __io(0xe60520CE), 0x00,  8, " Port206  ULPID(3) "        , 0},
	{ __io(0xe60520CF), 0x00,  8, " Port207  ULPID(4) "        , 0},
	{ __io(0xe60520D0), 0x00,  8, " Port208  ULPID(5) "        , 0},
	{ __io(0xe60520D1), 0x00,  8, " Port209  ULPID(6) "        , 0},
	{ __io(0xe60520D2), 0x00,  8, " Port210  ULPID(7) "        , 0},
	{ __io(0xe60520D3), 0x00,  8, " Port211  ULPI_CLK "        , 0},
	{ __io(0xe60520D4), 0x00,  8, " Port212  ULPI_STP "        , 0},
	{ __io(0xe60520D5), 0x00,  8, " Port213  ULPI_DIR "        , 0},
	{ __io(0xe60520D6), 0x00,  8, " Port214  ULPI_NXT "        , 0},
	{ __io(0xe60520D9), 0x00,  8, " Port217  ULPI_REF_CLK "    , 0},
	{ __io(0xe6050048), 0xA0,  8, " Port72  HW_REV0 "          , 0},
	{ __io(0xe6050049), 0xA0,  8, " Port73  HW_REV1 "          , 0},
	{ __io(0xe605004A), 0xA0,  8, " Port74  HW_REV2 "          , 0},
	{ __io(0xe605004B), 0xA0,  8, " Port75  HW_REV3 "          , 0},
	{ __io(0xe605004C), 0x00,  8, " Port76  GPS_RTS "          , 0},
	{ __io(0xe605004D), 0x00,  8, " Port77  GPS_CTS "          , 0},
	{ __io(0xe605004E), 0x00,  8, " Port78  GPS_TXD "          , 0},
	{ __io(0xe605004F), 0x80,  8, " Port79  GPS_RXD "          , 0},
	{ __io(0xe6050028), 0xA0,  8, " Port40   "                 , 0},
	{ __io(0xe6050029), 0xA0,  8, " Port41   "                 , 0},
	{ __io(0xe605002A), 0xA0,  8, " Port42   "                 , 0},
	{ __io(0xe605002B), 0xA0,  8, " Port43   "                 , 0},
	{ __io(0xe605108E), 0xA0,  8, " Port142   "                , 0},
	{ __io(0xe6051085), 0xA1,  8, " Port133  MODECTRL(1) "     , 0},
	{ __io(0xe6051086), 0xA1,  8, " Port134  MODECTRL(2) "     , 0},
	{ __io(0xe6051087), 0xE1,  8, " Port135  MODECTRL(3) "     , 0},
	{ __io(0xe6051088), 0xA1,  8, " Port136  MODECTRL(4) "     , 0},
	{ __io(0xe6051080), 0x00,  8, " Port128  UART_TXD "        , 0},
	{ __io(0xe6051081), 0x00,  8, " Port129  UART_RXD "        , 0},
	{ __io(0xe60520E0), 0xA0,  8, " Port224  TP_Oen "          , 0},
	{ __io(0xe60520E1), 0xA0,  8, " Port225  TP_Wen "          , 0},
	{ __io(0xe60520E2), 0x10,  8, " Port226  VIB_ON "          , 0},
	{ __io(0xe60520E3), 0x10,  8, " Port227  CS0n "            , 0},
	{ __io(0xe60520E4), 0xA0,  8, " Port228  VIB_PWM1 "        , 0},
	{ __io(0xe60520E5), 0xA0,  8, " Port229  CS4n "            , 0},
	{ __io(0xe60520E6), 0xA0,  8, " Port230  DEBUGCTRL(5) "    , 0},
	{ __io(0xe60520E7), 0xA0,  8, " Port231  DEBUGCTRL(1) "    , 0},
	{ __io(0xe60520E8), 0xA0,  8, " Port232  MD(15) "          , 0},
	{ __io(0xe60520E9), 0xA0,  8, " Port233  MD(14) "          , 0},
	{ __io(0xe60520EA), 0xA0,  8, " Port234  MD(13) "          , 0},
	{ __io(0xe60520EB), 0xA0,  8, " Port235  MD(12) "          , 0},
	{ __io(0xe60520EC), 0xA0,  8, " Port236  MD(11) "          , 0},
	{ __io(0xe60520ED), 0xA0,  8, " Port237  MD(10) "          , 0},
	{ __io(0xe60520EE), 0xA0,  8, " Port238  MD(9) "           , 0},
	{ __io(0xe60520EF), 0xA0,  8, " Port239  MD(8) "           , 0},
	{ __io(0xe60520F0), 0xA0,  8, " Port240  MD(7) "           , 0},
	{ __io(0xe60520F1), 0xA0,  8, " Port241  MD(6) "           , 0},
	{ __io(0xe60520F2), 0xA0,  8, " Port242  MD(5) "           , 0},
	{ __io(0xe60520F3), 0xA0,  8, " Port243  MD(4) "           , 0},
	{ __io(0xe60520F4), 0xA0,  8, " Port244  MD(3) "           , 0},
	{ __io(0xe60520F5), 0xA0,  8, " Port245  MD(2) "           , 0},
	{ __io(0xe60520F6), 0xA0,  8, " Port246  MD(1) "           , 0},
	{ __io(0xe60520F7), 0xA0,  8, " Port247  MD(0) "           , 0},
	{ __io(0xe60520F8), 0x10,  8, " Port248   "                , 0},
	{ __io(0xe60520F9), 0XE0,  8, " Port249  NC "              , 0},
	{ __io(0xe60520FA), 0XE0,  8, " Port250  AD(9) "           , 0},
	{ __io(0xe60520FB), 0XE0,  8, " Port251  AD(8) "           , 0},
	{ __io(0xe60520FC), 0XE0,  8, " Port252  AD(7) "           , 0},
	{ __io(0xe60520FD), 0XE0,  8, " Port253  AD(6) "           , 0},
	{ __io(0xe60520FE), 0XE0,  8, " Port254  AD(5) "           , 0},
	{ __io(0xe60520FF), 0XE0,  8, " Port255  AD(4) "           , 0},
	{ __io(0xe6052100), 0XE0,  8, " Port256  AD(3) "           , 0},
	{ __io(0xe6052101), 0XE0,  8, " Port257  AD(2) "           , 0},
	{ __io(0xe6052102), 0xA0,  8, " Port258  AD(1) "           , 0},
	{ __io(0xe6052103), 0xA0,  8, " Port259  ADVn "            , 0},
};

static void check_n_save_pin_conf( void )
{
	int i;
	port_ctrl_register_t* p_reg;
	int cnt = (sizeof(port_ctrl_register)/sizeof(port_ctrl_register[0]));

		for( i=0; i<cnt; i++ )
	{
		p_reg = &port_ctrl_register[i];
		switch( p_reg->size )
		{
			case 8  : p_reg->saved = __raw_readb( p_reg->addr ); break;
			case 16 : p_reg->saved = __raw_readw( p_reg->addr ); break;
			case 32 : p_reg->saved = __raw_readl( p_reg->addr ); break;
		}
		if( p_reg->saved != p_reg->val )
		{
			printk("[%s] expected=0x%x, cur val=0x%x\n", 
				p_reg->sz, 
				p_reg->val, 
				p_reg->saved );
		#if Enable_Change_Pin
			switch( p_reg->size )
			{
				case 8  : __raw_writeb( p_reg->val, p_reg->addr ); break;
				case 16 : __raw_writew( p_reg->val, p_reg->addr ); break;
				case 32 : __raw_writel( p_reg->val, p_reg->addr ); break;
			}
		#endif // #if Enable_Change_Pin
		}
	}
}

static void restore_pin_conf( void )
{
	int i;
	port_ctrl_register_t* p_reg;
	int cnt = (sizeof(port_ctrl_register)/sizeof(port_ctrl_register[0]));

	for( i=0; i<cnt; i++ )
	{
		p_reg = &port_ctrl_register[i];
		switch( p_reg->size )
		{ 
			case 8   : __raw_writeb( p_reg->saved, p_reg->addr ); break;
			case 16  : __raw_writew( p_reg->saved, p_reg->addr ); break;
			case 32  : __raw_writel( p_reg->saved, p_reg->addr ); break;
		}
	}
}

const unsigned long set_register[] =
{
	0xE6054100, /* PORTD31_0DSR */
	0xE6054104, /* PORTD63_32DSR */
	0xE6054108, /* PORTD95_64DSR */
	0xE605410C, /* PORTD127_96DSR */
	0xE6055100, /* PORTD159_128DSR */
	0,
	0xE6056100, /* PORTD223_192DSR */
	0xE6056104, /* PORTD255_224DSR */
	0xE6056108, /* PORTD287_256DSR */
	0xE605610C, /* PORTD319_288DSR */
	0xE6056110, /* PORTD351_320DSR */
};

const unsigned long clr_register[] = {
	0xE6054200, /* PORTD31_0DCR */
	0xE6054204, /* PORTD63_32DCR */
	0xE6054208, /* PORTD95_64DCR */
	0xE605420C, /* PORTD127_96DCR */
	0xE6055200, /* PORTD159_128DCR */
	0,
	0xE6056200, /* PORTD223_192DCR */
	0xE6056204, /* PORTD255_224DCR */
	0xE6056208, /* PORTD287_256DCR */
	0xE605620C, /* PORTD319_288DCR */
	0xE6056210, /* PORTD351_320DCR */
};

void _gpio_set( int port_num, int val )
{
	int register_val;
	int bit;

	// check whether port is gpio function or not
	
	
	if( val )
	{
		// high
		register_val = set_register[port_num/32];
		bit = port_num%32;
	}
	else
	{
		// low
		register_val = clr_register[port_num/32];
		bit = port_num%32;
	}
	//printk("[%d] register_val=0x%x, bit=0x%x. INX=%d\n", port_num, register_val, bit, port_num/32 );
	__raw_writel( 0x1<<bit, __io(IO_ADDRESS(register_val)) );
}

#endif // #if Enable_PM_Test_Mode

/*
 * suspend_set_clock
 *
 * Arguments:
 *		@is_restore:
 *			0: set clock when suspending
 *			1: restore clock when resuming
 * Return:
 *		0: successful
 */
int suspend_set_clock(unsigned int is_restore)
{
	unsigned int clocks_ret = 0;
	unsigned int frqcrA_suspend_clock;
	unsigned int frqcrB_suspend_clock;
	unsigned int zb3_clock;
	unsigned int frqcrA_mask;
	unsigned int frqcrB_mask;

	if (es >= ES_REV_2_0) {
		/* I:1/6, ZG:1/4, M3: 1/8, B:1/48, M1:1/6, M5: 1/8*/
		/* Z: Not change, ZTR: 1/4, ZT: 1/6 */
		/* ZX:1/48, ZS:1/48, HP:1/48 */
		frqcrA_suspend_clock = POWERDOWN_FRQCRA_ES2;
		frqcrB_suspend_clock = POWERDOWN_FRQCRB_ES2;
		zb3_clock = ZB3_CLK_SUSPEND;
		frqcrA_mask = FRQCRA_MASK_ES2;
		frqcrB_mask = FRQCRB_MASK_ES2;
	} else {
		/* I:1/6, ZG:1/4, M3:1/8, B:1/12, M1:1/6 */
		/* Z:No change, ZTR:1/4, ZT:1/6, ZX:1/6 */
		/* ZS:1/12, HP:1/12 */
		frqcrA_suspend_clock = POWERDOWN_FRQCRA_ES1;
		frqcrB_suspend_clock = POWERDOWN_FRQCRB_ES1;
		frqcrA_mask = FRQCRA_MASK_ES1;
		frqcrB_mask = FRQCRA_MASK_ES1;
	}

	if (!is_restore) {
		pr_info("[%s]: Suspend: Set clock for suspending\n",\
			__func__);
		/* Backup FRQCRA/B */
		frqcrA_save = __raw_readl(FRQCRA);
		frqcrB_save = __raw_readl(FRQCRB);

		clocks_ret = clock_update(frqcrA_suspend_clock, frqcrA_mask,
				frqcrB_suspend_clock, frqcrB_mask);
		if (clocks_ret < 0) {
			pr_info("[%s]: Set clocks FAILED\n",\
				__func__);
		} else {
			pr_info("[%s]: Set clocks OK\n",
				__func__);
		}
	#if (defined ZB3_CLK_SUSPEND_ENABLE) && (defined ZB3_CLK_DFS_ENABLE)
		if (es > ES_REV_2_1) {
			frqcrD_save = suspend_ZB3_backup();
			if (frqcrD_save > 0) {
				clocks_ret = cpg_set_sbsc_freq(zb3_clock);
				if (clocks_ret < 0) {
					pr_info("[%s]: Set ZB3 clocks"
					"FAILED\n", __func__);
				} else {
					pr_info("[%s]: Set ZB3 "
					"clocks OK\n", __func__);
				}
			} else {
				pr_info("[%s]: Backup ZB3 "
					"clock FAILED\n", __func__);
				clocks_ret = frqcrD_save;
			}
		}
	#endif
	} else {
		pr_info("[%s]: Restore clock for resuming\n",
			__func__);
		clocks_ret = clock_update(frqcrA_save, frqcrA_mask,
					frqcrB_save, frqcrB_mask);
		if (clocks_ret < 0) {
			pr_info("[%s]: Restore clocks FAILED\n",
				__func__);
		} else {
			pr_info("[%s]: Restore clocks OK\n",
				__func__);
		}
	#if (defined ZB3_CLK_SUSPEND_ENABLE) && \
		(defined ZB3_CLK_DFS_ENABLE)
		if (es > ES_REV_2_1) {
			clocks_ret = cpg_set_sbsc_freq(frqcrD_save);
			if (clocks_ret < 0) {
				pr_info("[%s]: Restore ZB3 "
					"clocks FAILED\n", __func__);
			} else {
				pr_info("[%s]: Restore ZB3 clocks OK\n",
					__func__);
			}
		}
	#endif
	}
	return clocks_ret;
}

static int shmobile_suspend(void)
{
	int locked;
#ifdef CONFIG_PM_HAS_SECURE
	unsigned int sec_hal_ret_cpu0;
	unsigned int sec_hal_ret_cpu1;
#endif /* CONFIG_PM_HAS_SECURE*/
	unsigned int bankState;
	unsigned int workBankState2Area;
	unsigned int dramPasrSettingsArea0;
	unsigned int dramPasrSettingsArea1;
	u32 bk_pll1stpcr = 0;

	/*Change clocks using DFS function*/
	int clocks_ret;
	/*Change clocks using DFS function*/

#if Enable_PM_Test_Mode
	check_n_save_pin_conf();

	#if Enable_Change_Pin
	_gpio_set( 11, 0 );		// GPS_EN
	_gpio_set( 10, 1 );		// GPS_RST_N

	_gpio_set( 1, 0 );		// Undefined
	_gpio_set( 3, 0 );		// CAM_PWR_EN
	_gpio_set( 8, 0 );		// MAIN_MIC_LDO_EN 
	_gpio_set( 9, 0 );		// SENSOR_LDO_EN
	_gpio_set( 12, 0 );		// NFC_EN
	_gpio_set( 16, 0 );		// CAM1_RST_N
	_gpio_set( 17, 0 );		// SUB_MIC_LDO_EN
	_gpio_set( 20, 0 );		// CAM0_RESET_N
	_gpio_set( 25, 0 );		// VBUS_EN
	_gpio_set( 29, 0 );		// TCKEY_LDO_EN
	_gpio_set( 30, 0 );		// TSP_LDO_EN
	_gpio_set( 33, 0 );		// LCD_TE
	_gpio_set( 80, 0 );		// SPI_MOSI
	_gpio_set( 82, 0 );		// SPI_SCLK
	_gpio_set( 89, 0 );		// LCD_PWR_EN
	_gpio_set( 91, 0 );		// CAM1_STBY
	_gpio_set( 101, 0 );	// NFC_FIRMWARE
	_gpio_set( 102, 0 );	// MHL_EN
	_gpio_set( 104, 0 );	// TCK_INT
	_gpio_set( 128, 0 );	// UART_TXD
	_gpio_set( 129, 0 );	// UART_RXD
	_gpio_set( 130, 0 );	// ULPI_CS
	_gpio_set( 131, 0 );	// ULPI_RST
	_gpio_set( 139, 0 );	// DIG_RFEN
	_gpio_set( 215, 0 );	// CAM0_MCLK
	_gpio_set( 216, 0 );	// CAM1_MCLK
	_gpio_set( 217, 0 );	// ULPI_REF_CLK
	_gpio_set( 219, 0 );	// HDMI_CLKO
	_gpio_set( 226, 0 );	// VIB_ON
	_gpio_set( 227, 0 );	// CS0n
	_gpio_set( 228, 0 );	// VIB_PWM1
	_gpio_set( 248, 0 );	// Undefined
	_gpio_set( 260, 0 );	// WLAN_EN
	_gpio_set( 262, 0 );	// BT_WAKEUP
	_gpio_set( 268, 0 );	// BT_EN
	_gpio_set( 288, 0 );	// WLAN_SDIO_CLK
	_gpio_set( 326, 0 );	// SDHI0_CLK
	/* case 1 End : ok */


	__raw_writew(0, __io(0xE6058110) ); // H'0003 // 16   BSC Driver Control Register    DRVCR_BSC    RW 
	__raw_writew(0, __io(0xE6058112) ); // H'0003 // 16   SYSTEM Driver Control Register DRVCR_SYS    RW 
	__raw_writew(0, __io(0xE6058114) ); // H'0003 // 16   CKO Driver Control Register    DRVCR_CKO    RW 
	__raw_writew(0, __io(0xE6058116) ); // H'0003 // 16   KEY Driver Control Register    DRVCR_KEY    RW 
	__raw_writew(0, __io(0xE6058118) ); // H'0003 // 16   MMCCK Driver Control Register  DRVCR_MMCCK  RW 
	__raw_writew(0, __io(0xE605811A) ); // H'0003 // 16   LCD Driver Control Register    DRVCR_LCD    RW 
	__raw_writew(0, __io(0xE605811C) ); // H'0003 // 16   SDCLK1 Driver Control Register DRVCR_SDCLK1 RW 
	__raw_writew(0, __io(0xE605811E) ); // H'0023 // 16   SDCLK0 Driver Control Register DRVCR_SDCLK0 RW 
	__raw_writew(0, __io(0xE6058120) ); // H'0003 // 16   VCCQ Driver Control Register   DRVCR_VCCQ   RW 
	__raw_writew(0, __io(0xE6058122) ); // H'0003 // 16   ISP Driver Control Register    DRVCR_ISP    RW 
	__raw_writew(0, __io(0xE6058124) ); // H'0003 // 16   SD2 Driver Control Register    DRVCR_SD2    RW 
	__raw_writew(0, __io(0xE6058126) ); // H'0003 // 16   VCK5 Driver Control Register   DRVCR_VCK5   RW 
	__raw_writew(0, __io(0xE6058128) ); // H'0003 // 16   SDCLK2 Driver Control Register DRVCR_SDCLK2 RW 
	__raw_writew(0, __io(0xE6058180) ); // H'0003 // 16   RSTO Driver Control Register   DRVCR_RSTO   RW 
	__raw_writew(0, __io(0xE6058182) ); // H'0003 // 16   VCK1 Driver Control Register   DRVCR_VCK1   RW 
	__raw_writew(0, __io(0xE6058184) ); // H'0003 // 16   VCK2 Driver Control Register   DRVCR_VCK2   RW 
	__raw_writew(0, __io(0xE6058186) ); // H'0003 // 16   VCK3 Driver Control Register   DRVCR_VCK3   RW 
	__raw_writew(0, __io(0xE6058188) ); // H'0003 // 16   VCK4 Driver Control Register   DRVCR_VCK4   RW 
	__raw_writew(0, __io(0xE605818A) ); // H'0003 // 16   JTAG Driver Control Register   DRVCR_JTAG   RW 
	__raw_writew(0, __io(0xE605818C) ); // H'0003 // 16   SD1 Driver Control Register    DRVCR_SD1    RW 
	__raw_writew(0, __io(0xE605818E) ); // H'0023 // 16   SD0 Driver Control Register    DRVCR_SD0    RW 
	__raw_writew(0, __io(0xE6058190) ); // H'0003 // 16   MMC Driver Control Register    DRVCR_MMC    RW 
	__raw_writew(0, __io(0xE6058192) ); // H'0023 // 16   SIM0 Driver Control Register   DRVCR_SIM0   RW 
	__raw_writew(0, __io(0xE6058194) ); // H'0023 // 16   SIM1 Driver Control Register   DRVCR_SIM1   RW 
	__raw_writew(0, __io(0xE6058196) ); // H'0003 // 16   ULPI Driver Control Register   DRVCR_ULPI   RW 
	__raw_writew(0, __io(0xE6058198) ); // H'0003 // 16   HSI Driver Control Register    DRVCR_HSI    RW 

	__raw_writeb( 0x01, __io(0xE6822004));	/* I2C1. ICCR register. Set init value */
	__raw_writeb( 0x01, __io(0xE6824004));	/* I2C2. ICCR register. Set init value */
	__raw_writeb( 0x01, __io(0xE6826004));	/* I2C3. ICCR register. Set init value */

	__raw_writel((__raw_readl(__io(0xE61F012C)) | 0x00000300), __io(0xE61F012C));
	__raw_writel((__raw_readl(__io(0xE61F022C)) | 0x00000300), __io(0xE61F022C));
	#endif // #if Enable_Change_Pin

#endif // #if Enable_PM_Test_Mode

	/* check wakelock */
	locked = has_wake_lock_no_expire(WAKE_LOCK_SUSPEND);
	if (locked)
		goto Cancel;

	/* check cpu#1 power down */
	if (core_shutdown_status(1) != 3) {
		not_core_shutdown = 1;
		shmobile_suspend_udelay(1000);  /*udelay(1000);*/
		barrier();
		if (core_shutdown_status(1) != 3)
			return -EBUSY;
	}

	/* Backup IP registers */
	/* irqx_eventdetectors_regs_save(); */
	shwy_regs_save();

#ifdef PASR_SUPPORT
	/* Get ram bank status */
	bankState = get_ram_banks_status();
	if (bankState == -ENOTSUPP)		/* Ram Defrag is disabled */
		bankState = 0xFFFF;
#else
	bankState = 0xFFFF;
#endif
	/*
	 * Get OP of DRAM area 0 and area 1
	 * Bit[0->7] : OP of area 0
	 * Bit[8->15]: OP of area 1
	 */
	workBankState2Area = (SbscDramPasr2Area & ~bankState);
	if (workBankState2Area != 0) {
		/* Get setting OP, MA of DRAM area 0
		 *(OP = Bit[0->7] of workBankState2Area)
		 */
		dramPasrSettingsArea0 = ((workBankState2Area & 0x00FF) << 8) \
								| MRW_MA_PASR;
		dramPasrSettingsArea1 = (workBankState2Area & 0xFF00) \
								| MRW_MA_PASR;
	} else { /* workBankState2Area == 0 */
		dramPasrSettingsArea0 = 0;
		dramPasrSettingsArea1 = 0;
	}

	pr_debug(PMDBG_PRFX "%s: RAM bank status:\n", __func__);
	pr_debug("bankState: 0x%x workBankState2Area: 0x%08x\n" \
				, bankState, workBankState2Area);
	pr_debug("dramPasrSettingsArea0: 0x%08x ", dramPasrSettingsArea0);
	pr_debug("dramPasrSettingsArea1: 0x%08x\n", dramPasrSettingsArea1);

	/* Save setting value to ram0 */
	pm_writel(dramPasrSettingsArea0, ram0DramPasrSettingArea0);
	pm_writel(dramPasrSettingsArea1, ram0DramPasrSettingArea1);

#ifdef CONFIG_PM_HAS_SECURE
	/* For access SBSC registers */
	pm_writel((unsigned long)(map[SBSC_SDCR0A_TZ].base), \
				ram0SBSC_SDCR0AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCRC0A_TZ].base), \
				ram0SBSC_SDWCRC0AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCRC1A_TZ].base), \
				ram0SBSC_SDWCRC1AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCR00A_TZ].base), \
				ram0SBSC_SDWCR00AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCR01A_TZ].base), \
				ram0SBSC_SDWCR01AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCR10A_TZ].base), \
				ram0SBSC_SDWCR10AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCR11A_TZ].base), \
				ram0SBSC_SDWCR11AIOremap);
	pm_writel((unsigned long)(map[SBSC_SDWCRC2A_TZ].base), \
				ram0SBSC_SDWCRC2AIOremap);
#endif
	if (es <= ES_REV_2_1)
		do_iicdvm_setting();

	xtal1_log_out = 1;

	/* - add A4MP state as PLL1 stop conditon */

	/* TO DO: Use semaphore before access to PLL1STPCR */
	unsigned int ret;
	if (pll_1_sem) {
		ret = hwspin_trylock_nospin(pll_1_sem);
		if ((ret == -EBUSY) || (ret == -EINVAL)) {
			pr_err("[%s]: Suspend: Can not get semaphore\n", \
					__func__);
			goto Cancel;
		}
	}
	pr_err("[%s]: Suspend: Get semaphore successfully\n", __func__);
	pr_err("[%s]: Suspend: Setting PLL1STPCR\n", __func__);
	bk_pll1stpcr = __raw_readl(PLL1STPCR);
	__raw_writel(bk_pll1stpcr | A4MP | C4STP, PLL1STPCR);
	pr_err("[%s]: Suspend: PLL1STPCR = 0x%8x\n", __func__, \
			__raw_readl(PLL1STPCR));

	/* TO DO: Release semaphore before access to PLL1STPCR */
	hwspin_unlock_nospin(pll_1_sem);

	__raw_writel((__raw_readl(WUPSMSK) | (1 << 28)), WUPSMSK);

#ifndef CONFIG_PM_HAS_SECURE
	pm_writel(1, ram0ZQCalib);
#endif	/*CONFIG_PM_HAS_SECURE*/

	/* Update clocks setting */
	is_suspend_setclock = 1;
	if (suspend_set_clock(CLOCK_SUSPEND) != 0) {
			pr_debug(PMDBG_PRFX "%s: Suspend without "
			"updating clock setting\n", __func__);
		is_clock_updated = 0;
	} else {
		is_clock_updated = 1;
	}
	/*
	 * do cpu suspend ...
	 */
	pr_err("[%s]: do cpu suspend ...\n\n", __func__);
	jump_systemsuspend();

	/* Update clocks setting */
	if (is_clock_updated == 1) {
		if (suspend_set_clock(CLOCK_RESTORE) != 0)
			pr_debug(PMDBG_PRFX "%s: Resume after "
				"restoring clock setting\n", __func__);
	} else {
		pr_debug(PMDBG_PRFX "%s: Resume without "
		"restoring clock setting\n", __func__);
	}
	is_suspend_setclock = 0;
#ifndef CONFIG_PM_HAS_SECURE
	pm_writel(0, ram0ZQCalib);
#endif	/*CONFIG_PM_HAS_SECURE*/

	/* Restore PLL1 stop conditon)*/
	/* TO DO: Use semaphore before access to PLL1STPCR */
	ret = hwspin_trylock_nospin(pll_1_sem);
	if ((ret == -EBUSY) || (ret == -EINVAL)) {
		pr_err("[%s]: Resume: Can not get semaphore\n", __func__);
		pr_err("[%s]: Resume: Can not restore PLL1STPCR setting\n",\
				__func__);
		pr_err("[%s]: Resume: PLL1STPCR = 0x%8x\n", \
				__func__, __raw_readl(PLL1STPCR));
	} else {
		pr_err("[%s]: Resume: Get semaphore successfully\n", __func__);
		pr_err("[%s]: Resume: Restore PLL1STPCR setting\n", __func__);
		__raw_writel(bk_pll1stpcr, PLL1STPCR);
		pr_err("[%s]: Resume: PLL1STPCR = 0x%8x\n",
			__func__, __raw_readl(PLL1STPCR));
		hwspin_unlock_nospin(pll_1_sem);
	}

	wakeups_factor();

#ifdef CONFIG_PM_HAS_SECURE
	sec_hal_ret_cpu0 = __raw_readl(ram0SecHalReturnCpu0);
	pr_debug(PMDBG_PRFX "%s: SEC HAL return CPU0: 0x%08x\n", \
			__func__, sec_hal_ret_cpu0);
#ifdef CONFIG_PM_SMP
	sec_hal_ret_cpu1 = __raw_readl(ram0SecHalReturnCpu1);
	pr_debug(PMDBG_PRFX "%s: SEC HAL return CPU1: 0x%08x\n", \
			__func__, sec_hal_ret_cpu1);
#endif
#endif /*CONFIG_PM_HAS_SECURE*/

	/* - set Wake up factor unmask to GIC.CPU0,CPU1 */
	__raw_writel((__raw_readl(WUPSMSK) &  ~(1 << 28)), WUPSMSK);


	/* Restore IP registers */
	shwy_regs_restore();
	/* irqx_eventdetectors_regs_restore(); */

#if Enable_PM_Test_Mode
	#if Enable_Change_Pin
	restore_pin_conf();
	
	_gpio_set( 29, 1 );		// Port29 TCKEY_LDO_EN
	_gpio_set( 30, 1 );		// Port30 TSP_LDO_EN
	_gpio_set( 131, 1 );	// Port131 ULPI_RST
	#endif // #if Enable_Change_Pin
#endif // Enable_PM_Test_Mode

	return 0;

Cancel:
	return -EBUSY;
}

static int shmobile_suspend_enter(suspend_state_t unused)
{
	int ret = 0;

	switch (shmobile_suspend_state) {
	case PM_SUSPEND_STANDBY:
	case PM_SUSPEND_MEM:
		ret = shmobile_suspend();
		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static int shmobile_suspend_valid(suspend_state_t state)
{
	return ((state > PM_SUSPEND_ON) && (state <= PM_SUSPEND_MAX));
}

static int shmobile_suspend_prepare(void)
{
	int ret;

	/* Execute RAM Defragmentation */
	ret = defrag();

	if (0 != ret) {
		pr_debug(PMDBG_PRFX "%s: RAM defragment is not supported.\n", \
				__func__);
	}

	return 0;
}

static int shmobile_suspend_prepare_late(void)
{
	disable_hlt();

	/* backup sys boot address */
	save_sbar_val = __raw_readl(__io(SBAR));

	/* set RAM1 vector */
	__raw_writel(RAM_ARM_VECT, __io(SBAR));

	return 0;
}

static void shmobile_suspend_wake(void)
{
#ifdef __EXTAL1_INFO__
	unsigned int reg_val = 0;
#endif
	enable_hlt();

	/* restore sys boot address */
	__raw_writel(save_sbar_val, __io(SBAR));

	/* Log information for disabling EXTAL1 */
#ifdef __EXTAL1_INFO__
	if (xtal1_log_out == 1) {
		pr_debug(PMDBG_PRFX "EXTAL1: Log information\n");
		pr_debug(PMDBG_PRFX "---[Before suspend]---\n");

		reg_val = pm_readl(ram0SaveEXMSKCNT1Phys_suspend);
		pr_debug(PMDBG_PRFX "EXMSKCNT1: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SaveAPSCSTPPhys_suspend);
		pr_debug(PMDBG_PRFX "APSCSTP: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SaveSYCKENMSKPhys_suspend);
		pr_debug(PMDBG_PRFX "SYCKENMSK: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SaveC4POWCRPhys_suspend);
		pr_debug(PMDBG_PRFX "C4POWCR: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SavePDNSELPhys_suspend);
		pr_debug(PMDBG_PRFX "PDNSEL: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SavePSTRPhys_suspend);
		pr_debug(PMDBG_PRFX "PSTR: 0x%08x\n", reg_val);

		pr_debug(PMDBG_PRFX "---[After suspend]---\n");

		reg_val = pm_readl(ram0SaveEXMSKCNT1Phys_resume);
		pr_debug(PMDBG_PRFX "EXMSKCNT1: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SaveAPSCSTPPhys_resume);
		pr_debug(PMDBG_PRFX "APSCSTP: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SaveSYCKENMSKPhys_resume);
		pr_debug(PMDBG_PRFX "SYCKENMSK: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SaveC4POWCRPhys_resume);
		pr_debug(PMDBG_PRFX "C4POWCR: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SavePDNSELPhys_resume);
		pr_debug(PMDBG_PRFX "PDNSEL: 0x%08x\n", reg_val);

		reg_val = pm_readl(ram0SavePSTRPhys_resume);
		pr_debug(PMDBG_PRFX "PSTR: 0x%08x\n", reg_val);

		xtal1_log_out = 0;
	}
#endif
}

#ifdef CONFIG_PM_DEBUG
int control_systemsuspend(int is_enabled)
{
	unsigned long irqflags;
	int cur_state = 0;
	spin_lock_irqsave(&systemsuspend_lock, irqflags);
	cur_state = get_suspend_state();

	if (cur_state == PM_SUSPEND_MEM)
		request_suspend_state(PM_SUSPEND_ON);

	enable_module = is_enabled;
	spin_unlock_irqrestore(&systemsuspend_lock, irqflags);
	return 0;
}
EXPORT_SYMBOL(control_systemsuspend);

int is_systemsuspend_enable(void)
{
	return enable_module;
}
EXPORT_SYMBOL(is_systemsuspend_enable);
#endif /* CONFIG_PM_DEBUG */

const struct platform_suspend_ops shmobile_suspend_ops = {
	.begin			= shmobile_suspend_begin,
	.end			= shmobile_suspend_end,
	.enter			= shmobile_suspend_enter,
	.valid			= shmobile_suspend_valid,
	.prepare		= shmobile_suspend_prepare,
	.prepare_late	= shmobile_suspend_prepare_late,
	.wake			= shmobile_suspend_wake,
};

static int __init shmobile_suspend_init(void)
{
	int i;
	void __iomem *virt;
	volatile struct base_map *tbl = map;
#ifdef CONFIG_PM_DEBUG
	log_wakeupfactor = 1;
#else /*CONFIG_PM_DEBUG*/
	log_wakeupfactor = 0;
#endif /*CONFIG_PM_DEBUG*/
	pr_debug(PMDBG_PRFX "%s: initialize\n", __func__);

#ifndef CONFIG_CPU_IDLE
	int ret;
	ret = shmobile_init_pm();
	if (ret != 0)
		return ret;
#endif
	/* Get chip revision */
	es = shmobile_chip_rev();

	/* create address table */
	for (i = 0; i < ARRAY_SIZE(map); i++) {
		if (tbl->base) {
			*tbl++;
			continue;
		}
		virt = ioremap_nocache(tbl->phys, tbl->size);
		if (!virt) {
			pr_emerg(PMDBG_PRFX \
					"%s: ioremap failed. base 0x%lx\n", \
					__func__, tbl->phys);
			*tbl++;
			continue;
		}
		tbl->base = (unsigned long)virt;
		pr_debug(PMDBG_PRFX \
			"%s: ioremap phys 0x%lx, virt 0x%lx, size %d\n", \
			__func__, tbl->phys, tbl->base, tbl->size);
		*tbl++;
	}

	wakeups_factor();

	suspend_set_ops(&shmobile_suspend_ops);

	shmobile_suspend_state = PM_SUSPEND_ON;
#ifndef CONFIG_PM_HAS_SECURE
	pm_writel(0, ram0ZQCalib);
#endif 	/*CONFIG_PM_HAS_SECURE*/

	return 0;
}
arch_initcall(shmobile_suspend_init);

