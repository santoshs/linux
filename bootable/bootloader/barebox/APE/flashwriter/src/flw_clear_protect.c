/*	flw_clear_protect.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "protect_option.h"
#include "flw_main.h"

/**
 * flw_clear_protect - clear protection task in Flash Writer.
 * @return FLW_OK       :Success
 *         FLW_WP_ERROR :Error
 */
RC flw_clear_protect( uchar* pbuff, ulong length )
{
	/* return code */
	RC ret = FLW_OK;
	ulong user_data = 0x00L;
	ulong start_group;
	ulong end_group;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

#ifdef ENABLE_EMMC_PROTECT

	/* get usb data */
	start_group = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);
	end_group = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 4]);
	user_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 8]);

	/* flash write protect clear */
	ret = Flash_Access_Clear_Protect(start_group, end_group, user_data);

	if( ret == FLASH_ACCESS_ERR_PARAM )
	{
		ret = FLW_DEVICE_PARAM_ERROR;
	}
	else if( ret == FLASH_ACCESS_ERR_NOT_MOUNT )
	{
		ret = FLW_DEVICE_NOT_MOUNT;
	}
	else if( ret == FLASH_ACCESS_ERR_NOT_INIT)
	{
		ret = FLW_DEVICE_NOT_INIT;
	}
	else if( ret == FLASH_ACCESS_ERR_CLEAR_PROTECT )
	{
		ret = FLW_DEVICE_CLEAR_PROTECT_ERROR;
	}
	else if( ret == FLASH_ACCESS_SUCCESS )
	{
		ret = FLW_OK;
	}
	else
	{
		ret = FLW_COMMAND_PARAM_ERROR;
	}
#endif

	/* make packet header */
	header.command  = src_header->command | SEND_CMD_ADD_DATA;
	header.sequence = src_header->sequence;
	header.status   = ret != FLW_OK ? FLW_ERROR : ret;
	header.length   = UL_CONV_ENDIAN(PACKET_RES_SIZE);
	data = UL_CONV_ENDIAN(ret);

	flw_create_packet(&header, &data, PACKET_RES_SIZE);

	flw_send_data(g_pSendBuff,PACKET_RES_SIZE+PACKET_HEADER_SIZE+PACKET_FOOTER_SIZE);

	g_next_phase = FLW_PHASE_NONE;

	return ret;
}

