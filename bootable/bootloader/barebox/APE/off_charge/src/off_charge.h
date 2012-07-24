/*	off_charge.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __OFF_CHARGE_H__
#define __OFF_CHARGE_H__

#include "log_output.h"

#include "string.h"
#include "common.h"
#include "com_type.h"
#include "cpg.h"
#include "sysc.h"
#include "com_api.h"
#include "ths_api.h"
#include "pmic.h"
#include "tmu_api.h"
#ifdef __RL_LCD_ENABLE__
#include "lcd_api.h"
#endif	/* __RL_LCD_ENABLE__ */

#define WAIT_TIME	1000
/************************************************************************************************/
/*	Prototypes																					*/
/************************************************************************************************/
void off_charge_main(void);

#ifdef __RL_LCD_ENABLE__
void off_charge_display(void);
#endif	/* __RL_LCD_ENABLE__ */

#endif /* __OFF_CHARGE_H__ */
