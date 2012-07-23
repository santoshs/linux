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
 
 #ifndef CRYS_HMAC_SEP_H
#define CRYS_HMAC_SEP_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_HASH.h"
#include "CRYS_HMAC.h"
#include "CRYS_Defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_HMAC_SEP.h    : %
   *  State           :  %state%
   *  Creation date   :  17 Apr. 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS HMAC APIs, as well as the APIs themselves
   *
   *  \version CRYS__HMAC.h#1:incl:13
   *  \author R.Levin
   */
 
/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structures ***************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/


/*************************************************************************************************/
/**
 * This function process a HMAC block of data via the HASH Hardware in multi LLI DMA mode.
 *
 * The function receives as input an handle to the  HMAC Context, and operated the HASH update on 
 * the data block, pointed by DMA LLI table.
 * 
 * @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
 *                       is used for the HMAC machine operation.
 *
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 * 
 * @param DataInSize  The size of the data to be hashed in bytes. 
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
CIMPORT_C CRYSError_t CRYS_HMAC_SEP_Update( CRYS_HMACUserContext_t  *ContextID_ptr,
										    DxUint8_t                 *intDataHead_ptr,     
										    DxUint32_t                intDataHeadSize, 
										    DxUint8_t                 *intDataTail_ptr,     
										    DxUint32_t                intDataTailSize, 
			                                DxUint32_t              *DataInLliTab_ptr,     
                                            DxUint32_t               InLliTabEntries, 
                                            DxUint32_t               DataInSize );
                                            
                                            
/*************************************************************************************************/
/**
 * This function provides HASH function to process one buffer of data.
 * The function allocates an internal HASH Context , it initializes the 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 * Then the function loads the Hardware with the initializing values and after 
 * that process the data block using the hardware to do hash .
 * At the end the function return the message digest of the data buffer .
 *
 *
 * @param[in] OperationMode - The operation mode : MD5 or SHA1.
 *
 * @param[in] key_ptr - The pointer to the users key buffer.
 *
 * @oaram[in] keySize - The size of the received key.
 * 
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 * 
 * @param DataInSize  The size of the data to be hashed in bytes. 
 *
 * @retval HashResultBuff a pointer to the target buffer where the 
 *                      HMAC result stored in the context is loaded to.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
CIMPORT_C CRYSError_t CRYS_HMAC_SEP( CRYS_HASH_OperationMode_t  OperationMode,
			                         DxUint8_t                 *key_ptr,
			                         DxUint16_t                 keySize,
			                         DxUint32_t                *DataInLliTab_ptr,     
                                     DxUint32_t                 InLliTabEntries, 
			                         DxUint32_t                 DataSize,
			                         CRYS_HASH_Result_t         HmacResultBuff );                                            

/**
* This function processes a HMAC block of data via the HASH hardware/software.
* The function receives as input a handle to the HMAC Context, 
* and performs a HASH update on the data described below.
* 
* @param[in] ContextID_ptr - A pointer to the HMAC context buffer allocated by the user 
*                       that is used for the HMAC machine operation.
*
* @param DataIn_ptr - A pointer to the buffer that stores the data to be hashed.
* 
* @param DataInSize - The size of the data to be hashed, in bytes. 
*
* @return CRYSError_t - On success the function returns CRYS_OK, 
*			and on failure a non-ZERO error.
*/

CIMPORT_C CRYSError_t CRYS_HMAC_Update_ExtApp(CRYS_HMACUserContext_t  *ContextID_ptr,
											  DxUint8_t                 *DataIn_ptr,
											  DxUint8_t                 *SepDataIn_ptr,
									          DxUint32_t                 DataInSize );


/**
* This function provide HASH function to process one buffer of data.
* The function allocates an internal HASH Context , it initializes the 
* HASH Context with the cryptographic attributes that are needed for 
* the HASH block operation ( initialize H's value for the HASH algorithm ).
* Then the function loads the Hardware with the initializing values and after 
* that process the data block using the hardware to do hash .
* At the end the function return the message digest of the data buffer .
*
*
* @param[in] OperationMode - The operation mode : MD5 or SHA1.
*
* @param[in] key_ptr - The pointer to the users key buffer.
*
* @oaram[in] keySize - The size of the received key.
* 
* @param[in] ContextID_ptr - a pointer to the HMAC context buffer allocated by the user that
*                       is used for the HMAC machine operation.
*
* @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HMAC. The pointer does 
*                         not need to be aligned. On CSI input mode the pointer must be equal to
*                         value (0xFFFFFFFC | DataInAlignment). 
* 
* @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
*                         multiple of HASH_BLOCK_SIZE for used HASH mode.
*
* param[out] HashResultBuff - a pointer to the target buffer where the 
*                      HMAC result stored in the context is loaded to.
*
* @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
*
*/
CIMPORT_C CRYSError_t CRYS_HMAC_ExtApp  ( CRYS_HASH_OperationMode_t  OperationMode,
								  DxUint8_t                    *key_ptr,
								  DxUint16_t                    keySize,
								  DxUint8_t                    *DataIn_ptr,
								  DxUint8_t                    *SepDataIn_ptr,
								  DxUint32_t                    DataSize,
								  CRYS_HASH_Result_t          HmacResultBuff );

#ifdef __cplusplus
}
#endif

#endif
