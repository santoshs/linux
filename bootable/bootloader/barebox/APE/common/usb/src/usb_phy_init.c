/*	usb_phy_init.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#include "pmic.h"
#include "usb_api.h"
#include "usb_private.h"
#include "cpu_register.h"
#include "common.h"


/**
 * usb_phy_reset - Reset PHY by USBHS
 * @return None
 */
void usb_phy_reset(void)
{
	/* PHY reset */
	PHYFUNCTR_USB |= PRESET;
	while(1) {
		if((PHYFUNCTR_USB & PRESET) == 0x0000U)
			break;
	}
}

/**
 * usb_phy_init - Initializes PHY by USBHS
 * @return None
 */
void usb_phy_init(void)
{
	SMSTPCR3				&= ~SMTP322;
	while (1) {
		if((MSTPSR3 & SMTP322) == 0UL)
			break;
	}

	/* Setting ULPI DATA 0 ~ DATA 7 */
	PORT203CR				= PORT_F1;
	PORT204CR				= PORT_F1;
	PORT205CR				= PORT_F1;
	PORT206CR				= PORT_F1;
	PORT207CR				= PORT_F1;
	PORT208CR				= PORT_F1;
	PORT209CR				= PORT_F1;
	PORT210CR				= PORT_F1;

	/* Setting ULPI_CLK */
	PORT211CR				= PORT_F1;

	/* Setting ULPI_STP */
	PORT212CR				= PORT_F1;

	/* Setting ULPI_DIR */
	PORT213CR				= PORT_F1;

	/* Setting ULPI_NXT */
	PORT214CR				= PORT_F1;
	
	/* Reset TUSB */
	if(CHIP_VERSION() >= CHIP_RMU2_ES20){
		PORTD159_128DSR_ES2 = PORT131_TUSB_RST;	
		PORTD159_128DSR_ES2 = PORT130_TUSB_CS;	
	}
			
	if(CHIP_VERSION() == CHIP_RMU2_ES10){
		PORTD159_128DR		&= ~PORT131_TUSB_RST;
	}else{
		PORTD159_128DR_ES2		&= ~PORT131_TUSB_RST;
	}
	
	PORT131CR			= PORT_OE_F0;

	/* Wait ~200 us */
	TimeWaitLoop(200);
	
	if(CHIP_VERSION() == CHIP_RMU2_ES10){
		PORTD159_128DR		|= PORT131_TUSB_RST;
	}else{
		PORTD159_128DR_ES2	|= PORT131_TUSB_RST;
	}

	/* VIO_CKO3 26MHz output */
	VCLKCR3 = CLK_26MHz;
	PORT217CR = PORT_OE_F1;
	VCLKCR3 &= ~CKSTP;
	
	/* wait SuspendM bit cleard by HW */
	while (1) {
		if((PHYFUNCTR_USB & SUSMON) != 0x0000U)
			break;
	}
	
	/* PHY reset */
    PHYFUNCTR_USB |= PRESET;

	while(1) {
		if((PHYFUNCTR_USB & PRESET) == 0x0000U)
			break;
	}

}
/**
 * usb_phy_enable_func - Enable function controller 
 * @return None
 */
void usb_phy_enable_func(void)
{
	/* Set USB function */
	SYSCFG = 0x0091;
	INTENB0 = 0x9F00;
}
/**
 * usb_phy_enable_func - Disable function controller 
 * @return None
 */
void usb_phy_disable_func(void)
{
	SYSCFG = 0x0020;
	INTENB0 = 0x0000;
}
