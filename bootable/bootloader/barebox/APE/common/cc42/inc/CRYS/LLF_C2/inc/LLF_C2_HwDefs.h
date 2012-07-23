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
 
 
#ifndef LLF_C2_HWDEFS_H
#define LLF_C2_HWDEFS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "cc_hw_defs.h"

#include "LLF_COMMON_HwDefs.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % LLF_AES_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:40:57 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes the hardware address of the registers relevant to the AES module.
   *
   *  \version LLF_AES_HwDefs.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

/* the definitions of the AES hardware registers offset above the base address 
   determaind by the user and the relevant bits */

#define LLF_C2_HW_CRYPTO_ADDR_SPACE ( 1UL << 12 )
   
/* spesific C2 registers */   
#define LLF_C2_HW_DIN_DOUT_ADDR                  HW_DIN_BUFFER_REG_ADDR 

#define LLF_HW_C2_CONTROL_REG_REG_ADDR           HW_C2_CONTROL_REG_REG_ADDR
#define LLF_HW_C2_KEY0_REG_ADDR                  HW_C2_KEY0_REG_ADDR
#define LLF_HW_C2_KEY1_REG_ADDR                  HW_C2_KEY1_REG_ADDR
#define LLF_HW_C2_SW_RESET_REG_ADDR              HW_C2_SW_RESET_REG_ADDR
#define LLF_HW_C2_BUSY_REG_ADDR                  HW_C2_BUSY_REG_ADDR
#define LLF_HW_C2_LOAD_SBOX_DATA_REG_ADDR        HW_C2_LOAD_SBOX_DATA_REG_ADDR
#define LLF_HW_C2_RST_KEY_AFTR_X_BLCKS_REG_ADDR  HW_C2_RST_KEY_AFTR_X_BLCKS_REG_ADDR
#define LLF_HW_C2_SUSPEND_RESUME_REG_ADDR        HW_C2_SUSPEND_RESUME_REG_ADDR
#define LLF_HW_C2_SUSPEND_RDATA_REG_ADDR         HW_C2_SUSPEND_RDATA_REG_ADDR
#define LLF_HW_C2_RESUME_WDATA_REG_ADDR          HW_C2_RESUME_WDATA_REG_ADDR
#define LLF_HW_C2_CLK_ENABLE_REG_ADDR            HW_C2_CLK_ENABLE_REG_ADDR

#define LLF_C2_HW_DOUT_READ_ALIGN_ADDR         HW_READ_ALIGN_REG_ADDR          /*0x0D3CUL*/
#define LLF_C2_HW_DIN_WRITE_ALIGN_ADDR        HW_WRITE_ALIGN_REG_ADDR         /*0x0C3CUL*/
#define LLF_C2_HW_READ_LAST_DATA_ADDR          HW_READ_ALIGN_LAST_REG_ADDR     /*0x0D44UL*/

#define LLF_C2_HW_DIN_MEM_DMA_BUSY_ADDR     	HW_DIN_MEM_DMA_BUSY_REG_ADDR    /*0x0C20UL*/
#define LLF_C2_HW_DOUT_MEM_DMA_BUSY_ADDR       HW_DOUT_MEM_DMA_BUSY_REG_ADDR   /*0x0D20UL*/

#define LLF_C2_HW_SRC_LLI_WORD0_ADDR           HW_SRC_LLI_WORD0_REG_ADDR       /*0x0C28UL*/
#define LLF_C2_HW_SRC_LLI_WORD1_ADDR           HW_SRC_LLI_WORD1_REG_ADDR 		/*0x0C2CUL*/

#define LLF_C2_HW_DST_LLI_WORD0_ADDR           HW_DST_LLI_WORD0_REG_ADDR 	    /*0x0D28UL*/
#define LLF_C2_HW_DST_LLI_WORD1_ADDR           HW_DST_LLI_WORD1_REG_ADDR 		/*0x0D2CUL*/

#define LLF_C2_HW_LLI_WORD1_FIRST_LLI_WORD_POS   LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS 
#define LLF_C2_HW_LLI_WORD1_LAST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS

#define LLF_C2_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS
#define LLF_C2_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS     LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS
#define LLF_C2_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS
#define LLF_C2_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS     LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS



#define LLF_C2_HW_DST_LLI_MEM_ADDR_REG_ADDR    HW_DST_LLI_SRAM_ADDR_REG_ADDR   /*0x0D24UL*/
#define LLF_C2_HW_DST_LLI_WORD0_ADDR           HW_DST_LLI_WORD0_REG_ADDR      /*0x0D28UL*/
#define LLF_C2_HW_DST_LLI_WORD1_ADDR           HW_DST_LLI_WORD1_REG_ADDR      /*0x0D2CUL*/

/* aligner registers */
#define LLF_C2_HW_DIN_WRITE_ALIGN_ADDR         HW_WRITE_ALIGN_REG_ADDR         /*0x0C3CUL*/
#define LLF_C2_HW_WRITE_ALIGN_LAST_REG_ADDR    HW_WRITE_ALIGN_LAST_REG_ADDR    /*0x0C4CUL*/ 
#define LLF_C2_HW_DOUT_READ_ALIGN_ADDR         HW_READ_ALIGN_REG_ADDR          /*0x0D3CUL*/
#define LLF_C2_HW_READ_LAST_DATA_ADDR          HW_READ_ALIGN_LAST_REG_ADDR     /*0x0D44UL*/

#define LLF_C2_HW_FIFO_IN_EMPTY_REG_ADDR       HW_FIFO_IN_EMPTY_REG_ADDR       /*0x0C50UL*/ 
#define LLF_C2_HW_FIFO_OUT_EMPTY_REG_ADDR      HW_DOUT_FIFO_EMPTY_REG_ADDR      /*0x0D50UL*/ 	 

    /*different ranges of memory for the DMA*/   

#define LLF_C2_HW_TMP_SRAM_SRC_LLI_TAB_ADDR	0x0100      /* SRAM offset */
#define LLF_C2_HW_TMP_SRAM_DST_LLI_TAB_ADDR	0x0200      /* SRAM offset */

    /* LLI definitions */
#define LLF_C2_HW_SRC_LLI_MEM_ADDR_REG_ADDR    HW_SRC_LLI_SRAM_ADDR_REG_ADDR 	/*0x0c24UL*/
#define LLF_C2_HW_SRC_LLI_WORD0_ADDR           HW_SRC_LLI_WORD0_REG_ADDR       /*0x0C28UL*/
#define LLF_C2_HW_SRC_LLI_WORD1_ADDR           HW_SRC_LLI_WORD1_REG_ADDR 		/*0x0C2CUL*/

#define LLF_C2_HW_DST_LLI_MEM_ADDR_REG_ADDR    HW_DST_LLI_SRAM_ADDR_REG_ADDR 	/*0x0D24UL*/
#define LLF_C2_HW_DST_LLI_WORD0_ADDR           HW_DST_LLI_WORD0_REG_ADDR 	    /*0x0D28UL*/
#define LLF_C2_HW_DST_LLI_WORD1_ADDR           HW_DST_LLI_WORD1_REG_ADDR 		/*0x0D2CUL*/

#define LLF_C2_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS
#define LLF_C2_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS     LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS
#define LLF_C2_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS
#define LLF_C2_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS     LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS


/* this value defines count of delays in waiting macros */
// !! Note: Change this value for increasing the performance 
#define LLF_C2_BUSY_DELAYS_COUNT  0 

/* defining a macro for waiting to the any control register */
#define LLF_C2_WAIT_ON_REGISTER( VirtualHwBaseAddr, RegisterAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val;\
   for(output_reg_val = 0; (DxInt32_t)output_reg_val < LLF_C2_BUSY_DELAYS_COUNT ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + (RegisterAddr) , output_reg_val ); \
   }while( output_reg_val & 0x1 ); \
}while(0)    


/* defining a macros for waiting to the AES busy registers */
#define LLF_C2_HW_WAIT_ON_C2_BUSY_BIT( VirtualHwBaseAddr )  LLF_C2_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_HW_C2_BUSY_REG_ADDR )


/* defining a macro for waitiong to the DMA busy register */
#define LLF_C2_HW_WAIT_ON_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr )  LLF_C2_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_C2_HW_DOUT_MEM_DMA_BUSY_ADDR )

#define LLF_C2_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr ) LLF_C2_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_C2_HW_DIN_MEM_DMA_BUSY_ADDR )

#define LLF_C2_HW_WAIT_ON_DOUT_FIFO_EMPTY_BIT( VirtualHwBaseAddr )  LLF_C2_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_C2_HW_FIFO_OUT_EMPTY_REG_ADDR )

#define LLF_C2_HW_WAIT_ON_DIN_FIFO_EMPTY_BIT( VirtualHwBaseAddr )  LLF_C2_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_C2_HW_FIFO_IN_EMPTY_REG_ADDR )

/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif


