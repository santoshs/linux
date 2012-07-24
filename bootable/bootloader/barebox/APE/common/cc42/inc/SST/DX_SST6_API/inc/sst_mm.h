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
* Title:						Discretix SST mode management API header file						 					
*																			
* Filename:					    sst_mm.h															
*																			
* Project, Target, subsystem:	SST 6.0, MM, API
* 
* Created:						14.08.2008														
*
* Modified:						14.08.2008										
*
* \Author						Miri S														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_MM_H_
#define _DX_SST_MM_H_
#ifdef __cplusplus
extern "C" {
#endif

    /*----------- Global Includes ------------------------------------------------*/
    #include "sst_types.h" 
    #include "NVS.h"	
    /*----------- Global defines -------------------------------------------------*/  
    
    /*----------- Global macro definitions ---------------------------------------*/
    /*----------- Global function prototypes -------------------------------------*/

    /* SST_MMGet */
    /*!
    \brief 
    This API responsible to get the current mode of the SST.

    @param aCurrentMode_ptr	    [in/out]  Pointer to current SST mode.        

    @return SST_RC_OK The operation completed successfully.    
    @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init)
    @return SST_RC_ERROR_NULL_POINTER At least one of the past arguments is null.    
    **/
    DxError_t SST_MMGet(SSTMode_t *aCurrentMode_ptr);



    /* SST_MMSet */
    /*!
    \brief 
    This API responsible to set the mode of the SST with the value 
    that comes from the argument aNewMode. 
    The function checks if the argument is legal, means the new mode 
    has equal or less permission than the current SST mode.

    @param aNewMode	    [in]  The new mode to set the SST        

    @return SST_RC_OK The operation completed successfully.    
    @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init).
    @return SST_RC_ERROR_MODE_OUT_OF_RANGE Mode argument is not in mode's range.
    @return SST_RC_ERROR_MODE_ILLEGAL Mode argument is illegal regarding to its permissions.    
    **/
    DxError_t SST_MMSet(SSTMode_t aNewMode);


    /* SST_MMAuthSet */
    /*!
    \brief 
    This API responsible to set the authenticator's mode with value 
    that comes from the argument.
    Authenticator mode can be change in the opposite way of SST mode
    means that the new mode can be equal or more permissions than the current
    authenticator mode. 
    e.g Authenticator can move from User mode to Privileged mode only

    @param aTransactionId	     [in]  The user's transaction ID.       
    @param aAuthHandle	         [in]  The authenticator's handle the user wishes to modify.       
    @param aNewMode	             [in]  The new mode to set the authenticator        
    @param aWorkspace_ptr	     [in]  A pointer to the workspace buffer to 
                                       be used by the SST. If the buffer is 
                                       not word aligned SST will check if 
                                       there is sufficient size after aligning. 
                                       if not an error will be returned
    @param aWorkspaceSizeInBytes [in]  The size in bytes of the 
                                       workspace buffer to be used by the SST. 

    @return SST_RC_OK The operation completed successfully.
    @return SST_RC_FAIL The operation failed (Relevant error from inside modules)   
    @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init)
    @return SST_RC_ERROR_NULL_POINTER At least one of the past arguments is null.    
    @return SST_RC_ERROR_MODE_OUT_OF_RANGE Mode argument is not in mode's range.
    @return SST_RC_ERROR_MODE_ILLEGAL Mode argument is illegal regarding to its permissions    
    @return SST_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED insufficient space in main NVS
    @return SST_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED insufficient space in transaction NVS
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_MMAuthSet(SSTTxnId_t          aTransactionId,  
                            SSTHandle_t         aAuthHandle,
                            SSTMode_t           aNewMode,                        
                            DxByte_t           *aWorkspace_ptr,
                            DxUint32_t          aWorkspaceSizeInBytes);


    /* SST_MMPrivAuthCreate */
    /*!
    \brief 
    This API creates an authenticator that will can be opened only in privileged mode

    @param aTransactionId	     [in]  The user's transaction ID.       
    @param aSessionId	         [in]  The user's session ID.       
    @param aAuthHandle_ptr       [in/out]  Pointer to authenticator's handle the user wishes to create        
    @param aWorkspace_ptr	     [in]  A pointer to the workspace buffer to 
                                       be used by the SST. If the buffer is 
                                       not word aligned SST will check if 
                                       there is sufficient size after aligning. 
                                       if not an error will be returned
    @param aWorkspaceSizeInBytes [in]  The size in bytes of the 
                                       workspace buffer to be used by the SST.
    @return SST_RC_OK The operation completed successfully.
    @return SST_RC_FAIL The operation failed (Relevant error from inside modules)    
    @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init)
    @return SST_RC_ERROR_NULL_POINTER At least one of the past arguments is null.     
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/        
    DxError_t SST_MMPrivAuthCreate( SSTTxnId_t          aTransactionId,
                                    SSTSessionId_t      aSessionId,
                                    SSTHandle_t        *aAuthHandle_ptr,                        
                                    DxByte_t           *aWorkspace_ptr,
                                    DxUint32_t          aWorkspaceSizeInBytes);


    /* SST_MMPrivAuthOpen */
    /*!
    \brief 
    This API opens a Privileged Mode Authenticator (as created by SST_MMPrivAuthCreate API)
    
    @param aSessionId	         [in]  The user's session ID.       
    @param aAuthHandle           [in]  Authenticator handle the user wishes to open, it must be password authenticator        
    @param aWorkspace_ptr	     [in]  A pointer to the workspace buffer to 
                                       be used by the SST. If the buffer is 
                                       not word aligned SST will check if 
                                       there is sufficient size after aligning. 
                                       if not an error will be returned
    @param aWorkspaceSizeInBytes [in]  The size in bytes of the 
                                       workspace buffer to be used by the SST.        
    @return SST_RC_OK The operation completed successfully.
    @return SST_RC_FAIL The operation failed (Relevant error from inside modules)    
    @return SST_RC_ERROR_CLOSED The SST is closed (SST hasn't gone through a proper init)
    @return SST_RC_ERROR_NULL_POINTER At least one of the past arguments is null.                
    @return SST_RC_ERROR_HANDLE_MISMATCH Tty to open privileged authenticator with handle different than password authenticator
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_MMPrivAuthOpen(SSTSessionId_t    aSessionId,
                                 SSTHandle_t       aAuthHandle,                        
                                 DxByte_t         *aWorkspace_ptr,
                                 DxUint32_t        aWorkspaceSizeInBytes);





#ifdef __cplusplus
}
#endif       
#endif  /* _DX_SST_MM_H_ */
