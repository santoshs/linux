/*	types.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 * File Name: types.h
 * Contents : Types Define
 ****************************************************************************/

#define PFAR
#define FFAR

typedef long W;             // signed 32-bit integer
typedef unsigned char UB;   // unsigned 8-bit integer
typedef unsigned short UH;  // unsigned 16-bit integer

typedef int INT;
typedef unsigned int UINT;

typedef INT BOOL;

typedef INT ER;
typedef INT PRI;
typedef UINT FLGPTN;
typedef UINT INHNO;

typedef unsigned long SIZE;
typedef W TMO;
typedef W DLYTIME;
typedef DLYTIME RELTIM;
typedef void PFAR *VP;
typedef VP VP_INT;

#ifdef __cplusplus
}
#endif

#endif
