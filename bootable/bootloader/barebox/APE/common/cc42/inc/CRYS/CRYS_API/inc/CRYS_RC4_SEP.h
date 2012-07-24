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
 
 
#ifndef CRYS_RC4_SEP_H
#define CRYS_RC4_SEP_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AES.h"
#include "LLF_AES_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_RC4_SEP.h    : %
   *  State           :  %state%
   *  Creation date   :  12 March 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AES APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AES_RC4.h#1:incl:5
   *  \author R.Levin.
   */

/************************ Defines ***************************************/



/************************ Enums *****************************************/

/************************ Typedefs  *************************************/

/************************ context Structs  ******************************/

/************************ Public Variables ******************************/

/************************ Public Functions ******************************/

                                        

/************************************************************************************************/
/**
 * This function process a block of data via the RC4 Hardware in SEP multi LLI mode.
 *
 * The function receives as input an handle to the  RC4 Context , that was initialized before
 * by an CRYS_RC4_Init function or by other CRYS_RC4_Stream function. \n
 * The function concatenates the previous residuary data (that is less than 4 bytes and where stored in
 * the CRYS RC4 context) with new data, updates input LLI table and then calls the LLF_RC4_SEP_Stream 
 * function. In the end of the process the function stores current residuary data in the RC4 context  
 * and exits.
 *
 * The function flow:
 *
 * 1) Checks the parameters validty if there is an error the function shall exit with an error code. 
 * 2) Acquires the working context from the CCM manager.
 * 5) Calls the hardware low level function to execute the update.
 * 6) Fills the previous update data buffer with the not processed data .
 * 7) Releases the CCM context.
 * 
 *      
 * @param[in] ContextID_ptr - a pointer to the RC4 context buffer allocated by the user that
 *                         is used for the machine operation. this should be the same context that was
 *                         used on the previous call of this session.
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 * @param[in] DataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of buffer for output result data from AES. The table need to be 
 *                         aligned and placed in SEP SRAM.
 * @param[in] OutLliTabEntries - The count of entries in the output LLI table.
 * @param[in] DataInSize  - Size of input data block in bytes. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                         value MODULE_* CRYS_RC4_error.h
 *
 *     NOTE: After performing the RC4_SEP_Stream operation the DataInLli table  and count of its 
 *           entryes may be changed (deleted the last block saved in the context ).
 *
 */
  CIMPORT_C CRYSError_t  CRYS_RC4_SEP_Stream( 
                                        CRYS_RC4UserContext_t   *ContextID_ptr,   
                                        DxUint32_t              *DataInLliTab_ptr,     
                                        DxUint32_t               InLliTabEntries, 
                                        DxUint32_t              *DataOutLliTab_ptr,
                                        DxUint32_t               OutLliTabEntries,
                                        DxUint32_t               DataInSize );
                                        
/************************************************************************************************/
/**
 * This function provide RC4 function to process one buffer of data in SEP multi LLI mode.
 *
 * The function performs all RC4 operations on data by continuously calling the CRYS_RC4_Init, 
 * CRYS_RC4_SEP_Stream and CRYS_RC4_Finish functions.
 *
 *
 * @param[in] Key_ptr -  A pointer to the user's key buffer.
 *
 * @param[in] KeySize - The size of the KEY in bytes.
 *
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of input data block. The table need to be aligned and placed in SEP SRAM.
 *
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 *
 * @param[in] DataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chuncks of buffer for output result data from RC4. The table need to be 
 *                         aligned and placed in SEP SRAM.
 *
 * @param[in] OutLliTabEntries - The count of entries in the output LLI table.
 * @param[in] DataInSize  - Size of input data in bytes. 
 *
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                       and on failure a value from CRYS_AES_error.h
 */
CIMPORT_C CRYSError_t  CRYS_RC4_SEP(      
		                        DxUint8_t                 *Key_ptr, 
							    DxUint32_t                 KeySizeInBytes,           
                                DxUint32_t                *DataInLliTab_ptr,     
                                DxUint32_t                 InLliTabEntries, 
                                DxUint32_t                *DataOutLliTab_ptr,
                                DxUint32_t                 OutLliTabEntries,     
		                        DxUint32_t                 DataInSize );     



/******************************************************************************************************/
/**
* @brief This function is used to process a stream on the RC4 machine.
*        This function should be called after the CRYS_RS4_Init. 
*      
*
* @param[in] ContextID_ptr - A pointer to the RC4 context buffer allocated by the user 
*                       that is used for the RC4 machine operation. This should be the 
*                       same context as was used for the previous call of this session.
*
* @param[in] DataIn_ptr - The pointer to the buffer of the input data to the RC4. 
*                   The pointer's value does not need to be word-aligned.
*
* @param[in] DataInSize - The size of the input data.
*
* @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the RC4. 
*                        The pointer's value does not need to be word-aligned.  
*
* @return CRYSError_t - CRYS_OK,
*                       CRYS_RC4_INVALID_USER_CONTEXT_POINTER_ERROR,
*                       CRYS_RC4_USER_CONTEXT_CORRUPTED_ERROR,
*                       CRYS_RC4_DATA_IN_POINTER_INVALID_ERROR,
*                       CRYS_RC4_DATA_OUT_POINTER_INVALID_ERROR,
*                       CRYS_RC4_DATA_OUT_DATA_IN_OVERLAP_ERROR,
*                       CRYS_RC4_DATA_SIZE_ILLEGAL
*/
CEXPORT_C CRYSError_t  CRYS_RC4_Stream_ExtApp(CRYS_RC4UserContext_t        *ContextID_ptr,   
											  DxUint8_t                    *DataIn_ptr,                                  
											  DxUint32_t                    DataInSize,
											  DxUint8_t                    *DataOut_ptr);


/***********************************************************************************************/
/**
* \brief This function provides a RC4 operation for processing one continuous block of data.
*
*        The function performs RC4 operation  by calling CRYS_RC4_Init, CRYS_RC4_Stream and 
*        CRYS_RC4_Free functions.
*
* @param[in] Key_ptr -  A pointer to the user's key buffer.
*
* @param[in] KeySize - The size of the KEY in bytes.
*
* @param[in] DataIn_ptr - The pointer to the buffer of the input data to the RC4. 
*                   The pointer's value does not need to be word-aligned.
*
* @param[in] DataInSize - The size of the input data.
*
* @param[in,out] The pointer to the buffer of the output data from the RC4.
*                The pointer's value does not need to be word-aligned. The size of this buffer
*                must be the same as the DataIn buffer.  
*
* @return CRYSError_t -  On success the function returns the value CRYS_OK, 
*				and on failure a value from CRYS_RC4_error.h
*
*/
CEXPORT_C    CRYSError_t CRYS_RC4_ExtApp( DxUint8_t   *Key_ptr, 
										 DxUint32_t    KeySizeInBytes,
										 DxUint8_t    *DataIn_ptr, 
										 DxUint32_t    DataInSize,
										 DxUint8_t    *DataOut_ptr );  



#ifdef __cplusplus
}
#endif

#endif
