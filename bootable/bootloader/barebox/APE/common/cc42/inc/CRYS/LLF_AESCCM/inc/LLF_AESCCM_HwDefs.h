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
 
 
#ifndef LLF_AESCCM_HWDEFS_H
#define LLF_AESCCM_HWDEFS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "cc_hw_defs.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % LLF_AES_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  Dec 17  2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains the hardware addresses of the registers relevant to the AES module.
   *
   *  \version LLF_AESCCM_HwDefs.h#1:incl:1
   *  \author R.Levin
   */




/************************ Defines ******************************/

/* define minimal size of multi-LLI entry in bytes */
#define LLF_AESCCM_MINIMAL_MLLI_ENTRY_SIZE_BYTES  8

/* the definitions of the specific AES hardware registers used in AESCCM. Addresses are 
   given as offset above the base address, defined by the user and the relevant bits */
   
/* specific AES registers */   

#define LLF_AES_HW_AES_CTR_0_ADDR_0              HW_AES_CTR0_ADDR_0_REG_ADDR      /*0x0460UL*/  
#define LLF_AES_HW_AES_CTR_0_ADDR_1              HW_AES_CTR0_ADDR_1_REG_ADDR      /*0x0464UL*/  
#define LLF_AES_HW_AES_CTR_0_ADDR_2              HW_AES_CTR0_ADDR_2_REG_ADDR      /*0x0468UL*/
#define LLF_AES_HW_AES_CTR_0_ADDR_3			   	 HW_AES_CTR0_ADDR_3_REG_ADDR      /*0x046CUL*/

#define LLF_AES_HW_AES_REMAINING_BYTES_ADDR      HW_AES_REMAINING_BYTES_REG_ADDR  /*0x04BCUL*/
/* AES control register fields for tunnel mode works and AESCCM */
#define AES_HW_AES_CTL_TUNNEL_IS_ON_VAL             (0x0001UL << 10 )
#define AES_HW_AES_CTL_TUN_B1_USES_PADDED_DATA_IN   (0x0001UL << 23 )  /* dataIn (padded) -> B1 */ 
#define AES_HW_AES_CTL_TUN_B0_ENCRYPT_VAL           (0x0001UL << 24 )  /* B0 do encrypt */
#define AES_HW_AES_CTL_OUT_MID_TUNNEL_DATA_VAL      (0x0001UL << 25 )  /* enable output data from B0 into DOUT */ 
#define AES_HW_AES_CTL_TUNNEL_B1_PAD_EN_VAL         (0x0001UL << 26 )  /* enable padding of B0 output (for input to B1 only) */
#define AES_HW_AES_CTL_PAD_DATA_IN_VAL              (0x0001UL << 27 )  /* enable padding of input data */
					    


/* ********************** Macros ******************************* */

/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif


