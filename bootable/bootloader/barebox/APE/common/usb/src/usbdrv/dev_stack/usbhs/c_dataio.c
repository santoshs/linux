/* C_DataIO.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "divmod.h"
#include "c_typedef.h"
#include "c_defusr.h"
#include "usb_private.h"

extern unsigned long gRecvDataEndPos;
extern unsigned char *gbBulkOutBuff;
extern unsigned char *gbBulkOutBuff_2nd;
extern unsigned long gbBulkOutBuffSize;
extern unsigned char gUsbRecvingState;
extern unsigned long 	gTransferCnt;
unsigned long gTcountBefore = 0;


/**
 * Send_Start - Send Data start
 * @return DONE  : Done
 *         ERROR : Error
 */
U16 Send_Start(U16 Pipe, U32 Bsize, U8 *Table)
{
	U16	EndFlag_K;

	SetNAK(Pipe);									// Set NAK
	dtcnt[Pipe] = Bsize;
	dtptr[Pipe] = (U8 *)Table;
	PipeIgnore[Pipe]	= 0;						// Ignore count clear
	PipeFlag[Pipe] = PIPE_WAIT;

	USB_CLR_STS(BEMPSTS, BitSet[Pipe]);				// BEMP Status Clear
	USB_CLR_STS(BRDYSTS, BitSet[Pipe]);				// BRDY Status Clear
	EndFlag_K	= Buf_Write(Pipe);
	SetBUF(Pipe);									// Set BUF
	return	(EndFlag_K);							// End or Err or Continue
}


/**
 * Buf_Write - Buffer Write
 * @return DONE  : Done
 *         ERROR : Error
 */
U16 Buf_Write(U16 Pipe)
{
	U16	EndFlag_K;

	PipeIgnore[Pipe]	= 0;						// Ignore count clear
	EndFlag_K	= Buf_Write_C(Pipe);

	switch (EndFlag_K) {
	case	WRITING:								// Continue of data write
		EnableIntR(Pipe);							// Enable Ready Interrupt
		break;
	case	WRITEEND:								// End of data write
	case	WRITESHRT:								// End of data write
		DisableIntR(Pipe);							// Disable Ready Interrupt
		EnableIntE(Pipe);							// Enable Empty Interrupt
		break;
	case	FIFOERROR:								// FIFO access error
	default:
		DisableIntR(Pipe);							// Disable Ready Interrupt
		DisableIntE(Pipe);							// Disable Empty Interrupt
		PipeFlag[Pipe] = PIPE_IDLE;
		break;
	}
	return	(EndFlag_K);							// End or Err or Continue
}


/**
 * Buf_Write_C - Buffer Write
 * @return DONE  : Done
 *         ERROR : Error
 */
U16 Buf_Write_C(U16 Pipe)
{
	U16	size, count, even, buffer, mxps;
	U16	EndFlag_K;
	int	i ;

	if (Pipe==PIPE0) {
		buffer	= FPortChange1(Pipe,CUSE,ISEL);
	}
	else {
		buffer	= FPortChange1(Pipe,CUSE,NO);
	}
	if (buffer==FIFOERROR) {						// FIFO access error
		return	(FIFOERROR);
	}
	size	= getBufSize(Pipe);						// Data buffer size
	mxps	= getMaxPacketSize(Pipe);				// Max Packet Size

	if (dtcnt[Pipe] <= (U32)size) {
		EndFlag_K = WRITEEND;						// write continues
		count = (U16)dtcnt[Pipe];
		if ( count == 0 ) {
			EndFlag_K = WRITESHRT;					// Null Packet is end of write
		}
		if ( UMOD32(count,mxps)!=0 ) {
			EndFlag_K = WRITESHRT;					// Short Packet is end of write
		}
	}
	else {
		EndFlag_K = WRITING;						// write continues
		count = size;
	}

	USBRD(CFIFOSEL, buffer);
	if ((buffer & MBW) == MBW_8) {					// 8bit FIFO access mode
		for (even = count; (even!=0) ; --even) {
			USBWR_FF(CFIFO_8, *dtptr[Pipe]);
			dtptr[Pipe]++;
		}
	}
	else if( (buffer & MBW) == MBW_16 ) {			// 16bit FIFO access mode
		for (even = (U16)(count>>1); (even!=0) ; --even) {
			USBWR_FF(CFIFO, *((U16 *)dtptr[Pipe]));
			dtptr[Pipe]++;
			dtptr[Pipe]++;
		}
		if ((count & (U16)0x0001) != 0) {			// if count == odd
			USBWR_FF(CFIFO_8, *dtptr[Pipe]);
			dtptr[Pipe]++;
		}
	}
	else {											// 32bit FIFO access mode
		for (even = (U16)(count>>2); (even!=0) ; --even) {
			USBWR_FF(CFIFO_32, *((U32 *)dtptr[Pipe]));
			dtptr[Pipe]+=4;
		}

		if ((count & (U16)0x0003) != 0) {
			USBRD(CFIFOSEL, buffer);
			buffer &= ~MBW ;
			USBWR(CFIFOSEL, buffer);

			for (i = 0 ; i< (count & 0x0003) ; i++) {
				USBWR_FF(CFIFO_32, *((U8 *)dtptr[Pipe]));
				dtptr[Pipe]++;
			}

			USBRD(CFIFOSEL, buffer);
			buffer |= MBW_32 ;
			USBWR(CFIFOSEL, buffer);
		}
	}

	if (dtcnt[Pipe] < (U32)size) {
		dtcnt[Pipe] = 0;
		USBRD(CFIFOCTR, buffer);
		if ((buffer & BVAL) == 0) {
			USBWR(CFIFOCTR, BVAL);					// Short Packet
		}
	}
	else {
		dtcnt[Pipe] -= count;
	}
	return	(EndFlag_K);							// End or Err or Continue
}


/**
 * Receive_Start - Receive Data start
 * @return None
 */
void Receive_Start(U16 Pipe, U32 Bsize, U8 *Table)
{
	SetNAK(Pipe);									// Set NAK
	dtcnt[Pipe] = Bsize;
	dtptr[Pipe] = (U8 *)Table;
	PipeIgnore[Pipe]	= 0;						// Ignore count clear

	rdcnt[Pipe] = Bsize;
	PipeDataSize[Pipe] = Bsize;
	PipeFlag[Pipe] = PIPE_WAIT;

	SetBUF(Pipe);									// Set BUF
	EnableIntR(Pipe);								// Enable Ready Interrupt
}


/**
 * Buf_Read - Buffer Read
 * @return DONE  : Done
 *         ERROR : Error
 */
U16 Buf_Read(U16 Pipe)
{
	U16	EndFlag_K;
    TBL_USBF_PARAM param;

	PipeIgnore[Pipe]	= 0;						// Ignore count clear
	EndFlag_K	= Buf_Read_C(Pipe);

	switch (EndFlag_K) {
	case	READING:								// Continue of data read
		break;
	case	READEND:								// End of data read
	case	READSHRT:								// End of data read
		DisableIntR(Pipe);
		PipeFlag[Pipe] = PIPE_DONE;
		PipeDataSize[Pipe] -= dtcnt[Pipe];
		rdcnt[Pipe] -= dtcnt[Pipe];					// Data Count

        // User program
        param.pipenum       = Pipe;
        param.param_data    = USBF_NULL;
        UsbF_CallBack( CBID_OUT_CPL, &param );
		break;
	case	READOVER:								// buffer over
		USBWR(CFIFOCTR, BCLR);						// Clear BVAL
		DisableIntR(Pipe);							// Disable Ready Interrupt
		PipeFlag[Pipe] = PIPE_IDLE;
		PipeDataSize[Pipe] -= dtcnt[Pipe];
		rdcnt[Pipe] -= dtcnt[Pipe];					// Data Count
		break;
	case	FIFOERROR:								// FIFO access error
	default:
		DisableIntR(Pipe);							// Disable Ready Interrupt
		PipeFlag[Pipe] = PIPE_IDLE;
		break;
	}
	return	(EndFlag_K);							// End or Err or Continue
}


/**
 * Buf_Read_C - Buffer Read
 * @return DONE  : Done
 *         ERROR : Error
 */
U16 Buf_Read_C(U16 Pipe)
{
	U16	count, even, buffer, mxps, dtln;
	U16	EndFlag_K;
    TBL_USBF_PARAM param;
    TBL_IN_DATABUF inbuf;
    U32 cb_retbal;
	U8	tempbuf[4];

	buffer	= FPortChange1(Pipe,CUSE,NO);
	if (buffer==FIFOERROR) {						// FIFO access error
		return	(FIFOERROR);
	}
	dtln	= (U16)(buffer & DTLN);

	inbuf.dtin_cnt = dtln ;
	// User program
    param.pipenum       = Pipe;
    param.param_data    = (void *)&inbuf;
    cb_retbal = UsbF_CallBack( CBID_OUT_REQ_BUF, &param );

    if (cb_retbal == USBF_E_OK) {
        dtptr[Pipe] = *inbuf.in_buf;			    // SetBuff
        dtcnt[Pipe] = dtln;          				// request size
    }

	mxps	= getMaxPacketSize(Pipe);				// Max Packet Size

	if (dtcnt[Pipe] < dtln) {						// Buffer Over
		EndFlag_K = READOVER;
		count = (U16)dtcnt[Pipe];
	}
	else if (dtcnt[Pipe] == dtln) {					// just Receive Size
		EndFlag_K = READEND;
		count = dtln;
		if (count == 0) {
			EndFlag_K = READSHRT;					// Null Packet receive
		}
		if ( UMOD32(count,mxps)!=0 ) {
			EndFlag_K = READSHRT;					// Short Packet receive
		}
	}
	else {											// continus Receive data
		EndFlag_K = READING;
		count = dtln;
		if (count == 0) {
			EndFlag_K = READSHRT;					// Null Packet receive
		}
		if ( UMOD32(count,mxps)!=0 ) {
			EndFlag_K = READSHRT;					// Short Packet receive
		}
	}

	if (count == 0) {								// 0 length packet
		USBWR(CFIFOCTR, BCLR);						// Clear BVAL
	}
	else {
		USBRD(CFIFOSEL, buffer);
		if ((buffer & MBW) == MBW_8) {				// 8bit FIFO access mode
			for (even = count; (even!=0) ; --even) {
				USBRD_FF(CFIFO_8, *dtptr[Pipe]);
				dtptr[Pipe]++;
			}
		}
		else if ((buffer & MBW) == MBW_16) {		// 16bit FIFO access mode
			for (even = (U16)(count>>1); (even!=0) ; --even) {
				USBRD_FF(CFIFO, *((U16 *)dtptr[Pipe]));
				dtptr[Pipe]++;
				dtptr[Pipe]++;
			}
			if ((count & (U16)0x0001) != 0) {		// if count == odd
				USBRD_FF(CFIFO_8, *dtptr[Pipe]);
				dtptr[Pipe]++;
			}
		}
		else {
			for (even = (U16)(count>>2); (even!=0) ; --even) {
				USBRD_FF(CFIFO_32, *((U32 *)dtptr[Pipe]));
				dtptr[Pipe]+=4;
			}

			if ((count & (U16)0x0003) != 0) {
				USBRD_FF(CFIFO_32, *((U32 *)tempbuf));

				if ((count & (U16)0x0002) != 0) {
					*((U16 *)dtptr[Pipe]) = *((U16 *)&tempbuf[0]) ;
					dtptr[Pipe]++;
					dtptr[Pipe]++;

					if ((count & (U16)0x0001) != 0) {
						*((U8 *)dtptr[Pipe]) = *((U8 *)&tempbuf[2]) ;
						dtptr[Pipe]++;
					}
				}
				else {
					if ((count & (U16)0x0001) != 0) {
						*((U8 *)dtptr[Pipe]) = *((U8 *)&tempbuf[0]) ;
						dtptr[Pipe]++;
					}
				}
			}
		}
	}

	dtcnt[Pipe] -= count;
	return	(EndFlag_K);							// End or Err or Continue
}

/**
 * DMA_Read - DMA Read
 * @return None
 */
void DMA_Read(void)
{
	U16	EndFlag_K;
    TBL_USBF_PARAM param;

	PipeIgnore[PIPE2]	= 0;						// Ignore count clear
	EndFlag_K	= DMA_Read_C();

	switch (EndFlag_K) {
	case	READING:								// Continue of data read
		break;
	case	READEND:								// End of data read
		PipeFlag[PIPE2] = PIPE_DONE;
		PipeDataSize[PIPE2] -= dtcnt[PIPE2];
		rdcnt[PIPE2] -= dtcnt[PIPE2];					// Data Count

        // User program
        param.pipenum       = PIPE2;
        param.param_data    = USBF_NULL;
        UsbF_CallBack( CBID_OUT_CPL, &param );
		break;
	case	FIFOERROR:								// FIFO access error
	default:
		PipeFlag[PIPE2] = PIPE_IDLE;
		break;
	}
	return;							// End or Err or Continue
}


/**
 * DMA_Read_C - DMA Read
 * @return DONE  : Done
 *         ERROR : Error
 */
U16 DMA_Read_C(void)
{
	U16	buffer, dtln;
	U32	buf;

	buffer	= FPortChange1(PIPE2,D0DMA,NO);
	if (buffer==FIFOERROR) {						// FIFO access error
		return	(FIFOERROR);
	}
	DisableIntR(PIPE2);
	dtln	= (U16)(buffer & DTLN);
	if ((gTransferCnt % 2) == 0) {
		dtptr[PIPE2] = gbBulkOutBuff;			    // SetBuff
	} else {
		dtptr[PIPE2] = gbBulkOutBuff_2nd;			    // SetBuff
	}
    dtcnt[PIPE2] = dtln;          				// request size
	dtptr[PIPE2] += gRecvDataEndPos;

	buf	  = USBDMA_CHCR0;								/* dummy read ( for clear TE ) */
	USBDMA_SAR0 = (U32)&CFIFO_32+12;							/* FIFO port address */
	USBDMA_DAR0 = (U32)dtptr[PIPE2];								/* Table address */
	USBDMA_CHCR0 = (U32)(TS_LONG);					/* Read, burst, 32bit */
	if (dtcnt[PIPE2]  == 0){
		USBWR(D0FIFOCTR, BCLR);						// Clear BVAL
		return	(READEND);
	} else if (dtcnt[PIPE2] == 512) {
		gTcountBefore = ((gbBulkOutBuffSize - gRecvDataEndPos) / 32);
		USBDMA_DMATCR0 = gTcountBefore;
		gTcountBefore -= 1;
		USBDMA_CHCR0 |= (U32)NULLE;
	} else {
		gTcountBefore =(U32)(((dtcnt[PIPE2] - 1) / 32) + 1);
		USBDMA_DMATCR0 = gTcountBefore;
	}
	USBDMA_CHCR0 |= (U32)IE;
	USBDMA_CHCR0 |= (U32)DE;
	USB_MDF_PAT(D0FIFOSEL, MBW_32, MBW);
	USB_SET_PAT(D0FIFOSEL, DREQE);
	gUsbRecvingState = 1;		// set for USB_RECVING_STATE
	return	(READING);
}


/**
 * FPortChange1 - Pipe Change
 * @return Except FIFOERROR : Normal end
 *                FIFOERROR : Abnormal end
 */
U16 FPortChange1(U16 Pipe, U16 fifosel, U16 isel)
{
	U16	buffer, i;

	switch (fifosel) {
	case	CUSE:
		USB_MDF_PAT(CFIFOSEL, (isel | Pipe), (ISEL | CURPIPE));		// ISEL=1, CURPIPE=0
		break;
	case	D0USE:
	case	D0DMA:
		USB_MDF_PAT(D0FIFOSEL, Pipe, CURPIPE);
		break;
	case	D1USE:
	case	D1DMA:
		USB_MDF_PAT(D1FIFOSEL, Pipe, CURPIPE);
		break;
	default:
		break;
	}

	for (i=0;i<4;i++) {
		switch (fifosel) {
		case	CUSE:
			USBRD(CFIFOCTR, buffer);
			break;
		case	D0USE:
		case	D0DMA:
			USBRD(D0FIFOCTR, buffer);
			break;
		case	D1USE:
		case	D1DMA:
			USBRD(D1FIFOCTR, buffer);
			break;
		default:
			buffer	= 0;
			break;
		}
		if ((U16)(buffer & FRDY) == FRDY) {
			return	(buffer);
		}
	}
	return	(FIFOERROR);
}

/**
 * FPortChange2 - Pipe Change
 * @return None
 */
void FPortChange2(U16 Pipe, U16 fifosel, U16 isel)
{
	switch (fifosel) {
	case	CUSE:
		USB_MDF_PAT(CFIFOSEL, (isel | Pipe), (ISEL | CURPIPE));		/* ISEL=1, CURPIPE=0 */
		break;
	case	D0USE:
	case	D0DMA:
		USB_MDF_PAT(D0FIFOSEL, Pipe, CURPIPE);
		break;
	case	D1USE:
	case	D1DMA:
		USB_MDF_PAT(D1FIFOSEL, Pipe, CURPIPE);
		break;
	default:
		break;
	}
}
