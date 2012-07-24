/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 
#ifndef GEN_HWDEFS_H
#define GEN_HWDEFS_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */

#include "host_hw_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % GeneralHwDefs.h    : %
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

/* the definitions of the HASH hardware registers offset above the base address 
   determaind by the user and the relevant bits */
 
#ifdef  DX_PPC_INTEGRATOR_DEBUG
#define GEN_CC_BASE_ADDR                    0x83F00000UL
#else   
#ifdef  DX_ARM_INTEGRATOR_DEBUG 

#ifndef DX_ARM1176
#define GEN_CC_BASE_ADDR                    0xC0000000UL
#else
#define GEN_CC_BASE_ADDR                    0x80000000UL      
#endif
#else


#ifdef DX_CRYS_DSM_FLAG
#define GEN_CC_BASE_ADDR                    0xC0000000UL     
#else
#define GEN_CC_BASE_ADDR                    0xFF700000UL

#endif //DX_CRYS_DSM_FLAG
#endif //   DX_ARM_INTEGRATOR_DEBUG    
#endif //DX_PPC_INTEGRATOR_DEBUG   
      
#define GEN_HW_CLK_STATUS_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_CLK_STATUS_REG_ADDR)        
#define GEN_HW_CLK_ENABLE_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_CLK_ENABLE_REG_ADDR)        
#define GEN_HW_SRAM_DATA_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_SRAM_DATA_REG_ADDR)      
#define GEN_HW_SRAM_ADDR_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_SRAM_ADDR_REG_ADDR)    
#define GEN_HW_SRAM_DATA_READY_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_SRAM_DATA_READY_REG_ADDR)      
#define GEN_HW_HOST_IRR_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_IRR_REG_ADDR)        
#define GEN_HW_HOST_IMR_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_IMR_REG_ADDR)         
#define GEN_HW_HOST_ICR_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_ICR_REG_ADDR)         
#define GEN_HW_HOST_SEP_SRAM_THRESHOLD_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_SRAM_THRESHOLD_REG_ADDR)         
#define GEN_HW_HOST_SEP_BUSY_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_BUSY_REG_ADDR)       
#define GEN_HW_HOST_SEP_LCS_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_LCS_REG_ADDR)      
#define GEN_HW_HOST_CC_SW_RST_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_CC_SW_RST_REG_ADDR)        
#define GEN_HW_HOST_SEP_SW_RST_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_SW_RST_REG_ADDR)      
#define GEN_HW_HOST_FLOW_DMA_SW_INT0_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_FLOW_DMA_SW_INT0_REG_ADDR)                 
#define GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_HOST_GPR0_REG_ADDR)        
#define GEN_HW_HOST_SEP_HOST_GPR1_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_HOST_GPR1_REG_ADDR)         
#define GEN_HW_HOST_SEP_HOST_GPR2_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_HOST_GPR2_REG_ADDR)         
#define GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_SEP_HOST_GPR3_REG_ADDR)          
#define GEN_HW_HOST_HOST_SEP_GPR0_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_HOST_SEP_GPR0_REG_ADDR )         
#define GEN_HW_HOST_HOST_SEP_GPR1_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_HOST_SEP_GPR1_REG_ADDR)         
#define GEN_HW_HOST_HOST_SEP_GPR2_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_HOST_SEP_GPR2_REG_ADDR)          
#define GEN_HW_HOST_HOST_SEP_GPR3_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_HOST_SEP_GPR3_REG_ADDR)         
#define GEN_HW_HOST_HOST_ENDIAN_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_HOST_ENDIAN_REG_ADDR)        
#define GEN_HW_HOST_HOST_COMM_CLK_EN_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_HOST_HOST_COMM_CLK_EN_REG_ADDR)       
#define GEN_HW_CLR_SRAM_BUSY_REG_REG_ADDR	 (GEN_CC_BASE_ADDR + HW_CLR_SRAM_BUSY_REG_REG_ADDR)

/* ********************** Macros ******************************* */
  
/************************ Enums ********************************/

#ifdef __cplusplus
}
#endif

#endif

