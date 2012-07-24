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
 
 
#ifndef CRYS_CCM_ERROR_H
#define CRYS_CCM_ERROR_H

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
   *  Object %CRYS_CCM_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:42 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes the error definitions of the CCM module on the CRYS layer.
   *
   *  \version CRYS_CCM_error.h#1:incl:1
   *  \author adams
   */


/************************ Defines ******************************/

#define CRYS_CCM_CONTEXT_TYPE_ERROR                  (CRYS_CCM_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_CCM_INIT_PKI_SEM_CREATION_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_CCM_INIT_HASH_SEM_CREATION_FAILURE      (CRYS_CCM_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_CCM_INIT_DES_SEM_CREATION_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_CCM_INIT_AES_SEM_CREATION_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_CCM_INIT_CCM_SEM_CREATION_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_CCM_INIT_RSA_SEM_CREATION_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_CCM_INIT_HMAC_SEM_CREATION_FAILURE      (CRYS_CCM_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_CCM_INIT_RC4_SEM_CREATION_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_CCM_INIT_ECDSA_SEM_CREATION_FAILURE     (CRYS_CCM_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_CCM_INIT_C2_CIPHER_SEM_CREATION_FAILURE (CRYS_CCM_MODULE_ERROR_BASE + 0xBUL)
#define CRYS_CCM_INIT_C2_HASH_SEM_CREATION_FAILURE   (CRYS_CCM_MODULE_ERROR_BASE + 0xCUL)
#define CRYS_CCM_INIT_OTF_SEM_CREATION_FAILURE	     (CRYS_CCM_MODULE_ERROR_BASE + 0xDUL)
#define CRYS_CCM_INIT_AESCCM_SEM_CREATION_FAILURE    (CRYS_CCM_MODULE_ERROR_BASE + 0xEUL)
#define CRYS_CCM_INIT_AESGCM_SEM_CREATION_FAILURE    (CRYS_CCM_MODULE_ERROR_BASE + 0xFUL)

#define CRYS_CCM_TERMINATE_PKI_SEM_DELETE_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x11UL)
#define CRYS_CCM_TERMINATE_HASH_SEM_DELETE_FAILURE      (CRYS_CCM_MODULE_ERROR_BASE + 0x12UL)
#define CRYS_CCM_TERMINATE_DES_SEM_DELETE_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x14UL)
#define CRYS_CCM_TERMINATE_AES_SEM_DELETE_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x15UL)
#define CRYS_CCM_TERMINATE_CCM_SEM_DELETE_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x16UL)
#define CRYS_CCM_TERMINATE_RSA_SEM_DELETE_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x17UL)
#define CRYS_CCM_TERMINATE_HMAC_SEM_DELETE_FAILURE      (CRYS_CCM_MODULE_ERROR_BASE + 0x18UL)
#define CRYS_CCM_TERMINATE_RC4_SEM_DELETE_FAILURE       (CRYS_CCM_MODULE_ERROR_BASE + 0x19UL)
#define CRYS_CCM_TERMINATE_ECDSA_SEM_DELETE_FAILURE     (CRYS_CCM_MODULE_ERROR_BASE + 0x1AUL)
#define CRYS_CCM_TERMINATE_C2_CIPHER_SEM_DELETE_FAILURE (CRYS_CCM_MODULE_ERROR_BASE + 0x1BUL)
#define CRYS_CCM_TERMINATE_C2_HASH_SEM_DELETE_FAILURE   (CRYS_CCM_MODULE_ERROR_BASE + 0x1CUL)
#define CRYS_CCM_TERMINATE_OTF_SEM_DELETE_FAILURE		(CRYS_CCM_MODULE_ERROR_BASE + 0x1DUL)
#define CRYS_CCM_TERMINATE_AESCCM_SEM_DELETE_FAILURE    (CRYS_CCM_MODULE_ERROR_BASE + 0x1EUL)
#define CRYS_CCM_TERMINATE_AESGCM_SEM_DELETE_FAILURE    (CRYS_CCM_MODULE_ERROR_BASE + 0x1FUL)

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/


#ifdef __cplusplus
}
#endif

#endif



