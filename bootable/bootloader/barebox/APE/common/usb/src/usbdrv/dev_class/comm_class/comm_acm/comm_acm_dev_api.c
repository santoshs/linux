/* comm_acm_dev_api.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "usbd_def.h"
#include "usbd_types.h"
#include "usbd_system.h"
#include "usb.h"
#include "dev_api.h"
#include "comm_acm_dev_api.h"
#include "usb_download.h"
#include "comm_acm_dev_vcom.h"
#include "comm_acm_dev_globals.h"
#include "usb_module_control.h"

pst_COMM_ACM_DEV_GLOBALS pstCommAcmDevGlobal = NULL;
st_USBDEV_MODCHG_GLOBALS stUsbDevModchgGlobal;

ER UsbDevPlugIn(UB	len_product, UB	*id_product, UB	len_usb_serial, UB	*id_usb_serial,
				UB	len_vender, UB	*id_vender);

/**
 * CommAcmDevOpen
 * @return        0 : Normal end 
 *         Except 0 : Abnormal end
 */
int CommAcmDevOpen(void)
{
	int                       iErr;
	st_USB_REG_DEV_CLASS      stUsbRegDevClass;
	pst_COMM_ACM_DEV_GLOBALS  gP;
	pst_USBDEV_MODCHG_GLOBALS gPMdChg;

	iErr    = 0;
	gP      = COMM_ACM_DEV_GLOBAL();
	gPMdChg = USBDEV_MODCHG_GLOBAL();

	MEM_SET((void*)gPMdChg, (int)0, sizeof(st_USBDEV_MODCHG_GLOBALS));

	if (gPMdChg == NULL) {
		iErr = COMM_ACM_DEV_STATUS_NO_MEMORY;
		return iErr;
	}

	gPMdChg->bUsbmoduleState = USBDEV_STATE_NORMAL; // USB driver is normal state

	if (gP != NULL) {
		gP->uOpenCount++;
		iErr = COMM_ACM_DEV_STATUS_ALREADY_OPENED;
		return iErr;
	}

	MEM_INITIALIZE();

	gP = pstCommAcmDevGlobal = (pst_COMM_ACM_DEV_GLOBALS)MEM_ALLOC(sizeof(st_COMM_ACM_DEV_GLOBALS));
	if (gP == NULL) {
		iErr = COMM_ACM_DEV_STATUS_NO_MEMORY;
		return iErr;
	}
	MEM_SET((void*)gP, (int)0, sizeof(st_COMM_ACM_DEV_GLOBALS));

	gPMdChg->uCommAcmDevTaskID = gP->uCommAcmDevTaskID;

	iErr = UsbDevOpen();
	if (iErr == USB_DEV_STATUS_ALREADY_OPENED) {
		iErr = COMM_ACM_DEV_STATUS_SUCCESS;
	}
	else if (iErr) {
		gPMdChg->uCommAcmDevTaskID = 0;
		MEM_FREE((void*)pstCommAcmDevGlobal);
		pstCommAcmDevGlobal = NULL;
		return iErr;
	}

	stUsbRegDevClass.pDeviceDescriptor = (PUCHAR)gDeviceDescriptor;
	stUsbRegDevClass.pConfigDescriptor = (PUCHAR)gConfigDescriptor;
	stUsbRegDevClass.pLangID = (PUCHAR)gLangID;
	stUsbRegDevClass.pManufacturer = (PUCHAR)gManufacturerString;
	stUsbRegDevClass.pProduct = (PUCHAR)gProductString;
	// stUsbRegDevClass.pSerial = (PUCHAR)NULL;
	stUsbRegDevClass.pSerial = (PUCHAR)gSerialString;
	stUsbRegDevClass.pDeviceQualifier = (PUCHAR)gDeviceQualifier;
	stUsbRegDevClass.pOtherSpeedCfgQualifier = (PUCHAR)gOtherSpeedCfgQualifier;
	stUsbRegDevClass.stUsbDevCallback.pvArg = gP;
	stUsbRegDevClass.stUsbDevCallback.ClassOrVendorReqHandler = CommAcmDevClassRequest;
	stUsbRegDevClass.stUsbDevCallback.SetConfigHandler = CommAcmDevConfigNotify;
	iErr = UsbDevRegisterClass(&stUsbRegDevClass);
	if (iErr) {
		UsbDevClose();
		gPMdChg->uCommAcmDevTaskID = 0;
		MEM_FREE((void*)pstCommAcmDevGlobal);
		pstCommAcmDevGlobal = NULL;
		return iErr;
	}

	gPMdChg->bUsbDevModchgOpenFlag = USBDEV_MODE_COMM_ACM;

	iErr = UsbDevInitController();
	if (iErr == USB_DEV_STATUS_ALL_CLASS_NOT_INIT) {
		iErr = COMM_ACM_DEV_STATUS_SUCCESS;
	}
	else if (iErr) {
		UsbDevClose();
		gPMdChg->uCommAcmDevTaskID = 0;
		MEM_FREE((void*)pstCommAcmDevGlobal);
		pstCommAcmDevGlobal = NULL;
		return iErr;
	}

	UsbDevPlugIn(0x00,0x00,0x00,0x00,0x00,0x00);

	gP->uOpenCount++;

	return iErr;
}

/**
 * CommAcmDevClose
 * @return        0 : Normal end 
 *         Except 0 : Abnormal end
 */
int CommAcmDevClose(void)
{
	int                       iErr;
	pst_COMM_ACM_DEV_GLOBALS  gP;
	pst_USBDEV_MODCHG_GLOBALS gPMdChg;

	iErr    = 0;
	gP      = COMM_ACM_DEV_GLOBAL();
	gPMdChg = USBDEV_MODCHG_GLOBAL();

	if (gP == NULL) {
		iErr = COMM_ACM_DEV_STATUS_NOT_OPENED;
		return iErr;
	}

	if (--gP->uOpenCount) {
		iErr = COMM_ACM_DEV_STATUS_STILL_OPENED;
		return iErr;
	}

	iErr = UsbDevClose();
	if (iErr == USB_DEV_STATUS_STILL_OPENED) {
		iErr = COMM_ACM_DEV_STATUS_SUCCESS;
	}

	gPMdChg->bUsbDevModchgOpenFlag = USBDEV_MODE_NONE;

	gPMdChg->uCommAcmDevTaskID = 0;

	MEM_FREE((void*)pstCommAcmDevGlobal);
	pstCommAcmDevGlobal = NULL;

	return iErr;
}
