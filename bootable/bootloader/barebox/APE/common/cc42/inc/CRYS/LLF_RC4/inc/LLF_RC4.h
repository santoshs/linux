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
 
 #ifndef CC6_LLF_RC4_H
#define CC6_LLF_RC4_H
  /*
   *  Object %name    :  %CC6_LLF_RC4.h
   *  State           :  %state%
   *  Creation date   :  06 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CC6_LLF_RC4.c#2:csrc:1
   *  \author R.Levin.
   */

/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS_RC4.h"

#ifdef __cplusplus
extern "C"
{
#endif


/************************ Defines ******************************************************/

/* Defining the minimum number of blocks that will activate a DMA operation.
   Less from this number the RC4 will work on the direct mode , since the DMA is not
   efficient on real time in this case */
#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_RC4_MIN_RC4_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_RC4_HW_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_RC4_MIN_RC4_BLOCKS_FOR_DMA_OPERATION (512/CRYS_RC4_HW_BLOCK_SIZE_IN_BYTES)
#endif
     
#define LLF_RC4_MIN_DATA_SIZE_BYTES_FOR_DMA_OPERATION  (LLF_RC4_MIN_RC4_BLOCKS_FOR_DMA_OPERATION/CRYS_RC4_HW_BLOCK_SIZE_IN_BYTES)

/* defining max and min key size according to HW requirements */
#ifndef LLF_RC4_MAX_KEY_SIZE_IN_BYTES
#define  LLF_RC4_MAX_KEY_SIZE_IN_BYTES   5 * sizeof(DxUint32_t)
#endif

#ifndef LLF_RC4_MIN_KEY_SIZE_IN_BYTES
#define  LLF_RC4_MIN_KEY_SIZE_IN_BYTES   1 * sizeof(DxUint32_t)
#endif

/* RC4 HW RAM size in words (includes S-box and additional two words */
#define  LLF_RC4_RAM_SIZE_IN_WORDS   66


/*#define  LLF_DEBUG_HW_WAIT_DELAY*/

/********************** Public function prototypes **************************************/


/****************************************************************************************/
/**
 * @brief This function is used to initialize the RC4 machine engine low level.
 *        This function is called from the CRYS_RC4_Init function.
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer that is allocated by the user 
 *                       and is used for the RC4 machine operation.
 *
 * @param[in] Key_ptr -  A pointer to the user's key buffer.
 *
 * @param[in] KeySize - The size of the KEY in bytes.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                        and on failure a value from CRYS_RC4_error.h
 */

CRYSError_t  LLF_RC4_Init(  RC4Context_t    *ContextID_ptr,
                            DxUint8_t 	   	*Key_ptr, 
                            DxUint32_t 		 KeySizeInBytes);

/************************************************************************************************/
/**
 * @brief The low level LLF_RC4_Stream function operates the RC4 engine in the hardware.
 *
 *        It calls the following low level functions:
 *         
 *        1. The function gets a semaphore to protect the access to the hardware 
 *           from any other process. The semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address 
 *           to the registers virtual address.
 *        3. The function calls the LLF_RC4_StreamInit to initialize the RC4 hardware.
 *        4. According to predefined flags and data size, the function calls appropriate 
 *           LLF_RC4_Stream private function:
 *           - if DataSize < LLF_RC4_MIN_RC4_BLOCKS_FOR_DMA_OPERATION  the function 
 *             calls the LLF_RC4_StreamDirect() function,
 *           - else calls the LLF_RC4_StreamDma() function,
 *           - if data size is not multiple of 4 bytes, then calls LLF_RC4_StreamRemainingData()
 *             function.
 *        5. On the end the function calls the LLF_RC4_StreamFinish function to save 
 *           the RC4 HW specific memory  for next stream operations and to shut down the RC4 machine.
 *        6. The function also calls a VOS system to un map the registers virtual address mapping.
 *        7. Releases the "crypto hardware busy" semaphore acquired in paragraph 1.
 *        8. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The RC4 context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 CRYSError_t  LLF_RC4_Stream(
                             RC4Context_t        *ContextID_ptr,   
                             DxUint8_t           *DataIn_ptr,
                             DxUint8_t           *DataOut_ptr,
                             DxUint32_t           DataInSize );
                 
                 
			                       
			                                       

#ifdef __cplusplus
}
#endif

#endif /* CRYS_NO_RC4_SUPPORT */

