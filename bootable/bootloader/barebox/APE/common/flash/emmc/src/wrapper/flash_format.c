/* flash_format.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flash_api.h"
#include "flash_private.h"

/**
 * Flash_Access_Format - Flash Access Format
 * @return FLASH_ACCESS_SUCCESS           : Successful
 *         FLASH_ACCESS_ERR_NOT_INIT      : Not initialize
 *         FLASH_ACCESS_ERR_ALREADY_MOUNT : Already mount
 *         FLASH_ACCESS_ERR_FORMAT        : Format error
 */
RC Flash_Access_Format(ulong param1)
{
	RC ret = FLASH_ACCESS_ERR_FORMAT;
	
	/* check status */
	ret = flash_Judge_Unmount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}
	
	/* format */
	if (Emmc_Format() != EMMC_SUCCESS)
	{
		return FLASH_ACCESS_ERR_FORMAT;
	}
	
	return FLASH_ACCESS_SUCCESS;
}

