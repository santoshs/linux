/*	emmc_clear_protect.c								*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.     		*/
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

static RC Emmc_Exec_Protect_Clear(ulong start_sector);



/**
 * Emmc_Clear_Protect_For_Format - protect clear for flash format commnand.
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
RC Emmc_Clear_Protect_For_Format(void)
{
	RC	ret = EMMC_SUCCESS;

#ifdef ENABLE_EMMC_PROTECT

	ret = Emmc_Clear_Protect(EMMC_PROTECT_AREA_ST, EMMC_PROTECT_AREA_ED, ALL_USER_DATA_PROTECT);

#endif

	return ret;
}



/**
 * Emmc_Clear_Protect - check WP reg. and call protection clear task.
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
RC Emmc_Clear_Protect(ulong start_sector, ulong end_sector, ulong user_data)
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

	ret = ProtectOperation(start_sector, end_sector, MMC_PROTECT_OPE_CLEAR);

	return ret;
}


extern RC Emmc_Exec_Protect(ulong start_sector);

/**
 * ProtectOperation - make protection area 
 *                    and do protect or protection clear.
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
RC ProtectOperation(unsigned long start_sector, unsigned long end_sector, unsigned int ope)
{
	RC (*protect_exec)(unsigned long addr);
	RC	ret = EMMC_SUCCESS;

	unsigned int wp_gp_size, es_size = 0, es_gp_size = 0, es_ml_size = 0;
	ulong protect_start = 0, protect_end = 0;
	ulong pt_size = 0;;
	ulong unit = 0;
	unsigned char es_mlgp_temp[2];

/*----------------------------------------------------------------------*/
	unit = (gDrv_Info.access_mode == EMMC_ACCESS_MODE_SECTOR) ? 1 : EMMC_BLOCK_LENGTH;

	wp_gp_size = (unsigned int)(gDrv_Info.csd_buf[4] & 0x1F);
	es_gp_size = (unsigned int)( (gDrv_Info.csd_buf[5] >> 2) & 0x1F);

	es_mlgp_temp[0] = (unsigned char)( (gDrv_Info.csd_buf[4] >> 5) & 0x07 ); // lower 3 bit
	es_mlgp_temp[1] = (unsigned char)( (gDrv_Info.csd_buf[5] << 3) & 0x18 ); // uper  2 bit

	es_ml_size  = (unsigned int)(es_mlgp_temp[0] | es_mlgp_temp[1]);
	es_size     = (unsigned int)(es_gp_size + 1) * (es_ml_size + 1);

	pt_size = (ulong)( ( (wp_gp_size + 1) * es_size) * unit);
/*----------------------------------------------------------------------*/

	protect_start = (ulong)(start_sector * unit);
	protect_end   = (ulong)(end_sector * unit);

	switch(ope){

	case MMC_PROTECT_OPE_CLEAR :
		protect_exec = Emmc_Exec_Protect_Clear;
		break;
		
	case MMC_PROTECT_OPE_WRITE :
		protect_exec = Emmc_Exec_Protect;
		break;

	default :
		return EMMC_ERR_PARAM;
	}

	while(protect_start < ( protect_end + (1 * unit) ) ){

		if( (protect_start + pt_size) > (protect_end + (1 * unit) ) ){

			protect_start = (protect_end + (1 * unit) ) - pt_size;
		}

		ret = (*protect_exec)(protect_start);

		if (ret != EMMC_SUCCESS) {
	
			return ret;
		}

		protect_start += pt_size;
	}

	return ret;
}



/**
 * Emmc_Exec_Protect_Clear - make protection clear command, 
 *                           and send command to device.
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
static RC Emmc_Exec_Protect_Clear(ulong start_sector)
{
	RC	ret = EMMC_SUCCESS;

	/* CMD29 */
	emmc_Make_Cmd(29, start_sector, NULL, 0);
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



/**
 * Emmc_Get_Max_Sector_Count - get max sector value from eMMC.
 * @return unsigned long : max sector value.
 */
ulong Emmc_Get_Max_Sector_Count(void)
{
	return gDrv_Info.sec_count;
}



/**
 * Emmc_Get_Write_Protect_Group_Size - get one write protect size from eMMC.
 * @return unsigned long : write protect group size. The unit is sector.
 */
ulong Emmc_Get_Write_Protect_Group_Size(void)
{
	unsigned long write_protect_group_size = 0L;
	unsigned int wp_gp_size, es_size = 0, es_gp_size = 0, es_ml_size = 0;
	unsigned char es_mlgp_temp[2];

	wp_gp_size = (unsigned int)(gDrv_Info.csd_buf[4] & 0x1F);				/* CSD [ WP_GRP_SIZE ]		*/
	es_gp_size = (unsigned int)( (gDrv_Info.csd_buf[5] >> 2) & 0x1F);		/* CSD [ ERASE_GRP_SIZE ]	*/

	es_mlgp_temp[0] = (unsigned char)( (gDrv_Info.csd_buf[4] >> 5) & 0x07 ); /* CSD [ ERASE_GRP_MULT ] lower 3 bit	*/
	es_mlgp_temp[1] = (unsigned char)( (gDrv_Info.csd_buf[5] << 3) & 0x18 ); /* CSD [ ERASE_GRP_MULT ] upper 2 bit	*/

	es_ml_size  = (unsigned int)(es_mlgp_temp[0] | es_mlgp_temp[1]);
	es_size     = (unsigned int)(es_gp_size + 1) * (es_ml_size + 1);

	write_protect_group_size = (ulong)( (wp_gp_size + 1) * es_size);

	return write_protect_group_size;
}
