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

#include <linux/types.h>
#include <linux/init.h>
#include <mach/pm.h>
#include <mach/r8a7373.h>
#include "../../pmRegisterDef.h"

enum hw_register_width {
	HW_REG_8BIT	= 1,
	HW_REG_16BIT	= 2,
	HW_REG_32BIT	= 4,
};
struct hw_register_range {
	phys_addr_t start;
	/* End address is included in the reading range */
	phys_addr_t end;
	enum hw_register_width width;
	/* todo: at the moment addresses are incremented by 4 bytes
	 * but int he future there might be need to increment by byte. */
	unsigned int inc;
	/* Power area which needs to be on before reading registers
	 * This is PTSR register bit mask */
	unsigned int pa;
	/* This one of the module stop registers */
	unsigned int msr;
	/* This is module stop register bit mask */
	unsigned int msb;
};

/* can we find these from some header? */
#define MSTPST525 (1 << 25)
#define MSTO007 (1 << 7)

/* todo: consider using __initdata */
static struct hw_register_range ramdump_res[] = {
	{
		.start	= 0xFE400000,	/*SBSC_SETTING_00_S*/
		.end	= 0xFE40011C,	/*SBSC_SETTING_00_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xFE400200,	/*SBSC_SETTING_01_S*/
		.end	= 0xFE400240,	/*SBSC_SETTING_01_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xFE400358,	/*SBSC_MON_SETTING*/
		.end	= 0xFE400358,	/*SBSC_MON_SETTING*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xFE401000,	/*SBSC_PHY_SETTING_00_S*/
		.end	= 0xFE401004,	/*SBSC_PHY_SETTING_00_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xFE4011F4,	/*SBSC_PHY_SETTING_01*/
		.end	= 0xFE4011F4,	/*SBSC_PHY_SETTING_01*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xFE401050,	/*SBSC_PHY_SETTING_02_S*/
		.end	= 0xFE4010BC,	/*SBSC_PHY_SETTING_02_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6150000,	/*CPG_SETTING_00_S*/
		.end	= 0xE6150200,	/*CPG_SETTING_00_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6151000,	/*CPG_SETTING_01_S*/
		.end	= 0xE6151180,	/*CPG_SETTING_01_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6020000,	/*RWDT_CONDITION_00*/
		.end	= 0xE6020000,	/*RWDT_CONDITION_00*/
		.width	= HW_REG_16BIT,
	},
	{
		.start	= 0xE6020004,	/*RWDT_CONDITION_01*/
		.end	= 0xE6020004,	/*RWDT_CONDITION_01*/
		.width	= HW_REG_8BIT,
	},
	{
		.start	= 0xE6020008,	/*RWDT_CONDITION_02*/
		.end	= 0xE6020008,	/*RWDT_CONDITION_02*/
		.width	= HW_REG_8BIT,
	},
	{
		.start	= 0xE6030000,	/*SWDT_CONDITION_00*/
		.end	= 0xE6030000,	/*SWDT_CONDITION_00*/
		.width	= HW_REG_16BIT,
	},
	{
		.start	= 0xE6030004,	/*SWDT_CONDITION_01*/
		.end	= 0xE6030004,	/*SWDT_CONDITION_01*/
		.width	= HW_REG_8BIT,
	},
	{
		.start	= 0xE6030008,	/*SWDT_CONDITION_02*/
		.end	= 0xE6030008,	/*SWDT_CONDITION_02*/
		.width	= HW_REG_8BIT,
	},
	{
		.start	= 0xE61D0000,	/*SUTC_CONDITION_00*/
		.end	= 0xE61D0000,	/*SUTC_CONDITION_00*/
		.width	= HW_REG_16BIT,
	},
	{
		.start	= 0xE61D0040,	/*SUTC_CONDITION_01*/
		.end	= 0xE61D0040,	/*SUTC_CONDITION_01*/
		.width	= HW_REG_16BIT,
	},
	{
		.start	= 0xE61D0044,	/*SUTC_CONDITION_02*/
		.end	= 0xE61D0048,	/*SUTC_CONDITION_03*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6130500,	/*CMT15_CONDITION_00*/
		.end	= 0xE6130500,	/*CMT15_CONDITION_00*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6130510,	/*CMT15_CONDITION_01*/
		.end	= 0xE6130510,	/*CMT15_CONDITION_01*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6130514,	/*CMT15_CONDITION_02*/
		.end	= 0xE6130514,	/*CMT15_CONDITION_02*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6130518,	/*CMT15_CONDITION_03*/
		.end	= 0xE6130518,	/*CMT15_CONDITION_03*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6180000,	/*SYSC_SETTING_00_S*/
		.end	= 0xE61800FC,	/*SYSC_SETTING_00_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6180200,	/*SYSC_SETTING_01_S*/
		.end	= 0xE618027C,	/*SYSC_SETTING_01_E*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE618801C,	/*SYSC_RESCNT_00*/
		.end	= 0xE6188024,	/*SYSC_RESCNT_02*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6100020,	/*DBG_SETTING_00*/
		.end	= 0xE6100020,	/*DBG_SETTING_00*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE6100028,	/*DBG_SETTING_01*/
		.end	= 0xE610002c,	/*DBG_SETTING_02*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xF000010C,	/*GIC_SETTING_00*/
		.end	= 0xF0000110,	/*GIC_SETTING_01*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xF0100100,	/*PL310_SETTING_00*/
		.end	= 0xF0100104,	/*PL310_SETTING_01*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE61C0100,	/*INTC_SYS_INFO_00*/
		.end	= 0xE61C0104,	/*INTC_SYS_INFO_01*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE61C0300,	/*INTC_SYS_INFO_02*/
		.end	= 0xE61C0304,	/*INTC_SYS_INFO_03*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE623000C,	/*INTC_BB_INFO_00*/
		.end	= 0xE623000C,	/*INTC_BB_INFO_00*/
		.width	= HW_REG_32BIT,
	},
	{
		.start	= 0xE623200C,	/*INTC_BB_INFO_01*/
		.end	= 0xE623200C,	/*INTC_BB_INFO_01*/
		.width	= HW_REG_32BIT,
	},
	{
		/*NOTE: at the moment address increment is done by 4 byte steps
		 * so this will read one byte from 004 and one byte form 008 */
		.start	= 0xE6820004,	/*IIC0_SETTING_00*/
		.end	= 0xE6820008,	/*IIC0_SETTING_01*/
		.width	= HW_REG_8BIT,
		.pa		= POWER_A3SP,
		.msr	= MSTPSR1,
		.msb	= MSTPST116,
	},
	{
		.start	= 0xE682002C,	/*IIC0_SETTING_02*/
		.end	= 0xE682002C,	/*IIC0_SETTING_02*/
		.width	= HW_REG_8BIT,
		.pa	= POWER_A3SP,
		.msr	= MSTPSR1,
		.msb	= MSTPST116,
	},
	{
		.start	= 0xE62A0004,	/*IICB_SETTING_00*/
		.end	= 0xE62A0008,	/*IICB_SETTING_01*/
		.width	= HW_REG_8BIT,
		.msr	= MSTPSR5,
		.msb	= MSTPST525,
	},
	{
		.start	= 0xE62A002C,	/*IICB_SETTING_02*/
		.end	= 0xE62A002C,	/*IICB_SETTING_02*/
		.width	= HW_REG_8BIT,
		.msr	= MSTPSR5,
		.msb	= MSTPST525,
	},
	{
		.start	= 0xFE951000,	/*IPMMU_SETTING_S*/
		.end	= 0xFE9510FC,	/*IPMMU_SETTING_E*/
		.width	= HW_REG_32BIT,
		.pa	= POWER_A3R,
		.msr	= SMSTPCR0,
		.msb	= MSTO007,
	},
};

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
	ulong		SYSC_Setting_00_0[32];
	ulong		SYSCPSTR;
	ulong		SYSC_Setting_00_1[31];
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

struct ramdump_plat_data {
	unsigned long reg_dump_base;
	size_t reg_dump_size;
	/* this is the size for each core */
	size_t core_reg_dump_size;
	u32 num_resources;
	struct hw_register_range *hw_register_range;
	struct map_desc *io_desc;
	u32 io_desc_size;
};



#endif /*__RMC_RAMDUMP_H__*/
