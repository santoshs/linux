/*	flw_main.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "flw_main.h"
#include "sbsc.h"
#include "cpg.h"
#include "sysc.h"
#include "gpio.h"
#include "common.h"
#include "boot_init.h"
#include "compile_option.h"


#ifdef __UPDATER__
static const char MODE_STRING_UPDATER[]	= {"updater"}; 
const char lcd_str[] 		= "Download mode";
#else
const char lcd_str[] 		= "Flash Writer ";
#endif
const char lcd_start[] 		= "Start              ";
const char lcd_success[] 	= "Success            ";
const char lcd_error[] 		= "Error              ";
const char protect_error[] 	= "Write Protect Error";

uchar* g_pSendBuff = (uchar *)SEND_BUFF_ADDRESS;
uchar* g_pReadBuff = (uchar *)READ_BUFF_ADDRESS;
ulong g_next_phase = FLW_PHASE_NONE;
ulong g_wait = 0;
static int g_pre_state = FLW_DATA_COMPLETE;
static int g_rem_length = PACKET_HEADER_SIZE;
static int g_offset = 0;			/* receive short packet in data area */
static int g_checksum_type = 0; 	/* check sum type identifier */
static char g_checksum_type_idfy = 0;

extern void	IntDevUSBCheck(void);
extern int usb_get_startpos(void);
extern unsigned long gTransferCnt;
extern unsigned char gDmaReq;

/**
 * flw_main - flashwriter command wait routine
 * @return  - none
 */
void flw_main(void)
{
	uchar* pbuff = (uchar *)RECEIVE_BUFF_ADDRESS;
	ulong pre = 0;
	RC ret;
	ulong err = FLW_OK;

	uchar short_flg = 0;
	uchar tmp_sel = 0;

	g_wait = 0;
	
#ifdef __FW_LCD_ENABLE__
	uchar lcd_flg;
#endif	/* __FW_LCD_ENABLE__ */

	/* Flashwriter Initialize */
	flw_init();
	
	/* Check and erase NMV flag */
	ret = flw_check_nvm();
	if (FLW_OK != ret) {
		PRINTF("FAIL Updater can not check or erase NVM flag - ret=%d\n", ret);
	}

#ifdef __FW_LCD_ENABLE__
	/* Show on LCD */
	flw_load_logo();
#endif 		/* __FW_LCD_ENABLE__ */

    PRINTF("Updater initialize successfully\n");
#ifdef __FW_LCD_ENABLE__
	lcd_flg = LCD_START;
#endif 		/* __FW_LCD_ENABLE__ */

	PRINTF("Updater is READY\n");
	while(1) {

		ret = flw_wait_command(g_wait);
		tmp_sel = gTransferCnt % 2;
		if (ret == FLW_USB_RECV_DATA) {
			if (short_flg == 0) {
				if (tmp_sel == 0) {
					pbuff = (uchar*)(RECEIVE_BUFF_ADDRESS + usb_get_startpos());
				} else {
					pbuff = (uchar*)(RECEIVE_BUFF_ADDRESS_2ND + usb_get_startpos());
				}
			}
			ret = flw_receive_command(pbuff);

			if (ret == FLW_OK) {
				short_flg = 0;
				/* received all packet data  */
				err = flw_analyze_command(pbuff, &pre);
			} else if (ret == FLW_DATA_SHORT) {
				short_flg = 1;
				PRINTF("FAIL Updater can not receive all command packages - ret=%d\n", ret);
				/* if flash writer can not received all packet, run continue routine */
				continue;
			} else {
				short_flg = 0;
				PRINTF("FAIL Updater to receiving - ret=%d\n", ret);
				/* store error command */
				err = ret;
			}
		}

		if (ret == FLW_DATA_DISCONNECT) {
			PRINTF("USB disconnected. Reconnect...\n");
			flw_connect();
			pre = 0;
			continue;
		}

		/* make error command */
		if (err != FLW_OK) {
			PRINTF("FAIL Command is NG\n");
			if (g_wait != 0) {
				PRINTF("FAIL Updater reading error...Ended!\n");
				flw_read_end(FLW_ERROR);
			} else {
				flw_err_command(pbuff, err);
				PRINTF("FAIL Updater send info\n");
			}

#ifdef __FW_LCD_ENABLE__
			if (lcd_flg == LCD_START) {
				LCD_Print(LOC_X, LOC_Y_2, lcd_error);
				LCD_Draw_Cmode();
				lcd_flg |= LCD_END;
			}
#endif	/* __FW_LCD_ENABLE__ */
			usb_skip_startpos();
			continue;
		}

		/* execute command */
#ifdef __FW_LCD_ENABLE__
		if (lcd_flg == (LCD_START | LCD_END)) {
			LCD_Print(LOC_X, LOC_Y_2, "     ");
			LCD_Draw_Cmode();
			lcd_flg = LCD_START;
		}
#endif	/* __FW_LCD_ENABLE__ */

			gTransferCnt++;
			if (g_next_phase == FLW_PHASE_WRITE_DATA_CMD) {
				IntDevUSBCheck();
			}
			ret = flw_execute_command(pbuff, pre);

		if(ret != FLW_OK) {
			PRINTF("FAIL error when executing command!!! - ret=%d\n", ret);
#ifdef __FW_LCD_ENABLE__
			if (lcd_flg == LCD_START) {
				if (ret == FLW_WP_ERROR) {
					LCD_Print(LOC_X, LOC_Y_2, protect_error);
				} else {
					LCD_Print(LOC_X, LOC_Y_2, lcd_error);
				}

				LCD_Draw_Cmode();
				lcd_flg |= LCD_END;
			}
#endif	/* __FW_LCD_ENABLE__ */
		} else {
			/* continue */
		}

		if (g_next_phase == FLW_PHASE_END) {
			PRINTF("Updater END\n");
#ifdef __FW_LCD_ENABLE__
			if (lcd_flg == LCD_START) {
				LCD_Print(LOC_X, LOC_Y_2, lcd_success);
				LCD_Draw_Cmode();
				lcd_flg |= LCD_END;
			}
#endif	/* __FW_LCD_ENABLE__ */
		}

		usb_skip_startpos();
	}
}

/**
 * flw_init - flashwriter initiarize routine
 * @return  - none
 */
void flw_init(void)
{
#ifndef __UPDATER__
	/* Set Clock SYS-CPU(Z)=988MHz, SHwy(ZS)=208MHz, Peripheral(HP)=104M/Hz */
	CPU_CLOCK clock;
	if (CHIP_VERSION() == CHIP_RMU2_ES10) {
		clock = CPU_CLOCK_988MHZ;
	} else {
		clock = CPU_CLOCK_1456MHZ;
	}
	CPG_Set_Clock(clock);
	
	/* Boot setting Initialize*/
	Boot_Init();
	
#ifndef __INTEGRITY_CHECK_ENABLE__	
	/* Initialize SBSC */
	SBSC_Init();
#endif /* __INTEGRITY_CHECK_ENABLE__ */

	/* Initialize GPIO */
	GPIO_Init();

	/* Power supply */
	SYSC_Power_Supply();
	
#endif /* __UPDATER__ */

	usb_init((uchar*)RECEIVE_BUFF_ADDRESS, (uchar*)RECEIVE_BUFF_ADDRESS_2ND, (ulong)RECEIVE_BUFF_SIZE);

#ifdef __INTEGRITY_CHECK_ENABLE__
	/* Initialize InterconnectRAM */
	CPG_InterconnectRAM_Enable();
#endif /* __INTEGRITY_CHECK_ENABLE__ */

	/* usb initialize */
	flw_connect();

	/* flash initialize */
	Flash_Access_Init( UNUSED );

	/* serial initialize */
	serial_init();

}

/**
 * flw_wait_command - flashwriter command wait routine
 * @return          - none
 */
ulong flw_wait_command( ulong wait )
{
	RC ret;
	ulong count=0;

	for(count=0; ;count++)
	{
		if(count % WDT_RESET == 0) {
			WDT_CLEAR();
#ifdef __INTEGRITY_CHECK_ENABLE__
			Secure_WDT_clear();
#endif /* __INTEGRITY_CHECK_ENABLE__ */
		}

		if(g_next_phase == FLW_PHASE_END) {
			continue;
		}

		ret = usb_check();
		if(ret == USB_STATE_RECV_DATA) {
			ret = FLW_USB_RECV_DATA;
			return ret;
		} else if(ret == USB_STATE_DISCONNECTED) {
			ret = FLW_DATA_DISCONNECT;
			return ret;
		}

		if(wait != 0) {
			if(count > wait * MSEC)	{
				return FLW_TIME_OUT;
			}
		}
	}
}

/**
 * flw_receive_command - flashwriter command wait routine
 * @return             - result check receive data
 */
RC flw_receive_command( uchar* pbuff )
{
	CMD_HEADER* header;
	RC ret;
	ulong length;

	if(g_pre_state == FLW_DATA_COMPLETE || g_pre_state == FLW_DATA_HEADER_SHORT) {
		/* receive header data */
		ret = usb_receive( (uchar *)(pbuff+(PACKET_HEADER_SIZE-g_rem_length)), g_rem_length, 0);
		if(ret == g_rem_length)	{
			// none
		} else if(ret != g_rem_length && ret >= 0) {
			/* this routine is can not get all packet header */
			g_rem_length -= ret;
			g_pre_state = FLW_DATA_HEADER_SHORT;
			return FLW_DATA_SHORT;
		} else	{
			/* Disconnect */

			/* Parameter clear */
			g_pre_state = FLW_DATA_COMPLETE;
			g_rem_length = PACKET_HEADER_SIZE;
			g_offset = 0;

			return FLW_DATA_DISCONNECT;
		}
	}

	/* get data length */
	if(g_pre_state != FLW_DATA_SHORT) {
		header = (CMD_HEADER*)pbuff;
		length = UL_CONV_ENDIAN(header->length) + PACKET_FOOTER_SIZE;
	} else {
		/* data still usb buffer */
		length = g_rem_length;
	}

	if(length > DATA_MAX_SIZE + PACKET_FOOTER_SIZE) {
		/* Parameter clear */
		g_pre_state = FLW_DATA_COMPLETE;
		g_rem_length = PACKET_HEADER_SIZE;
		g_offset = 0;

		return FLW_COMMAND_PARAM_ERROR;
	}

	/* Data area stored */
	ret = usb_receive((uchar *)(pbuff + PACKET_HEADER_SIZE + g_offset), length, 0);
	/* check receive size */
	if(ret == length) {
		/* packet data all received */
		g_pre_state = FLW_DATA_COMPLETE;
		g_rem_length = PACKET_HEADER_SIZE;
		g_offset = 0;
		ret = FLW_OK;
	} else if(ret < length) {
		/* data still usb buffer */
		g_pre_state = FLW_DATA_SHORT;
		g_rem_length = length - ret;
		g_offset += ret;
		ret = FLW_DATA_SHORT;
	}

	return ret;
}


/**
 * flw_analyze_command - Analyze command routine
 * @return             - command data
 */
RC flw_analyze_command(uchar* pbuff ,ulong *pre)
{
	static ulong check_sequence = 0;
	RC ret;
	ulong sequence = 0;


	/* receive buffer command read */
	ulong command = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_CMD_POSITION]);
	ulong length = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_LEN_POSITION]);

	/* analyze command data */
	ret = flw_analyze_checksum( pbuff, length );

	if (command != RECEIVE_WRITE_CMD || command != *pre) {
		/* first command */
		check_sequence = 0;
	}

	sequence = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_SEQ_POSITION]);
	if ( check_sequence != sequence ) {
		ret = FLW_DEVICE_WRITE_SEQUENCE_ERROR;
	}

	check_sequence += 1;
	*pre = command;

	return ret;
}

/**
 * flw_execute_command - Execute command routine
 * @return             - command data
 */
RC flw_execute_command(uchar* pbuff, ulong exec)
{
	RC ret = FLW_OK;
	ulong length;
	length = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_LEN_POSITION]);

	ret = flw_check_phase(g_next_phase, exec);
	if (ret != FLW_OK) {
		PRINTF("FAIL checking phase is NG!!! - ret=%d\n", ret);
		flw_err_command(pbuff,ret);
		return ret;
	}

		switch( exec )
	{
	/* GET_STATUS command */
	case RECEIVE_GET_STATE_CMD:
		PRINTF("Command: GET_STATUS received\n");
		ret = flw_get_status(pbuff, length);
		break;
	/* check status command */
	case RECEIVE_CHECK_STATE_CMD:
		PRINTF("Command: check_status received\n");
		ret = flw_check_status(pbuff, length);
		break;
	/* set status command( unpopulated ) */
	case RECEIVE_SET_STATE_CMD:
		PRINTF("Command: set_status received\n");
		ret = flw_set_status(pbuff, length);
		break;
	/* write start command */
	case RECEIVE_WRITE_START_CMD:
		PRINTF("Command: write START\n");
		ret = flw_write_start(pbuff);
		break;
	/* write command */
	case RECEIVE_WRITE_CMD:
		PRINTF("Command: writing...\n");
		ret = flw_write(pbuff, length);	
		break;
	/* write end command */
	case RECEIVE_WRITE_END_CMD:
		PRINTF("Command: write END\n");
		ret = flw_write_end(pbuff, g_data_sum);
		break;
	/* mount command */
	case RECEIVE_MOUNT_CMD:
		PRINTF("Command: mount emmc\n");
		ret = flw_mount(pbuff, length);
		break;
	/* unmount command */
	case RECEIVE_UNMOUNT_CMD:
		PRINTF("Command: unmount emmc\n");
		ret = flw_unmount(pbuff, length);
		break;
	/* erase command */
	case RECEIVE_ERASE_CMD:
		PRINTF("Command: rasing data\n");
		ret = flw_erase(pbuff, length);
		break;
	/* format command */
	case RECEIVE_FORMAT_CMD:
		PRINTF("Command: formatting\n");
		ret = flw_format(pbuff, length);
		break;
	/* read command( unpopulated ) */
	case RECEIVE_READ_CMD:
		PRINTF("Command: reading data...\n");
		ret = flw_read(pbuff, length);
		break;
	/* clear proctect command */
	case RECEIVE_CLEAR_PROTECT_CMD:
		PRINTF("Command: Clear procect\n");
		ret = flw_clear_protect(pbuff, length);
		break;
	/* write proctect command */
	case RECEIVE_WRITE_PROTECT_CMD:
		PRINTF("Command: write procect\n");
		ret = flw_write_protect(pbuff, length);
		break;
	
	/* end command */
	case RECEIVE_END_CMD:
		PRINTF("Command: flashwriter is terminated\n");
		ret = flw_end(pbuff);
		break;
	/* Other commands return FLW_COMMAND_PARAM_ERROR. */
	default:
		PRINTF("Command: command is not supported\n");
		ret = FLW_COMMAND_PARAM_ERROR;
		flw_err_command(pbuff,FLW_COMMAND_PARAM_ERROR);
	}
	return ret;
}

/**
 * flw_check_status   - This function processes it by check_status command.
 * @return            - return code
 */
RC flw_check_status( uchar* pbuff, ulong length )
{
	/* return code */
	RC ret = FLW_OK;
	ulong class_data;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

	class_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);
	
	if ( class_data != FLW_BOOT_INFO_CHECK ) {
		ret = FLW_COMMAND_PARAM_ERROR;
	}

	/* make packet header */
	header.command  = src_header->command | SEND_CMD_ADD_DATA;
	header.sequence = src_header->sequence;
	header.status   = ret != FLW_OK ? FLW_ERROR : ret;
	header.length   = UL_CONV_ENDIAN(PACKET_RES_SIZE);
	data = UL_CONV_ENDIAN(ret);

	flw_create_packet(&header, &data, PACKET_RES_SIZE);

	flw_send_data(g_pSendBuff, PACKET_RES_SIZE+PACKET_HEADER_SIZE+PACKET_FOOTER_SIZE);

	g_next_phase = FLW_PHASE_NONE;

	return ret;
}

/**
 * flw_mount   - This function processes it by mount command.
 * @return     - return code
 */
RC flw_mount( uchar* pbuff, ulong length )
{
	/* return code */
	RC ret;
	ulong device_data;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

	device_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);
	
	if(device_data == TARGET_FLASH){
		/* flash mount */
		ret = Flash_Access_Mount( UNUSED );
		if(ret == FLASH_ACCESS_ERR_MOUNT){
			ret = FLW_DEVICE_MOUNT_ERROR;
		} else if(ret == FLASH_ACCESS_ERR_NOT_INIT)	{
			ret = FLW_DEVICE_NOT_INIT;
		} else if(ret == FLASH_ACCESS_ERR_ALREADY_MOUNT) {
			ret = FLW_DEVICE_ALREADY_MOUNT;
		} else {
			ret = FLW_OK;
		}
	} else {
		ret = FLW_COMMAND_PARAM_ERROR;
	}

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

/**
 * flw_unmount - This function processes it by unmount command.
 * @return     - return code
 */
RC flw_unmount( uchar* pbuff, ulong length )
{
	/* return code */
	RC ret;
	/* data buffer */
	ulong device_data;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

	device_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);

	if(device_data == TARGET_FLASH) {
		/* flash unmount */
		ret = Flash_Access_Unmount( UNUSED );
		if(ret == FLASH_ACCESS_ERR_NOT_INIT) {
			ret = FLW_DEVICE_NOT_INIT;
		} else if(ret == FLASH_ACCESS_ERR_NOT_MOUNT) {
			ret = FLW_DEVICE_NOT_MOUNT;
		} else {
			ret = FLW_OK;
		}
	} else {
		ret = FLW_COMMAND_PARAM_ERROR;
	}

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

/**
 * flw_format   - This function processes it by format command.
 * @return      - return code
 */
RC flw_format( uchar* pbuff, ulong length )
{
	/* return code */
	RC ret;
	ulong device_data;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

	device_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);

	if(device_data == TARGET_FLASH) {
		/* flash unmount */
		ret = Flash_Access_Format( UNUSED );
		if(ret == FLASH_ACCESS_ERR_NOT_INIT) {
			ret = FLW_DEVICE_NOT_INIT;
		} else if(ret == FLASH_ACCESS_ERR_FORMAT) {
			ret = FLW_DEVICE_FOMAT_ERROR;
		} else if(ret == FLASH_ACCESS_ERR_ALREADY_MOUNT) {
			ret = FLW_DEVICE_ALREADY_MOUNT;
		} else {
			ret = FLW_OK;
		}
	} else {
		ret = FLW_COMMAND_PARAM_ERROR;
	}

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

/**
 * flw_erase   - This function processes it by erase command.
 * @return     - return code
 */
RC flw_erase( uchar* pbuff, ulong length )
{
	/* return code */
	RC ret;
	ulong start_block;
	ulong end_block;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

	/* get usb data */
	start_block = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);

	/* get usb data */
	end_block = UL_CONV_ENDIAN(*(ulong*)&pbuff[20]);

	/* flash erase */
	ret = Flash_Access_Erase( start_block, end_block, UNUSED );
	
	if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
		ret = FLW_DEVICE_NOT_INIT;
	} else if (ret == FLASH_ACCESS_ERR_NOT_MOUNT) {
		ret = FLW_DEVICE_NOT_MOUNT;
	} else if (ret == FLASH_ACCESS_ERR_PARAM) {
		ret = FLW_DEVICE_PARAM_ERROR;
	} else if (ret == FLASH_ACCESS_ERR_ERASE) {
		ret = FLW_DEVICE_ERASE_ERROR;
	} else {
		ret = FLW_OK;
	}

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

/**
 * flw_set_status   - This function processes it by set_status command.
 * @return          - return code
 */
RC flw_set_status( uchar* pbuff, ulong length )
{
	RC ret;
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data;

	/* unpopulated function */
	ret = FLW_COMMAND_PARAM_ERROR;

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

/**
 * flw_get_status   - This function processes it by GET_STATUS command.
 * @return          - return code
 */
RC flw_get_status(uchar *pbuff, ulong length)
{
	CMD_HEADER header;
	ulong data;

	/* make packet header */
	header.command  = 0x000000B0;
	header.sequence = 0;
	header.status   = 0;
	header.length   = UL_CONV_ENDIAN(PACKET_RES_SIZE);
	data = 0x000000F0;

	flw_create_packet(&header, &data, PACKET_RES_SIZE);

	flw_send_data(g_pSendBuff,PACKET_RES_SIZE+PACKET_HEADER_SIZE+PACKET_FOOTER_SIZE);

	g_next_phase = FLW_PHASE_NONE;

	return FLW_OK;
}
/**
 * flw_analyze_checksum - This function analyzes the checksum.
 * @return              - return code
 */
RC flw_analyze_checksum( uchar *pbuff, ulong length )
{
	RC ret = FLW_OK;
	ulong check_data = 0;
	ulong check_data_tmp = 0;
	ulong header_checksum = 0;
	ulong data_checksum = 0;
	ulong check_sum_type = 0;

	check_data_tmp = (ulong)pbuff[length + PACKET_HEADER_SIZE] << 24;
	check_data |= check_data_tmp;
	check_data_tmp = (ulong)pbuff[length + PACKET_HEADER_SIZE + 1] << 16;
	check_data |= check_data_tmp;
	check_data_tmp = (ulong)pbuff[length + PACKET_HEADER_SIZE + 2] << 8;
	check_data |= check_data_tmp;
	check_data_tmp = (ulong)pbuff[length + PACKET_HEADER_SIZE + 3];
	check_data |= check_data_tmp;

	if (!g_checksum_type_idfy) {
		/* Check type of checksum (1 byte unit or 4 bytes unit */
		check_data_tmp = (ulong)pbuff[(length - 4) + PACKET_HEADER_SIZE] << 24;
		check_sum_type |= check_data_tmp;
		check_data_tmp = (ulong)pbuff[(length - 4) + PACKET_HEADER_SIZE + 1] << 16;
		check_sum_type |= check_data_tmp;
		check_data_tmp = (ulong)pbuff[(length - 4) + PACKET_HEADER_SIZE + 2] << 8;
		check_sum_type |= check_data_tmp;
		check_data_tmp = (ulong)pbuff[(length - 4) + PACKET_HEADER_SIZE + 3];
		check_sum_type |= check_data_tmp;
		g_checksum_type_idfy = 1;

		if (check_sum_type == CHECKSUM_TYPE_4BYTE) {
			g_checksum_type = CHECKSUM_TYPE_4BYTE;
		} else {
			g_checksum_type = CHECKSUM_TYPE_1BYTE;
		}
	}

	if (g_checksum_type == CHECKSUM_TYPE_4BYTE) {
		header_checksum = flw_calculate_checksum_4byte( &pbuff[0], PACKET_HEADER_SIZE );
		data_checksum = flw_calculate_checksum_4byte( &pbuff[PACKET_HEADER_SIZE], length );
	} else {
		header_checksum = flw_calculate_checksum_1byte( &pbuff[0], PACKET_HEADER_SIZE );
		data_checksum = flw_calculate_checksum_1byte( &pbuff[PACKET_HEADER_SIZE], length );
	}

	if( header_checksum + data_checksum != check_data )
	{
		ret = FLW_DEVICE_CHECKSUM_ERROR;
	}
	
	if( g_next_phase == FLW_PHASE_NONE) {
		g_data_sum = 0;
	} else if( g_next_phase == FLW_PHASE_WRITE_DATA_CMD) {
		g_data_sum += data_checksum;
	} 

	return ret;
}

/**
 * flw_calculate_checksum_1byte - This function calculates the checksum
 *		with checksum unit is 1byte
 * @return                - Result calculate data.
 */
ulong flw_calculate_checksum_1byte(uchar* pbuff, ulong length )
{
	ulong calculate_sum = 0;
	ulong loop;

	for( loop = 0; loop < length; loop++ )
	{
		/* check sum */
		calculate_sum = calculate_sum + pbuff[loop];
	}

	return calculate_sum;
}

/**
 * flw_calculate_checksum_4byte - This function calculates the checksum
 * 		with checksum unit is 4byte
 * @return                - Result calculate data.
 */
ulong flw_calculate_checksum_4byte(uchar* pbuff, ulong length )
{
	ulong calculate_sum = 0;
	ulong loop = 0;
	ulong remain_len;
	ulong calc_length;
	ulong *pbuff_long;

	remain_len = length % 4;
	calc_length = (length - remain_len)/4;

	pbuff_long = (unsigned long *)pbuff;
	while (loop < calc_length) {
		calculate_sum += pbuff_long[loop];
		loop++ ;
	}

	if (remain_len == 3) {
		calculate_sum += (pbuff_long[calc_length] & 0x00FFFFFF);
	} else if (remain_len == 2) {
		calculate_sum += (pbuff_long[calc_length] & 0x0000FFFF);
	} else if (remain_len == 1) {
		calculate_sum += (pbuff_long[calc_length] & 0x000000FF);
	}

	return calculate_sum;
}

/**
 * flw_connect - This function connects USB.
 * @return     - None.
 */
void flw_connect( void )
{
	USB_STATE state;
	volatile ulong count=0;

	usb_close();
	/* WDT clear */
	WDT_CLEAR();
#ifdef __INTEGRITY_CHECK_ENABLE__
	Secure_WDT_clear();
#endif /* __INTEGRITY_CHECK_ENABLE__ */
	
	for (count=0; count<0x100000; count++) {
		if (count % WDT_RESET == 0) {
			WDT_CLEAR();
#ifdef __INTEGRITY_CHECK_ENABLE__
			Secure_WDT_clear();
#endif /* __INTEGRITY_CHECK_ENABLE__ */
		}
	}

	count = 0;

	do {
		
		if(count % WDT_RESET == 0) {
			WDT_CLEAR();
#ifdef __INTEGRITY_CHECK_ENABLE__
			Secure_WDT_clear();
#endif /* __INTEGRITY_CHECK_ENABLE__ */
			count = 0;
		}
		count++;
		state = usb_check();
	} while ( state != USB_STATE_CLOSE );

	/* initialize global data  */
	g_next_phase = FLW_PHASE_NONE;
	g_pre_state = FLW_DATA_COMPLETE;
	g_rem_length = PACKET_HEADER_SIZE;
	g_offset = 0;

	usb_open();
}

/**
 * flw_end     - This function processes it by end command.
 * @return     - return code
 */
RC flw_end( uchar* pbuff )
{
	RC ret = FLW_OK;
	/* check type data */
	ulong type_data = UL_CONV_ENDIAN(*(ulong*)&pbuff[PACKET_RET_CODE_POSITION]);

	g_next_phase = FLW_PHASE_NONE;

	if (type_data == FLW_END) {
		/* go to end routine(only loop) */
		usb_close();
		g_next_phase = FLW_PHASE_END;
	} else if( type_data == FLW_RESET_END ) {
		Reset();
	} else if( type_data == FLW_FLASHING_MODE_END ) {
		/* Flashing mode */
		ChangeFlashingMode();
	}

	return ret;
}

/**
 * flw_send_data - This function processes send deta for PC..
 * @return       - return code
 */
RC flw_send_data( uchar *pbuff, ulong length )
{
	RC ret = FLW_OK;
	
	ret = usb_send( pbuff, length , 5000 );
	
	if (ret == USB_ERR_PARAM) {
		ret = FLW_DEVICE_PARAM_ERROR;
	}
	return ret;
}

/**
 * flw_err_command - This function processes it by send error command.
 * @return         - none
 */
void flw_err_command( uchar* pbuff, ulong err)
{
	CMD_HEADER header;
	CMD_HEADER* src_header = (CMD_HEADER*)pbuff;
	ulong data[3];		/* write_data response =12byte , other 4byte */
	ulong data_length = 0;

	memset(data, 0x00, sizeof(data));

	header.command	= src_header->command | SEND_CMD_ADD_DATA;
	header.sequence = src_header->sequence;
	header.status	= err != FLW_OK ? (ulong)FLW_ERROR : err;
	data_length		= src_header->command == RECEIVE_WRITE_CMD ? PACKET_WRITE_DATA_RES_SIZE : PACKET_RES_SIZE;
	header.length	= UL_CONV_ENDIAN(data_length);

	data[0] = UL_CONV_ENDIAN(err);

	flw_create_packet(&header, &data[0], data_length);

	flw_send_data(g_pSendBuff, data_length+PACKET_HEADER_SIZE);
}

/**
 * flw_create_packet - This function processes it by create packet.
 * @return           - none
 */
void flw_create_packet(CMD_HEADER* header, ulong* data, ulong length)
{
	CMD_HEADER* send_header = (CMD_HEADER*)&g_pSendBuff[0];

	/* make packet header */
	send_header->command	= header->command;
	send_header->sequence	= header->sequence;
	send_header->status		= header->status;
	send_header->length		= header->length;

	memcpy(&g_pSendBuff[PACKET_HEADER_SIZE], data, length);

		if (g_checksum_type == CHECKSUM_TYPE_4BYTE) {
		*(ulong*)&g_pSendBuff[length+PACKET_HEADER_SIZE] 
				= UL_CONV_ENDIAN(flw_calculate_checksum_4byte(g_pSendBuff, length+PACKET_HEADER_SIZE));
	} else {
		*(ulong*)&g_pSendBuff[length+PACKET_HEADER_SIZE] 
				= UL_CONV_ENDIAN(flw_calculate_checksum_1byte(g_pSendBuff, length+PACKET_HEADER_SIZE));
	}
}

/**
 * flw_check_phase - This function check phase.
 * @return         - return code
 */
RC flw_check_phase(ulong expect, ulong now)
{
	ulong tmp_expect_no=0, tmp_expect_head=0;
	/***
		phase pattern
		0x0000---- : normal.
		0xFFFF---- : error occured.
	***/

	if(expect == FLW_PHASE_NONE) {
		/* write data command and write end command permit in write phase*/
		if(now == RECEIVE_WRITE_CMD || now == RECEIVE_WRITE_END_CMD) {
			return FLW_PHASE_ERROR;
		}
		return FLW_OK;
	}

	tmp_expect_no = expect & FLW_PHASE_MASK_DATA;
	if (tmp_expect_no == now) {
		return FLW_OK;
	}

	tmp_expect_head = expect & FLW_PHASE_ERROR_MASK;
	if (tmp_expect_head == FLW_PHASE_ERROR_MASK) {
		/* if an error occured in write_data command. permit write end command */
		if (tmp_expect_no == RECEIVE_WRITE_CMD && now == RECEIVE_WRITE_END_CMD) {
			return FLW_OK;
		}
	}

	return FLW_PHASE_ERROR;
}

#ifdef __FW_LCD_ENABLE__
/**
 * flw_load_logo - Load logo to LCD
 * @return None
 */
void flw_load_logo(void)
{
#ifndef __UPDATER__
	int ret;
	ulong  logo_addr = LOGO_RAM_ADDR;
	
	/* LCD Display Show */
	LCD_Init();
	ret = LCD_Check_Board();
	if (ret == LCD_SUCCESS) {
		LCD_Clear( LCD_COLOR_WHITE );
		LCD_Set_Text_color(LCD_COLOR_RED);
		LCD_Display_On();
	}
	
	/* flash memory mount */
	ret = Flash_Access_Mount(UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		PRINTF("FAIL to mount eMMC - ret=%d\n", ret);
	}
	
	/* flash read (load module) */
	ret = Flash_Access_Read((uchar *)logo_addr, (ulong)(LOGO_BUFF_SIZE), (uint64)(LOGO_eMMC_ADDR), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		PRINTF("FAIL to read eMMC - ret=%d\n", ret);
	}

	/* flash memory unmount */
	ret = Flash_Access_Unmount( UNUSED );
	if (ret != FLASH_ACCESS_SUCCESS) {
		PRINTF("FAIL to unmount eMMC - ret=%d\n", ret);
	}

	if (ret != FLASH_ACCESS_SUCCESS) {
		LCD_Print(LOC_X, LOC_Y, "Renesas Mobile");
		LCD_Print(LOC_X, LOC_Y_2, lcd_start);
		LCD_Draw_Cmode();
		LCD_Backlight_On();
		return;
	}

	/* Check image is valid or not */
	ret = LCD_BMP_Draw( LOGO_X, LOGO_Y, LOGO_WIDTH, LOGO_HIGH, logo_addr);
	if (ret != LCD_SUCCESS ) {
		PRINTF("FAIL to draw Bitmap file - ret=%d\n", ret);
		LCD_Print(LOC_X, LOC_Y, "Renesas Mobile");
	}
#else
	LCD_Init();	
#endif /* __UPDATER__ */
	LCD_Print(LOC_X, LOC_Y_1, lcd_str);
	LCD_Print(LOC_X, LOC_Y_2, lcd_start);
	LCD_Draw_Cmode();
	LCD_Backlight_On();
}
#endif 	/* __FW_LCD_ENABLE__ */

/**
 * flw_check_nvm - Check and clear NVM flag
 * @return None
 */
RC flw_check_nvm(void)
{
#ifdef __UPDATER__
	int ret;
	uchar nvm_flag[BOOTFLAG_SIZE];
	
	/* flash memory mount */
	ret = Flash_Access_Mount(UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		return ret;
	}
	
	/* flash read */
	ret = Flash_Access_Read((uchar *)(&nvm_flag), (ulong)(BOOTFLAG_SIZE), (uint64)(BOOTFLAG_ADDR), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS) {
		return ret;
	}
	
	/* check the boot flag */
	if (strncmp(((const char *)(nvm_flag)), MODE_STRING_UPDATER, BOOTFLAG_SIZE) == 0) {
		/* erase NVM boot flag */
		memset((void *)&nvm_flag, 0x00, BOOTFLAG_SIZE);
		ret = Flash_Access_Write((uchar *)(&nvm_flag), (ulong)(BOOTFLAG_SIZE), (uint64)(BOOTFLAG_ADDR), UNUSED);
		if (ret != FLASH_ACCESS_SUCCESS) {
			return ret;
		}
		PRINTF("Updater erases NVM flag\n");
	}
	
	/* flash memory unmount */
	ret = Flash_Access_Unmount( UNUSED );
	if (ret != FLASH_ACCESS_SUCCESS) {
		return ret;
	}
	
	
#endif /* __UPDATER__ */
	return FLW_OK;
}
