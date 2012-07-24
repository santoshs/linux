/*
 * fb_adapt_usb.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __FB_ADAPT_USB_H__
#define __FB_ADAPT_USB_H__
/*******************************
 * Definitions
 *******************************/
 
/*******************************
 * Global data
 *******************************/
 
/*******************************
 * Prototypes
 *******************************/
void comm_init(void);
void comm_remove(void);
int fb_adapt_comm_state(void *param1);
int fb_adapt_comm_init(unsigned char *pbuff);
int fb_adapt_comm_open(void *param1);
int fb_adapt_comm_close(void *param1);
int fb_adapt_comm_receive(unsigned char *pBuff, unsigned long length,
									unsigned long timeout, void* param1);
int fb_adapt_comm_send(unsigned char *pBuff, unsigned long length,
									unsigned long timeout, void* param1);
void fb_adap_comm_register(void);

#endif /* __FB_ADAPT_USB_H__ */