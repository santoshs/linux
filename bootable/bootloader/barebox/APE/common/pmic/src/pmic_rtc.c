/*
 * pmic_rtc.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "pmic.h"

/*
 * dec2bcd: Convert decimal number to setting value to RTC register
 * @dec: decimal number
 * return:
 *        uchar: setting value to RTC register
 */

uchar dec2bcd(uchar dec)
{
	return ((dec/10) <<4 ) + (dec%10);
}

/*
 * bcd2dec: Convert value read from RTC register to decimal number
 * @bcd: value read from RTC register
 * return:
 *        uchar: decimal number
 */

uchar bcd2dec(uchar bcd)
{
	return (bcd >> 4) * 10 + (bcd & 0xF);
}

/*
 * pmic_rtc_read_time: Get time from RTC register
 * @tm: rtc_time struture, store the time read from RTC register
 * return:
 *        PMIC_OK     : 		Normal operation
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
 
RC pmic_rtc_read_time(RTC_TIME *tm)
{
	uchar val[7];
	RC ret= PMIC_OK;

	/*Read system time from RTC hardware*/
	ret = block_read(PMIC_ID1_REG_ADD, RTC_SECONDS_REG, 7, val);
	if (ret < 0) {
		PRINTF("FAIL I2C to read time - ret=%d\n", ret);
		goto exit;
	}
	
	/*Convert the register setting value to decimal value*/
	tm->tm_sec  = bcd2dec(val[0] & 0x7F);
	tm->tm_min  = bcd2dec(val[1] & 0x7F);
	tm->tm_hour = bcd2dec(val[2] & 0x3F);
	tm->tm_mday = bcd2dec(val[3] & 0x3F);
	tm->tm_mon  = bcd2dec(val[4] & 0x1F) + 1;
	tm->tm_year = bcd2dec(val[5] & 0xFF);
	tm->tm_wday = bcd2dec(val[6] & 0x07);
	
exit:
	return ret;
}

/*
 * pmic_rtc_set_time: Set time to RTC register
 * @tm: rtc_time struture, store the time to writer to RTC register
 * return:
 *        PMIC_OK     : 		Normal operation
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
 
RC pmic_rtc_set_time(RTC_TIME *tm)
{
	uchar val[7];
	RC ret;
	
	/*Convert decimal value to register setting value*/
	val[0] = dec2bcd(tm->tm_sec);
	val[1] = dec2bcd(tm->tm_min);
	val[2] = dec2bcd(tm->tm_hour);
	val[3] = dec2bcd(tm->tm_mday);
	val[4] = dec2bcd(tm->tm_mon - 1);
	val[5] = dec2bcd(tm->tm_year);
	val[6] = dec2bcd(tm->tm_wday);
	
	/*Stop RTC before setting time*/
	ret = pmic_rtc_stop();
	if (ret < 0)
		goto exit;
	
	ret = block_write(PMIC_ID1_REG_ADD, RTC_SECONDS_REG, 7, val);
	
	if (ret < 0)
		goto exit;
	
	ret = pmic_rtc_start();
	
exit:
	return ret;
}

/*
 * pmic_rtc_start: Start RTC Harware
 * return:
 *        PMIC_OK     : 		Normal operation
 *        PMIC_ERR_I2C:			Error relating to I2C
 */

RC pmic_rtc_start(void)
{
	RC ret;
	uchar reg;
	uchar val;
	
	/*Read control register*/
	ret = I2C_Read(PMIC_ID1_REG_ADD, RTC_CTRL_REG, &reg);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read RTC_CTRL_REG error - ret=%d data=0x%x\n", ret, reg);
		return PMIC_ERR_I2C;
	}
		
	val = (uchar) reg | STOP_RTC;
	
	/*Start RTC*/
	ret = I2C_Write(PMIC_ID1_REG_ADD, RTC_CTRL_REG, val);
	if (ret < 0)
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write RTC_CTRL_REG error - ret=%d data=0x%x\n", ret, val);
		return PMIC_ERR_I2C;
	}
	
	
	return ret;
}


/*
 * pmic_rtc_stop: Stop RTC Hardware
 * return:
 *        PMIC_OK     : 		Normal operation
 *        PMIC_ERR_I2C:			Error relating to I2C
 */

RC pmic_rtc_stop(void)
{
	RC ret;
	uchar val;
	uchar reg;
	
	/*Read control register*/
	ret = I2C_Read(PMIC_ID1_REG_ADD, RTC_CTRL_REG, &reg);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C read RTC_CTRL_REG error - ret=%d data=0x%x\n", ret, reg);
		return PMIC_ERR_I2C;
	}
	val = (uchar) reg & (~STOP_RTC);
	
	/*Stop RTC*/
	ret = I2C_Write(PMIC_ID1_REG_ADD, RTC_CTRL_REG, val);
	if (ret != I2C_SUCCESS) {
		PRINTF("FAIL I2C write RTC_CTRL_REG error - ret=%d data=0x%x\n", ret, val);
		return PMIC_ERR_I2C;
	}
	
	return PMIC_OK;
}

/*
 * pmic_rtc_unlock: Unlock write to RTC registers
 * return: None
 *        
 */
void pmic_rtc_unlock(void)
{
	/* MSECURE */
	*GPIO_PORT0CR = 0x10;
	*GPIO_PORTL031_000DSR = 0x00000001;
}

/*
 * pmic_rtc_lock: Lock write to RTC registers
 * return: None
 *        
 */
void pmic_rtc_lock(void)
{
	/* MSECURE */
	*GPIO_PORT0CR = GPIO_PORT0CR_DATA;
}

/*
 * block_read: Read block of registers by I2C
 * @client: The I2C client slave address.
 * @reg   : The I2C register address.
 * @size  : size of block
 * @val	  : block of value
 * return:
 *        PMIC_OK     : 		Normal operation
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
 
RC block_read(uchar client, RC reg, RC size, uchar *val)
{
	RC i, ret;
	uchar r_data;
	for(i=0; i<size; i++)
	{
		ret = I2C_Read(client, reg + i, &r_data);
		if (ret != I2C_SUCCESS) {
			PRINTF("FAIL block_read error - ret=%d time=%d\n", ret, i);
			return PMIC_ERR_I2C;
		}
		*val = r_data;
		val++;
	}
	return PMIC_OK;
}

/*
 * block_write: Write block of registers by I2C
 * @client: The I2C client slave address.
 * @reg   : The I2C register address.
 * @size  : size of block
 * @val	  : block of value
 * return:
 *        PMIC_OK	  : 		Normal operation
 *        PMIC_ERR_I2C:			Error relating to I2C
 */
 
RC block_write(uchar client, RC reg, RC size, uchar *val)
{
	RC i, ret;

	for(i=0; i<size; i++)
	{
		ret = I2C_Write(client, reg + i, *val);
		if (ret != I2C_SUCCESS) {
			PRINTF("FAIL block_write error - ret=%d time=%d\n", ret, i);
			return PMIC_ERR_I2C;
		}
		val++;
	}
	return PMIC_OK;
}