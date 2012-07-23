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
 
 
#ifndef CRYS_AESCCM_ERROR_H
#define CRYS_AESCCM_ERROR_H

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
   *  Object % CRYS_AESCCM_error.h    : %
   *  State           :  %state%
   *  Creation date   :  25 Sept.2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module contains the definitions of the CRYS AESCCM errors.
   *
   *  \version CRYS_AESCCM_error.h#1:incl:1
   *  \author R.Levin
   */

/************************ Defines ******************************/

/* The CRYS AESCCM module errors. 
   CRYS_AESCCM_MODULE_ERROR_BASE = 0x00F01500 */
#define CRYS_AESCCM_INVALID_USER_CONTEXT_POINTER_ERROR     (CRYS_AESCCM_MODULE_ERROR_BASE + 0x00UL)
#define CRYS_AESCCM_ILLEGAL_KEY_SIZE_ERROR                 (CRYS_AESCCM_MODULE_ERROR_BASE + 0x01UL)
#define CRYS_AESCCM_INVALID_KEY_POINTER_ERROR              (CRYS_AESCCM_MODULE_ERROR_BASE + 0x02UL)
#define CRYS_AESCCM_INVALID_ENCRYPT_MODE_ERROR             (CRYS_AESCCM_MODULE_ERROR_BASE + 0x03UL)
#define CRYS_AESCCM_USER_CONTEXT_CORRUPTED_ERROR           (CRYS_AESCCM_MODULE_ERROR_BASE + 0x04UL) 
#define CRYS_AESCCM_DATA_IN_POINTER_INVALID_ERROR          (CRYS_AESCCM_MODULE_ERROR_BASE + 0x05UL)
#define CRYS_AESCCM_DATA_OUT_POINTER_INVALID_ERROR         (CRYS_AESCCM_MODULE_ERROR_BASE + 0x06UL)
#define CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL                   (CRYS_AESCCM_MODULE_ERROR_BASE + 0x07UL)
#define CRYS_AESCCM_DATA_OUT_DATA_IN_OVERLAP_ERROR         (CRYS_AESCCM_MODULE_ERROR_BASE + 0x08UL)
#define CRYS_AESCCM_DATA_OUT_SIZE_INVALID_ERROR            (CRYS_AESCCM_MODULE_ERROR_BASE + 0x09UL)
#define CRYS_AESCCM_ADDITIONAL_BLOCK_NOT_PERMITTED_ERROR   (CRYS_AESCCM_MODULE_ERROR_BASE + 0x0AUL)
#define CRYS_AESCCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR          (CRYS_AESCCM_MODULE_ERROR_BASE + 0x0BUL)
#define CRYS_AESCCM_ILLEGAL_PARAMETER_SIZE_ERROR           (CRYS_AESCCM_MODULE_ERROR_BASE + 0x0CUL)
#define CRYS_AESCCM_ILLEGAL_PARAMETER_PTR_ERROR            (CRYS_AESCCM_MODULE_ERROR_BASE + 0x0DUL)
#define CRYS_AESCCM_ILLEGAL_DATA_TYPE_ERROR                (CRYS_AESCCM_MODULE_ERROR_BASE + 0x0EUL)
#define CRYS_AESCCM_CCM_MAC_INVALID_ERROR                  (CRYS_AESCCM_MODULE_ERROR_BASE + 0x0FUL)
#define CRYS_AESCCM_LAST_BLOCK_NOT_PERMITTED_ERROR         (CRYS_AESCCM_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_AESCCM_ILLEGAL_PARAMETER_ERROR                (CRYS_AESCCM_MODULE_ERROR_BASE + 0x11UL)
#define CRYS_AESCCM_ILLEGAL_SECRET_KEY_ERROR               (CRYS_AESCCM_MODULE_ERROR_BASE + 0x12UL)
#define CRYS_AESCCM_NOT_ALL_ADATA_WAS_PROCESSED_ERROR      (CRYS_AESCCM_MODULE_ERROR_BASE + 0x13UL)
#define CRYS_AESCCM_NOT_ALL_DATA_WAS_PROCESSED_ERROR       (CRYS_AESCCM_MODULE_ERROR_BASE + 0x14UL)


/* The CRYS_AES_SEP additional errors */
#define CRYS_AESCCM_DATA_IN_LLI_TAB_POINTER_INVALID_ERROR  (CRYS_AESCCM_MODULE_ERROR_BASE + 0x20UL)
#define CRYS_AESCCM_DATA_IN_LLI_TAB_SIZE_INVALID_ERROR     (CRYS_AESCCM_MODULE_ERROR_BASE + 0x21UL)
#define CRYS_AESCCM_DATA_OUT_LLI_TAB_POINTER_INVALID_ERROR (CRYS_AESCCM_MODULE_ERROR_BASE + 0x22UL)
#define CRYS_AESCCM_DATA_OUT_LLI_TAB_SIZE_INVALID_ERROR    (CRYS_AESCCM_MODULE_ERROR_BASE + 0x23UL)

/* The CRYS_AES_CF errors */
#define CRYS_AESCCM_CF_ILLEGAL_PAGE_SIZE_ERROR             (CRYS_AESCCM_MODULE_ERROR_BASE + 0x24UL)
#define CRYS_AESCCM_CF_ILLEGAL_SKIP_FIRST_CNT_ERROR        (CRYS_AESCCM_MODULE_ERROR_BASE + 0x25UL)
#define CRYS_AESCCM_CF_ILLEGAL_SKIP_CNT_ERROR      	       (CRYS_AESCCM_MODULE_ERROR_BASE + 0x26UL)
#define CRYS_AESCCM_CF_ILLEGAL_SKIP_FIRST_AND_CNT_ERROR    (CRYS_AESCCM_MODULE_ERROR_BASE + 0x27UL)
#define CRYS_AESCCM_CF_ILLEGAL_SKIP_OPER_MODE_ERROR        (CRYS_AESCCM_MODULE_ERROR_BASE + 0x28UL)


#define CRYS_AESCCM_IS_NOT_SUPPORTED                       (CRYS_AESCCM_MODULE_ERROR_BASE + 0xFFUL)

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/

#ifdef __cplusplus
}
#endif

#endif


