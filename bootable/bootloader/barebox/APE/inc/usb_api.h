/* usb_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USB_API_H__
#define __USB_API_H__

#include "com_type.h"

typedef enum usb_state
{
	USB_STATE_CLOSE = 0,	/* CLOSE state */
	USB_STATE_OPEN,			/* OPEN state */
	USB_STATE_RECV_DATA,	/* RECV_DATA state */
	USB_STATE_RECVING_DATA,	/* RECVING_DATA state */
	USB_STATE_SEND_DATA,	/* SEND_DATA state */
	USB_STATE_DISCONNECTED,	/* DISCONNECTED state */
} USB_STATE;

typedef struct _usb_info_ {
	ushort	 usb_type;
	char aca;
	char usb_connect;
} USB_INFO;

#define USB_SUCCESS				 0			// Normal end
#define USB_ERR_PARAM			-1			// Parameter error
#define USB_ERR_ALREADY_OPEN	-2			// Open already implementation
#define USB_ERR_NOT_OPEN		-3			// Open unexecuted
#define USB_ERR_OPEN			-4			// Open failure
#define USB_ERR_DISCONNECTED	-5			// USB disconnected

/* usb_init.c */
void usb_init(uchar *pbuff, uchar *pbuff_2nd, ulong length);

/* usb_open.c */
RC usb_open(void);

/* usb_close.c */
RC usb_close(void);

/* usb_check.c */
USB_STATE usb_check(void);

/* usb_receive.c */
RC usb_receive(uchar* pBuff, ulong length, ulong timeout);

/* usb_send.c */
RC usb_send(uchar* pBuff, ulong length, ulong timeout);

void usb_skip_startpos(void);
int usb_get_startpos(void);
unsigned long dma_get_dest_address(void);
void dma_transfer_cancel(void);
void set_usb_no_recving_state(void);
/* usb_phy_init.c */
void usb_phy_init(void);
void usb_phy_disable_func(void);
void usb_phy_enable_func(void);
void usb_phy_reset(void);

/* usb_phy_read.c */
uchar usb_phy_read(ulong reg);

/* usb_phy_write.c */
void usb_phy_write(ulong reg, uchar val);

/* usb_phy_get_info.c */
void usb_phy_get_info(USB_INFO *info);

#endif /* __USB_API_H__ */
