
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  17 Jul. 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_KDF.c#1:csrc:8
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/
#include "DX_VOS_Mem.h"
#include "CRYS_CCM.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON_math.h"
#include "CRYS.h"
#include "CRYS_KDF.h"
#include "CRYS_KDF_error.h"


/************************ Defines *******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums *********************************/

/************************ macros ********************************/

/* this macro is required to remove compilers warnings if the HASH or PKI is not supported */

#if( defined CRYS_NO_HASH_SUPPORT || defined CRYS_NO_KDF_SUPPORT )
#define RETURN_IF_KDF_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0; \
  (a1)=0;(b1)=0;(c1)=0;(d1)=0;(e1)=0;(f1)=0;(g1)=0;(h1)=0;(i1)=0;(j1)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l); \
  (a1)=(a1);(b1)=(b1);(c1)=(c1);(d1)=(d1);(e1)=(e1);(f1)=(f1);(g1)=(g1);(h1)=(h1);(i1)=(i1);(j1)=(j1); \
  return CRYS_KDF_IS_NOT_SUPPORTED
#else  /* CRYS_NO_HASH_SUPPORT */
#define RETURN_IF_KDF_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1)
#endif /* !CRYS_NO_KDF_SUPPORT !CRYS_NO_HASH_SUPPORT */


/************************ Public Functions ******************************/


/****************************************************************/
/**
 * @brief _DX_KDF_KeyDerivFunc performs key derivation according to one of some modes defined in standards:
			   ANSI X9.42-2001, ANSI X9.63, OMA_TS_DRM_DRM_V2_0-20050712-C.

			   The present implementation of the function allows the following operation modes:
			   - CRYS_KDF_ASN1_DerivMode - mode based on  ASN.1 DER encoding;
			   - CRYS_KDF_ConcatDerivMode - mode based on concatenation;
               - CRYS_KDF_X963_DerivMode = CRYS_KDF_ConcatDerivMode;
			   - CRYS_KDF_OMA_DRM_DerivMode - specific mode for OMA DRM.

			The purpose of this function is to derive a keying data from the shared secret value and some
			other optional shared information (SharedInfo).

	The actual APIs that will be used by the user are:
		- CRYS_KDF_ASN1_KeyDerivFunc ;
		- CRYS_KDF_ConcatKeyDerivFunc ;
        - CRYS_KDF_OMADRM_DerivFunc .

  \note The length in Bytes of the hash result buffer is denoted by "hashlen".
  \note All buffers arguments are represented in Big-Endian format.

  @param[in] ZZSecret_ptr	 - A pointer to shared secret value octet string.
  @param[in] ZZSecretSize	 - The shared secret value Size, in bytes.
  @param[in] OtherInfo       - The pointer to structure, containing pointers and sizes of optional data shared
                               by two entities intended to share the secret value. This argument is optional
							   (if not needed - set it to NULL).
  @param[in] KDFhashMode	 - The KDF identifier of hash function to be used. The hash function output must be at least 160 bits.
  @param[out] KeyingData_ptr - A pointer to the keying data derived from the secret key, of length KeyLenInBits
  @param[in] KeyingDataSizeBytes	 - The size in bytes of the keying data to be generated. In our implementation -
							   KeyLenInBytes <= 2^32-1 .
  @param[in] derivation_mode - Specifies one of above described derivation modes.

      NOTE:
	        1.  The size of shared secret value , and size of each of parties of OtherInfo must be less than
			    maximal size of input data for used HASH function.

  @return CRYSError_t - On success the value CRYS_OK is returned,
			            and on failure an ERROR as defined in CRYS_KDF_error.h:
						CRYS_KDF_INVALID_ARGUMENT_POINTER_ERROR
						CRYS_KDF_INVALID_KEY_DERIVATION_MODE_ERROR
						CRYS_KDF_INVALID_SHARED_SECRET_VALUE_SIZE_ERROR
						CRYS_KDF_INVALID_SIZE_OF_DATA_TO_HASHING_ERROR
						CRYS_KDF_INVALID_ARGUMENT_HASH_MODE_ERROR
						CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR
						CRYS_KDF_INVALID_KEYING_DATA_SIZE_ERROR
*/

CEXPORT_C CRYSError_t  _DX_KDF_KeyDerivFunc(
	                                  DxUint8_t                *ZZSecret_ptr,
									  DxUint32_t                ZZSecretSize,
									  CRYS_KDF_OtherInfo_t     *OtherInfo_ptr,
								      CRYS_KDF_HASH_OpMode_t    KDFhashMode,
				                      CRYS_KDF_DerivFuncMode_t  derivation_mode,
				                      DxUint8_t                *KeyingData_ptr,
				                      DxUint32_t                KeyingDataSizeBytes )
{

    /* FUNCTION DECLARATIONS */

    /* The return error identifier */
    CRYSError_t Error;

	/* Hash operation mode identifier hashMode according to HASH module definitions */
	CRYS_HASH_OperationMode_t hashMode;

    /* HASH function output size and context structure buffer */
    DxUint32_t  HashOutputSizeBytes;

    CRYS_HASHUserContext_t  HashContext;

    /* Count of full HASH outputs for deriving the keying data of needed size */
    DxUint32_t  CountOfHashBlocks;

    /* Loop counters */
    DxUint32_t  i;

    /* big endian counter of Hash blocks (to be hashed with ZZ and OtherInfo) */
    DxUint32_t Counter;

	/* Current output buffer position */
	DxUint32_t CurrentOutputBuffPos = 0;

	/*The result buffer for the Hash*/
	CRYS_HASH_Result_t         HashResultBuff;

	/* The HASH input maximal block size */
	DxUint32_t  MaxHashInputSize;

	/* Other info Entry pointer */
	DxUint32_t  *OtherInfoEntry_ptr;
	/* loop counter */
	DxUint32_t  j;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_KDF_UNSUPPORTED( ZZSecret_ptr , ZZSecretSize , OtherInfo_ptr , KDFhashMode ,
                             derivation_mode , KeyingData_ptr , KeyingDataSizeBytes , hashMode ,
                             HashOutputSizeBytes , HashContext.valid_tag ,
                             CountOfHashBlocks , i , j , Counter ,
                             CurrentOutputBuffPos , HashResultBuff[0] , MaxHashInputSize ,
							 OtherInfoEntry_ptr , Error ,  Error , Error , Error );

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_KDF_SUPPORT


   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if an argument pointer is DX_NULL return an error */
	if( ZZSecret_ptr == DX_NULL || KeyingData_ptr == DX_NULL )
		return CRYS_KDF_INVALID_ARGUMENT_POINTER_ERROR;

	if( derivation_mode >= CRYS_KDF_DerivFunc_NumOfModes )
		return CRYS_KDF_INVALID_KEY_DERIVATION_MODE_ERROR;

    /* On ASN1 mode check the OtehrInfo pointer, on other modes it is optional */
	if( derivation_mode == CRYS_KDF_ASN1_DerivMode )
	{
		if(OtherInfo_ptr == DX_NULL)
			return CRYS_KDF_INVALID_ARGUMENT_POINTER_ERROR;

        if( OtherInfo_ptr->AlgorithmID_ptr == DX_NULL )
            return CRYS_KDF_INVALID_ALGORITHM_ID_POINTER_ERROR;

		if( OtherInfo_ptr->SizeOfAlgorithmID == 0 )
		    return CRYS_KDF_INVALID_ALGORITHM_ID_SIZE_ERROR;
	}

	/* Check sizes of the input data to be hashed (according to HASH conditions)*/
	MaxHashInputSize = 1 << 29;

	if(ZZSecretSize == 0 || ZZSecretSize > MaxHashInputSize )
		return CRYS_KDF_INVALID_SHARED_SECRET_VALUE_SIZE_ERROR;

	if( OtherInfo_ptr != DX_NULL )
	{
		if( OtherInfo_ptr->AlgorithmID_ptr != DX_NULL &&
			OtherInfo_ptr->SizeOfAlgorithmID > MaxHashInputSize )
			    return CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR;

		if( OtherInfo_ptr->PartyUInfo_ptr != DX_NULL &&
			OtherInfo_ptr->SizeOfPartyUInfo > MaxHashInputSize )
			    return CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR;

		if( OtherInfo_ptr->PartyVInfo_ptr != DX_NULL &&
			OtherInfo_ptr->SizeOfPartyVInfo > MaxHashInputSize )
			    return CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR;

		if( OtherInfo_ptr->SuppPrivInfo_ptr != DX_NULL &&
			OtherInfo_ptr->SizeOfSuppPrivInfo > MaxHashInputSize )
			    return CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR;

		if( OtherInfo_ptr->SuppPubInfo_ptr != DX_NULL &&
			OtherInfo_ptr->SizeOfSuppPubInfo > MaxHashInputSize )
			    return CRYS_KDF_INVALID_OTHER_INFO_SIZE_ERROR;
	}

	/* Check the size of keying data. Note: because max size limited in our implementation by
	   value 2^32-1, then we need to check the 0 value only */
	if( KeyingDataSizeBytes == 0 )
		return CRYS_KDF_INVALID_KEYING_DATA_SIZE_ERROR;


    /*................ Setting parameters according to current operation modes .......... */
    /*------------------------------------------------------------------------------------*/
    switch( KDFhashMode )
    {
		case CRYS_KDF_HASH_SHA1_mode:
			hashMode = CRYS_HASH_SHA1_mode;
			HashOutputSizeBytes = CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES;
			break;
		case CRYS_KDF_HASH_SHA224_mode:
			hashMode = CRYS_HASH_SHA224_mode;
			HashOutputSizeBytes = CRYS_HASH_SHA224_DIGEST_SIZE_IN_BYTES;
			break;
		case CRYS_KDF_HASH_SHA256_mode:
			hashMode = CRYS_HASH_SHA256_mode;
			HashOutputSizeBytes = CRYS_HASH_SHA256_DIGEST_SIZE_IN_BYTES;
			break;

		case CRYS_KDF_HASH_SHA384_mode:
			hashMode = CRYS_HASH_SHA384_mode;
			HashOutputSizeBytes = CRYS_HASH_SHA384_DIGEST_SIZE_IN_BYTES;
			break;
		case CRYS_KDF_HASH_SHA512_mode:
			hashMode = CRYS_HASH_SHA512_mode;
			HashOutputSizeBytes = CRYS_HASH_SHA512_DIGEST_SIZE_IN_BYTES;
			break;

		default:
		    return CRYS_KDF_INVALID_ARGUMENT_HASH_MODE_ERROR;

   }


   /* Calculate count of HASH blocks needed for key derivation */
   CountOfHashBlocks = ( KeyingDataSizeBytes + HashOutputSizeBytes - 1 )/ HashOutputSizeBytes;


   /**********  Keying data derivation loop *************/

	for( i = 0; i < CountOfHashBlocks; i++ )
	{
		/*.... HASH Init function .....*/
		Error = CRYS_HASH_Init(&HashContext,
					hashMode);
		if(Error != CRYS_OK)
			return Error;

		/*....... Hashing input data by calling HASH_Update function .......*/
		/*------------------------------------------------------------------*/

		/*.... Hashing of the shared secret value ....*/
		Error = CRYS_HASH_Update(&HashContext,
	                             ZZSecret_ptr,
	                             ZZSecretSize );
		if(Error != CRYS_OK)
			return Error;

        /*.... Hashing of the AlgorithmID (on ASN1 Derivation Mode only) ....*/
		if( derivation_mode == CRYS_KDF_ASN1_DerivMode )
		{
			Error = CRYS_HASH_Update( &HashContext,
		                             (DxUint8_t *)(OtherInfo_ptr->AlgorithmID_ptr),
		                              OtherInfo_ptr->SizeOfAlgorithmID );
			if(Error != CRYS_OK)
				return Error;
		}

		/* Set the blocks counter in big endianness mode */
		Counter = CRYS_COMMON_REVERSE32( i + 1 );

		/*.... Hashing of the blocks counter ....*/

		Error = CRYS_HASH_Update(&HashContext,
									(DxUint8_t *)&Counter,
									sizeof(DxUint32_t) );
		if(Error != CRYS_OK)
			return Error;

		/* ..... Hashing of remaining data of the OtherInfo ..... */
		if( OtherInfo_ptr != DX_NULL )
		{
			/* Set OtherInfoEntry_ptr to second entry pointer */
			OtherInfoEntry_ptr = (DxUint32_t*)OtherInfo_ptr + 2;

			/* OtherInfo data concatenating and hashing loop */
			for( j = 0; j < CRYS_KDF_COUNT_OF_OTHER_INFO_ENTRIES - 1; j++ )
			{
				/* if entry exists hash it */
				if( (*OtherInfoEntry_ptr != DX_NULL) && (*(OtherInfoEntry_ptr + 1) != 0) )
				{
					Error = CRYS_HASH_Update(&HashContext,
											(DxUint8_t *)(*OtherInfoEntry_ptr)/*pointer to entry*/,
											*(OtherInfoEntry_ptr + 1)/*size of entry*/ );
					if(Error != CRYS_OK)
						return Error;
				}
				/* Shift the pointer to the next entry */
				OtherInfoEntry_ptr += 2;
			}
		}

		/* ..........  HASH Finish operation ............. */
		Error = CRYS_HASH_Finish( &HashContext ,
	                              HashResultBuff );
		if(Error != CRYS_OK)
			return Error;

		/* Correction of output data size for last block ( if it is not full ) */
		if( i == CountOfHashBlocks - 1 )
			HashOutputSizeBytes = KeyingDataSizeBytes - i * HashOutputSizeBytes;

		/* Copying HASH data into output buffer */
		DX_VOS_FastMemCpy(&KeyingData_ptr[CurrentOutputBuffPos],(DxUint8_t *)HashResultBuff, HashOutputSizeBytes);

		/* Increment the output buffer position */
		CurrentOutputBuffPos += HashOutputSizeBytes;
	}

	return Error;

  #endif /*CRYS_NO_KDF_SUPPORT*/
  #endif /*CRYS_NO_HASH_SUPPORT*/


}/* END OF _DX_KDF_KeyDerivationFunc */
