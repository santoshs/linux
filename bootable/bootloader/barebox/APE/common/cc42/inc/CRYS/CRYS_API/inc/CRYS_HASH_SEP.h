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
 
 #ifndef CRYS_HASH_SEP_H
#define CRYS_HASH_SEP_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_HASH.h"
#include "LLF_HASH_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_HASH_SEP.h    : %
   *  State           :  %state%
   *  Creation date   :  15 Apr. 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions 
   *         that are used for the CRYS HASH APIs, as well as the APIs themseleves
   *
   *  \version CRYS_HASH_SEP.h#1:incl:4
   *  \author R.Levin
   */
 
/************************ Defines ******************************/


/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions **********************/


/************************************************************************************************/
/**
 * This function process a block of data via the HASH Hardware in SEP multi LLI mode.
 *
 * The function receives as input an handle to the  HASH Context , that was initialized before
 * by an CRYS_HASH_Init function or by other CRYS_HASH_Update function. \n
 * The function Sets the hardware with the last H's value that where stored in
 * the CRYS HASH context and then process the data block using the hardware 
 * and in the end of the process stores in the HASH context the H's value 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 * This function is used in cases not all the data is arrange in one 
 * continues buffer . \n
 *
 * The function flow:
 *
 * 1) checking the parameters validty if there is an error the function shall exit with an error code. 
 * 2) Aquiring the working context from the CCM manager.
 * 3) If there isn't enough data in the previous update data buffer in the context plus the received data
 *    load the input data to the context buffer,  and exit the function.
 * 4) Fill the previous update data buffer from LLI table to contain an entire block and update the LLI table. 
 * 5) Calling the hardware low level function to execute the update.
 * 6) Fill the previous update data buffer with the data not processed at the end of the received data 
 *    and update the LLI table.
 * 7) Release the CCM context.
 * 
 * @param[in] ContextID_ptr - a pointer to the HASH context buffer allocated by the user that
 *                       is used for the HASH machine operation.
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 * @param[in] DataInSize  - Size of input data block in bytes. 
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
 CIMPORT_C CRYSError_t CRYS_HASH_SEP_Update(CRYS_HASHUserContext_t  *ContextID_ptr,
                                            DxUint8_t                 *intDataHead_ptr,     
	                                        DxUint32_t                intDataHeadSize, 
	                                        DxUint8_t                 *intDataTail_ptr,     
	                                        DxUint32_t                intDataTailSize, 
                                            DxUint32_t                 *DataInLliTab_ptr,     
                                            DxUint32_t                  InLliTabEntries, 
                                            DxUint32_t                  DataInSize );
                                        
                                        
/************************************************************************************************/
/**
 * This function provide HASH function to process one buffer of data in SEP multi LLI mode.
 * The function allocates an internal HASH Context , it initializes the 
 * HASH Context with the cryptographic attributes that are needed for 
 * the HASH block operation ( initialize H's value for the HASH algorithm ).
 * Then the function loads the Hardware with the initializing values and after 
 * that process the data block using the hardware to do hash .
 * At the end the function return the message digest of the data buffer .
 *
 *
 * @param[in] OperationMode - The operation mode : MD5 or SHA1.
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 * @param DataInSize  The size of the data to be hashed in bytes. 
 *
 * @retval HashResultBuff a pointer to the target buffer where the 
 *                      HASE result stored in the context is loaded to.
 *
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */
CIMPORT_C CRYSError_t CRYS_HASH_SEP( CRYS_HASH_OperationMode_t  OperationMode,
                                     DxUint32_t                *DataInLliTab_ptr,     
                                     DxUint32_t                 InLliTabEntries, 
                                     DxUint32_t                 DataInSize,
                                     CRYS_HASH_Result_t         HashResultBuff );                                        


/************************************************************************************************/
/**
* This function process a block of data via the HASH Hardware/Software.
*
* The function receives as input an handle to the HASH Context that was previously 
* initialized by a CRYS_HASH_Init function or by another call to a CRYS_HASH_Update function. \n
* The function sets the last H's value that was stored in 
* the CRYS HASH context, and then processes the data block. 
* At the end of the process, the function stores in the HASH context the H's value, 
* together with the cryptographic attributes that are needed for 
* the HASH block operation (initialize H's value for the HASH algorithm).
* This function is used in cases where the data is not all arranged in one 
* continuous buffer. \n
* 
* @param[in] ContextID_ptr - a pointer to the HASH context buffer allocated by the user that
*                       is used for the HASH machine operation.
*
* @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HASH. The pointer does 
*                         not need to be aligned. On CSI input mode the pointer must be equal to
*                         value (0xFFFFFFFC | DataInAlignment). 
* 
* @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
*                         multiple of HASH_BLOCK_SIZE for used HASH mode.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                        value MODULE_* CRYS_HASH_error.h
*
*
*/

CIMPORT_C CRYSError_t CRYS_HASH_Update_ExtApp(CRYS_HASHUserContext_t  *ContextID_ptr,
											  DxUint8_t                 *DataIn_ptr,
											  DxUint8_t                 *SepDataIn_ptr,
  									          DxUint32_t                 DataInSize );

/************************************************************************************************/
/**
* \brief This function provides a HASH function for processing one buffer of data.
*
* The function allocates an internal HASH Context, and initializes the 
* HASH Context with the cryptographic attributes that are needed for 
* the HASH block operation (initialize H's value for the HASH algorithm).
* Next the function loads the engine with the initializing values, 
* and then processes the data block, calculating the hash.
* Finally, the function return the message digest of the data buffer .
*
* @param[in] OperationMode - The operation mode: CRYS_HASH_SHA1_mode, CRYS_HASH_SHA224_mode, CRYS_HASH_SHA256_mode,
*												  CRYS_HASH_SHA384_mode, CRYS_HASH_SHA512_mode, CRYS_HASH_MD5_mode
* 
* @param[in] DataIn_ptr - The pointer to the buffer of the input data to the HASH. The pointer does 
*                         not need to be aligned. On CSI input mode the pointer must be equal to
*                         value (0xFFFFFFFC | DataInAlignment). 
* 
* @param[in] DataInSize - The size of the data to be hashed in bytes. On CSI data transfer mode the size must  
*                         multiple of HASH_BLOCK_SIZE for used HASH mode.
*
* @retval HashResultBuff - A pointer to the target buffer where the 
*                      HASE result stored in the context is loaded to.
*
* @return CRYSError_t -  On success the function returns the value CRYS_OK, 
*				and on failuer a non-ZERO error.
*
*/

CIMPORT_C CRYSError_t CRYS_HASH_ExtApp  ( CRYS_HASH_OperationMode_t  OperationMode,
										 DxUint8_t                   *DataIn_ptr,
										 DxUint8_t                   *SepDataIn_ptr,
								         DxUint32_t                   DataSize,
								         CRYS_HASH_Result_t         HashResultBuff );


#ifdef __cplusplus
}
#endif

#endif
