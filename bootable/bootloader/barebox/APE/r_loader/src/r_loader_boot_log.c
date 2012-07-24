/*
 * r_loader_boot_log.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "r_loader.h"
#include "pmic.h"
#include "r_loader_boot_log.h"
#include "r_loader_boot_matrix.h"
#include "log_output.h"
#include "usb_api.h"
#include "string.h"
#include "flash_api.h"
#include "ths_api.h"

/*
 * Definition 
 */
 
/* Boot log */
extern BOOT_MATRIX matrix_info;
BOOT_LOG log_info;

/*
 * Function implementation
 */

/*
 * Save_Boot_Log_To_eMMC: Save BOOT_LOG to eMMC
 * return:
 *        R_LOADER_ERR_FLASH			Error in accesing Flash memory
 *        PMIC_ERR_I2C					Error in accessing I2C
 *        > 0							Offset of new boot log entry
 */
RC Save_Boot_Log_To_eMMC(ulong line_num, RC return_val, uchar branch_mode)
{
	RC ret;
	ulong num, offset;
	uint64 log_pos = 0;
	RTC_TIME timer;
	
	/* Clear boot log */
	boot_log_clear();
	
	/* Copy boot matrix */
	memcpy((void *)&(log_info.boot_matrix), (void*)&matrix_info, sizeof(BOOT_MATRIX));
	
	/* Update branch mode */
	log_info.branching_mode = branch_mode; 
	
	/* Update separator */
	log_info.header = 0x5B;		/* "[" */
	log_info.separator = 0x5D;	/* "]" */
	
	/* Update progress and returned value */
	log_info.progress_info = line_num;
	log_info.return_value = return_val;
	
	/* Get time */
	ret = pmic_rtc_read_time(&timer);
	if (PMIC_OK != ret) {
		PRINTF("FAIL RTC to read time - ret=%d\n", ret);
		return ret;
	}
	
	log_info.year = timer.tm_year;
	log_info.month = timer.tm_mon;
	log_info.day = timer.tm_mday;
	log_info.hour = timer.tm_hour;
	log_info.minute = timer.tm_min;
	log_info.second = timer.tm_sec;
	
	/* Get STBCHRB0-1 */
	log_info.stbchrb0 = *STBCHRB0;
	log_info.stbchrb1 = *STBCHRB1;
	
	/* Get STBCHR0-3 */
	log_info.stbchr0 = *STBCHR0;
	log_info.stbchr1 = *STBCHR1;
	log_info.stbchr2 = *STBCHR2;
	log_info.stbchr3 = *STBCHR3;
	
	/* Read number of log entry */
	/* The number of log entry can by reset to 0 by flashing boot_log_num_zero.bin to eMMC */
#ifndef EMMC_NO_WRITE
	ret = Flash_Access_Read((uchar *)(&num), sizeof(ulong), (uint64)(LOG_NUM_OFFSET), UNUSED);
	if (FLASH_ACCESS_SUCCESS != ret) {
		PRINTF("FAIL read number of log entry at 0x%x - ret=%d\n", (ulong)LOG_NUM_OFFSET, ret);
		return R_LOADER_ERR_FLASH;
	}
#endif //EMMC_NO_WRITE
	offset = num; 
	
	/* Reset */
	if (0 > num) {
		num = 0;
		offset = 0;
	} 
	
	/* Ring buffer, reset position if exceed MAX_LOG_NUM */
	if (MAX_LOG_NUM < offset) {
		offset = div_mode(offset, MAX_LOG_NUM);
	}
	
	log_pos = LOG_START_OFFSET + (offset * sizeof(BOOT_LOG));
	
#ifndef EMMC_NO_WRITE
	/* Write boot log entry */
	ret = Flash_Access_Write((uchar *)(&log_info), sizeof(BOOT_LOG), (uint64)(log_pos), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		PRINTF("FAIL write log entry to 0x%x - ret=0x%x; num_ent=%d\n", (ulong)log_pos, ret, num);
		return R_LOADER_ERR_FLASH;
	}
#endif //EMMC_NO_WRITE
	
#ifdef _R_LOADER_BOOT_LOG_WRITE_CHECK_
	/* Read boot log entry for checking */
	BOOT_LOG check_log;
#ifndef EMMC_NO_WRITE
	ret = Flash_Access_Read((uchar *)(&check_log), sizeof(BOOT_LOG), (uint64)(log_pos), UNUSED);
	if (FLASH_ACCESS_SUCCESS != ret) {
		PRINTF("FAIL read boot log for checking at 0x%x - ret=%d\n", (ulong)log_pos, ret);
		return R_LOADER_ERR_FLASH;
	}
#endif //EMMC_NO_WRITE

	uchar item_write;
	uchar item_read;
	ulong index = 0;
	PRINTF("Check writen boot log\n");
	while (index < sizeof(BOOT_LOG)) {
		item_write = ((uchar *)(&log_info))[index];
		item_read = ((uchar *)(&check_log))[index];
		if ((item_write & (~item_read)) != 0) {
			PRINTF("FAIL compare[%d] write[%x] read[%x]\n", (ulong)index, (uchar)item_write, (uchar)item_read);
		}
		index++;
	}
#endif
	
	/* Update number of log entry */
	num++;
	
	/* Write number of log entry */
#ifndef EMMC_NO_WRITE
	ret = Flash_Access_Write((uchar *)(&num), sizeof(ulong), (uint64)(LOG_NUM_OFFSET), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		PRINTF("FAIL write number of entry to 0x%x - ret=0x%x\n", LOG_NUM_OFFSET, ret);
		return R_LOADER_ERR_FLASH;
	}
#endif //EMMC_NO_WRITE
	
	/* Print boot log info */
	PRINTF("Boot Log Info [%d/%d/%d - %d:%d:%d]\n", log_info.month, log_info.day, log_info.year, log_info.hour, log_info.minute, log_info.second);
	PRINTF("   .[Boot matrix is printed aboved]\n");
	PRINTF("   .Progress info=%d\n", log_info.progress_info);
	PRINTF("   .Return value=%d\n", log_info.return_value);
	PRINTF("   .Branching mode=%d\n", log_info.branching_mode);
	PRINTF("   .[R-MU2]STBCHRB0=0x%X\n", log_info.stbchrb0);
	PRINTF("   .[R-MU2]STBCHRB1=0x%X\n", log_info.stbchrb1);
	PRINTF("   .[R-MU2]STBCHR0=0x%X\n", log_info.stbchr0);
	PRINTF("   .[R-MU2]STBCHR1=0x%X\n", log_info.stbchr1);
	PRINTF("   .[R-MU2]STBCHR2=0x%X\n", log_info.stbchr2);
	PRINTF("   .[R-MU2]STBCHR3=0x%X\n", log_info.stbchr3);
	PRINTF("Save boot log %d at line %d with return val %d\n", (ulong)offset, (ulong)line_num, (RC)return_val);
	
	return offset;
}

/*
 * boot_log_get: get the content of log_info
 * return:
 *        BOOT_MATRIX(struct):				Content of log_info
 */
BOOT_LOG boot_log_get(void)
{
	return log_info;
}

/*
 * boot_log_clear: clear content of log_info
 * return: none
 */
void boot_log_clear(void)
{
	memset((void *)&log_info, 0x00, sizeof(BOOT_LOG));
	return;
}


