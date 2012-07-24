/*	usb_receive.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usb_api.h"
#include "usb_private.h"
#include "usb_download.h"
#include "comm_acm_dev_vcom.h"
#include "common.h"
#include "usbf_api.h"
#include "divmod.h"

#define RECV_RETRY_MULTIPLE	200									// Loop count for 1msec
#define WDT_CLEAR_LOOP_COUNT	(5000 * RECV_RETRY_MULTIPLE)	// Equivalent to 5 seconds


extern void	IntDevUSBCheck(void);
extern USB_STATE	gUsbState;	// The state of the USB transfer control function

unsigned long gRecvDataEndPos = 0;		// The receive data ending point
unsigned long gRecvDataStartPos = 0;	// The receive data starting point
unsigned char gUsbRecvingState = USB_NO_RECVING_STATE;		// The reception condition of the USB transfer control unit


/**
 * usb_receive - Acquires received data
 * @return USB_SUCCESS             : Normal end
 *         USB_ERR_PARAM           : Parameter error
 *         USB_ERR_NOT_OPEN        : Open unexcecuted
 *         USB_ERR_DISCONNECTED    : USB disconnected
 */
RC usb_receive(uchar* pBuff, ulong length, ulong timeout)
{
	ulong recvCnt		= 0;
	ulong noRecvCnt		= 0;
	ulong loopCnt		= 0;
	ulong beforeRecvCnt	= 0;
	ulong recvRetryCnt	= (timeout * RECV_RETRY_MULTIPLE);
	
	// Connect check
	if (vbus_check() == 0) {
		Usb_Disconnection();
		InitializeState();
		gUsbState = USB_STATE_DISCONNECTED;
		return USB_ERR_DISCONNECTED;
	}
	
	// DISCONNECTED state check
	if (gUsbState == USB_STATE_DISCONNECTED) {
		gUsbState = USB_STATE_CLOSE;
	}
	
	// CLOSE state check
	if (gUsbState == USB_STATE_CLOSE) {
		return USB_ERR_NOT_OPEN;
	}
	
	// Parameter check
	if ((pBuff == NULL) || (length == 0)) {
		return USB_ERR_PARAM;
	}
	
	while (1) {
		recvCnt = (gRecvDataEndPos-gRecvDataStartPos);
		if (recvCnt >= length)
		{
			// The receive data is above the request size
			gRecvDataStartPos += length;
			return length;
		} else {
			// The receive data is request size less than
			if (recvCnt > beforeRecvCnt) {
				noRecvCnt = 0;
			} else {
				noRecvCnt++;
				if (noRecvCnt > recvRetryCnt) {
					// The data non-reception continues for equal to or more than argument "timeout" milliseconds
					// Return the size of the acquired data
					gRecvDataStartPos = gRecvDataEndPos;
					return recvCnt;
				}
			}
			beforeRecvCnt = recvCnt; 
		}
		loopCnt++;

		// WDT is clear every WDT_CLEAR_LOOP_COUNT multiple (5 seconds)
		if (!(loopCnt % WDT_CLEAR_LOOP_COUNT)) {
			WDT_CLEAR();
		}
	}

	if (UsbVcomReadCount() <= 0) {
		// There is not remainder in the receive data storage buffer
		// Transitions to the OPEN state
		gUsbState = USB_STATE_OPEN;
	}

	return length;
}
