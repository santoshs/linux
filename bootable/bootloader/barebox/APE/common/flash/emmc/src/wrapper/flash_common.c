/* flash_common.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flash_api.h"
#include "flash_private.h"


/*****************************************************************************
; Global Data Declaration
******************************************************************************/
FLASH_ACCESS_STATE gFlashStatus = FLASH_ACCESS_STATE_START;
uchar gTempBuffer[FLASH_ACCESS_MAX_TEMP_BUFF_SIZE] = { 0 };


/**
 * flash_Judge_Start - Judge status start
 * @return TRUE  : START state
 *         FALSE : Not START state
 */
RC flash_Judge_Start(void)
{
	RC ret;
	
	/* status check */
	if (gFlashStatus == FLASH_ACCESS_STATE_START)
	{
		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}
	
	return ret;
}


/**
 * flash_Judge_Unmount - Judge status unmount
 * @return FLASH_ACCESS_SUCCESS           : UNMOUNT state
 *         FLASH_ACCESS_ERR_ALREADY_MOUNT : MOUNT state
 *         FLASH_ACCESS_ERR_NOT_INIT      : other state
 */
RC flash_Judge_Unmount(void)
{
	RC ret = FLASH_ACCESS_SUCCESS;
	
	/* status check */
	switch (gFlashStatus)
	{
	case FLASH_ACCESS_STATE_UNMOUNT:
		ret = FLASH_ACCESS_SUCCESS;
		break;
		
	case FLASH_ACCESS_STATE_MOUNT:
		ret = FLASH_ACCESS_ERR_ALREADY_MOUNT;
		break;
		
	default:
		ret = FLASH_ACCESS_ERR_NOT_INIT;
		break;
	}
	
	return ret;
}


/**
 * flash_Judge_Mount - Judge status mount
 * @return FLASH_ACCESS_SUCCESS       : MOUNT state
 *         FLASH_ACCESS_ERR_NOT_MOUNT : UNMOUNT state
 *         FLASH_ACCESS_ERR_NOT_INIT  : other state
 */
RC flash_Judge_Mount(void)
{
	RC ret = FLASH_ACCESS_SUCCESS;
	
	/* status check */
	switch (gFlashStatus)
	{
	case FLASH_ACCESS_STATE_MOUNT:
		ret = FLASH_ACCESS_SUCCESS;
		break;
		
	case FLASH_ACCESS_STATE_UNMOUNT:
		ret = FLASH_ACCESS_ERR_NOT_MOUNT;
		break;
		
	default:
		ret = FLASH_ACCESS_ERR_NOT_INIT;
		break;
	}
	
	return ret;
}


/**
 * flash_Set_Status - set status
 * @return None
 */
void flash_Set_Status(FLASH_ACCESS_STATE status)
{
	/* update status */
	gFlashStatus = status;
}


/**
 * flash_Check_RW_Param - Check read/write parameter
 * @return FLASH_ACCESS_SUCCESS   : Successful
 *         FLASH_ACCESS_ERR_PARAM : Parameter error
 */
RC flash_Check_RW_Param(uchar *pBuff, ulong length, uint64 physical, EMMC_PARTITION part)
{
#if 0	/* Don't check MAX */
	uint64 physical_limit = 0;
#endif	/* Don't check MAX */
	
	/* check NULL */
	if (pBuff == NULL)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	if (length == 0)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}

	/* illegal partition */
	if (part == EMMC_PARTITION_MAX)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}

#if 0	/* Don't check MAX */
	/* address limit over */
	switch (part)
	{
	case EMMC_PARTITION_USER:
		physical_limit = PHYSICAL_MAX_ADDR_USER;
		break;
		
	case EMMC_PARTITION_BOOT1:
		physical_limit = PHYSICAL_MAX_ADDR_BOOT1 ;
		break;
		
	case EMMC_PARTITION_BOOT2:
		physical_limit = PHYSICAL_MAX_ADDR_BOOT2;
		break;
		
	default:
		/* illegal partition */
		return FLASH_ACCESS_ERR_PARAM;
	}
	if (physical > physical_limit)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
	if ((physical + length - 1) > physical_limit)
	{
		return FLASH_ACCESS_ERR_PARAM;
	}
#endif	/* Don't check MAX */	
	
	return FLASH_ACCESS_SUCCESS;
}


/**
 * flash_Get_Partition_from_Blk - Get partition from block
 * @return EMMC_PARTITION_USER  : User data partition
 *         EMMC_PARTITION_BOOT1 : Boot partition1
 *         EMMC_PARTITION_BOOT2 : Boot partition2
 *         EMMC_PARTITION_MAX   : Parameter invalid
 */
EMMC_PARTITION flash_Get_Partition_from_Blk(ulong block)
{
	EMMC_PARTITION part = EMMC_PARTITION_MAX;
	uchar work;
	
	/* get bit[31-28] */
	work = block >> 28;
	
	/* get partition */
	switch (work)
	{
	case 0x00:
		part = EMMC_PARTITION_USER;
		break;
		
	case 0x0F:
		part = EMMC_PARTITION_BOOT1;
		break;
		
	case 0x0E:
		part = EMMC_PARTITION_BOOT2;
		break;
		
	default:
		break;
	}
	
	return part;
}


/**
 * flash_Get_Partition_from_Addr - Get partition from logical address
 * @return EMMC_PARTITION_USER  : User data partition
 *         EMMC_PARTITION_BOOT1 : Boot partition1
 *         EMMC_PARTITION_BOOT2 : Boot partition2
 *         EMMC_PARTITION_MAX   : Parameter invalid
 */
EMMC_PARTITION flash_Get_Partition_from_Addr(uint64 logical)
{
	EMMC_PARTITION part = EMMC_PARTITION_MAX;
	uchar work;
	
	/* get bit[63-60] */
	work = logical >> 60;
	
	/* get partition */
	switch (work)
	{
	case 0x00:
		part = EMMC_PARTITION_USER;
		break;
		
	case 0x0F:
		part = EMMC_PARTITION_BOOT1;
		break;
		
	case 0x0E:
		part = EMMC_PARTITION_BOOT2;
		break;
		
	default:
		break;
	}
	
	return part;
}


/**
 * flash_Get_Sector - Get sector from block
 * @return ulong : Sector address
 */
ulong flash_Get_Sector(ulong block, uint64 address, ulong data_size)
{
	ulong sector = 0;

	if(block) {
		/* get convert block into sector */
		sector = (block & 0x0FFFFFFFL) << BLOCK_TO_SECTOR_SHIFT;
	} else if(address) {
		typedef union
		{
			ulong  data32[2];
			uint64 data64;
		} Bit64_32;
		
		Bit64_32 uAddress, uLogical;
		uLogical.data64 = address;
		uAddress.data32[0] = uLogical.data32[0] & 0xFFFFFFFF;
		uAddress.data32[1] = uLogical.data32[1] & 0x0FFFFFFF;
		/* get convert address into sector */
		sector = (uAddress.data64 >> ADDRESS_TO_SECTOR_SHIFT);
	} else if(data_size) {
		/* get convert data size into sector */
		sector = (data_size >> ADDRESS_TO_SECTOR_SHIFT);
	} else {
		/* Do nothing; */
	}
	
	return sector;
}


/**
 * flash_Get_Physical_Addr - Get physical address from logical address
 * @return uint64 : Physical address
 */
uint64 flash_Get_Physical_Addr(uint64 logical)
{
	typedef union
	{
		ulong  data32[2];
		uint64 data64;
	} Bit64_32;

	Bit64_32 uPhysical, uLogical;
	uLogical.data64 = logical;

	/* get physical address */
	uPhysical.data32[0] = uLogical.data32[0] & 0xFFFFFFFF;
	uPhysical.data32[1] = uLogical.data32[1] & 0x0FFFFFFF;
	
	return uPhysical.data64;
}
