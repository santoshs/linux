/* flash_init.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flash_api.h"
#include "flash_private.h"

/**
 * Flash_Access_Init - Flash Access Initialize
 * @return FLASH_ACCESS_SUCCESS  : Successful
 *         FLASH_ACCESS_ERR_INIT : Initialize error
 */
RC Flash_Access_Init(ulong param1)
{
	RC ret = FLASH_ACCESS_ERR_INIT;
	
	/* check already init */
	if (flash_Judge_Start() == FALSE)
	{
		return FLASH_ACCESS_SUCCESS;
	}
	
	/* execute initialize */
	if (Emmc_Init() == EMMC_SUCCESS) 
	{
		flash_Set_Status(FLASH_ACCESS_STATE_UNMOUNT);
		ret = FLASH_ACCESS_SUCCESS;
	}
	
	return ret;
}

