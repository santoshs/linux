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

#ifndef CRYS_AES_H
#define CRYS_AES_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "LLF_AES_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object % CRYS_AES.h    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:42:30 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains all of the enums and definitions that are used for the 
   *         CRYS AES APIs, as well as the APIs themselves. 
   *
   *  \version CRYS_AES.h#1:incl:13
   *  \author adams
   */

/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/

#define CRYS_AES_ENGINE_TYPE  LLF_AES_ENGINE_TYPE

/* The AES block size in words and in bytes */
#define CRYS_AES_BLOCK_SIZE_IN_WORDS 4
#define CRYS_AES_BLOCK_SIZE_IN_BYTES  (CRYS_AES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t))

/* The size of the IV or counter buffer */
#define CRYS_AES_IV_COUNTER_SIZE_IN_WORDS   CRYS_AES_BLOCK_SIZE_IN_WORDS
#define CRYS_AES_IV_COUNTER_SIZE_IN_BYTES  (CRYS_AES_IV_COUNTER_SIZE_IN_WORDS * sizeof(DxUint32_t))

/* The maximum size of the AES KEY in words and bytes */
#define CRYS_AES_KEY_MAX_SIZE_IN_WORDS 16
#define CRYS_AES_KEY_MAX_SIZE_IN_BYTES (CRYS_AES_KEY_MAX_SIZE_IN_WORDS * sizeof(DxUint32_t))

/* The AES_WRAP minimum data size in bytes  (one 64-bits block)  */
#define CRYS_AES_WRAP_DATA_MIN_SIZE_IN_BYTES   8

/* The AES_WRAP maximum data size in bytes:  2^28 */
#define CRYS_AES_WRAP_DATA_MAX_SIZE_IN_BYTES   0x10000000

/* The CRYS_AES_WRAP block size in bytes and in words */
#define CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES 8
#define CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS  (CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES / sizeof(DxUint32_t))


/************************ Enums ********************************/

/* Enum defining the user's key size argument */
typedef enum
{
   CRYS_AES_Key128BitSize   = 0,
   CRYS_AES_Key192BitSize   = 1,
   CRYS_AES_Key256BitSize   = 2,
   CRYS_AES_Key512BitSize   = 3,

   CRYS_AES_KeySizeNumOfOptions,
   
   CRYS_AES_KeySizeLast    = 0x7FFFFFFF,

}CRYS_AES_KeySize_t;  

/* Enum defining the Encrypt or Decrypt operation mode */
typedef enum 
{
   CRYS_AES_Encrypt = 0,
   CRYS_AES_Decrypt = 1,
   
   CRYS_AES_EncryptNumOfOptions,

   CRYS_AES_EncryptModeLast= 0x7FFFFFFF,

}CRYS_AES_EncryptMode_t;

/* Enum defining the AES operation mode */
typedef enum
{
   CRYS_AES_ECB_mode          = 0,
   CRYS_AES_CBC_mode          = 1,
   CRYS_AES_MAC_mode          = 2,
   CRYS_AES_CTR_mode          = 3,
   CRYS_AES_XCBC_MAC_mode     = 4,
   CRYS_AES_CMAC_mode         = 5,
   CRYS_AES_XTS_mode          = 6,
   
   CRYS_AES_NumOfModes,

   CRYS_AES_OperationModeLast= 0x7FFFFFFF,

}CRYS_AES_OperationMode_t;

/************************ Typedefs  ****************************/

/* Defines the IV counter buffer  - 16 bytes array */
typedef DxUint8_t CRYS_AES_IvCounter_t[CRYS_AES_IV_COUNTER_SIZE_IN_BYTES];

/* Define the XTS Tweak value type - 16 bytes array */
typedef  CRYS_AES_IvCounter_t  CRYS_AES_XTS_Tweak_t;

/* Defines the AES key buffer */
typedef DxUint8_t CRYS_AES_Key_t[CRYS_AES_KEY_MAX_SIZE_IN_BYTES];

/* Defines the AES MAC result maximum size buffer */
typedef DxUint8_t CRYS_AES_MAX_MAC_RESULT_t[CRYS_AES_IV_COUNTER_SIZE_IN_BYTES];


/************************ context Structs  ******************************/

/* The context data-base used by the AES functions on the low level */

typedef struct  
{ 
   /* Fields that are common to all platforms and operation modes */

   /* AESContextIV contains: on CTR mode - Counter, on XTS mode - Tweak, on other modes - IV */ 
   DxUint32_t                 AESContextIV[CRYS_AES_IV_COUNTER_SIZE_IN_WORDS];
   /* AES context working Key (on XTS mode - Tweak-Key or Data-Key). It's max size is 256 bit = 512/2*/
   DxUint32_t                 AESContextKey[CRYS_AES_KEY_MAX_SIZE_IN_WORDS / 2];
   CRYS_AES_KeySize_t         AESContextKeySize;
   /* Flags */
   CRYS_AES_EncryptMode_t     EncryptDecryptFlag;
   CRYS_AES_OperationMode_t   OperationMode;  
   DxUint8_t                  is_secret_key;   
     
   /* Fields that are used on XCBC, CMAC modes with all platforms */ 
     
   DxUint32_t  NotAddBlocks; /* flag prohibiting additional data blocks receiving */   
   /* last block size and value */
   DxUint32_t  lastBlockSize;   
   DxUint32_t  lastBlock[CRYS_AES_BLOCK_SIZE_IN_WORDS];
   /* previous IV */
   DxUint32_t  previousIV[CRYS_AES_BLOCK_SIZE_IN_WORDS];
 
   /* remaining size of data to process in operation */
   DxUint32_t  remainSize;
   
   /* Specific data for XCBC, CMAC, XTS modes */
   union{
	   struct{
		   /* derived keys */
		   DxUint32_t  derivedKey1[CRYS_AES_BLOCK_SIZE_IN_WORDS];
		   DxUint32_t  derivedKey2[CRYS_AES_BLOCK_SIZE_IN_WORDS];
		   DxUint32_t  derivedKey3[CRYS_AES_BLOCK_SIZE_IN_WORDS];
	   } XCBC_CMAC_Data;

	   struct{
		   /* AES-XTS Key2. It's max size is 256 bit = 512/2 */
		   DxUint32_t  AES_XTS_Key2[CRYS_AES_KEY_MAX_SIZE_IN_WORDS / 2];
		   DxUint32_t  TempX[CRYS_AES_BLOCK_SIZE_IN_WORDS];
		   DxUint32_t  TempX2[CRYS_AES_BLOCK_SIZE_IN_WORDS];
	   } XTS_Data;
   } SpecificData;
   
   /* Data specific to the platform we are working on - include from LLF */
   #include "LLF_AES_context_def.h"
   
   /* Dummy buffer - added for right AES encrypting of the context data.
      Note: This block must be always at the end of the context.  */
   DxUint8_t DummyBufAESBlockSize[CRYS_DEFS_DUMMY_AES_BLOCK_SIZE];
     
}AESContext_t; 


/* The user's context prototype - the argument type that will be passed by the user 
   to the APIs called by him */
typedef struct CRYS_AESUserContext_t 
{
   DxUint32_t valid_tag;
   DxUint32_t AES_iv;/* for use of the AES CBC mode of Encryption and Decryption of the context in CCM */ 
   DxUint32_t   OperationMode;  /* for using in non secured parts of programms */
   DxUint8_t  context_buff[ sizeof(AESContext_t) ]; 
}CRYS_AESUserContext_t;

/************************ Public Variables **********************/


/************************ Public Functions **********************/

/****************************************************************************************************/
/** 
 * @brief This function is used to initialize the AES machine or SW structures.
 *        To perform the AES operations this should be the first function called.
 *
 *        The actual macros, that will be used by the user for calling this function, are described 
 *        in CRYS_AES.h file.
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
 * @param[in] KeySize  -  An enum parameter, defines size of used key (128, 192, 256, 512 bits):
 *                        On XCBC mode allowed 128 bit size only, on XTS - 256 or 512 bit, on other modes <= 256 bit.
 * @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0) 
 *                                 or a Decrypt operation (1). In XCBC and CMAC modes it must be Encrypt.
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, CTR, XCBC (PRF and 96), CMAC.
 * @param[in] is_secret_key - A parameter specifying whether to use a secret key (1) 
 *                            or to use a key defined by the user (0). 
 *                            The secret key function depends on product configuration. 
 *                            Please consult the reference manual.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, and on failure - a value from CRYS_AES_error.h
 */
CIMPORT_C CRYSError_t  _DX_AES_Init(  
							CRYS_AESUserContext_t    *ContextID_ptr,
                            CRYS_AES_IvCounter_t     IVCounter_ptr,
                            CRYS_AES_Key_t           Key_ptr,
                            CRYS_AES_KeySize_t       KeySize,
                            CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                            CRYS_AES_OperationMode_t OperationMode,
                            DxUint8_t                is_secret_key );


/**
  \brief The macros that the user should call when performing the AES initializations.
   Note that the return value is CRYSError_t.
   
   Macro CRYS_AES_Init is called when the user performs an AES operation with a key supplied by him.
   Macro CRYS_AES_InitSecretKey is called when the user performs an AES operation using the secret key on the chip.
   Macro CRYS_AES_InitWrapKey is called when the user is using a wrapped key - supported only on the SSDMA platforms.
   Macro CRYS_AES_Init_XCBCandCMAC is called when the user performs an AES operation on XCBC and CMAC modes.  
*/

#define CRYS_AES_Init( ContextID_ptr, IVCounter_ptr, Key_ptr, KeySize, EncryptDecryptFlag, OperationMode ) \
     _DX_AES_Init( (ContextID_ptr),(IVCounter_ptr),(Key_ptr),(KeySize),(EncryptDecryptFlag),(OperationMode),DX_FALSE)  

/* Availability of this API depends on product configuration. Please consult the reference manual. */
#define CRYS_AES_InitSecretKey( ContextID_ptr, IVCounter_ptr, EncryptDecryptFlag, OperationMode )   \
     _DX_AES_Init( (ContextID_ptr),(IVCounter_ptr),(DxUint8_t*)DX_NULL,(CRYS_AES_KeySize_t)0,(EncryptDecryptFlag),(OperationMode),DX_TRUE)  

#define CRYS_AES_Init_XCBCandCMAC( ContextID_ptr, Key_ptr, KeySize, OperationMode ) \
     _DX_AES_Init( (ContextID_ptr),DX_NULL,(Key_ptr),(KeySize),CRYS_AES_Encrypt,(OperationMode), DX_FALSE)

#define CRYS_AES_Init_XCBCandCMAC_SecretKey( ContextID_ptr, OperationMode ) \
     _DX_AES_Init( (ContextID_ptr),DX_NULL,(DxUint8_t*)DX_NULL,(CRYS_AES_KeySize_t)0,CRYS_AES_Encrypt,(OperationMode), DX_TRUE)

#define CRYS_AES_XTS_Init( ContextID_ptr, Tweak_ptr, Key_ptr, KeySize, EncryptDecryptFlag ) \
     _DX_AES_Init( (ContextID_ptr),(Tweak_ptr),(Key_ptr),(KeySize),(EncryptDecryptFlag),CRYS_AES_XTS_mode,DX_FALSE )


/****************************************************************************************************/
/** 
 * @brief The CRYS_AES_SET_IV macros sets the user passed data into AES context IV.
 *
 *    Note: This macro may be used with not encrypted context only.
 *
 * @param[in] ContextID_ptr - A pointer to the AES context structure of type CRYS_AESUserContext_t.
 * @param[in] IVCounter_ptr - A buffer containing IV (or counter, or tweak) value of size 4 32-bits words.
 **/
#define  CRYS_AES_SET_IV( ContextID_ptr, IVCounter_ptr ) \
                	DX_VOS_FastMemCpy( ((AESContext_t*)((ContextID_ptr)->context_buff))->AESContextIV, \
                                    	(IVCounter_ptr), sizeof(CRYS_AES_IvCounter_t) );

/****************************************************************************************************/
/** 
 * @brief The CRYS_AES_GET_IV macros copies the IV from AES context into user passed buffer.
 *
 *    Note: This macro may be used with not encrypted context only.
 *
 * @param[in]  ContextID_ptr - A pointer to the AES context structure of type CRYS_AESUserContext_t.
 * @param[out] IVCounter_ptr - A buffer for output the IV (or counter, or tweak) value of size 4 32-bits words.
 **/
#define  CRYS_AES_GET_IV( ContextID_ptr, IVCounter_ptr ) \
                	DX_VOS_FastMemCpy( (IVCounter_ptr), ((AESContext_t*)((ContextID_ptr)->context_buff))->AESContextIV, \
                                        sizeof(CRYS_AES_IvCounter_t) );
 	
/****************************************************************************************************/
/** 
 * @brief This function is used to operate a block of data on the SW or on AES machine.
 *        This function should be called after the appropriate CRYS AES init function 
 *        (according to used AES operation mode).
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
 CIMPORT_C CRYSError_t  CRYS_AES_Block( CRYS_AESUserContext_t   *ContextID_ptr,   
                                        DxUint8_t               *DataIn_ptr,     
                                        DxUint32_t               DataInSize,     
                                        DxUint8_t               *DataOut_ptr ); 
                              

/****************************************************************************************************/
/**
 * @brief This function is used to end the AES operation session on all modes besides XCBC,CMAC,XTS.
 *        It is the last function called on the AES operation on appropriate modes .
 *
 *
 *        The function executes the following major steps:
 *
 *        1. Checks the validity of all of the inputs of the function. 
 *           If one of the received parameters is not valid it shall return an error.
 *           The major checkers that are run over the received parameters:
 *           - verifying that the context pointer is not DX_NULL (*ContextID_ptr). 
 *        2. Clears the users context.
 *        3. Exits the handler with the OK code.
 *      
 *
 * @param[in] ContextID_ptr - a pointer to the AES context buffer allocated by the user that
 *                       is used for the AES machine operation. this should be the same context that was
 *                       used on the previous call of this session.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure - a value defined in CRYS_AES_error.h
 */
CIMPORT_C CRYSError_t  CRYS_AES_Free(CRYS_AESUserContext_t  *ContextID_ptr );


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
CIMPORT_C CRYSError_t  _DX_AES( CRYS_AES_IvCounter_t       IVCounter_ptr,       
		                        CRYS_AES_Key_t             Key_ptr,								
		                        CRYS_AES_KeySize_t         KeySize,            
		                        CRYS_AES_EncryptMode_t     EncryptDecryptFlag, 
		                        CRYS_AES_OperationMode_t   OperationMode ,      
		                        DxUint8_t                  is_secret_key,       
		                        DxUint8_t                  *DataIn_ptr,        
		                        DxUint32_t                 DataInSize,         
		                        DxUint8_t                  *DataOut_ptr );     


/****************************************************************************************************/
/**
  \brief The macros that the user should call when operating the AES in one operation.
   Note that the return value is CRYSError_t.
   
   Macro CRYS_AES - is called when the user performs an AES operation with a key supplied by him.
   Macro CRYS_AES_SecretKey - is called when the user performs an AES operation using the secret key on the chip.  
   Macro CRYS_AES_XCBCandCMAC - is called when the user is using a AES_XCBC_MAC or AES_XCBC_CMAC modes.  
   Macro CRYS_AES_XCBCandCMAC_SecretKey - is called when the user is using a AES_XCBC_MAC or AES_XCBC_CMAC modes with secret key. 
   Macro CRYS_AES_XTS - is called when the user is using a AES_XTS mode.
     
*/

#define CRYS_AES( IVCounter_ptr,Key_ptr,KeySize,EncryptDecryptFlag,OperationMode,DataIn_ptr,DataInSize,DataOut_ptr ) \
     _DX_AES( (IVCounter_ptr),(Key_ptr),(KeySize),(EncryptDecryptFlag),(OperationMode),DX_FALSE,(DataIn_ptr),(DataInSize),(DataOut_ptr))  

#define CRYS_AES_SecretKey( IVCounter_ptr,EncryptDecryptFlag,OperationMode,DataIn_ptr,DataInSize,DataOut_ptr )   \
     _DX_AES( (IVCounter_ptr),(DxUint8_t *)DX_NULL,CRYS_AES_Key128BitSize,(EncryptDecryptFlag),(OperationMode),DX_TRUE,(DataIn_ptr),(DataInSize),(DataOut_ptr))  

#define CRYS_AES_XCBCandCMAC( Key_ptr,KeySize,OperationMode,DataIn_ptr,DataInSize,DataOut_ptr ) \
     _DX_AES( DX_NULL,(Key_ptr),(KeySize),CRYS_AES_Encrypt,(OperationMode),DX_FALSE,(DataIn_ptr),(DataInSize),(DataOut_ptr))  

#define CRYS_AES_XCBCandCMAC_SecretKey( OperationMode,DataIn_ptr,DataInSize,DataOut_ptr ) \
     _DX_AES( (DxUint8_t*)DX_NULL,(DxUint8_t*)DX_NULL,CRYS_AES_Key128BitSize,CRYS_AES_Encrypt,(OperationMode),DX_TRUE,(DataIn_ptr),(DataInSize),(DataOut_ptr))  

#define CRYS_AES_XTS( Tweak_ptr, Key1_ptr,KeySize,EncryptDecryptFlag,DataIn_ptr,DataInSize,DataOut_ptr ) \
	_DX_AES( (Tweak_ptr),(Key1_ptr),(KeySize),(EncryptDecryptFlag),CRYS_AES_XTS_mode,DX_FALSE,(DataIn_ptr),(DataInSize),(DataOut_ptr))  

/****************************************************************************************************/
/** 
 * @brief This function is used as finish operation of AES on XCBC, CMAC, CBC and other modes 
 *        besides XTS mode.
 *
 *        The function must be called after AES_Block operations on CRYS_AES_XCBC_MAC_mode and 
 *        CRYS_AES_CMAC_mode. On other modes it may be called also instead the CRYS_AES_Free
 *        function. 
 *
 *        The actual macros, that will be used by the user for calling this function on different modes, 
 *        are described in CRYS_AES.h file.
 *
 *        The function performs  specific operations for last blocks of data and puts out the result 
 *        authentication value on XCBC, CMAC and MAC modes. After all operations the function 
 *        cleanes secure sensetive data from context.  
 *
 * @param[in] ContextID_ptr - A pointer to the AES context buffer allocated by the user that
 *                            should be the same context that was used on the previous call
 *                            of this session.
 * @param[out] MacResult  - A pointer to the buffer for output MAC data from the AES. This parameter 
 *                          is actual for XCBC-MAC and CMAC modes only. The pointer does not need to
 *                          be aligned.   
 * @return CRYSError_t    - On success CRYS_OK is returned, on failure - a value defined in CRYS_AES_error.h.
 */
 CIMPORT_C CRYSError_t  _DX_AES_Finish( CRYS_AESUserContext_t     *ContextID_ptr,  
                                        CRYS_AES_MAX_MAC_RESULT_t  MacResult );
                                        
 /**
  \brief The macros that the user should call when performs the AES finish operation on
         different modes.          
   
   Macro CRYS_AES_Finish  - is called when the user performs finish operation on all modes instead 
         CRYS_AES_XCBC_MAC_mode, CRYS_AES_CMAC_mode, CRYS_AES_XTS_mode.
         In fact, this macro performs the CRYS_Free operation and replace it.
         
   Macro CRYS_AES_XCBCandCMAC_Finish  - is called when the user performs specific finish operation on 
         CRYS_AES_XCBC_MAC_mode, CRYS_AES_CMAC_mode modes only. 
         NOTE: The size of CRYS_AES_XCBCandCMAC_Finish output is 16 bytes, but user must use only the  
               beginning part of this MAC result according to used mode, in particular:
                  - AES_XCBC_MAC_96    - 12 bytes of CRYS_AES_XCBC_MAC_mode result,
                  - AES_XCBC_PRF_128   - 16 bytes of CRYS_AES_XCBC_MAC_mode result,
                  - AES_CMAC           - as defined by user application, prior to 16 bytes of 
				                         CRYS_AES_XCBC_MAC_mode result.                 
    
   Macro CRYS_AES_MAC_Finish  - is called when the user performs finish operation on CRYS_AES_MAC_mode,
                                in this case the function puts out the last MAC result calculated in 
								Block operation and cleans the context buffer.        
*/
CEXPORT_C CRYSError_t  CRYS_AES_Finish(CRYS_AESUserContext_t *ContextID_ptr, DxUint8_t *DataIn_ptr,DxUint32_t DataInSize, DxUint8_t *DataOut_ptr);  //for cc5
  /*#define CRYS_AES_Finish( ContextID_ptr ) \
          _DX_AES_Finish( (ContextID_ptr), DX_NULL ) */                                                 

  #define CRYS_AES_XCBCandCMAC_Finish( ContextID_ptr, MacResult ) \
          _DX_AES_Finish( (ContextID_ptr) , (MacResult) )

  #define CRYS_AES_MAC_Finish( ContextID_ptr, MacResult ) \
          _DX_AES_Finish( (ContextID_ptr) , (MacResult) )
          

/****************************************************************************************************/
/** 
 * @brief This function is used as finish operation of AES on XTS mode.
 *
 *        The function must be called after AES_Block operations. 
 *        The function performs  specific operations for two last blocks of data, puts out the result 
 *        values and deletes secure sensitive data from context as follows:  
 *        
 *        1. Checks the validation of all of the inputs of the function.
 *           If one of the received parameters is not valid it shall return an error.
 *        2. Decrypts the received context to the working context  by calling the 
 *           CRYS_CCM_GetContext function.
 *        3. Calls the LLF_AES_XTS_Finish function. 
 *        4. Cleans working context.
 *        5. Exits   
 *      
 * @param[in] ContextID_ptr - A pointer to the AES context buffer allocated by the user that
 *                            should be the same context that was used on the previous call
 *                            of this session.
 * @param[in] DataIn_ptr - A pointer to the buffer of the input data. The pointer does not need to 
 *                         be aligned. In case DataInSize = 0 the pointer may be NULL or other value.
 * @param[in] DataInSize - A size of the input data. The size must be:
 *                         - 0, if full data size is multiple of 16 bytes, 
 *                         - or 17 <= DataInSize <= 31 bytes, if full data size is not multiple of 16 bytes. 
 * 
 * @param[out] DataOut_ptr - A pointer to the output buffer for XTS result. The pointer does not need 
 *                           to be aligned to 32 bits.  
 * @return CRYSError_t    - On success CRYS_OK is returned, on failure - a value defined in CRYS_AES_error.h.
 */
 CIMPORT_C CRYSError_t  CRYS_AES_XTS_Finish( 
	                                    CRYS_AESUserContext_t     *ContextID_ptr,  
	                                    DxUint8_t                 *DataIn_ptr,     
                                        DxUint32_t                 DataInSize,
										DxUint8_t                 *DataOut_ptr );
										
										
/****************************************************************************************************/
/** 
 * @brief This function is used as finish operation of AES on CMAC mode.
 *
 *        The function must be called after AES_Block operations. This function is must process last block of data, whether 
 *        the block is full or partial. DataSize can not be 0 and can not be be more then AES block size
 *        The function performs  specific operations for last block of data, puts out the result 
 *        values and deletes secure sensitive data from context as follows:  
 *        
 *        1. Checks the validation of all of the inputs of the function.
 *           If one of the received parameters is not valid it shall return an error.
 *        2. Decrypts the received context to the working context  by calling the 
 *           CRYS_CCM_GetContext function.
 *        3. Calls the LLF_AES_CMAC_Finish function. 
 *        4. Cleans working context.
 *        5. Exits   
 *      
 * @param[in] ContextID_ptr - A pointer to the AES context buffer allocated by the user that
 *                            should be the same context that was used on the previous call
 *                            of this session.
 * @param[in] DataIn_ptr - A pointer to the buffer of the input data. The pointer does not need to 
 *                         be aligned. In case DataInSize = 0 the pointer may be NULL or other value.
 * @param[in] DataInSize - A size of the input data. The size must be:
 *                         - 0, if full data size is multiple of 16 bytes, 
 *                         - or 17 <= DataInSize <= 31 bytes, if full data size is not multiple of 16 bytes. 
 * 
 * @param[out] MacResult - The result of the operation
 *  
 * @return CRYSError_t    - On success CRYS_OK is returned, on failure - a value defined in CRYS_AES_error.h.
 */
 CIMPORT_C CRYSError_t  CRYS_AES_CMAC_Finish(CRYS_AESUserContext_t*     ContextID_ptr,  
	                                           DxUint8_t*                 DataIn_ptr,
	                                           DxUint32_t                 DataInSize,
	                                           CRYS_AES_MAX_MAC_RESULT_t  MacResult );


/**************************************************************************
 *	              CRYS_AES_Wrap function                                  *
 **************************************************************************/
/**
   @brief  The CRYS_AES_Wrap function implements the following algorithm 
           (rfc3394, Sept. 2002):  
   
   Inputs:  Plaintext DataIn, n 64-bit values {P1, P2, ..., Pn}, 
            KeyData, K (the KEK).                
   Outputs: Ciphertext, WrapDataOut (n+1) 64-bit values {C0, C1, ..., Cn}.
  

  @param[in]  DataIn_ptr - A pointer to plain text data to be wrapped
                            NOTE: Overlapping between the data input and data output buffer
                                  is not allowed, except the inplace case that is legal . 			
   @param[in]  DataInLen  - Length of data in bytes. DataLen must be multiple of 
                            8 bytes and  must be in range [8, 2^28].
   @param[in]  KeyData	  - A pointer to  key data (key encryption key - KEK). 
   @param[in]  KeySize	  - Enumerator variable, defines length of key.
   @param[in]  isSecretKey - Variable, defining is secret key used (1) or not (0).
   @param[out] WrapDataOut_ptr -	A pointer to buffer for output of wrapped data.
   @param[in/out] WrapDataLen_ptr - A pointer to a buffer for input of size of 
                                    user passed buffer and for output actual 
                                    size of unwrapped data in bytes. Buffer size must 
                                    be not less than DataLen+CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES.                         
   @return CRYSError_t - CRYS_OK, or error message                         
                         CRYS_AES_WRAP_ILLEGAL_DATA_PTR_ERROR            
                         CRYS_AES_WRAP_DATA_LENGTH_ERROR                 
                         CRYS_AES_WRAP_ILLEGAL_KEY_PTR_ERROR             
                         CRYS_AES_WRAP_KEY_LENGTH_ERROR                  
                         CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_PTR_ERROR      
                         CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_LEN_PTR_ERROR  
                         CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_LENGTH_ERROR
						 CRYS_AES_WRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR 
						 CRYS_AES_WRAP_IS_SECRET_KEY_FLAG_ILLEGAL_ERROR

     NOTE:  On error exiting from function the output buffer may be zeroed by the function.
*/
CIMPORT_C  CRYSError_t CRYS_AES_Wrap (
                               DxUint8_t            *DataIn_ptr,      /*in*/   
                               DxUint32_t            DataInLen,       /*in*/
                               CRYS_AES_Key_t        KeyData,         /*in*/
                               CRYS_AES_KeySize_t    KeySize,         /*in*/
							   DxInt8_t              isSecretKey,     /*in*/
                               DxUint8_t            *WrapDataOut_ptr, /*out*/
                               DxUint32_t           *WrapDataLen_ptr  /*in/out*/ ); 



/**************************************************************************
 *	              CRYS_AES_Uwnrap function                                *
 **************************************************************************/
/**
   @brief  The CRYS_AES_Unwrap function performs inverse AES_Wrap transformation 
           and implements the following algorithm (rfc3394, Sept. 2002):  

   Inputs:  Ciphertext, (n+1) 64-bit values {C0, C1, ..., Cn}, and
            K  - key (the KEK).
  
   Outputs: Plaintext, n 64-bit values {P1, P2, ..., Pn}.
   
		       
   @param[in]  WrapDataIn_ptr - A pointer to wrapped data to be unwrapped 
                                NOTE: Overlapping between the data input and data output buffer
                                      is not allowed, except the inplace case that is legal . 			
   @param[in]  WrapDataInLen  - Length of wrapped data in bytes. DataLen must be multiple of 
                                8 bytes and  must be in range [16, 2^29].
   @param[in]  KeyData        - A pointer to  key data (key encryption key - KEK). 
   @param[in]  KeySize        - Enumerator variable, defines length of key.
   @param[in]  isSecretKey - Variable, defining is secret key used (1) or not (0).
   @param[out] DataOut_ptr     - A pointer to buffer for output of unwrapped data.
   @param[in/out]  DataOutLen_ptr - A pointer to a buffer for input of size of user passed 
                              buffer and for output of actual size of unwrapped data in bytes.
                              DataOutLen must be multiple of 8 bytes and must be not less
                              than WrapDataInLen - CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES.
                           
   @return CRYSError_t - CRYS_OK, or error message 
                         CRYS_AES_UNWRAP_WRAP_DATA_LENGTH_ERROR          
                         CRYS_AES_UNWRAP_ILLEGAL_KEY_PTR_ERROR           
                         CRYS_AES_UNWRAP_KEY_LEN_ERROR                   
                         CRYS_AES_UNWRAP_ILLEGAL_DATA_PTR_ERROR          
                         CRYS_AES_UNWRAP_ILLEGAL_DATA_LEN_PTR_ERROR      
                         CRYS_AES_UNWRAP_ILLEGAL_DATA_LENGTH_ERROR       
                         CRYS_AES_UNWRAP_FUNCTION_FAILED_ERROR 
						 CRYS_AES_UNWRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR 
						 CRYS_AES_UNWRAP_IS_SECRET_KEY_FLAG_ILLEGAL_ERROR

    NOTE:  On error exiting from function the output buffer may be zeroed by the function.
                         
*/                               
CIMPORT_C  CRYSError_t CRYS_AES_Unwrap(
                                   DxUint8_t            *WrapDataIn_ptr, /*in*/   
                                   DxUint32_t            WrapDataInLen,    /*in*/
                                   CRYS_AES_Key_t        KeyData,        /*in*/
                                   CRYS_AES_KeySize_t    KeySize,        /*in*/ 
								   DxInt8_t              isSecretKey,     /*in*/
                                   DxUint8_t            *DataOut_ptr,    /*out*/
                                   DxUint32_t           *DataOutLen_ptr     /*in/out*/ );
  

/***********************************************************************************/


#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_AES_H */ 


/**************************************************/

#else /* CRYS_AES_CF_VERSION_USED */

#include "CRYS_AES_CF.h"

#endif



