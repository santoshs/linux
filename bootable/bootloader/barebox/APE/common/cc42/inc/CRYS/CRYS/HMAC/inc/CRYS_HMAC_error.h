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
 
 #ifndef CRYS_HMAC_ERROR_H
#define CRYS_HMAC_ERROR_H

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
   *  Object % CRYS_HASH_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module containes the definitions of the CRYS AES errors.
   *
   *  \version CRYS_AES_error.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

/* The CRYS HASH module errors */
#define CRYS_HMAC_INVALID_USER_CONTEXT_POINTER_ERROR     (CRYS_HMAC_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_HMAC_ILLEGAL_OPERATION_MODE_ERROR           (CRYS_HMAC_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_HMAC_USER_CONTEXT_CORRUPTED_ERROR           (CRYS_HMAC_MODULE_ERROR_BASE + 0x2UL) 
#define CRYS_HMAC_DATA_IN_POINTER_INVALID_ERROR          (CRYS_HMAC_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_HMAC_DATA_SIZE_ILLEGAL                      (CRYS_HMAC_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_HMAC_INVALID_RESULT_BUFFER_POINTER_ERROR    (CRYS_HMAC_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_HMAC_INVALID_KEY_POINTER_ERROR              (CRYS_HMAC_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_HMAC_UNVALID_KEY_SIZE_ERROR                 (CRYS_HMAC_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_HMAC_FIPS_MODE_FAIL_ERROR									 (CRYS_HMAC_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_HMAC_FIPS_MODE_SF_NOT_PERFORMED_ERROR			 (CRYS_HMAC_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_HMAC_FIPS_MODE_NOT_ALLOWED_ERROR						 (CRYS_HMAC_MODULE_ERROR_BASE + 0xAUL)


#define CRYS_HMAC_IS_NOT_SUPPORTED                       (CRYS_HMAC_MODULE_ERROR_BASE + 0xFUL)



/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


