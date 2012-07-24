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
 
 
#ifndef CRYS_C2_H_1
#define CRYS_C2_H_1

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_C2_error.h"

#include "LLF_C2_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_C2.h    : %
   *  State           :  %state%
   *  Creation date   :  Feb 19 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS C2 APIs, as well as the APIs themselves.
   *
   *  \version CRYS_C2.h#1:incl:4
   *  \author R.Levin
   */


/************************ Defines ******************************/
/* this define is need if data is ordered according to big endian order inside of each 4 bytes */
//#define CRYS_C2_BIG_ENDIAN_DATA

/* definition describing the low level Engine type ( SW , Hardware , Etc ) */
#define CRYS_C2_ENGINE_TYPE  LLF_C2_ENGINE_TYPE

/* The key size in bytes and in words */
#define CRYS_C2_KEY_SIZE_IN_BYTES 7
#define CRYS_C2_KEY_SIZE_IN_WORDS 2

/* The C2 block size in bytes and in words */
#define CRYS_C2_BLOCK_SIZE_IN_BYTES 8
#define CRYS_C2_BLOCK_SIZE_IN_WORDS 2

/* C2 HASH function result type definition */
typedef DxUint32_t CRYS_C2HASH_Result_t[CRYS_C2_BLOCK_SIZE_IN_WORDS];


/* The C2 cipher has 10 rounds:*/
#define CRYS_C2_MAX_ROUND  10
#define CRYS_C2_TRUNCATED_ROUND 2

/* defining the minimum number of blocks that will activate a DMA operation.
   less from this number the AES will work on the direct mode , since it is not
   efficient on real time */  
#define CRYS_C2_SW_HW_SEC_CONST_SIZE 256

/************************ Enums ********************************/

/* Enum definitions for the Encrypt or Decrypt operation mode */
typedef enum 
{
   CRYS_C2_Encrypt = 0,
   CRYS_C2_Decrypt = 1,
   
   CRYS_C2_EncryptNumOfOptions,

   CRYS_C2_EncryptModeLast= 0x7FFFFFFF,

}CRYS_C2_EncryptMode_t;

/* Enum definitions for the C2 operation mode */
typedef enum
{
   CRYS_C2_ECB_mode    = 0,
   CRYS_C2_CBC_mode    = 1,

   CRYS_C2_NumOfModes,

   CRYS_C2_OperationModeLast= 0x7FFFFFFF,

}CRYS_C2_OperationMode_t;      
   
/************************ Typedefs  ****************************/

/* Defines the C2 key type */
typedef DxUint8_t CRYS_C2_Key_t[CRYS_C2_KEY_SIZE_IN_BYTES];

/* Defines the KEY buffer in 32 bits words */
typedef DxUint32_t CRYS_C2_Key_32bit_t[CRYS_C2_KEY_SIZE_IN_WORDS];

/* Defines the C2 block data buffer in bytes */
typedef DxUint8_t  CRYS_C2_BLOCK_BYTES_t[CRYS_C2_BLOCK_SIZE_IN_BYTES];


/************************ Structs  ******************************/

/*--------------- C2 Cipher structures definitions -------------*/
/* The context data base used by the C2 Cipher functions on the low level */
typedef struct  
{ 
   
   /* Include the context that is specific for the platform we are working on */ 
   #include "LLF_C2_Cipher_context_def.h"
  
   /* This Buffer is defined so that the context will pass a size that is 0 modulo 16.
    * Note: This block must be at the end of the context.
    */
   DxUint8_t DummyBufC2BlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];

   
}C2CipherContext_t; 


/* The user's context prototype - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_C2CipherUserContext_t 
{
   DxUint32_t valid_tag;
   DxUint32_t AES_iv;    /* For use of the AES Encryption and Decryption of the context in CCM*/            
   DxUint8_t  context_buff[ sizeof(C2CipherContext_t) ]; 

}CRYS_C2CipherUserContext_t;


/* -------------- C2 HASH structures definitions --------------------*/
#ifndef LLF_C2_HW_ENGINE_TYPE
/* The context data base used by the C2 HASH functions on the low level */
typedef struct 
{ 
   CRYS_C2HASH_Result_t        HASH_Result;
      
   /* A block buffer used for all cases where the update data size 
      is not aligned to a block size - we cannot load the block to the hardware 
      therefore the first block is always loaded from this buffer 
    */
   DxUint32_t DataFromThePreviousUpdateBuff[CRYS_C2_BLOCK_SIZE_IN_WORDS]; 
   
   /* The number of bytes in the previous update */
   DxUint16_t NumOfBytesOnThePreviousUpdateBuff;   
   
   /* Including the context that is specific to the platform we are working on */
   #include "LLF_C2_HASH_context_def.h"
   
   /* This Buffer is defined so that the context will pass a size that is 0 modulo 16.
    * This block must be at the end of the context.*/  
   DxUint8_t DummyBufC2BlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];
      
}C2HASHContext_t; 



/* The user's context prototype - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_C2HASHUserContext_t 
{
   DxUint32_t valid_tag; 
   DxUint32_t AES_iv;/* For use of the AES CBC mode of Encryption and Decryption of the context in CCM*/
   DxUint8_t  context_buff[ sizeof(C2HASHContext_t) ]; 

}CRYS_C2HASHUserContext_t;

#endif

/************************ Public Functions **********************/

/*****************************************************************************************************/ 
/*****************************************************************************************************/ 
/* 
 * @brief This function is used to initialize the C2 Cipher secret constant
 *
 *        The function executes the following major steps:
 *
 *        1. Validating all of the input parameters. If of them is not valid it shall return an error 
 *           parameters is not valid it shall return an error:
 *          
 *        2. Call to LLF_C2_SBoxInitto load SBox
 *
 * @param[in] aSecretConstant - A pointer to the Secret Constant that
 *                            is used for the C2 Cipher operations.
 *
 * @param[in] aConstantSize -  secret constant size.
 *       
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_C2_error.h
 */
 
CIMPORT_C CRYSError_t  CRYS_C2_SBoxInit(const DxUint8_t *aSecretConstant,
                                        DxUint32_t aConstantSize);


/*****************************************************************************************************/ 
/* 
 * @brief This function is used to initialize the C2 Cipher structures and machine registers .
 *        In order to perform the C2 Cipher this function should be called first.
 *
 *        The function executes the following major steps:
 *
 *        1. Validating all of the inputs of the function. If one of the received 
 *           parameters is not valid it shall return an error:
 *            - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *            - verifying that the pointer to the key buffer is not DX_NULL. 
 *            - verifying the value of the operation mode is valid . 
 *        2. Capturing the working context by calling the CRYS_CCM_GetContext().
 *        3. Initializing the working context by the following:
 *            - loading the key.
 *            - loading the control field 
 *            - loading the operation mode. 
 *            - loading the DecryptEncrypt flag .
 *        4. Encrypting the information in the working context and storing it to the 
 *           users received context. After then the working context is released.
 *           This state is operated by calling the CRYS_CCM_EncryptAndReleaseContext call.
 *        5. Exit the handler with the OK code.
 *      
 *
 * @param[in] ContextID_ptr - A pointer to the context buffer allocated by the user that
 *                            is used for the C2 Cipher operations.
 *
 * @param[in] Key_ptr -  A pointer to the users key buffer.
 *
 * @param[in] EncryptDecryptMode - This flag determines if the C2 shall perform an Encrypt 
 *                                 operation [0] or a Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] CBC_ResetInterval - CBC chain breaking interval. The value not used in ECB mode
 * 
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_C2_error.h
 */
CIMPORT_C CRYSError_t  CRYS_C2_CipherInit( 
                            CRYS_C2CipherUserContext_t    *ContextID_ptr,
                            CRYS_C2_Key_t                 Key_ptr,
                            CRYS_C2_EncryptMode_t         EncryptDecryptMode,
                            CRYS_C2_OperationMode_t        OperationMode,
                            DxUint32_t                     CBC_ResetInterval );
                            
                            
/*****************************************************************************************************/                         
/** 
 * @brief This function is used to perform C2 Cipher operation on a block of data.
 *
 *        This function should be called after the CRYS_C2_Init function
 *        was called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying that the pointer of the data_in buffer is not DX_NULL.
 *          - verifying that the pointer to the data_out buffer is not DX_NULL. 
 *          - verifying the values of the data_in buffers size is not 0 and is a multiple of 8 bytes.
 *
 *        2.Decrypts the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *        3.Executes the C2 Cipher operation on the SW or HW by calling the 
 *          low level function LLF_C2_CipherBlock.
 *        4.Encrypts the information in the working context and stores it 
 *          to the users received context and releases the context.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exits the handler with the error code.  
 *      
 *
 * @param[in] ContextID_ptr - A pointer to the C2 Cipher context buffer allocated by the user that
 *                            is used for the C2 operation. This should be the same context that was
 *                            used on the previous call of this session.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the C2 Cipher. The pointer does 
 *                         not need to be aligned to 32 bits word.
 *
 * @param[in] DataInSize - The size of the input data (must be not 0 and must be multiple of 8 bytes).
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data from the C2 Cipher. 
 *                              The pointer does not need to be aligned to 32 bits word.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
 CIMPORT_C CRYSError_t  CRYS_C2_CipherBlock( 
                                CRYS_C2CipherUserContext_t       *ContextID_ptr,   
                                DxUint8_t                        *DataIn_ptr,     
                                DxUint32_t                        DataInSize,     
                                DxUint8_t                        *DataOut_ptr );
 

/*****************************************************************************************************/
/**
 * @brief This function is used to end the C2 Cipher operation session.
 *
 *        It is the last function called on the C2 Cipher operation.
 *
 *        The function executes the following major steps:
 *
 *        1. Checks that the context pointer is not DX_NULL (*ContextID_ptr). . 
 *           If received parameter is not valid it shall return an error.
 *        2. Clearing the users context.
 *        3. Exits the handler with the OK code.
 *      
 *
 * @param[in] ContextID_ptr - a pointer to the C2_CIPHER context buffer allocated by the user that
 *                            was used on the previous call of this session.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value from CRYS_C2_error.h
 */
CIMPORT_C CRYSError_t  CRYS_C2_CipherFree(CRYS_C2CipherUserContext_t  *ContextID_ptr );


/********************************************************************************************************/
/**
 * @brief This function is used to operate the C2 Cipher in one integrated operation.
 *
 *        The function simply calls the C2_Cipher Init, Block and Finish functions sequentially.
 *
 * @param[in] Key_ptr - a pointer to the users key buffer.
 *
 * @param[in] EncryptDecryptMode - This flag determines if the C2 shall perform an Encrypt operation [0] or a
 *                                 Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] CBC_ResetInterval - CBC chain breaking interval. The value not used in ECB mode
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the C2 Cipher.
 *                         The pointer does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data (must be not 0 and must be multiple of 8 bytes).
 *
 * @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the C2 Cipher. 
 *                              The pointer does not need to be word-aligned.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
 CIMPORT_C CRYSError_t  CRYS_C2_Cipher(
                CRYS_C2_Key_t           Key_ptr,                /* in */
                CRYS_C2_EncryptMode_t    EncryptDecryptFlag,     /* in */
                CRYS_C2_OperationMode_t  OperationMode,          /* in */  
                DxUint32_t               CBC_ResetInterval , 
                DxUint8_t               *DataIn_ptr,             /* in */ 
                DxUint32_t               DataInSize,             /* in */ 
                DxUint8_t               *DataOut_ptr );          /* in/out */
                
                
 #if(LLF_C2_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE)
/****************************************************************************************************/
/******************                  C2 HASH FUNCTIONS PROTOTYPES                ********************/ 
/****************************************************************************************************/

/****************************************************************************************************/
/**
 * This function initializes the C2 HASH structures and machine registers on the CRYS level.
 *
 * This function allocates and initializes the C2 HASH Context .
 * The function receives as input a pointer to store the context handle to C2 HASH Context, 
 * it initializes the  C2 HASH Context with the cryptographic attributes that are needed for 
 * the C2 HASH block operation ( initialize H's value for the C2 HASH algorithm ).
 *
 * The function flow:
 *
 *  1) Checking the validity of the arguments - returns an error on an illegal argument case.
 *  2) Acquiring the working context from the CCM manager.
 *  3) Initializing the context with the parameters passed by the user and with the init values
 *     of the C2 HASH.
 *  4) Loading the user tag to the context.
 *  5) Release the CCM context.
 * 
 * @param[in] ContextID_ptr - A pointer to the C2 HASH context buffer allocated by the user.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 *      
 */
CIMPORT_C CRYSError_t CRYS_C2_HASH_Init(CRYS_C2HASHUserContext_t     *ContextID_ptr);


/****************************************************************************************************/
/**
 * This function performs C2 HASH operations on block of data.
 *
 * The function flow:
 *
 *  1) Checking the parameters validity if there is an error the function shall exit with an error code. 
 *  2) Acquiring the working context from the CCM manager.
 *  3) If there isn't enough data in the previous update data buff in the context plus the received data,
 *     the function loads received data to the context buffer and exits.
 *  4) Filling the previous update data buffer to contain an entire block. 
 *  5) Calling the low level C2 HASH function to execute the update.
 *  6) Filling the previous update data buffer with the data not processed at the end of the received data.
 *  7) Releases the CCM context.
 * 
 * @param[in] ContextID_ptr - A pointer to the C2 HASH context buffer allocated by the user.
 *
 * @param[in] DataIn_ptr - A pointer to the buffer that stores the data to be hashed .
  * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 *
 */
CIMPORT_C CRYSError_t CRYS_C2_HASH_Update( CRYS_C2HASHUserContext_t  *ContextID_ptr,
                                           DxUint8_t                 *DataIn_ptr,
                                           DxUint32_t                 DataInSize ); 
                                           
                                           
/****************************************************************************************************/
/**
 * This function finalize the hashing process of data block.
 *
 *  The function flow:
 *
 *  1) Checks the parameters validity if there is an error the function shall exit with an error code. 
 *  2) Acquiring the working context from the CCM manager.
 *  3) Setting the padding bit to last block according to CPRM requirements.
 *  4) Calling the C2 ECB Encrypt low level function to execute the last padded block.
 *  5) Output HASH result.
 *  6) Calls the CRYS_C2_HASH_Free function and exits.
 *
 *  @param[in] ContextID_ptr - A pointer to the HASH context buffer allocated by the user that
 *                             is used for the HASH machine operation.
 *
 *  @param[out] HashResultBuff - A pointer to the target buffer where the 
 *                              C2 HASH result stored in the context is loaded to.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
CIMPORT_C CRYSError_t CRYS_C2_HASH_Finish( CRYS_C2HASHUserContext_t  *ContextID_ptr ,
                                           CRYS_C2HASH_Result_t       HashResultBuff );


/****************************************************************************************************/
/**
 * @brief This function is a service function that frees the context if the operation has
 *        failed or C2 is finished. 
 *
 *        The function executes the following steps:
 *
 *        1. Checks that the context pointer is not DX_NULL (*ContextID_ptr). 
 *        2. Clearing the users context.
 *        3. Exit the handler with the OK code.
 *
 * @param[in] ContextID_ptr - a pointer to the C2 HASH context buffer allocated by the user that
 *                       was used for the C2 HASH operations. This should be the same context 
 *                       that was used on the previous call of this session.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
CIMPORT_C CRYSError_t  CRYS_C2_HASH_Free(CRYS_C2HASHUserContext_t  *ContextID_ptr ); 



/****************************************************************************************************/                     
/**
 * This function performs all C2 HASH operations on one buffer of data.
 *
 * The function simply sequentially calls the C@ HASH Init, Block and Finish functions.
 *
 *
 * @param[in] DataIn_ptr - A pointer to the buffer that stores the data to be hashed .
 * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. 
 *
 * @param[out] HashResultBuff - A pointer to the target buffer where the 
 *                              C2 HASH result stored in the context is loaded to.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
CIMPORT_C CRYSError_t CRYS_C2_HASH( 
                                DxUint8_t              *DataIn_ptr,
                                DxUint32_t              DataSize,
                                CRYS_C2HASH_Result_t    HashResultBuff );
                                
 

/*****************************************************************************************************/                        
/** 
* @brief This function is used to reset CBC Key chain.
*
*        This function should be called for CBC Key Chain reseting.
*
*        The function executes the following major steps:
*
*        1.Checks the validation of all of the inputs of the function.
*          If one of the received parameters is not valid it shall return an error.
*
*        2.Decrypting the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext() call.

*        3.executing the C2 CBC Key Reset operation on the SW by calling the 
*          low level function LLF_C2_CipherBlock.
*        4.Encrypting the information in the working context and storing it 
*          to the users received context. After then the working context is released.
*          This state is operated by calling the CRYS_CCM_ReleaseContext call.
*        5.Exit the handler with the OK code.  
*      
*
* @param[in] ContextID_ptr - A pointer to the C2 Cipher context buffer allocated by the user that
*                            is used for the C2 operation. This should be the same context that was
*                            used on the previous call of this session.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value from CRYS_C2_error.h
*/
CIMPORT_C CRYSError_t  CRYS_C2_CBC_ResetChain(CRYS_C2CipherUserContext_t *ContextID_ptr );

 
/****************************************************************************************************/
/******************                  C2 ONE WAY FUNCTIONS PROTOTYPES             ********************/ 
/****************************************************************************************************/

                                
/****************************************************************************************************/                         
/** 
 * @brief This function is used to perform C2 One Way operation on 64-bit block of data.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error. 
 *          The checkers that are run over the received parameters:
 *             - verifying that the pointers are not DX_NULL.
 *        2.Executing the C2 ECB encrypt operation of Data2 with key = Data1 by calling the 
 *          CRYS function CRYS_C2_Cipher.
 *        3.Performs XOR of encrypt output with Data2 and output this result.
 *
 *        4.Exit.  
 *
 * @param[in] Data1_ptr -  The pointer to the buffer of the input Data1 used as encrypting key. 
 *                         The pointer need to be aligned to 32 bits.
 *
 * @param[in] Data2 - The pointer to the 64-bits buffer of the input Data2.
 *                    The pointer not need to be aligned to 32 bits.
 *
 * @param[in/out] DataOut_ptr - The pointer to the 64-bits buffer for output data.
 *                              The pointer not need to be aligned to 32 bits. 
 *
 *  Note: Overlapping between the data input and data output buffers is not allowed,
 *        except the in place case that is legal.
 *                               
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
 CIMPORT_C CRYSError_t  CRYS_C2_OneWayFunc( CRYS_C2_Key_t           Data1_ptr,     
                                            CRYS_C2_BLOCK_BYTES_t   Data2_ptr,
                                            CRYS_C2_BLOCK_BYTES_t   DataOut_ptr );                                

                                                         
 
 /***************************************************************************************************/
 /******************                 Prepare secret constant Utility             ********************/ 
 /***************************************************************************************************/

 /************************ Prepare secret constant Utility   **********************/
 /*
 *
 *   This function calculates optimized Secret Constant words-array on base of bytes-array
 *   received from 4C Entity, LLC.
 * 
 *   The user must call this function only on programming stage for creation the optimized 
 *   Secret Constant array. Then, the created data must be inserted (hard coded) into SecretConst32[]  
 *   array in the file LLF_C2_tables.c .
 *
 *   \algorithm: according to 4C Entity, LLC. C2 Block Cipher Specification Revision 1.0. 
 *
 * @param[in]  SecretConstant8_ptr  - The pointer to the Secret Constant buffer (bytes), received 
 *                                    from 4C Entity. The size of the buffer is 256 bytes.
 * @param[out] SecretConstant32_ptr - The pointer to the optimized SecretConstant32 (words).
 *                                    The size of the buffer must be 256 words.
 *
 */
 CIMPORT_C CRYSError_t  CRYS_C2_PrepareSecretConstant( DxUint8_t  *secretConstant8_ptr, 
	                                                   DxUint32_t *secretConstant32_ptr );
#endif

#ifdef __cplusplus
}
#endif

#endif
