/*
 * pmic_battery.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "pmic.h"
#include "common.h"

static short BAT_VOLT_THRESHOLD[100];

/*
 * pmic_read_bat_volt: read the battery voltage
 * @client: The I2C client slave address.
 * return:
 *        > 0: 					Battery voltage
 *        PMIC_ERR_I2C:			Error relating to I2C
 *        PMIC_ERR_TIMEOUT:		Time out
 */
RC pmic_read_bat_volt(void)
{
	RC result = 0;
	uchar r_data = 0;
	RC ret, count_timer = 0;
	RC ret_MSB, ret_LSB;

	/* Set 5V scaler and other internal ADC reference */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPADC_CTRL, 0x6B);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write GPADC_CTRL error - ret=%d data=0x%x\n", ret, 0x6B);
		return PMIC_ERR_I2C;
	}

	/* Set 5V scaler and enable VBAT */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPADC_CTRL2, 0x0C);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write GPADC_CTRL error - ret=%d data=0x%x\n", ret, 0x0C);
		return PMIC_ERR_I2C;
	}

	/* Enable GPADC */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_TOGGLE1, 0x0E);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_TOGGLE1 error - ret=%d data=0x%x\n", ret, 0x0E);
		return PMIC_ERR_I2C;
	}

	/* Select VBAT measurement channel */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPSELECT_ISB, 0x12);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write GPSELECT_ISB error - ret=%d data=0x%x\n", ret, 0x12);
		return PMIC_ERR_I2C;
	}

	/* Start GPADC */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CTRL_P1, 0x08);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write CTRL_P1 error - ret=%d data=0x%x\n", ret, 0x08);
		return PMIC_ERR_I2C;
	}

	/* Wait for ADC interrupt */
	while (count_timer <= CONST_WAIT_TIME) {
		TMU_Wait_MS(1);

		/* Check ADC intterupt bit */
		ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_INT_STS_B, &r_data);
		
		if (ret != I2C_SUCCESS) {
			PRINTF("FAIL I2C read INT_STS_B error - ret=%d data=0x%x\n", ret, r_data);
			result = PMIC_ERR_I2C;
			goto disable;
		} else if (0 != (r_data & BIT5)) {
			/* Conversion finished */
			ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_B, MSK_DISABLE);
			if (ret != I2C_SUCCESS) {
				PRINTF("FAIL I2C write INT_STS_B error - ret=%d data=0x%x ## go to disable\n", ret, MSK_DISABLE);
				goto disable;
			}
			break;
		} else {
			count_timer++;
			/* Do nothing */
		}
	}

	if (CONST_WAIT_TIME < count_timer) {
		/* Time out */
		PRINTF("FAIL time out ## go to disable\n");
		goto disable;
	}
	
	/* Read the VBAT conversion result */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_GPCH0_MSB, &r_data);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read GPCH0_MSB error - ret=%d data=0x%x ## go to disable\n", ret, r_data);
		result = PMIC_ERR_I2C;
		goto disable;
	} else {
		ret_MSB = r_data;
	}

	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_GPCH0_LSB, &r_data);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read GPCH0_LSB error - ret=%d data=0x%x ## go to disable\n", ret, r_data);
		result = PMIC_ERR_I2C;
		goto disable;
	} else {
		ret_LSB = r_data;
	}

	/* Correct the result */
	result = ((ret_MSB & 0x0F)<<8) | ret_LSB;
	result = (result * 5000) / 4096;

disable:
	/* Disable GPAD */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_TOGGLE1, 0x01);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write TOGGLE1 error - ret=%d data=0x%x ## go to disable\n", ret, 0x01);
	}

	// /*Disable scaler */
	// ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPADC_CTRL, 0x00);
	// if (ret != I2C_SUCCESS) {
		// PRINTF("FAIL I2C write TOGGLE1 error - ret=%d data=0x%x ## go to disable\n", ret, 0x00);
	// }

	// /*Disable scaler */
	// ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPADC_CTRL2, 0x00);
	// if (ret != I2C_SUCCESS) {
		// PRINTF("FAIL I2C write TOGGLE1 error - ret=%d data=0x%x ## go to disable\n", ret, 0x00);
	// }

	/* Correct the battery voltage */
	if (0 <= result) {
		result = pmic_gpadc_correct_voltage(result);
	}
	return result;
}

/*
 * pmic_gpadc_correct_voltage: correct the battery voltage
 * @volt: the battery voltage
 * return:
 *        > 0: the battery voltage after correct
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
RC pmic_gpadc_correct_voltage(RC volt)
{
	RC d1, d2;
	uchar ret;
	RC ret_temp1, ret_temp2;
	uchar ret_trim1, ret_trim2, ret_trim3, ret_trim4, ret_trim5, ret_trim6;
	RC sign_trim1, sign_trim2, sign_trim5, sign_trim6;
	RC offset, gain, result;

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM1, &ret_trim1);
	if ((ret != I2C_SUCCESS) || (0 > ret_trim1)) {
		PRINTF("FAIL I2C read GPADC_TRIM1 error - ret=%d data=0x%x ## go to disable\n", ret, ret_trim1);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM2, &ret_trim2);
	if ((ret != I2C_SUCCESS) || (0 > ret_trim2)) {
		PRINTF("FAIL I2C read GPADC_TRIM2 error - ret=%d data=0x%x ## go to disable\n", ret, ret_trim2);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM3, &ret_trim3);
	if ((ret != I2C_SUCCESS) || (0 > ret_trim3)) {
		PRINTF("FAIL I2C read GPADC_TRIM3 error - ret=%d data=0x%x ## go to disable\n", ret, ret_trim3);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM4, &ret_trim4);
	if ((ret != I2C_SUCCESS) || (0 > ret_trim4)) {
		PRINTF("FAIL I2C read GPADC_TRIM4 error - ret=%d data=0x%x ## go to disable\n", ret, ret_trim4);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM5, &ret_trim5);
	if ((ret != I2C_SUCCESS) || (0 > ret_trim5)){
		PRINTF("FAIL I2C read GPADC_TRIM5 error - ret=%d data=0x%x ## go to disable\n", ret, ret_trim5);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM6, &ret_trim6);
	if ((ret != I2C_SUCCESS) || (0 > ret_trim6)) {
		PRINTF("FAIL I2C read GPADC_TRIM6 error - ret=%d data=0x%x ## go to disable\n", ret, ret_trim6);
		return PMIC_ERR_I2C;
	}

	sign_trim1 = ret_trim1 & BIT0;
	sign_trim2 = ret_trim2 & BIT0;
	sign_trim5 = ret_trim5 & BIT0;
	sign_trim6 = ret_trim6 & BIT0;

	ret_trim1 = ret_trim1 & (BIT2 | BIT1);
	ret_trim2 = ret_trim2 & (BIT2 | BIT1);
	ret_trim3 = ret_trim3 & (BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
	ret_trim4 = ret_trim4 & (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
	ret_trim5 = ret_trim5 & (BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1);
	ret_trim6 = ret_trim6 & (BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1);

	ret_temp1 = ret_trim3 *4 + ret_trim1;
	ret_temp2 = ret_trim5;

	if (1 == sign_trim1) {
		ret_temp1 = -ret_temp1;
	}

	if (1 == sign_trim5) {
		ret_temp2 = -ret_temp2;
	}

	d1 = ret_temp1 + ret_temp2;

	ret_temp1 = ret_trim4 *4 + ret_trim2;
	ret_temp2 = ret_trim6;

	if (1 == sign_trim2) {
		ret_temp1 = -ret_temp1;
	}

	if (1 == sign_trim6) {
		ret_temp2 = -ret_temp2;
	}

	d2 = ret_temp1 + ret_temp2;

	gain = bi_div((d2 - d1), (CONST_X2 - CONST_X1)) + 1;
	offset = d1 - (gain - 1) * CONST_X1;

	result = bi_div((volt - offset), gain);

	return result;
}

/*
 * pmic_calc_bat_capacity: calculate the battery capacity
 * return:
 *        > 0: Battery capacity
 *        PMIC_ERR_I2C:			Error relating to I2C
 *        PMIC_ERR_TIMEOUT:		Time out
 */
RC pmic_calc_bat_capacity(void)
{
	int ret;
	int ret_vbat;
	uchar ret_vsysmin_hi;
	uchar ret_voreg;
	int soc;
	int i;
	
	int VOREG[63] = {3500, 3250, 3540, 3560, 3580, 3600, 3620, 3640, 3660, 3680, 
					   3700, 3720, 3740, 3760, 3780, 3800, 3820, 3840, 3860, 3880,
					   3900, 3920, 3940, 3960, 3980, 4000, 4020, 4060, 4080, 4100,
					   4120, 4140, 4160, 4180, 4200, 4220, 4240, 4260, 4280, 4300,
					   4320, 4340, 4360, 4380, 4400, 4420, 4440, 4460, 4480, 4500,
					   4520, 4540, 4560, 4580, 4600, 4620, 4640, 4660, 4680, 4700,
					   4720, 4740, 4760};

	int VSYS[52] = {2050, 2100, 2150, 2200, 2250, 2300, 2350, 2400, 2450, 2500,
					  2550, 2600, 2650, 2700, 2750, 2800, 2850, 2900, 2950, 3000,
					  3050, 3100, 3150, 3200, 3250, 3300, 3350, 3400, 3450, 3500,
					  3550, 3600, 3650, 3700, 3750, 3800, 3850, 3900, 3950, 4000,
					  4050, 4100, 4150, 4200, 4250, 4300, 4350, 4400, 4450, 4500, 
					  4550, 4600};

	/* Set value for BAT_VOLT_THRESHOLD array */
	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_VSYSMIN_HI_THRESHOLD, &ret_vsysmin_hi);
	if (0 > ret) {
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_VOREG, &ret_voreg);
	if (0 > ret_voreg) {
		return PMIC_ERR_I2C;
	}

	for(i=0; i<100; i++) {
		BAT_VOLT_THRESHOLD[i] = (short)((((VOREG[ret_voreg] - VSYS[ret_vsysmin_hi])*(i+1)) + 50)/100+VSYS[ret_vsysmin_hi]);
	}

	ret_vbat = pmic_read_bat_volt();

	for(i=99; i>0; i--) {
		if(BAT_VOLT_THRESHOLD[i] <= ret_vbat) {
			break;
		}
	}
	soc = i + 1;

	return soc;
}

/*
 * pmic_force_off_hw: force off PMIC hardware, including all power resources
 *
 * return: PMIC_ERR_I2C: Error relating to I2C
*/
RC pmic_force_off_hw(void)
{
	RC ret;
	RC val;
	uchar r_data;
	
	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_PHOENIX_DEV_ON, &r_data);
	if ((ret != I2C_SUCCESS) || (r_data < 0)) {
		PRINTF("FAIL I2C read PHOENIX_DEV_ON error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}

	val = r_data & ~BIT6; /* make sure SW_RESET bit is cleared */
	val |= BIT0; /* set DEVOFF bit */

	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_PHOENIX_DEV_ON, val);
	if (ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write PHOENIX_DEV_ON error - ret=%d data=0x%x\n", ret, val);
		return PMIC_ERR_I2C;
	}
	
	while(1){
		/* Loop forever */
	}
}

/*
 * pmic_soft_reset: soft reset used PMIC
 *
 * return: PMIC_ERR_I2C: Error relating to I2C
*/
RC pmic_soft_reset(void)
{
	RC ret;
	RC val;
	uchar r_data ;

	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_PHOENIX_DEV_ON, &r_data);
	if ((ret != I2C_SUCCESS) || (0 > r_data)) {
		PRINTF("FAIL I2C read PHOENIX_DEV_ON error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	val = r_data | BIT6; /* make SW_RESET bit is set */

	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_PHOENIX_DEV_ON, val);
	if (ret != I2C_SUCCESS){
		PRINTF("FAIL I2C read PHOENIX_DEV_ON error - ret=%d data=0x%x\n", ret, val);
		return PMIC_ERR_I2C;
	}
	
	while(1){
		/* Loop forever */
	}
}

/*
 * pmic_init_battery_hw: init hardware for battery management
 * return:
 *        PMIC_OK: 					Normal operation
 *        PMIC_ERR_I2C: 			Error relating to I2C
 */
RC pmic_init_battery_hw(void)
{
	RC ret = 0;
	uchar r_data;
	RC val;

	/* Mask interrupt line signal A */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_MSK_LINE_STS_A, MSK_INT_LINE_A);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_MSK_LINE_STS_A error - ret=%d; data=0x%x\n", ret, MSK_INT_LINE_A);
		return PMIC_ERR_I2C;
	}

	/* Mask interrupt line signal B */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_MSK_LINE_STS_B, MSK_INT_LINE_B);
	if (ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_MSK_LINE_STS_B error - ret=%d; data=0x%x\n", ret, MSK_INT_LINE_B);
		return PMIC_ERR_I2C;
	}

	/* Mask interrupt line signal C */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_MSK_LINE_STS_C, MSK_INT_LINE_C);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_MSK_LINE_STS_C error - ret=%d; data=0x%x\n", ret, MSK_INT_LINE_C);
		return PMIC_ERR_I2C;
	}

	
	/* Mask interrupt signal A */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_MSK_STS_A, MSK_INT_SRC_A);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_MSK_STS_A error - ret=%d; data=0x%x\n", ret, MSK_INT_SRC_A);
		return PMIC_ERR_I2C;
	}

	/* Mask interrupt signal B */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_MSK_STS_B, MSK_INT_SRC_B);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_MSK_STS_ error - ret=%d; data=0x%x\n", ret, MSK_INT_SRC_B);
		return PMIC_ERR_I2C;
	}

	/* Mask interrupt signal C */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_MSK_STS_C, MSK_INT_SRC_C);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_MSK_STS_C error - ret=%d; data=0x%x\n", ret, MSK_INT_SRC_C);
		return PMIC_ERR_I2C;
	}

	/* Setting for constant voltage (CV) for full-charge phase */
	//3700mV ->0x0A
	//3760mV -> 0x0D
	//3600mV -> 0x05
	//3660mV -> 0x08
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_VOREG, 0x08);
	if (0 > ret) {
		return ret;
	}

	/* Setting for constant current (CC) for full-charge phase */
	//300mA -> 0x02
	//400mA -> 0x04
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_VICHRG, 0x04);
	if (0 > ret) {
		return ret;
	}

	/* Setting for charging current for pre-charge phase */
	//400mA -> 0x03
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_VICHRG_PC, 0x03);
	if (0 > ret) {
		return ret;
	}

	/* Setting for Vbat tracking for full-charge phase */
	//DLIN = 50mV, VBAT_FULLCHRG = 3050mV, VBAT_SHORT = 2100mV
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_VSEL_COMP, 0x10);
	if (0 > ret) {
		return ret;
	}

	/* Setting for low voltage limitation */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_VSYSMIN_HI_THRESHOLD, 0x17);	/* 3.15 V */
	if (0 > ret) {
		return ret;
	}

	/* Enable Charge current termination interrupt */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_CTRL1, &r_data);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C read CHARGERUSB_CTRL1 error - ret=%d; data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}

	r_data |= BIT4;

	/* Set 1 to TERM bit at CHARGERUSB_CTRL1 (0xE8) register */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_CTRL1, r_data);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write CHARGERUSB_CTRL1 error - ret=%d; data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}

	/* Enable charge one feature */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_CTRL3, &r_data);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C read CHARGERUSB_CTRL3 error - ret=%d; data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}

	r_data |= BIT6;

	/* Set 1 to CHARGE_ONCE bit at CHARGERUSB_CTRL3 (0xEA) register */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_CTRL3, r_data);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write CHARGERUSB_CTRL3 error - ret=%d; data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	/* Disable interrupt related to EXT_CHRG */	
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_INT_MASK, MSK_CONTROLLER_INT);
	if (ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write CONTROLLER_INT_MASK error - ret=%d; data=0x%x\n", ret, MSK_CONTROLLER_INT);
		return PMIC_ERR_I2C;
	}

	/* Clear all interrupt source */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_A, MSK_DISABLE);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_STS_A error - ret=%d; data=0x%x\n", ret, MSK_DISABLE);
		return PMIC_ERR_I2C;
	}
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_B, MSK_DISABLE);
	if (ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_STS_B error - ret=%d; data=0x%x\n", ret, MSK_DISABLE);
		return PMIC_ERR_I2C;
	}
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_C, MSK_DISABLE);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C write INT_STS_C error - ret=%d; data=0x%x\n", ret, MSK_DISABLE);
		return PMIC_ERR_I2C;
	}
	/* Enable charger if charger is present at the boot up of driver */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_STAT1, &r_data);
	if(ret != I2C_SUCCESS){
		PRINTF("FAIL I2C read CONTROLLER_STAT1 error - ret=%d; data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	if (0 != (r_data & BIT2)) {
		ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_CTRL1, &r_data);
		if (ret != I2C_SUCCESS){
			PRINTF("FAIL I2C read CONTROLLER_CTRL1 error - ret=%d; data=0x%x\n", ret, r_data);
			return PMIC_ERR_I2C;
		}
		
		/* Enable charger and set VBUS is source charge */
		val = r_data | BIT4 | BIT5;
		val = val & (~BIT3);

		ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_CTRL1, val);
		if (ret != I2C_SUCCESS){
			PRINTF("FAIL I2C write CONTROLLER_CTRL1 error - ret=%d; data=0x%x\n", ret, val);
			return PMIC_ERR_I2C;
		}
	}
	
	return PMIC_OK;
}

/*
 * pmic_check_bat_state: detect the baterry state
 * return:
 *          PMIC_BAT_DETECT: 		Battery present
 *          PMIC_BAT_NOT_DETECT: 	Battery not present 
 *          PMIC_BAT_OVER_VOLT: 	Battery present but over voltage
 *          PMIC_ERR_I2C: 			I2C error
 */
RC pmic_check_bat_state(void)
{
	RC ret = 0;
	uchar r_data;
	
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_STAT1, &r_data);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read CONTROLLER_STAT1 error - ret=%d data=0x%x\n", ret, r_data);
		goto exit;
	}
	
	if (0 != (r_data & BIT1)) {
		return PMIC_BAT_NOT_DETECT;
	}
	
	/* Read the state of battery voltage is over or not */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CHARGERUSB_STATUS_INT1, &r_data);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read CHARGERUSB_STATUS_INT1 error - ret=%d data=0x%x\n", ret, r_data);
		goto exit;
	}
	
	if (0 == (r_data & BIT3)) {
		return PMIC_BAT_DETECT;
	} else {
		return PMIC_BAT_OVER_VOLT;
	}
	
exit:
	return PMIC_ERR_I2C;
}


/*
 * pmic_read_bat_temp: read the battery temperature
 * return:
 *        > 0: Battery temperature
 *        PMIC_ERR_I2C: I2C error
 *        PMIC_ERR_TIMEOUT:		Time out
 */
RC pmic_read_bat_temp(void)
{
	RC result = 0;
	RC ret;
	uchar r_data;
	RC count_timer = 0;
	RC ret_MSB, ret_LSB;

	/*Set 5V scaler and other internal ADC reference */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPADC_CTRL, 0x6B);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_GPADC_CTRL error - ret=%d data=0x%x\n", ret, 0x6B);
		return PMIC_ERR_I2C;
	}

	/*Enable GPADC */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_TOGGLE1, 0x0E);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_TOGGLE1 error - ret=%d data=0x%x\n", ret, 0x0E);
		return PMIC_ERR_I2C;
	}

	/*Select TEMP measurement channel */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPSELECT_ISB, 0x01);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_GPSELECT_ISB error - ret=%d data=0x%x\n", ret, 0x01);
		return PMIC_ERR_I2C;
	}

	/*Start GPADC */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_CTRL_P1, 0x08);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_CTRL_P1 error - ret=%d data=0x%x\n", ret, 0x08);
		return PMIC_ERR_I2C;
	}

	/*Wait for ADC interrupt */
	while (count_timer <= CONST_WAIT_TIME) {
		TMU_Wait_MS(1);

		/* Check ADC intterupt bit */
		ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_INT_STS_B, &r_data);
		if (ret != I2C_SUCCESS) {
			PRINTF("FAIL I2C read PMIC_CTRL_P1 error - ret=%d data=0x%x\n", ret, 0x08);
			return PMIC_ERR_I2C;
		}

		if (0 != (r_data & BIT5)) {
			/* Conversion finished */
			ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_A, MSK_DISABLE);
			if (ret != I2C_SUCCESS) {
				PRINTF("FAIL I2C write PMIC_INT_STS_A error - ret=%d data=0x%x\n", ret, MSK_DISABLE);
				goto disable;
			}
			ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_B, MSK_DISABLE);
			if (ret != I2C_SUCCESS) {
				PRINTF("FAIL I2C write PMIC_INT_STS_B error - ret=%d data=0x%x\n", ret, MSK_DISABLE);
				goto disable;
			}
			ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_INT_STS_C, MSK_DISABLE);
			if (ret != I2C_SUCCESS) {
				PRINTF("FAIL I2C write PMIC_INT_STS_C error - ret=%d data=0x%x\n", ret, MSK_DISABLE);
				goto disable;
			}
			break;
		} else {
			count_timer++;
			/* Do nothing */
		}
	}

	if (CONST_WAIT_TIME < count_timer) {
		/* Time out */
		PRINTF("FAIL time out ## go to disable\n");
		goto disable;
	}

	/*Read the VBAT conversion result */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_GPCH0_MSB, &r_data);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read PMIC_GPCH0_MSB error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	ret_MSB = r_data;
	

	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_GPCH0_LSB,  &r_data);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read PMIC_GPCH0_LSB error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}
	
	ret_LSB = r_data;
	

	/*Correct the result */
	result = ((ret_MSB & 0x0F)<<8) | ret_LSB;

disable:
	/*Disable GPAD */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_TOGGLE1, 0x01);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_TOGGLE1 error - ret=%d data=0x%x\n", ret, 0x01);
		return PMIC_ERR_I2C;
	}

	/*Disable scaler */
	ret = I2C_Write(PMIC_ID2_REG_ADD, PMIC_GPADC_CTRL, 0x00);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write PMIC_GPADC_CTRL error - ret=%d data=0x%x\n", ret, 0x00);
		return PMIC_ERR_I2C;
	}

	if (CONST_WAIT_TIME < count_timer) {
		return PMIC_ERR_TIMEOUT;
	}
	
	PRINTF("pmic_gpadc_correct_temp - %d d", result);
	/* Correct the battery voltage */
	if (0 < result) {
		result = pmic_gpadc_correct_temp(result);
	}
	return result;
}

/*
 * pmic_gpadc_correct_temp: correct the battery temperature
 * @temp: the battery temperature
 * return:
 *        > 0: the battery temperature after correct
 *        PMIC_ERR_I2C: I2C error
 */
RC pmic_gpadc_correct_temp(RC temp)
{
	RC d1, d2;
	RC offset, gain;
	uchar ret_trim1, ret_trim2, ret_trim3, ret_trim4;
	RC sign_trim1, sign_trim2;
	RC ret;
	RC result;

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM1, &ret_trim1);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read PMIC_GPADC_TRIM1 error - ret=%d data=0x%x\n", ret, ret_trim1);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM2, &ret_trim2);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read PMIC_GPADC_TRIM2 error - ret=%d data=0x%x\n", ret, ret_trim2);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM3, &ret_trim3);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read PMIC_GPADC_TRIM3 error - ret=%d data=0x%x\n", ret, ret_trim3);
		return PMIC_ERR_I2C;
	}

	ret = I2C_Read(PMIC_ID3_REG_ADD, PMIC_GPADC_TRIM4, &ret_trim4);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read PMIC_GPADC_TRIM4 error - ret=%d data=0x%x\n", ret, ret_trim4);
		return PMIC_ERR_I2C;
	}

	sign_trim1 = ret_trim1 & BIT0;
	sign_trim2 = ret_trim2 & BIT0;

	ret_trim1 = ret_trim1 & (BIT2 | BIT1);
	ret_trim2 = ret_trim2 & (BIT2 | BIT1);
	ret_trim3 = ret_trim3 & (BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
	ret_trim4 = ret_trim4 & (BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);

	if (0 == sign_trim1) {
		d1 = ret_trim3 * 4 + ret_trim1;
	} else {
		d1 = ret_trim3 * 4 + ret_trim1;
		d1 = -d1;
	}

	if (0 == sign_trim2) {
		d2 = ret_trim4 * 4 + ret_trim2;
	} else {
		d2 = ret_trim4 * 4 + ret_trim2;
		d2 = -d2;
	}

	gain = 1 + bi_div((d2 - d1), (CONST_X2 - CONST_X1));
	offset = d1 - (gain - 1) * CONST_X1;

	result = bi_div((temp - offset), gain);

	return result;
}

/*
 * pmic_set_power_off_resource: change resource state
 * return:
 *        I2C_SUCCESS: 			Successful
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
RC pmic_set_power_off_resource(void)
{
	RC ret = 0;
	/* E_POWER_VANA1_RF */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_SMPS4_CFG_STATE, 0x00);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write SMPS4_CFG_STATE error - ret=%d data=0x%x\n", ret, 0x00);
		return PMIC_ERR_I2C;
	}
	/* E_POWER_VIO_SD */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_LDO1_CFG_STATE, 0x00);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write LDO1_CFG_STATE error - ret=%d data=0x%x\n", ret, 0x00);
		return PMIC_ERR_I2C;
	}
	/* E_POWER_VANA_MM */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_LDO5_CFG_STATE, 0x00);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write LDO5_CFG_STATE error - ret=%d data=0x%x\n", ret, 0x00);
		return PMIC_ERR_I2C;
	}
	/* E_POWER_VUSIM1 */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_LDO7_CFG_STATE, 0x00);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write LDO7_CFG_STATE error - ret=%d data=0x%x\n", ret, 0x00);
		return PMIC_ERR_I2C;
	}
	/* eMMC/SDcard */
	ret = I2C_Write(PMIC_ID1_REG_ADD, PMIC_LDO6_CFG_STATE, 0x00);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write LDO6_CFG_STATE error - ret=%d data=0x%x\n", ret, 0x00);
		return PMIC_ERR_I2C;
	}

	return ret;
}
/*
 * pmic_read_register: read PMIC register
 * return:
 *        Read value
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
uchar pmic_read_register(uchar client, uchar addr)
{
	RC ret;
	uchar r_data;
	ret = I2C_Read(client, addr, &r_data);
	if (I2C_SUCCESS != ret) {
		return PMIC_ERR_I2C;
	}
	return r_data;
}