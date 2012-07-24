/*! \file 
**********************************************************************************	
* Title:						Discretix SST Debug Configurations Header File						 		
*																			
* Filename:						sst_debug_config.h 					
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


#ifndef _DX_SST_DEBUG_CONFIG_H_
    #define _DX_SST_DEBUG_CONFIG_H_
	
	#define SST_DEBUG_DEFAULT_FILE_NAME				"sstDebugLog.bin"
	#define SST_DEBUG_DEFAULT_SEMAPHORE_NAME		"sstDebugSem"
	#define SST_DEBUG_SEMAPHORE_WAITING_TIME		3000L				/*mSec*/

	/*definitions regarding which data to output, as default no code is printed*/
	#define SST_DBG_PRINT_ALL_CODE_FLAG				1
	#define SST_DBG_PRINT_LOG_CODE_FLAG				0

	
#endif  /* _DX_SST_DEBUG_CONFIG_H_ */
