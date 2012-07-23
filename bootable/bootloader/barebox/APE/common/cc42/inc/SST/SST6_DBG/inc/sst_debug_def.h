/*! \file 
**********************************************************************************	
* Title:						Discretix SST Debug Definitions Header File						 		
*																			
* Filename:						sst_debug_def.h 					
*																			
* Project, Target, subsystem:	SST 6.0 , Debug
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


#ifndef _DX_SST_DEBUG_DEF_H_
    #define _DX_SST_DEBUG_DEF_H_

	/*----------------------------------------------------------------------------*/
	#define SST_DBG_ACTIVE_FLAG						1	
	#define SST_DBG_NUM_ERROR_EVENT					1UL
	#define SST_DBG_NUM_NON_RELEVANT_EVENT			13UL
	#define SST_DBG_NUM_PRINT_EVENT					42UL
    #define SST_DBG_PRINT_EXTRA_DATA_FLAG           0UL

    /*definition for type of printing (screen, log file or no print out)*/
    /*#define SST_DEBUG_PRINT_SCREEN*/
    /*#define SST_DEBUG_ENUM_LOG_FILE*/

	/*----------- Global macro definitions ---------------------------------------*/
	
	/*definition for type of printing (screen, log file or no print out)*/
	#ifdef SST_DEBUG_PRINT_SCREEN
		#define SST_DBG_EVENT_SEND_EXTRA_DATA_FUNC(data)		SST_DBG_RC_OK; \
																DX_VOS_Printf("%d ",data);
		#define SST_DEBUG_PRINT(arg,...)    					DX_VOS_Printf("\n"); \
																DX_VOS_Printf(arg)
		#define SST_DBG_VRPC_LOG_PARSER()						SST_DBG_RC_OK
		#define SST_DBG_LOG_INIT()								SST_DBG_RC_OK
		#define SST_DBG_LOG_TERMINATE()							SST_DBG_RC_OK

	#elif defined SST_DEBUG_ENUM_LOG_FILE
		/*! \brief Print to Debug Events to Log File**/
		#define SST_DEBUG_PRINT(arg,...)					
		#define SST_DBG_VRPC_LOG_PARSER()						SST_VRPC_DBG_DebugLogCheck()
		#define SST_DBG_LOG_INIT()								SST_DBG_DebugLogInit()
		#define SST_DBG_LOG_TERMINATE()							SST_DBG_DebugLogTerminate()
	#else
		#define SST_DBG_EVENT_SEND_EXTRA_DATA_FUNC(data)		SST_DBG_RC_OK
		#define SST_DEBUG_PRINT(arg,...)		
		#define SST_DBG_VRPC_LOG_PARSER()						SST_DBG_RC_OK
		#define SST_DBG_LOG_INIT()								SST_DBG_RC_OK
		#define SST_DBG_LOG_TERMINATE()							SST_DBG_RC_OK
	#endif	
	
	
	/*defining the extra data output*/
	#if	(SST_DBG_ACTIVE_FLAG == SST_DBG_PRINT_EXTRA_DATA_FLAG)
		#define SST_DBG_PRINT_EXTRA_DATA(extraData)		SST_DBG_EVENT_SEND_EXTRA_DATA_FUNC(extraData)
	#else
		#define SST_DBG_PRINT_EXTRA_DATA(dbgEvent)		SST_DBG_RC_OK
	#endif

	/*Define the start and end events VCRYS*/
	#define SST_DBG_EVENT_VCRYS_ERROR_START_CODE			SST_DBG_EVENT_VCRYS_ERROR_VOS
	#define SST_DBG_EVENT_VCRYS_ERROR_END_CODE				SST_DBG_EVENT_VCRYS_ERROR_VDB

	#define SST_DBG_EVENT_VCRYS_API_START_CODE				SST_DBG_EVENT_VCRYS_INIT_OK	
	#define SST_DBG_EVENT_VCRYS_API_END_CODE				SST_DBG_EVENT_VCRYS_DECRYPT_FAILED

	#define SST_DBG_EVENT_VCRYS_EXTERNAL_API_START_CODE		SST_DBG_EVENT_VCRYS_AES_KEY_GENERATE_OK
	#define SST_DBG_EVENT_VCRYS_EXTERNAL_API_END_CODE		SST_DBG_EVENT_VCRYS_RSA_CRT_KEY_GET_FAILED
	
	#define SST_DBG_EVENT_VCRYS_INTERNAL_FUNC_START_CODE	SST_DBG_EVENT_VCRYS_MAC_CALC_INIT_OK	
	#define SST_DBG_EVENT_VCRYS_INTERNAL_FUNC_END_CODE		SST_DBG_EVENT_VCRYS_MAC_CALC_BLOCK_RESIDUE_FAILED

	#define SST_DBG_EVENT_VCRYS_API_TEST_START_CODE			SST_DBG_EVENT_VCRYS_ENCRYPT_DECRYPT_TEST_OK
	#define SST_DBG_EVENT_VCRYS_API_TEST_END_CODE			SST_DBG_EVENT_VCRYS_BASIC_TEST_FAILED

	#define SST_DBG_EVENT_VCRYS_EXTERNAL_API_TEST_START_CODE	SST_DBG_EVENT_VCRYS_AES_KEY_GENERATE_TEST_OK
	#define SST_DBG_EVENT_VCRYS_EXTERNAL_API_TEST_END_CODE		SST_DBG_EVENT_VCRYS_EXTERNAL_BASIC_TEST_FAILED


	/*Define the start and end events VDB*/
	#define SST_DBG_EVENT_VDB_ERROR_START_CODE	SST_DBG_EVENT_VDB_ERROR_NULL_POINTER
	#define SST_DBG_EVENT_VDB_ERROR_END_CODE	SST_DBG_EVENT_VDB_ERROR_SCRATCH_BUFF_NOT_SUFFICIENT

	#define SST_DBG_EVENT_VDB_API_START_CODE	SST_DBG_EVENT_VDB_INIT1_OK
	#define SST_DBG_EVENT_VDB_API_END_CODE		SST_DBG_EVENT_VDB_SEQ_ITERATOR_GET_ALL_FAIL
	
	#define SST_DBG_EVENT_VDB_INTERNAL_FUNC_START_CODE	SST_DBG_EVENT_VDB_RECORD_HANDLE_GENRATE_OK
	#define SST_DBG_EVENT_VDB_INTERNAL_FUNC_END_CODE		SST_DBG_EVENT_VDB_RECORD_INTERNAL_ENCRYPT_AND_MODIFY_FAIL

	#define SST_DBG_EVENT_VDB_API_TEST_START_CODE	SST_DBG_EVENT_VDB_TEST_RECORD_GENERATE_OK
	#define SST_DBG_EVENT_VDB_API_TEST_END_CODE		SST_DBG_EVENT_VDB_TEST_DATA_GET_SIZE_FAIL

	/*Define the start and end events VRPC*/
	#define SST_DBG_EVENT_VRPC_SEMAPHORE_START_CODE	SST_DBG_EVENT_VRPC_SEMAPHORE_FAILED
	#define SST_DBG_EVENT_VRPC_SEMAPHORE_END_CODE	SST_DBG_EVENT_VRPC_TIMED_OUT

	#define SST_DBG_EVENT_VRPC_API_START_CODE		SST_DBG_EVENT_VRPC_INIT_OK	
	#define SST_DBG_EVENT_VRPC_API_END_CODE			SST_DBG_EVENT_VRPC_SET_FAILED

	#define SST_DBG_EVENT_VRPC_FILE_START_CODE		SST_DBG_EVENT_VRPC_READ_OK
	#define SST_DBG_EVENT_VRPC_FILE_END_CODE		SST_DBG_EVENT_VRPC_CREATE_FILE_FAILED

	#define SST_DBG_EVENT_VRPC_TEST_START_CODE		SST_DBG_EVENT_VRPC_BASIC_TEST_OK
	#define SST_DBG_EVENT_VRPC_TEST_END_CODE		SST_DBG_EVENT_VRPC_ITERATION_TEST_FAILED

	#define SST_DBG_EVENT_VRPC_TASK_START_CODE		SST_DBG_EVENT_VRPC_TASK_CREATE_OK
	#define SST_DBG_EVENT_VRPC_TASK_END_CODE		SST_DBG_EVENT_VRPC_TASK_TIMED_OUT

    /* This define is used for collision simulation */
    #define SST_DBG_COLLISION_DIGEST                (0x11223344)

    #define SST_DBG_DISACTIVATE                     (0UL)
    #define SST_DBG_ACTIVATE                        (1UL)

    /* A flag which defines that IX test will be performed with collision => */
    /* A hash function will return a constant value and not a digest         */ 
    #define SST_DBG_TEST_COLLISION                   SST_DBG_DISACTIVATE

#endif  /* _DX_SST_DEBUG_DEF_H_ */
