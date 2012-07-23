/*	ths_api.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */


#include "ths_api.h"

/**
 * ths_init - Init thernal sensor
 * @return      : None
 */
void ths_init(void)
{
	/* Enable THS*/
	*CPG_SMSTPCR5 &= ~(1<<22);
}

/**
 * ths_get_cpu_temp - Get CPU temperature
 * @return      : CPU temperature
 */
int ths_get_cpu_temp(void)
{
	uchar temp;
	int thscr_val;
	int val;
		
	thscr_val = THSCR0;
	if((thscr_val & THIDLE_MASK) != THIDLE_NORMAL_FULL)
	{
		/* Enable THS normal operation */
		THSCR0 &= ~THIDLE_MASK;
		TMU_Wait_MS(300);
	}
	
	THSCR0 |= CPCTL;
	TMU_Wait_MS(300);
		
	
	temp = (THSSR0 & CTEMP_MASK);
	val = (temp * 5) - 65;
	
	/* Restored THS status*/
	THSCR0 = thscr_val;
	
	return val;	
}

