/*
 * fb_comm.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include <string.h>
#include <fb_common.h>
#include "fb_comm.h"
#include "fb_dev_mgmr.h"
 
struct t_fb_comm_operation *usb = NULL;

extern int usb_get_startpos(void);
extern void set_usb_no_recving_state(void);
extern void dma_transfer_cancel(void);
extern unsigned long dma_get_dest_address(void);
extern unsigned long gbBulkOutBuffSize;
extern unsigned char gUsbRecvingState;

#define USB_NO_RECVING_STATE	0

/*
 * fb_comm_init: initialize communication module
 * Input			:None
 * Output			:None
 * Return			:None
 */
void fb_comm_init(void)
{
	/* Choose using device */
	usb = fb_dev_use_dev(DEV_ID_USB);
	if(!usb){
		return;
	}
	usb->fb_dev_comm_init((unsigned char*)COMMAND_BUFF_ADDR);

	/* connect to device */
	fb_comm_connect();
}

/*
 * fb_comm_connect: connect to device
 * Input			:None
 * Output			:None
 * Return			:None
 */
void fb_comm_connect(void)
{
	enum t_fb_dev_comm_state state;		/* state of device */

	/* close device */
	usb->fb_dev_comm_close(UNUSED);

	do {
		/* check status of device */
		state = usb->fb_dev_comm_state(UNUSED);
	} while (state != DEV_COMM_STA_CLOSE);

	/* open device */
	usb->fb_dev_comm_open(UNUSED);
}

/*
 * fb_comm_detect_conn: detect connection
 * Input			:None
 * Output			:None
 * Return: 
 *     	FB_OK
 *		FB_ERR_DISC
 */
int fb_comm_detect_conn(void) 
{
	int ret = FB_OK;

	/* check status of device */
	ret = usb->fb_dev_comm_state(UNUSED);
	if (ret == DEV_COMM_STA_DISC) {
		ret = FB_ERR_DISC;
	} else {
		ret = FB_OK;
	}

	return ret;
}

/*
 * fb_comm_get_cmd	: get command sent by Host
 * Input			:None
 * Output
 * 		@fb_buff	: pointer to received buffer
 * Return: 			:None
 */
void fb_comm_get_cmd(unsigned char *fb_buff)
{
	int ret = FB_OK;

	do {
		/* check device status */
		ret = usb->fb_dev_comm_state(UNUSED);
	} while (ret != DEV_COMM_STA_RECV);

	ret = usb->fb_dev_comm_receive(fb_buff + usb_get_startpos(), MAX_LEN_CMD, 0, UNUSED);
}

/*
 * fb_comm_get_data: get data sent by host
 * Input
 * 		@length			: received size
 * 		@timeout		: timeout
 * Output
 * 		@fb_buff		: pointer to received buffer
 * Return:
 *		Received data length
 *		FB_ERR_PARAM
 *		FB_ERR_OPEN
 *		FB_ERR_DISC
 */
int fb_comm_get_data(unsigned char *fb_buff, unsigned long length,
												unsigned long timeout)
{
	unsigned long rec = 0;
	unsigned long ret = 0;

	if(length % 512 == 0) {
		gbBulkOutBuffSize = length;
	} else {
		gbBulkOutBuffSize = MAX_DMA_TRANSFER_SIZE;
	}

	do {
		/* Check usb connection first */
		ret = usb->fb_dev_comm_state(UNUSED);
		if (ret == DEV_COMM_STA_RECV) {
			ret = usb->fb_dev_comm_receive(fb_buff + usb_get_startpos(), length, timeout, UNUSED);
			if (ret < 0) {
				return ret;
			}
			rec += ret;
		} else if (ret == DEV_COMM_STA_DISC) {
			fb_comm_connect(); /* Try to re-connect */
			break; /* Since Host terminated */
		} else {
		/* Calculate the size of data transfered by DMA */
			ret = dma_get_dest_address();
			ret -= (unsigned long)DATA_BUFF_ADDR;
			if( (length - ret) == 0) {		/* DMA transfer finished already */
				dma_transfer_cancel();		/* Cancel DMA request */				
				set_usb_no_recving_state();	/* Set usb state to no recving data */
				return ret;
			}
		}
	} while (rec < length); /* Until received all transefered data */

	return rec;
}

/*
 * fb_comm_respond: respond message to Host
 * Input
 * 		@msg		: message to send
 * Output			:None
 * Return: 
 *		FB_OK
 *		FB_PARAM_ERR
 *		FB_ERR_OPEN
 *		FB_ERR_DISC
 */
int fb_comm_respond(char *msg)
{
	int ret = FB_OK;
	int len = strlen(msg);

	/* Copy message to sending buffer */
	memcpy((unsigned char*)SEND_BUFF_ADDR, msg, len);

	/* Send data */
	ret = usb->fb_dev_comm_send((unsigned char*)SEND_BUFF_ADDR, len, 0, UNUSED);

	return ret;
}
