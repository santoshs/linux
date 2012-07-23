/*	usb_workmem.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 * All rights reserved.
 *
 */

#include "types.h"
#include "usb_download.h"
#include "usbd_system.h"
#include "comm_acm_dev_vcom.h"

__attribute__ ((aligned(16))) unsigned char	gbBulkInBuff[BULK_IN_BUFF_SIZE];
__attribute__ ((aligned(16))) SFifo			BulkOutFifo;
__attribute__ ((aligned(16))) SFifo			BulkInFifo;
__attribute__ ((aligned(16))) USBD_MEM 		usbd_mem[mem_num];
__attribute__ ((aligned(16))) unsigned long gbBulkOutBuffSize;
__attribute__ ((aligned(32))) unsigned char	*gbBulkOutBuff;
__attribute__ ((aligned(32))) unsigned char	*gbBulkOutBuff_2nd;
