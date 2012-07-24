/* C_UsbInt.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_extern.h"


/**
 * usbint - USB interrupt
 * @return None
 */
void usbint(void)
{
	P_usbint();
}
