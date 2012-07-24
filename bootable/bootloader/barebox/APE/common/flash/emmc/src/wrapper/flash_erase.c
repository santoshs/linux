/* flash_erase.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flash_api.h"
#include "flash_private.h"

static RC flash_Check_Erase_Param(EMMC_PARTITION start_part, 
                                  EMMC_PARTITION end_part, 
                                  ulong start_sector, 
                                  ulong end_sector);
extern unsigned long Emmc_Get_Max_Sector_Count(void);

/**
 * Flash_Access_Erase - Flash Access Erase
 * @return FLASH_ACCESS_SUCCESS       : Successful
 *         FLASH_ACCESS_ERR_NOT_INIT  : Not initialize
 *         FLASH_ACCESS_ERR_NOT_MOUNT : Not mount
 *         FLASH_ACCESS_ERR_ERASE     : Erase error
 *         FLASH_ACCESS_ERR_PARAM     : Parameter error
 */
RC Flash_Access_Erase(ulong start_block, ulong end_block, ulong param1)
{
	RC ret = FLASH_ACCESS_SUCCESS;
	EMMC_PARTITION start_part = EMMC_PARTITION_USER;
	EMMC_PARTITION end_part = EMMC_PARTITION_USER;
	ulong start_sector = 0;
	ulong end_sector = 0;
	ulong sector_count = 0;
	
	/* check status */
	ret = flash_Judge_Mount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}
	
	/* get partition */
	start_part = flash_Get_Partition_from_Blk(start_block);
	end_part = flash_Get_Partition_from_Blk(end_block);
	
	/* get sector */
	start_sector = flash_Get_Sector(start_block, 0, 0);
	end_sector = flash_Get_Sector(end_block, 0, 0);
	sector_count = end_sector - start_sector + 1;
	
	/* check erase parameter */
	ret = flash_Check_Erase_Param(start_part, end_part, start_sector, end_sector);
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	
	/* partition setting(boot partition only) */
	if (start_part != EMMC_PARTITION_USER)
	{
		if (Emmc_Set_Partition(start_part) != EMMC_SUCCESS)
		{
			return FLASH_ACCESS_ERR_ERASE;
		}
	}
	
	/* execute erase */
	ret = Emmc_Erase(start_sector, sector_count);
	if (ret != EMMC_SUCCESS)
	{
		ret = FLASH_ACCESS_ERR_ERASE;
	}
	else
	{
		ret = FLASH_ACCESS_SUCCESS;
	}
	
	/* restore partition settings */
	if (start_part != EMMC_PARTITION_USER)
	{
		if (Emmc_Set_Partition(EMMC_PARTITION_USER) != EMMC_SUCCESS)
		{
			return FLASH_ACCESS_ERR_ERASE;
		}
	}
	
	return ret;
}


/**
 * flash_Check_Erase_Param - Check erase parameter
 * @return FLASH_ACCESS_SUCCESS       : Successful
 *         FLASH_ACCESS_ERR_PARAM     : Parameter error
 */
RC flash_Check_Erase_Param(EMMC_PARTITION start_part, 
                           EMMC_PARTITION end_part, 
	                       ulong start_sector, 
	                       ulong end_sector)
{
#if 0	/* Don't check MAX */
	ulong sector_limit = 0;
#endif	/* Don't check MAX */
	
	/* illegal partition */
	if (start_part == EMMC_PARTITION_MAX)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	
	/* different partition */
	if (start_part != end_part)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	
	/* reverse start-end */
	if ((start_sector > end_sector) || (Emmc_Get_Max_Sector_Count() < end_sector))
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	
#if 0	/* Don't check MAX */	
	/* sector limit over */
	switch (start_part)
	{
	case EMMC_PARTITION_USER:
		sector_limit = SECTOR_MAX_ADDR_USER;
		break;
		
	case EMMC_PARTITION_BOOT1:
		sector_limit = SECTOR_MAX_ADDR_BOOT1 ;
		break;
		
	case EMMC_PARTITION_BOOT2:
		sector_limit = SECTOR_MAX_ADDR_BOOT2;
		break;
		
	default:
		/* illegal partition */
		return FLASH_ACCESS_ERR_PARAM;
	}
	if (end_sector > sector_limit)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	if (start_sector > sector_limit)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
#endif	/* Don't check MAX */
	
	return FLASH_ACCESS_SUCCESS;
}
