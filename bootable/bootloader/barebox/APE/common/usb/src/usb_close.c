/*	usb_close.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usb_api.h"
#include "usb_private.h"

extern USB_STATE	gUsbState;	// The state of the USB transfer control function

/**
 * usb_close - Closes USB connection
 * @return USB_SUCCESS          : Normal end
 *         USB_ERR_DISCONNECTED : USB disconnected
 */
RC usb_close(void)
{
	// Connect check
	if (vbus_check() == 0) {
		Usb_Disconnection();
		InitializeState();
		gUsbState = USB_STATE_DISCONNECTED;
		return USB_ERR_DISCONNECTED;
	}
	
	// DISCONNECTED state check
	if (gUsbState == USB_STATE_DISCONNECTED) {
		// Transitions to the CLOSE state
		gUsbState = USB_STATE_CLOSE;
	}
	
	Usb_Disconnection();
	
	// Transitions to the CLOSE state
	gUsbState = USB_STATE_CLOSE;
	
	return USB_SUCCESS;
}
