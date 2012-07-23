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

#ifndef CRYS_AESGCM_H
#define CRYS_AESGCM_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AES.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_AESGCM.h    : %
   *  State           :  %state%
   *  Creation date   :  21 Marth. 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AESGCM APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AESGCM.h#1:incl:13
   *  \author Levin R.
   */

/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/

	/* The CRYS_AESGCM special definitions */
#define CRYS_AESGCM_ENGINE_TYPE  LLF_AESGCM_ENGINE_TYPE

#define CRYS_AESGCM_BLOCK_SIZE_IN_WORDS      4
#define CRYS_AESGCM_BLOCK_SIZE_IN_BYTES 	(CRYS_AESGCM_BLOCK_SIZE_IN_WORDS*sizeof(DxUint32_t))

#define CRYS_AESGCM_KEY_MAX_SIZE_BYTES          32
#define CRYS_AESGCM_KEY_MAX_SIZE_WORDS          (CRYS_AESGCM_KEY_MAX_SIZE_BYTES>>2)

/* IV (nonce) and AESGCM sizes definitions */
#define CRYS_AESGCM_IV_RECOMMENDED_SIZE_BYTES   12
#define CRYS_AESGCM_TAG_MIN_SIZE_BYTES       4
#define CRYS_AESGCM_TAG_MAX_SIZE_BYTES      16

/* AESGCM encrypt-decrypt mode */
#define CRYS_AESGCM_EncryptMode_t  CRYS_AES_EncryptMode_t
#define CRYS_AESGCM_Decrypt   CRYS_AES_Decrypt
#define CRYS_AESGCM_Encrypt   CRYS_AES_Encrypt


/************************ Enums ********************************/

/* Key size enum definition */
typedef  enum {
	 CRYS_AESGCM_Key128BitSize   = CRYS_AES_Key128BitSize,
	 CRYS_AESGCM_Key192BitSize   = CRYS_AES_Key192BitSize,
	 CRYS_AESGCM_Key256BitSize   = CRYS_AES_Key256BitSize,
	 CRYS_AESGCM_KeySizeNumOfOptions    
} CRYS_AESGCM_KeySize_t; 


/*	AESGCM context structure definition:  */
/*----------------------------------------*/
typedef struct 
{    
	DxUint32_t    H[CRYS_AESGCM_BLOCK_SIZE_IN_WORDS];   /*hash subkey*/
	DxUint32_t    Mac[CRYS_AESGCM_BLOCK_SIZE_IN_WORDS]; /*GMAC result*/
	CRYS_AESGCM_EncryptMode_t   EncryptMode;

	/* buffer for sizes of processed additional and text data in bits */
	DxUint64_t    DataSizes[2]; 
	
	/* Data specific to the platform we are working on 
	   (including the GHASH tables for SW) */
    #include "LLF_AESGCM_context_def.h" 	
	/* Dummy buffer - added for right AES encrypting of the context */
	DxUint8_t DummyBufAESBlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];

} AESGCMContext_t;

/* The user's AESGCM structure definition: */
typedef struct  CRYS_AESGCM_UserContext_t {
		DxUint32_t valid_tag;
		DxUint32_t AES_iv; 
		DxUint8_t  context_buff[sizeof(AESGCMContext_t)]; 
} CRYS_AESGCM_UserContext_t;

/************************ Typedefs  ****************************/

/* Defines the AESGCM key buffer */
typedef DxUint8_t CRYS_AESGCM_Key_t[CRYS_AESGCM_KEY_MAX_SIZE_BYTES];


/************************ Public Variables **********************/


/************************ Public Functions **********************/


/****************************************************************************************************/
/********                       AESGCM  FUNCTIONS                                              ******/
/****************************************************************************************************/

/****************************************************************************************************/
/** 
* @brief The CRYS_AESGCM_Init function initializes the AESGCM machine and SW structures 
*        for GHASH and GCTR (cipher) operations.
*
*        To perform the AESGCM operations this function should be called the first.
*        The function executes the following steps (algorithm according to NIST 800-38D):
*
*        1. Validates all of the inputs of the function. If one of the received 
*           parameters is not valid it shall return an error,
*        2. Allocates the working context by calling the CRYS_CCM_GetContext() function and 
*        3. Calculates GHASH subkey H by AES encryption of 0^128  bit string and saves it in the
*           the Context H – buffer. 
*        4. Calculates GCTR initial counter value J0 from input IV and saves it in the 
*           CTR buffer of the Context.
*        5. Initializes the context, sets not initialized fields to 0.
*        6. Encrypts (if requirred) the working context and stores it to the users context. 
*           Releases the working context by calling the CRYS_CCM_EncryptAndReleaseContext function.
*        7. Exits the handler with the OK code.
*
*        NOTE: 1.The GCM mode requieres uniqueness of Keys and IVs according to standard NIST 800-38D 
*                (including parts 8,9 and app. A,B).
*              2.Using the Tags with short sizes (32,64 bits)requires additional limitations on  
*                maximal Combined lengts of Ciphertext and AddData and on maximum Invocations of 
*                the Authenticated Decryption function (NIST 800-38D app.C).
*              3.Management of both above stated issues is out of scope of the CRYS_GCM module and   
*                is on responsibility of the Application, which uses this module.
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer, that is allocated by the user 
*                            and is used for the AESGCM operations.
* @param[in] encrMode      - Enumerator variable, defining operation mode (0 - encrypt; 1 - decrypt).
* @param[in] key           - A pointer, to the AESGCM key passed by user.
* @param[in] keySizeId     - An ID of AESGCM key size (according to 128, 192, or 256 bits size).
* @param[in] IV_ptr	       - A pointer to Initial Value (nonce) - unique value assigned to all data passed 
*                            into GCM. 
* @param[in] ivSize        - The size of the user passed IV (in bytes). Our implementation now allows only the
*                            value, recommended by standard - 12 bytes. 
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to CRYS_AESGCM_error.h
*/
CIMPORT_C CRYSError_t CRYS_AESGCM_Init(
									   CRYS_AESGCM_UserContext_t *ContextID_ptr, /*GCM user context*/
									   CRYS_AESGCM_EncryptMode_t  encrMode,      /*encrypt-decrypt mode*/ 
									   CRYS_AESGCM_Key_t          key,           /*AESGCM key*/
									   CRYS_AESGCM_KeySize_t      keySizeID,     /*key size ID*/ 
									   DxUint8_t                 *iv_ptr,        /*initial value*/
									   DxUint32_t                 ivSize );      /*size of IV in bytes*/


/****************************************************************************************************/
/** 
* @brief This function calculates (updates) the AESGCM MAC on current block of additional data.
*
*       This function is called for each block of additional data, if it present, and 
*       executes the following major steps (algorithm according to NIST 800-38D):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.
*          In partial, checks that previous processed block of data not was not multiple of 
*          16 bytes, otherwise returns an error. 
*          If DataIn_ptr = NULL or DataInSize = 0, the function returns an error.
*        2.Decrypts the received context to the working context after capturing the 
*          working context by calling the CRYS_CCM_GetContext function.
*        3.Checks that the data size not exceeds the maximal allowed value and updates the
*          total size (AddDataSize field) according to received input size.
*        4.Calculates GHASH value for all full 16-bytes blocks of the additional data. 
*        5.Pads the last not full AES-block by 0-s, calculates GHASH and saves it 
*          in the context buffer.       
*        6.Encrypts the working context and stores it to the users received context. 
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        7.Exits the handler with the OK code.  
*
*      Note: The padding and GHASH calculation are performed on LLF level by calling  
*            appropriate LLF functions.
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer allocated by the user that
*                            is used for the AESGCM machine operation. This should be the same 
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input additional data. It is required:
*                         DataIn_ptr != NULL. The pointer does not need to be aligned. 
*                         On CSI input mode the pointer must be equal to value 
*                         (0xFFFFFFFC | DataInAlignment). 
* @param[in] DataInSize   - A size of the additional data in bytes. Required: DataInSize > 0.
*                           Size of each intermediate block of data must be multiple of 16 bytes, 
*                           excluding the last block which may have any size. The total size 
*                           of the Additional data in all callings of the function must be less, 
*                           than 2^64-1 bytes.
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to
*                       CRYS_AESGCM_error.h
*   Notes:   
*      1. Temporary the CSI input is not allowed on AESGCM mode.
*
*/
CIMPORT_C CRYSError_t  CRYS_AESGCM_BlockAdata( 
											CRYS_AESGCM_UserContext_t    *ContextID_ptr,   
											DxUint8_t                    *DataIn_ptr,
											DxUint32_t                    DataInSize );


/****************************************************************************************************/
/** 
* @brief This function performs authenticated encryption/decryption  of current block of text data.
*
*       This function is called for each block of text data with size multiple 16 bytes,
*       excluding the last block, which may have any size > 0. 
*       
*       The function executes the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error,  
*          in partial, checks that previously processed additional and text data not have 
*          remaining bytes (not multiple 16). Checks the size of the user passed output buffer, 
*          which must be not less, than input data size.
*        2.Decrypts the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.Calculates the total bit length of the text data to process (including the previous 
*          processed data), checks that the length is less than (2^39-256) bits and saves it in 
*          the context. 
*      On encrypt mode 
*          -	if the data size is not multiple of 16 bytes, then the function pads the last 
*               data by 0-s to full 16-bytes block;
*          -	encrypts the padded plain text to cypher text and calculates the GHASH value for 
*               the cypher, outputs the cypher text and saves the GHASH in the context IV buffer. 
*      On decrypt mode 
*          -	calculates GHASH of cipher text (with padding by 0-s, if necessary) and then decrypts 
*               it (these operations are similar to described for encryption).
*        4.Encrypts the working context and stores it to the users received context. 
*          Releases the working context by calling the CRYS_CCM_ReleaseContext function.
*        5.Exits. 
*
*      Note: The padding, cipher operations and GHASH calculation are performed on LLF level 
*            by calling appropriate LLF functions.
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer allocated by the user that
*                            is used for the AESGCM operations. This should be the same 
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input data (plain or cypher text). 
*                         The pointer does not need to be aligned. On CSI input mode the 
*                         pointer must be equal to value (0xFFFFFFFC | DataInAlignment). 
* @param[in] DataInSize  - A size of the data block in bytes: must be not null and multiple of  
*                          16 bytes. The total size of processed data must be less than 
*                          2^36-32 bytes.
* @param[in] DataOut_ptr - A pointer to the output buffer (cypher or plain text according to  
*                          encryption mode ).The pointer does not need to be aligned. On CSI input 
*                          mode the pointer must be equal to value (0xFFFFFFFC | DataOutAlignment).
*                          Size of the output buffer must be not less, than DataInSize. 
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_AESGCM_error.h
*   Notes:   
*      1. Temporary the CSI input or output are not allowed on AESGCM mode.
*      2. Overlapping of the in-out buffers is not allowed, excluding the in placement case:
*         DataIn_ptr = DataOut_ptr.
*
*/
CIMPORT_C CRYSError_t  CRYS_AESGCM_BlockTextData(
								CRYS_AESGCM_UserContext_t    *ContextID_ptr,   
								DxUint8_t                    *DataIn_ptr,
								DxUint32_t                    DataInSize,
								DxUint8_t                    *DataOut_ptr );


/****************************************************************************************************/
/** 
* @brief This function finishes the AESGCM operation.
*
*       This function must be called after processing all additional and text data by appropriate
*       AESGCM_Block functions. 
*       The function performs the following major steps (algorithm according to NIST 800-38D):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*          
*        2.Decrypts the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.Encrypts/Decrypts by AESCTR the text data with padded last block and calculates GHASH value.
*        4.Concatenates saved in the Context two 64-bit values of AddDataSize and  
*          TextDataSize, converts endianness MSBytes order and calculates final GHASH value.
*        5.Encrypts the final GHASH by GCTR using the initial value of J0 and derives from it 
*          the GMAC Tag of required size.
*        6.On encrypt mode: puts the Tag out. 
*   	 7.On decrypt mode: compares the calculated tag value to received input tag’ value. 
*          If tag == tag’ and no any errors occurs, then sets returning Error value = OK, else = FAIL.
*          Note: GHASH calculation is performed on LLF level by calling the LLF_AESGCM_Finish function.
*        8.Cleans and releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        9.Exits the handler with the return Error code. 
*
*         NOTE:
*            1.Using the Tags with short sizes (32,64 bits)requires additional limitations on  
*              maximal Combined lengts of Ciphertext and AddData and on maximum Invocations of 
*              the Authenticated Decryption function (NIST 800-38D app.C).
*              Management of this issue is out of scope of the CRYS GCM module and is on  
*              responsibility of the Application ,which uses this module.
*
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer, allocated by the user. 
*                            This should be the same context that was used on the previous call 
*                            of this session.
* @param[in] TextDataIn_ptr - A pointer to the buffer of the input data (plain or cypher text). 
*                             The pointer does not need to be aligned. On CSI input mode the 
*                             pointer must be equal to value (0xFFFFFFFC | DataInAlignment). 
* @param[in] TextDataSize  - A size of the data block in bytes: must be not null. The total  
*                            size of processed data must be less than 2^36-32 bytes.
* @param[in/out] tag_ptr   - A pointer to the buffer of the AESGCM Tag. The pointer does not need 
*                            to be aligned. 
* @param[in] tagSize       - The size of the Tag in bytes. The allowed values are: 12...16 bytes,
*                            For certain applications the size may be also 4 or 8 bytes with additional
*                            limitations on total data size (see NIST 800-38D).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a 
*                       value MODULE_* CRYS_AESGCM_error.h
*
*/
CIMPORT_C CRYSError_t CRYS_AESGCM_Finish( 
							   CRYS_AESGCM_UserContext_t    *ContextID_ptr, 
							   DxUint8_t                    *TextDataIn_ptr,
							   DxUint32_t                    TextDataSize,
							   DxUint8_t                    *TextDataOut_ptr,
							   DxUint8_t                    *tag_ptr,       
							   DxUint32_t                    tagSize );     


/****************************************************************************************************/
/**
* @brief This function is used to perform the AESGCM operation in one integrated process. 
*
*        The function preforms CCM algorithm according to NIST 800-38D by call the AESGCM
*        Init, Block and Finish functions. 
*
*    NOTE: 
*         1.The GCM mode strongly requieres uniqueness of Keys and IVs according to standard 
*           NIST 800-38D (including parts 8,9 and app. A,B).
*         2.Using the Tags with short sizes (32,64 bits) requires additional limitations on  
*           maximal Combined lengts of Ciphertext and AddData and on maximum Invocations of 
*           the Authenticated Decryption function (NIST 800-38D including app.C).
*         3.Management of above stated issues is out of scope of the CRYS_GCM module and   
*           is on responsibility of the Application, which uses this module.

*
* @param[in] encrMode      - Enumerator variable, defining operation mode (0 - encrypt; 1 - decrypt).
* @param[in] key           - A pointer, to the AESGCM key passed by user.
* @param[in] keySizeId     - An ID of AESGCM key size (according to 128, 192, or 256 bits size).
* @param[in] IV_ptr	       - A pointer to Initial Value (nonce) - unique value assigned to all data passed 
*                            into GCM. 
* @param[in] ivSize        - The size of the user passed IV (in bytes). Our implementation now allows only the
*                            value, recommended by standard - 12 bytes. 
* @param[in] AddDataIn_ptr  - A pointer to the additional data buffer. The pointer does 
*                             not need to be aligned. On CSI input mode the pointer must be equal to 
*                             value (0xFFFFFFFC | DataInAlignment). 
* @param[in] AddDataInSize  - The size of the additional data in bytes;  
* @param[in] TextDataIn_ptr - A pointer to the input text data buffer (plain or cypher according to
*                             encrypt-decrypt mode). The pointer does not need to be aligned.
*                             On CSI input mode the pointer must be equal to value (0xFFFFFFFC | DataInAlignment). 
* @param[in] TextDataInSize - A size of the data block in bytes: must be not null. 
* @param[out] TextDataOut_ptr - The output text data pointer (cypher or plain text according to encrypt/decrypt mode). 
* @param[in/out] tag_ptr    - A pointer to the buffer of the AESGCM tag. The pointer does not need 
*                             to be aligned. 
* @param[in] tagSize        - The size of the tag in bytes. The allowed values are: 12...16 bytes. For certain 
*                             applications the size may be 4 or 8 bytes (see NIST 800-38D).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in CRYS_AESGCM_error.h
*
*     NOTES: 
*              1. Temporarily it is not allowed, that both the Input and the Output data simultaneously 
*               were on CSI mode.
*
*/
CIMPORT_C   CRYSError_t CRYS_AESGCM ( 	
									 CRYS_AESGCM_EncryptMode_t  encrMode,      /*encrypt-decrypt mode*/ 
									 CRYS_AESGCM_Key_t          key,           /*AESGCM key*/
									 CRYS_AESGCM_KeySize_t      keySizeId,     /*key size ID*/ 
									 DxUint8_t                 *iv_ptr,        /*initial value*/
									 DxUint32_t                 ivSize,        /*size of IV in bytes*/
									 DxUint8_t                 *AddDataIn_ptr, /*Input data*/
									 DxUint32_t                 AddDataSize,   /*Data size, bytes*/
									 DxUint8_t                 *TextDataIn_ptr,/*Input data*/
									 DxUint32_t                 TextDataSize,  /*Data size, bytes*/
									 DxUint8_t                 *TextDataOut_ptr,/*Output data*/   
									 DxUint8_t                 *tag_ptr,       /*Input/output of tag*/
									 DxUint32_t                 tagSize );     /*Tag size, bytes*/        



#ifdef __cplusplus
}
#endif

#endif /*#ifndef CRYS_AESGCM_H*/

