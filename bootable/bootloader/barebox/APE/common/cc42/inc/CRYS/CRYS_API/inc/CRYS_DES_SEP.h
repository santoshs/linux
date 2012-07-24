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
 
 
#ifndef CRYS_DES_SEP_H
#define CRYS_DES_SEP_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "LLF_DES_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_DES_SEP.h    : %
   *  State           :  %state%
   *  Creation date   :  27 March 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains DES functions APIs, spcified for using with SEP only.
   *
   *  \version CRYS_DES_SEP.h#1:incl:13
   *  \author R.Levin.
   */


/************************ Defines ***************************************/

/************************ Enums *****************************************/

/************************ Typedefs  *************************************/

/************************ context Structs  ******************************/

/************************ Public Variables ******************************/

/************************ Public Functions ******************************/


/**************************************************************************************************/
/** 
 * @brief This function is used to operate a block on the DES machine in Multi LLI mode from SEP.
 *        This function should be called after the CRYS_DES_Init function were called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying the pointers of the DataIn LLI table is not DX_NULL.
 *          - verifying that the pointer to the DataOut LLI table  is not DX_NULL, 
 *          - verifying the data_in size is not 0 and is a multiple of 16 bytes.
 *
 *        2.Decrypting the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *
 *        3.Executes the DES operation on the hardware by calling the 
 *          low level DES function LLF_SEP_DES_Block.
 *        4.Encrypting the information in the working context and storing it 
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *         
 *        5.Exit the handler with the OK code.  
 *      
 * @param[in] ContextID_ptr - a pointer to the DES context buffer allocated by the user that
 *                         is used for the DES machine operation. this should be the same context that was
 *                         used on the previous call of this session.
*
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 *
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 *
 * @param[in] DataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of buffer for output result data from DES. The table need to be 
 *                         aligned and placed in SEP SRAM.
 *
  * @param[in] OutLliTabEntries - The count of entries in the output LLI table.
 *
 * @param[in] DataInSize  - Size of input data block in bytes. 
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                         value MODULE_* CRYS_DES_error.h
 */
 CIMPORT_C CRYSError_t  CRYS_DES_SEP_Block( 
                                        CRYS_DESUserContext_t   *ContextID_ptr,   
                                        DxUint32_t              *DataInLliTab_ptr,     
                                        DxUint32_t               InLliTabEntries, 
                                        DxUint32_t              *DataOutLliTab_ptr,
                                        DxUint32_t               OutLliTabEntries,
                                        DxUint32_t               DataInSize );
                                        
                                        
/***********************************************************************************************************/
/**
 * @brief This function is used by SEP to operate  the DES machine in one integrated operation.
 *
 *        The actual macros that will be used by the users are:
 *      
 *
 * @param[in,out] IVCounter_ptr - this parameter is the buffer of the IV counter used on CBC mode.
 *                          On ECB mode this parameter has no use.
 *
 * @param[in] Key_ptr - A pointer to the users key buffer.
 *
 * @param[in] KeySize - The number of keys used by the DES as defined in the enum.
 *
 * @param[in] EncryptDecryptFlag - This flag determinate if the DES shall perform an Encrypt operation [0] 
 *                           or a Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 *                         used on the previous call of this session.
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 *
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 *
 * @param[in] DataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of buffer for output result data from DES. The table need to be 
 *                         aligned and placed in SEP SRAM.
 *
 * @param[in] OutLliTabEntries - The count of entries in the output LLI table.
 *
 * @param[in] DataInSize  - Size of input data block in bytes. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 */
 CIMPORT_C CRYSError_t  CRYS_DES_SEP(
				                CRYS_DES_Iv_t              IV_ptr,
				                CRYS_DES_Key_t             *Key_ptr,
				                CRYS_DES_NumOfKeys_t       NumOfKeys,
				                CRYS_DES_EncryptMode_t     EncryptDecryptFlag,
				                CRYS_DES_OperationMode_t   OperationMode,             
                                DxUint32_t                *DataInLliTab_ptr,     
                                DxUint32_t                 InLliTabEntries, 
                                DxUint32_t                *DataOutLliTab_ptr,
                                DxUint32_t                 OutLliTabEntries,     
		                        DxUint32_t                 DataInSize );                                        


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
 CIMPORT_C CRYSError_t  CRYS_DES_Block_ExtApp( CRYS_DESUserContext_t       *ContextID_ptr, /* in */ 
	 DxUint8_t                     *DataIn_ptr,    /* in */ 
	 DxUint32_t                    DataInSize,     /* in */ 
	 DxUint8_t                     *DataOut_ptr ); /* in/out */ 




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
 CIMPORT_C CRYSError_t  CRYS_DES_ExtApp(
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
