/*	flw_write.c	
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flw_main.h"

static ulong g_write_class;
static uint64 g_start_address;
static ulong g_data_size;
static ulong g_packet_data_size;
static ulong g_user_data;
static ulong g_data_verify;
static int g_write_return = FLW_OK;
extern ulong flash_Get_Sector(ulong block, uint64 address, ulong data_size);
extern unsigned long Emmc_Get_Max_Sector_Count(void);
uchar* pRamBuff;
ulong MASK1 = 0x000000FF;
ulong MASK2 = 0x0000FFFF;
ulong MASK3 = 0x00FFFFFF;

char lcd_prog[]				= "                   ";
char prog_num = 1;
uchar prog_char = 0x23;

/**
 * flw_write_start - This function processes it by write_start command.
 * @return         - None
 */
RC flw_write_start( uchar* pbuff)
{
	int ret = FLW_OK;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data = FLW_OK;
	union {
		ulong data32[2];
		uint64 data64;
	} address;
	
	ulong start_add_sector = 0;
	ulong data_size_sector = 0;

	g_packet_data_size = 0;
	g_write_class = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);
	address.data32[1] = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 4]);
	address.data32[0] = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 8]);
	g_start_address = address.data64;
	g_data_size = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 12]);
	g_data_verify = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 16]);
	g_user_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 20]);

	/* check input data size */
	start_add_sector = flash_Get_Sector(0, g_start_address, 0);
	data_size_sector = flash_Get_Sector(0, 0, g_data_size);
	if(Emmc_Get_Max_Sector_Count() < (start_add_sector + data_size_sector)) {
		g_write_return = FLW_DEVICE_PARAM_ERROR;
		data = UL_CONV_ENDIAN(g_write_return);
		ret = g_write_return;
	}
	
	/* global data initialize */
	g_data_sum = 0;

	/* make packet header */
	header.command	= src_header->command | SEND_CMD_ADD_DATA;
	header.sequence = src_header->sequence;
	header.status	= g_write_return;
	header.length	= UL_CONV_ENDIAN(PACKET_RES_SIZE);


	flw_create_packet(&header, &data, PACKET_RES_SIZE);

	flw_send_data(g_pSendBuff, PACKET_HEADER_SIZE+PACKET_RES_SIZE+PACKET_FOOTER_SIZE);

	g_next_phase = FLW_PHASE_WRITE_DATA_CMD;

	return ret;
}

/**
 * flw_write - This function processes it by write command.
 * @return   - command data
 */
RC flw_write( uchar* pbuff, ulong length )
{
	/* sequence data */
	RC ret;
	uchar i = 1;

	if ( g_write_return != FLW_OK ) {
		g_write_return = FLW_DEVICE_WRITE_ERROR;
		return FLW_DEVICE_WRITE_ERROR;
	}
	
#ifdef __FW_LCD_ENABLE__
	lcd_prog[0] = 0x5B;
	lcd_prog[11] = 0x5D;
	while (i < prog_num + 1)
	{
		lcd_prog[i] = prog_char;
		i++;
	}
	prog_num++;
	if (prog_num > 10) {
		prog_num = 1;
		i = 1;
		if (prog_char == 0x23) {
			prog_char = 0x20;
		} else {
			prog_char = 0x23;
		}
	}
	LCD_Print(LOC_X, LOC_Y_2, lcd_prog);
#endif	/* __FW_LCD_ENABLE__ */
	
	/* Data is written in flash. */
	ret = flw_write_data( pbuff, length );
	g_write_return = ret;

	g_next_phase = (g_packet_data_size >= g_data_size) ? FLW_PHASE_WRITE_END_CMD : FLW_PHASE_WRITE_DATA_CMD;
	
	if (ret != FLW_OK) {
		g_next_phase |= FLW_PHASE_ERROR_MASK;
	}

	return ret;
}

/**
 * flw_write_data - This function processes it by write command.
 * @return        - flash command
 */
RC flw_write_data(uchar* pbuff, ulong length)
{
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data[3];		/* write_data response =12byte , other 4byte */
	RC ret= FLW_OK;
	ulong data_length = 0;
	union {
		ulong data32[2];
		uint64 data64;
	} address;

	if (g_write_class == FLW_WRITE_RAM_AREA) {
		address.data64 = g_start_address;
		memcpy((void*)(address.data32[0]),&pbuff[PACKET_HEADER_SIZE],length);
	} else {
		/* flash write */
		ret = Flash_Access_Write(&pbuff[PACKET_HEADER_SIZE], length, g_start_address, g_user_data);

		if (ret == FLASH_ACCESS_SUCCESS) {
			if (1 == g_data_verify) {
				/* verify data in flash */
				ret = Flash_Access_Read(g_pReadBuff, length, g_start_address, g_user_data);
				if (ret == FLASH_ACCESS_SUCCESS) {
					PRINTF("Write data verifying...\n");
					ret = flw_write_verify(pbuff, length);
				}
			} else if (0 == g_data_verify) {
				/* do nothing */
			} else {
				ret = FLW_COMMAND_PARAM_ERROR;
			}
		}
				PRINTF("         OK\n");
	}

	g_packet_data_size += length;
	g_start_address += length;

	if (ret != FLW_OK) {
		header.command	= src_header->command | SEND_CMD_ADD_DATA;
		header.sequence = src_header->sequence;
		header.status	= (ret != FLW_OK) ? FLW_ERROR : ret;

		data_length		= PACKET_WRITE_DATA_RES_SIZE;
		header.length	= UL_CONV_ENDIAN(data_length);

		if (ret == FLASH_ACCESS_ERR_PARAM) {
			ret = FLW_DEVICE_PARAM_ERROR;
		} else if (ret == FLASH_ACCESS_ERR_NOT_MOUNT) {
			ret = FLW_DEVICE_MOUNT_ERROR;
		} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
			ret = FLW_DEVICE_INIT_ERROR;
		} else if (ret == FLASH_ACCESS_ERR_WRITE) {
			ret = FLW_DEVICE_WRITE_ERROR;
		} else if (ret == FLASH_ACCESS_ERR_READ) {
			ret = FLW_DEVICE_VERIFY_READ_ERROR;
		} else if (ret == FLW_VERIFY_ERROR) {
			ret = FLW_DEVICE_VERIFY_ERROR;
		} else if (ret == FLASH_ACCESS_SUCCESS) {
			ret = FLW_OK;
		} else {
			ret = FLW_COMMAND_PARAM_ERROR;
		}
		data[0] = UL_CONV_ENDIAN(ret);
		memcpy(&data[1],&g_start_address,8);

		flw_create_packet(&header, data, PACKET_WRITE_DATA_RES_SIZE);

		flw_send_data(g_pSendBuff,PACKET_WRITE_DATA_RES_SIZE+PACKET_HEADER_SIZE+PACKET_FOOTER_SIZE);
	}

	return ret;
}

/**
 * flw_write_verify - This function is used for checking data from flash.
 * @return        - flash command
 */
RC flw_write_verify(uchar* pbuff, ulong length)
{
	RC ret = FLW_OK;
	ulong block;
	ulong remain;
	ulong step;
	ulong mask;

	ulong *read_buff = (ulong*)READ_BUFF_ADDRESS;
	ulong *check_buff =(ulong*)(&pbuff[PACKET_HEADER_SIZE]);

	block = length / 4;
	remain = length % 4;

	for (step = 0; step < block; step++) {
		if (read_buff[step] != check_buff[step]) {
			ret = FLW_VERIFY_ERROR;
			break;
		}
	}

	if ((remain > 0) && (ret == FLW_OK)) {
		if (1 == remain) {
			mask = MASK1;
		} else if (2 == remain) {
			mask = MASK2;
		} else {
			mask = MASK3;
		}

		read_buff[step] &= mask;
		check_buff[step] &= mask;
		if (read_buff[step] != check_buff[step]) {
			ret = FLW_VERIFY_ERROR;
		}
	}

	return ret;
}

/**
 * flw_write_end - This function processes it by write_end command.
 * @return        - flash command
 */
RC flw_write_end( uchar* pbuff, ulong datasum)
{
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	RC ret = FLW_OK;
	ulong check_data;
	ulong data;

	/* write command error check and send packet error data check */
	if ((g_write_return == FLW_OK )|| 
		(UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]) != FLW_OK)) {
		/* data size check */
		if ( g_packet_data_size == g_data_size ) {
			check_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 4]);
			if (datasum == check_data ) {
				ret = FLW_OK;
			} else {
				ret = FLW_DEVICE_CHECKSUM_ERROR;
			}
		} else {
			ret = FLW_DEVICE_SEND_SIZE_ERROR;
		}
	} else if ( g_write_return != FLW_OK ) {
		ret = g_write_return;
	} else {
		ret = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);
	}

	/* make packet header */
	header.command	= src_header->command | SEND_CMD_ADD_DATA;
	header.sequence = src_header->sequence;
	header.status	= (ret != FLW_OK) ? FLW_ERROR : ret;
	header.length	= UL_CONV_ENDIAN(PACKET_RES_SIZE);

	data = UL_CONV_ENDIAN(ret);

	flw_create_packet(&header, &data, PACKET_RES_SIZE);

	flw_send_data(g_pSendBuff, PACKET_HEADER_SIZE+PACKET_RES_SIZE+PACKET_FOOTER_SIZE);

	g_next_phase = FLW_PHASE_NONE;

	return ret;
}

