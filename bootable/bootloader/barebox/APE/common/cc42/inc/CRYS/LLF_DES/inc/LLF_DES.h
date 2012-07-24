/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
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
 
 
#ifndef LLF_DES_H
#define LLF_DES_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "LLF_DES_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:48 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_DES.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/
/* defining the minimum number of blocks that will activate a DMA operation.
   less from this number the AES will work on the direct mode , since it is not
   efficient on real time */  

#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_DES_MIN_DES_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_DES_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_DES_MIN_DES_BLOCKS_FOR_DMA_OPERATION  (512/CRYS_DES_BLOCK_SIZE_IN_BYTES)
#endif


/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/*********************************************************************************************************/
/***********                           Public Functions                                        ***********/
/*********************************************************************************************************/


/************************************************************************************************/
/**
 * @brief The low level LLF_DES_Block function operates the DES engine in the hardware.
 *        It calls the following low level functions:
 *         
 *        1. The function gets a semaphore to protect the access to the hardware 
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address 
 *           to the registers virtual address.
 *        3. The function calls the LLF_DES_BlockInit to initialize the DES hardware.
 *        4. If DataInSize is enough large or at least one of the data In-Out pointers is 
 *           in CSI state, then calls the LLF_DES_DMA_BlockECBandCBCModes function and go 
 *           to step 8. 
 *        5. The function calls the LLF_DES_BlockExecFirstOperation to execute the first operation.
 *        6. The function calls the level 3 LLFCD_DES_BlockECBandCBCModes implemented block operation 
 *           if there is more then 2 block to work on. 
 *           If there is 2 block left or less then the function will continue to step 6. 
 *
 *        7. If there are 2 blocks to work on the function will call the LLF_DES_BlockSingle function. 
 *           If only one block is left it will continue to step 7.
 *
 *        8. The function calls the LLF_DES_BlockFinish function to execute the last block (if needed) 
 *           and to save the IV temporary values for the next block operation and to shut down 
 *           the hardware machine.
 *        9. The function calls a VOS system call to un map the registers virtual address mapping.
 *       10. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *       11. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The DES context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_DES_Block( DESContext_t  *WorkingContextID_ptr,    
                              DxUint8_t       *DataIn_ptr,                     
                              DxUint32_t       DataInSize,                      
                              DxUint8_t       *DataOut_ptr ); 
                              

/************************************************************************************************/
/**
 * @brief The low level LLF_DES_InitInit initializes the spesific low level initializatios required
 *        on the spesific platform.
 *
 *        On the lite platform currently this function does nothing.         
 *         
 * @param[in] WorkingContextID_ptr - The DES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_DES_InitInit( DESContext_t  *WorkingContextID_ptr );   




#ifdef __cplusplus
}
#endif

#endif


