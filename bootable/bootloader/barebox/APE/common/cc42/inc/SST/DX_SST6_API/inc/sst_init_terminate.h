/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST init/terminate API header file						 					
*																			
* Filename:					    sst_init_terminate.h															
*																			
* Project, Target, subsystem:	SST 6.0, Stubs, API
* 
* Created:						06.06.2007														
*
* Modified:						07.06.2007										
*
* \Author						Ira Boguslavsky														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_INIT_TERMINATE_H_
    #define _DX_SST_INIT_TERMINATE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "NVS.h"
   
	 /*----------- Global defines -------------------------------------------------*/  

	 /*----------- Global function prototypes -------------------------------------*/
     /*General services*/
      
        /*SST_Init*/
        /*!
        \brief Initializes the SST system for use.
        

		@param aInitInfo_ptr [out]
		Information regarding the SST after initialization have succeeded, including 
		indication if the system recovered from an unexpected power down,
		Indication if a new database was created
		may accept DX_NULL, for no indication.

		@param aWorkspace_ptr           [in]
		A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
		check if there is sufficient size after aligning. if not an error will be returned

		@param aWorkspaceSizeInBytes    [in]
		The size in bytes of the workspace buffer to be used by the SST.

        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_FATAL              The fatal error occurred.
        @return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
        @return SST_RC_ERROR_MODULE_INIT_FAIL	At least one of the modules comprising the SST failed to init correctly.
		@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_Init(SSTInitInfo_t	*aInitInfo_ptr,
						   DxByte_t			*aWorkspace_ptr,
						   DxUint32_t		aWorkspaceSizeInBytes);
        
        /*SST_Terminate*/
        /*!
        \brief 
        Terminates the SST system. 
		Notes:
		1.	If an open transaction still exists the SST will remain open for further operation.
		2.	In all other cases, such as VOS error, the SST is terminated and no further use is 
			possible (until an Init takes place).
		3.	Termination of the SST is possible even if an integrity error was detected 
			to allow further use with different DB. If a transaction has not been committed
			But the integrity was compromised, the SST will be terminated as well. 
        @return	SST_RC_OK	                        The operation completed successfully.
        @return SST_RC_FAIL	                        The operation failed.
        @return SST_RC_ERROR_CLOSED                 The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_MODULE_TERMINATE_FAIL	At least one of the modules comprising the SST failed to init correctly.				    
		@return	SST_RC_ERROR_TRANSACTION_NOT_ENDED	An open transaction still exists
		@return	SST_RC_ERROR_VOS					An error with one of the VOS related services has occurred  
		@return SST_RC_ERROR_FATAL                  The fatal error occurred.    
        **/
        DxError_t SST_Terminate(void);

#ifdef __cplusplus
}
#endif       
#endif  /* _DX_SST_INIT_TERMINATE_H_ */
