/*
 * r_loader_boot_matrix.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "pmic.h"
#include "r_loader_boot_matrix.h"
#include "r_loader_boot_log.h"
#include "log_output.h"
#include "usb_api.h"
#include "string.h"
#include "ths_api.h"
#include "libc.h"
#include "gpio.h"

/**
 * Definition 
 */
/* Boot matrix */
BOOT_MATRIX matrix_info;
 
/* Bit mask for sub ID */
static uint start_sub_event[MAX_SUB_EVENT] = {
	(uint) 0x00000000,
	(uint) 0x00010000,
	(uint) 0x00000100,
	(uint) 0x00080000,
	(uint) 0x00200000,
	(uint) 0x00100000,
	(uint) 0x00000014,
	(uint) 0x0000000C,
	(uint) 0x00000024,
	(uint) 0x00000026,
	(uint) 0x00000084
};

/* Boot cube for search: [batt status][[charging status][action]
   Please refer to EOS_R-Loader_Boot_matrix.xls for the detail of this cube */
static uchar boot_cube_dict[MAX_BATT_STS][MAX_CHRG_STS][MAX_EVENT] = {
{{	MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_OFF_CHARGE,		MAT_HIGH_TEMP,	},
 {	MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_OFF_CHARGE,		MAT_HIGH_TEMP,	},
 {	MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_HIGH_TEMP,	},
 {	MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_HIGH_TEMP,	},
 {	MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_BOOT_SYS,		MAT_PWR_OFF,		MAT_HIGH_TEMP,	}},

{{	MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_HIGH_TEMP,	},
 {	MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_OFF_CHARGE,		MAT_HIGH_TEMP,	},
 {	MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_HIGH_TEMP,	},
 {	MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_HIGH_TEMP,	},
 {	MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_HIGH_TEMP,	}},

{{	MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_PWR_OFF,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,	},
 {	MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_PWR_OFF,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,		MAT_NO_BATT,	},
 {	MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,	},
 {	MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,	},
 {	MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,		MAT_PWR_OFF,	}} 
};

/**
 * Function implementation
 */

/**
 * [Internal function] get_event_id: Get start event ID
 * intput:
 *      ulong input_mask:                  Bit mask of Start event
 * return:
 *      Start event ID
 */
ulong get_event_id (ulong input_mask)
{
	int sub_event;
	int pos = 0;
	ulong res_id = 0;
	ulong event_mask = input_mask;
	
	while (event_mask != 0x0)
	{
		sub_event = MAX_SUB_EVENT - 1;
		while (sub_event >= 0)
		{	
			if ((event_mask & start_sub_event[sub_event]) == start_sub_event[sub_event]) {
				break;
			}
			sub_event--;
		}
		
		int pwr = 1;
		int i;
		for (i = 0; i < pos; i++)
		{
			pwr = pwr * 16;
		}
		
		res_id = res_id + (sub_event * pwr);
		pos++;

		/* Get remain part of event mask */
		event_mask = event_mask & ~(start_sub_event[sub_event]);
		
		/* Expire */
		if (pos > 10) {
			PRINTF ("Get event ID loop over 10 times - might error in event mask 0x%x\n", input_mask);
			break;
		}
	}
	
	return res_id;
}

/**
 * [Internal function] get_power_control: Get value of POWER_CONTROL register of TUSB1211
 * return:
 *      (uchar)                     Value of POWER_CONTROL register
 */
uchar get_power_control(void)
{
	uchar port130cr_bck = 0;
	ulong port159_128dr_bck = 0;
	uchar r_data;
	
	/* Back up GPIO */
	port130cr_bck = *GPIO_PORT130CR;
	port159_128dr_bck = *GPIO_PORTD159_128DR_ES2;
	
	/** Step 1: Change to HW control */
	/* CS = 0 - GPIO130 = LOW */
	*GPIO_PORT130CR = GPIO_PORT130DR_BOOTMATRIX_DATA;
	*GPIO_PORTD159_128DR_ES2 &= ~GPIO_PORTD159_128DSR_PORT130_DATA;
	
	/* SW_CONTROL = 0 */
	usb_phy_write(TUSB_POWER_CONTROL, TUSB_POWER_CONTROL_HWCONTROL);
	
	/* PHY reset */
	usb_phy_reset();
	
	/* Wait for detecting charger status */
	TMU_Wait_MS(200);
	
	/** Step 2: Change to SW control */
	/* CS = 1 */
	*GPIO_PORTD159_128DR_ES2 |= GPIO_PORTD159_128DSR_PORT130_DATA;
	
	/* SW_CONTROL = 1 */
	usb_phy_write(TUSB_POWER_CONTROL, TUSB_POWER_CONTROL_SWCONTROL);
	
	/* Wait for updating to HWDETECT bit */
	TMU_Wait_MS(100);
	
	/* Read HWDETECT */
	r_data = usb_phy_read(TUSB_POWER_CONTROL);
	
	/** Step 3: Restore GPIO and return */
	*GPIO_PORTD159_128DR_ES2 = port159_128dr_bck;
	*GPIO_PORT130CR = port130cr_bck;
	return r_data;
}

/**
 * [Internal function] detect_charger: Detect charger type
 * return:
 *      CHRG_USB_SDP				Charger is SDP
 *      CHRG_USB_CDP_DCP			Charger is Chaging Port (CDP/DCP)
 *      CHRG_NONE					No charger
 *      CHRG_VAC					Charger is VAC
 */
uchar detect_charger (void)
{
	uchar vbus_det = 0;
	uchar vac_det = 0;
	uchar chrg_det = 0;
	
	uchar ret = CHRG_NONE;
	
	/* Detect information */
	vbus_det = (matrix_info.ctlr_stat1 & (uchar)BIT2) >> 2;
	vac_det = (matrix_info.ctlr_stat1 & (uchar)BIT3) >> 3;
	chrg_det = (matrix_info.power_control & (uchar)BIT7) >> 7;
	
	/* USB type detect flow */
	if (vac_det > 0) {
		ret = CHRG_VAC;
	} else {
		if (vbus_det > 0) {
			if (chrg_det > 0) {
				/* In this case, CDP/DCP can be seperated */
				ret = CHRG_USB_CDP_DCP;
			} else {
				ret = CHRG_USB_SDP;
			}
		} else {
			ret = CHRG_NONE;
		}
	}
	
	return ret;
}

/**
 * [Internal function] detect_battery: Detect battery status
 * return:
 *      BATT_ENOUGH			Battery voltage is exceed 3.45 vol
 *      BATT_EMPTY			Battery voltage is less than 3.45 vol
 *      BATT_OFF			No battery
 */
uchar detect_battery (void)
{
	uchar batt_det = 0;
	
	uchar ret = BATT_OFF;
	
	/* Detect information */
	batt_det = ((~matrix_info.ctlr_stat1) & (uchar)BIT1) >> 1;
	
	/* Battery status detect flow */
	if (batt_det > 0) {
		if (matrix_info.batt_vol > BOOT_BATT_VOL) {
			ret = BATT_ENOUGH;
		} else {
			ret = BATT_EMPTY;
		}
	} else {
		ret = BATT_OFF;
	}
	
	return ret;
}

/**
 * boot_matrix_update: Update boot matrix info
 * return:
 *        BOOT_LOG_OK: 				Normal return
 *        BOOT_LOG_ERR_INIT_PMIC:	Error in initializing PMIC
 *        BOOT_LOG_ERR_PMIC:		Error relating to PMIC module
 */
RC boot_matrix_update(void)
{
	RC ret;
	RC bat_volt1;
	RC bat_volt2;
	RC temp_bat;
	/* Get SRSTFR */
	matrix_info.srstfr = *SRSTFR;
	
	/* Get STBCHRB2 */
	matrix_info.stbchrb2 = *STBCHRB2;
	
	/* Get ctlr_stat1 */
	ret = pmic_read_ctrlr_stat1(&(matrix_info.ctlr_stat1));
	if (PMIC_OK > ret) {
		PRINTF("FAIL PMIC read CONTROLLER_STAT1 error - ret=%d\n", ret);
		return BOOT_LOG_ERR_PMIC;
	}
	
	/* POWER off early to avoid dead window (condition: SW_RES_PWROFF + No VBUS + No VAC) */
	if ((STBCHRB2_PWROFF == (matrix_info.stbchrb2 & STBCHRB2_PWROFF)) \
		&& (SRSTFR_RESCNT2 == (matrix_info.srstfr & SRSTFR_RESCNT2))  \
		&& (0x00 == (0x0C & matrix_info.ctlr_stat1))) {
		/* Power OFF */
		PRINTF("Power Off sequence - POWER OFF\n");
		pmic_force_off_hw();
	}
	
	/* Get TUSB1211 - POWER_CONTROL */
	matrix_info.power_control = get_power_control();
	
	/* USB charger detection */
	matrix_info.charger = detect_charger();

	/* Get PHOENIX_START_CONDITION */
	ret = pmic_read_phoenix_start_condition(&(matrix_info.phoenix_start_con));
	if (PMIC_OK > ret) {
		PRINTF("FAIL PMIC read PHOENIX_START_CONDITION error - ret=%d\n", ret);
		return BOOT_LOG_ERR_PMIC;
	}
	
	/* Get STS_HW_CONDITIONS */
	ret = pmic_read_sts_hw_conditions(&(matrix_info.sts_hw_conditions));
	if (PMIC_OK > ret) {
		PRINTF("FAIL PMIC read STS_HW_CONDITIONS error - ret=%d\n", ret);
		return BOOT_LOG_ERR_PMIC;
	}
	
	/* Get long power key press */
	if ((0x01 & matrix_info.sts_hw_conditions) == 0x00) {
		matrix_info.long_pw_key = 1;
	}
	
	/* Get Battery state */
	ret = pmic_check_bat_state();
	if (PMIC_BAT_DETECT == ret) {
		matrix_info.batt_det = 1;
	} else if (PMIC_BAT_NOT_DETECT == ret) {
		matrix_info.batt_det = 0;
	} else {
		PRINTF("FAIL PMIC check batt error - ret=%d\n", ret);
		return BOOT_LOG_ERR_PMIC;
	}
	
	/* Reset PMIC for getting battery capacity */
	ret = pmic_init_battery_hw();
	if (0 > ret) {
		PRINTF("FAIL PMIC init error - ret=%d\n", ret);
		return BOOT_LOG_ERR_INIT_PMIC;
	}
	
	/* Get battery voltage */
	while(1){
		ret = pmic_init_battery_hw();
		if (0 > ret) {
			PRINTF("FAIL PMIC init error - ret=%d\n", ret);
			return BOOT_LOG_ERR_INIT_PMIC;
		}
		bat_volt1 = pmic_read_bat_volt();
		TMU_Wait_MS(50);
		ret = pmic_init_battery_hw();
		if (0 > ret) {
			PRINTF("FAIL PMIC init error - ret=%d\n", ret);
			return BOOT_LOG_ERR_INIT_PMIC;
		}
		bat_volt2 = pmic_read_bat_volt();
		temp_bat = bat_volt2 - bat_volt1;
		if((temp_bat > -5) || (temp_bat < 5)){
			matrix_info.batt_vol = (ushort)bat_volt2;
			break;
		}
	}

	/* Get CPU temperature */
	if (CHIP_VERSION() >= CHIP_RMU2_ES20) {
		ths_init();
		matrix_info.cpu_temp = ths_get_cpu_temp();
	}

	/* Clear PMIC Registers of Start event */
	pmic_clear_phoenix_start_condition();
	
	return BOOT_LOG_OK;
}

/**
 * boot_matrix_check: check the boot matrix
 * return:
 *        MAT_BOOT_SYS:				Continue booting system
 *        MAT_OFF_CHARGE:			Branch to Off-charge module
 *        MAT_PWR_OFF:				Power of system
 *        MAT_DONT_CARE:			Don't care result. Continue.
 */
uchar boot_matrix_check(void)
{	
	uchar batt_stat = BATT_ENOUGH;
	uchar charger_type = CHRG_NONE;
	uchar act = MAT_BOOT_SYS;
	
	/**-- Get ACT --*/
	ulong event_mask = 0;
	ulong event_id = 0;
	
	/* Get event mask - PHOENIX_START_CONDITION: Get Bit 5,4,3,2,0 */
	event_mask = event_mask | ((uint)(matrix_info.phoenix_start_con & (BIT5 | BIT4 | BIT3 | BIT2 | BIT0)) << 16);
	/* Get event mask - STS_HW_CONDITIONS: Get bit 0 and Invert (bit STS_PWRON is active 0) */
	event_mask = event_mask | ((uint)(~(matrix_info.sts_hw_conditions) & BIT0) << 8);
	/* Get event mask - SRSTFR: Get bit 31,5,4,3,0 */
	event_mask = event_mask | ((matrix_info.srstfr & BIT31) >> 24);
	event_mask = event_mask | (matrix_info.srstfr & (BIT5 | BIT4 | BIT3));
	event_mask = event_mask | ((matrix_info.srstfr & BIT0) << 2);
	/* Get event mask - STBCHRB2: Get bit 7 */
	event_mask = event_mask | ((uint)(matrix_info.stbchrb2 & BIT7) >> 6);
	PRINTF("Start event mask 0x%X\n", event_mask);
	
	/* Get event ID */
	event_id = get_event_id(event_mask);
	PRINTF("Start event ID 0x%X\n", event_id);
	
	/* Get start event */
	act = div_mode(event_id, 16);
	
	if (MAX_SUB_EVENT < act) {
		PRINTF("Start event not found... BOOT ANDROID\n");
		return MAT_BOOT_SYS;
	}
	
	/**-- Get BATT_STAT --*/
	matrix_info.batt_det = detect_battery();
	
	/** Get CHARGER_TYPE */
	matrix_info.charger = detect_charger();
	/* BEGIN: QA1037: Change request on the power up method */
	charger_type = matrix_info.charger;
	/* END: QA1037: Change request on the power up method */

#ifndef _R_LOADER_IGNORE_BATT_CHECK_
	PRINTF("BATT check on boot\n");
	batt_stat = matrix_info.batt_det;
#else
	PRINTF ("Ignore BATT check on boot\n");
	batt_stat = BATT_ENOUGH;
#endif	/* _R_LOADER_IGNORE_BATT_CHECK_ */

	/* Return result */
	PRINTF ("Boot matrix - batt_stat=%d, charger_type=%d, start_event=%d\n", batt_stat, charger_type, act);
	return boot_cube_dict[batt_stat][charger_type][act];
}

/**
 * boot_matrix_print: Print boot matrix info
 * return:
 *        None
 */
void boot_matrix_print(void)
{
	PRINTF("Boot matrix info\n");
	PRINTF("   .[PMIC]PHOENIX_START_CONDITINOS=0x%X\n", matrix_info.phoenix_start_con);
	PRINTF("   .[PMIC]STS_HW_CONDITION=0x%X\n", matrix_info.sts_hw_conditions);
	PRINTF("   .[PMIC]CONTROLLER_STAT1=0x%X\n", matrix_info.ctlr_stat1);
	PRINTF("   .[R-MU2]SRSTFR=0x%X\n", matrix_info.srstfr);
	PRINTF("   .[R-MU2]STBCHRB2=0x%X\n", matrix_info.stbchrb2);
	PRINTF("   .[USB]POWER_CONTROL=0x%X\n", matrix_info.power_control);
	PRINTF("   .Charger STS=%d\n", matrix_info.charger);
	PRINTF("   .Battery STS=%d\n", matrix_info.batt_det);
	PRINTF("   .Battery VOL=%d\n", matrix_info.batt_vol);
	PRINTF("   .CPU Temperature=%d\n", matrix_info.cpu_temp);
	PRINTF("   .Long PWR key STS=%d\n", matrix_info.long_pw_key);
}
