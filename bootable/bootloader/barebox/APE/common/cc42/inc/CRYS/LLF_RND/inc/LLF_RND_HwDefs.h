
#ifndef LLF_RND_HW_DEFS_H
#define LLF_RND_HW_DEFS_H

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
   *  Object % LLF_RND_HwDefs.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:40:57 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes the hardware address of the registers relevant to the AES module.
   *
   *  \version LLF_RND_HwDefs.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************************ Defines ******************************/

/* the definitions of the RANDOM generate hardware registers offset above the base address 
   determaind by the user and the relevant bits */

/* sampling interval values for fast-TRNG */
#define LLF_RND_HW_SAMPLE_CNT_MIN_VAL           1UL
#define LLF_RND_HW_SAMPLE_CNT_MAX_VAL           (65535UL - LLF_RND_FAST_HW_SAMPLE_CNT_STEP_VAL)
#define LLF_RND_FAST_HW_SAMPLE_CNT_STEP_VAL     200UL

/* ring oscillator length maximal level */
#define LLF_RND_FAST_HW_RING_OSC_MAX_LEVEL        3UL
#define LLF_RND_HW_RING_OSCILLATOR_LENGTH_MASK    LLF_RND_FAST_HW_RING_OSC_MAX_LEVEL

/* HW_TRNG_DEBUG_CONTROL register value on Fast mode */
#define  LLF_RND_HW_TRNG_DEBUG_CONTROL_Value_ON_FAST_MODE  0x0000000E

/* TRNG configuration register address */
#define LLF_RND_HW_TRNG_CONFIG_REG_ADDR      HW_TRNG_CONFIG_REG_ADDR /*0x10CUL*/

/* register for reading the first random byte data */
#define LLF_RND_HW_RND_DATA_ADDR_0           HW_EHR_DATA_ADDR_0_REG_ADDR      /*0x114UL*/

/* register for reading start addr of random byte data */
#define LLF_RND_HW_RND_DATA_ADDR 			LLF_RND_HW_RND_DATA_ADDR_0

/* The number of words generated in the entropy holding register (EHR) 
   4 or 6 words (128 or 192 bit) according to implementation */	
//#define LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS	/*4*/  6 
#define LLF_RND_HW_TRNG_EHR_WIDTH_IN_BYTES	(LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS * sizeof(DxUint32_t))
/* HW TRNG EHR VALID REG Address */
#define LLF_RND_HW_TRND_BUSY_ADDR 			HW_TRNG_VALID_REG_ADDR

/* sampling interval register address*/
#define LLF_SAMPLE_CNT1_REG_ADDR 		 	HW_SAMPLE_CNT1_REG_ADDR

#define LLF_RND_SOURCE_ENABLE_REG_ADDR 		HW_RND_SOURCE_ENABLE_REG_ADDR
#define LLF_RND_HW_RND_SRC_ENABLE_VAL      	0x0001UL
#define LLF_RND_HW_RND_SRC_DISABLE_VAL     	0x0000UL

#define LLF_RNG_CLK_ENABLE_REG_ADDR 	 	HW_RNG_CLK_ENABLE_REG_ADDR
#define LLF_RND_HW_RND_CLK_ENABLE_VAL      	0x0001UL
#define LLF_RND_HW_RND_CLK_DISABLE_VAL     	0x0000UL

#define LLF_RND_HW_VERSION_ADDR            	HW_VERSION_REG_ADDR 		   /*0x0928UL*/
#define LLF_RND_HW_VERSION_VAL             	0x2200UL

/* the HW address space */
#define LLF_RND_HW_CRYPTO_ADDR_SPACE 		( 1UL << 12 )

#define LLF_RND_HW_CLOCK_ENABLE_MASK 		0x00000040

#define LLF_RND_HW_TRNG_DEBUG_CONTROL_REG_ADDR   HW_TRNG_DEBUG_CONTROL_REG_ADDR

/* HW TRNG ISR and ICR registers addresses */
#define LLF_RND_HW_RNG_ISR_REG_ADDR  HW_RNG_ISR_REG_ADDR /*0x104UL*/
#define LLF_RND_HW_RNG_ICR_REG_ADDR  HW_RNG_ICR_REG_ADDR /*0x108UL*/

/* HW TRNG Errors masks (for ISR and ICR registers*/
#define LLF_RND_HW_TRNG_ERRORS_MASK         0xEUl  /*mask for joined 3 next erros */
#define LLF_RND_HW_RNG_AUTOCORR_ERROR_MASK  0x2Ul  /*Autocorrelation test error*/
#define LLF_RND_HW_RNG_CTRNGT_ERROR_MASK    0x4Ul  /*Continuous test error*/
#define LLF_RND_HW_RNG_VNC_ERROR_MASK       0x8Ul  /*Von Neumann correction error*/

#define LLF_RND_HW_RNG_ISR_ERRORS_MASK      0xEUl  /*EHR valid mask = 4b'1110 */

#ifdef CRYS_SEP_SIDE_WORK_MODE
/*This register is used only in the DSM simulation */
#define HW_SEP_OVERRIDE_REG_ADDR               0x400090ACUl
#endif
#define  DX_SEP_HW_RESET_SEED_OVERRIDE_FLAG    0x2Ul
/*********************** Macros ********************************/
/* This busy loop waits for the valid bit to be set to 0x1 */
#define LLF_RND_HW_WAIT_VALID_EHR_BIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   for(output_reg_val = 0; output_reg_val < 2 ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + LLF_RND_HW_TRND_BUSY_ADDR , output_reg_val ); \
   }while( output_reg_val == 0x0 ); \
}while(0)   
/************************ Enums ********************************/



#ifdef __cplusplus
}
#endif

#endif


