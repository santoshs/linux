/* common_extern.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#ifndef __COMMON_EXTERN_H__
#define __COMMON_EXTERN_H__

#include <linux/kernel.h>

#ifdef __COMMON_CTRL_NO_EXTERN__
#define COMMON_CTRL_NO_EXTERN
#else
#define COMMON_CTRL_NO_EXTERN	extern
#endif

/* Struct declaration */
/* Register setting table */
typedef struct {
	u_int	uiReg;
	u_int	uiValue;
	u_int	uiDelay;
} common_reg_table;

/* FSI base address (PortA) */
COMMON_CTRL_NO_EXTERN u_long g_fsi_Base;
/* SCUW base address */
COMMON_CTRL_NO_EXTERN u_long g_scuw_Base;
/* SCUW base address to FFD */
COMMON_CTRL_NO_EXTERN u_long g_scuw_Base_FFD;
/* CLKGEN base address */
COMMON_CTRL_NO_EXTERN u_long g_clkgen_Base;

COMMON_CTRL_NO_EXTERN void iomodify32(u_int uiClr, u_int uiSet, u_int uiReg);
COMMON_CTRL_NO_EXTERN int common_ioremap(void);
COMMON_CTRL_NO_EXTERN void common_iounmap(void);
COMMON_CTRL_NO_EXTERN void audio_ctrl_func(sndp_hw_audio drv, int stat);
COMMON_CTRL_NO_EXTERN void common_set_register(sndp_hw_audio drv, common_reg_table *reg_tbl, u_int size);

#endif /* __COMMON_EXTERN_H__ */



