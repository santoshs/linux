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
 
 
#ifndef CRYS_DES_H
#define CRYS_DES_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#ifndef CRYS_NO_DES_SUPPORT
#include "LLF_DES_EngineInfo.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_DES.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:55:05 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS DES APIs, as well as the APIs themselves.
   *
   *  \version CRYS_DES.h#1:incl:13
   *  \author adams
   */


/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/

#define CRYS_DES_ENGINE_TYPE  LLF_DES_ENGINE_TYPE

/* The size of the IV or counter buffer */
#define CRYS_DES_IV_SIZE_IN_WORDS 2
#define CRYS_DES_IV_SIZE_IN_BYTES ( CRYS_DES_IV_SIZE_IN_WORDS * sizeof(DxUint32_t) )

/* The maximum number of KEYS supported by DES */
#define CRYS_DES_MAX_NUMBER_OF_KEYS 3

/* The key size in words on the DES machine */
#define CRYS_DES_KEY_SIZE_IN_WORDS 2
#define CRYS_DES_KEY_SIZE_IN_BYTES ( CRYS_DES_KEY_SIZE_IN_WORDS * sizeof(DxUint32_t) )

/* The DES block size in bytes */
#define CRYS_DES_BLOCK_SIZE_IN_BYTES 8

/* The DES block size in words */
#define CRYS_DES_BLOCK_SIZE_IN_WORDS 2

/************************ Enums ********************************/

/* The number of keys supported on the DES machine */
typedef enum
{
   CRYS_DES_1_KeyInUse  = 1,
   CRYS_DES_2_KeysInUse = 2,
   CRYS_DES_3_KeysInUse = 3,
   
   CRYS_DES_NumOfKeysOptions,
   
   CRYS_DES_NumOfKeysLast= 0x7FFFFFFF,

}CRYS_DES_NumOfKeys_t;

/* Enum definitions for the Encrypt or Decrypt operation mode */
typedef enum 
{
   CRYS_DES_Encrypt = 0,
   CRYS_DES_Decrypt = 1,
   
   CRYS_DES_EncryptNumOfOptions,

   CRYS_DES_EncryptModeLast= 0x7FFFFFFF,

}CRYS_DES_EncryptMode_t;

/* Enum definitions for the DES operation mode */
typedef enum
{
   CRYS_DES_ECB_mode = 0,
   CRYS_DES_CBC_mode = 1,

   CRYS_DES_NumOfModes,

   CRYS_DES_OperationModeLast= 0x7FFFFFFF,

}CRYS_DES_OperationMode_t;      
   
/************************ Typedefs  ****************************/

/* The IV buffer definition */
typedef DxUint8_t CRYS_DES_Iv_t[CRYS_DES_IV_SIZE_IN_BYTES];

/* defining the KEY argument - containing 3 keys maximum */
typedef struct CRYS_DES_Key_t
{
   /* the key variables */
   DxUint8_t key1[CRYS_DES_KEY_SIZE_IN_BYTES]; 
   DxUint8_t key2[CRYS_DES_KEY_SIZE_IN_BYTES];
   DxUint8_t key3[CRYS_DES_KEY_SIZE_IN_BYTES];

}CRYS_DES_Key_t;   

/************************ Structs  ******************************/

/* Defines the KEY argument - contains 3 keys maximum */
typedef struct 
{
   /* the key variables */
   DxUint32_t key1[CRYS_DES_KEY_SIZE_IN_WORDS]; 
   DxUint32_t key2[CRYS_DES_KEY_SIZE_IN_WORDS];
   DxUint32_t key3[CRYS_DES_KEY_SIZE_IN_WORDS];

}CRYS_DES_Key_32bit_t;   

/************************ Structs  ******************************/

/* The context data base used by the DES functions on the low level */
typedef struct  
{ 
   DxUint32_t                  DESContextIV[CRYS_DES_IV_SIZE_IN_WORDS];
   CRYS_DES_Key_32bit_t      DESContextKey;
   CRYS_DES_NumOfKeys_t      NumOfKeys;
   CRYS_DES_EncryptMode_t    EncryptDecryptFlag;
   CRYS_DES_OperationMode_t  OperationMode;  

   /* Include the context that is specific for the platform we are working on */
   #include "LLF_DES_context_def.h"

   /* This Buffer is added for encrypting the context ( encrypted part's size must be 0 modulo 16).
    * Note: This block must be at the end of the context.
    */
   DxUint8_t DummyBufAESBlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];
   
}DESContext_t; 

/* The user's context prototype - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_DESUserContext_t 
{
   DxUint32_t valid_tag;
   DxUint32_t AES_iv;/* For use of the AES CBC mode of Encryption and Decryption of the context in CCM*/            
   DxUint8_t  context_buff[ sizeof(DESContext_t) ]; 

}CRYS_DESUserContext_t;

/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief This function is used to initialize the DES machine.
 *        To operate the DES machine, this should be the first function called.
 *
 * @param[in] ContextID_ptr  - A pointer to the DES context buffer allocated by the user 
 *                       that is used for the DES machine operation.
 *
 * @param[in,out] IV_ptr - The buffer of the IV.
 *                          In ECB mode this parameter is not used.
 *                          In CBC this parameter should contain the IV values.
 *
 * @param[in] Key_ptr - A pointer to the user's key buffer.
 *
 * @param[in] NumOfKeys - The number of keys used: 1, 2, or 3 (defined in the enum).
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

CIMPORT_C CRYSError_t  CRYS_DES_Init( CRYS_DESUserContext_t    *ContextID_ptr,
                            CRYS_DES_Iv_t            IV_ptr,
                            CRYS_DES_Key_t           *Key_ptr,
                            CRYS_DES_NumOfKeys_t     NumOfKeys,
                            CRYS_DES_EncryptMode_t   EncryptDecryptFlag,
                            CRYS_DES_OperationMode_t OperationMode );


/**
 * @brief This function is used to process a block on the DES machine.
 *        This function should be called after the CRYS_DES_Init function was called.
 *      
 *
 * @param[in] ContextID_ptr - a pointer to the DES context buffer allocated by the user that
 *                       is used for the DES machine operation. this should be the same context that was
 *                       used on the previous call of this session.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the DES. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 *
 * @param[in] DataInSize - The size of the input data in bytes: must be not 0 and must be multiple 
 *                         of 8 bytes.
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data from the DES. The pointer does not 
 *                              need to be aligned. On CSI output mode the pointer must be equal to
 *                              value (0xFFFFFFFC | DataOutAlignment).  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 *
 *     NOTE: Temporarily it is not allowed, that both the Input and the Output simultaneously 
 *           were on CSI mode
 */
 CIMPORT_C CRYSError_t  CRYS_DES_Block( CRYS_DESUserContext_t       *ContextID_ptr, /* in */ 
                              DxUint8_t                     *DataIn_ptr,    /* in */ 
                              DxUint32_t                    DataInSize,     /* in */ 
                              DxUint8_t                     *DataOut_ptr ); /* in/out */ 
 

/**
 * @brief This function is used to end the DES processing session.
 *        It is the last function called for the DES process.
 *      
 *
 * @param[in] ContextID_ptr  - A pointer to the DES context buffer allocated by the user that
 *                       is used for the DES machine operation. this should be the 
 *                       same context that was used on the previous call of this session.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_error.h
 */
CIMPORT_C CRYSError_t  CRYS_DES_Free(CRYS_DESUserContext_t  *ContextID_ptr );


/**
 * @brief This function is used to operate the DES machine in one integrated operation.
 *
 *        The actual macros that will be used by the users are:
 *      
 *
 * @param[in,out] IVCounter_ptr - this parameter is the buffer of the IV or counters on mode CTR.
 *                          On ECB mode this parameter has no use.
 *                          On CBC mode this parameter should containe the IV values.
 *
 * @param[in] Key_ptr - a pointer to the users key buffer.
 *
 * @param[in] KeySize - Thenumber of keys used by the DES as defined in the enum.
 *
 * @param[in] EncryptDecryptFlag - This flag determains if the DES shall perform an Encrypt operation [0] or a
 *                           Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the DES. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 *
 * @param[in] DataInSize - The size of the input data in bytes: must be not 0 and must be multiple 
 *                         of 8 bytes.
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data from the DES. The pointer does not 
 *                              need to be aligned. On CSI output mode the pointer must be equal to
 *                              value (0xFFFFFFFC | DataOutAlignment).  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 *
 *
 *     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously 
 *               were on CSI mode.
 *            2. Temporarily the CSI input or output are not allowed on XCBC, CMAC and XTS modes.    
 *
 */
 CIMPORT_C CRYSError_t  CRYS_DES(
                CRYS_DES_Iv_t             IV_ptr,                 /* in */ 
                CRYS_DES_Key_t           *Key_ptr,                /* in */ 
                CRYS_DES_NumOfKeys_t      NumOfKeys,              /* in */ 
                CRYS_DES_EncryptMode_t    EncryptDecryptFlag,     /* in */  
                CRYS_DES_OperationMode_t  OperationMode,          /* in */     
                DxUint8_t                *DataIn_ptr,             /* in */ 
                DxUint32_t                DataInSize,             /* in */ 
                DxUint8_t                *DataOut_ptr );          /* out */ 

#ifdef __cplusplus
}
#endif

#endif
