/* dev_api.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/

#include "usbd_def.h"
#include "usbd_types.h"
#include "usbd_system.h"
#include "usb.h"
#include "dev_api.h"
#include "dev_globals.h"

#include "usbfunc_hs.h"

#include "usb_download.h"

pst_USB_DEV_GLOBALS pstUsbDevGlobal = NULL;
unsigned char	gb_UseUSB_HISPEED_IP = 0 ;

/**
 * UsbDevOpen
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevOpen(void)
{
	int                 iErr;
	pst_USB_DEV_GLOBALS gP;

	iErr = 0;
	gP   = USB_DEV_GLOBAL();

	// check MemoryResorse
	if (gP != NULL) {
		// Already opened
		gP->uOpenCount++;
		iErr = USB_DEV_STATUS_ALREADY_OPENED;
		return iErr;
	}

	// USB Device Stack Memory Allocate
	gP = pstUsbDevGlobal = (pst_USB_DEV_GLOBALS)MEM_ALLOC(sizeof(st_USB_DEV_GLOBALS));
	if (gP == NULL) {
		// Failed
		iErr = USB_DEV_STATUS_NO_MEMORY;
		return iErr;
	}

	MEM_SET(gP, 0, sizeof(st_USB_DEV_GLOBALS));
	gP->uOpenCount=0;

	// G3 HS
	// USBMD Check
	if (gb_UseUSB_HISPEED_IP == 1) {
		// HiSpeed IP
		gP->stUsbDevFuncTbl.DeinitDevUSB = DeinitDevUSB_HS;
		gP->stUsbDevFuncTbl.InitDevUSB = InitDevUSB_HS;
		gP->stUsbDevFuncTbl.IntDevUSB = IntDevUSB_HS;
		gP->stUsbDevFuncTbl.ConvertEpToChannel = ConvertEpToChannel_HS;
		gP->stUsbDevFuncTbl.WriteEp = WriteEp_HS;
		gP->stUsbDevFuncTbl.ReadEp = ReadEp_HS;
		gP->stUsbDevFuncTbl.StallEp = StallEp_HS;
		gP->stUsbDevFuncTbl.UsbFuncPlugIn = UsbFuncPlugIn_HS;
		gP->stUsbDevFuncTbl.UsbFuncPlugOut = UsbFuncPlugOut_HS;
		gP->stUsbDevFuncTbl.CheckEpStatus = CheckEpStatus_HS;
		gP->stUsbDevFuncTbl.SetDeviceAddress = SetDeviceAddress_HS;
		gP->stUsbDevFuncTbl.OpenOtherEndpoint = OpenOtherEndpoint_HS;
	}
	else {
		// FullSpeed IP
		iErr = USB_DEV_STATUS_NO_MEMORY;
		return iErr;
	}

	gP->uOpenCount++;

	return 0;
}


/**
 * UsbDevClose
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevClose(void)
{
	int                 iErr;
	pst_USB_DEV_GLOBALS gP;

	gP = USB_DEV_GLOBAL();

	if (gP == NULL) {
		iErr = USB_DEV_STATUS_NOT_OPENED;
		return iErr;
	}

	if (--gP->uOpenCount) {
		iErr = USB_DEV_STATUS_STILL_OPENED;
		return iErr;
	}

	// End
	iErr = gP->stUsbDevFuncTbl.DeinitDevUSB();// G3 HS
	if (iErr) {
		;
	}

	// StringDescriptor(Serial) memory pool release
	if (gP->pbSStringDescriptor) {
		MEM_FREE(gP->pbSStringDescriptor);
		gP->pbSStringDescriptor = NULL;
	}

	// StringDescriptor(Product) memory pool release
	if (gP->pbPStringDescriptor) {
		MEM_FREE(gP->pbPStringDescriptor);
		gP->pbPStringDescriptor = NULL;
	}

	// StringDescriptor(Manufacturer) memory pool release
	if (gP->pbMStringDescriptor) {
		MEM_FREE(gP->pbMStringDescriptor);
		gP->pbMStringDescriptor = NULL;
	}

	// ConfigrationDescriptor memory pool release
	if (gP->pbConfigDescriptor) {
		MEM_FREE(gP->pbConfigDescriptor);
		gP->pbConfigDescriptor = NULL;
	}

	// USB device stuck memory pool release
	MEM_FREE(pstUsbDevGlobal);
	pstUsbDevGlobal = NULL;

	return iErr;
}


/**
 * UsbDevRegisterClass
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevRegisterClass(pst_USB_REG_DEV_CLASS pstUsbRegDevClass)
{
	int 				  iErr;					// ReturnCode
	unsigned short		  wLength;				// ConfigDescriptorLength
	pst_USB_DEV_CALLBACK  pstUsbDevCallback;	// ClassDriverCallBack
	pst_DEVICE_DESCRIPTOR pstDeviceDescriptor;	// DeviceDescriptorAddress
	pst_CONFIG_DESCRIPTOR pstConfigDescriptor;	// ConfigDescriptorAddress
	pst_STRING_DESCRIPTOR pstStringDescriptor;	// StringDescriptorAddress
	pst_USB_DEV_GLOBALS   gP;					// USB Device Stack Info

	gP                = USB_DEV_GLOBAL();
	iErr              = 0;
	pstUsbDevCallback = &pstUsbRegDevClass->stUsbDevCallback;

	// Check USB Device Stuck memory pool
	if (gP == NULL) {
		// Memory not opened
		iErr = USB_DEV_STATUS_NOT_OPENED;
		return iErr;
	}

	MEM_COPY(&gP->stUsbDevCallback, pstUsbDevCallback, sizeof(st_USB_DEV_CALLBACK));

	// DeviceDescriptor is copied onto USB device stack information
	pstDeviceDescriptor = (pst_DEVICE_DESCRIPTOR)pstUsbRegDevClass->pDeviceDescriptor;
	MEM_COPY((void *)&gP->stDeviceDescriptor, (void *)pstDeviceDescriptor, (unsigned long)pstDeviceDescriptor->bLength);

	// DEVICE_QUALIFIER
	gP->stDeviceQualifierDesc.bLength = 10 ;
	gP->stDeviceQualifierDesc.bDescriptorType = DESC_TYPE_DEVICE_QUAL ;
	gP->stDeviceQualifierDesc.bcdUSB = gP->stDeviceDescriptor.bcdUSB ;
	gP->stDeviceQualifierDesc.bDeviceClass = gP->stDeviceDescriptor.bDeviceClass ;
	gP->stDeviceQualifierDesc.bDeviceSubClass = gP->stDeviceDescriptor.bDeviceSubClass ;
	gP->stDeviceQualifierDesc.bDeviceProtocol = gP->stDeviceDescriptor.bDeviceProtocol ;
	gP->stDeviceQualifierDesc.bMaxPacketSize0 = gP->stDeviceDescriptor.bMaxPacketSize0 ;
	gP->stDeviceQualifierDesc.bNumConfigurations = 0 ;
	gP->stDeviceQualifierDesc.bReserved = 0 ;

	// ConfigrationDescriptor is copied onto USB device stack information
	pstConfigDescriptor = (pst_CONFIG_DESCRIPTOR)pstUsbRegDevClass->pConfigDescriptor;
	wLength = pstConfigDescriptor->wTotalLength;

	// securing memory pool (ConfigrationDescriptor)
	gP->pbConfigDescriptor = MEM_ALLOC((unsigned long)wLength);
	if (gP->pbConfigDescriptor == NULL) {
		// Memory securing failed
		iErr = USB_DEV_STATUS_NO_MEMORY;
		return iErr;
	}

	// ConfigrationDescriptor is copied onto secured memory pool
	MEM_COPY(gP->pbConfigDescriptor, (void *)pstConfigDescriptor, (unsigned long)wLength);

	// language ID is copied onto USB device stack information
	pstStringDescriptor = (pst_STRING_DESCRIPTOR)pstUsbRegDevClass->pLangID;
	MEM_COPY((void *)&gP->stLanguageID, (void *)pstStringDescriptor, (unsigned long)pstStringDescriptor->bLength);

	// language ID is copied onto USB device stack information
	if (gP->stDeviceDescriptor.iManufacturer) {
		pstStringDescriptor = (pst_STRING_DESCRIPTOR)pstUsbRegDevClass->pManufacturer;
		gP->pbMStringDescriptor = (unsigned char *)MEM_ALLOC((unsigned long)pstStringDescriptor->bLength);
		if (gP->pbMStringDescriptor == NULL) {
			iErr = USB_DEV_STATUS_NO_MEMORY;
			if (gP->pbConfigDescriptor) {
				MEM_FREE(gP->pbConfigDescriptor);
				gP->pbConfigDescriptor = NULL;
			}
			return iErr;
		}
		MEM_COPY(gP->pbMStringDescriptor, (void *)pstStringDescriptor, (unsigned long)pstStringDescriptor->bLength);
	}

	// Product ID is copied onto USB device stack information
	if (gP->stDeviceDescriptor.iProduct) {
		pstStringDescriptor = (pst_STRING_DESCRIPTOR)pstUsbRegDevClass->pProduct;
		gP->pbPStringDescriptor = MEM_ALLOC((unsigned long)pstStringDescriptor->bLength);
		if (gP->pbPStringDescriptor == NULL) {
			iErr = USB_DEV_STATUS_NO_MEMORY;
			if (gP->pbMStringDescriptor) {
				MEM_FREE(gP->pbMStringDescriptor);
				gP->pbMStringDescriptor = NULL;
			}
			if (gP->pbConfigDescriptor) {
				MEM_FREE(gP->pbConfigDescriptor);
				gP->pbConfigDescriptor = NULL;
			}
			return iErr;
		}
		MEM_COPY(gP->pbPStringDescriptor, (void *)pstStringDescriptor, (unsigned long)pstStringDescriptor->bLength);
	}

	// Serial ID is copied onto USB device stack information
	if (gP->stDeviceDescriptor.iSerialNumber) {
		pstStringDescriptor = (pst_STRING_DESCRIPTOR)pstUsbRegDevClass->pSerial;
		gP->pbSStringDescriptor = MEM_ALLOC((unsigned long)pstStringDescriptor->bLength);
		if (gP->pbSStringDescriptor == NULL) {
			iErr = USB_DEV_STATUS_NO_MEMORY;
			if (gP->pbPStringDescriptor) {
				MEM_FREE(gP->pbPStringDescriptor);
				gP->pbPStringDescriptor = NULL;
			}
			if (gP->pbMStringDescriptor) {
				MEM_FREE(gP->pbMStringDescriptor);
				gP->pbMStringDescriptor = NULL;
			}
			if (gP->pbConfigDescriptor) {
				MEM_FREE(gP->pbConfigDescriptor);
				gP->pbConfigDescriptor = NULL;
			}
			return iErr;
		}
		MEM_COPY(gP->pbSStringDescriptor, (void *)pstStringDescriptor, (unsigned long)pstStringDescriptor->bLength);
	}

	gP->StandardRequestDataCB[0].wChannel    = gP->StandardRequestDataCB[1].wChannel    = 0;
	gP->StandardRequestDataCB[0].bEpAddress  = gP->StandardRequestDataCB[1].bEpAddress  = 0;
	gP->StandardRequestDataCB[0].wMaxPktSize = gP->StandardRequestDataCB[1].wMaxPktSize = gP->stDeviceDescriptor.bMaxPacketSize0;

	return iErr;
}


/**
 * UsbDevInitController
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevInitController(void)
{
	int 				iErr;	// ReturnCode
	pst_USB_DEV_GLOBALS gP;		// USB Device Info

	iErr = 0;
	gP   = USB_DEV_GLOBAL();

	if (gP->uOpenCount == DEV_CLASS_DRIVER_NUM) {
		// Set Interapt MaskLevel
		gP->uDcIntNumber = DC_IRQ_NUM;

		iErr = gP->stUsbDevFuncTbl.InitDevUSB();// G3 HS
		if (iErr) {
			goto END;
		}
	}
	else {
		iErr = USB_DEV_STATUS_ALL_CLASS_NOT_INIT;
		goto END;
	}

END:
	return iErr;
}


/**
 * UsbDevWriteEp
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevWriteEp(pst_DATA_CALLBACK pstDataCallback)
{
	int 				iErr;	// ReturnCode
	pst_USB_DEV_GLOBALS gP;		// USB Device Stack Info

	iErr = 0;
	gP   = USB_DEV_GLOBAL();

	// Check USB Device Stuck memory pool
	if (gP == NULL) {
		// Memory not opened
		iErr = USB_DEV_STATUS_NOT_OPENED;
		return iErr;
	}

	// Convert Ep To Channel
	pstDataCallback->wChannel = (unsigned short)gP->stUsbDevFuncTbl.ConvertEpToChannel(pstDataCallback->bEpAddress);// G3 HS

	// Data write endpoint, and data send interrupt Enable
	iErr = gP->stUsbDevFuncTbl.WriteEp(pstDataCallback);	// G3 HS
	if (iErr) {
		// Write EndPoint Failled
		iErr = USB_DEV_STATUS_INVALID_PARAMETER;
		return iErr;
	}

	return iErr;
}


/**
 * UsbDevReadEp
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevReadEp(pst_DATA_CALLBACK pstDataCallback)
{
	int 				iErr;	// ReturnCode
	pst_USB_DEV_GLOBALS gP;		// USB Device Stack Info
	UH					wChannel;

	iErr = 0;
	gP   = USB_DEV_GLOBAL();

	// Check USB Device Stuck memory pool
	if (gP == NULL) {
		// Memory not opened
		iErr = USB_DEV_STATUS_NOT_OPENED;
		return iErr;
	}

	// Epx Set Call Back
	if(pstDataCallback->bEpAddress != 0) {
		wChannel = (unsigned short)gP->stUsbDevFuncTbl.ConvertEpToChannel(pstDataCallback->bEpAddress);// G3 HS
		gP->pvEpManage[wChannel * 2 + 1] = (void *)pstDataCallback;
		return 0 ;
	}

	// Convert Ep To Channel
	pstDataCallback->wChannel = (unsigned short)gP->stUsbDevFuncTbl.ConvertEpToChannel(pstDataCallback->bEpAddress);// G3 HS

	// Data read endpoint, and data receive interrupt Enable
	iErr = gP->stUsbDevFuncTbl.ReadEp(pstDataCallback);	// G3 HS
	if (iErr) {
		// Read EndPoint Failed
		iErr = USB_DEV_STATUS_INVALID_PARAMETER;
		return iErr;
	}

	return iErr;
}


/**
 * UsbDevStallEp
 * @return        0 : Normal end
 *         Except 0 : Abnormal end
 */
int UsbDevStallEp(unsigned char bEpAddress, BOOL bFlag)
{
	int 				iErr;	// ReturnCode
	pst_USB_DEV_GLOBALS gP;		// USB Device Stack Info

	iErr = 0;
	gP	 = USB_DEV_GLOBAL();

	// Check USB Device Stuck memory pool
	if (gP == NULL) {
		// Memory not opened
		iErr = USB_DEV_STATUS_NOT_OPENED;
		return iErr;
	}

	// Set EndPoint Status
	iErr = gP->stUsbDevFuncTbl.StallEp(bEpAddress, bFlag);	// G3 HS

	return iErr;
}


/**
 * UsbDevPlugIn
 * @return        CUDD_RC_NOR : Normal end
 *         Except CUDD_RC_NOR : Abnormal end
 */
ER UsbDevPlugIn(UB	len_product, UB	*id_product, UB	len_usb_serial, UB	*id_usb_serial,
				UB	len_vender, UB	*id_vender)
{
	ER						iErr;
	pst_USB_DEV_GLOBALS		gP;					// USB Device Stack Info

	gP                = USB_DEV_GLOBAL();
	iErr              = CUDD_RC_NOR;

	if (gP == NULL) {
		// No Memory
		iErr = CUDD_RC_NG_PHASE;
		return iErr;
	}

	gP->stUsbDevFuncTbl.UsbFuncPlugIn();	// G3 HS

	if (len_vender != 0) {
		MEM_COPY(gP->pbMStringDescriptor, id_vender, (unsigned long)len_vender);
	}

	if (len_product != 0) {
		MEM_COPY(gP->pbPStringDescriptor, id_product, (unsigned long)len_product);
	}

	if (len_usb_serial != 0) {
		MEM_COPY(gP->pbSStringDescriptor, id_usb_serial, (unsigned long)len_usb_serial);
	}

	return iErr ;
}


/**
 * IntDevUSBCheck
 * @return None
 */
void	IntDevUSBCheck(void)
{
	pst_USB_DEV_GLOBALS		gP;					// USB Device Stack Info

	gP                = USB_DEV_GLOBAL();
	gP->stUsbDevFuncTbl.IntDevUSB();	// G3 HS
}


/**
 * UsbDevSetConfigEnd
 * @return None
 */
void UsbDevSetConfigEnd(pst_DATA_CALLBACK pstDataCallbackArg)
{
	pst_USB_DEV_GLOBALS    gP;					// USB Device Stack Info

	gP = USB_DEV_GLOBAL();

	// EP other than EP0 are opened
	gP->stUsbDevFuncTbl.OpenOtherEndpoint();	// G3 HS

	if (gP->stUsbDevCallback.SetConfigHandler != NULL) {
		// Call classdriver configration notify function
		(gP->stUsbDevCallback.SetConfigHandler)(gP->stUsbDevCallback.pvArg);
	}
}
