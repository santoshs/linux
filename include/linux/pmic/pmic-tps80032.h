/*
 * include/linux/pmic/pmic-tps80032.h
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include <linux/rtc.h>
 
#ifndef _LINUX_PMIC_TPS80032_H
#define _LINUX_PMIC_TPS80032_H

/* #define PMIC_DEBUG_ENABLE */
/* Define for use Non_volatile value */
/* #define PMIC_NON_VOLATILE_ENABLE */

#define PINT_IRQ_BASE		512
#define pint2irq(bit)		(PINT_IRQ_BASE + (bit))

#define IRQPIN_IRQ_BASE		512
#define irqpin2irq(nr)		(IRQPIN_IRQ_BASE + (nr))

#define TPS80032_IRQ(_reg, _mask)	\
	{							\
		.mask_reg = (MSK_INT_LINE_##_reg) -	\
				MSK_INT_LINE_A,	\
		.mask_mask = (_mask),				\
	}

#define TPS80032_IRQ_SEC(_reg, _mask, _pint, _sint_mask_bit, _sint_sts_bit) \
	{								\
		.mask_reg = (MSK_INT_LINE_##_reg) -		\
				MSK_INT_LINE_A,		\
		.mask_mask = (_mask),					\
		.is_sec_int = true,					\
		.parent_int = TPS80032_INT_##_pint,			\
		.mask_sec_int_reg = HW_REG_CONTROLLER_INT_MASK,	\
		.int_mask_bit = MSK_CONTROLLER_STAT_##_sint_mask_bit,	\
		.int_sec_sts_reg = HW_REG_CONTROLLER_STAT1,		\
		.int_sts_bit = BIT_CONTROLLER_STAT1_##_sint_sts_bit		\
	}

struct tps80032_subdev_info {
	int		id;
	const char	*name;
	void		*platform_data;
};

struct tps80032_platform_data {
	int num_subdevs;
	struct tps80032_subdev_info *subdevs;
	int irq_base;
};

struct tps80032_rtc_platform_data {
	int irq;
	struct rtc_time time;
};

enum {
	SLAVE_ID0 = 0,
	SLAVE_ID1 = 1,
	SLAVE_ID2 = 2,
	SLAVE_ID3 = 3,
};

enum {
	I2C_ID0_ADDR = 0x12,
	I2C_ID1_ADDR = 0x48,
	I2C_ID2_ADDR = 0x49,
	I2C_ID3_ADDR = 0x4A,
};

enum {
	TPS80032_INT_PWRON,
	TPS80032_INT_RPWRON,
	TPS80032_INT_SYS_VLOW,
	TPS80032_INT_RTC_ALARM,
	TPS80032_INT_RTC_PERIOD,
	TPS80032_INT_HOT_DIE,
	TPS80032_INT_VXX_SHORT,
	TPS80032_INT_SPDURATION,
	TPS80032_INT_WATCHDOG,
	TPS80032_INT_BAT,
	TPS80032_INT_SIM,
	TPS80032_INT_MMC,
	TPS80032_INT_RES,
	TPS80032_INT_GPADC_RT,
	TPS80032_INT_GPADC_SW2_EOC,
	TPS80032_INT_CC_AUTOCAL,
	TPS80032_INT_ID_WKUP,
	TPS80032_INT_VBUSS_WKUP,
	TPS80032_INT_ID,
	TPS80032_INT_VBUS,
	TPS80032_INT_CHRG_CTRL,
	TPS80032_INT_EXT_CHRG,
	TPS80032_INT_INT_CHRG,
	TPS80032_INT_RES2,
	TPS80032_INT_BAT_TEMP_OVRANGE,
	TPS80032_INT_BAT_REMOVED,
	TPS80032_INT_VBUS_DET,
	TPS80032_INT_VAC_DET,
	TPS80032_INT_FAULT_WDG,
	TPS80032_INT_LINCH_GATED,

	/* Last interrupt id to get the end number */
	TPS80032_INT_NR,
};

enum {
	USB_SDP_FULL_SPEED = 0,
    USB_SDP_HIGH_SPEED,
    USB_CDP_FULL_SPEED,
    USB_CDP_HIGH_SPEED,
    USB_DCP,
};

struct tps80032_irq_data {
	u8	mask_reg;
	u8	mask_mask;
	u8	is_sec_int;
	u8	parent_int;
	u8	mask_sec_int_reg;
	u8	int_mask_bit;
	u8	int_sec_sts_reg;
	u8	int_sts_bit;
};


/*
 * Define the address of the bin file which contain non-volatile value
 */

#define MAP_BASE_NV 					0x5FC00000
#define MAP_BASE_PMIC_NV 				0x0010800
#define MAP_SIZE_PMIC_NV 				1280
#define get_map_data(base,offset) 		__raw_readb(base + offset)

/*
 * Define the contanst value of HW register address
 */

#define HW_REG_PREQ1_RES_ASS_A			0xD7
#define HW_REG_PREQ1_RES_ASS_B			0xD8
#define HW_REG_PREQ1_RES_ASS_C			0xD9

#define HW_REG_PHOENIX_MSK_TRANSITION	0x20

#define HW_REG_CHARGEUSB_CTRL1			0xE8
#define HW_REG_CHARGEUSB_VSYSREG		0xDC
#define HW_REG_INT_STS_C				0xD2
#define HW_REG_INT_STS_B				0xD1
#define HW_REG_INT_STS_A				0xD0
#define HW_REG_INT_MSK_LINE_STS_A		0xD3
#define HW_REG_INT_MSK_LINE_STS_B		0xD4
#define HW_REG_INT_MSK_LINE_STS_C		0xD5
#define HW_REG_INT_MSK_STS_A			0xD6
#define HW_REG_INT_MSK_STS_B			0xD7
#define HW_REG_INT_MSK_STS_C			0xD8
#define HW_REG_CONTROLLER_STAT1			0xE3
#define HW_REG_CONTROLLER_CTRL1			0xE1
#define HW_REG_CONTROLLER_INT_MASK		0xE0
#define HW_REG_CHARGERUSB_STATUS_INT1	0xE6
#define HW_REG_CHARGERUSB_STATUS_INT2	0xE7
#define HW_REG_CHARGERUSB_CTRL1			0xE8
#define HW_REG_CHARGERUSB_CTRL3			0xEA
#define HW_REG_CHARGERUSB_CINLIMIT		0xEE
#define HW_REG_CHARGERUSB_VOREG     	0xEC
#define HW_REG_CHARGERUSB_VICHRG     	0xED
#define HW_REG_CHARGERUSB_VSYSREG		0xDC
#define HW_REG_USB_ID_INT_SRC			0x0F
#define HW_REG_USB_ID_CTRL_SET			0x06
#define HW_REG_USB_ID_INT_LATCH_CLR		0x11
#define HW_REG_USB_ID_EN_LO_SET			0x12
#define HW_REG_USB_ID_EN_HI_SET			0x14
#define HW_REG_MISC1					0xE4
#define HW_REG_MISC2					0xE5
#define HW_REG_TOGGLE1					0x90
#define HW_REG_LINEAR_CHRG_STS			0xDE

#define HW_REG_GPSELECT_ISB				0x35
#define HW_REG_CTRL_P1					0x36
#define HW_REG_GPADC_CTRL				0x2E
#define HW_REG_GPADC_CTRL2				0x2F
#define HW_REG_GPCH0_MSB				0x3C
#define HW_REG_GPCH0_LSB				0x3B

#define HW_REG_PHOENIX_DEV_ON			0x25
#define HW_REG_VSYSMIN_HI_THRESHOLD 	0x24

#define HW_REG_SMPS4_CFG_STATE			0x42
#define HW_REG_LDO1_CFG_STATE 			0x9E
#define HW_REG_LDO5_CFG_STATE			0x9A
#define HW_REG_LDO7_CFG_STATE			0xA6
#define HW_REG_LDOUSB_CFG_STATE			0xA2

#define HW_REG_CLK32KAO_CFG_TRANS		0xBA
#define HW_REG_CLK32KAO_CFG_STATE		0xBB
#define HW_REG_CLK32KG_CFG_TRANS		0xBD
#define HW_REG_CLK32KG_CFG_STATE		0xBE
#define HW_REG_CLK32KAUDIO_CFG_TRANS	0xC0
#define HW_REG_CLK32KAUDIO_CFG_STATE	0xC1


#define HW_REG_GPADC_TRIM1				0xCD
#define HW_REG_GPADC_TRIM2				0xCE
#define HW_REG_GPADC_TRIM3				0xCF
#define HW_REG_GPADC_TRIM4				0xD0
#define HW_REG_GPADC_TRIM5				0xD1
#define HW_REG_GPADC_TRIM6				0xD2

#define HW_REG_LDO1_CFG_VOLTAGE			0x9F
#define HW_REG_LDO2_CFG_VOLTAGE			0x87
#define HW_REG_LDO4_CFG_VOLTAGE			0x8B
#define HW_REG_LDO5_CFG_VOLTAGE			0x9B
#define HW_REG_LDO6_CFG_VOLTAGE			0x93
#define HW_REG_LDO7_CFG_VOLTAGE			0xA7

#define HW_REG_SMPS1_CFG_TRANS			0x53
#define HW_REG_SMPS2_CFG_TRANS			0x59
#define HW_REG_SMPS3_CFG_TRANS			0x65
#define HW_REG_SMPS4_CFG_TRANS			0x41
#define HW_REG_SMPS5_CFG_TRANS			0x47
#define HW_REG_LDO1_CFG_TRANS			0x9D
#define HW_REG_LDO2_CFG_TRANS			0x85
#define HW_REG_LDO3_CFG_TRANS			0x8D
#define HW_REG_LDO4_CFG_TRANS			0x89
#define HW_REG_LDO5_CFG_TRANS			0x99
#define HW_REG_LDO6_CFG_TRANS			0x91
#define HW_REG_LDO7_CFG_TRANS			0xA5

#define HW_REG_FG_REG_00				0xC0
#define HW_REG_FG_REG_01				0xC1
#define HW_REG_FG_REG_02				0xC2
#define HW_REG_FG_REG_03				0xC3
#define HW_REG_FG_REG_04				0xC4
#define HW_REG_FG_REG_05				0xC5
#define HW_REG_FG_REG_06				0xC6
#define HW_REG_FG_REG_07				0xC7
#define HW_REG_FG_REG_08				0xC8
#define HW_REG_FG_REG_09				0xC9

/*
 * Define the mask value from bit 0 to bit 7
 */

#define MSK_BIT_0						0x01
#define MSK_BIT_1						0x02
#define MSK_BIT_2						0x04
#define MSK_BIT_3						0x08
#define MSK_BIT_4						0x10
#define MSK_BIT_5						0x20
#define MSK_BIT_6						0x40
#define MSK_BIT_7						0x80

#define MSK_POWER_STATE					0x03
#define MSK_POWER_VOLTAGE				0x1F

#define TPS80032_STATE_ON				(1)
#define TPS80032_STATE_OFF				(0)

#define	CLK32KAO 						(0)
#define	CLK32KG  						(1)
#define	CLK32KAUDIO 					(2)

/*
 * Define the contain non-volatile value
 */

#ifdef PMIC_NON_VOLATILE_ENABLE
#define MSK_GET_INT_SRC					get_map_data(virt_addr, 0x0F)
#define MSK_GET_EXT_DEVICE				get_map_data(virt_addr, 0x0E)
#define MSK_INT_SRC_A					get_map_data(virt_addr, 0x10)
#define MSK_INT_SRC_B					get_map_data(virt_addr, 0x11)
#define MSK_INT_SRC_C					get_map_data(virt_addr, 0x12)

#define CONST_TIMER_BATTERY_UPDATE		(get_map_data(virt_addr, 0x00) | (get_map_data(virt_addr, 0x01) << 8))
#define CONST_0C_DEGREE					(get_map_data(virt_addr, 0x03) | (get_map_data(virt_addr, 0x04) << 8))
#define CONST_WAIT_TIME					get_map_data(virt_addr, 0x02)

#define THR_BAT_FULL					get_map_data(virt_addr, 0x0A)
#define THR_BAT_HIGH					get_map_data(virt_addr, 0x0B)
#define THR_BAT_NORMAL					get_map_data(virt_addr, 0x0C)
#define THR_BAT_LOW						get_map_data(virt_addr, 0x0D)

#define CONST_CONVERT_VOLT				(get_map_data(virt_addr, 0x05) | (get_map_data(virt_addr, 0x06) << 8))
#define CONST_CONVERT_TEMP				(get_map_data(virt_addr, 0x07) | (get_map_data(virt_addr, 0x08) << 8))
#define CONST_BAT_MIN					get_map_data(virt_addr, 0x09)
#define CONST_INT_ID					get_map_data(virt_addr, 0x13)

#define CONST_X1						(get_map_data(virt_addr, 0x14) | (get_map_data(virt_addr, 0x15) << 8))
#define CONST_X2						(get_map_data(virt_addr, 0x16) | (get_map_data(virt_addr, 0x17) << 8))

#else
/* If no use BIN */
#define CONST_TIMER_BATTERY_UPDATE		0x2710
#define CONST_WAIT_TIME					0x05
#define CONST_WAIT_TIME_CURRENT			0xFF
#define CONST_0C_DEGREE					0x0000
#define CONST_CONVERT_VOLT				0x03E8
#define CONST_CONVERT_TEMP				0x0AAA
#define CONST_BAT_MIN					0x01
#define THR_BAT_FULL					0x64
#define THR_BAT_HIGH					0x46
#define THR_BAT_NORMAL					0x0F
#define THR_BAT_LOW						0x05
#define MSK_GET_EXT_DEVICE				0x0E
#define MSK_GET_INT_SRC_A				0xFF
#define MSK_GET_INT_SRC_C				0x7F
#define MSK_INT_LINE_A					0x44
#define MSK_INT_LINE_B					0xFF
#define MSK_INT_LINE_C					0x8A
#define MSK_INT_SRC_A					0x44
#define MSK_INT_SRC_B					0x8F
#define MSK_INT_SRC_C					0x8A
#define MSK_DISABLE 					0x00
#define MSK_CONTROLLER_INT				0x00
#define MSK_PREQ1_ASS_A					0x11
#define MSK_PREQ1_ASS_B					0xA0
#define MSK_TRANSITION					0x00
#define CONST_INT_ID					0x1C
#define CONST_X1						0x05A1
#define CONST_X2						0x0CCC
#define CONST_VAC_CURRENT_LIMIT			0x01	/* 100mA */
#define CONST_DEF_CURRENT_LIMIT			0x01	/* 100mA */
#define CONST_VOREG						0x05	/* 3.6V */
#define CONST_VICHRG					0x02	/* 300mA with POP = 1*/

/* Define interrupt bit for interrupt register */
#define MSK_CONTROLLER_STAT_MVAC_DET		0
#define MSK_CONTROLLER_STAT_MVBUS_DET		1
#define MSK_CONTROLLER_STAT_MBAT_TEMP		2
#define MSK_CONTROLLER_STAT_MFAULT_WDG		3
#define MSK_CONTROLLER_STAT_MBAT_REMOVED	4
#define MSK_CONTROLLER_STAT_MLINCH_GATED	5

#define BIT_CONTROLLER_STAT1_BAT_TEMP		0
#define BIT_CONTROLLER_STAT1_BAT_REMOVED	1
#define BIT_CONTROLLER_STAT1_VBUS_DET		2
#define BIT_CONTROLLER_STAT1_VAC_DET		3
#define BIT_CONTROLLER_STAT1_FAULT_WDG		4
#define BIT_CONTROLLER_STAT1_LINCH_GATED	6
#endif

#define RESOURCE_COUNTER_MAX			13

static const struct tps80032_irq_data tps80032_irqs[] = {

	[TPS80032_INT_PWRON]			= TPS80032_IRQ(A, 0),
	[TPS80032_INT_RPWRON]			= TPS80032_IRQ(A, 1),
	[TPS80032_INT_SYS_VLOW]			= TPS80032_IRQ(A, 2),
	[TPS80032_INT_RTC_ALARM]		= TPS80032_IRQ(A, 3),
	[TPS80032_INT_RTC_PERIOD]		= TPS80032_IRQ(A, 4),
	[TPS80032_INT_HOT_DIE]			= TPS80032_IRQ(A, 5),
	[TPS80032_INT_VXX_SHORT]		= TPS80032_IRQ(A, 6),
	[TPS80032_INT_SPDURATION]		= TPS80032_IRQ(A, 7),
	[TPS80032_INT_WATCHDOG]			= TPS80032_IRQ(B, 0),
	[TPS80032_INT_BAT]				= TPS80032_IRQ(B, 1),
	[TPS80032_INT_SIM]				= TPS80032_IRQ(B, 2),
	[TPS80032_INT_MMC]				= TPS80032_IRQ(B, 3),
	[TPS80032_INT_RES]				= TPS80032_IRQ(B, 4),
	[TPS80032_INT_GPADC_RT]			= TPS80032_IRQ(B, 5),
	[TPS80032_INT_GPADC_SW2_EOC]	= TPS80032_IRQ(B, 6),
	[TPS80032_INT_CC_AUTOCAL]		= TPS80032_IRQ(B, 7),
	[TPS80032_INT_ID_WKUP]			= TPS80032_IRQ(C, 0),
	[TPS80032_INT_VBUSS_WKUP]		= TPS80032_IRQ(C, 1),
	[TPS80032_INT_ID]				= TPS80032_IRQ(C, 2),
	[TPS80032_INT_VBUS]				= TPS80032_IRQ(C, 3),
	[TPS80032_INT_CHRG_CTRL]		= TPS80032_IRQ(C, 4),
	[TPS80032_INT_EXT_CHRG]			= TPS80032_IRQ(C, 5),
	[TPS80032_INT_INT_CHRG]			= TPS80032_IRQ(C, 6),
	[TPS80032_INT_RES2]				= TPS80032_IRQ(C, 7),
	[TPS80032_INT_BAT_TEMP_OVRANGE]	= TPS80032_IRQ_SEC(C, 4, CHRG_CTRL, MBAT_TEMP,	BAT_TEMP),
	[TPS80032_INT_BAT_REMOVED]		= TPS80032_IRQ_SEC(C, 4, CHRG_CTRL, MBAT_REMOVED,	BAT_REMOVED),
	[TPS80032_INT_VBUS_DET]			= TPS80032_IRQ_SEC(C, 4, CHRG_CTRL, MVBUS_DET,	VBUS_DET),
	[TPS80032_INT_VAC_DET]			= TPS80032_IRQ_SEC(C, 4, CHRG_CTRL, MVAC_DET,	VAC_DET),
	[TPS80032_INT_FAULT_WDG]		= TPS80032_IRQ_SEC(C, 4, CHRG_CTRL,	MFAULT_WDG,	FAULT_WDG),
	[TPS80032_INT_LINCH_GATED]		= TPS80032_IRQ_SEC(C, 4, CHRG_CTRL, MLINCH_GATED,	LINCH_GATED),
};

#endif
