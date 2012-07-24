/* flash_unmount.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flash_api.h"
#include "flash_private.h"
#include "emmc.h"

/**
 * Flash_Access_Unmount - Flash Access Unmount
 * @return FLASH_ACCESS_SUCCESS           : Successful
 *         FLASH_ACCESS_ERR_NOT_INIT      : Not initialize
 *         FLASH_ACCESS_ERR_NOT_MOUNT     : Not mount
 */
RC Flash_Access_Unmount(ulong param1)
{
	RC ret = FLASH_ACCESS_SUCCESS;
	
	/* check status */
	ret = flash_Judge_Mount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}
	
	/* unmount */
	Emmc_Unmount();
	
	/* update status */
	flash_Set_Status(FLASH_ACCESS_STATE_UNMOUNT);
	
	
	return FLASH_ACCESS_SUCCESS;
}

