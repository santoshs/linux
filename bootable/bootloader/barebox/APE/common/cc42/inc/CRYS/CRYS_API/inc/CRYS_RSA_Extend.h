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
 
 #ifndef CRYS_RSA_EXTEND_H
#define CRYS_RSA_EXTEND_H

#include "CRYS_RSA_Types.h"
#include "LLF_PKI_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  Object name     :  CRYS_RSA_EXTEND.h
 */
  
/** @file
 * \brief This module defines the API that supports PKCS#1 v2.1 primitive functions
 *
 * \version 
 * \author R.Levin
 */


/************************         Defines       *************************************/

/************************ Public functions prototipes  ******************************/

/*********************************************************************************************/
/**
@brief
CRYS_RSA_PRIM_EncryptExactSize implements the RSAEP algorithm as defined in PKCS#1 v2.1 6.1.1

  @param[in] UserPubKey_ptr - A pointer to the public key data structure
  @param[in] PrimeData_ptr - A pointer to a structure containing internal temp buffers
  @param[in] DataIn_ptr - A pointer to the plain data to encrypt 
  @param[in] DataInSize - The size, in bytes, of the data to encrypt.
                        \note This value must be <= the size of the modulus.
  @param[out] Output_ptr - Pointer to the encrypted data.
                           The buffer must be at least PubKey_ptr->N.len bytes long 
                           (that is, the size of the modulus, in bytes).

  @return CRYSError_t - CRYS_OK,
                        CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                        CRYS_RSA_PUB_KEY_VALIDATION_TAG_ERROR,
                        CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID,
                        CRYS_RSA_DATA_POINTER_INVALID_ERROR,
                        CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
                        CRYS_RSA_INVALID_MESSAGE_BUFFER_SIZE,
                        CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
                        CRYS_RSA_INVALID_MESSAGE_VAL
 */
#define CRYS_RSA_PRIM_EncryptExactSize( UserPubKey_ptr, PrimeData_ptr, \
                                        Data_ptr, DataInSize, Output_ptr ) \
        CRYS_RSA_PRIM_Encrypt( UserPubKey_ptr, PrimeData_ptr, \
                               Data_ptr, DataInSize, Output_ptr )                                



/*********************************************************************************************/
/**
@brief
CRYS_RSA_PRIM_DecryptExactSize - implements the RSADP algorithm as defined in PKCS#1 v2.1 6.1.2

  @param[in] UserPrivKey_ptr - Pointer to the private key data structure.
                           \note The representation (pair or quintuple) 
                           and hence the algorithm (CRT or not-CRT) 
                           is determined by the Private Key data structure - 
                           using CRYS_Build_PrivKey or CRYS_Build_PrivKeyCRT 
                           determines which algorithm will be used.                           
  @param[in] PrimeData_ptr - A pointer to a structure containing internal buffers
                             required for the RSA operation                        
  @param[in] DataIn_ptr - A pointer to the data to be decrypted. Size of this data must be 
                          equaled to key modulus size exactly. 
  @param[out] Output_ptr - A pointer to the decrypted data.
                           The buffer must be at least PrivKey_ptr->N.len bytes long 
                           (that is, the size of the modulus, in bytes).
  @param[in] DataOutSize - The expected size of output data, in bytes. 
                           Must be not great than the size of the modulus.
                           
    /Note: The differnce of this function from CRYS_RSA_PRIM_Decrypt is that the function
           outputs the message of exact size, given by user.

  @return CRYSError_t - CRYS_OK,
                        CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                        CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID,
                        CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR,
                        CRYS_RSA_DATA_POINTER_INVALID_ERROR,
                        CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
                        CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
                        CRYS_RSA_INVALID_MESSAGE_VAL
 */
CIMPORT_C CRYSError_t CRYS_RSA_PRIM_DecryptExactSize(
                                  CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,
                                  CRYS_RSAPrimeData_t   *PrimeData_ptr,
                                  DxUint8_t             *DataIn_ptr,
                                  DxUint8_t             *Output_ptr,
                                  DxUint16_t             DataOutSize );




/***************************************************************************************/
/**
  @brief  The function calculates modular inverse of 32-bits numbers, if it exists. 
         
		 Algorithm:
		   *invMod_ptr = 1/aVal mod modVal
          If inverse is not exists, the function set output value to 0.
           
		  This API has two implementations:
		  1. Implementation based on Binary Right Shift algorithm, which uses multiplication
		     and division operations. This is the default implementation.
		  2. Implementation based on shifting Euclidean algorithm, which not uses 
		     multiplication and division. For choosing this implementation the user must to 
			 define the flag CRYS_RSA_INV_MOD_SHIFTING_EUCLID_ALG in the project definitions
			 (Make-file for SEP).

  @param[in]  aVal - value to be inverted.
  @param[in]  modVal - modulus.
  @param[out] invMod_ptr - pointer to the result buffer.

  @return CRYSError_t - CRYS_OK,
                        CRYS_RSA_INVALID_MODULUS_ERROR,
						
 */
CIMPORT_C CRYSError_t CRYS_RSA_InvModWord(
                                  DxUint32_t     aVal,       /*in*/
                                  DxUint32_t     modVal,     /*in*/
								  DxUint32_t    *invMod_ptr  /*out*/ );
								  
								  
/***********************************************************************************************/
/**
   @brief CRYS_RSA_KG_GeneratePQprimes generates two valid prime factors P,Q used in 
          RSA key generation.          
 
   @param[in] pubExp_ptr - The pointer to the public exponent (little endian bytes array).
                           Allowed values: Exp = 0x3, 0x11, 0x010001.
   @param[in] pubExpSizeInBytes - The public exponent size in bytes (allowed sizes 1,3).
   @param[in] primeSizeInBits  - The size of the prime factor in bits equalled to keySizeInBits/2). 
                                 Supported sizes of the prime factors are all 128 bit multiples 
                                 between 256 - 1024;
   @param[in] testsCount - count of Rabin-Miller tests needed for accepting the prime candidate as 
                           valid prime number.                             
   @param[out] p_ptr - A pointer to the first prime factor in words (LSWord is the left most). 
   @param[out] q_ptr - A pointer to the second prime factor in words (LSWord is the left most). 
   @param[in]  keyGenData_ptr - A pointer to a temp buffer structure required for the 
                               KeyGen operation. The buffer not needs initialization.                      

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR
                         CRYS_RSA_INVALID_EXPONENT_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_VAL,
                         CRYS_RSA_INVALID_PTR_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
*/
CIMPORT_C CRYSError_t  CRYS_RSA_KG_GeneratePQprimes(
                                        DxUint8_t             *pubExp_ptr,           /* in : public exponent – little endian bytes array (LSB – left most) */
                                        DxUint16_t             pubExpSizeInBytes,    /* in: size of public exponent */
                                        DxUint32_t             primeSizeInBits,      /* in: size of prime in bits= keySizeInBits/2  (according to CRYS limitations on key size) */
                                        DxUint32_t             testsCount,           /* count of Rabin-Miller tests needed for accepting the number as prime */  
                                        DxUint32_t            *p_ptr,                /* out:  first prime – little endian words array */
                                        DxUint32_t            *q_ptr,                /* out: second prime  – little endian words array */
                                        CRYS_RSAKGData_t      *keyGenData_ptr );     /* temp buffer – not need initialization */


/***********************************************************************************************/
/**
   @brief CRYS_RSA_KG_ClculateKeyPairFromPQ calculates a Pair of public and private keys 
          on NonCRT or CRT modes.
          
          The function receives P,Q prime factors and calculates all parameters 
          of private and public keys if appropriate pointer is not NULL.
          If compilation flag CRYS_NO_SELF_TEST_SUPPORT is not defined, then the function
          performs RSA_Encrypt and RSA_Decrypt operations on predefined data and compares
          the output with expected results.            
 
   @param[in] pubExp_ptr - The pointer to the public exponent (little endian bytes array).
                           Allowed values: Exp = 0x3, 0x11, 0x010001.
   @param[in] pubExpSizeInBytes - The public exponent size in bytes (allowed sizes 1,3).
   @param[in] p_ptr - A pointer to the valid first prime factor in words (LSWord is the left most). 
   @param[in] q_ptr - A pointer to the valid second prime factor in words (LSWord is the left most). 
   @param[in] primeSizeInBits  - The size of the prime factor in bits equalled to keySizeInBits/2). 
                                 Supported sizes of the prime factors are all 128 bit multiples 
                                 between 256 - 1024;
   @param[out] UserPrivKey_ptr - A pointer to the private key structure (if UserPrivKey_ptr == NULL,
                                then private key is not calculated).  
                           This structure is used as input to the CRYS_RSA_PRIM_Decrypt API.
   @param[out] UserPubKey_ptr - A pointer to the public key structure (if UserPubKey_ptr == NULL,
                                then public key is not calculated). 
                           This structure is used as input to the CRYS_RSA_PRIM_Encrypt API.
   @param[in] KeyGenData_ptr - a pointer to a structure required for the KeyGen operation.                       

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PTR_ERROR,
                         CRYS_RSA_INVALID_DECRYPRION_MODE_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_SIZE,
                         CRYS_RSA_KEY_GEN_CONDITIONAL_TEST_FAIL_ERROR
*/
CIMPORT_C  CRYSError_t CRYS_RSA_KG_CalculateKeyPairFromPQ(
                                        DxUint8_t                 *pubExp_ptr,         /* in : public exponent – little endian bytes array (LSB – left most) */                                        
                                        DxUint16_t                 pubExpSizeInBytes,  /* in: size of public exponent */
                                        DxUint32_t                *p_ptr,              /* in: first prime –  little endian words array */
                                        DxUint32_t                *q_ptr,              /* in: second prime  –  little endian words array */
                                        DxUint16                   primeSizeInBits,    /* in: size of prime factors p, q in bits */
                                        CRYS_RSA_DecryptionMode_t  privKeyMode,        /* in: mode of RSA key: 10 – non CRT,  11 – CRT */
                                        CRYS_RSAUserPrivKey_t     *userPrivKey_ptr,    /* out: user private key structure*/
                                        CRYS_RSAUserPubKey_t      *userPubKey_ptr,     /* out: user public key structure */
                                        CRYS_RSAKGData_t          *keyGenData_ptr );   /* temp buffer – not need initialization */
								  


/***********************************************************************************************/
/**
   @brief CRYS_RSA_KG_ExtendGenerateKeyPair generates a Pair of public and private keys on non CRT mode.
 
   @param[in] pubExp_ptr - The pointer to the public exponent (public key)
                           Allowed values: Exp = 0x3, 0x11, 0x010001.
   @param[in] pubExpSizeInBytes - The public exponent size in bytes (allowed sizes 1,3).
   @param[in] keySizeBits   - The size of the key modulus, in bits. Supported sizes are:
                              256 bit multiples between 512 - 2048.
                           
   @param[in] privKeyMode - flag of private key mode: CRYS_RSA_NoCrt = 10 or CRYS_RSA_Crt = 11. 
   @param[out] userPrivKey_ptr - A pointer to the private key structure. 
                           This structure is used as input to the CRYS_RSA_PRIM_Decrypt API.
   @param[out] userPubKey_ptr - A pointer to the public key structure. 
                           This structure is used as input to the CRYS_RSA_PRIM_Encrypt API.
   @param[in] keyGenData_ptr - a pointer to a structure required for the KeyGen operation.                       

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_SIZE
*/
CIMPORT_C  CRYSError_t DX_RSA_KG_ExtendGenerateKeyPair(
	                                    DxUint8_t             *pubExp_ptr,
                                        DxUint16_t             pubExpSizeInBytes, 
                                        DxUint32_t             keySizeBits,
                                        CRYS_RSA_DecryptionMode_t  privKeyMode,  
                                        CRYS_RSAUserPrivKey_t *userPrivKey_ptr,
                                        CRYS_RSAUserPubKey_t  *userPubKey_ptr,
                                        CRYS_RSAKGData_t      *keyGenData_ptr );


#ifdef __cplusplus
}
#endif

#endif
