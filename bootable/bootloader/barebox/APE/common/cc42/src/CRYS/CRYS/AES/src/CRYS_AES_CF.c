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
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_AES_Local.h"
#include "CRYS_AES_error.h"
#include "CRYS_CCM.h"

/* .............. LLF level includes .................. */

#ifndef CRYS_NO_AES_SUPPORT
#include "LLF_AES.h"
#endif /* CRYS_NO_AES_SUPPORT */

/******************************** Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ MACROS ***************************************/

/************************ Global Data ***********************************/

/************* Private function prototype *******************************/


/****************************************************************************************************/
/**
 * @brief Performs specific initialization operations for 4 modes ECB,CBC,CTR,MAC.
 *
 **/
static CRYSError_t  CRYS_AES_EcbCbcCtrMac_Init(
						   AESContext_t            *AESContext_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           DxUint32_t               KeySizeBytes,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key );


#ifndef CRYS_NO_AES_XCBC_CMAC_MODES_SUPPORT
/****************************************************************************************************/
/**
 * @brief Performs specific initialization operations for XCBC and CMAC modes.
 *
 **/
static CRYSError_t  CRYS_AES_XcbcCmac_Init(
						   AESContext_t            *AESContext_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           DxUint32_t               KeySizeBytes,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key );
#endif

#ifndef CRYS_NO_AES_XTS_MODE_SUPPORT
/****************************************************************************************************/
/**
 * @brief Performs specific initialization operations for XTS mode.
 *
 **/
static CRYSError_t  CRYS_AES_Xts_Init(
						   AESContext_t            *AESContext_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           DxUint32_t               KeySizeBytes,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key );
#endif


/****************************************************************************************************/
/****************************************************************************************************/
/*                                         Public Functions                                         */
/****************************************************************************************************/
/****************************************************************************************************/


/****************************************************************************************************/
/**
 * @brief This function is used to initialize the AES machine or SW structures.
 *        To perform the AES operations this should be the first function called.
 *
 *        The actual macros, that will be used by the user for calling this function, are described
 *        in CRYS_AES.h file.
 *
 *        The function executes the following major steps:
 *
 *        1. Validates all of the inputs of the function. If one of the received
 *           parameters is not valid it shall return an error:
 *        2. Decrypts the received context to the working context after capturing
 *           the working context by calling the CRYS_CCM_GetContext() call.
 *        3. Initializes the working context by the parameters used in other AES operations.
 *           On some modes  calculates derived keys (for XCBC,CMAC)or Tweak value (for XTS).
 *        4. Encrypts the working context and stores it to the users received context.
 *           Releases the working context by calling the CRYS_CCM_EncryptAndReleaseContext call.
 *        5. Exits the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the AES context buffer that is allocated by the user
 *                            and is used for the AES machine operation.
 * @param[in] IVCounter_ptr - A buffer containing an initial value: IV, Counter or Tweak according
 *                            to operation mode:
 *                            - on ECB, MAC, XCBC, CMAC mode this parameter is not used and may be NULL,
 *                            - on CBC mode it contains the IV value,
 *                            - on CTR mode it contains the init counter,
 *                            - on XTS mode it contains the initial tweak value - 128-bit consecutive number
 *                              of data unit (in little endian).
 * @param[in] Key_ptr    -  A pointer to the user's key buffer.
 * @param[in] KeySizeID  -  An enum parameter, defines size of used key (128, 192, 256, 512 bits):
 *                          On XCBC mode allowed 128 bit size only, on XTS - 256 or 512 bit, on other modes <= 256 bit.
 * @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0)
 *                                 or a Decrypt operation (1). In XCBC and CMAC modes it must be Encrypt.
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, CTR, XCBC (PRF and 96), CMAC.
 * @param[in] is_secret_key - This parameter intended for internal Discretix using only !.
 *                            The parameter specifying whether to use a secret key (1)
 *                            or to use a key defined by the user (0).
 *                            The secret key function depends on product configuration.
 *                            Please consult the reference manual.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, and on failure - a value from CRYS_AES_error.h
 */

CEXPORT_C CRYSError_t  _DX_AES_Init(
						   CRYS_AESUserContext_t   *ContextID_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           CRYS_AES_KeySize_t       KeySizeID,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key )
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifiers */
   CRYSError_t Error, ErrorCcm;

   /* defining a pointer to the active context allocated by the CCM */
   AESContext_t *ccmWorkingContext_ptr;

   /* Aes key size bytes */
   DxUint32_t KeySizeBytes;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AES_UNSUPPORTED( ContextID_ptr , IVCounter_ptr , Key_ptr , KeySizeID ,
                              EncryptDecryptFlag , OperationMode , ccmWorkingContext_ptr ,
                              is_secret_key ,  ErrorCcm , KeySizeBytes , Error , Error ,
                              Error , Error , Error , Error );

   #ifndef CRYS_NO_AES_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_AES_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* init the valid_tag filed to avoid warnings*/
   ContextID_ptr->valid_tag = 0;

   /* check if the operation mode is legal */
   if( OperationMode >= CRYS_AES_NumOfModes )
      return  CRYS_AES_ILLEGAL_OPERATION_MODE_ERROR;

#ifdef CRYS_NO_AES_XTS_MODE_SUPPORT
   /* illegal mode XTS */
   if( OperationMode == CRYS_AES_XTS_mode )
      return  CRYS_AES_ILLEGAL_OPERATION_MODE_ERROR;
#endif

#ifdef CRYS_NO_AES_XCBC_CMAC_MODES_SUPPORT
   /* illegal mode XCBC_CMAC */
   if( OperationMode == CRYS_AES_XCBC_MAC_mode || OperationMode == CRYS_AES_CMAC_mode )
      return  CRYS_AES_ILLEGAL_OPERATION_MODE_ERROR;
#endif

   /* if illegal secret key mode return an error */
   if( is_secret_key != DX_TRUE && is_secret_key != DX_FALSE)
      return CRYS_AES_ILLEGAL_SECRET_KEY_MODE;

   /* if the secret key mode is not selected then check the validity of the key pointer */
   if( is_secret_key == DX_FALSE && Key_ptr == DX_NULL )
      return  CRYS_AES_INVALID_KEY_POINTER_ERROR;

   /* check the Encrypt / Decrypt flag validity */
   if( EncryptDecryptFlag >= CRYS_AES_EncryptNumOfOptions )
      return  CRYS_AES_INVALID_ENCRYPT_MODE_ERROR;

#ifndef CRYS_NO_FIPS_SUPPORT
   /* check if we are in FIPS mode */
   if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_STATE)
   {
	/* check if there was a FIPS error */
		if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_ERROR_STATE)
	{
		return CRYS_AES_FIPS_MODE_FAIL_ERROR;
	}
	/* no check if the self test was already executed */
	if( !(DX_GLOBAL_FIPS_SF_STATUS & CRYS_SELF_TEST_AES_BIT) )
	{
		return CRYS_AES_FIPS_MODE_SF_NOT_PERFORMED_ERROR;
	}
	/* check for CBC-MAC - not allowed by FIPS */
	if(OperationMode == CRYS_AES_MAC_mode)
	{
		return CRYS_AES_FIPS_MODE_NOT_ALLOWED_ERROR;
	}
   }
#endif

  /* in case is_secret_key == DX_FALSE set KeySizeBytes value and copy AES key into Context */
   switch( KeySizeID )
   {
		case CRYS_AES_Key128BitSize:
			KeySizeBytes = 16;
			break;

		case CRYS_AES_Key192BitSize:
			KeySizeBytes = 24;
			break;

		case CRYS_AES_Key256BitSize:
			KeySizeBytes = 32;
			break;

		case CRYS_AES_Key512BitSize:

		    if( OperationMode != CRYS_AES_XTS_mode )
			return CRYS_AES_ILLEGAL_KEY_SIZE_ERROR;

			KeySizeBytes = 64;
			break;

		default:
			Error = CRYS_AES_ILLEGAL_KEY_SIZE_ERROR; /*for preventing compiler warnings*/
			goto End;
   }

   /* ................. acquiring the AES context ........................... */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,                      /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,   /* the CCM context returned - out */
                                DX_AES_CONTEXT,                     /* AES type - in */
                                AES_DONT_DECRYPT_CONTEXT);		/*No need to decrypt context in Init*/
  if( Error != CRYS_OK )

      return Error;


   /* set AES input parameters into context */
   ccmWorkingContext_ptr->AESContextKeySize  = KeySizeID;
   ccmWorkingContext_ptr->EncryptDecryptFlag = EncryptDecryptFlag;
   ccmWorkingContext_ptr->OperationMode      = OperationMode;
   ccmWorkingContext_ptr->is_secret_key      = is_secret_key;
   ccmWorkingContext_ptr->lastBlockSize      = 0x0;
   ccmWorkingContext_ptr->remainSize         = 0x0;

   /* set operation mode also in non secured part of context */
   ContextID_ptr->OperationMode = OperationMode;
   /* set the AES tag to the users context */
   ContextID_ptr->valid_tag = AES_CONTEXT_VALIDATION_TAG;


   /* ----------------------------------------------------------------------- */
   /*                     calling the low level init functions                */
   /*                       for specific modes modes                          */
   /* ----------------------------------------------------------------------- */

   switch( OperationMode )
   {
		case CRYS_AES_ECB_mode:
		case CRYS_AES_CBC_mode:
		case CRYS_AES_CTR_mode:
		case CRYS_AES_MAC_mode:

			Error =  CRYS_AES_EcbCbcCtrMac_Init(
						           ccmWorkingContext_ptr,
                                   IVCounter_ptr,
                                   Key_ptr,
                                   KeySizeBytes,
                                   EncryptDecryptFlag,
                                   OperationMode,
                                   is_secret_key );
           break;

#ifndef  CRYS_NO_AES_XCBC_CMAC_MODES_SUPPORT

		case CRYS_AES_XCBC_MAC_mode:
		case CRYS_AES_CMAC_mode:

			Error =  CRYS_AES_XcbcCmac_Init(
						           ccmWorkingContext_ptr,
                                   IVCounter_ptr,
                                   Key_ptr,
                                   KeySizeBytes,
                                   EncryptDecryptFlag,
                                   OperationMode,
                                   is_secret_key );

		   /* set NotAddBlocks = 0xFFFFFFFF, means AES_Init is performed, any data is not processed
		      previously (needed for XCBC and CMAC modes) */
		   ccmWorkingContext_ptr->NotAddBlocks = 0xFFFFFFFF;

           break;
#endif

#ifndef  CRYS_NO_AES_XTS_MODE_SUPPORT
		case CRYS_AES_XTS_mode:

			Error =  CRYS_AES_Xts_Init(
						           ccmWorkingContext_ptr,
                                   IVCounter_ptr,
                                   Key_ptr,
                                   KeySizeBytes,
                                   EncryptDecryptFlag,
                                   OperationMode,
                                   is_secret_key );
           break;
#endif

		default:
			return  CRYS_AES_ILLEGAL_OPERATION_MODE_ERROR;
   }

 End:

   /* ................. release the context ................................. */

   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,       /* the users context space - in */
                                   ccmWorkingContext_ptr,  /* the CCM context returned - in */
                                   DX_AES_CONTEXT);        /* AES type - in */


   /* ...................... end of function ................................ */

   /*if the error occurred before release context do not override it*/
   if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
		Error = ErrorCcm;

   if( Error != CRYS_OK )
       /* clearing the users context */
	   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_AESUserContext_t) );

   return Error;

   #endif /* !CRYS_NO_AES_SUPPORT */

}/* END OF _DX_AES_Init */


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
 *          low level AES function LLF_AES_Block.
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
 * @param[in] DataInSize - A size of the input data must be multiple of 16 bytes and not 0,
 *                         on all modes. Note last chunk (block) of data must be processed by
 *                         CRYS_AES_Finish function and not by CRYS_AES_Block function;
 *
 * @param[out] DataOut_ptr - A pointer to the buffer of the output data from the AES. The pointer  does not
 *                             need to be aligned. On CSI output mode the pointer must be equal to
 *                             value (0xFFFFFFFC | DataOutAlignment). On all MAC modes (MAC,XCBC, CMAC) CSI
 *                             output is not allowed. In case of an error, the output buffer is being zeroed
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_AES_error.h
 *
 *     NOTES: 1. Temporarily not allowed, that both the Input and the Output simultaneously
 *               were on CSI mode.
 *            2. Temporarily the CSI input or output are not allowed on XCBC, CMAC and XTS modes.
 */
/*CEXPORT_C CRYSError_t  CRYS_AES_Block( CRYS_AESUserContext_t   *ContextID_ptr,
                                        DxUint8_t               *DataIn_ptr,
                                        DxUint32_t               DataInSize,
                                        DxUint8_t               *DataOut_ptr )
{ */
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
//	CRYSError_t Error;
//	CRYSError_t ErrorCcm;

	/* defining a pointer to the active context allocated by the CCM */
//	AESContext_t *ccmWorkingContext_ptr;

	/* Data out size - temp variable */
//	DxUint32_t DataOutSize;


	/******************* FUNCTION LOGIC *************************************/

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
//	Error = CRYS_OK;

	/* ............... if not supported exit .............................. */
	/* -------------------------------------------------------------------- */

//	RETURN_IF_AES_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize ,
  //                            DataOut_ptr , ccmWorkingContext_ptr , Error , Error , Error,
    //                          ErrorCcm , DataOutSize , Error , Error , Error ,
      //                        Error , Error , Error );

//#ifndef CRYS_NO_AES_SUPPORT

	/* ............... checking the parameters validity ................... */
	/* -------------------------------------------------------------------- */

	/* if the users context ID pointer is DX_NULL return an error */
//	if( ContextID_ptr == DX_NULL )

  //    return CRYS_AES_INVALID_USER_CONTEXT_POINTER_ERROR;

	/* if the users context TAG is illegal return an error - the context is invalid */
//	if( ContextID_ptr->valid_tag != AES_CONTEXT_VALIDATION_TAG )

  //    return CRYS_AES_USER_CONTEXT_CORRUPTED_ERROR;

	/* if the users Data In pointer is illegal return an error */
//	if( DataIn_ptr == DX_NULL )

 //     return CRYS_AES_DATA_IN_POINTER_INVALID_ERROR;

	/* if the DataInSize = 0 or is not multiple of 16 bytes, then return an error */
//	if( DataInSize == 0 ||
//	    DataInSize % CRYS_AES_BLOCK_SIZE_IN_BYTES != 0 )

  //    return CRYS_AES_DATA_IN_SIZE_ILLEGAL;

	/* Temporarily is not allowed, that both the Input and Output simultaneously
		are on CSI mode */
//	if( ((DxUint32_t)DataIn_ptr >= DX_CSI) && ((DxUint32_t)DataOut_ptr >= DX_CSI) )

  //	  return CRYS_AES_CSI_IN_OUT_ILLEGAL_MODE_ERROR;


	/* on MAC, CMAC,XCBC and XTS modes check, that output is not on CSI mode */
//	if( ( ContextID_ptr->OperationMode == CRYS_AES_XTS_mode      ||
  //       ContextID_ptr->OperationMode == CRYS_AES_CMAC_mode      ||
    //     ContextID_ptr->OperationMode == CRYS_AES_XCBC_MAC_mode ) &&
      //   ( (DxUint32_t)DataIn_ptr >= DX_CSI || (DxUint32_t)DataOut_ptr >= DX_CSI ) )

	//   return CRYS_AES_CSI_IN_OUT_ILLEGAL_MODE_ERROR;


	/* ................. acquiring the AES contest ........................... */
	/* ----------------------------------------------------------------------- */

	//Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
          //                      (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
              //                  DX_AES_CONTEXT,                       /* AES type - in */
            //                    AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/

	//if( Error != CRYS_OK )

     //return Error;


   /* ...... checking the data size and data in / out overlapping ........... */
   /* ----------------------------------------------------------------------- */

	/* checking data pointers and overlapping (besides the XCBC and CMAC modes) */
//	if( ContextID_ptr->OperationMode != CRYS_AES_XCBC_MAC_mode &&
//			ContextID_ptr->OperationMode != CRYS_AES_CMAC_mode  )
//	{
		/* if the users DataOut pointer is illegal return an error */
//		if( DataOut_ptr == DX_NULL )
//		{
//			Error = CRYS_AES_DATA_OUT_POINTER_INVALID_ERROR;
//			goto End;
//		}

		/* on non CSI mode: check that there is no overlapping between the data input and data output buffers,
			except the inplace case that is legal */
//		if( (DxUint32_t)DataIn_ptr < DX_CSI && (DxUint32_t)DataOut_ptr < DX_CSI &&
//	       DataIn_ptr != DataOut_ptr )
//		{
			/* checking the case that the input buffer is in a higher address then the output buffer */
//			if ( DataIn_ptr > DataOut_ptr )
//			{
				/* on all MAC modes the data out size is as the IV size, else it is equal DataInSize */
//				if( ContextID_ptr->OperationMode == CRYS_AES_MAC_mode )
//				{
//					DataOutSize = CRYS_AES_IV_COUNTER_SIZE_IN_BYTES;
//				}

//				else
//					DataOutSize = DataInSize;  /* initialize the data out size as the data in size  */

				/* if after adding the full DataInSize to the data out pointer it is larger then the
				data in pointer, return the overlap error */
//				if( DataOut_ptr + DataOutSize > DataIn_ptr )
//				{
//					Error = CRYS_AES_DATA_OUT_DATA_IN_OVERLAP_ERROR;
//					goto End;
//				}
//			}

			/* checking the case that the output buffer is in a higher address than the input buffer */
//			else
//			{
				/* if after adding the full DataInSize to the data in pointer it is larger than the
				data out pointer, return the overlap error */
//				if( DataIn_ptr + DataInSize > DataOut_ptr )
//				{
//					Error = CRYS_AES_DATA_OUT_DATA_IN_OVERLAP_ERROR;
//					goto End;
//				}
//			}
//		}/* end of DataIn_ptr != DataOut_ptr case */
//	}


	/* ----------------------------------------------------------------------- */
	/*    perform Block operation on LLF                                       */
	/* ----------------------------------------------------------------------- */
//	ccmWorkingContext_ptr->remainSize = DataInSize ;
//	ccmWorkingContext_ptr->NotAddBlocks = 0; /* means that this operation is AES_Block */

//	Error = LLF_AES_Block(
//	                      ccmWorkingContext_ptr,      /* the working context - in */
//						  DataIn_ptr,                 /* the input data buffer - in */
//						  DataInSize,                 /* the data in size - in */
//						  DataOut_ptr );              /* the output data buffer i/o */

//	if( Error != CRYS_OK )
//		goto End_Clear_Output;


//	goto End;


	/* ................. release the context ................................. */
	/* ----------------------------------------------------------------------- */

//End_Clear_Output:
	/* clear the output buffer */
//	if(DataOut_ptr != DX_NULL)
//		DX_VOS_MemSetZero(DataOut_ptr , DataInSize);

	/* clear the user context */
//	DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_AESUserContext_t) );

//End:

//	ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,		   /* the users context space - in */
  //                                    ccmWorkingContext_ptr,   /* the CCM context returned - in */
    //                                  DX_AES_CONTEXT);         /* AES type - in */

	/*if the error occurred before release context do not override it*/
//	if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
//	{
//		Error = ErrorCcm;
//	}
	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

//	return Error;

//#endif /* !CRYS_NO_AES_SUPPORT */

//}/* END OF CRYS_AES_Block */



/****************************************************************************************************/
/**
 * @brief This function is used as finish operation on all AES modes.
 *
 *        The function must be called after AES_Block operations (or instead) for last chunk
 *        of data with size > 0.
 *
 *        The function performs all operations, including specific operations for last blocks of
 *        data on some modes (XCBC, CMAC, MAC) and puts out the result. After all operations
 *        the function cleans the secure sensitive data from context.
 *
 *        1. Checks the validation of all of the inputs of the function.
 *           If one of the received parameters is not valid it shall return an error.
 *        2. Decrypts the received context to the working context  by calling the
 *           CRYS_CCM_GetContext function.
 *        3. Calls the LLF_AES_Finish function.
 *        4. Outputs the result and cleans working context.
 *        5. Exits
 *
 *
 * @param[in] ContextID_ptr - A pointer to the AES context buffer allocated by the user that
 *                            should be the same context that was used on the previous call
 *                            of this session.
 * @param[in] DataIn_ptr    - A pointer to the buffer of the input data to the AES. The pointer does
 *                            not need to be aligned. On CSI input mode the pointer must be equal to
 *                            value (0xFFFFFFFC | DataInAlignment).
 * @param[in] DataInSize    - A size of the input data must be:  DataInSize >= minimalSize, where:
 *                            minimalSize =
 *                                  -  1 byte for CTR, XCBC, CMAC mode;
 *                                  - 16 bytes for other modes.
 *                            On ECB,CBC and MAC modes the data size must be also a multiple of 16 bytes.
 * @param[out] DataOut_ptr  - A pointer to the output buffer. The pointer  does not need to be aligned.
 *                            On CSI output mode the pointer must be equal to value
 *                            (0xFFFFFFFC | DataOutAlignment). On some modes (MAC,XCBC,CMAC,XTS)
 *                            CSI output is not allowed. Temporarily is not allowed, that both the
 *                            Input and the output are on CSI mode simultaneously.
 *                            The size of the output buffer must be not less than:
 *                                - 16 bytes for MAC, XCBC, CMAC modes;
 *                                - DataInSize for ECB,CBC,CTR,XTS modes.
 *
 * @return CRYSError_t    - On success CRYS_OK is returned, on failure - a value defined in CRYS_AES_error.h.
 *
 */
 CEXPORT_C CRYSError_t  CRYS_AES_Finish(
                                        CRYS_AESUserContext_t   *ContextID_ptr,
                                        DxUint8_t               *DataIn_ptr,
                                        DxUint32_t               DataInSize,
                                        DxUint8_t               *DataOut_ptr )
 {
   /* FUNCTION DECLERATIONS */

   /* The return error identifiers */
   CRYSError_t Error, ErrorCcm;

   /* defining a pointer to the active context allocated by the CCM */
   AESContext_t *ccmWorkingContext_ptr;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AES_UNSUPPORTED( ContextID_ptr , DataIn_ptr , DataInSize , DataOut_ptr, Error ,
                              ErrorCcm , ccmWorkingContext_ptr , Error , Error ,
							  Error , Error , Error , Error , Error , Error , Error );


 #ifndef CRYS_NO_AES_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if the users context ID pointer is DX_NULL return an error */
   if( ContextID_ptr == DX_NULL )
      return CRYS_AES_INVALID_USER_CONTEXT_POINTER_ERROR;

   /* if the users context TAG is illegal return an error - the context is invalid */
   if( ContextID_ptr->valid_tag != AES_CONTEXT_VALIDATION_TAG )
      return CRYS_AES_USER_CONTEXT_CORRUPTED_ERROR;

   /* if the users DataIn pointer is illegal return an error, besides CMAC and
      XCBC-MAC modes, which should be checked separately on LLF Level*/
   if( ContextID_ptr->OperationMode != CRYS_AES_CMAC_mode &&
       ContextID_ptr->OperationMode != CRYS_AES_XCBC_MAC_mode  &&
       DataIn_ptr == DX_NULL )
      return CRYS_AES_DATA_IN_POINTER_INVALID_ERROR;

   /* if the users DataOut pointer is illegal return an error */
   if( DataOut_ptr == DX_NULL )
      return CRYS_AES_DATA_OUT_POINTER_INVALID_ERROR;

   /* check, that data size is multiple of 16 bytes on relevant modes */
   if( ContextID_ptr->OperationMode == CRYS_AES_ECB_mode ||
       ContextID_ptr->OperationMode == CRYS_AES_CBC_mode ||
       ContextID_ptr->OperationMode == CRYS_AES_MAC_mode  )
   {
	     if( DataInSize %  CRYS_AES_BLOCK_SIZE_IN_BYTES != 0 )
	          return CRYS_AES_DATA_IN_SIZE_ILLEGAL;
   }

   /* check the minimum data size according to mode */
   if( ContextID_ptr->OperationMode == CRYS_AES_ECB_mode ||
       ContextID_ptr->OperationMode == CRYS_AES_CBC_mode ||
       ContextID_ptr->OperationMode == CRYS_AES_MAC_mode ||
       ContextID_ptr->OperationMode == CRYS_AES_XTS_mode  )
   {
	     if( DataInSize <  CRYS_AES_BLOCK_SIZE_IN_BYTES )
	          return CRYS_AES_DATA_IN_SIZE_ILLEGAL;
   }

   /* on MAC, CMAC,XCBC and XTS modes check, that input or output is not on CSI mode */
   if( ( ContextID_ptr->OperationMode == CRYS_AES_XCBC_MAC_mode  ||
         ContextID_ptr->OperationMode == CRYS_AES_CMAC_mode      ||
         ContextID_ptr->OperationMode == CRYS_AES_XTS_mode  ) &&
         ( (DxUint32_t)DataIn_ptr >= DX_CSI || (DxUint32_t)DataOut_ptr >= DX_CSI ) )

	   return CRYS_AES_CSI_IN_OUT_ILLEGAL_MODE_ERROR;


   /* For other AES modes temporarily it is not allowed, that both the
      Input and Output simultaneously are on CSI mode */
   else if( ((DxUint32_t)DataIn_ptr >= DX_CSI) && ((DxUint32_t)DataOut_ptr >= DX_CSI) )

	  return CRYS_AES_CSI_IN_OUT_ILLEGAL_MODE_ERROR;


   /* ................. acquiring the AES contest ............................ */
   /* ----------------------------------------------------------------------- */

   Error = CRYS_CCM_GetContext( ContextID_ptr,					 /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,    /* the CCM context returned - out */
                                DX_AES_CONTEXT,                      /* AES type - in */
                                AES_DECRYPT_CONTEXT);				 /* need to decrypt context in AES_block*/
   if( Error != CRYS_OK )

     return Error;

   /*  call the low level finish function */
   Error = LLF_AES_Finish( ccmWorkingContext_ptr,
                           DataIn_ptr,
                           DataInSize,
                           DataOut_ptr );


   /* ................. release the context ................................. */
   /* ----------------------------------------------------------------------- */

   ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,		   /* the users context space - in */
                                      ccmWorkingContext_ptr,   /* the CCM context returned - in */
                                      DX_AES_CONTEXT);         /* AES type - in */

   /*if the error occurred before release context do not override it*/
   if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
		Error = ErrorCcm;

   /*  set 0 to the context buffer */
   DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_AESUserContext_t) );

   /* in case of an error - clear the output if exists */
   if(Error && DataOut_ptr != DX_NULL)
   {
	/* clear the out put if exists */
	DX_VOS_MemSetZero(DataOut_ptr , DataInSize);
   }


   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   return Error;

 #endif /* !CRYS_NO_AES_SUPPORT */

}/* END OF CRYS_AES_Finish */


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
 *                            - on ECB, MAC, XCBC, CMAC modes this parameter is not used and may be NULL,
 *                            - on CBC mode it contains the IV value,
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
 *                           need to be aligned. On CSI output mode the pointer must be equal to
 *                           value (0xFFFFFFFC | DataOutAlignment). On all MAC modes (MAC,XCBC, CMAC) CSI
 *                           output is not allowed.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in CRYS_AES_error.h
 *
 *     NOTES: 1. Temporarily it is not allowed, that both the Input and the Output simultaneously
 *               were on CSI mode.
 *            2. Temporarily the CSI input or output are not allowed on XCBC, CMAC and XTS modes.
 *
 */
CEXPORT_C CRYSError_t  _DX_AES( CRYS_AES_IvCounter_t       IVCounter_ptr,
		                        CRYS_AES_Key_t             Key_ptr,
		                        CRYS_AES_KeySize_t         KeySize,
		                        CRYS_AES_EncryptMode_t     EncryptDecryptFlag,
		                        CRYS_AES_OperationMode_t   OperationMode ,
		                        DxUint8_t                  is_secret_key,
		                        DxUint8_t                  *DataIn_ptr,
		                        DxUint32_t                 DataInSize,
		                        DxUint8_t                  *DataOut_ptr )
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* a users context used to pass to all of the CRYS functions */
   CRYS_AESUserContext_t  ContextID;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   Error = CRYS_OK;


   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AES_UNSUPPORTED( IVCounter_ptr , Key_ptr , KeySize ,
                              EncryptDecryptFlag , OperationMode , is_secret_key ,
                              DataIn_ptr , DataInSize , DataOut_ptr , ContextID.valid_tag ,
                              Error , Error , Error , Error , Error , Error );

   #ifndef CRYS_NO_AES_SUPPORT


   /* calculate remaining data size for processing in Finish functions */


   /* ............... calling the CRYS init function ...................... */
   /* --------------------------------------------------------------------- */

   Error = _DX_AES_Init( &ContextID,
                         IVCounter_ptr,
                         Key_ptr,
						 KeySize,
                         EncryptDecryptFlag,
                         OperationMode,
                         is_secret_key);

   if( Error != CRYS_OK )
     goto End;

   /* ............... calling the CRYS BLOCK function ..................... */
   /* --------------------------------------------------------------------- */

      Error = CRYS_AES_Finish( &ContextID,
							   DataIn_ptr,
							   DataInSize,
							   DataOut_ptr );


   /* ............. end of function on other AES modes .................... */
   /* --------------------------------------------------------------------- */
 End:

   return Error;

   #endif /* !CRYS_NO_AES_SUPPORT  */

}/* END OF _DX_AES */



/****************************************************************************************************/
/****************************************************************************************************/
/*                                     PRIVATE FUNCTIONS                                            */
/****************************************************************************************************/
/****************************************************************************************************/



/****************************************************************************************************/
/**
 * @brief Performs specific initialization operations for 4 modes ECB,CBC,CTR,MAC.
 *
 **/
static CRYSError_t  CRYS_AES_EcbCbcCtrMac_Init(
						   AESContext_t            *AESContext_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           DxUint32_t               KeySizeBytes,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key )
{

   /* FUNCTION LOGIC */

   CRYSError_t Error = CRYS_OK;

   /*        Check input parameters     */
   /*-----------------------------------*/

   /* check the max key size */
   if( AESContext_ptr->AESContextKeySize > CRYS_AES_Key256BitSize )
      return  CRYS_AES_ILLEGAL_KEY_SIZE_ERROR;


   /* in MAC, CTR modes enable only encrypt mode  */
   if( ( OperationMode == CRYS_AES_MAC_mode || OperationMode == CRYS_AES_CTR_mode) &&
        EncryptDecryptFlag != CRYS_AES_Encrypt )

      return  CRYS_AES_DECRYPTION_NOT_ALLOWED_ON_THIS_MODE;

   /* if the operation mode selected is CBC, MAC, CTR then check the IV counter pointer */
   if( (OperationMode == CRYS_AES_CBC_mode  ||
        OperationMode == CRYS_AES_MAC_mode  ||
        OperationMode == CRYS_AES_CTR_mode  ) &&
        IVCounter_ptr == DX_NULL )
   {
      return  CRYS_AES_INVALID_IV_OR_TWEAK_PTR_ERROR;
   }

   /* ................. loading the context data ............................ */
   /* ----------------------------------------------------------------------- */

   /* load IV */
   if( OperationMode != CRYS_AES_ECB_mode)
      DX_VOS_FastMemCpy( AESContext_ptr->AESContextIV, IVCounter_ptr, sizeof(CRYS_AES_IvCounter_t) );

   /* set Key into AESContextKey */
   if( is_secret_key == DX_FALSE )
	   DX_VOS_FastMemCpy( AESContext_ptr->AESContextKey, Key_ptr, KeySizeBytes );

   /* init keys on LLF level  */
   Error = LLF_AES_InitInit( AESContext_ptr );

   /*  end of function  */

   return Error;

}/* END OF CRYS_AES_EcbCbcCtrMac_Init */


#ifndef CRYS_NO_AES_XCBC_CMAC_MODES_SUPPORT
/****************************************************************************************************/
/**
 * @brief Performs specific initialization operations for XCBC and CMAC modes.
 *
 *  Author: R.Levin.
 *
 * @param[in] AESContext_ptr - A pointer to the AES context buffer inside the the user context.
 * @param[in] IVCounter_ptr  - A buffer containing an initial value: IV, Counter or Tweak according
 *                             to operation mode:
 * @param[in] Key_ptr        - A pointer to the user's key buffer.
 * @param[in] KeySizeBytes   - The size of used key. On XCBC mode allowed 128 bit size only.
 * @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0)
 *                                 or a Decrypt operation (1). In XCBC and CMAC modes it must be Encrypt.
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, CTR, XCBC (PRF and 96), CMAC.
 * @param[in] is_secret_key - This parameter intended for internal Discretix using only !.
 *                            The parameter specifying whether to use a secret key (1)
 *                            or to use a key defined by the user (0).
 *                            The secret key function depends on product configuration.
 *                            Please consult the reference manual.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_AES_error.h
 **/
static CRYSError_t  CRYS_AES_XcbcCmac_Init(
						   AESContext_t            *AESContext_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           DxUint32_t               KeySizeBytes,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key )
{
   /* The return error identifier */
   CRYSError_t Error;

   /* FUNCTION LOGIC */


   /*        Check input parameters     */
   /*-----------------------------------*/

   /* check the max key size */
   if( AESContext_ptr->AESContextKeySize > CRYS_AES_Key256BitSize )
      return  CRYS_AES_ILLEGAL_KEY_SIZE_ERROR;

   /* in XCBC mode enabled only key size = 128 bit */
   if( OperationMode == CRYS_AES_XCBC_MAC_mode &&
       AESContext_ptr->AESContextKeySize != CRYS_AES_Key128BitSize )
      return  CRYS_AES_NOT_ALLOWED_KEY_TYPE_ON_THIS_MODE;

   /* in XCBC,CMAC modes enable only encrypt mode  */
   if( EncryptDecryptFlag != CRYS_AES_Encrypt )
      return  CRYS_AES_DECRYPTION_NOT_ALLOWED_ON_THIS_MODE;



   /* ................. loading the context data ............................ */
   /* ----------------------------------------------------------------------- */

   /* set IV value = 0 ) */
   DX_VOS_MemSetZero( AESContext_ptr->AESContextIV, sizeof(CRYS_AES_IvCounter_t) );

   /* set Key into AESContextKey */
   if( is_secret_key == DX_FALSE )
	   DX_VOS_FastMemCpy( AESContext_ptr->AESContextKey, Key_ptr, KeySizeBytes );

   /* init keys on LLF level  */
   Error = LLF_AES_InitInit( AESContext_ptr );

   if( Error != CRYS_OK )
     goto End;

   /* set 0 into context LLF lastBlock  */
   DX_VOS_MemSetZero( &AESContext_ptr->lastBlock, CRYS_AES_BLOCK_SIZE_IN_BYTES );
   /* Set 0 to ContextIV and previousIV for next operations */
   DX_VOS_MemSetZero( AESContext_ptr->AESContextIV, CRYS_AES_IV_COUNTER_SIZE_IN_BYTES );
   DX_VOS_MemSetZero( AESContext_ptr->previousIV, CRYS_AES_IV_COUNTER_SIZE_IN_BYTES );



   /*  end of function  */
End:
   return CRYS_OK;


}/* END OF CRYS_AES_XcbcCmac_Init */

#endif  /* CRYS_NO_AES_XCBC_CMAC_MODES_SUPPORT */



#ifndef CRYS_NO_AES_XTS_MODE_SUPPORT

/****************************************************************************************************/
/**
 * @brief Performs specific initialization operations for XTS mode.
 *
 *  Author: R.Levin.
 *
 * @param[in] AESContext_ptr - A pointer to the AES context buffer inside the the user context.
 * @param[in] IVCounter_ptr  - A buffer containing an initial value: IV, Counter or Tweak according
 *                             to operation mode:
 * @param[in] Key_ptr        - A pointer to the user's key buffer.
 * @param[in] KeySizeBytes   - The size of used key (128, 192, 256, 512 bits).
 *                             On XCBC mode allowed 128 bit size only, on XTS - 256 or 512 bit, on other modes <= 256 bit.
 * @param[in] EncryptDecryptFlag - A flag specifying whether the AES should perform an Encrypt operation (0)
 *                                 or a Decrypt operation (1). In XCBC and CMAC modes it must be Encrypt.
 * @param[in] OperationMode - The operation mode: ECB, CBC, MAC, CTR, XCBC (PRF and 96), CMAC.
 * @param[in] is_secret_key - This parameter intended for internal Discretix using only !.
 *                            The parameter specifying whether to use a secret key (1)
 *                            or to use a key defined by the user (0).
 *                            The secret key function depends on product configuration.
 *                            Please consult the reference manual.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_AES_error.h
 **/
static CRYSError_t  CRYS_AES_Xts_Init(
						   AESContext_t            *AESContext_ptr,
                           CRYS_AES_IvCounter_t     IVCounter_ptr,
                           CRYS_AES_Key_t           Key_ptr,
                           DxUint32_t               KeySizeBytes,
                           CRYS_AES_EncryptMode_t   EncryptDecryptFlag,
                           CRYS_AES_OperationMode_t OperationMode,
                           DxUint8_t                is_secret_key )
{

   /* The return error identifier */
   CRYSError_t Error;


   /* FUNCTION LOGIC */

   /*        Check input parameters     */
   /*-----------------------------------*/

   /* check the IV counter pointer  */
   if( IVCounter_ptr == DX_NULL )
      return  CRYS_AES_INVALID_IV_OR_TWEAK_PTR_ERROR;

   /* check key size in XTS mode  */
   if( AESContext_ptr->AESContextKeySize != CRYS_AES_Key256BitSize &&
       AESContext_ptr->AESContextKeySize != CRYS_AES_Key512BitSize )
         return  CRYS_AES_ILLEGAL_KEY_SIZE_ERROR;

   /* secret key is not supported */
   if( is_secret_key != DX_FALSE )
		 return CRYS_AES_ILLEGAL_SECRET_KEY_MODE;


   /* ................. loading the context data ............................ */
   /* ----------------------------------------------------------------------- */

   /* load Tweak value, note: on XTS mode the Tweak is set in IV buffer */
   DX_VOS_FastMemCpy( AESContext_ptr->AESContextIV, IVCounter_ptr, sizeof(CRYS_AES_IvCounter_t) );


   /* ----------------------------------------------------------------------- */
   /*                     calling the low level init function                 */
   /*              (keys calculation on XCBC,CMAC and XTS modes)              */
   /* ----------------------------------------------------------------------- */

   /* copy Tweak Key (used for encryption tweak) to the context */
   DX_VOS_FastMemCpy( (void*)AESContext_ptr->AESContextKey, (void*)(Key_ptr + KeySizeBytes / 2), KeySizeBytes / 2 );

   /* copy Data-Key for encryption data to the context buffer Key 2 */
   DX_VOS_FastMemCpy( (void*)AESContext_ptr->SpecificData.XTS_Data.AES_XTS_Key2,
                       (void*)Key_ptr, KeySizeBytes / 2 );

   Error = LLF_AES_InitInit( AESContext_ptr );

   /*  end of function  */

   return Error;


}/* END OF CRYS_AES_Xts_Init */

#endif /* CRYS_NO_AES_XTS_MODE_SUPPORT */
