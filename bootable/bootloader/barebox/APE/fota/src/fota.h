/*	fota.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __FOTA_H__
#define __FOTA_H__

#include "log_output.h"

#include "string.h"
#include "common.h"
#include "com_type.h"
#include "com_api.h"
#include "tmu_api.h"

#define WAIT_TIME	3000

#define FOTA_OK		0

/************************************************************************************************/
/*	Prototypes																					*/
/************************************************************************************************/
void fota_main(void);
RC fota_check_nvm(void);

#endif /* __FOTA_H__ */
