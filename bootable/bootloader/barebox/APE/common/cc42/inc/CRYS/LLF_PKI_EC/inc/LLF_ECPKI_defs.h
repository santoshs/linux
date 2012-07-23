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
 
 #ifndef LLF_ECPKI_defs_H
#define LLF_ECPKI_defs_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  18 Sept. 2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_ECPKI_defs.h#1:incl:1
   *  \author R.Levin
   */

/************************ Defines ******************************/

/* Define the maximal allowed width of the exponentiation sliding window 
in range 1...6 for ECC algorithms. This define is actual for projects on 
soft platform. To minimize code size use the minimum value. To optimize 
the performance choose the maximum value */
#define LLF_ECPKI_EXP_SLIDING_WINDOW_MAX_VALUE  LLF_PKI_EXP_SLIDING_WINDOW_MAX_VALUE


/* size of buffer for Barrett modulus tag NP, used in PKI algorithms */
#define LLF_ECPKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS  LLF_PKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS   


/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/


#ifdef __cplusplus
}
#endif

#endif
