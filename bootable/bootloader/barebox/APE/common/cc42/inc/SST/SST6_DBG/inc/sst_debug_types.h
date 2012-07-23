/*! \file 
**********************************************************************************	
* Title:						Discretix SST Debug Types Header File						 		
*																			
* Filename:						sst_debug_types.h 					
*																			
* Project, Target, subsystem:	SST 6.0, Debug
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/


#ifndef _DX_SST_DEBUG_TYPES_H_
    #define _DX_SST_DEBUG_TYPES_H_

	#include "DX_VOS_File.h"
	#include "DX_VOS_Sem.h"
	#include "sst_debug_def.h"
	/*----------- EXTERN variables -------------------------------------*/
	#if defined EXTERN
		#error EXTERN ALREADY DEFINED !!! (sst_debug_types.h) 
	#else
		#if defined _DX_SST_DEBUG_NO_EXTERN_
			#define EXTERN 
		#else
			#define EXTERN extern
		#endif /* _DX_SST_DEBUG_NO_EXTERN_*/
	#endif /*EXTERN*/

	/*----------- Local type definitions -----------------------------------------*/

	/*----------- Package global variables --------------------------------------*/
	#if defined SST_DEBUG_MODE_ENABLED
		EXTERN	DxVosSem	g_sstDebugSem;
		EXTERN	DxVosFile	g_sstDebugLogFile_ptr;
		EXTERN 	SSTDBGEventLogger_t	g_sstDebugErrorEventArr[SST_DBG_NUM_ERROR_EVENT];
		EXTERN 	SSTDBGEventLogger_t	g_sstDebugNonRelevantEventArr[SST_DBG_NUM_NON_RELEVANT_EVENT];
	#endif


	#undef EXTERN


#endif  /* _DX_SST_DEBUG_TYPES_H_ */
