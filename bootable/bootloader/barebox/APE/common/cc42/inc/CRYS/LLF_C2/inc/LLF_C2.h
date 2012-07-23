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
 
 
#ifndef LLF_C2_H
#define LLF_C2_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_C2.h"
#include "LLF_C2_Cipher_context_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Feb 19 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_C2.h#1:incl:1
   *  \author R. Levin .
   */


/************************ Defines ******************************/

/* defining the minimum number of blocks that will activate a DMA operation.
   less from this number the C2 will work on the direct mode , since it is not
   efficient on real time */  
#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_C2_MIN_C2_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_C2_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_C2_MIN_C2_BLOCKS_FOR_DMA_OPERATION    2 /*(512/CRYS_C2_BLOCK_SIZE_IN_BYTES)*/
#endif	
	
	
#define LLF_HW_SEC_CONST_SIZE 256

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/


/* 
 * @brief This function is used to initialize the C2 Cipher secret constant
 *
 *        The function executes the following major steps:
 *          
 *         1. Activate HW semaphore
 *         2. load SBox
 *	       3. Release HW semaphore
 *       
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_C2_error.h
 */
 
CEXPORT_C CRYSError_t  LLF_C2_SBoxInit();


/**
 * @brief The low level LLF_C2_CipherBlock function operates the C2 engine .
 *        
 *    The low level LLF_C2Cipher_Block function initializes needed hardware flags and registers, 
 *    and start C2 engine which actually perform the C2 Cipher cryptographic algorithm according to
 *    user defined operation mode (ECB, CBC) and encrypt-decrypt mode.  
 *  1.	 Checks the operation mode (EBC, C-CBC) and encrypt-decrypt mode (encrypt, decrypt) 
 *  2. Activate HW semaphore
 *  3.	Resume the engine
 *  4.	Initializes hardware flags and registers using contextBuff (defined in paragraph 5.1.1.1)
 *  5.	LLF C2 engine enc/dec operations
 *      •   Direct Data Load Mode
 *           i.	Load data block to be processed into DIN
 *          ii.	poll DOUT for data availability
 *          iii.	read the processed data from DOUT 
 *      •   DMA Mode
 *          i.  set up DMA engine
 *         ii.  poll DMA engine for DOUT completion
 *  6.	Suspend the engine
 *  7.	Read flags and registers to contextBuff (defined in paragraph 5.1.1.1) to use it in next block operation.
 *  8.	Release HW semaphore
 *
 * @param[in] WorkingContextID_ptr - The C2Cipher context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  CRYSError_t  LLF_C2_CipherBlock( C2CipherContext_t  *WorkingContextID_ptr,    
                                   DxUint8_t          *DataIn_ptr,                     
                                   DxUint32_t          DataInSize,                      
                                   DxUint8_t          *DataOut_ptr );  


/*************************************************************************************/
/**
 * @brief The low level LLF_C2_Init initializes the spesific low level initializatios 
 *        required on the spesific platform.
 *  The function initialized registers of C2 engine and save them in contextBuff 
 *
 * 1.	Activate HW semaphore
 * 2.	Initialize C2 engine registers
 * 3.	Suspend the engine
 * 4.	Read engine state registers and save them in contextBuff (defined in paragraph 5.1.1.1 of C2 SDD) 
 * 5.	Release HW semaphore 
 *
  * @param[in] WorkingContextID_ptr - A pointer to the context buffer allocated by the user that
 *                            is used for the C2 Cipher operations.
 *
 * @param[in] Key_ptr -  A pointer to the users key buffer.
 *
 * @param[in] EncryptDecryptMode - This flag determains if the C2 shall perform an Encrypt 
 *                                 operation [0] or a Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] CBC_ResetInterval -  CBC chain breaking interval. The value not used in ECB mode .
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_C2_CipherInit( C2CipherContext_t          *WorkingContextID_ptr,
                                CRYS_C2_Key_t               Key_ptr,
                                CRYS_C2_EncryptMode_t       EncryptDecryptMode,
                                CRYS_C2_OperationMode_t     OperationMode,
                                DxUint32_t                  CBC_ResetInterval);  





 	          
#ifdef __cplusplus
}
#endif

#endif


