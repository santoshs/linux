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
 
 
#ifndef LLF_RC4_HWDEFS_H
#define LLF_RC4_HWDEFS_H

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
   *  Object % LLF_RC4_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  07 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes the hardware address of the registers relevant to the DES module.
   *
   *  \version LLF_RC4_HwDefs.h#1:incl:1
   *  \author R.Levin
   */




/************************ Defines ******************************/


/* the definitions of the RC4 hardware registers offset above the base address 
   determaind by the user and the relevant bits */
   
/* spesific RC4 registers */                                                     /* VAL */
#define LLF_RC4_HW_RC4_INIT_REG_ADDR	         HW_RC4_INIT_REG_ADDR   		 /*0x0300UL*/          
#define LLF_RC4_HW_RC4_READY_REG_ADDR 	         HW_RC4_READY_REG_ADDR  		 /*0x0304UL*/          
#define LLF_RC4_HW_RC4_KEY_REG_ADDR 		  	 HW_RC4_KEY_REG_REG_ADDR  		 /*0x0308UL*/          
#define LLF_RC4_HW_RC4_RST_KEY_PTR_REG_ADDR      HW_RC4_RST_KEY_PTR_REG_ADDR     /*0x030CUL*/
#define LLF_RC4_HW_RC4_KEY_LENGTH_REG_ADDR 		 HW_RC4_KEY_LENGTH_REG_ADDR 	 /*0x0318UL*/          
#define LLF_RC4_HW_RC4_SUSPEND_REQUEST_REG_ADDR  HW_RC4_SUSPEND_REQUEST_REG_ADDR /*0x031CUL*/          
#define LLF_RC4_HW_RC4_SUSPEND_ANSWER_REG_ADDR 	 HW_RC4_SUSPEND_ANSWER_REG_ADDR  /*0x0320UL*/          
#define LLF_RC4_HW_RC4_RESUME_REQUEST_REG_ADDR 	 HW_RC4_RESUME_REQUEST_REG_ADDR  /* 0x0324UL*/          
#define LLF_RC4_HW_RC4_RESUME_ANSWER_REG_ADDR 	 HW_RC4_RESUME_ANSWER_REG_ADDR 	 /*0x0328UL*/          
#define LLF_RC4_HW_RC4_SPECIAL_REGISTER_REG_ADDR HW_RC4_SPECIAL_REGISTER_REG_ADDR /*0x032CUL*/          
#define LLF_RC4_HW_RC4_DATA_READY_REG_ADDR 		 HW_RC4_DATA_READY_REG_ADDR 	 /*0x0330UL*/          

/* RC4 registers control values */
#define LLF_RC4_HW_SUSPEND_REQUEST_VAL              0x00000001
#define LLF_RC4_HW_SUSPEND_ANSWER_VAL               0x00000002
#define LLF_RC4_HW_RESUME_REQUEST_VAL               0x00000001
#define LLF_RC4_HW_DATA_READY_VAL                   0x00000001


/* data interface registers on direct acess */
#define LLF_RC4_HW_DIN_DOUT_ADDR                HW_DIN_BUFFER_REG_ADDR         /*0x0C00UL*/
#define LLF_RC4_HW_DIN_WRITE_ALIGN_ADDR         HW_WRITE_ALIGN_REG_ADDR        /*0x0C3CUL*/
#define LLF_RC4_HW_WRITE_ALIGN_LAST_REG_ADDR    HW_WRITE_ALIGN_LAST_REG_ADDR   /*0x0C4CUL*/ 
#define LLF_RC4_HW_DOUT_READ_ALIGN_ADDR         HW_READ_ALIGN_REG_ADDR         /*0x0D3CUL*/
#define LLF_RC4_HW_READ_ALIGN_LAST_REG_ADDR     HW_READ_ALIGN_LAST_REG_ADDR    /*0x0D44UL*/


/* data interface registers to execute the SSDMA DMA operation */
#define LLF_RC4_HW_DIN_MEM_DMA_BUSY_ADDR        HW_DIN_MEM_DMA_BUSY_REG_ADDR   /*0x0C20UL*/
#define LLF_RC4_HW_SRC_LLI_MEM_ADDR_REG_ADDR    HW_SRC_LLI_SRAM_ADDR_REG_ADDR  /*0x0C24UL*/
#define LLF_RC4_HW_SRC_LLI_WORD0_ADDR           HW_SRC_LLI_WORD0_REG_ADDR      /*0x0C28UL*/
#define LLF_RC4_HW_SRC_LLI_WORD1_ADDR           HW_SRC_LLI_WORD1_REG_ADDR      /*0x0C2CUL*/

#define LLF_RC4_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS   LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS 
#define LLF_RC4_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS 

#define LLF_RC4_HW_SRAM_SRC_ADDR_REG_ADDR       HW_SRAM_SRC_ADDR_REG_ADDR      /*0x0C30UL*/
#define LLF_RC4_HW_SRAM_BYTES_LEN_ADDR          HW_DIN_SRAM_BYTES_LEN_REG_ADDR /*0x0C34UL*/
#define LLF_RC4_HW_DIN_SRAM_DMA_BUSY_ADDR       HW_DIN_SRAM_DMA_BUSY_REG_ADDR  /*0x0C38UL*/

#define LLF_RC4_HW_DOUT_MEM_DMA_BUSY_ADDR       HW_DOUT_MEM_DMA_BUSY_REG_ADDR  /*0x0D20UL*/
#define LLF_RC4_HW_DST_LLI_MEM_ADDR_REG_ADDR    HW_DST_LLI_SRAM_ADDR_REG_ADDR  /*0x0D24UL*/
#define LLF_RC4_HW_DST_LLI_WORD0_ADDR           HW_DST_LLI_WORD0_REG_ADDR      /*0x0D28UL*/
#define LLF_RC4_HW_DST_LLI_WORD1_ADDR           HW_DST_LLI_WORD1_REG_ADDR      /*0x0D2CUL*/

#define LLF_RC4_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS   LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS 
#define LLF_RC4_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS 

#define LLF_RC4_HW_DOUT_SRAM_DEST_ADDR          HW_SRAM_DEST_ADDR_REG_ADDR      /*0x0D30UL*/
#define LLF_RC4_HW_DOUT_SRAM_BYTES_LEN_ADDR     HW_DOUT_SRAM_BYTES_LEN_REG_ADDR /*0x0D34UL*/
#define LLF_RC4_HW_DOUT_SRAM_DMA_BUSY_ADDR      HW_DOUT_SRAM_DMA_BUSY_REG_ADDR  /*0x0D38UL*/

/* general CRYPTOCELL interface registers */
#define LLF_RC4_HW_RC4_CLK_ENABLE_ADDR          HW_RC4_CLK_ENABLE_REG_ADDR      /*0x0854UL*/
#define LLF_RC4_HW_CRYPTO_CTL_ADDR              HW_CRYPTO_CTL_REG_ADDR          /*0x0900UL*/

#define LLF_RC4_HW_CRYPTO_CTL_RC4_MODE_VAL      0x000BUL

#define LLF_RC4_HW_VERSION_ADDR                 HW_VERSION_REG_ADDR             /*0x0928UL*/

#define LLF_RC4_HW_VERSION_VAL                   0x2200UL 

/*different ranges of memory for the DMA*/   
#define LLF_RC4_HW_DMA_TMP_SRC_ADDR				0xC0010000
#define LLF_RC4_HW_DMA_TMP_DST_ADDR				0xC0012000 /*0xC001C000*/

#define LLF_RC4_HW_TMP_SRAM_SRC_LLI_TAB_ADDR	0x0100      /* SRAM offset */
#define LLF_RC4_HW_TMP_SRAM_DST_LLI_TAB_ADDR	0x0200      /* SRAM offset */

/* the HW address space */
#define LLF_RC4_HW_CRYPTO_ADDR_SPACE ( 1UL << 12 )

/* ********************** Macros ******************************* */

/* defining a macro for waiting to the RC4 mashine ready register */

#define LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + HW_CRYPTO_BUSY_REG_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01 ); \
}while(0) 



#define LLF_RC4_HW_WAIT_ON_RC4_READY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_RC4_HW_RC4_READY_REG_ADDR , output_reg_val ); \
   }while( !(output_reg_val & 0x01) ); \
}while(0) 

/* defining a macro for waiting to the RC4 data ready register */
#define LLF_RC4_HW_WAIT_ON_RC4_DATA_READY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_RC4_HW_RC4_DATA_READY_REG_ADDR , output_reg_val ); \
   }while( !(output_reg_val & 0x01) ); \
}while(0)    
   

/* defining a macro for waiting to the DMA busy register */
#define LLF_RC4_HW_WAIT_ON_DOUT_MEM_DMA_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_RC4_HW_DOUT_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01); \
}while(0)    


/* defining a macro for waiting to the DMA busy register */
#define LLF_RC4_HW_WAIT_ON_DIN_MEM_DMA_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_RC4_HW_DIN_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01); \
}while(0)    




  
/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif


