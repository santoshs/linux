/* emmc_common.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "string.h"
#include "emmc.h"
#include "emmc_private.h"


/*****************************************************************************
; Data Declaration
******************************************************************************/
#define EXEC_CMD_MAX_SECTOR		(0x0000FFFFL)
#define EXEC_CMD_MAX_BYTE		(EMMC_BLOCK_LENGTH * EXEC_CMD_MAX_SECTOR)

const EMMC_CMD_SPEC CMD_SPEC[EMMC_MAX_CMD_NUMBER + 1] = {
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* CMD0 */
	{EMMC_CMD_TYPE_BCR,  EMMC_CMD_RESP_R3,  EMMC_DATA_DIR_NONE,     0x01405000}, /* CMD1 */
	{EMMC_CMD_TYPE_BCR,  EMMC_CMD_RESP_R2,  EMMC_DATA_DIR_NONE,     0x02806000}, /* CMD2 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x03400000}, /* CMD3 */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x04000000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1B, EMMC_DATA_DIR_NONE,     0x05600000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1B, EMMC_DATA_DIR_NONE,     0x06600000}, /* CMD6 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x07400000}, /* CMD7 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_READ,     0x08480000}, /* CMD8 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R2,  EMMC_DATA_DIR_NONE,     0x09806000}, /* CMD9 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R2,  EMMC_DATA_DIR_NONE,     0x0A806000}, /* don't use */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_READ,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x0C400000}, /* CMD12 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x0D400000}, /* CMD13 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_READBT,   0x0E480400}, /* CMD14 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x0F000000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x10400000}, /* CMD16 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_READ,     0x11480000}, /* CMD17 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_READ,     0x124A0000}, /* CMD18 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_WRITEBT,  0x134C0100}, /* CMD19 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_WRITE,    0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x17400000}, /* CMD23 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_WRITE,    0x184C0000}, /* CMD24 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_WRITE,    0x194E0000}, /* CMD25 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_WRITE,    0x1A4C0000}, /* don't use */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_WRITE,    0x1B4C0000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1B, EMMC_DATA_DIR_NONE,     0x1C600000}, /* CMD28 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1B, EMMC_DATA_DIR_NONE,     0x1D600000}, /* CMD29 */
	{EMMC_CMD_TYPE_ADTC, EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_READ,     0x1E480000}, /* don't use */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x23400000}, /* CMD35 */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1,  EMMC_DATA_DIR_NONE,     0x24400000}, /* CMD36 */
	{EMMC_CMD_TYPE_BC,   EMMC_CMD_RESP_NON, EMMC_DATA_DIR_NONE,     0x00000000}, /* don't use */
	{EMMC_CMD_TYPE_AC,   EMMC_CMD_RESP_R1B, EMMC_DATA_DIR_NONE,     0x26600000}, /* CMD38 */
};  


EMMC_DRV_INFO gDrv_Info;
EMMC_CMD_INFO gCmd_Info;


/**
 * Emmc_Set_Partition - eMMC Set Partition
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
RC Emmc_Set_Partition(EMMC_PARTITION id)
{
	RC ret = EMMC_SUCCESS;
	ulong cmd_arg = 0;
	
	/* check parameter */
	if (id >= EMMC_PARTITION_MAX)
	{
		return EMMC_ERR_PARAM;
	}
	
	cmd_arg = (gDrv_Info.ext_csd_buf[179] & 0xF8) | id;
	cmd_arg = 0x03B30000 | (cmd_arg << 8);
	
	ret = emmc_Set_Ext_Csd(cmd_arg);
	
	return ret;
}

/**
 * emmc_Go_Idle_State - Go Idle State
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
RC emmc_Go_Idle_State(void)
{
	ulong i;
	RC ret = EMMC_SUCCESS;
	
	/* CMD0 (MMC clock is current frequency. if Data transfer mode, 20MHz or higher.) */
	for (i = 0; i < 3; i++)
	{
		emmc_Make_Cmd(0, 0x00000000L, NULL, 0);
		ret = emmc_Issue_Cmd();
		if (ret == EMMC_SUCCESS)
		{
			break;
		}
	}
	
	if (ret != EMMC_SUCCESS)
	{
		return EMMC_ERR_TIMEOUT;
	}
	
	/* initialize driver info */
	gDrv_Info.during_transfer = FALSE;
	gDrv_Info.current_state = EMMC_R1_STATE_IDLE;
	gDrv_Info.bus_width = EMMC_BUSWIDTH_1BIT;
	
	/* clock enable */
	gDrv_Info.freq = EMMC_CLK_200KHZ;
	emmc_Clock_Enable();
	
	return ret;
}

/**
 * emmc_Go_Transfer_State - Go Transfer State
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
 */
RC emmc_Go_Transfer_State(void)
{
	RC	ret = EMMC_SUCCESS;
	ulong retry;
	uchar csd_spec_vars = 0;
	ulong freq = EMMC_CLK_200KHZ;
	
	/* clock on (force change) */
	gDrv_Info.freq = EMMC_CLK_200KHZ;
	emmc_Clock_Enable();
	
	/* wait */
	emmc_Wait(1);

	/* CMD0, arg=0x00000000 */
	ret = emmc_Go_Idle_State();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* CMD1 */
	emmc_Make_Cmd(1, EMMC_HOST_OCR_VALUE, NULL, 0);
	for (retry = CMD1_RETRY_COUNT; retry > 0; retry--)
	{
		ret = emmc_Issue_Cmd();
		if (ret != EMMC_SUCCESS)
		{
			return ret;
		}
		if (gDrv_Info.r3_ocr & EMMC_OCR_STATUS_BIT)
		{
			break;
		}
		emmc_Wait(1);
	}
	
	if (retry == 0)
	{
		return EMMC_ERR_TIMEOUT;
	}

	if (gDrv_Info.r3_ocr & EMMC_OCR_ACCESS_MODE_MASK)
	{
		/* sector mode */
		gDrv_Info.access_mode = EMMC_ACCESS_MODE_SECTOR;
	}
	else
	{
		/* byte mode */
		gDrv_Info.access_mode = EMMC_ACCESS_MODE_BYTE;
	}

	/* CMD2 */
	emmc_Make_Cmd(2, 0x00000000, NULL, 0);
	gCmd_Info.pRsp = (ulong *)&(gDrv_Info.cid_buf[0]);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* CMD3 */
	emmc_Make_Cmd(3, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* CMD9 (CSD) */
	emmc_Make_Cmd(9, EMMC_RCA << 16, NULL, 0);
	gCmd_Info.pRsp = (ulong *)&(gDrv_Info.csd_buf[0]);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}
	
	/* check SPEC_VERS(CSD[125-122]) */
	csd_spec_vars = gDrv_Info.csd_buf[EMMC_CSD_SPEC_VERS_BYTE];
	csd_spec_vars = (csd_spec_vars >> EMMC_CSD_SPEC_VERS_SHIFT) & EMMC_CSD_SPEC_VERS_MASK;
	if (csd_spec_vars < 4)
	{
		return EMMC_ERR_ILLEGAL_CARD;
	}

	/* CMD7 (select card) */
	emmc_Make_Cmd(7, EMMC_RCA << 16, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* CMD16 */
	emmc_Make_Cmd(16, EMMC_BLOCK_LENGTH, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* CMD8 (EXT_CSD) */
	emmc_Make_Cmd(8, 0x00000000, (ulong *)(&gDrv_Info.ext_csd_buf[0]), EMMC_MAX_EXT_CSD_LENGTH);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* High Speed (MMC clock 200kHz --> 52MHz(Max) */
	if (gDrv_Info.ext_csd_buf[EMMC_EXT_CSD_CARD_TYPE] & EMMC_EXT_CSD_CARD_TYPE_52MHZ)
	{
		freq = EMMC_CLK_52MHZ;
	}
	else
	{
		emmc_Error_Reset();
		return EMMC_ERR_HIGH_SPEED;
	}

	/* CMD6 */
	emmc_Make_Cmd(6, EMMC_SWITCH_HS_TIMING, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		if ((GET_REG32(CE_INT) & CE_INT_ALL_ERROR) == CE_INT_RBSYTO)
		{
			/* Response Busy timeout */
			emmc_Error_Reset();
			ret = emmc_Wait_Ready_For_Data();
			if (ret != EMMC_SUCCESS)
			{
				return EMMC_ERR_HIGH_SPEED;
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
	
	/* clock change */
	gDrv_Info.freq = freq;
	emmc_Clock_Enable();

	/* CMD8 (EXT_CSD) */
	emmc_Make_Cmd(8, 0x00000000, (ulong *)(&gDrv_Info.ext_csd_buf[0]), EMMC_MAX_EXT_CSD_LENGTH);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* check HS_TIMING */
	if (gDrv_Info.ext_csd_buf[EMMC_EXT_CSD_HS_TIMING] != 0x01)
	{
		ret = EMMC_ERR_HIGH_SPEED;
		goto EXIT;
	}
	
	/* Get SEC_COUNT(EXT_CSD[215-212]) */
	/*     8GB device = 0x00EC0000     */
	/*     2GB device = 0x003A8000     */
	memcpy(&(gDrv_Info.sec_count), &(gDrv_Info.ext_csd_buf[EMMC_EXT_CSD_SEC_COUNT]), sizeof(ulong));
	
	
	return EMMC_SUCCESS;
	
	
EXIT:
	if (gDrv_Info.during_transfer)
	{
		emmc_Error_Reset();
		if (emmc_Send_Stop())
		{
			goto EXIT;
		}
	}
	else
	{
		emmc_Error_Reset();
	}
	return ret;
}

/**
 * emmc_Set_Ext_Csd - Set EXT_CSD
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
RC emmc_Set_Ext_Csd(ulong cmd_arg)
{
	RC	ret = EMMC_SUCCESS;

	/* CMD6 */
	emmc_Make_Cmd(6, cmd_arg, NULL, 0);
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
	emmc_Make_Cmd(8, 0x00000000, (ulong *)(&gDrv_Info.ext_csd_buf[0]), EMMC_MAX_EXT_CSD_LENGTH);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
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
	return ret;
}

/**
 * emmc_Issue_Cmd - Issue CMD
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
RC emmc_Issue_Cmd(void)
{
	RC	ret = EMMC_SUCCESS;
	uint64	temp_req = 0;
	uint64	temp_remain = 0; 
	
	/* check multi sector transfer */
	if ((gCmd_Info.cmd == 18) || (gCmd_Info.cmd == 25))
	{
		do
		{
			/* check requset size >= 32MB */
			if (gCmd_Info.req_length > EXEC_CMD_MAX_BYTE)
			{
				temp_req = gCmd_Info.req_length;
				gCmd_Info.req_length = EXEC_CMD_MAX_BYTE;
				
				temp_remain = gCmd_Info.remain_length;
				gCmd_Info.remain_length = EXEC_CMD_MAX_BYTE;
				
				ret = emmc_Exec_Cmd();
				if (ret != EMMC_SUCCESS)
				{
					gCmd_Info.req_length = temp_req;
					gCmd_Info.remain_length = temp_remain;
					goto EXIT;
				}
				
				/* update addres & length */
				temp_remain -= EXEC_CMD_MAX_BYTE;
				
				gCmd_Info.req_length = temp_remain;
				gCmd_Info.remain_length = temp_remain;
				
				gCmd_Info.pBuff += EXEC_CMD_MAX_BYTE;
				
				if (gDrv_Info.access_mode == EMMC_ACCESS_MODE_SECTOR)
				{
					gCmd_Info.arg += EXEC_CMD_MAX_SECTOR;
				}
				else
				{
					gCmd_Info.arg += (EXEC_CMD_MAX_SECTOR << EMMC_SECTOR_SIZE_SHIFT);
				}
			}
			else
			{
				/* final exec */
				ret = emmc_Exec_Cmd();
				if (ret != EMMC_SUCCESS)
				{
					goto EXIT;
				}
				temp_remain = 0;
			}
		} while (temp_remain > 0);
	}
	else
	{
		/* final exec */
		ret = emmc_Exec_Cmd();
		if (ret != EMMC_SUCCESS)
		{
			goto EXIT;
		}
	}
	
	return EMMC_SUCCESS;
	
	
EXIT:
	emmc_Error_Reset();

	return ret;
}

/**
 * emmc_Exec_Cmd - Exec CMD
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
RC emmc_Exec_Cmd(void)
{
	RC	ret = EMMC_SUCCESS;
	
	/* RWDT clear */
	SET_REG32(RWTCNT, RWDT_CLEAR);
	
	/* Busy check */
	if (GET_REG32(CE_HOST_STS1) & CE_HOST_STS1_CMDSEQ)
	{
		return EMMC_ERR_CARD_BUSY;
	}
	
	/* interrupt clear */
	SET_REG32(CE_INT_MASK, 0x00000000);
	SET_REG32(CE_INT, 0x00000000);
	
	/* clock on */
	ret = emmc_Clock_Enable();
	
	/* Data transfer cmd setting */
	if (gCmd_Info.type == EMMC_CMD_TYPE_ADTC)
	{
		SET_REG32(CE_ARG_CMD12, 0x00000000);
		if (gCmd_Info.req_length <= EMMC_BLOCK_LENGTH)
		{
			/* single block  */
			SET_REG32(CE_BLOCK_SET, gCmd_Info.req_length);
		}
		else
		{
			/* multiple R/W command */
			SET_REG32(CE_BLOCK_SET, ((gCmd_Info.req_length << (16-EMMC_SECTOR_SIZE_SHIFT))&0xffff0000) |CE_BLOCK_SET_SECTOR);
		}
	}
	/* ARG */
	SET_REG32(CE_ARG, gCmd_Info.arg);

	/* issue cmd */
	SET_REG32(CE_CMD_SET, gCmd_Info.hw);
	
	if (gCmd_Info.rsp == EMMC_CMD_RESP_NON)
	{
		/* wait respons complete interrupt */
		ret = emmc_Wait_Int(CE_INT_CRSPE, EMMC_RESP_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return ret;
		}
		
		/* interrupt clear */
		SET_REG32(CE_INT, CE_INT_CRSPE_CLEAR);
		
		return EMMC_SUCCESS;
	}
	
	/* wait respons complete interrupt */
	ret = emmc_Wait_Int(CE_INT_CRSPE, EMMC_RESP_TIMEOUT);
	if (ret != EMMC_SUCCESS)
	{
		return ret;
	}

	/* interrupt clear */
	SET_REG32(CE_INT, CE_INT_CRSPE_CLEAR);

	/* check response buffer */
	if (gCmd_Info.pRsp == NULL)
	{
		return EMMC_ERR_RESPONSE;
	}

	/* read response */
	if (gCmd_Info.rsp_length == EMMC_MAX_RESPONSE_LENGTH)
	{
		/* CSD or CID */
		*(gCmd_Info.pRsp) = GET_REG32(CE_RESP0);		/* [31:0] */
		*(gCmd_Info.pRsp + 1) = GET_REG32(CE_RESP1);	/* [63:32] */
		*(gCmd_Info.pRsp + 2) = GET_REG32(CE_RESP2);	/* [95:64] */
		*(gCmd_Info.pRsp + 3) = GET_REG32(CE_RESP3);	/* [127:96] */
	}
	else
	{
		*(gCmd_Info.pRsp) = GET_REG32(CE_RESP0);	/* [39:8] */
	}
	
	if (gCmd_Info.rsp == EMMC_CMD_RESP_R1B) {
		/* wait busy clear interrupt */
		ret = emmc_Wait_Int(CE_INT_RBSYE, EMMC_RESP_BUSY_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return ret;
		}
	
		SET_REG32(CE_INT, ~(CE_INT_RBSYE|CE_INT_CRSPE));
	}
	
	/* response check */
	if (gCmd_Info.rsp <= EMMC_CMD_RESP_R1B)
	{
		/* R1 or R1b */
		gDrv_Info.current_state = (*(gCmd_Info.pRsp) & EMMC_R1_STATE_MASK) >> EMMC_R1_STATE_SHIFT;
		if (*(gCmd_Info.pRsp) & EMMC_R1_ERROR_MASK)
		{
			return EMMC_ERR_RESPONSE;
		}
	}
	else if (gCmd_Info.rsp == EMMC_CMD_RESP_R4)
	{
		/* R4 */
		if (!(*(gCmd_Info.pRsp) & EMMC_R4_STATUS))
		{
			return EMMC_ERR_RESPONSE;
		}
	}
	
	/* data transfer */
	if (gCmd_Info.type == EMMC_CMD_TYPE_ADTC)
	{
		gDrv_Info.during_transfer = TRUE;
		if ((gCmd_Info.cmd == 14) || (gCmd_Info.cmd == 19))
		{
			ret = emmc_Trans_Byte((ulong *)(gCmd_Info.pBuff), gCmd_Info.req_length);
		}
		else
		{
			ret = emmc_Trans_Data((ulong *)(gCmd_Info.pBuff), gCmd_Info.req_length);
		}
		
		if (ret != EMMC_SUCCESS)
		{
			return ret;		/* error */
		}
	}
	
	gDrv_Info.during_transfer = FALSE;
	
	return EMMC_SUCCESS;
}

/**
 * emmc_Make_Cmd - Make CMD
 * @return None
 */
void emmc_Make_Cmd(ulong cmd, ulong cmd_arg, ulong *pBuff, uint64 length)
{
	gCmd_Info.cmd = cmd;
	gCmd_Info.type = CMD_SPEC[cmd].type;
	gCmd_Info.rsp = CMD_SPEC[cmd].resp;
	gCmd_Info.arg = cmd_arg;
	gCmd_Info.dir = CMD_SPEC[cmd].dir;
	gCmd_Info.hw = CMD_SPEC[cmd].hw;
	gCmd_Info.req_length = 0;
	gCmd_Info.remain_length = 0;
	gCmd_Info.pBuff = NULL;
	gCmd_Info.rsp_length = EMMC_MAX_RESP_SHORT_LENGTH;

	switch (gCmd_Info.rsp)
	{
		case EMMC_CMD_RESP_NON:
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.response_buf[0]);
			gCmd_Info.rsp_length = 0;
			break;
		case EMMC_CMD_RESP_R1:
		case EMMC_CMD_RESP_R1B:
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.r1_card_status);
			break;
		case EMMC_CMD_RESP_R2:
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.response_buf[0]);
			gCmd_Info.rsp_length = EMMC_MAX_RESPONSE_LENGTH;
			break;
		case EMMC_CMD_RESP_R3:
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.r3_ocr);
			break;
		case EMMC_CMD_RESP_R4:
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.r4_resp);
			break;
		case EMMC_CMD_RESP_R5:
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.r5_resp);
			break;
		default :
			gCmd_Info.pRsp = (ulong *)&(gDrv_Info.response_buf[0]);
	}

	/* for data transfer CMD */
	if (gCmd_Info.type == EMMC_CMD_TYPE_ADTC)
	{
		if (gDrv_Info.bus_width == EMMC_BUSWIDTH_8BIT)
		{
			gCmd_Info.hw |= EMMC_BUSWIDTH_8BIT;
		}
		gCmd_Info.pBuff = (uchar *)pBuff;
		gCmd_Info.req_length = length;
		gCmd_Info.remain_length = length;
	}

	return ;
}

/**
 * emmc_Trans_Data - Transfer Data
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_PARAM         : Parameter Error
 *         EMMC_ERR_STATE         : State Error
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 */
RC emmc_Trans_Data(ulong *pBuff, ulong count)
{
	RC	ret = EMMC_SUCCESS;
	ulong	i, j, k;
	ulong	cnt;
	ulong	*pData;
	uchar	*bData;
	ulong	work;
	ulong	alignment;
	

	if ((count < EMMC_BLOCK_LENGTH) || (pBuff == NULL))
	{
		return EMMC_ERR_PARAM;
	}

	if (!(gDrv_Info.during_transfer))
	{
		return EMMC_ERR_STATE;
	}

	cnt = count >> EMMC_SECTOR_SIZE_SHIFT;		/* number of sectors */
	pData = pBuff;
	alignment = (ulong)pData & 0x00000003L;

	/* data transefer */
	if (gCmd_Info.dir >= EMMC_DATA_DIR_WRITE )
	{
		/* Write */
		for (i=0; i < cnt; i++)
		{
			ret = emmc_Wait_Int(CE_INT_BUFWEN, EMMC_DATA_TIMEOUT);
			if (ret != EMMC_SUCCESS)
			{
				return EMMC_ERR_TRANSFER;
			}

			SET_REG32(CE_INT, ~CE_INT_BUFWEN);	/* interrupt clear */
			for (j=0; j < EMMC_BLOCK_LENGTH_DW; j++)
			{
				/* check pBuff 4byte alignment */
				if (alignment == 0)
				{
					/* word access */
					SET_REG32(CE_DATA, *pData);
					pData++;
				}
				else
				{
					/* byte access */
					work = 0;
					bData = (uchar *)pData;
					for (k = 0; k < 4; k++)
					{
						work += bData[k] << (k * 8);
					}
					SET_REG32(CE_DATA, work);
					pData++;
				}
			}
			gCmd_Info.remain_length -= EMMC_BLOCK_LENGTH;
			
			/* RWDT clear */
			SET_REG32(RWTCNT, RWDT_CLEAR);
		}
		
		if (gCmd_Info.remain_length != 0)
		{
			return EMMC_SUCCESS;
		}

		/* wait transefer complete interrupt */
		ret = emmc_Wait_Int(CE_INT_DTRANE, EMMC_DATA_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return EMMC_ERR_TRANSFER;
		}
		
		SET_REG32(CE_INT, ~CE_INT_DTRANE);	/* interrupt clear */
	}
	else
	{
		/* read */
		for (i = 0; i < cnt; i++)
		{
			ret = emmc_Wait_Int(CE_INT_BUFREN, EMMC_DATA_TIMEOUT);
			if (ret != EMMC_SUCCESS)
			{
				return EMMC_ERR_TRANSFER;
			}

			SET_REG32(CE_INT, ~CE_INT_BUFREN);	/* interrupt clear */
			for (j = 0; j < EMMC_BLOCK_LENGTH_DW; j++)
			{
				/* check pBuff 4byte alignment */
				if (alignment == 0)
				{
					/* word access */
					*pData = GET_REG32(CE_DATA);
					pData++;
				}
				else
				{
					/* byte access */
					work = GET_REG32(CE_DATA);
					bData = (uchar *)pData;
					for (k = 0; k < 4; k++)
					{
						bData[k] = (work >> (k * 8)) & 0xFF;
					}
					pData++;
				}
			}
			gCmd_Info.remain_length -= EMMC_BLOCK_LENGTH;
			
			/* RWDT clear */
			SET_REG32(RWTCNT, RWDT_CLEAR);
		}
		
		if (gCmd_Info.remain_length != 0)
		{
			return EMMC_SUCCESS;
		}

		/* wait transefer complete interrupt */
		ret = emmc_Wait_Int(CE_INT_BUFRE, EMMC_DATA_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return EMMC_ERR_TRANSFER;
		}
		
		SET_REG32(CE_INT, ~CE_INT_BUFRE);	/* interrupt clear */
	}

	return EMMC_SUCCESS;
}

/**
 * emmc_Trans_Byte - Transfer Byte Data
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_PARAM         : Parameter Error
 *         EMMC_ERR_STATE         : State Error
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 */
RC emmc_Trans_Byte(ulong *pBuff, ulong length)
{
	RC	ret = EMMC_SUCCESS;
	ulong	i, j, k;
	ulong	tmp;
	uchar	*bData;
	ulong	*pData;
	ulong	work;
	ulong	alignment;

	/* parameter check */
	if ((pBuff==NULL) || (length == 0) || (length >= EMMC_BLOCK_LENGTH))
	{
		return EMMC_ERR_PARAM;
	}

	if (gCmd_Info.remain_length != length)
	{
		return EMMC_ERR_PARAM;
	}

	if (!(gDrv_Info.during_transfer))
	{
		return EMMC_ERR_STATE;
	}

	pData = pBuff;
	alignment = (ulong)pData & 0x00000003L;

	/* data transefer */
	if (gCmd_Info.dir >= EMMC_DATA_DIR_WRITE) 
	{
		/* write */
		/* wait interrupt (Write enable) */
		ret = emmc_Wait_Int(CE_INT_BUFWEN, EMMC_DATA_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return EMMC_ERR_TRANSFER;
		}

		SET_REG32(CE_INT, ~CE_INT_BUFWEN);	/* interrupt clear */

		/* Buff --> FIFO */
		for (i = length; i > 0; i -= 4)
		{
			/* check pBuff 4byte alignment */
			if (alignment == 0)
			{
				/* word access */
				SET_REG32(CE_DATA, *pData);
				pData++;
			}
			else
			{
				/* byte access */
				work = 0;
				bData = (uchar *)pData;
				for (k = 0; k < 4; k++)
				{
					work += bData[k] << (k * 8);
				}
				SET_REG32(CE_DATA, work);
				pData++;
			}
		}
		
		/* wait interrupt (Write complete) */
		ret = emmc_Wait_Int(CE_INT_DTRANE, EMMC_DATA_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return EMMC_ERR_TRANSFER;
		}

		SET_REG32(CE_INT, ~CE_INT_DTRANE);	/* interrupt clear */
		
	}
	else
	{
		/* read */
		/* wait interrupt (read enable) */
		ret = emmc_Wait_Int(CE_INT_BUFREN, EMMC_DATA_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return EMMC_ERR_TRANSFER;
		}
		
		SET_REG32(CE_INT, ~CE_INT_BUFREN);	/* interrupt clear */
		
		/* FIFO --> Buff */
		for (i = length; i > 3; i-=4)
		{
			/* check pBuff 4byte alignment */
			if (alignment == 0)
			{
				/* word access */
				*pData = GET_REG32(CE_DATA);
				pData++;
			}
			else
			{
				/* byte access */
				work = GET_REG32(CE_DATA);
				bData = (uchar *)pData;
				for (k = 0; k < 4; k++)
				{
					bData[k] = (work >> (k * 8)) & 0xFF;
				}
				pData++;
			}
		}

		if (i > 0)
		{
			/* fraction data */
			bData = (uchar *)pData;
			tmp = GET_REG32(CE_DATA);
			for (j = 0; j < i; j++)
			{
				*bData = (uchar)tmp;
				bData++;
				tmp = tmp >> 8;
			}
		}

		/* wait interrupt (read complete) */
		ret = emmc_Wait_Int(CE_INT_BUFRE, EMMC_DATA_TIMEOUT);
		if (ret != EMMC_SUCCESS)
		{
			return EMMC_ERR_TRANSFER;
		}

		SET_REG32(CE_INT, ~CE_INT_BUFRE);	/* interrupt clear */
	}

	gCmd_Info.remain_length = 0;

	return EMMC_SUCCESS;
}

/**
 * emmc_Error_Reset - Error Reset
 * @return None
 */
void emmc_Error_Reset(void)
{
	ulong	loop = ERROR_RESET_RETRY_COUNT;
	ulong	retry = 3;

	gDrv_Info.during_transfer = FALSE;
	
	/* check during operation */
	if (GET_REG32(CE_HOST_STS1) & CE_HOST_STS1_CMDSEQ)
	{
		/* forced termination */
		SET_REG32(CE_CMD_CTRL, CE_CMD_CTRL_BREAK);
		SET_REG32(CE_CMD_CTRL, 0x00000000);
		
		/* wait CMDSEQ = 0 */
		while (loop > 0)
		{
			if (!(GET_REG32(CE_HOST_STS1) & CE_HOST_STS1_CMDSEQ))
			{
				break;
			}
			
			loop--;
			if ((loop == 0) && (retry > 0) )
			{
				emmc_Wait(1);	/* wait */
				loop = ERROR_RESET_RETRY_COUNT;
				retry--;
			}
		}
	}

	/* reset */
	SET_REG32(CE_VERSION, CE_VERSION_SWRST);
	SET_REG32(CE_VERSION, 0x00000000);

	/* initialize */
	SET_REG32(CE_INT, 0x00000000);
	SET_REG32(CE_INT_MASK, 0x00000000);			/* all interrupt disable */
	SET_REG32(CE_BUF_ACC, CE_BUF_ACC_SWAP_ENABLE);

	/* restart MMC clock */
	emmc_Clock_Enable();
	
	return;
}

/**
 * emmc_Wait_Int - Wait Interrupt
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 */
RC emmc_Wait_Int(ulong event, ulong loop)
{
	ulong	i;
	ulong	wait_count = EMMC_INT_TIMEOUT_COUNT * loop;
	volatile ulong	status;
	ulong	interested_event = event | CE_INT_ALL_ERROR;

	/* wait interrupt */
	for (i = 0; i < wait_count; i++)
	{
		status = GET_REG32(CE_INT) & interested_event;
		
		if (status)
		{
			if (status & CE_INT_ALL_ERROR)
			{
				/* occured error interrupt */
				if (status & CE_INT_CMDVIO)
				{
					return EMMC_ERR_CMD_ISSUE;
				}
				else if (status & CE_INT_BUFVIO)
				{
					return EMMC_ERR_BUFFER_ACCESS;
				}
				else if (status & (CE_INT_WDATERR | CE_INT_RDATERR))
				{
					return EMMC_ERR_TRANSFER;
				}
				else if (status & (CE_INT_RIDXERR | CE_INT_RSPERR))
				{
					return EMMC_ERR_RESPONSE;
				}
				else if (status & CE_INT_CRCSTO)
				{
					return EMMC_ERR_CRC;
				}
				else
				{
					return EMMC_ERR_TIMEOUT;
				}
			}
			else
			{
				return EMMC_SUCCESS;	/* expected event */
			}
		}
	}

	return EMMC_ERR_TIMEOUT;
}


/**
 * emmc_Clock_Enable - clock enable
 * @return EMMC_SUCCESS       : Successful
 *         EMMC_ERR_CARD_BUSY : Card status busy
 */
RC emmc_Clock_Enable(void)
{
	/* busy */
	if (GET_REG32(CE_HOST_STS1) & CE_HOST_STS1_CMDSEQ)
	{
		return EMMC_ERR_CARD_BUSY;
	}
	
	if (gDrv_Info.current_freq == gDrv_Info.freq)
	{
		return EMMC_SUCCESS;
	}

	/* clock on */
	SET_REG32(CE_CLK_CTRL, gDrv_Info.freq);
	SET_REG32(CE_CLK_CTRL, CE_CLOCK_ENABLE | gDrv_Info.freq);

	gDrv_Info.current_freq = gDrv_Info.freq;
	
	return EMMC_SUCCESS;
}

/**
 * emmc_Wait_Ready_For_Data - Wait card ctatus(READY_FOR_DATA)
 * @return EMMC_SUCCESS           : Successful
 *         EMMC_ERR_CARD_BUSY : Card status busy
 *         EMMC_ERR_CMD_ISSUE     : CMD Issue Error
 *         EMMC_ERR_BUFFER_ACCESS : Buffer Access Error
 *         EMMC_ERR_TRANSFER      : Data Transfer Error
 *         EMMC_ERR_RESPONSE      : CMD Response Error
 *         EMMC_ERR_CRC           : CRC Error
 *         EMMC_ERR_TIMEOUT       : Card status busy
 *         EMMC_ERR_PARAM         : Parameter Error
 *         EMMC_ERR_STATE         : State Error
 */
RC emmc_Wait_Ready_For_Data(void)
{
	RC ret = EMMC_SUCCESS;
	ulong	i;

	for (i = 0; i < CMD13_RETRY_COUNT; i++)
	{
		emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
		ret = emmc_Issue_Cmd();
		if (ret != EMMC_SUCCESS)
		{
			/* error information not update */
			emmc_Error_Reset();
			return ret;
		}
		
		if (gDrv_Info.r1_card_status & EMMC_R1_READY)
		{
			/* card is ready. (without R1 error status) */
			return EMMC_SUCCESS;
		}
		
		emmc_Wait(1);	/* wait */
	}

	return EMMC_ERR_TIMEOUT;
}

/**
 * emmc_Wait - eMMC software timer
 * @return None
 */
void emmc_Wait(ulong loop)
{
#if 0
	ulong i;
	ulong wait_count = EMMC_TIMEOUT_COUNT * loop;

	/* 3 CPU cycle */
	for (i = wait_count; i > 0; i--)
	{
	}
#else
	/* It corrects not to be optimized by the GCC compiler. */
	asm volatile("        ldr r1, __Wait_waitCount");
	asm volatile("        mul r0, r0, r1");
	asm volatile("        b   __Wait_loopCmp");
	asm volatile("__Wait_loopSub:");
	asm volatile("        sub r0, r0, #1");
	asm volatile("__Wait_loopCmp:");
	asm volatile("        cmp r0, #0");
	asm volatile("        bne __Wait_loopSub");
	asm volatile("__Wait_loopEnd:");
	asm volatile("__Wait_waitCount:");
	asm volatile("        .word  %a0" : : "i" (EMMC_TIMEOUT_COUNT));
#endif
}

/**
 * emmc_Send_Stop - Stop sending
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
RC emmc_Send_Stop(void)
{
	RC ret = EMMC_SUCCESS;
	
	/* CMD12 */
	emmc_Make_Cmd(12, 0x00000000, NULL, 0);

	/* R1 for write case and R1b for write case */
	if ((gCmd_Info.dir == EMMC_DATA_DIR_WRITE)||(gCmd_Info.dir == EMMC_DATA_DIR_WRITEBT))
	{
		gCmd_Info.rsp = EMMC_CMD_RESP_R1B;
		gCmd_Info.hw |= CE_CMD_SET_RBSY;
	}

	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS)
	{
		if ((gCmd_Info.rsp == EMMC_CMD_RESP_R1B) && ((GET_REG32(CE_INT) & CE_INT_ALL_ERROR) == CE_INT_RBSYTO))
		{
			/* Response Busy timeout */
			emmc_Error_Reset();
			
			/* wait ready */
			ret = emmc_Wait_Ready_For_Data();
			if (ret != EMMC_SUCCESS)
			{
				/* not ready */
				return ret;
			}
			else
			{
				/* ready without R1 status error */
				return EMMC_SUCCESS;
			}
		}
		else
		{
			emmc_Error_Reset();
			return ret;
		}
	}

	return EMMC_SUCCESS;
}

/**
 * emmc_Check_Param_Multi - check parameter for Read_Multi/Write_Multi
 * @return EMMC_SUCCESS   : Successful
 *         EMMC_ERR_PARAM : Parameter Error
 */
RC emmc_Check_Param_Multi(uchar* pBuff, ulong start_sector, ulong sector_count)
{
	/* parameter check */
	if (pBuff == NULL)
	{
		return EMMC_ERR_PARAM;
	}
	if (sector_count < 2)
	{
		return EMMC_ERR_PARAM;
	}
	
#if 0	/* Don't check MAX */
	if (start_sector > EMMC_END_SECTOR_USER)
	{
		return EMMC_ERR_PARAM;
	}
	if ((start_sector + sector_count - 1) > EMMC_END_SECTOR_USER)
	{
		return EMMC_ERR_PARAM;
	}
#endif	/* Don't check MAX */	
	
	return EMMC_SUCCESS;
}

/**
 * emmc_Check_Param_Single - check parameter for Read_Single/Write_Single
 * @return EMMC_SUCCESS   : Successful
 *         EMMC_ERR_PARAM : Parameter Error
 */
RC emmc_Check_Param_Single(uchar* pBuff, ulong start_sector)
{
	/* parameter check */
	if (pBuff == NULL)
	{
		return EMMC_ERR_PARAM;
	}

#if 0	/* Don't check MAX */
	if (start_sector > EMMC_END_SECTOR_USER)
	{
		return EMMC_ERR_PARAM;
	}
#endif	/* Don't check MAX */
	
	return EMMC_SUCCESS;
}

