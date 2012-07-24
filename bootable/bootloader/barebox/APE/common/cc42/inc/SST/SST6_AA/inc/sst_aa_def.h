/*! \file 
**********************************************************************************	
* Title:						Discretix SST AA definitions header file						 					
*																			
* Filename:						sst_aa_def.h 															
*																			
* Project, Target, subsystem:	SST 6.0, A&A
* 
* Created:						05.02.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
*           Copyright (C) 2007 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/


#ifndef _DX_SST_AA_DEF_H_
    #define _DX_SST_AA_DEF_H_
    
    #include "sst_general_def.h"
    #include "sst_debug.h"
    #include "sst_vcrys.h"

	/* Enable testing and debug mode.*/
	#if ((defined SST_DEBUG_MODE_ENABLED) && (!defined SST_AA_DEBUG_MODE_ENABLED))
		#define SST_AA_DEBUG_MODE_ENABLED
	#endif

	#if ((defined SST_TESTING_MODE_ENABLED) && (!defined SST_AA_TESTING_MODE_ENABLED))
		#define SST_AA_TESTING_MODE_ENABLED
	#endif
    
	/*-------------------------------------------------------------------------*/	
    /* The hash size as defined by VCRYS (20 byte - SHA-1, 32 bytes - SHA-256).*/ 
    #define SST_AA_PWD_HASH_SIZE_IN_BYTES	    SST_WORDS_TO_BYTES(SST_VCRYS_HASH_RESULT_SIZE_IN_WORDS)

    /* This session ID indicates that the session table entry is vacant */
    #define SST_AA_EMPTY_SESSION_ID             ((SSTSessionId_t)0xBABADADA)

    /* This session ID indicates that the challenge table entry is vacant */
    #define SST_AA_EMPTY_CHALLENGE_ID           ((SSTChallengeId_t)0xBABADADA)
    
    /* The offset in bytes of the reference counter in an authentication object */
    #define SST_AA_REF_COUNTER_OFFSET_IN_BYTES  (sizeof(SSTAAAuthenticationType_t))
    
    /* The size in bytes of the reference counter in an authentication object */
    #define SST_AA_REF_COUNTER_SIZE_IN_BYTES    (sizeof(DxUint32_t))
    
    /* The size in bytes of a password authenticator */
    #define SST_AA_PWD_AUTH_SIZE_IN_BYTES       (sizeof(SSTAAAuthenticatorPWD_t))
    
    /* The auth' handle indicating that the entry is vacant*/
    #define SST_AA_AUTH_ENTRY_EMPTY             ((SSTObjectId_t)0xAF000000)
    
    /* The session table semaphore name */
    #define SST_AA_SESSION_TABLE_SEM_NAME       "sst_aa_session_semaphore"

    /* The challenge table semaphore name*/
    #define SST_AA_CHALLENGE_TABLE_SEM_NAME     "sst_aa_challenge_semaphore"

    /*----------- SST AA macro definitions ---------------------------------------*/

    /* Check if the AA has finished its init sequence correctly.*/
    #define SST_AA_IS_ACTIVE_CONDITION_FAILED(param)                            (DX_FALSE==(param))
             
    /*check max number of authenticators per session */
    #define SST_AA_SESSION_AUTH_INDEX_CONDITION_FAILED(sessionAuthIndex)        (sessionAuthIndex == SST_AA_MAX_AUTHS_PER_SESSION)

    /* Check the session ID (see if its valid, return an error if not.)*/
    #define SST_AA_SESSION_INDEX_CONDITION_FAILED(sessionIndex) 		        (sessionIndex == SST_AA_MAX_OPEN_SESSIONS)

	/* Checks for valid challenge request and returns an ERROR if too long.*/
    #define SST_AA_CHALLENGE_SIZE_CONDITION_FAILED(challengeSize)               (challengeSize != SST_AA_CHALLENGE_SIZE_IN_BYTES)

    /* Check the challenge ID (see if its valid, return an error if not.)*/
    #define SST_AA_CHALLENGE_INDEX_CONDITION_FAILED(challengeIndex)             (challengeIndex == SST_AA_MAX_PENDING_CHALLENGES)

	/* Is the handle of an authentication object? (mask and compare only the lower byte) */
    #define SST_AA_IS_AUTH(handle)															\
               ((SST_HANDLE_IS_INTERNAL(handle))                                    &&		\
                ((SST_RECORD_TYPE_AUTH_CLOSE   == SST_HANDLE_TYPE_FIELD_GET(handle))||		\
                 (SST_RECORD_TYPE_AUTH_PWD     == SST_HANDLE_TYPE_FIELD_GET(handle))||		\
                 (SST_RECORD_TYPE_AUTH_APP_ID  == SST_HANDLE_TYPE_FIELD_GET(handle))||		\
                 (SST_RECORD_TYPE_AUTH_AES_SS  == SST_HANDLE_TYPE_FIELD_GET(handle))||		\
				 (SST_RECORD_TYPE_AUTH_PK_ECC_SV == SST_HANDLE_TYPE_FIELD_GET(handle))||	\
                 (SST_RECORD_TYPE_AUTH_PK_RSA_SV == SST_HANDLE_TYPE_FIELD_GET(handle))))
               
	/*check if the handle given is a password authenticator handle*/
	#define SST_AA_IS_PWD_AUTH(handle) (SST_RECORD_TYPE_AUTH_PWD == SST_HANDLE_TYPE_FIELD_GET(handle))

	/*Clear challenge from the challenge table given its index in the table*/
	#define SST_AA_CHALLENGE_IN_TABLE_CLEAR(index)	\
			((g_sstAaVars.pendingChallenges[index].challengeId) = (SST_AA_EMPTY_CHALLENGE_ID))

    #ifdef SST_AA_DEBUG_MODE_ENABLED
   
        /* Check if the amount read is indeed the one requested.*/
        #define SST_AA_DBG_CHECK_SIZE_AFTER_VDB_READ_CONDITION_FAILED(requestedSize,actualReadSize)	(requestedSize != actualReadSize)
                        
   #else    /*SST_AA_DEBUG_MODE_ENABLED*/
        
	    #define SST_AA_DBG_CHECK_SIZE_AFTER_VDB_READ_CONDITION_FAILED(requestedSize,actualReadSize)     DX_FALSE    
   #endif   /*SST_AA_DEBUG_MODE_ENABLED*/

	#ifdef SST_RUN_TIME_VAR_SIZE_PRINT
		#define SST_AA_RUN_TIME_VAR_SIZE_PRINT()	\
				SST_DEBUG_PRINT("AA global size in bytes:%d\n", sizeof(g_sstAaVars))
	#else
		#define SST_AA_RUN_TIME_VAR_SIZE_PRINT()
	#endif

#endif  /* _DX_SST_AA_DEF_H_ */
