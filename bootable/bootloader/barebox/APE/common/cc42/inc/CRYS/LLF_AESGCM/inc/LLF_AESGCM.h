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
 
 
#ifndef LLF_AESGCM_H
#define LLF_AESGCM_H

/*
 * All the includes, that are needed to compile correctly 
 * this module should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "LLF_AES.h"
#include "CRYS_AESGCM.h"
#include "LLF_AESGCM_error.h"



#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  21 March 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AESGCM.h#1:incl:1
   *  \author R.Levin
   */


/************************ Defines ******************************/

/* defining the minimum size and number of blocks that will activate a DMA operation.
   less from this number the AESGCM will work on the direct mode , since it is not
   efficient on real time */  
#ifdef CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA    
	#define LLF_AESGCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION  (CRYS_COMMON_MIN_SIZE_IN_BYTES_FOR_HW_DMA/CRYS_AES_BLOCK_SIZE_IN_BYTES)
#else
	#define LLF_AESGCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION  (512/CRYS_AES_BLOCK_SIZE_IN_BYTES)
#endif	

/************************ Enums ********************************/

/************************ Typedefs  ****************************/


/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions ******************************/

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_Init function calculates Initial GCTR counter value value J0 
*        according to NIST 800-38D.
*         
* @param[in] Context_ptr   - The pointer to AESGCM context.
* @param[in] key_ptr       - The pointer to AESGCM key.
* @param[in] keySizeID     - The key size ID; 
* @param[in] IV_ptr	       - A pointer to initial value (nonce) - unique value assigned to all data,
*                            passed into GCM. 
* @param[in] ivSize        - The size of the user passed IV (in bytes).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a 
*                       value defined in LLF_AESGCM_error.h
*/
CRYSError_t LLF_AESGCM_Init(
							 AESGCMContext_t       *Context_ptr,   /*GCM context*/
							 DxUint8_t             *key_ptr,       /*AESGCM key*/
							 CRYS_AESGCM_KeySize_t  keySizeID,     /*size of key in bytes*/
							 DxUint8_t             *iv_ptr,        /*initial value*/
							 DxUint32_t             ivSize );      /*size of IV in bytes*/
							 

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_GHASH function calculates the GHASH MAC value on block of 
*        Additional data according to NIST 800-38D 6.4.
*         
* @param[in] GcmContext_ptr - The pointer to AESGCM context.
* @param[in] data_ptr - The pointer to input data - bytes array.
* @param[in] size - The input data size in bytes; 
* @param[in] size -  The size of the input data in bytes.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a 
*                       value defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_GHASH(
							  AESGCMContext_t *Context_ptr,
							  DxUint8_t       *DataIn_ptr,  
							  DxUint32_t       DataInSize );        

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_BlockTextData function processes the GCM authenticated  
*        encryption/decryption on block of Text data.
*         
* @param[in] GcmContext_ptr - The AESGCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value 
*                       defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_TextData( 
                         AESGCMContext_t     *Context_ptr,
						 DxUint8_t           *DataIn_ptr,
						 DxUint32_t           DataInSize,
						 DxUint8_t           *DataOut_ptr);

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_Finish function processes the GCM authenticated  
*        encryption/decryption on last block of the Text data.
*         
* @param[in] GcmContext_ptr - The AESGCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value 
*                       defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_Finish( 
                         AESGCMContext_t     *Context_ptr,
						 DxUint8_t           *DataIn_ptr,
						 DxUint32_t           DataInSize,
						 DxUint8_t           *DataOut_ptr );



#ifdef __cplusplus
}
#endif

#endif


