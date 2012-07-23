

  /*
   *  Object % CRYS_C2_Cipher    : %
   *  State           :  %state%
   *  Creation date   :  Feb. 20 2007
   *  Last modified   :  Sept. 09 2009
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_C2_Cipher.c#1:csrc:
   *  \author R.Levin .
   *
   *  \remarks Copyright (C) 2009 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

/* .............. CRYS level includes ................. */

#include "DX_VOS_Mem.h"
#include "CRYS_C2_error.h"
#include "CRYS.h"
#include "CRYS_CCM.h"
#include "CRYS_C2_Local.h"
#include "CRYS_C2.h"
#include "log_output.h"
/* .............. LLF level includes .................. */

#ifndef CRYS_NO_C2_SUPPORT

#include "LLF_C2.h"

#endif /* CRYS_NO_C2_SUPPORT */

/************************ Defines **********************8***************/


/* canceling the PC-lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums *****************************************/

/************************ MACROS ****************************************/

/************************ Global Data ***********************************/

/************* Private function prototype *******************************/


/*****************************************************************************************************/
/**********                                Public Functions                                ***********/
/*****************************************************************************************************/


/*****************************************************************************************************/
/*
 * @brief This function is used to initialize the C2 Cipher secret constant
 *
 *        The function executes the following major steps:
 *
 *        1. Validating all of the input parameters. If of them is not valid it shall return an error
 *           parameters is not valid it shall return an error:
 *
 *        2. Call to LLF_C2_SBoxInitto to initialize the Secret constant.
 *
 * @param[in] aSecretConstant - A pointer to the Secret Constant that
 *                            is used for the C2 Cipher operations.
 *
 * @param[in] aConstantSize -  secret constant size.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_C2_error.h
 */

CEXPORT_C CRYSError_t  CRYS_C2_SBoxInit(const DxUint8_t *aSecretConstant,
                                        DxUint32_t aConstantSize)
{
  CRYSError_t Error = CRYS_OK;

  if(aSecretConstant == DX_NULL || aConstantSize == 0)
     return CRYS_C2_CIPHER_INVALID_ARGUMENT_ERROR;

 // Error = CRYS_AES_SecretKey(DX_NULL, CRYS_AES_Decrypt, CRYS_AES_ECB_mode,
 //                            aSecretConstant, aConstantSize,aSecretConstant);

  Error = LLF_C2_SBoxInit(aSecretConstant,aConstantSize);
  if (Error != CRYS_OK)
  {
  //	CRYS_AES_SecretKey(DX_NULL, CRYS_AES_Encrypt, CRYS_AES_ECB_mode,
    //                   aSecretConstant, aConstantSize,aSecretConstant);

    return CRYS_C2_CIPHER_SEC_CONST_LOAD_ERROR;
  }

 // Error = CRYS_AES_SecretKey(DX_NULL, CRYS_AES_Encrypt, CRYS_AES_ECB_mode,
   //                          aSecretConstant, aConstantSize,aSecretConstant);

  return Error;

}//End of CRYS_C2_SBoxInit

/*****************************************************************************************************/
/*
 * @brief This function is used to initialize the C2 Cipher structures and machine registers .
 *        In order to perform the C2 Cipher this function should be called first.
 *
 *        The function executes the following major steps:
 *
 *        1. Validating all of the inputs of the function. If one of the received
 *           parameters is not valid it shall return an error:
 *            - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *            - verifying that the pointer to the key buffer is not DX_NULL.
 *            - verifying the value of the operation mode is valid .
 *        2. Capturing the working context by calling the CRYS_CCM_GetContext().
 *        3. Initializing the working context by the following:
 *            - loading the key.
 *            - loading the control field
 *            - loading the operation mode.
 *            - loading the DecryptEncrypt flag .
 *        4. Encrypting the information in the working context and storing it to the
 *           users received context. After then the working context is released.
 *           This state is operated by calling the CRYS_CCM_EncryptAndReleaseContext call.
 *        5. Exit the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the context buffer allocated by the user that
 *                            is used for the C2 Cipher operations.
 *
 * @param[in] Key_ptr -  A pointer to the users key buffer.
 *
 * @param[in] EncryptDecryptMode - This flag determines if the C2 shall perform an Encrypt
 *                                 operation [0] or a Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] CBC_ResetInterval -  CBC chain breaking interval. The value not used in ECB mode .
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_C2_error.h
 */
CEXPORT_C CRYSError_t  CRYS_C2_CipherInit(
                            CRYS_C2CipherUserContext_t    *ContextID_ptr,
                            CRYS_C2_Key_t                 Key_ptr,
                            CRYS_C2_EncryptMode_t         EncryptDecryptMode,
                            CRYS_C2_OperationMode_t       OperationMode,
                            DxUint32_t                    CBC_ResetInterval)
{
   /* FUNCTION DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* defining a pointer to the active context allocated by the CCM */
   C2CipherContext_t *ccmWorkingContext_ptr;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_C2_UNSUPPORTED( ContextID_ptr , Key_ptr , EncryptDecryptMode ,
                             OperationMode , CBC_ResetInterval , ccmWorkingContext_ptr ,
                             key_ptr , Error , Error, Error );

   #ifndef CRYS_NO_C2_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */
    PRINTF("Entered CRYS_C2_CipherInit\n");
   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )

      return CRYS_C2_CIPHER_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* init the context_valid_tag field to avoid warnings */
   ContextID_ptr->valid_tag = 0;

   /* check if the operation mode is legal */
   if( OperationMode >= CRYS_C2_NumOfModes )

      return CRYS_C2_CIPHER_ILLEGAL_OPERATION_MODE_ERROR;

   /*check the validity of the key pointer */
   if( Key_ptr == DX_NULL )

	   return CRYS_C2_CIPHER_INVALID_KEY_POINTER_ERROR;

   /* Check the Encrypt / Decrypt flag validity */
   if( EncryptDecryptMode >= CRYS_C2_EncryptNumOfOptions )

      return CRYS_C2_CIPHER_INVALID_ENCRYPT_MODE_ERROR;


   /* ......... acquiring the C2 Cipher contest ............................. */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_C2_CIPHER_CONTEXT,                /* AES type - in */
                                AES_DONT_DECRYPT_CONTEXT);		     /* need to decrypt context in AES_block*/
    if( Error != CRYS_OK )

      return Error;

   /* ................. loading the context .................................. */
   /* ------------------------------------------------------------------------ */

   /* set the C2 Cipher tag to the users context */
   ContextID_ptr->valid_tag = C2_CIPHER_CONTEXT_VALIDATION_TAG;


    /* ................ calling the low level init function ................. */
   /* ----------------------------------------------------------------------- */

   ccmWorkingContext_ptr = (C2CipherContext_t *)ContextID_ptr->context_buff;

   LLF_C2_CipherInit(ccmWorkingContext_ptr,
                     Key_ptr,
                     EncryptDecryptMode,
                     OperationMode,
                     CBC_ResetInterval);


   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_ReleaseContext(ContextID_ptr,				/* the users context space - in */
                                   ccmWorkingContext_ptr,       /* the CCM context returned - in */
                                   DX_C2_CIPHER_CONTEXT);       /* C2Cipher type - in */


   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
    if( Error != CRYS_OK )

      /* call the free function - release the users context */
      CRYS_C2_CipherFree( ContextID_ptr );

   return Error;

   #endif /* CRYS_NO_C2_SUPPORT */

}/* END OF CRYS_C2_CipherInit */


/*****************************************************************************************************/
/**
 * @brief This function is used to perform C2 Cipher operation on a block of data.
 *
 *        This function should be called after the CRYS_C2_Init function
 *        was called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *          The major checkers that are run over the received parameters:
 *          - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *          - verifying that the pointer of the data_in buffer is not DX_NULL.
 *          - verifying that the pointer to the data_out buffer is not DX_NULL.
 *          - verifying the values of the data_in buffers size is not 0 and is a multiple of 8 bytes.
 *
 *        2.Decrypts the received context to the working context after
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *        3.Executes the C2 Cipher operation on the SW or HW by calling the
 *          low level function LLF_C2_CipherBlock.
 *        4.Encrypts the information in the working context and stores it
 *          to the users received context and releases the context.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exits the handler with the error code.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the C2 Cipher context buffer allocated by the user that
 *                            is used for the C2 operation. This should be the same context that was
 *                            used on the previous call of this session.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the C2 Cipher. The pointer does
 *                         not need to be aligned to 32 bits word.
 *
 * @param[in] DataInSize - The size of the input data (must be not 0 and must be multiple of 8 bytes).
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data from the C2 Cipher.
 *                              The pointer does not need to be aligned to 32 bits word.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
 CEXPORT_C CRYSError_t  CRYS_C2_CipherBlock(
                                CRYS_C2CipherUserContext_t       *ContextID_ptr,
                                DxUint8_t                        *DataIn_ptr,
                                DxUint32_t                        DataInSize,
                                DxUint8_t                        *DataOut_ptr )
 {

   /* FUNCTION DECLARATIONS */

   /* The return error identifiers */
   CRYSError_t Error;

   /* defining a pointer to the active context allocated by the CCM */
   C2CipherContext_t *ccmWorkingContext_ptr;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;



   PRINTF("Entered CRYS_C2_CipherBlock\n");
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */
   RETURN_IF_C2_UNSUPPORTED(  ContextID_ptr , DataIn_ptr , DataInSize ,
                              DataOut_ptr , ccmWorkingContext_ptr , Error ,
                              Error , Error , Error , Error );

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */
#ifndef CRYS_NO_C2_SUPPORT
   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )

      return CRYS_C2_CIPHER_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != C2_CIPHER_CONTEXT_VALIDATION_TAG )

      return CRYS_C2_CIPHER_USER_CONTEXT_CORRUPTED_ERROR;

   /* if the users Data In pointer is illegal return an error */
   if( DataIn_ptr == DX_NULL )

      return CRYS_C2_CIPHER_DATA_IN_POINTER_INVALID_ERROR;

   /* if the users Data Out pointer is illegal return an error */
   if( DataOut_ptr == DX_NULL )

      return CRYS_C2_CIPHER_DATA_OUT_POINTER_INVALID_ERROR;

   /* checking that there is no overlapping between the data input and data out put buffer
      except the in place case that is legal */
   if( DataIn_ptr != DataOut_ptr )
   {
      /* checking the case that the input buffer is in a higher address then the output buffer */
      if ( DataIn_ptr > DataOut_ptr )
      {
         /* if after adding the size to the data out pointer it is larger then the data in pointer
            return the overlap error */
         if( DataOut_ptr + DataInSize > DataIn_ptr )

            return CRYS_C2_CIPHER_DATA_OUT_DATA_IN_OVERLAP_ERROR;

      }/* end of DataIn_ptr > DataOut_ptr */

      /* checking the case that the output buffer is in a higher address then the input buffer */
      else
      {
         /* if after adding the size to the data in pointer it is larger then the data out pointer
         return the overlap error */
         if( DataIn_ptr + DataInSize > DataOut_ptr )

            return CRYS_C2_CIPHER_DATA_OUT_DATA_IN_OVERLAP_ERROR;

      }/* end of DataOut_ptr > DataIn_ptr */

   }/* end of DataIn_ptr != DataOut_ptr case */

   /* if the data size is zero or not a multiple of 8 bytes return error */
   if( DataInSize == 0 || (DataInSize % CRYS_C2_BLOCK_SIZE_IN_BYTES) != 0 )
      return CRYS_C2_CIPHER_DATA_SIZE_ILLEGAL;


   /* ................. acquiring the C2 CIPHER contest ............................. */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_C2_CIPHER_CONTEXT,                /* C2_CIPHER type - in */
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/

   if( Error != CRYS_OK )
     return Error;

   /* .................. calling the hardware low level block function ....... */
   /* ------------------------------------------------------------------------ */

   Error = LLF_C2_CipherBlock(
                          ccmWorkingContext_ptr,      /* the working context - in */
                          DataIn_ptr,                 /* the input data buffer - in */
                          DataInSize,                 /* the data in size - in */
                          DataOut_ptr );              /* the output data buffer i/o */

   if( Error != CRYS_OK )

     goto End;


        /* ................. end of function ..................................... */
/* ----------------------------------------------------------------------- */
End:
   CRYS_CCM_ReleaseContext(ContextID_ptr,          /* the users context space - in */
                           ccmWorkingContext_ptr,  /* the CCM context returned - in */
                           DX_C2_CIPHER_CONTEXT);  /* C2 Cipher type - in */

#endif /* CRYS_NO_C2_SUPPORT */

   return Error;
}/* END OF CRYS_C2_CipherBlock */


/*****************************************************************************************************/
/**
 * @brief This function is used to end the C2 Cipher operation session.
 *
 *        It is the last function called on the C2 Cipher operation.
 *
 *        The function executes the following major steps:
 *
 *        1. Checks that the context pointer is not DX_NULL (*ContextID_ptr). .
 *           If received parameter is not valid it shall return an error.
 *        2. Clearing the users context.
 *        3. Exits the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - a pointer to the C2_CIPHER context buffer allocated by the user that
 *                            was used on the previous call of this session.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value from CRYS_C2_error.h
 */
CEXPORT_C CRYSError_t  CRYS_C2_CipherFree(CRYS_C2CipherUserContext_t  *ContextID_ptr )
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* FUNCTION LOGIC */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_C2_UNSUPPORTED( ContextID_ptr , Error, Error , Error , Error ,
                              Error , Error , Error , Error , Error );

 #ifndef CRYS_NO_C2_SUPPORT
   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */
   PRINTF("Entered CRYS_C2_CipherFree \n");
   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_C2_CIPHER_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != C2_CIPHER_CONTEXT_VALIDATION_TAG )
      return CRYS_C2_CIPHER_USER_CONTEXT_CORRUPTED_ERROR;

   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */

   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_C2CipherUserContext_t) );

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
#endif
   return Error;


}/* END OF CRYS_C2_ipherFree */


/********************************************************************************************************/
/**
 * @brief This function is used to operate the C2 Cipher in one integrated operation.
 *
 *        The function simply calls the C2_Cipher Init, Block and Finish functions sequentially.
 *
 * @param[in] Key_ptr - a pointer to the users key buffer.
 *
 * @param[in] EncryptDecryptMode - This flag determines if the C2 shall perform an Encrypt operation [0] or a
 *                                 Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] CBC_ResetInterval - CBC chain breaking interval. The value not used in ECB mode
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the C2 Cipher.
 *                         The pointer does not need to be word-aligned.
 *
 * @param[in] DataInSize - The size of the input data (must be not 0 and must be multiple of 8 bytes).
 *
 * @param[in,out] DataOut_ptr - The pointer to the buffer of the output data from the C2 Cipher.
 *                              The pointer does not need to be word-aligned.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */

 CEXPORT_C CRYSError_t  CRYS_C2_Cipher(
                CRYS_C2_Key_t           Key_ptr,                /* in */
                CRYS_C2_EncryptMode_t    EncryptDecryptFlag,     /* in */
                CRYS_C2_OperationMode_t  OperationMode,          /* in */
                DxUint32_t               CBC_ResetInterval ,
                DxUint8_t               *DataIn_ptr,             /* in */
                DxUint32_t               DataInSize,             /* in */
                DxUint8_t               *DataOut_ptr )           /* in/out */
{

   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* a users context used to pass to all of the CRYS functions */
   CRYS_C2CipherUserContext_t  ContextID;

   /* FUNCTION LOGIC */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_C2_UNSUPPORTED(  Key_ptr , EncryptDecryptFlag , OperationMode , CBC_ResetInterval ,
                              DataIn_ptr , DataInSize , DataOut_ptr , ContextID.valid_tag ,
                              Error , Error  );

 PRINTF("CRYS_C2_Cipher func starts\n");
#ifndef CRYS_NO_C2_SUPPORT
        /* ............... calling the CRYS init function ...................... */
   /* --------------------------------------------------------------------- */
   Error = CRYS_C2_CipherInit(
                           &ContextID,
                           Key_ptr,
                           EncryptDecryptFlag,
                           OperationMode,
                           CBC_ResetInterval );

   if( Error != CRYS_OK )
     goto End;

   /* ............... calling the CRYS BLOCK function ...................... */
   /* --------------------------------------------------------------------- */
   Error = CRYS_C2_CipherBlock(
                            &ContextID,
                            DataIn_ptr,
                            DataInSize,
                            DataOut_ptr );

   if( Error != CRYS_OK )
     goto End;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

End:
   /* call the free function - release the users context */
   CRYS_C2_CipherFree( &ContextID );

#endif /* CRYS_NO_C2_SUPPORT */

   return Error;

}/* END OF CRYS_C2_Cipher */


#if (LLF_C2_ENGINE_TYPE == CRYS_DEFS_SW_ENGINE_TYPE)
 /*****************************************************************************************************/
 /**
 * @brief This function is used to reset CBC Key chain.
 *
 *        This function should be called for CBC Key Chaim reseting.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *        2.Decrypting the received context to the working context after
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.

 *        3.executing the C2 CBC Key Reset operation on the SW by calling the
 *          low level function LLF_C2_CipherBlock.
 *        4.Encrypting the information in the working context and storing it
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exit the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the C2 Cipher context buffer allocated by the user that
 *                            is used for the C2 operation. This should be the same context that was
 *                            used on the previous call of this session.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value from CRYS_C2_error.h
 */
 CEXPORT_C CRYSError_t  CRYS_C2_CBC_ResetChain(CRYS_C2CipherUserContext_t *ContextID_ptr )
 {
	 /* FUNCTION DECLARATIONS */

	 /* The return error identifier */
	 CRYSError_t Error;

	 /* defining a pointer to the active context allocated by the CCM */
	 C2CipherContext_t *ccmWorkingContext_ptr;

	 /* FUNCTION LOGIC */

	 /* initializing the Error to O.K */
	 Error = CRYS_OK;

	 /* ............... if not supported exit .............................. */
	 /* -------------------------------------------------------------------- */

	 RETURN_IF_C2_UNSUPPORTED( ContextID_ptr , ccmWorkingContext_ptr, Error , Error ,
		                       Error , Error , Error , Error , Error , Error );

#ifndef CRYS_NO_C2_SUPPORT
	 /* ............... checking the parameters validity ................... */
	 /* -------------------------------------------------------------------- */

	 /* if the users context ID pointer is DX_NULL return an error */
	 if( ContextID_ptr == DX_NULL )
		 return CRYS_C2_CIPHER_INVALID_USER_CONTEXT_POINTER_ERROR;

	 /* if the users context TAG is illegal return an error - the context is invalid */
	 if( ContextID_ptr->valid_tag != C2_CIPHER_CONTEXT_VALIDATION_TAG )
		 return CRYS_C2_CIPHER_USER_CONTEXT_CORRUPTED_ERROR;

	 /* ................. acquiring the C2 CIPHER contest ............................. */
	 /* ----------------------------------------------------------------------- */
	 Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
		 (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
		 DX_C2_CIPHER_CONTEXT,                /* C2_CIPHER type - in */
		 AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/

	 if( Error != CRYS_OK )
		 return Error;

	 /* .................. calling the software low level reset key function ....... */
	 /* ------------------------------------------------------------------------ */

     LLF_C2_CipherResetKey(ccmWorkingContext_ptr);

	 Error = CRYS_CCM_ReleaseContext(ContextID_ptr,          /* the users context space - in */
		                     ccmWorkingContext_ptr,  /* the CCM context returned - in */
		                     DX_C2_CIPHER_CONTEXT);  /* C2 Cipher type - in */

#endif
	 return Error;
 }
#endif
