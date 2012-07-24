/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS External API header file		
*																			
* Filename:						sst_vcrys_ext.h															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Cryptography Services
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved		
*								This file works with CRYS6
**********************************************************************************/

#ifndef _DX_SST_VCRYS_EXT_H_
    #define _DX_SST_VCRYS_EXT_H_

	#include "sst_vcrys.h"
	#include "sst_vcrys_config.h"
	#include "sst_debug.h"
	#include "CRYS.h"   
	#include "CRYS_SST.h"
	
	/*----------- Global defines -------------------------------------------------*/
    #if ((defined SST_DEBUG_MODE_ENABLED) && (!defined SST_VCRYS_DEBUG_MODE_ENABLED))
        #define SST_VCRYS_DEBUG_MODE_ENABLED
    #endif

	#if ((defined SST_TESTING_MODE_ENABLED) && (!defined SST_VCRYS_TESTING_MODE_ENABLED))
		#define SST_VCRYS_TESTING_MODE_ENABLED
	#endif
	
    /*----------- Global macro definitions ---------------------------------------*/
	/*! brief  the maximum size of AES key in bytes**/
	#define SST_VCRYS_AES_MAX_KEY_SIZE_IN_BYTES		CRYS_AES_KEY_MAX_SIZE_IN_BYTES
	
	/*! brief  the size of 1DES key in bytes**/
	#define SST_VCRYS_1DES_KEY_SIZE_IN_BYTES		CRYS_DES_KEY_SIZE_IN_BYTES

	/*! brief  the size of 2DES key in bytes**/
	#define SST_VCRYS_2DES_KEY_SIZE_IN_BYTES		(CRYS_DES_KEY_SIZE_IN_BYTES*2)

	/*! brief  the size of 3DES key in bytes**/
	#define SST_VCRYS_3DES_KEY_SIZE_IN_BYTES		(CRYS_DES_KEY_SIZE_IN_BYTES*3)	

	/*! brief  the maximum size of DES key in bytes**/
	#define SST_VCRYS_DES_MAX_KEY_SIZE_IN_BYTES		(CRYS_DES_KEY_SIZE_IN_BYTES*\
													 CRYS_DES_MAX_NUMBER_OF_KEYS)

	/*! brief  the maximum size of HMAC key in bytes**/
	#define SST_VCRYS_HMAC_512_KEY_SIZE_IN_BYTES	CRYS_HMAC_KEY_SIZE_IN_BYTES

	/*! brief  the maximum size of HMAC key in bytes**/
	#define SST_VCRYS_HMAC_1024_KEY_SIZE_IN_BYTES	CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES
	
	/*! brief  the maximum size of HMAC key in bytes**/
	#define SST_VCRYS_HMAC_MAX_KEY_SIZE_IN_BYTES	CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES

	/*! brief  the maximum size of RSA key in bytes**/
	#define SST_VCRYS_RSA_KEY_SIZE					sizeof(CRYS_SST_RSA_pair_keys_container_t)

    /*! brief check whether a given return code has originated in CRYS**/
    #define SST_VCRYS_IS_CRYS_RC(crysRc)            ((crysRc&CRYS_ERROR_BASE)==CRYS_ERROR_BASE)
    /*----------- Global type definitions ----------------------------------------*/

	/*! brief SST VCRYS external function for RSA \ RSA-CRT generate key**/
	typedef CRYS_SST_RSA_keys_and_data_container_t	SSTVCRYSRSADataContainer_t;

	/*! brief SST definition for RSA \ RSA-CRT key data**/
	typedef struct  
	{
		SSTRSAKeyType_t	type;
		DxByte_t		data[SST_VCRYS_RSA_KEY_SIZE];
	}SSTKeyRSA_t;

    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Generate a AES key
	@param transactionId	[in]		The opened transaction id.
	@param sessionId		[in]		The opened authenticator Session Id
	@param masterAuthHandle [in]		The master authenticator that has all of the 
										access writes on the new inserted data including 
										binding writes to additional authenticators.
	@param dataType			[in]		user depend for internal SST use
	@param keySize			[in]		The AES key type: 128, 196 or 256 bits
	@param keyHandle_ptr	[in\out]	Handle to SST entry. The user can enter an empty 
										handle and get a new handle, or enter handle and 
										force the SST to use it if possible.
	@param aWorkspace_ptr	[in]        A pointer to the workspace buffer to be used by the SST.
    @param aWorkspaceSizeInBytes [in]   The size in bytes of the workspace buffer to be used by the SST.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSAESKeyGenerate (SSTTxnId_t		transactionId,
													SSTSessionId_t	sessionId,
													SSTHandle_t		masterAuthHandle,
													SSTDataType_t	dataType,
													SSTAESKeyType_t	keySize, 
													SSTHandle_t		*keyHandle_ptr,
													DxByte_t       *workspace_ptr,
													DxUint32_t      workspaceSizeInBytes);
	


	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Generate a DES key
	@param transactionId	[in]		The opened transaction id.
	@param sessionId		[in]		The opened authenticator Session Id
	@param masterAuthHandle [in]		The master authenticator that has all of the 
										access writes on the new inserted data including 
										binding writes to additional authenticators.
	@param dataType			[in]		user depend for internal SST use
	@param nKeys			[in]		The DES key type: 1DES, 3DES
	@param keyHandle_ptr	[in\out]	Handle to SST entry. The user can enter an empty 
										handle and get a new handle, or enter handle and 
										force the SST to use it if possible.
	@param aWorkspace_ptr	[in]        A pointer to the workspace buffer to be used by the SST.
    @param aWorkspaceSizeInBytes [in]   The size in bytes of the workspace buffer to be used by the SST.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSDESKeyGenerate (SSTTxnId_t		transactionId,
													SSTSessionId_t	sessionId, 
													SSTHandle_t		masterAuthHandle,
													SSTDataType_t	dataType,
													SSTDESKeyType_t	nKeys, 
													SSTHandle_t		*keyHandle_ptr,
													DxByte_t       *workspace_ptr,
													DxUint32_t      workspaceSizeInBytes);
	
	
	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Generate a HMAC key  
	@param transactionId	[in]		The opened transaction id.
	@param sessionId		[in]		The opened authenticator Session Id
	@param masterAuthHandle [in]		The master authenticator that has all of the 
										access writes on the new inserted data including 
										binding writes to additional authenticators.
	@param dataType			[in]		user depend for internal SST use
	@param keySize			[in]		the HMAC key size
	@param keyHandle_ptr	[in\out]	Handle to SST entry. The user can enter an empty 
										handle and get a new handle, or enter handle and 
										force the SST to use it if possible.
	@param aWorkspace_ptr	[in]        A pointer to the workspace buffer to be used by the SST.
    @param aWorkspaceSizeInBytes [in]   The size in bytes of the workspace buffer to be used by the SST.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	
	SSTVCRYSReturnCodes_t SST_VCRYSHMACKeyGenerate (SSTTxnId_t     transactionId,
													 SSTSessionId_t sessionId,
												   	 SSTHandle_t    masterAuthHandle,
													 SSTDataType_t	dataType,
													 DxUint32_t     keySize, 
													 SSTHandle_t    *keyHandle_ptr,
													DxByte_t       *workspace_ptr,
													DxUint32_t      workspaceSizeInBytes);




	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Generate a RSA key   
	@param transactionId	 [in]		The opened transaction id.
	@param sessionId		 [in]		The opened authenticator Session Id
	@param masterAuthHandle  [in]		The master authenticator that has all of the 
										access writes on the new inserted data including 
										binding writes to additional authenticators.
	@param dataType			 [in]		user depend for internal SST use
	@param pubExp_ptr		 [in] 		Public exponent  
	@param pubExpSizeInBytes [in]		Size of public exponent in bytes. 
	@param keySize			 [in]		Size of the modulus.  
	@param keyGenData_ptr	 [in]		Internal buffer for the key generation operation
	@param keyHandle_ptr	[in\out]	Handle to SST entry. The user can enter an empty 
										handle and get a new handle, or enter handle and 
										force the SST to use it if possible.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSRSAKeyGenerate (SSTTxnId_t				transactionId,
													SSTSessionId_t			sessionId,
													SSTHandle_t				masterAuthHandle,
													SSTDataType_t			dataType,
													SSTHandle_t				*keyHandle_ptr,
													DxByte_t				*pubExp_ptr,
													DxUint32_t				pubExpSizeInBytes,
													DxUint32_t				keySize,
													SSTVCRYSRSADataContainer_t *keyGenData_ptr);




	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Generate a CRT-RSA key    
	@param transactionId	 [in]		The opened transaction id.
	@param sessionId		 [in]		The opened authenticator Session Id
	@param masterAuthHandle  [in]		The master authenticator that has all of the 
										access writes on the new inserted data including 
										binding writes to additional authenticators.
	@param dataType			 [in]		user depend for internal SST use
	@param pubExp_ptr		 [in] 		Public exponent  
	@param pubExpSizeInBytes [in]		Size of public exponent in bytes. 
	@param keySize			 [in]		Size of the modulus.  
	@param keyGenData_ptr	 [in]		Internal buffer for the key generation operation
	@param keyHandle_ptr	 [in\out]	Handle to SST entry. The user can enter an empty 
										handle and get a new handle, or enter handle and 
										force the SST to use it if possible.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSCRTRSAKeyGenerate ( SSTTxnId_t				transactionId,
														SSTSessionId_t			sessionId,
														SSTHandle_t				masterAuthHandle,
														SSTDataType_t			dataType,
														SSTHandle_t				*keyHandle_ptr,
														DxByte_t				*pubExp_ptr,
														DxUint32_t				pubExpSizeInBytes,
														DxUint32_t				keySize,
														SSTVCRYSRSADataContainer_t *keyGenData_ptr);




	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Build a RSA key using parameters from user  
	@param n_ptr			[in]	The modulus vector
	@param nSize			[in]	The modulus vector size in bytes
	@param e_ptr			[in]	The public exponent vector. 
									This vector can be set to NULL if the private exponent, D exists.
	@param eSize			[in]	The public exponent size
	@param d_ptr			[in]	The private exponent vector. 
									This vector can be set to NULL if the public exponent, E exists.
	@param dSize			[in]	The private exponent size.
	@param keyResult_ptr	[out]	The result RSA key built 
	@param keyResultSize_ptr[out]	The size of the result RSA key
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSRSAKeyBuild (DxByte_t     *n_ptr,
												 DxUint32_t   nSize,
												 DxByte_t     *d_ptr,
												 DxUint32_t   dSize,
												 DxByte_t     *e_ptr, 
												 DxUint32_t   eSize,
												 SSTKeyRSA_t  *keyResult_ptr);

	
	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Get a RSA key parameters from a RSA key
	@param keyType			[in]	The type of key record (private, public or both)
	@param key_ptr		[in]	The RSA key used to extract the parameters
	@param n_ptr		[out]	The modulus vector output, if this pointer is set to NULL 
								the required size will be returned on the next parameter.
	@param nSize_ptr	[out]	The modulus vector size in bytes.
	@param e_ptr		[out]	The public exponent vector output. if this pointer is set 
								to NULL the required size will be returned on the next parameter
	@param eSize_ptr	[out]	The public exponent size.
	@param d_ptr		[out]	The private exponent vector output. if this pointer is set 
								to NULL the required size will be returned on the next parameter
	@param dSize_ptr	[out]	The private exponent size. 
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/

	SSTVCRYSReturnCodes_t SST_VCRYSRSAKeyGet ( SSTRSAKeyType_t	keyType,
												SSTKeyRSA_t		*key_ptr, 
												DxByte_t		*n_ptr, 
												DxUint32_t		*nSize_ptr, 
												DxByte_t		*d_ptr, 
												DxUint32_t		*dSize_ptr, 
												DxByte_t		*e_ptr, 
												DxUint32_t		*eSize_ptr);





	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Build a CRT-RSA key using parameters from user
	@param n_ptr	[out]	The modulus
	@param nSize_ptr[out]	The modulus size.
	@param e_ptr			[in] The public exponent vector output. if this pointer is set 
								 to NULL the required size will be returned on the next parameter
	@param eSize			[in] The public exponent size.
	@param p_ptr			[in] The first factor vector.
	@param pSize			[in] The first factor vector size.
	@param q_ptr			[in] The second factor vector.
	@param qSize			[in] The second factor vector size.
	@param dP_ptr			[in] The first factor exponent vector.
	@param dPSize			[in] The first factor exponent vector size.
	@param dQ_ptr			[in] The second factor exponent vector.
	@param dQSize			[in] The second factor exponent vector size.
	@param qInv_ptr			[in] The coefficient vector .
	@param qInvSize			[in] The coefficient vector size.
	@param keyResult_ptr	[out] The result CRT-RSA key built 
	@param keyResultSize_ptr[out] The size of the result CRT-RSA key
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/

	SSTVCRYSReturnCodes_t SST_VCRYSCRTRSAKeyBuild (	DxByte_t	*n_ptr, 
													DxUint32_t	nSize,
													DxByte_t	*e_ptr, 
													DxUint32_t	eSize,
													DxByte_t    *p_ptr,
													DxUint32_t  pSize,
													DxByte_t    *q_ptr,
													DxUint32_t  qSize,
													DxByte_t    *dP_ptr, 
													DxUint32_t  dPSize, 
													DxByte_t    *dQ_ptr, 
													DxUint32_t  dQSize,
													DxByte_t    *qInv_ptr,
													DxUint32_t  qInvSize,
													SSTKeyRSA_t *keyResult_ptr);


	/****** TODO – need to be implemented by the user  Where CRYS is not used******/
	/*!
	\brief 
	Get a CRT-RSA key parameters from a CRT-RSA key
	@param keyType	[in]	The type of key record (private, public or both)
	@param key_ptr	[in] The CRT-RSA key used to extract the parameters
	@param n_ptr	[out]	The modulus
	@param nSize_ptr[out]	The modulus size.
	@param e_ptr	[out] The public exponent vector output. if this pointer is set 
						  to NULL the required size will be returned on the next parameter
	@param eSize_ptr[out] The public exponent size.
	@param p_ptr	[out] The first factor vector.
	@param pSize	[out] The first factor vector size.
	@param q_ptr	[out] The second factor vector.
	@param qSize	[out] The second factor vector size.
	@param dP_ptr	[out] The first factor exponent vector.
	@param dPSize	[out] The first factor exponent vector size.
	@param dQ_ptr	[out] The second factor exponent vector.
	@param dQSize	[out] The second factor exponent vector size.
	@param qInv_ptr	[out] The coefficient vector .
	@param qInvSize	[out] The coefficient vector size.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/

	SSTVCRYSReturnCodes_t SST_VCRYSCRTRSAKeyGet (  SSTRSAKeyType_t keyType,
													SSTKeyRSA_t	*key_ptr,
													DxByte_t	*n_ptr, 
													DxUint32_t	*nSize_ptr,
													DxByte_t	*e_ptr, 
													DxUint32_t	*eSize_ptr,
													DxByte_t    *p_ptr,
													DxUint32_t  *pSize, 
													DxByte_t    *q_ptr, 
													DxUint32_t  *qSize,
													DxByte_t    *dP_ptr,
													DxUint32_t  *dPSize,
													DxByte_t    *dQ_ptr,
													DxUint32_t  *dQSize,
													DxByte_t    *qInv_ptr,
													DxUint32_t  *qInvSize);



#endif  /* _DX_SST_VCRYS_H_ */
