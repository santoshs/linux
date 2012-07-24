/*	usb_init.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "usb_api.h"
#include "usb_private.h"
#include "cpu_register.h"
#include "common.h"
#include "c_def596.h"

unsigned long			gUSB_DMAC_BaseAddr;		// The base address of DMAC
unsigned long			gUSB_BaseAddr;			// The base address of USB high speed module (USBHS)

extern USB_STATE		gUsbState;				// The state of the USB transfer control function
extern unsigned char	gb_UseUSB_HISPEED_IP;	// The use right or wrong of HiSpeed IP(1:Uses, Except 1:Doesn't use)
extern unsigned char	gSendZLP;				// The Zero-Length packet transmission right or wrong in case of data transmission completion(1:Transmits Zero-Length packet, Except 1:Doesn't transmit Zero-Length packet)

extern unsigned char	*gbBulkOutBuff;
extern unsigned char	*gbBulkOutBuff_2nd;
extern unsigned long	gbBulkOutBuffSize;
extern unsigned long 	gTransferCnt;

/**
 * usb_init - Initializes the resource to use in the USB control API
 * @return None
 */
void usb_init(uchar *pbuff, uchar *pbuff_2nd, ulong length)
{
	gUSB_BaseAddr			= USB_BASE_REG;
	gb_UseUSB_HISPEED_IP	= 1;
	gSendZLP				= 0;
	
	/* Transitions to the CLOSE state */
	gUsbState				= USB_STATE_CLOSE;

	/* Initializes for USBDMA */
	gUSB_DMAC_BaseAddr		= USB_DMAC_BASE_REG;
	/* Set USBHS-DMAC operation */
	SMSTPCR2				&= ~MSTP214;

	/* Initialize PHY */
	usb_phy_init();

	InitializeState();

	gbBulkOutBuff			= pbuff;
	gbBulkOutBuff_2nd		= pbuff_2nd;
	gbBulkOutBuffSize		= length;
	gTransferCnt = 0;

	CFIFOSEL_USB &= ~(MBW|ISEL);
	CFIFOSEL_USB |= MBW_32;

	D0FIFOSEL_USB &= ~MBW;
	D0FIFOSEL_USB |= MBW_32;
	
	// setting for USBDMA
	USBDMA_CHCR0_USB &= ~TS;
	USBDMA_CHCR0_USB |= INT_ENABLE;

}

