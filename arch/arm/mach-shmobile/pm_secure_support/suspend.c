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

#include <linux/wakelock.h>
#include "pm_ram0.h"
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

#define RAM0_ARM_VECT			ram0ArmVectorPhys
#define PMDBG_PRFX				"PM-DBG: "

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
};

static suspend_state_t shmobile_suspend_state;
static int not_core_shutdown;

extern void jump_systemsuspend(void);
extern int has_wake_lock_no_expire(int type);

static unsigned int save_sbar_val;

static int log_wakeupfactor;
module_param(log_wakeupfactor, int, S_IRUGO | S_IWUSR | S_IWGRP);

static int es;

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
		pr_debug(PMDBG_PRFX "APE WakeUpFactor Value = 0x%08x \n", dummy);
		pr_debug(PMDBG_PRFX "APE SysteCPU WakeUpS Mask Value = 0x%08x \n", \
					__raw_readl(WUPSMSK));
	}
	
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
	volatile struct reg_info *info = regs;
	int i;

	for (i = 0; i < count; i++, *info++) {
		if (!*info->vbase)
			continue;
		if ((info->esrev & es) == es) {
			switch (info->size) {
			case 8:
				info->val = pm_readb(*info->vbase + info->offset);
				break;
			case 16:
				info->val = pm_readw(*info->vbase + info->offset);
				break;
			case 32:
				info->val = pm_readl(*info->vbase + info->offset);
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
	volatile struct reg_info *info;
	int i;

	info = regs + count;

	for (i = count; i > 0; i--) {
		*info--;
		if (!*info->vbase)
			continue;
		if ((info->esrev & es) == es) {
			switch (info->size) {
			case 8:
				pm_writeb(info->val, (*info->vbase + info->offset));
				break;
			case 16:
				pm_writew(info->val, (*info->vbase + info->offset));
				break;
			case 32:
				pm_writel(info->val, (*info->vbase + info->offset));
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

static void irqx_eventdetectors_regs_restore(void)
{
	DO_RESTORE_REGS(irqx_eventdetectors_regs);
}

static void shwy_regs_restore(void)
{
	DO_RESTORE_REGS(shwy_regs);
}

/*
 * Helper functions for checking CPU status
 */
static int core_shutdown_status(unsigned int cpu)
{
	if (es == ES_REV_1_0)
		return (__raw_readl(CPG_PSTR) >> (4 * cpu)) & 3;
	else
		return (__raw_readl(CPG_SCPUSTR) >> (4 * cpu)) & 3;
}

/*
 * System suspend callback functions' implementation
 */
static int shmobile_suspend_begin(suspend_state_t state)
{
	shmobile_suspend_state = state;

#if 0
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

	if (not_core_shutdown) {
		pr_debug(PMDBG_PRFX "%s: CPU0 waited until the CPU1 power down.\n", \
				__func__);
		not_core_shutdown = 0;
	}

#if 0
	int ret;
	ret = resume_cpufreq();
	if (ret != 0) {
		pr_debug(PMDBG_PRFX "%s: resume_cpufreq() returns %d.\n", \
				__func__, ret);
	}
#endif
}

static int shmobile_suspend(void)
{
	int locked;
	unsigned long sec_hal_ret_cpu0;
	unsigned long sec_hal_ret_cpu1;
	unsigned int bankState;
	unsigned int workBankState2Area;
	unsigned int dramPasrSettingsArea0;
	unsigned int dramPasrSettingsArea1;

	/* check wakelock */
	locked = has_wake_lock_no_expire(WAKE_LOCK_SUSPEND);
	if (locked)
		goto Cancel;

	/* check cpu#1 power down */
	if (core_shutdown_status(1) != 3) {
		not_core_shutdown = 1;
		/*udelay(1000);*/
		barrier();
		if (core_shutdown_status(1) != 3)
			return -EBUSY;
	}

	/* Backup IP registers */
	irqx_eventdetectors_regs_save();
	shwy_regs_save();

	/* Get ram bank status */
	bankState = get_ram_banks_status();
	if (bankState == -ENOTSUPP)
		bankState = 0xFFFF;
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
		dramPasrSettingsArea1 = (workBankState2Area & 0xFF00) | MRW_MA_PASR;
	}
	else { /* workBankState2Area == 0 */
		dramPasrSettingsArea0 = 0;
		dramPasrSettingsArea1 = 0;
	}

	pr_debug(PMDBG_PRFX "%s: RAM bank status: \n \
	bankState: 0x%x workBankState2Area: 0x%08x \n \
	dramPasrSettingsArea0: 0x%08x dramPasrSettingsArea1: 0x%08x \n\n", \
	__func__, bankState, workBankState2Area,\
	dramPasrSettingsArea0, dramPasrSettingsArea1);

	/* Save setting value to ram0 */
	pm_writel(dramPasrSettingsArea0, ram0DramPasrSettingArea0);
	pm_writel(dramPasrSettingsArea1, ram0DramPasrSettingArea1);

	pm_writel((es < ES_REV_2_0)?FRQCRA_ES1_MASK:FRQCRA_ES2_MASK
			, ram0FRQCRAMask); 
	pm_writel((es < ES_REV_2_0)?POWERDOWN_FRQCRA_ES1:POWERDOWN_FRQCRA_ES2
			, ram0FRQCRADown); 

	/*
	 * do cpu suspend ...
	 */
	pr_debug(PMDBG_PRFX "%s: do cpu suspend ...\n\n", __func__);
	jump_systemsuspend();

	wakeups_factor();

	sec_hal_ret_cpu0 = __raw_readl(ram0SecHalReturnCpu0);
	pr_debug(PMDBG_PRFX "%s: SEC HAL return CPU0: 0x%08x \n", \
					__func__, sec_hal_ret_cpu0);
#ifdef CONFIG_PM_SMP
	sec_hal_ret_cpu1 = __raw_readl(ram0SecHalReturnCpu1);
	pr_debug(PMDBG_PRFX "%s: SEC HAL return CPU1: 0x%08x\n", \
					__func__, sec_hal_ret_cpu1);
#endif

	/* Restore IP registers */
	shwy_regs_restore();
	irqx_eventdetectors_regs_restore();

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
		pr_debug(PMDBG_PRFX "%s: RAM defragment is not supported.\n", __func__);
	}

	return 0;
}

static int shmobile_suspend_prepare_late(void)
{
	disable_hlt();

	/* backup sys boot address */
	save_sbar_val = __raw_readl(__io(SBAR));

	/* set RAM0 vector */
	__raw_writel(RAM0_ARM_VECT, __io(SBAR));

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

	/* Log information for Disabling EXTAL1 */

#ifdef __EXTAL1_INFO__
	pr_debug(PMDBG_PRFX "EXTAL1: Log information\n");
	pr_debug(PMDBG_PRFX "---------[Before suspend]------------------\n");
	
	reg_val = pm_readl(ram0SaveEXMSKCNT1Phys_suspend);
	pr_debug(PMDBG_PRFX "EXTAL1 Mask Count Register (EXMSKCNT1): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SaveAPSCSTPPhys_suspend);
	pr_debug(PMDBG_PRFX "EXTAL1 Clock Stop Control Register (APSCSTP): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SaveSYCKENMSKPhys_suspend);
	pr_debug(PMDBG_PRFX "EXTAL1 Control Register (SYCKENMSK): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SaveC4POWCRPhys_suspend);
	pr_debug(PMDBG_PRFX "C4 Area Power Control Register (C4POWCR): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SavePDNSELPhys_suspend);
	pr_debug(PMDBG_PRFX "C4 Area Power Control Register2 (PDNSEL): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SavePSTRPhys_suspend);
	pr_debug(PMDBG_PRFX "Power Status Register (PSTR): 0x%08x\n", reg_val);
	
	pr_debug(PMDBG_PRFX "---------[After suspend]-------------------\n");
	
	reg_val = pm_readl(ram0SaveEXMSKCNT1Phys_resume);
	pr_debug(PMDBG_PRFX "EXTAL1 Mask Count Register (EXMSKCNT1): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SaveAPSCSTPPhys_resume);
	pr_debug(PMDBG_PRFX "EXTAL1 Clock Stop Control Register (APSCSTP): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SaveSYCKENMSKPhys_resume);
	pr_debug(PMDBG_PRFX "EXTAL1 Control Register (SYCKENMSK): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SaveC4POWCRPhys_resume);
	pr_debug(PMDBG_PRFX "C4 Area Power Control Register (C4POWCR): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SavePDNSELPhys_resume);
	pr_debug(PMDBG_PRFX "C4 Area Power Control Register2 (PDNSEL): 0x%08x\n", reg_val);
	
	reg_val = pm_readl(ram0SavePSTRPhys_resume);
	pr_debug(PMDBG_PRFX "Power Status Register (PSTR): 0x%08x\n", reg_val);
#endif
}

#ifdef CONFIG_PM_DEBUG
extern void request_suspend_state(suspend_state_t state);
extern suspend_state_t get_suspend_state(void);
int control_systemsuspend(int is_enabled)
{
	unsigned long irqflags;
	int cur_state = 0;
	spin_lock_irqsave(&systemsuspend_lock, irqflags);
	cur_state = get_suspend_state();
	if (cur_state == PM_SUSPEND_MEM){
		request_suspend_state(PM_SUSPEND_ON);
	}
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

struct platform_suspend_ops shmobile_suspend_ops = {
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
	log_wakeupfactor = 0;
	
	pr_debug(PMDBG_PRFX "%s: initialize\n", __func__);

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
			pr_emerg(PMDBG_PRFX "%s: ioremap failed. base 0x%lx\n", \
					__func__, tbl->phys);
			*tbl++;
			continue;
		}
		tbl->base = (unsigned long)virt;
		pr_debug(PMDBG_PRFX "%s: ioremap phys 0x%lx, virt 0x%lx, size %d\n", \
				__func__, tbl->phys, tbl->base, tbl->size);
		*tbl++;
	}

	wakeups_factor();

	suspend_set_ops(&shmobile_suspend_ops);

	shmobile_suspend_state = PM_SUSPEND_ON;

	return 0;
}
arch_initcall(shmobile_suspend_init);
