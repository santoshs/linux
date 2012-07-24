/*	usb_phy_write.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "usb_api.h"
#include "usb_private.h"
#include "cpu_register.h"

/**
 * otg_phy_write - Write data of TUSB1211 register via PHY
 * @return None
 */
void usb_phy_write(ulong reg, uchar val)
{
    ulong monreg;
    *USB_SPADDR= reg;
    *USB_SPWDAT = val;
    
    /* Issue write comand */
    *USB_SPCTRL |= USB_SPWR;
    do {
    monreg = *USB_SPCTRL;
    } while(monreg & USB_SPWR);
}

