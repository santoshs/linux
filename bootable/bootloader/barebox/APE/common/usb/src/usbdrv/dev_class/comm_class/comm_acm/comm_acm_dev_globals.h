/* comm_acm_dev_globals.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __COMM_ACM_DEV_GLOBALS_H__
#define __COMM_ACM_DEV_GLOBALS_H__

// struct _st_COMM_ACM_DEV_GLOBALS
// brief ACM Information Struct
typedef struct _st_COMM_ACM_DEV_GLOBALS
{
    unsigned long    uOpenCount;                 // Open Count
    st_DATA_CALLBACK stClassRequestDataCB[2];    // Class Request CallBack
    st_DATA_CALLBACK stBulkOutDataCB;            // BulkOut Data CallBack
    st_DATA_CALLBACK stBulkInDataCB;             // BulkIn Data CallBack
    unsigned long    uBulkInTimeoutCallbackFlag; // Send Data CallBackFlag(0:disable 1:enable)
    unsigned long    uCommAcmDevTaskID;          // VCOM TaskID
    unsigned long    uBulkOutFifoOverDataSize;   // VCOMFIFO OverFlowSize(Byte)
    unsigned char    bBulkOutTransferFlag;       // BulkOut TransferFlag(0:no data transmittingr 1:data transmitting)
    unsigned char    bBulkInTransferFlag;        // BulkIn TransferFlag(0:no data transmittingr 1:data transmitting)
} st_COMM_ACM_DEV_GLOBALS, *pst_COMM_ACM_DEV_GLOBALS;

extern pst_COMM_ACM_DEV_GLOBALS     pstCommAcmDevGlobal;
#define COMM_ACM_DEV_GLOBAL()       (pstCommAcmDevGlobal)

// Interface Max Packet Size
#define COMM_ACM_CTRL_MAX_PKT_SIZE          0x0040			// Control Max Packet Size
#define COMM_ACM_BULK_MAX_PKT_SIZE          0x0040			// Bulk Max Packet Size
#define COMM_ACM_BULK_MAX_PKT_SIZE_HS       0x0200			// HiSpeed Bulk Max Packet Size

// Interface Buffer Size
#define COMM_ACM_BULKIN_BUFF_SIZE           4096			// BulkIn Buffer Size
#define COMM_ACM_BULKOUT_BUFF_SIZE          4096			// BulkOut Buffer Size

// Interface Setting
#define COMM_ACM_COMM_CLASS_INTERFACE_NUM   0x00			// Comunication Class Interface

#define COMM_ACM_BULKOUT_EP_ADDRESS         0x02			// Data Class Interface for Bulkin EndPoint
#define COMM_ACM_BULKIN_EP_ADDRESS          0x81			// Data Class Interface for Bulkout EndPoint

// BulkIn Data Transfer Flag
#define DCD_COMM_BULKIN_DATA_NOT_TRANSFER   0				// with no data transmitting
#define DCD_COMM_BULKIN_DATA_TRANSFER       1				// with data transmitting

// comm_acm_dev_tools.c
int CommAcmDevConfigNotify(void *);
void CommAcmGetBulkOut(void);
void CommAcmPutBulkIn(unsigned char *, int);
int  CommAcmDevClassRequest(void *, pst_DEVICE_REQUEST);
void CommAcmBulkInDone(pst_DATA_CALLBACK);

int UsbCommInitialize(void);
int UsbSetControlLineState(void *, pst_DEVICE_REQUEST);

void UsbVcomBulkOutCallback(unsigned short);
void UsbVcomBulkInCallback(void);

extern const unsigned char gDeviceDescriptor[];
extern const unsigned char gConfigDescriptor[];
extern const unsigned char gLangID[];
extern const unsigned char gManufacturerString[];
extern const unsigned char gProductString[];
extern const unsigned char gSerialString[];

extern const unsigned char gDeviceQualifier[];
extern const unsigned char gOtherSpeedCfgQualifier[];

#endif /* __COMM_ACM_DEV_GLOBALS_H__ */
