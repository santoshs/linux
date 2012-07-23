/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS header file						 					
*																			
* Filename:						sst_vcrys.h															
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
**********************************************************************************/

#ifndef _DX_SST_VCRYS_H_
    #define _DX_SST_VCRYS_H_
	
	#include "sst_vcrys_config.h"
	#include "sst_vcrys_def.h"
	#include "sst_debug.h"
	#include "sst_types.h"
		
	/*----------- Global defines -------------------------------------------------*/
	#define SST_VCRYS_MAC_BLOCK_SIZE_IN_BYTES			(16UL)
	#define SST_VCRYS_HASH_RESULT_SIZE_IN_WORDS			(16UL)
	#define SST_VCRYS_ENCRYPT_IV_COUNTER_SIZE_IN_BYTES	(16UL)
	#define SST_VCRYS_ECC_PK_MAX_SIZE_IN_BYTES			(148UL) /*(sizeof(CRYS_ECPKI_PublKey_t))*/
    /*----------- Global macro definitions ---------------------------------------*/
	
    /*----------- Global type definitions ----------------------------------------*/

	/*! \brief SST VCRYS Return Codes**/
	typedef enum
	{
		SST_VCRYS_RC_OK							 = 0,
		SST_VCRYS_RC_FAIL						 = 1,
		SST_VCRYS_RC_ERROR_NULL_POINTER			 = 2,
		SST_VCRYS_RC_ERROR_BAD_PARAM			 = 3,
		SST_VCRYS_RC_ERROR_VOS					 = 4,
		SST_VCRYS_RC_ERROR_CRYS					 = 5,
		SST_VCRYS_RC_ERROR_VDB					 = 6,
		SST_VCRYS_RC_ERROR_WORKSPACE_SIZE		 = 7,
		SST_VCRYS_RC_ERROR_NOT_SUPPORTED		 = 8,
		SST_VCRYS_RC_ERROR_SIGNITURE_SIZE		 = 9,
        SST_VCRYS_RC_ERROR_FATAL                 = 10, /**/

        /* vcrys_ext related errors - errors that might be received from crys_sst 
        due to internal calls to SST from within crys (SST-->CRYS_SST-->SST)*/
        SST_VCRYS_RC_ERROR_HANDLE_NOT_FREE                = 11, 
        SST_VCRYS_RC_ERROR_TXN_INVALID                    = 12,
        SST_VCRYS_RC_ERROR_HANDLE_MISMATCH                = 13,
        SST_VCRYS_RC_ERROR_BUFFER_NOT_ALIGN               = 15,
        SST_VCRYS_RC_ERROR_WORKSPACE_NOT_ALIGN            = 16,
        SST_VCRYS_RC_ERROR_INVALID_PARAM                  = 17,
        SST_VCRYS_RC_ERROR_FEATURE_NOT_SUPPORTED          = 18,
        SST_VCRYS_RC_ERROR_INTEGRITY_COMPROMIZED          = 19,
        SST_VCRYS_RC_ERROR_WORKSPACE                      = 20,
        SST_VCRYS_RC_ERROR_TIMEOUT                        = 21,
        SST_VCRYS_RC_ERROR_ACCESS_DENIED                  = 22,
        SST_VCRYS_RC_ERROR_SESSION_INVALID                = 23,
        SST_VCRYS_RC_ERROR_HANDLE_NOT_FOUND               = 24,
        SST_VCRYS_RC_ERROR_NVS_MAIN_MAX_SIZE_EXCEEDED     = 25,
        SST_VCRYS_RC_ERROR_NVS_TXN_MAX_SIZE_EXCEEDED      = 26,

        SST_VCRYS_RETURN_CODES_FORCE_INT32                = 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTVCRYSReturnCodes_t;

	/*! \brief SST VCRYS Compare result**/
	typedef enum
	{
		SST_VCRYS_CMP_ARG1_EQ_ARG2		= 0,
		SST_VCRYS_CMP_ARG1_GT_ARG2		= 1,
		SST_VCRYS_CMP_ARG1_LT_ARG2		= 2,

		SST_VCRYS_CMP_RESULT_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTVCRYSCmpResult_t;
	
	/*! \brief SST VCRYS Encrypt of Decrypt operation mode**/
	typedef enum 
	{
		SST_VCRYS_ENCRYPT	= 0,
		SST_VCRYS_DECRYPT	= 1,

		SST_VCRYS_ENCRYPT_MODE_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTVCRYSEncryptMode_t;


	/*! \brief SST VCRYS MAC Descriptor Entry, describes the offset and size 
	of the data to perform calculation on **/
	typedef struct 
	{
		DxUint32_t offset;
		DxUint32_t size;
	}SSTVCRYSMACDescriptorEntry_t;

	/*! \brief SST VCRYS MAC Descriptor array, where the array size is predefined in vcrys_config.h
		and its actual used size is detailed in usedCapacity**/
	typedef struct  
	{
		SSTVCRYSMACDescriptorEntry_t	descriptorArray[SST_VCRYS_NUM_DESCRIPTOR_ENTRIES];
		DxUint32_t						usedCapacity;

	}SSTVCRYSMACDescriptorsArr_t;
	
	/*! \brief SST VCRYS type of operation that require a workspace**/
	typedef enum  
	{
		SST_VCRYS_WORKSPACE_RSA_VERIFY = 0,
		SST_VCRYS_WORKSPACE_ECC_VERIFY = 1,

		SST_VCRYS_WORKSPACE_OPERATION_FORCE_INT32= 0x7FFFFFFF /* force enum to 32 bit in all compilers */

	}SSTVCRYSWorkspaceOperation_t;

	/*! \brief SST VCRYS MAC field, the size is predefined in vcrys_def.h**/
	typedef	DxByte_t SSTVCRYSMACfield_t[SST_VCRYS_MAC_BLOCK_SIZE_IN_BYTES];

	/*! \brief SST VCRYS HASH result buffer, the size is predefined in vcrys_def.h**/
	typedef DxUint32_t	SSTVCRYSHASHResult_t[SST_VCRYS_HASH_RESULT_SIZE_IN_WORDS];

	/*! \bruef SST VCRYS IV counter field for encrypt and decrypt operation**/
	typedef DxByte_t	SSTVCRYSIvCounter_t[SST_VCRYS_ENCRYPT_IV_COUNTER_SIZE_IN_BYTES];


    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/

    /*----------- Global function prototypes -------------------------------------*/
	
	
	/* Init functions*/

	/*!
	\brief 
	Initialize the VCRYS, includes creating\reading the keys used for integrity 
	and encryption operations
	@return SST_VCRYS_RC_OK				On success
	@return	SST_VCRYS_RC_FAIL			Operation failed 
	@return SST_VCRYS_RC_ERROR_CRYS		The CRYS operation failed
	@return SST_VCRYS_RC_ERROR_VDB		the VDB operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSInit		(void);
	
	/*!
	\brief 
	Terminate the VCRYS and free any used resources 
	@return SST_VCRYS_RC_OK			On success
	@return	SST_VCRYS_RC_FAIL		Operation failed 
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSTerminate	(void);

    /*!
    \brief 
    Initialize the globals and free any used resources 
    @return SST_VCRYS_RC_OK			On success    
    **/
    SSTVCRYSReturnCodes_t SST_VCRYSTerminate_dbg	(void);
	
	/*----------------------------------------------------------------------------*/
	/* Utility functions*/

	/*!
	\brief 
	returns the minimum workspace buffer size per VCRYS operation (that require a workspace)
	@param vcrysWsOp			[in]	The data on which the MAC will be calculated
	@param minWorkspaceSize_ptr	[out]		The data on which the MAC will be calculated
	@return SST_VCRYS_RC_OK					On success
	@return	SST_VCRYS_RC_FAIL				Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER	At least one of the arguments is null
	@return	SST_VCRYS_RC_ERROR_BAD_PARAM	Input parameter are illegal
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSUtilWorkspaceSizeGet(SSTVCRYSWorkspaceOperation_t vcrysWsOp,
														DxUint32_t		*minWorkspaceSize_ptr);
	/*----------------------------------------------------------------------------*/
	/* API functions*/
	/*!
	\brief 
	Start Calculation the MAC value of data in a buffer, using a secret key 
	
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@return SST_VCRYS_RC_ERROR_VOS				The VOS operation failed
	**/
	
	SSTVCRYSReturnCodes_t SST_VCRYSMACCalcStart (void);

	/*!
	\brief 
	Calculate the MAC value of data in a buffer, using a secret key 
	@param dataBuffer_ptr		[in]		The data on which the MAC will be calculated
	@param dataDescriptorArr	[in]		Pointer to an array of descriptors indicating on 
	what part of the data the MAC calculation should 
	be performed
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@return SST_VCRYS_RC_ERROR_VOS				The VOS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSMACCalcSingle (const DxByte_t		*dataBuffer_ptr, 
												  SSTVCRYSMACDescriptorsArr_t	*dataDescriptorArr);
	/*!
	\brief 
	Calculate the MAC value of data in a buffer, using a secret key 
	@param dataBuffer_ptr		[in]		The data on which the MAC will be calculated
	@param dataDescriptorArr	[in]		Pointer to an array of descriptors indicating on 
											what part of the data the MAC calculation should 
											be performed
	@param result_ptr			[out]		The result MAC of the data
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@return SST_VCRYS_RC_ERROR_VOS				The VOS operation failed
	**/
	
	SSTVCRYSReturnCodes_t SST_VCRYSMACCalc (const DxByte_t					*dataBuffer_ptr, 
											 SSTVCRYSMACDescriptorsArr_t	dataDescriptorArr,
											 SSTVCRYSMACfield_t				*result_ptr);


	/*!
	\brief 
	Finish Calculation of MAC value of data in a buffer, using a secret key 
	
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	
	SSTVCRYSReturnCodes_t SST_VCRYSMACCalcFinish (const DxByte_t		*dataBuffer_ptr, 
												  SSTVCRYSMACDescriptorsArr_t	*dataDescriptorArr,
												  SSTVCRYSMACfield_t * result_ptr);


	/*!
	\brief 
	Perform HASH operation for processing one buffer of data 
	@param dataIn_ptr		[in]	A pointer to the buffer that stores the data to be hashed.
	@param dataSize			[in]	The size of the data to be hashed in bytes. 
	@param hashResultBuff	[out]	A pointer to the target buffer where the 
									HASH result stored in the context is loaded to.
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@notes:	1. the HASH operation mode will be configurate in "sst_vcrys_config.h"
	**/
	SSTVCRYSReturnCodes_t	SST_VCRYSHash (	DxByte_t           *dataIn_ptr,
											DxUint32_t         dataSize,
											SSTVCRYSHASHResult_t hashResultBuff);

	/*!
	\brief 
	Perform RSA sign operation - NOT SUPPORTED
	@param modulus_ptr		[in]		A pointer to the modulus stream of bytes 
										(Big-Endian format).The MS bit must be set to '1'.
	@param privExponent_ptr [in]		A pointer to the private exponent stream 
										of bytes (Big-Endian format)
	@param dataIn_ptr		[in]		Pointer to the data to sign.
	@param dataInSize		[in]		The size, in bytes, of the data to sign.
	@param result_ptr		[out]		Pointer to the signature
	@param resultSize_ptr	[in\out]	Pointer to the Signature Size value (max size 264 byte)
										The input values the signature buffer size allocated 
										The output value is the signature size actually used 
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@notes:	1. size of modulus is 2048 bits
			2. the public exponent constant and equal to (2^16+1)
	**/
	SSTVCRYSReturnCodes_t	SST_VCRYSRSASign ( DxByte_t		*modulus_ptr,
												DxByte_t    *privExponent_ptr,
												DxByte_t	*dataIn_ptr, 
												DxUint32_t	dataInSize, 
												DxByte_t	*result_ptr, 
												DxUint32_t	*resultSize_ptr);



	/*!
	\brief 
	Perform RSA signature verify operation  
	@param modulus_ptr		[in]	A pointer to the modulus stream of bytes 
									(Big-Endian format).The MS bit must be set to '1'.
	@param pubExponent_ptr		[in] Pointer to the public exponent
	@param pubExponentSizeInByte[in] size of the public exponent
	@param dataIn_ptr		[in]	Pointer to the data to verify.
	@param dataInSize		[in]	The size, in bytes, of the data to verify.
	@param signature_ptr	[in]	Pointer to the signature to be verified.
	@param isSignatureValid [in]	Flag indicating if the signature is valid
	@param workspace_ptr	[in]	Pointer to workspace
	@param workspaceSize	[in]	Size of the workspace
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@notes:	1. size of modulus is 2048 bits
			2. the public exponent constant and equal to (2^16+1)
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSRSAVerify  (	DxByte_t	*modulus_ptr,
												DxByte_t	*pubExponent_ptr,
												DxUint32_t	pubExponentSizeInBytes,	
												DxByte_t	*dataIn_ptr, 
												DxUint32_t	dataInSize, 
												DxByte_t	*signature_ptr,
												DxBool_t	*isSignatureValid,
												DxByte_t	*workspace_ptr,
												DxUint32_t	workspaceSize);
	

		/*!
	\brief 
	Perform ECC sign operation - NOT SUPPORTED
	@param privKey_ptr		[in]		A pointer to the Private key
	@param privKeySizeinBytes [in]		The Size of the Private key
	@param dataIn_ptr		[in]		Pointer to the data to sign.
	@param dataInSize		[in]		The size, in bytes, of the data to sign.
	@param result_ptr		[out]		Pointer to the signature
	@param resultSize_ptr	[in\out]	Pointer to the Signature Size value 
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@notes:	1. Max size of private key 72 bytes
	**/
	SSTVCRYSReturnCodes_t	SST_VCRYSECCSign (  DxByte_t    *privKey_ptr,
												DxUint32_t	privKeySizeinBytes, 
												DxByte_t	*dataIn_ptr, 
												DxUint32_t	dataInSize, 
												DxByte_t	*result_ptr, 
												DxUint32_t	*resultSize_ptr);



	/*!
	\brief 
	Perform ECC signature verify operation  
	@param pubKey_ptr			[in]	Pointer to the public key
	@param pubKeySizeInBytes	[in]	size of the public key
	@param dataIn_ptr			[in]	Pointer to the data to verify.
	@param dataInSize			[in]	The size, in bytes, of the data to verify.
	@param signature_ptr		[in]	Pointer to the signature to be verified.
	@param signatureSizeInBytes [in]	Size of the signature to be verified.
	@param isSignatureValid		[in]	Flag indicating if the signature is valid
	@param workspace_ptr		[in]	Pointer to workspace
	@param workspaceSize		[in]	Size of the workspace
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	@notes:	1. Max size of public key is 148 bytes
			2. signature is constant and depends on the Domain ID of the ECC
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSECCVerify  (	DxByte_t	*pubKey_ptr,
                                                DxUint32_t	pubKeySizeInBytes,	
												DxByte_t	*dataIn_ptr, 
												DxUint32_t	dataInSize, 
												DxByte_t	*signature_ptr,
												DxUint32_t	signatureSizeInBytes,
												DxBool_t	*isSignatureValid,
												DxByte_t	*workspace_ptr,
												DxUint32_t	workspaceSize);


	/*!
	\brief 
	Perform initialization for the Encrypt or decrypt operation
	@param encryptDecryptFlag	[in]	The operation mode
	@param ivCounter			[in]	used to perform the operation
	@return SST_VCRYS_RC_OK				On success
	@return	SST_VCRYS_RC_FAIL			Operation failed 
	@return SST_VCRYS_RC_ERROR_CRYS		The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSCryptOpStart (SSTVCRYSEncryptMode_t	encryptDecryptFlag,
												 SSTVCRYSIvCounter_t ivCounter);

	/*! \brief Macros that the user should call when operating with crypt operations
	Macro SST_VCRYSEncryptStart is starting an encrypt operation
	Macro SST_VCRYSDecryptStart is starting a decrypt operation
	**/
	#define SST_VCRYSEncryptStart(ivCounter)	SST_VCRYSCryptOpStart(SST_VCRYS_ENCRYPT,ivCounter)
	#define SST_VCRYSDecryptStart(ivCounter)	SST_VCRYSCryptOpStart(SST_VCRYS_DECRYPT,ivCounter)
	
	/*!
	\brief 
	Encrypt or decrypt data  
	@param encryptDecryptFlag	[in]	The operation mode 
	@param dataIn_ptr			[in]	Pointer to the data
	@param dataInSize			[in]	The size, in bytes, of the data
	@param result_ptr			[out]	Pointer to the result data (can be in place)
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return	SST_VCRYS_RC_ERROR_BAD_PARAM		Input parameter are illegal
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSCryptOperation (SSTVCRYSEncryptMode_t	encryptDecryptFlag,
													const DxByte_t			*dataIn_ptr, 
													DxUint32_t				dataInSize, 
													DxByte_t				*result_ptr);

	/*! \brief Macros that the user should call when operating with crypt operations
	Macro SST_VCRYSEncryptStart is perform an encrypt operation
	Macro SST_VCRYSDecryptStart is perform a decrypt operation
	**/
	#define SST_VCRYSEncrypt(dataIn_ptr,dataInSize,result_ptr) \
			SST_VCRYSCryptOperation(SST_VCRYS_ENCRYPT,dataIn_ptr,dataInSize,result_ptr)

	#define SST_VCRYSDecrypt(dataIn_ptr,dataInSize,result_ptr) \
			SST_VCRYSCryptOperation(SST_VCRYS_DECRYPT,dataIn_ptr,dataInSize,result_ptr)
	

	/*!
	\brief 
	Finish Encrypt or decrypt operation
	@param encryptDecryptFlag	[in]	The operation mode 
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSCryptOpFinish ( SSTVCRYSEncryptMode_t	encryptDecryptFlag);

	/*! \brief Macros that the user should call when operating with crypt operations
	Macro SST_VCRYSEncryptFinish is finishing an encrypt operation
	Macro SST_VCRYSDecryptFinish is finishing a decrypt operation
	**/
	#define SST_VCRYSEncryptFinish()	SST_VCRYSCryptOpFinish(SST_VCRYS_ENCRYPT)
	
	#define SST_VCRYSDecryptFinish()	SST_VCRYSCryptOpFinish(SST_VCRYS_DECRYPT)


	/*!
	\brief 
	Decrypt a single block of data, encrypted in AES ECB mode
	@param keyBuff_ptr	[in]	The buffer in which the key to perform 
								this operation. the size of the key is 
								configurated in "sst_vcrys_config.h"
    @param aKeyType     [in]    A key size type (may be 128, 192 or 256 bits)
	@param dataIn_ptr	[in]	Pointer to the data
	@param dataInSize	[in]	The size, in bytes, of the data
	@param result_ptr	[out]	Pointer to the result data (can be in place)
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	@return	SST_VCRYS_RC_ERROR_BAD_PARAM		Input parameter are illegal
	@return SST_VCRYS_RC_ERROR_CRYS				The CRYS operation failed
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSSingleDecrypt (	DxByte_t            *keyBuff_ptr,
                                                    SSTAESKeyType_t      aKeyType,
													DxByte_t            *dataIn_ptr,
													DxUint32_t           dataInSize,
													DxByte_t            *result_ptr);


	/*!
	\brief 
	Generate a random sequence of bytes
	@param size					[in]	The size of the required random sequence of bytes
	@param result_ptr			[out]	The random sequence of bytes generated 
	@return SST_VCRYS_RC_OK						On success
	@return	SST_VCRYS_RC_FAIL					Operation failed 
	@return SST_VCRYS_RC_ERROR_NULL_POINTER		At least one of the arguments is null
	**/
	SSTVCRYSReturnCodes_t SST_VCRYSRandomGenerate (	DxUint16_t	size, 
													DxByte_t	*result_ptr);



	/*!
	\brief 
	Compare between 2 positive input arguments. The function execute in constant time
	regardless of the arguments (for a given size)
	@param arg1Size				[in]	The size of the first value in bytes
	@param arg1Buff_ptr			[in]	The first value
	@param arg2Size				[in]	The size of the second value in bytes
	@param arg2Buff_ptr			[in]	The second value
	@return SST_VCRYS_CMP_ARG1_EQ_ARG2		The values equal
	@return SST_VCRYS_CMP_ARG1_GT_ARG2		The first value is greater
	@return SST_VCRYS_CMP_ARG1_LT_ARG2		The second value is greater
	**/
	SSTVCRYSCmpResult_t SST_VCRYSCompare ( DxByte_t	*arg1Buff_ptr, 
											DxUint16_t  arg1Size, 
											DxByte_t	*arg2Buff_ptr,
											DxUint16_t  arg2Size);




#endif  /* _DX_SST_VCRYS_H_ */
