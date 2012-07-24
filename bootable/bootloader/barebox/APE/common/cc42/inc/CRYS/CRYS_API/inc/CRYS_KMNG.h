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
 
 
#ifndef CRYS_KMNG_H
#define CRYS_KMNG_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AES.h"
#include "CRYS_DES.h"
#include "CRYS_HASH.h"
#include "CRYS_HMAC.h"
#include "CRYS_RSA_SCHEMES.h"
#include "CRYS_RSA_PRIM.h"
#include "KMNG_Defs.h"
#include "CRYS_DH.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_SST.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:42:30 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AES APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_SST.h#1:incl:13
   *  \author adams
   */

/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/


/************************ Enums ********************************/

/* Enum defining the user's key size argument */
typedef enum
{
    CRYS_KMNG_enc_dec_op,
    CRYS_KMNG_sign_ver_mac_op,
    CRYS_KMNG_wrap_op
}CRYS_KMNG_operation;

/************************ Typedefs  ****************************/

/* typedef for verification function */
typedef DxBool_t (*CRYS_KMNG_VerifyFunc) (DxUint32_t usage, CRYS_KMNG_operation op);

/************************ context Structs  ******************************/


/************************ Public Variables **********************/

/************************ Public Functions **********************/

/**
 * @brief This function is used to initialize the AES context for KMNG.
 *        To operate the AES machine, this should be the first function called.
 *
 * The function gets key from SST and calls CRYS_AES_Init
 *
 * @param[in] AES_WrappedKey_ptr - wrapped AES key from KMNG
 *
 * @param[in] ContextID_ptr - A pointer to the KMNG AES context buffer that is allocated by the user 
 *                       and is used for the AES machine operation.
 *
 * @param[in,out] IVCounter_ptr - This parameter is the buffer of the IV or counters on mode CTR.
 *                          In ECB mode this parameter is not used.
 *                          In CBC and MAC modes this parameter should contain the IV values.
 *                          In CTR mode this value should contain the init counter.
 *
 * @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0) 
 *                           or a Decrypt operation (1).
 *
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, or CTR
 *
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_AES_error.h
 */

CIMPORT_C CRYSError_t  CRYS_KMNG_AES_Init(	KMNG_AES_WrappedKey_t		AES_WrappedKey_ptr,
											CRYS_AESUserContext_t		*ContextID_ptr,
											CRYS_AES_IvCounter_t		IVCounter_ptr,
											CRYS_AES_EncryptMode_t		EncryptDecryptFlag,
											CRYS_AES_OperationMode_t	OperationMode);


/**
 * @brief This function is used to operate the AES machine in one integrated operation.
 *
 * The function gets key from SST and calls CRYS_AES
 *
 *
 * @param[in] AES_WrappedKey_ptr - wrapped AES key from KMNG
 *
 * @param[in,out] IVCounter_ptr - This parameter is the buffer of the IV or counters on mode CTR.
 *                          In ECB mode this parameter is not used.
 *                          In CBC and MAC modes this parameter should contain the IV values.
 *                          In CTR mode this value shuld contain the init counter.
 *
 * @param[in] EncryptDecryptFlag - A flag specifying  whether the AES should perform an Encrypt operation (0) 
 *                           or a Decrypt operation (1).
 *
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, or CTR.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the AES. 
 *                   The pointer's value does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data (must be not 0 and must be multiple of 16 bytes).
 *
 * @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the AES. 
 *                        The pointer's value does not need to be word-aligned.  
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_AES_error.h
 */

CIMPORT_C CRYSError_t  CRYS_KMNG_AES(	KMNG_AES_WrappedKey_t		AES_WrappedKey_ptr,
										CRYS_AES_IvCounter_t		IVCounter_ptr,       
										CRYS_AES_EncryptMode_t		EncryptDecryptFlag, 
										CRYS_AES_OperationMode_t	OperationMode,       
										DxUint8_t					*DataIn_ptr,        
										DxUint32_t					DataInSize,         
										DxUint8_t					*DataOut_ptr);     


/**
 * @brief This function is used to initialize the DES machine.
 *        To operate the DES machine, this should be the first function called.
 *
 * @param[in] DES_WrappedKey_ptr - wrapped DES key from KMNG
 *
 * @param[in] ContextID_ptr  - A pointer to the DES context buffer allocated by the user 
 *                       that is used for the DES machine operation.
 *
 * @param[in,out] IV_ptr - The buffer of the IV.
 *                          In ECB mode this parameter is not used.
 *                          In CBC this parameter should contain the IV values.
 *
 * @param[in] EncryptDecryptFlag - A flag that determines whether the DES should perform 
 *                           an Encrypt operation (0) or a Decrypt operation (1).
 *
 * @param[in] OperationMode - The operation mode: ECB or CBC.
 *
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_error.h
 */

CIMPORT_C CRYSError_t  CRYS_KMNG_DES_Init(	KMNG_DES_WrappedKey_t	DES_WrappedKey_ptr,
											CRYS_DESUserContext_t    *ContextID_ptr,
											CRYS_DES_Iv_t            IV_ptr,
											CRYS_DES_EncryptMode_t   EncryptDecryptFlag,
											CRYS_DES_OperationMode_t OperationMode);


/**
 * @brief This function is used to operate the DES machine in one integrated operation.
 *
 *
 *
 * @param[in] DES_WrappedKey_ptr - wrapped DES key from KMNG
 *
 * @param[in,out] IV_ptr - This parameter is the buffer of the IV.
 *                          In ECB mode this parameter is not used.
 *                          In CBC this parameter should contain the IV values.
 *
 * @param[in] EncryptDecryptFlag - A flag that determines if the DES should perform 
 *                           an Encrypt operation (0) or a Decrypt operation (1).
 *
 * @param[in] OperationMode - The operation mode: ECB or CBC.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the DES.
 *                   The pointer does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data (must be not 0 and must be multiple of 8 bytes).
 *
 * @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the DES. 
 *                        The pointer does not need to be word-aligned.  
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_error.h
 */
 
CIMPORT_C CRYSError_t  CRYS_KMNG_DES(KMNG_DES_WrappedKey_t	DES_WrappedKey_ptr,
									CRYS_DES_Iv_t            IV_ptr,
									CRYS_DES_EncryptMode_t   EncryptDecryptFlag,
									CRYS_DES_OperationMode_t OperationMode,             
									DxUint8_t                  *DataIn_ptr,              
									DxUint32_t                 DataInSize,              
									DxUint8_t                  *DataOut_ptr);


/**
 * This function initializes the HMAC machine on the CRYS level.
 *
 * The function allocates and initializes the HMAC Context .
 *
 * The function receives as input a pointer to store the context handle to HMAC Context. 
 *
 * @param[in] HMAC_WrappedKey_ptr - wrapped HMAC key from KMNG
 *
 * @param[in] ContextID_ptr - A pointer to the HMAC context buffer allocated by the user 
 *                       that is used for the HMAC machine operation.
 *
 * @param[in] OperationMode - The operation mode: MD5 or SHA1.
 *
 * @return CRYSError_t - On success the function returns the value CRYS_OK, 
 *			and on failure a non-ZERO error.
 *      
 */

CIMPORT_C CRYSError_t CRYS_KMNG_HMAC_Init(	KMNG_HMAC_WrappedKey_t	HMAC_WrappedKey_ptr,
											CRYS_HMACUserContext_t     *ContextID_ptr,
											CRYS_HASH_OperationMode_t  OperationMode);


/**
 * This function provides a HASH function for processing one buffer of data.
 *
 *
 * @param[in] HMAC_WrappedKey_ptr - wrapped HMAC key from KMNG
 *
 * @param[in] OperationMode - The operation mode: MD5 or SHA1.
 *
 * @param[in] DataIn_ptr - A pointer to the buffer that stores the data to be hashed.
 * 
 * @param[in] DataSize - The size of the data to be hashed, in bytes. 
 *
 * @retval HmacResultBuff - A pointer to the target buffer where the 
 *                      HMAC result stored in the context is loaded to.
 *
 * @return CRYSError_t - On success the function returns CRYS_OK, 
 *				and on failure a non-ZERO error.
 *
 */

CIMPORT_C CRYSError_t CRYS_KMNG_HMAC(KMNG_HMAC_WrappedKey_t	HMAC_WrappedKey_ptr, 	
									   CRYS_HASH_OperationMode_t  OperationMode,
									DxUint8_t                    *DataIn_ptr,
									DxUint32_t                    DataSize,
									CRYS_HASH_Result_t          HmacResultBuff);


/**
 *  @brief
 *  This function gets key from the SST and than encrypt data according to the shcemes.
 *
 *	The actual macro that will be used by the user is:
 *	CRYS_KMNG_RSA_OAEP_Encrypt 	  - for v2.1
 *	CRYS_KMNG_RSA_PKCS1v15_Encrypt - for v1.5
 *	
 *
 * @param[in] CRYS_KMNG_RSA_PubWrappedKey_t - wrapped RSA public key from KMNG
 *
 *  @param[in] PrimeData_ptr - A pointer to a CRYS_RSAPrimeData_t 
 *  				that is used for the Encryption operation
 *  @param[in] hashFunc - The hash function to be used. 
 *                        Currently only CRYS_PKCS1_SHA1 is supported. 
 *                        The other hash functions recommended 
 *                        by PKCS#1 v2.1 are SHA-256/284/512.
 *  @param[in] L - The label input. 
 *                 Version 2.1 of the standard defines L as the empty string, 
 *                 so this argument is currently ignored. 
 * @param[in] Llen - The label length. 
 *                Version 2.1 of the standard defines L as the empty string, 
 *                so this argument is currently ignored.
 * @param[in] MGF - The mask generation function. PKCS#1 v2.1 defines MGF1, 
 *                  so the only value allowed here is CRYS_PKCS1_MGF1.
 * @param[in] DataIn_ptr - A pointer to the data to encrypt.
 * @param[in] DataSize - The size, in bytes, of the data to
 *                       encrypt. \note Must be <= (modulus_size - 2*Hash_output_length - 2).
 *
 * @param[out] Output_ptr - A pointer to the encrypted data. 
 *                          The buffer must be at least PubKey_ptr->N.len bytes long 
 *                          (that is, the size of the modulus, in bytes).
 * @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
 *
 * @return CRYSError_t - CRYS_OK, CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE
 */ 
   
CIMPORT_C CRYSError_t _DX_CRYS_KMNG_RSA_SCHEMES_Encrypt(KMNG_RSA_WrappedKey_t		RSA_WrappedKey_ptr,
														CRYS_RSAPrimeData_t*		PrimeData_ptr,
														CRYS_RSA_HASH_OpMode_t		hashFunc,
														DxUint8_t					*L,
														DxUint16_t					Llen,
														CRYS_PKCS1_MGF_t			MGF,
														DxUint8_t					*DataIn_ptr,
														DxUint16_t					DataInSize,
														DxUint8_t					*Output_ptr,
														CRYS_PKCS1_version			PKCS1_ver);


/**
   @brief
   CRYS_KMNG_RSA_OAEP_PSS21_Encrypt implements the RSAES-OAEP algorithm 
   as defined in PKCS#1 v2.1 8.1
   The function will eventually call CRYS_KMNG_RSA_OAEP_Encrypt.   
*/
				  
				  
#define CRYS_KMNG_RSA_OAEP_Encrypt(RSA_WrappedKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr)\
_DX_CRYS_KMNG_RSA_SCHEMES_Encrypt (RSA_WrappedKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr,CRYS_PKCS1_VER21)

/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_Encrypt implements the RSAES-PKCS1v15 algorithm 
   as defined in PKCS#1 v2.1 8.2
   The function will eventually call CRYS_KMNG_RSA_PKCS1v15_Encrypt.   
*/

#define CRYS_KMNG_RSA_PKCS1v15_Encrypt(RSA_WrappedKey_ptr,PrimeData_ptr,DataIn_ptr,DataInSize,Output_ptr)\
		_DX_CRYS_KMNG_RSA_SCHEMES_Encrypt(RSA_WrappedKey_ptr,PrimeData_ptr,CRYS_RSA_HASH_NO_HASH_mode,DX_NULL,0,CRYS_PKCS1_NO_MGF,DataIn_ptr,DataInSize,Output_ptr,CRYS_PKCS1_VER15)
		

/**
 *   @brief
 *   This function implements the Decrypt algorithm as defined
 *   in PKCS#1 v2.1 and PKCS#1 v1.5
 *
 *	The actual macro that will be used by the user is:
 *	CRYS_KMNG_RSA_OAEP_Decrypt		- for v2.1
 *	CRYS_KMNG_RSA_PKCS1v15_Decrypt	- for v1.5
 *
 *
 *   @param[in] CRYS_KMNG_RSA_PrivWrappedKey_t - wrapped RSA private key from KMNG
 *
 *   @param[in] PrimeData_ptr - A pointer to a CRYS_RSAPrimeData_t, 
 *   				which is used for the Encryption operation
 *   							                            
 *   @param[in] hashFunc - The hash function to be used. 
 *                         Currently only CRYS_PKCS1_SHA1 is supported. 
 *                         The other hash functions recommended by 
 *                         PKCS#1 v2.1 are SHA-256/284/512.
 *   @param[in] L - The label input. 
 *                  Version 2.1 of the standard defines L as the empty string, 
 *                  so this argument is currently ignored. 
 *   @param[in] Llen - The label length. 
 *                  Version 2.1 of the standard defines L as the empty string, 
 *                  so this argument is currently ignored.
 *   @param[in] MGF - The mask generation function. PKCS#1 v2.1 defines MGF1, 
 *                    so the only value allowed here is CRYS_PKCS1_MGF1.
 *   @param[in] Data_ptr - A pointer to the data to decrypt.
 *   @param[in] DataSize - The size, in bytes, of the data to decrypt. 
 *                         \note Must be <= the size of the modulus.
 *
 *   @param[out] Output_ptr - Pointer to the decrypted data. 
 *                            The buffer must be at least PrivKey_ptr->N.len bytes long 
 *                            (that is, the size of the modulus, in bytes).
 *   @param[in,out] OutputSize_ptr - The size of the Output_buffer ptr  [bytes].
 *                                   The input size value must be at least the size of the modulus 
 *                                   This value is updated with the actual number 
 *                                   of bytes that are loaded to Output_ptr buffer
 *
 *   @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
 *
 *   @return CRYSError_t - CRYS_OK, CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE
 **/
 
 CIMPORT_C CRYSError_t _DX_CRYS_KMNG_RSA_SCHEMES_Decrypt(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
														CRYS_RSAPrimeData_t*	PrimeData_ptr,
														CRYS_RSA_HASH_OpMode_t	hashFunc,
														DxUint8_t				*L,
														DxUint16_t				Llen,
														CRYS_PKCS1_MGF_t		MGF,
														DxUint8_t				*Data_ptr,
														DxUint16_t				DataSize,
														DxUint8_t				*Output_ptr,
														DxUint16_t				*OutputSize_ptr,
														CRYS_PKCS1_version		PKCS1_ver);


/**
   @brief
   CRYS_KMNG_RSA_OAEP_Decrypt implements the RSAES-OAEP algorithm 
   as defined in PKCS#1 v2.1 8.1
   The function will eventually call CRYS_RSA_OAEP_Decrypt.   
   
*/
#define CRYS_KMNG_RSA_OAEP_Decrypt(RSA_WrappedKey_ptr,PrimeData_ptr,HashMode,L,Llen,MGF,Data_ptr,DataSize,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_SCHEMES_Decrypt(RSA_WrappedKey_ptr,PrimeData_ptr,HashMode/*CRYS_RSA_HASH_SHA1_mode*/,L,Llen,MGF,Data_ptr,DataSize,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)
	
		
/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_Decrypt implements the RSAES-PKCS1v15 algorithm as defined
   in PKCS#1 v2.1 8.2
   The function will eventually call CRYS_RSA_PKCS1v15_Decrypt.   
*/
#define CRYS_KMNG_RSA_PKCS1v15_Decrypt(RSA_WrappedKey_ptr,PrimeData_ptr,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_SCHEMES_Decrypt(RSA_WrappedKey_ptr,PrimeData_ptr,CRYS_RSA_HASH_NO_HASH_mode,DX_NULL,0,CRYS_PKCS1_NO_MGF,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER15)


/**
 *  \brief RSA_SignInit initializes the Signing
 *  multi-call algorithm as defined in PKCS#1 v1.5 and 2.1
 *
 *	The actual macro that will be used by the user is:
 *	CRYS_KMNG_RSA_PSS_SignInit		 - for v2.1
 *	CRYS_KMNG_RSA_PKCS1v15_SignInit	 - for v1.5
 *
 *  This function does not do cryptographic processing. Rather, it
 *  prepares a context that is used by the Update
 *  and Finish functions.
 *  
 *   @param[in] CRYS_KMNG_RSA_PrivWrappedKey_t - wrapped RSA private key from KMNG
 *
 *   @param[in,out] UserContext_ptr - A pointer to a Context. The value
 *                                returned here must be passed to
 *                                the Update and Finish functions.
 *   @param[in] hashFunc - The hash function to be used. Currently
 *                         only CRYS_PKCS1_SHA1 is supported. The
 *                         other hash functions recommended by PKCS#1
 *                         v2.1 are SHA-256/284/512.
 *   @param[in] MGF - The mask generation function. PKCS#1 v2.1
 *                    defines MGF1, so the only value allowed here is CRYS_PKCS1_MGF1.
 *   @param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
 *   						Typical lengths are 0 and hLen (20 for SHA1)
 *   						The maximum length allowed is NSize - hLen - 2                    
 *   @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
 *                       
 *   @return CRYSError_t - CRYS_OK, CRYS_BAD_PARAM
*/
				  
CIMPORT_C CRYSError_t _DX_CRYS_KMNG_RSA_SignInit(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
												CRYS_RSAPrivUserContext_t	*UserContext_ptr,
												CRYS_RSA_HASH_OpMode_t		hashFunc,
												CRYS_PKCS1_MGF_t			MGF,
												DxUint16_t					SaltLen,				  
												CRYS_PKCS1_version			PKCS1_ver);


/**
   \brief CRYS_KMNG_RSA_PSS_SignInit initializes the RSASSA-PSS
   multi-call version.
*/			
#define CRYS_KMNG_RSA_PSS_SignInit(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc,MGF,SaltLen)\
			_DX_CRYS_KMNG_RSA_SignInit(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc,MGF,SaltLen,CRYS_PKCS1_VER21)				  


/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SignInit initializes the PKCS#1 v1.5
   multi-call version.
*/
#define CRYS_KMNG_RSA_PKCS1v15_SignInit(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc)\
			_DX_CRYS_KMNG_RSA_SignInit(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc,CRYS_PKCS1_NO_MGF,0,CRYS_PKCS1_VER15)
			

/**
 *   @brief
 *   RSA_Sign implements the Signing algorithm as defined
 *   in PKCS#1 v1.5 or v2.1, using a single function
 *
 *   The actual macros that will be used by the user are:
 *   CRYS_KMNG_RSA_PKCS1v15_Sign		- for v1.5
 *   CRYS_KMNG_RSA_PKCS1v15_SHA1_Sign	- for v1.5 data is hashed with SHA1
 *   CRYS_KMNG_RSA_PKCS1v15_MD5_Sign	- for v1.5 data is hashed with MD5
 *   CRYS_KMNG_RSA_PSS_Sign			- for v2.1
 *   CRYS_KMNG_RSA_PSS_SHA1_Sign		- for v2.1 data is hashed with SHA1
 *					- for v2.1 MD5 is not supported, since 
 *					according to the PKCS#1 ver2.1 it is not recommended
 *  
 *
 *   @param[in] CRYS_KMNG_RSA_PrivWrappedKey_t - wrapped RSA private key from KMNG
 *
 *   @param[in] UserContext_ptr - A pointer to a Context. For the use of the 
 *   								function as a space to work on.
 *   @param[in] hashFunc - The hash function to be used. Currently
 *                         only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
 *                         are supported. 
 *   @param[in] MGF - The mask generation function. Only for PKCS#1 v2.1
 *                    defines MGF1, so the only value allowed for v2.1 
 *                    is CRYS_PKCS1_MGF1. 
 *   @param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
 *   						Typical lengths are 0 and hLen (20 for SHA1)
 *   						The maximum length allowed is NSize - hLen - 2                                                                                       
 *   @param[in] DataIn_ptr - A pointer to the data to sign.
 *   @param[in] DataInSize - The size, in bytes, of the data to sign.
 *   @param[out] Output_ptr - A pointer to the signature. 
 *                            The buffer must be at least PrivKey_ptr->N.len bytes long 
 *                            (that is, the size of the modulus in bytes).
 *   @param[in,out] OutputSize_ptr - A pointer to the Signature Size value - the input value
 *                            is the signature buffer size allocated, the output value is 
 *                            the signature size actually used.
 *                            The buffer must be at least PrivKey_ptr->N.len bytes long 
 *                            (that is, the size of the modulus in bytes).
 *   @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
 *      
 *   @return CRYSError_t - CRYS_OK,
 *                         CRYS_RSA_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
 *                         CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR,
 *                         CRYS_RSA_HASH_ILLEGAL_OPERATION_MODE_ERROR,
 *                         CRYS_RSA_MGF_ILLEGAL_ARG_ERROR,
 *                         CRYS_RSA_PKCS1_VER_ARG_ERROR,
 *                         CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
 *                         CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
 *                         CRYS_RSA_INVALID_SIGNATURE_BUFFER_SIZE
*/

CIMPORT_C CRYSError_t _DX_CRYS_KMNG_RSA_Sign(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
											CRYS_RSAPrivUserContext_t	*UserContext_ptr,
											CRYS_RSA_HASH_OpMode_t		hashFunc,
											CRYS_PKCS1_MGF_t			MGF,
											DxUint16_t					SaltLen,				  			      
											DxUint8_t					*DataIn_ptr,
											DxUint32_t					DataInSize,
											DxUint8_t					*Output_ptr,
											DxUint16_t					*OutputSize_ptr,
											CRYS_PKCS1_version			PKCS1_ver);


/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5

   This function combines the RSASP1 signature primitive and the
   EMSA-PKCS1v15 encoding method, to provide an RSA-based signature scheme. 
   For more details, please refer to the PKCS#1 standard.
  
*/	
	
#define CRYS_KMNG_RSA_PKCS1v15_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),(hashFunc),(CRYS_PKCS1_NO_MGF),0,(DataIn_ptr),(DataInSize),(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)
		

/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_SHA1_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-1
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA1_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),(CRYS_RSA_After_SHA1_mode),(CRYS_PKCS1_NO_MGF),0,(DataIn_ptr),CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		

/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_MD5_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using MD5
   
   Note: The data_in size is already known after the Hash
*/

#define CRYS_KMNG_RSA_PKCS1v15_MD5_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),CRYS_RSA_After_MD5_mode,CRYS_PKCS1_NO_MGF,0,(DataIn_ptr),CRYS_HASH_MD5_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)


/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_SHA224_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-224
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA224_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),(CRYS_RSA_After_SHA224_mode),(CRYS_PKCS1_NO_MGF),0,(DataIn_ptr),CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		




/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_SHA256_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-256
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA256_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),(CRYS_RSA_After_SHA256_mode),(CRYS_PKCS1_NO_MGF),0,(DataIn_ptr),CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		

/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_SHA1_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-384
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA384_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),(CRYS_RSA_After_SHA384_mode),(CRYS_PKCS1_NO_MGF),0,(DataIn_ptr),CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		


/**
   @brief
   CRYS_KMNG_RSA_PKCS1v15_SHA512_Sign implements the RSASSA-PKCS1v15 algorithm 
   as defined in PKCS#1 v1.5, but without performing a HASH function - 
   it assumes that the data in has already been hashed using SHA-512
   
   Note: The data_in size is already known after the Hash
      
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA512_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,(UserContext_ptr),(CRYS_RSA_After_SHA512_mode),(CRYS_PKCS1_NO_MGF),0,(DataIn_ptr),CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,(Output_ptr),(OutputSize_ptr),CRYS_PKCS1_VER15)		



/**
   @brief CRYS_KMNG_RSA_PSS_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1, in a single function call.

    Note: According to the PKCS#1 ver2.1 it is not recommended to use MD5 Hash, 
		therefore it is not supported
   
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PSS_Sign
*/	

#define CRYS_KMNG_RSA_PSS_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   @brief 
   CRYS_KMNG_RSA_PSS_SHA1_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-1

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PSS_SHA1_Sign
*/	

#define CRYS_KMNG_RSA_PSS_SHA1_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA1_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   @brief 
   CRYS_KMNG_RSA_PSS_SHA224_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-224

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PSS_SHA224_Sign
*/	

#define CRYS_KMNG_RSA_PSS_SHA224_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA224_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   @brief 
   CRYS_KMNG_RSA_PSS_SHA256_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-256

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PSS_SHA256_Sign
*/	

#define CRYS_KMNG_RSA_PSS_SHA256_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA256_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	



/**
   @brief 
   CRYS_KMNG_RSA_PSS_SHA384_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-384

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PSS_SHA384_Sign
*/	

#define CRYS_KMNG_RSA_PSS_SHA384_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA384_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
   @brief 
   CRYS_KMNG_RSA_PSS_SHA512_Sign implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 9.1 in a single function call, but without 
   performing a HASH function - it assumes that the data in
   has already been hashed using SHA-512

   Note: The data_in size is already known after the Hash
   
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PSS_SHA512_Sign
*/	

#define CRYS_KMNG_RSA_PSS_SHA512_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Output_ptr,OutputSize_ptr)\
		_DX_CRYS_KMNG_RSA_Sign(RSA_Priv_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA512_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,Output_ptr,OutputSize_ptr,CRYS_PKCS1_VER21)	


/**
 *   @brief 
 *	 CRYS_KMNG_RSA_VerifyInit initializes the Verify
 *   multi-call algorithm as defined in PKCS#1 v1.5 and 2.1
 *
 *	The actual macro that will be used by the user is:
 *	CRYS_KMNG_RSA_PKCS1v15_VerifyInit - for v1.5
 *	CRYS_KMNG_RSA_PSS_VerifyInit		 - for v2.1
 *
 *   @param[in] CRYS_KMNG_RSA_PubWrappedKey_t - wrapped RSA public key from KMNG
 *
 *   @param[in] UserContext_ptr - A pointer to the public Context
 *	                           structure of the User.
 *   @param[in] hashFunc - The hash function to be used. Currently
 *                         only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
 *                         are supported. The other hash functions
 *                         recommended by PKCS#1 v2.1 are
 *                         SHA-256/284/512. If PKCS1_ver is CRYS_PKCS1_VER15, then
 *			 the value CRYS_RSA_After_HASH_NOT_KNOWN_mode will allow
 *			 the signature data to determine the hash function to be used.
 *   @param[in] MGF - The mask generation function. Only for PKCS#1 v2.1
 *                    defines MGF1, so the only value allowed for v2.1 
 *                    is CRYS_PKCS1_MGF1.
 *   @param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
 *   						Typical lengths are 0 and hLen (20 for SHA1)
 *   						The maximum length allowed is NSize - hLen - 2
 *                       If the salt length is not available in this process, the user can use the define:
 *                       CRYS_RSA_VERIFY_SALT_LENGTH_UNKNOWN
 *                       Security Note: it is recommended not to use this flag and to support the Salt length on each verify
 *  @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
 *  
 *  @return CRYSError_t - CRYS_OK, CRYS_INVALID_SIGNATURE,
 *                        CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE 
*/

CIMPORT_C CRYSError_t _DX_CRYS_KMNG_RSA_VerifyInit(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
													CRYS_RSAPubUserContext_t	*UserContext_ptr,
													CRYS_RSA_HASH_OpMode_t		hashFunc,
													CRYS_PKCS1_MGF_t			MGF,
													DxUint16_t					SaltLen,
													CRYS_PKCS1_version			PKCS1_ver);                     
  				  

/**
   \brief CRYS_KMNG_RSA_PKCS1v15_VerifyInit implements the RSASSA-PKCS1v15 algorithm
   as defined in PKCS#1 v2.1 8.2.1
*/				     
#define CRYS_KMNG_RSA_PKCS1v15_VerifyInit(RSA_Pub_WrappedKey_ptr,UserContext_ptr,hashFunc)\
		_DX_CRYS_KMNG_RSA_VerifyInit(RSA_Pub_WrappedKey_ptr,UserContext_ptr,hashFunc,CRYS_PKCS1_NO_MGF,0,CRYS_PKCS1_VER15)

/*
   \brief CRYS_KMNG_RSA_PSS_VerifyInit implements the RSASSA-PSS algorithm
   as defined in PKCS#1 v2.1 
   
   Note: According to PKCS#1 ver2.1 MD5 hash is not recommended, 
		therefore it is not supported
*/   
#define CRYS_KMNG_RSA_PSS_VerifyInit(RSA_Pub_WrappedKey_ptr,UserContext_ptr, hashFunc, MGF,SaltLen)\
		_DX_CRYS_KMNG_RSA_VerifyInit(RSA_Pub_WrappedKey_ptr,UserContext_ptr, hashFunc, MGF, SaltLen, CRYS_PKCS1_VER21)


/**
 *  \brief CRYS_KMNG_RSA_Verify implements the RSASSA-PKCS1v15 algorithm
 *   in a single function, as defined in PKCS#1 v1.5 and v2.1
 *
 *	The actual macro that will be used by the users is:
 *	CRYS_KMNG_RSA_PKCS1v15_Verify			- for v1.5
 *	CRYS_KMNG_RSA_PKCS1v15_MD5_Verify		- for v1.5 data in is hashed with MD5
 *	CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify		- for v1.5 data in is hashed with SHA1
 *	CRYS_KMNG_RSA_PSS_Verify					- for v2.1
 *	CRYS_KMNG_RSA_PSS_SHA1_Verify			- for v2.1 data in is hashed with SHA1
 *							- for v2.1 MD5 is not supported, since
 *							according to the PKCS#1 ver2.1 it is not recommended	
 *	
 *  @param[in] CRYS_KMNG_RSA_PubWrappedKey_t - wrapped RSA public key from KMNG
 *
 *  @param[in] UserContext_ptr - A pointer to the public Context, 
 *  				for the use of the function as a space to work on
 *  @param[in] hashFunc - The hash function to be used. 
 *                        Currently only CRYS_PKCS1_SHA1 and CRYS_PKCS1_MD5
 *                        are supported. The other hash functions
 *                        recommended by PKCS#1 v2.1 are
 *                        SHA-256/284/512.  If PKCS1_ver is CRYS_PKCS1_VER15, then
 *			 the value CRYS_RSA_After_HASH_NOT_KNOWN_mode will allow
 *			 the signature data to determine the hash function to be used.
 *  @param[in] MGF - The mask generation function. only for PKCS#1 v2.1
 *                   defines MGF1, so the only value allowed for v2.1 
 *                   is CRYS_PKCS1_MGF1. 
 *  @param[in] SaltLen - The Length of the Salt buffer. relevant for PKCS#1 Ver 2.1 Only
 *  						Typical lengths are 0 and hLen (20 for SHA1)
 *  						The maximum length allowed is NSize - hLen - 2                                             
 *  @param[in] DataIn_ptr - A pointer to the data whose signature is
 *                        to be verified.
 *  @param[in] DataInSize - The size, in bytes, of the data whose
 *                        signature is to be verified.
 *  @param[in] Sig_ptr - A pointer to the signature to be verified. 
 *                      The length of the signature is PubKey_ptr->N.len bytes 
 *                      (that is, the size of the modulus, in bytes).
 * @param[in] PKCS1_ver - Ver 1.5 or 2.1, according to the functionality required
 *
 * @return CRYSError_t - CRYS_OK, CRYS_INVALID_SIGNATURE,
 *                        CRYS_BAD_PARAM, CRYS_OUT_OF_RANGE 
**/
				     
CIMPORT_C CRYSError_t _DX_CRYS_KMNG_RSA_Verify(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
												CRYS_RSAPubUserContext_t	*UserContext_ptr,
												CRYS_RSA_HASH_OpMode_t		hashFunc,
												CRYS_PKCS1_MGF_t			MGF,
												DxUint16_t					SaltLen,
												DxUint8_t					*DataIn_ptr,
												DxUint32_t					DataInSize,
												DxUint8_t					*Sig_ptr,
												CRYS_PKCS1_version			PKCS1_ver);

				     
#define CRYS_KMNG_RSA_PKCS1v15_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,hashFunc,DataIn_ptr,DataInSize,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,hashFunc,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,DataInSize,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_KMNG_RSA_PKCS1v15_MD5_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes the DataIn_ptr data has already been hashed using MD5
   
   \note The data_in size is already known after the Hash
   
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_MD5_Verify   
*/

#define CRYS_KMNG_RSA_PKCS1v15_MD5_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_MD5_mode,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,CRYS_HASH_MD5_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA1
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify   
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA1_mode,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SHA224_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA224
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_PKCS1v15_SHA224_Verify   
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA224_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA224_mode,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)



/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SHA256_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA256
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_RSA_PKCS1v15_SHA256_Verify   
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA256_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA256_mode,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SHA384_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA384
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PKCS1v15_SHA384_Verify   
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA384_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA384_mode,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)



/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SHA512_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr data has already been hashed using SHA512
   
   Note: The data_in size is already known after the Hash
      
	The actual macro that will be used by the users is:
	CRYS_KMNG_RSA_PKCS1v15_SHA512_Verify   
*/
#define CRYS_KMNG_RSA_PKCS1v15_SHA512_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA512_mode,CRYS_PKCS1_NO_MGF,0,DataIn_ptr,CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER15)


/**
   \brief CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify implements the RSASSA-PKCS1v15 Verify algorithm
   as defined in PKCS#1 v1.5, but without operating the HASH function - 
   it assumes that the DataIn_ptr has already been hashed using SHA1
   
   Note: The data_in size is already known after Hash
   Note: According to the PKCS#1 ver2.1 it is not recommended to use MD5 Hash, 
		therefore it is not supported
	      
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify   
*/
		
#define CRYS_KMNG_RSA_PSS_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,hashFunc,MGF,SaltLen,DataIn_ptr,DataInSize,Sig_ptr,CRYS_PKCS1_VER21)
			   
			      
/**
   \brief CRYS_KMNG_RSA_PSS_SHA1_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA1
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_SHA1_Verify   
*/

#define CRYS_KMNG_RSA_PSS_SHA1_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA1_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)			      				      	      


/**
   \brief CRYS_KMNG_RSA_PSS_SHA224_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA224
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_SHA224_Verify   
*/

#define CRYS_KMNG_RSA_PSS_SHA224_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA224_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)

/**
   \brief CRYS_KMNG_RSA_PSS_SHA256_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA256
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_SHA256_Verify   
*/

#define CRYS_KMNG_RSA_PSS_SHA256_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA256_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)			      


/**
   \brief CRYS_KMNG_RSA_PSS_SHA384_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA384
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_SHA384_Verify   
*/

#define CRYS_KMNG_RSA_PSS_SHA384_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA384_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)


/**
   \brief CRYS_KMNG_RSA_PSS_SHA512_Verify implements the PKCS1v21 Verify algorithm
   as defined in PKCS#1 v2.1, but without operating the HASH function - 
   it assumes the DataIn_ptr has already been hashed using SHA512
      
   \note The data_in size is already known after the Hash
      
	The actual macro that will be used by the user is:
	CRYS_KMNG_RSA_PKCS1v15_SHA512_Verify   
*/

#define CRYS_KMNG_RSA_PSS_SHA512_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,MGF,SaltLen,DataIn_ptr,Sig_ptr)\
		_DX_CRYS_KMNG_RSA_Verify(RSA_Pub_WrappedKey_ptr,UserContext_ptr,CRYS_RSA_After_SHA512_mode,MGF,SaltLen,DataIn_ptr,CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES,Sig_ptr,CRYS_PKCS1_VER21)

/**
 * @brief
 * CRYS_KMNG_RSA_PRIM_Encrypt implements the RSAEP algorithm as defined in PKCS#1 v2.1 6.1.1
 * 
 * @param[in] CRYS_KMNG_RSA_PubWrappedKey_t - wrapped RSA public key from KMNG
 * @param[in] PrimeData_ptr - A pointer to a structure containing internal buffers
 * @param[in] Data_ptr - A pointer to the data to encrypt
 * @param[in] DataSize - The size, in bytes, of the data to encrypt.
 *                      \note This value must be <= the size of the modulus.
 * @param[out] Output_ptr - Pointer to the encrypted data.
 *                         The buffer must be at least PubKey_ptr->N.len bytes long 
 *                         (that is, the size of the modulus, in bytes).
 * 
 *   @return CRYSError_t - CRYS_OK,
 *                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
 *                         CRYS_RSA_PUB_KEY_VALIDATION_TAG_ERROR,
 *                         CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID,
 *                         CRYS_RSA_DATA_POINTER_INVALID_ERROR,
 *                         CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
 *                         CRYS_RSA_INVALID_MESSAGE_BUFFER_SIZE,
 *                         CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
 *                         CRYS_RSA_INVALID_MESSAGE_VAL
**/

CIMPORT_C CRYSError_t CRYS_KMNG_RSA_PRIM_Encrypt(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
												CRYS_RSAPrimeData_t	        *PrimeData_ptr,
												DxUint8_t           *Data_ptr,
												DxUint16_t          DataSize,
												DxUint8_t           *Output_ptr);


/**
 * @brief
 * CRYS_KMNG_RSA_PRIM_Decrypt implements the RSADP algorithm as defined in PKCS#1 v2.1 6.1.2
 * 
 *   
 *   @param[in] CRYS_KMNG_RSA_PrivWrappedKey_t - wrapped RSA private key from KMNG
 *   @param[in] PrimeData_ptr - A pointer to a structure containing internal buffers
 *                              required for the RSA operation                        
 *   @param[in] Data_ptr - A pointer to the data to be decrypted
 *   @param[in] DataSize - The size, in bytes, of the data to decrypt. 
 *                         \note Must be <= the size of the modulus.
 * 
 *   @param[out] Output_ptr - A pointer to the decrypted data.
 *                            The buffer must be at least PrivKey_ptr->N.len bytes long 
 *                            (that is, the size of the modulus, in bytes).
 * 
 *   @return CRYSError_t - CRYS_OK,
 *                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
 *                         CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID,
 *                         CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR,
 *                         CRYS_RSA_DATA_POINTER_INVALID_ERROR,
 *                         CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
 *                         CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
 *                         CRYS_RSA_INVALID_MESSAGE_VAL
**/
 
CIMPORT_C CRYSError_t CRYS_KMNG_RSA_PRIM_Decrypt(KMNG_RSA_WrappedKey_t	RSA_WrappedKey_ptr,
												CRYS_RSAPrimeData_t	        *PrimeData_ptr,
												DxUint8_t			*Data_ptr,
												DxUint16_t			DataSize,
												DxUint8_t			*Output_ptr);


/**
   \brief CRYS_KMNG_RSA_PRIM_Sign implements the RSASP1 algorithm as defined in PKCS#1 v2.1 6.2.1

 \def CRYS_KMNG_RSA_PRIM_Sign The signature primitive is identical to the decryption algorithm

*/
#define CRYS_KMNG_RSA_PRIM_Sign CRYS_KMNG_RSA_PRIM_Decrypt

/**
   \brief CRYS_KMNG_RSA_PRIM_Verify implements the RSAVP1 algorithm as defined in PKCS#1 v2.1 6.2.2

 \def CRYS_KMNG_RSA_PRIM_Verify The verification primitive is identical to the encryption algorithm

*/
#define CRYS_KMNG_RSA_PRIM_Verify CRYS_KMNG_RSA_PRIM_Encrypt


/**
 * @brief This function is used to execute AES wrap using key(in the context) from the key management.
 *
 * The function unwraps the key from KMNG and calls CRYS_AES_Wrap
 *
 *
 * @param[in] AES_WrappedKey_ptr - wrapped AES key from KMNG
 *
 * @param[in]  DataIn_ptr - A pointer to plain text data to be wrapped
                            NOTE: Overlapping between the data input and data output buffer
                            is not allowed, except the inplace case that is legal .                                  
 * @param[in]  DataInLen  - Length of data in bytes. DataLen must be multiple of 
                            8 bytes and  must be in range [16,  2^28].
 * @param[out] WrapDataOut_ptr -          A pointer to buffer for output of wrapped data.
 * @param[in/out] WrapDataLen_ptr - A pointer to a buffer for input of size of 
                                    user passed buffer and for output actual 
                                    size of unwrapped data in bytes. Buffer size must 
                                    be not less than DataLen+CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES.                         
 * @return CRYSError_t - CRYS_OK, or error message                         
                         CRYS_AES_WRAP_ILLEGAL_DATA_PTR_ERROR            
                         CRYS_AES_WRAP_DATA_LENGTH_ERROR                 
                         CRYS_AES_WRAP_ILLEGAL_KEY_PTR_ERROR             
                         CRYS_AES_WRAP_KEY_LENGTH_ERROR                  
                         CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_PTR_ERROR      
                         CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_LEN_PTR_ERROR  
                         CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_LENGTH_ERROR
                         CRYS_AES_WRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR 
*/

CIMPORT_C CRYSError_t  CRYS_KMNG_AESWrap(KMNG_AES_WrappedKey_t	AES_WrappedKey_ptr,
                                          DxUint8_t*              DataIn_ptr,      /*in*/   
                                          DxUint32_t              DataInLen,
                                          DxUint8_t*              WrapDataOut_ptr, /*out*/
                                          DxUint32_t*             WrapDataLen_ptr);



/**
   @brief  The CRYS_KMNG_AESUnwrap is used to execute AES unwrap on data using the key from KMNG
   @param[in]  AES_WrappedKey_ptr - key strucutre from KMNG
   @param[in]  WrapDataIn_ptr - A pointer to wrapped data to be unwrapped 
                                NOTE: Overlapping between the data input and data output buffer
                                      is not allowed, except the inplace case that is legal .                              
   @param[in]  WrapDataInLen  - Length of wrapped data in bytes. DataLen must be multiple of 
                                8 bytes and  must be in range [24, 2^29].
   @param[out] DataOut_ptr     - A pointer to buffer for output of unwrapped data.
   @param[in/out]  DataOutLen_ptr - A pointer to a buffer for input of size of user passed 
                              buffer and for output of actual size of unwrapped data in bytes.
                              DataOutLen must be multiple of 8 bytes and must be not less
                              than WrapDataInLen - CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES.
   @return CRYSError_t - CRYS_OK, or error message 
                         CRYS_AES_UNWRAP_WRAP_DATA_LENGTH_ERROR          
                         CRYS_AES_UNWRAP_ILLEGAL_KEY_PTR_ERROR           
                         CRYS_AES_UNWRAP_KEY_LEN_ERROR                   
                         CRYS_AES_UNWRAP_ILLEGAL_DATA_PTR_ERROR          
                         CRYS_AES_UNWRAP_ILLEGAL_DATA_LEN_PTR_ERROR      
                         CRYS_AES_UNWRAP_ILLEGAL_DATA_LENGTH_ERROR       
                         CRYS_AES_UNWRAP_FUNCTION_FAILED_ERROR 
                         CRYS_AES_UNWRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR 
                         CRYS_AES_UNWRAP_IS_SECRET_KEY_FLAG_ILLEGAL_ERROR
    NOTE:  On error exiting from function the output buffer may be zeroed by the function.
*/                               

CIMPORT_C  CRYSError_t CRYS_KMNG_AESUnwrap(KMNG_AES_WrappedKey_t  AES_WrappedKey_ptr,
                                           DxUint8_t*             WrapDataIn_ptr, 
                                           DxUint32_t             WrapDataInLen,
                                           DxUint8_t*             DataOut_ptr,
                                           DxUint32_t*            DataOutLen_ptr);


/**
 * @brief CRYS_DH_PKCS3_GetSecretKey computes the shared secret key in the following computation:
 	SecretKey = ServerPubKey ^ ClientPrvKey mod Prime
 
  	Note: All buffers arguments are represented in Big-Endian
 
  @param[in] ServerPubKey_ptr   - Pointer to the Server public key octet string - 
  @param[in] ServerPubKeySize   - The Server Public key Size (in bytes)
  @param[in] UserPubKey_ptr		- a pointer to the publick key structure. used for the Exp operation function
                                  the struct doesn't need to be initialized
  @param[in] PrimeData_ptr 		- a pointer to a structure containing internal buffers
                                  the struct doesn't need to be initialized    
  @param[out] SecretKey_ptr 	- Pointer to the secret key octet string.
  							  	  This buffer should be at least PrimeSize Bytes
  							  
  @param[in/out] SecretKeySize_ptr - The user should provide the actual size in bytes of the buffer indicated by SecretKey_ptr
                                    The function will return the actual size in bytes of the output secret key

  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
                        CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;      
   		                CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
		                CRYS_DH_SECRET_KEY_SIZE_OUTPUT_ERROR;
*/
CIMPORT_C CRYSError_t CRYS_KMNG_DH_PKCS3_GetSecretKey(KMNG_DH_WrappedKey_t  DH_WrappedKey_ptr,
													  DxUint8_t*            ServerPubKey_ptr,
													  DxUint16_t            ServerPubKeySize,
													  CRYS_DHUserPubKey_t*  UserPubKey_ptr,
													  CRYS_DHPrimeData_t*   PrimeData_ptr,
													  DxUint8_t*            SecretKey_ptr,
													  DxUint16_t*           SecretKeySize_ptr);

/**
 * @brief This function sets the verification function for the CRYS_KMNG APIs, and returns 
 *        pointer to function that was set previously
 *
 *
 * @param[in] newFunc_ptr - new verification function pointer 
 * @param[in/out] prevFunc_ptr - pointer to the previous verification function pointer 
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_AES_error.h
 */
CIMPORT_C void CRYS_KMNG_SetValidationCallbackFunc(	CRYS_KMNG_VerifyFunc newFunc_ptr,
													CRYS_KMNG_VerifyFunc* prevFunc_ptr);

#ifndef CRYS_NO_CMLA_SUPPORT
CIMPORT_C  CRYSError_t CRYS_KMNG_CMLA_RSA_Decrypt(  KMNG_RSA_WrappedKey_t CMLA_WrappedKey,
                                                  CRYS_CMLA_RSA_MESSAGE_t     EncryptMessage,
                                                  CRYS_CMLA_RSA_MESSAGE_t     Message,
                                                  CRYS_CMLA_RSA_TempBuff_t   *TempBuff_ptr);


CIMPORT_C  CRYSError_t CRYS_KMNG_CMLA_Unwrap(KMNG_AES_WrappedKey_t           CMLA_WrappedKey,
                                             CRYS_CMLA_WRAPPED_KEY_t         WrappedKey,
                                             CRYS_CMLA_UNWRAPPED_KEY_t       UnwrappedKey);

#endif
#ifdef __cplusplus
}
#endif

#endif
