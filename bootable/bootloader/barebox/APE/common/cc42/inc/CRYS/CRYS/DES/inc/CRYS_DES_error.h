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
 
 #ifndef CRYS_DES_ERROR_H
#define CRYS_DES_ERROR_H

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
   *  Object % CRYS_DES_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module containes the definitions of the CRYS AES errors.
   *
   *  \version CRYS_DES_error.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

/* The CRYS DES module errors */
#define CRYS_DES_INVALID_USER_CONTEXT_POINTER_ERROR     (CRYS_DES_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_DES_INVALID_IV_PTR_ON_NON_ECB_MODE_ERROR   (CRYS_DES_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_DES_ILLEGAL_OPERATION_MODE_ERROR           (CRYS_DES_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_DES_ILLEGAL_NUM_OF_KEYS_ERROR              (CRYS_DES_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_DES_INVALID_KEY_POINTER_ERROR              (CRYS_DES_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_DES_INVALID_ENCRYPT_MODE_ERROR             (CRYS_DES_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_DES_USER_CONTEXT_CORRUPTED_ERROR           (CRYS_DES_MODULE_ERROR_BASE + 0x6UL) 
#define CRYS_DES_DATA_IN_POINTER_INVALID_ERROR          (CRYS_DES_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_DES_DATA_OUT_POINTER_INVALID_ERROR         (CRYS_DES_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_DES_DATA_SIZE_ILLEGAL                      (CRYS_DES_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_DES_DATA_OUT_DATA_IN_OVERLAP_ERROR         (CRYS_DES_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_DES_DATA_IN_LLI_TAB_POINTER_INVALID_ERROR  (CRYS_DES_MODULE_ERROR_BASE + 0xBUL)
#define CRYS_DES_DATA_IN_LLI_TAB_SIZE_INVALID_ERROR     (CRYS_DES_MODULE_ERROR_BASE + 0xCUL)
#define CRYS_DES_DATA_OUT_LLI_TAB_POINTER_INVALID_ERROR (CRYS_DES_MODULE_ERROR_BASE + 0xDUL)
#define CRYS_DES_DATA_OUT_LLI_TAB_SIZE_INVALID_ERROR    (CRYS_DES_MODULE_ERROR_BASE + 0xEUL)
#define CRYS_DES_IN_OUT_CSI_TO_CSI_IS_NOT_ALLOWED_ERROR (CRYS_DES_MODULE_ERROR_BASE + 0xFUL)
#define CRYS_DES_FIPS_MODE_FAIL_ERROR										(CRYS_DES_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_DES_FIPS_MODE_SF_NOT_PERFORMED_ERROR				(CRYS_DES_MODULE_ERROR_BASE + 0x11UL)
#define CRYS_DES_FIPS_MODE_NOT_ALLOWED_ERROR						(CRYS_DES_MODULE_ERROR_BASE + 0x12UL)


#define CRYS_DES_IS_NOT_SUPPORTED                       (CRYS_DES_MODULE_ERROR_BASE + 0x1FUL)

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


