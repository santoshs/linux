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
 
 
#ifndef LLF_HASH_H
#define LLF_HASH_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "LLF_HASH_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %LLF_HASH.h    : %
   *  State           :  %state%
   *  Creation date   :  Mon Nov 29 14:09:42 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_HASH.h#1:incl:4
   *  \author adams
   */




/************************ Defines ******************************/
/* defining the minimum number of blocks that will activate a DMA operation.
   less from this number the HASH will work on the direct mode , since it is not
   efficient on real time */  
#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_HASH_MIN_HASH_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_HASH_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_HASH_MIN_HASH_BLOCKS_FOR_DMA_OPERATION (512/CRYS_HASH_BLOCK_SIZE_IN_BYTES)
#endif

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief The low level LLF_HASH_Update function operates the HASH engine in the hardware.
 *        It calls the appropriate low level functions:
 *         
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param DataIn_ptr a pointer to the buffer that stores the data to be 
 *                       hashed . 
 * @param DataInSize  The size of the data to be hashed in bytes. 
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */

CRYSError_t LLF_HASH_Update(HASHContext_t  *WorkingContextID_ptr,
                            DxUint8_t        *DataIn_ptr,
                            DxUint32_t        DataInSize );

/**
 * @brief The low level LLF_HASH_Finish executes the last data block stored in the previous update context buffer
 *        plus the padding data stored in the context.     
 *
 * @param[in] WorkingContextID_ptr -     The HASH context allocated by the CCM
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */                              
CRYSError_t  LLF_HASH_Finish( HASHContext_t *WorkingContextID_ptr );


/**
 * @brief The low level LLF_HASH_InitContext function initializes the low level context according to the current low level settings
 *         This function is usually called from the CRYS_HASH_Init function 
 *       
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t   LLF_HASH_InitContext(HASHContext_t  *WorkingContextID_ptr);




#ifdef __cplusplus
}
#endif

#endif



