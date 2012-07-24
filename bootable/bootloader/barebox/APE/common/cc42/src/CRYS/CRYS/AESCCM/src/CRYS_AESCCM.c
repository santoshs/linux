
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  14 Sept. 2008
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_AESCCM.c#1:csrc:6
   *  \author R.Levin
   *  \remarks Copyright (C) 2008 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

/* .............. CRYS level includes ................. */

#include "DX_VOS_Mem.h"
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_CCM.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_AESCCM_Local.h"
#include "CRYS_AESCCM_error.h"
#include "LLF_AESCCM.h"


/* .............. LLF level includes .................. */

#if !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)
#include "LLF_AES.h"
#include "LLF_AESCCM.h"
#endif /* CRYS_NO_AESCCM_SUPPORT */

/******************************** Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************* MACROS ***************************************/

/* the minimum and maximum macros */
#undef min
#define min( a , b ) ( ( (a) < (b) ) ? (a) : (b) )

#undef max
#define max( a , b ) ( ( (a) > (b) ) ? (a) : (b) )

/************************ Global Data ***********************************/

/********************* Private functions  *******************************/


/************************ Public Functions ******************************/


/****************************************************************************************************/
/**
* @brief This function initializes the AESCCM machine or SW structures and calculates
*        AES-MAC of control block B0 of payload.
*
*        To perform the AESCCM operations this function should be called the first.
*        The function executes the following steps (algorithm according to NIST 800-38C):
*
*        1. Validates all of the inputs of the function. If one of the received
*           parameters is not valid it shall return an error:
*        2. Allocates the working context by calling the CRYS_CCM_GetContext() function.
*        3. Initializes the context and performs formatting of the input additional data:
*           block B0, containing control information, and 'a' field of block B1. This field
*           are saved in the lastBlock buffer of the context.
*        4. Calculates AES-MAC of block B0 and saves it in the AESCCM_Context CCM_IV.
*        5. Sets sizes of formatted additional (without padding) and text data in the
*           RemainAsize and RemainTextSize fields of CCM context.
*        6. Encrypts the working context and stores it to the users context.
*           Releases the working context by calling the CRYS_CCM_EncryptAndReleaseContext function.
*        7. Exits the handler with the OK code.
*
*        Note: Parameters of AESCCM algorithm: CCM_Key, Nonce, QFieldSize, SizeOfN, SizeOfT, AdataSize,
*              TextSizeQ must be the same for both CCM-encrypt and CCM-decrypt operations on the
*              same given data.
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer, that is allocated by the user
*                            and is used for the AESCCM operations.
* @param[in] EncrDecrMode  - Enumerator variable defining operation mode (0 - encrypt; 1 - decrypt).
* @param[in] CCM_Key       - A buffer, containing the AESCCM key passed by user (predefined size 128 bits).
* @param[in] KeySizeId     - An ID of AESCCM key size (according to 128, 192, or 256 bits size).
* @param[in] isSecretKey   - This parameter intended for internal Discretix using only !.
*                            The parameter specifying whether to use a secret key (=TRUE)
*                            or to use a key defined by the user (=FALSE).
*                            The secret key function depends on product configuration.
*                            Please consult the reference manual for AES functions.
* @param[in] AdataSize     - Full size of additional data in bytes, which must be processed.
*                            Limitation in our implementation is: AdataSize < 2^32. If Adata is absent,
*                            then AdataSize = 0.
* @param[in] TextSizeQ     - The full size of text data (in bytes), which must be processed by CCM.

* @param[in] QFieldSize    - Byte-size of formatted field for writing significant bytes of the TextSizeQ
*                            value. Valid values: [2,3,4,5,6,7,8].
* @param[in] N_ptr	       - A pointer to Nonce - unique value assigned to all data passed into CCM.
*                            Bytes order - big endian form (MSB is the first).
* @param[in] SizeOfN       - The size of the user passed Nonce (in bytes).	Valid values:
*                            7 <= SizeOfN <= (15-QFieldSize).
* @param[in] SizeOfT	   - Size of AESCCM MAC output T in bytes. Valid values: [4,6,8,10,12,14,16].
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to CRYS_AESCCM_error.h
*/
CEXPORT_C CRYSError_t  CRYS_AESCCM_Init(
										CRYS_AESCCM_UserContext_t   *ContextID_ptr,
										CRYS_AES_EncryptMode_t       EncrDecrMode,
										CRYS_AESCCM_Key_t            CCM_Key,    /*AESCCM key*/
										CRYS_AESCCM_KeySize_t        KeySizeId,  /* key size ID*/
										DxUint8_t                    isSecretKey, /*secret key flag*/
										DxUint32_t                   AdataSize,  /*full size of additional data*/
										DxUint32_t                   TextSizeQ,  /*full size of text data*/
										DxUint8_t                    QFieldSize, /*size of Q field in control block*/
										DxUint8_t                   *N_ptr,      /*Nonce*/
										DxUint8_t                    SizeOfN,    /*size of N buffer*/
										DxUint8_t                    SizeOfT )   /*size of CCM-MAC (T) */


{
   /* FUNCTION DECLARATIONS */

   /* The return error identifiers */
   CRYSError_t Error, ErrorCcm;

   /* defining a pointer to the active context allocated by the CCM */
   AESCCM_Context_t *ccmWorkingContext_ptr;
   AESContext_t     *AESContext_ptr;

   /* temp buffer pointer */
   DxUint8_t *temp_ptr;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AESCCM_UNSUPPORTED( ContextID_ptr , EncrDecrMode , CCM_Key[0] , KeySizeId , AdataSize ,
                                 TextSizeQ , QFieldSize , N_ptr , SizeOfN , SizeOfT , isSecretKey ,
                                 ccmWorkingContext_ptr , AESContext_ptr , ErrorCcm , temp_ptr ,
								 Error , Error , Error , Error , Error ,
								 Error , Error , Error , Error, Error);

#if !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_AESCCM_INVALID_USER_CONTEXT_POINTER_ERROR;

   /*Init the valid_tag filed to avoid warnings*/
   ContextID_ptr->valid_tag = 0;

   /* Check the Encrypt / Decrypt mode */
   if( EncrDecrMode >= CRYS_AES_EncryptNumOfOptions )
	   return  CRYS_AESCCM_INVALID_ENCRYPT_MODE_ERROR;

   /* check Key and Nonce pointers */
   if( N_ptr == DX_NULL )
      return  CRYS_AESCCM_ILLEGAL_PARAMETER_PTR_ERROR;

   if (isSecretKey != DX_TRUE && CCM_Key == DX_NULL)
      return  CRYS_AESCCM_ILLEGAL_PARAMETER_PTR_ERROR;

   /* check Key size ID */
   if( KeySizeId > CRYS_AES_Key256BitSize )
	   return  CRYS_AESCCM_ILLEGAL_KEY_SIZE_ERROR;

   /* check the Q field size: according to our implementation QFieldSize <= 4*/
   if( QFieldSize < 2 || QFieldSize > 8 )
	   return  CRYS_AESCCM_ILLEGAL_PARAMETER_SIZE_ERROR;

   /* check, that actual size of TextData is not great, than its control field size */
   if( (QFieldSize < 4) && ((TextSizeQ >> (QFieldSize*8)) > 0) )
      return CRYS_AESCCM_ILLEGAL_PARAMETER_SIZE_ERROR;

   /* check Nonce size. Note: QFieldSize + SizeOfN <= 15 */
   if( SizeOfN < 7 || SizeOfN > 15 - QFieldSize )
      return  CRYS_AESCCM_ILLEGAL_PARAMETER_SIZE_ERROR;

    /* check CCM MAC size: [4,6,8,10,12,14,16] */
   if( SizeOfT < 4   || SizeOfT > 16  || (SizeOfT & 1) != 0 )
	   return  CRYS_AESCCM_ILLEGAL_PARAMETER_SIZE_ERROR;

   /* check, the text data size on encrypt and decrypt modes */


   /* check secret key flag */
   if( isSecretKey != DX_TRUE && isSecretKey != DX_FALSE )
	   return CRYS_AESCCM_ILLEGAL_SECRET_KEY_ERROR;


   /* ................. acquiring the AES contest ........................... */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,                      /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,   /* the CCM context returned - out */
                                DX_AESCCM_CONTEXT,                  /* AESCCM type - in */
                                AES_DONT_DECRYPT_CONTEXT);		    /*No need to decrypt context in Init*/
   if( Error != CRYS_OK )

      return Error;

   /* clean working context */
   DX_VOS_MemSetZero( ccmWorkingContext_ptr, sizeof(AESCCM_Context_t) );


   /* ................. loading the context data ............................ */
   /* ----------------------------------------------------------------------- */
   /* set pointer to AESContext inside the AESCCM context */
   AESContext_ptr = (AESContext_t*)ccmWorkingContext_ptr;


   /* Data in the AESContext inside the AESCCM context */
   AESContext_ptr->AESContextKeySize  = KeySizeId;
   AESContext_ptr->is_secret_key = isSecretKey;

   /* if not secret key is used, then copy the user passed key into context */
   if( isSecretKey == DX_FALSE )
	  DX_VOS_FastMemCpy( (DxUint8_t*)AESContext_ptr->AESContextKey, (DxUint8_t*)CCM_Key, sizeof(CRYS_AESCCM_Key_t) );

   /* set current AES operation mode for AES_CBC_MAC operation on first block B0 and next blocks */
   AESContext_ptr->EncryptDecryptFlag = CRYS_AES_Encrypt;
   AESContext_ptr->OperationMode      = CRYS_AES_MAC_mode;

   /* set data into AESCCM part of the context */
   /*------------------------------------------*/

   /* control field sizes */
   ccmWorkingContext_ptr->QFieldSize = QFieldSize;
   ccmWorkingContext_ptr->SizeOfN = SizeOfN;
   ccmWorkingContext_ptr->SizeOfT = SizeOfT;

   /* AESCCM encrypt_decrypt mode */
   ccmWorkingContext_ptr->CCM_EncryptMode = EncrDecrMode;

   /* set CurrentDataType for next Update operation on Adata*/
   ccmWorkingContext_ptr->CurrentDataType = CRYS_AESCCM_FirstAdata;

   /* set the AESCCM tag to the users context */
   ContextID_ptr->valid_tag = AESCCM_CONTEXT_VALIDATION_TAG;


   /* ----------------------------------------------------------------------- */
   /*       formatting the first block of CCM input B0, using temp buffer     */
   /* ----------------------------------------------------------------------- */

   /* set temp_ptr to ccmWorkingContext_ptr->lastBlock, used now as temp buffer */
   temp_ptr = (DxUint8_t*)ccmWorkingContext_ptr->lastBlock;

   /* set Adata flag (bit 6 for big endian form) */
   if( AdataSize > 0 )
	   temp_ptr[0] = 1 << 6;

   /* next flag bits: (t-2)/2 concatenated with (q-1) */
   temp_ptr[0] |= (SizeOfT - 2)/2 << 3;   /* bits 3 - 5 */
   temp_ptr[0] |= (QFieldSize - 1);       /* bits 0 - 2 */

   /* set N and Q (in big endian form) into B0 */
   DX_VOS_FastMemCpy( temp_ptr + 16 - QFieldSize - SizeOfN , N_ptr, SizeOfN );
   CRYS_COMMON_ReverseMemcpy( temp_ptr + 16 - min(QFieldSize, 4), (DxUint8_t*)&TextSizeQ, min(QFieldSize, 4) );


   /* ----------------------------------------------------------------------- */
   /* ................. calculating AES_CCM_MAC of block B0 ................. */
   /* ----------------------------------------------------------------------- */

   Error = LLF_AES_InitInit( AESContext_ptr );

   if( Error != CRYS_OK )
     goto End;

   /* calculating MAC and save it in CCM_IV*/
   Error = LLF_AES_Block( AESContext_ptr,
	                      temp_ptr, /* data in*/
						  CRYS_AES_BLOCK_SIZE_IN_BYTES,
						  (DxUint8_t*)ccmWorkingContext_ptr->CCM_IV /* data out*/);

   if( Error != CRYS_OK )
	   goto End;

   /* ----------------------------------------------------------------------- */
   /* ...... formatting ASize and setting it into AES lastBlock buffer ...... */
   /* ----------------------------------------------------------------------- */

   /* zero the lastBlock temp buffer */
   DX_VOS_MemSetZero( temp_ptr, CRYS_AES_BLOCK_SIZE_IN_BYTES );

   /* set formatted ASize into temp buffer.
      Note: case ASize > 2^32 is not allowed by implementation */

   /* if ASize > 2^16-2^8, then set 6 bytes = 0xff||0xfe||ASize */
   if ( AdataSize >= 0xff00 ) /* if ASize >= 2^16 - 2^8, then set 0xff||0xfe||ASize */
   {
	   temp_ptr[0] = 0xff;
	   temp_ptr[1] = 0xfe;
	   /* reverse copy 4 bytes */
	   CRYS_COMMON_ReverseMemcpy( temp_ptr + 6 - 4, (DxUint8_t*)&AdataSize, 4 );
	   ccmWorkingContext_ptr->lastBlockSize = 6;
	   ccmWorkingContext_ptr->SizeOfA = 6;
   }
   else if ( AdataSize > 0 ) /* else if 0 < ASize < 2^16 - 2^8, then set 2 bytes = ASize */
   {
	   /* reverse copy 2 bytes */
	   CRYS_COMMON_ReverseMemcpy( temp_ptr + 2 - 2, (DxUint8_t*)&AdataSize, 2 );
	   ccmWorkingContext_ptr->lastBlockSize = 2;
	   ccmWorkingContext_ptr->SizeOfA = 2;
   }
   else /* no Adata */
   {
	   ccmWorkingContext_ptr->lastBlockSize = 0;
	   ccmWorkingContext_ptr->SizeOfA = 0;
       /* set data type according to text data */
	   ccmWorkingContext_ptr->CurrentDataType = CRYS_AESCCM_FirstTextData;
   }

   /* set the formatted AdataSize to remaining Adata size */
   ccmWorkingContext_ptr->RemainAdataSize = AdataSize;
   if( AdataSize > 0 )
	   ccmWorkingContext_ptr->RemainAdataSize += ccmWorkingContext_ptr->SizeOfA;

   /* set full TextSize Q into context remaining size */
   ccmWorkingContext_ptr->RemainTextSize = TextSizeQ;


   /*--------------------------------------------------------------------------------------*/
   /* set initial Counter value into CCM_CTR buffer for AES CTR operations on text data    */
   /*--------------------------------------------------------------------------------------*/

   /* Flags byte = (QFieldSize - 1) */
   temp_ptr = (DxUint8_t*)ccmWorkingContext_ptr->CCM_CTR;
   temp_ptr[0] = QFieldSize - 1;

   /* copy Nonce into bytes 1...(15-QFieldSize) in big endian form */
   DX_VOS_FastMemCpy( temp_ptr + 16 - QFieldSize - SizeOfN , N_ptr, SizeOfN );
   /* set counter i = 1 (in byte 15) */
   temp_ptr[15] = 1;  /* Note: value i = 0 reserved for encoding MAC value */

 End:
   /*-------------------------------------------------------------------------*/
   /* ................. release the context ................................. */
   /*-------------------------------------------------------------------------*/

    ErrorCcm = CRYS_CCM_ReleaseContext( ContextID_ptr,          /* the users context space - in */
                                        ccmWorkingContext_ptr,  /* the CCM context returned - in */
                                        DX_AESCCM_CONTEXT);     /* AESCCM type - in */


   /* ...................... end of function ................................ */

   /*if the error occurred before release context do not override it*/
   if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
		Error = ErrorCcm;

   if( Error != CRYS_OK )
       /* clearing the users context */
	   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_AESCCM_UserContext_t) );


   return Error;

#endif /* !CRYS_NO_AESCCM_SUPPORT */

}/* END OF CRYS_AESCCM_Init */



/****************************************************************************************************/
/**
* @brief This function calculates (updates) AESCCM MAC on current block of additional data.
*
*       This function is called for each block of additional data, if it present, and
*       executes the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*
*        2.Decrypts the received context to the working context after
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.If AdataBlock is not last and herewith DataInSize%16 != 0, then the
*        4.If DataIn_ptr = NULL or DataInSize = 0, the function returns an Error.
*          function returns an error, else the function concatenates the data, saved
*          in the lastBlock of the context with some bytes of input data into full AES-block
*          and calculates (updates)AES-MAC value by invoking LLF_AESCCM_BlockAdata function.
*        4.Updates RemainAsize field according to received input data size.
*        5.Encrypts the working context and stores it to the users received context.
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        6.Exits the handler with the OK code.
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer allocated by the user that
*                            is used for the AESCCM machine operation. This should be the same
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input additional data. It is required
*                         DataIn_ptr != NULL. The pointer does not need to be aligned.
*                         On CSI input mode the pointer must be equal to value
*                         (0xFFFFFFFC | DataInAlignment).
* @param[in] DataInSize   - A size of the additional data in bytes. Required: DataInSize > 0.
*                           Size of each intermediate block of data must be multiple of 16 bytes,
*                           excluding the last block which may have any size.
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to
*                       CRYS_AESCCM_error.h
*   Notes:
*      1. Temporary the CSI input is not allowed on CCM mode.
*
*/
CEXPORT_C CRYSError_t  CRYS_AESCCM_BlockAdata(
											CRYS_AESCCM_UserContext_t    *ContextID_ptr,
											DxUint8_t                    *DataIn_ptr,
											DxUint32_t                    DataInSize )
 {
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;
   CRYSError_t ErrorCcm;

   /* defining a pointers to the AESCCM and AES contexts */
   AESCCM_Context_t *ccmWorkingContext_ptr;

   /* reminder from dividing the block size by CRYS_AES_BLOCK_SIZE_IN_BYTES */

   CRYS_AESCCM_DataType_t CurrentDataType;

   /* size of data to save in context for next CRYS_AESCCM_BlockAdata */
   DxUint32_t remainDataSize, dataSizeToContext;


   /******************* FUNCTION LOGIC *************************************/

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AESCCM_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize ,
                              ccmWorkingContext_ptr , CurrentDataType , remainDataSize ,
							  dataSizeToContext ,ErrorCcm , Error , Error , Error , Error ,
							  Error , Error , Error , Error , Error , Error , Error ,
							  Error ,Error , Error , Error , Error , Error );

#if !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_AESCCM_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != AESCCM_CONTEXT_VALIDATION_TAG )
      return CRYS_AESCCM_USER_CONTEXT_CORRUPTED_ERROR;

   /* if the users dataIn pointer is illegal return an error */
   if( DataIn_ptr == DX_NULL )
      return CRYS_AESCCM_DATA_IN_POINTER_INVALID_ERROR;

   if( (DxUint32_t)DataIn_ptr >= DX_CSI )
	   return CRYS_AESCCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR;

   /* if the data size is illegal return an error */
   if( DataInSize == 0 )
	   return CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL;



   /* ................. acquiring the AES contest ............................ */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_AESCCM_CONTEXT,                   /* AES type - in */
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/
   if( Error != CRYS_OK )
     return Error;


   /* ...... checking the data size and data in / out overlapping ........... */
   /* ----------------------------------------------------------------------- */

   /* set variables */
   CurrentDataType = ccmWorkingContext_ptr->CurrentDataType;

   /* check the data type mode */
   if( CurrentDataType > CRYS_AESCCM_LastAdata )
   {
	   Error = CRYS_AESCCM_ILLEGAL_DATA_TYPE_ERROR;
	   goto End;
   }

    /* if all Adata was processed, then additional block is not permitted:  return an Error */
   if( ccmWorkingContext_ptr->RemainAdataSize == 0 )
   {
	   Error = CRYS_AESCCM_ADDITIONAL_BLOCK_NOT_PERMITTED_ERROR;
	   goto End;
   }

   /* check that DataInSize is not great, than remaining Adata size */
   if( DataInSize + ccmWorkingContext_ptr->lastBlockSize > ccmWorkingContext_ptr->RemainAdataSize )
   {
	   Error = CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL;
	   goto End;
   }

   if( DataInSize + ccmWorkingContext_ptr->lastBlockSize == ccmWorkingContext_ptr->RemainAdataSize )
   {
	   ccmWorkingContext_ptr->CurrentDataType = CRYS_AESCCM_LastAdata;
   }
   else
   {
	   ccmWorkingContext_ptr->CurrentDataType = CRYS_AESCCM_IntermedAdata;

	   /* if data size is not multiple of 16 bytes or == 0 */
	   if( DataInSize % CRYS_AES_BLOCK_SIZE_IN_BYTES != 0 )
	   {
		   Error = CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL;
		   goto End;
	   }
   }

   /* ------------------------------------------------------------------------------ */
   /* ..........................  setting data into context buffer ................. */
   /* ------------------------------------------------------------------------------ */

   /* set Adata into context lastBlock buffer to fill it */
   if( ccmWorkingContext_ptr->lastBlockSize + DataInSize >= CRYS_AES_BLOCK_SIZE_IN_BYTES )
   {
	   /* the size of the data to write into context */
	   dataSizeToContext = CRYS_AES_BLOCK_SIZE_IN_BYTES - ccmWorkingContext_ptr->lastBlockSize;
       DX_VOS_FastMemCpy( (DxUint8_t*)ccmWorkingContext_ptr->lastBlock + ccmWorkingContext_ptr->lastBlockSize,
		                  DataIn_ptr, dataSizeToContext );

       /* update input pointer and size */
	   DataIn_ptr += dataSizeToContext;
       ccmWorkingContext_ptr->lastBlockSize = CRYS_AES_BLOCK_SIZE_IN_BYTES;
   }

   else /* may occur only for last block */
   {
       DX_VOS_FastMemCpy( (DxUint8_t*)ccmWorkingContext_ptr->lastBlock + ccmWorkingContext_ptr->lastBlockSize,
		                  DataIn_ptr, DataInSize );

       ccmWorkingContext_ptr->lastBlockSize += DataInSize;
       dataSizeToContext = DataInSize;
   }


   /* ------------------------------------------------------------------------------ */
   /* ...........  processing the Adata (if present) by LLF_AESCCM ................. */
   /* ------------------------------------------------------------------------------ */

   if( ccmWorkingContext_ptr->lastBlockSize + DataInSize - dataSizeToContext >= CRYS_AES_BLOCK_SIZE_IN_BYTES ||
       ccmWorkingContext_ptr->CurrentDataType == CRYS_AESCCM_LastAdata )
   {
       /* calculate remaining size for saving in context */
       if( ccmWorkingContext_ptr->CurrentDataType == CRYS_AESCCM_LastAdata )
		remainDataSize = 0;
       else
	        remainDataSize = (DataInSize - dataSizeToContext) % CRYS_AES_BLOCK_SIZE_IN_BYTES;

	   Error = LLF_AESCCM_BlockAdata(
								 ccmWorkingContext_ptr,         /* the working context - in */
								 DataIn_ptr,                    /* the input data buffer - in */
								 DataInSize - dataSizeToContext - remainDataSize ); /* the output data buffer i/o */
	   if ( Error != CRYS_OK )
		   goto End;

	   /* zeroing the lastBlock */
	   DX_VOS_MemSetZero( (DxUint8_t*)ccmWorkingContext_ptr->lastBlock, CRYS_AES_BLOCK_SIZE_IN_BYTES );

	   /* if Adata is not last, then copy not processed remaining data into context */
	   if( ccmWorkingContext_ptr->CurrentDataType < CRYS_AESCCM_LastAdata )
	   {
	       DX_VOS_FastMemCpy( (DxUint8_t*)ccmWorkingContext_ptr->lastBlock,
			                   DataIn_ptr + (DataInSize - dataSizeToContext - remainDataSize), remainDataSize );
		   /* update RemainAdataSize and lastBlockSize */
           ccmWorkingContext_ptr->lastBlockSize = remainDataSize;

		   /* update the context Adata remaining size  */
		   ccmWorkingContext_ptr->RemainAdataSize -= DataInSize;
	   }

	   else  /* if last Adata, prepare context to operations with TextData */
	   {
		   ccmWorkingContext_ptr->CurrentDataType = CRYS_AESCCM_FirstTextData;
		   ccmWorkingContext_ptr->lastBlockSize   = 0;
		   ccmWorkingContext_ptr->RemainAdataSize = 0;
	   }
   }


   /* ----------------------------------------------------------------------- */
   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

 End:

   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,		   /* the users context space - in */
                                      ccmWorkingContext_ptr,   /* the CCM context returned - in */
                                      DX_AESCCM_CONTEXT);      /* AESCCM type - in */

   /*if the error occurred before release context do not override it*/
   if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
   {
		Error = ErrorCcm;
   }

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   return Error;

#endif /* !CRYS_NO_AESCCM_SUPPORT */


}/* END OF CRYS_AESCCM_BlockAdata */


/****************************************************************************************************/
/**
* @brief This function encrypts (decrypts) the current block of text data and calculates
*        CCM_MAC value.
*
*       This function is called for each block of text data with size multiple 16 bytes,
*       excluding the last block, which must be processed by CRYS_AESCCM_BlockLastTextData
*       function.
*
*       The function executes the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*
*        2.Decrypts the received context to the working context after
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.If DataInSize = 0 or DataInSize%16 != 0, then the function returns an error.
*        4.Encrypts (decrypts)the input data and calculates AESCCM_MAC by invoking
*          LLF_AESCCM_BlockTextData function.
*        5.Updates RemainTextSize field of the context according to received input data size.
*        6.Encrypts the working context and stores it to the users received context.
*          Releases the working context by calling the CRYS_CCM_ReleaseContext function.
*        7.Exits.
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer allocated by the user that
*                            is used for the AES machine operation. This should be the same
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input data (plain or cipher text).
*                         The pointer does not need to be aligned. On CSI input mode the
*                         pointer must be equal to value (0xFFFFFFFC | DataInAlignment).
* @param[in] DataInSize  - A size of the data block in bytes: must be multiple of 16 bytes and not Null.
*                          The block of data must be not a last block, that means:
*                            - on Encrypt mode: DataInSize < CCM_Context->RemainTextSize;
*                            - on Decrypt mode: DataInSize < CCM_Context->RemainTextSize - SizeOfT;
* @param[in] DataOut_ptr - A pointer to the output buffer (cipher or plain text).
*                          The pointer does not need to be aligned. On CSI input mode the
*                          pointer must be equal to value (0xFFFFFFFC | DataOutAlignment).
*                          Size of the output buffer must be not less, than DataInSize.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_AESCCM_error.h
*   Notes:
*      1. Temporary the CSI input or output are not allowed on CCM mode.
*      2. Overlapping of the in-out buffers is not allowed, excluding the in placement case:
*         DataIn_ptr = DataOut_ptr.
*
*/
CEXPORT_C CRYSError_t  CRYS_AESCCM_BlockTextData(
											CRYS_AESCCM_UserContext_t    *ContextID_ptr,
											DxUint8_t                    *DataIn_ptr,
											DxUint32_t                    DataInSize,
											DxUint8_t                    *DataOut_ptr )
{
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;
	CRYSError_t ErrorCcm;

	/* defining a pointers to the AESCCM and AES contexts */
	AESCCM_Context_t *ccmWorkingContext_ptr;


	/******************* FUNCTION LOGIC *************************************/

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;


	/* ............... if not supported exit .............................. */
	/* -------------------------------------------------------------------- */

	RETURN_IF_AESCCM_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize ,
		DataOut_ptr , ccmWorkingContext_ptr , ErrorCcm , Error ,
		Error , Error , Error , Error , Error , Error ,
		Error , Error , Error , Error , Error , Error , Error ,
		Error , Error , Error , Error , Error );

#if !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

	/* ............... checking the parameters validity .................. */
	/* -------------------------------------------------------------------- */

	/* if the users context ID pointer is DX_NULL return an error */
	if( ContextID_ptr == DX_NULL )
		return CRYS_AESCCM_INVALID_USER_CONTEXT_POINTER_ERROR;

	/* if the users context TAG is illegal return an error - the context is invalid */
	if( ContextID_ptr->valid_tag != AESCCM_CONTEXT_VALIDATION_TAG )
		return CRYS_AESCCM_USER_CONTEXT_CORRUPTED_ERROR;

	/* if the users Data In pointer is illegal return an error */
	if( DataIn_ptr == DX_NULL )
		return CRYS_AESCCM_DATA_IN_POINTER_INVALID_ERROR;

	/* if the Data In size is 0, return an error */
	if( DataInSize == 0 )
		return CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL;

	if( ((DxUint32_t)DataIn_ptr >= DX_CSI) || ((DxUint32_t)DataOut_ptr >= DX_CSI) )
		return CRYS_AESCCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR;

	/* if the users Data Out pointer is illegal return an error */
	if( DataOut_ptr == DX_NULL )
		return CRYS_AESCCM_DATA_OUT_POINTER_INVALID_ERROR;

	/* check that DataInsize is multiple of AES- block size */
	if( DataInSize % CRYS_AES_BLOCK_SIZE_IN_BYTES != 0 )
		return CRYS_AESCCM_ILLEGAL_PARAMETER_SIZE_ERROR;


	/* ................. acquiring the AESCCM contest ........................ */
	/* ----------------------------------------------------------------------- */

	Error = CRYS_CCM_GetContext( ContextID_ptr, /* the users context space - in */
								(void **) &ccmWorkingContext_ptr,       /* the CCM context returned - out */
								DX_AESCCM_CONTEXT,                      /* AES type - in */
								AES_DECRYPT_CONTEXT);				    /* need to decrypt context in AES_block*/

	if( Error != CRYS_OK )

		return Error;

	/* check that there no additional data for processing */
	if( ccmWorkingContext_ptr->RemainAdataSize != 0 )
	{
		Error = CRYS_AESCCM_NOT_ALL_ADATA_WAS_PROCESSED_ERROR;
		goto End;
	}

	/* ...... checking the data size and data in/out overlapping ............. */
	/* ----------------------------------------------------------------------- */

	/* check that the current block is not the last block (i.e. DataInSize is not less,
	   than remaining TextData size according to Encrypt-Decrypt mode */

	 /* check that additional block is permitted */
	if( ccmWorkingContext_ptr->RemainTextSize == 0 )
	{
		Error = CRYS_AESCCM_ADDITIONAL_BLOCK_NOT_PERMITTED_ERROR;
		goto End;
	}

	if( DataInSize > ccmWorkingContext_ptr->RemainTextSize )
	{
		Error = CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL;
		goto End;
	}


	/* check that the the current block is not the last */
	if( DataInSize == ccmWorkingContext_ptr->RemainTextSize )
	{
		Error = CRYS_AESCCM_LAST_BLOCK_NOT_PERMITTED_ERROR;
		goto End;
	}

	/* check overlapping of input-output buffers: overlapping is not allowed, excluding
	   in placement, i.e. DataIn_ptr = DataOut_ptr */
	if( DataIn_ptr != DataOut_ptr )
	{
		/* checking the case that the input buffer is in a higher address then the output buffer */
		if ( DataIn_ptr > DataOut_ptr )
		{
			/* if after adding the size to the data out pointer it is larger then the data in pointer
			return the overlap error */
			if( DataOut_ptr + DataInSize > DataIn_ptr )
			{
				Error = CRYS_AESCCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}

		}/* end of DataIn_ptr > DataOut_ptr */

		/* checking the case that the output buffer is in a higher address then the input buffer */
		else
		{
			/* if after adding the size to the data in pointer it is larger then the data out pointer
			return the overlap error */
			if( DataIn_ptr + DataInSize > DataOut_ptr )
			{
				Error = CRYS_AESCCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}

		}/* end of DataOut_ptr > DataIn_ptr */
	}


	/* process full AES-blocks  by LLF_AESCCM_BlockTextData */
	Error = LLF_AESCCM_BlockTextData(
							ccmWorkingContext_ptr,      /* the working context - in */
							DataIn_ptr,                 /* the input data buffer - in */
							DataInSize,                 /* the data in size - in */
							DataOut_ptr );              /* the output data buffer i/o */

   /* update the context Adata remaining size  */
   ccmWorkingContext_ptr->RemainTextSize -= DataInSize;


	/* ................. release the context ................................. */
	/* ----------------------------------------------------------------------- */
End:

	ErrorCcm = CRYS_CCM_ReleaseContext( ContextID_ptr,		     /* the users context space - in */
										ccmWorkingContext_ptr,   /* the CCM context returned - in */
										DX_AESCCM_CONTEXT);      /* AESCCM type - in */

	/*if the error occurred before release context do not override it*/
	if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
	{
		Error = ErrorCcm;
	}

	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

	return Error;

#endif /* !CRYS_NO_AESCCM_SUPPORT */

}/* END OF CRYS_AESCCM_BlockTextData */


/****************************************************************************************************/
/**
* @brief This function encrypts (decrypts) the last block of text data and calculates CCM_MAC value.
*
*       This function must be called for the last block of text data and executes
*       the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*
*        2.Decrypts the received context to the working context after
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.Encrypts (decrypts)the input data by AES-CTR,  calculates and perform verification of AES-MAC value
*          by invoking LLF_AESCCM_Finish function.
*        4.Encrypts the working context and stores it to the users received context.
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        5.Exits the handler with the OK code.
*
* @param[in] ContextID_ptr - A pointer to the AESCCM context buffer, allocated by the user,
*                          that is used for the AESCCM operations. This should be the same
*                          context that was used on the previous call of this session.
* @param[in] DataIn_ptr  - A pointer to the buffer of the input data (plain or cipher text).
*                          The pointer does not need to be aligned. On CSI input mode the
*                          pointer must be equal to value (0xFFFFFFFC | DataInAlignment).
* @param[in] DataInSize  - A size of the data block in bytes. The size must be equal to remaining
*                          size value, saved in the context.
* @param[in] DataOut_ptr - A pointer to the output buffer (cipher or plain text). If
*                          user passes DataInSize 0 bytes the DataOut_ptr may be equal to NULL.
*                          The pointer does not need to be aligned. On CSI input mode the
*                          pointer must be equal to value (0xFFFFFFFC | DataOutAlignment).
* @param[in] MacRes -   A pointer to the Mac buffer.
* @param[out] SizeOfT - size of MAC in bytes as defined in CRYS_AESCCM_Init function.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_AESCCM_error.h
*   Notes:
*      1. Temporary the CSI input or output are not allowed on CCM mode.
*      2. Overlapping of the in-out buffers is not allowed, excluding the in placement case:
*         DataIn_ptr = DataOut_ptr.
*
*
*/
CEXPORT_C CRYSError_t  CRYS_AESCCM_Finish(CRYS_AESCCM_UserContext_t    *ContextID_ptr,
	                                      DxUint8_t                    *DataIn_ptr,
	                                      DxUint32_t                    DataInSize,
	                                      DxUint8_t                    *DataOut_ptr,
	                                      CRYS_AESCCM_Mac_Res_t         MacRes,
	                                      DxUint8_t                    *SizeOfT )  /*size of CCM-MAC (T) */
{
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;
	CRYSError_t ErrorCcm;

	/* defining a pointers to the AESCCM and AES contexts */
	AESCCM_Context_t *ccmWorkingContext_ptr;
	AESContext_t     *AESContext_ptr;


	/******************* FUNCTION LOGIC *************************************/

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;


	/* ............... if not supported exit .............................. */
	/* -------------------------------------------------------------------- */

	RETURN_IF_AESCCM_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize ,
								DataOut_ptr , ccmWorkingContext_ptr ,
								AESContext_ptr , MacRes , SizeOfT , Error , Error , Error ,
								Error , Error , Error , Error , Error , Error , Error ,
								Error , Error , Error , Error , Error, Error, Error);

#if !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

	/* ............... checking the parameters validity ................... */
	/* -------------------------------------------------------------------- */

	/* if the users context ID pointer is DX_NULL return an error */
	if( ContextID_ptr == DX_NULL )
		return CRYS_AESCCM_INVALID_USER_CONTEXT_POINTER_ERROR;

	if( SizeOfT == DX_NULL )
		return CRYS_AESCCM_ILLEGAL_PARAMETER_PTR_ERROR;

	/* if the users context TAG is illegal return an error - the context is invalid */
	if( ContextID_ptr->valid_tag != AESCCM_CONTEXT_VALIDATION_TAG )
		return CRYS_AESCCM_USER_CONTEXT_CORRUPTED_ERROR;

	/* if the users Data In pointer is illegal return an error */
	if( DataIn_ptr == DX_NULL && DataInSize != 0)
		return CRYS_AESCCM_DATA_IN_POINTER_INVALID_ERROR;

	/* if the users Data In pointer is illegal return an error */
	if( MacRes == DX_NULL )
		return CRYS_AESCCM_ILLEGAL_PARAMETER_PTR_ERROR;

	if( DataIn_ptr != DX_NULL && DataOut_ptr == DX_NULL)
		return CRYS_AESCCM_DATA_OUT_POINTER_INVALID_ERROR;

	if( ( DxUint32_t)DataIn_ptr >= DX_CSI )
		return CRYS_AESCCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR;



	/* ................. acquiring the AESCCM contest ........................ */
	/* ----------------------------------------------------------------------- */

	Error = CRYS_CCM_GetContext( ContextID_ptr,                   /* the users context space - in */
								(void **) &ccmWorkingContext_ptr, /* the CCM context returned - out */
								 DX_AESCCM_CONTEXT,               /* AES type - in */
								 AES_DECRYPT_CONTEXT);			  /* need to decrypt context in AES_block*/
	if( Error != CRYS_OK )
		return Error;


	/* set pointer to AESContext inside the AESCCM context */
	AESContext_ptr = (AESContext_t*)ccmWorkingContext_ptr;



	/* ......   checking the data and data in/out overlapping     ............ */
	/* ----------------------------------------------------------------------- */

	/* check that  remaining AData = 0 */
	if( ccmWorkingContext_ptr->RemainAdataSize != 0 )
	{
		Error = CRYS_AESCCM_NOT_ALL_ADATA_WAS_PROCESSED_ERROR;
		goto End;
	}

	/* check that the current block is the last: i.e. DataInSize is equal to
	   remaining TextData size */
	if( DataInSize != ccmWorkingContext_ptr->RemainTextSize )
	{
		Error = CRYS_AESCCM_DATA_IN_SIZE_ILLEGAL;
		goto End;
	}

	/* check overlapping of input-output buffers: overlapping is not allowed, excluding
	in placement, i.e. DataIn_ptr = DataOut_ptr */
	if( DataIn_ptr != DataOut_ptr )
	{
		/* checking the case that the input buffer is in a higher address then the output buffer */
		if ( DataIn_ptr > DataOut_ptr )
		{
			/* if after adding the size to the data out pointer it is larger then the data in pointer
			return the overlap error */
			if( DataOut_ptr + DataInSize > DataIn_ptr )
			{
				Error = CRYS_AESCCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}

		}/* end of DataIn_ptr > DataOut_ptr */

		/* checking the case that the output buffer is in a higher address then the input buffer */
		else
		{
			/* if after adding the size to the data in pointer it is larger then the data out pointer
			return the overlap error */
			if( DataIn_ptr + DataInSize > DataOut_ptr )
			{
				Error = CRYS_AESCCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}

		}/* end of DataOut_ptr > DataIn_ptr */
	}

    /* set indication of lastTextData */
    ccmWorkingContext_ptr->CurrentDataType = CRYS_AESCCM_LastTextData;

	/* process text data by LLF_AESCCM_BlockTextData, excluding encrypt-decrypt of
	   CCM-MAC value */
	if( DataInSize > 0 )
	{
		Error = LLF_AESCCM_BlockLastTextData(
								ccmWorkingContext_ptr,   /* the working context - in */
								DataIn_ptr,              /* the input data buffer - in */
								DataInSize,           /* the data in size - in */
								DataOut_ptr,             /* the output data buffer i/o */
								DataInSize );         /* the output buffer size - in */
		if( Error != CRYS_OK )
			goto End;
	}


	/* .................. encrypt (decrypt) the CCM-MAC value .................. */
	/* ------------------------------------------------------------------------- */

	/* set operation mode and AESContextIV in the AES context according to
	AES-CTR operation on Text data */
	AESContext_ptr->OperationMode = CRYS_AES_CTR_mode;

	/* set CTR value into AESContextIV */
	DX_VOS_FastMemCpy( (DxUint8_t *)AESContext_ptr->AESContextIV, (DxUint8_t*)ccmWorkingContext_ptr->CCM_CTR,
		               CRYS_AES_BLOCK_SIZE_IN_BYTES );

	/* set CTR value = CTR0 for encrypt(decrypt) CCM-MAC */
	DX_VOS_MemSetZero( (DxUint8_t*)AESContext_ptr->AESContextIV + CRYS_AES_BLOCK_SIZE_IN_BYTES - ccmWorkingContext_ptr->QFieldSize,
		               ccmWorkingContext_ptr->QFieldSize );

	/* on CCM encrypt mode*/
	if( ccmWorkingContext_ptr->CCM_EncryptMode == CRYS_AES_Encrypt )
	{
		Error = LLF_AES_Block(  AESContext_ptr,
								(DxUint8_t *)ccmWorkingContext_ptr->CCM_IV,  /* the input data buffer - in */
								CRYS_AES_BLOCK_SIZE_IN_BYTES,                /* the data in size - in */
								(DxUint8_t*)ccmWorkingContext_ptr->temp );    /* the output data buffer i/o */

		if( Error != CRYS_OK )
			goto End;

		/* copy encrypted CCM_MAC value into output buffer */
		DX_VOS_MemSetZero(MacRes,CRYS_AES_BLOCK_SIZE_IN_BYTES);
		DX_VOS_FastMemCpy( MacRes, ccmWorkingContext_ptr->temp,
						   ccmWorkingContext_ptr->SizeOfT );
	}

	else /* on CCM decrypt mode*/
	{
		/* copy decrypted CCM-MAC from input into last block buffer */
		DX_VOS_MemSetZero( ccmWorkingContext_ptr->lastBlock, CRYS_AES_BLOCK_SIZE_IN_BYTES );
		DX_VOS_FastMemCpy( ccmWorkingContext_ptr->lastBlock, MacRes,
			               ccmWorkingContext_ptr->SizeOfT );

		/* decrypt the MAC value and save it in the ccmWorkingContext_ptr->temp buffer */
		Error = LLF_AES_Block(  AESContext_ptr,
								(DxUint8_t *)ccmWorkingContext_ptr->lastBlock,      /* the input data buffer - in */
								CRYS_AES_BLOCK_SIZE_IN_BYTES,                /* the data in size - in */
								(DxUint8_t*)ccmWorkingContext_ptr->temp );    /* the output data buffer i/o */

		if( Error != CRYS_OK )
			goto End;

		/* compare calculated and decrypted MAC results */
		Error = DX_VOS_MemCmp( (DxUint8_t *)ccmWorkingContext_ptr->CCM_IV,
			(DxUint8_t*)ccmWorkingContext_ptr->temp,
			ccmWorkingContext_ptr-> SizeOfT );

		/* if MAC results are different, return an Error */
		if( Error != 0 )
		{
			Error = CRYS_AESCCM_CCM_MAC_INVALID_ERROR;
			goto End;
		}
	}



   *SizeOfT = ccmWorkingContext_ptr-> SizeOfT;


	/* ................. release the context ................................. */
	/* ----------------------------------------------------------------------- */

End:

	ErrorCcm = CRYS_CCM_ReleaseContext( ContextID_ptr,		     /* the users context space - in */
										ccmWorkingContext_ptr,   /* the CCM context returned - in */
										DX_AESCCM_CONTEXT);      /* AES type - in */

	/*if the error occurred before release context do not override it*/
	if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
	{
		Error = ErrorCcm;
	}

	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */
	/*  set 0 to the context buffer */
    if((ErrorCcm == CRYS_OK) && (Error == CRYS_OK))
	   DX_VOS_MemSetZero( ContextID_ptr , sizeof(CRYS_AESCCM_UserContext_t) );

	return Error;

#endif /* !CRYS_NO_AESCCM_SUPPORT */


}/* END OF CRYS_AESCCM_BlockLastTextData */



/****************************************************************************************************/



/****************************************************************************************************/
/**
 * @brief This function is used to perform the AES_CCM operation in one integrated process.
 *
 *        The function preforms CCM algorithm according to NIST 800-38C by call the CRYS_CCM
 *        Init, Block and Finish functions.
 *
 *        The input-output parameters of the function are the following:
 *
 * @param[in] EncrDecrMode  - Enumerator variable defining operation mode (0 - encrypt; 1 - decrypt).
 * @param[in] CCM_Key       - A buffer, containing the AESCCM key passed by user (predefined size 128 bits).
 * @param[in] KeySizeId     - An ID of AESCCM key size (according to 128, 192, or 256 bits size).
 * @param[in] isSecretKey   - This parameter intended for internal Discretix using only !.
 *                            The parameter specifying whether to use a secret key (=TRUE)
 *                            or to use a key defined by the user (=FALSE).
 *                            The secret key function depends on product configuration.
 *                            Please consult the reference manual for AES functions.
* @param[in] QFieldSize    - Byte-size of formatted field for writing significant bytes of the TextSizeQ
 *                            value. Valid values: [2,3,4,5,6,7,8].
 * @param[in] N_ptr	        - A pointer to Nonce - unique value assigned to all data passed into CCM.
 *                            Bytes order - big endian form (MSB is the first).
 * @param[in] SizeOfN       - The size of the user passed Nonce (in bytes).	Valid values:
 *                            7 <= SizeOfN <= (15-QFieldSize).
 * @param[in] ADataIn_ptr    - A pointer to the additional data buffer. The pointer does
 *                             not need to be aligned. On CSI input mode the pointer must be equal to
 *                             value (0xFFFFFFFC | DataInAlignment).
 * @param[in] ADataInSize    - The size of the additional data in bytes;
 * @param[in] TextDataIn_ptr - A pointer to the input text data buffer (plain or cipher according to
 *                             encrypt-decrypt mode). The pointer does not need to be aligned.
 *                             On CSI input mode the pointer must be equal to value (0xFFFFFFFC | DataInAlignment).
 * @param[in] TextDataInSize - The size of the input text data in bytes:
 *                               - on encrypt mode: (2^32 - SizeOfT) > DataInSize >= 0;
 *                               - on Decrypt mode: 2^32 > DataInSize >= SizeOfT (SizeOfT from context).
 * @param[out] TextDataOut_ptr - The output text data pointer (cipher or plain text data).
 *
 * @param[in] SizeOfT	    - Size of AES-CCM MAC output T in bytes. Valid values: [4,6,8,10,12,14,16].
*
 * @param[in/out] Mac_Res	    -  AES-CCM MAC input/output .
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in CRYS_AESCCM_error.h
 *
 *     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously
 *               were on CSI mode.
 *
 */

CEXPORT_C CRYSError_t  CRYS_AESCCM(
								   CRYS_AES_EncryptMode_t       EncrDecrMode,   /*CCM encrypt-decrypt mode*/
								   CRYS_AESCCM_Key_t            CCM_Key,        /*AES-CCM key*/
								   CRYS_AESCCM_KeySize_t        KeySizeId,      /*Key size ID*/
								   DxUint8_t                    isSecretKey,    /*secret key flag*/
								   DxUint8_t                    QFieldSize,     /*size of Q field in control block*/
								   DxUint8_t                   *N_ptr,          /*Nonce*/
								   DxUint8_t                    SizeOfN,        /*size of N buffer*/
								   DxUint8_t                   *ADataIn_ptr,    /*input data pointer*/
								   DxUint32_t                   ADataInSize,    /*input data size*/
								   DxUint8_t                   *TextDataIn_ptr, /*input data pointer*/
								   DxUint32_t                   TextDataInSize, /*input data size*/
								   DxUint8_t                   *TextDataOut_ptr,/*output data pointer*/
								   DxUint8_t                    SizeOfT ,         /*size of CCM-MAC (T) */
								  CRYS_AESCCM_Mac_Res_t         MacRes)

{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* a users context used to pass to all of the CRYS functions */
   CRYS_AESCCM_UserContext_t  ContextID;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AESCCM_UNSUPPORTED( EncrDecrMode , CCM_Key , KeySizeId , QFieldSize , N_ptr , SizeOfN ,
                              SizeOfT , isSecretKey , ADataIn_ptr , ADataInSize , TextDataIn_ptr ,
							  TextDataInSize , TextDataOut_ptr , SizeOfT ,
							  ContextID.valid_tag , Error , Error , Error , Error ,
							  Error , Error , Error , Error , Error , Error);

   #if !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

   /* ............... calling the Init function ........................... */
   /* --------------------------------------------------------------------- */

   Error = CRYS_AESCCM_Init( &ContextID,
	                          EncrDecrMode,
							  CCM_Key,       /*AES-CCM key*/
							  KeySizeId,     /*AESCCM key size ID*/
							  isSecretKey,   /*secret key flag*/
							  ADataInSize,   /*full size of additional data*/
							  TextDataInSize, /*full size of text data*/
							  QFieldSize,    /*size of Q field in control block*/
							  N_ptr,         /*Nonce*/
							  SizeOfN,       /*size of Nonce*/
							  SizeOfT );     /*size of CCM MAC  (SizeOfT) */

   if( Error != CRYS_OK )
     goto End;


   /* ............... calling the BlockAdata function >.................... */
   /* --------------------------------------------------------------------- */
   if( ADataInSize > 0 )
   {
	   Error = CRYS_AESCCM_BlockAdata( &ContextID,
									   ADataIn_ptr,
									   ADataInSize );

	   if( Error != CRYS_OK )
		   goto End;
   }


   /* ............... calling the Finish function ......................... */
   /* --------------------------------------------------------------------- */


   Error = CRYS_AESCCM_Finish(&ContextID,
                               TextDataIn_ptr,
	                           TextDataInSize,
							   TextDataOut_ptr,
							   MacRes,
							   &SizeOfT);
   if( Error != CRYS_OK )
	   goto End;




  /* ................   End of function        ............................ */
   /* --------------------------------------------------------------------- */
 End:

   return Error;

   #endif /* !(defined CRYS_NO_AESCCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)  */

}/* END OF CRYS_AESCCM function */
