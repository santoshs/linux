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
 
 
#ifndef CRYS_RC4_H
#define CRYS_RC4_H

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
   *  Object % CRYS_RC4.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:42:30 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS RC4 APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_RC4.h#1:incl:1
   *  \author adams
   */

/************************ Defines ******************************/

/** 
@brief - This module defines the interface to the RC4 cryptographic support
*/

#define CRYS_RC4_S_BOX_SIZE_IN_BYTES 256
#define CRYS_RC4_MAX_KEY_SIZE_IN_BYTES 256

/* HW required input block size */
#define  CRYS_RC4_HW_BLOCK_SIZE_IN_BYTES   4
#define  CRYS_RC4_HW_BLOCK_SIZE_IN_WORDS   (CRYS_RC4_HW_BLOCK_SIZE_IN_BYTES / sizeof(DxUint32_t))


/************************ context Structs  ******************************/

/* The context data structure used by the RC4 functions */
typedef struct  
{ 

   /* the state */
   DxUint8_t	 RC4_I, RC4_J;
   
   /* IsSboxInitialized - flag: if S-box initialized the flag value is 1, else 0.*/
   DxUint32_t  IsSboxInitialized;

   /* Including the context that is specific to the platform we are working on */
   #include "LLF_RC4_context_def.h"
   
   /* This Buffer is added for encrypting the context ( encrypted part's size must be 0 modulo 16).
    * Note: This block must be at the end of the context.
    */
   DxUint8_t DummyBufAESBlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];
     
}RC4Context_t; 

/* The user's context prototype - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_RC4UserContext_t 
{
   DxUint32_t valid_tag;
   DxUint32_t AES_iv;/* For use of the AES CBC mode of Encryption and Decryption of the context in CCM */ 
   DxUint8_t  context_buff[ sizeof(RC4Context_t) ]; 

}CRYS_RC4UserContext_t;

/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief This function is used to initialize the RC4 machine.
 *        To operate the RC4 machine, this should be the first function called.      
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer that is allocated by the user 
 *                       and is used for the RC4 machine operation.
 * @param[in] Key_ptr -  A pointer to the user's key buffer.
 * @param[in] KeySize - The size of the KEY in bytes. Requirements:
 *             - for SW implementation    0 < KeySize < CRYS_RC4_MAX_KEY_SIZE_IN_BYTES,
 *             - for HW implementation    LLF_RC4_MIN_KEY_SIZE_IN_BYTES  < KeySize < LLF_RC4_MAX_KEY_SIZE_IN_BYTES,
 *
 * @return CRYSError_t - CRYS_OK,
 *                       CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                       CRYS_RC4_ILLEGAL_KEY_SIZE_ERROR,
 *                       CRYS_RC4_INVALID_KEY_POINTER_ERROR
 */
CIMPORT_C CRYSError_t  CRYS_RC4_Init( CRYS_RC4UserContext_t    *ContextID_ptr,
                                      DxUint8_t                *Key_ptr, 
                                      DxUint32_t                KeySizeInBytes);


/************************************************************************************************/
/**
 * @brief This function is used to process a stream on the RC4 machine.
 *        This function should be called after the CRYS_RS4_Init. 
 *      
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer allocated by the user 
 *                       that is used for the RC4 machine operation. This should be the 
 *                       same context as was used for the previous call of this session.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the RC4. 
 *                   The pointer's value does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data.
 *
 * @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the RC4. 
 *                        The pointer's value does not need to be word-aligned.  
 *
 * @return CRYSError_t - CRYS_OK,
 *                       CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                       CRYS_RC4_ILLEGAL_KEY_SIZE_ERROR,
 *                       CRYS_RC4_INVALID_KEY_POINTER_ERROR
 */
CIMPORT_C   CRYSError_t  CRYS_RC4_Stream(
	                          CRYS_RC4UserContext_t        *ContextID_ptr,   
                              DxUint8_t                    *DataIn_ptr,                                  
                              DxUint32_t                    DataInSize,
                              DxUint8_t                    *DataOut_ptr);  


/************************************************************************************************/
/**
 * @brief This function is used to end the RC4 processing session.
 *        It is the last function called for the RC4 process.
 *      
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer allocated by the user that
 *                       is used for the RC4 machine operation. This should be the 
 *                       same context as was used for the previous call of this session.
 *
 *
 * @return CRYSError_t - CRYS_OK,
 *                       CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                       CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR,
 *                       CRYS_RC4_DATA_IN_POINTER_INVALID_ERROR,
 *                       CRYS_RC4_DATA_OUT_POINTER_INVALID_ERROR,
 *                       CRYS_RC4_DATA_OUT_DATA_IN_OVERLAP_ERROR,
 *                       CRYS_RC4_DATA_SIZE_ILLEGAL
 */
CIMPORT_C CRYSError_t  CRYS_RC4_Free(CRYS_RC4UserContext_t  *ContextID_ptr );


/************************************************************************************************/
/**
 * \brief This function provides a RC4 function for processing data.
 *
 * The function allocates an internal RC4 Context, and initializes the RC4 Context with the
 * cryptographic attributes that are needed for the RC4 cryptographic operation. Next the
 * function loads the engine with the initializing values, and then processes the data,
 * returning the processed data in the output buffer. Finally, the function frees the
 * internally allocated context.
 *
 * @param[in] Key_ptr -  A pointer to the user's key buffer.
 *
 * @param[in] KeySize - The size of the KEY in bytes.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the RC4. 
 *                   The pointer's value does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data.
 *
 * @param[in,out] The pointer to the buffer of the output data from the RC4.
 *                The pointer's value does not need to be word-aligned. The size of this buffer
 *                must be the same as the DataIn buffer.  
 *
 * @return CRYSError_t -  CRYS_OK, 
 *                        CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
 *                        CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR
 *
 */
CIMPORT_C   CRYSError_t CRYS_RC4( DxUint8_t    *Key_ptr, 
							      DxUint32_t    KeySizeInBytes,
								  DxUint8_t    *DataIn_ptr, 
								  DxUint32_t    DataInSize,
								  DxUint8_t    *DataOut_ptr );  
								  
								  
                                      								  

#ifdef __cplusplus
}
#endif

#endif
