/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Additions Header file 				 					
*																			
* Filename:						itzam_dx_convert.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						15.02.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _ITZAM_DX_CONVERT_H_
	#define _ITZAM_DX_CONVERT_H_

	#include "DX_VOS_BaseTypes.h"

	/* 
	 * This flag is intended to be used in other files to distinguish
	 * DX platforms.
	 */
	#define SST_DX_ITZAM_CONVERT

	#ifndef _SIZE_T_DEFINED
	typedef		DxUint_t   size_t; /*TBD 64bit */
	#define _SIZE_T_DEFINED
	#endif

	/* Define _CRTIMP */


	#ifndef _CRTIMP
	#ifdef  _DLL
	#define _CRTIMP __declspec(dllimport)
	#else   /* ndef _DLL */
	#define _CRTIMP
	#endif  /* _DLL */
	#endif  /* _CRTIMP */

	#ifndef __STDC__
	/* Non-ANSI name for compatibility */
	typedef long off_t;
	#endif


	/*sprintf_s checks the format string for valid formatting characters, 
	whereas sprintf only checks if the format string or buffer are NULL pointers*/
	/*
	_O_CREAT	Creates and opens new file for writing. 
				Has no effect if the file specified by filename exists. 
				The pmode argument is required when _O_CREAT is specified
	_O_BINARY	Opens a file in binary (untranslated) mode. 
				(See fopen for a description of binary mode.)
	_O_RDWR		Opens file for both reading and writing; 
				cannot be specified with _O_RDONLY or _O_WRONLY.
	_O_RANDOM	Specifies primarily random access from disk

	_SH_DENYNO	Permits read and write access.

	_S_IREAD | _S_IWRITE  Reading and writing permitted.
	*/


	/* Defining Itzam internal types according to DX base types */
	
	/* type definitions */
	typedef DxUint64_t    uint64_t;
	typedef DxInt64_t     int64_t;
	typedef DxUint32_t    uint32_t;
	typedef DxInt32_t     int32_t;
	typedef DxUint16_t    uint16_t;
	typedef DxInt16_t	  int16_t;
	typedef DxUint8_t     uint8_t;
	typedef DxInt8_t      int8_t;
	typedef DxChar_t	  BOOL;

	/* constant declarations */
#define true  (DxChar_t)1
#define false (DxChar_t)0

#endif /*_ITZAM_DX_CONVERT_H_*/
