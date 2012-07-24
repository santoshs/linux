/*	dev_globals.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __DEV_GLOBALS_H__
#define __DEV_GLOBALS_H__

typedef int (*DEINIT_DEV_USB)(void);
typedef int (*INIT_DEV_USB)(void);
typedef void (*INT_DEV_USB)(void);
typedef unsigned short (*CONVERT_EP_TO_CHANNEL)(unsigned char);
typedef int (*STALL_EP)(unsigned char, BOOL);
typedef int (*WRITE_EP)(pst_DATA_CALLBACK);
typedef int (*READ_EP)(pst_DATA_CALLBACK);
typedef ER (*USB_FUNC_PLUG_IN)(void);
typedef ER (*USB_FUNC_PLUG_OUT)(void);
typedef int (*CHECK_EP_STATUS)(unsigned char, unsigned char *);
typedef int (*SET_DEVICE_ADDRESS)(unsigned char);
typedef int (*OPEN_OTHER_ENDPOINT)(void);

typedef struct _st_USB_FUNC_API_TBLS {
	DEINIT_DEV_USB							DeinitDevUSB;
	INIT_DEV_USB							InitDevUSB;
	INT_DEV_USB								IntDevUSB;
	CONVERT_EP_TO_CHANNEL					ConvertEpToChannel;
	WRITE_EP								WriteEp;
	READ_EP									ReadEp;
	STALL_EP								StallEp;
	USB_FUNC_PLUG_IN						UsbFuncPlugIn;
	USB_FUNC_PLUG_OUT						UsbFuncPlugOut;
	CHECK_EP_STATUS							CheckEpStatus;
	SET_DEVICE_ADDRESS						SetDeviceAddress;
	OPEN_OTHER_ENDPOINT						OpenOtherEndpoint;
} st_USB_FUNC_API_TBLS, *pst_USB_FUNC_API_TBLS;

// struct _st_USB_DEV_GLOBALS
// brief Device Stuck Globals Struct
typedef struct _st_USB_DEV_GLOBALS {
	st_DEVICE_DESCRIPTOR stDeviceDescriptor;			// DeviceDescriptor
	st_DEVICE_QUALIFIER	 stDeviceQualifierDesc;			// Add for USB2.0
	st_LANGUAGE_ID		 stLanguageID;					// LanguageID
	st_DATA_CALLBACK	 StandardRequestDataCB[2];		// StandardRequestDataCallBack
	st_USB_DEV_CALLBACK  stUsbDevCallback;				// ClassDriverCallBack
	st_USB_FUNC_API_TBLS stUsbDevFuncTbl;				// USB LowLevel Function
	unsigned long		 uOpenCount;					// OpenCount
	unsigned long		 uDcIntNumber;					// IntNumber
	void				 *pvEpManage[DCD_MAX_EP*2];		// EndPointCallBackNumber
	unsigned char		 bChannelEpTable[DCD_MAX_EP];	// EndPoint to Channel Table
	unsigned char		 *pbConfigDescriptor;			// ConfigDescriptor
	unsigned char		 *pbMStringDescriptor;			// ManufacturerDescriptor
	unsigned char		 *pbPStringDescriptor;			// ProductDescriptor
	unsigned char		 *pbSStringDescriptor;			// Serial NumberDescriptor
	unsigned char		 bInterfaceNumber;				// InterfaceNumber
	unsigned char		 bAltSetting;					// AltSetting
} st_USB_DEV_GLOBALS, *pst_USB_DEV_GLOBALS;

extern pst_USB_DEV_GLOBALS  pstUsbDevGlobal;
#define USB_DEV_GLOBAL()    (pstUsbDevGlobal)

#endif /* __DEV_GLOBALS_H__ */
