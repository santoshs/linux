
#ifndef LLF_COMMON_DEFS_H
#define LLF_COMMON_DEFS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */

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
   *  \version LLF_COMMON_HwDefs.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




#include "cc_hw_defs.h"
/************************ Defines ******************************/



#define LLF_COMMON_HW_DIN_WRITE_ALIGN_ADDR         					HW_WRITE_ALIGN_REG_ADDR         /*0x0C3CUL*/
#define LLF_COMMON_HW_DOUT_READ_ALIGN_ADDR         					HW_READ_ALIGN_REG_ADDR          /*0x0D3CUL*/

#define LLF_COMMON_HW_CRYPTO_ADDR_SPACE 							( 1UL << 12 )
#define LLF_COMMON_HW_VERSION_ADDR                 					HW_VERSION_REG_ADDR 		    /*0x0928UL*/

#define LLF_COMMON_HW_VERSION_VAL                  					0x2200UL 

#define LLF_COMMON_HW_CRYPTO_CTL_ADDR              					HW_CRYPTO_CTL_REG_ADDR 		    /*0x0900UL*/
#define LLF_COMMON_HW_CRYPTO_CTL_BYPASS_MODE_VAL   					0x0000UL

#define LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS    		    	30
#define LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS     		    	31

/* LLI definitions */
#define LLF_COMMON_HW_DST_LLI_WORD0_ADDR           					HW_DST_LLI_WORD0_REG_ADDR 	    /*0x0D28UL*/               
#define LLF_COMMON_HW_SRC_LLI_WORD0_ADDR           					HW_SRC_LLI_WORD0_REG_ADDR       /*0x0C28UL*/
#define LLF_COMMON_HW_DST_LLI_WORD1_ADDR           					HW_DST_LLI_WORD1_REG_ADDR 		/*0x0D2CUL*/
#define LLF_COMMON_HW_SRC_LLI_WORD1_ADDR           					HW_SRC_LLI_WORD1_REG_ADDR 		/*0x0C2CUL*/
#define LLF_COMMON_HW_DST_SRAM_ADDR		 							HW_SRAM_DEST_ADDR_REG_ADDR 		/*0x0D30UL*/
#define LLF_COMMON_HW_DST_SRAM_BYTES_LEN							HW_DOUT_SRAM_BYTES_LEN_REG_ADDR /*0x0D34UL*/

#define LLF_COMMON_HW_DOUT_MEM_DMA_BUSY_ADDR       					HW_DOUT_MEM_DMA_BUSY_REG_ADDR   /*0x0D20UL*/
#define LLF_COMMON_HW_DOUT_SRAM_DMA_BUSY_ADDR       				HW_DOUT_SRAM_DMA_BUSY_REG_ADDR  /*0x0D38UL*/

/* SRAM register*/
#define LLF_COMMON_HW_SRAM_DATA_REG_ADDR	                        HW_SRAM_DATA_REG_ADDR 		    /*0x0F00UL */
#define LLF_COMMON_HW_SRAM_ADDR_REG_ADDR 		                    HW_SRAM_ADDR_REG_ADDR           /*0x0F04UL*/
#define LLF_COMMON_HW_SRAM_DATA_READY_REG_ADDR 	                    HW_SRAM_DATA_READY_REG_ADDR     /*0x0F08UL*/

#define LLF_COMMON_HW_AHB_RD_WR_BURSTS_ADDR        HW_AHB_RD_WR_BURSTS_REG_ADDR
#define LLF_COMMON_HW_AHB_FLAGS_ADDR               HW_AHB_HW_FLAGS_REG_ADDR

#ifdef DX_ARM_INTEGRATOR_DEBUG

	/*different ranges of memory for the DMA*/   
	#define LLF_COMMON_HW_DMA_TMP_SRC_ADDR								0xC0010000  /* in integrator memory */
	#define LLF_COMMON_HW_DMA_TMP_DST_ADDR								0xC0012000  /* in integrator memory */

	#define LLF_COMMON_HW_TMP_SRAM_SRC_LLI_TAB_ADDR	    				0x0100      /* SRAM offset */
	#define LLF_COMMON_HW_TMP_SRAM_DST_LLI_TAB_ADDR	    				0x0200      /* SRAM offset */
	
	/* select the input into HOST memory from integrator memory or from CryptoCore (in direct mode) */
	#define LLF_COMMON_HW_HOST_MEM_INPUT_SELECT_REG_ADDR                0x8010 
	#define LLF_COMMON_HW_HOST_MEM_INPUT_FROM_CRYPTO_MEM_VAL            0
	#define LLF_COMMON_HW_HOST_MEM_INPUT_FROM_INEGRAT_MEM_VAL           1          
	 
	/* select the input into integrator mem from ARM_CPU or from CryptoCore (in direct mode) */ 
	#define LLF_COMMON_HW_INTEGRAT_MEM_INPUT_SELECT_REG_ADDR            0x8014  
	#define LLF_COMMON_HW_INTEGRAT_MEM_INPUT_FROM_CRYPTO_MEM_VAL        0
	#define LLF_COMMON_HW_INTEGRAT_MEM_INPUT_FROM_HOST_MEM_VAL          1          

#endif

/* ********************** Macros ******************************* */

#define LLF_COMMON_HW_WAIT_ON_MEM_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   for(output_reg_val = 0; output_reg_val < 10 ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_COMMON_HW_DOUT_MEM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x1 ); \
}while(0)    

#define LLF_COMMON_HW_WAIT_ON_SRAM_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   for(output_reg_val = 0; output_reg_val < 10 ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_COMMON_HW_DOUT_SRAM_DMA_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val & 0x1 ); \
}while(0)    

#define LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   for(output_reg_val = 0; output_reg_val < 10 ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_COMMON_HW_SRAM_DATA_READY_REG_ADDR , output_reg_val ); \
   }while( !(output_reg_val & 0x1) ); \
}while(0)    

/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif


