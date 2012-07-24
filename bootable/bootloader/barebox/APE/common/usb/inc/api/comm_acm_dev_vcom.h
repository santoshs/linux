/*	comm_acm_dev_vcom.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */


#ifndef __COMM_ACM_DEV_VCOM_H__
#define __COMM_ACM_DEV_VCOM_H__


#define _VCOMBUFFERSIZE_    (BULK_IN_FIFO_SIZE)          // Vcom(FIFO) buffer size

/**
 * VCOM communication setting structure
 */
typedef struct _USB_VCOM_SETTING
{
    unsigned short TimeOut;                 // time out(for compatibility)
    void (*RxNotify)(int size);             // callback of Data Receive
    int RxNotifyCount;                      // Receive data count
    void (*TxNotify)(int size);             // callback of Data Send
    int TxNotifyCount;                      // Send data count
} USB_VCOM_SETTING, *PUSB_VCOM_SETTING;

/**
 * FIFO structure for VCOM
 */
typedef struct _SFifo
{
    char    bBuff[_VCOMBUFFERSIZE_];        // data buffer
    int     iFifoSize;                      // FIFO size
    int     iRPoint;                        // pointer of Data Receive
    int     iWPoint;                        // pointer of Data Send
    int     iCount;                         // number of data
} SFifo,*pSFifo;

// API of Vcom
void UsbVcomInitialize(void);
void UsbVcomSetting(PUSB_VCOM_SETTING pVcomParam);
int UsbVcomWriteData(unsigned char *pBuff, int iDataSize);
int UsbVcomReadData(unsigned char *pBuff, int iDataSize);
int UsbVcomReadCount(void);
void UsbVcomReadFifoClear(void);
void UsbVcomWriteFifoClear(void);

unsigned char GetBulkInTransferFlag_Check(void);

#endif  /* __COMM_ACM_DEV_VCOM_H__ */
