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
 
 
#ifndef LLF_DES_HWDEFS_H
#define LLF_DES_HWDEFS_H

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
   *  Object % LLF_DES_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:40:57 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes the hardware address of the registers relevant to the DES module.
   *
   *  \version LLF_DES_HwDefs.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

/*#define mask M = 0xFFFF */
#define M 0xFFFFUL

/* the definitions of the DES hardware registers offset above the base address 
   determaind by the user and the relevant bits */
   
/* spesific DES registers */                                                  /* VAL */
#define LLF_DES_HW_DES_KEY_0_ADDR               HW_DES_KEY_0_REG_ADDR        /*0x0208UL*/
#define LLF_DES_HW_DES_KEY_1_ADDR               HW_DES_KEY_1_REG_ADDR        /*0x020CUL*/   
#define LLF_DES_HW_DES_KEY_2_ADDR               HW_DES_KEY_2_REG_ADDR        /*0x0210UL*/   
#define LLF_DES_HW_DES_KEY_3_ADDR               HW_DES_KEY_3_REG_ADDR        /*0x0214UL*/   
#define LLF_DES_HW_DES_KEY_4_ADDR               HW_DES_KEY_4_REG_ADDR        /*0x0218UL*/  
#define LLF_DES_HW_DES_KEY_5_ADDR               HW_DES_KEY_5_REG_ADDR        /*0x021CUL*/   

#define LLF_DES_HW_DES_CTL_0_ADDR               HW_DES_CONTROL_0_REG_ADDR    /*0x0220UL*/
#define LLF_DES_HW_DES_CTL_1_ADDR               HW_DES_CONTROL_1_REG_ADDR    /*0x0224UL*/

#define LLF_DES_HW_DES_CTL_ENCRYPT_VAL          0x0000UL
#define LLF_DES_HW_DES_CTL_DECRYPT_VAL          0x0001UL
#define LLF_DES_HW_DES_CTL_1_KEY_VAL            (0x0000UL << 1 )
#define LLF_DES_HW_DES_CTL_2_KEYS_VAL           (0x0001UL << 1 )
#define LLF_DES_HW_DES_CTL_3_KEYS_VAL           (0x0002UL << 1 )
#define LLF_DES_HW_DES_CTL_ECB_MODE_VAL         (0x0000UL << 3 )
#define LLF_DES_HW_DES_CTL_CBC_MODE_VAL         (0x0001UL << 3 )


#define LLF_DES_HW_DES_IV_0_ADDR                HW_DES_IV_0_REG_ADDR           /*0x0228UL*/
#define LLF_DES_HW_DES_IV_1_ADDR                HW_DES_IV_1_REG_ADDR           /*0x022CUL*/   
 
#define LLF_DES_HW_DES_BUSY_ADDR                HW_DES_BUSY_REG_ADDR           /*0x0918UL*/

/* data interface registers on direct acess */
#define LLF_DES_HW_DIN_DOUT_ADDR                HW_DIN_BUFFER_REG_ADDR         /*0x0C00UL */
#define LLF_DES_HW_DIN_WRITE_ALIGN_ADDR         HW_WRITE_ALIGN_REG_ADDR        /*0x0C3CUL*/
#define LLF_DES_HW_DOUT_READ_ALIGN_ADDR         HW_READ_ALIGN_REG_ADDR         /*0x0D3CUL*/
#define LLF_DES_HW_READ_LAST_DATA_ADDR          HW_READ_ALIGN_LAST_REG_ADDR    /*0x0D44UL*/

/* data interface registers to execute the SSDMA DMA operation */
#define LLF_DES_HW_DIN_MEM_DMA_BUSY_ADDR        HW_DIN_MEM_DMA_BUSY_REG_ADDR   /*0x0C20UL*/
#define LLF_DES_HW_SRC_LLI_MEM_ADDR_REG_ADDR    HW_SRC_LLI_SRAM_ADDR_REG_ADDR   /*0x0C24UL*/
#define LLF_DES_HW_SRC_LLI_WORD0_ADDR           HW_SRC_LLI_WORD0_REG_ADDR      /*0x0C28UL*/
#define LLF_DES_HW_SRC_LLI_WORD1_ADDR           HW_SRC_LLI_WORD1_REG_ADDR      /*0x0C2CUL*/

#define LLF_DES_HW_LLI_WORD1_FIRST_LLI_WORD_POS   LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS 
#define LLF_DES_HW_LLI_WORD1_LAST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS 

#define LLF_DES_HW_SRAM_SRC_ADDR_REG_ADDR       HW_SRAM_SRC_ADDR_REG_ADDR      /*0x0C30UL*/
#define LLF_DES_HW_SRAM_BYTES_LEN_ADDR          HW_DIN_SRAM_BYTES_LEN_REG_ADDR /*0x0C34UL*/
#define LLF_DES_HW_DIN_SRAM_DMA_BUSY_ADDR       HW_DIN_SRAM_DMA_BUSY_REG_ADDR  /*0x0C38UL*/

#define LLF_DES_HW_DOUT_MEM_DMA_BUSY_ADDR       HW_DOUT_MEM_DMA_BUSY_REG_ADDR  /*0x0D20UL*/
#define LLF_DES_HW_DST_LLI_MEM_ADDR_REG_ADDR    HW_DST_LLI_SRAM_ADDR_REG_ADDR   /*0x0D24UL*/
#define LLF_DES_HW_DST_LLI_WORD0_ADDR           HW_DST_LLI_WORD0_REG_ADDR      /*0x0D28UL*/
#define LLF_DES_HW_DST_LLI_WORD1_ADDR           HW_DST_LLI_WORD1_REG_ADDR      /*0x0D2CUL*/

#define LLF_DES_HW_DOUT_SRAM_DEST_ADDR          HW_SRAM_DEST_ADDR_REG_ADDR      /*0x0D30UL*/
#define LLF_DES_HW_DOUT_SRAM_BYTES_LEN_ADDR     HW_DOUT_SRAM_BYTES_LEN_REG_ADDR /*0x0D34UL*/
#define LLF_DES_HW_DOUT_SRAM_DMA_BUSY_ADDR      HW_DOUT_SRAM_DMA_BUSY_REG_ADDR  /*0x0D38UL*/

/* general CRYPTOCELL interface registers */
#define LLF_DES_HW_DES_CLK_ENABLE_ADDR          HW_DES_CLK_ENABLE_REG_ADDR      /*0x0814UL*/
#define LLF_DES_HW_CRYPTO_CTL_ADDR              HW_CRYPTO_CTL_REG_ADDR          /*0x0900UL*/

#define LLF_DES_HW_CRYPTO_CTL_DES_MODE_VAL      0x0004UL

#define LLF_DES_HW_VERSION_ADDR                 HW_VERSION_REG_ADDR             /*0x0928UL*/

#define LLF_DES_HW_VERSION_VAL                  0x2202UL 

/*different ranges of memory for the DMA*/   
#define LLF_DES_HW_DMA_TMP_SRC_ADDR				0xC2010000
#define LLF_DES_HW_DMA_TMP_DST_ADDR				0xC2020000 /*0xC001C000*/

#define LLF_DES_HW_TMP_SRAM_SRC_LLI_TAB_ADDR	0x0100      /* SRAM offset */
#define LLF_DES_HW_TMP_SRAM_DST_LLI_TAB_ADDR	0x0200      /* SRAM offset */

/* the HW address space */
#define LLF_DES_HW_CRYPTO_ADDR_SPACE ( 1UL << 12 )

/* ********************** Macros ******************************* */

/* defining a macro for waitiong to the DES busy register */
#define LLF_DES_HW_WAIT_ON_DES_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val = 0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_DES_HW_DES_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01 ); \
}while(0)    

/* defining a macro for waitiong to the DMA busy register */
#define LLF_DES_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val = 0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_DES_HW_DIN_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01); \
}while(0)    


/* defining a macro for waitiong to the DMA busy register */
#define LLF_DES_HW_WAIT_ON_DMA_DST_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val = 0; \
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_DES_HW_DOUT_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x01); \
}while(0)    
  
/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif


