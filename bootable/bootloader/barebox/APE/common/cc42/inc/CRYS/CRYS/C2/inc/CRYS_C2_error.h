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
 
 #ifndef CRYS_C2_ERROR_H
#define CRYS_C2_ERROR_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "CRYS_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_C2_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Feb. 19 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module contains the definitions of the CRYS C2 errors.
   *
   *  \version CRYS_C2_error.h#1:incl:1
   *  \author R. Levin (using as templet the DES files of adams).
   */




/************************ Defines ******************************/
/* C2 module on the CRYS layer base address - 0x00F01000 */

/* The CRYS C2 Cipher module errors */
#define CRYS_C2_CIPHER_INVALID_USER_CONTEXT_POINTER_ERROR   (CRYS_C2_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_C2_CIPHER_ILLEGAL_OPERATION_MODE_ERROR         (CRYS_C2_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_C2_CIPHER_INVALID_KEY_POINTER_ERROR            (CRYS_C2_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_C2_CIPHER_INVALID_ENCRYPT_MODE_ERROR           (CRYS_C2_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_C2_CIPHER_USER_CONTEXT_CORRUPTED_ERROR         (CRYS_C2_MODULE_ERROR_BASE + 0x4UL) 
#define CRYS_C2_CIPHER_DATA_IN_POINTER_INVALID_ERROR        (CRYS_C2_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_C2_CIPHER_DATA_OUT_POINTER_INVALID_ERROR       (CRYS_C2_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_C2_CIPHER_DATA_SIZE_ILLEGAL                    (CRYS_C2_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_C2_CIPHER_DATA_OUT_DATA_IN_OVERLAP_ERROR       (CRYS_C2_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_C2_INVALID_SEC_CONSTANT_POINTER_ERROR          (CRYS_C2_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_C2_DATA_IN_LLI_TAB_SIZE_INVALID_ERROR          (CRYS_C2_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_C2_DATA_OUT_LLI_TAB_SIZE_INVALID_ERROR         (CRYS_C2_MODULE_ERROR_BASE + 0xBUL)

/* The CRYS C2 HASH module errors */
#define CRYS_C2_HASH_INVALID_USER_CONTEXT_POINTER_ERROR     (CRYS_C2_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_C2_HASH_USER_CONTEXT_CORRUPTED_ERROR           (CRYS_C2_MODULE_ERROR_BASE + 0x11UL) 
#define CRYS_C2_HASH_DATA_IN_POINTER_INVALID_ERROR          (CRYS_C2_MODULE_ERROR_BASE + 0x12UL)
#define CRYS_C2_HASH_DATA_SIZE_ILLEGAL                      (CRYS_C2_MODULE_ERROR_BASE + 0x13UL)
#define CRYS_C2_HASH_INVALID_RESULT_BUFFER_POINTER_ERROR    (CRYS_C2_MODULE_ERROR_BASE + 0x14UL)

/* The CRYS C2 One Way function errors */
#define CRYS_C2_ONE_WAY_FUNC_DATA1_POINTER_INVALID_ERROR    (CRYS_C2_MODULE_ERROR_BASE + 0x20UL)
#define CRYS_C2_ONE_WAY_FUNC_DATA2_POINTER_INVALID_ERROR    (CRYS_C2_MODULE_ERROR_BASE + 0x21UL)
#define CRYS_C2_ONE_WAY_FUNC_DATA_OUT_POINTER_INVALID_ERROR (CRYS_C2_MODULE_ERROR_BASE + 0x22UL)
#define CRYS_C2_ONE_WAY_FUNC_IN_OUT_OVERLAP_ERROR           (CRYS_C2_MODULE_ERROR_BASE + 0x23UL)

/* The CRYS_C2_PrepareSecretConstant function errors */
#define CRYS_C2_PREPARE_SECRET_CONSTANT_POINTER_INVALID_ERROR  (CRYS_C2_MODULE_ERROR_BASE + 0x28UL)
#define CRYS_C2_CIPHER_INVALID_ARGUMENT_ERROR               (CRYS_C2_MODULE_ERROR_BASE + 0x30UL)
#define CRYS_C2_CIPHER_SEC_CONST_LOAD_ERROR                 (CRYS_C2_MODULE_ERROR_BASE + 0x31UL)


#define CRYS_C2_CIPHER_INVALID_ARGUMENT_ERROR               (CRYS_C2_MODULE_ERROR_BASE + 0x30UL)
#define CRYS_C2_CIPHER_SEC_CONST_LOAD_ERROR                 (CRYS_C2_MODULE_ERROR_BASE + 0x31UL)

/*************/
#define CRYS_C2_IS_NOT_SUPPORTED                            (CRYS_C2_MODULE_ERROR_BASE + 0xFFUL)

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


