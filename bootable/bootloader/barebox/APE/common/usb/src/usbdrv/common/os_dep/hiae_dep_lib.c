/*	hiae_dep_lib.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "types.h"
#include "usbd_system.h"

extern USBD_MEM usbd_mem[mem_num];

/**
 * MEM_INITIALIZE
 * @return 0
 */
int MEM_INITIALIZE(void)
{
	unsigned char	i;

	for (i=0; i<mem_num; i++) {
		usbd_mem[i].status = 0;
	}
	return 0 ;
}

/**
 * MEM_ALLOC
 * @return 0
 */
void *MEM_ALLOC(unsigned long uSize)
{
	void		*adr;
	unsigned char	HeapNo = 0;

	if (uSize > mem_size) {
		return 0 ;
	}

	for (HeapNo=0; HeapNo<mem_num; HeapNo++) {
		if (usbd_mem[HeapNo].status == 0) {
			adr = usbd_mem[HeapNo].heapdata;
			usbd_mem[HeapNo].status = 1;
			return (adr);
		}
	}

	return 0 ;
}

/**
 * MEM_FREE
 * @return None
 */
void MEM_FREE(void *pBuffer)
{
	unsigned char	HeapNo = 0;

	for (HeapNo=0; HeapNo<mem_num; HeapNo++) {
		if (usbd_mem[HeapNo].heapdata == pBuffer) {
			usbd_mem[HeapNo].status = 0;
		}
	}
}

/**
 * MEM_SET
 * @return None
 */
void MEM_SET(void *pDest, int uVal, unsigned long uSize)
{
	unsigned char *ptr=pDest;
	unsigned int len = uSize;

	while (len--) {
		*ptr++ = uVal;
	}
}

/**
 * MEM_SET_DW
 * @return None
 */
void MEM_SET_DW(void *pDest, int uVal, unsigned long uSize)
{
	unsigned long *pdwtr=pDest;
	unsigned char *pbtr;
	unsigned long dwVal;
	unsigned int len_dw;
	unsigned int len;

	len = uSize % 4 ;
	len_dw = uSize / 4 ;

	if (len_dw != 0) {
		dwVal = (uVal << 24) | (uVal << 16) | (uVal << 8) | uVal ;

		while (len_dw--) {
			*pdwtr++ = dwVal;
		}

	}

	if (len != 0) {
		pbtr = (unsigned char *)pdwtr ;
		while (len--) {
			*pbtr++ = uVal;
		}
	}
}

/**
 * MEM_COPY
 * @return None
 */
void MEM_COPY(void *pDest, void *pSrc, unsigned long uSize)
{
	unsigned char *dest=pDest;
	unsigned char *src=pSrc;
	unsigned int len = uSize;

    while (len--) {
        *dest++ = *src++;
    }
}
