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
 
 
#ifndef CRYS_AESGCM_ERROR_H
#define CRYS_AESGCM_ERROR_H

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
   *  Object % CRYS_AESGCM_error.h    : %
   *  State           :  %state%
   *  Creation date   :  21 March 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module contains the definitions of the CRYS AESGCM errors.
   *
   *  \version CRYS_AESGCM_error.h#1:incl:1
   *  \author R.Levin
   */

/************************ Defines ******************************/

/* The CRYS AESGCM module errors. 
   CRYS_AESGCM_MODULE_ERROR_BASE = 0x00F01800 */
#define CRYS_AESGCM_INVALID_USER_CONTEXT_POINTER_ERROR     (CRYS_AESGCM_MODULE_ERROR_BASE + 0x00UL)
#define CRYS_AESGCM_ILLEGAL_KEY_SIZE_ERROR                 (CRYS_AESGCM_MODULE_ERROR_BASE + 0x01UL)
#define CRYS_AESGCM_INVALID_KEY_POINTER_ERROR              (CRYS_AESGCM_MODULE_ERROR_BASE + 0x02UL)
#define CRYS_AESGCM_INVALID_ENCRYPT_MODE_ERROR             (CRYS_AESGCM_MODULE_ERROR_BASE + 0x03UL)
#define CRYS_AESGCM_USER_CONTEXT_CORRUPTED_ERROR           (CRYS_AESGCM_MODULE_ERROR_BASE + 0x04UL) 
#define CRYS_AESGCM_DATA_IN_POINTER_INVALID_ERROR          (CRYS_AESGCM_MODULE_ERROR_BASE + 0x05UL)
#define CRYS_AESGCM_DATA_OUT_POINTER_INVALID_ERROR         (CRYS_AESGCM_MODULE_ERROR_BASE + 0x06UL)
#define CRYS_AESGCM_DATA_IN_SIZE_ILLEGAL                   (CRYS_AESGCM_MODULE_ERROR_BASE + 0x07UL)
#define CRYS_AESGCM_DATA_OUT_DATA_IN_OVERLAP_ERROR         (CRYS_AESGCM_MODULE_ERROR_BASE + 0x08UL)
#define CRYS_AESGCM_DATA_OUT_SIZE_INVALID_ERROR            (CRYS_AESGCM_MODULE_ERROR_BASE + 0x09UL)
#define CRYS_AESGCM_ADDITIONAL_BLOCK_NOT_PERMITTED_ERROR   (CRYS_AESGCM_MODULE_ERROR_BASE + 0x0AUL)
#define CRYS_AESGCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR          (CRYS_AESGCM_MODULE_ERROR_BASE + 0x0BUL)
#define CRYS_AESGCM_ILLEGAL_IV_SIZE_ERROR				   (CRYS_AESGCM_MODULE_ERROR_BASE + 0x0CUL)
#define CRYS_AESGCM_ILLEGAL_IV_PTR_ERROR				   (CRYS_AESGCM_MODULE_ERROR_BASE + 0x0DUL)
#define CRYS_AESGCM_TAG_PTR_INVALID_ERROR				   (CRYS_AESGCM_MODULE_ERROR_BASE + 0x0EUL)
#define CRYS_AESGCM_TAG_SIZE_INVALID_ERROR                 (CRYS_AESGCM_MODULE_ERROR_BASE + 0x0FUL)
#define CRYS_AESGCM_ILLEGAL_PARAMETER_ERROR                (CRYS_AESGCM_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_AESGCM_NOT_ALL_DATA_WAS_PROCESSED_ERROR       (CRYS_AESGCM_MODULE_ERROR_BASE + 0x11UL)
#define CRYS_AESGCM_TOTAL_DATA_SIZE_EXCEED_ERROR		   (CRYS_AESGCM_MODULE_ERROR_BASE + 0x12UL)
#define CRYS_AESGCM_AUTENTICATION_FAIL_ERROR		       (CRYS_AESGCM_MODULE_ERROR_BASE + 0x13UL)
#define CRYS_AESGCM_DATA_POINTERS_INVALID_ERROR            (CRYS_AESGCM_MODULE_ERROR_BASE + 0x14UL)
#define CRYS_AESGCM_DATA_SIZES_INVALID_ERROR               (CRYS_AESGCM_MODULE_ERROR_BASE + 0x15UL)



#define CRYS_AESGCM_IS_NOT_SUPPORTED                       (CRYS_AESGCM_MODULE_ERROR_BASE + 0xFFUL)

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/

#ifdef __cplusplus
}
#endif

#endif


