
#ifndef LLFCD_HASH_H
#define LLFCD_HASH_H

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
   *  Object % LLFCD_HASH_H %
   *  State           :  %state%
   *  Creation date   :  Tue Nov 23 17:28:52 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief the file containing the functions of the CPU depended functions releated
   *         to the HASH module. 
   *
   *  \version LLFCD_HASH.h#1:incl:1
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
 * @brief This function operates the update execution of all of the blocks except the first
 *        and last ones ( not including the padding ).
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .             
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_HASH_Updat calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the HASH with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_HASH_Update( 
              DxUint8_t            *DataIn_ptr,          
              DxUint32_t            DataInSize,         
              DxUint32_t           VirtualHwBaseAddr );
              
              
/**
 * @brief This function operates the update execution of all of the blocks except the first
 *        and last ones ( not including the padding ).This function is for SHA-2 algorithm(128 bytes loading)
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .             
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_HASH_Updat calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the HASH with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_HASH_SHA2_Update( 
              DxUint8_t            *DataIn_ptr,          
              DxUint32_t            DataInSize,         
              DxUint32_t           VirtualHwBaseAddr ); 
              
              
/*
 * ==================================================================
 * Function name: LLFCD_HASH_MD5_SingleBlockUpdate
 *
 * Description: This module performs MD5 hashing of 512-bit block.
 *
 * Author: Victor Elkonin
 * 
 * Last Rivision: 1.00.00
 *
 * Update History:
 * Rev 1.00.00, Date 23 September 2004, By Victor Elkonin: Initial version.
 *                   14 December  2004, By Adam Schapira - ported to CRYS. 
 * ========================================================================
 */
void LLFCD_HASH_MD5_SingleBlockUpdate( HASHContext_t *WorkingContextID_ptr, DxUint32_t *DataIn_ptr );
               

#ifdef __cplusplus
}
#endif

#endif

