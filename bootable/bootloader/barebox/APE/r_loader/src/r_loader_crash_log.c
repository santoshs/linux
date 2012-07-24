/*
 * r_loader_crash_log.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "r_loader_boot_log.h"
#include "string.h"
#include "r_loader.h"
#include "flash_api.h"
#include "log_output.h"
#include "r_loader_crash_log.h"

const reset_log_area_info area_info[5] = {
	{ KMSG_OFFSET,          KMSG_SIZE },
	{ LOGCAT_MAIN_OFFSET,   LOGCAT_MAIN_SIZE },
	{ LOGCAT_SYSTEM_OFFSET, LOGCAT_SYSTEM_SIZE },
	{ LOGCAT_RADIO_OFFSET,  LOGCAT_RADIO_SIZE},
	{ LOGCAT_EVENTS_OFFSET, LOGCAT_EVENTS_SIZE}
};

/* prototype for Save_Crash_Log_To_eMMC */
static void Save_Reset_Log_To_eMMC(BOOT_LOG *boot_log, unsigned int boot_log_offset);
static void Save_Temporary_Log_To_eMMC(BOOT_LOG *boot_log);
static RC Save_Kmsg_To_eMMC(uchar *log_buf, int log_buf_len, ulong log_end, ulong logged_chars, ulong offset);
static RC Save_Logcat_To_eMMC(dump_log_type type, uchar *buffer, int size, int w_off, int head, ulong offset);
static RC Write_Logs_To_eMMC(volatile ulong emmc_addr, const char *s1, ulong l1,const char *s2, ulong l2, ulong max_size);

static volatile unsigned int gResetLogOffset = 0;


/**
 * Save_Reset_Log_To_eMMC - Save Reset Log to eMMC
 * @return none
 */
static void Save_Reset_Log_To_eMMC(BOOT_LOG *boot_log, unsigned int boot_log_offset)
{
	RC ret = FLASH_ACCESS_SUCCESS;
	unsigned int  reset_log_offset = 0;	
	
	if(boot_log == NULL) {
		return;
	}
	
	if((boot_log->stbchr2 & APE_RESETLOG_INIT_COMPLETE) == 0) {
		return;
	}
	
	if((boot_log->stbchr3 & APE_RESETLOG_DEBUG) == APE_RESETLOG_DEBUG) {
		;
	} else if((boot_log->boot_matrix.srstfr & SRSTFR_RCWD0) == SRSTFR_RCWD0) {
		if(boot_log->stbchr2 & APE_RESETLOG_PANIC_END) {
			return;
		}
	}
	
	ret = Flash_Access_Read((uchar* )&reset_log_offset, sizeof(unsigned int), RESET_LOG_COUNTER_ADDRESS, UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS)return;

	// initialize
	if(reset_log_offset >= RESET_LOG_NUM)
	{
		reset_log_offset = 0;
	}

	/* write header info 1 */
	{
		gResetLogOffset = reset_log_offset;
		reset_log_header_info header;
		header.counter = boot_log_offset;
		memcpy(&header.boot_info, boot_log, sizeof(BOOT_LOG));
		Flash_Access_Write((uchar *)&header, sizeof(header), RESET_LOG_ADDRESS + RESET_LOG_SIZE * reset_log_offset, UNUSED);
	}

	/* Dump kmsg */
	do {
		uchar *logbuf      = (uchar*)*KMSG_LOGBUF_ADDRESS;
		int   *logbuflen   = (int*)*KMSG_LOGBUFLEN_ADDRESS;
		ulong *logend      = (ulong*)*KMSG_LOGEND_ADDRESS;
		ulong *loggedchars = (ulong*)*KMSG_LOGGEDCHARS_ADDRESS;

		if( ! IS_ON_SDRAM(logbuflen,   sizeof(int)   )) break;
		if( ! IS_ON_SDRAM(logend,      sizeof(ulong) )) break;
		if( ! IS_ON_SDRAM(loggedchars, sizeof(ulong) )) break;

		Save_Kmsg_To_eMMC(logbuf, *logbuflen, *logend, *loggedchars, reset_log_offset);
	} while(0);

	/* Dump logcat main */
	do {
		uchar* pbuf   = (uchar*)*LOGCAT_MAIN_BUFFER_ADDRESS;
		size_t* size  = (size_t*)*LOGCAT_MAIN_SIZE_ADDRESS;
		size_t* woff  = (size_t*)*LOGCAT_MAIN_WOFF_ADDRESS;
		size_t* head  = (size_t*)*LOGCAT_MAIN_HEAD_ADDRESS;

		if( ! IS_ON_SDRAM(size, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(woff, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(head, sizeof(size_t) )) break;

		Save_Logcat_To_eMMC(type_main, pbuf, *size, *woff, *head, reset_log_offset);
	} while(0);

	/* Dump logcat system */
	do {
		uchar* pbuf   = (uchar*)*LOGCAT_SYSTEM_BUFFER_ADDRESS;
		size_t* size  = (size_t*)*LOGCAT_SYSTEM_SIZE_ADDRESS;
		size_t* woff  = (size_t*)*LOGCAT_SYSTEM_WOFF_ADDRESS;
		size_t* head  = (size_t*)*LOGCAT_SYSTEM_HEAD_ADDRESS;

		if( ! IS_ON_SDRAM(size, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(woff, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(head, sizeof(size_t) )) break;

		Save_Logcat_To_eMMC(type_system, pbuf, *size, *woff, *head, reset_log_offset);
	} while(0);

	/* Dump logcat radio */
	do {
		uchar* pbuf   = (uchar*)*LOGCAT_RADIO_BUFFER_ADDRESS;
		size_t* size  = (size_t*)*LOGCAT_RADIO_SIZE_ADDRESS;
		size_t* woff  = (size_t*)*LOGCAT_RADIO_WOFF_ADDRESS;
		size_t* head  = (size_t*)*LOGCAT_RADIO_HEAD_ADDRESS;

		if( ! IS_ON_SDRAM(size, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(woff, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(head, sizeof(size_t) )) break;

		Save_Logcat_To_eMMC(type_radio, pbuf, *size, *woff, *head, reset_log_offset);
	} while(0);

	/* Dump logcat events */
	do {
		uchar* pbuf   = (uchar*)*LOGCAT_EVENTS_BUFFER_ADDRESS;
		size_t* size  = (size_t*)*LOGCAT_EVENTS_SIZE_ADDRESS;
		size_t* woff  = (size_t*)*LOGCAT_EVENTS_WOFF_ADDRESS;
		size_t* head  = (size_t*)*LOGCAT_EVENTS_HEAD_ADDRESS;

		if( ! IS_ON_SDRAM(size, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(woff, sizeof(size_t) )) break;
		if( ! IS_ON_SDRAM(head, sizeof(size_t) )) break;

		Save_Logcat_To_eMMC(type_events, pbuf, *size, *woff, *head, reset_log_offset);
	} while(0);

	// countup for next Reset Log
	reset_log_offset++;
	if(reset_log_offset >= RESET_LOG_NUM)
	{
		reset_log_offset = 0;
	}
	Flash_Access_Write((uchar *)&reset_log_offset, sizeof(unsigned int), RESET_LOG_COUNTER_ADDRESS, UNUSED);
	
	return;
}


/**
 * Save_Kmsg_To_eMMC - Save Kmsg to eMMC
 * @return FLASH_ACCESS_SUCCESS   : success
 *         FLASH_ACCESS_ERR_PARAM : error
 */
static RC Save_Kmsg_To_eMMC(uchar *log_buf, int log_buf_len, ulong log_end, ulong logged_chars, ulong offset)
{
	RC ret = FLASH_ACCESS_ERR_PARAM;
	ulong end;
	const char *s1, *s2;
	ulong l1, l2;
	dump_info info;

	ulong log_address  = RESET_LOG_ADDRESS + RESET_LOG_SIZE * offset;
	ulong info_address = log_address + sizeof(reset_log_header_info);
	ulong body_address = log_address + area_info[type_kmsg].offset;

	end = log_end & LOG_BUF_MASK;

	if(NULL != log_buf) {
		if (logged_chars > end) {
			s1 = (char*)(log_buf + log_buf_len - logged_chars + end);
			l1 = logged_chars - end;

			s2 = (char*)log_buf;
			l2 = end;
		} else {
			s1 = "";
			l1 = 0;

			s2 = (char*	)(log_buf + end - logged_chars);
			l2 = logged_chars;
		}

		/* create dump info */
		info.size = l1 + l2;
		info.str1 = (ulong)s1;
		info.len1 = l1;
		info.str2 = (ulong)s2;
		info.len2 = l2;
		info.buffer = (ulong)log_buf;
		info.buf_len = log_buf_len;
		info.buf_end = log_end;
		info.logged_chars = logged_chars;

		/* write dump info */
		ret = Flash_Access_Write((uchar *)&info, sizeof(dump_info), info_address, UNUSED);
	}

	/* write kmsg */
	if(ret == FLASH_ACCESS_SUCCESS)
		ret = Write_Logs_To_eMMC(body_address, s1, l1, s2, l2, area_info[type_kmsg].size);

	return ret;
}


/**
 * Save_Logcat_To_eMMC - Save Logcat to eMMC
 * @return FLASH_ACCESS_SUCCESS   : success
 *         FLASH_ACCESS_ERR_PARAM : error
 */
static RC Save_Logcat_To_eMMC(dump_log_type type, uchar *pbuf, size_t size, size_t w_off, size_t head, ulong offset)
{
	RC ret = FLASH_ACCESS_ERR_PARAM;
	const char *pstr1 = NULL;
	const char *pstr2 = NULL;
	ulong strlen1 = 0;
	ulong strlen2 = 0;
	dump_info info;

	ulong log_address  = RESET_LOG_ADDRESS + RESET_LOG_SIZE * offset;
	ulong info_address = log_address + sizeof(reset_log_header_info) + (sizeof(dump_info) * type);
	ulong body_address = log_address + area_info[type].offset;

	if(NULL != pbuf){
		if(w_off > head){
			pstr1 = (const char *)(pbuf + head);
			strlen1 = w_off - head;
			pstr2 = (const char *)(pbuf);
			strlen2 = 0;
		} else {
			pstr1 = (const char *)(pbuf + head);
			strlen1 = size - head;
			pstr2 = (const char *)(pbuf);
			strlen2 = w_off;
		}

		/* create dump info */
		info.size = strlen1 + strlen2;
		info.str1 = (ulong)pstr1;
		info.len1 = strlen1;
		info.str2 = (ulong)pstr2;
		info.len2 = strlen2;
		info.buffer = (ulong)pbuf;
		info.buf_len = size;
		info.buf_end = w_off;
		info.logged_chars = head;

		/* write dump info */
		ret = Flash_Access_Write((uchar *)&info, sizeof(dump_info), info_address, UNUSED);
	}

	/* write logcat */
	if(ret == FLASH_ACCESS_SUCCESS)
		ret = Write_Logs_To_eMMC(body_address, pstr1, strlen1, pstr2, strlen2, area_info[type].size);

	return ret;
}


/**
 * Write_Logs_To_eMMC - Write logs to eMMC
 * @return FLASH_ACCESS_SUCCESS   : success
 *         FLASH_ACCESS_ERR_PARAM : error
 */
static RC Write_Logs_To_eMMC(volatile ulong emmc_addr, const char *s1, ulong l1,const char *s2, ulong l2, ulong max_size)
{
	RC ret = FLASH_ACCESS_ERR_PARAM;
	uchar rest_data[PADDING_UNIT_SIZE];

	memset(rest_data, 0x00, PADDING_UNIT_SIZE);
	ulong rest_offset =0;
	ulong rest_size = max_size;

	if((l1 + l2) <= max_size)
	{
		if(l1 != 0)
		{
			// check s1 is on SDRAM
			if(IS_ON_SDRAM(s1, l1))
			{
				ret = Flash_Access_Write((uchar *)s1, l1, emmc_addr, UNUSED);
			}
		} else {
			ret = FLASH_ACCESS_SUCCESS;
		}

		if(ret == FLASH_ACCESS_SUCCESS)
		{
			rest_offset += l1;
			rest_size   -= l1;

			if(l2 != 0)
			{
				//check s2 is on SDRAM
				if(IS_ON_SDRAM(s2, l2))
				{
					ret = Flash_Access_Write((uchar *)s2, l2, emmc_addr + rest_offset, UNUSED);
					if(ret == FLASH_ACCESS_SUCCESS)
					{
						rest_offset += l2;
						rest_size   -= l2;
					}
				}
			} else {
				ret = FLASH_ACCESS_SUCCESS;
			}

		}
	}

	// set zero to rest area
	if(rest_size < PADDING_UNIT_SIZE) {
		Flash_Access_Write( rest_data, rest_size, emmc_addr + rest_offset, UNUSED);
	} else {
		Flash_Access_Write( rest_data, (rest_size & (PADDING_UNIT_SIZE-1)) , emmc_addr + rest_offset, UNUSED);
		rest_offset += rest_size & (PADDING_UNIT_SIZE-1);
		rest_size -= rest_size & (PADDING_UNIT_SIZE-1);

		while(rest_size >= PADDING_UNIT_SIZE)
		{
			Flash_Access_Write( rest_data, PADDING_UNIT_SIZE, emmc_addr + rest_offset, UNUSED);
			rest_offset += PADDING_UNIT_SIZE;
			rest_size -= PADDING_UNIT_SIZE;
		}
	}

	return ret;
}


/**
 * Save_Temporary_Log_To_eMMC - Save Temporary Log to eMMC
 * @return none
 */
static void Save_Temporary_Log_To_eMMC(BOOT_LOG *boot_log)
{
	/* Dump temp log */
	do {
		if((boot_log->stbchr3 & APE_RESETLOG_TMPLOG_END) == APE_RESETLOG_TMPLOG_END) {
			Flash_Access_Write((uchar *)TMPLOG_BASE_ADDRESS, RESET_TMPLOG_SIZE, RESET_TMPLOG_ADDRESS, UNUSED);
		}
	} while(0);
	
	return;
}


/**
 * Save_Crash_Log_To_eMMC - Save Crash Log to eMMC
 * @return none
 */
void Save_Crash_Log_To_eMMC(BOOT_LOG *boot_log, ulong boot_log_offset)
{
	*STBCHR0 = 0x00;
	*STBCHR1 = 0x00;
	*STBCHR2 = 0x00;
	*STBCHR3 = 0x00;
#ifndef EMMC_NO_WRITE
	Save_Reset_Log_To_eMMC(boot_log, boot_log_offset);
	
	Save_Temporary_Log_To_eMMC(boot_log);
	
	/* WARNING:
	   In case of Save_Boot_Log_To_eMMC is failed in saving to eMMC, boot_log_offset will
	   less than 0. The content of boot_log is remained but have not saved to eMMC
	*/
#endif //EMMC_NO_WRITE
	return;
}


