/* comm_acm_dev_tools.c
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
#include "usb_download.h"
#include "comm_acm_dev_vcom.h"
#include "usbf_api.h"

void CommAcmBulkOutDone(pst_DATA_CALLBACK pstDataCB);

extern st_LINE_CODE gstMcpcLineCode;

extern unsigned char *gbBulkOutBuff;


void UsbReadBuffClear(void);

/**
 * CommAcmDevConfigNotify
 * @return 0
 */
int CommAcmDevConfigNotify(void *pvArg)
{
	unsigned char            bDevSpeed;
	pst_COMM_ACM_DEV_GLOBALS gP = (pst_COMM_ACM_DEV_GLOBALS)pvArg;

	UsbF_GetSpeed(&bDevSpeed);
	if (bDevSpeed == USBF_TRUE) {
		gP->stBulkOutDataCB.wMaxPktSize = COMM_ACM_BULK_MAX_PKT_SIZE_HS;
		gP->stBulkInDataCB.wMaxPktSize  = COMM_ACM_BULK_MAX_PKT_SIZE_HS;
	}
	else {
		gP->stBulkOutDataCB.wMaxPktSize = COMM_ACM_BULK_MAX_PKT_SIZE;
		gP->stBulkInDataCB.wMaxPktSize  = COMM_ACM_BULK_MAX_PKT_SIZE;
	}

	gP->stBulkOutDataCB.bEpAddress = COMM_ACM_BULKOUT_EP_ADDRESS;
	gP->stBulkOutDataCB.bToggle    = 0;

	gP->stBulkInDataCB.bEpAddress = COMM_ACM_BULKIN_EP_ADDRESS;
	gP->stBulkInDataCB.bToggle    = 0;

	gP->bBulkInTransferFlag = 0;

	// VcomFIFO Clear
	UsbReadBuffClear();
	UsbVcomWriteFifoClear();

	// TODO: Notify to upper layer
	UsbCommInitialize();

	CommAcmGetBulkOut();

	return 0;
}

/**
 * CommAcmGetBulkOut
 * @return None
 */
void CommAcmGetBulkOut(void)
{
	pst_COMM_ACM_DEV_GLOBALS gP;

	gP = COMM_ACM_DEV_GLOBAL();

	gP->stBulkOutDataCB.DataCallback = (DATA_CALLBACK)CommAcmBulkOutDone;
	gP->stBulkOutDataCB.pbBuffer     = gbBulkOutBuff;
	gP->stBulkOutDataCB.wBufferSize  = COMM_ACM_BULKOUT_BUFF_SIZE;
	gP->stBulkOutDataCB.wDataSize    = 1;
	gP->stBulkOutDataCB.wXferSize    = 0;
	gP->stBulkOutDataCB.bEpAddress   = COMM_ACM_BULKOUT_EP_ADDRESS;

	UsbDevReadEp(&gP->stBulkOutDataCB);
}

/**
 * CommAcmPutBulkIn
 * @return None
 */
void CommAcmPutBulkIn(unsigned char *pbDataBuf, int iDataSize)
{
	pst_COMM_ACM_DEV_GLOBALS gP;

	gP = COMM_ACM_DEV_GLOBAL();

	gP->stBulkInDataCB.DataCallback = (DATA_CALLBACK)CommAcmBulkInDone;
	gP->stBulkInDataCB.pbBuffer     = pbDataBuf;
	gP->stBulkInDataCB.wBufferSize  = (unsigned short)iDataSize;
	gP->stBulkInDataCB.wDataSize    = 0;
	gP->stBulkInDataCB.wXferSize    = 0;
	gP->stBulkInDataCB.bEpAddress   = COMM_ACM_BULKIN_EP_ADDRESS;
	gP->bBulkInTransferFlag         = DCD_COMM_BULKIN_DATA_TRANSFER;

	UsbDevWriteEp(&gP->stBulkInDataCB);
}


/**
 * CommAcmDevClassRequest
 * @return 0
 */
int CommAcmDevClassRequest(void *pvArg, pst_DEVICE_REQUEST pstDevRequest)
{
	int                      iErr;
	unsigned short           wLength;
	pst_COMM_ACM_DEV_GLOBALS gP;

	iErr = 0;
	gP   = (pst_COMM_ACM_DEV_GLOBALS)pvArg;

	// communication class request
	if (pstDevRequest->bmRequestType & REQ_TYPE_CLASS) {
		if (pstDevRequest->bmRequestType & REQ_DEVICE_TO_HOST) {
			if ((pstDevRequest->bmRequestType & REQ_RECIPIENT_MASK) == REQ_RECIPIENT_INTERFACE) {
				switch (pstDevRequest->bRequest) {
				case COMM_REQ_GET_LINE_CODING:
					wLength = (unsigned short)min(pstDevRequest->wLength, (unsigned short)sizeof(st_LINE_CODE));

					gP->stClassRequestDataCB[0].DataCallback = NULL;
					gP->stClassRequestDataCB[0].pbBuffer     = (unsigned char *)&gstMcpcLineCode;
					gP->stClassRequestDataCB[0].wBufferSize  = wLength;
					gP->stClassRequestDataCB[0].wXferSize    = 0;
					gP->stClassRequestDataCB[0].wMaxPktSize  = COMM_ACM_CTRL_MAX_PKT_SIZE;
					gP->stClassRequestDataCB[0].bEpAddress   = 0;
					gP->stClassRequestDataCB[0].bToggle      = 1;

					UsbDevWriteEp(&gP->stClassRequestDataCB[0]);
					break;

				default:
					UsbDevStallEp(0x00, TRUE);
					break;
				}
			}
			else {
				UsbDevStallEp(0x00, TRUE);
			}
		}
		else {
			if ((pstDevRequest->bmRequestType & REQ_RECIPIENT_MASK) == REQ_RECIPIENT_INTERFACE) {
				switch (pstDevRequest->bRequest) {
				case COMM_REQ_SEND_ENCAPSULATED_COM:
				case COMM_REQ_SET_COMM_FEATURE:

					gP->stClassRequestDataCB[0].DataCallback = NULL;
					gP->stClassRequestDataCB[0].pbBuffer     = NULL;
					gP->stClassRequestDataCB[0].wBufferSize  = 0;
					gP->stClassRequestDataCB[0].wXferSize    = 0;
					gP->stClassRequestDataCB[0].wMaxPktSize  = COMM_ACM_CTRL_MAX_PKT_SIZE;
					gP->stClassRequestDataCB[0].bEpAddress   = 0;
					gP->stClassRequestDataCB[0].bToggle      = 1;

					UsbDevWriteEp(&gP->stClassRequestDataCB[0]);
					break;

				case COMM_REQ_CLEAR_COMM_FEATURE:
				case COMM_REQ_SEND_BREAK:

					gP->stClassRequestDataCB[0].DataCallback = NULL;
					gP->stClassRequestDataCB[0].pbBuffer     = NULL;
					gP->stClassRequestDataCB[0].wBufferSize  = 0;
					gP->stClassRequestDataCB[0].wXferSize    = 0;
					gP->stClassRequestDataCB[0].wMaxPktSize  = COMM_ACM_CTRL_MAX_PKT_SIZE;
					gP->stClassRequestDataCB[0].bEpAddress   = 0;
					gP->stClassRequestDataCB[0].bToggle      = 1;

					UsbDevWriteEp(&gP->stClassRequestDataCB[0]);
					break;

				case COMM_REQ_SET_LINE_CODING:

					gP->stClassRequestDataCB[1].DataCallback = NULL;
					gP->stClassRequestDataCB[1].pbBuffer     = (unsigned char *)&gstMcpcLineCode;
					gP->stClassRequestDataCB[1].wBufferSize  = (unsigned short)min(pstDevRequest->wLength, (unsigned short)sizeof(st_LINE_CODE));
					gP->stClassRequestDataCB[1].wXferSize    = 0;
					gP->stClassRequestDataCB[1].wMaxPktSize  = COMM_ACM_CTRL_MAX_PKT_SIZE;
					gP->stClassRequestDataCB[1].bEpAddress   = 0;
					gP->stClassRequestDataCB[1].bToggle      = 1;

					UsbDevReadEp(&gP->stClassRequestDataCB[1]);
					break;

				case COMM_REQ_SET_CONTROL_LINE_STATE:

					UsbSetControlLineState(gP, pstDevRequest);

					gP->stClassRequestDataCB[0].DataCallback = NULL;
					gP->stClassRequestDataCB[0].pbBuffer     = NULL;
					gP->stClassRequestDataCB[0].wBufferSize  = 0;
					gP->stClassRequestDataCB[0].wXferSize    = 0;
					gP->stClassRequestDataCB[0].wMaxPktSize  = COMM_ACM_CTRL_MAX_PKT_SIZE;
					gP->stClassRequestDataCB[0].bEpAddress   = 0;
					gP->stClassRequestDataCB[0].bToggle      = 1;

					UsbDevWriteEp(&gP->stClassRequestDataCB[0]);
					break;

				default:

					UsbDevStallEp(0x00, TRUE);
					break;
				}
			}
			else {
				UsbDevStallEp(0x00, TRUE);
			}
		}
	}
	// vendor(mcpc) request
	else if (pstDevRequest->bmRequestType & REQ_TYPE_VENDOR) {
		// illegal request
		UsbDevStallEp(0x00, TRUE);
	}

	return iErr;
}

/**
 * CommAcmBulkOutDone
 * @return None
 */
void CommAcmBulkOutDone(pst_DATA_CALLBACK pstDataCB)
{
	if (pstDataCB->bEpAddress == COMM_ACM_BULKOUT_EP_ADDRESS)
		UsbVcomBulkOutCallback(pstDataCB->wXferSize);
}

/**
 * CommAcmBulkInDone
 * @return None
 */
void CommAcmBulkInDone(pst_DATA_CALLBACK pstDataCB)
{
	if (pstDataCB->bEpAddress == COMM_ACM_BULKIN_EP_ADDRESS) {
		UsbVcomBulkInCallback();
	}
}
