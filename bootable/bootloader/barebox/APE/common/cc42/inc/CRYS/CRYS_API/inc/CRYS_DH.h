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
 
 
#ifndef _15_CRYS_DH_h_H
#define _15_CRYS_DH_h_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */



#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  Object name     :  CRYS_DH.h
 */

  /*
   *  Object %name    : %CRYS_DH.h
   *  State           :  %state%
   *  Creation date   :  Wed Jan 26 16:54:02 2005
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module defines the API that supports Diffie-Hellman key exchange 
   *  as defined in PKCS#3 and ANSI9.42 v1.4 (Revised from November 1,1993)
   *
   *  \version CRYS_DH.h#1:incl:15
   *  \author ohads
   */

#include "CRYS_RSA_Types.h"
#include "CRYS_KDF.h"

/************************ Defines ******************************/

/*The following defines are for the readability of the user*/
#define CRYS_DHPrimeData_t   CRYS_RSAPrimeData_t
#define CRYS_DHUserPubKey_t  CRYS_RSAUserPubKey_t
#define CRYS_DH_OtherInfo_t  CRYS_KDF_OtherInfo_t
#define CRYS_DH_COUNT_OF_OTHER_INFO_ENTRIES  CRYS_KDF_COUNT_OF_OTHER_INFO_ENTRIES


/************************ Enums ********************************/

/* DH operations mode (used in some functions of DH) */
typedef enum
{
   CRYS_DH_PKCS3_mode  = 0,
   CRYS_DH_ANSI_X942_mode = 1,
   CRYS_DH_NumOfModes,

   CRYS_DH_OpModeLast    = 0x7FFFFFFF,

}CRYS_DH_OpMode_t;   

/* HASH operation modes, used in DH */
typedef enum
{
	CRYS_DH_HASH_SHA1_mode		= CRYS_HASH_SHA1_mode,
	CRYS_DH_HASH_SHA224_mode	= CRYS_HASH_SHA224_mode,
	CRYS_DH_HASH_SHA256_mode	= CRYS_HASH_SHA256_mode,
	CRYS_DH_HASH_SHA384_mode	= CRYS_HASH_SHA384_mode,
	CRYS_DH_HASH_SHA512_mode	= CRYS_HASH_SHA512_mode,
	CRYS_DH_HASH_MD5_mode       = CRYS_HASH_MD5_mode, /* not used in DH */

	CRYS_DH_HASH_NumOfModes		= CRYS_HASH_MD5_mode,
	CRYS_DH_HASH_OperationModeLast = 0x7FFFFFFF,

}CRYS_DH_HASH_OpMode_t;   

/* key derivation modes, used in DH */
typedef enum
{
	CRYS_DH_ASN1_Der_mode    = CRYS_KDF_ASN1_DerivMode,
	CRYS_DH_Concat_Der_mode  = CRYS_KDF_ConcatDerivMode,
	CRYS_DH_X963_DerMode    = CRYS_KDF_ConcatDerivMode,
	CRYS_DH_OMADRM_DerMode  = CRYS_KDF_OMADRM_DerivMode, /* not used in DH */

	CRYS_DH_der_NumOfModes  = CRYS_DH_OMADRM_DerMode,

	CRYS_DH_DerivationFunc_ModeLast= 0x7FFFFFFF,

}CRYS_DH_DerivationFunc_Mode;


/************************ Typedefs  *************************************/

/************************ Structs  **************************************/

/************************ Public Variables ******************************/

/************************ Public Functions ******************************/

/*******************************************************************************************/

/**
 * @brief _DX_DH_GeneratePubPrv has 2 functionalities:

 	   1. Randomly generates the Client private key Prv.
 	   2. Compute the Client public key which is 
 		  ClientPub = Generator ^ Prv mod Prime. 
 		
 	Note: All buffers arguments are represented in Big-Endian
 	
  @param[in] Generator_ptr 		- Pointer to the Generator octet string
  @param[in] GeneratorSize 		- Size of the Generator String (in bytes)
  @param[in] Prime_ptr 			- Pointer to the Prime octet string P.
  @param[in] PrimeSize 			- Size of the Prime string (in bytes)
  @param[in] L 					- Exact length of Private key in bits (relevant only for PKCS#3), defined 
                                  by central authority. 
  								   - If L != 0, then L must met to requirements:  1 <= L and 2^(L-1) <= P,
								     Note: in our implementation required L >= 3.
								     in this case force the private key size to be 2^(L-1) <= Prv < 2^l.
  				 				   - If L  = 0, then: 0 < Prv < P-1.
  @param[in] Q_ptr 				- Pointer to the order Q of generator relevant only for ANSI X9.42):
  									1<= Prv <= q-1   or   1 < Prv < q-1.
  @param[in] QSize	 			- Size of the Q string in bytes. Relevant only for ANSI X9.42.   				 				  
  @param[in] DH_mode			- Enumerator, declaring whether the mode is PKCS#3 or ANSI X9.42.  
  @param[in] UserPubKey_ptr		- A pointer to the public key structure. Used for the Exp operation.
  								  The structure doesn't need to be initialized.
  @param[in] PrimeData_ptr 		- A pointer to a structure containing internal buffers, the structure
                                  doesn't need to be initialized.
  @param[out] ClientPrvKey_ptr  - Pointer to the buffer for output the Private key octet string (Prv). 
  							  	  In PKCS#3 this buffer should be at least the following size:
	  							  	  - if L is provided the size of buffer: bufSizeBytes*8 >= L.
	  							  	  - if L is DX_NULL, then size of buffer >= PrimeSize.
								  In X9.42 size of buffer >= QSize.	  							  	  
  @param[in/out] ClientPrvKeySize_ptr - The user should provide the size of the buffer indicated by ClientPrvKey_ptr.
                                        The function returns the actual size in bytes of the Private key.
  @param[out] ClientPub_ptr 	- Pointer to the Public key octet string
  						  		  This Buffer should be at least PrimeSize bytes
  							  
  @param[in/out] ClientPubSize_ptr -    The user should provide the size of the buffer indicated by ClientPub_ptr
                                        The function returns the actual size in bytes of the generated client public key.
 
  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
                        CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;      
   						CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
    					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;   						
						CRYS_DH_ARGUMENT_GENERATOR_SMALLER_THAN_ZERO_ERROR;
						CRYS_DH_ARGUMENT_PRIME_SMALLER_THAN_GENERATOR_ERROR;						
						CRYS_DH_INVALID_L_ARGUMENT_ERROR;
						CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;						
    					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
 */
CIMPORT_C CRYSError_t _DX_DH_GeneratePubPrv(
										DxUint8_t *Generator_ptr,              /*generator*/
										DxUint16_t GeneratorSize,
										DxUint8_t *Prime_ptr,                  /*modulus*/
										DxUint16_t PrimeSize,
										DxUint16_t L,
										DxUint8_t *Q_ptr,                      /*order*/ 
										DxUint16_t QSize,
										CRYS_DH_OpMode_t DH_mode,
										CRYS_DHUserPubKey_t *tmpPubKey_ptr,    /*temp buff*/
										CRYS_DHPrimeData_t  *tmpPrimeData_ptr, /*temp buff*/
										DxUint8_t *ClientPrvKey_ptr,           /*out*/
										DxUint16_t *ClientPrvKeySize_ptr,      /*in/out*/
										DxUint8_t *ClientPub1_ptr,             /*out*/
										DxUint16_t *ClientPubSize_ptr);        /*in/out*/


/* macro for calling the GeneratePubPrv function on PKCS#3 mode:  Q is irrelevant */
#define CRYS_DH_PKCS3_GeneratePubPrv(Generator_ptr,GeneratorSize,\
									Prime_ptr,PrimeSize,\
									L,\
									tmpPubKey_ptr,tmpPrimeData_ptr,\
									ClientPrvKey_ptr,ClientPrvKeySize_ptr,\
									ClientPub_ptr,ClientPubSize_ptr)\
		_DX_DH_GeneratePubPrv(Generator_ptr,GeneratorSize,\
								Prime_ptr,PrimeSize,\
								L,\
								(DxUint8_t *)DX_NULL,(DxUint16_t)0,\
								CRYS_DH_PKCS3_mode,\
								tmpPubKey_ptr,tmpPrimeData_ptr,\
								ClientPrvKey_ptr,ClientPrvKeySize_ptr,\
								ClientPub_ptr,ClientPubSize_ptr)

/* macro for calling the GeneratePubPrv function on ANSI X9.42 mode:  L is irrelevant */
#define CRYS_DH_ANSI_X942_GeneratePubPrv(Generator_ptr,GeneratorSize,Prime_ptr,PrimeSize,\
									Q_ptr,QSize,\
									tmpPubKey_ptr,tmpPrimeData_ptr,\
									ClientPrvKey_ptr,ClientPrvKeySize_ptr,\
									ClientPub_ptr,ClientPubSize_ptr)\
		_DX_DH_GeneratePubPrv(Generator_ptr,GeneratorSize,\
								Prime_ptr,PrimeSize,\
								(DxUint16_t)0,\
								Q_ptr,QSize,\
								CRYS_DH_ANSI_X942_mode,\
								tmpPubKey_ptr,tmpPrimeData_ptr,\
								ClientPrvKey_ptr,ClientPrvKeySize_ptr,\
								ClientPub_ptr,ClientPubSize_ptr)

/*******************************************************************************************/
/**
 * @brief CRYS_DH_GetSecretKey computes the shared secret key in the following computation:
 	               SecretKey = ServerPubKey ^ ClientPrvKey mod Prime.

   Note:
     - All buffers arguments are represented in Big-Endian.
	 - The actual size of private key in bits must be not less than 2 and not great, 
	   than actual size of Prime (modulus) in bits (in our implementation); 
	 - The user may call this function also by the following macro 
	   CRYS_DH_PKCS3_GetSecretKey(), inserted for compatibility with other applications;
 
  @param[in] ClientPrvKey_ptr 	- Pointer to the private key octet string. 
  @param[in] ClientPrvKeySize  	- The Private key Size (in bytes).
  @param[in] ServerPubKey_ptr   - Pointer to the Server public key octet string - 
  @param[in] ServerPubKeySize   - The Server Public key Size (in bytes)
  @param[in] Prime_ptr 			- Pointer to the Prime octet string.
  @param[in] PrimeSize 			- Size of the Prime string. 
  @param[in] UserPubKey_ptr		- A pointer to the public key structure. used for the Exp operation function
                                  the struct doesn't need to be initialized.
  @param[in] PrimeData_ptr 		- A pointer to a structure containing internal buffers
                                  the struct doesn't need to be initialized.    
  @param[out] SecretKey_ptr 	- Pointer to the secret key octet string.
  							  	  This buffer should be at least PrimeSize Bytes.  							  
  @param[in/out] SecretKeySize_ptr - The user should provide the actual size in bytes of the buffer indicated by SecretKey_ptr
                                    The function will return the actual size in bytes of the output secret key

  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
					    CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;      
   					    CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
					    CRYS_DH_SECRET_KEY_SIZE_OUTPUT_ERROR;
*/
 CIMPORT_C CRYSError_t CRYS_DH_GetSecretKey(
	                                DxUint8_t *ClientPrvKey_ptr,
				                    DxUint16_t ClientPrvKeySize,
 				                    DxUint8_t *ServerPubKey_ptr,
				                    DxUint16_t ServerPubKeySize,				                    
				                    DxUint8_t *Prime_ptr,
				                    DxUint16_t PrimeSize,
                                    CRYS_DHUserPubKey_t *tmpUserPubKey_ptr,
                                    CRYS_DHPrimeData_t  *tmpPrimeData_ptr, 				                    
				                    DxUint8_t *SecretKey_ptr,
				                    DxUint16_t *SecretKeySize_ptr);


/* macro for calling the DH_GetSecretKey function on PKCS#3 mode */
#define CRYS_DH_PKCS3_GetSecretKey( ClientPrvKey_ptr, ClientPrvKeySize, \
 				                    ServerPubKey_ptr, ServerPubKeySize, Prime_ptr, PrimeSize, \
                                    tmpUserPubKey_ptr, tmpPrimeData_ptr, \
				                    SecretKey_ptr, SecretKeySize_ptr ) \
        CRYS_DH_GetSecretKey( (ClientPrvKey_ptr), (ClientPrvKeySize), \
 				                    (ServerPubKey_ptr), (ServerPubKeySize), (Prime_ptr), (PrimeSize), \
                                    (tmpUserPubKey_ptr), (tmpPrimeData_ptr), \
				                    (SecretKey_ptr), (SecretKeySize_ptr) )



/****************************************************************************************/
/**
* @brief CRYS_DH_KeyDerivFunc - macro for calling the _DX_KDF_KeyDerivFunc with full functionality
*        and explicit given arguments.  
*
*        This macro is recommended to use in all projects instead old macros.
*
*        For detailed description of arguments and requirements see definitions of the function
*        _DX_KDF_KeyDerivFunc in file CRYS_KDF.h
*/				                    
#define CRYS_DH_KeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,hashMode,derivation_mode,KeyingData_ptr,KeyLenInBits)\
	_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(hashMode),(derivation_mode),(KeyingData_ptr),(KeyLenInBits))

 /**
 * @brief CRYS_DH_Asn1KeyDerivFunc - macro for calling the _DX_KDF_KeyDerivFunc on
 *        ASN1 mode with OtherInfo argument.  
 *
 *        For detailed description of arguments and requirements see definitions of the function
 *        _DX_KDF_KeyDerivFunc in file CRYS_KDF.h
*/				                    
#define CRYS_DH_Asn1KeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,hashMode,KeyingData_ptr,KeyLenInBits)\
	_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(hashMode),(CRYS_DH_ASN1_Der_mode),(KeyingData_ptr),(KeyLenInBits))

 /**
 * @brief CRYS_DH_ConcatKeyDerivFunc - macro for calling the _DX_KDF_KeyDerivFunc on
 *        concatenation mode with OtherInfo argument.  
 *
 *        For detailed description of arguments and requirements see definitions of the function
 *        _DX_KDF_KeyDerivFunc in file CRYS_KDF.h
 */
#define CRYS_DH_ConcatKeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,hashMode,KeyLenInBits,KeyingData_ptr)\
	_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(hashMode),(CRYS_DH_Concat_Der_mode),(KeyingData_ptr),(KeyLenInBits))

 
 /****************************************************************************************/
 /**
 * @brief CRYS_DH_ASN1_KeyDerivation_SHA1Func - macro for calling the _DX_DH_KeyDerivationFunc on
 *        ASN1 SHA1 mode without OtherInfo argument.  
 *        
 *        The macro is given for backward compatibility of old projects.
 */				                    
#define CRYS_DH_ASN1_KeyDerivation_SHA1Func(ZZSecret_ptr,ZZSecretSize,KeyingData_ptr,KeyLenInBits)\
	_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(DX_NULL),(CRYS_DH_HASH_SHA1_mode),(CRYS_DH_ASN1_Der_mode),(KeyingData_ptr),(KeyLenInBits))
				                    
/**
* @brief CRYS_DH_Concat_KeyDerivation_SHA1Func - macro for calling the _DX_DH_KeyDerivationFunc on
*        concatenation SHA1 mode without OtherInfo argument.  
*
*        The macro is given for backward compatibility of old projects.
*/
#define CRYS_DH_Concat_KeyDerivation_SHA1Func(ZZSecret_ptr,ZZSecretSize,KeyingData_ptr,KeyLenInBits)\
	_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(DX_NULL),(CRYS_DH_HASH_SHA1_mode),(CRYS_DH_Concat_Der_mode),(KeyingData_ptr),(KeyLenInBits))


/******************************************************************************************/
/**
 * @brief CRYS_DH_X942_GetSecretData computes the shared secret key as follows:
 		1. OPTIONAL - validate the correctness of the argument keys
 		2. SecretKey = ServerPubKey ^ ClientPrvKey mod Prime
 		3. Use of Derivation function to derive a data key from the secret key 
 
 	The actual APIs that will be used by the user are:
 	CRYS_DH_X942_GetSecret_ASN1_2_Data
 	CRYS_DH_X942_GetSecret_Concat_2_Data
 
  	Note: All buffers arguments are represented in Big-Endian format

  @param[in] tmpUserPubKey_ptr	- A pointer to the public key data structure. Not initialized.
  @param[in] tmpPrimeData_ptr 	- A pointer to a CRYS_RSAPrimeData_t structure 
   								  that is used for the Exp operation
  @param[in] hashMode			- The hash function to be used
  @param[in] ClientPrvKey_ptr 	- A pointer to the Private key octet string 
  @param[in] ClientPrvKeySize  	- The Private key size, in bytes
  @param[in] ServerPubKey_ptr   - A pointer to the Server public key octet string - 
  @param[in] ServerPubKeySize   - The Server Public key size, in bytes
  @param[in] Prime_ptr 			- A pointer to the Prime octet string
  @param[in] PrimeSize 			- The size of the Prime string
  @param[in] DerFunc_mode 		- The type of function to derive the secret key to the key data.
  								  We use ASN.1-based function or Hash concatenation function mode.
  @param[in] OtherInfo          - The pointer to predefined structure, containing pointers and sizes of optional data  
                                  shared by two entities intended to share the secret value. This argument (structure) 
							      and also its members are optional (if any is not need - set its pointer and size to NULL).
							      Note: OtherInfo not includes Counter, because the Counter is calculated inside the 
							      function .
  @param[out] SecretKeyData_ptr	- A pointer to the secret key octet string. 
				  				  This buffer should be at least PrimeSize bytes.
  @param[in] SecretKeyDataSizeBitsNeeded - Specifies the derived Secret Key data size needed in Bits. This value converted to bytes 
                                  cannot be larger than PrimeSize (required by implementation)

				  Note: deleting of private ephemeral key (if it used) is out of scope of this function and must be performed 
				  by user after calling this function. 

  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR;
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_BIGGER_THAN_PRIME_SIZE;
*/
CIMPORT_C CRYSError_t CRYS_DH_X942_GetSecretData(
	                                   CRYS_DHUserPubKey_t        *tmpUserPubKey_ptr,
                                       CRYS_DHPrimeData_t         *tmpPrimeData_ptr, 
                                       CRYS_DH_HASH_OpMode_t       hashMode,									
                                       DxUint8_t                  *ClientPrvKey_ptr,
                                       DxUint16_t                  ClientPrvKeySize,
                                       DxUint8_t                  *ServerPubKey_ptr,
                                       DxUint16_t                  ServerPubKeySize,				                    
                                       DxUint8_t                  *Prime_ptr,
                                       DxUint16_t                  PrimeSize,
                                       CRYS_DH_DerivationFunc_Mode DerivFunc_mode,
									   CRYS_DH_OtherInfo_t        *otherInfo_ptr,
                                       DxUint8_t                  *SecretKeyData_ptr,
                                       DxUint16_t                  SecretKeyDataSizeBitsNeeded );

/****************************************************************/
/**
   \@brief CRYS_DH_X942_GetSecretDataAsn1 implements the DH ANSI X9.42 standard. It derives a secret key 
   		  using the Derivation function based on ASN1.
          Note: deleting of private ephemeral key (if it used) is out of scope of this function and must be performed 
                by user after calling this function. 
*/
#define CRYS_DH_X942_GetSecretDataAsn1(UserPubKey_ptr,PrimeData_ptr,hashMode,ClientPrvKey_ptr,ClientPrvKeySize,ServerPubKey_ptr,ServerPubKeySize,Prime_ptr,PrimeSize,otherInfo_ptr,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_GetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashMode),(ClientPrvKey_ptr),(ClientPrvKeySize),(ServerPubKey_ptr),(ServerPubKeySize),(Prime_ptr),(PrimeSize),(CRYS_DH_ASN1_Der_mode),(otherInfo_ptr),(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))
/**
   \@brief  CRYS_DH_X942_GetSecretDataConcat implements the DH ANSI X9.42 standard. It derives a secret key 
   			using the Derivation function based on concatenation of SHA1 hash 
          Note: deleting of private ephemeral key (if it used) is out of scope of this function and must be performed 
                by user after calling this function. 
*/				                    
#define CRYS_DH_X942_GetSecretDataConcat(UserPubKey_ptr,PrimeData_ptr,hashMode,ClientPrvKey_ptr,ClientPrvKeySize,ServerPubKey_ptr,ServerPubKeySize,Prime_ptr,PrimeSize,otherInfo_ptr,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_GetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashMode),(ClientPrvKey_ptr),(ClientPrvKeySize),(ServerPubKey_ptr),(ServerPubKeySize),(Prime_ptr),(PrimeSize),(CRYS_DH_Concat_Der_mode),(otherInfo_ptr),(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))
				                    
/****************************************************************/
/*  The next two macros are given for backward compatibility    */
/*  of old projects and should not be used in new development   */
/****************************************************************/
/**
\@brief CRYS_DH_X942_GetSecret_ASN1_2_Data implements the DH ANSI X9.42 standard. It derives a secret key 
using the Derivation function based on ASN1.
Note: deleting of private ephemeral key (if it used) is out of scope of this function and must be performed 
by user after calling this function. 
*/
#define CRYS_DH_X942_GetSecret_ASN1_2_Data(UserPubKey_ptr,PrimeData_ptr,hashMode,ClientPrvKey_ptr,ClientPrvKeySize,ServerPubKey_ptr,ServerPubKeySize,Prime_ptr,PrimeSize,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_GetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashMode),(ClientPrvKey_ptr),(ClientPrvKeySize),(ServerPubKey_ptr),(ServerPubKeySize),(Prime_ptr),(PrimeSize),(CRYS_DH_ASN1_Der_mode),DX_NULL,(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))
/**
\@brief  CRYS_DH_X942_GetSecret_Concat_2_Data implements the DH ANSI X9.42 standard. It derives a secret key 
using the Derivation function based on concatenation of SHA1 hash 
Note: deleting of private ephemeral key (if it used) is out of scope of this function and must be performed 
by user after calling this function. 
*/				                    
#define CRYS_DH_X942_GetSecret_Concat_2_Data(UserPubKey_ptr,PrimeData_ptr,hashMode,ClientPrvKey_ptr,ClientPrvKeySize,ServerPubKey_ptr,ServerPubKeySize,Prime_ptr,PrimeSize,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_GetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashMode),(ClientPrvKey_ptr),(ClientPrvKeySize),(ServerPubKey_ptr),(ServerPubKeySize),(Prime_ptr),(PrimeSize),(CRYS_DH_Concat_Der_mode),DX_NULL,(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))


/****************************************************************/
/**
 * @brief CRYS_DH_X942_HybridGetSecretData computes the shared secret key as follows:
 		1. OPTIONAL - validate the correctness of the argument keys
 		2. SecretKey1 = ServerPubKey1 ^ ClientPrvKey1 mod Prime
 		3. SecretKey2 = ServerPubKey2 ^ ClientPrvKey2 mod Prime
 		3. Use of Derivation function to derive a data key from the 2 secret keys 
 
 	The actual APIs that will be used by the user are:
 	CRYS_DH_X942_Hybrid_GetSecret_ASN1_2_Data
 	CRYS_DH_X942_Hybrid_GetSecret_Concat_2_Data
 
  	Note: All buffers arguments are represented in Big-Endian format.
 
  @param[in] tmpUserPubKey_ptr	- A pointer to the public key data structure. Not initialized.
  @param[in] tmpPrimeData_ptr 	- A pointer to a CRYS_RSAPrimeData_t structure 
                                  that is used for the Exp operation
  @param[in] hashMode			- The hash function to be used.
  @param[in] ClientPrvKey_ptr1 	- A pointer to the First Private key octet string number 
  @param[in] ClientPrvKeySize1 	- The First Private key Size, in bytes
  @param[in] ClientPrvKey_ptr2 	- A pointer to the Second Private key octet string
  @param[in] ClientPrvKeySize2 	- The Second Private key Size, in bytes
  @param[in] ServerPubKey_ptr1  - A pointer to the First Server public key octet string
  @param[in] ServerPubKeySize1  - The First Server Public key Size, in bytes
  @param[in] ServerPubKey_ptr2  - A pointer to the Second Server public key octet string
  @param[in] ServerPubKeySize2  - The Second Server Public key Size, in bytes
  @param[in] Prime_ptr 			- A pointer to the Prime octet string
  @param[in] PrimeSize 			- The size of the Prime string
  @param[in] DerFunc_mode 		- The type of function to derive the secret key to the key data.
                                  We use an ASN.1-based function or a Hash concatenation function mode.
  @param[in] OtherInfo          - The pointer to predefined structure, containing pointers and sizes of optional data  
                                  shared by two entities intended to share the secret value. This argument (structure) 
							      and also its members are optional (if any is not need - set its pointer and size to NULL).
							      Note: OtherInfo not includes Counter, because the Counter is calculated inside the 
							      function .
  @param[out] SecretKeyData_ptr - A pointer to the secret key octet string.
			  	                  This buffer should be at least 2*PrimeSize bytes.
  							  
  @param[in] SecretKeyDataSizeBitsNeeded - Specifies the derived Secret Key data size needed in Bits. This value,  
                                  converted to bytes, cannot be larger than 2*PrimeSize (required by implementation). 
								  The size of buffer for secret key data (in bytes) must be not less, than 
								  2*PrimeSize (also required by implementation).

  @return CRYSError_t - On success the value CRYS_OK is returned, and on failure an ERROR as defined in CRYS_DH_error.h:
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR;
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_BIGGER_THAN_PRIME_SIZE;
						CRYS_DH_X942_HYBRID_SIZE1_BUFFER_ERROR;
                        CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;      
   						CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
    					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;   						
						CRYS_DH_ARGUMENT_GENERATOR_SMALLER_THAN_ZERO_ERROR;
						CRYS_DH_ARGUMENT_PRIME_SMALLER_THAN_GENERATOR_ERROR;						
						CRYS_DH_INVALID_L_ARGUMENT_ERROR;
						CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;						
    					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
*/
CIMPORT_C CRYSError_t CRYS_DH_X942_HybridGetSecretData(
	                                          CRYS_DHUserPubKey_t  *tmpUserPubKey_ptr,
                                              CRYS_DHPrimeData_t   *tmpPrimeData_ptr, 
                                              CRYS_DH_HASH_OpMode_t hashMode,		
                                              DxUint8_t            *ClientPrvKey_ptr1,
                                              DxUint16_t            ClientPrvKeySize1,
                                              DxUint8_t            *ClientPrvKey_ptr2,
                                              DxUint16_t            ClientPrvKeySize2,				                    
                                              DxUint8_t            *ServerPubKey_ptr1,
                                              DxUint16_t            ServerPubKeySize1,
                                              DxUint8_t            *ServerPubKey_ptr2,
                                              DxUint16_t            ServerPubKeySize2,				                    				                    
                                              DxUint8_t            *Prime_ptr,
                                              DxUint16_t            PrimeSize,
                                              CRYS_DH_DerivationFunc_Mode DerivFunc_mode,
											  CRYS_DH_OtherInfo_t  *otherInfo_ptr,
                                              DxUint8_t            *SecretKeyData_ptr,
                                              DxUint16_t            SecretKeyDataSizeBitsNeeded );

				                    			  
/****************************************************************/
/**
\brief   The CRYS_DH_X942_HybridGetSecretDataAsn1 function implements the DH ANSI X9.42 
standard for deriving a hybrid secret key from 2 public-private pair of keys using 
the Derivation function based on ASN1.
*/
#define CRYS_DH_X942_HybridGetSecretDataAsn1(UserPubKey_ptr,PrimeData_ptr,hashFunc,ClientPrvKey_ptr1,ClientPrvKeySize1,ClientPrvKey_ptr2,ClientPrvKeySize2,ServerPubKey_ptr1,ServerPubKeySize1,ServerPubKey_ptr2,ServerPubKeySize2,Prime_ptr,PrimeSize,otherInfo_ptr,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_HybridGetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashFunc),(ClientPrvKey_ptr1),(ClientPrvKeySize1),(ClientPrvKey_ptr2),(ClientPrvKeySize2),(ServerPubKey_ptr1),(ServerPubKeySize1),(ServerPubKey_ptr2),(ServerPubKeySize2),(Prime_ptr),(PrimeSize),CRYS_DH_ASN1_Der_mode,(otherInfo_ptr),(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))

/**
   \brief CRYS_DH_X942_HybridGetSecretDataConcat implements the DH ANSI X9.42 standard deriving a hybrid secret key 
   			from 2 public-private pair of keys using the Derivation function based on concatenation using SHA-1 Hash
*/				                    			  
#define CRYS_DH_X942_HybridGetSecretDataConcat(UserPubKey_ptr,PrimeData_ptr,hashFunc,ClientPrvKey_ptr1,ClientPrvKeySize1,ClientPrvKey_ptr2,ClientPrvKeySize2,ServerPubKey_ptr1,ServerPubKeySize1,ServerPubKey_ptr2,ServerPubKeySize2,Prime_ptr,PrimeSize,otherInfo_ptr,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
        CRYS_DH_X942_HybridGetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashFunc),(ClientPrvKey_ptr1),(ClientPrvKeySize1),(ClientPrvKey_ptr2),(ClientPrvKeySize2),(ServerPubKey_ptr1),(ServerPubKeySize1),(ServerPubKey_ptr2),(ServerPubKeySize2),(Prime_ptr),(PrimeSize),CRYS_DH_Concat_Der_mode,(otherInfo_ptr),(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))
                             
/****************************************************************/
/*  The next two macros are given for backward compatibility    */
/*  of old projects and should not be used in new development   */
/****************************************************************/
/**
\brief   The CRYS_DH_X942_Hybrid_GetSecret_ASN1_2_Data function implements the DH ANSI X9.42 
standard for deriving a hybrid secret key from 2 public-private pair of keys using 
the Derivation function based on ASN1.
*/
#define CRYS_DH_X942_Hybrid_GetSecret_ASN1_2_Data(UserPubKey_ptr,PrimeData_ptr,hashFunc,ClientPrvKey_ptr1,ClientPrvKeySize1,ClientPrvKey_ptr2,ClientPrvKeySize2,ServerPubKey_ptr1,ServerPubKeySize1,ServerPubKey_ptr2,ServerPubKeySize2,Prime_ptr,PrimeSize,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_HybridGetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashFunc),(ClientPrvKey_ptr1),(ClientPrvKeySize1),(ClientPrvKey_ptr2),(ClientPrvKeySize2),(ServerPubKey_ptr1),(ServerPubKeySize1),(ServerPubKey_ptr2),(ServerPubKeySize2),(Prime_ptr),(PrimeSize),CRYS_DH_ASN1_Der_mode,DX_NULL,(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))

/**
\brief CRYS_DH_X942_Hybrid_GetSecret_Concat_2_Data implements the DH ANSI X9.42 standard deriving a hybrid secret key 
from 2 public-private pair of keys using the Derivation function based on conctenation using SHA-1 Hash
*/				                    			  
#define CRYS_DH_X942_Hybrid_GetSecret_Concat_2_Data(UserPubKey_ptr,PrimeData_ptr,hashFunc,ClientPrvKey_ptr1,ClientPrvKeySize1,ClientPrvKey_ptr2,ClientPrvKeySize2,ServerPubKey_ptr1,ServerPubKeySize1,ServerPubKey_ptr2,ServerPubKeySize2,Prime_ptr,PrimeSize,SecretKeyData_ptr,SecretKeyDataSizeBitsNeeded)\
	CRYS_DH_X942_HybridGetSecretData((UserPubKey_ptr),(PrimeData_ptr),(hashFunc),(ClientPrvKey_ptr1),(ClientPrvKeySize1),(ClientPrvKey_ptr2),(ClientPrvKeySize2),(ServerPubKey_ptr1),(ServerPubKeySize1),(ServerPubKey_ptr2),(ServerPubKeySize2),(Prime_ptr),(PrimeSize),CRYS_DH_Concat_Der_mode,DX_NULL,(SecretKeyData_ptr),(SecretKeyDataSizeBitsNeeded))


#ifdef __cplusplus
}
#endif

#endif
