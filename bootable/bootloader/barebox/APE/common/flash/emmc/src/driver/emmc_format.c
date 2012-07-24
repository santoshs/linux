/* emmc_format.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "emmc.h"
#include "emmc_private.h"
#include "string.h"

#define EMMC_DUMMY_ERASE 		(0x00000FFF);
#define BUFF_ADDRESS			(0x40000000);
#define BUFF_SIZE				(64*1024);
#define BUFF_SECTOR_SIZE		(0x00000080);

static RC emmc_Boot_Config(void);
static RC emmc_Erase_All(void);

extern EMMC_DRV_INFO gDrv_Info;

/**
 * Emmc_Format - eMMC Format
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_CARD_BUSY     : Card status busy
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 *         EMMC_ERR_PARAM         : Parameter Error
 *         EMMC_ERR_STATE         : State Error
 */
RC Emmc_Format(void)
{
	RC	ret = EMMC_SUCCESS;
	
	/* initialize card (IDLE state --> Transfer state) */
	ret = emmc_Go_Transfer_State();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* clear write protect */
	ret = Emmc_Clear_Protect_For_Format();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* boot config */
	ret = emmc_Boot_Config();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* erase all */
	ret = emmc_Erase_All();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* CMD0 */
	ret = emmc_Go_Idle_State();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	return EMMC_SUCCESS;
	
}

/**
 * emmc_Boot_Config - Set boot config
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_CARD_BUSY     : Card status busy
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 *         EMMC_ERR_PARAM         : Parameter Error
 *         EMMC_ERR_STATE         : State Error
 */
RC emmc_Boot_Config(void)
{
	RC	ret = EMMC_SUCCESS;
	ulong arg = 0;
	
	/* PARTITION_CONFIG (EXT_CSD[179]) */
	arg = 0x03000000 | (179 << 16) | (BOOT_CONFIG_PARTITION_CONFIG << 8);
	ret = emmc_Set_Ext_Csd(arg);
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* BOOT_BUS_WIDTH (EXT_CSD[177]) */
	arg = 0x03000000 | (177 << 16) | (BOOT_CONFIG_BOOT_BUS_WIDTH << 8);
	ret = emmc_Set_Ext_Csd(arg);
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* RST_n_FUNCTION (EXT_CSD[162]) */
	arg = 0x03000000 | (162 << 16) | (BOOT_CONFIG_RST_n_FUNCTION << 8);
	ret = emmc_Set_Ext_Csd(arg);
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	return EMMC_SUCCESS;
}

/**
 * emmc_Erase_All - Erase all data
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_CARD_BUSY     : Card status busy
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 *         EMMC_ERR_PARAM         : Parameter Error
 *         EMMC_ERR_STATE         : State Error
 */
RC emmc_Erase_All(void)
{
	RC	ret = EMMC_SUCCESS;
	ulong i;
	EMMC_PARTITION part = EMMC_PARTITION_USER;
	ulong start_sector = 0;
	ulong sector_count = 0;
	ulong data_size = 0;
	uchar* pReadBuff = (uchar *)BUFF_ADDRESS;
	
	/* Correction by device bug */
	ret = Emmc_Set_Partition(part);
	if( ret != EMMC_SUCCESS )
	{
		return ret;
	}

	/* User area dummy erase */
	sector_count = EMMC_DUMMY_ERASE;
	ret = Emmc_Erase(start_sector, sector_count);
	if( ret != EMMC_SUCCESS )
	{
		return ret;
	}

	/* user area dummy write (64k) */
	data_size = BUFF_SIZE;
	sector_count = BUFF_SECTOR_SIZE;
	memset(pReadBuff,0xff,data_size);

	ret = Emmc_Write_Multi(pReadBuff, start_sector, sector_count);
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}
	
	
	for (i = 0; i < EMMC_PARTITION_MAX; i++)
	{
		switch (i)
		{
		case 0:
			part = EMMC_PARTITION_BOOT1;
			start_sector = 0;
			sector_count = EMMC_MAX_SECTOR_COUNT_BOOT1;
			break;
			
		case 1:
			part = EMMC_PARTITION_BOOT2;
			start_sector = 0;
			sector_count = EMMC_MAX_SECTOR_COUNT_BOOT2;
			break;

		case 2:
			part = EMMC_PARTITION_USER;
			start_sector = 0;
			sector_count = gDrv_Info.sec_count;		/* EXT_CSD[SEC_COUNT] */
			break;
			
		default:
			break;
		}
		
		/* partition setting */
		ret = Emmc_Set_Partition(part);
		if (ret != EMMC_SUCCESS)
		{
			return ret;
		}		
		
		/* execute erase */
		ret = Emmc_Erase(start_sector, sector_count);
	}	

	return EMMC_SUCCESS;

EXIT:
	/* restore partition settings */
	if (part != EMMC_PARTITION_USER)
	{
		if (Emmc_Set_Partition(EMMC_PARTITION_USER) != EMMC_SUCCESS)
		{
			return EMMC_ERR_BUFFER_ACCESS;
		}
	}

	return ret;
}

