/* comm_acm_dev_mcpc.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usbd_def.h"
#include "usbd_types.h"
#include "usb.h"
#include "usb_comm.h"
#include "dev_api.h"
#include "comm_acm_dev_globals.h"

st_LINE_CODE   gstMcpcLineCode;
unsigned short gwUARTStateBitmap[COMM_INTERFACE_NUM];
unsigned short gwHostStateBitmap[COMM_INTERFACE_NUM];

/**
 * UsbCommInitialize
 * @return 0
 */
int UsbCommInitialize(void)
{
	unsigned short wCounter;

	gstMcpcLineCode.dwDTERate   = 0;
	gstMcpcLineCode.bCharFormat = 0;
	gstMcpcLineCode.bParityType = 0;
	gstMcpcLineCode.bDataBits   = 0;

	for (wCounter = 0; wCounter < COMM_INTERFACE_NUM; wCounter++) {
		gwUARTStateBitmap[wCounter] = 0;
		gwHostStateBitmap[wCounter] = 0;
	}

	return 0;
}

/**
 * UsbSetControlLineState
 * @return 0
 */
int UsbSetControlLineState(void *pvArg, pst_DEVICE_REQUEST pstDevRequest)
{
	unsigned short wInterfaceNum;

	wInterfaceNum = pstDevRequest->wIndex;
	if (wInterfaceNum == COMM_ACM_COMM_CLASS_INTERFACE_NUM) {
		gwHostStateBitmap[wInterfaceNum] = pstDevRequest->wValue;
	}

	return 0;
}
