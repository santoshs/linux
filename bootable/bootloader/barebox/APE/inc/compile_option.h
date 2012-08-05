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

#endif /* __COMPILE_OPTION_H__ */
