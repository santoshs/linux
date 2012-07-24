/*! \file
**********************************************************************************
* Title:						Discretix SST API definitions header file
*
* Filename:						sst_api_def.h
*
* Project, Target, subsystem:	SST 6.0, API
*
* Created:						11.03.2007
*
* Modified:						07.06.2007
*
* \Author						Raviv Levi
*
* \Remarks
*           Copyright (C) 2007 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/
#ifndef _DX_SST_API_DEF_H_
    #define _DX_SST_API_DEF_H_


    #include "sst_general_def.h"
    #include "sst_api_config.h"

    /*----------- SST API macro definitions ---------------------------------------*/
	
	/*size to add to workspace size in order to allow internal alignment of the workspace
	4 bytes to align the pointer and 4 bytes to align the workspace size*/
	#define SST_WORKSPACE_ALIGNMENT_SIZE_IN_BYTES	(8UL)

    /* Validation macros*/
	#define SST_IS_EXTERNAL(handle)			(!SST_HANDLE_IS_INTERNAL(handle))
	#define SST_IS_ACTIVE					(DX_TRUE == g_sstApiVars.isActive)
	
	#define SST_SIZE_IN_BYTES_ALIGN(arg)	(((DxUint32_t)(arg))&0x3)
	#define SST_IS_BUFFER_WORD_ALIGN(arg)	(SST_SIZE_IN_BYTES_ALIGN(arg) == 0)
	

	#ifdef SST_RUN_TIME_VAR_SIZE_PRINT
		#define SST_API_RUN_TIME_VAR_SIZE_PRINT()	\
				SST_DEBUG_PRINT("API global size in bytes:%d\n", sizeof(g_sstApiVars))
	#else
		#define SST_API_RUN_TIME_VAR_SIZE_PRINT()
	#endif

#endif  /* _DX_SST_API_DEF_H_ */
