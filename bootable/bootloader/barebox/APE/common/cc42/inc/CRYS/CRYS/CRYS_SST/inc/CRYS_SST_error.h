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
 
 #ifndef CRYS_SST_ERROR_H
#define CRYS_SST_ERROR_H

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
   *  Object % CRYS_SST_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module containes the definitions of the CRYS AES errors.
   *
   *  \version CRYS_SST_error.h#1:incl:1
   *  \author avis
   */




/************************ Defines ******************************/

	/* The CRYS SST module errors */
#define CRYS_SST_MODULE_WRONG_AES_KEY_TYPE					(CRYS_SST_MODULE_ERROR_BASE+0x1UL)
#define CRYS_SST_MODULE_WRONG_DES_KEY_TYPE					(CRYS_SST_MODULE_ERROR_BASE+0x2UL)
#define CRYS_SST_MODULE_WRONG_HMAC_KEY_SIZE					(CRYS_SST_MODULE_ERROR_BASE+0x3UL)
#define CRYS_SST_READ_AES_KEY_FROM_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x4UL)
#define CRYS_SST_READ_DES_KEY_FROM_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x5UL)
#define CRYS_SST_READ_HMAC_KEY_FROM_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x6UL)
#define CRYS_SST_READ_RSA_KEY_FROM_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x7UL)

#define CRYS_SST_INSEART_AES_KEY_TO_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x10UL)
#define CRYS_SST_INSEART_DES_KEY_TO_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x11UL)
#define CRYS_SST_INSEART_HMAC_KEY_TO_SST_ERR				(CRYS_SST_MODULE_ERROR_BASE+0x12UL)
#define CRYS_SST_INSEART_RSA_KEY_TO_SST_ERR					(CRYS_SST_MODULE_ERROR_BASE+0x13UL)
#define CRYS_SST_INVALID_INTERNAL_BUFFER_POINTER			(CRYS_SST_MODULE_ERROR_BASE+0x14UL)
#define CRYS_SST_KEY_NOT_FOUND_ERR							(CRYS_SST_MODULE_ERROR_BASE+0x15UL)
#define CRYS_SST_MODULE_WRONG_RSA_KEY_SIZE					(CRYS_SST_MODULE_ERROR_BASE+0x16UL)		

#define CRYS_SST_IS_NOT_SUPPORTED							(CRYS_SST_MODULE_ERROR_BASE+0xFFUL)


/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


