
#ifndef LLFCD_RC4_H
#define LLFCD_RC4_H

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
   *  Object % LLFCD_RC4_H %
   *  State           :  %state%
   *  Creation date   :  12 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief the file containing the functions of the CPU depended functions releated
   *         to the RC4 module. 
   *
   *  \version LLFCD_RC4.h#1:incl:1
   *  \author R.Levin (using as template LLFCD_DES.c created by adams) 
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief This function operates the RC4 stream operation on aligned input data with when the
 *        size is a multiple of 4 bytes.
 *
 *        In order to improve the performance this function may be implemented also in assembler.
 *        Therefore do not change the arguments order !! .             
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_RC4_SteramDirect calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_RC4_SteramDirect calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the RC4 with this function (does not include the
 *                          first and finish operations). NOTE: the size must be a multiple of 4 bytes.
 *
 * @param[in] VirtualHwBaseAddr - The HW base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 *
 *	NOTE:  It is assumed that both DataIn_ptr and DataOut_ptr are aligned when this function is called.
 *         
 */
void LLFCD_RC4_StreamDirect( 
                      DxUint8_t            *DataIn_ptr,         
                      DxUint8_t            *DataOut_ptr ,          
                      DxUint32_t            DataInSize,          
                      DxUint32_t            VirtualHwBaseAddr );               

#ifdef __cplusplus
}
#endif

#endif

