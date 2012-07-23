/* 
 *	tmu_api.c 
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */



#include "lcd_common.h"
#include "tmu_api.h"
#include "timer_drv.h"



/****************************************************************************************************/
/* Function name   : TMU_Wait_MS(void)                                                     		*/
/* Input    : void                                                                               	*/
/* Return   : void												*/
/* Processing : wait in ms															*/
/****************************************************************************************************/
void TMU_Wait_MS(u4 ms)
{
	vog_timer_ms_wait(ms);
}

/****************************************************************************************************/
/* Function name   : TMU_Wait_US(void)                                                     		*/
/* Input    : void                                                                               	*/
/* Return   : void												*/
/* Processing : wait in us															*/
/****************************************************************************************************/
void TMU_Wait_US(u4 us)
{
	vog_timer_us_wait(us);
}
