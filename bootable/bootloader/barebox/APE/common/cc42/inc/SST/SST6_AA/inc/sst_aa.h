/*! \file 
**********************************************************************************	
* Title:						Discretix SST AA header file						 					
*																			
* Filename:					    sst_aa.h															
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


/************************************************************************/

/*FUNCTION NAME*/
/*!
\brief Description.

@param PARAM [in]
    Description.
@param aHandle_ptr	[in/out]
    Description.

@return SST_RC_OK	                    The operation completed successfully.
@return SST_RC_FAIL	                    The operation failed.
@return SST_RC_ERROR_HANDLE_NOT_FREE	Description.

**/

/************************************************************************/
 


#ifndef _DX_SST_AA_H_
    #define _DX_SST_AA_H_

	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 
    #include "sst_general_types.h"
    #include "sst_vcrys.h"    

	/*----------- Global defines -------------------------------------------------*/
    
    /*! \brief SST AA close auth' handle - Auth that can never be open, used for App Id control auth **/
    #define SST_AA_CLOSE_AUTH_HANDLE           ((SSTObjectId_t) 0xABFFFFFF)

    /*! \brief SST AA minimum workspace size in bytes **/
    #define SST_AA_MIN_WORKSPACE_SIZE_IN_BYTES   (108UL) /* was 128 */

    /*! \brief SST challenge ID for opening a PWD auth' **/
    #define SST_AA_PWD_AUTH_CHALLENGE_ID         ((SSTChallengeId_t)0xBADABADA)
    
    /*! \brief SST AA Meta data size in bytes **/
    #define SST_AA_META_DATA_SIZE_IN_BYTES       (108UL) /* was 128 */
    
    /*! \brief SST AA Meta data size in bytes **/
    #define SST_AA_AUTH_PUB_KEY_N_SIZE_IN_BYTES  (256UL)

    /*! \brief SST AA AES Shared secret size in bytes (the size of the AES key) **/
    #define SST_AA_AES_KEY_SIZE_IN_BYTES	     (32UL)    

    /*! \brief SST AA AES auth' challenge size in bytes (follows security restrictions and guidelines) **/
    #define SST_AA_CHALLENGE_SIZE_IN_BYTES       (32UL)
    
    /*! \brief SST AA AES auth' response size in bytes **/
    #define SST_AA_AES_RESPONSE_SIZE_IN_BYTES    SST_AA_CHALLENGE_SIZE_IN_BYTES

    /*! \brief the RSA (PubKey) modulus size. (Currently follows VCRYS definitions) **/
    #define SST_AA_PK_MOD_SIZE_IN_BYTES	        SST_VCRYS_RSA_MODULU_SIZE  

    /*! \brief SST AA PKCS auth' signature size in bytes.
               The signature size is determined by the modulus size **/    
    #define SST_AA_PK_SIGNATURE_SIZE_IN_BYTES    SST_AA_PK_MOD_SIZE_IN_BYTES 
    
	/*! \brief the ECC Public Key (Currently follows VCRYS definitions) **/
	#define SST_AA_ECC_PK_MAX_SIZE_IN_BYTES		SST_VCRYS_ECC_PK_MAX_SIZE_IN_BYTES   
    
	/*! \brief Value to be passed when creating a password authenticator that specifies
	           unlimited password failure retires **/
    #define SST_AA_UNLIMITED_FAILURE			(0UL)

	/*----------- Global macro definitions ---------------------------------------*/


    /*----------- Global type definitions ----------------------------------------*/
    /*! \brief SST AA Return codes enumeration **/
    typedef enum
    {
        SST_AA_RC_OK	                            =0,
        SST_AA_RC_FAIL	                            =1,
        SST_AA_RC_ERROR_MEM_ALLOC	                =2,
        SST_AA_RC_ERROR_NULL_POINTER	            =3,
        SST_AA_RC_ERROR_SESSION_INVALID	            =4,
        SST_AA_RC_ERROR_VCRYS	                    =5,
        SST_AA_RC_ERROR_VDB	                        =6,
        SST_AA_RC_ERROR_TXN_INVALID	                =7,
        SST_AA_RC_ERROR_CHALLENGE_INVALID	        =8,
        SST_AA_RC_ERROR_HANDLE_NOT_FOUND	        =9,
        SST_AA_RC_ERROR_HANDLE_MISMATCH	            =10,
        SST_AA_RC_ERROR_ACCESS_DENIED		        =11,
        SST_AA_RC_ERROR_SESSION_EMPTY 	            =12,
        SST_AA_RC_ERROR_HANDLE_OUT_OF_RANGE	        =13,
        SST_AA_RC_ERROR_HANDLE_IN_USE	            =14,
        SST_AA_RC_ERROR_PASSWORD	                =16,
        SST_AA_RC_ERROR_ZOMBIE	                    =17,
        SST_AA_RC_ERROR_MAX_BOUND_REACHED	        =18,
        SST_AA_RC_ERROR_MAX_SESSIONS_OPEN           =19,
        SST_AA_RC_ERROR_CHALLENGE_SIZE	            =20,
        SST_AA_RC_ERROR_MAX_CHALLENGES_PENDING      =21,
        SST_AA_RC_ERROR_MODULUS_SIZE                =23,
        SST_AA_RC_ERROR_VOS                         =24,
        SST_AA_RC_ERROR_SIZE_MISMATCH               =25,
        SST_AA_RC_ERROR_SESSION_FULL                =26,
		SST_AA_RC_ERROR_NOT_ACTIVE  			    =27,
		SST_AA_RC_ERROR_AUTH_FAILED				    =28,
		SST_AA_RC_ERROR_AUTH_ALREADY_BOUND	        =29,
		SST_AA_RC_ERROR_NOT_SUPPORTED		   	    =30,
        SST_AA_RC_ERROR_FATAL                       =31,
		SST_AA_RC_ERROR_TXN_NOT_ENDED		 	    =32,
        SST_AA_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED  =33,
        SST_AA_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED   =34,
        SST_AA_RC_ERROR_NVS_WRITE                   =35,
        SST_AA_RC_ERROR_NVS_READ                    =36,
        SST_AA_RC_ERROR_NVS_ACCESS                  =37,
        SST_AA_RC_ERROR_MODE_ILLEGAL                =38,
        SST_AA_RC_ERROR_AUTH_WRONG_TYPE 	        =39,
		SST_AA_RC_ERROR_INTEGRITY_COMPROMIZED       =40,
    }SSTAAReturnCodes_t;                        
    
    /*! \brief SST AA reference counter increment / decrement enumeration **/
    typedef enum
    {
        SST_AA_REF_COUNTER_INC,
        SST_AA_REF_COUNTER_DEC
    }SSTAAIncDec_t;  
    
    /*! \brief SST AA Meta Data Place holder definition 
               (used to allow higher levels to allocate space in the meta data object) **/
    typedef struct  
    {
        DxByte_t    byteArr[SST_AA_META_DATA_SIZE_IN_BYTES];
    }SSTAAMetaDataPlaceHolder_t;

    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/


    /*SST_AAInit*/
    /*!
    \brief 
        Initializes the SST's authentication and authorization module for use. 
        This API must be called prior to any use of this module.


    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_AA_RC_ERROR_MEM_ALLOC	    The operation failed, memory 
                                                    allocation did not succeed.

    **/
    SSTAAReturnCodes_t SST_AAInit(void);


    /*SST_AATerminate*/
    /*!
    \brief 
        Terminates the AA module. No further use of the SST AA component is possible.


    @return SST_RC_OK   The operation completed successfully.

    **/
    SSTAAReturnCodes_t SST_AATerminate(void);


    /*SST_AATerminate_dbg*/
    /*!
    \brief 
    Terminates the AA module. No further use of the SST AA component is possible.
    Release sources and initialize globals

    @return SST_RC_OK   The operation completed successfully.

    **/
    SSTAAReturnCodes_t SST_AATerminate_dbg(void);

    
    /*SST_AASessionCreate*/
    /*!
    \brief 
        Creates a new authentication session. 
        Once an authentication session is created, a user can use the session ID to access SST objects 
        that require authentication, to add authenticators to the session and to remove them.
        
    @param aSessionId_ptr [out]
        A pointer to the place where the new session ID will be stored.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	One of the received pointer was NULL, 
                                                    operation terminated
    **/
    SSTAAReturnCodes_t SST_AASessionCreate(SSTSessionId_t *aSessionId_ptr);

    
    /*SST_AASessionDelete*/
    /*!
    \brief 
        Deletes an open authentication session.
    
    @param aSessionId   [in]
        The ID of the session to be deleted.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_AA_RC_ERROR_NULL_POINTER	One of the received pointers 
                                                    was NULL, operation terminated
    @return SST_AA_RC_ERROR_SESSION_INVALID	The session received is invalid
                                                    (e.g. no such session)

    **/
    SSTAAReturnCodes_t SST_AASessionDelete(SSTSessionId_t aSessionId);


    /*SST_AAChallengeGet*/
    /*!
    \brief 
        Generates a challenge (random sequence of bytes) according to the requested
        size and returns it with a corresponding ID. 
        Later on, the user should answer the challenge (with the corresponding cookie) 
        and by that unlock an authenticator.

    @param aChallengeBuffSize   [in]
        The size of the required challenge buffer.
    @param aChallengeBuff_ptr   [out] 
        A pointer to the buffer in which the challenge will be placed.
    @param aChallengeId_ptr [out]
        A pointer to the returned challenge ID   

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	One of the received pointers was NULL, 
                                                    operation terminated
    @return SST_AA_RC_ERROR_CRYS	        General error has occurred when using CRYS.

    **/
    SSTAAReturnCodes_t SST_AAChallengeGet(DxByte_t         *aChallengeBuff_ptr,
                                          DxUint32_t        aChallengeBuffSize,  
                                          SSTChallengeId_t *aChallengeId_ptr);

      
    /*SST_AAMetaDataCreate*/
    /*!
    \brief 
        Creates a meta-data object in a given location. Later on, this meta-data
        will be part of the object's meta-data object and used to grant access to an SST element 
        according to the authenticators and permissions in it.


    @param aTransactionId          [in]
        The transaction ID.
    @param aSessionId              [in]
        The session ID.
    @param aElementHandle          [in]
        The handle of the SST element to which the meta-data points (usually a new record).
    @param aInitialCtrlAuth        [in]
        The handle of the initial control authenticator for the object 
        (one that has R/W/control privileges). 
    @param aAAMetaDataBuffer       [out]
        A pointer to the location where the AA meta-data structure will be placed.
    @param aWorkspace_ptr          [in]
        A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
        The size in bytes of the provided workspace.
   
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	One of the received pointers was NULL, operation terminated
    **/
    SSTAAReturnCodes_t SST_AAMetaDataCreate(SSTTxnId_t                    aTransctionId,
                                            SSTSessionId_t                aSessionId, 
                                            SSTHandle_t					 aElementHandle,  
                                            SSTHandle_t					 aInitialCtrlAuth,
                                            SSTAAMetaDataPlaceHolder_t   *aAAMetaDataBuffer,
                                            DxByte_t                     *aWorkspace_ptr,
                                            DxUint32_t                    aWorkspaceSizeInBytes);  /* todo fix the documentation above */                                       
        
    
    /*SST_AASessionAuthAdd*/
    /*!
    \brief 
        Unlocks an authenticator and adds it to an open session, by that the 
        privileges of this session's user may extend.

    @param aAuthHandle             [in]
        The handle of the authenticator to unlock and add to the session.
    @param aAuthBuff_ptr           [in]
        A pointer to a buffer containing information needed to unlock the authenticator.
        aAuthBuff_ptr can be null (relevant for password) 
    @param aAuthBuffSize           [in]
        The size of the buffer.
    @param aChallengeId            [in]
        The ID of the challenge the user got. If no challenge is required this field is overlooked.
    @param aSessionId              [in]  
        The ID of the authentication session to which the change will be made.
    @param aWorkspace_ptr          [in]
        A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
        The size in bytes of the provided workspace.
        

    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL	                        The operation failed.
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.
    @return SST_AA_RC_ERROR_SESSION_FULL	    The session is full, remove one of the authenticators from the 
                                                session or start a new session.
    @return SST_AA_RC_ERROR_NULL_POINTER	    One of the received pointer was NULL, operation terminated
    @return SST_AA_RC_ERROR_CHALLENGE_INVALID	The challenge ID received is invalid (e.g. the challenge 
                                                       related to that cookie was already used,challenge don't exist …)
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND	The handle was not found in the DB
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	    The authenticator handle refers to an object of different 
                                                       type (not authentication)
    @return SST_AA_RC_ERROR_ACCESS_DENIED		The user does not have an adequate permission to perform 
                                                       this operation.
    @return SST_AA_RC_ERROR_SESSION_INVALID	    The session received is invalid (e.g. no such session)
    **/
    SSTAAReturnCodes_t SST_AASessionAuthAdd(SSTHandle_t		      aAuthHandle,
                                            DxByte_t             *aAuthBuff_ptr,
                                            DxUint32_t            aAuthBuffSize,
                                            SSTChallengeId_t      aChallengeId,
                                            SSTSessionId_t        aSessionId,
                                            DxByte_t             *aWorkspace_ptr,
                                            DxUint32_t            aWorkspaceSizeInBytes);


    /*SST_AASessionAuthRemove*/
    /*!
    \brief 
        Removes an authenticator from a user's session, may reduce a user's privileges.

    @param aAuthHandle  [in]
        The handle of the authenticator to unlock and add to the session.
    @param aSessionId   [in]
        The authentication session ID to which the change will be made.

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND The handle was not found in the relevant session
    @return SST_AA_RC_ERROR_SESSION_EMPTY 	 The last authenticator was removed from the session, the 
                                                     session is now empty. (this is not an error)
    @return SST_AA_RC_ERROR_SESSION_INVALID	 The session ID received is invalid as it refers to
                                                     no active session. 
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	 The authenticator handle refers to an object of different 
                                                     type (not authentication)
    **/
    SSTAAReturnCodes_t SST_AASessionAuthRemove(SSTHandle_t    aAuthHandle,
                                               SSTSessionId_t aSessionId);


    /*SST_AASessionIsEmpty*/
    /*!
    \brief 
        Returns DX_TRUE if no authenticators are registered for this session and DX_FALSE otherwise.

    @param aSessionId_ptr   [in]
        The ID of the session we wish to query.
    @param aAnswer_ptr      [out]
        A pointer to the location in which the answer will be put.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_FATAL              The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	One of the received pointers was NULL, operation terminated.
    @return SST_AA_RC_ERROR_SESSION_INVALID	There is no session related to that ID, operation terminated.
    @return SST_AA_RC_ERROR_SESSION_EMPTY 	The last authenticator was removed from the session, 
                                                    the session is now empty. (this is not an error)
    **/
    SSTAAReturnCodes_t SST_AASessionIsEmpty(SSTSessionId_t  aSessionId,
                                            DxBool_t       *aAnswer_ptr);


    /*SST_AAAuthBind*/
    /*!
    \brief 
        Binds an authenticator to an SST element (an item in the DB). Once an authenticator is bound to an element
        (with certain permission), access to that element is achieved by unlocking that authenticator and adding 
        it to the user's session.

    @param aTransactionId    [in]
        The user's transaction ID.
    @param aAuthToBind       [in]
        The handle of the authenticator to bind to the SST element.
    @param aSessionId        [in]
        The authentication session ID, will be used to make sure that the user is allowed to bind authenticators to that element.
    @param aPermission       [in]
        An enum value that states the permission level that will be granted once the bound authenticator will be unlocked (with relevance to the SST element of course).
    @param aAAMetaDataBuffer [in]
        A pointer to the AA meta data buffer in which the change will be made.    

    @return SST_RC_OK	                      The operation completed successfully.
    @return SST_RC_FAIL	                      The operation failed.
    @return SST_AA_RC_ERROR_MAX_BOUND_REACHED The bounded objects pool for that SST element is full. 
    @return SST_RC_ERROR_FATAL                The fatal error occurred.
    @return SST_AA_RC_ERROR_TXN_INVALID	      No active transaction corresponds to the received ID, operation terminated.
    @return SST_AA_RC_ERROR_NULL_POINTER	  One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND  The handle was not found in the DB
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	  The authenticator handle refers to an object of different type (not authentication)
    @return SST_AA_RC_ERROR_ACCESS_DENIED	  The user doesn't have the needed permission to perform this operation
    @return SST_AA_RC_ERROR_SESSION_INVALID	  There is no session related to that ID, operation terminated.
    @return SST_AA_RC_ERROR_CRYS              An error that involves the cryptography services package has occurred. 
	@return SST_AA_RC_ERROR_AUTH_ALREADY_BOUNDif the authenticator is already bound to the object

	@notes: it allowed to bind an authenticator to a data object even if the authenticator is not open on the user's session
    **/
    SSTAAReturnCodes_t SST_AAAuthBind(SSTTxnId_t                  aTransactionId, 
                                      SSTHandle_t				  aAuthToBind,             
                                      SSTSessionId_t              aSessionId,
                                      SSTPermissions_t            aPermission,
                                      SSTAAMetaDataPlaceHolder_t *aAAMetaDataBuffer);


    /*SST_AAAuthUnbind*/
    /*!
    \brief 
        This function unbinds an authenticator from an SST element (an item in the DB).
        Once an authenticator is unbound from an element it will be no longer eligible 
        to grant access to that element.

    @param aTransactionId   [in]
        The user's transaction ID.
    @param aAuthToUnbind    [in]
        The handle of the authenticator to unbind from the SST element.
    @param aSessionId       [in]
        The ID of the  authentication session to which the change will be made.
    @param aPermission       [in]
        An enum value that states the permission level that will be granted once the bound authenticator will be unlocked (with relevance to the SST element of course).
    @param aAAMetaDataBuffer [in]
        A pointer to the AA meta data buffer in which the change will be made.    

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_AA_RC_ERROR_NOTHING_BOUNDED	 The bounded objects pool for that SST element is empty.
    @return SST_AA_RC_ERROR_TXN_INVALID	     No active transaction corresponds to the received ID, 
                                                     operation terminated.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND The handle was not found in the DB
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	 The authenticator handle refers to an object of different type (not authentication)
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The user doesn't have the needed permission to perform this operation
    @return SST_AA_RC_ERROR_SESSION_INVALID	 There is no session related to that ID, operation terminated.
    @return SST_AA_RC_ERROR_ZOMBIE	         No authenticator can provide read permission to that object (zombie alert).
    **/
    SSTAAReturnCodes_t SST_AAAuthUnbind(SSTTxnId_t                  aTransactionId, 
                                        SSTHandle_t					aAuthToUnbind,             
                                        SSTSessionId_t              aSessionId,
                                        SSTPermissions_t            aPermission,
                                        SSTAAMetaDataPlaceHolder_t *aAAMetaDataBuffer); 
                                        

    /*SST_AAAuthIsBound*/
    /*!
    \brief 
        Returns DX_TRUE if the authenticator which handle was provided has any elements bound to it and DX_FALSE otherwise.

    @param aAuthHandle  [in]
        The handle of the element the query relates to.
    @param aAnswer_ptr  [out]
        A pointer to the location where the answer will be put.    
    
    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The user doesn't have the needed permission to perform this 
                                                     operation
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND The handle was not found in the DB
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	 The authenticator handle refers to an object of different 
                                                     type (not authentication)
    @return SST_AA_RC_ERROR_SESSION_INVALID	 No session identifier corresponds with the received cookie

    **/
    SSTAAReturnCodes_t SST_AAAuthIsBound(SSTHandle_t    aAuthHandle,              
                                         DxBool_t             *aAnswer_ptr);

    /*SST_AAElementIsBound*/
    /*!
    \brief 
        Returns DX_TRUE if the element which handle was provided has any authenticators bound to it and DX_FALSE otherwise.
        (no use to ask for permission here since if the access will be denied it will meant that the object is bound)

    @param aElementHandle   [in]
        The handle of the element the query relates to.
    @param aAnswer_ptr      [out]
        A pointer to the location where the answer will be put.
 
    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND The handle was not found in the DB
    **/
    SSTAAReturnCodes_t SST_AAElementIsBound(SSTHandle_t    aElementHandle,              
                                            DxBool_t             *aAnswer_ptr);




    /*SST_AAAuthAppIDCreate*/
    /*!
    \brief 
    Creates a ‘Applicatio Id’ type authenticator and stores it in the SST. 
    Further usage of that authenticator is possible through the provided handle.

    @param aSessionId          [in]	
    The authentication session ID, required to verify the user's privileges.
    @param aTransactionId      [in]
    The transaction ID under which this operation will be performed.
    @param aAppID_ptr          [in]
    A pointer to the application ID to be used in this authenticator.
    This pointer can be NULL. 
    When we create special authenticator with privileged mode, this
    pointer will be null
    @param aAppIDSizeInBytes   [in]
    The size of the provided application ID (note that this parameter may not be 0).    
    @param aAuthHandle_ptr    [in/out]
    A pointer to the location from which the handle of the newly created authenticator 
    may be taken from. If the value in that location is SST_HANDLE_RANDOM_GENERATE a 
    new handle will be randomly generated.
    This is also a pointer to the location where the created authenticator’s handle will be put.        
    @param aWorkspace_ptr          [in]
    A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
    The size in bytes of the provided workspace.


    @return SST_RC_OK	                                The operation completed successfully.
    @return SST_RC_FAIL	                                The operation failed.
    @return SST_AA_RC_ERROR_NOT_ACTIVE	                AA has gone through a proper init.
    @return SST_RC_ERROR_FATAL                          The fatal error occurred/wrong workspace    
    @return SST_AA_RC_ERROR_NULL_POINTER	            One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_AUTH_WRONG_TYPE	            authenticator type wrong, can be pwd or appId.
    @return SST_AA_RC_ERROR_SESSION_INVALID	            No session identifier corresponds with the received cookie
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	            The handle given for control authenticator is indeed of an authenticator
    @return SST_AA_RC_ERROR_VCRYS                       VCRYS error
    @return SST_AA_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED  Insufficient space handling, main NVS
    @return SST_AA_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED   Insufficient space handling, transaction NVS
    **/

    SSTAAReturnCodes_t SST_AAAuthAppIDCreate(SSTSessionId_t            aSessionId,                              
                                             SSTTxnId_t                aTransactionId,  
                                             DxByte_t                 *aAppID_ptr,
                                             DxUint32_t                aAppIDSizeInBytes,                                                                              
                                             SSTHandle_t              *aAuthHandle_ptr,
                                             DxByte_t                 *aWorkspace_ptr,
                                             DxUint32_t                aWorkspaceSizeInBytes);

    /*SST_AAAuthPwdCreate*/
    /*!
    \brief 
        Creates a ‘password’ type authenticator and stores it in the SST. 
        Further usage of that authenticator is possible through the provided handle.

        @param aSessionId          [in]	
            The authentication session ID, required to verify the user's privileges.
        @param aTransactionId      [in]
            The transaction ID under which this operation will be performed.
        @param aPwd_ptr            [in]
            A pointer to the password to be used in this authenticator.
            This pointer can be NULL. 
            When we create special authenticator with privileged mode, this
            pointer will be null
        @param aPwdSizeInBytes     [in]
            The size of the provided password (note that this parameter may not be 0).
        @param aMaxTrials          [in]
            The maximal number of trials a user can try to open the authenticator with
            a wrong password. If this number has exceeded this authenticator must be reset.
            SST_UNLIMITED_FAILURE means infinite retires.
        @param aMasterAuthHandle   [in]
            The handle of the authenticator that will be assigned as the master authenticator 
            for the newly created authenticator (will allow access to it).
			It is allowed to pass a SST_CLOSE_AUTH_HANDLE. In this case, there will be no way to reset the
			authenticator so it is advised to also set number of failure retires to inifinite.
        @param aAuthHandle_ptr    [in/out]
            A pointer to the location from which the handle of the newly created authenticator 
            may be taken from. If the value in that location is SST_HANDLE_RANDOM_GENERATE a 
            new handle will be randomly generated.
            This is also a pointer to the location where the created authenticator’s handle will be put.        
        @param aWorkspace_ptr          [in]
            A pointer to the workspace provided.
        @param aWorkspaceSizeInBytes   [in]
            The size in bytes of the provided workspace.
            
   
    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL	                        The operation failed.
    @return SST_AA_RC_ERROR_HANDLE_IN_USE	    This handle is used by another object.
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.

    TODO: remove error from doc and code. @return SST_AA_RC_ERROR_PASSWORD	        The password provided does not comply with the requirements as defined
                                                in sst_aa_config.h
    TODO: ADD TO DOC @return SST_AA_RC_ERROR_HANDLE_MISMATCH	        The handle provided doesn't refer to an authenticator.
                                                in sst_aa_config.h
    TODO: ADD TO DOC @return SST_AA_RC_ERROR_WORKSPACE	        The worlspace provided is to small.
                                                in sst_aa_config.h
    @return SST_AA_RC_ERROR_NULL_POINTER	    One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_TXN_INVALID	        No active transaction corresponds to the received ID, 
                                                        operation terminated.
    @return SST_AA_RC_ERROR_SESSION_INVALID	    No session identifier corresponds with the received cookie
    @return SST_AA_RC_ERROR_HANDLE_OUT_OF_RANGE	The handle that the user requests to assign is not the 
                                                        range allocated for user defined handles.
    **/
    SSTAAReturnCodes_t SST_AAAuthPwdCreate(SSTSessionId_t           aSessionId,                              
                                           SSTTxnId_t               aTransactionId,  
                                           DxByte_t                 *aPwd_ptr,
                                           DxUint32_t               aPwdSizeInBytes,
                                           DxUint32_t               aMaxTrials,
                                           SSTHandle_t			    aMasterAuthHandle,
                                           SSTHandle_t			    *aAuthHandle_ptr,
                                           DxByte_t                 *aWorkspace_ptr,
                                           DxUint32_t               aWorkspaceSizeInBytes);
    
    /*SST_AAAuthPwdResetAndChange*/
    /*!
    \brief 
        Resets a ‘password’ authenticator’s retry counter and change its password.

    @param aSessionId_ptr          [in]
        A pointer to the user’s session cookie.
    @param aAuthHandle             [in]
        The handle of the authenticator which counter should be reset.
    @param aNewPwdsizeInBytes      [in]
        The size of the new password.
    @param aNewPwd_ptr             [in]
        A pointer to the location of the new password.
        This pointer can be NULL. Means that we reset & change special 
        authenticator with privileged mode.
        note: the authenticator state will not be reset
    @param aNewMaxRetry            [in]
        The new maximal retry number (0 for infinity).
    @param aWorkspace_ptr          [in]
        A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
        The size in bytes of the provided workspace.
    
    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_SESSION_INVALID	 No active session corresponds to the received cookie, 
                                                     operation terminated.
    @return SST_AA_RC_ERROR_TXN_INVALID	     No active transaction corresponds to the received ID, 
                                                     operation terminated.
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND The handle was not found in the DB
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	 The authenticator handle refers to an object of different 
                                                     type (not password authenticator)
    @return SST_AA_RC_ERROR_PASSWORD	     The password provided does not comply with the requirements 
                                                     as defined in sst_aa_config.h

    **/
    SSTAAReturnCodes_t SST_AAAuthPwdResetAndChange(SSTTxnId_t            aTransactionId,  
                                                   SSTSessionId_t        aSessionId,                     
                                                   SSTHandle_t			 aAuthHandle,
                                                   DxUint32_t            aNewPwdsizeInBytes,                     
                                                   DxByte_t             *aNewPwd_ptr,
                                                   DxUint32_t            aNewMaxRetry,
                                                   DxByte_t             *aWorkspace_ptr,
                                                   DxUint32_t            aWorkspaceSizeInBytes);
    
    /*SST_AAAuthPubKeyRSACreate*/
    /*!
    \brief 
        Creates a RSA Public Key type authenticator and stores it in the SST. 
        Further usage of that authenticator is possible through the provided handle.
    
    @param aTransactionId         [in]
        The transaction ID under which this operation will be performed.
    @param aN_ptr                 [in]
        A pointer to the RSA key modulus.
    @param aE_ptr                 [in]
        A pointer to the RSA public key exponent.
    @param aESizeInBytes          [in]
        The size of the RSA public key exponent.
    @param aAuthHandle_ptr        [in/out]] 
        A pointer to the location from which the handle of the newly created authenticator 
        may be taken from. If the value in that location is SST_HANDLE_RANDOM_GENERATE a new 
        handle will be randomly generated.  This is also a pointer to the location where the 
        created authenticator’s handle will be put.
    @param aWorkspace_ptr          [in]
        A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
        The size in bytes of the provided workspace.
        

    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL	                        The operation failed.
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.
    @return SST_AA_RC_ERROR_HANDLE_IN_USE	    This handle is used by another object.
    @return SST_AA_RC_ERROR_TXN_INVALID	        No active transaction corresponds to the received ID, 
                                                        operation terminated.
    @return SST_AA_RC_ERROR_NULL_POINTER	    One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_OUT_OF_RANGE	The handle that the user requests to assign is not 
                                                        the range allocated for user defined handles.
    **/
    SSTAAReturnCodes_t SST_AAAuthPubKeyRSACreate(SSTTxnId_t            aTransactionId,  
												 DxByte_t             *aN_ptr,
												 DxByte_t             *aE_ptr,
												 DxUint32_t            aESize,                       
												 SSTHandle_t		  *aAuthHandle_ptr,
												 DxByte_t             *aWorkspace_ptr,
												 DxUint32_t            aWorkspaceSizeInBytes); 
                                              
        
	
	
	/*SST_AAAuthPubKeyECCCreate*/
    /*!
    \brief 
        Creates an ECC Public Key type authenticator and stores it in the SST. 
        Further usage of that authenticator is possible through the provided handle.
    
    @param aTransactionId         [in]
        The transaction ID under which this operation will be performed.
    @param aECCPubKey_ptr         [in]
        A pointer to the ECC public key.
    @param aECCPubKeySizeInBytes  [in]
        The size of the ECC public key.
    @param aAuthHandle_ptr        [in/out]] 
        A pointer to the location from which the handle of the newly created authenticator 
        may be taken from. If the value in that location is SST_HANDLE_RANDOM_GENERATE a new 
        handle will be randomly generated.  This is also a pointer to the location where the 
        created authenticator’s handle will be put.
    @param aWorkspace_ptr          [in]
        A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
        The size in bytes of the provided workspace.
        

    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL	                        The operation failed.
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.
    @return SST_AA_RC_ERROR_HANDLE_IN_USE	    This handle is used by another object.
    @return SST_AA_RC_ERROR_TXN_INVALID	        No active transaction corresponds to the received ID, 
                                                        operation terminated.
    @return SST_AA_RC_ERROR_NULL_POINTER	    One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_OUT_OF_RANGE	The handle that the user requests to assign is not 
                                                        the range allocated for user defined handles.
    **/
    SSTAAReturnCodes_t SST_AAAuthPubKeyECCCreate(SSTTxnId_t            aTransactionId,  
												 DxByte_t             *aECCPubKey_ptr,
												 DxUint32_t            aECCPubKeySizeInBytes,                       
												 SSTHandle_t		  *aAuthHandle_ptr,
												 DxByte_t             *aWorkspace_ptr,
												 DxUint32_t            aWorkspaceSizeInBytes); 




    /*SST_AAAuthAESSharedSecretCreate*/
    /*!
    \brief 
        Creates a ‘Shared Secret’ type authenticator and stores it in the SST. 
        Further usage of that authenticator is possible through the provided handle.

    @param aTransactionId           [in]
        The transaction ID under which this operation will be performed.
    @param aSecret_ptr              [in]
        A pointer to the secret to be used in this authenticator. 
    @param aKeyType                 [in]
        A key size type (may be 128, 192 or 256 bits)
    @param aAuthHandle_ptr          [in/out]
        A pointer to the location from which the handle of the newly created authenticator may 
        be taken from. If the value in that location is SST_HANDLE_RANDOM_GENERATE a new handle 
        will be randomly generated.
        This is also a pointer to the location where the created authenticator’s handle will be put.
    @param aWorkspace_ptr          [in]
        A pointer to the workspace provided.
    @param aWorkspaceSizeInBytes   [in]
        The size in bytes of the provided workspace.
     
    @return SST_RC_OK	                        The operation completed successfully.
    @return SST_RC_FAIL	                        The operation failed.
    @return SST_AA_RC_ERROR_HANDLE_IN_USE	    This handle is used by another object.
    @return SST_RC_ERROR_FATAL                  The fatal error occurred.
    @return SST_AA_RC_ERROR_TXN_INVALID	        No active transaction corresponds to the received ID, operation terminated.
    @return SST_AA_RC_ERROR_NULL_POINTER	    One of the received pointers was NULL, operation terminated
    @return SST_AA_RC_ERROR_HANDLE_OUT_OF_RANGE	The handle that the user requests to assign is not the range allocated for user defined handles.

    **/
    SSTAAReturnCodes_t SST_AAAuthAESSharedSecretCreate(SSTTxnId_t            aTransactionId,   
                                                       DxByte_t             *aSecret_ptr,
                                                       SSTAESKeyType_t       aKeyType,
                                                       SSTHandle_t			*aAuthHandle_ptr,
                                                       DxByte_t             *aWorkspace_ptr,
                                                       DxUint32_t            aWorkspaceSizeInBytes);

    /*SST_AAAuthDelete*/
    /*!
    \brief 
        Deletes an unbound authenticator from the SST DB.

    @param aTransactionId  [in]
        The transaction ID under which this operation will be performed.
    @param aAuthHandle     [in]
        The handle of the authenticator to delete.

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.    
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointer was NULL, operation terminated
    @return SST_AA_RC_ERROR_SESSION_INVALID	 No active session corresponds to the received cookie, 
                                                     operation terminated.
    @return SST_AA_RC_ERROR_TXN_INVALID	     No active transaction corresponds to the received ID, 
                                                     operation terminated.
    @return SST_AA_RC_ERROR_HANDLE_NOT_FOUND The handle was not found 
    @return SST_AA_RC_ERROR_HANDLE_MISMATCH	 The authenticator handle refers to an object of different 
                                                     type (not authentication)
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The user does not have an adequate permission to perform 
                                                     this operation
    **/
    SSTAAReturnCodes_t SST_AAAuthDelete(SSTTxnId_t    aTransactionId,   
                                        SSTHandle_t   aAuthHandle);

    /*SST_AAMetaDataHandleGet*/
    /*!
    \brief 
        Returns the requested element’s handle if access with the requested permission is allowed according 
        to the given session ID. (if the return value is not OK, the output should be ignored) 

    @param aSessionId           [in]
        The session ID.
    @param aRequestedPermission [in]
        An enum value that states the requested access level to the SST element.
    @param aElementHandle_ptr   [out]
        A pointer to the location in which the handle of the element will be put (relevant only if the return value is OK).
    @param aAAMetaDataBuffer    [in]
        A buffer that consists of the AA'a meta data.

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated.
    @return SST_AA_RC_ERROR_SESSION_INVALID	 There is no session related to that cookie, operation terminated.
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The session cookie cannot provide with the required privileges.
    @return SST_AA_RC_ERROR_HANDLE_MISSMATCH The handle provided is not of a meta-data object.

    **/
    SSTAAReturnCodes_t SST_AAMetaDataHandleGet(SSTSessionId_t              aSessionId,
                                               SSTPermissions_t            aRequestedPermission,
                                               SSTHandle_t				*aElementHandle_ptr,
                                               SSTAAMetaDataPlaceHolder_t *aAAMetaDataBuffer);
    /*SST_AAMetaDataHandleSet*/
    /*!
    \brief 
        Sets the requested element’s handle to the provided one (if allowed according 
        to the given session ID). (if the return value is not OK, the output should be ignored) 

    @param aSessionId           [in]
        The session ID.
    @param aElementHandle       [in]
        The handle to be set as the new element handle in the meta data.
    @param aAAMetaDataBuffer    [in]
        A buffer that consists of the AA'a meta data.

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated.
    @return SST_AA_RC_ERROR_SESSION_INVALID	 There is no session related to that cookie, operation terminated.
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The session cookie cannot provide with the required privileges.
    @return SST_AA_RC_ERROR_HANDLE_MISSMATCH The handle provided is not of a meta-data object.

    **/
    SSTAAReturnCodes_t SST_AAMetaDataHandleSet(SSTSessionId_t               aSessionId,
                                               SSTHandle_t					aElementHandle,
                                               SSTAAMetaDataPlaceHolder_t  *aAAMetaDataBuffer);
                                                             
                                                 
    /*SST_AAReadOnlySet*/
    /*!
    \brief 
    Given pointer to an AA meta data buffer and a session ID this function sets the element to
    which the meta data point to be read only. all current auth's that can authorize any kind
    of access to it will be able to authorize only read only access from now on.
    SST_PERMISSION_CONTROL permission is necessary to complete the operation.

    @param aSessionId           [in]
        The session ID.
    @param aAAMetaDataBuffer    [in]
        A buffer that consists of the AA'a meta data.

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated.
    @return SST_AA_RC_ERROR_SESSION_INVALID	 There is no session related to that cookie, operation terminated.
    @return SST_AA_RC_ERROR_HANDLE_MISSMATCH The handle provided is not of a meta-data object.
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The session cookie cannot provide with the required privileges.

    **/
    SSTAAReturnCodes_t SST_AAReadOnlySet(SSTAAMetaDataPlaceHolder_t *aAAMetaDataBuffer,
                                         SSTSessionId_t              aSessionId);  
                                         
    /*SST_AAAuthAllUnbind*/
    /*!
    \brief 
    Given pointer to an AA meta data buffer, a transaction ID and a session ID this function unbinds
    all of the authenticators bound to the object.
    SST_PERMISSION_CONTROL permission is necessary to complete the operation.

    @param aTransactionId  [in]
        The transaction ID under which this operation will be performed.
    @param aSessionId           [in]
        The session ID.
    @param aAAMetaDataBuffer    [in]
        A buffer that consists of the AA'a meta data.
    

    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.
    @return SST_AA_RC_ERROR_NULL_POINTER	 One of the received pointers was NULL, operation terminated.
    @return SST_AA_RC_ERROR_SESSION_INVALID	 There is no session related to that cookie, operation terminated.
    @return SST_AA_RC_ERROR_HANDLE_MISSMATCH The handle provided is not of a meta-data object.
    @return SST_AA_RC_ERROR_ACCESS_DENIED	 The session cookie cannot provide with the required privileges.
	 @return SST_AA_RC_ERROR_ZOMBIE	         No authenticator can provide read permission to that object (zombie alert).

    **/
    SSTAAReturnCodes_t SST_AAAuthAllUnbind(SSTTxnId_t                  aTransactionId,
                                           SSTSessionId_t              aSessionId,
                                           SSTAAMetaDataPlaceHolder_t *aAAMetaDataBuffer); 


    /*SST_AAAuthModeModify*/
    /*!
    \brief 
    Change an authenticator mode.
    The new mode can be with equal or more permission than the current
    authenticator mode.

    @param aTransactionId       [in]
    The transaction ID under which this operation will be performed.
    
    @param aAuthHandle                  [in]
    The authenticator handle.
    
    @param aNewMode                     [in]
    New mode for the authenticator

    @param aWorkspace_ptr               [in]
    A pointer to the workspace buffer to be used by the SST. If the buffer is 
    not word aligned SST will check if there is sufficient size after aligning. 
    if not an error will be returned

    @param aWorkspaceSizeInBytes        [in]
    The size in bytes of the workspace buffer to be used by the SST.


    @return SST_RC_OK	                     The operation completed successfully.
    @return SST_RC_FAIL	                     The operation failed.
    @return SST_RC_ERROR_FATAL               The fatal error occurred.
    @return SST_AA_RC_ERROR_MODE_ILLEGAL	 Mode illegal, with wrong permission.
    @return SST_AA_RC_ERROR_VDB	             VDB function error.
    
    **/
    SSTAAReturnCodes_t SST_AAAuthModeModify (SSTTxnId_t          aTransactionId,
                                             SSTHandle_t         aAuthHandle,
                                             SSTMode_t           aNewMode,
                                             DxByte_t           *aWorkspace_ptr,
                                             DxUint32_t          aWorkspaceSizeInBytes);

                                                  
#endif  /* _DX_SST_API_H_ */
