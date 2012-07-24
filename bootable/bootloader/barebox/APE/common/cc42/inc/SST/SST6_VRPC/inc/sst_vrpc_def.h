/*! \file 
**********************************************************************************	
* Title:						Discretix SST VRPC Definitions Header File						 					
*																			
* Filename:						sst_vrpc_def.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Reflash Protection Counter
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


#ifndef _DX_SST_VRPC_DEF_H_
    #define _DX_SST_VRPC_DEF_H_

    #include "sst_vrpc_config.h"

	/*Enable debug and testing mode*/
    #if ((defined SST_DEBUG_MODE_ENABLED) && (!defined SST_VRPC_DEBUG_MODE_ENABLED))
        #define SST_VRPC_DEBUG_MODE_ENABLED
    #endif

	#if ((defined SST_TESTING_MODE_ENABLED) && (!defined SST_VRPC_TESTING_MODE_ENABLED))
		#define SST_VRPC_TESTING_MODE_ENABLED
	#endif

	#define SST_VRPC_SEMAPHORE_NAME						"sstVrpcSemaphore"

	#ifdef SST_RUN_TIME_VAR_SIZE_PRINT
		#define SST_VRPC_RUN_TIME_VAR_SIZE_PRINT()					\
				SST_DEBUG_PRINT("VRPC global size in bytes:%d\n", sizeof(g_sstVrpcVar));
	#else
		#define SST_VRPC_RUN_TIME_VAR_SIZE_PRINT()
	#endif

#endif  /* _DX_SST_VRPC_DEF_H_ */
