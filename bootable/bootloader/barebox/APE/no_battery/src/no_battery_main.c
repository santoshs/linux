/*	no_battery_main.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "no_battery.h"

/**
 * no_battery_main - high_temp_main routine
 * @return  - none
 */
void no_battery_main(void)
{
	PRINTF("\n NO BATTERY \n");
	LCD_Print(LOC_X, LOC_Y, "No battery");
	pmic_force_off_hw();
	
	/* this code can't be reached */
	while(1);

}

