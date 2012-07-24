/*	usb_open.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usb_api.h"
#include "usb_private.h"
#include "usb_download.h"
#include "comm_acm_dev_api.h"
#include "comm_acm_dev_vcom.h"
#include "usbdrv/dev_stack/usbhs/usbf_api.h"
#include "c_def596.h"

#define PHYFUNCTR_PRESET		0x2000									// Reset PHY
#define PHYFUNCTR_DATA_FS		0x0500									// Pin Function Select 
#define PHYIFCTR_DATA_DFT		0x0000									// I/F Select 

extern USB_STATE		gUsbState;			// The state of the USB transfer control function
extern void				IntDevUSBCheck(void);

unsigned char gUsbConnectState = USB_NO_CONNECT_STATE; // The connection condition of the USB transfer control unit

/**
 * usb_open - Opens USB connection
 * @return USB_SUCCESS          : Normal end
 *         USB_ERR_ALREADY_OPEN : Open already implementation
 *         USB_ERR_OPEN         : Open failure
 *         USB_ERR_DISCONNECTED : USB disconnected
 */
RC usb_open(void)
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
	
	// CLOSE state check
	if (gUsbState != USB_STATE_CLOSE) {
		return USB_ERR_ALREADY_OPEN;
	}
	
	if (Usb_Connection() != 0) {
		return USB_ERR_OPEN;
	}
	
	// Transitions to the OPEN state
	gUsbState = USB_STATE_OPEN;
	
	return USB_SUCCESS;
}

/**
 * Usb_Connection - Processes in the USB connection
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int Usb_Connection(void)
{
	unsigned long		phyfunctr_value;
	int					rtn;
	USB_VCOM_SETTING	Vcomparam;
	PUSB_VCOM_SETTING	pVcomParam;

	pVcomParam = &Vcomparam;

	CPU_CONTA_OPORT_ON();
	UsbF_SetDp(1);
	TimeWaitLoop(10);			// 1ms wait
	
	UsbVcomInitialize();		// Vcom initialize process
	rtn = CommAcmDevOpen();		// communication class resource create
	if (rtn != 0) {
		return -1;
	}
	
	pVcomParam->TimeOut			= 0xFFFF;			// time out(for compatibility)
	pVcomParam->RxNotify		= NULL;				// callback of Data Receive
	pVcomParam->RxNotifyCount	= 64;				// Receive data count
	pVcomParam->TxNotify		= NULL;				// callback of Data Send
	pVcomParam->TxNotifyCount	= 64;				// Send data count

	UsbVcomSetting((PUSB_VCOM_SETTING )pVcomParam);	// Vcom communication setting
	UsbVcomWriteFifoClear();
	UsbReadBuffClear();
	PHYFUNCTR |= PHYFUNCTR_PRESET;
	/* Wait for PHYFUNCTR.PRESET =0 */
	phyfunctr_value = PHYFUNCTR;
	while(1) {
		if(!(phyfunctr_value & PHYFUNCTR_PRESET))
			break;
	}

	return rtn;	
}
