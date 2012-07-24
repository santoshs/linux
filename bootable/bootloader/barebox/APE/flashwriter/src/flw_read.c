/*	flw_read.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "flw_main.h"

//Client Param
static ulong g_read_class;
static uint64 g_start_address;
static ulong g_data_size;
static ulong g_user_data;

static ulong g_sequence;
static ulong g_src_header_command;
static int g_read_return;
static ulong g_packet_data_size;

extern ulong flash_Get_Sector(ulong block, uint64 address, ulong data_size);
extern unsigned long Emmc_Get_Max_Sector_Count(void);

ulong g_data_sum;
uchar* pReadRamBuff;
ulong g_read_checksum_type = 0;

/**
 * flw_read        - This function processes it by read command.
 * @return         - flash command
 */
RC flw_read( uchar* pbuff, ulong length )
{
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	RC ret= FLW_OK;
	union {
		ulong data32[2];
		uint64 data64;
	} address;
	
	ulong start_add_sector = 0;
	ulong data_size_sector = 0;

	if( g_wait == 0)
	{
		g_packet_data_size = 0;
		g_read_class = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);
		address.data32[1] = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 4]);
		address.data32[0] = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 8]);
		g_start_address = address.data64;
		g_data_size = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 12]);
		g_user_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 16]);
		g_read_checksum_type = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION + 20]);

		/* check input data size */
		start_add_sector = flash_Get_Sector(0, g_start_address, 0);
		data_size_sector = flash_Get_Sector(0, 0, g_data_size);
		if(Emmc_Get_Max_Sector_Count() < (start_add_sector + data_size_sector)) {
			g_read_return = FLW_DEVICE_PARAM_ERROR;
			flw_read_end(g_read_return);
			return FLW_COMMAND_PARAM_ERROR;
		}
	
		/* global data initialize */
		g_data_sum = 0;
		g_read_return = FLW_OK;
		g_sequence = 0;
		
		g_src_header_command = src_header->command;
		g_wait = 1;
	}

	if(g_data_size > 0) {
		ret = flw_read_data();
	}else{
		ret = flw_read_end(g_read_return);
	}
	
	if(ret != FLW_OK){
		flw_read_end(g_read_return);
	}

	return ret;
}

/**
 * flw_read_data  - This function processes it by read command.
 * @return        - flash command
 */
RC flw_read_data( void )
{
	CMD_HEADER header;
	RC ret= FLW_OK;
	ulong read_length;
	ulong checksum_data;
	union {
		ulong data32[2];
		uint64 data64;
	} address;
	ulong reg_addr;
	ulong temp_data;

	read_length = g_data_size > DATA_MAX_SIZE ? DATA_MAX_SIZE : g_data_size;
	
	if (g_read_class == FLW_WRITE_RAM_AREA) {
		address.data64 = g_start_address;
		memcpy(&g_pReadBuff[PACKET_HEADER_SIZE],(void*)(address.data32[0]),read_length);
	} else if ( g_read_class == FLW_WRITE_RAM_AREA_REG ) {
		address.data64 = g_start_address;
		reg_addr = address.data32[0];
		temp_data = *((ulong *)reg_addr);
		memcpy(&g_pReadBuff[PACKET_HEADER_SIZE],(void*)&(temp_data),read_length);
	} else {
		/* flash read */
		ret = Flash_Access_Read( &g_pReadBuff[PACKET_HEADER_SIZE], read_length, g_start_address, g_user_data);

		if ( ret == FLASH_ACCESS_ERR_PARAM ) {
			ret = FLW_DEVICE_PARAM_ERROR;
		} else if ( ret == FLASH_ACCESS_ERR_NOT_MOUNT ) {
			ret = FLW_DEVICE_MOUNT_ERROR;
		} else if ( ret == FLASH_ACCESS_ERR_NOT_INIT) {
			ret = FLW_DEVICE_INIT_ERROR;
		} else if ( ret == FLASH_ACCESS_ERR_READ ) {
			ret = FLW_DEVICE_READ_ERROR;
		} else if ( ret == FLASH_ACCESS_SUCCESS ) {
			ret = FLW_OK;
		} else {
			ret = FLW_COMMAND_PARAM_ERROR;
		}
	}

	if (ret == FLW_OK) {
		PRINTF ("         send...");
		g_start_address += read_length;
		g_data_size -= read_length;
		if (g_read_checksum_type == CHECKSUM_TYPE_4BYTE) {
			g_data_sum += flw_calculate_checksum_4byte(&g_pReadBuff[PACKET_HEADER_SIZE], read_length);

		} else {
			g_data_sum += flw_calculate_checksum_1byte(&g_pReadBuff[PACKET_HEADER_SIZE], read_length);
		}
		header.command	= g_src_header_command | SEND_CMD_ADD_DATA;
		header.sequence = UL_CONV_ENDIAN(g_sequence);
		g_sequence++;
		header.status	= ret != FLW_OK ? FLW_ERROR : ret;
		header.length	= UL_CONV_ENDIAN(read_length);

		memcpy(&g_pReadBuff[0], &header, PACKET_HEADER_SIZE);
		if (g_read_checksum_type == CHECKSUM_TYPE_4BYTE) {
			checksum_data 	= UL_CONV_ENDIAN(flw_calculate_checksum_4byte(g_pReadBuff, PACKET_HEADER_SIZE + read_length));
		} else {
			checksum_data 	= UL_CONV_ENDIAN(flw_calculate_checksum_1byte(g_pReadBuff, PACKET_HEADER_SIZE + read_length));
		}
		memcpy(&g_pReadBuff[PACKET_HEADER_SIZE + read_length], &checksum_data, PACKET_FOOTER_SIZE);

		flw_send_data(g_pReadBuff,PACKET_HEADER_SIZE + read_length + PACKET_FOOTER_SIZE);
		PRINTF ("         OK\n");
	}

	g_read_return = ret;
	return ret;
}

/**
 * flw_read_end   - This function processes it by read_end command.
 * @return        - flash command
 */
RC flw_read_end( int err )
{
	CMD_HEADER header;
	ulong data[2];
	
	/* make packet header */
	header.command	= UL_CONV_ENDIAN(RECEIVE_READ_CMD_END) | SEND_CMD_ADD_DATA;
	header.sequence = 0;
	header.status   = UL_CONV_ENDIAN(err);
	header.length	= UL_CONV_ENDIAN(PACKET_READ_END_RES_SIZE);

	data[0] = UL_CONV_ENDIAN(err);
	data[1] = UL_CONV_ENDIAN(g_data_sum);

	memcpy(&g_pReadBuff[0],&header,PACKET_HEADER_SIZE);
	memcpy(&g_pReadBuff[PACKET_HEADER_SIZE],&data,PACKET_READ_END_RES_SIZE);
	
	if (g_read_checksum_type == CHECKSUM_TYPE_4BYTE) {
		*(ulong*)&g_pReadBuff[PACKET_HEADER_SIZE+PACKET_READ_END_RES_SIZE] 
				= UL_CONV_ENDIAN(flw_calculate_checksum_4byte(g_pReadBuff, PACKET_HEADER_SIZE+PACKET_READ_END_RES_SIZE));
	} else {
		*(ulong*)&g_pReadBuff[PACKET_HEADER_SIZE+PACKET_READ_END_RES_SIZE] 
				= UL_CONV_ENDIAN(flw_calculate_checksum_1byte(g_pReadBuff, PACKET_HEADER_SIZE+PACKET_READ_END_RES_SIZE));
	}

	flw_send_data(g_pReadBuff,PACKET_HEADER_SIZE+PACKET_READ_END_RES_SIZE+PACKET_FOOTER_SIZE);

	g_next_phase = FLW_PHASE_NONE;

	g_wait = 0;
	PRINTF("Read ended SUCCESSFULLY");
	
	return FLW_OK;
}
