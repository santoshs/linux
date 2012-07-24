/* emmc_read.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "emmc.h"
#include "emmc_private.h"


extern EMMC_DRV_INFO gDrv_Info;

/**
 * Emmc_Read_Multi - eMMC Read Multi Sector
 * @return EMMC_SUCCESS       : Success
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
RC Emmc_Read_Multi(uchar* pBuff, ulong start_sector, ulong sector_count)
{
	RC	ret = EMMC_SUCCESS;
	
	/* parameter check */
	ret = emmc_Check_Param_Multi(pBuff, start_sector, sector_count);
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* CMD23 */
	emmc_Make_Cmd(23, sector_count & 0x0000FFFF, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	/* CMD18 */
	if (gDrv_Info.access_mode == EMMC_ACCESS_MODE_SECTOR)
	{
		emmc_Make_Cmd(18, start_sector, (ulong *)pBuff, sector_count << EMMC_SECTOR_SIZE_SHIFT);
	}
	else
	{
		emmc_Make_Cmd(18, start_sector << EMMC_SECTOR_SIZE_SHIFT, (ulong *)pBuff, sector_count << EMMC_SECTOR_SIZE_SHIFT);
	}

	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	/* CMD13 */
	emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	if (!(gDrv_Info.r1_card_status & EMMC_R1_READY))
	{
		return EMMC_ERR_CARD_BUSY;
	}

	if (gDrv_Info.current_state != EMMC_R1_STATE_TRAN)
	{
		return EMMC_ERR_STATE;
	}
		
	return EMMC_SUCCESS;

EXIT:
	if (gDrv_Info.during_transfer)
	{
		emmc_Error_Reset();
		if (emmc_Send_Stop())
			goto EXIT;
	}
	else
	{
		emmc_Error_Reset();
	}

	return ret;
}

/**
 * Emmc_Read_Single - eMMC Read Single Sector
 * @return EMMC_SUCCESS       : Success
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
RC Emmc_Read_Single(uchar* pBuff, ulong start_sector)
{
	RC	ret = EMMC_SUCCESS;
	
	/* parameter check */
	ret = emmc_Check_Param_Single(pBuff, start_sector);
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* CMD17 */
	if (gDrv_Info.access_mode == EMMC_ACCESS_MODE_SECTOR)
		emmc_Make_Cmd(17, start_sector, (ulong *)pBuff, EMMC_BLOCK_LENGTH);
	else
		emmc_Make_Cmd(17, start_sector << EMMC_SECTOR_SIZE_SHIFT, (ulong *)pBuff, EMMC_BLOCK_LENGTH);

	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	/* CMD13 */
	emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	if (!(gDrv_Info.r1_card_status & EMMC_R1_READY))
	{
		return EMMC_ERR_CARD_BUSY;
	}

	if (gDrv_Info.current_state != EMMC_R1_STATE_TRAN)
	{
		return EMMC_ERR_STATE;
	}

	return EMMC_SUCCESS;

EXIT:
	if (gDrv_Info.during_transfer)
	{
		emmc_Error_Reset();
		if (emmc_Send_Stop())
			goto EXIT;
	}
	else
	{
		emmc_Error_Reset();
	}

	return ret;
}

