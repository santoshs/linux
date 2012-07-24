/*
 * r_loader_crash_log.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __R_LOADER_CRASH_LOG_H__
#define __R_LOADER_CRASH_LOG_H__

#include "compile_option.h"
#include "com_type.h"
#include "common.h"

#define RENESAS_LOG_AREA (0x51000000)

#define LOG_BUF_MASK (log_buf_len-1)
#define RESET_LOG_NUM 2
#define RESET_LOG_ADDRESS (RENESAS_LOG_AREA + 0x360C00)  /* 0x51000000 + 1153KB*3 */
#define RESET_LOG_SIZE    (1153*1024) /* 1153KB */

#define RESET_LOG_HEADER_SIZE (0x200) /* 512B */
#define KMSG_OFFSET          (RESET_LOG_HEADER_SIZE)
#define KMSG_SIZE            (0x20000) /* 128KB */
#define LOGCAT_MAIN_OFFSET   (KMSG_OFFSET + KMSG_SIZE)
#define LOGCAT_MAIN_SIZE     (0x40000) /* 256KB */
#define LOGCAT_SYSTEM_OFFSET (LOGCAT_MAIN_OFFSET + LOGCAT_MAIN_SIZE)
#define LOGCAT_SYSTEM_SIZE   (0x40000) /* 256KB */
#define LOGCAT_RADIO_OFFSET  (LOGCAT_SYSTEM_OFFSET + LOGCAT_SYSTEM_SIZE)
#define LOGCAT_RADIO_SIZE    (0x40000)  /* 256KB */
#define LOGCAT_EVENTS_OFFSET (LOGCAT_RADIO_OFFSET + LOGCAT_RADIO_SIZE)
#define LOGCAT_EVENTS_SIZE   (0x40000)  /* 256KB */

#define RESET_TMPLOG_ADDRESS (RENESAS_LOG_AREA + 0x360C00 + 0x240800)   /* 0x51000000 + 1153KB*3 + 1153KB*2 */
#define RESET_TMPLOG_SIZE    (256*1024)  /* 256KB */

#define RESET_LOG_COUNTER_ADDRESS (LOG_NUM_OFFSET + 4) 

/* Physical Address list on SDRAM. used to handshake with Android. */
#define BASE_ADDRESS             (0x4C801020)

#define KMSG_BASE_ADDRESS        (BASE_ADDRESS)
#define KMSG_LOGBUF_ADDRESS      ((volatile ulong *)(KMSG_BASE_ADDRESS))
#define KMSG_LOGBUFLEN_ADDRESS   ((volatile ulong *)(KMSG_BASE_ADDRESS + 0x04))
#define KMSG_LOGEND_ADDRESS      ((volatile ulong *)(KMSG_BASE_ADDRESS + 0x08))
#define KMSG_LOGGEDCHARS_ADDRESS ((volatile ulong *)(KMSG_BASE_ADDRESS + 0x0C))

#define LOGCAT_MAIN_BASE_ADDRESS   (BASE_ADDRESS + 0x10)
#define LOGCAT_MAIN_BUFFER_ADDRESS ((volatile ulong *)(LOGCAT_MAIN_BASE_ADDRESS))
#define LOGCAT_MAIN_SIZE_ADDRESS   ((volatile ulong *)(LOGCAT_MAIN_BASE_ADDRESS + 0x04))
#define LOGCAT_MAIN_WOFF_ADDRESS   ((volatile ulong *)(LOGCAT_MAIN_BASE_ADDRESS + 0x08))
#define LOGCAT_MAIN_HEAD_ADDRESS   ((volatile ulong *)(LOGCAT_MAIN_BASE_ADDRESS + 0x0C))

#define LOGCAT_EVENTS_BASE_ADDRESS   (BASE_ADDRESS + 0x20)
#define LOGCAT_EVENTS_BUFFER_ADDRESS ((volatile ulong *)(LOGCAT_EVENTS_BASE_ADDRESS))
#define LOGCAT_EVENTS_SIZE_ADDRESS   ((volatile ulong *)(LOGCAT_EVENTS_BASE_ADDRESS + 0x04))
#define LOGCAT_EVENTS_WOFF_ADDRESS   ((volatile ulong *)(LOGCAT_EVENTS_BASE_ADDRESS + 0x08))
#define LOGCAT_EVENTS_HEAD_ADDRESS   ((volatile ulong *)(LOGCAT_EVENTS_BASE_ADDRESS + 0x0C))

#define LOGCAT_RADIO_BASE_ADDRESS   (BASE_ADDRESS + 0x30)
#define LOGCAT_RADIO_BUFFER_ADDRESS ((volatile ulong *)(LOGCAT_RADIO_BASE_ADDRESS))
#define LOGCAT_RADIO_SIZE_ADDRESS   ((volatile ulong *)(LOGCAT_RADIO_BASE_ADDRESS + 0x04))
#define LOGCAT_RADIO_WOFF_ADDRESS   ((volatile ulong *)(LOGCAT_RADIO_BASE_ADDRESS + 0x08))
#define LOGCAT_RADIO_HEAD_ADDRESS   ((volatile ulong *)(LOGCAT_RADIO_BASE_ADDRESS + 0x0C))

#define LOGCAT_SYSTEM_BASE_ADDRESS   (BASE_ADDRESS + 0x40)
#define LOGCAT_SYSTEM_BUFFER_ADDRESS ((volatile ulong *)(LOGCAT_SYSTEM_BASE_ADDRESS))
#define LOGCAT_SYSTEM_SIZE_ADDRESS   ((volatile ulong *)(LOGCAT_SYSTEM_BASE_ADDRESS + 0x04))
#define LOGCAT_SYSTEM_WOFF_ADDRESS   ((volatile ulong *)(LOGCAT_SYSTEM_BASE_ADDRESS + 0x08))
#define LOGCAT_SYSTEM_HEAD_ADDRESS   ((volatile ulong *)(LOGCAT_SYSTEM_BASE_ADDRESS + 0x0C))

#define TMPLOG_BASE_ADDRESS          (0x4C821200)

/* Address check. if addr is on SDRAM */
#define SDRAM_AREA_START (0x40000000)
#define SDRAM_AREA_END   (0x7FFFFFFF)
#define IS_ON_SDRAM(addr, size) ( (SDRAM_AREA_START < (ulong)addr) && ((ulong)addr < SDRAM_AREA_END - size) )

#define PADDING_UNIT_SIZE (0x400) // 1024B for zero padding

#define SRSTFR_RCWD0		 (0x00000010)
/* for STBCHR2 */
#define APE_RESETLOG_PANIC_END     (0x02)
#define APE_RESETLOG_INIT_COMPLETE (0x80)
/* for STBCHR3 */
#define APE_RESETLOG_DEBUG         (0x04)
#define APE_RESETLOG_TMPLOG_END    (0x08)

typedef enum {
	type_kmsg = 0,
	type_main,
	type_system,
	type_radio,
	type_events
} dump_log_type;


typedef  struct {
	ulong offset;
	ulong size;
} reset_log_area_info;

/* header info */
typedef struct {
	ulong counter;
	BOOT_LOG boot_info;
} reset_log_header_info;

/* dump data info */
typedef struct {
	ulong size;
	ulong str1;
	ulong len1;
	ulong str2;
	ulong len2;
	ulong buffer;       // kmsg:log_buf       logcat:pbuf
	ulong buf_len;      // kmsg:log_buf_len   logcat:size
	ulong buf_end;      // kmsg:buf_end       logcat:w_off
	ulong logged_chars; // kmsg:logged_chars  logcat:head
} dump_info;


/* Function Prototypes */
void Save_Crash_Log_To_eMMC(BOOT_LOG *boot_log, ulong boot_log_offset);

#endif /* __R_LOADER_CRASH_LOG_H__ */
