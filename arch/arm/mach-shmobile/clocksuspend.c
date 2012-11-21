/*
 * arch/arm/mach-shmobile/clocksuspend.c
 *
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
 */
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/hwspinlock.h>
#include <linux/io.h>

#include <mach/r8a73734.h>
#include <mach/pm.h>

#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) "dvfs[clocksuspend.c<%4d>]:" fmt, __LINE__
#endif

#define CPG_BASE		IO_ADDRESS(0xE6150000)
#define ZDIVCR5			IO_ADDRESS(0xE61501b4)
#define CPG_FRQCRA		(CPG_BASE + 0x0000)
#define CPG_FRQCRB		(CPG_BASE + 0x0004)
#define CPG_ZBCKCR		(CPG_BASE + 0x0010)
#define CPG_FRQCRD		(CPG_BASE + 0x00E4)
#define CPG_PLLECR		(CPG_BASE + 0x00D0)
#define CPG_PLL0CR		(CPG_BASE + 0x00D8)
#define CPG_PLL1CR		(CPG_BASE + 0x0028)
#define CPG_PLL2CR		(CPG_BASE + 0x002C)
#define CPG_PLL3CR		(CPG_BASE + 0x00DC)
#define CPG_CKSCR		(CPG_BASE + 0x00C0)
#define PLLCR_STC_MASK		0x3F000000
#define PLLCR_BIT24_SHIFT	24

#define FRQCRD_ZB30SEL		BIT(4)
#define KICK_WAIT_INTERVAL_US	500

#ifndef SMGP100_DFS_ZS
#define SMGP100_DFS_ZS		SMGP002
#endif /* SMGP100_DFS_ZS */
#ifndef SMGP001_DFS
#define SMGP001_DFS		SMGP001
#endif /* SMGP001_DFS */
#define SEM_TIMEOUT		2	/* 2ms */

static struct {
	unsigned int zs_disabled_cnt;
	unsigned int hp_disabled_cnt;
} the_clock;

/* #define SHM_CLK_TEST_MODE	1 */
/* #define CLKSUS_DEBUG_ENABLE	1 */
#ifdef pr_fmt
#undef pr_fmt
#define pr_fmt(fmt) "[CLK] - " fmt
#endif

#ifdef CLKSUS_DEBUG_ENABLE
#define pr_log(fmt, ...)	printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_log(fmt, ...)
#endif /* CLKSUS_DEBUG_ENABLE */

/*******************************************************************************
 * PM APIs *********************************************************************
 ******************************************************************************/
struct clk_hw_info {
	unsigned int	mask_bit;
	unsigned int	shift_bit;
	int		div_val[16];
	void __iomem	*addr;
};

/*******************************************************************************
 * PM APIs *********************************************************************
 ******************************************************************************/
static DEFINE_SPINLOCK(freq_change_lock);
static DEFINE_SPINLOCK(zs_lock);
static struct clk_hw_info __clk_hw_info_es1_x[] = {
	[I_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 20,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[ZG_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 16,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = -1,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = -1,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[B_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 8,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[M1_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 4,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = 0xc,
			[DIV1_8]  = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[M3_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 12,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = 0xc,
			[DIV1_8]  = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[Z_CLK] = {
		.mask_bit	= 0x1f,
		.shift_bit	= 24,
		.div_val = {
			[DIV1_1] = 0x0,
			[DIV1_2] = 0x10,
			[DIV1_3] = 0x11,
			[DIV1_4] = 0x12,
			[DIV1_5] = -1,
			[DIV1_6] = 0x13,
			[DIV1_7] = -1,
			[DIV1_8] = 0x14,
			[DIV1_12] = 0x15,
			[DIV1_16] = 0x16,
			[DIV1_18] = -1,
			[DIV1_24] = 0x18,
			[DIV1_32] = -1,
			[DIV1_36] = -1,
			[DIV1_48] = 0x1b,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZTR_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 20,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZT_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 16,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZX_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 12,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[HP_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 4,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZS_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 8,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZB_CLK] = { /* 1/2*(setting + 1) ~ 1/2, 1/4, 1/6, 1/8 */
		.mask_bit	= 0x3f,
		.shift_bit	= 0,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = -1,
			[DIV1_4] = 0x1,
			[DIV1_5] = -1,
			[DIV1_6] = 0x2,
			[DIV1_7] = -1,
			[DIV1_8] = 0x3,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x7,
			[DIV1_18] = 0x8,
			[DIV1_24] = 0xb,
			[DIV1_32] = 0xf,
			[DIV1_36] = 0x11,
			[DIV1_48] = 0x27,
			[DIV1_96] = 0x2f
		},
		.addr = __io(CPG_ZBCKCR)
	},
	[ZB3_CLK] = {
		.mask_bit	= 0x1f,
		.shift_bit	= 0,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = -1,
			[DIV1_4] = 0x10,
			[DIV1_5] = -1,
			[DIV1_6] = 0x11,
			[DIV1_7] = -1,
			[DIV1_8] = 0x12,
			[DIV1_12] = 0x13,
			[DIV1_16] = 0x14,
			[DIV1_18] = -1,
			[DIV1_24] = 0x15,
			[DIV1_32] = 0x16,
			[DIV1_36] = -1,
			[DIV1_48] = 0x18,
			[DIV1_96] = 0x1b
		},
		.addr = __io(CPG_FRQCRD)
	}
};
static struct clk_hw_info __clk_hw_info_es2_x[] = {
	[I_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 20,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[ZG_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 16,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = -1,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = -1,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[B_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 8,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[M1_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 4,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = 0xc,
			[DIV1_8]  = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[M3_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 12,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = 0xc,
			[DIV1_8]  = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[M5_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 0,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = 0xc,
			[DIV1_8]  = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRA)
	},
	[Z_CLK] = {
		.mask_bit	= 0x1f,
		.shift_bit	= 24,
		.div_val = {
			[DIV1_1] = 0x0,
			[DIV1_2] = 0x10,
			[DIV1_3] = 0x11,
			[DIV1_4] = 0x12,
			[DIV1_5] = -1,
			[DIV1_6] = 0x13,
			[DIV1_7] = -1,
			[DIV1_8] = 0x14,
			[DIV1_12] = 0x15,
			[DIV1_16] = 0x16,
			[DIV1_18] = -1,
			[DIV1_24] = 0x18,
			[DIV1_32] = -1,
			[DIV1_36] = -1,
			[DIV1_48] = 0x1b,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZTR_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 20,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZT_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 16,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZX_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 12,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[HP_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 4,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZS_CLK] = {
		.mask_bit	= 0xf,
		.shift_bit	= 8,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = 0x1,
			[DIV1_4] = 0x2,
			[DIV1_5] = -1,
			[DIV1_6] = 0x3,
			[DIV1_7] = -1,
			[DIV1_8] = 0x4,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x6,
			[DIV1_18] = 0x7,
			[DIV1_24] = 0x8,
			[DIV1_32] = -1,
			[DIV1_36] = 0xa,
			[DIV1_48] = 0xb,
			[DIV1_96] = -1
		},
		.addr = __io(CPG_FRQCRB)
	},
	[ZB_CLK] = { /* 1/2*(setting + 1) ~ 1/2, 1/4, 1/6, 1/8 */
		.mask_bit	= 0x3f,
		.shift_bit	= 0,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = -1,
			[DIV1_4] = 0x1,
			[DIV1_5] = -1,
			[DIV1_6] = 0x2,
			[DIV1_7] = -1,
			[DIV1_8] = 0x3,
			[DIV1_12] = 0x5,
			[DIV1_16] = 0x7,
			[DIV1_18] = 0x8,
			[DIV1_24] = 0xb,
			[DIV1_32] = 0xf,
			[DIV1_36] = 0x11,
			[DIV1_48] = 0x27,
			[DIV1_96] = 0x2f
		},
		.addr = __io(CPG_ZBCKCR)
	},
	[ZB3_CLK] = {
		.mask_bit	= 0x1f,
		.shift_bit	= 0,
		.div_val = {
			[DIV1_1] = -1,
			[DIV1_2] = 0x0,
			[DIV1_3] = -1,
			[DIV1_4] = 0x10,
			[DIV1_5] = -1,
			[DIV1_6] = 0x11,
			[DIV1_7] = -1,
			[DIV1_8] = 0x12,
			[DIV1_12] = 0x13,
			[DIV1_16] = 0x14,
			[DIV1_18] = -1,
			[DIV1_24] = 0x15,
			[DIV1_32] = 0x16,
			[DIV1_36] = -1,
			[DIV1_48] = 0x18,
			[DIV1_96] = 0x1b
		},
		.addr = __io(CPG_FRQCRD)
	}
};

#ifndef SHM_CLK_TEST_MODE
static
#endif
struct clk_hw_info *__clk_hw_info;
#ifndef SHM_CLK_TEST_MODE
static
#endif
struct clk_rate __shmobile_freq_modes_es1_x[] = {
	/* ES1.x */
	{
		/* suspend */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_24,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_2,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_6,
		.hp_clk	= DIV1_24,
		.zs_clk = DIV1_24,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_16
	},
	{
		/* Normal, CPU:MAX */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_1,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
	{
		/* Earlysuspend, CPU:MAX */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_1,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
	{
		/* Earlysuspend, CPU:MID */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_2,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_12,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
	{
		/* Earlysuspend, CPU:MIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_24,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_4,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_6,
		.hp_clk	= DIV1_24,
		.zs_clk = DIV1_24,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_16
	}
};
#ifndef SHM_CLK_TEST_MODE
static
#endif
struct clk_rate __shmobile_freq_modes_es2_x[] = {
	/* ES2.x */
	{
		/* Suspend */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_2,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_12,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4,
		.pll0 = PLLx46
	},
	{
		/* Normal, CPU:MAX */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_1,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx56
	},
	{
		/* Earlysuspend, CPU:HIGH */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_1,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx46
	},
	{
		/* Earlysuspend, CPU:MID */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_2,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_12,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4,
		.pll0 = PLLx46
	},
	{
		/* Earlysuspend, CPU:MIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_24,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_4,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_24,
		.hp_clk = DIV1_24,
		.zs_clk = DIV1_24,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_16,
		.pll0 = PLLx46
	}
};

/* waveform mapping table */
static struct {
	int mode;
	unsigned int waveform;
} waveform_map[] = {
	{.mode = LLx16_16,	.waveform = 0xffff},
	{.mode = LLx15_16,	.waveform = 0xfffe},
	{.mode = LLx14_16,	.waveform = 0xfefe},
	{.mode = LLx13_16,	.waveform = 0xfeee},
	{.mode = LLx12_16,	.waveform = 0xeeee},
	{.mode = LLx11_16,	.waveform = 0xeeea},
	{.mode = LLx10_16,	.waveform = 0xeaea},
	{.mode = LLx9_16,	.waveform = 0xeaaa},
	{.mode = LLx8_16,	.waveform = 0xaaaa},
	{.mode = LLx7_16,	.waveform = 0xab56},
	{.mode = LLx6_16,	.waveform = 0xb6b6},
	{.mode = LLx5_16,	.waveform = 0xdb6c},
	{.mode = LLx4_16,	.waveform = 0xcccc},
	{.mode = LLx3_16,	.waveform = 0xe738},
	{.mode = LLx2_16,	.waveform = 0xf0f0},
	{.mode = LLx1_16,	.waveform = 0xff00}
};

#ifndef SHM_CLK_TEST_MODE
static
#endif
struct clk_rate *__shmobile_freq_modes;
#ifdef FRQCRA_B_SEM
static struct hwspinlock *gen_sem0;
#endif /* FRQCRA_B_SEM */
static struct hwspinlock *gen_sem1;
#define DIV_TO_HW(clk, div)	\
	(__clk_hw_info[clk].div_val[div] << __clk_hw_info[clk].shift_bit)
/*
 * __match_div_rate: get div-rate by hw setting value
 *
 * Arguments:
 *		@clk: wait time.
 *		@val: hw setting value.
 *
 * Return: divrate
 *
 */
inline int __match_div_rate(int clk, int val)
{
	const enum clk_div div_table[] = {
		DIV1_1, DIV1_2, DIV1_3, DIV1_4, DIV1_5,
		DIV1_6, DIV1_7, DIV1_8, DIV1_12, DIV1_16,
		DIV1_18, DIV1_24, DIV1_32, DIV1_36,
		DIV1_48, DIV1_96
	};
	int i = 0;
	int len = (int)ARRAY_SIZE(div_table);

	for (i = 0; i < len; i++)
		if (__clk_hw_info[clk].div_val[i] == val)
			return div_table[i];

	/* not expected(invalid value), just in case! */
	pr_err("invalid clk/div-rate<%d/%d>\n", clk, val);
	return -EINVAL;
}

/*
 * __div: convert the divrate value
 *
 * Arguments:
 *		@c_div: input divrate
 *
 * Return: div value
 *
 */
inline int __div(int c_div)
{
	switch (c_div) {
	case DIV1_1:	return 1;
	case DIV1_2:	return 2;
	case DIV1_3:	return 3;
	case DIV1_4:	return 4;
	case DIV1_5:	return 5;
	case DIV1_6:	return 6;
	case DIV1_7:	return 7;
	case DIV1_8:	return 8;
	case DIV1_12:	return 12;
	case DIV1_16:	return 16;
	case DIV1_18:	return 18;
	case DIV1_24:	return 24;
	case DIV1_32:	return 32;
	case DIV1_36:	return 36;
	case DIV1_48:	return 48;
	case DIV1_96:	return 96;
	default:
		pr_err("invalid div-rate<%d>\n", c_div);
	return -EINVAL;
	}
	}

/*
 * check_restriction: validate the clock ratio
 * (return fail if violate restriction)
 *
 * Arguments:
 *		@rates: clocks div-rate.
 *
 * Return:
 *		0: normal
 *		-EINVAL: input clocks are violated restriction
 */
inline int check_restriction(const struct clk_rate rates)
{
	const int ratio[8] = {16, 12, 8, 6, 4, 3, 2, 1};
	int result[5] = {0, 0, 0, 0, 0};
	int i_div = 0, zs_div = 0, b_div = 0, hp_div = 0, zx_div = 0;
	int i = 0;

	/* get the div */
	i_div  = __div(rates.i_clk);
	zs_div = __div(rates.zs_clk);
	b_div  = __div(rates.b_clk);
	hp_div = __div(rates.hp_clk);
	zx_div = __div(rates.zx_clk);

	/* check for restriction */
	for (i = 0; i < (int)ARRAY_SIZE(ratio); i++) {
		/* I:ZS = N:1 * */
		if ((i_div * ratio[i]) == zs_div)
			result[0] = 1;
		/* ZS:B = N:1 * */
		if ((zs_div * ratio[i]) == b_div)
			result[1] = 1;
		/* I:HP = N:1 * */
		if ((i_div * ratio[i]) == hp_div)
			result[2] = 1;
		/* ZX:ZS = N:1 * */
		if ((zx_div * ratio[i]) == zs_div)
			result[3] = 1;
		/* ZS:HP = N:1 * (ZS:HP = 1:1 never happened???) */
		if ((zs_div * ratio[i]) == hp_div)
			result[4] = 1;
	}

	/* verify the result */
	for (i = 0; i < (int)ARRAY_SIZE(result); i++) {
		if (!result[i]) {
			pr_err("violate clock restriction, condition<%d>\n", i);
			return -EINVAL;
		}
	}

	return 0;
}

/*
 * cpg_set_pll: set pll ratio
 *
 * Arguments:
 *		@pll: pll id
 *		@val: ratio
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_set_pll(int pll, unsigned int val)
{
	unsigned int stc_val = 0;
	unsigned int pllcr = 0;
	unsigned int addr = CPG_PLL0CR;
	int timeout = 500;

	if (pll != PLL0) {
			/* support PLL0 only */
		pr_err("invalid parameter, PLL%d not supported\n", pll);
		return -EINVAL;
	}

	pllcr = __raw_readl(__io(IO_ADDRESS(addr)));
	stc_val = ((pllcr & PLLCR_STC_MASK) >> PLLCR_BIT24_SHIFT) + 1;
	if (val != stc_val) {
		int ret = 0;

		pllcr &= ~PLLCR_STC_MASK;
		pllcr |= ((val - 1) << PLLCR_BIT24_SHIFT);
#ifdef FRQCRA_B_SEM
		/* try to get the lock, timout:2ms */
		ret = hwspin_lock_timeout_nospin(gen_sem0, SEM_TIMEOUT);
		if (ret) {
			pr_err("fail to get hwsem, quit\n");
			return ret;
		}
#endif /* FRQCRA_B_SEM */
		__raw_writel(pllcr, __io(IO_ADDRESS(addr)));
		/* wait for status bit set */
		while (--timeout) {
			if (__raw_readl(CPG_PLLECR) & (1 << 8))
				break;
			ndelay(1000);
		}

		if (timeout <= 0)
			pr_err("wait PLL%d status timeout\n", pll);
#ifdef FRQCRA_B_SEM
		/* UNLOCK semaphore */
		hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
		return (timeout <= 0) ? -ETIMEDOUT : 0;
	}

	return 0;
}
/*
 * cpg_get_pll: set pll ratio
 *
 * Arguments:
 *		@pll: pll id
 *
 * Return:
 *		pll0 mult-ratio
 *		negative: operation fail
 */
int cpg_get_pll(int pll)
{
	unsigned int stc_val = 0;
	unsigned int pllcr = 0;
	unsigned int addr = CPG_PLL0CR;

	switch (pll) {
	case PLL0:
		addr = CPG_PLL0CR;
		break;
	case PLL1:
		addr = CPG_PLL1CR;
		break;
	case PLL2:
		addr = CPG_PLL2CR;
		break;
	case PLL3:
		addr = CPG_PLL3CR;
		break;
	default:
		pr_err("PLL<%d> not supported\n", pll);
			return -EINVAL;
		}

	pllcr = __raw_readl(__io(IO_ADDRESS(addr)));
	stc_val = ((pllcr & PLLCR_STC_MASK) >> PLLCR_BIT24_SHIFT) + 1;

	return (int)stc_val;
}

/*
 * cpg_wait_kick: wait for KICK bit change
 *
 * Arguments:
 *		@time: wait time.
 *
 * Return:
 *		0: successful
 * -EBUSY: unsuccessful
 */
int cpg_wait_kick(unsigned int time)
{
	unsigned int wait_time = time;

	while (0 < wait_time--) {
		if ((__raw_readl(CPG_FRQCRB) >> 31) == 0)
			break;
		ndelay(1000);
	}

	return (wait_time <= 0) ? -EBUSY : 0;
}

/*
 * cpg_set_kick: set and wait for KICK bit cleared
 *
 * Arguments:
 *		@time: wait time.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_set_kick(unsigned int time)
{
	unsigned int wait_time = time;

	if ((wait_time <= 0) || (wait_time > KICK_WAIT_INTERVAL_US))
		wait_time = KICK_WAIT_INTERVAL_US;
	__raw_writel(BIT(31) | __raw_readl(CPG_FRQCRB), CPG_FRQCRB);

	return cpg_wait_kick(wait_time);
}

/*
 * cpg_set_sbsc_freq: set SBSC clock(ZB3) frequency
 *
 * Arguments:
 *		@div: div-rate value.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_set_sbsc_freq(int div)
{
#if 0
	unsigned int reg = 0;
	unsigned int zb3rate = 0;
	unsigned int ckscr = 0;

	/* limit */
	if (ES_REV_2_2 > shmobile_chip_rev())
		return 0;

	if (div == DIV1_2) {
		/* upgrade frequency */
		reg |= BIT(15);
		__raw_writel(reg, CPG_FRQCRD);

		/* set and wait for KICK bit changed */
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			pr_err("error! set & wait KICK timeout\n");
			return -ETIMEDOUT;
		}

		/* CKSCR.ZB3MODE[14] = b'0 */
		ckscr = __raw_readl(CPG_CKSCR);
		ckscr &= ~BIT(14);
		__raw_writel(ckscr, CPG_CKSCR);

		/* W/A */
		reg |= BIT(2);
		__raw_writel(reg, CPG_FRQCRD);

		/* set and wait for KICK bit changed */
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			pr_err("error! set & wait KICK timeout\n");
			return -ETIMEDOUT;
		}

		/* CKSCR.ZB3MODE[14] = b'1 */
		ckscr = __raw_readl(CPG_CKSCR);
		ckscr |= BIT(14);
		__raw_writel(ckscr, CPG_CKSCR);
	} else {
		reg |= DIV_TO_HW(ZB3_CLK, div);
		/* CKSCR.ZB3MODE[14] = b'1 */
		ckscr = __raw_readl(CPG_CKSCR);
		__raw_writel(BIT(14) | ckscr, CPG_CKSCR);

		/* downgrade frequency */
		__raw_writel(reg, CPG_FRQCRD);
		/* set and wait for KICK bit changed */
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			pr_err("error! set & wait KICK timeout\n");
			return -ETIMEDOUT;
		}
	}

	pr_log("SDRAM REQ[0x%08x] REG[E0/E4/E8]=[0x%08x/0x%08x/0x%08x]\n", reg
		__raw_readl(CPG_FRQCRD - 0x4),
		__raw_readl(CPG_FRQCRD),
		__raw_readl(CPG_FRQCRD + 0x4));
#endif
	return 0;
}

#define HW_TO_DIV(reg, clk)	((reg >> __clk_hw_info[clk].shift_bit) & \
	__clk_hw_info[clk].mask_bit)
/*
 * cpg_get_freq: get div-rate of set of clock
 *
 * Arguments:
 *		@rates: return clocks div-rate.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_get_freq(struct clk_rate *rates)
{
	unsigned int frqcra = __raw_readl(CPG_FRQCRA);
	unsigned int frqcrb = __raw_readl(CPG_FRQCRB);
	unsigned int frqcrd = __raw_readl(CPG_FRQCRD);
	/* unsigned int zbckcr = __raw_readl(CPG_ZBCKCR); */

	if (!rates) {
		pr_err("invalid parameter<NULL>\n");
		return -EINVAL;
	}

	/* get the clock setting
	 * must execute in spin lock context
	 */
	rates->i_clk = __match_div_rate(I_CLK,
		HW_TO_DIV(frqcra, I_CLK));
	rates->zg_clk = __match_div_rate(ZG_CLK,
		HW_TO_DIV(frqcra, ZG_CLK));
	rates->b_clk = __match_div_rate(B_CLK,
		HW_TO_DIV(frqcra, B_CLK));
	rates->m1_clk = __match_div_rate(M1_CLK,
		HW_TO_DIV(frqcra, M1_CLK));
	rates->m3_clk = __match_div_rate(M3_CLK,
		HW_TO_DIV(frqcra, M3_CLK));
	rates->z_clk = __match_div_rate(Z_CLK,
		HW_TO_DIV(frqcrb, Z_CLK));
	rates->ztr_clk = __match_div_rate(ZTR_CLK,
		HW_TO_DIV(frqcrb, ZTR_CLK));
	rates->zt_clk = __match_div_rate(ZT_CLK,
		HW_TO_DIV(frqcrb, ZT_CLK));
	rates->zx_clk = __match_div_rate(ZX_CLK,
		HW_TO_DIV(frqcrb, ZX_CLK));
	rates->hp_clk = __match_div_rate(HP_CLK,
		HW_TO_DIV(frqcrb, HP_CLK));
	rates->zs_clk = __match_div_rate(ZS_CLK,
		HW_TO_DIV(frqcrb, ZS_CLK));
		rates->m5_clk = __match_div_rate(M5_CLK,
			HW_TO_DIV(frqcra, M5_CLK));
	rates->zb_clk = DIV1_6; /* dummy one */

	if ((frqcrd & FRQCRD_ZB30SEL) != 0)
		rates->zb3_clk = __match_div_rate(ZB3_CLK,
			HW_TO_DIV(frqcrd, ZB3_CLK));
	else
		rates->zb3_clk = DIV1_2;

	/* verify again */
	if ((rates->i_clk < 0) || (rates->zg_clk < 0)
		|| (rates->b_clk < 0) || (rates->m1_clk < 0)
		|| (rates->m3_clk < 0) || (rates->z_clk < 0)
		|| (rates->ztr_clk < 0) || (rates->zt_clk < 0)
		|| (rates->zx_clk < 0) || (rates->hp_clk < 0)
		|| (rates->zs_clk < 0) || (rates->zb3_clk < 0)) {
		pr_err("invalid divrate\n");
		return -EINVAL;
	}

	return 0;
}
EXPORT_SYMBOL(cpg_get_freq);

/*
 * cpg_set_freqval: set div-rate of special clock
 *
 * Arguments:
 *		@clk: clock.
 *		@div: set div-rate
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_set_freqval(int clk, int div)
{
	struct clk_hw_info *info = NULL;
	unsigned long flags = 0;
	unsigned long zsflags = 0;
	unsigned int reg = 0;

	spin_lock_irqsave(&freq_change_lock, flags);
	if (clk == ZB3_CLK) {
		int ret = 0;
#ifdef FRQCRA_B_SEM
		/* try to get the lock, timout:2ms */
		if (hwspin_lock_timeout_nospin(gen_sem0, SEM_TIMEOUT)) {
			pr_err("fail to get hwsem, quit\n");
			spin_unlock_irqrestore(&freq_change_lock, flags);
			return -EBUSY;
		}
#endif /* FRQCRA_B_SEM */
		ret = cpg_set_sbsc_freq(div);
		if (ret)
			pr_err("fail to change sdram div-rate<%d>\n", div);
#ifdef FRQCRA_B_SEM
		/* UNLOCK semaphore */
		hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_log("REQ[%d] REG[E0/E4/E8]=[0x%08x/0x%08x/0x%08x]\n", div,
			__raw_readl(CPG_FRQCRD - 0x4),
			__raw_readl(CPG_FRQCRD),
			__raw_readl(CPG_FRQCRD + 0x4));
		return ret;
	}

	/* HP clock setting busy */
	if (the_clock.zs_disabled_cnt && (clk == ZS_CLK)) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("error! ZS clock setting busy\n");
		return -EBUSY;
	}

	/* HP clock setting busy */
	if (the_clock.hp_disabled_cnt && (clk == HP_CLK)) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("error! HP clock setting busy\n");
		return -EBUSY;
	}

	/* invalid clock div */
	info = &__clk_hw_info[clk];
	if (info->div_val[div] < 0) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("error! invalid div-rate<%d>\n", info->div_val[div]);
		return -EINVAL;
	}

	/* need KICK bit */
	if ((clk != ZB_CLK) && (clk != ZB3_CLK)) {
		if (cpg_wait_kick(KICK_WAIT_INTERVAL_US)) {
			spin_unlock_irqrestore(&freq_change_lock, flags);
			pr_err("error! wait KICK timeout\n");
			return -ETIMEDOUT;
		}
	}

	reg = __raw_readl(info->addr);
	reg &= ~(info->mask_bit << info->shift_bit);
	reg |= DIV_TO_HW(clk, div);
#ifdef FRQCRA_B_SEM
	/* try to get the lock, timout:2ms */
	if (hwspin_lock_timeout_nospin(gen_sem0, SEM_TIMEOUT)) {
		pr_err("fail to get hwsem, quit\n");
		if (clk == ZS_CLK)
			spin_unlock_irqrestore(&zs_lock, zsflags);

		spin_unlock_irqrestore(&freq_change_lock, flags);
		return -EBUSY;
	}
#endif /* FRQCRA_B_SEM */
	/* need to ensure ZS-phy is permissed to change */
	if (clk == ZS_CLK) {
		spin_lock_irqsave(&zs_lock, zsflags);
		/* try to get the lock, timout:2ms */
		if (hwspin_lock_timeout_nospin(gen_sem1, SEM_TIMEOUT)) {
			pr_err("fail to get hwsem, quit\n");
			spin_unlock_irqrestore(&zs_lock, zsflags);
			spin_unlock_irqrestore(&freq_change_lock, flags);
			return -EBUSY;
		}
	}

	__raw_writel(reg, info->addr);
	/* need KICK bit */
	if ((clk != ZB_CLK) && (clk != ZB3_CLK)) {
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			/* ZS change */
			if (clk == ZS_CLK) {
				hwspin_unlock_nospin(gen_sem1);
				spin_unlock_irqrestore(&zs_lock, zsflags);
			}
#ifdef FRQCRA_B_SEM
			/* UNLOCK semaphore */
			hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
			spin_unlock_irqrestore(&freq_change_lock, flags);
			pr_err("error! set & wait KICK timeout\n");
			return -ETIMEDOUT;
		}
	}

	/* ZS change */
	if (clk == ZS_CLK) {
		hwspin_unlock_nospin(gen_sem1);
		spin_unlock_irqrestore(&zs_lock, zsflags);
	}
#ifdef FRQCRA_B_SEM
	/* UNLOCK semaphore */
	hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
	spin_unlock_irqrestore(&freq_change_lock, flags);

	return 0;
}

/*
 * cpg_get_freqval: get div-rate of special clock
 *
 * Arguments:
 *		@clk: clock.
 *		@div: return div-rate
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_get_freqval(int clk, int *div)
{
	unsigned long flags;
	int div_rate = 0;

	spin_lock_irqsave(&freq_change_lock, flags);
	if (!div) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("error! invalid parameter<NULL>\n");
		return -EINVAL;
	}

	/* get divrate */
	if (clk == ZB3_CLK) {
		div_rate = HW_TO_DIV(__raw_readl(CPG_FRQCRD), clk);
	} else if (clk == ZB_CLK) {
		div_rate = HW_TO_DIV(__raw_readl(CPG_ZBCKCR), clk);
	} else if ((clk == ZB_CLK) || (clk == ZG_CLK) || (clk == B_CLK)
		|| (clk == M1_CLK) || (clk == M3_CLK) || (clk == I_CLK)) {
		div_rate = HW_TO_DIV(__raw_readl(CPG_FRQCRA), clk);
	} else if ((clk == Z_CLK) || (clk == ZTR_CLK) || (clk == ZT_CLK)
		|| (clk == ZX_CLK) || (clk == HP_CLK) || (clk == ZS_CLK)) {
		div_rate = HW_TO_DIV(__raw_readl(CPG_FRQCRB), clk);
	} else if ((shmobile_chip_rev() >= ES_REV_2_0) &&
		(clk == M5_CLK)) {
		div_rate = HW_TO_DIV(__raw_readl(CPG_FRQCRA), clk);
	} else {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("error! invalid clock<%d>\n", clk);
		return -EINVAL;
	}

	/* ZB3 == x1/2? */
	if ((clk == ZB3_CLK) && ((div_rate & 0x1f) == 0))
		div_rate = DIV1_2;
	else
	div_rate = (int)__match_div_rate((enum clk_type)clk, div_rate);

	/* invalid H/W setting? */
	if (div_rate < 0) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("error! invalid div-rate<%d>\n", div_rate);
		return -EINVAL;
	}

	*div = div_rate;
	spin_unlock_irqrestore(&freq_change_lock, flags);

	return 0;
}

/*
 * cpg_set_freq: set div-rate for set of clock
 *
 * Arguments:
 *		@rates: clocks div-rate.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int cpg_set_freq(const struct clk_rate rates)
{
	struct clk_rate curr_rates;
	unsigned long zsflags = 0;
	unsigned long flags = 0;
	unsigned int reg = 0;
	int frq_change = 0;
	int zb3_change = 0;
	int zs_change = 0;
	int ret = 0;

	spin_lock_irqsave(&freq_change_lock, flags);
	/* violate restriction? */
	if (check_restriction(rates)) {
		/* ret = -EINVAL; */
		pr_log("%s()[%d]: warning, violate restriction\n",
			__func__, __LINE__);
		/* goto done; */
	}

	/* get current setting, if nor success try to apply new one */
	if (cpg_get_freq(&curr_rates)) {
		pr_warning("error! can not get current setting, try new one\n");
		/* previous H/W setting not correct, try with new one */
		(void)memcpy(&curr_rates, &rates, sizeof(struct clk_rate));
	}

	/* change FRQCR(A/B) ? */
	if ((curr_rates.i_clk != rates.i_clk)
		|| (curr_rates.zg_clk != rates.zg_clk)
		|| (curr_rates.b_clk != rates.b_clk)
		|| (curr_rates.m1_clk != rates.m1_clk)
		|| (curr_rates.m3_clk != rates.m3_clk)
		|| (curr_rates.ztr_clk != rates.ztr_clk)
		|| (curr_rates.zt_clk != rates.zt_clk)
		|| (curr_rates.zx_clk != rates.zx_clk)
		|| (curr_rates.hp_clk != rates.hp_clk)
		|| (curr_rates.zs_clk != rates.zs_clk)) {
		frq_change = 1;
	}

	/* enable HP change? */
	if ((!the_clock.hp_disabled_cnt) &&
		(curr_rates.hp_clk != rates.hp_clk))
		frq_change = 1;

	/* enable ZS change? */
	if ((!the_clock.zs_disabled_cnt) &&
		(curr_rates.zs_clk != rates.zs_clk)) {
		frq_change = 1;
		zs_change = 1;

		/* try to get the lock, timout:2ms */
		ret = hwspin_lock_timeout_nospin(gen_sem1, SEM_TIMEOUT);
		if (ret) {
			pr_err("fail to get hwsem, quit\n");
			goto done;
		}
	}

	/* change ZB3 clock ? */
	if (curr_rates.zb3_clk != rates.zb3_clk)
		zb3_change = 1;

	/* wait for KICK bit change (if any) */
	ret = cpg_wait_kick(KICK_WAIT_INTERVAL_US);
	if (ret) {
		pr_err("error! wait KICK timeout\n");
			goto done;
	}
#ifdef FRQCRA_B_SEM
	/* try to get the lock, timout:2ms */
	ret = hwspin_lock_timeout_nospin(gen_sem0, SEM_TIMEOUT);
	if (ret) {
		pr_err("fail to get hwsem, quit\n");
		goto done;
	}
#endif /* FRQCRA_B_SEM */
	/* change FRQCR(A/B) ? */
	if (frq_change) {
		/* not change Z-Phy, use the current one */
		reg = DIV_TO_HW(Z_CLK, curr_rates.z_clk);
		/* apply new setting */
		reg |= DIV_TO_HW(ZTR_CLK, rates.ztr_clk);
		reg |= DIV_TO_HW(ZT_CLK, rates.zt_clk);
		reg |= DIV_TO_HW(ZX_CLK, rates.zx_clk);

		/* change HP */
		if (!the_clock.hp_disabled_cnt)
			reg |= DIV_TO_HW(HP_CLK, rates.hp_clk);
		else /* keep current HP-Phy */
			reg |= DIV_TO_HW(HP_CLK, curr_rates.hp_clk);

		/* change ZS clock */
		if (zs_change) {
			spin_lock_irqsave(&zs_lock, zsflags);
			reg |= DIV_TO_HW(ZS_CLK, rates.zs_clk);
		} else {
			/* keep current ZS-Phy */
			reg |= DIV_TO_HW(ZS_CLK, curr_rates.zs_clk);
		}

		/* apply setting */
		__raw_writel(reg, CPG_FRQCRB);

		reg = DIV_TO_HW(I_CLK, rates.i_clk);
		reg |= DIV_TO_HW(ZG_CLK, rates.zg_clk);
		reg |= DIV_TO_HW(B_CLK, rates.b_clk);
		reg |= DIV_TO_HW(M1_CLK, rates.m1_clk);
		reg |= DIV_TO_HW(M3_CLK, rates.m3_clk);
		reg |= DIV_TO_HW(M5_CLK, curr_rates.m5_clk);

		/* apply setting */
		__raw_writel(reg, CPG_FRQCRA);
		pr_log("FRQCR[A/B]=[0x%08X/0x%08X]\n",
			__raw_readl(CPG_FRQCRA),
			__raw_readl(CPG_FRQCRB));

		/* set and wait for KICK bit changed */
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			pr_err("error! set & wait KICK timeout\n");
			if (zs_change) {
				spin_unlock_irqrestore(&zs_lock, zsflags);
				hwspin_unlock_nospin(gen_sem1);
			}
#ifdef FRQCRA_B_SEM
			/* UNLOCK semaphore */
			hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
			goto done;
		}

		if (zs_change) {
			spin_unlock_irqrestore(&zs_lock, zsflags);
			hwspin_unlock_nospin(gen_sem1);
		}
	}

	/* change ZB3 clock ? */
	if (zb3_change)
		ret = cpg_set_sbsc_freq((int)rates.zb3_clk);
#ifdef FRQCRA_B_SEM
	/* UNLOCK semaphore */
	hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
done:
	spin_unlock_irqrestore(&freq_change_lock, flags);
	return ret;
}

/*
 * corestandby_cpg_set_freq: set div-rate for set of clock
 *
 * Arguments:
 *		@rates: clocks div-rate.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int corestandby_cpg_set_freq(const struct clk_rate rates)
{
	struct clk_rate curr_rates;
	unsigned long zsflags = 0;
	unsigned long flags = 0;
	unsigned int reg = 0;
	int frq_change = 0;
	int zb3_change = 0;
	int zs_change = 0;
	int ret = 0;

	/* violate restriction? */
	if (check_restriction(rates)) {
		/* ret = -EINVAL; */
		pr_log("%s()[%d]: warning, violate restriction\n",
			__func__, __LINE__);
		/* goto done; */
	}

	/* get current setting, if nor success try to apply new one */
	if (cpg_get_freq(&curr_rates)) {
		pr_warning("error! can not get current setting, try new one\n");
		/* previous H/W setting not correct, try with new one */
		(void)memcpy(&curr_rates, &rates, sizeof(struct clk_rate));
	}

	/* change FRQCR(A/B) ? */
	if ((curr_rates.i_clk != rates.i_clk)
		|| (curr_rates.zg_clk != rates.zg_clk)
		|| (curr_rates.b_clk != rates.b_clk)
		|| (curr_rates.m1_clk != rates.m1_clk)
		|| (curr_rates.m3_clk != rates.m3_clk)
		|| (curr_rates.ztr_clk != rates.ztr_clk)
		|| (curr_rates.zt_clk != rates.zt_clk)
		|| (curr_rates.zx_clk != rates.zx_clk)
		|| (curr_rates.hp_clk != rates.hp_clk)
		|| (curr_rates.zs_clk != rates.zs_clk)) {
		frq_change = 1;
	}

	/* enable HP change? */
	if ((!the_clock.hp_disabled_cnt) &&
		(curr_rates.hp_clk != rates.hp_clk))
		frq_change = 1;

	/* enable ZS change? */
	if ((!the_clock.zs_disabled_cnt) &&
		(curr_rates.zs_clk != rates.zs_clk)) {
		frq_change = 1;
		zs_change = 1;

		/* try to get the lock, timout:2ms */
		ret = hwspin_lock_timeout_nospin(gen_sem1, SEM_TIMEOUT);
		if (ret) {
			pr_err("fail to get hwsem, quit\n");
			goto done;
		}
	}

	/* change ZB3 clock ? */
	if (curr_rates.zb3_clk != rates.zb3_clk)
		zb3_change = 1;

	/* wait for KICK bit change (if any) */
	ret = cpg_wait_kick(KICK_WAIT_INTERVAL_US);
	if (ret) {
		pr_err("error! wait KICK timeout\n");
			goto done;
	}
#ifdef FRQCRA_B_SEM
	/* try to get the lock, timout:2ms */
	ret = hwspin_lock_timeout_nospin(gen_sem0, SEM_TIMEOUT);
	if (ret) {
		pr_err("fail to get hwsem, quit\n");
		goto done;
	}
#endif /* FRQCRA_B_SEM */
	/* change FRQCR(A/B) ? */
	if (frq_change) {
		/* not change Z-Phy, use the current one */
		reg = DIV_TO_HW(Z_CLK, curr_rates.z_clk);
		/* apply new setting */
		reg |= DIV_TO_HW(ZTR_CLK, rates.ztr_clk);
		reg |= DIV_TO_HW(ZT_CLK, rates.zt_clk);
		reg |= DIV_TO_HW(ZX_CLK, rates.zx_clk);

		/* change HP */
		if (!the_clock.hp_disabled_cnt)
			reg |= DIV_TO_HW(HP_CLK, rates.hp_clk);
		else /* keep current HP-Phy */
			reg |= DIV_TO_HW(HP_CLK, curr_rates.hp_clk);

		/* change ZS clock */
		if (zs_change) {
			reg |= DIV_TO_HW(ZS_CLK, rates.zs_clk);
		} else {
			/* keep current ZS-Phy */
			reg |= DIV_TO_HW(ZS_CLK, curr_rates.zs_clk);
		}

		/* apply setting */
		__raw_writel(reg, CPG_FRQCRB);

		reg = DIV_TO_HW(I_CLK, rates.i_clk);
		reg |= DIV_TO_HW(ZG_CLK, rates.zg_clk);
		reg |= DIV_TO_HW(B_CLK, rates.b_clk);
		reg |= DIV_TO_HW(M1_CLK, rates.m1_clk);
		reg |= DIV_TO_HW(M3_CLK, rates.m3_clk);
		reg |= DIV_TO_HW(M5_CLK, curr_rates.m5_clk);

		/* apply setting */
		__raw_writel(reg, CPG_FRQCRA);
		pr_log("FRQCR[A/B]=[0x%08X/0x%08X]\n",
			__raw_readl(CPG_FRQCRA),
			__raw_readl(CPG_FRQCRB));

		/* set and wait for KICK bit changed */
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			pr_err("error! set & wait KICK timeout\n");
			if (zs_change)
				hwspin_unlock_nospin(gen_sem1);
#ifdef FRQCRA_B_SEM
			/* UNLOCK semaphore */
			hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
			goto done;
		}

		if (zs_change)
			hwspin_unlock_nospin(gen_sem1);
	}

	/* change ZB3 clock ? */
	if (zb3_change)
		ret = cpg_set_sbsc_freq((int)rates.zb3_clk);
#ifdef FRQCRA_B_SEM
	/* UNLOCK semaphore */
	hwspin_unlock_nospin(gen_sem0);
#endif /* FRQCRA_B_SEM */
done:
	return ret;
}

/*
 * cpg_set_zdiv: change waveform controlling Z-div clock
 *
 * Arguments:
 *		@mode: waveform level
 *
 * Return:
 *		0: operation success
 *		negative: operation fail
 */
static int cpg_set_zdiv(int mode)
{
	unsigned int zdivcr = 0xFFF00000;
	int i = 0;
	int ret = -EINVAL;

	/* Validate LSI revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	/* Validate waveform mode
	 * -> support LLx1_16 to LLx16_16
	 */
	if ((mode < LLx1_16) || (mode > LLx16_16)) {
		pr_err("invalid div mode<%d>\n", mode);
		return ret;
	}

	/* Change waveform setting
	 * -> setting indecates by 'mode' and define by 'waveform_map'
	 *    need to search
	 */
	for (i = 0; i < ARRAY_SIZE(waveform_map); i++) {
		if (waveform_map[i].mode == mode) {
			/* BIT(18) should be set when 'mode' from 1 -> 8
			 * else, pls clear
			 */
			if (mode <= 8)
				zdivcr |= BIT(18);
			else
				zdivcr &= ~BIT(18);

			zdivcr |= waveform_map[i].waveform;
			__raw_writel(zdivcr, ZDIVCR5);

			/* BIT(29) should be clear when all done
			 */
			zdivcr &= ~BIT(29);
			__raw_writel(zdivcr, ZDIVCR5);

			pr_log("ZDIVCR[0x%08x]\n", __raw_readl(ZDIVCR5));
			return 0;
		}
	}

	return ret;
}
/******************************************************************************/
/* PM APIs ********************************************************************/
/******************************************************************************/
int pm_setup_clock(void)
{
	unsigned long flags;
	unsigned int zbckcr = 0;
	unsigned int cnt_zb = 1000;/*Wait 1ms*/

	spin_lock_irqsave(&freq_change_lock, flags);
	if (shmobile_chip_rev() >= ES_REV_2_0) {
		__shmobile_freq_modes = __shmobile_freq_modes_es2_x;
		__clk_hw_info = __clk_hw_info_es2_x;
	} else {
		__shmobile_freq_modes = __shmobile_freq_modes_es1_x;
		__clk_hw_info = __clk_hw_info_es1_x;
	}

	the_clock.zs_disabled_cnt = 0;
	the_clock.hp_disabled_cnt = 0;

	gen_sem1 = hwspin_lock_request_specific(SMGP100_DFS_ZS);
	if (!gen_sem1) {
		pr_err("error! fail to get semaphore<SMGP100_DFS_ZS>\n");
		the_clock.zs_disabled_cnt = ~0;
	}
#ifdef FRQCRA_B_SEM
	gen_sem0 = hwspin_lock_request_specific(SMGP001_DFS);
	if (!gen_sem0)
		pr_err("error! fail to get semaphore<SMGP001_DFS>\n");
#endif /* FRQCRA_B_SEM */
	/* stop ZB-Phy */
	while (cnt_zb--) {
		zbckcr = __raw_readl(CPG_ZBCKCR);
		if (zbckcr & BIT(8)) {
			pr_info("ZB-Phy stopped, ZBCKCR<0x%08x>\n", zbckcr);
			break;
		}
		zbckcr |= BIT(8);
		__raw_writel(zbckcr, CPG_ZBCKCR);
		udelay(1);
	}
	if (!cnt_zb)
		pr_err("fail to stop ZB-Phy, ZBCKCR<0x%08x>\n", zbckcr);

	spin_unlock_irqrestore(&freq_change_lock, flags);

	return 0;
}

/*
 * pm_set_clocks: set frequencies value
 *
 * Arguments:
 *		@rates: frequencies.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_set_clocks(const struct clk_rate rates)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate())
		return cpg_set_freq(rates);

	return 0;
}
EXPORT_SYMBOL(pm_set_clocks);

/*
 * corestandby_pm_set_clocks: set frequencies value
 *
 * Arguments:
 *		@rates: frequencies.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int corestandby_pm_set_clocks(const struct clk_rate rates)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate())
		return corestandby_cpg_set_freq(rates);

	return 0;
}
EXPORT_SYMBOL(corestandby_pm_set_clocks);


/*
 * pm_set_clock_mode: set frequencies value
 *
 * Arguments:
 *		@mode: frequency mode.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_set_clock_mode(int mode)
{
	struct clk_rate rates;
	int z_rate = 0;
	int size = 0;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	if (shmobile_chip_rev() >= ES_REV_2_0)
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es2_x);
	else
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es1_x);

	if ((mode >= size) || (mode < 0)) {
		pr_err("invalid clock mode<%d>\n", mode);
		return -EINVAL;
	}

	(void)memcpy(&rates, &__shmobile_freq_modes[mode],
		sizeof(struct clk_rate));

	/* not allow to change Z-Phy using this APIs */
	ret = cpg_get_freqval(Z_CLK, &z_rate);
	if (!ret) {
		rates.z_clk = z_rate;
		ret = cpg_set_freq(rates);
	}

	return ret;
}
EXPORT_SYMBOL(pm_set_clock_mode);

/*
 * pm_get_clock_mode: get frequencies value
 *
 * Arguments:
 *		@mode: frequency mode.
 *		@rate: frequencies value
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_get_clock_mode(int mode, struct clk_rate *rate)
{
	int size = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return 0;

	if (shmobile_chip_rev() >= ES_REV_2_0)
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es2_x);
	else
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es1_x);

	/* invalid parameter? */
	if ((!rate) || (mode >= size) || (mode < 0)) {
		pr_err("invalid clock mode<%d>\n", mode);
		return -EINVAL;
	}

	(void)memcpy(rate, &__shmobile_freq_modes[mode],
		sizeof(struct clk_rate));

	return 0;
}
EXPORT_SYMBOL(pm_get_clock_mode);

/*
 * pm_set_syscpu_frequency: change SYS-CPU frequency
 *
 * Arguments:
 *		div-rate.
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_set_syscpu_frequency(int div)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate()) {
#ifdef ZFREQ_MODE
		return cpg_set_zdiv(div);
#else /* !ZFREQ_MODE */
	return cpg_set_freqval(Z_CLK, div);
#endif /* ZFREQ_MODE */
	}

	return 0;
}
EXPORT_SYMBOL(pm_set_syscpu_frequency);

/*
 * pm_get_syscpu_frequency: get SYS-CPU frequency
 *
 * Arguments:
 *		none
 *
 * Return:
 *    0: unsuccessful
 */
unsigned int pm_get_syscpu_frequency(void)
{
	int ret = 0;
	int div = 0;
	int pll0 = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return 0;

#ifdef ZFREQ_MODE
	pr_alert("should not use\n");
#endif /* ZFREQ_MODE */
	ret = cpg_get_freqval(Z_CLK, &div);
	if (!ret) {
		pll0 = cpg_get_pll(PLL0);
		if (pll0 > 0) {
			switch (div) {
			case DIV1_1: return (pll0 * 26000);
			case DIV1_2: return (pll0 * 13000);
			case DIV1_4: return (pll0 *  6500);
			}
		}
	}

	return 0;
}
EXPORT_SYMBOL(pm_get_syscpu_frequency);

/*
 * pm_disable_clock_change: disable ZS/HP clock
 *
 * Arguments:
 *		@clk: clock
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_disable_clock_change(int clk)
{
	unsigned long flags;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock_irqsave(&freq_change_lock, flags);
	/* support ZS/HP only */
	if (!(clk & (ZSCLK | HPCLK))) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("invalid parameter\n");
		return -EINVAL;
	}

	/* ZS clock is requested? */
	if ((clk & ZSCLK))
		the_clock.zs_disabled_cnt++;

	/* HP clock is requested? */
	if ((clk & HPCLK))
		the_clock.hp_disabled_cnt++;

	spin_unlock_irqrestore(&freq_change_lock, flags);

	return ret;
}
EXPORT_SYMBOL(pm_disable_clock_change);

/*
 * pm_enable_clock_change: enable ZS/HP clock
 *
 * Arguments:
 *		@clk: clock
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_enable_clock_change(int clk)
{
	unsigned long flags;
	int ret = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (!validate())
		return ret;

	spin_lock_irqsave(&freq_change_lock, flags);

	/* validate parameter, support ZS/HP only */
	if (!(clk & (ZSCLK | HPCLK))) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		return -EINVAL;
	}

	/* ZS clock is enabled? */
	if ((clk & ZSCLK)) {
		if (the_clock.zs_disabled_cnt)
			the_clock.zs_disabled_cnt--;
	}

	/* HP clock is enabled? */
	if ((clk & HPCLK)) {
		if (the_clock.hp_disabled_cnt)
			the_clock.hp_disabled_cnt--;
	}

	spin_unlock_irqrestore(&freq_change_lock, flags);

	return ret;
}
EXPORT_SYMBOL(pm_enable_clock_change);

/*
 * pm_set_pll_ratio: change pll ratio
 *
 * Arguments:
 *		@pll: pll type
 *		@val: set value
 *
 * Return:
 *		0: successful
 *		negative: operation fail
 */
int pm_set_pll_ratio(int pll, unsigned int val)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate())
		return cpg_set_pll(pll, val);

	return 0;
}
EXPORT_SYMBOL(pm_set_pll_ratio);

/*
 * pm_get_pll_ratio: get pll ratio
 *
 * Arguments:
 *		@pll: pll type
 *
 * Return:
 *		pll0 mult-ratio
 *		negative: operation fail
 */
int pm_get_pll_ratio(int pll)
{
	return cpg_get_pll(pll);
}
EXPORT_SYMBOL(pm_get_pll_ratio);

/*
 * pm_spin_lock_irqrestore: spinlock(zs lock)
 *
 * Arguments:
 *		@flag: irq flag
 *
 * Return:
 *		None
 */
unsigned long pm_get_spinlock(void)
{
	unsigned long flag = 0;

	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate())
		spin_lock_irqsave(&zs_lock, flag);

	return flag;
}
EXPORT_SYMBOL(pm_get_spinlock);

/*
 * pm_spin_unlock_irqrestore: spinunlock(zs lock)
 *
 * Arguments:
 *		@flag: irq flag
 *
 * Return:
 *		None
 */
void pm_release_spinlock(unsigned long flag)
{
	/* validate chip revision
	 * -> support revision 2.x and later
	 */
	if (validate())
		spin_unlock_irqrestore(&zs_lock, flag);
}
EXPORT_SYMBOL(pm_release_spinlock);

/**** DebugFS Tuning****/
#if defined(CONFIG_DEBUG_FS)

#include <linux/debugfs.h>

static struct dentry *clk_debugfs_root;

struct dentry *array_dent[ARRAY_SIZE(__shmobile_freq_modes_es2_x)];

static int clk_debugfs_register_one(unsigned int index)
{
	int err;
	struct dentry *d;
	char s[255];
	char *p = s;

	p += sprintf(p, "opp-%d", index);
	d = debugfs_create_dir(s, clk_debugfs_root);
	if (!d)
		return -ENOMEM;
	array_dent[index] = d;

	d = debugfs_create_u8("i_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].i_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("zg_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].zg_clk);
	if (!d) {
		err = -ENOMEM;
	goto err_out;
	}

	d = debugfs_create_u8("b_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].b_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("m1_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].m1_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("m3_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].m3_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("m5_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].m5_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("z_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].z_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("ztr_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].ztr_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("zt_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].zt_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("zx_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].zx_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("hp_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].hp_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("zs_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].zs_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("zb_clk", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].zb_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u32("zb3_clk", S_IRWXUGO, array_dent[index],
			(u32 *)&__shmobile_freq_modes_es2_x[index].zb3_clk);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	d = debugfs_create_u8("pll0", S_IRWXUGO, array_dent[index],
			(u8 *)&__shmobile_freq_modes_es2_x[index].pll0);
	if (!d) {
		err = -ENOMEM;
		goto err_out;
	}

	return 0;

err_out:
	d = array_dent[index];
	debugfs_remove_recursive(array_dent[index]);
	return err;
}

static int __init clk_debugfs_init(void)
{
	struct dentry *d;
	int err;
	unsigned int i;

	printk(KERN_INFO "\n%s() started!");
	d = debugfs_create_dir("clk_dfs", NULL);
	if (!d)
		return -ENOMEM;

	clk_debugfs_root = d;

	for (i = 0; i < ARRAY_SIZE(__shmobile_freq_modes_es2_x); i++) {
		err = clk_debugfs_register_one(i);
		if (err)
			goto err_out;
	}

	return 0;
err_out:
	debugfs_remove_recursive(clk_debugfs_root);
	printk(KERN_INFO "\n%s() completed -> ret = %d", err);
	return err;
}
late_initcall(clk_debugfs_init);

#endif
