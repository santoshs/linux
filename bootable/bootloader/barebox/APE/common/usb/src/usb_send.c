/*	usb_send.c
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

#define SEND_RETRY_MULTIPLE	100									// Loop count for 1msec
#define WDT_CLEAR_LOOP_COUNT	(5000 * SEND_RETRY_MULTIPLE)	// Equivalent to 5 seconds
#define MAX_PACKET_SIZE		512

extern void	IntDevUSBCheck(void);
extern USB_STATE	gUsbState;	// The state of the USB transfer control function
unsigned char		gSendZLP;	// The Zero-Length packet transmission right or wrong in case of data transmission completion

/**
 * usb_send - Transmits data
 * @return USB_SUCCESS             : Normal end
 *         USB_ERR_PARAM           : Parameter error
 *         USB_ERR_NOT_OPEN        : Open unexcecuted
 *         USB_ERR_DISCONNECTED    : USB disconnected
 */
RC usb_send(uchar* pBuff, ulong length, ulong timeout)
{
	ulong sendCnt		= 0;
	ulong totalSendCnt	= 0;
	ulong noSendCnt		= 0;
	ulong loopCnt		= 0;
	ulong sendRetryCnt	= (timeout * SEND_RETRY_MULTIPLE);
	
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
	if ((pBuff == NULL) || (length == 0) || (length > USB_MAX_TRANSFER_SIZE)) {
		return USB_ERR_PARAM;
	}
	
	while (1) {
		sendCnt = UsbVcomWriteData((pBuff + totalSendCnt), (length - totalSendCnt));
		IntDevUSBCheck();
		totalSendCnt += sendCnt;
		if (sendCnt > 0) {
			noSendCnt = 0;
		}
		else {
			noSendCnt++;
			if (noSendCnt > sendRetryCnt) {
				// Send data(the return value of UsbVcomWriteData()) == 0 continues for equal to or more than argument "timeout" milliseconds
				// Return the size of the transferred data
				return totalSendCnt;
			}
		}
		if (totalSendCnt >= length) {
			// The send data(the return value of UsbVcomWriteData()) is above the size of being left
			break;
		}
		loopCnt++;
		// WDT is clear every WDT_CLEAR_LOOP_COUNT multiple (5 seconds)
		if (!(loopCnt % WDT_CLEAR_LOOP_COUNT)) {
			WDT_CLEAR();
		}
	}
	
	if (GetBulkInTransferFlag_Check()) {
		// The transmission flag of the USB control function is ON
		if (!(length % MAX_PACKET_SIZE)) {
			// Length is the multiple of MaxPacketSize
			gSendZLP = 1;
		}
		
		// Transitions to the SEND_DATA state
		gUsbState = USB_STATE_SEND_DATA;
	}
	else {
		if (!(length % MAX_PACKET_SIZE)) {
			// Length is the multiple of MaxPacketSize
			// Zero-Length packet transmission
			SendZLP();
		}
	}
	
	return length;
}
