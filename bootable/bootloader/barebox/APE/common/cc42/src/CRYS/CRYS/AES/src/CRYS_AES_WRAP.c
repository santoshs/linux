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
#include "DX_VOS_Errors.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_AES_Local.h"
#include "CRYS_AES_error.h"

#include "LLF_AES.h"

/************************ Defines ******************************/

/* this macro is required to remove compilers warnings if the AES is not supported */
#ifdef CRYS_NO_AES_SUPPORT
#define RETURN_IF_AES_WRAP_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m , n , o , p , q , r )\
 (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0;(m)=0;(n)=0;(o)=(0);(p)=(0);(q)=(0);(r)=(0); \
 (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=j;(k)=(k);(l)=(l);(m)=(m);(n)=(n); \
  (o)=(o);(p)=(p);(q)=(q);(r)=(r); \
   return CRYS_AES_IS_NOT_SUPPORTED
#else  /* !CRYS_NO_AES_SUPPORT */
#define RETURN_IF_AES_WRAP_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m , n , o , p , q , r )
#endif /* !CRYS_NO_AES_SUPPORT */

/* canceling the lint warning:
   Unusual pointer cast (incompatible indirect types) */
/*lint --e{740} */

/************************ Enums *****************************************/

/************************ Typedefs **************************************/

/************************ Global Data ***********************************/

/* Define CRYS_AES_WRAP_IV according to AES WRAP standard rfc3394 and
   CMLA v1.0-05-12-21 definitions */
const DxUint8_t	CRYS_AES_WRAP_RFC3394_IV[] = {0xA6,0xA6,0xA6,0xA6,0xA6,0xA6,0xA6,0xA6};

/************* Private function prototype *******************************/

/************************ Public Functions ******************************/
#ifndef _INTERNAL_CRYS_NO_AES_WRAP_SUPPORT
/**************************************************************************
 *	              CRYS_AES_Wrap function                                 *
 **************************************************************************/
/**
   @brief  The CRYS_AES_Wrap function implements the following algorithm
           (rfc3394, Sept. 2002):

   Inputs:  Plaintext DataIn, n 64-bit values {P1, P2, ..., Pn},
            KeyData, K (the KEK).
   Outputs: Ciphertext, WrapDataOut (n+1) 64-bit values {C0, C1, ..., Cn}.

   Steps:
		   1. Initialize variables.
		       Set A = IV, an initial value (see 2.2.3)
		       For i = 1 to n
		           R[i] = P[i]
		   2. Calculate intermediate values.
		       For j = 0 to 5
		           For i=1 to n
		               B = AES(K, A | R[i])
		               A = MSB(64, B) ^ t ,
		                    where: t = (n*j)+i  and  "^"  is the  XOR  operation.
		               R[i] = LSB(64, B)
		   3. Output the result C.
		       Set C[0] = A
		       For i = 1 to n
		           C[i] = R[i].

   @param[in]  DataIn_ptr - A pointer to plain text data to be wrapped
                            NOTE: Overlapping between the data input and data output buffer
                                  is not allowed, except the inplace case that is legal .
   @param[in]  DataInLen  - Length of data in bytes. DataLen must be multiple of
                            8 bytes and  must be in range [8,2^28].
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
CEXPORT_C  CRYSError_t CRYS_AES_Wrap (
                               DxUint8_t            *DataIn_ptr,      /*in*/
                               DxUint32_t            DataInLen,       /*in*/
                               CRYS_AES_Key_t        KeyData,         /*in*/
                               CRYS_AES_KeySize_t    KeySize,         /*in*/
							   DxInt8_t              isSecretKey,     /*in*/
                               DxUint8_t            *WrapDataOut_ptr, /*out*/
                               DxUint32_t           *WrapDataLen_ptr  /*in/out*/ )
{
   /*****************  LOCAL DECLARATIONS  ********************************/

   CRYSError_t	Error = CRYS_OK;

   DxInt32_t	NumOfBlocks;
   DxInt32_t	i, j;
   DxUint32_t   *A_ptr, *B_ptr;
   AESContext_t *AEScontext_ptr;
   DxUint32_t	DataSize = 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES;
   DxUint32_t   T,baseT;
   DxUint32_t   TempBuff[4*CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS + (sizeof(AESContext_t) + 3)/4 ];
   /* Aes key size bytes */
   DxUint32_t KeySizeBytes;



   /* -------------------------------------------------------------------- */
   /* ............... If not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AES_WRAP_UNSUPPORTED( DataIn_ptr , DataInLen , KeyData ,
                              KeySize , isSecretKey , WrapDataOut_ptr , WrapDataLen_ptr ,
                              NumOfBlocks , i, j, A_ptr, B_ptr, AEScontext_ptr,
                              DataSize, T, TempBuff[0] , KeySizeBytes , Error );

#ifndef CRYS_NO_AES_SUPPORT


   /*----------------------------------------------------------------------*/
   /*            Check input parameters                                    */
   /*----------------------------------------------------------------------*/

   /* Check input pointers */

   if(DataIn_ptr == DX_NULL)
      return CRYS_AES_WRAP_ILLEGAL_DATA_PTR_ERROR;

   /* Check isSecretKey value */
   if( isSecretKey != DX_FALSE && isSecretKey != DX_TRUE )
      return CRYS_AES_WRAP_IS_SECRET_KEY_FLAG_ILLEGAL_ERROR;

   if(KeyData == DX_NULL && isSecretKey == DX_FALSE)
      return CRYS_AES_WRAP_ILLEGAL_KEY_PTR_ERROR;

   if(WrapDataOut_ptr == DX_NULL)
      return CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_PTR_ERROR;

   if(WrapDataLen_ptr == DX_NULL)
      return CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_LEN_PTR_ERROR;

   /* Check length of input parameters */
   if(DataInLen % CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES)
      return CRYS_AES_WRAP_DATA_LENGTH_ERROR;

   if(DataInLen < CRYS_AES_WRAP_DATA_MIN_SIZE_IN_BYTES)
      return CRYS_AES_WRAP_DATA_LENGTH_ERROR;

   if(DataInLen > CRYS_AES_WRAP_DATA_MAX_SIZE_IN_BYTES)
      return CRYS_AES_WRAP_DATA_LENGTH_ERROR;

   if(KeySize >= CRYS_AES_KeySizeNumOfOptions)
      return CRYS_AES_WRAP_KEY_LENGTH_ERROR;

   /* Check wrapped data buffer length */
   if( *WrapDataLen_ptr < DataInLen + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES)
      return CRYS_AES_WRAP_ILLEGAL_WRAP_DATA_LENGTH_ERROR;

    /* Check that there is no overlapping between the data input and data output buffer
     except the inplace case that is legal */
   if ( DataIn_ptr > WrapDataOut_ptr &&
	    DataIn_ptr < WrapDataOut_ptr + DataInLen + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES )
            return CRYS_AES_WRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR;

   if ( DataIn_ptr < WrapDataOut_ptr &&
	    DataIn_ptr > WrapDataOut_ptr - DataInLen )
            return CRYS_AES_WRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR;


   /* -------------------------------------------------------------------- */
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* Initialize A_ptr, B_ptr and AES context */
   A_ptr = TempBuff;
   B_ptr = A_ptr + 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS;
   AEScontext_ptr = (AESContext_t*)( A_ptr + 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS );

   /* in case isSecretKey == DX_FALSE set KeySizeBytes value and copy AES key into Context */
   if( isSecretKey == DX_FALSE )
   {
	   /* get AES_Key size in bytes */
	   switch( KeySize )
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

			default:
				return	CRYS_AES_WRAP_KEY_LENGTH_ERROR; /*for preventing compiler warnings*/
	   }

       DX_VOS_FastMemCpy(AEScontext_ptr->AESContextKey, KeyData , KeySizeBytes );
   }

   AEScontext_ptr->AESContextKeySize  = KeySize;
   AEScontext_ptr->EncryptDecryptFlag = CRYS_AES_Encrypt;
   AEScontext_ptr->OperationMode      = CRYS_AES_ECB_mode;
   AEScontext_ptr->is_secret_key      = isSecretKey;
   AEScontext_ptr->lastBlockSize      = 0x0;
   AEScontext_ptr->remainSize      = 0x0;

   Error = LLF_AES_InitInit( AEScontext_ptr);

   if(Error != CRYS_OK)
	goto EndWithError;

   /* Initialize a number of 64-bit blocks */
   NumOfBlocks = (DxInt32_t)(DataInLen/CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);


   /*********************   FUNCTION LOGIC  ********************************/

   /* Copy IV into A */
   DX_VOS_FastMemCpy((void *)A_ptr, (void*)CRYS_AES_WRAP_RFC3394_IV, CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

   /* Copy input data into output buffer for next operations */
   DX_VOS_FastMemCpy( WrapDataOut_ptr + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES, DataIn_ptr, DataInLen );


   /* Calculate intermediate values */
   baseT = 0x0;
   for(j = 0; j <= 5; j++)
   {
      for(i = 1; i <= NumOfBlocks; i++)
      {
         DX_VOS_FastMemCpy( (void *)(A_ptr+CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS),
                      &WrapDataOut_ptr[i*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES],
                      CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES );


         Error = LLF_AES_Block( AEScontext_ptr,
                                (DxUint8_t*)A_ptr,
                                DataSize,
                                (DxUint8_t*)B_ptr );

         if(Error != CRYS_OK)
		goto EndWithError;

         /* A_ptr = MSB(64, B)^T, where: T = (n*j)+i */

         #ifdef BIG__ENDIAN
			T = (DxUint32_t)(baseT + i);
         #else
		 /* Calculate T and reverse its endian for XORing */
		T = CRYS_COMMON_REVERSE32((DxUint32_t)(baseT + i));
         #endif
         A_ptr[0] = B_ptr[0];
         A_ptr[1] = B_ptr[1] ^ T;

         /* WrapData[i] = R[i] = LSB(64, B); */
         DX_VOS_FastMemCpy( &WrapDataOut_ptr[i*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES],
                     (void*)(&B_ptr[CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS]),
                     CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES );

      } /* END for(i) */
	  baseT = baseT + NumOfBlocks;
   } /* END for(j) */

   /* Copy A into 0-block of the wrapped data */
   DX_VOS_FastMemCpy(WrapDataOut_ptr, (void*)A_ptr, CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

   /* Set the actual length of the output data in bytes */
   *WrapDataLen_ptr = (DxUint32_t)((NumOfBlocks + 1) * CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

   goto End;

EndWithError:
      /* Clean wrapped data buffer  */
	DX_VOS_MemSetZero(WrapDataOut_ptr, DataInLen + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

End:
    DX_VOS_MemSetZero(TempBuff, sizeof(TempBuff));

	return Error;

#endif /* End of CRYS_NO_AES_SUPPORT */

}/* End of CRYS_AES_Wrap */

#endif /*_INTERNAL_CRYS_NO_AES_WRAP_SUPPORT */

#ifndef _INTERNAL_CRYS_NO_AES_UNWRAP_SUPPORT



/**************************************************************************
 *	              CRYS_AES_Unwrap function                                *
 **************************************************************************/
/**
   @brief  The CRYS_AES_Unwrap function performs inverse AES_Wrap transformation
           and implements the following algorithm (rfc3394, Sept. 2002):

   Inputs:  Ciphertext, WrapDataIn (n+1) 64-bit values {C0, C1, ..., Cn}, and
            K  - key (the KEK).
   Outputs: Plaintext, DataOut n 64-bit values {P1, P2, ..., Pn}.

   Steps:
		   1. Initialize variables:
		       Set A = C[0]
		       For i = 1 to n
		           R[i] = C[i]
		   2. Compute intermediate values:
		       For j = 5 to 0
		           For i = n to 1
		               B = AES-1(K, (A ^ t) | R[i]) ,
		                    where:  t = n*j+i  and  "^" is the  XOR  operation.
		               A = MSB(64, B)
		               R[i] = LSB(64, B)
		   3. Output results:
		   If A is an appropriate initial value (see 2.2.3), then
		       For i = 1 to n
		           P[i] = R[i]
		   Else
		       Return an error.

   @param[in]  WrapDataIn_ptr - A pointer to wrapped data to be unwrapped
                                NOTE: Overlapping between the data input and data output buffer
                                      is not allowed, except the inplace case that is legal .
   @param[in]  WrapDataInLen  - Length of wrapped data in bytes. DataLen must be multiple of
                                8 bytes and  must be in range [16, (2^28+8)].
   @param[in]  KeyData        - A pointer to  key data (key encryption key - KEK).
   @param[in]  KeySize        - Enumerator variable, defines length of key.
   @param[in]  isSecretKey    - Variable, defining is secret key used (1) or not (0).
   @param[out] DataOut_ptr    - A pointer to buffer for output of unwrapped data.
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

CEXPORT_C  CRYSError_t CRYS_AES_Unwrap(
                                   DxUint8_t            *WrapDataIn_ptr,  /*in*/
                                   DxUint32_t            WrapDataInLen,   /*in*/
                                   CRYS_AES_Key_t        KeyData,         /*in*/
                                   CRYS_AES_KeySize_t    KeySize,         /*in*/
								   DxInt8_t              isSecretKey,     /*in*/
                                   DxUint8_t            *DataOut_ptr,     /*out*/
                                   DxUint32_t           *DataOutLen_ptr   /*in/out*/ )

{
  /*****************  LOCAL DECLARATIONS  ********************************/

   CRYSError_t	Error = CRYS_OK;

   DxInt32_t	NumOfBlocks;
   DxInt32_t	i, j;
   DxUint32_t   *A_ptr, *B_ptr;
   AESContext_t  *AEScontext_ptr;
   DxUint32_t	DataSize = 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES;
   DxUint32_t   T,baseT;
   DxUint32_t   TempBuff[4*CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS + (sizeof(AESContext_t) + 3)/4 ];
   /* Aes key size bytes */
   DxUint32_t   KeySizeBytes;


   /* -------------------------------------------------------------------- */
   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_AES_WRAP_UNSUPPORTED( WrapDataIn_ptr , WrapDataInLen , KeyData ,
                              KeySize , DataOut_ptr , DataOutLen_ptr ,
                              NumOfBlocks , i, j, A_ptr, AEScontext_ptr, B_ptr,
                              DataSize, T, isSecretKey , TempBuff[0] , KeySizeBytes , Error );

#ifndef CRYS_NO_AES_SUPPORT

   /*----------------------------------------------------------------------*/
   /*            Check input parameters                                    */
   /*----------------------------------------------------------------------*/

   /* Check input pointers */

   if(WrapDataIn_ptr == DX_NULL)
      return CRYS_AES_UNWRAP_ILLEGAL_WRAP_DATA_PTR_ERROR;

   /* Check isSecretKey value */
   if( isSecretKey != DX_FALSE && isSecretKey != DX_TRUE )
        return CRYS_AES_UNWRAP_IS_SECRET_KEY_FLAG_ILLEGAL_ERROR;

   /* Check key pointer */
   if(KeyData == DX_NULL && isSecretKey == DX_FALSE)
      return CRYS_AES_UNWRAP_ILLEGAL_KEY_PTR_ERROR;

   if(DataOut_ptr == DX_NULL)
      return CRYS_AES_UNWRAP_ILLEGAL_DATA_PTR_ERROR;

   if(DataOutLen_ptr == DX_NULL)
      return CRYS_AES_UNWRAP_ILLEGAL_DATA_LEN_PTR_ERROR;

   /* Check length of input parameters */
   if(WrapDataInLen % CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES)
      return CRYS_AES_UNWRAP_WRAP_DATA_LENGTH_ERROR;

   if(WrapDataInLen < (CRYS_AES_WRAP_DATA_MIN_SIZE_IN_BYTES + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES))
      return CRYS_AES_UNWRAP_WRAP_DATA_LENGTH_ERROR;

   if(WrapDataInLen > (CRYS_AES_WRAP_DATA_MAX_SIZE_IN_BYTES + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES))
      return CRYS_AES_UNWRAP_WRAP_DATA_LENGTH_ERROR;

   if(KeySize >= CRYS_AES_KeySizeNumOfOptions )
      return CRYS_AES_UNWRAP_KEY_LEN_ERROR;

   /* Check unwrapped data buffer length */
   if( *DataOutLen_ptr < WrapDataInLen - CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES)
      return CRYS_AES_UNWRAP_ILLEGAL_DATA_LENGTH_ERROR;

  /* Check that there is no overlapping between the data input and data output buffer
     except the inplace case that is legal */
   if ( WrapDataIn_ptr > DataOut_ptr &&
	    WrapDataIn_ptr < DataOut_ptr + WrapDataInLen - CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES )
            return CRYS_AES_UNWRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR;

   if ( WrapDataIn_ptr < DataOut_ptr &&
	    WrapDataIn_ptr > DataOut_ptr - WrapDataInLen )
            return CRYS_AES_UNWRAP_DATA_OUT_DATA_IN_OVERLAP_ERROR;


   /* -------------------------------------------------------------------- */
   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* Initialize A_ptr, B_ptr and AES context */
   A_ptr = TempBuff;
   B_ptr = A_ptr + 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS;
   AEScontext_ptr = (AESContext_t*)( A_ptr + 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS );

   /* in case isSecretKey == DX_FALSE set KeySizeBytes value and copy AES key into Context */
   if( isSecretKey == DX_FALSE )
   {
	   /* get AES_Key size in bytes */
	   switch( KeySize )
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

			default:
				return CRYS_AES_WRAP_KEY_LENGTH_ERROR; /*for preventing compiler warnings*/

	   }

       DX_VOS_FastMemCpy(AEScontext_ptr->AESContextKey, KeyData , KeySizeBytes );
   }

   AEScontext_ptr->AESContextKeySize  = KeySize;
   AEScontext_ptr->EncryptDecryptFlag = CRYS_AES_Decrypt;
   AEScontext_ptr->OperationMode      = CRYS_AES_ECB_mode;
   AEScontext_ptr->is_secret_key      = isSecretKey;
   AEScontext_ptr->lastBlockSize      = 0x0;
   AEScontext_ptr->remainSize      = 0x0;

   /* calculate n - number of 64-bit blocks of unwrapped data */
   NumOfBlocks = (DxInt32_t)(WrapDataInLen / CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES - 1);

   Error = LLF_AES_InitInit( AEScontext_ptr);

   if(Error != CRYS_OK)
	   goto EndWithError;


   /*********************   FUNCTION LOGIC  ********************************/

   /* Copy C[0] into A */
   DX_VOS_FastMemCpy((void *)A_ptr, WrapDataIn_ptr, CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

   /* Copy remining input data into output buffer for next operations */
   DX_VOS_FastMemCpy( DataOut_ptr /*+ CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES*/,
                   WrapDataIn_ptr + CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES,
                   WrapDataInLen - CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES );

   /* Calculate intermediate values */
   baseT = (NumOfBlocks << 2 ) + NumOfBlocks; /*NumOfBlocks * 5 */
   for(j = 5; j >= 0; j--)
   {
      for(i = NumOfBlocks; i >= 1; i--)
      {
	#ifdef BIG__ENDIAN
		T = (DxUint32_t)(baseT + i);
	#else
		/* Calculate T and reverse its endian for XORing */
		T = CRYS_COMMON_REVERSE32((DxUint32_t)(baseT + i));
         #endif

         /* Calculate (A^T)|R[i]   */
         A_ptr[1] = A_ptr[1] ^ T;
         DX_VOS_FastMemCpy((void *)(&A_ptr[2]), DataOut_ptr + (i-1)*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES,
                         CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES );

         Error = LLF_AES_Block( AEScontext_ptr,
                                (DxUint8_t*)A_ptr,
                                DataSize,
                                (DxUint8_t*)B_ptr );
         if(Error != CRYS_OK)
		goto EndWithError;

         /* A = MSB(64, B)   */
         A_ptr[0] = B_ptr[0];
         A_ptr[1] = B_ptr[1];

         /* WrapData[i] = R[i] = LSB(64, B); */
         DX_VOS_FastMemCpy( &DataOut_ptr[(i-1)*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES],
                         (void*)(&B_ptr[CRYS_AES_WRAP_BLOCK_SIZE_IN_WORDS]),
                         CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES );

      } /* for(i) */
	  baseT = baseT - NumOfBlocks;
   } /* for(j) */

   /* Check that A = AES_WRAP_IV , else goto end with error */
   Error = DX_VOS_MemCmp( (void*)A_ptr, CRYS_AES_WRAP_RFC3394_IV, CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES)==(DxInt32_t)DX_SUCCESS ?
			DX_SUCCESS : CRYS_AES_UNWRAP_FUNCTION_FAILED_ERROR;

   if( Error != DX_SUCCESS )
   {
		goto EndWithError;
   }

   /* Set the actual length of the output data in bytes */
   *DataOutLen_ptr = (DxUint32_t)(NumOfBlocks * CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

   goto End;

EndWithError:

   /* Clean plain data buffer  */
	DX_VOS_MemSetZero(DataOut_ptr, WrapDataInLen - CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES);

End:
    DX_VOS_MemSetZero(TempBuff, sizeof(TempBuff));

	return Error;

#endif /* End of CRYS_NO_AES_SUPPORT */

}/* End of CRYS_AES_Uwnrap */

#endif /* _INTERNAL_CRYS_NO_AES_UNWRAP_SUPPORT */
