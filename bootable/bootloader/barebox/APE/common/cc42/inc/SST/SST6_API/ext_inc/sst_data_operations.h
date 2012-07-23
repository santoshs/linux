/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
 *   protected by copyright, international treaties and various patents.   *
 *   Any copy or reproduction of this Software as permitted below, must    *
 *   include this Copyright Notice as well as any other notices provided   *
 *   under such license.                                                   *
 *                                                                         *
 *   This program shall be governed by, and may be used and redistributed  *
 *   under the terms and conditions of the GNU Lesser General Public       *
 *   License, version 2.1, as published by the Free Software Foundation.   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY liability and WARRANTY; without even the implied      *
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, please write to the          *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST API Data Operations header file						 					
*																			
* Filename:					    sst_data_operations.h 														
*																			
* Project, Target, subsystem:	SST 6.0, API, Data Operations
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_API_DO_H_
    #define _DX_SST_API_DO_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 	
	
    /*----------- Global Includes ------------------------------------------------*/
    #include "sst_types.h" 	    

	
	/*----------- Global type definitions-----------------------------------------*/

     
	/*----------- Global defines -------------------------------------------------*/
    /*! \brief Handle value to be passed when random generation is required **/
    #define SST_HANDLE_RANDOM_GENERATE		((SSTObjectId_t)0xFFFFFFFF)
	#define SST_HANDLE_DIGEST_DEFAULT		((SSTObjectDigest_t)0xDBDBDBDB)
    
    /*Basic DB services*/
    
    /*SST_DataInsert*/
    /*!
    \brief Insert a data element to the SST. 
	    
    @param aTransactionId       [in]
    The user's transaction ID.
    
    @param aSessionId           [in]
    The user's session ID.
    
    @param aInitialCtrlAuth     [in]
    The initial control authenticator for the newly created data object. A user that can open that authenticator has full access to the newly created object.
    This authenticator may be replaced later on.
    
    @param aDataIn_ptr          [in]
    A pointer to the data to be inserted to the SST. 
    
    @param aDataInsizeInBytes   [in]
    The size of the data to be inserted to the SST.
    
    @param aEncrypt             [in]
    A Boolean that states whether or not the data should be encrypted in the SST.
    
    @param aDataType            [in]
    The type of the data object.
    
    @param aDataHandle_ptr      [in/out]
    The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
    This parameter also contains the location to which the handle of the created object will be put. 
    
    @param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes    [in]
    The size in bytes of the workspace buffer to be used by the SST.


    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
    @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
    @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.

	@notes	This operation Must be performed inside a transaction.
			Note that transaction does not protect from performing concurrent operations. 
			It is the user responsibility not to perform simultaneity more than one operation 
			to a particular record. 
    **/
    DxError_t SST_DataInsert(SSTTxnId_t        aTransactionId,
                             SSTSessionId_t    aSessionId, 
                             SSTHandle_t       aInitialCtrlAuth,       
                             DxByte_t         *aDataIn_ptr,
                             DxUint32_t        aDataInsizeInBytes,
                             DxBool_t          aEncrypt,
                             SSTDataType_t     aDataType,                
                             SSTHandle_t      *aDataHandle_ptr,
                             DxByte_t         *aWorkspace_ptr,
                             DxUint32_t        aWorkspaceSizeInBytes);

    
    /*SST_DataReadOnlySet*/
    /*!
    \brief         Set an object in the SST to be read only. Use with caution.

    @param aTransactionId           [in] 
    The user's transaction ID.
    
    @param aSessionId               [in]
    The user's session ID.
    
    @param aObjHandle               [in]
    The handle of the object the users wishes to make read-only.
    
    @param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes    [in]
    The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_DataReadOnlySet(SSTTxnId_t      aTransactionId,
                                  SSTSessionId_t  aSessionId, 
                                  SSTHandle_t     aObjHandle,
                                  DxByte_t       *aWorkspace_ptr,
                                  DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_DataRead*/
    /*!
    \brief  Read data from a data object. 
            Allows the user to read any legal amount of data from any legal offset in the desired object.
			    
    @param  aSessionId          [in]
    The user's session ID.
    
    @param aDataHandle          [in] 
    The handle of the data object to be read.
    
    @param aDataOffset          [in] 
    The offset within the data object from which the read will begin. 
    
    @param aDataOut_ptr         [out] 
    A pointer to the location where the requested data will be put. 

    @param aDataSizeInBytes_ptr [in\out]
    A pointer to the maximal amount of bytes to read, updated to the number of bytes retrieved.        

    @param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes[in]
    The size in bytes of the workspace buffer to be used by the SST.
    
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
    @return SST_RC_ERROR_DATA_OFFSET	    Illegal data offset. (the requested start point exceeds the object size)
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS               An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.

	@notes	Note that there is no protection from performing concurrent operations. 
			It is the user responsibility not to perform simultaneity more than one operation 
			to a particular record. 
    **/
    DxError_t SST_DataRead(SSTSessionId_t  aSessionId,
                           SSTHandle_t     aDataHandle,
                           DxUint32_t      aDataOffset,
                           DxByte_t       *aDataOut_ptr,
                           DxUint32_t     *aDataSizeInBytes_ptr,
                           DxByte_t       *aWorkspace_ptr,
                           DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_DataModify*/
    /*!
    \brief         Modify a data object. 
                   Allows the user to modify any legal amount of data from any legal offset in the desired object.
				  
    @param aTransactionId       [in] 
    The user's transaction ID.
    
    @param aSessionId           [in]
    The user's session ID.
    
    @param aDataHandle          [in]
    The handle of the object the user wishes to modify.
    
    @param aDataIn_ptr          [in]
    A pointer to the new data to be inserted to the object. 
    
    @param aDataSizeInBytes     [in]
    The amount of data to be modified.
    
    @param aDataOffset          [in]
    The offset from the beginning of the object from which the modification will begin.
    
    @param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes[in]
    The size in bytes of the workspace buffer to be used by the SST.
    
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_DATA_OFFSET	    Illegal data offset.
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
		
	@notes	This operation Must be performed inside a transaction.
			Note that transaction does not protect from performing concurrent operations. 
			It is the user responsibility not to perform simultaneity more than one operation 
			to a particular record.
    **/
    DxError_t SST_DataModify(SSTTxnId_t      aTransactionId,
                             SSTSessionId_t  aSessionId,                           
                             SSTHandle_t     aDataHandle,   
                             DxByte_t       *aDataIn_ptr,
                             DxUint32_t      aDataSizeInBytes,
                             DxUint32_t      aDataOffset,
                             DxByte_t       *aWorkspace_ptr,
                             DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_DataSizeGet*/
    /*!
    \brief Retrieve the size of an object in the SST.

    @param aSessionId               [in]
    The user's session ID.
    
    @param aDataHandle              [in]
    The handle of the object which size its queried.

    @param  aDatasizeInBytes_ptr    [out]
    A pointer to the location where the object size will be stored.
    
    @param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes    [in]
    The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
    **/
    DxError_t SST_DataSizeGet(SSTSessionId_t  aSessionId,  
                              SSTHandle_t     aDataHandle,
                              DxUint32_t     *aDatasizeInBytes_ptr,
                              DxByte_t       *aWorkspace_ptr,
                              DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_DataDelete*/
    /*!
    \brief      Delete a data object. This API removes a data object from the SST DB ? Use with caution.
				 
    @param aTransactionId [in]
    The user's transaction.
    
    @param aSessionId [in]
    The user's session ID 
    
    @param aDataHandle [in]
    The handle of the object which deletion is requested.
    
    @param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes    [in]
    The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type  or an invalid handle
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
	
	@notes	This operation Must be performed inside a transaction.
			Note that transaction does not protect from performing concurrent operations. 
			It is the user responsibility not to perform simultaneity more than one operation 
			to a particular record.
    **/
    DxError_t SST_DataDelete(SSTTxnId_t      aTransactionId,
                             SSTSessionId_t  aSessionId,  
                             SSTHandle_t     aDataHandle,
                             DxByte_t       *aWorkspace_ptr,
                             DxUint32_t      aWorkspaceSizeInBytes);

    
    /*SST_TransactionStart*/
    /*!
    \brief        Start an SST transaction and return the new transaction's ID. 

    @param aTransactionId_ptr [out]
    A pointer to the location where the transaction ID will be stored.
    
    @return SST_RC_OK	                The operation completed successfully.
    @return SST_RC_FAIL	                The operation failed.
    @return SST_RC_ERROR_CLOSED         The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL          The fatal error occurred.    
    @return SST_RC_ERROR_TIMEOUT	    Timeout occurred while waiting for a resource to be available. 
    @return SST_RC_ERROR_VOS	        An error with one of the VOS related services has occurred 
    @return SST_RC_ERROR_VCRYS           An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_NULL_POINTER	At least one of the past arguments is null.
	@return SST_RC_ERROR_TRANSACTION_NOT_ENDED	Transaction has not been committed.
	**/
    DxError_t SST_TransactionStart(SSTTxnId_t *aTransactionId_ptr);
    
    
    /*SST_TransactionEnd*/
    /*!
    \brief         End an SST transaction.

    @param aTransactionId [in]
    The ID of the transaction to be ended.
	@param aIsReflashSensitive [in]
	Flag indicating if the transaction should be reflash sensitive
	@param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                The operation completed successfully.
    @return SST_RC_FAIL	                The operation failed.
    @return SST_RC_ERROR_CLOSED         The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL          The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_VOS	        An error with one of the VOS related services has occurred 
    @return SST_RC_ERROR_VCRYS           An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_NULL_POINTER	At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE	    Insufficient workspace given by user.
	@return SST_RC_ERROR_NULL_POINTER   Null pointer received as input.
    **/
	DxError_t SST_TransactionEnd(SSTTxnId_t aTransactionId,
								 DxBool_t	aIsReflashSensitive,
								 DxByte_t   *aWorkspace_ptr,
								 DxUint32_t aWorkspaceSizeInBytes);
                              									     

    /*SST_TransactionRollback*/
    /*!
    \brief         Rollback an SST transaction.

    @param aTransactionId [in]
    The ID of the transaction to rollback.
	@param aWorkspace_ptr           [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned
	@param aWorkspaceSizeInBytes    [in]
	The size in bytes of the workspace buffer to be used by the SST.

    @return SST_RC_OK	                The operation completed successfully.
    @return SST_RC_FAIL	                The operation failed.
    @return SST_RC_ERROR_CLOSED         The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL          The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_VOS	        An error with one of the VOS related services has occurred 
    @return SST_RC_ERROR_VCRYS           An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_NULL_POINTER	At least one of the past arguments is null.
	@return SST_RC_ERROR_WORKSPACE	    Insufficient workspace given by user.
    **/
	DxError_t SST_TransactionRollback(SSTTxnId_t aTransactionId,
									 DxByte_t   *aWorkspace_ptr,
									 DxUint32_t aWorkspaceSizeInBytes);






    /*SST_DataReplace*/
    /*!
    \brief         Replace a data object. 
                   Allows the user to replace the data of the desired object.
				  
    @param aTransactionId       [in] 
    The user's transaction ID.
    
    @param aSessionId           [in]
    The user's session ID.
    
    @param aDataHandle          [in]
    The handle of the object the user wishes to modify.
    
    @param aNewData_ptr          [in]
    A pointer to the new data to be inserted to the object. 
    
    @param aNewDataSizeBytes     [in]
    The amount of data to be modified.
    
    @param aEncrypt             [in]
    A Boolean that states whether or not the data should be encrypted in the SST.
    
    @param aWorkspace_ptr       [in]
	A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
	check if there is sufficient size after aligning. if not an error will be returned

    @param aWorkspaceSizeInBytes[in]
    The size in bytes of the workspace buffer to be used by the SST.
    
    @return SST_RC_OK	                    The operation completed successfully.
    @return SST_RC_FAIL	                    The operation failed.
    @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
    @return SST_RC_ERROR_FATAL              The fatal error occurred.    
    @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
    @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
    @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
    @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
    @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
    @return SST_RC_ERROR_DATA_OFFSET	    Illegal data offset.
    @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
    @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
	@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
	@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
		
	@notes	This operation Must be performed inside a transaction.
			Note that transaction does not protect from performing concurrent operations. 
			It is the user responsibility not to perform simultaneity more than one operation 
			to a particular record.
    **/
    DxError_t SST_DataReplace(SSTTxnId_t      aTransactionId,
                              SSTSessionId_t  aSessionId,                           
                              SSTHandle_t     aDataHandle,   
                              DxByte_t       *aNewData_ptr,
                              DxUint32_t      aNewDataSizeInBytes,
                              DxBool_t        aEncrypt,
                              DxByte_t       *aWorkspace_ptr,
                              DxUint32_t      aWorkspaceSizeInBytes);
#ifdef __cplusplus
}
#endif

#endif  /* _DX_SST_API_DO_H_ */
