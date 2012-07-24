/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 
/*! \file DX_VOS_config.h
    \brief This file defines the options for using 64bits variables
*/


#ifndef _DX_VOS_CONFIG_H
#define _DX_VOS_CONFIG_H

#ifndef __FUNCTION__
#define __FUNCTION__ ""
#endif

#define DX_VOS_THREAD_RESOLUTION    256
#define LINUX 
#define LINUX_SYSCALL_SUCCESS 0
#define LINUX_SYSCALL_ERROR -1
#define DX_VOS_PATH_DIVIDER		"/" 

#define DX_VOS_MAX_PATH 260

#if !defined(LITTLE__ENDIAN) && !defined(BIG__ENDIAN)
#define LITTLE__ENDIAN
#endif
//#ifdef  __cplusplus
//#define DX_ADDRESSOF(v)   ( &reinterpret_cast<const char &>(v) )
//#else
#define DX_ADDRESSOF(v)   ( &(v) )
//#endif
#define DX_INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

typedef char * DX_VA_LIST;

#define DX_VA_START(ap,v)  ( ap = (DX_VA_LIST)DX_ADDRESSOF(v) + DX_INTSIZEOF(v) )
#define DX_VA_ARG(ap,t)    ( *(t *)((ap += DX_INTSIZEOF(t)) - DX_INTSIZEOF(t)) )
#define DX_VA_END(ap)      ( ap = (DX_VA_LIST)0 )

// TODO: temoporary for compiation, sort out 64 bit issues on linux build
#define HASLONGLONG


#ifndef IMPORT_C
#define IMPORT_C
#endif
#ifndef EXPORT_C
#define EXPORT_C
#endif

#ifdef DX_USE_LEGACY_VOS

#define DX_VOS_DEFAULT_ROOT		"~/Test" 
#define DX_VOS_DRM_ROOT_PATH	"~/private/DX"
#define DX_VOS_DRM_PUBLIC_ROOT_PATH	"~/shared/DX"

#endif
#endif /* ifndef _DX_VOS_CONFIG_H*/
