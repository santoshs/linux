/*
 * pmic_battery.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __PMIC_BATTERY_H__
#define __PMIC_BATTERY_H__

/*
 * Define the contanst value of HW register address
 */

/* PMIC error code */
#define		PMIC_ERR_I2C				-1		/* Error relating to I2C */
#define		PMIC_ERR_TIMEOUT			-2		/* Time out in reading interrupt */
#define		PMIC_BAT_NOT_DETECT			-3		/* Battery is not detect */
#define		PMIC_BAT_OVER_VOLT			-4		/* Battery is over voltage */
#define		PMIC_ERR_ARGUMENT			-5		/* Argument is not correct */

/* PMIC return value */
#define		PMIC_OK						0
#define		PMIC_BAT_DETECT				1
 
/* PMIC Slave register */
#define PMIC_ID1_REG_ADD				0x48 /* CTL-I2C for RTC and power */
#define PMIC_ID2_REG_ADD				0x49 /* CTL-I2C for battery */
#define PMIC_ID3_REG_ADD				0x4A /* CTL-I2C for JTAG */

/* BAT LEVEL */
#define PMIC_BAT_FULL					0x64 /* 100% */
#define PMIC_BAT_HIGH					0x46 /*  70% */
#define PMIC_BAT_NORMAL					0x0F /*  15% */
#define PMIC_BAT_LOW					0x05 /*   5% */

/* Current limit value */
#define PMIC_CUR_LIMIT_100mA			(uchar)0x01
#define PMIC_CUR_LIMIT_300mA			(uchar)0x05
#define PMIC_CUR_LIMIT_500mA			(uchar)0x09
#define PMIC_CUR_LIMIT_1500mA			(uchar)0x2E
/* */
#define PMIC_END_OF_CHARGE				0x20

#define PMIC_PREQ1_RES_ASS_A			0xD7
#define PMIC_PREQ1_RES_ASS_B			0xD8
#define PMIC_PREQ1_RES_ASS_C			0xD9

#define PMIC_CHARGEUSB_CTRL1			0xE8
#define PMIC_CHARGEUSB_VSYSREG			0xDC
#define PMIC_INT_STS_C					0xD2
#define PMIC_INT_STS_B					0xD1
#define PMIC_INT_STS_A					0xD0
#define PMIC_INT_MSK_LINE_STS_A			0xD3
#define PMIC_INT_MSK_LINE_STS_B			0xD4
#define PMIC_INT_MSK_LINE_STS_C			0xD5
#define PMIC_INT_MSK_STS_A				0xD6
#define PMIC_INT_MSK_STS_B				0xD7
#define PMIC_INT_MSK_STS_C				0xD8
#define PMIC_CONTROLLER_STAT1			0xE3
#define PMIC_CONTROLLER_CTRL1			0xE1
#define PMIC_CONTROLLER_CTRL2			0xDA
#define PMIC_CONTROLLER_INT_MASK		0xE0
#define PMIC_CHARGERUSB_STATUS_INT1		0xE6
#define PMIC_CHARGERUSB_STATUS_INT2		0xE7
#define PMIC_CHARGERUSB_CTRL1			0xE8
#define PMIC_CHARGERUSB_CTRL3			0xEA
#define PMIC_CHARGERUSB_CINLIMIT		0xEE
#define PMIC_USB_ID_INT_SRC				0x0F
#define PMIC_MISC1						0xE4
#define PMIC_TOGGLE1					0x90
#define PMIC_GPADC_CTRL					0x2E
#define PMIC_GPADC_CTRL2				0x2F
#define PMIC_GPSELECT_ISB				0x35
#define PMIC_CTRL_P1					0x36
#define PMIC_GPCH0_MSB					0x3C
#define PMIC_GPCH0_LSB					0x3B
#define PMIC_PHOENIX_DEV_ON				0x25
#define PMIC_CHARGERUSB_VSYSREG			0xDC
#define PMIC_VSYSMIN_HI_THRESHOLD 		0x24
#define PMIC_CHARGERUSB_VOREG     		0xEC
#define PMIC_CHARGERUSB_VICHRG			0xED
#define PMIC_CHARGERUSB_VICHRG_PC		0xDE
#define PMIC_CONTROLLER_VSEL_COMP		0xDB
#define PMIC_VBATMIN_HI_THRESHOLD		0x26
#define PMIC_STS_HW_CONDITIONS			0x21
#define PMIC_PHOENIX_START_CONDITION	0x1F
#define PMIC_LINEAR_CHRG_STS			0xDE
#define PMIC_SMPS4_CFG_STATE			0x42
#define PMIC_LDO1_CFG_STATE 			0x9E
#define PMIC_LDO5_CFG_STATE				0x9A
#define PMIC_LDO6_CFG_STATE				0x92
#define PMIC_LDO7_CFG_STATE				0xA6

#define PMIC_GPADC_TRIM1				0xCD
#define PMIC_GPADC_TRIM2				0xCE
#define PMIC_GPADC_TRIM3				0xCF
#define PMIC_GPADC_TRIM4				0xD0
#define PMIC_GPADC_TRIM5				0xD1
#define PMIC_GPADC_TRIM6				0xD2

#define PMIC_LDO1_CFG_VOLTAGE			0x9F
#define PMIC_LDO2_CFG_VOLTAGE			0x87
#define PMIC_LDO4_CFG_VOLTAGE			0x8B
#define PMIC_LDO5_CFG_VOLTAGE			0x9B
#define PMIC_LDO6_CFG_VOLTAGE			0x93
#define PMIC_LDO7_CFG_VOLTAGE			0xA7

#define PMIC_SMPS1_CFG_TRANS			0x53
#define PMIC_SMPS2_CFG_TRANS			0x59
#define PMIC_SMPS3_CFG_TRANS			0x65
#define PMIC_SMPS4_CFG_TRANS			0x41
#define PMIC_SMPS5_CFG_TRANS			0x47
#define PMIC_LDO1_CFG_TRANS				0x9D
#define PMIC_LDO2_CFG_TRANS				0x85
#define PMIC_LDO3_CFG_TRANS				0x8D
#define PMIC_LDO4_CFG_TRANS				0x89
#define PMIC_LDO5_CFG_TRANS				0x99
#define PMIC_LDO6_CFG_TRANS				0x91
#define PMIC_LDO7_CFG_TRANS				0xA5

#define RTC_SECONDS_REG					0x00
#define RTC_ALARM_SECONDS_REG			0x08
#define RTC_CTRL_REG					0x10
#define RTC_STATUS_REG					0x11
#define RTC_INTERRUPTS_REG				0x12
#define RTC_RESET_STATUS_REG			0x16

#define PMIC_STS_HW_CONDITION_CHRG		0x08
#define PMIC_CONTROLLER_STAT1_CHRG_DET_N		0x20
#define PMIC_CONTROLLER_STAT1_VAC		0x08

#define CONST_WAIT_TIME					0xA0
#define MSK_INT_LINE_A					0xFF
#define MSK_INT_LINE_B					0xFF
#define MSK_INT_LINE_C					0x80
#define MSK_INT_SRC_A					0xFF
#define MSK_INT_SRC_B					0x0F
#define MSK_INT_SRC_C					0x80
#define MSK_DISABLE 					0x00
#define MSK_CONTROLLER_INT				0xC0
#define CONST_INT_ID					0x1C
#define CONST_X1						0x05A1
#define CONST_X2						0x0CCC

# define MSK_POWER_STATE				0x03
# define MSK_POWER_VOLTAGE				0x1F

/* TUSB */
#define USB_BASE				(0xE6890000ul)
#define PHYFUNCTR   			(*((volatile unsigned short *)(USB_BASE + 0x104)))
#define PORT131_TUSB_RST		0x00000008								/* PORT131 (nTUSB_RST) */
#define USB_INT_STS						0x13
#define HOSTDISCONNECT					0x01
#define IDGND							0x10
/* Interrupt Status Register 0 */
#define	INTSTS0				(*((volatile unsigned short *)(USB_BASE + 0x040)))
#define	VBINT				(0x8000)	// b15: VBUS interrupt
#define	RESM				(0x4000)	// b14: Resume interrupt
#define	SOFR				(0x2000)	// b13: SOF frame update interrupt
#define	DVST				(0x1000)	// b12: Device state transition interrupt
#define	CTRT				(0x0800)	// b11: Control transfer stage transition interrupt
#define	BEMP				(0x0400)	// b10: Buffer empty interrupt
#define	NRDY				(0x0200)	// b9: Buffer not ready interrupt
#define	BRDY				(0x0100)	// b8: Buffer ready interrupt
#define	DVSQ				(0x0070)	// b6-4: Device state
#define	DS_SPD_CNFG			 0x0070		// Suspend Configured
#define	DS_SPD_ADDR		 	 0x0060		// Suspend Address
#define	DS_SPD_DFLT		 	 0x0050		// Suspend Default
#define	DS_SPD_POWR			 0x0040		// Suspend Powered
#define	DS_CNFG			 	 0x0030		// Configured
#define	DS_ADDS			 	 0x0020		// Address
#define	DS_DFLT			 	 0x0010		// Default
#define	DS_POWR				 0x0000		// Powered
#define	VALID				(0x0008)	// b3: Setup packet detected flag
#define	CTSQ				(0x0007)	// b2-0: Control transfer stage
#define	CS_SQER				 0x0006		// Sequence error
#define	CS_WRND				 0x0005		// Control write nodata status stage
#define	CS_WRSS				 0x0004		// Control write status stage
#define	CS_WRDS				 0x0003		// Control write data stage
#define	CS_RDSS				 0x0002		// Control read status stage
#define	CS_RDDS				 0x0001		// Control read data stage
#define	CS_IDST				 0x0000		// Idle or setup stage
/* System Configuration Control Register */
#define	SYSCFG				(*((volatile unsigned short *)(USB_BASE + 0x00)))
#define	SCKE				(0x0400)	// b10: USB clock enable
#define	HSE					(0x0080)	// b7: Hi-speed enable
#define	DCFM				(0x0040)	// b6: Controller Function Select(1:host controller)
#define	DPRPU				(0x0010)	// b4: D+ pull up control
#define	USBE				(0x0001)	// b0: USB module operation enable
/* Interrupt Enable Register 0 */
#define	INTENB0				(*((volatile unsigned short *)(USB_BASE + 0x030)))
#define	VBSE				(0x8000)	// b15: VBUS interrupt
#define	RSME				(0x4000)	// b14: Resume interrupt
#define	SOFE				(0x2000)	// b13: Frame update interrupt
#define	DVSE				(0x1000)	// b12: Device state transition interrupt
#define	CTRE				(0x0800)	// b11: Control transfer stage transition interrupt
#define	BEMPE				(0x0400)	// b10: Buffer empty interrupt
#define	NRDYE				(0x0200)	// b9: Buffer not ready interrupt
#define	BRDYE				(0x0100)	// b8: Buffer ready interrupt

#define MAX_VBUS_STS			2							
#define MAX_CHG_STS				2							
#define MAX_ENUMER_STS			2	
						
#define USB_NOT_CONNECT			0x0	
#define USB_UNKNOWN				0x1	
#define USB_SDP					0x2	
#define USB_CDP					0x10	
#define USB_DCP					0x11	

#define PMIC_CHRG_NONE		0
#define PMIC_CHRG_VBUS		1
#define PMIC_CHRG_VAC		2

#include "tmu_api.h"
#include "string.h"
#include "common.h"
#include "gpio.h"
#include "i2c.h"
#include "log_output.h"

/* Based year that RTC system refers is 1900,
 * 100 is used as OFFSET in TPS80032 to set the initial year is 2000
*/
#define RTC_POR_YEAR			0
#define RTC_POR_MONTH			1
#define RTC_POR_DAY				1

#define STOP_RTC				0x01
#define GETTIME					BITMASK_6
#define SET_32_COUNTER			BITMASK_5
#define ALARM_INT_STATUS		0x40
#define ENABLE_ALARM_INT 		0x08

typedef struct rtc_time {
	uchar tm_sec;
	uchar tm_min;
	uchar tm_hour;
	uchar tm_mday;
	uchar tm_mon;
	uchar tm_year;
	uchar tm_wday;
} RTC_TIME;


/* Function Prototypes */
/* pmic_battery.c */
RC pmic_init_battery_hw(void);
RC pmic_check_bat_state(void);

RC pmic_gpadc_correct_voltage(RC volt);
RC pmic_read_bat_volt(void);
RC pmic_calc_bat_capacity(void);

RC pmic_gpadc_correct_temp(RC temp);
RC pmic_read_bat_temp(void);

RC pmic_force_off_hw(void);
RC pmic_soft_reset(void);
RC pmic_set_power_off_resource(void);

/* pmic_charger.c */
RC pmic_check_charger(void);
RC pmic_enable_charger(uchar vac_vbus);
RC pmic_disable_charger(void);
RC pmic_set_current_limit(uchar current_limit);

uchar pmic_read_register(uchar client, uchar addr);

RC pmic_read_ctrlr_stat1(uchar *out);
RC pmic_read_sts_hw_conditions(uchar *out);
RC pmic_read_phoenix_start_condition(uchar *out);
void pmic_clear_phoenix_start_condition(void);

/* pmic_rtc.c */
uchar dec2bcd(uchar dec);
uchar bcd2dec(uchar bcd);
RC block_read(uchar client, RC reg, RC size, uchar *val);
RC block_write(uchar client, RC reg, RC size, uchar *val);
RC pmic_rtc_start(void);
RC pmic_rtc_stop(void);
RC pmic_rtc_read_time(RTC_TIME *tm);
RC pmic_rtc_set_time(RTC_TIME *tm);
void pmic_rtc_unlock(void);
void pmic_rtc_lock(void);

#endif /* __PMIC_BATTERY_H__ */
