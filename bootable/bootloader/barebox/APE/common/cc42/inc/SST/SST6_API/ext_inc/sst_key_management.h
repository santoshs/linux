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
* Title:						Discretix SST API Key management header file						 					
*																			
* Filename:					    sst_key_management.h 															
*																			
* Project, Target, subsystem:	SST 6.0, API, Key Management
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_API_KM_H_
    #define _DX_SST_API_KM_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Global Includes ------------------------------------------------*/
    #include "DX_VOS_BaseTypes.h" 	
    
	/*----------- Global Includes ------------------------------------------------*/
    #include "sst_types.h" 	
 
    /*----------- Global function prototypes -------------------------------------*/


    
 /*Key management services */
        
        /*SST_AESKeyInsert*/
        /*!
        \brief        Store an AES key in the SST.

        @param aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in] 
        The handle of the initial authenticator to the newly created SST object. That authenticator has the highest level permission and it may be replaced later on.
        @param aKey_ptr [in]
        A pointer to the location of the key. 
        @param aKeyType [in]
        An enum value specifying the AES Key type: 128 , 196 or 256 bits.
        @param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        The pointer to the location where the handle of the newly created object will be put.
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
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_AESKeyInsert(SSTTxnId_t         aTransactionId,
                                    SSTSessionId_t     aSessionId,
                                    SSTHandle_t        aInitialCtrlAuth,  
                                    DxByte_t          *aKey_ptr,
                                    SSTAESKeyType_t    aKeyType,
                                    SSTDataType_t      aDataType,
                                    SSTHandle_t       *aKeyHandle_ptr,
									DxByte_t          *aWorkspace_ptr,
                                    DxUint32_t		  aWorkspaceSizeInBytes);

        
        /*SST_AESKeyGenerate*/
        /*!
        \brief        Generate and AES key and store it in the SST.

        @param aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in]
        The handle of the initial authenticator to the newly created SST object. That authenticator has the highest level permission and it may be replaced later on.
        @param aKeyType [in]
        An enum value specifying the AES Key type: 128 , 196 or 256 bits.
		@param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        The pointer to the location where the handle of the newly created object will be put.
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
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_AESKeyGenerate(SSTTxnId_t         aTransactionId,
                                      SSTSessionId_t     aSessionId,
                                      SSTHandle_t        aInitialCtrlAuth,                                
                                      SSTAESKeyType_t    aKeyType,
									  SSTDataType_t      aDataType,
                                      SSTHandle_t       *aKeyHandle_ptr,
									DxByte_t          *aWorkspace_ptr,
                                    DxUint32_t		  aWorkspaceSizeInBytes);

        
        /*SST_AESKeyRead*/
        /*!
        \brief     Retrieve an AES key from the SST.

        @param aSessionId [in] 
        The user's session ID.
        @param aKeyHandle [in]
        The handle of the requested key.
        @param aKey_ptr [out]
        The pointer to the location where the retrieved key will be stored.
        IMPORTANT - if the size of the provided buffer is smaller than the stored key a memory overrun will
                    occur. It is the user's responsibility to ensure that a buffer big enough is allocated.
        @param aKeyType_ptr [out]
        A pointer to the location where the retrieved key's type will be stored.
		@param aWorkspace_ptr [in]
		A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
		check if there is sufficient size after aligning. if not an error will be returned
        @param aWorkspaceSizeInBytes [in]
        The size in bytes of the workspace buffer to be used by the SST.
       
        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_DATA_SIZE          Illegal data size. (Data size equals to 0)
        @return SST_RC_ERROR_DATA_OFFSET	    Illegal data offset. (the requested start point exceeds the object size)
        @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_AESKeyRead( SSTSessionId_t     aSessionId,
                                   SSTHandle_t        aKeyHandle,   
                                   DxByte_t          *aKey_ptr,
                                   SSTAESKeyType_t   *aKeyType_ptr,
								   DxByte_t       *aWorkspace_ptr,
                                   DxUint32_t      aWorkspaceSizeInBytes);

        
        /*SST_DESKeyInsert*/
        /*!
        \brief      Store a DES key in the SST.
   
        @param aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in]
        The handle of the initial authenticator to the newly created SST object. 
        That authenticator has the highest level permission and it may be replaced later on.
        @param aKey_ptr [in]
        A pointer to the location of the key, 
		if 3DES All of the keys will be stored in this buffer consecutively
        @param aKeyType [in]
        An enum value specifying the DES Key type: 1DES , 3DES.
        @param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        The pointer to the location where the handle of the newly created object will be put.
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
        @return  SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_DESKeyInsert(SSTTxnId_t         aTransactionId,
                                    SSTSessionId_t     aSessionId,
                                    SSTHandle_t        aInitialCtrlAuth,
                                    DxByte_t          *aKey_ptr,
                                    SSTDESKeyType_t    aKeyType,
                                    SSTDataType_t      aDataType,
                                    SSTHandle_t       *aKeyHandle_ptr,
									DxByte_t       *aWorkspace_ptr,
                                    DxUint32_t      aWorkspaceSizeInBytes);

        
        /*SST_DESKeyGenerate*/
        /*!
        \brief  Generate a DES key and store it in the SST.

        @param aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in]
        The handle of the initial authenticator to the newly created SST object. That authenticator has the highest level permission and it may be replaced later on.
        @param aKeyType [in]
        An enum value specifying the DES Key type: 1DES , 3DES.
		@param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        The pointer to the location where the handle of the newly created object will be put.
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
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_DESKeyGenerate(SSTTxnId_t        aTransactionId,
                                      SSTSessionId_t    aSessionId,                           
                                     SSTHandle_t        aInitialCtrlAuth,   
                                     SSTDESKeyType_t    aKeyType,
									 SSTDataType_t      aDataType,
                                     SSTHandle_t       *aKeyHandle_ptr,
									DxByte_t          *aWorkspace_ptr,
                                    DxUint32_t		  aWorkspaceSizeInBytes);

        
        /*SST_DESKeyRead*/
        /*!
        \brief  Retrieve a DES key by its handle.

        @param aSessionId       [in]
        The user's session ID.
        @param aKeyHandle       [in]
        The handle of the key object to read.
        @param aKeyType_ptr     [out]
        A pointer to the location where the requested key type will be stored.
        @param aKey_ptr         [out]
        a pointer to the location where the requested key will be stored. 
		if 2/3DES All of the keys will be stored in this buffer consecutively
		@param aWorkspace_ptr   [in]
		A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
		check if there is sufficient size after aligning. if not an error will be returned
        @param aWorkspaceSizeInBytes [in]
        The size in bytes of the workspace buffer to be used by the SST.
        
        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_ACCESS_DENIED	    The user doesn't have the needed permission to perform this operation
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
        @return SST_RC_ERROR_DATA_OFFSET        Illegal data offset. (the requested start point exceeds the object size)
        @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_DESKeyRead(SSTSessionId_t     aSessionId,
                                  SSTHandle_t        aKeyHandle,
                                  DxByte_t          *aKey_ptr,
                                  SSTDESKeyType_t   *aKeyType_ptr,
								  DxByte_t       *aWorkspace_ptr,
                                  DxUint32_t      aWorkspaceSizeInBytes);

        
        /*SST_HMACKeyInsert*/
        /*!
        \brief Insert an HMAC key.
        
        @param  aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in]
        The handle of the initial authenticator to the newly created SST object. That authenticator has the highest level permission and it may be replaced later on.
        @param aKey_ptr [in]
        A pointer to the location of the key. 
        @param aKeySizeInBytes[in]
        The size of the key in bytes. CRYS SUPPORTS only 64 or 128 Byte size
        @param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr[in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        The pointer to the location where the handle of the newly created object will be put.
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
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_HMACKeyInsert(SSTTxnId_t      aTransactionId,                             
                                     SSTSessionId_t  aSessionId,
                                     SSTHandle_t     aInitialCtrlAuth,   
                                     DxByte_t       *aKey_ptr,
                                     DxUint32_t      aKeySizeInBytes,
                                     SSTDataType_t   aDataType,
                                     SSTHandle_t    *aKeyHandle_ptr,
									 DxByte_t       *aWorkspace_ptr,
                                     DxUint32_t      aWorkspaceSizeInBytes);

        
        /*SST_HMACKeyGenerate*/
        /*!
        \brief      Generate an HMAC key and store it in the SST.

        @param aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in]
        The handle of the initial authenticator to the newly created SST object. That authenticator has the highest level permission and it may be replaced later on.
        @param aKeysizeInBytes [in]
        The size of the required key in bytes.
		@param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr[in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        This pointer to the location where the handle of the newly created object will be put.
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
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_HMACKeyGenerate(SSTTxnId_t      aTransactionId,
                                      SSTSessionId_t  aSessionId,    
                                      SSTHandle_t     aInitialCtrlAuth,   
                                      DxUint32_t      aKeySizeInBytes,
									  SSTDataType_t   aDataType,
                                      SSTHandle_t     *aKeyHandle_ptr,
									  DxByte_t        *aWorkspace_ptr,
                                      DxUint32_t	  aWorkspaceSizeInBytes);
  
        
        /*SST_HMACKeyRead*/
        /*!
        \brief    Retrieve an HMAC key.

        @param aSessionId [in]
        The user's session ID.
        @param aKeyHandle [in]
        The handle of the requested key object.
        @param aKey_ptr [out]
        A pointer to the location where the key type will be stored.  
        @param aKeysizeInBytes [in/out]
        In: the size of the provided buffer. Out: the actual size used.
		@param aWorkspace_ptr [in]
		A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
		check if there is sufficient size after aligning. if not an error will be returned
        @param aWorkspaceSizeInBytes [in]
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
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_HMACKeyRead(SSTSessionId_t  aSessionId,
                                   SSTHandle_t     aKeyHandle,
                                   DxByte_t       *aKey_ptr,
                                   DxUint32_t     *aKeysizeInBytes,
								   DxByte_t       *aWorkspace_ptr,
                                   DxUint32_t      aWorkspaceSizeInBytes);

        
        /*_DX_SST_RSAKeyInsert*/
        /*!
        \brief      Insert an RSA key to the SST, either public or private key or both.

		the actual macro that will be used by the user are:

		SST_RSAPublicKeyInsert	- Insert RSA public key
		SST_RSAPrivateKeyInsert	- Insert RSA private key
		SST_RSAPairKeyInsert	- Insert RSA publicand private key
   
        @param aTransactionId	[in]	The user's transaction ID.
        @param aSessionId		[in]    The user's session ID.
        @param aInitialCtrlAuth [in]    The handle of the initial authenticator to the 
										newly created SST object. That authenticator has 
										the highest level permission and it may be replaced 
										later on.
		@param aKeyType			[in]	The type of key record (private, public or both)
        @param aN_ptr			[in]	The modulus vector. 
        @param aNsizeInBytes	[in]    The modulus vector's size.
        @param aD_ptr			[in]    The private exponent vector. This vector can be set 
										to NULL if the public exponent E exists. 
        @param aDsizeInBytes	[in]	The private exponent vector's size.
        @param aE_ptr			[in]	The public exponent vector. This vector may be set 
										to NULL if the private exponent D exists. 
        @param aEsizeInBytes	[in]	The public exponent vector's size.
        @param aDataType		[in]    The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]	The user may ask the SST to randomly generate a handle 
										by setting the handle to which this pointer points to 
										SST_HANDLE_RANDOM_GENERATE. This is a pointer to the 
										location where the handle of the newly created object 
										will be put.
		@param aWorkspace_ptr	[in]	A pointer to the workspace buffer to be used by the SST. 
										If the buffer is not word aligned SST will check if there 
										is sufficient size after aligning. if not an error will 
										be returned
        @param aWorkspaceSizeInBytes [in]	The size in bytes of the workspace buffer to be used 
											by the SST.
       
        @return  SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t _DX_SST_RSAKeyInsert( SSTTxnId_t      aTransactionId,
										SSTSessionId_t  aSessionId,                          
										SSTHandle_t     aInitialCtrlAuth, 
										SSTRSAKeyType_t aKeyType,
										DxUint8_t      *aN_ptr,
										DxUint32_t      aNsizeInBytes,
										DxUint8_t      *aD_ptr,
										DxUint32_t      aDsizeInBytes,
										DxUint8_t      *aE_ptr, 
										DxUint32_t      aEsizeInBytes,
										SSTDataType_t   aDataType,
										SSTHandle_t    *aKeyHandle_ptr,
										DxByte_t       *aWorkspace_ptr,
										DxUint32_t      aWorkspaceSizeInBytes);

        /**
		\brief The macros that the user should call when performing the RSA key insert
		Note that the return value is DxError_t.
		   
		Macro SST_RSAPublicKeyInsert is called when the user inserts an public RSA key
		Macro SST_RSAPrivateKeyInsert is called when the user inserts an private RSA key
		Macro SST_RSAPairKeyInsert is called when the user inserts an both public and private RSA keys
		
		*/

		#define SST_RSAPublicKeyInsert(aTransactionId,aSessionId,aInitialCtrlAuth,aN_ptr,\
				aNsizeInBytes,aE_ptr,aEsizeInBytes,aDataType,aKeyHandle_ptr,aWorkspace_ptr,\
				aWorkspaceSizeInBytes) \
				_DX_SST_RSAKeyInsert((aTransactionId),(aSessionId),(aInitialCtrlAuth),\
				(SST_KEY_TYPE_RSA_ONLY_PUBLIC_KEY),(aN_ptr),(aNsizeInBytes),(DX_NULL),(0),\
				(aE_ptr),(aEsizeInBytes),(aDataType),(aKeyHandle_ptr),(aWorkspace_ptr),\
				(aWorkspaceSizeInBytes))


		#define SST_RSAPrivateKeyInsert(aTransactionId,aSessionId,aInitialCtrlAuth,aD_ptr,\
				aDsizeInBytes,aN_ptr,aNsizeInBytes,aDataType,aKeyHandle_ptr,aWorkspace_ptr,\
				aWorkspaceSizeInBytes) \
				_DX_SST_RSAKeyInsert((aTransactionId),(aSessionId),(aInitialCtrlAuth),\
				(SST_KEY_TYPE_RSA_ONLY_PRIVATE_KEY),(aN_ptr),(aNsizeInBytes),(aD_ptr),\
				(aDsizeInBytes),(DX_NULL),(0),(aDataType),(aKeyHandle_ptr),\
				(aWorkspace_ptr),(aWorkspaceSizeInBytes))


		#define SST_RSAPairKeyInsert(aTransactionId,aSessionId,aInitialCtrlAuth,aN_ptr,aNsizeInBytes,\
				aD_ptr,aDsizeInBytes,aE_ptr,aEsizeInBytes,aDataType,aKeyHandle_ptr,aWorkspace_ptr,\
				aWorkspaceSizeInBytes) \
				_DX_SST_RSAKeyInsert((aTransactionId),(aSessionId),(aInitialCtrlAuth),\
				(SST_KEY_TYPE_RSA_BOTH_PUBLIC_AND_PRIVATE_KEY),(aN_ptr),(aNsizeInBytes),(aD_ptr),\
				(aDsizeInBytes),(aE_ptr),(aEsizeInBytes),(aDataType),(aKeyHandle_ptr),(aWorkspace_ptr),\
				(aWorkspaceSizeInBytes))




        /*SST_RSAKeyGenerate*/
        /*!
        \brief         Generate an RSA key and store it in the SST.

        @param aTransactionId [in]
        The user's transaction ID.
        @param aSessionId [in]
        The user's session ID.
        @param aInitialCtrlAuth [in]
        The handle of the initial authenticator to the newly created SST object. That authenticator has the highest level permission and it may be replaced later on.
        @param aNsizeInBytes [in]
        The modulus vector's size.
        @param aE_ptr [in]
        The public exponent vector.  
        @param aEsizeInBytes [in]
        The public exponent vector's size.
		@param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        This is a pointer to the location where the handle of the newly created object will be put.
        
        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_RSAKeyGenerate(SSTTxnId_t      aTransactionId,
                                     SSTSessionId_t  aSessionId,
                                     SSTHandle_t     aInitialCtrlAuth,  
                                     DxUint32_t      aNsizeInBytes,
                                     DxByte_t       *aE_ptr,                                     
                                     DxUint32_t      aEsizeInBytes,
									 SSTDataType_t   aDataType,
                                     SSTHandle_t    *aKeyHandle_ptr,
									 DxByte_t       *aWorkspace_ptr,
									 DxUint32_t      aWorkspaceSizeInBytes);

        
        /*_DX_SST_RSAKeyRead*/
        /*!
        \brief Retrieve an RSA key.

		The actual macro that will be used by the user are:

		SST_RSAPublicKeyRead	- Read RSA public key
		SST_RSAPrivateKeyRead	- Read RSA private key
		
        @param aSessionId       [in]  
        The user's session ID.
        @param aKeyHandle       [in]  
        The handle of the key object which read is requested.
		@param aKeyType			[in]	The type of key record (private, public or both)to read
        @param aN_ptr           [out]
        A pointer to the location of the modulus vector. 
        @param aD_ptr           [out]
        A pointer to the location of the private exponent vector. 
        @param aE_ptr           [out]
        A pointer to the location of the public exponent vector. 
        @param aNsizeInBytes_ptr [in\out]
        A pointer to the location of the modulus vector's size.
        In: the size of the provided buffer. Out: the actual size used.
        @param aDsizeInBytes_ptr [in\out]
        A pointer to the location of the private exponent vector's size.
        In: the size of the provided buffer. Out: the actual size used.
        @param aEsizeInBytes_ptr[in\out]
        A pointer to the location of the public exponent vector's size.
        In: the size of the provided buffer. Out: the actual size used.
		@param aWorkspace_ptr [in]
		A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
		check if there is sufficient size after aligning. if not an error will be returned
        @param aWorkspaceSizeInBytes [in]
        The size in bytes of the workspace buffer to be used by the SST.

        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_ACCESS_DENIED  	The user doesn't have the needed permission to perform this operation
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
        @return SST_RC_ERROR_DATA_OFFSET	    Illegal data offset. (the requested start point exceeds the object size)
        @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t _DX_SST_RSAKeyRead(SSTSessionId_t   aSessionId,  
                                   SSTHandle_t     aKeyHandle, 
								   SSTRSAKeyType_t aKeyType,
                                   DxUint8_t      *aN_ptr,
                                   DxUint32_t     *aNsizeInBytes_ptr,
                                   DxUint8_t      *aD_ptr,
                                   DxUint32_t     *aDsizeInBytes_ptr,
                                   DxUint8_t      *aE_ptr,
                                   DxUint32_t     *aEsizeInBytes_ptr,
								   DxByte_t       *aWorkspace_ptr,
                                   DxUint32_t      aWorkspaceSizeInBytes);


		/**
		\brief The macros that the user should call when performing the RSA key read
		Note that the return value is DxError_t.
		   
		Macro SST_RSAPublicKeyRead is called when the user read an public RSA key
		Macro SST_RSAPrivateKeyRead is called when the user read an private RSA key
				
		*/

		#define SST_RSAPublicKeyRead(aSessionId,aKeyHandle,aN_ptr,aNsizeInBytes_ptr,aE_ptr,\
				aEsizeInBytes_ptr,aWorkspace_ptr,aWorkspaceSizeInBytes) \
				_DX_SST_RSAKeyRead((aSessionId),(aKeyHandle),(SST_KEY_TYPE_RSA_ONLY_PUBLIC_KEY),\
				(aN_ptr),(aNsizeInBytes_ptr),(DX_NULL),(DX_NULL),(aE_ptr),(aEsizeInBytes_ptr),\
				(aWorkspace_ptr),(aWorkspaceSizeInBytes))

		#define SST_RSAPrivateKeyRead(aSessionId,aKeyHandle,aD_ptr,aDsizeInBytes_ptr,aN_ptr,\
				aNsizeInBytes_ptr,aWorkspace_ptr,aWorkspaceSizeInBytes) \
				_DX_SST_RSAKeyRead((aSessionId),(aKeyHandle),(SST_KEY_TYPE_RSA_ONLY_PRIVATE_KEY),\
				(aN_ptr),(aNsizeInBytes_ptr),(aD_ptr),(aDsizeInBytes_ptr),(DX_NULL),(DX_NULL),\
				(aWorkspace_ptr),(aWorkspaceSizeInBytes))



        
        /*_DX_SST_CRTRSAKeyInsert*/
        /*!
        \brief      Store a CRT RSA key in the SST.
		
		the actual macro that will be used by the user are:

		SST_CRTRSAPublicKeyInsert	- Insert CRT RSA public key
		SST_CRTRSAPrivateKeyInsert	- Insert CRT RSA private key
		SST_CRTRSAPairKeyInsert		- Insert CRT RSA public and private key

        @param aTransactionId	[in]	The user's transaction ID.
        @param aSessionId		[in]	The user's session ID.
        @param aInitialCtrlAuth [in]	The handle of the initial authenticator to the newly created SST 
										object. That authenticator has the highest level permission and 
										it may be replaced later on. The public exponent vector's size 
										in bytes.
		@param aKeyType			[in]	The type of key record (private, public or both)
        @param aN_ptr			[in]	The modulus vector. 
        @param aNsizeInBytes	[in]    The modulus vector's size.
        @param aE_ptr			[out]	A pointer to the RSA public key exponent.
		@param aEsizeInBytes_ptr[out]   A pointer to the location of the public exponent vector's size. 
                                        If this value is NULL, the required size will be stored in the 
										location aEsizeInBytes_ptr.
		@param aP_ptr			[in]    A pointer to the first factor vector. 
        @param aPsizeInBytes	[in]    The first factor vector's size in bytes.
        @param aQ_ptr			[in]    A pointer to the second factor vector. 
        @param aQsizeInBytes	[in]    The second factor vector's size in bytes.
        @param aDP_ptr			[in]    A pointer to the first factor exponent vector. 
        @param aDPsizeInBytes	[in]    The first factor exponent vector's size in bytes.
        @param aDQ_ptr			[in]    A pointer to the second factor exponent vector. 
        @param aDQsizeInBytes	[in]    The second factor exponent vector's size in bytes.
        @param aQinv_ptr		[in]    A pointer to the coefficient vector. 
        @param aQinvsizeInBytes [in]    The coefficient vector's size in bytes.
		@param aDataType		[in]    The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]  The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        This is a pointer to the location where the handle of the newly created object will be put.
		@param aWorkspace_ptr	[in]    A pointer to the workspace buffer to be used by the SST.
										If the buffer is not word aligned SST will check if there 
										is sufficient size after aligning. if not an error will be returned
        @param aWorkspaceSizeInBytes [in] The size in bytes of the workspace buffer to be used by the SST.

        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t _DX_SST_CRTRSAKeyInsert(	SSTTxnId_t    aTransactionId,
											SSTSessionId_t  aSessionId,
											SSTHandle_t     aInitialCtrlAuth, 
											SSTRSAKeyType_t aKeyType,
											DxByte_t		*aN_ptr,
											DxUint32_t      aNsizeInBytes,
											DxByte_t       *aE_ptr,
											DxUint32_t      aEsizeInBytes,
											DxByte_t       *aP_ptr,
											DxUint32_t      aPsizeInBytes,
											DxByte_t       *aQ_ptr,
											DxUint32_t      aQsizeInBytes,
											DxByte_t       *aDP_ptr,
											DxUint32_t      aDPsizeInBytes,
											DxByte_t       *aDQ_ptr,
											DxUint32_t      aDQsizeInBytes,
											DxByte_t       *aQinv_ptr,
											DxUint32_t      aQinvsizeInBytes,
											SSTDataType_t   aDataType,
											SSTHandle_t    *aKeyHandle_ptr,
											DxByte_t       *aWorkspace_ptr,
											DxUint32_t      aWorkspaceSizeInBytes);



		 /**
		\brief The macros that the user should call when performing the CRT RSA key insert
		Note that the return value is DxError_t.
		   
		Macro SST_CRTRSAPublicKeyInsert is called when the user inserts an public CRT RSA key
		Macro SST_CRTRSAPrivateKeyInsert is called when the user inserts an private CRT RSA key
		Macro SST_CRTRSAPairKeyInsert is called when the user inserts an both public and private 
		CRT RSA keys
		
		*/

		#define SST_CRTRSAPublicKeyInsert(aTransactionId,aSessionId,aInitialCtrlAuth,aN_ptr,\
				aNsizeInBytes,aE_ptr,aEsizeInBytes,aDataType,aKeyHandle_ptr,aWorkspace_ptr,\
				aWorkspaceSizeInBytes) \
				_DX_SST_CRTRSAKeyInsert((aTransactionId),(aSessionId),(aInitialCtrlAuth),\
				(SST_KEY_TYPE_RSA_ONLY_PUBLIC_KEY),(aN_ptr),(aNsizeInBytes),(aE_ptr),(aEsizeInBytes),\
				(DX_NULL),(0),(DX_NULL),(0),(DX_NULL),(0),(DX_NULL),(0),\
				(DX_NULL),(0),(aDataType),(aKeyHandle_ptr),(aWorkspace_ptr),(aWorkspaceSizeInBytes))


		#define SST_CRTRSAPrivateKeyInsert(aTransactionId,aSessionId,aInitialCtrlAuth,aP_ptr,\
				aPsizeInBytes,aQ_ptr,aQsizeInBytes,aDP_ptr,aDPsizeInBytes,aDQ_ptr,aDQsizeInBytes,\
				aQinv_ptr,aQinvsizeInBytes,aDataType,aKeyHandle_ptr,aWorkspace_ptr,\
				aWorkspaceSizeInBytes) \
				_DX_SST_CRTRSAKeyInsert((aTransactionId),(aSessionId),(aInitialCtrlAuth),\
				(SST_KEY_TYPE_RSA_ONLY_PRIVATE_KEY),(DX_NULL),(0),(DX_NULL),(0),(aP_ptr),\
				(aPsizeInBytes),(aQ_ptr),(aQsizeInBytes),(aDP_ptr),(aDPsizeInBytes),(aDQ_ptr),\
                (aDQsizeInBytes),(aQinv_ptr),(aQinvsizeInBytes),(aDataType),(aKeyHandle_ptr),\
				(aWorkspace_ptr),(aWorkspaceSizeInBytes))


		#define SST_CRTRSAPairKeyInsert(aTransactionId,aSessionId,aInitialCtrlAuth,aN_ptr,aNsizeInBytes,\
				aE_ptr,aEsizeInBytes,aP_ptr,aPsizeInBytes,aQ_ptr,aQsizeInBytes,aDP_ptr,aDPsizeInBytes,\
				aDQ_ptr,aDQsizeInBytes,aQinv_ptr,aQinvsizeInBytes,aDataType,aKeyHandle_ptr,\
				aWorkspace_ptr,aWorkspaceSizeInBytes) \
				_DX_SST_CRTRSAKeyInsert((aTransactionId),(aSessionId),(aInitialCtrlAuth),\
				(SST_KEY_TYPE_RSA_BOTH_PUBLIC_AND_PRIVATE_KEY),(aN_ptr),(aNsizeInBytes),(aE_ptr),\
				(aEsizeInBytes),(aP_ptr),(aPsizeInBytes),(aQ_ptr),(aQsizeInBytes),(aDP_ptr),\
				(aDPsizeInBytes),(aDQ_ptr),(aDQsizeInBytes),(aQinv_ptr),(aQinvsizeInBytes),(aDataType),\
				(aKeyHandle_ptr),(aWorkspace_ptr),(aWorkspaceSizeInBytes))


        
        /*SST_CRTRSAKeyGenerate*/
        /*!
        \brief         Generate and store a CRT RSA key in the SST.

        @param aTransactionId [in] 
        The user's transaction ID.
        @param aSessionId [in] 
        The user's session ID.
        @param aInitialCtrlAuth [in] 
        The handle of the initial authenticator to the newly created SST object. 
		That authenticator has the highest level permission and it may be replaced later on.
        @param aNSizeInBytes [in] 
        The modulus vector's size in bytes.
        @param aE_ptr [in] 
        A pointer to the public exponent vector. 
        @param aEsizeInBytes [in] 
        the public exponent vector's size in bytes.
		@param aDataType [in]
        The data type with which this key will be associated.
        @param aKeyHandle_ptr [in/out]
        The user may ask the SST to randomly generate a handle by setting the handle to which this pointer points to SST_HANDLE_RANDOM_GENERATE.
        This is the location to which the handle of the created object will be put.

        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_TXN_INVALID	    The received transaction ID is not valid. (probably this is not the active transaction)
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_NULL_POINTER		At least one of the past arguments is null.
        @return SST_RC_ERROR_HANDLE_NOT_FREE	The requested handle is already taken. 
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_HANDLE_MISMATCH	The authenticator handle refers to an object of different type or an invalid handle
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t SST_CRTRSAKeyGenerate(SSTTxnId_t      aTransactionId,
                                        SSTSessionId_t  aSessionId,
                                        SSTHandle_t     aInitialCtrlAuth, 
                                        DxUint32_t      aNsizeInBytes,
                                        DxByte_t       *aE_ptr,
                                        DxUint32_t      aEsizeInBytes,
										SSTDataType_t   aDataType,
                                        SSTHandle_t    *aKeyHandle_ptr,
										DxByte_t       *aWorkspace_ptr,
										DxUint32_t      aWorkspaceSizeInBytes);

        
        /*_DX_SST_CRTRSAKeyRead*/
        /*!
        \brief         Read a CRT RSA Key object.
		
		the actual macro that will be used by the user are:

		SST_CRTRSAPublicKeyRead		- Read CRT RSA public key
		SST_CRTRSAPrivateKeyRead	- Read CRT RSA private key

        @param aSessionId [in] 
        The user's session ID.
        @param aKeyHandle [in] 
        The handle of the desired key object.
		@param aKeyType [in]	The type of key record (private, public or both) to read
        @param aP_ptr   [out]
        A pointer to the first factor vector. 
        @param aQ_ptr   [out]
        A pointer to the second factor vector. 
        @param aDP_ptr  [out]
        A pointer to the first factor exponent vector. 
        @param aDQ_ptr  [out]
        A pointer to the second factor exponent vector. 
        @param aQinv_ptr[out]
        A pointer to the coefficient vector. 
		@param aE_ptr   [out]
        A pointer to the location of the public exponent vector. 
		@param aEsizeInBytes_ptr[in\out]
            In:  the size of the allocated buffer aE_ptr.
            Out: A pointer to the location of the public exponent vector's size.
        @param aPsizeInBytes_ptr  [in\out]
            In:  the size of the allocated buffer aP_ptr.
            Out: A pointer to the first factor vector's size in bytes
        @param aQsizeInBytes_ptr  [in\out]
            In:  the size of the allocated buffer aQ_ptr.
            Out: A pointer to the second factor vector's size in bytes.
        @param aDPsizeInBytes_ptr  [in\out]
            In:  the size of the allocated buffer aDP_ptr.
            Out: A pointer to the first factor exponent vector's size in bytes.
        @param aDQsizeInBytes_ptr [in\out] 
            In:  the size of the allocated buffer aDQ_ptr.
            Out: A pointer to he second factor exponent vector's size in bytes.
        @param aQinvsizeInBytes_ptr [in\out]
            In:  the size of the allocated buffer aQinv_ptr.
            Out: A pointer to the coefficient vector's size in bytes.
		@param aWorkspace_ptr [in]
		A pointer to the workspace buffer to be used by the SST. If the buffer is not word aligned SST will 
		check if there is sufficient size after aligning. if not an error will be returned
        @param aWorkspaceSizeInBytes [in]
        The size in bytes of the workspace buffer to be used by the SST.

        @return SST_RC_OK	                    The operation completed successfully.
        @return SST_RC_FAIL	                    The operation failed.
        @return SST_RC_ERROR_CLOSED             The SST is closed (The SST_Init function was not performed)
        @return SST_RC_ERROR_FATAL              The fatal error occurred.    
        @return SST_RC_ERROR_SESSION_INVALID	The received session ID is not valid. (probably this is not an active session)
        @return SST_RC_ERROR_ACCESS_DENIED  	The user doesn't have the needed permission to perform this operation
        @return SST_RC_ERROR_HANDLE_NOT_FOUND	A handle was not found.
        @return SST_RC_ERROR_DATA_SIZE	        Illegal data size. (Data size equals to 0)
        @return SST_RC_ERROR_DATA_OFFSET	    Illegal data offset. (the requested start point exceeds the object size)
        @return SST_RC_ERROR_NULL_POINTER	    At least one of the past arguments is null.
        @return SST_RC_ERROR_VCRYS       	    An error that involves the cryptography services package has occurred.
		@return SST_RC_ERROR_MEM_ALLOC	        The operation failed due to memory allocation failure.
		@return SST_RC_ERROR_WORKSPACE	        Insufficient workspace given by user.
        **/
        DxError_t _DX_SST_CRTRSAKeyRead(SSTSessionId_t  aSessionId,
										SSTHandle_t     aKeyHandle,
										SSTRSAKeyType_t aKeyType,
										DxByte_t		*aN_ptr,
										DxUint32_t     *aNsizeInBytes_ptr,
										DxByte_t       *aE_ptr,
										DxUint32_t     *aEsizeInBytes_ptr,
										DxByte_t       *aP_ptr,
										DxUint32_t     *aPsizeInBytes_ptr,
										DxByte_t       *aQ_ptr,
										DxUint32_t     *aQsizeInBytes_ptr,
										DxByte_t       *aDP_ptr,
										DxUint32_t     *aDPsizeInBytes_ptr,
										DxByte_t       *aDQ_ptr,
										DxUint32_t     *aDQsizeInBytes_ptr,
										DxByte_t       *aQinv_ptr,
										DxUint32_t     *aQinvsizeInBytes_ptr,
										DxByte_t       *aWorkspace_ptr,
										DxUint32_t      aWorkspaceSizeInBytes);


		/**
		\brief The macros that the user should call when performing the CRT RSA key insert
		Note that the return value is DxError_t.
		   
		Macro SST_CRTRSAPublicKeyRead is called when the user read an public CRT RSA key
		Macro SST_CRTRSAPrivateKeyRead is called when the user read an private CRT RSA key
				
		*/

		#define SST_CRTRSAPublicKeyRead(aSessionId,aKeyHandle,aN_ptr,aNsizeInBytes_ptr,aE_ptr,\
			    aEsizeInBytes_ptr,aWorkspace_ptr,aWorkspaceSizeInBytes) \
				_DX_SST_CRTRSAKeyRead((aSessionId),(aKeyHandle),(SST_KEY_TYPE_RSA_ONLY_PUBLIC_KEY),\
				(aN_ptr),(aNsizeInBytes_ptr),(aE_ptr),(aEsizeInBytes_ptr),(DX_NULL),(DX_NULL),\
				(DX_NULL),(DX_NULL),(DX_NULL),(DX_NULL),(DX_NULL),(DX_NULL),(DX_NULL),(DX_NULL),\
				(aWorkspace_ptr),(aWorkspaceSizeInBytes))


		#define SST_CRTRSAPrivateKeyRead(aSessionId,aKeyHandle,aP_ptr,aPsizeInBytes_ptr,\
				aQ_ptr,aQsizeInBytes_ptr,aDP_ptr,aDPsizeInBytes_ptr,aDQ_ptr,aDQsizeInBytes_ptr,\
                aQinv_ptr,aQinvsizeInBytes_ptr,aWorkspace_ptr,aWorkspaceSizeInBytes) \
				_DX_SST_CRTRSAKeyRead((aSessionId),(aKeyHandle),(SST_KEY_TYPE_RSA_ONLY_PRIVATE_KEY),\
				(DX_NULL),(DX_NULL),(DX_NULL),(DX_NULL),(aP_ptr),(aPsizeInBytes_ptr),(aQ_ptr),\
				(aQsizeInBytes_ptr),(aDP_ptr),(aDPsizeInBytes_ptr),(aDQ_ptr),(aDQsizeInBytes_ptr),\
				(aQinv_ptr),(aQinvsizeInBytes_ptr),(aWorkspace_ptr),(aWorkspaceSizeInBytes))

#ifdef __cplusplus
}
#endif 									 
       
#endif  /* _DX_SST_API_KM_H_ */
