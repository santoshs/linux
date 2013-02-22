/*
 * arch/arm/mach-shmobile/include/mach/rmc_ramdump.h
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
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

#ifndef __RMC_RAMDUMP_H__
#define __RMC_RAMDUMP_H__

#include <linux/types.h> /* ssize_t */

/* hw register address */
/* -----	SBSC1 ----- */
#define	SBSC_SETTING_00_S			IO_ADDRESS(0xFE400000)
#define	SBSC_SETTING_00_E			IO_ADDRESS(0xFE40011C)
#define	SBSC_SETTING_01_S			IO_ADDRESS(0xFE400200)
#define	SBSC_SETTING_01_E			IO_ADDRESS(0xFE400240)
#define	SBSC_MON_SETTING			IO_ADDRESS(0xFE400358)
#define	SBSC_PHY_SETTING_00_S			IO_ADDRESS(0xFE401000)
#define	SBSC_PHY_SETTING_00_E			IO_ADDRESS(0xFE401004)
#define	SBSC_PHY_SETTING_01			IO_ADDRESS(0xFE4011F4)
#define	SBSC_PHY_SETTING_02_S			IO_ADDRESS(0xFE401050)
#define	SBSC_PHY_SETTING_02_E			IO_ADDRESS(0xFE4010BC)
/* -----	CPG ----- */
#define	CPG_SETTING_00_S			IO_ADDRESS(0xE6150000)
#define	CPG_SETTING_00_E			IO_ADDRESS(0xE6150200)
#define	CPG_SETTING_01_S			IO_ADDRESS(0xE6151000)
#define	CPG_SETTING_01_E			IO_ADDRESS(0xE6151180)
/* -----	RWDT ----- */
#define	RWDT_CONDITION_00			IO_ADDRESS(0xE6020000)
#define	RWDT_CONDITION_01			IO_ADDRESS(0xE6020004)
#define	RWDT_CONDITION_02			IO_ADDRESS(0xE6020008)
/* -----	SWDT ----- */
#define	SWDT_CONDITION_00			IO_ADDRESS(0xE6030000)
#define	SWDT_CONDITION_01			IO_ADDRESS(0xE6030004)
#define	SWDT_CONDITION_02			IO_ADDRESS(0xE6030008)
/* -----	Secure Up-time Clock ----- */
#define	SUTC_CONDITION_00			IO_ADDRESS(0xE61D0000)
#define	SUTC_CONDITION_01			IO_ADDRESS(0xE61D0040)
#define	SUTC_CONDITION_02			IO_ADDRESS(0xE61D0044)
#define	SUTC_CONDITION_03			IO_ADDRESS(0xE61D0048)
/* -----	CMT15 ----- */
#define	CMT15_CONDITION_00			IO_ADDRESS(0xE6130500)
#define	CMT15_CONDITION_01			IO_ADDRESS(0xE6130510)
#define	CMT15_CONDITION_02			IO_ADDRESS(0xE6130514)
#define	CMT15_CONDITION_03			IO_ADDRESS(0xE6130518)
/* -----	SYSC ----- */
#define	SYSC_SETTING_00_S			IO_ADDRESS(0xE6180000)
#define	SYSC_SETTING_00_E			IO_ADDRESS(0xE61800FC)
#define	SYSC_SETTING_01_S			IO_ADDRESS(0xE6180200)
#define	SYSC_SETTING_01_E			IO_ADDRESS(0xE618027C)
#define	SYSC_RESCNT_00				IO_ADDRESS(0xE618801C)
#define	SYSC_RESCNT_01				IO_ADDRESS(0xE6188020)
#define	SYSC_RESCNT_02				IO_ADDRESS(0xE6188024)
/* -----	DBG ----- */
#define	DBG_SETTING_00				IO_ADDRESS(0xE6100020)
#define	DBG_SETTING_01				IO_ADDRESS(0xE6100028)
#define	DBG_SETTING_02				IO_ADDRESS(0xE610002c)
/* -----	GIC ----- */
#define	GIC_SETTING_00				IO_ADDRESS(0xF000010C)
#define	GIC_SETTING_01				IO_ADDRESS(0xF0000110)
/* -----	L2C-310 ----- */
#define	PL310_SETTING_00			IO_ADDRESS(0xF0100100)
#define	PL310_SETTING_01			IO_ADDRESS(0xF0100104)
/* -----	INTC-SYS ----- */
#define	INTC_SYS_INFO_00			IO_ADDRESS(0xE61C0100)
#define	INTC_SYS_INFO_01			IO_ADDRESS(0xE61C0104)
#define	INTC_SYS_INFO_02			IO_ADDRESS(0xE61C0300)
#define	INTC_SYS_INFO_03			IO_ADDRESS(0xE61C0304)
/* -----	INTC-BB ----- */
#define	INTC_BB_INFO_00				IO_ADDRESS(0xE623000C)
#define	INTC_BB_INFO_01				IO_ADDRESS(0xE623200C)
/* -----	ICC0 ----- */
#define	IIC0_SETTING_00				IO_ADDRESS(0xE6820004)
#define	IIC0_SETTING_01				IO_ADDRESS(0xE6820008)
#define	IIC0_SETTING_02				IO_ADDRESS(0xE682002C)
/* -----	ICCB ----- */
#define	IICB_SETTING_00				IO_ADDRESS(0xE62A0004)
#define	IICB_SETTING_01				IO_ADDRESS(0xE62A0008)
#define	IICB_SETTING_02				IO_ADDRESS(0xE62A002C)
/* -----	IPMMU ----- */
#define	IPMMU_SETTING_S				IO_ADDRESS(0xFE951000)
#define	IPMMU_SETTING_E				IO_ADDRESS(0xFE9510FC)

/* Hardware-Register info */
struct	hw_register_dump {
	/* -----	SBSC1 ----- */
	ulong		SBSC_Setting_00[72];
	ulong		SBSC_Setting_01[17];
	ulong		SBSC_Mon_Setting;
	ulong		SBSC_PHY_Setting_00[2];
	ulong		SBSC_PHY_Setting_01;
	ulong		SBSC_PHY_Setting_02[28];
	/* -----	CPG ----- */
	ulong		CPG_Setting_00[129];
	ulong		CPG_Setting_01[97];
	/* -----	RWDT ----- */
	ulong		RWDT_Condition[3];
	/* -----	SWDT ----- */
	ulong		SWDT_Condition[3];
	/* -----	Secure Up-time Clock ----- */
	ulong		SUTC_Condition[4];
	/* -----	CMT15 ----- */
	ulong		CMT15_Condition[4];
	/* -----	SYSC ----- */
	ulong		SYSC_Setting_00[64];
	ulong		SYSC_Setting_01[32];
	ulong		SYSC_Rescnt[3];
	/* -----	DBG ----- */
	ulong		DBG_Setting[3];
	/* -----	GIC ----- */
	ulong		GIC_Setting[2];
	/* -----	L2C-310 ----- */
	ulong		PL310_Setting[2];
	/* -----	INTC-SYS ----- */
	ulong		INTC_SYS_Info[4];
	/* -----	INTC-BB ----- */
	ulong		INTC_BB_Info[2];
	/* -----	ICC0 ----- */
	ulong		IIC0_Setting[3];
	/* -----	ICCB ----- */
	ulong		IICB_Setting[3];
	/* -----	IPMMU ----- */
	ulong		IPMMU_Setting[64];
};

/* ioremap variables for captureing Hardware-Register value */

struct ramdump_plat_data {
	unsigned long reg_dump_base;
	size_t reg_dump_size;
	/* this is the size for each core */
	size_t core_reg_dump_size;
};



#endif /*__RMC_RAMDUMP_H__*/
