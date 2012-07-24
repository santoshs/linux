/*	usb_private.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USB_PRIVATE_H__
#define __USB_PRIVATE_H__

#define USB_MAX_TRANSFER_SIZE	524308	// The maximum size of the transfer data
#define USB_MAX_DMA_TRANSFER_SIZE	((((USB_MAX_TRANSFER_SIZE-1)/32) + 1) * 32)	// The maximum size of the DMA transfer data

#define USB_CONNECT_STATE	 	1
#define USB_NO_CONNECT_STATE	0
#define USB_RECVING_STATE		1
#define USB_NO_RECVING_STATE	0

/* usb_open.c */
int Usb_Connection(void);

/* usb_common.c */
void Usb_Disconnection(void);
void TimeWaitLoop(unsigned long count_us);
void CPU_CONTA_OPORT_ON(void);
void CPU_CONTA_OPORT_OFF(void);
int vbus_check(void);
void SendZLP(void);
void InitializeState(void);
void UsbReadBuffClear(void);


#endif /* __USB_PRIVATE_H__ */
