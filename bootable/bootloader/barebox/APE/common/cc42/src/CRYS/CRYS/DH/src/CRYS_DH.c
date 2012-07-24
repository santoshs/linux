
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Tue Feb 01 17:15:04 2005
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_DH.c#1:csrc:8
   *  \author ohads
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/
#include "DX_VOS_Mem.h"
#include "CRYS_DH_errors.h"
#include "CRYS.h"
#include "CRYS_CCM.h"
#include "CRYS_DH_local.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_RSA_BUILD.h"
#include "log_output.h"

/************************ Defines *******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums *********************************/

/************************ macros ********************************/

/* this macro is required to remove compilers warnings if the HASH or PKI is not supported */

#if defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_PKI_SUPPORT) || defined(CRYS_NO_DH_SUPPORT)
#define RETURN_IF_DH_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0; \
  (a1)=0;(b1)=0;(c1)=0;(d1)=0;(e1)=0;(f1)=0;(g1)=0;(h1)=0;(i1)=0;(j1)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l); \
  (a1)=(a1);(b1)=(b1);(c1)=(c1);(d1)=(d1);(e1)=(e1);(f1)=(f1);(g1)=(g1);(h1)=(h1);(i1)=(i1);(j1)=(j1); \
  return CRYS_DH_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_HASH_SUPPORT && ! CRYS_NO_PKI_SUPPORT && !CRYS_NO_DH_SUPPORT */
#define RETURN_IF_DH_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1)
#endif /* !CRYS_NO_HASH_SUPPORT && ! CRYS_NO_PKI_SUPPORT */

/************************ global data ***********************************/
#ifdef CRYS_DH_SET_PRIV_KEY_NON_RANDOM
DxUint8_t CRYS_DH_DEBUG_PrivKey[4*CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS];
#endif

/************************ Public Functions ******************************/


/*******************************************************************************************/

/**
 * @brief _DX_DH_GeneratePubPrv has 2 functionalities:

	   1. Randomly generates the Client private key Prv.
	   2. Compute the Client public key which is
		  ClientPub = Generator ^ Prv mod Prime.

	Note: All buffers arguments are represented in Big-Endian

  @param[in] Generator_ptr		- Pointer to the Generator octet string
  @param[in] GeneratorSize		- Size of the Generator String (in bytes)
  @param[in] Prime_ptr			- Pointer to the Prime octet string P.
  @param[in] PrimeSize			- Size of the Prime string (in bytes)
  @param[in] L					- Exact length of Private key in bits (relevant only for PKCS#3), defined
                                  by central authority.
								   - If L != 0, then L must met to requirements:  1 <= L and 2^(L-1) <= P,
								     Note: in our implementation required L >= 3.
								     in this case force the private key size to be 2^(L-1) <= Prv < 2^l.
								   - If L  = 0, then: 0 < Prv < P-1.
  @param[in] Q_ptr				- Pointer to the order Q of generator relevant only for ANSI X9.42):
									1<= Prv <= q-1   or   1 < Prv < q-1.
  @param[in] QSize				- Size of the Q string in bytes. Relevant only for ANSI X9.42.
  @param[in] DH_mode			- Enumerator, declaring whether the mode is PKCS#3 or ANSI X9.42.
  @param[in] UserPubKey_ptr		- A pointer to the public key structure. Used for the Exp operation.
								  The structure doesn't need to be initialized.
  @param[in] PrimeData_ptr		- A pointer to a structure containing internal buffers, the structure
                                  doesn't need to be initialized.
  @param[out] ClientPrvKey_ptr  - Pointer to the buffer for output the Private key octet string (Prv).
								  In PKCS#3 this buffer should be at least the following size:
									  - if L is provided the size of buffer: bufSizeBytes*8 >= L.
									  - if L is DX_NULL, then size of buffer >= PrimeSize.
								  In X9.42 size of buffer >= QSize.
  @param[in/out] ClientPrvKeySize_ptr - The user should provide the size of the buffer indicated by ClientPrvKey_ptr.
                                        The function returns the actual size in bytes of the Private key.
  @param[out] ClientPub_ptr	- Pointer to the Public key octet string
								  This Buffer should be at least PrimeSize bytes

  @param[in/out] ClientPubSize_ptr -    The user should provide the size of the buffer indicated by ClientPub_ptr
                                        The function returns the actual size in bytes of the generated client public key.

  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
                        CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
						CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
						CRYS_DH_ARGUMENT_GENERATOR_SMALLER_THAN_ZERO_ERROR;
						CRYS_DH_ARGUMENT_PRIME_SMALLER_THAN_GENERATOR_ERROR;
						CRYS_DH_INVALID_L_ARGUMENT_ERROR;
						CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;
					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
 */
CEXPORT_C CRYSError_t _DX_DH_GeneratePubPrv(
	                                DxUint8_t *Generator_ptr,              /*generator*/
                                    DxUint16_t GeneratorSize,
                                    DxUint8_t *Prime_ptr,                  /*modulus*/
                                    DxUint16_t PrimeSize,
                                    DxUint16_t L,
                                    DxUint8_t *Q_ptr,                      /*order*/
                                    DxUint16_t QSize,
                                    CRYS_DH_OpMode_t DH_mode,
                                    CRYS_DHUserPubKey_t *tmpPubKey_ptr,    /*temp buff*/
                                    CRYS_DHPrimeData_t  *tmpPrimeData_ptr, /*temp buff*/
                                    DxUint8_t *ClientPrvKey_ptr,           /*out*/
                                    DxUint16_t *ClientPrvKeySize_ptr,      /*in/out*/
                                    DxUint8_t *ClientPub1_ptr,             /*out*/
                                    DxUint16_t *ClientPubSize_ptr)         /*in/out*/

{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* temporary byte shift masks */
   DxUint8_t tmpByte, mask, mask1, shift;

   /* the vector 2^(L-1) size*/
   DxUint16_t tmpSize;

   /* the comparing value, returned from the vectors compare */
   CRYS_COMMON_CmpCounter_t comp;

   PRINTF("Entered _DX_DH_GeneratePubPrv\n");

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DH_UNSUPPORTED( Generator_ptr, GeneratorSize, Prime_ptr,
                             PrimeSize, L, Q_ptr, QSize ,
                             Error/*DH_mode*/, tmpPubKey_ptr, tmpPrimeData_ptr, ClientPrvKey_ptr ,
                             ClientPrvKeySize_ptr, ClientPub1_ptr, ClientPubSize_ptr,
                             tmpByte, tmpSize, comp, mask, mask1, Error, Error, Error);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT
   #ifndef CRYS_NO_DH_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if an argument pointer is DX_NULL return an error */
   if( Generator_ptr == DX_NULL || Prime_ptr == DX_NULL ||
	   ClientPrvKey_ptr == DX_NULL || ClientPub1_ptr == DX_NULL ||
	   ClientPrvKeySize_ptr == DX_NULL || ClientPubSize_ptr == DX_NULL ||
	   tmpPubKey_ptr == DX_NULL || tmpPrimeData_ptr == DX_NULL){

PRINTF("Failed as an argument pointer is DX_NULL\n");
       return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
}

   /*If an argument buffer size is zero return an error*/
   if( GeneratorSize == 0 || PrimeSize == 0 ||
	   *ClientPrvKeySize_ptr == 0 || *ClientPubSize_ptr < PrimeSize )   {
               PRINTF("Failed as an argument buffer size is zero\n");
		return CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
   }
   if(DH_mode > CRYS_DH_NumOfModes){
               PRINTF("DH_mode > CRYS_DH_NumOfModes\n");
		return CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
    }

   if(DH_mode == CRYS_DH_ANSI_X942_mode)
   {
	   if(Q_ptr == DX_NULL){
                PRINTF("Q_ptr = NULL\n");
		   return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
         }
	   if(QSize == 0) {
                       PRINTF("QSize ==0\n");
			return CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
                  }
   }

   /*Compare the generator with zero: requested g > 0 */
   tmpByte = 0;
   comp = CRYS_COMMON_CmpMsbUnsignedCounters( Generator_ptr,
                                              GeneratorSize,
                                              &tmpByte, 1);

   if(comp != CRYS_COMMON_CmpCounter1GraterThenCounter2){
          PRINTF("comp failed\n");
       return CRYS_DH_ARGUMENT_GENERATOR_SMALLER_THAN_ZERO_ERROR;
  }

   /*Compare the generator and the Prime vectors: requested that g < P */
   comp = CRYS_COMMON_CmpMsbUnsignedCounters( Generator_ptr,
                                              GeneratorSize,
                                              Prime_ptr,
                                              PrimeSize );

   if(comp != CRYS_COMMON_CmpCounter2GraterThenCounter1){
       PRINTF("comp failed again\n");
       return CRYS_DH_ARGUMENT_PRIME_SMALLER_THAN_GENERATOR_ERROR;
    }

   /*--------------------------------------------------------------------------------*/
   /*         DH public-private keys generation                                      */
   /*--------------------------------------------------------------------------------*/

   /* temporary set prime modulus into temp buffer in little endianness */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)tmpPrimeData_ptr->DataIn , Prime_ptr , PrimeSize );

   /* get actual size of prime in bits */
   tmpSize = (DxUint16_t)CRYS_COMMON_GetBytesCounterEffectiveSizeInBits(
	                                                        (DxUint8_t*)tmpPrimeData_ptr->DataIn,
	                                                         PrimeSize );

   /* correction of Prime_ptr pointer and Size for removing of not significant zero-bytes */
   if( PrimeSize - (tmpSize + 7) / 8 > 0 )
   {
	   Prime_ptr += PrimeSize - (tmpSize + 7) / 8;
	   PrimeSize = (tmpSize + 7) / 8;
   }

   switch(DH_mode)
   {
		case CRYS_DH_PKCS3_mode:
		/* ----------------------------------------------------------- *
            PKCS#3:  set x private random value according to following:
				   1) If L = 0: set    0 < x < P-1;
				   2) If L > 0: set  2^(L-1) <= x < 2^L ,
				           where 2^(L-1) <= P.
		  ----------------------------------------------------------- */
           if(L == 0)
           {
               PRINTF("Entered CRYS_DH_PKCS3_mode\n");
                PRINTF("Entered L=0\n");
               /* Option 1: L is not provided - check the minimum size of the private key buffer */
               if(*ClientPrvKeySize_ptr < PrimeSize)
                   return CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;

			   /* random generation in range:  0 < x < P-1  (in little endian */
			   Error = CRYS_RND_GenerateVectorInRange( tmpSize /*rndSizeInBits*/,
				                                       (DxUint8_t*)tmpPrimeData_ptr->DataIn/*maxVect*/,
				                                       (DxUint8_t*)tmpPrimeData_ptr->DataOut/*out*/ );
			   if(Error != CRYS_OK){
                                   PRINTF("CRYS_RND_GenerateVectorInRange failed\n");
				   goto EXIT_WITH_ERROR;
                           }

			   /* reverse privKey to big endianness */
			   CRYS_COMMON_ReverseMemcpy( ClientPrvKey_ptr , (DxUint8_t*)tmpPrimeData_ptr->DataOut , PrimeSize );

		       /* private key size in bytes */
			   *ClientPrvKeySize_ptr = PrimeSize;
		   }
           else  /* Option 2:  L > 0 and bit length of privKey must be exactly L bit */
           {
			   /* check L and the minimum size of the private key buffer */
			   if( L > tmpSize || L < CRYS_RSA_MIN_PUB_EXP_VALUE )
			   {
				   Error = CRYS_DH_INVALID_L_ARGUMENT_ERROR;
				   goto EXIT_WITH_ERROR;
			   }
			   if((*ClientPrvKeySize_ptr)*8 < L)
			   {
				   Error = CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;
				   goto EXIT_WITH_ERROR;
			   }

				/* actual private key size in bytes and shift value */
				*ClientPrvKeySize_ptr = (L+7)/8;
				shift = ((8 - (L & 7)) & 7);

				/* if L = modulus size, then generate random x with exact bit-size = L
				   and value in range:  2^(L-1) < x < P */
				if( L == tmpSize )
				{
					mask   = 0x7F >> shift;
					mask1  = 0x80 >> shift;

					/* set temporary MSBit of modulus = 0 for generation random in range without MSbit */
                    ((DxUint8_t*)tmpPrimeData_ptr->DataIn)[*ClientPrvKeySize_ptr - 1] &= mask;

					/* generate random in range */
					Error = CRYS_RND_GenerateVectorInRange( tmpSize /*rndSizeInBits*/,
														   (DxUint8_t*)tmpPrimeData_ptr->DataIn/*maxVect*/,
														   (DxUint8_t*)tmpPrimeData_ptr->DataOut/*out*/ );

					if(Error != CRYS_OK)
						goto EXIT_WITH_ERROR;

					/* set MSBit of random to 1 */
					((DxUint8_t*)tmpPrimeData_ptr->DataIn)[*ClientPrvKeySize_ptr - 1] |= mask1;

					/* reverse privKey to big endianness */
					CRYS_COMMON_ReverseMemcpy( ClientPrvKey_ptr , (DxUint8_t*)tmpPrimeData_ptr->DataOut , *ClientPrvKeySize_ptr );
				}
				/* if L < modulus size, then generate random x of size L bits */
				else
				{
				   /* random generation */
				   Error = CRYS_RND_GenerateVector( *ClientPrvKeySize_ptr, ClientPrvKey_ptr );

					if(Error != CRYS_OK)
						goto EXIT_WITH_ERROR;

					/* set two appropriate high bits of privKey to 00..1 to met the requirement 2^(L-1) <= x < 2^L */
					if( (L & 7) > 0 )
					{
						mask  = 0xFF >> shift;
						mask1 = 0x80 >> shift;
						ClientPrvKey_ptr[0] = (ClientPrvKey_ptr[0] & mask) | mask1;
					}
					else /*if ( (L & 7) == 0 )*/
					{
						ClientPrvKey_ptr[0] |= 0x80;
					}
				}
          }

	  break;

		case CRYS_DH_ANSI_X942_mode:
        /* ----------------------------------------------------------- *
				      ANSI X9.42:
	                 1<= X <= q -1 or    1< X <= q -1
           ----------------------------------------------------------- */
		if( *ClientPrvKeySize_ptr < QSize )
                   return CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;

				/* set order Q into temp buffer in little endianness */
				CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)tmpPrimeData_ptr->DataIn , Q_ptr , QSize );

				/* get actual size in bits */
				tmpSize = (DxUint16_t)CRYS_COMMON_GetBytesCounterEffectiveSizeInBits(
					                                                       (DxUint8_t*)tmpPrimeData_ptr->DataIn,
					                                                       QSize );
                /* private key size in bytes */
                *ClientPrvKeySize_ptr = (tmpSize + 7) / 8;


				/* random generation in range:  0 < x < Q-1  (little endianness) */
				Error = CRYS_RND_GenerateVectorInRange( tmpSize /*rndSizeInBits*/,
					                                   (DxUint8_t*)tmpPrimeData_ptr->DataIn/*maxVect*/,
					                                   (DxUint8_t*)tmpPrimeData_ptr->DataOut/*out*/ );
				if(Error != CRYS_OK)
					goto EXIT_WITH_ERROR;

				/* reverse privKey to big endianness */
				CRYS_COMMON_ReverseMemcpy( ClientPrvKey_ptr , (DxUint8_t*)tmpPrimeData_ptr->DataOut , *ClientPrvKeySize_ptr );

				break;

    default:
			return CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;

   }

    /* ----------------------------------------------------------- */
	/*           Create the public key                             */
	/* ----------------------------------------------------------- */

	/* Build the RSA PublKey data structure for the Exp operation, using RSA_Encrypt primitive */
	Error = CRYS_RSA_Build_PubKey(  tmpPubKey_ptr,
				                    ClientPrvKey_ptr,
				                    *ClientPrvKeySize_ptr,
				                    Prime_ptr,
				                    PrimeSize );
    /* check error */
    if(Error != CRYS_OK){
       PRINTF("CRYS_RSA_Build_PubKey failed\n");
	return Error;
     }

	/* Debug print */
    PLAT_LOG_DEV_PRINT_DisplayBuffer(( 10 , (char *)" \n In CRYS_DH_GeneratePubPrv Before Exp operation Data = " ,(DxUint8_t *)Generator_ptr , GeneratorSize));
    PLAT_LOG_DEV_PRINT_DisplayBuffer(( 10 , (char *)" \n In CRYS_DH_GeneratePubPrv Before Exp operation Exp = " ,(DxUint8_t *)ClientPrvKey_ptr , *ClientPrvKeySize_ptr));
    PLAT_LOG_DEV_PRINT_DisplayBuffer(( 10 , (char *)" \n In CRYS_DH_GeneratePubPrv Before Exp operation Modulus = " ,(DxUint8_t *)Prime_ptr , PrimeSize));

	/*Call the exponent operation to calculate the ClientPub1 = Generator^privKey mod Prime */
    Error = CRYS_RSA_PRIM_Encrypt( tmpPubKey_ptr,
                                   tmpPrimeData_ptr,
				                   Generator_ptr,
				                   GeneratorSize,
				                   ClientPub1_ptr );

	if(Error != CRYS_OK){
               PRINTF("CRYS_RSA_PRIM_Encrypt failed\n");
		return Error;
       }

	*ClientPubSize_ptr = PrimeSize;

	/* Debug print */
    PLAT_LOG_DEV_PRINT_DisplayBuffer(( 10 , (char *)" \n In CRYS_DH_GeneratePubPrv After Exp operation Output = " ,(DxUint8_t *)ClientPub1_ptr , *ClientPubSize_ptr));

    /* delete secure sensitive data */
	DX_VOS_MemSetZero( tmpPubKey_ptr, sizeof(CRYS_DHUserPubKey_t) );
	DX_VOS_MemSetZero( tmpPrimeData_ptr, sizeof(CRYS_DHPrimeData_t) );

     PRINTF("Executed whole function\n");
EXIT_WITH_ERROR:

     return Error;

#endif /*CRYS_NO_DH_SUPPORT*/
#endif /*CRYS_NO_PKI_SUPPORT*/
#endif /*CRYS_NO_HASH_SUPPORT*/


}/* END OF _DX_DH_GeneratePubPrv function */


/*******************************************************************************************/
/**
 * @brief CRYS_DH_GetSecretKey computes the shared secret key in the following computation:
	               SecretKey = ServerPubKey ^ ClientPrvKey mod Prime.

   Note:
     - All buffers arguments are represented in Big-Endian.
	 - The actual size of private key in bits must be not less than 2 and not great,
	   than actual size of Prime (modulus) in bits (in our implementation);
	 - The user may call this function also by the following macro
	   CRYS_DH_PKCS3_GetSecretKey(), inserted for compatibility with other applications;

  @param[in] ClientPrvKey_ptr	- Pointer to the private key octet string.
  @param[in] ClientPrvKeySize	- The Private key Size (in bytes).
  @param[in] ServerPubKey_ptr   - Pointer to the Server public key octet string -
  @param[in] ServerPubKeySize   - The Server Public key Size (in bytes)
  @param[in] Prime_ptr			- Pointer to the Prime octet string.
  @param[in] PrimeSize			- Size of the Prime string.
  @param[in] UserPubKey_ptr		- A pointer to the public key structure. used for the Exp operation function
                                  the struct doesn't need to be initialized.
  @param[in] PrimeData_ptr		- A pointer to a structure containing internal buffers
                                  the struct doesn't need to be initialized.
  @param[out] SecretKey_ptr	- Pointer to the secret key octet string.
								  This buffer should be at least PrimeSize Bytes.
  @param[in/out] SecretKeySize_ptr - The user should provide the actual size in bytes of the buffer indicated by SecretKey_ptr
                                    The function will return the actual size in bytes of the output secret key

  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
					    CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
					    CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
					    CRYS_DH_SECRET_KEY_SIZE_OUTPUT_ERROR;
*/
 CEXPORT_C CRYSError_t CRYS_DH_GetSecretKey(
	                                DxUint8_t *ClientPrvKey_ptr,
				                    DxUint16_t ClientPrvKeySize,
				                    DxUint8_t *ServerPubKey_ptr,
				                    DxUint16_t ServerPubKeySize,
				                    DxUint8_t *Prime_ptr,
				                    DxUint16_t PrimeSize,
                                    CRYS_DHUserPubKey_t *tmpUserPubKey_ptr,
                                    CRYS_DHPrimeData_t  *tmpPrimeData_ptr,
				                    DxUint8_t *SecretKey_ptr,
				                    DxUint16_t *SecretKeySize_ptr)
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   CRYS_COMMON_CmpCounter_t cmpResult;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DH_UNSUPPORTED( ClientPrvKey_ptr , ClientPrvKeySize , ServerPubKey_ptr ,
                             ServerPubKeySize , Prime_ptr , PrimeSize , tmpUserPubKey_ptr ,
                             tmpPrimeData_ptr , SecretKey_ptr , SecretKeySize_ptr ,
                             Error , Error , Error , Error , Error , Error ,
                             Error , Error , Error , Error , Error , Error);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT
   #ifndef CRYS_NO_DH_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /* if an argument pointer is DX_NULL return an error */
   if( ClientPrvKey_ptr == DX_NULL || ServerPubKey_ptr == DX_NULL ||
	   Prime_ptr == DX_NULL || tmpUserPubKey_ptr == DX_NULL ||
	   tmpPrimeData_ptr == DX_NULL || SecretKey_ptr == DX_NULL || SecretKeySize_ptr == DX_NULL )

       return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;

   /*If an argument buffer size is zero return an error*/
   if( ClientPrvKeySize == 0 || ServerPubKeySize == 0 ||
	   PrimeSize == 0 || *SecretKeySize_ptr < PrimeSize )
		return CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;

   /* 1. verifying that the private exponent is less than modulus, else subtract the modulus */
   cmpResult = CRYS_COMMON_CmpMsbUnsignedCounters( ClientPrvKey_ptr, ClientPrvKeySize,
											       Prime_ptr, PrimeSize );

   if( cmpResult != CRYS_COMMON_CmpCounter2GraterThenCounter1 )
   {
	    /* subtract modulus prime from private key and set result in temp buffer */
		CRYS_COMMON_SubtractMSBUint8Arrays( ClientPrvKey_ptr, ClientPrvKeySize, Prime_ptr, PrimeSize,
		                                    (DxUint8_t*)tmpPrimeData_ptr->DataIn );

		/* build the Data for the Exp operation.
		  Note: for convenience the user private key is set into public key structure */
	    Error = CRYS_RSA_Build_PubKey(  tmpUserPubKey_ptr,
									   (DxUint8_t*)tmpPrimeData_ptr->DataIn,
									   ClientPrvKeySize,
									   Prime_ptr,
									   PrimeSize );
   }
   else
   {
	   /* build the Data for the Exp operation */
	   Error = CRYS_RSA_Build_PubKey(  tmpUserPubKey_ptr,
									   ClientPrvKey_ptr,
									   ClientPrvKeySize,
									   Prime_ptr,
									   PrimeSize );
   }

   if( Error != CRYS_OK )
	return Error;

   /* 3. create the public key: Secret_key = Server_public_key ^ Prv mod Prime */
   Error = CRYS_RSA_PRIM_Encrypt( tmpUserPubKey_ptr, /* Note: this is the private key */
                                  tmpPrimeData_ptr,
								  ServerPubKey_ptr,
								  ServerPubKeySize,
				                  SecretKey_ptr );
   if(Error != CRYS_OK)
	return Error;

   *SecretKeySize_ptr = PrimeSize;

   /* delete secure sensitive data */
   DX_VOS_MemSetZero( tmpUserPubKey_ptr, sizeof(CRYS_DHUserPubKey_t) );
   DX_VOS_MemSetZero( tmpPrimeData_ptr, sizeof(CRYS_DHPrimeData_t) );


   return Error;

#endif /*CRYS_NO_DH_SUPPORT*/
#endif /*CRYS_NO_PKI_SUPPORT*/
#endif /*CRYS_NO_HASH_SUPPORT*/

}  /* END of CRYS_DH_PKCS3_GetSecretKey function */



/****************************************************************/
/**
 * @brief CRYS_DH_X942_GetSecretData computes the shared secret key as follows:
		1. OPTIONAL - validate the correctness of the argument keys
		2. SecretKey = ServerPubKey ^ ClientPrvKey mod Prime
		3. Use of Derivation function to derive a data key from the secret key

	The actual APIs that will be used by the user are:
	CRYS_DH_X942_GetSecret_ASN1_2_Data
	CRYS_DH_X942_GetSecret_Concat_2_Data

	Note: All buffers arguments are represented in Big-Endian format

  @param[in] tmpUserPubKey_ptr	- A pointer to the public key data structure. Not initialized.
  @param[in] tmpPrimeData_ptr	- A pointer to a CRYS_RSAPrimeData_t structure
								  that is used for the Exp operation
  @param[in] hashMode			- The hash function to be used
  @param[in] ClientPrvKey_ptr	- A pointer to the Private key octet string
  @param[in] ClientPrvKeySize	- The Private key size, in bytes
  @param[in] ServerPubKey_ptr   - A pointer to the Server public key octet string -
  @param[in] ServerPubKeySize   - The Server Public key size, in bytes
  @param[in] Prime_ptr			- A pointer to the Prime octet string
  @param[in] PrimeSize			- The size of the Prime string
  @param[in] DerFunc_mode		- The type of function to derive the secret key to the key data.
								  We use ASN.1-based function or Hash concatenation function mode.
  @param[in] OtherInfo          - The pointer to predefined structure, containing pointers and sizes of optional data
                                  shared by two entities intended to share the secret value. This argument (structure)
							      and also its members are optional (if any is not need - set its pointer and size to NULL).
							      Note: OtherInfo not includes Counter, because the Counter is calculated inside the
							      function .
  @param[out] SecretKeyData_ptr	- A pointer to the secret key octet string.
								  This buffer should be at least PrimeSize bytes.
  @param[in] SecretKeyDataSizeBitsNeeded - Specifies the derived Secret Key data size needed in Bits. This value converted to bytes
                                  cannot be larger than PrimeSize (required by implementation)

  @return CRYSError_t - On success CRYS_OK is returned, on failure an ERROR as defined CRYS_DH_error.h:
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR;
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_BIGGER_THAN_PRIME_SIZE;
*/
/*CEXPORT_C CRYSError_t CRYS_DH_X942_GetSecretData(
	                                   CRYS_DHUserPubKey_t        *tmpUserPubKey_ptr,
                                       CRYS_DHPrimeData_t         *tmpPrimeData_ptr,
                                       CRYS_DH_HASH_OpMode_t       hashMode,
                                       DxUint8_t                  *ClientPrvKey_ptr,
                                       DxUint16_t                  ClientPrvKeySize,
                                       DxUint8_t                  *ServerPubKey_ptr,
                                       DxUint16_t                  ServerPubKeySize,
                                       DxUint8_t                  *Prime_ptr,
                                       DxUint16_t                  PrimeSize,
                                       CRYS_DH_DerivationFunc_Mode DerivFunc_mode,
									   CRYS_DH_OtherInfo_t        *otherInfo_ptr,
                                       DxUint8_t                  *SecretKeyData_ptr,
                                       DxUint16_t                  SecretKeyDataSizeBitsNeeded )
{*/
  /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   //CRYSError_t Error;

   /*The return Secret key size from the PKCS#3 function*/
   /*The assignment to Prime size is according to the real size of the buffer SecretKeyData_ptr*/
   //DxUint16_t SecretKeyDataSize = PrimeSize;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   //Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   //RETURN_IF_DH_UNSUPPORTED( tmpUserPubKey_ptr, tmpPrimeData_ptr, Error/*hashMode*/, ClientPrvKey_ptr,
     //                        ClientPrvKeySize,  ServerPubKey_ptr, ServerPubKeySize,
       //                      Prime_ptr, PrimeSize, Error/*DerivFunc_mode*/, otherInfo_ptr,
         //                    SecretKeyData_ptr, SecretKeyDataSizeBitsNeeded, SecretKeyDataSize,
           //                  Error, Error, Error, Error, Error, Error, Error, Error);

   //#ifndef CRYS_NO_HASH_SUPPORT
   //#ifndef CRYS_NO_PKI_SUPPORT
   //#ifndef CRYS_NO_DH_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /*check that the derived secret key needed is not zero*/
   //if(SecretKeyDataSizeBitsNeeded == 0)
     //  return CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR;

   /*check that the derived secret key needed is smaller than PrimeSize*/
   //if(SecretKeyDataSizeBitsNeeded > PrimeSize*8)
     //  return CRYS_DH_SECRET_KEY_SIZE_NEEDED_BIGGER_THAN_PRIME_SIZE;

   /*Call the PKCS#3 get secret key function*/
   /*Error = CRYS_DH_PKCS3_GetSecretKey(ClientPrvKey_ptr,
				                    ClientPrvKeySize,
				                    ServerPubKey_ptr,
				                    ServerPubKeySize,
				                    Prime_ptr,
				                    PrimeSize,
				                    tmpUserPubKey_ptr,
									tmpPrimeData_ptr,
				                    SecretKeyData_ptr,
				                    &SecretKeyDataSize);

	if(Error != CRYS_OK)
		return Error;

*/
	/*Let the keydataSize from the previous function determine the key data length in the next function*/
/*	Error = CRYS_DH_KeyDerivFunc(SecretKeyData_ptr,
			                      SecretKeyDataSize,
								  otherInfo_ptr,
							      hashMode,
			                      DerivFunc_mode,
			                      (DxUint8_t *)tmpPrimeData_ptr->DataIn,
			                      (SecretKeyDataSizeBitsNeeded+7)>>3 );


	if(Error != CRYS_OK)
		return Error;

	DX_VOS_FastMemCpy( SecretKeyData_ptr, (DxUint8_t*)tmpPrimeData_ptr->DataIn, (SecretKeyDataSizeBitsNeeded+7)>>3 );

	return Error;

#endif*/ /*CRYS_NO_DH_SUPPORT*/
//#endif /*CRYS_NO_PKI_SUPPORT*/
//#endif /*CRYS_NO_HASH_SUPPORT*/

//}/* END OF _DX_DH_X942_GetSecretData */


/****************************************************************/
/**
 * @brief CRYS_DH_X942_HybridGetSecretData computes the shared secret key as follows:
		1. OPTIONAL - validate the correctness of the argument keys
		2. SecretKey1 = ServerPubKey1 ^ ClientPrvKey1 mod Prime
		3. SecretKey2 = ServerPubKey2 ^ ClientPrvKey2 mod Prime
		3. Use of Derivation function to derive a data key from the 2 secret keys

	The actual APIs that will be used by the user are:
	CRYS_DH_X942_Hybrid_GetSecret_ASN1_2_Data
	CRYS_DH_X942_Hybrid_GetSecret_Concat_2_Data

	Note: All buffers arguments are represented in Big-Endian format.

  @param[in] tmpUserPubKey_ptr	- A pointer to the public key data structure. Not initialized.
  @param[in] tmpPrimeData_ptr	- A pointer to a CRYS_RSAPrimeData_t structure
                                  that is used for the Exp operation
  @param[in] hashMode			- The hash function to be used.
  @param[in] ClientPrvKey_ptr1	- A pointer to the First Private key octet string number
  @param[in] ClientPrvKeySize1	- The First Private key Size, in bytes
  @param[in] ClientPrvKey_ptr2	- A pointer to the Second Private key octet string
  @param[in] ClientPrvKeySize2	- The Second Private key Size, in bytes
  @param[in] ServerPubKey_ptr1  - A pointer to the First Server public key octet string
  @param[in] ServerPubKeySize1  - The First Server Public key Size, in bytes
  @param[in] ServerPubKey_ptr2  - A pointer to the Second Server public key octet string
  @param[in] ServerPubKeySize2  - The Second Server Public key Size, in bytes
  @param[in] Prime_ptr			- A pointer to the Prime octet string
  @param[in] PrimeSize			- The size of the Prime string
  @param[in] DerFunc_mode		- The type of function to derive the secret key to the key data.
                                  We use an ASN.1-based function or a Hash concatenation function mode.
  @param[in] OtherInfo          - The pointer to predefined structure, containing pointers and sizes of optional data
                                  shared by two entities intended to share the secret value. This argument (structure)
							      and also its members are optional (if any is not need - set its pointer and size to NULL).
							      Note: OtherInfo not includes Counter, because the Counter is calculated inside the
							      function .
  @param[out] SecretKeyData_ptr - A pointer to the secret key octet string.
				                  This buffer should be at least 2*PrimeSize bytes.

  @param[in] SecretKeyDataSizeBitsNeeded - Specifies the derived Secret Key data size needed in Bits. This value,
                                  converted to bytes, cannot be larger than 2*PrimeSize (required by implementation).
								  The size of buffer for secret key data (in bytes) must be not less, than
								  2*PrimeSize (also required by implementation).

  @return CRYSError_t - On success the value CRYS_OK is returned, and on failure an ERROR as defined in CRYS_DH_error.h:
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR;
						CRYS_DH_SECRET_KEY_SIZE_NEEDED_BIGGER_THAN_PRIME_SIZE;
						CRYS_DH_X942_HYBRID_SIZE1_BUFFER_ERROR;
                        CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
						CRYS_DH_INVALID_ARGUMENT_SIZE_ERROR;
					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
						CRYS_DH_ARGUMENT_GENERATOR_SMALLER_THAN_ZERO_ERROR;
						CRYS_DH_ARGUMENT_PRIME_SMALLER_THAN_GENERATOR_ERROR;
						CRYS_DH_INVALID_L_ARGUMENT_ERROR;
						CRYS_DH_ARGUMENT_PRV_SIZE_ERROR;
					CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
*/
/*CEXPORT_C CRYSError_t CRYS_DH_X942_HybridGetSecretData(
	                                          CRYS_DHUserPubKey_t  *tmpUserPubKey_ptr,
                                              CRYS_DHPrimeData_t   *tmpPrimeData_ptr,
                                              CRYS_DH_HASH_OpMode_t hashMode,
                                              DxUint8_t            *ClientPrvKey_ptr1,
                                              DxUint16_t            ClientPrvKeySize1,
                                              DxUint8_t            *ClientPrvKey_ptr2,
                                              DxUint16_t            ClientPrvKeySize2,
                                              DxUint8_t            *ServerPubKey_ptr1,
                                              DxUint16_t            ServerPubKeySize1,
                                              DxUint8_t            *ServerPubKey_ptr2,
                                              DxUint16_t            ServerPubKeySize2,
                                              DxUint8_t            *Prime_ptr,
                                              DxUint16_t            PrimeSize,
                                              CRYS_DH_DerivationFunc_Mode DerivFunc_mode,
											  CRYS_DH_OtherInfo_t  *otherInfo_ptr,
                                              DxUint8_t            *SecretKeyData_ptr,
                                              DxUint16_t            SecretKeyDataSizeBitsNeeded )

{*/
  /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   //CRYSError_t Error;

   /*The return Secret key size from the PKCS#3 function*/
   /*The assignment to Prime size is according to the real size of the buffer SecretKeyData_ptr*/
   //DxUint16_t SecretKeyDataSize1 = PrimeSize;
   //DxUint16_t SecretKeyDataSize2 = PrimeSize;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   //Error = CRYS_OK;

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   //RETURN_IF_DH_UNSUPPORTED( tmpUserPubKey_ptr, tmpPrimeData_ptr, Error/*hashMode*/ ,
     //                        ClientPrvKey_ptr1, ClientPrvKeySize1, ClientPrvKey_ptr2,
       //                      ClientPrvKeySize2, ServerPubKey_ptr1, ServerPubKeySize1,
         //                    ServerPubKey_ptr2, ServerPubKeySize2, Prime_ptr, PrimeSize,
           //                  Error/*DerFunc_mode*/, SecretKeyData_ptr, SecretKeyDataSizeBitsNeeded,
             //                SecretKeyDataSize1, SecretKeyDataSize2,
               //              Error, Error, Error, Error );

//   #ifndef CRYS_NO_HASH_SUPPORT
  // #ifndef CRYS_NO_PKI_SUPPORT
//   #ifndef CRYS_NO_DH_SUPPORT

   /* ............... checking the parameters validity ................... */
   /* -------------------------------------------------------------------- */

   /*check that the derived secret key needed is not zero*/
  // if(SecretKeyDataSizeBitsNeeded == 0)
    //   return CRYS_DH_SECRET_KEY_SIZE_NEEDED_ERROR;

   /*check that the derived secret key needed is smaller than PrimeSize*/
   /*if(SecretKeyDataSizeBitsNeeded > 2*PrimeSize*8)
       return CRYS_DH_SECRET_KEY_SIZE_NEEDED_BIGGER_THAN_PRIME_SIZE;


	 Error = CRYS_DH_PKCS3_GetSecretKey(
		                            ClientPrvKey_ptr1,
				                    ClientPrvKeySize1,
				                    ServerPubKey_ptr1,
				                    ServerPubKeySize1,
				                    Prime_ptr,
				                    PrimeSize,
								    tmpUserPubKey_ptr,
                                    tmpPrimeData_ptr,
				                    SecretKeyData_ptr,
				                    &SecretKeyDataSize1);

	if(Error != CRYS_OK)
		return Error;
*/
    /*check that there will not be a memory overrun in SecretKeyData_ptr*/
  /*  if(SecretKeyDataSize1 > PrimeSize)
        return CRYS_DH_X942_HYBRID_SIZE1_BUFFER_ERROR;

	Error = CRYS_DH_PKCS3_GetSecretKey(ClientPrvKey_ptr2,
				                    ClientPrvKeySize2,
				                    ServerPubKey_ptr2,
				                    ServerPubKeySize2,
				                    Prime_ptr,
				                    PrimeSize,
				                    tmpUserPubKey_ptr,
									tmpPrimeData_ptr,
				                    &SecretKeyData_ptr[SecretKeyDataSize1],
				                    &SecretKeyDataSize2);

	if(Error != CRYS_OK)
		return Error;
*/
	/*Derive the secret key according to the secret key size in bits argument */
/*	 Error = CRYS_DH_KeyDerivFunc(SecretKeyData_ptr,
				                     (DxUint16_t)(SecretKeyDataSize1 + SecretKeyDataSize2),
									 otherInfo_ptr,
								     hashMode,
									 DerivFunc_mode,
				                     (DxUint8_t *)tmpPrimeData_ptr->DataIn,
				                     (SecretKeyDataSizeBitsNeeded + 7)>>3 );


	if(Error != CRYS_OK)
		return Error;

	DX_VOS_FastMemCpy(SecretKeyData_ptr, (DxUint8_t *)tmpPrimeData_ptr->DataIn, (SecretKeyDataSizeBitsNeeded + 7)>>3);

	return Error;

#endif*/ /*CRYS_NO_DH_SUPPORT*/
//#endif /*CRYS_NO_PKI_SUPPORT*/
//#endif /*CRYS_NO_HASH_SUPPORT*/

//}/* END OF CRYS_DH_X942_HybridGetSecretData */
