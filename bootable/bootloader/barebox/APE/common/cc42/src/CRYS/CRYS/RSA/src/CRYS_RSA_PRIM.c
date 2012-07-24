/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_RSA_error.h"
#include "CRYS_RSA_Local.h"
#include "LLF_PKI_RSA.h"
#include "PLAT_SystemDep.h"
/************************ Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/



/************************ Public Functions ******************************/

#if !defined(_INTERNAL_CRYS_NO_RSA_ENCRYPT_SUPPORT) && !defined(_INTERNALCRYS_NO_RSA_VERIFY_SUPPORT)
/**
@brief
CRYS_RSA_PRIM_Encrypt implements the RSAEP algorithm as defined in PKCS#1 v2.1 6.1.1

  @param[in] UserPubKey_ptr - Pointer to the public key data structure.
  @param[in] PrimeData_ptr - a pointer to a structure containing internal buffers
  @param[in] Data_ptr - Pointer to the data to encrypt.
  @param[in] DataSize - The size, in bytes, of the data to
                        encrypt. \note Must be <= the size of the
                        modulus.
  @param[out] Output_ptr - Pointer to the encrypted data, the buffer
                           must be at least PubKey_ptr->N.len bytes
                           long (that is, the size of the modulus, in
                           bytes).

  @return CRYSError_t - CRYS_OK,
                        CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                        CRYS_RSA_PUB_KEY_VALIDATION_TAG_ERROR,
                        CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID,
                        CRYS_RSA_DATA_POINTER_INVALID_ERROR,
                        CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
                        CRYS_RSA_INVALID_MESSAGE_BUFFER_SIZE,
                        CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
                        CRYS_RSA_INVALID_MESSAGE_VAL
 */
CEXPORT_C CRYSError_t CRYS_RSA_PRIM_Encrypt(CRYS_RSAUserPubKey_t *UserPubKey_ptr,
                                  CRYS_RSAPrimeData_t  *PrimeData_ptr,
                                  DxUint8_t              *Data_ptr,
                                  DxUint16_t              DataSize,
                                  DxUint8_t              *Output_ptr)
{
   /* FUNCTION LOCAL DECLERATIONS */

    /* the effective size in bits of the data message buffer */
   DxUint32_t DataEffectiveSizeInBits;

   /* the counter compare result */
   CRYS_COMMON_CmpCounter_t CounterCmpResult;

   /* the public key database pointer */
   CRYSRSAPubKey_t *PubKey_ptr;

   /* the modulus size in bytes */
   DxUint32_t nsizeInBytes;

   /* The return error identifier */
   CRYSError_t Error;

   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RSA_UNSUPPORTED( UserPubKey_ptr , PrimeData_ptr , Data_ptr ,
                              DataSize , Output_ptr , DataEffectiveSizeInBits , CounterCmpResult ,
                              PubKey_ptr , nsizeInBytes ,
                              Error , Error , Error , Error ,
                              Error,Error,Error,Error,Error,Error,Error,Error,Error);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT

   /* ............... initialize local variables ......................... */
   /* -------------------------------------------------------------------- */

   /* initialize the error identifier to O.K - success */
   Error = CRYS_OK;

   /* ............... checking the parameters pointers validity .......... */
   /* -------------------------------------------------------------------- */

   /* checking the key database handle pointer */
   if( UserPubKey_ptr == DX_NULL )

      return CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR;

   /* if the users TAG is illegal return an error - the context is invalid */
   if( UserPubKey_ptr->valid_tag != CRYS_RSA_PUB_KEY_VALIDATION_TAG )

      return CRYS_RSA_PUB_KEY_VALIDATION_TAG_ERROR;

   /* checking the Prime Data pointer */
   if( PrimeData_ptr == DX_NULL )

      return CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID;

   /* if the users Data pointer is illegal return an error */
   if( Data_ptr == DX_NULL )

      return CRYS_RSA_DATA_POINTER_INVALID_ERROR;

   /* if the users output pointer is illegal return an error */
   if( Output_ptr == DX_NULL )

      return CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR;

   /* if the data size is larger then the internal buffer return error */
   if( DataSize > sizeof(PrimeData_ptr->DataIn) )

      return CRYS_RSA_INVALID_MESSAGE_BUFFER_SIZE;
#ifndef CRYS_NO_FIPS_SUPPORT
   if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_STATE)
   {
	/* check if there was a FIPS error */
		if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_ERROR_STATE)
	{
		return CRYS_RSA_FIPS_MODE_FAIL_ERROR;
	}
	/* no check if the self test was already executed */
	if( !(DX_GLOBAL_FIPS_SF_STATUS & CRYS_SELF_TEST_RSA_ENCDEC_BIT) )
	{
		return CRYS_RSA_FIPS_MODE_SF_NOT_PERFORMED_ERROR;
	}
   }
#endif
   /* ................ copying the input data to the buffer .............. */
   /* -------------------------------------------------------------------- */

   /* clear the input data */
   DX_VOS_MemSet( PrimeData_ptr->DataIn,
               0 ,
               sizeof(PrimeData_ptr->DataIn));

   /* copy the input data to the aligned buffer on the data handler */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PrimeData_ptr->DataIn ,
                              Data_ptr ,
                              DataSize );

   /* ........... checking the data size and the message value ...................... */
   /* ------------------------------------------------------------------------------- */

   /* initializing the effective counters size in bits */
   DataEffectiveSizeInBits =
      CRYS_COMMON_GetCounterEffectiveSizeInBits( (DxUint8_t*)PrimeData_ptr->DataIn,DataSize );

   /* if the size is 0 - return an error */
   if( DataEffectiveSizeInBits == 0 )
   {
      Error = CRYS_RSA_INVALID_MESSAGE_DATA_SIZE;
      goto End;
   }

   /* setting the pointer to the key database */
   PubKey_ptr = ( CRYSRSAPubKey_t * )UserPubKey_ptr->PublicKeyDbBuff;

   /* setting the modulus size in bytes */
   nsizeInBytes = PubKey_ptr->nSizeInBits / 8;

   if( PubKey_ptr->nSizeInBits % 8 )

     nsizeInBytes++;

   /* compare the size of the message with the modulus it must be less then the modulous */
   CounterCmpResult = CRYS_COMMON_CmpLsbUnsignedCounters(
                           (DxUint8_t*)PrimeData_ptr->DataIn , DataSize,
                           (DxUint8_t*)PubKey_ptr->n , (DxUint16_t)nsizeInBytes );

   if( CounterCmpResult != CRYS_COMMON_CmpCounter2GraterThenCounter1 )
   {
      Error = CRYS_RSA_INVALID_MESSAGE_VAL;
      goto End;
   }

   /* ..................... executing the encryption ...................... */
   /* --------------------------------------------------------------------- */

   PLAT_LOG_DEV_PRINT_DisplayBuffer(( 22 , (char *)" \n IN CRYS_RSA_PRIM_Encrypt: Before Exp operation PrimeData_ptr->DataIn = " ,
                                           (DxUint8_t *)PrimeData_ptr->DataIn ,
                                           sizeof(PrimeData_ptr->DataIn)));

   /* executing the encryption */
   Error = LLF_PKI_RSA_ExecPubKeyExp( PubKey_ptr, PrimeData_ptr );

   if( Error != CRYS_OK )

      goto End;

   /* copy the output data from the aligned buffer to the users data on big endian format */
   CRYS_COMMON_ReverseMemcpy( Output_ptr , (DxUint8_t*)PrimeData_ptr->DataOut , nsizeInBytes );

   PLAT_LOG_DEV_PRINT_DisplayBuffer(( 22 , (char *)" \n IN CRYS_RSA_PRIM_Encrypt: After Exp operation PrimeData_ptr->DataOut = " ,
                                           (DxUint8_t *)PrimeData_ptr->DataOut ,
                                           sizeof(PrimeData_ptr->DataOut)));

   End:

   /* clear the data buffer */
   DX_VOS_MemSet( PrimeData_ptr , 0 , sizeof(CRYS_RSAPrimeData_t) );

   return Error;

   #endif /* !CRYS_NO_HASH_SUPPORT */
   #endif /* !CRYS_NO_PKI_SUPPORT */

}/* END OF CRYS_RSA_PRIM_Encrypt */

#endif /*!defined(_INTERNAL_CRYS_NO_RSA_ENCRYPT_SUPPORT) && !defined(_INTERNAL_CRYS_NO_RSA_VERIFY_SUPPORT)*/





#if !defined(_INTERNALCRYS_NO_RSA_DECRYPT_SUPPORT) && !defined(_INTERNAL_CRYS_NO_RSA_SIGN_SUPPORT)
/**
@brief
CRYS_RSA_PRIM_Decrypt implements the RSADP algorithm as defined in PKCS#1 v2.1 6.1.2

  @param[in] PrivKey_ptr - Pointer to the private key data
                           structure. \note The representation (pair
                           or quintiple) and hence the algorithm (CRT
                           or not) is determined by the Private Key
                           data structure. Using CRYS_Build_PrivKey or
                           CRYS_Build_PrivKeyCRT determines which
                           algorithm will be used.

  @param[in] PrimeData_ptr - a pointer to a structure containing internal buffers
                             required for the RSA operation.
  @param[in] Data_ptr - Pointer to the data to be decrypted.
  @param[in] DataSize - The size, in bytes, of the data to
                        decrypt. \note Must be <= the size of the
                        modulus.

  @param[out] Output_ptr - Pointer to the decrypted data, the buffer
                           must be at least PrivKey_ptr->N.len bytes
                           long (that is, the size of the modulus, in bytes).

  @return CRYSError_t - CRYS_OK,
                        CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                        CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID,
                        CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR,
                        CRYS_RSA_DATA_POINTER_INVALID_ERROR,
                        CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR,
                        CRYS_RSA_INVALID_MESSAGE_DATA_SIZE,
                        CRYS_RSA_INVALID_MESSAGE_VAL
 */

CEXPORT_C CRYSError_t CRYS_RSA_PRIM_Decrypt(
                                  CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,
                                  CRYS_RSAPrimeData_t   *PrimeData_ptr,
                                  DxUint8_t             *Data_ptr,
                                  DxUint16_t             DataSize,
                                  DxUint8_t             *Output_ptr)
{
   /* FUNCTION LOCAL DECLERATIONS */

    /* the effective size in bits of the data message buffer */
   DxUint32_t DataEffectiveSizeInBits;

   /* the counter compare result */
   CRYS_COMMON_CmpCounter_t CounterCmpResult;

   /* the private key database pointer */
   CRYSRSAPrivKey_t *PrivKey_ptr;

   /* the modulus size in bytes */
   DxUint32_t nSizeInBytes;

   /* The return error identifier */
   CRYSError_t Error;

   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RSA_UNSUPPORTED( UserPrivKey_ptr , PrimeData_ptr , Data_ptr ,
                              DataSize , Output_ptr , DataEffectiveSizeInBits , CounterCmpResult ,
                              PrivKey_ptr , nSizeInBytes ,
                              Error , Error , Error , Error ,
                              Error,Error,Error,Error,Error,Error,Error,Error,Error);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT

   /* ............... initialize local variables ......................... */
   /* -------------------------------------------------------------------- */

   /* initialize the error identifier to O.K - success */
   Error = CRYS_OK;


   /* ............... checking the parameters pointers validity .......... */
   /* -------------------------------------------------------------------- */

   /* ...... checking the key database handle pointer .................... */
   if( UserPrivKey_ptr == DX_NULL )

      return CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR;

   /* ...... checking the Prime Data pointer .................... */
   if( PrimeData_ptr == DX_NULL )

      return CRYS_RSA_PRIM_DATA_STRUCT_POINTER_INVALID;

   /* if the users TAG is illegal return an error - the context is invalid */
   if( UserPrivKey_ptr->valid_tag != CRYS_RSA_PRIV_KEY_VALIDATION_TAG )

      return CRYS_RSA_PRIV_KEY_VALIDATION_TAG_ERROR;

   /* if the users Data pointer is DX_NULL return an error */
   if( Data_ptr == DX_NULL )

      return CRYS_RSA_DATA_POINTER_INVALID_ERROR;

   /* if the users Output pointer is DX_NULL return an error */
   if( Output_ptr == DX_NULL )

      return CRYS_RSA_INVALID_OUTPUT_POINTER_ERROR;
#ifndef CRYS_NO_FIPS_SUPPORT
	 if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_STATE)
   {
	/* check if there was a FIPS error */
		if(DX_GLOBAL_FIPS_MODE & DX_CRYS_FIPS_MODE_ERROR_STATE)
	{
		return CRYS_RSA_FIPS_MODE_FAIL_ERROR;
	}
	/* no check if the self test was already executed */
	if( !(DX_GLOBAL_FIPS_SF_STATUS & CRYS_SELF_TEST_RSA_ENCDEC_BIT) )
	{
		return CRYS_RSA_FIPS_MODE_SF_NOT_PERFORMED_ERROR;
	}
   }
#endif
   /* ................ copying the input data to the buffer .............. */
   /* -------------------------------------------------------------------- */

   /* clear the input data */
   DX_VOS_MemSet( PrimeData_ptr->DataIn,
               0 ,
               sizeof(PrimeData_ptr->DataIn));

   /* copy the input data to the aligned buffer on the data handler in little endian format */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PrimeData_ptr->DataIn ,
                              Data_ptr ,
                              min( sizeof(PrimeData_ptr->DataIn) , DataSize ) );

   /* ................ checking the data size and the message value ....... */
   /* --------------------------------------------------------------------- */

   /* initializing the effective counters size in bits */
   DataEffectiveSizeInBits =  CRYS_COMMON_GetCounterEffectiveSizeInBits( (DxUint8_t*)PrimeData_ptr->DataIn,DataSize );

   /* if the size is 0 - return an error */
   if( DataEffectiveSizeInBits == 0 )
   {
     Error = CRYS_RSA_INVALID_MESSAGE_DATA_SIZE;
     goto End;
   }

   /* setting the pointer to the key database */
   PrivKey_ptr = ( CRYSRSAPrivKey_t * )UserPrivKey_ptr->PrivateKeyDbBuff;

   /* setting the modulus size in bytes */
   nSizeInBytes = PrivKey_ptr->nSizeInBits / 8;

   if( PrivKey_ptr->nSizeInBits % 8 )

       nSizeInBytes++;

   /* compare the message with the modulus: it must be less then the modulus */
   CounterCmpResult = CRYS_COMMON_CmpLsbUnsignedCounters(
                           (DxUint8_t*)PrimeData_ptr->DataIn , DataSize,
                           (DxUint8_t*)PrivKey_ptr->n , (DxUint16_t)nSizeInBytes );

   if( CounterCmpResult != CRYS_COMMON_CmpCounter2GraterThenCounter1 )
   {
     Error = CRYS_RSA_INVALID_MESSAGE_VAL;
     goto End;
   }

   /* ..................... executing the encryption ...................... */
   /* --------------------------------------------------------------------- */

   PLAT_LOG_DEV_PRINT_DisplayBuffer(( 22 , (char *)" \n IN CRYS_RSA_PRIM_Decrypt: Before Exp operation PrimeData_ptr->DataIn = " ,
                                           (DxUint8_t *)PrimeData_ptr->DataIn ,
                                           sizeof(PrimeData_ptr->DataIn)));


   /* executing the encryption */
   Error = LLF_PKI_RSA_ExecPrivKeyExp( PrivKey_ptr, PrimeData_ptr );

   if( Error != CRYS_OK )

      goto End;

   /* copy the output data from the aligned words-buffer to the users data in MS-LS bytes order */
   CRYS_COMMON_LsMsWordsArrayToMsLsBytes( Output_ptr , (DxUint8_t*)PrimeData_ptr->DataOut , DataSize );

   PLAT_LOG_DEV_PRINT_DisplayBuffer(( 22 , (char *)" \n IN CRYS_RSA_PRIM_Decrypt: After Exp operation PrimeData_ptr->DataOut = " ,
                                           (DxUint8_t *)PrimeData_ptr->DataOut ,
                                           sizeof(PrimeData_ptr->DataOut)));

   End:

   /* clear the data buffer, besides the PrimeData_ptr->DataOut, used by
      CRYS_RSA_PRIM_DecryptExactSize() function */

   DX_VOS_MemSetZero( PrimeData_ptr->DataIn , sizeof(PrimeData_ptr->DataIn) );
   DX_VOS_MemSetZero( (DxUint8_t*)&PrimeData_ptr->DataOut + sizeof(PrimeData_ptr->DataOut),
                      sizeof(CRYS_RSAPrimeData_t) - 2*sizeof(PrimeData_ptr->DataIn));

   return Error;

   #endif /* !CRYS_NO_HASH_SUPPORT */
   #endif /* !CRYS_NO_PKI_SUPPORT */

}/* END OF CRYS_RSA_PRIM_Decrypt */



#endif /*!defined(_INTERNAL_CRYS_NO_RSA_DECRYPT_SUPPORT) && !defined(_INTERNAL_CRYS_NO_RSA_SIGN_SUPPORT)*/
