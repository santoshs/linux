/*	usb_check.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usb_api.h"
#include "usbf_api.h"
#include "usb_private.h"
#include "usb_download.h"
#include "comm_acm_dev_vcom.h"
#include "c_def596.h"

// The maximum receive data size that the work buffer inside for the receive data storage can receive data without overflowing
#define USB_MAX_READ_COUNT_FOR_CHECK 3584
#define USBDMA0_DMICR		(*(volatile unsigned long  *)0xE68A0010)

extern void	IntDevUSBCheck(void);
extern void usb_dma_read_buf(void);
extern void usb_skip_startpos(void);
extern void set_usb_no_recving_state(void);
extern int usb_get_startpos(void);
extern void set_usb_no_recving_state(void);
extern unsigned long dma_get_dest_address(void);
extern void dma_transfer_cancel(void);

extern USB_STATE		gUsbState;	// The state of the USB transfer control function
extern unsigned char	gSendZLP;	// The Zero-Length packet transmission right or wrong in case of data transmission completion
extern unsigned char    gUsbRecvingState;
 
extern unsigned long    gRecvDataEndPos;
extern unsigned long    gRecvDataStartPos;
extern unsigned char    gUsbConnectState;

unsigned long 	gTransferCnt = 0;
unsigned char 	gDmaReq = 0;

extern unsigned long gTcountBefore;

/**
 * usb_check - Checks the state of the USB driver
 * @return USB_STATE_CLOSE        : CLOSE state
 *         USB_STATE_OPEN         : OPEN state
 *         USB_STATE_RECV_DATA    : RECV_DATA state
 *         USB_STATE_SEND_DATA    : SEND_DATA state
 *         USB_STATE_DISCONNECTED : DISCONNECTED state
 */
USB_STATE usb_check(void)
{
	// Connect check
	if (vbus_check() == 0) {
		Usb_Disconnection();
		InitializeState();
		gUsbState = USB_STATE_DISCONNECTED;
		return USB_STATE_DISCONNECTED;
	}
	
	// DISCONNECTED state check
	if (gUsbState == USB_STATE_DISCONNECTED) {
		// Transitions to the CLOSE state
		gUsbState = USB_STATE_CLOSE;
	}
	
	// CLOSE state check
	if (gUsbState == USB_STATE_CLOSE) {
		return USB_STATE_CLOSE;
	}

	IntDevUSBCheck();
	if (GetBulkInTransferFlag_Check()) {
		// The flag during transmission of the USB control function is ON
		gUsbState = USB_STATE_SEND_DATA;
		return USB_STATE_SEND_DATA;
	}

	if (gSendZLP == 1) {
		// Zero-Length packet transmission
		SendZLP();
		gSendZLP = 0;
	}

	// check for read data in USBDMA
	unsigned short reg_val = USBDMA0_DMICR;
	if ((reg_val & 0x00003) != 0) {
		usb_dma_read_buf();
	}
	
	if ((gRecvDataEndPos-gRecvDataStartPos) > 0) {
		return USB_STATE_RECV_DATA;
	} else {
		gUsbState = USB_STATE_OPEN;
		return USB_STATE_OPEN;
	}
	
	// RECEIVING state check
	if (gUsbRecvingState == USB_RECVING_STATE) {
		return USB_STATE_RECVING_DATA;
	}
	
}


/**
 * usb_dma_read_buf - Processes the interrupt of the USBHS-DMAC module
 * @return None
 */
void usb_dma_read_buf(void)
{
	unsigned long tcount_after;
	TBL_USBF_PARAM param;
	
	USBDMA_CHCR0 &= (U32)~IE;
	USBDMA_CHCR0 &= (U32)~NULLE;
	if (gUsbRecvingState == USB_RECVING_STATE) {
		gUsbRecvingState = USB_NO_RECVING_STATE;
	} else {
		return;
	}
	
	if (USBDMA_CHCR0 & NULL_BIT) {
		// Cancels the DMA transfer request permission of FIFO port select register(D0FIFOSEL)
		USB_CLR_PAT(D0FIFOSEL, DREQE);
		
		// Sets FTE bit, clear NULL bit, negate DE bit
		USBDMA_CHCR0 |= (U32)FTE;
		USBDMA_CHCR0 &= (U32)~NULL_BIT;
		USBDMA_CHCR0 &= (U32)~DE;
	}

	PipeFlag[PIPE2] = PIPE_DONE;
	PipeDataSize[PIPE2] -= dtcnt[PIPE2];
	rdcnt[PIPE2] -= dtcnt[PIPE2];					// Data Count

	USBRD(USBDMA_DMATCR0, tcount_after);
	gRecvDataEndPos += ((gTcountBefore - tcount_after) * 32);

    // User program
    param.pipenum       = PIPE2;
    param.param_data    = USBF_NULL;
    UsbF_CallBack( CBID_OUT_CPL, &param );
}


/**
 * usb_skip_startpos - Skip start position of gbBulkOutBuff
 * @return None
 */
void usb_skip_startpos(void)
{
	if (gRecvDataStartPos % 32) {
		// start position is not 32 multiples
		// start position makes 32 multiples
		gRecvDataStartPos += (32 - (gRecvDataStartPos % 32));
	}
	
	if ((gRecvDataStartPos >= gRecvDataEndPos) && (gUsbRecvingState == USB_NO_RECVING_STATE)) {
		// initialize receive buffer
		UsbReadBuffClear();
	}
}

/**
 * usb_get_startpos - Get start position of gbBulkOutBuff
 * @return Start position of gbBulkOutBuff
 */
int usb_get_startpos(void)
{
	return gRecvDataStartPos;
}

unsigned long dma_get_dest_address()
{
	unsigned long ret = 0;
	USBRD(USBDMA_DAR0, ret);
	return ret;
}

void dma_transfer_cancel()
{
USB_CLR_PAT(D0FIFOSEL, DREQE);
	// Sets FTE bit, clear NULL bit, negate DE bit
	USBDMA_CHCR0 |= (ulong)FTE;
	USBDMA_CHCR0 &= (ulong)~NULL_BIT;
	USBDMA_CHCR0 &= (ulong)~DE;
}
void set_usb_no_recving_state()
{
	gUsbRecvingState = USB_NO_RECVING_STATE;
}