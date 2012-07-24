/*
 * fb_adapt_usb.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include <usb_api.h>
#include <fb_common.h>
#include "fb_dev_mgmr.h"
#include "fb_adapt_usb.h"

 /*
 * fb_adapt_comm_init(): initialization for USB device
 * Input
 * 		@param1	:UNUSED.
 * Output		:None. 
 * Return
 *		FB_OK.
 *		FB_ERR_DISC.
 */
int fb_adapt_comm_init(unsigned char *pBuff)
{
	/* USB initialization */
	usb_init(pBuff, (unsigned char *)DATA_BUFF_ADDR, MAX_DMA_TRANSFER_SIZE );

	return FB_OK;

}
 /*
 * fb_adapt_comm_state(): check state of USB device
 * Input
 * 		@param1	: UNUSED.
 * Output		:None.
 * Return
 *		DEV_COMM_STA_DISC.
 *		DEV_COMM_STA_OPEN.
 *		DEV_COMM_STA_CLOSE.
 *		DEV_COMM_STA_RECV.
 *		DEV_COMM_STA_SEND.
 */
int fb_adapt_comm_state(void *param1)
{
	enum t_fb_dev_comm_state ret;

	/* check USB state */
	ret = usb_check();
	if (ret == USB_STATE_DISCONNECTED) {
		ret = DEV_COMM_STA_DISC;
	} else if ( ret == USB_STATE_OPEN) {
		ret = DEV_COMM_STA_OPEN;
	} else if (ret == USB_STATE_CLOSE) {
		ret = DEV_COMM_STA_CLOSE;
	} else if (ret == USB_STATE_RECV_DATA) {
		ret = DEV_COMM_STA_RECV;
	} else if (ret == USB_STATE_SEND_DATA) {
		ret = DEV_COMM_STA_SEND;
	}

	return ret;
}
/*
 * fb_adapt_comm_open(): open USB device
 * Input
 * 		@param1	:UNUSED.
 * Output		:None.
 * Return
 *		FB_OK.
 *		FB_ERR_OPEN.
 *		FB_ERR_DISC.
 *		FB_ERR_ALREADY_OPEN.
 */
int fb_adapt_comm_open(void *param1)
{
	int ret = USB_SUCCESS;

	/* open USB device */
	ret = usb_open();
	if (ret == USB_SUCCESS) {
		ret = FB_OK;
	} else if (ret == USB_ERR_OPEN) {
		ret = FB_ERR_OPEN;
	} else if (ret == USB_ERR_DISCONNECTED) {
		ret = FB_ERR_DISC;
	} else {
		ret = FB_ERR_ALREADY_OPEN;
	}

	return ret;
}

/*
 * fb_adapt_comm_close(): close USB device
 * Input
 * 		@param1	: UNUSED.
 * Output		:None.
 * Return
 *		FB_OK.
 *		FB_ERR_DISC.
 */
int fb_adapt_comm_close(void *param1)
{
	int ret = USB_SUCCESS;

	/* close USB device */
	ret = usb_close();
	if (ret != USB_SUCCESS) {
		return FB_ERR_DISC;
	}

	return FB_OK;
}

/*
 * fb_adapt_comm_receive(): receive data from USB buffer
 * Input
 * 		@pBuff	: buffer to stored data
 * 		@length	: length of data received
 * 		@timeout: timeout
 * 		@param1	: UNUSED.
 * Output		: None.
 * Return
 *		Size of received data.
 *		FB_OK
 *		FB_ERR_PARAM
 *		FB_ERR_OPEN
 *		FB_ERR_DISC
 */
int fb_adapt_comm_receive(unsigned char *pBuff, unsigned long length,
									unsigned long timeout, void* param1)
{
	int ret = 0;

	/* received data from USB buffer */
	ret = usb_receive(pBuff, length, timeout);
	if (ret == USB_ERR_PARAM) {
		ret = FB_ERR_PARAM;
	} else if (ret == USB_ERR_NOT_OPEN) {
		ret = FB_ERR_OPEN;
	} else if (ret == USB_ERR_DISCONNECTED) {
		ret = FB_ERR_DISC;
	}

	return ret;
}

/*
 * fb_adapt_comm_send(): send data to USB buffer
 * Input
 * 		@pBuff	: buffer to send data
 * 		@length	: length of data
 * 		@timeout: timeout
 * 		@param1	: UNUSED.
 * Output		: None.
 * Return
  *		Number of bytes were sent
 *		FB_OK				Normal end
 *		FB_ERR_DISC			USB is disconnected
 *		FB_ERR_OPEN			USB is not opened
 *		FB_ERR_PARAM		Parameter error
 */
int fb_adapt_comm_send(unsigned char *pBuff, unsigned long length,
									unsigned long timeout, void* param1)
{
	int ret = 0;

	/* send data to USB buffer */
	ret = usb_send(pBuff, length, timeout);
	if (ret == USB_ERR_PARAM) {
		ret = FB_ERR_PARAM;
	} else if (ret == USB_ERR_NOT_OPEN) {
		ret = FB_ERR_OPEN;
	} else if (ret == USB_ERR_DISCONNECTED) {
		ret = FB_ERR_DISC;
	}

	return ret;
}

static const struct t_fb_comm_operation comm_device_operation = {
	.fb_dev_comm_init		= fb_adapt_comm_init,
	.fb_dev_comm_open		= fb_adapt_comm_open,
	.fb_dev_comm_close		= fb_adapt_comm_close,
	.fb_dev_comm_state		= fb_adapt_comm_state,
	.fb_dev_comm_receive	= fb_adapt_comm_receive,
	.fb_dev_comm_send		= fb_adapt_comm_send,
};

static struct t_fb_device comm_device = {
	.dev_id			= DEV_ID_USB,
	.used_count		= 0,
	.next_dev		= NULL,
	.file_operation	= (void*)&comm_device_operation,
	.init			= comm_init,
	.remove			= comm_remove,
};

/*
 * fb_adap_comm_register(): register device with device manager
 * Input		None
 * Output		None
 * Return		None
 */
void fb_adap_comm_register()
{
	fb_dev_register_dev(&comm_device);
}

/*
 * comm_init(): initialization device
 * Input		None
 * Output		None
 * Return		None
 */
// void comm_init(unsigned char *pbuff)
void comm_init(void)
{
	fb_adapt_comm_init((unsigned char *)COMMAND_BUFF_ADDR);
}

/*
 * comm_remove(): remove device
 * Input		None
 * Output		None
 * Return		None
 */
void comm_remove()
{
	fb_adapt_comm_close(UNUSED);
}

