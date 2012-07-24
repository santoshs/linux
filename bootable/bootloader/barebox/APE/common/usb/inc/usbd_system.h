/*	usbd_system.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USBD_SYSTEM_H__
#define __USBD_SYSTEM_H__

int MEM_INITIALIZE(void);
void *MEM_ALLOC(unsigned long uSize);
void MEM_FREE(void *pBuffer);
void MEM_SET(void *pDest, int uVal, unsigned long uSize);
void MEM_SET_DW(void *pDest, int uVal, unsigned long uSize);
void MEM_COPY(void *pDest, void *pSrc, unsigned long uSize);

#define USB_COMMON_MEM_BLOCKS	(6)
#define USB_COMMON_MEM_BLK_SIZE	(0x300-4)

#define mem_num		USB_COMMON_MEM_BLOCKS
#define mem_size	USB_COMMON_MEM_BLK_SIZE

typedef struct _USBD_MEM
{
	unsigned long	status;
	unsigned char	heapdata[mem_size];
}USBD_MEM ,*pUSBD_MEM;

#endif /* __USBD_SYSTEM_H__ */
