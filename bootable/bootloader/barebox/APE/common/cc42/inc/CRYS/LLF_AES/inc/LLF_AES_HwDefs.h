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
 
 
#ifndef LLF_AES_HWDEFS_H
#define LLF_AES_HWDEFS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "cc_hw_defs.h"
#ifdef CRYS_AES_RKEK2_SUPPORT
#include "mng_hw_defs.h"
#endif
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
   
/* spesific AES registers */   
#define LLF_AES_HW_AES_KEY_0_ADDR_0            HW_AES_KEY_0_ADDR_0_REG_ADDR  /*0x0400UL*/
#define LLF_AES_HW_AES_KEY_0_ADDR_1            HW_AES_KEY_0_ADDR_1_REG_ADDR  /*0x0404UL*/  
#define LLF_AES_HW_AES_KEY_0_ADDR_2            HW_AES_KEY_0_ADDR_2_REG_ADDR  /*0x0408UL*/  
#define LLF_AES_HW_AES_KEY_0_ADDR_3            HW_AES_KEY_0_ADDR_3_REG_ADDR  /*0x040CUL*/  
#define LLF_AES_HW_AES_KEY_0_ADDR_4            HW_AES_KEY_0_ADDR_4_REG_ADDR  /*0x0410UL*/   
#define LLF_AES_HW_AES_KEY_0_ADDR_5            HW_AES_KEY_0_ADDR_5_REG_ADDR  /*0x0414UL*/  
#define LLF_AES_HW_AES_KEY_0_ADDR_6            HW_AES_KEY_0_ADDR_6_REG_ADDR  /*0x0418UL*/   
#define LLF_AES_HW_AES_KEY_0_ADDR_7            HW_AES_KEY_0_ADDR_7_REG_ADDR  /*0x041CUL*/

#define LLF_AES_HW_AES_KEY_1_ADDR_0            HW_AES_KEY_1_ADDR_0_REG_ADDR  /*0x0420UL*/
#define LLF_AES_HW_AES_KEY_1_ADDR_1            HW_AES_KEY_1_ADDR_1_REG_ADDR  /*0x0424UL*/  
#define LLF_AES_HW_AES_KEY_1_ADDR_2            HW_AES_KEY_1_ADDR_2_REG_ADDR  /*0x0428UL*/  
#define LLF_AES_HW_AES_KEY_1_ADDR_3            HW_AES_KEY_1_ADDR_3_REG_ADDR  /*0x042CUL*/  
#define LLF_AES_HW_AES_KEY_1_ADDR_4            HW_AES_KEY_1_ADDR_4_REG_ADDR  /*0x0430UL*/   
#define LLF_AES_HW_AES_KEY_1_ADDR_5            HW_AES_KEY_1_ADDR_5_REG_ADDR  /*0x0434UL*/  
#define LLF_AES_HW_AES_KEY_1_ADDR_6            HW_AES_KEY_1_ADDR_6_REG_ADDR  /*0x0438UL*/   
#define LLF_AES_HW_AES_KEY_1_ADDR_7            HW_AES_KEY_1_ADDR_7_REG_ADDR  /*0x043CUL*/

#define LLF_AES_HW_AES_IV_0_ADDR_0             HW_AES_IV_0_ADDR_0_REG_ADDR /*0x0440UL*/
#define LLF_AES_HW_AES_IV_0_ADDR_1             HW_AES_IV_0_ADDR_1_REG_ADDR /*0x0444UL*/   
#define LLF_AES_HW_AES_IV_0_ADDR_2             HW_AES_IV_0_ADDR_2_REG_ADDR /*0x0448UL*/
#define LLF_AES_HW_AES_IV_0_ADDR_3             HW_AES_IV_0_ADDR_3_REG_ADDR /*0x044CUL*/
#define LLF_AES_HW_AES_IV_0_ADDR	           HW_AES_IV_0_REG_ADDR  /* the same as IV_0_ADDR_0  0x0440UL */	      

#define LLF_AES_HW_AES_IV_1_ADDR_0             HW_AES_IV_1_ADDR_0_REG_ADDR /*0x0450UL*/
#define LLF_AES_HW_AES_IV_1_ADDR_1             HW_AES_IV_1_ADDR_1_REG_ADDR /*0x0454UL*/   
#define LLF_AES_HW_AES_IV_1_ADDR_2             HW_AES_IV_1_ADDR_2_REG_ADDR /*0x0458UL*/
#define LLF_AES_HW_AES_IV_1_ADDR_3             HW_AES_IV_1_ADDR_3_REG_ADDR /*0x045CUL*/
#define LLF_AES_HW_AES_IV_1_ADDR	           HW_AES_IV_1_REG_ADDR  /* the same as IV_0_ADDR_0  0x0450UL */	      

/* previous IV addressses */
#define LLF_AES_HW_PREV_AES_IV_ADDR_0          HW_AES_PREV_IV_0_ADDR_0_REG_ADDR /*0x0490UL*/   
#define LLF_AES_HW_PREV_AES_IV_ADDR_1          HW_AES_PREV_IV_0_ADDR_1_REG_ADDR /*0x0494UL*/
#define LLF_AES_HW_PREV_AES_IV_ADDR_2          HW_AES_PREV_IV_0_ADDR_2_REG_ADDR /*0x0498UL*/  
#define LLF_AES_HW_PREV_AES_IV_ADDR_3          HW_AES_PREV_IV_0_ADDR_3_REG_ADDR /*0x049CUL*/
#define LLF_AES_HW_PREV_AES_IV_ADDR	           HW_AES_PREV_IV_0_ADDR_0_REG_ADDR /* the same as PREV_AES_IV_ADDR_0 0x0490UL */

/* CTR_0 registers */
#define LLF_AES_HW_AES_CTR_ADDR_0              HW_AES_CTR0_ADDR_0_REG_ADDR      /*0x0460UL*/  
#define LLF_AES_HW_AES_CTR_ADDR_1              HW_AES_CTR0_ADDR_1_REG_ADDR      /*0x0464UL*/  
#define LLF_AES_HW_AES_CTR_ADDR_2              HW_AES_CTR0_ADDR_2_REG_ADDR      /*0x0468UL*/
#define LLF_AES_HW_AES_CTR_ADDR_3			   HW_AES_CTR0_ADDR_3_REG_ADDR      /*0x046CUL*/
/* CTR_0 alias  */
#define LLF_AES_HW_AES_CTR_0_ADDR_0            HW_AES_CTR0_ADDR_0_REG_ADDR      /*0x0460UL*/  
#define LLF_AES_HW_AES_CTR_0_ADDR_1            HW_AES_CTR0_ADDR_1_REG_ADDR      /*0x0464UL*/  
#define LLF_AES_HW_AES_CTR_0_ADDR_2            HW_AES_CTR0_ADDR_2_REG_ADDR      /*0x0468UL*/
#define LLF_AES_HW_AES_CTR_0_ADDR_3			   HW_AES_CTR0_ADDR_3_REG_ADDR      /*0x046CUL*/

/* CTR_1 registers */
#define LLF_AES_HW_AES_CTR_1_ADDR_0            HW_AES_CTR1_ADDR_0_REG_ADDR      /*0x04E0UL*/  
#define LLF_AES_HW_AES_CTR_1_ADDR_1            HW_AES_CTR1_ADDR_1_REG_ADDR      /*0x04E4UL*/  
#define LLF_AES_HW_AES_CTR_1_ADDR_2            HW_AES_CTR1_ADDR_2_REG_ADDR      /*0x04E8UL*/
#define LLF_AES_HW_AES_CTR_1_ADDR_3			   HW_AES_CTR1_ADDR_3_REG_ADDR      /*0x04ECUL*/

#define LLF_AES_REMAINING_BYTES_REG_ADDR	   HW_AES_REMAINING_BYTES_REG_ADDR  /*0x04BCUL*/

#define LLF_AES_CMAC_INIT_REG_ADDR             HW_AES_CMAC_INIT_REG_ADDR
#define LLF_AES_CMAC_SIZE0_KICK_REG_ADDR       HW_AES_CMAC_SIZE0_KICK_REG_ADDR  /*0x0524UL*/

#define LLF_AES_HW_AES_SK_ADDR                 HW_AES_SK_REG_ADDR 		        /*0x0478UL*/
#define LLF_AES_HW_AES_SK_LOAD_VAL             0x0001UL
#define LLF_AES_HW_AES_SK_SIGN_ENC_VAL         0x0002UL


#define LLF_AES_HW_AES_MAC_OK_ADDR             HW_AES_MAC_OK_REG_ADDR           /*0x0480UL*/
#define LLF_AES_HW_AES_MAC_OK_VAL              0x0480UL


#define LLF_AES_HW_AES_CTL_ADDR                 HW_AES_CONTROL_REG_ADDR 		 /*0x04C0UL*/
#define LLF_AES_HW_AES_CTL_KEY_0_ENCRYPT_VAL    0x0000UL
#define LLF_AES_HW_AES_CTL_KEY_0_DECRYPT_VAL    0x0001UL

#define LLF_AES_HW_AES_CTL_ENCRYPT_VAL          LLF_AES_HW_AES_CTL_KEY_0_ENCRYPT_VAL
#define LLF_AES_HW_AES_CTL_DECRYPT_VAL          LLF_AES_HW_AES_CTL_KEY_0_DECRYPT_VAL 

#define LLF_AES_HW_AES_CTL_KEY_0_ECB_MODE_VAL   (0x0000UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_CBC_MODE_VAL   (0x0001UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_CTR_MODE_VAL   (0x0002UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_MAC_MODE_VAL   (0x0003UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_XTS_MODE_VAL   (0x0004UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_XCBC_MODE_VAL  (0x0005UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_GCTR_MODE_VAL  (0x0006UL << 2 )
#define LLF_AES_HW_AES_CTL_KEY_0_CMAC_MODE_VAL  (0x0007UL << 2 )



#define LLF_AES_HW_AES_CTL_KEY_1_ECB_MODE_VAL   (0x0000UL << 5 )
#define LLF_AES_HW_AES_CTL_KEY_1_CBC_MODE_VAL   (0x0001UL << 5 )
#define LLF_AES_HW_AES_CTL_KEY_1_CTR_MODE_VAL   (0x0002UL << 5 )
#define LLF_AES_HW_AES_CTL_KEY_1_MAC_MODE_VAL   (0x0003UL << 5 )
#define LLF_AES_HW_AES_CTL_KEY_1_XTS_MODE_VAL   (0x0004UL << 5 )
#define LLF_AES_HW_AES_CTL_KEY_1_XCBC_MODE_VAL  (0x0005UL << 5 )
#define LLF_AES_HW_AES_CTL_KEY_1_CMAC_MODE_VAL  (0x0007UL << 5 )


#define LLF_AES_HW_AES_CTL_KEY_0_SELECT_VAL     (0x0000UL << 9 )

#define LLF_AES_HW_AES_CTL_KEY_0_KEY128_VAL     (0x0000UL << 12 )
#define LLF_AES_HW_AES_CTL_KEY_0_KEY192_VAL     (0x0001UL << 12 )
#define LLF_AES_HW_AES_CTL_KEY_0_KEY256_VAL     (0x0002UL << 12 )

#define LLF_AES_HW_AES_CTL_KEY_1_KEY128_VAL     (0x0000UL << 14 )
#define LLF_AES_HW_AES_CTL_KEY_1_KEY192_VAL     (0x0001UL << 14 )
#define LLF_AES_HW_AES_CTL_KEY_1_KEY256_VAL     (0x0002UL << 14 )

#define LLF_AES_HW_AES_CTL_KEY128_VAL           LLF_AES_HW_AES_CTL_KEY_0_KEY128_VAL 
#define LLF_AES_HW_AES_CTL_KEY192_VAL           LLF_AES_HW_AES_CTL_KEY_0_KEY192_VAL 
#define LLF_AES_HW_AES_CTL_KEY256_VAL           LLF_AES_HW_AES_CTL_KEY_0_KEY256_VAL 
/* bit of direct access to AES-DIN-DOUT without DIN-DOUT buffer */
#define LLF_AES_HW_DIRECT_ACC_N_DIN_DOUT        (0x0001UL << 31 )

#define LLF_AES_HW_AES_BUSY_ADDR                HW_AES_BUSY_REG_ADDR            /*0x0914UL*/

#define LLF_AES_HW_TUNNEL_MODE_VAL				0x400UL

#define LLF_AES_HW_TUNNEL_MID_TO_HASH_VAL		0x10000000UL

/* data interface registers on direct acess */

/* direct access AES-DIN-DOUT - 4 registers */
#define LLF_AES_HW_AES_DIN_DOUT_ADDR            HW_AES_DIN_DOUT_ADDR_0_REG_ADDR /*0x04A0UL*/
#define LLF_AES_HW_DIN_DOUT_ADDR_               LLF_AES_HW_AES_DIN_DOUT_ADDR    /*0x04A0UL*/

/* access to DIN-DOUT buffer - 4 registers */
#define LLF_AES_HW_DIN_DOUT_ADDR                HW_DIN_BUFFER_REG_ADDR          /*0x0C00UL*/
/* aligner registers */
#define LLF_AES_HW_DIN_WRITE_ALIGN_ADDR         HW_WRITE_ALIGN_REG_ADDR         /*0x0C3CUL*/
#define LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR    HW_WRITE_ALIGN_LAST_REG_ADDR    /*0x0C4CUL*/ 
#define LLF_AES_HW_DOUT_READ_ALIGN_ADDR         HW_READ_ALIGN_REG_ADDR          /*0x0D3CUL*/
#define LLF_AES_HW_READ_LAST_DATA_ADDR          HW_READ_ALIGN_LAST_REG_ADDR     /*0x0D44UL*/

/* data interface registers to execute the SSDMA DMA operation */
#define LLF_AES_HW_DIN_DMA_LEN_ADDR		        HW_DIN_SRAM_BYTES_LEN_REG_ADDR  /*0x0C34UL*/
#define LLF_AES_HW_DOUT_DMA_LEN_ADDR		    HW_DOUT_SRAM_BYTES_LEN_REG_ADDR /*0x0D34UL*/

#define LLF_AES_HW_DIN_MEM_DMA_BUSY_ADDR     	HW_DIN_MEM_DMA_BUSY_REG_ADDR    /*0x0C20UL*/
#define LLF_AES_HW_DOUT_MEM_DMA_BUSY_ADDR       HW_DOUT_MEM_DMA_BUSY_REG_ADDR   /*0x0D20UL*/

#define LLF_AES_HW_DIN_DMA_SRC_ADDR          	HW_SRAM_SRC_ADDR_REG_ADDR 	    /*0x0C30UL*/
#define LLF_AES_HW_DOUT_DMA_DEST_ADDR          	HW_SRAM_DEST_ADDR_REG_ADDR 	    /*0x0D30UL*/

#define LLF_AES_HW_FIFO_IN_EMPTY_REG_ADDR       HW_FIFO_IN_EMPTY_REG_ADDR       /*0x0C50UL*/ 
#define LLF_AES_HW_FIFO_OUT_EMPTY_REG_ADDR      HW_DOUT_FIFO_EMPTY_REG_ADDR      /*0x0D50UL*/ 	 
	 

#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_0		HW_AES_XEX_HW_T_CALC_KEY_ADDR_0_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_1		HW_AES_XEX_HW_T_CALC_KEY_ADDR_1_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_2		HW_AES_XEX_HW_T_CALC_KEY_ADDR_2_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_3		HW_AES_XEX_HW_T_CALC_KEY_ADDR_3_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_4		HW_AES_XEX_HW_T_CALC_KEY_ADDR_4_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_5		HW_AES_XEX_HW_T_CALC_KEY_ADDR_5_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_6		HW_AES_XEX_HW_T_CALC_KEY_ADDR_6_REG_ADDR
#define LLF_AES_HW_AES_XEX_T_CALC_KEY_ADDR_7		HW_AES_XEX_HW_T_CALC_KEY_ADDR_7_REG_ADDR

#define LLF_AES_HW_DATA_UNIT_SIZE_REG_ADDR			HW_AES_DATA_UNIT_REG_ADDR			/*0x520UL*/
#define LLF_AES_HW_XEX_T_CALC_KICK_REG_ADDR			HW_AES_XEX_HW_T_CALC_KICK_REG_ADDR  /*0x4CCUL*/
#define LLF_AES_HW_XEX_T_CALC_IS_ON					HW_AES_XEX_HW_T_CALC_IS_ON_REG_ADDR /*0x4D4UL*/

/* LLI definitions */
#define LLF_AES_HW_SRC_LLI_MEM_ADDR_REG_ADDR    HW_SRC_LLI_SRAM_ADDR_REG_ADDR 	/*0x0c24UL*/
#define LLF_AES_HW_SRC_LLI_WORD0_ADDR           HW_SRC_LLI_WORD0_REG_ADDR       /*0x0C28UL*/
#define LLF_AES_HW_SRC_LLI_WORD1_ADDR           HW_SRC_LLI_WORD1_REG_ADDR 		/*0x0C2CUL*/

#define LLF_AES_HW_DST_LLI_MEM_ADDR_REG_ADDR    HW_DST_LLI_SRAM_ADDR_REG_ADDR 	/*0x0D24UL*/
#define LLF_AES_HW_DST_LLI_WORD0_ADDR           HW_DST_LLI_WORD0_REG_ADDR 	    /*0x0D28UL*/
#define LLF_AES_HW_DST_LLI_WORD1_ADDR           HW_DST_LLI_WORD1_REG_ADDR 		/*0x0D2CUL*/

#define LLF_AES_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS
#define LLF_AES_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS     LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS
#define LLF_AES_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS    LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS
#define LLF_AES_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS     LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS

#define LLF_AES_HW_DMA_EN_EOM_ADDR          	0x201CUL


/* general CRYPTOCELL interface registers */
#define LLF_AES_HW_AES_CLK_ENABLE_ADDR          HW_AES_CLK_ENABLE_REG_ADDR 	    /*0x0810UL*/
#define LLF_AES_HW_CLK_STATUS_ADDR              HW_CLK_STATUS_REG_ADDR 		    /*0x0824UL*/
#define LLF_AES_HW_CRYPTO_CTL_ADDR              HW_CRYPTO_CTL_REG_ADDR 		    /*0x0900UL*/
#define LLF_AES_HW_CRYPTO_CTL_BYPASS_MODE_VAL   0x0000UL
#define LLF_AES_HW_CRYPTO_CTL_AES_MODE_VAL      0x0001UL

/* HW version address and validation values */
#define LLF_AES_HW_VERSION_ADDR                 HW_VERSION_REG_ADDR 		    /*0x0928UL*/

#define LLF_AES_CUST_CODE_ONE_VAL               0x1
#define LLF_AES_CUST_CODE_TWO_VAL               0x2
#define LLF_AES_CUST_CODE_THREE_VAL             0x3
#define LLF_AES_CUST_CODE_FOUR_VAL              0x4

#ifndef RELEASE_VERSION
	#define LLF_AES_HW_VERSION_VAL                  0x2200UL 
#else
	#define LLF_AES_HW_VERSION_VAL                  0x2200UL 
#endif

/* the HW address space */
#define LLF_AES_HW_CRYPTO_ADDR_SPACE ( 1UL << 12 )

/*different ranges of memory for the DMA*/   
#define LLF_AES_HW_DMA_TMP_SRC_ADDR				0xC2010000
#define LLF_AES_HW_DMA_TMP_DST_ADDR				0xC2012000 /*0xC001C000*/

#define LLF_AES_HW_TMP_SRAM_SRC_LLI_TAB_ADDR	0x0100      /* SRAM offset */
#define LLF_AES_HW_TMP_SRAM_DST_LLI_TAB_ADDR	0x0200      /* SRAM offset */

#define LLF_AES_HW_FLAGS_ADDR					HW_AES_HW_FLAGS_REG_ADDR /*0x04C8UL*/
#define LLF_AES_HW_GBG_RND_SEED_ADDR			HW_AES_RBG_SEED_REG_ADDR /*0x04D0UL*/
#define LLF_AES_HW_HLAG_192_256_VAL             0x1UL 
#define LLF_AES_HW_HLAG_LRKEK_VAL               0x2UL 
#define LLF_AES_HW_HLAG_CTMR_VAL                0x4UL 

#define LLF_AES_HW_AHB_RD_WR_BURSTS_ADDR        HW_AHB_RD_WR_BURSTS_REG_ADDR
#define LLF_AES_HW_AHB_DISABLE_DBUFFER_REG_ADDR HW_AHB_DISABLE_DBUFFER_REG_ADDR /*0xC0000E18*/

#ifdef CRYS_AES_RKEK2_SUPPORT
/* crypto key selection RKEK1/RKEK2 */
#define LLF_AES_HW_CRYPTOKEY_SEL_REG_ADDR				HW_AES_CRYPTOKEY_SELECTION_REG_ADDR
#endif

 
/* ********************** Macros ******************************* */

/* this value defines count of delays in waiting macros */
// !! Note: Change this value for increasing the performance 
#define LLF_AES_BUSY_DELAYS_COUNT  100 

/* defining a macro for waiting to the any control register */
#define LLF_AES_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, RegisterAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val;\
   for(output_reg_val = 0; (DxInt32_t)output_reg_val < LLF_AES_BUSY_DELAYS_COUNT ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + (RegisterAddr) , output_reg_val ); \
   }while( output_reg_val & 0x1 ); \
}while(0)    


/* defining a macros for waiting to the AES busy registers */
#define LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr )  LLF_AES_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_AES_HW_AES_BUSY_ADDR )

#define LLF_AES_HW_WAIT_ON_AES_SK2_BUSY_BIT( VirtualHwBaseAddr )  LLF_AES_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_AES_HW_AES_LOAD_SK2_BUSY_ADDR )

#define LLF_AES_HW_WAIT_ON_DOUT_FIFO_EMPTY_BIT( VirtualHwBaseAddr )  LLF_AES_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_AES_HW_FIFO_OUT_EMPTY_REG_ADDR )


/* defining a macro for waitiong to the DMA busy register */
#define LLF_AES_HW_WAIT_ON_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr )  LLF_AES_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_AES_HW_DOUT_MEM_DMA_BUSY_ADDR )

#define LLF_AES_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr )  LLF_AES_HW_WAIT_ON_REGISTER( VirtualHwBaseAddr, LLF_AES_HW_DIN_MEM_DMA_BUSY_ADDR )


#ifdef DX_REG_ACCESS_WITH_NO_PRINT
/*THIS DEBUG IS ONLY FOR THE CF3 LLFCD C implementation (till we can use */
/* the assembler implementation 										 */

#define LLF_AES_HW_WAIT_ON_AES_BUSY_BIT_NO_PRINT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val;\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister_NO_PRINT( (VirtualHwBaseAddr) + LLF_AES_HW_AES_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x1 ); \
}while(0)  

#endif /*DX_REG_ACCESS_WITH_NO_PRINT*/


/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif


