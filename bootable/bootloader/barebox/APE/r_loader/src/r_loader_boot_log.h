/*
 * r_loader_boot_log.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __R_LOADER_BOOT_LOG_H__
#define __R_LOADER_BOOT_LOG_H__

#include "compile_option.h"
#include "com_type.h"
#include "log_output.h"
#include "r_loader_boot_matrix.h"

#define EMMC_NO_WRITE

/* Structure of emmc boot log (44 bytes) */
typedef struct __boot_log__{
	uchar		header;			/* Header "[" */
	uchar		year;			/* Year */
	uchar		month;			/* Month */
	uchar		day;			/* Date */
	uchar		hour;			/* Hour */
	uchar		minute;			/* Minute */
	uchar		second;			/* Second */
	uchar		separator;		/* Separator "]" */
	BOOT_MATRIX	boot_matrix;	/* Boot matrix */
	ulong		progress_info;	/* Progress info */
	RC			return_value;	/* Return value */
	uchar		stbchrb0;		/* STBCHRB0 register */
	uchar		stbchrb1;		/* STBCHRB1 register */
	uchar 		stbchr0;		/* STBCHR0 register */
	uchar 		stbchr1;		/* STBCHR1 register */
	uchar 		stbchr2;		/* STBCHR2 register */
	uchar 		stbchr3;		/* STBCHR3 register */
	uchar		branching_mode;	/* Branching mode */
	uchar 		reservation1; 	/* Reserved 1 */
	uchar 		reservation2[4]; /* Reserved 2 */
} BOOT_LOG;

/* Boot log position */ 
#define MAX_LOG_NUM 								((ulong)(1024))	
#define LOG_START_OFFSET 							((uint64)(0x00000000515EB800))
#define LOG_NUM_OFFSET 								LOG_START_OFFSET + (sizeof(BOOT_LOG) * MAX_LOG_NUM)

/* Function Prototypes */
RC Save_Boot_Log_To_eMMC(ulong line_num, RC return_val, uchar branch_mode);
BOOT_LOG boot_log_get(void);
void boot_log_clear(void);

#endif /* __R_LOADER_BOOT_LOG_H__ */
