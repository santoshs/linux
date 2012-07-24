/* C_UsbSig.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/******* include file ********************************************************/
#include "c_typedef.h"
#include "c_defusr.h"
#include "c_macusr.h"


/**
 * stopClock - Stop Clock
 * @return None
 */
void stopClock(void)
{
	USB_CLR_PAT(SYSCFG, SCKE);						// SCLK Disable
}
