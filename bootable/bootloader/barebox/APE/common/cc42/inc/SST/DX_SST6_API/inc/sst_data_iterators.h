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
* Title:						Discretix SST API Data Iterators header file						 					
*																			
* Filename:					    sst_data_iterators.h 															
*																			
* Project, Target, subsystem:	SST 6.0,API, Data Iterators
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_API_DI_H_
    #define _DX_SST_API_DI_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 	
    //#include "sst_ix.h"
	
    /*----------- Global Includes ------------------------------------------------*/
    #include "sst_types.h" 	
    
	/*----------- Global defines -------------------------------------------------*/

	/*----------- Global type definitions ----------------------------------------*/
       
    /*SST_IndexLookupDelete*/
    /*!
    \brief         Delete all index lookup objects which are contain a specific string.

    @param aTransactionId             [in]
    The user's transaction ID.
    @param aSessionId                 [in]
    The user's session ID (reserved for a future using)
    @param aStringSizeInBytes         [in]
    The size of the string
    @param aString                    [in]
    The pointer to the string. 
    @param aWorkspace_ptr             [in]
	The scratch buffer pointer   If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    @param aWorkspaceSizeInBytes      [in]
    The scratch buffer pointer size 

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_IX                 General Index lookup error (For example: any DB error, internal index lookup errors and incompatibility problems)
    @return SST_RC_ERROR_NO_MORE_HANDLES    There are no more handles in data base.
    @return SST_RC_ERROR_TXN_INVALID        The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_HANDLE_NOT_FOUND   A handle was not found.
    @return SST_RC_ERROR_VCRYS              An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
	@return SST_RC_ERROR_SESSION_INVALID	Invalid session
    **/
    DxError_t SST_IndexLookupDelete (SSTTxnId_t     aTransactionId,
                                     SSTSessionId_t aSessionId,
                                     DxUint32_t     aStringSizeInBytes,
                                     DxChar_t      *aString,
                                     DxByte_t      *aWorkspace_ptr,
                                     DxUint32_t     aWorkspaceSizeInBytes);

    /*SST_IndexLookupBind*/
    /*!
    \brief       Bind an SST handle to a string in the context of the relevant lookup.

    @param aTransactionId [in]
    The user's transaction ID.
    @param aSessionId [in]
    The user's session ID.
    @param aHandle [in]
    The handle to bind to the string.
    @param aStringSize [in]
    The size of the string to be bound to the handle (in bytes).
    @param aString [in]
    The string to be bound to the handle. 
    @param aWorkspace_ptr[in]
	The scratch buffer. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    @param aWorkspaceSizeInBytes [in]
    The scratch buffer size.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_IX                 General Index lookup error (For example: any DB error, internal index lookup errors and incompatibility problems)
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type  or an invalid handle
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
	@return SST_RC_ERROR_DUPLICATE_STRING   The index is already bound to the object. A duplicate String.
	@return SST_RC_ERROR_NO_MORE_HANDLES    No more handles error.
    **/
    DxError_t SST_IndexLookupBind( SSTTxnId_t      aTransactionId,
                                   SSTSessionId_t  aSessionId,
                                   SSTHandle_t     aHandle,
                                   DxUint32_t      aStringSize,
                                   DxChar_t       *aString,
                                   DxByte_t       *aWorkspace_ptr,
                                   DxUint32_t      aWorkspaceSizeInBytes);

    /*SST_IndexLookupUnbind*/
    /*!
    \brief         Unbind an SST handle from a string in the context of the relevant lookup.

    @param aTransactionId [in]
    The user's transaction ID.
    @param aSessionId [in]
    The user's session ID.
    @param aLookupHandle [in]
    The handle of the lookup under which the reference is be kept.
    @param aHandle [in]
    The handle to unbind from the string.
    @param aStringSize [in]
    The size of the string to be unbound from the handle.
    @param aString [in]
    The string to be unbound from the handle. 
    @param aWorkspace_ptr[in]
	The scratch buffer. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    @param aWorkspaceSizeInBytes [in]
    The scratch buffer size.

    @return SST_RC_OK	                            The operation completed successfully.
    @return SST_RC_FAIL	                            The operation failed.
    @return SST_RC_ERROR_CLOSED                     The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_LOOKUP_HANDLE_NOT_BOUND    User tried to unbind string from object which was not bound to this string.  
    @return SST_RC_ERROR_IX                         General Index lookup error (For example: any DB error, internal index lookup errors and incompatibility problems)
    @return SST_RC_ERROR_TXN_INVALID	            The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	        The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	            The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	        A handle was not found.
    @return SST_RC_ERROR_HANDLE_MISMATCH	        The authenticator handle refers to an object of different type  or an invalid handle
    @return SST_RC_ERROR_NULL_POINTER	            At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	            An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	                The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE					Insufficient workspace given by user.
    **/
    DxError_t SST_IndexLookupUnbind( SSTTxnId_t      aTransactionId,
                                     SSTSessionId_t  aSessionId,
                                     SSTHandle_t     aHandle,
                                     DxUint32_t      aStringSize,  
                                     DxChar_t       *aString,
                                     DxByte_t       *aWorkspace_ptr,
                                     DxUint32_t      aWorkspaceSizeInBytes);

    /*SST_IndexLookUpBegin*/
    /*!
    \brief         Starts an index lookup 

    @param aString_ptr [in]
    Pointer to the string
    @param aStringLengthInBytes [in]
    Size of string in bytes 
    @param aIterCookie_ptr [out]
    The pointer to the Index lookup iterator

    @return SST_RC_OK                       The iterator create operation was successful 
    @return SST_RC_FAIL                     The iterator create operation failed
    @return SST_RC_ERROR_IX                 General Index lookup error (For example: any DB error, internal index lookup errors and incompatibility problems)
    @return SST_RC_ERROR_VDB                DB operation failed
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
	@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
    */
    DxError_t SST_IndexLookUpBegin (DxChar_t             *aString_ptr,
                                    DxUint32_t            aStringLengthInBytes, 
                                    SSTIXCookie_t        *aIterCookie_ptr);


    /*SST_IndexLookUpHandlesGet*/
    /*!
    \brief    Find all the handles that are bound to a certain iterator handle and are allowed by the session.

    aSessionId [in]
    The user's session ID.
    aIterCookie_ptr [in/out]
    The iterator handle.
    aMaxHandlesToRead_ptr [in/out]
    The maximal number of handles to be read to the buffer. 
    The actual number of handles stored in the buffer.
    aHandlesBuff [out]
    The buffer of found handles.
    aHandlesLeft_ptr [out]
    The number of handles left bound to the specified string.
    @param aWorkspace_ptr[in]
	The scratch buffer. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
    @param aWorkspaceSizeInBytes [in]
    The scratch buffer size.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_NO_MORE_HANDLES    There are no more handles in data base. 
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_IX                 General Index lookup error (For example: any DB error, internal index lookup errors and incompatibility problems)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	The lookup handle was not found.
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_IndexLookUpHandlesGet(SSTSessionId_t       aSessionId,
                                        SSTIXCookie_t       *aIterCookie_ptr,
                                        SSTHandle_t         *aHandlesBuff,
                                        DxUint32_t          *aNumberOfHandlesToRead_ptr,
                                        DxUint32_t          *aHandlesLeft_ptr,
                                        DxByte_t            *aWorkspace_ptr,
                                        DxUint32_t           aWorkspaceSizeInBytes);

    /*SST_IndexLookUpEnd*/
    /*!
    \brief    Finished the lookup.

    aIterCookie_ptr [in]
    The lookup iterator handle.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_IX                 General Index lookup error (For example: any DB error, internal index lookup errors and incompatibility problems)
	@return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
	@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.

    */

     DxError_t  SST_IndexLookUpEnd (SSTIXCookie_t        *aIterCookie_ptr);

    /************************************************************************/
    /*                         TYPE ITERATOR                                */
    /************************************************************************/
    /*!	\brief	TYPE ITERTOR MECHANISM
		The Type iterator mechanism allows the user to retrieve handles of  
		data objects with the use of a predefined type. 
		Any changes performed in the SST between the SST_IteratorBegin to
		SST_IteratorTerminate will be part of the iteration (changes like 
		data insert, delete), and will influence the result handle.
	**/
	 
	 
	 /*SST_TypeIteratorBegin*/
    /*!
    \brief   Begin a type iteration, the returned iterator handle should be used for 
             future reference for that iterator.

    @param aSessionId [in]
    The user's session ID.
    @param aDataType [in]
    The type of the SST element to iterate.
    @param aIteratorCookie_ptr [in/out]
    A pointer to the location of the returned iterator handle, using this handle the user can
    access the desired iterator.
	@param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.
    
    @return SST_RC_OK							The operation completed successfully.
    @return SST_RC_FAIL							The operation failed.
    @return SST_RC_ERROR_CLOSED                 The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_NULL_POINTER			At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE				insufficient workspace size
    @return SST_RC_ERROR_SESSION_INVALID		The received session ID is not valid. (probably this is not an active session)
    **/
    DxError_t SST_TypeIteratorBegin( SSTSessionId_t       aSessionId,
									 SSTDataType_t        aDataType,
									 SSTIteratorCookie_t *aIteratorCookie_ptr,
									 DxByte_t			 *aWorkspace_ptr,
									 DxUint32_t			  aWorkspaceSizeInBytes);

    
    /*SST_TypeIteratorTerminate*/
    /*!
    \brief       Terminate a type iteration.

    @param aSessionId [in]
    The user's session ID.
    @param aIteratorCookie_ptr [in/out]
	The handle of the iteration to terminate.
	@param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.
    
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
	@return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE			insufficient workspace size
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ITER_INVALID		The received iteration handle is not valid.
    **/
    DxError_t SST_TypeIteratorTerminate(SSTSessionId_t      aSessionId,
										SSTIteratorCookie_t *aIteratorCookie_ptr,
										DxByte_t			*aWorkspace_ptr,
										DxUint32_t			aWorkspaceSizeInBytes);

    
    
    /*SST_TypeIteratorHandleIncAndGet*/
    /*!
    \brief Increment the type iterator and retrieve the next handle.

    @param aSessionId [in]
    The user's session ID.
    @param aIteratorCookie_ptr [in/out]
    The iteration handle. iteration can cause the handle to change
    @param aHandle_ptr [out]
    The returned handle.
	@param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.
    
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_ITER_END_OF_TYPE	There are no more handles from the specified type to retrieve.
	@return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE			insufficient workspace size
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ITER_INVALID		The received iteration handle is not valid.
    **/
    DxError_t SST_TypeIteratorHandleIncAndGet( SSTSessionId_t		 aSessionId,
											   SSTIteratorCookie_t  *aIteratorCookie_ptr,
											   SSTHandle_t			*aHandle_ptr,
											   DxByte_t				*aWorkspace_ptr,
											   DxUint32_t			 aWorkspaceSizeInBytes);

    
    
    /*SST_TypeIteratorHandleDecAndGet*/
    /*!
    \brief        Decrement the type iterator and retrieve the previous handle.

    @param aSessionId [in]
    The user's session ID.
    @param aIteratorCookie_ptr [in/out]
    The iteration handle. iteration can cause the handle to change
    @param aHandle_ptr [out]
    The returned handle.
	@param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_ITER_END_OF_TYPE	There are no more handles from the specified type to retrieve.
	@return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE			insufficient workspace size
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ITER_INVALID		The received iteration handle is not valid.
    **/
    DxError_t SST_TypeIteratorHandleDecAndGet( SSTSessionId_t		 aSessionId,
											   SSTIteratorCookie_t  *aIteratorCookie_ptr,
											   SSTHandle_t			*aHandle_ptr,
											   DxByte_t				*aWorkspace_ptr,
											   DxUint32_t			 aWorkspaceSizeInBytes);

    
    /*SST_TypeIteratorCurrentGet*/
    /*!
    \brief    Retrieve the current handle in the type iterator.

    @param aSessionId [in]
    The user's session ID.
    @param aIteratorCookie_ptr [in/out]
    The iteration handle. iteration can cause the handle to change
    @param aHandle_ptr [out]
    A pointer to the location where the handle will be returned.
	@param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_ITER_END_OF_TYPE	There are no more handles from the specified type to retrieve.
	@return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE			insufficient workspace size
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ITER_INVALID		The received iteration handle is not valid.
    **/
    DxError_t SST_TypeIteratorCurrentGet(SSTSessionId_t			aSessionId,
										 SSTIteratorCookie_t	*aIteratorCookie_ptr,
										 SSTHandle_t			*aHandle_ptr,
										 DxByte_t				*aWorkspace_ptr,
										 DxUint32_t				aWorkspaceSizeInBytes);

    
    /*SST_TypeIteratorHandlesGet*/
    /*!
    \brief		Retrieve all relevant handles (up to a user defined maximal amount).
				Starting from the next handle.	

    @param aSessionId [in]
    The user's session ID.
    @param aIteratorCookie_ptr [in/out] 
    The iteration handle. iteration can cause the handle to change
    @param aHandlesRead_ptr [in/out]
    The maximal numbers of handles to read.
    The number of handles that were actually read..
    @param aHandleBuff_ptr [out]
    A pointer to the buffer where the returned handle will be stored.
    @param aHandlesLeft_ptr [out]
    A pointer to a location where the MAX number of the handles that are left, will be stored.
	if DX_NULL is used in this field, the MAX number of handles left will not returned
	@param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_ITER_END_OF_TYPE	There are no more handles from the specified type to retrieve.
	@return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE			insufficient workspace size
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ITER_INVALID		The received iteration handle is not valid.
  
	@notes: 1. the handles returned are from the next available handle in the iterator
    **/
    DxError_t SST_TypeIteratorHandlesGet(SSTSessionId_t       aSessionId,
										 SSTIteratorCookie_t *aIteratorCookie_ptr,
										 DxUint32_t          *aHandlesRead_ptr,                                                
										 SSTHandle_t         *aHandleBuff_ptr,
										 DxUint32_t          *aHandlesLeft_ptr,
										 DxByte_t			 *aWorkspace_ptr,
										 DxUint32_t			 aWorkspaceSizeInBytes);

#ifdef __cplusplus
}
#endif
#endif  /* _DX_SST_API_DI_H_ */
