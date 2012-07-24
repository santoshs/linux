/*	high_temp_main.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "high_temp.h"

/**
 * high_temp_main - high_temp_main routine
 * @return  - none
 */
void high_temp_main(void)
{
	int temp;
	CPU_CLOCK clock;
	
	ths_init();
	temp = ths_get_cpu_temp();
	PRINTF("\n CPU temp: %d degree\n", temp);
	if (temp > HIGH_TEMP) {
		/* Power off Modem */
		pmic_set_power_off_resource();
		
		/* Power off areas D4, A3SG, A2RV, A2RI, A4MP */
		*SWBCR &= MSK_ALLOW_CTRL;
		do {
			SYSC_Reduce_Power(MSK_PWRD_REQ);
		} while ((*PSTR | MSK_PWRD_STS) != MSK_PWRD_STS);
		
		/* Set CPU freg to low */
		clock = CPU_CLOCK_26MHZ;
		do {
			CPG_Set_Clock(clock);
		} while ((*CPG_PLLECR & CPG_PLLECR_PLL1E) != CPG_PLLECR_PLL1E);
	}
	
	do {
		TMU_Wait_MS(WAIT_TIME);
		temp = ths_get_cpu_temp();
		PRINTF("\n CPU temp: %d degree\n", temp);
	} while (temp > HIGH_TEMP);
	
	/* Soft power on reset */
	pmic_soft_reset();
	
	/* this code can't be reached */
	while(1);
}
