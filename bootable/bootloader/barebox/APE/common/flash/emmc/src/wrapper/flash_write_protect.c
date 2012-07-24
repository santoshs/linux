/*	flash_writeprot.c									*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.     		*/
/* All rights reserved.									*/
/*														*/

#include "protect_option.h"
#include "flash_api.h"
#include "flash_private.h"

extern RC Emmc_Write_Protect(ulong, ulong, ulong);

/**
 * Flash_Access_Write_Protect - eMMC write protect wrapper. 
 * @return FLASH_ACCESS_SUCCESS           : Success
 *         FLASH_ACCESS_ERR_PARAM         : Parameter error
 *         FLASH_ACCESS_ERR_MOUNT         : Mount error
 *         FLASH_ACCESS_ERR_NOT_INIT      : Initialize error
 *         FLASH_ACCESS_ERR_WRITE_PROTECT : Write Protect Error
 */
RC Flash_Access_Write_Protect(ulong start_group, ulong end_group, ulong user_data)
{
	RC ret = FLASH_ACCESS_SUCCESS;

	ulong max_write_protect_count = 0;
	ulong start_sector = 0;
	ulong end_sector = 0;

	/* check status */
	ret = flash_Judge_Mount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}

	if(user_data != ALL_USER_DATA_PROTECT)
	{
		max_write_protect_count = flash_Get_Max_Protect_Group_Count();

		if(max_write_protect_count == 0)
		{
			ret = FLASH_ACCESS_ERR_PARAM;
		}

		if(start_group > end_group)
		{
			ret = FLASH_ACCESS_ERR_PARAM;
		}

		if(end_group >= max_write_protect_count)
		{
			ret = FLASH_ACCESS_ERR_PARAM;
		}
	}

	if(ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}

	start_sector = flash_Get_Sector_From_WPGroup(start_group);
	end_sector = (flash_Get_Sector_From_WPGroup(end_group + 1) - 1);

	ret = Emmc_Write_Protect( start_sector , end_sector, user_data );

	if(ret == EMMC_SUCCESS)
	{
		ret = FLASH_ACCESS_SUCCESS;
	}
	else if(ret == EMMC_ERR_PARAM)
	{
		ret = FLASH_ACCESS_ERR_PARAM;
	}
	else
	{
		ret = FLASH_ACCESS_ERR_WRITE_PROTECT;
	}

	return ret;
}