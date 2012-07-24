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

#ifndef CRYS_AES_XTS_TW_INC_H
#define CRYS_AES_XTS_TW_INC_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "CRYS_AES.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_AES.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:42:30 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AES APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AES.h#1:incl:13
   *  \author adams
   */

/************************ Defines ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/** 
 * @brief This function is used to operate a block of data on the SW or on AES machine in XTS Tweak Increment per sector block mode.
 *        This function should be called after the appropriate CRYS AES init function 
 *        (according to used AES operation mode).
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validity of all inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying the pointer of the data_in buffer is not DX_NULL.
 *              verifying that the pointer to the data_out buffer is not DX_NULL, 
 *              verifying that the DataInSize is not 0 and is a size of sector.
 *
 *        2.Decrypts the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *        3.Executes the AES XTS operation on the software or hardware by calling the 
 *          low level AES function LLF_AES_Block.
 *        4.Encrypts the working context and stores it to the users received context. 
 *          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exits the handler with the OK code.  
 *      
 *
 * @param[in] ContextID_ptr - A pointer to the AES context buffer allocated by the user that
 *                            is used for the AES machine operation. This should be the same context that was
 *                            used on the previous call of this session.
 *
 * @param[in] DataIn_ptr - A pointer to the buffer of the input data to the AES. The pointer does 
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment). 
 *
 * @param[in] DataInSize - A size of the input data must be multiply of 16 bytes and all block have to be with identical DataInSize value .
 *                                              Note last chunk (block) of data may be processed by 
 *                                              CRYS_AES_Finish function  and 
 *                                              last block size  has to be the same or less than DataInSize in
 *                                              CRYS_AES_Block operation;    
 *
 * @param[out] DataOut_ptr - A pointer to the buffer of the output data from the AES. The pointer  does not 
 *                             need to be aligned. On CSI output mode the pointer must be equal to
 *                             value (0xFFFFFFFC | DataOutAlignment). 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_AES_error.h
 *
 *     NOTES: 1. Temporarily not allowed, that both the Input and the Output simultaneously 
 *               were on CSI mode.
 *            2. Temporarily the CSI input or output are not allowed .    
 */
CIMPORT_C  CRYSError_t  CRYS_AES_XTS_TwInc_Block(CRYS_AESUserContext_t   *ContextID_ptr,   
                                                 DxUint8_t               *DataIn_ptr,     
                                                 DxUint32_t               DataInSize,     
                                                 DxUint8_t               *DataOut_ptr );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_AES_XTS_TW_INC_H */ 


/**************************************************/




