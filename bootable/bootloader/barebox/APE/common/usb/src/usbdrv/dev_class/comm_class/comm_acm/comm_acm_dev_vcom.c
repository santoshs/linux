/* comm_acm_dev_vcom.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "usbd_def.h"
#include "usbd_types.h"
#include "usb_download.h"
#include "usbd_system.h"
#include "usb.h"
#include "dev_api.h"
#include "comm_acm_dev_globals.h"
#include "comm_acm_dev_vcom.h"


extern unsigned char         gbBulkInBuff[BULK_IN_BUFF_SIZE];
extern SFifo                 BulkOutFifo;
extern SFifo                 BulkInFifo;

// for VCOM Notification
void (*RxNotify)(int size);
int  iRxNotifyCount;
void (*TxNotify)(int size);
int  iTxNotifyCount;

void UsbReadBuffClear(void);

/**
 * FifoInit
 * @return 0
 */
int FifoInit(pSFifo fifo)
{
	// initialize Fifo
	fifo->iFifoSize = _VCOMBUFFERSIZE_;
	fifo->iCount    = 0;
	fifo->iRPoint   = 0;
	fifo->iWPoint   = 0;

	return 0;
}

/**
 * FifoRead
 * @return Read size
 */
int FifoRead(pSFifo fifo, char *bBuff, int iSize)
{
	int iTrsSize;
	int iRetSize;

	if (iSize > fifo->iCount)
		iSize = fifo->iCount;
	iRetSize = iSize;
	if (iRetSize == 0) {
		return iRetSize;
	}

	// loopback
	if ((iSize + fifo->iRPoint) > fifo->iFifoSize) {
		iTrsSize = fifo->iFifoSize - fifo->iRPoint;
		MEM_COPY((void *)bBuff, (void *)&fifo->bBuff[fifo->iRPoint], (unsigned long)iTrsSize);
		iSize         -= iTrsSize;
		fifo->iRPoint  = 0;
		fifo->iCount  -= iTrsSize;
		bBuff         += iTrsSize;
	}
	MEM_COPY((void *)bBuff, (void *)&fifo->bBuff[fifo->iRPoint], (unsigned long)iSize);
	fifo->iRPoint += iSize;
	fifo->iCount  -= iSize;

	return iRetSize;
}

/**
 * FifoWrite
 * @return Write size
 */
int FifoWrite(pSFifo fifo, char *bBuff, int iSize)
{
	int  iTrsSize;
	int  iRetSize;

	if (iSize > (fifo->iFifoSize - fifo->iCount))
		iSize = (fifo->iFifoSize - fifo->iCount);
	iRetSize = iSize;
	if (iRetSize == 0) {
		return iRetSize;
	}

	if ((iSize + fifo->iWPoint) > fifo->iFifoSize) {
		iTrsSize = fifo->iFifoSize - fifo->iWPoint;
		MEM_COPY((void *)&fifo->bBuff[fifo->iWPoint], (void *)bBuff, (unsigned long)iTrsSize);
		iSize         -= iTrsSize;
		fifo->iWPoint  = 0;
		fifo->iCount  += iTrsSize;
		bBuff         += iTrsSize;
	}
	MEM_COPY((void *)&fifo->bBuff[fifo->iWPoint], (void *)bBuff, (unsigned long)iSize);
	fifo->iWPoint += iSize;
	fifo->iCount  += iSize;

	return iRetSize;
}

/**
 * FifoRemain
 * @return Remain size
 */
int FifoRemain(pSFifo fifo)
{
	return (fifo->iFifoSize - fifo->iCount);
}

/**
 * FifoCount
 * @return Fifo size
 */
int FifoCount(pSFifo fifo)
{
	return (fifo->iCount);
}

/**
 * FifoClear
 * @return None
 */
void FifoClear(pSFifo fifo)
{
	fifo->iCount = 0;
	fifo->iRPoint = 0;
	fifo->iWPoint = 0;
}

/**
 * UsbVcomInitialize
 * @return None
 */
void UsbVcomInitialize(void)
{
	RxNotify        = NULL;
	iRxNotifyCount  = 0;
	TxNotify        = NULL;
	iTxNotifyCount  = 0;

	// communication class initialize
	UsbCommInitialize();
}

/**
 * UsbVcomSetting
 * @return None
 */
void UsbVcomSetting(PUSB_VCOM_SETTING pVcomParam)
{
	pst_COMM_ACM_DEV_GLOBALS gP;

	gP = COMM_ACM_DEV_GLOBAL();

	FifoInit(&BulkInFifo);
	UsbReadBuffClear();

	RxNotify=pVcomParam->RxNotify;
	TxNotify=pVcomParam->TxNotify;

	if (pVcomParam->RxNotifyCount <= _VCOMBUFFERSIZE_)
		iRxNotifyCount=pVcomParam->RxNotifyCount;
	else
		iRxNotifyCount=_VCOMBUFFERSIZE_;
	if (iRxNotifyCount) { // Threshold Mode
		if (gP->stBulkOutDataCB.wBufferSize > iRxNotifyCount)
			gP->stBulkOutDataCB.wDataSize = (unsigned short)iRxNotifyCount;
		else
			gP->stBulkOutDataCB.wDataSize = gP->stBulkOutDataCB.wBufferSize;
	}
	else { // Poling Mode
		gP->stBulkOutDataCB.wDataSize = 1;
	}

	if (pVcomParam->TxNotifyCount > _VCOMBUFFERSIZE_)
		iTxNotifyCount = _VCOMBUFFERSIZE_;
	else
		iTxNotifyCount = pVcomParam->TxNotifyCount;
}

/**
 * UsbVcomBulkOutCallback
 * @return None
 */
void UsbVcomBulkOutCallback(unsigned short wDataSize)
{
	int                      iDataSize;
	int                      iFifoRemain;
	pst_COMM_ACM_DEV_GLOBALS gP;

	iFifoRemain = 0;
	gP = COMM_ACM_DEV_GLOBAL();

	// there is remainder in Bulk-Out Fifo
	iFifoRemain = FifoRemain(&BulkOutFifo);

	if (iFifoRemain >= wDataSize) {
		FifoWrite(&BulkOutFifo, (char *)gP->stBulkOutDataCB.pbBuffer, (int)wDataSize);

		// callback
		iDataSize = FifoCount(&BulkOutFifo);

		if ((iRxNotifyCount != 0) && (iDataSize >= iRxNotifyCount) && (RxNotify != NULL)) {
			RxNotify(iDataSize);
		}

		CommAcmGetBulkOut();
	}
	else {
		gP->uBulkOutFifoOverDataSize = wDataSize;
	}
}

/**
 * UsbVcomBulkInCallback
 * @return None
 */
void UsbVcomBulkInCallback(void)
{
	int                      iFifoSize;
	int                      iSendSize;
	int                      iRemainSize;
	pst_COMM_ACM_DEV_GLOBALS gP;

	iRemainSize = 0;
	gP = COMM_ACM_DEV_GLOBAL();

	iFifoSize = FifoCount(&BulkInFifo);

	if (iFifoSize) {
		// Data Transfer
		iFifoSize = min(iFifoSize, COMM_ACM_BULKIN_BUFF_SIZE);

		iSendSize = FifoRead(&BulkInFifo, (char *)gbBulkInBuff, iFifoSize);

		if(iSendSize) {
			CommAcmPutBulkIn(gbBulkInBuff, iSendSize);
		}
	}
	else {
		gP->bBulkInTransferFlag = DCD_COMM_BULKIN_DATA_NOT_TRANSFER;
	}

	iRemainSize = FifoCount(&BulkInFifo);
	// callback
	if ((iTxNotifyCount != 0) && (iRemainSize <= iTxNotifyCount) && (TxNotify != NULL))
		TxNotify(iRemainSize);
}

/**
 * UsbVcomWriteData
 * @return Write size
 */
int UsbVcomWriteData(unsigned char *pBuff, int iDataSize)
{
	int                      iWriteSize;
	int                      iFifoSize;
	int                      iSendSize;
	pst_COMM_ACM_DEV_GLOBALS gP;

	gP = COMM_ACM_DEV_GLOBAL();


	if (iDataSize == 0)
		return 0;

	// Data store to Fifo
	iWriteSize = FifoWrite(&BulkInFifo, (char *)pBuff, iDataSize);

	++gP->uBulkInTimeoutCallbackFlag;

	if (!gP->bBulkInTransferFlag) {
		// Data Transfer
		iFifoSize = FifoCount(&BulkInFifo);

		iFifoSize = min(iFifoSize, COMM_ACM_BULKIN_BUFF_SIZE);

		iSendSize = FifoRead(&BulkInFifo, (char *)gbBulkInBuff, iFifoSize);

		if (iSendSize)
			CommAcmPutBulkIn(gbBulkInBuff, iSendSize);
	}

	return iWriteSize;
}

/**
 * UsbVcomReadData
 * @return Read size
 */
int UsbVcomReadData(unsigned char *pBuff, int iDataSize)
{
	int  iSize;
	int  iSendSize;

	iSize = FifoCount(&BulkOutFifo);

	if (iSize == 0) {
		return 0;
	}
	else {
		iSendSize = FifoRead(&BulkOutFifo, (char *)pBuff, iDataSize);
		return iSendSize;
	}
}

/**
 * UsbVcomReadCount
 * @return Read size
 */
int UsbVcomReadCount(void)
{
	int  ret;

	ret = FifoCount(&BulkOutFifo);

	return ret;
}

/**
 * UsbVcomWriteCount
 * @return Write size
 */
int UsbVcomWriteCount(void)
{
	int  ret;

	ret = FifoCount(&BulkInFifo);

	return ret;
}

/**
 * UsbVcomReadFifoClear
 * @return None
 */
void UsbVcomReadFifoClear(void)
{
	FifoClear(&BulkOutFifo);
}

/**
 * UsbVcomWriteFifoClear
 * @return None
 */
void UsbVcomWriteFifoClear(void)
{
	FifoClear(&BulkInFifo);
}

/**
 * GetBulkInTransferFlag_Check
 * @return bBulkInTransferFlag
 */
unsigned char GetBulkInTransferFlag_Check(void)
{
	pst_COMM_ACM_DEV_GLOBALS gP;
	gP = COMM_ACM_DEV_GLOBAL();

	return(gP->bBulkInTransferFlag);
}
