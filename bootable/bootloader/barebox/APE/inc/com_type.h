/* com_type.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __COMM_TYPE_H__
#define __COMM_TYPE_H__

/************************************************************************************************/
/*	native type																					*/
/************************************************************************************************/
#ifndef NULL
#define NULL				(void*)0
#endif

#ifndef TRUE
#define TRUE				1
#endif

#ifndef FALSE
#define FALSE				0
#endif

typedef int					RC;
typedef unsigned long long	uint64;
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned long		ulong;
typedef unsigned int		uint;

typedef int					size_t;


#endif /* __COMM_TYPE_H__ */
