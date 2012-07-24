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
 
 

/********************************************************/

#ifndef CRYS_C2_SEP_H
#define CRYS_C2_SEP_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_C2.h"
#include "LLF_C2_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif
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
 *          - verifying the pointer of the data_in table is not DX_NULL.
 *          - verifying that the pointer to the data_out tableis not DX_NULL. 
 *          - verifying the values of the data_in buffers size is not 0 and a multiple of 8 bytes.
 *
 *        2.Decrypting the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *        3.executing the C2 Cipher operation on the SW or HW by calling the 
 *          low level function LLF_C2_Sep_CipherBlock.
 *        4.Encrypting the information in the working context and storing it 
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exit the handler with the OK code.  
 *      
 *
 * @param[in] ContextID_ptr - A pointer to the C2 Cipher context buffer allocated by the user that
 *                            is used for the C2 operation. This should be the same context that was
 *                            used on the previous call of this session.
 * @param[in] DataInLliTab_ptr -         The pointer to MLLI input table .
 *
 * @param[in] InLliTabSize -                The size of the MLLI input table.
 *
 * @param[in] DataOutLliTab_ptr -      The pointer to MLLI output table .
 *
 * @param[in] InLliTabSize -               The size of the MLLI output table.
 *
 * @param[in] DataOutLliTab_ptr -     The size of input data.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */

  CEXPORT_C CRYSError_t  CRYS_C2_Sep_CipherBlock(CRYS_C2CipherUserContext_t       *ContextID_ptr,   
                                           DxUint32_t              *DataInLliTab_ptr,     
                                           DxUint32_t               InLliTabEntries, 
                                           DxUint32_t              *DataOutLliTab_ptr,
                                           DxUint32_t               OutLliTabEntries,
                                           DxUint32_t               DataInSize );
#endif  /* End of #ifndef CRYS_AES_SEP_CF_H */

