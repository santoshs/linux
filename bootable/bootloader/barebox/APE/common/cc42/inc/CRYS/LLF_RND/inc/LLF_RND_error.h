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
 
#ifndef LLF_RND_ERROR_H
#define LLF_RND_ERROR_H

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
   *  Object % LLF_RND_error.h    : %
   *  State           :  %state%
   *  Creation date   :  ý28 Jan. ý2009 
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief The error definitions of the LLF AES module
   *
   *  \version LLF_RND_error.h#1:incl:1
   *  \author R.Levin
   *           All Rights reserved
   */




/************************ Defines ******************************/

#define LLF_RND_HW_VERSION_NOT_CORRECT_ERROR   	(LLF_RND_MODULE_ERROR_BASE + 0x0UL)
#define LLF_RND_CPRNG_TEST_FAIL_ERROR			(LLF_RND_MODULE_ERROR_BASE + 0x1UL)
#define LLF_RND_CTRNG_TEST_FAIL_ERROR			(LLF_RND_MODULE_ERROR_BASE + 0x2UL)
#define LLF_RND_STATE_PTR_INVALID_ERROR			(LLF_RND_MODULE_ERROR_BASE + 0x3UL)
#define LLF_RND_AES_256_NOT_SUPPORTED_ERROR     (LLF_RND_MODULE_ERROR_BASE + 0x4UL)

#define LLF_RND_TRNG_ERRORS_BASE            	(LLF_RND_MODULE_ERROR_BASE + 0x10UL)
#define LLF_RND_TRNG_AUTOCORR_TEST_FATAL_ERROR	(LLF_RND_MODULE_ERROR_BASE + 0x12UL)
#define LLF_RND_HW_RNG_CTRNG_TEST_ERROR			(LLF_RND_MODULE_ERROR_BASE + 0x14UL)
#define LLF_RND_TRNG_VN_CORRECTION_ERROR		(LLF_RND_MODULE_ERROR_BASE + 0x18UL)

#define LLF_RND_TRNG_SAMPLES_LOSS_ERROR	   	    (LLF_RND_MODULE_ERROR_BASE + 0x19UL)
#define LLF_RND_TRNG_TIME_LIMIT_EXCEEDED_ERROR  (LLF_RND_MODULE_ERROR_BASE + 0x20UL)
#define LLF_RND_TRNG_ENTR_ESTIM_SIZE_EXCEED_ERROR (LLF_RND_MODULE_ERROR_BASE + 0x21UL)


/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif

