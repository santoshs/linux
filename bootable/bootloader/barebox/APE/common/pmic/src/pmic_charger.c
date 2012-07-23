/*
 * pmic_charger.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "pmic.h"
#include "usb_api.h"
#include "common.h"

/*
 * pmic_read_ctrlr_stat1: Read data of CONTROLLER_STAT1 register
 * return:
 *          PMIC_OK: 				Normal return
 *          PMIC_ERR_I2C:			Error relating to I2C
 */
int pmic_read_ctrlr_stat1 (uchar *out)
{
	int ret;
	uchar r_data;
	
	/* Read the current value of CONTROLLER_STAT1 register */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_STAT1, &r_data);
	if ((ret != I2C_SUCCESS) || (0 > r_data)) {
		PRINTF("FAIL I2C read CONTROLLER_STAT1 error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	*out = r_data;
	return PMIC_OK;
}

/*
 * pmic_read_sts_hw_conditions: Read data of STS_HW_CONDITIONS register
 * return:
 *          PMIC_OK: 				Normal return
 *          PMIC_ERR_I2C:			Error relating to I2C
 */
int pmic_read_sts_hw_conditions (uchar *out)
{
	int ret;
	uchar r_data;
	
	/* Read the current value of STS_HW_CONDITIONS register */
	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_STS_HW_CONDITIONS, &r_data);
	if ((ret != I2C_SUCCESS) || (0 > r_data)) {
		PRINTF("FAIL I2C read STS_HW_CONDITIONS error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	*out = r_data;
	return PMIC_OK;
}

/*
 * pmic_read_phoenix_start_condition: Read data of PHOENIX_START_CONDITION register
 * return:
 *          PMIC_OK: 				Normal return
 *          PMIC_ERR_I2C:			Error relating to I2C
 */
int pmic_read_phoenix_start_condition (uchar *out)
{
	int ret;
	uchar r_data;
	
	/* Read the current value of PHOENIX_START_CONDITION register */
	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_PHOENIX_START_CONDITION, &r_data);
	if ((ret != I2C_SUCCESS) || (0 > r_data)) {
		PRINTF("FAIL I2C read PHOENIX_START_CONDITION error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	*out = r_data;
	return PMIC_OK;
}

/*
 * pmic_clear_phoenix_start_condition: Clear data of PHOENIX_START_CONDITION register
 * return:
 *          None
 */
void pmic_clear_phoenix_start_condition (void)
{
	int ret;
	/* Write 0 to PHOENIX_START_CONDITION register */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_PHOENIX_START_CONDITION, (uchar)0x0);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PHOENIX_START_CONDITION error - ret=%d data=0x%x\n", ret, 0);
	}
	return;
}

/*
 * pmic_enable_charger: Enable charger
 * input:
 *          vac_vbus          : 1 = VBUS
 *                            : 2 = VAC
 * return:
 *          PMIC_OK           : Normal return
 *          PMIC_ERR_I2C      : Error relating to I2C
 */
RC pmic_enable_charger(uchar vac_vbus)
{
	RC ret = 0;
	uchar ctrl_ctrl1;
	
	/* Read the current value of CONTROLLER_CTRL1 register */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_CTRL1, &ctrl_ctrl1);
	if ((I2C_SUCCESS != ret) || (0 > ctrl_ctrl1)) {
		PRINTF("FAIL I2C read CONTROLLER_CTRL1 error - ret=%d data=0x%x\n", ret, ctrl_ctrl1);
		return PMIC_ERR_I2C;
	}
	
	if (PMIC_CHRG_VBUS == vac_vbus) {
		/* Charge by VBUS */
		ctrl_ctrl1 = ctrl_ctrl1 & (~(uchar)BIT3);
		PRINTF("Enable charger VBUS\n");
	} else if (PMIC_CHRG_VAC == vac_vbus) {
		/* Charge by VAC */
		ctrl_ctrl1 = ctrl_ctrl1 | BIT3;
		PRINTF("Enable charger VAC\n");
	} else {
		return PMIC_ERR_ARGUMENT;
	}
	
	/* Enable charger if charger is present */
	ctrl_ctrl1 = ctrl_ctrl1 | BIT4 | BIT5;;
	
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_CTRL1, ctrl_ctrl1);
	if (I2C_SUCCESS != ret) {
		PRINTF("FAIL I2C write CONTROLLER_CTRL1 error - ret=%d data=0x%x\n", ret, ctrl_ctrl1);
		return PMIC_ERR_I2C;
	}
	
	return PMIC_OK;
}

/*
 * pmic_disable_charger: Disable charger
 * return:
 *          PMIC_OK           : Normal return
 *          PMIC_ERR_I2C      : Error relating to I2C
 */
RC pmic_disable_charger(void)
{
	RC ret = 0;
	uchar ctrl_ctrl1;
	
	/* Read the current value of CONTROLLER_CTRL1 register */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_CTRL1, &ctrl_ctrl1);
	if ((I2C_SUCCESS != ret) || (0 > ctrl_ctrl1)) {
		PRINTF("FAIL I2C read CONTROLLER_CTRL1 error - ret=%d data=0x%x\n", ret, ctrl_ctrl1);
		return PMIC_ERR_I2C;
	}
	
	/* Disable charger */
	ctrl_ctrl1  = ctrl_ctrl1 & (~BIT4);
	
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_CTRL1, ctrl_ctrl1);
	if (I2C_SUCCESS != ret) {
		PRINTF("FAIL I2C write CONTROLLER_CTRL1 error - ret=%d data=0x%x\n", ret, ctrl_ctrl1);
		return PMIC_ERR_I2C;
	}
	
	PRINTF("Charger is disabled\n");	
	return PMIC_OK;
}

/*
 * pmic_set_current_limit: sets VBUS input current limit
 * input:
 *          uchar current_limit       : Current limit for setting
 * return:
 *          PMIC_OK:                  : Normal return
 *          PMIC_ERR_I2C              : Error occurs
 */
RC pmic_set_current_limit(uchar current_limit)
{
	RC ret;
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_CINLIMIT, current_limit);
	if (I2C_SUCCESS != ret) {
		PRINTF("FAIL I2C write PMIC_CHARGERUSB_CINLIMIT error - ret=%d data=0x%x\n", ret, current_limit);
		return PMIC_ERR_I2C;
	}
	
	/* Print log */
	switch (current_limit) {
		case PMIC_CUR_LIMIT_100mA:
			PRINTF("Set current limit 100mA\n");
			break;
		case PMIC_CUR_LIMIT_300mA:
			PRINTF("Set current limit 300mA\n");
			break;
		case PMIC_CUR_LIMIT_500mA:
			PRINTF("Set current limit 500mA\n");
			break;
		case PMIC_CUR_LIMIT_1500mA:
			PRINTF("Set current limit 1.5A\n");
			break;
		default:
			/* Nothing to print */
			break;
	}

	return PMIC_OK;
}

/*
 * pmic_check_charger: Check the present of charger
 * return:
 *          PMIC_CHRG_NONE    : Charger is not present
 *          PMIC_CHRG_VBUS    : Charger is VBUS
 *          PMIC_CHRG_VAC     : Charger is VAC
 *          PMIC_ERR_I2C      : Error relating to I2C
 */
RC pmic_check_charger(void)
{
	RC ret = 0;
	uchar r_data;
	uchar charger_det = 0;
	
	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_STS_HW_CONDITIONS, &r_data);
	if ((I2C_SUCCESS != ret) || (0 > r_data)) {
		PRINTF("FAIL I2C read STS_HW_CONDITIONS error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	charger_det = ((r_data & PMIC_STS_HW_CONDITION_CHRG) == PMIC_STS_HW_CONDITION_CHRG) ? 1 : 0;
	
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_STAT1, &r_data);
	if ((I2C_SUCCESS != ret) || (0 > r_data)) {
		PRINTF("FAIL I2C read CONTROLLER_STAT1 error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	if (charger_det == 1) {
		if ((r_data & PMIC_CONTROLLER_STAT1_VAC) == PMIC_CONTROLLER_STAT1_VAC) {
			return PMIC_CHRG_VAC;
		} else {
			return PMIC_CHRG_VBUS;
		}
	} else {
		return PMIC_CHRG_NONE;
	}
}