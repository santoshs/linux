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
 
 #ifndef CRYS_RSA_SEP_SCHEMES_H
#define CRYS_RSA_SEP_SCHEMES_H

#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_RSA_Types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  %Object name    :  CRYS_RSA_SEP_SCHEMES.h
 *  State           :  %state%
 *  Creation date   :  18 Apr 2007
 *  Last modified   :  %modify_time%
 */  
/** @file
 * \brief This module defines the API that supports PKCS#1 v1.5 and PKCS#1 v2.1 schemes
 *        with multi LLI DMA interface 
 *
 * \version 
 * \author ronys ohads (added multi LLI DMA interface - R.Levin)
 */
 

/**********************************************************************************************************/
/**
   \brief CRYS_RSA_SEP_SignUpdate processes the data to be signed
          with multi LLI DMA interface.
  
	The actual macro that will be used by the users is (one of):
	CRYS_RSA_SEP_PSS_SignUpdate
	CRYS_RSA_SEP_PKCS1v15_SignUpdate

   \note CRYS_RSA_SEP_SignUpdate can be called multiple times
   with some blcks of data 

   @param[in] UserContext_ptr - A pointer to a valid context,
                            as returned by CRYS_RSA_PSS_SignInit.
   @param[in] InLliTabEntries - The count of entries in the input LLI table.
   @param[in] DataSize - The size, in bytes, of the data whose
                         signature is to be verified.
   @param[in] DataInSize - The size, in bytes, of the data to sign.

   @return CRYSError_t - CRYS_OK, CRYS_BAD_PARAM
*/

CIMPORT_C CRYSError_t _DX_RSA_SEP_SignUpdate(CRYS_RSAPrivUserContext_t *UserContext_ptr,
							                 DxUint8_t                 *intDataHead_ptr,     
							                 DxUint32_t                intDataHeadSize, 
							                 DxUint8_t                 *intDataTail_ptr,     
							                 DxUint32_t                intDataTailSize, 
							                 DxUint32_t                *DataInLliTab_ptr,
							                 DxUint32_t                InLliTabEntries,
							                 DxUint32_t                DataInSize );


	
			
/**********************************************************************************************************/
/**
   @brief
   RSA_Sign implements the Signing algorithm as defined
   in PKCS#1 v1.5 or v2.1, using a single function with multi LLI DMA interface.

   The actual macros that will be used by the user are:
   CRYS_RSA_SEP_PKCS1v15_Sign		- for v1.5
   CRYS_RSA_SEP_PKCS1v15_SHA1_Sign	- for v1.5 data is hashed with SHA1
   CRYS_RSA_SEP_PKCS1v15_MD5_Sign	- for v1.5 data is hashed with MD5
   CRYS_RSA_SEP_PSS_Sign			- for v2.1
   CRYS_RSA_SEP_PSS_SHA1_Sign		- for v2.1 data is hashed with SHA1
					- for v2.1 MD5 is not supported, since 
					according to the PKCS#1 ver2.1 it is not recommended
   
   @param[in] UserContext_ptr - A pointer to a Context. For the use of the 
   								function as a space to work on.
   @param[in] UserPrivKey_ptr - A pointer to the private key data
                            structure of the user. \note The representation
                            (pair or quintuple) and hence the
                            algorithm (CRT or not) is determined
                            by the Private Key data
                            structure - using CRYS_Build_PrivKey
                            or CRYS_Build_PrivKeyCRT determines
                            which algorithm will be used.
   @param[in] hashFunc - The hash function to be used. Currently
                         only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
                         are supported. 
   @param[in] MGF - The mask generation function. Only for PKCS#1 v2.1
                    defines MGF1, so the only value allowed for v2.1 
                    is CRYS_PKCS1_MGF1. 
   @param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
   						Typical lengths are 0 and hLen (20 for SHA1)
   						The maximum length allowed is NSize - hLen - 2                                                                                       
   @param[in] InLliTabEntries - The count of entries in the input LLI table.
   @param[in] DataSize - The size, in bytes, of the data whose
                         signature is to be verified.
   @param[in] DataInSize - The size, in bytes, of the data to sign.
   @param[out] Output_ptr - A pointer to the signature. 
                            The buffer must be at least PrivKey_ptr->N.len bytes long 
                            (that is, the size of the modulus in bytes).
   @param[in,out] OutputSize_ptr - A pointer to the Signature Size value - the input value
                            is the signature buffer size allocated, the output value is 
                            the signature size actually used.
                            The buffer must be at least PrivKey_ptr->N.len bytes long 
                            (that is, the size of the modulus in bytes).
   @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
      
   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_USER_CONTEXT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR,
                         CRYS_RSA_HASH_ILLEGAL_OPERATION_MODE_ERROR,
                         CRYS_RSA_MGF_ILLEGAL_ARG_ERROR,
                         CRYS_RSA_PKCS1_VER_ARG_ERROR,
                         CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
                         CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
                         CRYS_RSA_INVALID_SIGNATURE_BUFFER_SIZE
*/

CIMPORT_C CRYSError_t _DX_RSA_SEP_Sign(
                         CRYS_RSAPrivUserContext_t *UserContext_ptr,
                         CRYS_RSAUserPrivKey_t     *UserPrivKey_ptr,
                         CRYS_RSA_HASH_OpMode_t     hashFunc,
                         CRYS_PKCS1_MGF_t           MGF,
                         DxUint16_t                 SaltLen,				  			      
                         DxUint32_t                *DataInLliTab_ptr,
                         DxUint32_t                 InLliTabEntries,
                         DxUint32_t                 DataInSize,
                         DxUint8_t                 *Output_ptr,
                         DxUint16_t                *OutputSize_ptr,
                         CRYS_PKCS1_version         PKCS1_ver );


/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5

   This function combines the RSASP1 signature primitive and the
   EMSA-PKCS1v15 encoding method, to provide an RSA-based signature scheme. 
   For more details, please refer to the PKCS#1 standard.
  
*/	
	
#define CRYS_RSA_SEP_PKCS1v15_Sign(UserContext_ptr,UserPrivKey_ptr,hashFunc,DataInLliTab_ptr,InLliTabEntries,DataInSize,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign((UserContext_ptr),(UserPrivKey_ptr),(hashFunc),(CRYS_PKCS1_NO_MGF),0,(DataInLliTab_ptr),(InLliTabEntries),(DataInSize),(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)
		

/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_SHA1_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-1
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA1_Sign(UserContext_ptr,UserPrivKey_ptr,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign((UserContext_ptr),(UserPrivKey_ptr),(CRYS_RSA_After_SHA1_mode),(CRYS_PKCS1_NO_MGF),0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		

/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_MD5_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using MD5
   
   Note: The data_in size is already known after the Hash
*/

#define CRYS_RSA_SEP_PKCS1v15_MD5_Sign(UserContext_ptr,UserPrivKey_ptr,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign((UserContext_ptr),(UserPrivKey_ptr),CRYS_RSA_After_MD5_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_MD5_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)


/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_SHA224_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-224
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA224_Sign(UserContext_ptr,UserPrivKey_ptr,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign((UserContext_ptr),(UserPrivKey_ptr),(CRYS_RSA_After_SHA224_mode),(CRYS_PKCS1_NO_MGF),0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		




/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_SHA256_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-256
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA256_Sign(UserContext_ptr,UserPrivKey_ptr,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign((UserContext_ptr),(UserPrivKey_ptr),(CRYS_RSA_After_SHA256_mode),(CRYS_PKCS1_NO_MGF),0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		

/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_SHA1_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-384
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA384_Sign(UserContext_ptr,UserPrivKey_ptr,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_Sign((UserContext_ptr),(UserPrivKey_ptr),(CRYS_RSA_After_SHA384_mode),(CRYS_PKCS1_NO_MGF),0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		


/**
   @brief
   CRYS_RSA_SEP_PKCS1v15_SHA512_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-512
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA512_Sign(UserContext_ptr,UserPrivKey_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign((UserContext_ptr),(UserPrivKey_ptr),(CRYS_RSA_After_SHA512_mode),(CRYS_PKCS1_NO_MGF),0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		



/**
   \brief CRYS_RSA_SEP_PSS_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1, in a single function call.

    Note: According to the PKCS#1 ver2.1 it is not recommended to use MD5 Hash, 
		therefore it is not supported
   
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PSS_Sign
*/	

#define CRYS_RSA_SEP_PSS_Sign(UserContext_ptr,UserPrivKey_ptr,hashFunc,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,DataInSize,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign(UserContext_ptr,UserPrivKey_ptr,hashFunc,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),(DataInSize),(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER21)	


/**
   \brief CRYS_RSA_PSS_SHA1_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-1

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_RSA_PSS_SHA1_Sign
*/	

#define CRYS_RSA_SEP_PSS_SHA1_Sign(UserContext_ptr,UserPrivKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign(UserContext_ptr,UserPrivKey_ptr,CRYS_RSA_After_SHA1_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),(CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   \brief CRYS_RSA_PSS_SHA224_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-224

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_RSA_PSS_SHA224_Sign
*/	

#define CRYS_RSA_SEP_PSS_SHA224_Sign(UserContext_ptr,UserPrivKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign(UserContext_ptr,UserPrivKey_ptr,CRYS_RSA_After_SHA224_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),(CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   \brief CRYS_RSA_SEP_PSS_SHA256_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-256

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PSS_SHA256_Sign
*/	

#define CRYS_RSA_SEP_PSS_SHA256_Sign(UserContext_ptr,UserPrivKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign(UserContext_ptr,UserPrivKey_ptr,CRYS_RSA_After_SHA256_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),(CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	



/**
   \brief CRYS_RSA_SEP_PSS_SHA384_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-384

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PSS_SHA384_Sign
*/	

#define CRYS_RSA_SEP_PSS_SHA384_Sign(UserContext_ptr,UserPrivKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign(UserContext_ptr,UserPrivKey_ptr,CRYS_RSA_After_SHA384_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),(CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   \brief CRYS_RSA_SEP_PSS_SHA512_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-512

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PSS_SHA512_Sign
*/	

#define CRYS_RSA_SEP_PSS_SHA512_Sign(UserContext_ptr,UserPrivKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Output_ptr,OutputSize_ptr)\
		_DX_RSA_SEP_Sign(UserContext_ptr,UserPrivKey_ptr,CRYS_RSA_After_SHA512_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),(CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


		

/**********************************************************************************************************/
/** 
   \brief RSA_VerifyUpdate performs the update phase for the Verify algorithm either for 
   PKCS1 Ver 1.5 or Ver 2.1 using DMA multi LLI interface.
   

   @param[in] UserContext_ptr - Pointer to a valid context as
                            returned by CRYS_RSA_VerifyInit.
   @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
                           chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
   @param[in] InLliTabEntries - The count of entries in the input LLI table.
   @param[in] DataSize - The size, in bytes, of the data whose
                         signature is to be verified.

   @return CRYSError_t - CRYS_OK, CRYS_BAD_PARAM
   
   NOTE: In case that input data is HASH digest of message (means we not need to perform HASH), all input 
         data must be plased in the first LLI entry.
*/
CIMPORT_C CRYSError_t _DX_RSA_SEP_VerifyUpdate(CRYS_RSAPubUserContext_t  *UserContext_ptr,
								               DxUint8_t                 *intDataHead_ptr,     
								               DxUint32_t                intDataHeadSize, 
								               DxUint8_t                 *intDataTail_ptr,     
								               DxUint32_t                intDataTailSize, 
                                               DxUint32_t                *DataInLliTab_ptr,
                                               DxUint32_t                 InLliTabEntries,
                                               DxUint32_t                 DataInSize );
		
		



/**********************************************************************************************************/
/**
   \brief CRYS_RSA_PKCS1v15_Verify implements the RSASSA-PKCS1v15 algorithm
   as defined in PKCS#1 v2.1 9.2 in one integrated function using DMA multi LLI interface.

	The actual macro that will be used by the users is:
	CRYS_RSA_PKCS1v15_Verify
	
   @param[in] UserContext_ptr - Pointer to the public Context, for the use of the 
   								function as a space to work on			
   @param[in] UserPubKey_ptr - Pointer to the public key data
                           structure of the user.
   @param[in] hashFunc - The hash function to be used. Currently
                         only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
                         are supported. The other hash functions
                         recommended by PKCS#1 v2.1 are
                         SHA-256/284/512. 
   @param[in] MGF - the mask generation function. only for PKCS#1 v2.1
                    defines MGF1, so the only value allowed for v2.1 
                    is CRYS_PKCS1_MGF1.
   @param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
   						Typical lengths are 0 and hLen (20 for SHA1)
   						The maximum length allowed is NSize - hLen - 2                                              
   @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
                           chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
   @param[in] InLliTabEntries - The count of entries in the input LLI table.
   @param[in] DataInSize - The size, in bytes, of the data whose
                         signature is to be verified.
   @param[in] Sig_ptr - Pointer to the signature to be
                        verified. The length of the signature is
                        PubKey_ptr->N.len bytes (that is, the
                        size of the modulus, in bytes).
   @param[in] PKCS1_ver - Ver 1.5 or 2.1 according to the functionality required

   @return CRYSError_t - CRYS_OK, CRYS_INVALID_SIGNATURE,
                         CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE 
*/
CIMPORT_C CRYSError_t _DX_RSA_SEP_Verify(
                           CRYS_RSAPubUserContext_t *UserContext_ptr,
                           CRYS_RSAUserPubKey_t     *UserPubKey_ptr,
                           CRYS_RSA_HASH_OpMode_t    hashFunc,
                           CRYS_PKCS1_MGF_t          MGF,
                           DxUint16_t                SaltLen,
                           DxUint32_t               *DataInLliTab_ptr,
                           DxUint32_t                InLliTabEntries,
                           DxUint32_t                DataInSize,
                           DxUint8_t                *Sig_ptr,
                           CRYS_PKCS1_version        PKCS1_ver );

				     
#define CRYS_RSA_SEP_PKCS1v15_Verify(UserContext_ptr,UserPubKey_ptr,hashFunc,DataInLliTab_ptr,InLliTabEntries,DataInSize,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,hashFunc,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),DataInSize,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_RSA_SEP_PKCS1v15_MD5_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes the DataIn_ptr data has already been hashed using MD5
   
   \note The data_in size is already known after the Hash
   
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_MD5_Verify   
*/

#define CRYS_RSA_SEP_PKCS1v15_MD5_Verify(UserContext_ptr,UserPubKey_ptr,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_MD5_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_MD5_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_RSA_SEP_PKCS1v15_SHA1_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA1
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PKCS1v15_SHA1_Verify   
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA1_Verify(UserContext_ptr,UserPubKey_ptr,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA1_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_RSA_SEP_PKCS1v15_SHA224_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA224
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PKCS1v15_SHA224_Verify   
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA224_Verify(UserContext_ptr,UserPubKey_ptr,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA224_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)



/**
   \brief CRYS_RSA_SEP_PKCS1v15_SHA256_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA256
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PKCS1v15_SHA256_Verify   
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA256_Verify(UserContext_ptr,UserPubKey_ptr,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA256_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_RSA_SEP_PKCS1v15_SHA384_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA384
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PKCS1v15_SHA384_Verify   
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA384_Verify(UserContext_ptr,UserPubKey_ptr,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA384_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)



/**
   \brief CRYS_RSA_SEP_PKCS1v15_SHA512_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA512
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_SEP_PKCS1v15_SHA512_Verify   
*/
#define CRYS_RSA_SEP_PKCS1v15_SHA512_Verify(UserContext_ptr,UserPubKey_ptr,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA512_mode,CRYS_PKCS1_NO_MGF,0,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_RSA_SEP_PKCS1v15_SHA1_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr has already been hashed using SHA1
   
   Note: The data_in size is already known after Hash
   Note: According to the PKCS#1 ver2.1 it is not recommended to use MD5 Hash, 
		therefore it is not supported
	      
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_SHA1_Verify   
*/
		
#define CRYS_RSA_SEP_PSS_Verify(UserContext_ptr,UserPubKey_ptr,hashFunc,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,DataInSize,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,hashFunc,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),DataInSize,Sig_ptr,CRYS_PKCS1_VER21)
			   
			      
/**
   \brief CRYS_RSA_SEP_PSS_SHA1_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA1
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_SHA1_Verify   
*/

#define CRYS_RSA_SEP_PSS_SHA1_Verify(UserContext_ptr,UserPubKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA1_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)			      				      	      


/**
   \brief CRYS_RSA_SEP_PSS_SHA224_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA224
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_SHA224_Verify   
*/

#define CRYS_RSA_SEP_PSS_SHA224_Verify(UserContext_ptr,UserPubKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA224_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)

/**
   \brief CRYS_RSA_SEP_PSS_SHA256_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA256
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_SHA256_Verify   
*/

#define CRYS_RSA_SEP_PSS_SHA256_Verify(UserContext_ptr,UserPubKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA256_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)			      


/**
   \brief CRYS_RSA_SEP_PSS_SHA384_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA384
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_SHA384_Verify   
*/

#define CRYS_RSA_SEP_PSS_SHA384_Verify(UserContext_ptr,UserPubKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA384_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)


/**
   \brief CRYS_RSA_SEP_PSS_SHA512_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA512
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_RSA_SEP_PKCS1v15_SHA512_Verify   
*/

#define CRYS_RSA_SEP_PSS_SHA512_Verify(UserContext_ptr,UserPubKey_ptr,MGF,SaltLen,DataInLliTab_ptr,InLliTabEntries,Sig_ptr)\
		_DX_RSA_SEP_Verify(UserContext_ptr,UserPubKey_ptr,CRYS_RSA_After_SHA512_mode,MGF,SaltLen,(DataInLliTab_ptr),(InLliTabEntries),CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)

/**********************************************************************************************************/

/**********************************************************************************************************/
/**
\brief CRYS_RSA_PSS_SignUpdate processes the data to be signed
in a given context.

\note RSA_PSS_SignUpdate can be called multiple times
with data 

The actual macro that will be used by the users is (one of):
CRYS_RSA_PSS_SignUpdate
CRYS_RSA_PKCS1v15_SignUpdate

\note CRYS_RSA_PSS_SignUpdate can be called multiple times
with data 

@param[in] UserContext_ptr - A pointer to a valid context,
as returned by CRYS_RSA_PSS_SignInit.
@param[in] DataIn_ptr - A pointer to the data to sign.
@param[in] DataInSize - The size, in bytes, of the data to sign.

@return CRYSError_t - CRYS_OK, CRYS_BAD_PARAM
*/

CEXPORT_C CRYSError_t _DX_RSA_SignUpdate_ExtApp(CRYS_RSAPrivUserContext_t *UserContext_ptr,
												DxUint8_t     *DataIn_ptr,
												DxUint8_t     *SepDataIn_ptr,
												DxUint32_t     DataInSize);

/**********************************************************************************************************/
/**
@brief
RSA_Sign implements the Signing algorithm as defined
in PKCS#1 v1.5 or v2.1, using a single function

The actual macros that will be used by the user are:
CRYS_RSA_PKCS1v15_Sign		- for v1.5
CRYS_RSA_PKCS1v15_SHA1_Sign	- for v1.5 data is hashed with SHA1
CRYS_RSA_PKCS1v15_MD5_Sign	- for v1.5 data is hashed with MD5
CRYS_RSA_PSS_Sign			- for v2.1
CRYS_RSA_PSS_SHA1_Sign		- for v2.1 data is hashed with SHA1
- for v2.1 MD5 is not supported, since 
according to the PKCS#1 ver2.1 it is not recommended

@param[in] UserContext_ptr - A pointer to a Context. For the use of the 
function as a space to work on.
@param[in] UserPrivKey_ptr - A pointer to the private key data
structure of the user. \note The representation
(pair or quintuple) and hence the
algorithm (CRT or not) is determined
by the Private Key data
structure - using CRYS_Build_PrivKey
or CRYS_Build_PrivKeyCRT determines
which algorithm will be used.
@param[in] hashFunc - The hash function to be used. Currently
only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
are supported. 
@param[in] MGF - The mask generation function. Only for PKCS#1 v2.1
defines MGF1, so the only value allowed for v2.1 
is CRYS_PKCS1_MGF1. 
@param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
Typical lengths are 0 and hLen (20 for SHA1)
The maximum length allowed is NSize - hLen - 2                                                                                       
@param[in] DataIn_ptr - A pointer to the data to sign.
@param[in] DataInSize - The size, in bytes, of the data to sign.
@param[out] Output_ptr - A pointer to the signature. 
The buffer must be at least PrivKey_ptr->N.len bytes long 
(that is, the size of the modulus in bytes).
@param[in,out] OutputSize_ptr - A pointer to the Signature Size value - the input value
is the signature buffer size allocated, the output value is 
the signature size actually used.
The buffer must be at least PrivKey_ptr->N.len bytes long 
(that is, the size of the modulus in bytes).
@param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required

@return CRYSError_t - CRYS_OK,
CRYS_RSA_INVALID_USER_CONTEXT_POINTER_ERROR,
CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR,
CRYS_RSA_HASH_ILLEGAL_OPERATION_MODE_ERROR,
CRYS_RSA_MGF_ILLEGAL_ARG_ERROR,
CRYS_RSA_PKCS1_VER_ARG_ERROR,
CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
CRYS_RSA_INVALID_SIGNATURE_BUFFER_SIZE
*/


CEXPORT_C CRYSError_t _DX_RSA_Sign_ExtApp(CRYS_RSAPrivUserContext_t *UserContext_ptr,
										  CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,
										  CRYS_RSA_HASH_OpMode_t hashFunc,
										  CRYS_PKCS1_MGF_t MGF,
										  DxUint16_t SaltLen,				  				  			      
										  DxUint8_t     *DataIn_ptr,
										  DxUint8_t     *SepDataIn_ptr,
										  DxUint32_t     DataInSize,
										  DxUint8_t     *Output_ptr,
										  DxUint16_t    *OutputSize_ptr,
										  CRYS_PKCS1_version PKCS1_ver);

/**********************************************************************************************************/
/**
\brief RSA_VerifyUpdate processes the data to be verified
in a given context, according to PKCS1 v1.5 and 2.1

\note RSA_VerifyUpdate can be called multiple times with data

The actual macro that will be used by the user is:
CRYS_RSA_PKCS1v15_VerifyUpdate		- for v1.5
CRYS_RSA_PSS_VerifyUpdate			- for v2.1

@param[in] UserContext_ptr - A pointer to the public Context
structure of the User.
@param[in] DataIn_ptr - A pointer to the data whose signature is
to be verified.
@param[in] DataInSize - The size, in bytes, of the data whose
signature is to be verified.

@return CRYSError_t - CRYS_OK, CRYS_INVALID_SIGNATURE,
CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE 
*/

CEXPORT_C CRYSError_t _DX_RSA_VerifyUpdate_ExtApp(CRYS_RSAPubUserContext_t *UserContext_ptr,
										   DxUint8_t     *DataIn_ptr,
										   DxUint8_t     *SepDataIn_ptr,
										   DxUint32_t     DataInSize);


/**********************************************************************************************************/
/**
\brief RSA_Verify implements the RSASSA-PKCS1v15 algorithm
in a single function, as defined in PKCS#1 v1.5 and v2.1

The actual macro that will be used by the users is:
CRYS_RSA_PKCS1v15_Verify			- for v1.5
CRYS_RSA_PKCS1v15_MD5_Verify		- for v1.5 data in is hashed with MD5
CRYS_RSA_PKCS1v15_SHA1_Verify		- for v1.5 data in is hashed with SHA1
CRYS_RSA_PSS_Verify					- for v2.1
CRYS_RSA_PSS_SHA1_Verify			- for v2.1 data in is hashed with SHA1
- for v2.1 MD5 is not supported, since
according to the PKCS#1 ver2.1 it is not recommended	

@param[in] UserContext_ptr - A pointer to the public Context, 
for the use of the function as a space to work on
@param[in] UserPubKey_ptr - A pointer to the public key data
structure of the user.
@param[in] hashFunc - The hash function to be used. 
Currently only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
are supported. The other hash functions
recommended by PKCS#1 v2.1 are
SHA-256/284/512.  If PKCS1_ver is CRYS_PKCS1_VER15, then
the value CRYS_RSA_After_HASH_NOT_KNOWN_mode will allow
the signature data to determine the hash function to be used.
@param[in] MGF - The mask generation function. only for PKCS#1 v2.1
defines MGF1, so the only value allowed for v2.1 
is CRYS_PKCS1_MGF1. 
@param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
Typical lengths are 0 and hLen (20 for SHA1)
The maximum length allowed is NSize - hLen - 2                                             
@param[in] DataIn_ptr - A pointer to the data whose signature is
to be verified.
@param[in] DataInSize - The size, in bytes, of the data whose
signature is to be verified.
@param[in] Sig_ptr - A pointer to the signature to be verified. 
The length of the signature is PubKey_ptr->N.len bytes 
(that is, the size of the modulus, in bytes).
@param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required

@return CRYSError_t - CRYS_OK, CRYS_INVALID_SIGNATURE,
CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE 
*/

CEXPORT_C CRYSError_t _DX_RSA_Verify_ExtApp(CRYS_RSAPubUserContext_t *UserContext_ptr,
									 CRYS_RSAUserPubKey_t *UserPubKey_ptr,
									 CRYS_RSA_HASH_OpMode_t hashFunc,
									 CRYS_PKCS1_MGF_t MGF,
									 DxUint16_t SaltLen,
									 DxUint8_t     *DataIn_ptr,
									 DxUint8_t     *SepDataIn_ptr,
									 DxUint32_t     DataInSize,
									 DxUint8_t     *Sig_ptr,
									 CRYS_PKCS1_version PKCS1_ver);

#ifdef __cplusplus
}
#endif			

#endif
