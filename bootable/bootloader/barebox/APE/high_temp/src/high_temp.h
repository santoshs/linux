/*	high_temp.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __HIGH_TEMP_H__
#define __HIGH_TEMP_H__

#include "log_output.h"

#include "string.h"
#include "common.h"
#include "com_type.h"
#include "cpg.h"
#include "com_api.h"
#include "ths_api.h"
#include "tmu_api.h"
#include "pmic.h"
#include "sysc.h"


#define HIGH_TEMP 80
#define WAIT_TIME (300) /* 300 miliseconds */

#define SWBCR				((volatile ulong *)(0xE6180204))
#define MSK_ALLOW_CTRL		(0xFFF91EFD)		/* Allow control area power: D4, A3SG, A3SP, A3R, A2RV, A2RI, A4MP */
#define MSK_PWRD_REQ		(SYS_SPDCR_D4 | SYS_SPDCR_A3SG | SYS_SPDCR_A2RV | SYS_SPDCR_A2RI | SYS_SPDCR_A4MP)
#define MSK_PWRD_STS		(0xFFF91EFD)

/************************************************************************************************/
/*	Prototypes																					*/
/************************************************************************************************/
void high_temp_main(void);

#endif /* __HIGH_TEMP_H__ */
