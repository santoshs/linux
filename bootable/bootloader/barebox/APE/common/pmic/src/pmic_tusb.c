/*
 * pmic_tusb.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "cpg.h"
#include "pmic.h"

/* USB type for search: [vbus status][[charging status][Enumeration status] */
static uchar usb_type_dict[MAX_VBUS_STS][MAX_CHG_STS][MAX_ENUMER_STS] = {
	/* VBUS not detect */
	/*                  		Not Enumeration		Enumeration */		
{{	/* Charger not detect    */	USB_NOT_CONNECT,	USB_NOT_CONNECT		},
 {	/* Charger detect		 */	USB_NOT_CONNECT,	USB_NOT_CONNECT		}},
 	/* VBUS detect */
	/*                  		Not Enumeration		Enumeration */			
{{	/* Charger not detect    */		USB_UNKNOWN,		USB_SDP		},
 {	/* Charger detect		 */			USB_DCP,		USB_CDP			}},

};

/**
 * tusb_type - Identify USB connection
 * @return USB_NOT_CONNECT          : USB connection not found
 *         USB_UNKNOWN				: Unknown type
 *         USB_SDP 					: SDP type
 *         USB_CDP         			: CDP type
 *         USB_DCP 					: DCP type
 *         PMIC_ERR_I2C				: Error relating to I2C
 */
RC tusb_type(void)
{
	uchar ret, r_data;
	ushort value;
	uchar vbus_state;
	uchar charger_state;
	uchar enumeration_state;
	
	/* Set default value */
	ret = 0;
	r_data = 0;
	vbus_state = 0;
	charger_state = 0;
	enumeration_state = 0;
	
	/* VBUS check */
	ret = I2C_Read(PMIC_ID2_REG_ADD, PMIC_CONTROLLER_STAT1, &r_data);
	if ((I2C_SUCCESS != ret) || (0 > r_data)){ 
		PRINTF("FAIL I2C read PMIC_CONTROLLER_STAT1 error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}	
	if (0 == (r_data & BIT2)) {
		vbus_state = 0;
	} else {
		vbus_state = 1;
	}
	
	/* Charger check */
	ret = I2C_Read(PMIC_ID1_REG_ADD, PMIC_STS_HW_CONDITIONS, &r_data);
	if ((I2C_SUCCESS != ret) || (0 > r_data)) {
		PRINTF("FAIL I2C read STS_HW_CONDITIONS error - ret=%d data=0x%x\n", ret, r_data);
		return PMIC_ERR_I2C;
	}	
	charger_state = ((r_data & PMIC_STS_HW_CONDITION_CHRG) == PMIC_STS_HW_CONDITION_CHRG) ? 1 : 0;
	

	
	/* Enumeration check */
	value = INTSTS0;
	PRINTF("INTSTS0 = 0x%x\n", value);
	if ( value & (DVST)) {
		enumeration_state = 1;
	}
	
	return usb_type_dict[vbus_state][charger_state][enumeration_state];
}

/**
 * tusb_phy_init - Init TUSB PHY
 * @return      : none
 */
void tusb_phy_init(void)
{
	ulong temp;
	
	/* Sets MSTPST322(bit 22) of MSTPSR3 to 0(the HS-USB operation) */
	*CPG_SMSTPCR3				&= ~(0x00400000);
	while(1)
	{
		temp = *CPG_SMSTPCR3;
		if((temp & 0x00400000) == 0UL)
			break;
	}
			
	/* Setting ULPI DATA 0 ~ DATA 7 */
	*GPIO_PORT203CR				= 0x01;
	*GPIO_PORT204CR				= 0x01;
	*GPIO_PORT205CR				= 0x01;
	*GPIO_PORT206CR				= 0x01;
	*GPIO_PORT207CR				= 0x01;
	*GPIO_PORT208CR				= 0x01;
	*GPIO_PORT209CR				= 0x01;
	*GPIO_PORT210CR				= 0x01;

	/* Setting ULPI_CLK */
	*GPIO_PORT211CR				= 0x01;

	/* Setting ULPI_STP */
	*GPIO_PORT212CR				= 0x01;

	/* Setting ULPI_DIR */
	*GPIO_PORT213CR				= 0x01;

	/* Setting ULPI_NXT */
	*GPIO_PORT214CR				= 0x01;
	
	/* Reset TUSB */
	if (CHIP_VERSION() == CHIP_RMU2_ES10){
		*GPIO_PORTD159_128DR &= ~PORT131_TUSB_RST;
	}
	else{
		*GPIO_PORTD159_128DR_ES2 &= ~PORT131_TUSB_RST;
	}
	
	*GPIO_PORT131CR				= 0x10;
	
	/* Wait ~100 us */
	TMU_Wait_US(500);
	if (CHIP_VERSION() == CHIP_RMU2_ES10){
		*GPIO_PORTD159_128DR |= PORT131_TUSB_RST;
	}
	else{
		*GPIO_PORTD159_128DR_ES2 |= PORT131_TUSB_RST;
	}
		
	/* VIO_CKO3 26MHz output */
	*CPG_VCLKCR3 = 0x00006100;
	*GPIO_PORT217CR = 0x11;
	*CPG_VCLKCR3 &= ~0x00000100;
	
	/* wait SuspendM bit cleard by HW */
	while(1)
	{
		if((PHYFUNCTR & 0x4000) != 0x0000U)
			break;
	}
	
	/* PHY reset */
    PHYFUNCTR |= 0x2000;
	
	while(1)
	{
		if((PHYFUNCTR & 0x2000) == 0x0000U)
			break;
	}
	/* Set USB function */
	SYSCFG = 0x0091;
	INTENB0 = 0x9F00;
	
	
}