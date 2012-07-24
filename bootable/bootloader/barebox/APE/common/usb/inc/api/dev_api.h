/*	dev_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __DEV_API_H__
#define __DEV_API_H__

#include "error_base.h"

#define USB_DEV_STATUS_NOT_OPENED           (USB_DEV_STATUS_BASE + 0x00000001L)     // device stack isn't open
#define USB_DEV_STATUS_STILL_OPENED         (USB_DEV_STATUS_BASE + 0x00000002L)     // device stack is still open
#define USB_DEV_STATUS_ALREADY_OPENED       (USB_DEV_STATUS_BASE + 0x00000003L)     // device stack has already opened
#define USB_DEV_STATUS_NO_MEMORY            (USB_DEV_STATUS_BASE + 0x00000004L)     // memory shortage
#define USB_DEV_STATUS_INVALID_PARAMETER    (USB_DEV_STATUS_BASE + 0x00000005L)     // invalid parameter
#define USB_DEV_STATUS_ALL_CLASS_NOT_INIT   (USB_DEV_STATUS_BASE + 0x00000007L)     // all ClassDriver hasn't opened

typedef int (*REQ_HANDLER)(void *pvArg, pst_DEVICE_REQUEST pstDevRequest);
typedef int (*DEV_CALLBACK)(void *pvArg);

/**
 * request handler struct
 */
typedef struct _st_USB_DEV_CALLBACK {
    void            *pvArg;                 // pointer to class private data
    REQ_HANDLER     ClassOrVendorReqHandler;// callback when class or vendor request received
    DEV_CALLBACK    SetConfigHandler;       // callback when set configuration received
} st_USB_DEV_CALLBACK, *pst_USB_DEV_CALLBACK;

/**
 * class driber struct
 */
typedef struct _st_USB_REG_DEV_CLASS {
    PUCHAR              pDeviceDescriptor;  // pointer to DeviceDescroptor
    PUCHAR              pConfigDescriptor;  // pointer to ConfigurationDescroptor
    PUCHAR              pLangID;            // pointer to LanguageID
    PUCHAR              pManufacturer;      // pointer to StringDescriptor(Manufacture)
    PUCHAR              pProduct;           // pointer to StringDescriptor(Product)
    PUCHAR              pSerial;            // pointer to StringDescriptor(Serial)
    PUCHAR              pDeviceQualifier;
    PUCHAR              pOtherSpeedCfgQualifier;
    st_USB_DEV_CALLBACK stUsbDevCallback;   // callback structure
} st_USB_REG_DEV_CLASS, *pst_USB_REG_DEV_CLASS;

struct _st_DATA_CALLBACK;

typedef void (*DATA_CALLBACK)(struct _st_DATA_CALLBACK *pstDataCallbackArg);

/**
 * data transfer callback struct
 */
typedef struct _st_DATA_CALLBACK {
    int             iStatus;                // result of transfer
    DATA_CALLBACK   DataCallback;           // callback function when transfer done
    PUCHAR          pbBuffer;               // buffer pointer for transfer data
    USHORT          wBufferSize;            // requested size to be transfered
    USHORT          wDataSize;              // data size to be transfered
    USHORT          wXferSize;              // actual transfered size
    USHORT          wMaxPktSize;            // max packet size of endpoint
    USHORT          wChannel;               // channel(endpoint) number
    UCHAR           bEpAddress;             // endpoint addres
    UCHAR           bToggle;                // toggle bit
} st_DATA_CALLBACK, *pst_DATA_CALLBACK;

int UsbDevOpen(void);
int UsbDevClose(void);
int UsbDevRegisterClass(pst_USB_REG_DEV_CLASS pstUsbRegDevClass);
int UsbDevInitController(void);
int UsbDevWriteEp(pst_DATA_CALLBACK pstDataCallback);
int UsbDevReadEp(pst_DATA_CALLBACK pstDataCallback);
int UsbDevStallEp(UCHAR bEpAddress, BOOL bFlag);

#endif /* __DEV_API_H__ */
