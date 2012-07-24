/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS Base Layer header file						 					
*																			
* Filename:						sst_vcrys_baselayer.h															
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
/****** TODO – need to be implemented by the user  Where CRYS is not used********/
#ifndef _DX_SST_VCRYS_BASELAYER_H_
    #define _DX_SST_VCRYS_BASELAYER_H_
	
	#include "CRYS.h"
	#include "CRYS_COMMON_math.h"
	#include "CRYS_RSA_error.h"

	/*----------- Global defines -------------------------------------------------*/

    /*----------- Global macro definitions ---------------------------------------*/
	/*******Return code definitions*********/
	/*!	\brief  sst crys return code when operation was successful**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RC_OK						CRYS_OK

	/*!\brief RSA_Verify return code if verification was inconsistent with given parameters **/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RC_RSA_INCONSISTENT_VERIFY CRYS_RSA_ERROR_PSS_INCONSISTENT_VERIFY

	/*!\brief ECC_Verify return code if verification was inconsistent with given parameters **/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RC_ECC_INCONSISTENT_VERIFY CRYS_ECDSA_VERIFY_INCONSISTENT_VERIFY_ERROR

	/*!\brief ECC_Verify return code if challenge size is invalid**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RC_ECC_SIGNATURE_SIZE_ERROR CRYS_ECDSA_VERIFY_INVALID_SIGNATURE_SIZE_ERROR

	/*******Size definitions*********/
	/*!\brief RSA verify salt length **/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RSA_VERIFY_SALT_LEN_IN_BYTES   CRYS_RSA_VERIFY_SALT_LENGTH_UNKNOWN

	/*!\brief Workspace minimum size in bytes used for RSA verify*/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RSA_VERIFY_WORKSPACE_MIN_SIZE_IN_BYTES						\
			(sizeof(CRYS_RSAPubUserContext_t)+sizeof(CRYS_RSAUserPubKey_t))

	/*!\brief Workspace minimum size in bytes used for RSA verify*/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_ECC_VERIFY_WORKSPACE_MIN_SIZE_IN_BYTES						\
			(sizeof(CRYS_ECDSA_VerifyUserContext_t)+sizeof(CRYS_ECPKI_UserPublKey_t))
	
	/*******Function definitions*********/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	/*!	\brief  This function is used to initialize the AES MAC machine	**/
	#define	SST_CRYS_AES_MAC_INIT(contextID_ptr,iVCounter_ptr,key_ptr)\
			CRYS_AES_Init(contextID_ptr,iVCounter_ptr,key_ptr,\
							CRYS_AES_Key128BitSize,CRYS_AES_Encrypt,CRYS_AES_CMAC_mode)

	/*!	\brief  This function is used to initialize the AES Encrypt machine	**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define	SST_CRYS_AES_ENCRYPT_INIT(contextID_ptr,iVCounter_ptr,key_ptr)\
			CRYS_AES_Init(contextID_ptr,iVCounter_ptr,key_ptr,\
						CRYS_AES_Key128BitSize,CRYS_AES_Encrypt,CRYS_AES_CBC_mode)

	/*!	\brief  This function is used to initialize the AES Decrypt machine	**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define	SST_CRYS_AES_DECRYPT_INIT(contextID_ptr,iVCounter_ptr,key_ptr)\
			CRYS_AES_Init(contextID_ptr,iVCounter_ptr,key_ptr,\
							CRYS_AES_Key128BitSize,CRYS_AES_Decrypt,CRYS_AES_CBC_mode)
	
	/*! \brief  This function is used to process a block on the AES machine **/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_AES_BLOCK(contextID_ptr,dataIn_ptr,dataInSize,dataOut_ptr)\
			CRYS_AES_Block(contextID_ptr,(DxUint8_t*)dataIn_ptr,dataInSize,dataOut_ptr)

	/*! \brief  This function is used to end the AES processing session**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
#ifdef SST_ON_SEP
#define SST_CRYS_AES_FREE(contextID_ptr) 0x0
#else
#define SST_CRYS_AES_FREE(contextID_ptr) 0x0;\
	    DX_VOS_MemSetZero((void*)contextID_ptr, sizeof(CRYS_AESUserContext_t) )
#endif

#ifdef CRYS_AES_CF_VERSION_USED 
/*! \brief  This function is used to end the AES CMAC processing session**/
   /****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_AES_FINISH( contextID_ptr ,dataIn, DataInSize, MacResult ) \
    CRYS_AES_Finish( contextID_ptr ,(dataIn) ,(DataInSize) ,(DxUint8_t *)MacResult )
#else
	#define SST_CRYS_AES_FINISH( contextID_ptr ,dataIn, DataInSize, MacResult ) \
    SST_CRYS_AES_BlockAndFinish( contextID_ptr ,(dataIn) ,(DataInSize) ,(DxUint8_t *)MacResult );
    
#endif

	/*! \brief operating the AES in one operation**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/


#define SST_CRYS_DECRYPT_SINGLE(key_ptr,keyType,dataIn_ptr,dataInSize,dataOut_ptr) \
			CRYS_AES(DX_NULL,key_ptr,keyType,CRYS_AES_Decrypt,CRYS_AES_ECB_mode,\
					 dataIn_ptr,dataInSize,dataOut_ptr)

	/*! \brief operating the AES Wrap with secret key in one operation**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
#ifdef DX_RKEK_SIZE_128	
    #define SST_CRYS_AES_WRAP(DataIn_ptr,DataInLen,WrapDataOut_ptr,WrapDataLen_ptr)\
			CRYS_AES_Wrap(DataIn_ptr,DataInLen,DX_NULL,CRYS_AES_Key128BitSize,DX_TRUE,\
							WrapDataOut_ptr,WrapDataLen_ptr)
#else
    #define SST_CRYS_AES_WRAP(DataIn_ptr,DataInLen,WrapDataOut_ptr,WrapDataLen_ptr)\
			CRYS_AES_Wrap(DataIn_ptr,DataInLen,DX_NULL,CRYS_AES_Key256BitSize,DX_TRUE,\
							WrapDataOut_ptr,WrapDataLen_ptr)
#endif

	/*! \brief operating the AES Unwrap with secret key in one operation**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
#ifdef DX_RKEK_SIZE_128	
	#define SST_CRYS_AES_UNWRAP(WrapDataIn_ptr,WrapDataInLen,DataOut_ptr,DataOutLen_ptr)\
			CRYS_AES_Unwrap(WrapDataIn_ptr,WrapDataInLen,DX_NULL,CRYS_AES_Key128BitSize,DX_TRUE,\
							DataOut_ptr,DataOutLen_ptr)
#else
	#define SST_CRYS_AES_UNWRAP(WrapDataIn_ptr,WrapDataInLen,DataOut_ptr,DataOutLen_ptr)\
			CRYS_AES_Unwrap(WrapDataIn_ptr,WrapDataInLen,DX_NULL,CRYS_AES_Key256BitSize,DX_TRUE,\
							DataOut_ptr,DataOutLen_ptr)
#endif

	/*!\brief This function provides a HASH function for processing one buffer of data**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_HASH(dataIn_ptr,dataSize,hashResultBuff) \
			CRYS_HASH(CRYS_HASH_SHA256_mode,dataIn_ptr,dataSize,hashResultBuff)

	/*!\brief populates a CRYSRSAPubKey_t structure with the provided modulus and exponent**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RSA_BUILD_PUBLIC_KEY(UserPubKey_ptr,pubExponent_ptr,pubExponentSizeInBytes,\
			modulus_ptr,modulusSize) \
			CRYS_RSA_Build_PubKey(UserPubKey_ptr,pubExponent_ptr,pubExponentSizeInBytes,\
			modulus_ptr,modulusSize)

	/*!\brief RSA_Verify implements the RSASSA-PKCS1v15 algorithm in a single function, 
	as defined in PKCS#1 v1.21**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RSA_VERIFY(userContext_ptr,userPubKey_ptr,dataIn_ptr,dataInSize,signature_ptr) \
			CRYS_RSA_PSS_Verify(userContext_ptr,userPubKey_ptr,CRYS_RSA_HASH_SHA1_mode,\
								CRYS_PKCS1_MGF1,SST_CRYS_RSA_VERIFY_SALT_LEN_IN_BYTES,\
								dataIn_ptr,dataInSize,signature_ptr)


	/*!\brief Builds (imports) the user public key structure from an existing public key
    so that this structure can be used by other EC primitives**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_ECC_BUILD_PUBLIC_KEY(publKeyIn_ptr, publKeySizeInBytes, userPublKey_ptr) \
			CRYS_ECPKI_BuildPublKey(CRYS_ECPKI_DomainID_secp256r1,publKeyIn_ptr, \
									publKeySizeInBytes, userPublKey_ptr)
        
	/*!\brief Performs all ECDSA verifying operation**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_ECC_VERIFY(verifyUserContext_ptr,userPublKey_ptr,signatureIn_ptr, \
								signatureSizeBytes,messageDataIn_ptr,messageSizeInBytes) \
			CRYS_ECDSA_Verify(verifyUserContext_ptr,userPublKey_ptr,CRYS_ECPKI_HASH_SHA256_mode,\
							  signatureIn_ptr,signatureSizeBytes,messageDataIn_ptr,messageSizeInBytes)

	/*!\brief generates a random vector**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_RND_GENERATE_VECTOR(size,result_ptr) \
			CRYS_RND_GenerateVector(size,result_ptr)

	/*!\brief This function compares a value of 2 large counter presented in a byte buffer**/
	/****** TODO – need to be implemented by the user  Where CRYS is not used********/
	#define SST_CRYS_COMPARE(arg1Buff_ptr,arg1Size,arg2Buff_ptr,arg2Size) \
			CRYS_COMMON_CmpMsbUnsignedCounters(arg1Buff_ptr,arg1Size,arg2Buff_ptr,arg2Size)


    /*----------- Global type definitions ----------------------------------------*/

	/*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/

    /*----------- Global function prototypes -------------------------------------*/
	
	
#endif  /* _DX_SST_VCRYS_BASELAYER_H_ */
