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
 
 
#ifndef LLF_AES_H
#define LLF_AES_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_AES.h"
#include "LLF_AES_error.h"


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
   *  \version LLF_AES.h#1:incl:1
   *  \author adams
   */




/************************ Defines ******************************/

/* defining the minimum number of blocks that will activate a DMA operation.
   less from this number the AES will work on the direct mode , since it is not
   efficient on real time */ 
#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_AES_MIN_AES_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_AES_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_AES_MIN_AES_BLOCKS_FOR_DMA_OPERATION  (512/CRYS_AES_BLOCK_SIZE_IN_BYTES)
#endif

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions ******************************/


/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_InitInit performs specific initializations required
 *        on the used HW or SW platform for necessary modes (currently only on XCBC is need).
 *
 *        On the lite platform currently this function does nothing.         
 *         
 * @param[in] WorkingContext_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_AES_InitInit( AESContext_t  *WorkingContext_ptr );

/****************************************************************************************/
/**
 * @brief The low level LLF_AES_InitSk initializes the SK using the HASH on the SEED.
 *        not relevant on the HW platform - used as a stub function                 
 *         
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM\
 * @param[in] - seedSizeInBytes the size of the seed in bytes
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_AES_InitSk( DxUint8_t *seed_PTR , DxUint16_t rkekSize ); 


/****************************************************************************************/
/**
 * @brief The low level LLF_AES_Block function operates the AES engine in the hardware.
 *        It calls the following low level functions:
 *         
 *        1. The function gets a semaphore to protect the access to the hardware 
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address 
 *           to the registers virtual address.
 *        3. The function calls the LLF_AES_BlockInit to initialize the AES hardware.
 *        4. The function calls the LLF_AES_BlockExecFirstOperation to execute the first operation.
 *        5. The function calls the level 3 implemented block operation if there is more then 2 
 *           block to work on. 
 *           If there is 2 block left or less then the function will continue to step 6. 
 *
 *             If the operation mode is ECB CBC or CTR the LLFCD_AES_BlockECBandCBCandCTRModes function is called
 *             If the operation mode is MAC, XCBC_MAC or CMAC the LLFCD_AES_BlockMacMode function is called.
 *
 *           These functions as described are level 3 functions implemented according to the major
 *            CPUs in assembler in order to achieve improved performances.
 *
 *        6. If there are 2 blocks to work on the function will call the LLF_AES_BlockSingle function. 
 *           If only one block is left it will continue to step 7.
 *
 *        7. The function calls the LLF_AES_BlockFinish function to execute the last block and to save 
 *           the IV temporary values for the next block operation and to shut down the hardware machine.
 *        8. The function calls a VOS system call to unmap the registers virtual address mapping.
 *        9. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *       10. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_AES_Block( AESContext_t  *WorkingContextID_ptr,    
                              DxUint8_t     *DataIn_ptr,                     
                              DxUint32_t     DataInSize,                      
                              DxUint8_t     *DataOut_ptr ); 
                              

/*********************************************************************************************************/
/** 
 * @brief The LLF_AES_Finish function is used as finish operation of AES on special modes.
 *
 *    The LLF_AES_Finish function perform AES operations on last block of data. 
 *    The function calls appropriate specific LLF AES functions according to AES mode.
 *
 *      1.	Calculates remaining size for specific modes.
 *      2.	If DataSize is enough large, the function calls LLF_AES_Block function for
 *          data size minus remaining size .
 *      3.  Calls appropriate LLF_AES_Finish___ functions according to AES mode.
 *      4.  Exits.
 *
 * @param[in] ContextID_ptr - a pointer to the AES context buffer allocated by the user that
 *                            should be the same context that was used on the previous call
 *                            of this session.
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
 * @return CRYSError_t      - On success CRYS_OK is returned, on failure a
 *                            value MODULE_* CRYS_AES_error.h
 */
 CRYSError_t  LLF_AES_Finish( 
                              AESContext_t    *ccmWorkingContext_ptr,                                       
                              DxUint8_t       *DataIn_ptr,                     
                              DxUint32_t       DataInSize,                      
                              DxUint8_t       *DataOut_ptr );


/****************************************************************************************/
/**
 * @brief These functions are used by the rnd for time optimizations.
 *        
 *      
 * @param[in] VirtualHwBaseAddr - The base address.
 * @param[in] DinDout_ptr - The pointer to the input-output buffer passed by the user.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void LLF_AES_OptAesBlock(DxUint32_t VirtualHwBaseAddr,
                           DxUint32_t *DinDout_ptr);

/****************************************************************************************/
/**
 * @brief These functions are used by the RND for time optimizations.
 *        
 *      
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void LLF_AES_CloseAesEngine(DxUint32_t VirtualHwBaseAddr);


          
#ifdef __cplusplus
}
#endif

#endif


