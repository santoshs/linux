/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS Definitions Header File			 					
*																			
* Filename:						sst_vcrys_def.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Cryptography Services
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


#ifndef _DX_SST_VCRYS_DEF_H_
    #define _DX_SST_VCRYS_DEF_H_

	#include "sst_general_config.h"
	
	/*Enable debug and testing mode*/
	#if ((defined SST_DEBUG_MODE_ENABLED) && (!defined SST_VCRYS_DEBUG_MODE_ENABLED))
        #define SST_VCRYS_DEBUG_MODE_ENABLED
    #endif

	#if ((defined SST_TESTING_MODE_ENABLED) && (!defined SST_VCRYS_TESTING_MODE_ENABLED))
		#define SST_VCRYS_TESTING_MODE_ENABLED
	#endif
	
	/*definition for secret key size*/
	#define SST_VCRYS_SK_MAX_SIZE_IN_BYTES					(32UL)


	/*Trust zone definitions*/
	#define SST_VCRYS_TRUST_ZONE
	
	#ifdef SST_VCRYS_TRUST_ZONE
		#define SST_VCRYS_INTERNAL_SK_SIZE_IN_BYTES			SST_VCRYS_INTERNAL_KEY_SIZE_IN_BYTES
		#define SST_VCRYS_UNWRAP_KEY_GLOBAL_STORE(a)		SST_VCRYS_InternalUnwrapKeyStore(a)
		#define SST_VCRYS_WRAP_KEY_GLOBAL_STORE(a)			SST_VCRYS_RC_OK
		#define SST_VCRYS_UNWRAP_FUNCTION(a,b)				SST_VCRYS_SingleKeyCopy(a,b)
	#else
		#define SST_VCRYS_INTERNAL_SK_SIZE_IN_BYTES			SST_VCRYS_INTERNAL_WRAPPED_KEY_SIZE_IN_BYTES		
		#define SST_VCRYS_UNWRAP_KEY_GLOBAL_STORE(a)		SST_VCRYS_RC_OK
		#define SST_VCRYS_WRAP_KEY_GLOBAL_STORE(a)			SST_VCRYS_InternalWrapKeyStore(a)		
		#define SST_VCRYS_UNWRAP_FUNCTION(a,b)				SST_VCRYS_SingleKeyUnwrap(a,b)		
	#endif

	#define SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT		(4UL)
	#define SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_SIZE_IN_BYTES	(1 << SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT) /*(16UL)*/
	#define SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK			(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_SIZE_IN_BYTES-1) /*(0xf)*/

	#define SST_VCRYS_MAC_BLOCK_MASK						SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK
	#define SST_VCRYS_MAC_BLOCK_NUM_SHIFT					SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT


	#define SST_VCRYS_NUMBER_OF_INTERNAL_KEYS				(2UL)
    #define SST_VCRYS_INTERNAL_KEY_SIZE_IN_BYTES			(32UL)
	#define SST_VCRYS_INTERNAL_WRAPPED_KEY_SIZE_IN_BYTES	(SST_VCRYS_INTERNAL_KEY_SIZE_IN_BYTES+8)
	#define SST_VCRYS_INTERNAL_KEY_VDB_RECORD_SIZE			(SST_VCRYS_NUMBER_OF_INTERNAL_KEYS*\
															SST_VCRYS_INTERNAL_WRAPPED_KEY_SIZE_IN_BYTES)


	#ifdef SST_RUN_TIME_VAR_SIZE_PRINT
		#define SST_VCRYS_RUN_TIME_VAR_SIZE_PRINT()	\
				SST_DEBUG_PRINT("VCRYS permanent global size in bytes:%d\n", sizeof(g_sstVcrysPermanentVar))
				SST_DEBUG_PRINT("VCRYS Non permanent global size in bytes:%d\n", sizeof(g_sstVcrysNonPermanentVar))	
	#else
		#define SST_VCRYS_RUN_TIME_VAR_SIZE_PRINT()
	#endif

#endif  /* _DX_SST_VCRYS_DEF_H_ */
