
/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

/* .............. CRYS level includes ................. */

#include "DX_VOS_Mem.h"
#include "CRYS_CCM.h"
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_DES_error.h"
#include "CRYS_DES_Local.h"

/* .............. LLF level includes .................. */

#ifndef CRYS_NO_DES_SUPPORT

#include "LLF_DES.h"

#endif /* CRYS_NO_DES_SUPPORT */

/************************ Defines ******************************/


/* canceling the PC-lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums ******************************/


/************************ MACROS ******************************/

/* this macro is required to remove compilers warnings if the DES is not supported */

#ifdef CRYS_NO_DES_SUPPORT
#define RETURN_IF_DES_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j ) \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j); \
  return CRYS_DES_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_DES_SUPPORT */
#define RETURN_IF_DES_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j )
#endif /* !CRYS_NO_DES_SUPPORT */

/************************ Global Data ******************************/


/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/* ------------------------------------------------------------
 * @brief This function is used to initialize the DES machine.
 *        In order to operate the DES machine the first function that should be
 *        called is this function.
 *
 *        The function executes the following major steps:
 *
 *        1. Validates all of the inputs of the function. If one of the received
 *           parameters is not valid it shall return an error:
 *
 *           - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *           - verifying the pointer of the IV counter is not DX_NULL
 *             if the modes that is selected are CBC.
 *           - verifying that the pointer to the key buffer is not DX_NULL
 *           - verifying the values of the number of keys is valid ( 0- 2 ).
 *           - verifying the value of the operation mode is valid ( 0 - 2 )
 *        2. Decrypting the received context to the working context after capturing
 *           the working context by calling the CRYS_CCM_GetContext() call.
 *
 *        3. Initializing the working context by the following:
 *           - loading the keys.
 *           - loading the IV counters.
 *           - loading the control field
 *           - loading the operation mode.
 *           - loading the DecryptEncrypt flag and the key size.
 *           - loading the key size.
 *        4. Encrypting the information in the working context and storing
 *           it to the users received context. After then the working context is released.
 *           This state is operated by calling the CRYS_CCM_EncryptAndReleaseContext call.
 *        5. Exit the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - a pointer to the DES context buffer allocated by the user that
 *                       is used for the DES machine operation.
 *
 * @param[in,out] IV_ptr - this parameter is the buffer of the IV or counters on mode CTR.
 *                          On ECB mode this parameter has no use.
 *                          On CBC this parameter should containe the IV values.
 *
 * @param[in] Key_ptr -  a pointer to the users key buffer.
 *
 * @param[in] NumOfKeys - the number of keys used by the module ( 1 - 3 )
 *
 * @param[in] EncryptDecryptFlag - This flag determains if the DES shall perform an Encrypt operation [0] or a
 *                           Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 */

CEXPORT_C CRYSError_t  CRYS_DES_Init(
                            CRYS_DESUserContext_t   *ContextID_ptr,
                            CRYS_DES_Iv_t            IV_ptr,
                            CRYS_DES_Key_t          *Key_ptr,
                            CRYS_DES_NumOfKeys_t     NumOfKeys,
                            CRYS_DES_EncryptMode_t   EncryptDecryptFlag,
                            CRYS_DES_OperationMode_t OperationMode )
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* defining a pointer to the active context allcated by the CCM */
   DESContext_t *ccmWorkingContext_ptr;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DES_UNSUPPORTED( ContextID_ptr , IV_ptr , Key_ptr ,
                              NumOfKeys , EncryptDecryptFlag , OperationMode ,
                              ccmWorkingContext_ptr , Error , Error , Error );

   #ifndef CRYS_NO_DES_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   {
      return CRYS_DES_INVALID_USER_CONTEXT_POINTER_ERROR;
   }

   /* init the vcontext_valid_tag field to avoid warnings */
   ContextID_ptr->valid_tag = 0;

   /* check if the operation mode is legal */
   if( OperationMode >= CRYS_DES_NumOfModes )
   {
      return CRYS_DES_ILLEGAL_OPERATION_MODE_ERROR;
   }

   /* if the operation mode selected is CBC then check the validity of
      the IV counter pointer */
   if( OperationMode == CRYS_DES_CBC_mode && IV_ptr == DX_NULL )
   {
      return CRYS_DES_INVALID_IV_PTR_ON_NON_ECB_MODE_ERROR;
   }

   /* If the number of keys in invalid return an error */
   if( NumOfKeys >= CRYS_DES_NumOfKeysOptions || NumOfKeys == 0 )
   {
      return CRYS_DES_ILLEGAL_NUM_OF_KEYS_ERROR;
   }

   /*check the valisity of the key pointer */
   if( Key_ptr == DX_NULL )
   {
      return CRYS_DES_INVALID_KEY_POINTER_ERROR;
   }

   /* Check the Encrypt / Decrypt flag validity */
   if( EncryptDecryptFlag >= CRYS_DES_EncryptNumOfOptions )
   {
      return CRYS_DES_INVALID_ENCRYPT_MODE_ERROR;
   }
#ifndef CRYS_NO_FIPS_SUPPORT
   /* check if we are in FIPS mode */
   if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_STATE)
   {
	/* check if there was a FIPS error */
		if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_ERROR_STATE)
	{
		return CRYS_DES_FIPS_MODE_FAIL_ERROR;
	}
	/* no check if the self test was already executed */
	if( !(DX_GLOBAL_FIPS_SF_STATUS & CRYS_SELF_TEST_DES_BIT) )
	{
		return CRYS_DES_FIPS_MODE_SF_NOT_PERFORMED_ERROR;
	}
	/* check for CBC-MAC - not allowed by FIPS */
	if(NumOfKeys == CRYS_DES_1_KeyInUse)
	{
		return CRYS_DES_FIPS_MODE_NOT_ALLOWED_ERROR;
	}
   }
#endif

   /* ................. aquiring the DES contest ............................. */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_DES_1KEY_CONTEXT,                       /* AES type - in */
                                AES_DONT_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/

    if( Error != CRYS_OK )

     return Error;

   /* ................. loading the context .................................. */
   /* ------------------------------------------------------------------------ */

   if( IV_ptr != DX_NULL )

        DX_VOS_FastMemCpy( ccmWorkingContext_ptr->DESContextIV , IV_ptr , sizeof(CRYS_DES_Iv_t) );

   DX_VOS_FastMemCpy( &ccmWorkingContext_ptr->DESContextKey , Key_ptr , sizeof(CRYS_DES_Key_t) );

   ccmWorkingContext_ptr->NumOfKeys          = NumOfKeys;
   ccmWorkingContext_ptr->EncryptDecryptFlag = EncryptDecryptFlag;
   ccmWorkingContext_ptr->OperationMode      = OperationMode;

   /* set the DES tag to the users context */
   ContextID_ptr->valid_tag = DES_CONTEXT_VALIDATION_TAG;

    /* ................. calling the low level init function ................. */
   /* ----------------------------------------------------------------------- */

   LLF_DES_InitInit( ccmWorkingContext_ptr );

   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_ReleaseContext(ContextID_ptr,				/* the users context space - in */
                                   ccmWorkingContext_ptr,       /* the CCM context returned - in */
                                   DX_DES_1KEY_CONTEXT);        /* DES type - in */

   if( Error != CRYS_OK )

     goto End;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   End:

   if( Error != CRYS_OK )

      /* call the free function - release the users context */
      CRYS_DES_Free( ContextID_ptr );

   return Error;

   #endif /* CRYS_NO_DES_SUPPORT */

}/* END OF DES_Init */

/** ------------------------------------------------------------
 * @brief This function is used to operate a block on the DES machine.
 *        This function should be called after the CRYS_DES_Init function
 *        was called.
 *
 *        The function executes the following major steps:
 *
 *        1.Checks the validation of all of the inputs of the function.
 *          If one of the received parameters is not valid it shall return an error.
 *
 *        2.Decrypting the received context to the working context after
 *          capturing the working context by calling the CRYS_CCM_GetContext() call.
 *
 *        3.executing the DES operation on the hardware by calling the
 *          low level DES function LLF_DES_Block.
 *        4.Encrypting the information in the working context and storing it
 *          to the users received context. After then the working context is released.
 *          This state is operated by calling the CRYS_CCM_ReleaseContext call.
 *        5.Exit the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - a pointer to the DES context buffer allocated by the user that
 *                       is used for the DES machine operation. this should be the same context that was
 *                       used on the previous call of this session.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the DES. The pointer does
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment).
 *
 * @param[in] DataInSize - The size of the input data in bytes: must be not 0 and must be multiple
 *                         of 8 bytes.
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data from the DES. The pointer does not
 *                              need to be aligned. On CSI output mode the pointer must be equal to
 *                              value (0xFFFFFFFC | DataOutAlignment).
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 *
 *     NOTE: Temporarily it is not allowed, that both the Input and the Output simultaneously
 *           were on CSI mode
 */
 CEXPORT_C CRYSError_t  CRYS_DES_Block(
                              CRYS_DESUserContext_t       *ContextID_ptr,
                              DxUint8_t                     *DataIn_ptr,
                              DxUint32_t                    DataInSize,
                              DxUint8_t                     *DataOut_ptr )
 {
   /* FUNCTION DECLARATIONS */

   /* The return error identifiers */
   CRYSError_t Error,Error1;

   /* defining a pointer to the active context allcated by the CCM */
   DESContext_t *ccmWorkingContext_ptr;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;
   Error1 = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DES_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize ,
                              DataOut_ptr , ccmWorkingContext_ptr , Error ,
                              DataOut_ptr , DataOut_ptr , DataOut_ptr , DataOut_ptr );

   #ifndef CRYS_NO_DES_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )

      return CRYS_DES_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != DES_CONTEXT_VALIDATION_TAG )

      return CRYS_DES_USER_CONTEXT_CORRUPTED_ERROR;

   /* if the users Data In pointer is illegal return an error */
   if( DataIn_ptr == DX_NULL )

      return CRYS_DES_DATA_IN_POINTER_INVALID_ERROR;

   /* if the users Data Out pointer is illegal return an error */
   if( DataOut_ptr == DX_NULL )

      return CRYS_DES_DATA_OUT_POINTER_INVALID_ERROR;

   if( ((DxUint32_t)DataIn_ptr >= DX_CSI) && ((DxUint32_t)DataOut_ptr >= DX_CSI) )

	  return CRYS_DES_IN_OUT_CSI_TO_CSI_IS_NOT_ALLOWED_ERROR;

   /* checking that there is no overlapping between the data input and data out put buffer
      except the inplace case that is legal */
   if( DataIn_ptr != DataOut_ptr )
   {
      /* checking the case that the input buffer is in a higher address then the output buffer */
      if ( DataIn_ptr > DataOut_ptr )
      {
         /* if after adding the size to the data out pointer it is larger then the data in pointer
            return the overlap error */
         if( DataOut_ptr + DataInSize > DataIn_ptr )

            return CRYS_DES_DATA_OUT_DATA_IN_OVERLAP_ERROR;

      }/* end of DataIn_ptr > DataOut_ptr */

      /* checking the case that the output buffer is in a higher address then the input buffer */
      else
      {
         /* if after adding the size to the data in pointer it is larger then the data out pointer
         return the overlap error */
         if( DataIn_ptr + DataInSize > DataOut_ptr )

            return CRYS_DES_DATA_OUT_DATA_IN_OVERLAP_ERROR;

      }/* end of DataOut_ptr > DataIn_ptr */

   }/* end of DataIn_ptr != DataOut_ptr case */

   /* if the data size is zero or not a multiple of 16 bytes return error */
   if( DataInSize == 0 || (DataInSize % CRYS_DES_BLOCK_SIZE_IN_BYTES) != 0 )

      return CRYS_DES_DATA_SIZE_ILLEGAL;

   /* ................. aquiring the DES contest ............................. */
   /* ----------------------------------------------------------------------- */
   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_DES_1KEY_CONTEXT,                       /* AES type - in */
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/

   if( Error != CRYS_OK )

     return Error;

   /* .................. calling the hardware low level block function ....... */
   /* ------------------------------------------------------------------------ */

   Error = LLF_DES_Block( ccmWorkingContext_ptr,      /* the working context - in */
                          DataIn_ptr,                 /* the input data buffer - in */
                          DataInSize,                 /* the data in size - in */
                          DataOut_ptr );              /* the output data buffer i/o */

   if( Error != CRYS_OK )

     goto End;

   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

   End:

   Error1 = CRYS_CCM_ReleaseContext(ContextID_ptr,         /* the users context space - in */
                           ccmWorkingContext_ptr, /* the CCM context returned - in */
                           DX_DES_1KEY_CONTEXT);  /* DES type - in */

   if(Error == CRYS_OK)
   {
        Error = Error1;
   }

   /* clear the output buffer in case of an error */
   if(Error)
   {
	DX_VOS_MemSetZero(DataOut_ptr , DataInSize);
   }
   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   return Error;

   #endif /* CRYS_NO_DES_SUPPORT */

}/* END OF CRYS_DES_Block */

/**
 * @brief This function is used to end the DES operation seesion.
 *        It is the last function called on the DES operation.
 *
 *
 *        The function executes the following major steps:
 *
 *        1. Checks the validity of all of the inputs of the function.
 *           If one of the received parameters is not valid it shall return an error.
 *
 *           The major checkers that are run over the received parameters:
 *           - verifying that the context pointer is not DX_NULL (*ContextID_ptr).
 *        2. Clearing the users context.
 *        3. Exit the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - a pointer to the DES context buffer allocated by the user that
 *                       is used for the DES machine operation. this should be the same context that was
 *                       used on the previous call of this session.
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 */
CEXPORT_C CRYSError_t  CRYS_DES_Free(CRYS_DESUserContext_t  *ContextID_ptr )
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DES_UNSUPPORTED( ContextID_ptr , Error , Error , Error ,
                              Error , Error , Error , Error , Error , Error );

   #ifndef CRYS_NO_DES_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
   {
      Error = CRYS_DES_INVALID_USER_CONTEXT_POINTER_ERROR;
      goto End;
   }

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != DES_CONTEXT_VALIDATION_TAG )
   {
      Error = CRYS_DES_USER_CONTEXT_CORRUPTED_ERROR;
      goto End;
   }

   /* .............. clearing the users context .......................... */
   /* -------------------------------------------------------------------- */

   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_DESUserContext_t) );

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   End:

   return Error;

   #endif /* CRYS_NO_DES_SUPPORT */

}/* END OF CRYS_DES_Free */

/**
 * @brief This function is used to operate the DES machine in one integrated operation.
 *
 *        The actual macros that will be used by the users are:
 *
 *
 * @param[in,out] IVCounter_ptr - this parameter is the buffer of the IV or counters on mode CTR.
 *                          On ECB mode this parameter has no use.
 *                          On CBC mode this parameter should containe the IV values.
 *
 * @param[in] Key_ptr - a pointer to the users key buffer.
 *
 * @param[in] KeySize - Thenumber of keys used by the DES as defined in the enum.
 *
 * @param[in] EncryptDecryptFlag - This flag determains if the DES shall perform an Encrypt operation [0] or a
 *                           Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] DataIn_ptr - The pointer to the buffer of the input data to the DES. The pointer does
 *                         not need to be aligned. On CSI input mode the pointer must be equal to
 *                         value (0xFFFFFFFC | DataInAlignment).
 *
 * @param[in] DataInSize - The size of the input data in bytes: must be not 0 and must be multiple
 *                         of 8 bytes.
 *
 * @param[in/out] DataOut_ptr - The pointer to the buffer of the output data from the DES. The pointer does not
 *                              need to be aligned. On CSI output mode the pointer must be equal to
 *                              value (0xFFFFFFFC | DataOutAlignment).
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* CRYS_DES_error.h
 *
 *     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously
 *               were on CSI mode.
 *            2. Temporarily the CSI input or output are not allowed on XCBC, CMAC and XTS modes.
 *
 */
 CEXPORT_C CRYSError_t  CRYS_DES(
                CRYS_DES_Iv_t             IV_ptr,                 /* in */
                CRYS_DES_Key_t           *Key_ptr,                /* in */
                CRYS_DES_NumOfKeys_t      NumOfKeys,              /* in */
                CRYS_DES_EncryptMode_t    EncryptDecryptFlag,     /* in */
                CRYS_DES_OperationMode_t  OperationMode,          /* in */
                DxUint8_t                *DataIn_ptr,             /* in */
                DxUint32_t                DataInSize,             /* in */
                DxUint8_t                *DataOut_ptr )           /* out */
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* a users context used to pass to all of the CRYS functions */
   CRYS_DESUserContext_t  ContextID;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DES_UNSUPPORTED( IV_ptr , Key_ptr , NumOfKeys ,
                              EncryptDecryptFlag , OperationMode , DataIn_ptr ,
                              DataInSize , DataOut_ptr , ContextID.valid_tag , Error );

   #ifndef CRYS_NO_DES_SUPPORT

   /* ............... calling the CRYS init function ...................... */
   /* --------------------------------------------------------------------- */

   Error = CRYS_DES_Init(  &ContextID,
                           IV_ptr,
                           Key_ptr,
                           NumOfKeys,
                           EncryptDecryptFlag,
                           OperationMode );

   if( Error != CRYS_OK )

     goto End;

   /* ............... calling the CRYS BLOCK function ...................... */
   /* --------------------------------------------------------------------- */

   Error = CRYS_DES_Block( &ContextID,
                            DataIn_ptr,
                            DataInSize,
                            DataOut_ptr );

   if( Error != CRYS_OK )

     goto End;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   End:

   /* call the free function - release the users context */
   CRYS_DES_Free( &ContextID );

   return Error;

   #endif /* CRYS_NO_DES_SUPPORT */

}/* END OF CRYS_DES */
