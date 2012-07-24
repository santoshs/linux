/*	usb_common.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "usb_api.h"
#include "usb_private.h"
#include "cpu_register.h"
#include "comm_acm_dev_api.h"
#include "usbdrv/dev_stack/usbhs/usbf_api.h"
#include "c_typedef.h"
#include "c_defusr.h"
#include "c_def596.h"


#define RSEL_H				0x10										// RSEL high
#define DRVCR_H				0x02										// DRVCR high

#define DRVCR_ULPI				(volatile unsigned long*)(0xE6058196)	

USB_STATE	gUsbState;	// The state of the USB transfer control function
extern unsigned char 	gUsbRecvingState;
extern unsigned char    gUsbConnectState;
extern unsigned long    gRecvDataEndPos;
extern unsigned long    gRecvDataStartPos;

/**
 * Usb_Disconnection - Processes USB disconnection
 * @return None
 */
void Usb_Disconnection(void)
{
	CommAcmDevClose();
	USB_CLR_PAT(PIPECFG, TYP);
	TimeWaitLoop(100);
	UsbF_SetDp(0);
	TimeWaitLoop(100);

	// RECEIVING state check 
	if (gUsbRecvingState == USB_RECVING_STATE) {
		// Cancels the DMA transfer request permission of FIFO port select register(D0FIFOSEL)
		USB_CLR_PAT(D0FIFOSEL, DREQE);

		// Sets FTE bit, clear NULL bit, negate DE bit
		USBDMA_CHCR0 |= (U32)FTE;
		USBDMA_CHCR0 &= (U32)~NULL_BIT;
		USBDMA_CHCR0 &= (U32)~DE;
	}

}

/**
 * TimeWaitLoop - Waits in the specified time
 * @return None
 */
void TimeWaitLoop(unsigned long count_us)
{
	unsigned long count ;

	count = count_us * 3000 ;

	while(count) {
		count -- ;
	}
}

/**
 * CPU_CONTA_OPORT_ON - Processes in the USB connection
 * @return None
 */
void CPU_CONTA_OPORT_ON(void)
{
	while (1) {
		if (vbus_check() == 0x0001) {
			break;
		}
	}
	
	
}

/**
 * CPU_CONTA_OPORT_OFF - Processes USB disconnection
 * @return None
 */
void CPU_CONTA_OPORT_OFF(void)
{
	return;
}

/**
 * vbus_check - Acquires the condition of VBUS
 * @return 0 : VBUS LOW condition (USB disconnection)
 *         1 : VBUS HIGH condition (USB is connected)
 */
int vbus_check(void)
{
	unsigned long	portdata ;

	portdata = SYSSTS ;

	if ((portdata & SYSSTS_VBUSST_MASK) == SYSSTS_VBUSST_VBUSVALID) {
		// VBUS HIGH
		return 1;
	}

	// VBUS LOW
	return 0; 
}

/**
 * SendZLP - Send zero-Length packet
 * @return None
 */
void SendZLP(void)
{
	// Sets BVAL to 1(Zero-Length packet transmission)
	CFIFOCTR |= BVAL;
}

/**
 * InitializeState - Initializes state
 * @return None
 */
void InitializeState(void)
{
	gUsbConnectState	= USB_NO_CONNECT_STATE;	// Initializes the connection condition of the USB transfer control unit
	gUsbRecvingState	= USB_NO_RECVING_STATE;	// Initializes the reception condition of the USB transfer control unit
}

/**
 * UsbReadBuffClear - Initializes a buffer for the receive data storage
 * @return None
 */
void UsbReadBuffClear(void)
{
	gRecvDataEndPos = 0;	// Initializes the receive data ending point
	gRecvDataStartPos = 0;	// Initializes the receive data starting point

	EnableIntR(PIPE2);		// Enable Ready Interrupt
}
