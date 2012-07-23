/* emmc_erase.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "emmc.h"
#include "emmc_private.h"


extern EMMC_DRV_INFO gDrv_Info;

static RC emmc_Exec_Erase(ulong start_sector, ulong sector_count);

/**
 * Emmc_Erase - eMMC Erase
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
RC Emmc_Erase(ulong start_sector, ulong sector_count)
{
	RC	ret = EMMC_SUCCESS;
	ulong start, count, remain;
	ulong unit = (gDrv_Info.access_mode == EMMC_ACCESS_MODE_SECTOR) ? 1 : EMMC_BLOCK_LENGTH;
	
	/* parameter check */
	if (sector_count < 1)
	{
		return EMMC_ERR_PARAM;
	}
	
#if 0	/* Don't check MAX */
	if ((start_sector + sector_count - 1) > EMMC_END_SECTOR_USER)
	{
		return EMMC_ERR_PARAM;
	}
#endif	/* Don't check MAX */
	
	/* execute erase */
	start = start_sector * unit;
	count = sector_count * unit;
	remain = count;
	while (remain > 0)
	{
		/* Erase size at the once <= 32MB */
		count = remain > (0x00010000L * unit) ? (0x00010000L * unit) : remain;
		
		ret = emmc_Exec_Erase(start, count);
		if (ret != EMMC_SUCCESS)
		{
			return ret;
		}
		
		remain -= count;
		start += count;
	}
	
	return ret;
}

/**
 * emmc_Exec_Erase - eMMC Erase Execution Process
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
RC emmc_Exec_Erase(ulong start_sector, ulong sector_count)
{
	RC	ret = EMMC_SUCCESS;
	
	
	/* CMD35 */
	emmc_Make_Cmd(35, start_sector, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	/* CMD36 */
	emmc_Make_Cmd(36, (start_sector + sector_count - 1), NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	/* CMD38 */
	emmc_Make_Cmd(38, 0x00000000L, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}
	
	/* wait Ready state */
	do
	{
		/* CMD13 */
		emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
		ret = emmc_Issue_Cmd();
		if (ret != EMMC_SUCCESS)
		{
			goto EXIT;
		}

	} while (!(gDrv_Info.r1_card_status & EMMC_R1_READY));

	return EMMC_SUCCESS;
	

EXIT:
	emmc_Error_Reset();
	
	return ret;
}

