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
 
 
#ifndef CRYS_RND_ERROR_H
#define CRYS_RND_ERROR_H

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
   *  Object % CRYS_RND_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module contains the definitions of the CRYS AES errors.
   *
   *  \version CRYS_RND_error.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/
/* RND module on the CRYS layer base address - 0x00F00C00 */
#define CRYS_RND_DATA_OUT_POINTER_INVALID_ERROR        (CRYS_RND_MODULE_ERROR_BASE + 0x0UL)
#define CRYS_RND_CAN_NOT_GENERATE_RAND_IN_RANGE        (CRYS_RND_MODULE_ERROR_BASE + 0x1UL)
#define CRYS_RND_CPRNG_TEST_FAIL_ERROR			       (CRYS_RND_MODULE_ERROR_BASE + 0x2UL)
#define CRYS_RND_ADDITIONAL_INPUT_BUFFER_NULL		   (CRYS_RND_MODULE_ERROR_BASE + 0x3UL)
#define CRYS_RND_ADDITIONAL_INPUT_SIZE_ERROR		   (CRYS_RND_MODULE_ERROR_BASE + 0x4UL)
#define CRYS_RND_DATA_SIZE_OVERFLOW_ERROR			   (CRYS_RND_MODULE_ERROR_BASE + 0x5UL)
#define CRYS_RND_VECTOR_SIZE_ERROR			           (CRYS_RND_MODULE_ERROR_BASE + 0x6UL)
#define CRYS_RND_RESEED_COUNTER_OVERFLOW_ERROR		   (CRYS_RND_MODULE_ERROR_BASE + 0x7UL)
#define CRYS_RND_INSTANTIATION_NOT_DONE_ERROR		   (CRYS_RND_MODULE_ERROR_BASE + 0x8UL)
#define CRYS_RND_TRNG_LOSS_SAMPLES_ERROR               (CRYS_RND_MODULE_ERROR_BASE + 0x9UL)
#define CRYS_RND_TRNG_TIME_EXCEED_ERROR         	   (CRYS_RND_MODULE_ERROR_BASE + 0xAUL)
#define CRYS_RND_TRNG_LOSS_SAMPLES_AND_TIME_EXCEED_ERROR (CRYS_RND_MODULE_ERROR_BASE + 0xBUL)
#define CRYS_RND_IS_KAT_MODE_ERROR                     (CRYS_RND_MODULE_ERROR_BASE + 0xCUL)
#define CRYS_RND_ESTIMATOR_BUFFER_PTR_INVALID_ERROR    (CRYS_RND_MODULE_ERROR_BASE + 0xDUL)

#define CRYS_RND_IS_NOT_SUPPORTED                      (CRYS_RND_MODULE_ERROR_BASE + 0xFUL)

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif


