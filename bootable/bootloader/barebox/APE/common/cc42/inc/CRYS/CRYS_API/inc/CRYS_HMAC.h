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
 
 #ifndef CRYS_HMAC_H
#define CRYS_HMAC_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_HASH.h"
#include "CRYS_Defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_MAC.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 28 20:51:23 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS HMAC APIs, as well as the APIs themselves
   *
   *  \version CRYS_HASH.h#1:incl:13
   *  \author adams
   */
 
/************************ Defines ******************************/

/* The HMAC key size after padding for MD5, SHA1, SHA256 */
#define CRYS_HMAC_KEY_SIZE_IN_BYTES 64

/* The HMAC key size after padding for SHA384, SHA512 */
#define CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES 128

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/* ********************** Structures *************************** */

/* The context data base used by the HMAC functions on the low level */
typedef struct
{    
   /* Key XOR opad ( 0x5C bytes ) result */
   DxUint8_t KeyXorOpadBuff[CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES];
   
   /* The operation mode */
   CRYS_HASH_OperationMode_t OperationMode;
  
   /* The user HASH context - required for operating the HASH described below */
   CRYS_HASHUserContext_t HashUserContext;

   /* This Buffer is added for encrypting the context ( encrypted part's size must be 0 modulo 16).
    * Note: This block must be at the end of the context.
    */
   DxUint8_t DummyBufAESBlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];
      
}HMACContext_t; 

/* The user's context prototype - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_HMACUserContext_t 
{
   DxUint32_t valid_tag;
   DxUint32_t AES_iv;/* For use of the AES CBC mode of Encryption and Decryption of the context in CCM */
   DxUint8_t  context_buff[ sizeof(HMACContext_t) ]; 

}CRYS_HMACUserContext_t;

/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * This function initializes the HMAC machine on the CRYS level.
 *
 * The function allocates and initializes the HMAC Context .
 * The function receives as input a pointer to store the context handle to HMAC Context. 
 *
 * If the key is larger than 64 bytes, this function performs an MD5 HASH on it.
 *
 * The function executes a HASH_init session and processes a HASH update 
 * on the Key XOR ipad and stores it in the context.
 *
 * @param[in] ContextID_ptr - A pointer to the HMAC context buffer allocated by the user 
 *                       that is used for the HMAC machine operation.
 *
 * @param[in] OperationMode - The operation mode: MD5 or SHA1.
 *
 * @param[in] key_ptr - The pointer to the user's key buffer.
 *
 * @param[in] keySize - The size of the received key.
 *
 * @return CRYSError_t - On success the function returns the value CRYS_OK, 
 *			and on failure a non-ZERO error.
 *      
 */
CIMPORT_C CRYSError_t CRYS_HMAC_Init(CRYS_HMACUserContext_t     *ContextID_ptr,
                           CRYS_HASH_OperationMode_t  OperationMode,
                           DxUint8_t                    *key_ptr,
                           DxUint16_t                    keySize );


/**
 * This function processes a HMAC block of data via the HASH hardware/software.
 * The function receives as input a handle to the HMAC Context, 
 * and performs a HASH update on the data described below.
 * 
 * @param[in] ContextID_ptr - A pointer to the HMAC context buffer allocated by the user 
 *                       that is used for the HMAC machine operation.
 *
 * @param DataIn_ptr - A pointer to the buffer that stores the data to be hashed.
 * 
 * @param DataInSize - The size of the data to be hashed, in bytes. 
 *
 * @return CRYSError_t - On success the function returns CRYS_OK, 
 *			and on failure a non-ZERO error.
 */

CIMPORT_C CRYSError_t CRYS_HMAC_Update(CRYS_HMACUserContext_t  *ContextID_ptr,
                             DxUint8_t                 *DataIn_ptr,
                             DxUint32_t                 DataInSize );


/**
 * This function finalizes the HMAC processing of a data block.
 * The function receives as input a handle to the HMAC Context that was previously initialized 
 * by a CRYS_HMAC_Init function or by a CRYS_HMAC_Update function.
 * This function finishes the HASH operation on the ipad and text, and then 
 * executes a new HASH operation with the key XOR opad and the previous HASH operation result.
 *
 *  @param[in] ContextID_ptr - A pointer to the HMAC context buffer allocated by the user 
 *                       that is used for the HMAC machine operation.
 *
 *  @retval HmacResultBuff - A pointer to the target buffer where the 
 *                       HMAC result stored in the context is loaded to.
 *
 * @return CRYSError_t - On success the function returns CRYS_OK, 
 *			and on failure a non-ZERO error.
 */

CIMPORT_C CRYSError_t CRYS_HMAC_Finish( CRYS_HMACUserContext_t  *ContextID_ptr ,
                              CRYS_HASH_Result_t       HmacResultBuff );


/**
 * @brief This function is a service function that frees the context if the operation has failed.
 *
 *        The function executes the following major steps:
 *
 *        1. Checks the validity of all of the inputs of the function. 
 *           If one of the received parameters is not valid it shall return an error.
 *           The major checks that are run over the received parameters:
 *           - verifying that the context pointer is not DX_NULL (*ContextID_ptr). 
 *        2. Clears the user's context.
 *        3. Exits the handler with the OK code.
 *      
 * @param[in] ContextID_ptr - A pointer to the HMAC context buffer allocated by the user 
 *                       that is used for the HMAC machine operation. This should be the 
 *                       same context that was used for the previous call of this session.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from  CRYS_error.h
 */

CIMPORT_C CRYSError_t  CRYS_HMAC_Free(CRYS_HMACUserContext_t  *ContextID_ptr );


/**
 * This function provide HASH function to process one buffer of data.
 * The function allocates an internal HASH Context , it initializes the 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 * Then the function loads the Hardware with the initializing values and after 
 * that process the data block using the hardware to do hash .
 * At the end the function return the message digest of the data buffer .
 *
 *
 * @param[in] OperationMode - The operation mode : MD5 or SHA1.
 *
 * @param[in] key_ptr - The pointer to the users key buffer.
 *
 * @oaram[in] keySize - The size of the received key.
 * 
 * @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HMAC. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 * 
 * @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
 *                         multiple of HASH_BLOCK_SIZE for used HASH mode.
 *
 * param[out] HashResultBuff - a pointer to the target buffer where the 
 *                      HMAC result stored in the context is loaded to.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
CIMPORT_C CRYSError_t CRYS_HMAC  ( CRYS_HASH_OperationMode_t  OperationMode,
                         DxUint8_t                    *key_ptr,
                         DxUint16_t                    keySize,
                         DxUint8_t                    *DataIn_ptr,
                         DxUint32_t                    DataSize,
                         CRYS_HASH_Result_t          HmacResultBuff );
#ifdef __cplusplus
}
#endif

#endif
