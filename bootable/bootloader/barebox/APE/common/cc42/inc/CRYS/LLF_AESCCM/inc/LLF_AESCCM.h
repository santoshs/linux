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
 
 
#ifndef LLF_AESCCM_H
#define LLF_AESCCM_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_AESCCM.h"
#include "LLF_AESCCM_error.h"
#include "LLF_AES.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  25 Sept. 2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AESCCM.h#1:incl:1
   *  \author R.Levin
   */


/************************ Defines ******************************/

/* Defining the minimum number of blocks that will activate a DMA operation.
   Less from this number the AESCCM will work on the direct mode, since the DMA 
   is not efficient in this case */  
#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_AESCCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_AES_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_AESCCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION  (512/CRYS_AES_BLOCK_SIZE_IN_BYTES)
#endif


/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions ******************************/

/****************************************************************************************/
/*                           LLF AESCCM Functions (non SEP)                             */
/****************************************************************************************/

/*********************************************************************************************/
/**
* @brief The LLF_AESCCM_BlockAdata function processes the AESCCM-MAC operation on block of Additional data.
*         
* @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in LLF_AESCCM_error.h
*/

CRYSError_t  LLF_AESCCM_BlockAdata( AESCCM_Context_t    *WorkingContextID_ptr,    
									DxUint8_t           *DataIn_ptr,                     
									DxUint32_t           DataInSize );

/*********************************************************************************************/
/**
* @brief The LLF_AESCCM_BlockTextData function processes the AESCCM-MAC operation on block of Additional data.
*         
* @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
  *
  * @return CRYSError_t - On success CRYS_OK is returned, on failure - 
  *                       a value defined in LLF_AESCCM_error.h
*/

CRYSError_t  LLF_AESCCM_BlockTextData( 
									AESCCM_Context_t    *WorkingContextID_ptr,
									DxUint8_t           *DataIn_ptr,
									DxUint32_t           DataInSize,
									DxUint8_t           *DataOut_ptr);


/*********************************************************************************************/
/**
* @brief The LLF_AESCCM_BlockLastTextData function processes the AESCCM-MAC operation on block of Additional data.
*         
* @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
* @param[in] DataOutSize - The size of the output buffer.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in LLF_AESCCM_error.h
*/

CRYSError_t  LLF_AESCCM_BlockLastTextData( 
										AESCCM_Context_t    *WorkingContextID_ptr,
										DxUint8_t           *DataIn_ptr,
										DxUint32_t           DataInSize,
							    		DxUint8_t           *DataOut_ptr,
									    DxUint32_t           DataOutSize );


 

/****************************************************************************************/
/*                           LLF AESCCM SEP Functions                                   */
/****************************************************************************************/


/*********************************************************************************************/
/**
* @brief The LLF_AESCCM_SEP_BlockAdata function processes the AESCCM-MAC operation on block of Additional data.
*         
* @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
* @param[in] tailData_ptr  - The pointer to the last bytes of the data, not included in LLI table.
* @param[in] tailDataSize  - The size of last bytes of Adata, not included in LLI table. 
* @param[in] inLliTab_ptr  -  The pointer to the input MLLI table.
* @param[in] inLliTabSize  -  The size of the input MLLI table.
* @param[in] DataInSize    -  The size of the buffer the user shall operate on.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in LLF_AESCCM_error.h
*/

CRYSError_t  LLF_AESCCM_SEP_BlockAdata( AESCCM_Context_t    *WorkingContextID_ptr,    
									    DxUint32_t          *tailData_ptr,         /* last not full AES block of data*/  
									    DxUint32_t           tailDataSize,         /* ltail size */
									    DxUint32_t          *DataInLliTab_ptr,     /* the data in LLI table - in */
									    DxUint32_t           InLliTabSize,         /* the in LLI table size - in */  
									    DxUint32_t           lliDataInSize );      /* size of data in LLI table */


/*********************************************************************************************/
/**
* @brief The LLF_AESCCM_SEP_BlockTextData function processes the AESCCM-MAC operation on block of 
*        Additional data.
 *        1. The function gets a semaphore to protect the access to the hardware 
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address 
 *           to the registers virtual address.
 *        3. The function calls the LLF_AESCCM_BlockInit to initialize the AES hardware.
 *        4. According to operation mode the function calls the LLF_AES_BlockEcbCbcCtrModesSepMLLI and 
 *           LLF_AES_BlockMacModesSepMLLI functions to execute the AESCCM operation on MLLI DMA mode.
 *        5. The function calls the LLF_AESCCM_BlockFinish function to save 
 *           the IV temporary values for the next block operation and to shut down the hardware machine.
 *        6. The function calls a VOS system call to un map the registers virtual address mapping.
 *        7. Releases the "crypto hardware busy" semaphore acquired in paragraph 1.
 *        8. Exits the function.
 *
 *
* @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
* @param[in] inLliTab_ptr  -  The pointer to the input MLLI table.
* @param[in] inLliTabSize  -  The size of the input MLLI table.
* @param[in] outLliTab_ptr -  The pointer to the output MLLI table.
* @param[in] outLliTabSize -  The size of the output MLLI table.
* @param[in] lliDataInSize    -  size of data included in MLLI table;
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure - 
*                       a value defined in LLF_AESCCM_error.h
*/

CRYSError_t  LLF_AESCCM_SEP_BlockTextData( 
									  AESCCM_Context_t    *WorkingContextID_ptr,
									  DxUint32_t          *inLliTab_ptr,         /* the data in LLI table - in */
									  DxUint32_t           inLliTabSize,         /* the in LLI table size - in */  
									  DxUint32_t          *outLliTab_ptr,        /* the data out LLI table - in */
									  DxUint32_t           outLliTabSize,        /* the out LLI table size - in */  
									  DxUint32_t           lliDataInSize );       /* size of data in LLI table */
			                      
			                      
/*********************************************************************************************/
/**
* @brief The LLF_AESCCM_SEP_BlockLastTextData function processes the AESCCM-MAC operation on 
*        last block of Additional data.
*         
* @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
* @param[in] tailData_ptr - The pointer to the last bytes of the data, not included in LLI table.
* @param[in] tailDataSize - The size of last bytes of Adata, not included in LLI table. 
* @param[in] inLliTab_ptr  -  The pointer to the input MLLI table.
* @param[in] inLliTabSize  -  The size of the input MLLI table.
* @param[in] outLliTab_ptr -  The pointer to the output MLLI table.
* @param[in] outLliTabSize -  The size of the output MLLI table.
* @param[in] dataInSize    -  size of data included in MLLI table;

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in LLF_AESCCM_error.h
*/
CRYSError_t  LLF_AESCCM_SEP_BlockLastTextData( 
									AESCCM_Context_t    *WorkingContextID_ptr,
									DxUint32_t          *tailData_ptr,
									DxUint32_t           tailDataSize,
									DxUint32_t          *inLliTab_ptr,        
									DxUint32_t           inLliTabSize,         
									DxUint32_t          *outLliTab_ptr,
									DxUint32_t           outLliTabSize,
									DxUint32_t           lliDataInSize );     
									    


#ifdef __cplusplus
}
#endif

#endif


