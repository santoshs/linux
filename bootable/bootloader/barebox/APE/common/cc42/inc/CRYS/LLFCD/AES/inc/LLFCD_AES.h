
#ifndef LLFCD_AES_H
#define LLFCD_AES_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % LLFCD_AES_H %
   *  State           :  %state%
   *  Creation date   :  Tue Nov 23 17:28:52 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief the file containing the functions of the CPU depended functions releated
   *         to the AES module. 
   *
   *  \version LLFCD_AES.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones.
 *
 *        This function should be operated in ECB or CBC mode.    
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .             
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the AES with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_AES_BlockECBandCBCandCTRModes( 
              DxUint8_t            *DataIn_ptr,         
              DxUint8_t            *DataOut_ptr ,          
              DxUint32_t            DataInSize,          
              DxUint32_t           VirtualHwBaseAddr );               


/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones.
 *
 *        This function should be operated in MAC mode.    
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .             
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataInSize  - The exect data size to operate the AES with this function.( does not include the
 *                          first and finish operations )
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_AES_BlockMacMode( 
              DxUint8_t            *DataIn_ptr,          
              DxUint32_t            DataInSize,         
              DxUint32_t           VirtualHwBaseAddr );                


#ifdef __cplusplus
}
#endif

#endif

