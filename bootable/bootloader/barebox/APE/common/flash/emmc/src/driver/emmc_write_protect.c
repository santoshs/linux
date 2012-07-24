/*	emmc_write_protect.c								*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.          	*/
/* All rights reserved.									*/
/*														*/

#include "protect_option.h"
#include "common.h"
#include "emmc.h"

/* RWDT_CLEAR is defined in "common.h" and "emmc_private.h"   */
/* It causes double definition warnings, and We want use      */
/* defined in "emmc_private.h". So, We undef RWDT_CLEAR here  */
#ifdef RWDT_CLEAR 
#undef RWDT_CLEAR
#endif

#include "emmc_private.h"
#include "emmc_protect_common.h"


extern EMMC_DRV_INFO gDrv_Info;

RC Emmc_Exec_Protect(ulong start_sector);

/**
 * Emmc_Write_Protect - check WP reg. and call protection task.
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
RC Emmc_Write_Protect(ulong start_sector, ulong end_sector, ulong user_data)
{
	RC	ret = EMMC_SUCCESS;

	if(user_data == ALL_USER_DATA_PROTECT)
	{
		start_sector = 0L;
		end_sector = (gDrv_Info.sec_count - 1);	/* EXT_CSD[SEC_COUNT] */
	}

	if(end_sector < start_sector)
	{
		return EMMC_ERR_PARAM;
	}

	if(start_sector >= gDrv_Info.sec_count)
	{
		return EMMC_ERR_PARAM;
	}

	if(end_sector >= gDrv_Info.sec_count)
	{
		return EMMC_ERR_PARAM;
	}

	if( ( (gDrv_Info.ext_csd_buf[171] & 0x04) != 0x00)     // check US_PERM_WP_EN
	 || ( (gDrv_Info.ext_csd_buf[171] & 0x01) != 0x00) ) { // check US_PWR_WP_EN

		return EMMC_ERR_STATE;
	}

	ret = ProtectOperation(start_sector, end_sector, MMC_PROTECT_OPE_WRITE);

	return ret;
}



/**
 * Emmc_Exec_Protect - make protection clear command, 
 *                     and send command to device.
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
RC Emmc_Exec_Protect(ulong start_sector)
{
	RC	ret = EMMC_SUCCESS;

	/* CMD28 */
	emmc_Make_Cmd(28, start_sector, NULL, 0);
	ret = emmc_Issue_Cmd();
	if (ret != EMMC_SUCCESS) {

		goto EXIT;
	}

	do {
		/* CMD13 */
		emmc_Make_Cmd(13, EMMC_RCA << 16, NULL, 0);
		ret = emmc_Issue_Cmd();
		if (ret != EMMC_SUCCESS) {

			goto EXIT;
		}

	} while (!(gDrv_Info.r1_card_status & EMMC_R1_READY));

	return EMMC_SUCCESS;

EXIT:
	emmc_Error_Reset();
	return ret;
}
