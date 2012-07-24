/*! \file 
**********************************************************************************	
* Title:						Discretix SST AA Configurations Header File						 					
*																			
* Filename:						sst_aa_config.h 															
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


#ifndef _DX_SST_AA_CONFIG_H_
    #define _DX_SST_AA_CONFIG_H_
	
	#include "sst_config.h"

    /*The maximal number of authenticators that can be open per session.*/
    #define SST_AA_MAX_AUTHS_PER_SESSION	    SST_MAX_AUTHS_PER_SESSION

    /*The maximal number of authenticators that can be referenced from an object. (for all access types)*/
    #define SST_AA_MAX_AUTHS_PER_OBJ	        (10UL)	

    /*The maximal number of sessions that can be open simultaneously.*/
    #define SST_AA_MAX_OPEN_SESSIONS	        SST_MAX_OPEN_SESSIONS

    /*The maximal number of challenges that can be pending simultaneously.*/
    #define SST_AA_MAX_PENDING_CHALLENGES	    SST_MAX_PENDING_CHALLENGES
    
    /* The maximal time the AA will wait for a semaphore. */
    #define SST_AA_SEMAPHORE_WAIT_TIME_IN_MS    SST_SEMAPHORE_WAITING_TIME_IN_MSEC
    
    /* The size of the exponent in the public key authentication scheme. */
    #define SST_AA_PK_EXP_SIZE_IN_BYTES         (4UL)
    
	// Debug configurations.
	#ifdef SST_AA_DEBUG_MODE_ENABLED	
	    #define SST_AA_ENCRYPT_AUTHS        DX_TRUE
	    
    #else //SST_AA_DEBUG_MODE_ENABLED
	    #define SST_AA_ENCRYPT_AUTHS        DX_TRUE
 
 	#endif //SST_AA_DEBUG_MODE_ENABLED
	
	
	
#endif  /* _DX_SST_AA_CONFIG_H_ */
