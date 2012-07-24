/* protect_option.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __PROTECT_OPTION_H_
#define __PROTECT_OPTION_H_

#include "com_type.h"


/************************************************************************************************/
/*	Compile option																				*/
/************************************************************************************************/

/* enable eMMC write protection. */
/* this option. */

#define ENABLE_EMMC_PROTECT 

// #define ENABLE_TUNEUPVAL_PROTECT 

#define PROTECT_SECTOR_OFFSET 0


/************************************************************************************************/
/*	Definitions																					*/
/************************************************************************************************/

/* Write protection area */
#define EMMC_TUNEUPVAL_AREA_ST 	(0x3A6000)	/* eMMC write protection tuneupval area start sector address */
#define EMMC_TUNEUPVAL_AREA_ED 	(0x3A7FFF)	/* eMMC write protection tuneupval area end  sector address  */
#define ALL_USER_DATA_PROTECT 0x01

#endif
