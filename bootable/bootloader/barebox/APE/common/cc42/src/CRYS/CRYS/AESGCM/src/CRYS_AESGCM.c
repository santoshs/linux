
  /*
   *  Object %name    : %CRYS_AESGCM.c
   *  State           :  %state%
   *  Creation date   :  21 March 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_AESGCM.c#1:csrc:6
   *  \author R.Levin
   *  \remarks Copyright (C) 2011 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

/* .............. CRYS level includes ................. */

#include "DX_VOS_Mem.h"
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_CCM.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_AESGCM_Local.h"
#include "CRYS_AESGCM_error.h"
#include "CRYS_AESGCM.h"


/* .............. LLF level includes .................. */

#if !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)
#include "LLF_AES.h"
#include "LLF_AESGCM.h"
#endif /* CRYS_NO_AESGCM_SUPPORT */

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
* @brief The CRYS_AESGCM_Init function initializes the AESGCM machine and SW structures
*        for GHASH and GCTR (cipher) operations.
*
*        To perform the AESGCM operations this function should be called the first.
*        The function executes the following steps (algorithm according to NIST 800-38D):
*
*        1. Validates all of the inputs of the function. If one of the received
*           parameters is not valid it shall return an error,
*        2. Allocates the working context by calling the CRYS_CCM_GetContext() function and
*        3. Calculates GHASH subkey H by AES encryption of 0^128  bit string and saves it in the
*           the Context H – buffer.
*        4. Calculates GCTR initial counter value J0 from input IV and saves it in the
*           CTR buffer of the Context.
*        5. Initializes the context, sets not initialized fields to 0.
*        6. Encrypts (if requirred) the working context and stores it to the users context.
*           Releases the working context by calling the CRYS_CCM_EncryptAndReleaseContext function.
*        7. Exits the handler with the OK code.
*
*        NOTE: 1.The GCM mode requieres uniqueness of Keys and IVs according to standard NIST 800-38D
*                (including parts 8,9 and app. A,B).
*              2.Using the Tags with short sizes (32,64 bits)requires additional limitations on
*                maximal Combined lengts of Ciphertext and AddData and on maximum Invocations of
*                the Authenticated Decryption function (NIST 800-38D app.C).
*              3.Management of both above stated issues is out of scope of the CRYS_GCM module and
*                is on responsibility of the Application, which uses this module.
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer, that is allocated by the user
*                            and is used for the AESGCM operations.
* @param[in] encrMode      - Enumerator variable, defining operation mode (0 - encrypt; 1 - decrypt).
* @param[in] key           - A pointer, to the AESGCM key passed by user.
* @param[in] keySizeId     - An ID of AESGCM key size (according to 128, 192, or 256 bits size).
* @param[in] IV_ptr	       - A pointer to Initial Value (nonce) - unique value assigned to all data passed
*                            into GCM.
* @param[in] ivSize        - The size of the user passed IV (in bytes). Our implementation now allows only the
*                            value, recommended by standard - 12 bytes.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to CRYS_AESGCM_error.h
*/
CEXPORT_C CRYSError_t CRYS_AESGCM_Init(
									   CRYS_AESGCM_UserContext_t *ContextID_ptr, /*GCM user context*/
									   CRYS_AESGCM_EncryptMode_t  encrMode,      /*encrypt-decrypt mode*/
									   CRYS_AESGCM_Key_t          key,           /*AESGCM key*/
									   CRYS_AESGCM_KeySize_t      keySizeID,     /*key size ID*/
									   DxUint8_t                 *iv_ptr,        /*initial value*/
									   DxUint32_t                 ivSize )       /*size of IV in bytes*/
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifiers */
   CRYSError_t Error, ErrorCcm;

   /* defining a pointer to the active context allocated by the CCM */
   AESGCMContext_t  *ccmWorkingContext_ptr;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AESGCM_UNSUPPORTED( ContextID_ptr , encrMode , key[0] , keySizeID ,
	                             iv_ptr , ivSize , ErrorCcm , ccmWorkingContext_ptr ,
								 iv_ptr , ivSize , Error , Error , Error ,
                                 Error , Error , Error , Error , Error , Error,
								 Error , Error , Error , Error , Error , Error);

#if !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_AESGCM_INVALID_USER_CONTEXT_POINTER_ERROR;

   /*Init the valid_tag filed to avoid warnings*/
   ContextID_ptr->valid_tag = 0;

   /* Check the Encrypt / Decrypt mode */
   if( encrMode >= CRYS_AES_EncryptNumOfOptions )
	   return  CRYS_AESGCM_INVALID_ENCRYPT_MODE_ERROR;

   /* check Key */
   if( key == DX_NULL )
      return  CRYS_AESGCM_INVALID_KEY_POINTER_ERROR;

   if( keySizeID >= CRYS_AESGCM_KeySizeNumOfOptions )
	   return  CRYS_AESGCM_ILLEGAL_KEY_SIZE_ERROR;

   /* check IV */
   if( iv_ptr == DX_NULL )
	   return  CRYS_AESGCM_ILLEGAL_IV_PTR_ERROR;

   if( ivSize != CRYS_AESGCM_IV_RECOMMENDED_SIZE_BYTES )
	   return  CRYS_AESGCM_ILLEGAL_IV_SIZE_ERROR;


   /* ................. acquiring the AES contest ........................... */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,                      /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,   /* the CCM context returned - out */
                                DX_AESGCM_CONTEXT,                  /* AESGCM type - in */
                                AES_DONT_DECRYPT_CONTEXT);		    /*No need to decrypt context in Init*/
   if( Error != CRYS_OK )

      return Error;

   /* clean working context */
   DX_VOS_MemSetZero( ccmWorkingContext_ptr, sizeof(AESGCMContext_t) );

   /*      initialize the AESGCM  context       */
   /*-------------------------------------------*/

   ccmWorkingContext_ptr->EncryptMode = encrMode;

   ContextID_ptr->valid_tag = AESGCM_CONTEXT_VALIDATION_TAG;

   /* ----------------------------------------------- */
   /*   call LLF GCM Init function                    */
   /* ----------------------------------------------- */
   Error = LLF_AESGCM_Init( ccmWorkingContext_ptr, key, keySizeID,
	                        iv_ptr, ivSize );


   /*-------------------------------------------------------------------------*/
   /* ................. release the context ................................. */
   /*-------------------------------------------------------------------------*/

    ErrorCcm = CRYS_CCM_ReleaseContext( ContextID_ptr,          /* the users context space - in */
                                        ccmWorkingContext_ptr,  /* the CCM context returned - in */
                                        DX_AESGCM_CONTEXT);     /* AESGCM type - in */


   /* ...................... end of function ................................ */

   /*if the error occurred before release context do not override it*/
   if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
		Error = ErrorCcm;

   if( Error != CRYS_OK )
       /* clearing the users context */
	   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_AESGCM_UserContext_t) );

   return Error;

#endif /* !CRYS_NO_AESGCM_SUPPORT */

}/* END OF CRYS_AESGCM_Init */



/****************************************************************************************************/
/**
* @brief This function calculates (updates) the AESGCM MAC on current block of additional data.
*
*       This function is called for each block of additional data, if it present, and
*       executes the following major steps (algorithm according to NIST 800-38D):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.
*          In partial, checks that previous processed block of data not was not multiple of
*          16 bytes, otherwise returns an error.
*          If DataIn_ptr = NULL or DataInSize = 0, the function returns an error.
*        2.Decrypts the received context to the working context after capturing the
*          working context by calling the CRYS_CCM_GetContext function.
*        3.Checks that the data size not exceeds the maximal allowed value and updates the
*          total size (AddDataSize field) according to received input size.
*        4.Calculates GHASH value for all full 16-bytes blocks of the additional data.
*        5.Pads the last not full AES-block by 0-s, calculates GHASH and saves it
*          in the context buffer.
*        6.Encrypts the working context and stores it to the users received context.
*          Releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        7.Exits the handler with the OK code.
*
*      Note: The padding and GHASH calculation are performed on LLF level by calling
*            appropriate LLF functions.
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer allocated by the user that
*                            is used for the AESGCM machine operation. This should be the same
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input additional data. It is required:
*                         DataIn_ptr != NULL. The pointer does not need to be aligned.
*                         On CSI input mode the pointer must be equal to value
*                         (0xFFFFFFFC | DataInAlignment).
* @param[in] DataInSize   - A size of the additional data in bytes. Required: DataInSize > 0.
*                           Size of each intermediate block of data must be multiple of 16 bytes,
*                           excluding the last block which may have any size. The total size
*                           of the Additional data in all callings of the function must be less,
*                           than 2^64-1 bytes.
* @return CRYSError_t - On success CRYS_OK is returned, on failure an error according to
*                       CRYS_AESGCM_error.h
*   Notes:
*      1. Temporary the CSI input is not allowed on AESGCM mode.
*
*/
CEXPORT_C CRYSError_t  CRYS_AESGCM_BlockAdata(
											CRYS_AESGCM_UserContext_t    *ContextID_ptr,
											DxUint8_t                    *DataIn_ptr,
											DxUint32_t                    DataInSize )
 {
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;
   CRYSError_t ErrorCcm;

   /*  pointer to the AESGCM context */
   AESGCMContext_t *ccmWorkingContext_ptr;

   DxUint64_t totalSize;


   /******************* FUNCTION LOGIC *************************************/

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AESGCM_UNSUPPORTED( ContextID_ptr, DataIn_ptr, DataInSize,
                              ErrorCcm, ccmWorkingContext_ptr, (DxUint32_t)totalSize,
							  Error, Error, Error, Error, Error, Error,
							  Error, Error, Error, Error, Error, Error, Error,
							  Error, Error, Error, Error, Error, Error );

#if !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_AESGCM_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != AESGCM_CONTEXT_VALIDATION_TAG )
      return CRYS_AESGCM_USER_CONTEXT_CORRUPTED_ERROR;

   /* if the users dataIn pointer is illegal return an error */
   if( DataIn_ptr == DX_NULL )
      return CRYS_AESGCM_DATA_IN_POINTER_INVALID_ERROR;

   if( (DxUint32_t)DataIn_ptr >= DX_CSI )
	   return CRYS_AESGCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR;

   /* if the data size is illegal return an error */
   if( DataInSize == 0 )
	   return CRYS_AESGCM_DATA_IN_SIZE_ILLEGAL;


   /* ................. acquiring the AESGCM contest ........................ */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_AESGCM_CONTEXT,                   /* AESGCM type - in */
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/
   if( Error != CRYS_OK )
     return Error;

   /* if previous processed additional data, was not multiple of 128 bits,
      then return the error */
   if( ccmWorkingContext_ptr->DataSizes[0] & (DxUint64_t)127 )
   {
	   Error = CRYS_AESGCM_ADDITIONAL_BLOCK_NOT_PERMITTED_ERROR;
	   goto End;
   }

   /* check the total AddData length: len(AddData) < 2^64 -1 ) */
   totalSize = DataInSize;
   totalSize <<= 3;
   totalSize += ccmWorkingContext_ptr->DataSizes[0];
   if( totalSize < ccmWorkingContext_ptr->DataSizes[0] )
   {
	   Error = CRYS_AESGCM_TOTAL_DATA_SIZE_EXCEED_ERROR;
	   goto End;
   }

   /* ----------------------------------------------------------------- */
   /* .......  processing the Adata by LLF_GHASH function ............. */
   /* ----------------------------------------------------------------- */

   Error = LLF_AESGCM_GHASH(
							 ccmWorkingContext_ptr,  /* the working context - in */
							 DataIn_ptr,             /* the input data buffer - in */
							 DataInSize );           /* the output data buffer i/o */
   if ( Error != CRYS_OK )
	   goto End;

   /* update the total length */
   ccmWorkingContext_ptr->DataSizes[0] = totalSize;

   /* ----------------------------------------------------------------------- */
   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

 End:

   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,		   /* the users context space - in */
                                      ccmWorkingContext_ptr,   /* the CCM context returned - in */
                                      DX_AESGCM_CONTEXT);      /* AESGCM type - in */

   /*if the error occurred before release context do not override it*/
   if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
   {
		Error = ErrorCcm;
   }

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   return Error;

#endif /* !CRYS_NO_AESGCM_SUPPORT */


}/* END OF CRYS_AESGCM_BlockAdata */


/****************************************************************************************************/
/**
* @brief This function performs authenticated encryption/decryption  of current block of text data.
*
*       This function is called for each block of text data with size multiple 16 bytes,
*       excluding the last block, which may have any size > 0.
*
*       The function executes the following major steps (algorithm according to NIST 800-38C):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error,
*          in partial, checks that previously processed additional and text data not have
*          remaining bytes (not multiple 16). Checks the size of the user passed output buffer,
*          which must be not less, than input data size.
*        2.Decrypts the received context to the working context after
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.Calculates the total bit length of the text data to process (including the previous
*          processed data), checks that the length is less than (2^39-256) bits and saves it in
*          the context.
*      On encrypt mode
*          -	if the data size is not multiple of 16 bytes, then the function pads the last
*               data by 0-s to full 16-bytes block;
*          -	encrypts the padded plain text to cypher text and calculates the GHASH value for
*               the cypher, outputs the cypher text and saves the GHASH in the context IV buffer.
*      On decrypt mode
*          -	calculates GHASH of cipher text (with padding by 0-s, if necessary) and then decrypts
*               it (these operations are similar to described for encryption).
*        4.Encrypts the working context and stores it to the users received context.
*          Releases the working context by calling the CRYS_CCM_ReleaseContext function.
*        5.Exits.
*
*      Note: The padding, cipher operations and GHASH calculation are performed on LLF level
*            by calling appropriate LLF functions.
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer allocated by the user that
*                            is used for the AESGCM operations. This should be the same
*                            context that was used on the previous call of this session.
* @param[in] DataIn_ptr - A pointer to the buffer of the input data (plain or cypher text).
*                         The pointer does not need to be aligned. On CSI input mode the
*                         pointer must be equal to value (0xFFFFFFFC | DataInAlignment).
* @param[in] DataInSize  - A size of the data block in bytes: must be not null and multiple of
*                          16 bytes. The total size of processed data must be less than
*                          2^36-32 bytes.
* @param[in] DataOut_ptr - A pointer to the output buffer (cypher or plain text according to
*                          encryption mode ).The pointer does not need to be aligned. On CSI input
*                          mode the pointer must be equal to value (0xFFFFFFFC | DataOutAlignment).
*                          Size of the output buffer must be not less, than DataInSize.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_AESGCM_error.h
*   Notes:
*      1. Temporary the CSI input or output are not allowed on AESGCM mode.
*      2. Overlapping of the in-out buffers is not allowed, excluding the in placement case:
*         DataIn_ptr = DataOut_ptr.
*
*/
CEXPORT_C CRYSError_t  CRYS_AESGCM_BlockTextData(
								CRYS_AESGCM_UserContext_t    *ContextID_ptr,
								DxUint8_t                    *DataIn_ptr,
								DxUint32_t                    DataInSize,
								DxUint8_t                    *DataOut_ptr )
{
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;
	CRYSError_t ErrorCcm;

	/*  pointer to the AESGCM context */
	AESGCMContext_t *ccmWorkingContext_ptr;

	DxUint64_t totalSize;


	/******************* FUNCTION LOGIC *************************************/

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* ............... if not supported exit .............................. */
	/* -------------------------------------------------------------------- */

	RETURN_IF_AESGCM_UNSUPPORTED( ContextID_ptr, DataIn_ptr, DataInSize,
								ErrorCcm, ccmWorkingContext_ptr, (DxUint32_t)totalSize,
								Error, Error, Error, Error, Error, Error,
								Error, Error, Error, Error, Error, Error, Error,
								Error, Error, Error, Error, Error, Error );

#if !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

	/* ............... checking the parameters validity ................... */
	/* -------------------------------------------------------------------- */

	/* if the users context ID pointer is DX_NULL return an error */
	if( ContextID_ptr == DX_NULL )
		return CRYS_AESGCM_INVALID_USER_CONTEXT_POINTER_ERROR;

	/* if the users context TAG is illegal return an error - the context is invalid */
	if( ContextID_ptr->valid_tag != AESGCM_CONTEXT_VALIDATION_TAG )
		return CRYS_AESGCM_USER_CONTEXT_CORRUPTED_ERROR;

	/* if the users dataIn pointer is illegal return an error */
	if( DataIn_ptr == DX_NULL )
		return CRYS_AESGCM_DATA_IN_POINTER_INVALID_ERROR;

	if( (DxUint32_t)DataIn_ptr >= DX_CSI )
		return CRYS_AESGCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR;

	/* if the data size is illegal return an error */
	if( DataInSize == 0 ||
		DataInSize % CRYS_AESGCM_BLOCK_SIZE_IN_BYTES != 0)
		return CRYS_AESGCM_DATA_IN_SIZE_ILLEGAL;

	/* ................. acquiring the AESGCM contest ........................ */
	/* ----------------------------------------------------------------------- */

	Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
								(void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
								DX_AESGCM_CONTEXT,                   /* AESGCM type - in */
								AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/
	if( Error != CRYS_OK )
		return Error;

	/* if previous processed additional data, was not multiple of 128 bits,
	   then return the error */
	if( ccmWorkingContext_ptr->DataSizes[1] & 127 )
	{
		Error = CRYS_AESGCM_ADDITIONAL_BLOCK_NOT_PERMITTED_ERROR;
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
				Error = CRYS_AESGCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
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
				Error = CRYS_AESGCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}
		}/* end of DataOut_ptr > DataIn_ptr */
	}

	/* check the total TextData length in bits: len(TextData) < 2^39 - 256 */
	totalSize = DataInSize;
	totalSize <<= 3;
	totalSize += ccmWorkingContext_ptr->DataSizes[1];
	if( (totalSize >> 7) > 0xFFFFFFFE )
	{
		Error = CRYS_AESGCM_TOTAL_DATA_SIZE_EXCEED_ERROR;
		goto End;
	}

	/* ----------------------------------------------------------------- */
	/* ...........     process the TextData by LLF     ................. */
	/* ----------------------------------------------------------------- */
	Error = LLF_AESGCM_TextData(
		                ccmWorkingContext_ptr,  /* the working context - in */
						DataIn_ptr,             /* the input data buffer - in */
						DataInSize,             /* the input data size - in */
						DataOut_ptr  );         /* the output data buffer - out */

	if ( Error != CRYS_OK )
	   goto End;

	/* update total TextData size  */
	ccmWorkingContext_ptr->DataSizes[1] = totalSize;


	/* ----------------------------------------------------------------------- */
	/* ................. release the context ................................. */
	/* ----------------------------------------------------------------------- */

End:

	ErrorCcm = CRYS_CCM_ReleaseContext( ContextID_ptr,		     /* the users context space - in */
										ccmWorkingContext_ptr,   /* the CCM context returned - in */
										DX_AESGCM_CONTEXT);      /* AESGCM type - in */

	/*if the error occurred before release context do not override it*/
	if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
	{
		Error = ErrorCcm;
	}

	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

	return Error;

#endif /* !CRYS_NO_AESGCM_SUPPORT */


}/* END OF CRYS_AESGCM_BlockTextData */



/****************************************************************************************************/
/**
* @brief This function finishes the AESGCM operation.
*
*       This function must be called after processing all additional and text data by appropriate
*       AESGCM_Block functions.
*       The function performs the following major steps (algorithm according to NIST 800-38D):
*
*        1.Checks the validity of all inputs of the function.
*          If one of the received parameters is not valid it shall return an error.*
*        2.Decrypts the received context to the working context after
*          capturing the working context by calling the CRYS_CCM_GetContext function.
*        3.Encrypts/Decrypts by AESCTR the text data with padded last block and calculates GHASH value.
*        4.Concatenates saved in the Context two 64-bit values of AddDataSize and
*          TextDataSize, converts endianness MSBytes order and calculates final GHASH value.
*        5.Encrypts the final GHASH by GCTR using the initial value of J0 and derives from it
*          the GMAC Tag of required size.
*        6.On encrypt mode: puts the Tag out.
*	 7.On decrypt mode: compares the calculated tag value to received input tag’ value.
*          If tag == tag’ and no any errors occurs, then sets returning Error value = OK, else = FAIL.
*          Note: GHASH calculation is performed on LLF level by calling the LLF_AESGCM_Finish function.
*        8.Cleans and releases the working context by calling the CRYS_CCM_ReleaseContext call.
*        9.Exits the handler with the return Error code.
*
*         NOTE:
*            1.Using the Tags with short sizes (32,64 bits)requires additional limitations on
*              maximal Combined lengts of Ciphertext and AddData and on maximum Invocations of
*              the Authenticated Decryption function (NIST 800-38D app.C).
*              Management of this issue is out of scope of the CRYS GCM module and is on
*              responsibility of the Application ,which uses this module.
*
*
* @param[in] ContextID_ptr - A pointer to the AESGCM context buffer, allocated by the user.
*                            This should be the same context that was used on the previous call
*                            of this session.
* @param[in] TextDataIn_ptr - A pointer to the buffer of the input data (plain or cypher text).
*                             The pointer does not need to be aligned. On CSI input mode the
*                             pointer must be equal to value (0xFFFFFFFC | DataInAlignment).
* @param[in] TextDataSize  - A size of the data block in bytes: must be not null. The total
*                            size of processed data must be less than 2^36-32 bytes.
* @param[in/out] tag_ptr   - A pointer to the buffer of the AESGCM Tag. The pointer does not need
*                            to be aligned.
* @param[in] tagSize       - The size of the Tag in bytes. The allowed values are: 12...16 bytes,
*                            For certain applications the size may be also 4 or 8 bytes with additional
*                            limitations on total data size (see NIST 800-38D).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value MODULE_* CRYS_AESGCM_error.h
*
*/
CEXPORT_C CRYSError_t CRYS_AESGCM_Finish(
							   CRYS_AESGCM_UserContext_t    *ContextID_ptr,
							   DxUint8_t                    *TextDataIn_ptr,
							   DxUint32_t                    TextDataSize,
							   DxUint8_t                    *TextDataOut_ptr,
							   DxUint8_t                    *tag_ptr,
							   DxUint32_t                    tagSize )
{
   /* FUNCTION DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;
	CRYSError_t ErrorCcm;

	/*  pointers to the AESGCM and AES contexts */
	AESGCMContext_t *ccmWorkingContext_ptr;

	DxUint64_t totalSize;
	CRYS_COMMON_CmpCounter_t cmp;


	/******************* FUNCTION LOGIC *************************************/

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* ............... if not supported exit .............................. */
	/* -------------------------------------------------------------------- */

	RETURN_IF_AESGCM_UNSUPPORTED( ContextID_ptr, TextDataIn_ptr, TextDataSize,
		TextDataOut_ptr, tag_ptr, tagSize, ErrorCcm, ccmWorkingContext_ptr,
		(DxUint32_t)totalSize, cmp, Error, Error, Error, Error, Error,
		Error, Error, Error, Error, Error, Error, Error, Error, Error, Error );

#if !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

	/* ............... checking the parameters validity ................... */
	/* -------------------------------------------------------------------- */

	/* check the users context ID and validation tag */
	if( ContextID_ptr == DX_NULL )
		return CRYS_AESGCM_INVALID_USER_CONTEXT_POINTER_ERROR;

	if( ContextID_ptr->valid_tag != AESGCM_CONTEXT_VALIDATION_TAG )
		return CRYS_AESGCM_USER_CONTEXT_CORRUPTED_ERROR;

	/* if the users dataIn pointer is illegal return an error */
	if( TextDataIn_ptr == DX_NULL && TextDataSize != 0 )
		return CRYS_AESGCM_DATA_IN_POINTER_INVALID_ERROR;

	if( (DxUint32_t)TextDataIn_ptr >= DX_CSI )
		return CRYS_AESGCM_CSI_IN_OUT_ILLEGAL_MODE_ERROR;

	/* check the Tag pointer and size: allowed 12...16 and 4,8 bytes */
	if( tag_ptr == DX_NULL )
		return CRYS_AESGCM_TAG_PTR_INVALID_ERROR;

	if( (tagSize < CRYS_AESGCM_TAG_MIN_SIZE_BYTES || tagSize > CRYS_AESGCM_TAG_MAX_SIZE_BYTES) &&
		tagSize != 4 && tagSize != 8 )
		return CRYS_AESGCM_TAG_SIZE_INVALID_ERROR;


	/* ................. acquiring the AESGCM contest ........................ */
	/* ----------------------------------------------------------------------- */

	Error = CRYS_CCM_GetContext(ContextID_ptr,					 /* the users context space - in */
								(void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
								DX_AESGCM_CONTEXT,                   /* AESGCM type - in */
								AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/
	if( Error != CRYS_OK )
		return Error;

	/* check overlapping of input-output buffers: overlapping is not allowed, excluding
	   in placement, i.e. DataIn_ptr = DataOut_ptr */
	if( TextDataIn_ptr != TextDataOut_ptr )
	{
		/* checking the case that the input buffer is in a higher address then the output buffer */
		if ( TextDataIn_ptr > TextDataOut_ptr )
		{
			/* if after adding the size to the data out pointer it is larger then the data in pointer
			return the overlap error */
			if( TextDataOut_ptr + TextDataSize > TextDataIn_ptr )
			{
				Error = CRYS_AESGCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}
		}/* end of TextDataIn_ptr > TextDataOut_ptr */

		/* checking the case that the output buffer is in a higher address then the input buffer */
		else
		{
			/* if after adding the size to the data in pointer it is larger then the data out pointer
			return the overlap error */
			if( TextDataIn_ptr + TextDataSize > TextDataOut_ptr )
			{
				Error = CRYS_AESGCM_DATA_OUT_DATA_IN_OVERLAP_ERROR;
				goto End;
			}
		}/* end of TextDataOut_ptr > TextDataIn_ptr */
	}

	/* check the total TextData length in bits: len(TextData) < 2^39 - 256 */
	totalSize = TextDataSize;
	totalSize <<= 3;
	ccmWorkingContext_ptr->DataSizes[1] += totalSize;
	if( (ccmWorkingContext_ptr->DataSizes[1] >> 7) > 0xFFFFFFFE )
	{
		Error = CRYS_AESGCM_TOTAL_DATA_SIZE_EXCEED_ERROR;
		goto End;
	}

	/* ----------------------------------------------------------------- */
	/* ...........     process the TextData by LLF     ................. */
	/* ----------------------------------------------------------------- */
	Error = LLF_AESGCM_Finish(
							ccmWorkingContext_ptr,
							TextDataIn_ptr,
							TextDataSize,
							TextDataOut_ptr );

	if ( Error != CRYS_OK )
		goto End;


	/* if encrypt mode, then output the GHASH Tag, else compare calculated
	   Tag with one passed by the user and output the Error */
	if(ccmWorkingContext_ptr->EncryptMode == CRYS_AESGCM_Encrypt)
	{
	    DX_VOS_FastMemCpy(tag_ptr, ccmWorkingContext_ptr->Mac, tagSize);
	}
	else
	{
		cmp =CRYS_COMMON_CmpLsbUnsignedCounters( (DxUint8_t*)&ccmWorkingContext_ptr->Mac[0], (DxUint16_t)tagSize,
			                                     tag_ptr, (DxUint16_t)tagSize );

		if(cmp != 0)
			Error = CRYS_AESGCM_AUTENTICATION_FAIL_ERROR;
	}


	/* ................. release the context ........................... */
	/* ----------------------------------------------------------------- */
End:

	ErrorCcm = CRYS_CCM_ReleaseContext( ContextID_ptr,		     /* the users context space - in */
										ccmWorkingContext_ptr,   /* the CCM context returned - in */
										DX_AESGCM_CONTEXT );     /* AESGCM type - in */

	/*if the error occurred before release context do not override it*/
	if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
	{
		Error = ErrorCcm;
	}

	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

	return Error;

#endif /* !CRYS_NO_AESGCM_SUPPORT */


}/* END OF CRYS_AESGCM_Finish function */


/****************************************************************************************************/
/**
* @brief This function is used to perform the AESGCM operation in one integrated process.
*
*        The function preforms CCM algorithm according to NIST 800-38D by call the AESGCM
*        Init, Block and Finish functions.
*
*    NOTE:
*         1.The GCM mode strongly requieres uniqueness of Keys and IVs according to standard
*           NIST 800-38D (including parts 8,9 and app. A,B).
*         2.Using the Tags with short sizes (32,64 bits) requires additional limitations on
*           maximal Combined lengts of Ciphertext and AddData and on maximum Invocations of
*           the Authenticated Decryption function (NIST 800-38D including app.C).
*         3.Management of above stated issues is out of scope of the CRYS_GCM module and
*           is on responsibility of the Application, which uses this module.

*
* @param[in] encrMode      - Enumerator variable, defining operation mode (0 - encrypt; 1 - decrypt).
* @param[in] key           - A pointer, to the AESGCM key passed by user.
* @param[in] keySizeId     - An ID of AESGCM key size (according to 128, 192, or 256 bits size).
* @param[in] IV_ptr	       - A pointer to Initial Value (nonce) - unique value assigned to all data passed
*                            into GCM.
* @param[in] ivSize        - The size of the user passed IV (in bytes). Our implementation now allows only the
*                            value, recommended by standard - 12 bytes.
* @param[in] AddDataIn_ptr  - A pointer to the additional data buffer. The pointer does
*                             not need to be aligned. On CSI input mode the pointer must be equal to
*                             value (0xFFFFFFFC | DataInAlignment).
* @param[in] AddDataInSize  - The size of the additional data in bytes;
* @param[in] TextDataIn_ptr - A pointer to the input text data buffer (plain or cypher according to
*                             encrypt-decrypt mode). The pointer does not need to be aligned.
*                             On CSI input mode the pointer must be equal to value (0xFFFFFFFC | DataInAlignment).
* @param[in] TextDataInSize - A size of the data block in bytes: must be not null.
* @param[out] TextDataOut_ptr - The output text data pointer (cypher or plain text according to encrypt/decrypt mode).
* @param[in/out] tag_ptr    - A pointer to the buffer of the AESGCM tag. The pointer does not need
*                             to be aligned.
* @param[in] tagSize        - The size of the tag in bytes. The allowed values are: 12...16 bytes. For certain
*                             applications the size may be 4 or 8 bytes (see NIST 800-38D).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in CRYS_AESGCM_error.h
*
*     NOTES:
*              1. Temporarily it is not allowed, that both the Input and the Output data simultaneously
*               were on CSI mode.
*
*/
CIMPORT_C   CRYSError_t CRYS_AESGCM (
									 CRYS_AESGCM_EncryptMode_t  encrMode,      /*encrypt-decrypt mode*/
									 CRYS_AESGCM_Key_t          key,           /*AESGCM key*/
									 CRYS_AESGCM_KeySize_t      keySizeId,     /*key size ID*/
									 DxUint8_t                 *iv_ptr,        /*initial value*/
									 DxUint32_t                 ivSize,        /*size of IV in bytes*/
									 DxUint8_t                 *AddDataIn_ptr, /*Input data*/
									 DxUint32_t                 AddDataSize,   /*Data size, bytes*/
									 DxUint8_t                 *TextDataIn_ptr,/*Input data*/
									 DxUint32_t                 TextDataSize,  /*Data size, bytes*/
									 DxUint8_t                 *TextDataOut_ptr,/*Output data*/
									 DxUint8_t                 *tag_ptr,       /*Input/output of tag*/
									 DxUint32_t                 tagSize )      /*Tag size, bytes*/
{
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;

	/* a users context used to pass to all of the CRYS functions */
	CRYS_AESGCM_UserContext_t  ContextID;


	/* FUNCTION LOGIC */

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* ............... if not supported exit .............................. */
	/* -------------------------------------------------------------------- */

	RETURN_IF_AESGCM_UNSUPPORTED(
						encrMode, key, keySizeId, iv_ptr, ivSize, AddDataIn_ptr, AddDataSize,
						AddDataIn_ptr, TextDataSize, TextDataOut_ptr, tag_ptr, tagSize,
						ContextID.valid_tag, Error, Error, Error, Error, Error, Error,
						Error, Error, Error, Error, Error, Error );


#if !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)

    /* check data pointers and sizes: both AddData and TextData equalled
       to NULL are not allowed */
    if( AddDataIn_ptr == DX_NULL && AddDataSize != 0  )
	return  CRYS_AESGCM_DATA_POINTERS_INVALID_ERROR;

    if( TextDataIn_ptr == DX_NULL && TextDataSize != 0 )
	return  CRYS_AESGCM_DATA_POINTERS_INVALID_ERROR;

    if( TextDataOut_ptr == DX_NULL && TextDataSize != 0 )
	return  CRYS_AESGCM_DATA_POINTERS_INVALID_ERROR;


	/* ...............       Init the AESGCM     ........................... */
	/* --------------------------------------------------------------------- */

	Error = CRYS_AESGCM_Init(
		                   &ContextID,     /*AESGCM Context*/
							encrMode,      /*AESGCM encrypt mode*/
							key,           /*AESGCM key*/
							keySizeId,     /*AESGCM key size ID*/
						    iv_ptr,        /*initial value*/
							ivSize );      /*size of IV in bytes*/

	if( Error != CRYS_OK )
		goto End;


	/* ...............   process the Additional data    .................... */
	/* --------------------------------------------------------------------- */
	if( AddDataSize > 0 )
	{
		Error = CRYS_AESGCM_BlockAdata( &ContextID,
										AddDataIn_ptr,
										AddDataSize );

		if( Error != CRYS_OK )
			goto End;
	}

	/* .............   process the Text data  and finish  .................. */
	/* --------------------------------------------------------------------- */


	Error = CRYS_AESGCM_Finish(
						&ContextID,
						TextDataIn_ptr,
						TextDataSize,
						TextDataOut_ptr,
						tag_ptr,
						tagSize );


	/* ................   End of function        ............................ */
	/* --------------------------------------------------------------------- */
End:

	return Error;

#endif /* !(defined CRYS_NO_AESGCM_SUPPORT || defined CRYS_NO_AES_SUPPORT)  */

}/* END OF CRYS_AESGCM function */
