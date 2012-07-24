/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
#ifndef __FVOS_CONFIG_H__
#define __FVOS_CONFIG_H__


/*
   *  Object %name    : FVOS_Config.h
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief FVOS h file (API and structures )
   *
   *  \version 
   *  \author yevgenys
   */




/*------------------------------
    DEFINES
--------------------------------*/

/* memory for the system data pool */
#define FVOS_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES   100

/* size of message memory area in bytes */
#define FVOS_MESSAGE_MEMORY_SIZE_IN_BYTES       (8 * 1024)

/* size of static memory area in bytes */
#define FVOS_STATIC_MEMORY_SIZE_IN_BYTES        (4 * 1024)

/* the size of the data pool shared area in bytes */
#define FVOS_SYSTEM_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES   (16 * 1024)

/* the size of the synchronic DMA tables in bytes */
#define FVOS_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES   (5 * 1024)

/* system data (time, caller id etc') pool */
#define FVOS_SYSTEM_MEMORY_AREA_SIZE_IN_BYTES   (3 * 1024)

/* token that always precedes valid time value */
#define FVOS_TIME_VAL_TOKEN                     0x12345678

/* the token that defines the start of time address */
#define SEP_RAR_VAL_TOKEN                       0xABABABAB

/* offset of the num entries in the block length entry of the LLI */
#define FVOS_NUM_ENTRIES_OFFSET_IN_BITS         24

/* offset of the interrupt flag in the block length entry of the LLI */
#define FVOS_INT_FLAG_OFFSET_IN_BITS            31

/* mask for extracting data size from LLI */
#define FVOS_TABLE_DATA_SIZE_MASK               0xFFFFFF

/* mask for entries after being shifted left */
#define FVOS_NUM_ENTRIES_MASK                   0x7F

/* maximum message of the add tables in words */
#define FVOS_MAX_ADD_MESSAGE_LENGTH_IN_WORDS    11

/* maximum message of the add tables in bytes */
#define FVOS_MAX_ADD_MESSAGE_LENGTH_IN_BYTES    (FVOS_MAX_ADD_MESSAGE_LENGTH_IN_WORDS << 2)  
              
/* the size in bytes of the time memory */
#define FVOS_TIME_MEMORY_SIZE_IN_BYTES                  8

/* the size in bytes of the RAR parameters memory */
#define FVOS_SYSTEM_RAR_MEMORY_SIZE_IN_BYTES            8


/************************ Defines ******************************/
#ifdef DX_PPC_INTEGRATOR_DEBUG
    /* SEP ROM write enabling register address */
   #define  SEP_ROM_BANK_register		    0x83f08420UL
   
   /* SEP ROM start address */    		 
   #define  CRYS_SEP_ROM_start_address		0x83f0C000UL
   
   #define ENV_CC_RST_N_REG_address         0x83f080A8UL
   #define ENV_CC_POR_N_ADDR_REG_address    0x83f080E0UL
   #define  CRYS_SEP_ROM_length		        0x4000
   #define DX_CC_BASE_ADDR 					0x83f04100
#endif //DX_PPC_INTEGRATOR_DEBUG

#ifdef DX_ARM_INTEGRATOR_DEBUG
#ifndef DX_ARM1176	 
 	 /* SEP ROM length in words and start address */ 	 
   #define  CRYS_SEP_ROM_length		        0x4000
   /* SEP ROM start address */    		 
   #define  CRYS_SEP_ROM_start_address		0xC0010000UL

   /* SEP ROM control address */    		 
   #define  CRYS_SEP_ROM_control_address	0xC0020004UL
   
   #define ENV_CC_RST_N_REG_address         0xC00080A8UL
   #define ENV_CC_POR_N_ADDR_REG_address    0xC00080E0UL
   
   #define DX_CC_BASE_ADDR 					0xC0020100
   
   #define HOST_SC_LOC      				0x1100001C
   #define HOST_SC_OSC      				0x11000004
   
   #define CLK_5MHZ    						0x0C
   #define CLK_10MHZ   						0x20
   #define CLK_20MHZ   						0x48
   
#else   
   /* SEP ROM write enabling register address */
   #define  SEP_ROM_BANK_register		    0x80008420UL
   
   /* SEP ROM start address */    		 
   #define  CRYS_SEP_ROM_start_address		0x8000C000UL
   
   #define ENV_CC_RST_N_REG_address         0x800080A8UL
   #define ENV_CC_POR_N_ADDR_REG_address    0x800080E0UL
   
   #define DX_CC_BASE_ADDR 					0x80004100

#ifndef DX_ARM1176_CC5
   /* SEP ROM length in words and start address */ 	 
   #define  CRYS_SEP_ROM_length		        0x2000
#else
   /* SEP ROM length in words and start address */ 	 
   #define  CRYS_SEP_ROM_length		        0x4000
#endif
#endif // DX_ARM1176
#endif // DX_ARM_INTEGRATOR_DEBUG


#define FVOS_WriteRegister(addr,val) \
{ \
  ( (*((volatile DxUint32_t*)(addr - g_FVOS_DiffRegAddress))) = (DxUint32_t)(val) ); \
}
#define FVOS_ReadRegister(addr,val) \
{ \
  ( (val) = (*((volatile DxUint32_t*)(addr - g_FVOS_DiffRegAddress))) ); \
}


#define FVOS_WAIT_SRAM_WRITE_COMPLETE() \
do \
{ \
  DxUint32_t  WAIT_WRITE_regVal; \
  do \
  { \
    FVOS_ReadRegister( GEN_HW_SRAM_DATA_READY_REG_ADDR, (WAIT_WRITE_regVal) ); \
  }while(!(WAIT_WRITE_regVal & 0x1)); \
}while(0)


/*-------------------------------
  typedefs
---------------------------------*/


typedef struct _FVOS_LliEntry_t
{
  /* physical address */
  DxUint32_t  physicalAddress;
  
  /* block length */
  DxUint32_t  blockLength;
  
}FVOS_LliEntry_t;




typedef enum
{
  DX_CC_START_IllaglResult_t,
  DX_CC_START_fatalError_t,
  DX_CC_START_ColdBootFirstPhaseComplete_t,
  DX_CC_START_ColdBootFirstPhaseCompleteWithWarning_t,
  DX_CC_START_ColdBootComplete_t,
  DX_CC_START_ColdBootCompleteWithWarning_t,
  DX_CC_START_WarmBootComplete_t,
  DX_CC_START_WarmBootCompleteWithWarning_t,
  DX_CC_START_ColdWarmBootComplete_t,
  DX_CC_START_ColdWarmBootCompleteWithWarning_t,
  DX_CC_START_ICacheFailure_t,
  DX_CC_START_DCacheFailure_t,
  DX_CC_START_BootLast       = 0x7FFFFFFF,
}DX_CC_START_CCResult_t;


#endif
