/*! \file 
**********************************************************************************	
* Title:						Discretix SST VDB Definitions Header File
*																			
* Filename:						sst_vdb_def.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual DataBase
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


#ifndef _DX_SST_VDB_DEF_H_
    #define _DX_SST_VDB_DEF_H_

	#include "sst_general_config.h"

	/*Enable debug and testing mode*/
	#if ((defined SST_DEBUG_MODE_ENABLED) && (!defined SST_VDB_DEBUG_MODE_ENABLED))
        #define SST_VDB_DEBUG_MODE_ENABLED
    #endif

	#if ((defined SST_TESTING_MODE_ENABLED) && (!defined SST_VDB_TESTING_MODE_ENABLED))
		#define SST_VDB_TESTING_MODE_ENABLED
	#endif
	
	/*definition for scratch buffer size*/
	#define SST_VDB_INTERNAL_MIN_SCRATCH_BUFFER_SIZE_IN_BYTES	(32UL)

    #define SST_VDB_SK_BUFFER_MAX_SIZE_IN_BYTES       (SST_VCRYS_NUMBER_OF_INTERNAL_KEYS * SST_VCRYS_INTERNAL_WRAPPED_KEY_SIZE_IN_BYTES)

	/*VDB global defines*/
	#define SST_VDB_ENCRYPT_BLOCK_ROUND_UP(size)	\
			(((size) & (SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK)) ? \
			((((size)>>(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT))+1)\
			<<(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT)) : (size))

	#define SST_VDB_ENCRYPT_BLOCK_ROUND_DOWN(size)\
			(((size) & (SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK)) ? \
			(((size)>>(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT))\
			<<(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT)) : (size))

	#define SST_VDB_ENCRYPT_BLOCK_REMINDER(size)	((size) & (SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK))

	#define SST_VDB_MIN_SIZE( a , b )				( ( (a) < (b) ) ? (a) : (b) )

    #define SST_VDB_ERROR_DBG_EVENT_SEND(functionName,eventName,returnCode)	\
            SST_DBG_EVENT_SEND(SST_DBG_EVENT_VDB_ERROR_##eventName, SST_VDB_RC_##returnCode); \
            SST_DBG_EVENT_SEND(SST_DBG_EVENT_VDB_##functionName##_FAIL, SST_VDB_RC_##returnCode)

	#define SST_VDB_ERROR_HANDLE_ASSERT(functionName,eventName,returnCode)	\
			SST_DBG_EVENT_SEND(SST_DBG_EVENT_VDB_ERROR_##eventName, SST_VDB_RC_##returnCode); \
			SST_DBG_EVENT_SEND(SST_DBG_EVENT_VDB_##functionName##_FAIL, SST_VDB_RC_##returnCode); \
			SST_ASSERT(SST_VDB_RC_##returnCode)

    #define SST_VDB_DB_ERROR_EVENT_SEND(functionName,dbRC)	\
            SST_DBG_EVENT_SEND(SST_DBG_EVENT_VDB_##functionName##_FAIL, dbRC); 


	#define SST_VDB_ITZAM_HANDLE_CONVERT(itzamHandle,sstHandle)	\
			sstHandle.objDigest = itzamHandle.msbField;	\
			sstHandle.objId = itzamHandle.lsbField

	#define SST_VDB_ITER_HANDLE_TO_ITZAM_ITER_PARAM(iterCookie,itzamIterParam) \
			itzamIterParam.fieldMsb = iterCookie.prefixMsb; \
			itzamIterParam.fieldLsb = iterCookie.prefixLsb; \
			itzamIterParam.size = iterCookie.prefixSize

	#define SST_VDB_ITER_PREFIX_TO_ITZAM_ITER_PARAM(iterPrefix,itzamIterParam) \
			itzamIterParam.fieldMsb = iterPrefix.prefixFieldMsb; \
			itzamIterParam.fieldLsb = iterPrefix.prefixFieldLsb; \
			itzamIterParam.size = iterPrefix.prefixSize

	#define SST_VDB_ITERATOR_BIT_MASK	0x80000000


	#ifdef SST_RUN_TIME_VAR_SIZE_PRINT
		#define SST_VDB_RUN_TIME_VAR_SIZE_PRINT()	\
				SST_DEBUG_PRINT("VDB global size in bytes:%d\n", sizeof(g_sstVdbPermanentVar))
	#else
		#define SST_VDB_RUN_TIME_VAR_SIZE_PRINT()
	#endif

#endif  /* _DX_SST_VDB_DEF_H_ */
