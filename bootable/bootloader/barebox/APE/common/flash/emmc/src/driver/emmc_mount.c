/* emmc_mount.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "emmc.h"
#include "emmc_private.h"


extern EMMC_DRV_INFO gDrv_Info;


/**
 * Emmc_Mount - eMMC mount
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
 *         EMMC_ERR_ILLEGAL_CARD  : Illegal card
 *         EMMC_ERR_HIGH_SPEED    : High Speed Mode Error
 *         EMMC_ERR_BUSWIDTH      : Bus Width Setting Error
 */
RC Emmc_Mount(void)
{
	RC	ret = EMMC_ERR_BUSWIDTH;
	ulong j = 0;
	uchar rcvData[8];
	ulong *pSend;
	ulong *pRcv;
	RC	retCMD19;
	RC	retCMD14;
	
	/* 8bit check pattern */
	uchar pattern[] = {0x55, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	ulong patternSize = 8;
	uchar *pCheck = NULL;
	uchar check_ref[] = {0xaa,0x55,0x00,0x00,0x00,0x00,0x00,0x00};
	
	/* state check */
	if (GET_REG32(CE_HOST_STS1) & CE_HOST_STS1_CMDSEQ)
	{
		return EMMC_ERR_CARD_BUSY;
	}

	/* initialize card (IDLE state --> Transfer state) */
	ret = emmc_Go_Transfer_State();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}
	
	/* bus width (1bit --> 8bit) */
	gDrv_Info.bus_width = EMMC_BUSWIDTH_8BIT;
	pSend = (ulong *)&(pattern[0]);
	pRcv = (ulong *)&(rcvData[0]);

	/* CMD19 */
	emmc_Make_Cmd(19, 0x00000000, pSend, patternSize);
	retCMD19 = emmc_Issue_Cmd();
	if (retCMD19)
	{
		/* Don't goto EXIT. if currect state is btst, send CMD14 */
		emmc_Error_Reset();
	}
	
	/* CMD13 */
	emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}
	
	if (gDrv_Info.current_state != EMMC_R1_STATE_BTST) 
	{
		ret = EMMC_ERR_STATE;
		goto EXIT;
	}
	
	/* CMD14 */
	emmc_Make_Cmd(14, 0x00000000, pRcv, patternSize);
	retCMD14 = emmc_Issue_Cmd();
	if (retCMD14)
	{
		/* Don't goto EXIT */
		emmc_Error_Reset();
	}

	/* CMD13 */
	emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	if (gDrv_Info.current_state != EMMC_R1_STATE_TRAN) 
	{
		/* Current state is btst. cannot send next cmd. change state btst --> idle (by CMD0) */
		ret = EMMC_ERR_STATE;
		goto EXIT;
	}
	
	/* check error (CMD19 or CMD14) */ 
	if (retCMD14 || retCMD19)
	{
		/* The card transmit Data, but MMCIF could not receive Data(rcvData[]=invalid). */
		ret = EMMC_ERR_BUSWIDTH;
		goto EXIT;
	}
	
	/* check patarn */
	pCheck = &(check_ref[0]);
	for(j = patternSize; j > 0; j--, pCheck++)
	{
		if(rcvData[8 - j] != *pCheck)
		{
			ret = EMMC_ERR_BUSWIDTH;
			goto EXIT;
		}
	}
	
	/* CMD6 */
	emmc_Make_Cmd(6, EMMC_SWITCH_BUS_WIDTH_1 | (EMMC_BUSWIDTH_8BIT << 8), NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		if ((GET_REG32(CE_INT) & CE_INT_ALL_ERROR) == CE_INT_RBSYTO)
		{
			/* Response Busy timeout */
			emmc_Error_Reset();

			/* CMD13 wait ready */
			ret = emmc_Wait_Ready_For_Data();
			if (ret != EMMC_SUCCESS)
			{
				goto EXIT;
			}
			
		}
		else
		{
			/* occurred error */
			goto EXIT;
		}
	}
	
	/* CMD13 */
	emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	/* CMD8 (EXT_CSD) */
	emmc_Make_Cmd(8, 0x00000000, (ulong *)(&(gDrv_Info.ext_csd_buf[0])), EMMC_MAX_EXT_CSD_LENGTH);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}

	if (ret != EMMC_SUCCESS)
	{
		goto EXIT;
	}
	
	
	/* check boot config */

	/* PARTITION_CONFIG (EXT_CSD[179]) */
	if (gDrv_Info.ext_csd_buf[179] != BOOT_CONFIG_PARTITION_CONFIG)
	{
		
		ret = EMMC_ERR_MOUNT;
		goto EXIT;
	}
	
	/* BOOT_BUS_WIDTH (EXT_CSD[177]) */
	if (gDrv_Info.ext_csd_buf[177] != BOOT_CONFIG_BOOT_BUS_WIDTH)
	{
		ret = EMMC_ERR_MOUNT;
		goto EXIT;
	}

	return EMMC_SUCCESS;


EXIT:
	if (gDrv_Info.during_transfer) {
		emmc_Error_Reset();
		if (emmc_Send_Stop())
		{
			goto EXIT;
		}
	} else {
		emmc_Error_Reset();
	}
	
	/* CMD0 (all state --> idle) */
	emmc_Go_Idle_State();

	/* check busy */
	if (GET_REG32(CE_HOST_STS1) & CE_HOST_STS1_CMDSEQ)
	{
		return EMMC_ERR_CARD_BUSY;
	}

	/* clock disable */
	SET_REG32(CE_CLK_CTRL, (GET_REG32(CE_CLK_CTRL)& CE_CLOCK_DISABLE));
	gDrv_Info.current_freq = 0;
	
	return ret;
}

/**
 * Emmc_Unmount - eMMC unmount
 * @return None
 */
void Emmc_Unmount(void)
{
	emmc_Go_Idle_State();
	
	return ;
}

