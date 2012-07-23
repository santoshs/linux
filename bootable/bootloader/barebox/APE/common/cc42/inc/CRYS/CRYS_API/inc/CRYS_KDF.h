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
 
 
#ifndef CRYS_KDF_h_H
#define CRYS_KDF_h_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */



#ifdef __cplusplus
extern "C"
{
#endif

/*
 *  Object name     :  CRYS_KDF.h
 */

  /*
   *  Object %name    : %CRYS_KDF.h
   *  State           :  %state%
   *  Creation date   :  13 May 2007.
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module defines the API that supports Key derivation function in modes
   *  as defined in PKCS#3, ,ANSI X9.42-2001, ANSI X9.63-1999, OMA-TS-DRM-DRM-V2_0-20050712-C.
   *
   *  \version CRYS_KDF.h#1:incl:15
   *  \author R.Levin (using part of code created by ohads)
   */

#include "CRYS_HASH.h"


/************************ Defines ******************************/

/* Count of OtherInfo entries (pointers to data buffers) */
#define  CRYS_KDF_COUNT_OF_OTHER_INFO_ENTRIES   5

/************************ Enums ********************************/

typedef enum
{
   CRYS_KDF_HASH_SHA1_mode      = 0,
   CRYS_KDF_HASH_SHA224_mode	= 1,
   CRYS_KDF_HASH_SHA256_mode	= 2,
   CRYS_KDF_HASH_SHA384_mode	= 3,
   CRYS_KDF_HASH_SHA512_mode	= 4,

   CRYS_KDF_HASH_NumOfModes,

   CRYS_KDF_HASH_OpModeLast    = 0x7FFFFFFF,

}CRYS_KDF_HASH_OpMode_t;   

typedef enum
{
   CRYS_KDF_ASN1_DerivMode    = 0,
   CRYS_KDF_ConcatDerivMode   = 1,
   CRYS_KDF_X963_DerivMode    = CRYS_KDF_ConcatDerivMode,
   CRYS_KDF_OMADRM_DerivMode  = 2,

   CRYS_KDF_DerivFunc_NumOfModes,

   CRYS_KDF_DerivFuncModeLast= 0x7FFFFFFF,

}CRYS_KDF_DerivFuncMode_t;   

/************************ Typedefs  ****************************/

/* Structure, containing the optional data for KDF, 
   if any data is not needed, then the pointer value and 
   the size must be set to NULL */ 
typedef struct
{
   /* a unique object identifier (OID), indicating algorithm(s) 
     for which the keying data will be used*/
   DxUint8_t   *AlgorithmID_ptr; 
   DxUint32_t   SizeOfAlgorithmID;
   /* Public information contributed by the initiator */
   DxUint8_t   *PartyUInfo_ptr; 
   DxUint32_t   SizeOfPartyUInfo;
   /* Public information contributed by the responder */   
   DxUint8_t   *PartyVInfo_ptr; 
   DxUint32_t   SizeOfPartyVInfo;
   /* Mutually-known private information, e.g. shared information 
   communicated throgh a separate channel */
   DxUint8_t   *SuppPrivInfo_ptr; 
   DxUint32_t   SizeOfSuppPrivInfo;
    /* Mutually-known public information, */
   DxUint8_t   *SuppPubInfo_ptr; 
   DxUint32_t   SizeOfSuppPubInfo;

}CRYS_KDF_OtherInfo_t;

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/

/****************************************************************/


/*********************************************************************************************************/
/**
 * @brief _DX_KDF_KeyDerivFunc performs key derivation according to one of some modes defined in standards: 
 			   ANSI X9.42-2001, ANSI X9.63, OMA_TS_DRM_DRM_V2_0-20050712-C.

			   The present implementation of the function allows the following operation modes:
			   - CRYS_KDF_ASN1_DerivMode - mode based on  ASN.1 DER encoding;
			   - CRYS_KDF_ConcatDerivMode - mode based on concatenation;
               - CRYS_KDF_X963_DerivMode = CRYS_KDF_ConcatDerivMode;
			   - CRYS_KDF_OMADRM_DerivMode - specific mode for OMA DRM.

 			The purpose of this function is to derive a keying data from the shared secret value and some 
			other optional shared information (SharedInfo).
 			
	The actual APIs that will be used by the user are:
		- CRYS_KDF_ASN1_KeyDerivFunc ;			
		- CRYS_KDF_ConcatKeyDerivFunc ;
        - CRYS_KDF_OMADRM_KeyDerivFunc .
 			
  \note The length in Bytes of the hash result buffer is denoted by "hashlen".
  \note All buffers arguments are represented in Big-Endian format.
 
  @param[in] ZZSecret_ptr 	 - A pointer to shared secret value octet string. 
  @param[in] ZZSecretSize  	 - The shared secret key Size, in bytes.
  @param[in] OtherInfo       - The pointer to structure, containing pointers and sizes of optional data shared by two  
                               entities intended to share the secret value. This argument (structure) and also its 
							   members are optional (if any is not need - set its pointer and size to NULL).
  @param[in] KDFhashMode	 - The hash function to be used. The hash function output must be at least 160 bits.
  @param[out] KeyingData_ptr - A pointer to the keying data derived from the secret key, of length KeyLenInBits
  @param[in] KeyLenInBytes	 - The size in bytes of the keying data to be generated. In our implementation - 
  							   KeyLenInBytes <= 2^32-1  (in standards KeyLengthInBits < hashlen*(2^32-1)).
  @param[in] derivation_mode - Specifies one of above described derivation modes.
  							  
  @return CRYSError_t - On success the value CRYS_OK is returned, 
			            and on failure an ERROR as defined in CRYS_KDF_error.h
*/

CIMPORT_C CRYSError_t  _DX_KDF_KeyDerivFunc( 
	                                  DxUint8_t                *ZZSecret_ptr,
									  DxUint32_t                ZZSecretSize,
									  CRYS_KDF_OtherInfo_t     *OtherInfo_ptr,
				  				      CRYS_KDF_HASH_OpMode_t    KDFhashMode,
				                      CRYS_KDF_DerivFuncMode_t  derivation_mode,
				                      DxUint8_t                *KeyingData_ptr,
				                      DxUint32_t                KeyLenInBytes )	;
				                    

/*********************************************************************************************************/
/**
 * @brief CRYS_KDF_ASN1_KeyDerivFunc performs key derivation according to ASN1 DER encoding method defined 
 		  in standard  ANSI X9.42-2001, 7.2.1.

		  This is the macro for calling the _DX_KeyDerivationFunc on said mode (arguments and errors description see in 
		  function definitions).
*/				                    
#define CRYS_KDF_ASN1_KeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,KDFhashMode,KeyingData_ptr,KeyLenInBytes)\
		_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(KDFhashMode),CRYS_KDF_ASN1_DerivMode,(KeyingData_ptr),(KeyLenInBytes))
				                    

/*********************************************************************************************************/
/**
 * @brief CRYS_KDF_ConcatKeyDerivFunc performs key derivation according to concatenation mode defined 
 		  in standard  ANSI X9.42-2001, 7.2.2.

		  This is the macro for calling the _DX_KeyDerivationFunc on said mode (arguments and errors description see in 
		  function definitions).
*/	
#define CRYS_KDF_ConcatKeyDerivFunc(ZZSecret_ptr,ZZSecretSize,OtherInfo_ptr,KDFhashMode,KeyingData_ptr,KeyLenInBytes)\
		_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),(OtherInfo_ptr),(KDFhashMode),CRYS_KDF_ConcatDerivMode,(KeyingData_ptr),(KeyLenInBytes))


/*********************************************************************************************************/
/**
 * @brief CRYS_KDF_OMADRM_KeyDerivFunc performs key derivation according to concatenation mode defined 
 		  in standard  ANSI X9.42-2001, 7.2.2 and specific requirements of OMA_TS_DRM_DRM_V2_0-20050712-C.

		  The specific requirements have respect to following arguments of called _DX_KeyDerivationFunc function:
		     - OtherInfo_ptr = NULL,
             - KDFhashMode = CRYS_KDF_HASH_SHA1_mode;
			 - derivation_mode = CRYS_KDF_ConcatDerivMode.

		  This is the macro for calling the _DX_KeyDerivationFunc on said mode (arguments and errors description see in 
		  function definitions).
*/	
#define CRYS_KDF_OMADRM_KeyDerivFunc(ZZSecret_ptr,ZZSecretSize,KeyingData_ptr,KeyLenInBytes)\
		_DX_KDF_KeyDerivFunc((ZZSecret_ptr),(ZZSecretSize),DX_NULL,CRYS_KDF_HASH_SHA1_mode,CRYS_KDF_ConcatDerivMode,(KeyingData_ptr),(KeyLenInBytes))



#ifdef __cplusplus
}
#endif

#endif

