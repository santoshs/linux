/* C_DefUsr.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __C_DEFUSR_H__
#define __C_DEFUSR_H__

/********** macro define (COMMON) ********************************************/

// USB register base address & pointer type
#define CFIFO_MBW_INIT          MBW_32

typedef volatile U16			REGP;
typedef volatile U8				REGP8;
typedef volatile U32			REGP32;

/********** macro define (HOST) **********************************************/
#define	PIPEERR					3					// Max pipe error count

/********** macro define (PERIPHERAL) ****************************************/
#define USB_CFG_DESC_FULL_MAX	512
#define USB_CFG_DESC_HI_MAX		512
#define USB_STR_DESC_IDX_MAX	5

#define USB_CNT_10us			300

/******* include file **********************************************/
#include "c_def596.h"
#include "usbf_api.h"

#endif //__C_DEFUSR_H__

