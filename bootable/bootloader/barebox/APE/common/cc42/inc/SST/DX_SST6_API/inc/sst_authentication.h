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
* Title:						Discretix SST API Authentication header file						 					
*																			
* Filename:					    sst_authentication.h 															
*																			
* Project, Target, subsystem:	SST 6.0, API, Authentication
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/

#ifndef _DX_SST_API_AUTH_H_
    #define _DX_SST_API_AUTH_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 	
	
	/*----------- Global Includes ------------------------------------------------*/
    #include "sst_types.h" 	
    #include "sst_data_operations.h"
    #include "sst_utility.h"
    
	/*----------- Global defines -------------------------------------------------*/
    /*! \brief SST AES auth' challenge size in bytes (follows security restrictions and guidelines) **/
    #define SST_CHALLENGE_SIZE_IN_BYTES                         (32UL)
    
    /*! \brief SST AES auth' response size in bytes **/
    #define SST_AES_RESPONSE_SIZE_IN_BYTES						SST_CHALLENGE_SIZE_IN_BYTES

    /*! \brief SST PKCS auth' signature size in bytes.
               The signature size is determined by the modulus size **/    
    #define SST_PK_AUTH_MODULU_SIZE_IN_BYTES                    (256UL) 

    /*! \brief SST Auth' PKCS auth' signature size in bytes.
               The signature size is determined by the modulus size **/    
    #define SST_PK_SIGNATURE_SIZE_IN_BYTES						SST_PK_AUTH_MODULU_SIZE_IN_BYTES 

    /*! \brief Handle value to be passed when the closed auth' is to be used **/
    #define SST_CLOSE_AUTH_HANDLE								((SSTObjectId_t) 0xABFFFFFF)

	/*! \brief Value to be passed when creating a password authenticator that specifies
	           unlimited password failure retires **/
    #define SST_UNLIMITED_FAILURE								(0UL)
    
    /*----------- Global macro definitions ---------------------------------------*/

    /*----------- Global function prototypes -------------------------------------*/
    /*Authentication and Authorization services*/
    
    /*SST_AuthenticatorPasswordCreate*/
    /*!
    \brief Creates a password type authenticator.

    @param aTransactionId [in]
    The transaction ID under which this operation will be performed.
    
    @param aSessionId	[in]
    The authentication session ID, required to verify the user's privileges.
    
    @param aPwd_ptr [in]
    A pointer to the requested password (an array of bytes).
    
    @param aPwdsizeInBytes	 [in]
    The size of the password.
    
    @param aMaxNumOfTrials [in]
    The maximum number of erroneous trials to be allowed before the authenticator will be locked. 
    For unlimited trial failures use SST_UNLIMITED_FAILURE.
    
    @param aInitialCtrlAuth [in]
    The initial authenticator that will enable the user to change a password or reset the locked 
    state of the password authenticator. Use the value SST_CLOSE_AUTH_HANDLE to disable the password 
    change and reset feature (use together with SST_UNLIMITED_FAILURE, otherwise the authenticator 
    can get locked with no way to reset it.)
    
    @param aAuthHandle_ptr	[in/out]
    The user may ask the SST to randomly generate a handle by setting the handle to which this 
    pointer points to SST_HANDLE_RANDOM_GENERATE.
    This parameter also indicates the  location to which the handle of the created object will be put. 
    The required handle id for the authenticator.
    
    @param aWorkspace_ptr [in]
    A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    
    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.
      

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is not free, it is in use by another object.
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.

    **/
    DxError_t SST_AuthenticatorPasswordCreate(SSTTxnId_t      aTransactionId,
                                              SSTSessionId_t  aSessionId,                              
                                              DxByte_t       *aPwd_ptr,
                                              DxUint32_t      aPwdsizeInBytes,
                                              DxUint32_t      aMaxNumOfTrials,
                                              SSTHandle_t     aInitialCtrlAuth,
                                              SSTHandle_t    *aAuthHandle_ptr,
                                              DxByte_t       *aWorkspace_ptr,
                                              DxUint32_t      aWorkspaceSizeInBytes);
	
    
    /*SST_AuthenticatorPubKeyRSACreate*/	
    /*!
    \brief Creates a RSA public key type authenticator.        
    
    @param aTransactionId [in] 
    The transaction ID under which this operation will be performed.

	@param aN_ptr [in]
	A pointer to the RSA key modulus.

    @param aE_ptr [in]
    A pointer to the RSA Public exponent.

	@param aESize [in]
	Size of the RSA Public exponent.

    @param aAuthHandle_ptr [in\out]		
    The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
    This parameter also holds the  location to which the handle of the created object will be put. 
    The required handle id for the authenticator. 
    
    @param aWorkspace_ptr [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.
 
    
    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL                     	The operation failed.
    @return SST_RC_ERROR_CLOSED                 The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	        The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_HANDLE_NOT_FREE       	The requested handle is not free, it is in use by another object.
    @return SST_RC_ERROR_NULL_POINTER	        At least one of the past arguments is null.
    @return SST_DBG_EVENT_CREATE_PKI_AUTH_OK	The operation completed successfully.
    @return SST_DBG_EVENT_CREATE_PWD_AUTH_FAIL	The operation failed.
    @return SST_DBG_EVENT_HANDLE_NOT_FREE	    The requested handle is not free, it is in use by another object.
    @return SST_DBG_EVENT_TXN_INVALID	        The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_DBG_EVENT_NULL_POINTER	        At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	        An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	            The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	            Insufficient workspace given by user.
    **/
    DxError_t SST_AuthenticatorPubKeyRSACreate(	SSTTxnId_t   aTransactionId,
												DxByte_t    *aN_ptr,
												DxByte_t    *aE_ptr,
												DxUint32_t   aESize,
												SSTHandle_t *aAuthHandle_ptr,
												DxByte_t    *aWorkspace_ptr,
												DxUint32_t   aWorkspaceSizeInBytes);
     
    
    /*SST_AuthenticatorPubKeyECCCreate*/	
    /*!
    \brief Creates an ECC public key type authenticator.        
    
    @param aTransactionId [in] 
    The transaction ID under which this operation will be performed.

    @param aECCPubKey_ptr [in]
	A pointer to the ECC Public Key. the EC Domain Identification should be secp256r1 
	(Reference: [13] SEC 2: Recommended elliptic curve domain parameters. Version 1.0. Certicom 2000.)

	@param aECCPubKeySizeInBytes [in]
	Size of the ECC Public key.

    @param aAuthHandle_ptr [in\out]		
    The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
    This parameter also holds the  location to which the handle of the created object will be put. 
    The required handle id for the authenticator. 
    
    @param aWorkspace_ptr [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.
 
    
    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL                     	The operation failed.
    @return SST_RC_ERROR_CLOSED                 The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	        The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_HANDLE_NOT_FREE       	The requested handle is not free, it is in use by another object.
    @return SST_RC_ERROR_NULL_POINTER	        At least one of the past arguments is null.
    @return SST_DBG_EVENT_CREATE_PKI_AUTH_OK	The operation completed successfully.
    @return SST_DBG_EVENT_CREATE_PWD_AUTH_FAIL	The operation failed.
    @return SST_DBG_EVENT_HANDLE_NOT_FREE	    The requested handle is not free, it is in use by another object.
    @return SST_DBG_EVENT_TXN_INVALID	        The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_DBG_EVENT_NULL_POINTER	        At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	        An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	            The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	            Insufficient workspace given by user.
    **/
    DxError_t SST_AuthenticatorPubKeyECCCreate(SSTTxnId_t   aTransactionId,
                                               DxByte_t    *aECCPubKey_ptr,
											   DxUint32_t   aECCPubKeySizeInBytes,
											   SSTHandle_t *aAuthHandle_ptr,
											   DxByte_t    *aWorkspace_ptr,
											   DxUint32_t   aWorkspaceSizeInBytes);

	
    /*SST_AuthenticatorAESSharedSecretCreate*/	
    /*!
    \brief Creates a 128 bit AES shared secret authenticator.
     
    @param aTransactionId [in]
    The transaction ID under which this operation will be performed.
    
    @param aAesKey_ptr [in]
    A pointer to the AES key. 
    
    @param aKeyType [in]	
    The AES key type (size of the key).
    
    @param aHandle_ptr [in\out]		
    The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
    this parameter also holds the location to which the handle of the created object will be put.
    The required handle id for the authenticator.  
    
    @param aWorkspace_ptr [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.        

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is not free. It is in use by another object.
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.        
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_AuthenticatorAESSharedSecretCreate(SSTTxnId_t      aTransactionId,
                                                     DxByte_t       *aAesKey_ptr,
                                                     SSTAESKeyType_t aKeyType,                    
                                                     SSTHandle_t    *aHandle_ptr,
                                                     DxByte_t       *aWorkspace_ptr,
                                                     DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_AuthenticatorDelete*/	
    /*!
    \brief  Deletes an authentication object

    @param aTransactionId [in] 
    The transaction ID under which this operation will be performed.
     
    @param aHandle	      [in]
    The handle of the object to be deleted
    
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.        
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	The handle was not found 
    @return SST_RC_ERROR_AUTH_IN_USE	    One or more objects in the DB point to this authenticator. An authenticator which is in use cannot be deleted.
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type (not authentication) or an invalid handle
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_ACCESS_DENIED      Access denied.
    **/
    DxError_t SST_AuthenticatorDelete(SSTTxnId_t aTransactionId,SSTHandle_t aHandle);

    
    /*SST_SessionCreate*/	
    /*!
    \brief         Creates a new authentication session with no authenticators. 
                   Once an authentication session is created, a user can use the 
                   session ID it got to add authenticators and then to access SST 
                   objects that require authentication.

    @param aSessionId_ptr [in]
    A pointer to the place where the new session ID will be stored.

    @return SST_RC_OK	                    The operation completed successfully 
    @return SST_RC_FAIL	                    The operation failed
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_NULL_POINTER	    One of the received pointer was NULL, operation terminated
    @return SST_RC_ERROR_NO_FREE_SESSION	The number of open session has reached its maximal value.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
    **/
    DxError_t SST_SessionCreate(SSTSessionId_t *aSessionId_ptr);
    
    
    /*SST_SessionDelete*/	
    /*!
    \brief Deletes an authentication session.

    @param aSessionId [in]  The ID of the session to delete.
 
    @return SST_RC_OK	                        The operation completed successfully 
    @return SST_RC_FAIL	                        The operation failed
    @return SST_RC_ERROR_CLOSED                 The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.    
    @return SST_RC_ERROR_SESSION_INVALID		The session ID received is invalid (e.g. no such session)
    @return SST_RC_ERROR_VCRYS       	        An error that involves the cryptography services package has occurred.
    **/
    DxError_t SST_SessionDelete(SSTSessionId_t  aSessionId);
    
    
    /*SST_AuthenticatorChallengeGet*/	
    /*!
    \brief         Generates a challenge according to the requested size and returns it with a corresponding 
                   challenge ID. Later on, the user should answer the challenge (with the corresponding ID)
                   and by that unlock an authenticator.
    
    @param aChallengeBuffSize [in]
    The size of the required challenge buffer in bytes. 
    
    @param aChallengeBuff_ptr [in]
    A pointer to the buffer in which the challenge will be placed.
    
    @param aChallengeId_ptr   [out]
    A pointer to the location of the challenge ID.
    
    @return SST_RC_OK						The operation completed successfully 
    @return SST_RC_FAIL						The operation failed
    @return SST_RC_ERROR_CLOSED				The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL				The fatal error occurred.        
    @return SST_RC_ERROR_NULL_POINTER		One of the received pointers was NULL, operation terminated
    @return SST_RC_ERROR_VCRYS       		An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_CHALLENGE_SIZE     Wrong challenge size received.
    **/
    DxError_t SST_AuthenticatorChallengeGet(DxByte_t          *aChallengeBuff_ptr,
                                            DxUint32_t         aChallengeBuffsize,  
                                            SSTChallengeId_t  *aChallengeId_ptr);

    
    
    /*SST_AuthenticationOpen*/	
    /*!
    \brief         Unlocks an authenticator and adds it to an open session, by that the privileges of this session's user may extend.

    @param aSessionId [in]
    The ID of the session to which the change will be made.

    @param aAuthHandle
    The handle of the authenticator to unlock and add to the session.

    @param aAuthBuff_ptr [in]
    A pointer to a buffer containing information needed to unlock the authenticator. 

    @param aAuthBuffSizeInBytes [in]
    The size of the buffer.

    @param aChallengeId [in]
    The ID of the challenge the user got. If no challenge is required this field is overlooked.
    
    @param aWorkspace_ptr [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    
	@param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.        

    @return SST_RC_OK	                    The operation completed successfully 
    @return SST_RC_FAIL	                    The authentication operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_AUTH_SESSION_FULL	The session is full, remove one of the authenticators from the session or start a new session.
    @return SST_RC_ERROR_NULL_POINTER	    One of the received pointer was NULL, operation terminated
    @return SST_RC_ERROR_INVALID_CHALLENGE	The challenge ID received is invalid (e.g. the challenge related to that cookie was already used, challenge don't exist ?)
	@return SST_RC_ERROR_CHALLENGE_SIZE		Wrong challenge size received
    @return SST_RC_ERROR_INVALID_SESSION	The session ID received is invalid as it refers to no active session. 
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	The handle was not found in the DB
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type (not authentication) or an invalid handle
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
    @return SST_RC_ERROR_AUTH_FAILED        User Authentication failed.
    @return SST_RC_ERROR_ACCESS_DENIED      The user doesn't have the needed permission to perform this operation.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
	@return SST_RC_ERROR_TRANSACTION_NOT_ENDED     Transaction has not been committed.

	@note:	1.	This operation must be performed outside of transaction
			2.	The challenge will be not be deleted in cases of illegal parameters (NULL pointers, workspace etc.)
			3.	When Using the RSA Public key authentication, sign operation should be performed 
				in Hash SHA1 mode.
			4.	When Using the ECC public key authentication, sign operation should be performed
				in Hash SHA256 mode
    **/
    DxError_t SST_AuthenticationOpen(SSTSessionId_t    aSessionId,
                                     SSTHandle_t       aAuthHandle,
                                     DxByte_t         *aAuthBuff_ptr,
                                     DxUint32_t        aAuthBuffSizeInBytes,
                                     SSTChallengeId_t  aChallengeId,
                                     DxByte_t         *aWorkspace_ptr,
                                     DxUint32_t        aWorkspaceSizeInBytes);

    
    /*SST_AuthenticationClose*/
    /*!
    \brief         Removes an authenticator from a user's session, may reduce a user's privileges.

    @param  aSessionId [in]
    The ID of the session to which the change will be made.
    
	@param aAuthHandle [in]
    The handle of the authenticator to close.

    
    @return SST_RC_OK	                    The operation completed successfully 
    @return SST_RC_FAIL	                    The operation failed
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.   
    @return SST_RC_ERROR_SESSION_EMPTY   	The last authenticator was removed from the session, the session is now empty. (this is not an error)
    @return SST_RC_ERROR_SESSION_INVALID	The session ID received is invalid as it refers to no active session. 
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type (not authentication) or an invalid handle
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	The handle was not found in the relevant session
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
    **/
    DxError_t SST_AuthenticationClose(SSTSessionId_t aSessionId,SSTHandle_t aAuthHandle);
    	
    
    /*SST_AuthenticatorPasswordResetAndChange*/	
    /*!
    \brief         Resets a ?password? authenticator?s retry counter.

    @param aTransactionId [in]
    The user's transaction ID.
    
    @param aSessionId [in]
    The user's session ID.
    
    @param aAuthHandle [in]
    The handle of the authenticator which counter should be reset.
    
    @param aNewPwdsizeInBytes [in]
    The size of the new password.
    
    @param aNewPwd_ptr [in]
    A pointer to the location of the new password.  
    
    @param aNewMaxRetry [in]
    The new maximal retry number (0 for infinity).
    
    @param aWorkspace_ptr [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully 
    @return SST_RC_FAIL	                    The operation failed
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have adequate permission to perform the desired action. The operation failed.
    @return SST_RC_ERROR_NULL_POINTER	    One of the received pointers was NULL, operation terminated
    @return SST_RC_ERROR_TXN_INVALID	    No active transaction corresponds to the received ID, operation terminated.
    @return SST_RC_ERROR_SESSION_INVALID	No active session corresponds to the received ID, operation terminated.
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	The handle was not found in the DB
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type (not password authenticator) or an invalid handle
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_AuthenticatorPasswordResetAndChange(SSTTxnId_t      aTransactionId,
                                                      SSTSessionId_t  aSessionId,
                                                      SSTHandle_t     aAuthHandle,  
                                                      DxUint32_t      aNewPwdsizeInBytes,                    
                                                      DxByte_t       *aNewPwd_ptr,
                                                      DxUint32_t      aNewMaxRetry,
                                                      DxByte_t       *aWorkspace_ptr,
                                                      DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_AuthenticatorBind*/	
    /*!
    \brief 
    description        Binds an authenticator to an SST element (an item in the DB). 
                       Once an authenticator is bound to an element (with certain permission), 
                       access to that element is achieved by unlocking that authenticator and 
                       adding it to the user's session.

    @param aTransactionId	        [in]
    The user's transaction ID.
    
    @param aSessionId	            [in]
    The authentication session ID, will be used to make sure that the user is allowed to bind authenticators to that element.
    
    @param aHandle	                [in]
    The handle of the element to which the authenticator will be bound.
    
    @param aAuthToBind	            [in]
    The handle of the authenticator to bind to the SST element.
    
    @param aPermission	            [in]
    An enum value that states the permission level that will be granted once the bound authenticator will be unlocked (with relevance to the SST element of course).
 
    @param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes    [in]
    The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                      The operation completed successfully 
    @return SST_RC_FAIL	                      The operation failed
    @return SST_RC_ERROR_CLOSED               The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL                The fatal error occurred.    
    @return SST_RC_ERROR_MAX_BOUND_REACHED	  The bounded objects pool for that SST element is full. 
    @return SST_RC_ERROR_NULL_POINTER	      One of the received pointers was NULL, operation terminated
    @return SST_RC_ERROR_SESSION_INVALID	  No active session corresponds to the received ID, operation terminated.
    @return SST_RC_ERROR_TXN_INVALID	      No active transaction corresponds to the received ID, operation terminated.
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	  A handle was not found in the DB
    @return SST_RC_ERROR_HANDLE_MISMATCH	  The authenticator handle refers to an object of different type (not authentication) or an invalid handle
    @return SST_RC_ERROR_ACCESS_DENIED	      The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_VCRYS       	      An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	          The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE			  Insufficient workspace given by user.
	@return SST_RC_ERROR_AUTH_ALREADY_BOUND	  The authenticator is already bound to the object.
    **/
    DxError_t SST_AuthenticatorBind(SSTTxnId_t        aTransactionId,  
                                    SSTSessionId_t    aSessionId, 
                                    SSTHandle_t       aHandle,              
                                    SSTHandle_t       aAuthToBind,             
                                    SSTPermissions_t  aPermission,
                                    DxByte_t         *aWorkspace_ptr,
                                    DxUint32_t        aWorkspaceSizeInBytes) ;

    
    /*SST_AuthenticatorUnbind*/
    /*!
    \brief         This function unbinds an authenticator from an SST element 
                   (an item in the DB). Once an authenticator is unbound from 
                   an element it will be no longer eligible to grant access to 
                   that element.

    @param aTransactionId           [in]
    The user's transaction ID.
    
    @param aSessionId               [in]
    The user's session ID.
    
    @param aHandle                  [in]
    The handle of the element from which the authenticator will be unbound.
    
    @param aAuthToUnbind            [in]
    The handle of the authenticator to unbind from the SST element.

	#param aPermission				[in]
	The permission of the authenticator.

    @param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes    [in]
    The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully 
    @return SST_RC_FAIL	                    The operation failed
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_NULL_POINTER	    One of the received pointers was NULL, operation terminated
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found in the DB / session
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type (not authentication) or an invalid handle
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_ZOMBIE	            No authenticator can provide read permission to that object (zombie alert).
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_AuthenticatorUnbind(SSTTxnId_t        aTransactionId,   
                                      SSTSessionId_t    aSessionId,
                                      SSTHandle_t       aHandle,              
                                      SSTHandle_t		aAuthToUnbind,
                                      SSTPermissions_t  aPermission,
                                      DxByte_t         *aWorkspace_ptr,
                                      DxUint32_t        aWorkspaceSizeInBytes);






    /*SST_AuthenticatorAppIDCreate*/    
    /*!
    \brief Creates a application ID type authenticator.

    @param aTransactionId [in]
    The transaction ID under which this operation will be performed.

    @param aSessionId	[in]
    The authentication session ID, required to verify the user's privileges.

    @param aAppID_ptr [in]
    A pointer to the requested application id (an array of bytes).

    @param aAppIDsizeInBytes	 [in]
    The size of the application id in bytes.
    
    @param aAuthHandle_ptr	[in/out]
    The user may ask the SST to randomly generate a handle by setting the handle to which this 
    pointer points to SST_HANDLE_RANDOM_GENERATE.
    This parameter also indicates the  location to which the handle of the created object will be put. 
    The required handle id for the authenticator.

    @param aWorkspace_ptr [in]
    A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
    check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.


    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.    
    @return SST_RC_ERROR_HANDLE_MISMATCH 	The handle given for control authenticator is not of an authenticator.
    @return SST_RC_ERROR_AUTH_WRONG_TYPE -  The authenticator type is not pwd or appId
    @return SST_RC_ERROR_SESSION_INVALID -  Non existent session ID
    @return SST_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED - Insufficient space handling, main NVS
    @return SST_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED - Insufficient space handling, transaction NVS
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_AuthenticatorAppIDCreate( SSTTxnId_t      aTransactionId,
                                            SSTSessionId_t  aSessionId,                              
                                            DxByte_t       *aAppID_ptr,
                                            DxUint32_t      aAppIDsizeInBytes,
                                            SSTHandle_t    *aAuthHandle_ptr,
                                            DxByte_t       *aWorkspace_ptr,
                                            DxUint32_t      aWorkspaceSizeInBytes);


/*SST_AuthenticatorAppIDCreate*/
/*!
    \brief Creates a application id type authenticator.

    @param aTransactionId [in]
    The transaction ID under which this operation will be performed.
    
    @param aSessionId	[in]
    The authentication session ID, required to verify the user's privileges.
    
    @param aAppID_ptr [in]
    A pointer to the requested application id (an array of bytes).
    
    @param aAppIDsizeInBytes	 [in]
    The size of the application id.
        
    @param aAuthHandle_ptr	[in/out]
    The user may ask the SST to randomly generate a handle by setting the handle to which this 
    pointer points to SST_HANDLE_RANDOM_GENERATE.
    This parameter also indicates the  location to which the handle of the created object will be put. 
    The required handle id for the authenticator.
    
    @param aWorkspace_ptr [in]
    A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    
    @param aWorkspaceSizeInBytes [in]
    The size in bytes of the workspace buffer to be used by the SST.
      

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is not free, it is in use by another object.
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.

    **/
DxError_t SST_AuthenticatorAppIDCreate( SSTTxnId_t      aTransactionId,
										SSTSessionId_t  aSessionId,          
										DxByte_t       *aAppID_ptr,
										DxUint32_t      aAppIDsizeInBytes,
										SSTHandle_t    *aAuthHandle_ptr,
										DxByte_t       *aWorkspace_ptr,
										DxUint32_t      aWorkspaceSizeInBytes);
                                      
#ifdef __cplusplus
}
#endif

#endif  /* _DX_SST_API_AUTH_H_ */
