/*	flw_main.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __FLW_MAIN_H__
#define __FLW_MAIN_H__

#include "log_output.h"

#include "string.h"
#include "flash_api.h"
#include "usb_api.h"
#include "common.h"
#include "com_type.h"
#include "com_api.h"

#ifdef __FW_LCD_ENABLE__
#include "lcd_api.h"
#endif	/* __FW_LCD_ENABLE__ */

/************************************************************************************************/
/*	Definitions																					*/
/************************************************************************************************/
/* detail return code parameter */
#define FLW_COMMAND_PARAM_ERROR			-1
#define FLW_COMMAND_STATUS_ERROR		-2
#define FLW_COMMAND_SETTING_ERROR		-3
#define FLW_USB_ERROR					-101
#define FLW_DEVICE_PARAM_ERROR			-201
#define FLW_DEVICE_NOT_MOUNT			-202
#define FLW_DEVICE_ALREADY_MOUNT		-203
#define FLW_DEVICE_MOUNT_ERROR			-204
#define FLW_DEVICE_READ_ERROR			-205
#define FLW_DEVICE_WRITE_ERROR			-206
#define FLW_DEVICE_SEND_SIZE_ERROR		-207
#define FLW_DEVICE_ERASE_ERROR			-208
#define FLW_DEVICE_FOMAT_ERROR			-209
#define FLW_DEVICE_INIT_ERROR 			-210
#define FLW_DEVICE_NOT_INIT				-211
#define FLW_DEVICE_CHECKSUM_ERROR		-212
#define FLW_PHASE_ERROR					-213
#define FLW_DEVICE_WRITE_SEQUENCE_ERROR -214
#define FLW_DEVICE_VERIFY_ERROR			-215
#define FLW_DEVICE_VERIFY_READ_ERROR    -216
#define FLW_DEVICE_CLEAR_PROTECT_ERROR  -217
#define FLW_DEVICE_WRITE_PROTECT_ERROR  -218
#define FLW_SECURITY_KEY_ERROR			-301
#define FLW_SECURITY_CHECK_ERROR		-302

/* command parameter */
#define FLW_OK		0
#define FLW_ERROR	-1
#define FLW_WP_ERROR	-2

/* receive data parameter */
#define FLW_DATA_DISCONNECT		1
#define FLW_DATA_SHORT			2
#define FLW_DATA_HEADER_SHORT	3
#define FLW_DATA_COMPLETE		4
#define FLW_TIME_OUT			10
#define FLW_USB_RECV_DATA		11

/* checsum type */
#define CHECKSUM_TYPE_4BYTE		1
#define CHECKSUM_TYPE_1BYTE		0

/* other parameter */
#define MSEC		1
#define WDT_RESET	150

/* packet data */
#define PACKET_HEADER_SIZE			16
#define PACKET_FOOTER_SIZE			4
#define DATA_MAX_SIZE				(512*1024)
#define PACKET_WRITE_DATA_RES_SIZE	12
#define PACKET_RES_SIZE				4
#define PACKET_READ_END_RES_SIZE	8

/* command */
#define RECEIVE_CHECK_STATE_CMD		0x0010
#define RECEIVE_SET_STATE_CMD		0x0011
#define RECEIVE_WRITE_START_CMD		0x0020
#define RECEIVE_WRITE_CMD			0x0021
#define RECEIVE_WRITE_END_CMD		0x0022
#define RECEIVE_READ_CMD			0x0023
#define RECEIVE_READ_CMD_END		0x0024
#define RECEIVE_MOUNT_CMD			0x0030
#define RECEIVE_UNMOUNT_CMD			0x0031
#define RECEIVE_ERASE_CMD			0x0032
#define RECEIVE_FORMAT_CMD			0x0033
#define RECEIVE_CLEAR_PROTECT_CMD	0x0040
#define RECEIVE_WRITE_PROTECT_CMD	0x0041
#define RECEIVE_END_CMD				0x0000
#define RECEIVE_GET_STATE_CMD		0x00B0		/* Command GET_STATUS */

/* end command parameter */
#define FLW_END						0x0000
#define FLW_RESET_END				0x0001
#define FLW_FLASHING_MODE_END		0x0002

/* phase */
#define FLW_PHASE_NONE				0
#define FLW_PHASE_WRITE_DATA_CMD	RECEIVE_WRITE_CMD
#define FLW_PHASE_WRITE_END_CMD		RECEIVE_WRITE_END_CMD
#define FLW_PHASE_END				0xFFFFFFFF

#define FLW_PHASE_MASK_DATA			0x0000FFFF
#define FLW_PHASE_ERROR_MASK		0xFFFF0000

/* return command ( big endian data ) */
#define SEND_CMD_ADD_DATA			(ulong)0x00000080

#define SEND_CHECK_STATE_CMD		(RECEIVE_CHECK_STATE_CMD	+ SEND_CMD_ADD_DATA)
#define SEND_SET_STATE_CMD			(RECEIVE_SET_STATE_CMD		+ SEND_CMD_ADD_DATA)
#define SEND_WRITE_START_CMD		(RECEIVE_WRITE_START_CMD	+ SEND_CMD_ADD_DATA)
#define SEND_WRITE_CMD				(RECEIVE_WRITE_CMD			+ SEND_CMD_ADD_DATA)
#define SEND_WRITE_END_CMD			(RECEIVE_WRITE_END_CMD		+ SEND_CMD_ADD_DATA)
#define SEND_MOUNT_CMD				(RECEIVE_MOUNT_CMD			+ SEND_CMD_ADD_DATA)
#define SEND_UNMOUNT_CMD			(RECEIVE_UNMOUNT_CMD		+ SEND_CMD_ADD_DATA)
#define SEND_ERASE_CMD				(RECEIVE_ERASE_CMD			+ SEND_CMD_ADD_DATA)
#define SEND_FORMAT_CMD				(RECEIVE_FORMAT_CMD			+ SEND_CMD_ADD_DATA)
#define SEND_READ_CMD				(RECEIVE_READ_CMD			+ SEND_CMD_ADD_DATA)
#define SEND_READ_CMD_END			(RECEIVE_READ_CMD_END		+ SEND_CMD_ADD_DATA)
#define SEND_END_CMD				(RECEIVE_END_CMD			+ SEND_CMD_ADD_DATA)

/* status check definition */
#define FLW_BOOT_INFO_CHECK			0x0001

/* write area definition */
#define FLW_WRITE_RAM_AREA			0x0000
#define FLW_WRITE_FLASH_AREA		0x0010

#define FLW_WRITE_RAM_AREA_REG		0x0100


/* target device definition */
#define TARGET_FLASH				0x0010

/* packet devider definition */
#define PACKET_CMD_POSITION			0
#define PACKET_SEQ_POSITION			4
#define PACKET_STATE_POSITION		8
#define PACKET_LEN_POSITION			12
#define PACKET_RET_CODE_POSITION	16

/* send/receive buffer */
#define RECEIVE_BUFF_ADDRESS		(0x4F000000)
#define RECEIVE_BUFF_ADDRESS_2ND	(0x4F800000)
#define SEND_BUFF_ADDRESS			(0x48000000)
#define READ_BUFF_ADDRESS			(0x40000000)
#define RECEIVE_BUFF_SIZE			(0x000D2000)

/* GPIO */
#define GPIO_BASE				(0xE6050000ul)
#define GPIO_ULPICR				((volatile ushort *)(GPIO_BASE + 0x8196))
#define GPIO_ULPICR_DATA		(0x0003)			/* Max drivability */

/* SYS Wakeup control register */
#define SWUCR						((volatile ulong *)(0xE6180014))

/* LCD */
#define LCD_CLEAR				0
#define LCD_START				1
#define LCD_END					2
#define LCD_EXIST				0
#define LCD_NON_EXIST			1

/************************************************************************************************/
/*	type																						*/
/************************************************************************************************/
typedef struct _CMD_HEADER_
{
	ulong command;
	ulong sequence;
	ulong status;
	ulong length;
}CMD_HEADER;

/************************************************************************************************/
/*	Global Data																					*/
/************************************************************************************************/
extern uchar* g_pSendBuff;
extern uchar* g_pReadBuff;
extern ulong g_next_phase;
extern ulong g_wait;
extern ulong g_data_sum;

/************************************************************************************************/
/*	Prototypes																					*/
/************************************************************************************************/
void flw_main(void);
void flw_init(void);
ulong flw_wait_command( ulong wait );
RC flw_receive_command( uchar* pbuff );
RC flw_analyze_command( uchar * pbuff,ulong *pre);
RC flw_execute_command(uchar* pbuff, ulong exec);
RC flw_check_status( uchar* pbuff, ulong length );
RC flw_mount( uchar* pbuff, ulong length );
RC flw_unmount( uchar* pbuff, ulong length );
RC flw_format( uchar* pbuff, ulong length );
RC flw_erase( uchar* pbuff, ulong length );
RC flw_clear_protect( uchar* pbuff, ulong length );
RC flw_write_protect( uchar* pbuff, ulong length );
RC flw_get_status(uchar* pbuff, ulong length);
RC flw_set_status( uchar* pbuff, ulong length );
RC flw_analyze_checksum( uchar *pbuff, ulong length );
ulong flw_calculate_checksum_1byte( uchar* pbuff, ulong length );
ulong flw_calculate_checksum_4byte( uchar* pbuff, ulong length );
void flw_connect( void );
RC flw_end( uchar * pbuff );
RC flw_send_data( uchar * pbuff, ulong length );
void flw_err_command( uchar* pbuff, ulong err);
RC flw_check_phase(ulong expect, ulong now);

RC flw_write_start( uchar* pbuff);
RC flw_write( uchar* pbuff, ulong length );
RC flw_write_data( uchar* pbuff, ulong length );
RC flw_write_end( uchar* pbuff, ulong datasum);
RC flw_write_verify( uchar* pbuff, ulong length);

RC flw_read( uchar* pbuff, ulong length );
RC flw_read_data( void );
RC flw_read_end( int err );

RC flw_check_nvm( void );

void flw_create_packet( CMD_HEADER* header, ulong* data, ulong length);

#ifdef __INTEGRITY_CHECK_ENABLE__
extern RC Secure_WDT_clear(void);
#endif /* __INTEGRITY_CHECK_ENABLE__ */

#ifdef __FW_LCD_ENABLE__
void flw_load_logo(void);
#endif	/* __FW_LCD_ENABLE__ */

#endif /* __FLW_MAIN_H__ */
