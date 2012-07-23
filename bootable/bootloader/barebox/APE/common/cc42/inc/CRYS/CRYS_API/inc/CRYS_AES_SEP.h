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
 
 #ifndef CRYS_AES_CF_VERSION_USED

/********************************************************/

#ifndef CRYS_AES_SEP_H
#define CRYS_AES_SEP_H

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
   *  Object % CRYS_AES_SEP.h    : %
   *  State           :  %state%
   *  Creation date   :  12 March 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AES APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AES_SEP.h#1:incl:5
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
 * @brief This function is used to operate a block on the AES machine in Multi LLI mode from SEP.
 *        This function should be called after the CRYS_AES_Init or CRYS_AES_InitSecretKey functions
 *        were called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying the pointers of the data in LLI table is not DX_NULL.
 *          - in all modes besides XCBC and CMAC:  
 *              verifying that the pointer to the data_out LLI table  is not DX_NULL, 
 *              verifying the data_in size is not 0 and is a multiple of 16 bytes.
 *
 *        2.Decrypting the received context to the working context after 
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *
 *        3.Executes the AES operation on the hardware by calling the 
 *          low level AES function LLF_SEP_AES_Block.
 *        4.Encrypting the information in the working context and storing it 
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *         
 *        5.Exit the handler with the OK code.  
 *      
 * @param[in] ContextID_ptr - a pointer to the AES context buffer allocated by the user that
 *                         is used for the AES machine operation. this should be the same context that was
 *                         used on the previous call of this session.
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chunks of input data block. The table need to be aligned and placed in SEP SRAM.
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 * @param[in] DataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chunks of buffer for output result data from AES. The table need to be 
 *                         aligned and placed in SEP SRAM.
 * @param[in] OutLliTabEntries - The count of entries in the output LLI table.
 * @param[in] DataInSize  - Size of input data block in bytes. 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                         value MODULE_* CRYS_AES_error.h
 *
 *     NOTE: After performing the CRYS_AES_SEP_Block operation the DataInLli table  and count of its 
 *           entryes may be changed (deleted the last block saved in the context ).
 *
 */
 CIMPORT_C CRYSError_t  CRYS_AES_SEP_Block( 
                                        CRYS_AESUserContext_t   *ContextID_ptr,   
                                        DxUint32_t              *DataInLliTab_ptr,     
                                        DxUint32_t               InLliTabEntries, 
                                        DxUint32_t              *DataOutLliTab_ptr,
                                        DxUint32_t               OutLliTabEntries,
                                        DxUint32_t               DataInSize,
                                        DxUint8_t               *RemainingData_ptr,
                                        DxUint32_t               RemainingSize);
                                         
                                        
/****************************************************************************************************/
/**
 * @brief This function is used to operate the AES machine in one integrated operation.
 *
 *        The actual macros that will be used by the users are:
 *
 *        CRYS_AES_SEP  - Operate the integrated _DX_AES with a key from the user.
 *        CRYS_AES_SEP_SecretKey - Operate the _DX_AES with the secret key.
 *        CRYS_AES_SEP_XCBCandCMAC - Operate the AES_XCBC_MAC or AES_XCBC_CMAC modes.
 *
 * @param[in,out] IVCounter_ptr - This parameter is the buffer of the IV, counter or Tweak according to op. mode:
 *                          - on ECB, XCBC, CMAC mode this parameter is not used.
 *                          - on CBC and MAC modes this parameter should contain the IV values.
 *                          - on CTR mode this value should contain the init counter.
 *                          - on XCBC and CMAC modes it may be NULL.
 *                          - on XTS mode this is the initial Tweak value (128-bit data unit number).
 *
 * @param[in] Key_ptr - A pointer to the user's key buffer.
 *
 * @param[in] KeySize - The size of the KEY used by the AES: 128, 192 or 256 bits, as defined in the enum.
 *
 * @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0) 
 *                           or a Decrypt operation (1). In XCBC and CMAC modes it must be 0.
 *
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, CTR, XCBC (PRF and 96), CMAC, XTS.
 *
 * @param[in] is_secret_key - A parameter specifying whether to use a secret key (1) 
 *                           or to use a key defined by the user (0). In XCBC and CMAC modes it must be 0.
 *
 * @param[in] DataInLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chunks of input data block. The table need to be aligned and placed in SEP SRAM.
 *
 * @param[in] InLliTabEntries - The count of entries in the input LLI table.
 *
 * @param[in] DataOutLliTab_ptr - The pointer to the buffer of LLI table, containing pointers and sizes of
 *                         chunks of buffer for output result data from AES. The table need to be 
 *                         aligned and placed in SEP SRAM.
 *
 * @param[in] OutLliTabEntries - The count of entries in the output LLI table.
 * @param[in] LliDataInSize    - Size of input LLI data in bytes. Note: Remaining data transferred trough the context if neecessary .
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, 
 *                       and on failure a value from CRYS_AES_error.h
 */
CIMPORT_C CRYSError_t  _DX_AES_SEP( CRYS_AES_IvCounter_t   IVCounter_ptr,       
		                        CRYS_AES_Key_t             Key_ptr,                 
		                        CRYS_AES_KeySize_t         KeySize,            
		                        CRYS_AES_EncryptMode_t     EncryptDecryptFlag, 
		                        CRYS_AES_OperationMode_t   OperationMode ,      
		                        DxUint8_t                  is_secret_key,       
                                DxUint32_t                *DataInLliTab_ptr,     
                                DxUint32_t                 InLliTabEntries, 
                                DxUint32_t                *DataOutLliTab_ptr,
                                DxUint32_t                 OutLliTabEntries,     
								DxUint32_t                 LliDataInSize );


/****************************************************************************************************/
/**
  \brief The macros that the user should call when operating the AES in one operation.
   Note that the return value is CRYSError_t.
   
   Macro CRYS_AES_SEP is called when the user performs an AES operation with a key supplied by him.
   Macro CRYS_AES_SEP_SecretKey is called when the user performs an AES operation using the secret key on the chip.
   Macro CRYS_AES_SEP_XCBCandCMAC is called when the user is using a AES_XCBC_MAC or AES_XCBC_CMAC modes.                    
   Macro CRYS_AES_SEP_XTS is called when the user is using a AES_XTS mode.                    
     
*/

#define CRYS_AES_SEP( IVCounter_ptr,Key_ptr,KeySize,EncryptDecryptFlag,OperationMode,DataInLliTab_ptr,InLliTabSize,DataOutLliTab_ptr,OutLliTabSize,DataInSize ) \
     _DX_AES_SEP( (IVCounter_ptr),(Key_ptr),(KeySize),(EncryptDecryptFlag),(OperationMode),DX_FALSE,(DataInLliTab_ptr),(InLliTabSize),(DataOutLliTab_ptr),(OutLliTabSize),(DataInSize) )  

#define CRYS_AES_SEP_SecretKey( IVCounter_ptr,EncryptDecryptFlag,OperationMode,DataInLliTab_ptr,InLliTabSize,DataOutLliTab_ptr,OutLliTabSize,DataInSize  )   \
     _DX_AES_SEP( (IVCounter_ptr), (DxUint8_t *)DX_NULL, CRYS_AES_Key128BitSize, (EncryptDecryptFlag),(OperationMode),DX_TRUE,(DataInLliTab_ptr),(InLliTabSize),(DataOutLliTab_ptr),(OutLliTabSize),(DataInSize) )  

#define CRYS_AES_SEP_XCBCandCMAC( Key_ptr,KeySize,OperationMode,DataInLliTab_ptr,InLliTabSize,DataOutLliTab_ptr,OutLliTabSize,DataInSize ) \
     _DX_AES_SEP( DX_NULL,(Key_ptr),(KeySize),CRYS_AES_Encrypt,(OperationMode),DX_FALSE,(DataInLliTab_ptr),(InLliTabSize),(DataOutLliTab_ptr),(OutLliTabSize),(DataInSize) )  

#define CRYS_AES_SEP_XTS( Tweak_ptr,Key_ptr,KeySize,EncryptDecryptFlag,DataInLliTab_ptr,InLliTabSize,DataOutLliTab_ptr,OutLliTabSize,DataInSize ) \
     _DX_AES_SEP( (Tweak_ptr),(Key_ptr),(KeySize),(EncryptDecryptFlag),CRYS_AES_XTS_mode,DX_FALSE,(DataInLliTab_ptr),(InLliTabSize),(DataOutLliTab_ptr),(OutLliTabSize),(DataInSize) )  


/****************************************************************************************************/
/** 
* @brief This function is used to operate a block of data on the SW or on AES machine.
*        This function should be called after the appropriate CRYS AES init function 
*        (according to used AES operation mode).
*
*        The function executes the following major steps:
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.
*        2.Decrypts the received context to the working context after 
*          capturing the working context by calling the CRYS_CCM_GetContext() call.
*        3.Executes the AES operation on the software or hardware by calling the 
*          low level AES function LLF_AES_Block or LLF_AES_XTS_Block.
*        4.Encrypts the working context and stores it to the users received context. 
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        5.Exits the handler with the OK code.  
*      
*
* @param[in] ContextID_ptr - A pointer to the AES context buffer allocated by the user that
*                            is used for the AES machine operation. This should be the same context that was
*                            used on the previous call of this session.
*
* @param[in] DataIn_ptr - A pointer to the buffer of the input data to the AES. The pointer does 
*                         not need to be aligned. On CSI input mode the pointer must be equal to
*                         value (0xFFFFFFFC | DataInAlignment). 
*
* @param[in] DataInSize - A size of the input data must be not 0 and must be multiple of 16 bytes, 
*                         excluding the following cases:
*                             - in CTR, XCBC, CMAC modes the size of the last block may be also 0 and not 
*                               multiple of 16 bytes;  
*
* @param[out] DataOut_ptr - A pointer to the buffer of the output data from the AES. The pointer  does not 
*                             need to be aligned. On CSI output mode the pointer must be equal to
*                             value (0xFFFFFFFC | DataOutAlignment). On all MAC modes (MAC,XCBC, CMAC) CSI 
*                             output is not allowed. 
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_DES_error.h
*
*     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously 
*               were on CSI mode.
*            2. Temporarily the CSI input or output are not allowed on XCBC, CMAC and XTS modes.    
*/
CEXPORT_C CRYSError_t  CRYS_AES_Block_ExtApp( CRYS_AESUserContext_t   *ContextID_ptr,   
											 DxUint8_t               *DataIn_ptr,     
											 DxUint8_t               *SepDataIn_ptr,     
											 DxUint32_t               DataInSize,     
											 DxUint8_t               *DataOut_ptr,
											 DxUint8_t               *SepDataOut_ptr );


/****************************************************************************************************/
/**
* @brief This function is used to perform the AES operation in one integrated process.
*
*        The actual macros, that will be used by the user for calling this function, are described 
*        in CRYS_AES.h file.
*
*        The input-output parameters of the function are the following:
*
* @param[in] ContextID_ptr - A pointer to the AES context buffer that is allocated by the user 
*                            and is used for the AES machine operation.
* @param[in] IVCounter_ptr - A buffer containing an initial value: IV, Counter or Tweak according 
*                            to operation mode:
*                            - on ECB, XCBC, CMAC mode this parameter is not used and may be NULL,
*                            - on CBC and MAC modes it contains the IV value,
*                            - on CTR mode it contains the init counter,
*                            - on XTS mode it contains the initial tweak value - 128-bit consecutive number 
*                              of data unit (in little endian).
* @param[in] Key_ptr  -  A pointer to the user's key buffer.
* @param[in] KeySize  -  An enum parameter, defines size of used key (128, 192, 256 bits).
* @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0) 
*                                 or a Decrypt operation (1). In XCBC and CMAC modes it must be 0.
* @param[in] OperationMode - The operation mode: ECB, CBC, MAC, CTR, XCBC (PRF and 96), CMAC, XTS.
* @param[in] is_secret_key - A parameter specifying whether to use a secret key (1) 
*                            or to use a key defined by the user (0). 
* @param[in] DataIn_ptr - A pointer to the buffer of the input data to the AES. The pointer does 
*                         not need to be aligned. On CSI input mode the pointer must be equal to
*                         value (0xFFFFFFFC | DataInAlignment). 
*
* @param[in] DataInSize - The size of the input data, it must be:
*                         - on ECB,CBC,MAC modes must be not 0 and must be a multiple of 16 bytes,
*                         - on CTR, XCBC and CMAC modes must be not 0,
*                         - on XTS mode must be or multiple of 16 bytes (not 0), or not less than 17 bytes.
* @param[out] DataOut_ptr - A pointer to the buffer of the output data from the AES. The pointer  does not 
*                             need to be aligned. On CSI output mode the pointer must be equal to
*                             value (0xFFFFFFFC | DataOutAlignment). On all MAC modes (MAC,XCBC, CMAC) CSI 
*                             output is not allowed.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in CRYS_AES_error.h
*
*     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously 
*               were on CSI mode.
*            2. Temporarily the CSI input or output are not allowed on XCBC, CMAC and XTS modes.    
*
*/
CEXPORT_C CRYSError_t  _DX_AES_ExtApp( CRYS_AES_IvCounter_t       IVCounter_ptr,       
									  CRYS_AES_Key_t             Key_ptr,								
									  CRYS_AES_KeySize_t         KeySize,            
									  CRYS_AES_EncryptMode_t     EncryptDecryptFlag, 
									  CRYS_AES_OperationMode_t   OperationMode ,      
									  DxUint8_t                  is_secret_key,       
									  DxUint8_t                  *DataIn_ptr,        
									  DxUint8_t                  *SepDataIn_ptr,        
									  DxUint32_t                 DataInSize,         
									  DxUint8_t                  *DataOut_ptr,
									  DxUint8_t                  *SepDataOut_ptr );


#ifdef __cplusplus
}
#endif

#endif  


/**************************************************/

#else /* CRYS_AES_CF_VERSION_USED */

#include "CRYS_AES_SEP_CF.h"

#endif  /* End of #ifndef CRYS_AES_SEP_CF_H */

