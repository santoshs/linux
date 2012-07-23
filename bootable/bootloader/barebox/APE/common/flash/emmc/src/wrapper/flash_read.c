/* flash_read.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "string.h"
#include "flash_api.h"
#include "flash_private.h"

extern uchar gTempBuffer[];

static RC flash_Exec_Read(uchar *pBuff, ulong length, uint64 addr_start);

/**
 * Flash_Access_Read - Flash Access Read
 * @return FLASH_ACCESS_SUCCESS       : Successful
 *         FLASH_ACCESS_ERR_NOT_INIT  : Not initialize
 *         FLASH_ACCESS_ERR_NOT_MOUNT : Not mount
 *         FLASH_ACCESS_ERR_PARAM     : Parameter error
 *         FLASH_ACCESS_ERR_READ      : Read error
 */

RC Flash_Access_Read(uchar* pBuff, ulong length, uint64 addr_start, ulong param1)
{
	RC ret = FLASH_ACCESS_ERR_READ;
	
	/* check status */
	ret = flash_Judge_Mount();
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return ret;
	}
	
	/* read */	
	ret = flash_Exec_Read(pBuff, length, addr_start);

	/* read */
	return ret;
}

/**
 * flash_Exec_Read - Read Execution Process
 * @return FLASH_ACCESS_SUCCESS       : Successful
 *         FLASH_ACCESS_ERR_PARAM     : Parameter error
 *         FLASH_ACCESS_ERR_READ      : Read error
 */
RC flash_Exec_Read(uchar *pBuff, ulong length, uint64 addr_start)
{
	RC ret = FLASH_ACCESS_SUCCESS;
	EMMC_PARTITION part = EMMC_PARTITION_USER;
	uint64 start_byte = 0x0000000000000000;
	uint64 remain_length = length;
	ulong start_sector = 0;
	ulong sector_count = 0;
	ulong copy_length = 0;
	ulong offset = 0;
	
	
	/* get partition */
	part = flash_Get_Partition_from_Addr(addr_start);
	
	/* get physical address */
	start_byte = flash_Get_Physical_Addr(addr_start);
	
	/* check erase parameter */
	ret = flash_Check_RW_Param(pBuff, length, start_byte, part);
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	
	/* partition setting(boot partition only) */
	if (part != EMMC_PARTITION_USER)
	{
		if (Emmc_Set_Partition(part) != EMMC_SUCCESS)
		{
			return FLASH_ACCESS_ERR_READ;
		}
	}
	
	/* get sector info */
	start_sector = start_byte >> SECTOR_SHIFT_LENGTH;
	offset = (start_byte & MIDDLE_IN_SECTOR_MASK);
	
	/* check at start_sector */
	if (offset != 0)
	{
		/* read top sector */
		if (Emmc_Read_Single(&(gTempBuffer[0]), start_sector) != EMMC_SUCCESS)
		{
			ret = FLASH_ACCESS_ERR_READ;
			goto EXIT;
		}
		
		/* set copy size */
		if (remain_length < (FLASH_ACCESS_MAX_TEMP_BUFF_SIZE - offset))
		{
			/* size = all data */
			copy_length = remain_length;
		}
		else
		{
			/* size = until the next sector */
			copy_length = FLASH_ACCESS_MAX_TEMP_BUFF_SIZE - offset;
		}
		
		/* copy buffer */
		memcpy(pBuff, &(gTempBuffer[offset]), copy_length);
		remain_length -= copy_length;
		
		/* check all data read */
		if (remain_length == 0)
		{
			/* complete */
			ret = FLASH_ACCESS_SUCCESS;
			goto EXIT;
		}
		pBuff += copy_length;
		start_sector++;
	}
	
	/* multi read */
	sector_count = remain_length >> SECTOR_SHIFT_LENGTH;
	if (sector_count > 1)
	{
		if (Emmc_Read_Multi(pBuff, start_sector, sector_count) != EMMC_SUCCESS)
		{
			ret = FLASH_ACCESS_ERR_READ;
			goto EXIT;
		}
	}
	else if (sector_count == 1)
	{
		if (Emmc_Read_Single(pBuff, start_sector) != EMMC_SUCCESS)
		{
			ret = FLASH_ACCESS_ERR_READ;
			goto EXIT;
		}
	}
	
	copy_length = (sector_count << SECTOR_SHIFT_LENGTH);
	remain_length -= copy_length;
	pBuff += copy_length;
	start_sector += sector_count;
	
	/* check the end of sector */
	if (remain_length > 0)
	{
		/* read sector */
		if (Emmc_Read_Single(&(gTempBuffer[0]), start_sector) != EMMC_SUCCESS)
		{
			ret = FLASH_ACCESS_ERR_READ;
			goto EXIT;
		}
		
		/* copy the remain data */
		copy_length = remain_length;
		memcpy(pBuff, &(gTempBuffer[0]), copy_length);
	}
	
	ret = FLASH_ACCESS_SUCCESS;
	
EXIT:
	/* restore partition settings */
	if (part != EMMC_PARTITION_USER)
	{
		if (Emmc_Set_Partition(EMMC_PARTITION_USER) != EMMC_SUCCESS)
		{
			return FLASH_ACCESS_ERR_READ;
		}
	}
	
	return ret;
}


