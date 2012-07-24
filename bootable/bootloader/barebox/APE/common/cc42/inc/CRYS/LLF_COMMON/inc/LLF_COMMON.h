
#ifndef LLF_COMMON_H
#define LLF_COMMON_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "LLF_COMMON_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  02 May 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief Contains macro and functions definitions used in LLF in different modules.
   *
   *  \version LLF_COMMON.h#1:incl:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines *************************************/


/** 
 * @brief The  LLF_COMMON_HW_WAIT_ON_READY_BIT macros performs waiting loop for ensure,  
 *         that some HW machine is ended their operations.
 * 
 *
 *        The macros is wait till the value in the ReadyRegister is 1.
 *
 *
 * @param[in] VirtualHwBaseAddr - The virtual base address for HW registers. 
 * @param[in] LLF_HW_ReadyRegisterAddress - Relative address of ReadyRegister. 
 *
 */
#define LLF_COMMON_HW_WAIT_ON_READY_BIT( VirtualHwBaseAddr, LLF_HW_ReadyRegisterAddress ) \
do \
{ \
   volatile DxUint32_t output_reg_val; \
   for(output_reg_val = 0; output_reg_val < 10 ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + (LLF_HW_ReadyRegisterAddress) , output_reg_val ); \
   }while( !(output_reg_val & 0x1) ); \
}while(0)    


/**********************************************************************/
/** 
 * @brief The  LLF_COMMON_ARM_DebugMemCpy macros copies data from ARM integrator 
 *        to RAM and back.
 *
 * @param[in] Dst_ptr - Destination address. 
 * @param[in] Src_ptr - Source address.
 * @param[in] SizeBytes - data size in bytes. 
 *
 */
#define LLF_COMMON_ARM_DebugMemCpy( Dst_ptr, Src_ptr, SizeBytes ) \
   \
	CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 1UL ); \
	CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 1UL ); \
    DX_VOS_FastMemCpy( (Dst_ptr) , (Src_ptr) , (SizeBytes) ); \
	CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 0UL ); \
	CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 0UL ); 


/************************ Enums ****************************************/

/************************ Typedefs  ************************************/

/************************ Structs  *************************************/

/************************ Public Variables *****************************/

/************************ Public Functions *****************************/


/**
 * @brief The low level LLF_COMMON_Init function initialisez the HW components common to al modules
 *         
 *
 * @param[in] param1 -  
 * @param[in] param2_ptr -  
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t  LLF_COMMON_Init(DxUint32_t   param1 , void* param2_ptr);

/**
 * @brief The low level LLF_COMMON_Bypass_Block function activates thye DMA to 
 *        copy data in to data out without changing it..
 *          It calls the following low level functions:
 *         
 *        1. The function gets a semaphore to protect the access to the hardware 
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address 
 *           to the registers virtual address.
 *        3. The function configures the HW to be in bypass mode
 *        4. The function calls the LLF_COMMON_Bypass_Activate_Dma to to the DMA action.
 *        5. The function calls a VOS system call to un map the registers virtual address mapping.
 *        6. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *
 *
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_COMMON_Bypass_Block(DxUint8_t       *DataIn_ptr,                     
                              DxUint32_t       DataInSize,                      
                              DxUint8_t       *DataOut_ptr );
                              

/**
 * @brief The low level LLF_COMMON_SetCCToBypass function configures the CC/CF to bypass  
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_COMMON_SetCCToBypass( void );
                              
 
#ifndef CRYS_SEP_SIDE_WORK_MODE /* because the next function is used when operating CC from HOST only
                                 and not used for SEP operating. 
                                 The defintion is placed in project properties */

 
/***************************************************************************************************/
/** 
 * @brief The  LLF_COMMON_CopyLliTabToSRAM() function copies LLI table from HOST memory  
 *        into SRAM of Crypto Core and copies the data pointed by this table into integrator memory.
 *
 *        The function is used in !CRYS_SEP_SIDE_WORK_MODE only:
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
		                           DxUint32_t    VirtualHwBaseAddr );
		                           
#endif	/* CRYS_SEP_SIDE_WORK_MODE */	                           
		                           

#ifdef DX_ARM_INTEGRATOR_DEBUG	/* defintion in project properties */
	                           
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
                                         DxUint32_t    VirtualHwBaseAddr );		                                       
                                         
                                         
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
                                         DxUint32_t    VirtualHwBaseAddr );                                                   
                                         


#endif /* DX_ARM_INTEGRATOR_DEBUG */

/***************************************************************************************************/
/**
 * @brief This macro is used for waiting to end of CSI machine Read. 
 *
 *        The macro function executes a polling on CSI_Status value until
 *        it is not 1.
 *
 *
 * @param[in] - none.
 * 
 * @return DxInt32_t - none
 */

/* defining a macro for waiting to the CSI_READ_FINISH busy register */
#define LLF_COMMON_CSI_READ_FINISH_WAIT( ) \
do \
{ \
   DxUint32_t csi_read_finish_wait_i; \
   volatile CSI_Status_t CSI_Status = CSI_BUSY; \
   for(csi_read_finish_wait_i = 0; csi_read_finish_wait_i < 10 ; csi_read_finish_wait_i++);\
   do \
   { \
      CSI_GetStatus( &CSI_Status ); \
   }while( CSI_Status != CSI_DONE ); \
}while(0)
  
/***************************************************************************************************/
/**
 * @brief This macro is used for waiting to FIFO_IN_EMPTY register. 
 *
 *        The macro function executes a polling on FIFO_IN_EMPTY value until
 *        it is 1.
 *
 *
 * @param[in] - VirtualHwBaseAddr.
 * 
 * @return DxInt32_t - none
 */
/* defining a macro for waiting to the FIFO_IN_EMPTY busy register */
#define LLF_COMMON_CSI_FIFO_IN_EMPTY_WAIT( VirtualHwBaseAddr ) \
do \
{ \
   volatile DxUint32_t output_reg_val;\
   for(output_reg_val = 0; output_reg_val < 20 ; output_reg_val++);\
   do \
   { \
      CRYS_PLAT_SYS_ReadRegister( (VirtualHwBaseAddr) + HW_FIFO_IN_EMPTY_REG_ADDR , output_reg_val ); \
   }while( !output_reg_val ); \
}while(0)  



#ifdef DX_ARM_INTEGRATOR_DEBUG
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
void LLF_COMMON_DebugDelay( DxUint32_t count );


                                        
                                         
#endif  /* DX_ARM_INTEGRATOR_DEBUG */ 


                                         
                              
                           
#ifdef __cplusplus
}
#endif

#endif


