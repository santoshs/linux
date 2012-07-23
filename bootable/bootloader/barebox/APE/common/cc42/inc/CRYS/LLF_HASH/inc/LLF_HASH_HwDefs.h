/**************************************************************************
 *   Copyright 2009 � Discretix Technologies Ltd. This software is         *
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
 
 
#ifndef LLF_HASH_HWDEFS_H
#define LLF_HASH_HWDEFS_H

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
   *  Object % LLF_HASH_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:40:57 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes the hardware address of the registers relevant to the HASH module.
   *
   *  \version LLF_HASH_HwDefs.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/
/* ---------------------------
	NEW HASH registers
-----------------------------*/

#define LLF_HASH_HW_HASH_H0_REG_LO		HW_HASH_H0_REG_ADDR  /*0x640UL*/
#define LLF_HASH_HW_HASH_H1_REG_LO		HW_HASH_H1_REG_ADDR  /*0x644UL*/
#define LLF_HASH_HW_HASH_H2_REG_LO		HW_HASH_H2_REG_ADDR  /*0x648UL*/
#define LLF_HASH_HW_HASH_H3_REG_LO		HW_HASH_H3_REG_ADDR  /*0x64cUL*/
#define LLF_HASH_HW_HASH_H4_REG_LO		HW_HASH_H4_REG_ADDR  /*0x650UL*/
#define LLF_HASH_HW_HASH_H5_REG_LO		HW_HASH_H5_REG_ADDR  /*0x654UL*/
#define LLF_HASH_HW_HASH_H6_REG_LO		HW_HASH_H6_REG_ADDR  /*0x658UL*/
#define LLF_HASH_HW_HASH_H7_REG_LO		HW_HASH_H7_REG_ADDR  /*0x65cUL*/

#ifndef LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
#define LLF_HASH_HW_HASH_H0_REG_HI		HW_HASH_H8_REG_ADDR  /*0x660UL*/
#define LLF_HASH_HW_HASH_H1_REG_HI		HW_HASH_H9_REG_ADDR  /*0x664UL*/
#define LLF_HASH_HW_HASH_H2_REG_HI		HW_HASH_H10_REG_ADDR /*0x668UL*/
#define LLF_HASH_HW_HASH_H3_REG_HI		HW_HASH_H11_REG_ADDR /*0x66cUL*/
#define LLF_HASH_HW_HASH_H4_REG_HI		HW_HASH_H12_REG_ADDR /*0x670UL*/
#define LLF_HASH_HW_HASH_H5_REG_HI		HW_HASH_H13_REG_ADDR /*0x674UL*/
#define LLF_HASH_HW_HASH_H6_REG_HI		HW_HASH_H14_REG_ADDR /*0x678UL*/
#define LLF_HASH_HW_HASH_H7_REG_HI		HW_HASH_H15_REG_ADDR /*0x67cUL*/
#endif

#define LLF_HASH_HW_HASH_PAD_EN			HW_HASH_PAD_EN_REG_ADDR     /*0x7C4UL*/
#define LLF_HASH_HW_HASH_PAD_CFG		HW_HASH_PAD_CFG_REG_ADDR    /*0x7C8UL*/

#define LLF_HASH_HW_HASH_CUR_LEN_0		HW_HASH_CUR_LEN_0_REG_ADDR  /*0x7CCUL*/
#define LLF_HASH_HW_HASH_CUR_LEN_1		HW_HASH_CUR_LEN_1_REG_ADDR  /*0x7D0UL*/
#ifndef LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
#define LLF_HASH_HW_HASH_CUR_LEN_2		HW_HASH_CUR_LEN_2_REG_ADDR  /*0x7D4UL*/
#define LLF_HASH_HW_HASH_CUR_LEN_3		HW_HASH_CUR_LEN_3_REG_ADDR  /*0x7D8UL*/
#endif
#define LLF_HASH_HW_HASH_PARAM			HW_HASH_PARAM_REG_ADDR      /*0x7DCUL*/
#define LLF_HASH_HW_HASH_INT_BUSY		HW_HASH_INT_BUSY_REG_ADDR   /*0x7E0UL*/
#define LLF_HASH_HW_HASH_SW_RESET		HW_HASH_SW_RESET_REG_ADDR   /*0x7E4UL*/

#define LLF_HASH_HW_HASH_TIME_COUNTER	0xc0008000UL   /* address in SEP area */

/* enabling of the padding in the hardware */
#define LLF_HASH_HW_HASH_PAD_EN_VAL	        0x1UL
/* hash padding configuration DO_VALUE */
#define LLF_HASH_HW_HASH_DO_PADDING_VAL	    0x4UL

/* hash sha-384/512 present bit */
#define LLF_HASH_HW_HASH_SHA2_PRS_VAL		0x1000UL

/* HASH controls  */
#define LLF_HASH_HW_HASH_CONTROL_ADDR           HW_HASH_CONTROL_REG_ADDR        /*0x07C0UL*/

#define LLF_HASH_HW_HASH_CTL_MD5_VAL            0x0000UL
#define LLF_HASH_HW_HASH_CTL_SHA1_VAL           0x0001UL
#define LLF_HASH_HW_HASH_CTL_SHA256_VAL	        0x0002UL
#define LLF_HASH_HW_HASH_CTL_SHA512_VAL  	    0x0004UL     /* not used at this stage */

/* HASH mashine busy register */ 
#define LLF_HASH_HW_HASH_BUSY_ADDR              HW_HASH_BUSY_REG_ADDR          /*0x091CUL*/

/* data interface registers on direct acess */
#define LLF_HASH_HW_DIN_ADDR                    HW_DIN_BUFFER_REG_ADDR         /*0x0C00UL*/
#define LLF_HASH_HW_DIN_WRITE_ALIGN_ADDR        HW_WRITE_ALIGN_REG_ADDR        /*0x0C3CUL*/
#define LLF_HASH_HW_DOUT_READ_ALIGN_ADDR        HW_READ_ALIGN_REG_ADDR         /*0x0D3CUL*/
#define LLF_HASH_HW_DIN_WRITE_ALIGN_LAST_ADDR   HW_WRITE_ALIGN_LAST_REG_ADDR   /*0x0C4CUL*/

/* data interface registers to execute the SSDMA DMA operation */
#define LLF_HASH_HW_DIN_MEM_DMA_BUSY_ADDR       HW_DIN_MEM_DMA_BUSY_REG_ADDR   /*0x0C20UL*/
#define LLF_HASH_HW_DOUT_MEM_DMA_BUSY_ADDR      HW_DOUT_MEM_DMA_BUSY_REG_ADDR  /*0x0D20UL*/

#define LLF_HASH_HW_SRC_LLI_MEM_ADDR_REG_ADDR   HW_SRC_LLI_SRAM_ADDR_REG_ADDR  /*0x0c24UL*/
#define LLF_HASH_HW_SRC_LLI_WORD0_ADDR          HW_SRC_LLI_WORD0_REG_ADDR      /*0x0C28UL*/
#define LLF_HASH_HW_SRC_LLI_WORD1_ADDR          HW_SRC_LLI_WORD1_REG_ADDR      /*0x0C2CUL*/

#define LLF_HASH_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS   LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS 
#define LLF_HASH_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS 

#define LLF_HASH_HW_SRAM_SRC_ADDR               HW_SRAM_SRC_ADDR_REG_ADDR      /*0x0C30UL*/
#define LLF_HASH_HW_SRAM_BYTES_LEN_ADDR         HW_DIN_SRAM_BYTES_LEN_REG_ADDR /*0x0C34UL*/
#define LLF_HASH_HW_DIN_SRAM_DMA_BUSY_ADDR      HW_DIN_SRAM_DMA_BUSY_REG_ADDR  /*0x0C38UL*/

#define LLF_HASH_HW_ENDIANESS_REG_ADDR          HW_HASH_ENDIANESS_REG_ADDR 	   /*0x07E8UL*/


/* general CRYPTOCELL interface registers */
#define LLF_HASH_HW_HASH_CLK_ENABLE_ADDR        HW_HASH_CLK_ENABLE_REG_ADDR    /*0x0818UL*/
#define LLF_HASH_HW_CRYPTO_CTL_ADDR             HW_CRYPTO_CTL_REG_ADDR         /*0x0900UL*/
#define LLF_HASH_HW_CRYPTO_BUSY_ADDR            HW_CRYPTO_BUSY_REG_ADDR 	   /*0x0910UL*/
#define LLF_HASH_HW_CRYPTO_CTL_HASH_MODE_VAL    0x0007UL

/* HW version address and validation value */
#define LLF_HASH_HW_VERSION_ADDR                HW_VERSION_REG_ADDR            /*0x0928UL*/
#define LLF_HASH_HW_VERSION_VAL                 0x2202UL


/* the HW address space */
#define LLF_HASH_HW_CRYPTO_ADDR_SPACE ( 1 << 12 )

/*different ranges of memory for the DMA*/   
#define LLF_HASH_HW_DMA_TMP_SRC_ADDR			0xC2010000

#define LLF_HASH_HW_TMP_SRAM_SRC_LLI_TAB_ADDR	0x0100      /* SRAM offset */

#define LLF_HASH_HW_FIFO_IN_EMPTY_REG_ADDR                HW_FIFO_IN_EMPTY_REG_ADDR /*0xC50UL*/
/* ********************** Macros ******************************* */

/* defining a macro for waiting to the HASH busy register */
#define LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr ) \
do\
{ \
   volatile DxUint32_t output_reg_val=0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_HASH_HW_HASH_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01 ); \
}while(0)    
  
/* defining a macro for waiting to the DMA DIN busy register */
#define LLF_HASH_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val =0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_HASH_HW_DIN_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01 ); \
}while(0) 

/* defining a macro for waiting to the DMA DST busy register */
#define LLF_HASH_HW_WAIT_ON_DMA_DST_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val = 0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_HASH_HW_DOUT_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01 ); \
}while(0) 


/* defining a macro for waiting to the CRYPTO busy register */
#define LLF_HASH_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val =0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_HASH_HW_CRYPTO_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01 ); \
}while(0)    
   


/* defining a macro for waiting to the CRYPTO busy register */
#define LLF_HASH_HW_WAIT_FIFO_IN_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val =0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_HASH_HW_FIFO_IN_EMPTY_REG_ADDR , output_reg_val ); \
   }while( !output_reg_val); \
}while(0)    

/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif

