/*	usbd_types.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __USBD_TYPES_H__
#define __USBD_TYPES_H__

typedef unsigned char			UCHAR;
typedef unsigned short			USHORT;
typedef unsigned long			ULONG;

#ifndef NULL
#define NULL			0
#endif /* ifndef NULL */

typedef UCHAR*			PUCHAR;

#ifndef TRUE
#define TRUE			1
#endif /* ifndef TRUE */

#ifndef	min
#define min(a,b) (((a)<=(b))?(a):(b))
#endif

#define CUDD_RC_NOR					0
#define CUDD_RC_NG_PHASE			1

#endif /* __USBD_TYPES_H__ */
