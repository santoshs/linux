/*	off_charge_main.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "off_charge.h"
#include "common.h"

#define PMIC_DLIN_VBAT		60
/**
 * off_charge_main - off_charge_main routine
 * @return  - none
 */
void off_charge_main(void)
{
	int bat_cap = 0;
	int charge_present;
	int bat_present;
	int ret;
	uchar long_press;
	int cnt = 0;
	char lcd_prog[] 	= "[          ]       ";
	char lcd_txt[] 		= "Off charge         ";
	uchar prog_num = 1;
	uchar prog_char = 0x23;
	uchar end_chrg = 0;
	int print_bat = 0;
	PRINTF("Start off_charge module\n");
	/* Reduce power consumption */
	SYSC_Reduce_Power(SYS_SPDCR_OFFCHARGE);
	
	/* Set CPU clock to 26MHz */
	CPG_Set_Clock(CPU_CLOCK_26MHZ);
	
	/* Turn off power resource */
	pmic_set_power_off_resource();
	/* Load logo after setting clock */
#ifdef __RL_LCD_ENABLE__
	off_charge_display();
	LCD_Print(LOC_X, LOC_Y_1, lcd_txt);
#endif /* __RL_LCD_ENABLE__ */

	/* Init hardware for battery management */
	pmic_init_battery_hw();
	do {
		/* Wait 1s*/
		TMU_Wait_MS(WAIT_TIME);
		
#ifdef __RL_LCD_ENABLE__
		uchar i = 1;
		while (i < prog_num + 1)
		{
			lcd_prog[i] = prog_char;
			i++;
		}
		LCD_Print(LOC_X, LOC_Y_2, lcd_prog);
		LCD_Draw_Cmode();
		prog_num++;
		if (prog_num > 10) {
			prog_num = 1;
			for (i = 1; i <= 10; i++)
			{
				lcd_prog[i] = 0x20;
			}
		}
#endif	/* __RL_LCD_ENABLE__ */
		
		/* Battery present */
		bat_present = pmic_check_bat_state();
		/* Charge present */
		charge_present = pmic_check_charger();
		if(cnt++ == 10) {
			cnt = 0;
			/* Battery voltage */
			bat_cap = pmic_read_bat_volt();
			print_bat = 100*(bat_cap - 3150)/(3660-3150);
			PRINTF("Current battery capacity %d - Charger %s\n", print_bat, charge_present ? "connected":"disconnected");
		}
		end_chrg = pmic_read_register(PMIC_ID2_REG_ADD, PMIC_LINEAR_CHRG_STS);

		if(((end_chrg & PMIC_END_OF_CHARGE) == PMIC_END_OF_CHARGE) || (charge_present == 0) || (bat_present == PMIC_BAT_NOT_DETECT)) {
			if((end_chrg & PMIC_END_OF_CHARGE) == PMIC_END_OF_CHARGE) {
			#ifdef __RL_LCD_ENABLE__
				snprintf(lcd_txt,16,"Off charge FULL ");
				LCD_Print(LOC_X, LOC_Y_1, lcd_txt);
			#endif	/* __RL_LCD_ENABLE__ */
			TMU_Wait_MS(5000);
		}
			/* Disable charger and power off */
			pmic_disable_charger();
			PRINTF("Power off device\n");
			pmic_force_off_hw();
		}
		if(print_bat != 0) {
#ifdef __RL_LCD_ENABLE__
		snprintf(lcd_txt,16,"Off charge %d ",print_bat);
		LCD_Print(LOC_X, LOC_Y_1, lcd_txt);
#endif	/* __RL_LCD_ENABLE__ */
		}
		/* Power key long press under 100ms */
		ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_STS_HW_CONDITIONS, &long_press);
		if((long_press & BIT0) == 0) {
			PRINTF("\nLong key press. Soft reset\n");
			pmic_soft_reset();
		}
	} while(1);
	
	/* this code can't be reached */
	while(1);

}

#ifdef __RL_LCD_ENABLE__
void off_charge_display(void)
{
	int ret;
	/* LCD Display Show */
	LCD_Init();
	ret = LCD_Check_Board();
	if(LCD_SUCCESS != ret) {
		PRINTF("FAIL to initialize LCD - ret=%d\n", ret);
		// LCD_Backlight_On_Level(10);
	}
	
	/* LCD setting */
	LCD_Clear(LCD_COLOR_BLACK);
	LCD_Set_Text_color(LCD_COLOR_WHITE);
	/*	LCD_Draw_Cmode(); */
	LCD_Display_On();
	
	// LCD_Backlight_On_Level(10);
	
}
#endif	/* __RL_LCD_ENABLE__ */

