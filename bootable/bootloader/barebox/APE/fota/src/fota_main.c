/*	fota_main.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "fota.h"
#include "flash_api.h"

static const char MODE_STRING_FOTA[]	= {"fota"}; 

/**
 * fota_main
 * @return  - none
 */
void fota_main(void)
{
	RC ret;
	PRINTF("FOTA mode\n");
	
	ret = fota_check_nvm();
	if (FOTA_OK != ret) {
		PRINTF("FAIL FOTA can not check or erase NVM flag - ret=%d\n", ret);
	}
	
	while(1)
	{
		/* Wait 3s*/
		PRINTF("Wait 3s...\n");
		TMU_Wait_MS(WAIT_TIME);
	};
	
	/* this code can't be reached */
	while(1);

}

/**
 * fota_check_nvm - Check and clear NVM flag
 * @return None
 */
RC fota_check_nvm(void)
{
	int ret;
	uchar nvm_flag[BOOTFLAG_SIZE];
	
	/* flash initialize */
	Flash_Access_Init( UNUSED );
	
	/* flash memory mount */
	ret = Flash_Access_Mount(UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		return ret;
	}
	
	/* flash read */
	ret = Flash_Access_Read((uchar *)(&nvm_flag), (ulong)(BOOTFLAG_SIZE), (uint64)(BOOTFLAG_ADDR), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		return ret;
	}
	
	/* check the boot flag */
	if (strncmp(((const char *)(nvm_flag)), MODE_STRING_FOTA, BOOTFLAG_SIZE) == 0) {
		/* erase NVM boot flag */
		memset((void *)&nvm_flag, 0x00, BOOTFLAG_SIZE);
		ret = Flash_Access_Write((uchar *)(&nvm_flag), (ulong)(BOOTFLAG_SIZE), (uint64)(BOOTFLAG_ADDR), UNUSED);
		if (ret != FLASH_ACCESS_SUCCESS) {
			return ret;
		}
		PRINTF("FOTA erases NVM flag\n");
	}
	
	/* flash memory unmount */
	ret = Flash_Access_Unmount( UNUSED );
	if (ret != FLASH_ACCESS_SUCCESS) {
		return ret;
	}
	
	return FOTA_OK;
}
