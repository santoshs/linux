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
 
 
#ifndef LLF_DES_ERROR_H
#define LLF_DES_ERROR_H

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
   *  Object % LLF_AES_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief The error definitions of the LLF AES module
   *
   *  \version CRYS_AES_error.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

#define LLF_DES_HW_VERSION_NOT_CORRECT_ERROR   (LLF_DES_MODULE_ERROR_BASE + 0x0UL)

#define LLF_DES_HW_INTERNAL_ERROR_1            (LLF_DES_MODULE_ERROR_BASE + 0x11UL)
#define LLF_DES_HW_INTERNAL_ERROR_2            (LLF_DES_MODULE_ERROR_BASE + 0x12UL)
#define LLF_DES_HW_INTERNAL_ERROR_3            (LLF_DES_MODULE_ERROR_BASE + 0x13UL)
#define LLF_DES_HW_INTERNAL_ERROR_4            (LLF_DES_MODULE_ERROR_BASE + 0x14UL)
#define LLF_DES_HW_INTERNAL_ERROR_5            (LLF_DES_MODULE_ERROR_BASE + 0x15UL)
#define LLF_DES_HW_INTERNAL_ERROR_6            (LLF_DES_MODULE_ERROR_BASE + 0x16UL)

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif

