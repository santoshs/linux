/* flash_mount.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flash_api.h"
#include "flash_private.h"


/**
 * Flash_Access_Mount - Flash Access Mount
 * @return FLASH_ACCESS_SUCCESS           : Successful
 *         FLASH_ACCESS_ERR_NOT_INIT      : Not initialize
 *         FLASH_ACCESS_ERR_MOUNT         : Mount failed
 *         FLASH_ACCESS_ERR_ALREADY_MOUNT : Already mount
 */
RC Flash_Access_Mount(ulong param1)
{
	RC ret = FLASH_ACCESS_ERR_MOUNT;

	/* check status */
	ret = flash_Judge_Unmount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}
	
	/* mount */
	if (Emmc_Mount() != EMMC_SUCCESS)
	{
		return FLASH_ACCESS_ERR_MOUNT;
	}
	
	/* partition settings initialize */
	if (Emmc_Set_Partition(EMMC_PARTITION_USER) != EMMC_SUCCESS)
	{
		return FLASH_ACCESS_ERR_MOUNT;
	}
	
	/* update status */
	flash_Set_Status(FLASH_ACCESS_STATE_MOUNT);
	
	return FLASH_ACCESS_SUCCESS;
}

