/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"


#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "LLF_COMMON_error.h"
#include "LLF_COMMON_HwDefs.h"

/************************ Defines **********************************/

/* NOTE:  Not change order of next two defines ! */

/* for debug printing LLI table from SRAM define the next statement as 1, else as 0 */
#define LLF_COMMON_DEBUG_PRINT_LLI_TABLE   0


/************************ Enums ************************************/

/************************ Typedefs *********************************/

/************************ Global Data ******************************/

/************************ Public functions *************************/



#ifdef DX_CC5_SEP_PLAT
#ifndef CRYS_SEP_SIDE_WORK_MODE /* This condition is set because the next function is used when operating
                                   CC from HOST only and not used for SEP operating.
                                   The defintion is placed in project properties */

/***************************************************************************************************/
/**
 * @brief The  LLF_COMMON_CopyLliTabToSRAM() function copies LLI table from HOST memory
 *        into SRAM of Crypto Core and copies the data pointed by this table into integrator memory.
 *
 *        The function is used in ARM Debug mode only:
 *
 *
 * @param[in] LliTab_ptr - The pointer to the source LLI table in HOST RAM, containing pointers
 *                            and sizes of chuncks of source data.
 * @param[in] LliTabSize -   The size of the source LLI table in words.
 * @param[in] DataSize  -  The count of bytes to copy.
 * @param[in] IntegrRamData_ptr  -  The integrator RAM start address for data.
 * @param[in] LliTabSramAddr - The SRAM address of updated LLI.
 * @param[in] VirtualHwBaseAddr - The HW RAM base address.
 *
 * @return - No return value
 *
 * NOTE: 1. Because the function is intended for internal using, it is presumed that all input parameters
 *          are valid.
 */
 void  LLF_COMMON_CopyLliTabToSRAM(
		                           DxUint32_t   *LliTab_ptr,
		                           DxUint32_t    LliTabSize,
		                           DxUint32_t    DataSize,
		                           DxUint8_t    *IntegrRamData_ptr,
		                           DxUint32_t    LliTabSramAddr,
		                           DxUint32_t    VirtualHwBaseAddr )
 {

   /* FUNCTION DECLARATIONS */

   /* temp LLI word0, word1 */
   DxUint32_t  TempLliWord0;

   /* curent LLI memory chunck word number and size of currently used chunck */
   DxUint32_t  i;

   /* FUNCTION LOGIC */


  /************** copy LLI table from HOST to SRAM ********************/


   #ifdef DX_ARM_INTEGRATOR_DEBUG

	   /* initialize temp LliWord0 to point to beginning of data copied into Integrator memory */
	   TempLliWord0 =  (DxUint32_t)IntegrRamData_ptr;

   #else
        IntegrRamData_ptr=IntegrRamData_ptr;
        DataSize=DataSize;

       /* copy LLI word0 from original table, because it is not needed to copy the data */
       TempLliWord0 = LliTab_ptr[0];

   #endif


   /* write LLI start address into SRAM address register  */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_ADDR_REG_ADDR,
                                LliTabSramAddr );

   for(i = 0; i < LliTabSize; i = i + 2)
   {
       /***** write LLI current word0 (address word) into SRAM *****/
       CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR,
                                    TempLliWord0);
       /* wait when SRAM is ready (after reading word0) */
       LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);


   #ifdef DX_ARM_INTEGRATOR_DEBUG

       /* in case ARM debug calculate new value of the next LLI word0 */
       TempLliWord0 = TempLliWord0 + (LliTab_ptr[i+1] & (0xFFFFFFFF >> (32-LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS)));

   #else
       /* copy LLI word0 from original table */
       TempLliWord0 = LliTab_ptr[i+2];

   #endif


       /***** write current LLI word1 (size word) in to SRAM *****/
       CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR ,
                                    LliTab_ptr[i+1] );

       /* wait when SRAM is ready (after writing word1) */
       LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);
   }


#ifdef DX_ARM_INTEGRATOR_DEBUG
#if    LLF_COMMON_DEBUG_PRINT_LLI_TABLE /* defined in beginning of this file */

   /*-------------------------------------------------------------*/
   /*                  Debug printing                             */
   /*-------------------------------------------------------------*/
  {
       /* temp LLI word1 */
       DxUint32_t  TempLliWord1;

	   /* write LLI table address into SRAM address register (in test LliTabSramAddr= 0x0000) */
	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_ADDR_REG_ADDR,
	                                LliTabSramAddr );
	   /* Dummy read from SRAM */
	   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR,
	                               TempLliWord0);

	   /* wait when SRAM is ready (after duumy read) */
	   LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);


	   for(i = 0; i < LliTabSize; i = i + 2)
	   {
	       /***** read LLI current word0 (address word) from SRAM *****/
	       CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR,
	                                   TempLliWord0);
	       /* wait when SRAM is ready (after reading word0) */
	       LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);

	       PLAT_LOG_DEV_PRINT(( 0 , " LLI_Word[%d]= %08X  \n", i, TempLliWord0 ));

	       /***** read LLI current word1 (address word) from SRAM *****/
	       CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR,
	                                   TempLliWord1);
	       /* wait when SRAM is ready (after reading word1) */
	       LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);

	       PLAT_LOG_DEV_PRINT(( 0 , " LLI_Word[%d]= %08X  \n", i+1, TempLliWord1 ));

	   }

  }

#endif /* LLF_COMMON_DEBUG_PRINT_LLI_TABLE */
#endif /* DX_ARM_INTEGRATOR_DEBUG */

   return;


}/* END OF LLF_COMMON_CopyLliTabToSRAM */

#endif   /* CRYS_SEP_SIDE_WORK_MODE */
#endif   /* DX_CC5_SEP_PLAT */


/***************************************************************************************************/





#ifdef DX_ARM_INTEGRATOR_DEBUG  /* This condition is set because next functions are used
                                  in ARM Debug mode only.
                                  The definition is placed in CRYS_test_flags.h file */

/***************************************************************************************************/
/**
 * @brief The  LLF_COMMON_CopyLliDataHostToIntegr() function copies data pointed by LLI table from HOST memory
 *        into ARM integrator memory.
 *
 *        The function is used in ARM Debug mode only:
 *
 *
 * @param[in] LliTab_ptr - The pointer to the source LLI table in HOST RAM, containing pointers
 *                            and sizes of chuncks of source data.
 * @param[in] LliTabSize -   The size of the source LLI table in words.
 * @param[in] DataSize  -  The count of bytes to copy.
 * @param[in] IntegrRamData_ptr  -  The integrator RAM start address for data.
 * @param[in] LliTabSramAddr - The SRAM address of updated LLI.
 * @param[in] VirtualHwBaseAddr - The HW RAM base address.
 *
 * @return - No return value
 *
 * NOTE: 1. Because the function is intended for internal using, it is presumed that all input parameters
 *          are valid.
 */
 void  LLF_COMMON_CopyLliDataHostToIntegr(
                                         DxUint32_t   *LliTab_ptr,
                                         DxUint32_t    LliTabSize,
                                         DxUint32_t    DataSize,
                                         DxUint8_t    *IntegrRamData_ptr,
                                         DxUint32_t    LliTabSramAddr,
                                         DxUint32_t    VirtualHwBaseAddr )
 {

   /* FUNCTION DECLARATIONS */

   /* source address pointer */
   DxUint8_t  *Src_ptr = DX_NULL;

   /* curent LLI memory chunck word number and size of currently used chunck */
   DxInt32_t  i;
   DxUint32_t ChunckSize;


   /* FUNCTION LOGIC */

    /* initialize the memory select registers to: IntegratorMem->HOST and HOST->IntegratorMem */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_HOST_MEM_INPUT_SELECT_REG_ADDR/*0xC0008010UL */,
                                (DxUint32_t)LLF_COMMON_HW_HOST_MEM_INPUT_FROM_INEGRAT_MEM_VAL /*1UL*/ );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_INTEGRAT_MEM_INPUT_SELECT_REG_ADDR/*0xC0008014UL */,
                                (DxUint32_t)LLF_COMMON_HW_INTEGRAT_MEM_INPUT_FROM_HOST_MEM_VAL /*1UL*/ );

   /* copy data */
   for(i = 0; i < LliTabSize; i = i + 2)
   {
        /* calculate current chunck size */
       ChunckSize = LliTab_ptr[i+1]  & (0xFFFFFFFF >> (32-LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS));

       /* current chunck address in HOST RAM */
	   Src_ptr = (DxUint8_t*)LliTab_ptr[i];

       /* Copy the current chunck of data into integrator RAM */
       DX_VOS_FastMemCpy(IntegrRamData_ptr, Src_ptr,ChunckSize);

       /* update destination pointer */
       IntegrRamData_ptr = IntegrRamData_ptr + ChunckSize;
   }

   /* initialize the memory select registers to: SRAM->HOST and HOST->SRAM  */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_HOST_MEM_INPUT_SELECT_REG_ADDR  /*0xC0008010UL */,
                                (DxUint32_t)LLF_COMMON_HW_HOST_MEM_INPUT_FROM_CRYPTO_MEM_VAL /*0UL*/ );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_INTEGRAT_MEM_INPUT_SELECT_REG_ADDR  /*0xC0008014UL */,
                                (DxUint32_t)LLF_COMMON_HW_INTEGRAT_MEM_INPUT_FROM_CRYPTO_MEM_VAL /*0UL*/ );


   return;


}/* END OF LLF_COMMON_CopyLliDataHostToIntegr */



/***************************************************************************************************/
/**
 * @brief The  LLF_COMMON_CopyLliDataIntegrToHOST() function copies result data from ARM integrator
 *        memory into HOST memory according to LLI table.
 *
 *        The function is used in ARM Debug mode only:
 *
 *
 * @param[in] LliTab_ptr - The pointer to the source LLI table in HOST RAM, containing pointers
 *                            and sizes of chuncks of source data.
 * @param[in] LliTabSize -   The size of the source LLI table in words.
 * @param[in] DataSize  -  The count of bytes to copy.
 * @param[in] IntegrRamData_ptr  -  The integrator RAM start address for data.
 * @param[in] VirtualHwBaseAddr - The HW RAM base address.
 *
 * @return - No return value
 *
 * NOTE: 1. Because the function is intended for internal using, it is presumed that all input parameters
 *          are valid.
 */
 void  LLF_COMMON_CopyLliDataIntegrToHOST(
                                         DxUint32_t   *LliTab_ptr,
                                         DxUint32_t    LliTabSize,
                                         DxUint32_t    DataSize,
                                         DxUint8_t    *IntegrRamData_ptr,
                                         DxUint32_t    VirtualHwBaseAddr )
 {

   /* FUNCTION DECLARATIONS */

   /* source address pointer */
   DxUint8_t  *Src_ptr = DX_NULL;

   /* curent LLI memory chunck word number and size of currently used chunck */
   DxInt32_t  i;
   DxUint32_t ChunckSize;


   /* FUNCTION LOGIC */

   /************* copy LLI data from ARM integrator memory to HOST  ************/

   /* initialize the memory select registers to: IntegratorMem->HOST and HOST->IntegratorMem */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_HOST_MEM_INPUT_SELECT_REG_ADDR  /*0xC0008010UL */,
                                (DxUint32_t)LLF_COMMON_HW_HOST_MEM_INPUT_FROM_INEGRAT_MEM_VAL /*1UL*/ );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_INTEGRAT_MEM_INPUT_SELECT_REG_ADDR  /*0xC0008014UL */,
                                (DxUint32_t)LLF_COMMON_HW_INTEGRAT_MEM_INPUT_FROM_HOST_MEM_VAL /*1UL*/ );

   /* copy data */
   for(i = 0; i < LliTabSize; i = i + 2)
   {
        /* calculate current chunck size */
       ChunckSize = LliTab_ptr[i+1]  & (0xFFFFFFFF >> (32-LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS));

       /* current chunck address in HOST RAM */
	   Src_ptr = (DxUint8_t*)LliTab_ptr[i] ;

       /* Copy the current chunck of data into integrator RAM */
       DX_VOS_FastMemCpy(Src_ptr, IntegrRamData_ptr, ChunckSize);

       /* update destination pointer */
	   IntegrRamData_ptr = IntegrRamData_ptr + ChunckSize;
   }

   /* initialize the memory select registers to: SRAM->HOST and HOST->SRAM  */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_HOST_MEM_INPUT_SELECT_REG_ADDR  /*0xC0008010UL */,
                                (DxUint32_t)LLF_COMMON_HW_HOST_MEM_INPUT_FROM_CRYPTO_MEM_VAL /*0UL*/ );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_INTEGRAT_MEM_INPUT_SELECT_REG_ADDR  /*0xC0008014UL */,
                                (DxUint32_t)LLF_COMMON_HW_INTEGRAT_MEM_INPUT_FROM_CRYPTO_MEM_VAL /*0UL*/ );

   return;


}/* END OF LLF_COMMON_CopyLliDataIntegrToHOST */




/***************************************************************************************************/
/**
 * @brief The  LLF_COMMON_DebugDelay() function performs delay for ensure, that some HW machines are
 *         ended their operations.
 *
 *
 *        The function is used in ARM Debug mode only:
 *
 *
 * @param[in] count - The count of calculation cycles in SW debug delay.
 *
 * @return - No return value
 *
 */
void LLF_COMMON_DebugDelay( DxUint32_t count )
{
	volatile DxInt32_t i;

	for(i = 0; i < 2*count; i++)

	   i++;
}


#endif /* DX_ARM_INTEGRATOR_DEBUG */
