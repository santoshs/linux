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

#ifndef CRYS_AESCCM_H
#define CRYS_AESCCM_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AES.h"
#include "LLF_AESCCM_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_AESCCM.h    : %
   *  State           :  %state%
   *  Creation date   :  24 Sept. 2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AESCCM APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AESCCM.h#1:incl:13
   *  \author Levin R.
   */

/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/

	/* The CRYS_AES_CCM special definitions */
#define CRYS_AESCCM_ENGINE_TYPE  LLF_AESCCM_ENGINE_TYPE

/* key and key buffer sizes definitions */
#define CRYS_AESCCM_KeySize_t  CRYS_AES_KeySize_t     
#define CRYS_AESCCM_KEY_SIZE_WORDS           8

/* nonce and AESCCM-MAC sizes definitions */
#define CRYS_AESCCM_NONCE_MIN_SIZE_BYTES     7
#define CRYS_AESCCM_NONCE_MAX_SIZE_BYTES    13
#define CRYS_AESCCM_MAC_MIN_SIZE_BYTES       4
#define CRYS_AESCCM_MAC_MAX_SIZE_BYTES      16

/* AES CCM encrypt-decrypt mode */
#define CRYS_AESCCM_Decrypt   CRYS_AES_Decrypt
#define CRYS_AESCCM_Encrypt   CRYS_AES_Encrypt

/************************ Enums ********************************/

/* AES_CCM data type enumerator */
typedef  enum {
	CRYS_AESCCM_FirstAdata       = 0,
	CRYS_AESCCM_IntermedAdata    = 1,
	CRYS_AESCCM_LastAdata        = 2,
	CRYS_AESCCM_FirstTextData    = 3,
	CRYS_AESCCM_IntermedTextData = 4,
	CRYS_AESCCM_LastTextData     = 5,

	CRYS_AESCCM_DataTypeOffModes,
	CRYS_AESCCM_DataTypeModeLast = 0x7FFFFFFF,

}CRYS_AESCCM_DataType_t;     



/************************ Typedefs  ****************************/

/* Defines the AES_CCM key buffer */
typedef DxUint8_t CRYS_AESCCM_Key_t[CRYS_AESCCM_KEY_SIZE_WORDS * sizeof(DxUint32_t)];

typedef DxUint8_t CRYS_AESCCM_Mac_Res_t[CRYS_AES_BLOCK_SIZE_IN_BYTES];

/******************* Context Structure  ***********************/

/* The context data-base used by the AES functions on the low level */
typedef struct  
{ 
	/*  Context buffer used for AES operations on separate special blocks of CCM data */
	AESContext_t   AESContext;
	/* Specific buffers for CCM mode */
	DxUint32_t    CCM_IV[CRYS_AES_BLOCK_SIZE_IN_WORDS];
	DxUint32_t    CCM_CTR[CRYS_AES_BLOCK_SIZE_IN_WORDS];
	DxUint32_t    temp[CRYS_AES_BLOCK_SIZE_IN_WORDS];
   
    /*    not encrypted part of AESCCM context */
   
    /* last block value (partly may be encrypted) */
    DxUint32_t  lastBlock[CRYS_AES_BLOCK_SIZE_IN_WORDS];
    /* last block size */
    DxUint32_t  lastBlockSize;   

	DxUint32_t    RemainAdataSize;
	DxUint32_t    RemainTextSize;
	CRYS_AES_EncryptMode_t   CCM_EncryptMode;
	CRYS_AESCCM_DataType_t   CurrentDataType;
	/* Count of bytes in control-data fields, assigned for sizes of Adata, 
	   PlainTextData, Nonce and CCM-MAC T values */ 
	DxUint8_t     SizeOfA;
	DxUint8_t     QFieldSize;
	DxUint8_t     SizeOfN;
	DxUint8_t     SizeOfT;

	/* Data specific to the platform we are working on – include from LLF */
    #include "LLF_AESCCM_context_def.h"
     
    /* Dummy buffer - added for right AES encrypting of the context data.
       Note: This block must be always at the end of the context.  */
    DxUint8_t DummyBufAESBlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];
 
}AESCCM_Context_t; 


/* The user's context structure - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_AESCCM_UserContext_t 
{
   DxUint32_t valid_tag;
   DxUint32_t AES_iv;/* for use of the AES CBC mode of Encryption and Decryption of the context in CCM */ 
   DxUint8_t  context_buff[ sizeof(AESCCM_Context_t) ]; 

}CRYS_AESCCM_UserContext_t;

/************************ Public Variables **********************/


/************************ Public Functions **********************/


/****************************************************************************************************/
/********                       AESCCM  FUNCTIONS                                              ******/
/****************************************************************************************************/

/****************************************************************************************************/
/** 
* @brief This function initializes the AESCCM machine or SW structures and calculates 
*        AES-MAC of control block B0 of payload.
*
*        To perform the AESCCM operations this function should be called the first.
*        The function executes the following steps (algorithm according to NIST 800-38C):
*
*        1. Validates all of the inputs of the function. If one of the received 
*           parameters is not valid it shall return an error:
*        2. Allocates the working context by calling the CRYS_CCM_GetContext() function.
*        3. Initializes the context and performs formatting of the input additional data:  
*           block B0, containing control information, and 'a' field of block B1. This field
*           are saved in the lastBlock buffer of the context.
*        4. Calculates AES-MAC of block B0 and saves it in the AESCCM_Context CCM_IV.
*        5. Sets sizes of formatted additional (without padding) and text data in the 
*           RemainAsize and RemainTextSize fields of CCM context.
*        6. Encrypts the working context and stores it to the users context. 
*           Releases the working context by calling the CRYS_CCM_EncryptAndReleaseContext function.
*        7. Exits the handler with the OK code.
*
*        Note: Parameters of AESCCM algorithm: CCM_Key, Nonce, QFieldSize, SizeOfN, SizeOfT, AdataSize,
*              TextSizeQ must be the same for both CCM-encrypt and CCM-decrypt operations on the 
*              same given data.
*         
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer, that is allocated by the user 
*                            and is used for the AESCCM operations.
* @param[in] EncrDecrMode  - Enumerator variable defining operation mode (0 - encrypt; 1 - decrypt).
* @param[in] CCM_Key       - A buffer, containing the AESCCM key passed by user (predefined size 128 bits).
* @param[in] KeySizeId     - An ID of AESCCM key size (according to 128, 192, or 256 bits size).
* @param[in] isSecretKey   - This parameter intended for internal Discretix using only !. 
*                            The parameter specifying whether to use a secret key (=TRUE) 
*                            or to use a key defined by the user (=FALSE). 
*                            The secret key function depends on product configuration. 
*                            Please consult the reference manual for AES functions.
* @param[in] AdataSize     - Full size of additional data in bytes, which must be processed.  
*                            Limitation in our implementation is: AdataSize < 2^32. If Adata is absent, 
*                            then AdataSize = 0.
* @param[in] TextSizeQ     - The full size of text data (in bytes), which must be processed by CCM. 

* @param[in] QFieldSize    - Byte-size of formatted field for writing significant bytes of the TextSizeQ 
*                            value. Valid values: [2,3,4,5,6,7,8]. 
* @param[in] N_ptr	       - A pointer to Nonce - unique value assigned to all data passed into CCM.
*                            Bytes order - big endian form (MSB is the first).
* @param[in] SizeOfN       - The size of the user passed Nonce (in bytes). 	Valid values: 
*                            7 <= SizeOfN <= (15-QFieldSize). 
* @param[in] SizeOfT	   - Size of AESCCM MAC output T in bytes. Valid values: [4,6,8,10,12,14,16].   
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to CRYS_AESCCM_error.h
*/
CIMPORT_C CRYSError_t  CRYS_AESCCM_Init( 
										CRYS_AESCCM_UserContext_t   *ContextID_ptr,
										CRYS_AES_EncryptMode_t       EncrDecrMode,
										CRYS_AESCCM_Key_t            CCM_Key,    /*AESCCM key*/ 
										CRYS_AESCCM_KeySize_t        KeySizeId,  /* key size ID*/ 
										DxUint8_t                    isSecretKey, /*secret key flag*/
										DxUint32_t                   AdataSize,  /*full size of additional data*/
										DxUint32_t                   TextSizeQ,  /*full size of text data*/
										DxUint8_t                    QFieldSize, /*size of Q field in control block*/
										DxUint8_t                   *N_ptr,      /*Nonce*/
										DxUint8_t                    SizeOfN,    /*size of N buffer*/
										DxUint8_t                    SizeOfT );  /*size of CCM-MAC (T) */



/****************************************************************************************************/
/** 
* @brief This function calculates (updates) AESCCM MAC on current block of additional data.
*
*       This function is called for each block of additional data, if it present, and 
*       executes the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*          
*        2.Decrypts the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.If AdataBlock is not last and herewith DataInSize%16 != 0, then the 
*        4.If DataIn_ptr = NULL or DataInSize = 0, the function returns an Error.
*          function returns an error, else the function concatenates the data, saved 
*          in the lastBlock of the context with some bytes of input data into full AES-block  
*          and calculates (updates)AES-MAC value by invoking LLF_AESCCM_BlockAdata function. 
*        4.Updates RemainAsize field according to received input data size.
*        5.Encrypts the working context and stores it to the users received context. 
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        6.Exits the handler with the OK code.  
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer allocated by the user that
*                            is used for the AESCCM machine operation. This should be the same 
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input additional data. It is required
*                         DataIn_ptr != NULL. The pointer does not need to be aligned. 
*                         On CSI input mode the pointer must be equal to value 
*                         (0xFFFFFFFC | DataInAlignment). 
* @param[in] DataInSize   - A size of the additional data in bytes. Required: DataInSize > 0.
*                           Size of each intermediate block of data must be multiple of 16 bytes, 
*                           excluding the last block which may have any size.
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to
*                       CRYS_AESCCM_error.h
*   Notes:   
*      1. Temporary the CSI input is not allowed on CCM mode.
*
*/
CIMPORT_C CRYSError_t  CRYS_AESCCM_BlockAdata( 
											CRYS_AESCCM_UserContext_t    *ContextID_ptr,   
											DxUint8_t                    *DataIn_ptr,
											DxUint32_t                    DataInSize );

/****************************************************************************************************/
/** 
* @brief This function encrypts (decrypts) the current block of text data and calculates 
*        CCM_MAC value.
*
*       This function is called for each block of text data with size multiple 16 bytes,
*       excluding the last block, which must be processed by CRYS_AESCCM_BlockLastTextData
*       function. 
*       
*       The function executes the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*          
*        2.Decrypts the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.If DataInSize = 0 or DataInSize%16 != 0, then the function returns an error. 
*        4.Encrypts (decrypts)the input data and calculates AESCCM_MAC by invoking 
*          LLF_AESCCM_BlockTextData function. 
*        5.Updates RemainTextSize field of the context according to received input data size.
*        6.Encrypts the working context and stores it to the users received context. 
*          Releases the working context by calling the CRYS_CCM_ReleaseContext function.
*        7.Exits. 
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer allocated by the user that
*                            is used for the AES machine operation. This should be the same 
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input data (plain or cipher text). 
*                         The pointer does not need to be aligned. On CSI input mode the 
*                         pointer must be equal to value (0xFFFFFFFC | DataInAlignment). 
* @param[in] DataInSize  - A size of the data block in bytes: must be multiple of 16 bytes and not Null.
*                          The block of data must be not a last block, that means:
*                            - on Encrypt mode: DataInSize < CCM_Context->RemainTextSize;
*                            - on Decrypt mode: DataInSize < CCM_Context->RemainTextSize - SizeOfT;
* @param[in] DataOut_ptr - A pointer to the output buffer (cipher or plain text). 
*                          The pointer does not need to be aligned. On CSI input mode the 
*                          pointer must be equal to value (0xFFFFFFFC | DataOutAlignment).
*                          Size of the output buffer must be not less, than DataInSize. 
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_AESCCM_error.h
*   Notes:   
*      1. Temporary the CSI input or output are not allowed on CCM mode.
*      2. Overlapping of the in-out buffers is not allowed, excluding the in placement case:
*         DataIn_ptr = DataOut_ptr.
*
*/
CIMPORT_C CRYSError_t  CRYS_AESCCM_BlockTextData(
											CRYS_AESCCM_UserContext_t    *ContextID_ptr,   
											DxUint8_t                    *DataIn_ptr,
											DxUint32_t                    DataInSize,
											DxUint8_t                    *DataOut_ptr );


/****************************************************************************************************/
/** 
* @brief This function encrypts (decrypts) the last block of text data and calculates CCM_MAC value.
*
*       This function must be called for the last block of text data and executes 
*       the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*          
*        2.Decrypts the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.Encrypts (decrypts)the input data by AES-CTR,  calculates and perform verification of AES-MAC value 
*          by invoking LLF_AESCCM_Finish function.
*        4.Encrypts the working context and stores it to the users received context. 
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        5.Exits the handler with the OK code. 
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer, allocated by the user, 
*                          that is used for the AESCCM operations. This should be the same 
*                          context that was used on the previous call of this session.
* @param[in] DataIn_ptr  - A pointer to the buffer of the input data (plain or cipher text). 
*                          The pointer does not need to be aligned. On CSI input mode the 
*                          pointer must be equal to value (0xFFFFFFFC | DataInAlignment). 
* @param[in] DataInSize  - A size of the data block in bytes. The size must be equal to remaining
*                          size value, saved in the context.
* @param[in] DataOut_ptr - A pointer to the output buffer (cipher or plain text). If 
*                          user passes DataInSize 0 bytes the DataOut_ptr may be equal to NULL. 
*                          The pointer does not need to be aligned. On CSI input mode the 
*                          pointer must be equal to value (0xFFFFFFFC | DataOutAlignment).
* @param[in] MacRes -   A pointer to the Mac buffer. 
* @param[out] SizeOfT - size of MAC in bytes as defined in CRYS_AESCCM_Init function.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a 
*                       value MODULE_* CRYS_AESCCM_error.h
*   Notes:   
*      1. Temporary the CSI input or output are not allowed on CCM mode.
*      2. Overlapping of the in-out buffers is not allowed, excluding the in placement case:
*         DataIn_ptr = DataOut_ptr.
*   
*
*/
CIMPORT_C CRYSError_t  CRYS_AESCCM_Finish(CRYS_AESCCM_UserContext_t    *ContextID_ptr,   
										  DxUint8_t                    *DataIn_ptr,
										  DxUint32_t                    DataInSize,
										  DxUint8_t                    *DataOut_ptr,
										  CRYS_AESCCM_Mac_Res_t         MacRes,
										  DxUint8_t                    *SizeOfT);



/****************************************************************************************************/
/**
 * @brief This function is used to perform the AES_CCM operation in one integrated process. 
 *
 *        The function preforms CCM algorithm according to NIST 800-38C by call the CRYS_CCM
 *        Init, Block and Finish functions. 
 *
 *        The input-output parameters of the function are the following:
 *
 * @param[in] EncrDecrMode  - Enumerator variable defining operation mode (0 - encrypt; 1 - decrypt).
 * @param[in] CCM_Key       - A buffer, containing the AESCCM key passed by user (predefined size 128 bits).
 * @param[in] KeySizeId     - An ID of AESCCM key size (according to 128, 192, or 256 bits size).
 * @param[in] isSecretKey   - This parameter intended for internal Discretix using only !. 
 *                            The parameter specifying whether to use a secret key (=TRUE) 
 *                            or to use a key defined by the user (=FALSE). 
 *                            The secret key function depends on product configuration. 
 *                            Please consult the reference manual for AES functions.
* @param[in] QFieldSize    - Byte-size of formatted field for writing significant bytes of the TextSizeQ 
 *                            value. Valid values: [2,3,4,5,6,7,8]. 
 * @param[in] N_ptr	        - A pointer to Nonce - unique value assigned to all data passed into CCM.
 *                            Bytes order - big endian form (MSB is the first).
 * @param[in] SizeOfN       - The size of the user passed Nonce (in bytes). 	Valid values: 
 *                            7 <= SizeOfN <= (15-QFieldSize). 
 * @param[in] ADataIn_ptr    - A pointer to the additional data buffer. The pointer does 
 *                             not need to be aligned. On CSI input mode the pointer must be equal to 
 *                             value (0xFFFFFFFC | DataInAlignment). 
 * @param[in] ADataInSize    - The size of the additional data in bytes;  
 * @param[in] TextDataIn_ptr - A pointer to the input text data buffer (plain or cipher according to
 *                             encrypt-decrypt mode). The pointer does not need to be aligned.
 *                             On CSI input mode the pointer must be equal to value (0xFFFFFFFC | DataInAlignment). 
 * @param[in] TextDataInSize - The size of the input text data in bytes:
 *                       
 * @param[out] TextDataOut_ptr - The output text data pointer (cipher or plain text data). 
 *
 * @param[in] SizeOfT	    - Size of AES-CCM MAC of T in bytes. Valid values: [4,6,8,10,12,14,16].  
* 
 * @param[in/out] Mac_Res	    -  AES-CCM MAC input/output .  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in CRYS_AESCCM_error.h
 *
 *     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously 
 *               were on CSI mode.
 *
 */
CIMPORT_C CRYSError_t  CRYS_AESCCM( 
								   CRYS_AES_EncryptMode_t       EncrDecrMode,   /*CCM encrypt-decrypt mode*/
								   CRYS_AESCCM_Key_t            CCM_Key,        /*AES-CCM key*/            
								   CRYS_AESCCM_KeySize_t        KeySizeId,      /*Key size ID*/ 
								   DxUint8_t                    isSecretKey,    /*secret key flag*/
								   DxUint8_t                    QFieldSize,     /*size of Q field in control block*/
								   DxUint8_t                   *N_ptr,          /*Nonce*/
								   DxUint8_t                    SizeOfN,        /*size of N buffer*/
								   DxUint8_t                   *ADataIn_ptr,    /*input data pointer*/
								   DxUint32_t                   ADataInSize,    /*input data size*/
								   DxUint8_t                   *TextDataIn_ptr, /*input data pointer*/
								   DxUint32_t                   TextDataInSize, /*input data size*/
								   DxUint8_t                   *TextDataOut_ptr,/*output data pointer*/
								   DxUint8_t                    SizeOfT,         /*size of CCM-MAC (T) */
								   CRYS_AESCCM_Mac_Res_t        Mac_Res);     


#ifdef __cplusplus
}
#endif

#endif /*#ifndef CRYS_AESCCM_H*/

