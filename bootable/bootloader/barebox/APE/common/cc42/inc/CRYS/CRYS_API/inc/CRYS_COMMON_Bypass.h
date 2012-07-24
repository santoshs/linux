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
 
 #ifndef CRYS_COMMON_BYPASS_H
#define CRYS_COMMON_BYPASS_H
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:25:55 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_COMMON_Bypass.h#1:csrc:6
   *  \author adams
   */



/************* Include Files ****************/

/* .............. CRYS level includes ................. */

//#include "DX_VOS_Mem.h"
#include "CRYS_Defs.h"
#include "CRYS_COMMON.h"
#include "CRYS_error.h"
#include "CRYS_COMMON_error.h"

/* .............. LLF level includes .................. */

#include "LLF_COMMON.h"
/************************ Defines ******************************/



/************************ MACROS ******************************/


/************************ Global Data ******************************/

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/** ------------------------------------------------------------
 * @brief This function is used to operate bypass action.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *          The major checkers that are run over the received parameters:
 *          - verifying the pointer of the data_in buffer is not DX_NULL.
 *          - verifying that the pointer to the data_out buffer is not DX_NULL. 
 *          - verifying the values of the data_in buffers size is not 0.
 *
 *
 *        2.executing bypass operation on the hardware.
 *        3.Exit the handler with the OK code.  
 *      
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data. The pointer does 
 *                   not need to be aligned.
 *
 * @param[in] DataInSize - The size of the input data.
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data . The pointer does not 
 *                        need to be aligned to 32 bits.  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_AES_error.h
 */
 CIMPORT_C CRYSError_t  CRYS_COMMON_Bypass( DxUint8_t       *DataIn_ptr,     
                              DxUint32_t                    DataInSize,     
                              DxUint8_t                     *DataOut_ptr );

                         


#endif /*CRYS_COMMON_BYPASS_H*/

