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
/****************************************************************************************************/
/*                                         Public Functions                                         */
/****************************************************************************************************/
/****************************************************************************************************/


/****************************************************************************************************/
/**
 * @brief This function is used to initialize the AES machine and SW structures for the CMAC mode with
 *        RKEK2 (secret key 2) only.
 *        To perform the AES operations this should be the first function called.
 *
 *
 *        The function executes the following major steps:
 *
 *        1. Validates all of the inputs of the function. If one of the received
 *           parameters is not valid it shall return an error:
 *        2. Decrypts the received context to the working context after capturing
 *           the working context by calling the CRYS_CCM_GetContext() call.
 *        3. Initializes the working context by the parameters used in other AES operations.
 *        4. Encrypts the working context and stores it to the users received context.
 *           Releases the working context by calling the CRYS_CCM_EncryptAndReleaseContext call.
 *        5. Exits the handler with the OK code.
 *
 *
 * @param[in] ContextID_ptr - A pointer to the AES context buffer that is allocated by the user
 *                            and is used for the AES machine operation.
 * @param[in] KeySizeID  -  An enum parameter, defines size of used key (128, 192, 256, 512 bits):
 *                          On XCBC mode allowed 128 bit size only, on XTS - 256 or 512 bit, on other modes <= 256 bit.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned, and on failure - a value from CRYS_AES_error.h
 */

CEXPORT_C CRYSError_t  CRYS_AES_CMAC_RKEK2_Init(CRYS_AESUserContext_t   *ContextID_ptr,
																								CRYS_AES_KeySize_t       KeySizeID)
{
	/* FUNCTION DECLARATIONS */

	/* The return error identifiers */
	CRYSError_t Error, ErrorCcm;

	/* defining a pointer to the active context allocated by the CCM */
	AESContext_t *ccmWorkingContext_ptr;

	/* FUNCTION LOGIC */

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* ............... checking the parameters validity ................... */
	/* -------------------------------------------------------------------- */

	/* if the users context ID pointer is DX_NULL return an error */
	if( ContextID_ptr == DX_NULL )
	{
		Error = CRYS_AES_INVALID_USER_CONTEXT_POINTER_ERROR;
		goto end_function;
	}

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
   }
#endif

	/* init the valid_tag field to avoid warnings*/
	ContextID_ptr->valid_tag = 0;

	/* ................. acquiring the AES context ........................... */
	/* ----------------------------------------------------------------------- */

	Error = CRYS_CCM_GetContext( ContextID_ptr,                      /* the users context space - in */
                                (void **) &ccmWorkingContext_ptr,   /* the CCM context returned - out */
                                DX_AES_CONTEXT,                     /* AES type - in */
                                AES_DONT_DECRYPT_CONTEXT);		/*No need to decrypt context in Init*/
	if( Error != CRYS_OK )
	{
		goto end_function;
	}

	/* set AES input parameters into context */
	ccmWorkingContext_ptr->AESContextKeySize  = KeySizeID;
  ccmWorkingContext_ptr->EncryptDecryptFlag = CRYS_AES_Encrypt;
	ccmWorkingContext_ptr->OperationMode      = CRYS_AES_CMAC_mode;
	ccmWorkingContext_ptr->is_secret_key      = CRYS_AES_IS_SECRET_KEY_RKEK2;

	/* set operation mode also in non secured part of context */
	ContextID_ptr->OperationMode = CRYS_AES_CMAC_mode;
	/* set the AES tag to the users context */
	ContextID_ptr->valid_tag = AES_CONTEXT_VALIDATION_TAG;
	DX_VOS_MemSetZero( ccmWorkingContext_ptr->AESContextIV, sizeof(CRYS_AES_IvCounter_t) );
	/* set 0 into context LLF lastBlock */
	DX_VOS_MemSetZero( (void*)&ccmWorkingContext_ptr->lastBlock, CRYS_AES_BLOCK_SIZE_IN_BYTES );


	/* ................. release the context ................................. */

	ErrorCcm = CRYS_CCM_ReleaseContext(ContextID_ptr,
																		 ccmWorkingContext_ptr,  /* the CCM context returned - in */
																		 DX_AES_CONTEXT);        /* AES type - in */


	/* ...................... end of function ................................ */

	/*if the error occurred before release context do not override it*/
	if((ErrorCcm != CRYS_OK) && (Error == CRYS_OK))
	{
		Error = ErrorCcm;
	}

	if( Error != CRYS_OK )
	{
		/* clearing the users context */
		DX_VOS_MemSet( ContextID_ptr , 0 , sizeof(CRYS_AESUserContext_t) );
	}

 end_function:

   return Error;

}/* END OF CRYS_AES_CMAC_RKEK2_Init */



/****************************************************************************************************/
/**
 * @brief This function is used to perform the AES operation for CMAC mode with RKEK2 in one integrated process.
 *
 *        The actual macros, that will be used by the user for calling this function, are described
 *        in CRYS_AES.h file.
 *
 *        The input-output parameters of the function are the following:
 *
 * @param[in] ContextID_ptr - A pointer to the AES context buffer that is allocated by the user
 *                            and is used for the AES machine operation.
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
CEXPORT_C CRYSError_t  CRYS_AES_CMAC_RKEK2(CRYS_AES_KeySize_t				KeySize,
																					 DxUint8_t*								DataIn_ptr,
																					 DxUint32_t								DataInSize,
																					 DxUint8_t*								DataOut_ptr )
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

	/* ............... calling the CRYS init function ...................... */
	/* --------------------------------------------------------------------- */

	Error = CRYS_AES_CMAC_RKEK2_Init(&ContextID,
																	KeySize);

	if( Error != CRYS_OK )
		goto End;

	/* ............... calling the CRYS BLOCK function ..................... */
	/* --------------------------------------------------------------------- */

	Error = CRYS_AES_Finish(&ContextID,
													DataIn_ptr,
													DataInSize,
													DataOut_ptr );


   /* ............. end of function on other AES modes .................... */
   /* --------------------------------------------------------------------- */
End:

	return Error;

}/* END OF _DX_AES */
