/*	usb_phy_read.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "usb_api.h"
#include "usb_private.h"
#include "cpu_register.h"

/**
 * usb_phy_read - Read data of TUSB1211 register via PHY
 * @return None
 */
uchar usb_phy_read(ulong reg)
{
	uchar monreg;
	*USB_SPADDR = reg;
	
	/* Issue read comand */
	*USB_SPCTRL |= USB_SPRD;
	do {
	monreg = *USB_SPCTRL;
	} while(monreg & USB_SPRD);
	
	monreg = *USB_SPRDAT;
	return monreg;
}

