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
 
 
#ifndef CRYS_AESCCM_SEP_H
#define CRYS_AESCCM_SEP_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AESCCM.h"
#include "LLF_AESCCM_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_AES_SEP.h    : %
   *  State           :  %state%
   *  Creation date   :  21 Dec 2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AESCCM APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AESCCM_SEP.h#1:incl:5
   *  \author R.Levin.
   */

/************************ Defines ***************************************/

/************************ Enums *****************************************/

/************************ Typedefs  *************************************/

/************************ context Structs  ******************************/

/************************ Public Variables ******************************/

/************************ Public Functions ******************************/



/****************************************************************************************************/
/** 
* @brief This function is used to operate a AESCCM block of Adata in Multi LLI mode with SEP.
*        This function should be called after the CRYS_AESCCM_Init function were called.
*
*        The function executes the following major steps:
*
*        1.Checks the validation of all of the inputs of the function.
*          If one of the received parameters is not valid it shall return an error.
*
*          The major checkers that are run over the received parameters:
*          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
*          - verifying the pointers of the data are not DX_NULL.
*        2.Decrypting the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext() call.
*        3.Executes the AESCCM operation on the hardware by calling the 
*          low level AESCCM function LLF_SEP_AESCCM_BlockAdata.
*        4.Encrypting the information in the working context and storing it 
*          to the users received context. After then the working context is released.
*          This state is operated by calling the CRYS_CCM_ReleaseContext call.
*        5.Exit the handler with the OK code.  
*      
* @param[in] ContextID_ptr -Te pointer to the AESCCM context buffer allocated by the user that
*                         is used for the AES machine operation. This should be the same context 
*                         that was used on the previous call of this session.
* @param[in] inHeadData_ptr - The pointer to the first bytes of the Adata, not included in LLI table.
* @param[in] inHeadDataSize - The size of first data, not aligned to word. 
* @param[in] inTailData_ptr - The pointer to the last bytes of the data, not included in LLI table.
* @param[in] inTailDataSize - The size of last bytes of Adata, not included in LLI table. 
* @param[in] dataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
*                               chunks of input data block. The table need to be aligned and placed in SEP SRAM.
* @param[in] inLliTabEntries  - The count of entries in the input LLI table.
* @param[in] lliDataInSize    - Size of input data, included in LLI table, in bytes. 
*
*                           Note: Full size of additional data in bytes (including the head, LLI and 
*                                 tail Adata sizes), must be less, than 2^32. 
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a values defined in CRYS_AESCCM_error.h
*
*/
	CIMPORT_C CRYSError_t  CRYS_AESCCM_SEP_BlockAdata( 
												CRYS_AESCCM_UserContext_t   *ContextID_ptr,   
												DxUint32_t                  *inHeadData_ptr,
												DxUint32_t                   inHeadDataSize,
												DxUint32_t                  *inTailData_ptr,
												DxUint32_t                   inTailDataSize,
												DxUint32_t                  *dataInLliTab_ptr,     
												DxUint32_t                   inLliTabEntries, 
												DxUint32_t                   lliDataInSize );
                                         

 /****************************************************************************************************/
 /** 
 * @brief This function is used to operate AESCCM on last block of TextData in Multi LLI mode with SEP.
 *        This function should be called after the CRYS_AESCCM_Init function were called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying the pointers of the data are not DX_NULL.
 *        2.Decrypting the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *        3.Executes the AESCCM operation on the hardware by calling the 
 *          low level function LLF_AESCCM_SEP_BlockTextata.
 *        4.Encrypting the information in the working context and storing it 
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exit the handler with the OK code.  
 *      
 * @param[in] ContextID_ptr -The pointer to the AESCCM context buffer allocated by the user that
 *                         is used for the AES machine operation. This should be the same context 
 *                         that was used on the previous call of this session.
 * @param[in] dataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] inLliTabEntries  - The count of entries in the input LLI table.
 * @param[in] dataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of buffer for output result data from AES. The table need to be 
 *                               aligned and placed in SEP SRAM.
 * @param[in] outLliTabEntries - The count of entries in the output LLI table.
 * @param[in] dataSize         - Size of input/output data, included in LLI table, in bytes. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure  a values defined in CRYS_AESCCM_error.h
 *
 */
 CIMPORT_C CRYSError_t  CRYS_AESCCM_SEP_BlockTextData( 
											 CRYS_AESCCM_UserContext_t   *ContextID_ptr,   
											 DxUint32_t                  *dataInLliTab_ptr,     
											 DxUint32_t                  inLliTabEntries, 
											 DxUint32_t                 *dataOutLliTab_ptr,
											 DxUint32_t                  outLliTabEntries,
											 DxUint32_t                  dataSize);

 
 /****************************************************************************************************/
 /** 
 * @brief This function is used to operate AESCCM on last block of TextData in Multi LLI mode with SEP.
 *        This function should be called after the CRYS_AESCCM_Init function were called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying the pointers of the data are not DX_NULL.
 *        2.Decrypting the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *        3.Executes the AESCCM operation on the hardware by calling the 
 *          low level AESCCM function LLF_SEP_AESCCM_BlockTextata.
 *        4.Encrypting the information in the working context and storing it 
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exit the handler with the OK code.  
 *      
 * @param[in] ContextID_ptr - The pointer to the AESCCM context buffer allocated by the user that
 *                            is used for the AES machine operation. This should be the same context 
 *                            that was used on the previous call of this session.
 * @param[in] tailData_ptr  - The pointer to the buffer, containing remaining text data (size < 16 bytes).
 * @param[in] tailDataSize  - The size of tail text data < 16 bytes. 
 * @param[in] ccmMac_ptr    - The pointer to encrypted/decrypted CCM_MAC value.
 * @param[in] ccmMacSize    - The size of CCM_MAC value. 
 * @param[in] dataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] inLliTabEntries  - The count of entries in the input LLI table.
 * @param[in] dataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of buffer for output result data from AES. The table need to be 
 *                               aligned and placed in SEP SRAM.
 * @param[in] outLliTabEntries - The count of entries in the output LLI table.
 * @param[in] lliDataSize      - Size of input/output data (only plain text without encrypted MAC), included 
 *                               in LLI table, in bytes, must be multiple of 16 bytes . 
 *
 *                           Note: Full size of input text data in bytes (including the tail, LLI and 
 *                                 CCMMAC text data), must be :
 *                                     - on decrypt mode less, than 2^32, 
 *                                     - on encrypt mode less, than 2^32-SizeOfT. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure  a values defined in CRYS_AESCCM_error.h.
 *
 */
 CIMPORT_C CRYSError_t  CRYS_AESCCM_SEP_BlockLastTextData( 
													 CRYS_AESCCM_UserContext_t   *ContextID_ptr,   
													 DxUint32_t                   *tailData_ptr,
													 DxUint32_t                   tailDataSize,
													 DxUint32_t                   *ccmMac_ptr,
													 DxUint32_t                   ccmMacSize,
													 DxUint32_t                  *dataInLliTab_ptr,     
													 DxUint32_t                   inLliTabEntries, 
													 DxUint32_t                  *dataOutLliTab_ptr,
													 DxUint32_t                   outLliTabEntries,
													 DxUint32_t                   lliDataSize );

 /****************************************************************************************************/
 /**
 * @brief This function is used to operate the AES machine in one integrated operation.
 *
 *        The actual macros that will be used by the users are:
 *
 * @param[in] ContextID_ptr - The pointer to the AESCCM context buffer allocated by the user that
 *                            is used for the AES machine operation. This should be the same context 
 *                            that was used on the previous call of this session.
 * @param[in] EncrDecrMode  - Enumerator variable defining operation mode (0 - encrypt; 1 - decrypt).
 * @param[in] CCM_Key       - A buffer, containing the AESCCM key passed by user (predefined size 128 bits).
 * @param[in] AdataSize     - Full size of additional data in bytes, which must be processed.  
 *                            Limitation in our implementation is: AdataSize < 2^32. If Adata is absent, 
 *                            then AdataSize = 0.
 * @param[in] TextSizeQ     - The full size of text data (in bytes), which must be processed by CCM. 
 *                            Limitation in our implementation is: TextSizeQ < 2^32 on decrypt mode 
 *                            and less, than 2^32-SizeOfT on encrypt mode. 
 * @param[in] QFieldSize    - Byte-size of formatted field for writing significant bytes of the TextSizeQ 
 *                            value. Valid values: [2,3,4,5,6,7,8]. 
 * @param[in] N_ptr	       - A pointer to Nonce - unique value assigned to all data passed into CCM.
 *                            Bytes order - big endian form (MSB is the first).
 * @param[in] SizeOfN       - The size of the user passed Nonce (in bytes). 	Valid values: 
 * @param[in] SizeOfT       - The size of CCM-MAC .
 * @param[in] headData_ptr  - The pointer to the first bytes of the Adata, not included in LLI table.
 * @param[in] headDataSize  - The size of first data, not aligned to word. 
 * @param[in] tailData_ptr  - The pointer to the last bytes of the data, not included in LLI table.
 * @param[in] tailDataSize  - The size of last bytes of Adata, not included in LLI table. 
 * @param[in] inAdataLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of input Adata block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] inAdataLliTabEntries  - The count of entries in the Adata input LLI table.
 * @param[in] inLliAdataSize  - Size of input Adata, included into LLI table, in bytes. 
 *                           Note: Full size of additional data in bytes (including also the head and tail Adata sizes), 
 *                                 must be less, than 2^32. 
 * @param[in] headTextData_ptr - The pointer to the the text data (used in case that the size of the plain text data,   
 *                               excluding the encrypted MAC value, is not great than 16 bytes).
 * @param[in] headTextDataSize - The size of text data if it <= 16 bytes. The size not includes the size of encrypted 
 *                               AESCCM_MAC value present on decrypt mode and passed by tailTextData_ptr. 
 * @param[in] tailTextData_ptr - The pointer to the last bytes of the text data, containing encrypted CCM_MAC value.
 * @param[in] tailTextDataSize - The size of last text data containing encrypted AESCCM_MAC value. The size 
 *                               must = SizeOfT. 
 * @param[in] ccmMac_ptr    - The pointer to encrypted/decrypted CCM_MAC value.
 * @param[in] ccmMacSize    - The size of CCM_MAC value. 
 * @param[in] inTextDataLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of input text data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] inTextDdataLliTabEntries  - The count of entries in the input text data LLI table.
 * @param[in] inLliTextDataSize  - Size of input text data, included in LLI table, in bytes. 
 *                           Note: Full size of text data in bytes (including also the head and tail text data sizes), 
 *                                 must be less, than 2^32 on decrypt mode and less, than 2^32-SizeOfT on encrypt mode. 
 * @param[in] outTailTextData_ptr - The pointer to the last bytes of the text data, not aligned to AES-block and word.
 * @param[in] outTailTextDataSize - The size of last text data, containing encrypted AESCCM_MAC value. On Encrypt mode 
 *                                 it must be  = SizeOfT, on decrypt mode it not required.
 * @param[in] outTextDataLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                               chunks of input text data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] outTextDdataLliTabEntries  - The count of entries in the input text data LLI table.
 * @param[in] lliTextDataSize  - Size of input/output text data, included in LLI table, in bytes. 
 *
 *                           Note: Full size of input text data in bytes (including also the tail text data size), 
 *                                 must be less, than 2^32 on decrypt mode and less, than 2^32-SizeOfT on encrypt mode. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure  a values defined in CRYS_AESCCM_error.h
 *
 */
 CIMPORT_C CRYSError_t  CRYS_AESCCM_SEP( 
								 CRYS_AES_EncryptMode_t       EncrDecrMode,     
								 CRYS_AESCCM_Key_t            CCM_Key,                     
								 DxUint8_t                    QFieldSize,       
								 DxUint8_t                   *N_ptr,            
								 DxUint8_t                    SizeOfN,
								 DxUint8_t                    SizeOfT,    
								 DxUint32_t                  *headAdata_ptr,
								 DxUint32_t                   headAdataSize,
								 DxUint32_t                  *tailAdata_ptr,
								 DxUint32_t                   tailAdataSize,
								 DxUint32_t                  *inAdataLliTab_ptr,     
								 DxUint32_t                   inAdataLliTabEntries, 
								 DxUint32_t                   inLliAdataSize,
								 DxUint32_t                  *tailTextData_ptr,
								 DxUint32_t                   tailTextDataSize,
								 DxUint32_t                  *ccmMac_ptr,
								 DxUint32_t                   ccmMacSize,
								 DxUint32_t                  *inTextDataLliTab_ptr,     
								 DxUint32_t                   inTextDataLliTabEntries, 
								 DxUint32_t                  *outTextDataLliTab_ptr,     
								 DxUint32_t                   outTextDataLliTabEntries, 
								 DxUint32_t                   lliTextDataSize ) ;

#ifdef __cplusplus
}
#endif

#endif
