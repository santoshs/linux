/* compile_option.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __COMPILE_OPTION_H__
#define __COMPILE_OPTION_H__

/************************************************************************************************/
/*	Compile option																				*/
/************************************************************************************************/

/* Ignore battery check on boot */
#define _R_LOADER_IGNORE_BATT_CHECK_

/* Ignore CPU temperature check on boot */
// #define _R_LOADER_IGNORE_CPU_TEMP_CHECK_

/* Enable Boot Log writing check */
// #define _R_LOADER_BOOT_LOG_WRITE_CHECK_

/* Enable TRACE-LOG */
#define __TRACELOG__

/* BATT voltage threadhold */
#define BOOT_BATT_VOL							3400	/* mV */

/* CPU temperature threadhold */
#define BOOT_CPU_TEMP							90		/* degree C */

/* LCD Message Position */
#define LOC_X									30		/* Upper X */
#define LOC_Y									250		/* Upper Y */
#define LOC_Y_1									650		/* Lower Y 1 */
#define LOC_Y_2									710		/* Lower Y 2 */
#define LOC_Y_3									770		/* Lower Y 3 */

/* LCD Logo */
#define LOGO_WIDTH								480		/* Logo width */
#define	LOGO_HIGH								132		/* Logo high */
#define	LOGO_X									30		/* X position of logo */
#define	LOGO_Y									366		/* Y position of logo */

/* R-loader compile option */
/* BEGIN: CR722: Apply GPT */
#define LOGO_RAM_ADDR 							(0x4C7C0000)				/* SDRAM address */
#define LOGO_eMMC_ADDR							((*STBCHRB1 & STBCHRB1_BOOT_PARTITION2_USED) == 0 ? (uint64)(0xF0000000000C7800ULL): (uint64)(0xE0000000000C7800ULL))
#define LOGO_BUFF_SIZE 							(0x38600)					/* 226 KB */
/* END: CR722: Apply GPT */

#endif /* __COMPILE_OPTION_H__ */
