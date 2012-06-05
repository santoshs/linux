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

#include <linux/io.h>

#include <mach/r8a73734.h>
#include <mach/pm.h>

#define CPG_BASE		IO_ADDRESS(0xE6150000)
#define CPG_FRQCRA		(CPG_BASE + 0x0000)
#define CPG_FRQCRB		(CPG_BASE + 0x0004)
#define CPG_ZBCKCR		(CPG_BASE + 0x0010)
#define CPG_FRQCRD		(CPG_BASE + 0x00E4)
#define CPG_PLL0CR		(CPG_BASE + 0x00D8)
#define CPG_PLL1CR		(CPG_BASE + 0x0028)
#define CPG_PLL2CR		(CPG_BASE + 0x002C)
#define CPG_PLL3CR		(CPG_BASE + 0x00DC)
#define PLLCR_STC_MASK			0x3F000000
#define PLLCR_BIT24_SHIFT		24

#define FRQCRD_ZB30SEL			BIT(4)
#define KICK_WAIT_INTERVAL_US	500

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
	int				div_val[16];
	void __iomem	*addr;
};

/*******************************************************************************
 * PM APIs *********************************************************************
 ******************************************************************************/
static DEFINE_SPINLOCK(freq_change_lock);
static DEFINE_SPINLOCK(zs_change_lock);
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
		.mask_bit	= 0x1bf,
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
		.mask_bit	= 0x1bf,
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
		/* suspend, SGX on/off */
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
		/* Normal, SGX on, CPU:MAX */
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
		/* Normal, SGX on, CPU:MID */
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
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
	{
		/* Normal, SGX on, CPU:MIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_4,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
#ifdef SH_CPUFREQ_VERYLOW
	{
		/* Normal, SGX on, CPU:ExMIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_6,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
#endif
	{
		/* Normal, SGX off, CPU:MAX */
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
		/* Normal, SGX off, CPU:MID */
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
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
	{
		/* Normal, SGX off, CPU:MIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_4,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
#ifdef SH_CPUFREQ_VERYLOW
	{
		/* Normal, SGX off, CPU:ExMIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_6,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.z_clk = DIV1_6,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_4
	},
#endif /* SH_CPUFREQ_VERYLOW */
	{
		/* Earlysuspend, SGX on, CPU:MAX */
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
		/* Earlysuspend, SGX on, CPU:MID */
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
		/* Earlysuspend, SGX on, CPU:MIN */
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
	},
	{
		/* Earlysuspend, SGX off, CPU:MAX */
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
		/* Earlysuspend, SGX off, CPU:MID */
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
		/* Earlysuspend, SGX off, CPU:MIN */
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
		.b_clk = DIV1_24,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_2,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_6,
		.hp_clk = DIV1_24,
		.zs_clk = DIV1_24,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_16,
		.pll0 = PLLx46
	},
#ifdef SH_CPUFREQ_OVERDRIVE
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
		.hp_clk = DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx56
	},
#endif /* SH_CPUFREQ_OVERDRIVE */
	{
		/* Normal, CPU:HIGH */
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
		.hp_clk = DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx46
	},
	{
		/* Normal, CPU:MID */
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
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx46
	},
	{
		/* Normal, CPU:MIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_4,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx46
	},
#ifdef SH_CPUFREQ_VERYLOW
	{
		/* Normal, CPU:ExMIN */
		.i_clk = DIV1_6,
		.zg_clk = DIV1_4,
		.b_clk = DIV1_12,
		.m1_clk = DIV1_6,
		.m3_clk = DIV1_8,
		.m5_clk = DIV1_7,
		.z_clk = DIV1_6,
		.ztr_clk = DIV1_4,
		.zt_clk = DIV1_6,
		.zx_clk = DIV1_3,
		.hp_clk	= DIV1_12,
		.zs_clk = DIV1_6,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_2,
		.pll0 = PLLx46
	},
#endif /* SH_CPUFREQ_VERYLOW */
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
		.zx_clk = DIV1_6,
		.hp_clk = DIV1_24,
		.zs_clk = DIV1_24,
		.zb_clk = DIV1_6,
		.zb3_clk = DIV1_16,
		.pll0 = PLLx46
	}
};
#ifndef SHM_CLK_TEST_MODE
static
#endif
struct clk_rate *__shmobile_freq_modes;
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

	for (i = 0; i < len; i++) {
		if (__clk_hw_info[clk].div_val[i] == val)
			return div_table[i];
		}
	/* not expected(invalid value), just in case! */
	pr_err("%s()[%d]: error! clk<%d>, div<%d> invalid\n",
		__func__, __LINE__, clk, val);
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
	default: {
	pr_err("%s()[%d]: error! divrate<%d> invalid\n",
		__func__, __LINE__, c_div);
	return -EINVAL;
	}
	}
}

/*
 * __validate: validate the clock ratio (return fail if violate restriction)
 *
 * Arguments:
 *		@rates: clocks div-rate.
 *
 * Return:
 *		0: normal
 *		-EINVAL: input clocks are violated restriction
 */
inline int __validate(const struct clk_rate rates)
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
			pr_log("%s()[%d]: warning! violate restriction[%d]\n",
				__func__, __LINE__, i);
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

	if (shmobile_chip_rev() >= ES_REV_2_0) {
		if ((pll != PLL0) || ((val != PLLx56) && (val != PLLx46))) {
			/* support PLL0 only */
			pr_err("%s()[%d]: PLL<%d>, ratio<%d> not supported, ret<%d>\n",
				__func__, __LINE__, pll, val, -EINVAL);
			return -EINVAL;
		}
	} else if (shmobile_chip_rev() >= ES_REV_1_0) {
		if ((pll != PLL0) || (val != PLLx38)) {
			/* support PLL0 only */
			pr_err("%s()[%d]: PLL<%d>, ratio<%d> not supported, ret<%d>\n",
				__func__, __LINE__, pll, val, -EINVAL);
			return -EINVAL;
		}
	} else {
		return -EINVAL;
	}

	pllcr = __raw_readl(__io(IO_ADDRESS(addr)));
	stc_val = ((pllcr & PLLCR_STC_MASK) >> PLLCR_BIT24_SHIFT) + 1;
	if (val != stc_val) {
		pllcr &= ~PLLCR_STC_MASK;
		pllcr |= ((val - 1) << PLLCR_BIT24_SHIFT);
		pr_log("%s()[%d]: set PLL<%d>, reg<0x%x>\n", __func__, __LINE__,
			pll, pllcr);
		__raw_writel(pllcr, __io(IO_ADDRESS(addr)));
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
		default: {
			pr_err("%s()[%d]: PLL<%d> not supported, ret<%d>\n",
				__func__, __LINE__, pll, -EINVAL);
			return -EINVAL;
		}
	}

	pllcr = __raw_readl(__io(IO_ADDRESS(addr)));
	stc_val = ((pllcr & PLLCR_STC_MASK) >> PLLCR_BIT24_SHIFT) + 1;
	pr_log("%s()[%d]: get PLL<%d>, STC<0x%x>\n", __func__, __LINE__, pll,
		stc_val);
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
	struct clk_rate get_rate;
	unsigned int frqcra = __raw_readl(CPG_FRQCRA);
	unsigned int frqcrb = __raw_readl(CPG_FRQCRB);
	unsigned int frqcrd = __raw_readl(CPG_FRQCRD);
	unsigned int zbckcr = __raw_readl(CPG_ZBCKCR);

	if (!rates) {
		pr_err("%s()[%d]: error<%d>! no memory\n",
			__func__, __LINE__, -EINVAL);
		return -EINVAL;
	}
	/* get the clock setting
	 * must execute in spin lock context
	 */
	get_rate.i_clk = __match_div_rate(I_CLK,
		HW_TO_DIV(frqcra, I_CLK));
	get_rate.zg_clk = __match_div_rate(ZG_CLK,
		HW_TO_DIV(frqcra, ZG_CLK));
	get_rate.b_clk = __match_div_rate(B_CLK,
		HW_TO_DIV(frqcra, B_CLK));
	get_rate.m1_clk = __match_div_rate(M1_CLK,
		HW_TO_DIV(frqcra, M1_CLK));
	get_rate.m3_clk = __match_div_rate(M3_CLK,
		HW_TO_DIV(frqcra, M3_CLK));
	get_rate.z_clk = __match_div_rate(Z_CLK,
		HW_TO_DIV(frqcrb, Z_CLK));
	get_rate.ztr_clk = __match_div_rate(ZTR_CLK,
		HW_TO_DIV(frqcrb, ZTR_CLK));
	get_rate.zt_clk = __match_div_rate(ZT_CLK,
		HW_TO_DIV(frqcrb, ZT_CLK));
	get_rate.zx_clk = __match_div_rate(ZX_CLK,
		HW_TO_DIV(frqcrb, ZX_CLK));
	get_rate.hp_clk = __match_div_rate(HP_CLK,
		HW_TO_DIV(frqcrb, HP_CLK));
	get_rate.zs_clk = __match_div_rate(ZS_CLK,
		HW_TO_DIV(frqcrb, ZS_CLK));
	get_rate.zb_clk = __match_div_rate(ZB_CLK,
		HW_TO_DIV(zbckcr, ZB_CLK));

	if (shmobile_chip_rev() >= ES_REV_2_0)
		get_rate.m5_clk = __match_div_rate(M5_CLK,
			HW_TO_DIV(frqcra, M5_CLK));
	else
		get_rate.m5_clk = DIV1_1; /* dummy one */

	if ((frqcrd & FRQCRD_ZB30SEL) != 0)
		get_rate.zb3_clk = __match_div_rate(ZB3_CLK,
			HW_TO_DIV(frqcrd, ZB3_CLK));
	else
		get_rate.zb3_clk = DIV1_2;
	/* verify again */
	if ((get_rate.i_clk < 0) || (get_rate.zg_clk < 0)
		|| (get_rate.b_clk < 0) || (get_rate.m1_clk < 0)
		|| (get_rate.m3_clk < 0) || (get_rate.z_clk < 0)
		|| (get_rate.ztr_clk < 0) || (get_rate.zt_clk < 0)
		|| (get_rate.zx_clk < 0) || (get_rate.hp_clk < 0)
		|| (get_rate.zs_clk < 0) || (get_rate.zb_clk < 0)
		|| (get_rate.zb3_clk < 0)) {
		pr_err("%s()[%d]: error<%d>! invalid setting value\n",
			__func__, __LINE__, -EINVAL);
		return -EINVAL;
	}

	if (shmobile_chip_rev() >= ES_REV_2_0) {
		if (get_rate.m5_clk < 0) {
			pr_err("%s()[%d]: error<%d>! invalid setting value\n",
				__func__, __LINE__, -EINVAL);
			return -EINVAL;
		}

		get_rate.pll0 = cpg_get_pll(PLL0);
	} else {
		get_rate.pll0 = PLLx38;
	}

	(void)memcpy(rates, &get_rate, sizeof(struct clk_rate));

	return 0;
}

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
	unsigned int zb3rate = 0;
	unsigned int reg = 0;

	spin_lock_irqsave(&freq_change_lock, flags);

	if ((the_clock.zs_disabled_cnt && (clk == ZS_CLK)) ||
		(the_clock.hp_disabled_cnt && (clk == HP_CLK)))	{
		spin_unlock_irqrestore(&freq_change_lock, flags);
		return -EBUSY;
	}

	info = &__clk_hw_info[clk];
	if (info->div_val[div] < 0) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("%s()[%d]: error<%d>! invalid divrate\n",
			__func__, __LINE__, -EINVAL);
		return -EINVAL;
	}

	/* need KICK bit */
	if ((clk != ZB_CLK) && (clk != ZB3_CLK)) {
		if (cpg_wait_kick(KICK_WAIT_INTERVAL_US)) {
			spin_unlock_irqrestore(&freq_change_lock, flags);
				pr_err("%s()[%d]: error<%d>! h/w busy\n",
					__func__, __LINE__, -ETIMEDOUT);
			return -ETIMEDOUT;
		}
	}

	/* need to ensure ZS-phy is permissed to change */
	if (clk == ZS_CLK)
		spin_lock_irqsave(&zs_change_lock, zsflags);

	reg = __raw_readl(info->addr);
	reg &= ~(info->mask_bit << info->shift_bit);
	reg |= DIV_TO_HW(clk, div);

	/* ES2.x, the ZB3SL SEL is used */
	if ((shmobile_chip_rev() >= ES_REV_2_0) && (clk == ZB3_CLK)) {
		zb3rate = cpg_get_pll(PLL3);
		zb3rate *= 26/(__div(div));

		if (zb3rate > 266) {
			reg |= BIT(15);
			pr_log("%s()[%d]: FRQCRD[0x%8x] ZB3SL SEL is set\n",
				__func__, __LINE__, reg);
		} else {
			reg &= ~BIT(15);
		}
	}

	__raw_writel(reg, info->addr);
	/* ZS change */
	if (clk == ZS_CLK)
		spin_unlock_irqrestore(&zs_change_lock, zsflags);

	/* need KICK bit */
	if ((clk != ZB_CLK) && (clk != ZB3_CLK)) {
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			spin_unlock_irqrestore(&freq_change_lock, flags);
				pr_err("%s()[%d]: error<%d>! set kick bit\n",
					__func__, __LINE__, -ETIMEDOUT);
			return -ETIMEDOUT;
		}
	}
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
		pr_err("%s()[%d]: error<%d>! no memory\n",
			__func__, __LINE__, -EINVAL);
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
		pr_err("%s()[%d]: error<%d>! invalid clock setting\n", __func__,
			__LINE__, -EINVAL);
		return -EINVAL;
	}

	/* ZB3 == x1/2? */
	if ((clk == ZB3_CLK) && ((div_rate & 0x1f) == 0))
		div_rate = DIV1_2;
	else
	div_rate = (int)__match_div_rate((enum clk_type)clk, div_rate);
	if (div_rate < 0) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		pr_err("%s()[%d]: error<%d>! invalid clock divrate\n", __func__,
			__LINE__, -EINVAL);
		return -EINVAL;
	}
	*div = div_rate;
	spin_unlock_irqrestore(&freq_change_lock, flags);

	return 0;
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
	unsigned int reg = 0;
	unsigned int zb3rate = 0;

	reg = __raw_readl(CPG_FRQCRD);
	reg &= ~0x1f;
	reg |= DIV_TO_HW(ZB3_CLK, div);
	/* ES2.x, the ZB3SL SEL is used */
	if (shmobile_chip_rev() >= ES_REV_2_0) {
		zb3rate = cpg_get_pll(PLL3);
		zb3rate *= 26/(__div(div));

		if (zb3rate > 266) {
			reg |= BIT(15);
		} else {
			reg &= ~BIT(15);
		}
	}
	pr_log("%s()[%d]: FRQCRD[0x%8x]\n",
		__func__, __LINE__, reg);
	__raw_writel(reg, CPG_FRQCRD);

	if (shmobile_chip_rev() >= ES_REV_2_0) {
		/* set and wait for KICK bit changed */
		if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
			pr_err("%s()[%d]: error! set kick bit\n",
				__func__, __LINE__);
			return -ETIMEDOUT;
		}
	}

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
	unsigned long flags;
	unsigned long zsflags;
	struct clk_rate curr_rates;
	unsigned int reg = 0;
	int ret = 0;
	int frq_change = 0;
	int zb3_change = 0;
	int zb_change = 0;
	int zs_change = 0;

	spin_lock_irqsave(&freq_change_lock, flags);
	/* violate restriction? */
	if (__validate(rates)) {
		/* ret = -EINVAL; */
		pr_log("%s()[%d]: warning, violate restriction\n",
			__func__, __LINE__);
		/* goto done; */
	}

	ret = cpg_get_freq(&curr_rates);
	if (ret) {
		pr_err("%s()[%d]: error<%d>! get frequencies\n",
			__func__, __LINE__, ret);
		goto done;
	}

	if (shmobile_chip_rev() >= ES_REV_2_0) {
		if (curr_rates.pll0 != rates.pll0) {
			ret = cpg_set_pll(PLL0, rates.pll0);
			if (0 > ret) {
				pr_err("%s()[%d]: error<%d>! set pll<%d>, STC<0x%x>\n",
					__func__, __LINE__, ret, PLL0,
					rates.pll0);
				goto done;
			}
		}
	}

	/* change FRQCR(A/B) ? */
	if ((curr_rates.i_clk != rates.i_clk)
		|| (curr_rates.zg_clk != rates.zg_clk)
		|| (curr_rates.b_clk != rates.b_clk)
		|| (curr_rates.m1_clk != rates.m1_clk)
		|| (curr_rates.m3_clk != rates.m3_clk)
		/* || (curr_rates.z_clk != rates.z_clk) */
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
	}
#if 0
	/* change M5 (ES2 only) */
	if ((shmobile_chip_rev() >= ES_REV_2_0) &&
		(curr_rates.m5_clk != rates.m5_clk))
		frq_change = 1;
#endif
	/* change ZB clock ? */
	if (curr_rates.zb_clk != rates.zb_clk)
		zb_change = 1;

	/* change ZB3 clock ? */
	if (curr_rates.zb3_clk != rates.zb3_clk)
		zb3_change = 1;

	/* change FRQCR(A/B) ? */
	if (frq_change) {
		/* wait for KICK bit change (if any) */
		ret = cpg_wait_kick(KICK_WAIT_INTERVAL_US);
		if (0 > ret) {
			pr_err("%s()[%d]: error<%d>! busy\n",
				__func__, __LINE__, ret);
			goto done;
		}
		reg = DIV_TO_HW(I_CLK, rates.i_clk);
		reg |= DIV_TO_HW(ZG_CLK, rates.zg_clk);
		reg |= DIV_TO_HW(B_CLK, rates.b_clk);
		reg |= DIV_TO_HW(M1_CLK, rates.m1_clk);
		reg |= DIV_TO_HW(M3_CLK, rates.m3_clk);

		if (shmobile_chip_rev() >= ES_REV_2_0)
			reg |= DIV_TO_HW(M5_CLK, curr_rates.m5_clk);

		/* apply setting */
		__raw_writel(reg, CPG_FRQCRA);
		pr_log("%s()[%d]: Set FRQCRA[0x%x]\n", __func__, __LINE__, reg);
		/* not change Z-Phy, use the current one */
		reg = DIV_TO_HW(Z_CLK, curr_rates.z_clk);
		/* apply new setting */
		reg |= DIV_TO_HW(ZTR_CLK, rates.ztr_clk);
		reg |= DIV_TO_HW(ZT_CLK, rates.zt_clk);
		reg |= DIV_TO_HW(ZX_CLK, rates.zx_clk);

		/* change HP */
		if (!the_clock.hp_disabled_cnt) {
			reg |= DIV_TO_HW(HP_CLK, rates.hp_clk);
		} else {
			/* keep current HP-Phy */
			reg |= DIV_TO_HW(HP_CLK, curr_rates.hp_clk);
		}

		/* change ZS */
		if (zs_change) {
			spin_lock_irqsave(&zs_change_lock, zsflags);
			reg |= DIV_TO_HW(ZS_CLK, rates.zs_clk);
		} else {
			/* keep current ZS-Phy */
			reg |= DIV_TO_HW(ZS_CLK, curr_rates.zs_clk);
		}

		/* apply setting */
		__raw_writel(reg, CPG_FRQCRB);
		pr_log("%s()[%d]: Set KICK - FRQCRB[0x%x]\n",
			__func__, __LINE__, reg);
	/* set and wait for KICK bit changed */
	if (cpg_set_kick(KICK_WAIT_INTERVAL_US)) {
		pr_err("%s()[%d]: error<%d>! set kick bit\n",
			__func__, __LINE__, ret);
			if (zs_change)
				spin_unlock_irqrestore(&zs_change_lock,
					zsflags);

		goto done;
	}

		if (zs_change)
			spin_unlock_irqrestore(&zs_change_lock, zsflags);
	}

	/* change ZB clock ? */
	if (zb_change) {
		reg = DIV_TO_HW(ZB_CLK, rates.zb_clk);
		__raw_writel(reg, CPG_ZBCKCR);
	}
#if 0
	/* change ZB3 clock ? */
	if (zb3_change) {
		if (cpg_set_sbsc_freq((int)rates.zb3_clk)) {
			pr_err("%s()[%d]: error<%d>! change ZB3<%d>\n",
				__func__, __LINE__, ret, (int)rates.zb3_clk);
			goto done;
		}
	}
#endif
done:
	spin_unlock_irqrestore(&freq_change_lock, flags);
	pr_log("%s()[%d]: frequency changed, ret<%d>\n",
		__func__, __LINE__, ret);
	return ret;
}

/******************************************************************************/
/* PM APIs ********************************************************************/
/******************************************************************************/
int pm_setup_clock(void)
{
	unsigned long flags;

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
#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return 0;
	}
#endif /* CONFIG_PM_DEBUG */
	return cpg_set_freq(rates);
}
EXPORT_SYMBOL(pm_set_clocks);

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

#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	if (shmobile_chip_rev() >= ES_REV_2_0)
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es2_x);
	else
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es1_x);

	if ((mode >= size) || (mode < 0))
		return -EINVAL;

	(void)memcpy(&rates, &__shmobile_freq_modes[mode],
		sizeof(struct clk_rate));

	/* we not allow to change Z-Phy using this APIs */
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
#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return 0;
	}
#endif /* CONFIG_PM_DEBUG */
	if (shmobile_chip_rev() >= ES_REV_2_0)
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es2_x);
	else
		size = (int)ARRAY_SIZE(__shmobile_freq_modes_es1_x);

	/* invalid parameter? */
	if ((!rate) || (mode >= size) || (mode < 0))
		return -EINVAL;
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
#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return 0;
	}
#endif /* CONFIG_PM_DEBUG */
	return cpg_set_freqval(Z_CLK, div);
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

#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
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
	/* error case */
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

#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_lock_irqsave(&freq_change_lock, flags);
	/* support ZS/HP only */
	if (!(clk & (ZSCLK | HPCLK))) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		return -EINVAL;
	}

	if ((clk & ZSCLK))
		the_clock.zs_disabled_cnt++;

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

#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return ret;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_lock_irqsave(&freq_change_lock, flags);
	/* support ZS/HP only */
	if (!(clk & (ZSCLK | HPCLK))) {
		spin_unlock_irqrestore(&freq_change_lock, flags);
		return -EINVAL;
	}

	if ((clk & ZSCLK)) {
		if (the_clock.zs_disabled_cnt)
			the_clock.zs_disabled_cnt--;
	}

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
#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return 0;
	}
#endif /* CONFIG_PM_DEBUG */
	return cpg_set_pll(pll, val);
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
	unsigned long flag;
#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return 0;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_lock_irqsave(&zs_change_lock, flag);
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
#ifdef CONFIG_PM_DEBUG
	if (!is_cpufreq_enable()) {
		/* runtime disabled */
		return;
	}
#endif /* CONFIG_PM_DEBUG */
	spin_unlock_irqrestore(&zs_change_lock, flag);
}
EXPORT_SYMBOL(pm_release_spinlock);
