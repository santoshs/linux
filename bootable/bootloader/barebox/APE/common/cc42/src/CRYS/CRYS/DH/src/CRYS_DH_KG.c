
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  24 Oct. 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains functions for generation and checking of the
   *         Diffie-Hellman (DLP) domain and public key parameters
   *
   *  \version CRYS_DH_KG.c#1:csrc:5
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/
#include "DX_VOS_Mem.h"
#include "CRYS_DH_errors.h"
#include "CRYS.h"
#include "CRYS_COMMON.h"
#include "CRYS_DH_local.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON_math.h"

#ifndef CRYS_NO_PKI_SUPPORT
#include "LLF_PKI_RSA.h"
#endif

/************************ Defines *******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* run time LLF_PKI_mode  definition */
#ifdef DX_PKI_MODE_DMA_ONLY
#define  LLF_PKI_mode LLF_PKI_DMA_MODE_TYPE
#else
#ifdef DX_PKI_MODE_DIRECT_ONLY
#define  LLF_PKI_mode LLF_PKI_DIRECT_MODE_TYPE
#else
extern DxUint32_t LLF_PKI_mode;
#endif
#endif


/************************ Enums *********************************/

/************************ macros ********************************/

/** @brief This macro is required to remove compilers warnings if the HASH or PKI is not supported */

#if (defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_PKI_SUPPORT) || defined(CRYS_NO_DH_SUPPORT))
#define RETURN_IF_DH_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m, n , o ,p , q, a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1, k1, l1, m1, n1 , o1 ,p1 , q1 ) \
  (a)=0;(b)=0;(c)=0;(d)=0;(e)=0;(f)=0;(g)=0;(h)=0;(i)=0;(j)=0;(k)=0;(l)=0;(m)=0;(n)=0;(o)=0;(p)=0;(q)=0; \
  (a1)=0;(b1)=0;(c1)=0;(d1)=0;(e1)=0;(f1)=0;(g1)=0;(h1)=0;(i1)=0;(j1)=0; \
  (a)=(a);(b)=(b);(c)=(c);(d)=(d);(e)=(e);(f)=(f);(g)=(g);(h)=(h);(i)=(i);(j)=(j);(k)=(k);(l)=(l); (m)=(m);(n)=(n);(o)=(o);(p)=(p);(q)=(q);\
  (a1)=(a1);(b1)=(b1);(c1)=(c1);(d1)=(d1);(e1)=(e1);(f1)=(f1);(g1)=(g1);(h1)=(h1);(i1)=(i1);(j1)=(j1);(k1)=(k1);(l1)=(l1); (m1)=(m1);(n1)=(n1);(o1)=(o1);(p1)=(p1);(q1)=(q1); \
  return CRYS_DH_IS_NOT_SUPPORTED
#else
#define RETURN_IF_DH_UNSUPPORTED( a , b , c , d , e , f , g , h , i , j , k , l , m, n , o ,p , q,a1 , b1 , c1 , d1 , e1 , f1 , g1 , h1 , i1 , j1,  k1, l1, m1, n1 , o1 ,p1 , q1)
#endif /* !CRYS_NO_HASH_SUPPORT && ! CRYS_NO_PKI_SUPPORT && ! CRYS_NO_DH_SUPPORT */

/*********************** Global data  ***************************/


/******************************************************************************************/
/************************         Private Functions          ******************************/
/******************************************************************************************/

#if !(defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_PKI_SUPPORT) || defined(CRYS_NO_DH_SUPPORT))

/** @brief The function finds prime number Q for key generation according to X9.42-2001.
 *
 * @param[in]  QSizeBits          - The size of order of generator in bits. According to ANSI X9.43:
 *                                  m must be multiple of 32 bits and m >= 160. According to ANSI X9.30-1:
 *                                  m = 160 bit. Because Q is prime factor of P-1, it must be < (P-1)/2.
 *                                  In our implementation required, that orderQSize <= modPSizeBytes/2.
 * @param[in]  seedSizeBits      - The  seed size in bits. Requirements:
 *                                  seedSize >= orderQSize and seedSize <= modPSizeBytes ( the last is required
 *                                  by our implementation).
 * @param[in]  generateSeed       - The  flag defining whether the seed to be generated (1) or not (0),
 * @param[out] Q_ptr              - The pointer to the order Q of generator. The buffer must be aligned to 4 bytes.
 * @param[out] S_ptr              - The random seed used for generation of primes. The buffer must be aligned to 4 bytes.
 * @param[in]  TempBuff1_ptr      - The temp buffer of size not less than max modulus size, aligned to 4 bytes.
 * @param[in]  TempBuff2_ptr      - The large temp buffer (aligned to 4 bytes) of size:
 *                                    - on HW platform not less than 8*CRYS_DH_MAX_MOD_BUFFER_SIZE_IN_WORDS.
 *                                    - on SW platform not less than 41*CRYS_DH_MAX_MOD_BUFFER_SIZE_IN_WORDS.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a predefined error code.
 *
 *
 */
static CRYSError_t CRYS_DH_X942_FindPrimeQ(
                          DxUint32_t          QsizeBits,           /*in */
                          DxUint32_t          seedSizeBits,       /*in */
                          DxUint32_t          generateSeed,        /*in */
                          DxUint8_t          *Q_ptr,               /*out*/
                          DxUint8_t          *S_ptr,               /*out*/
                          DxUint32_t         *TempBuff1_ptr,       /*in */
                          DxUint32_t         *TempBuff2_ptr )      /*in - large buffer*/
{

   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* size of order in 160-bit blocks: M1 */
   DxUint32_t  M1;

   /* primality flag (if prime, then isPrime = 1, else 0 ) */
   DxInt8_t  isPrime;

   /* flag of first hash calculating */
   DxUint8_t  isFirst = 1;


   /* HASH input and result pointers */
   DxUint8_t  *hashDataIn1_ptr, *hashDataIn2_ptr;
   CRYS_HASH_Result_t   *hashRes1_ptr, *hashRes2_ptr;

   /* current data pointer and size */
   DxUint8_t  *current_ptr;
   DxUint32_t  currentSize, remainingSize;

   /* order size in bytes and in words */
   DxUint32_t  QsizeBytes, QsizeWords;

   /* exact seed size in bits and in words */
   DxUint32_t  seedSizeBytes, seedSizeWords;

   /* shift value (in bits) for adding counter to seed */
   DxUint32_t shift;

   /* loop counters */
   DxUint32_t i, j;

   /* temp buffers pointers */
   DxUint32_t  *TempBuff3_ptr;

   /* FUNCTION  LOGIC */

	Error = CRYS_OK;



   /* Step 1. Check input parameters */
   /*------------------------------- */

   /* check pointers: modP, generator and tempBuff. Note: other pointers may be NULL  */
   if( Q_ptr == DX_NULL ||
       S_ptr == DX_NULL ||
       TempBuff1_ptr == DX_NULL ||
       TempBuff2_ptr == DX_NULL  )
   {
	   return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
   }

   /* --------------------------------- */
   /*  Step 2.  Initializations         */
   /* --------------------------------- */

   /* order and seed sizes */
   QsizeBytes = (QsizeBits + 7) / 8;
   QsizeWords = (QsizeBits + 31) / 32;
   seedSizeWords = (seedSizeBits + 31) / 32;
   seedSizeBytes = (seedSizeBits + 7) / 8;

   /* temp buffers pointers */
   TempBuff3_ptr = TempBuff2_ptr + max(QsizeWords, seedSizeWords) + 2;

   /* order size M1 in 160-bit blocks (rounded up) */
   M1 =  (QsizeBits + CRYS_DH_SEED_MIN_SIZE_IN_BITS - 1) / CRYS_DH_SEED_MIN_SIZE_IN_BITS;

   /* zeroing  Q buffer */
   DX_VOS_MemSetZero( Q_ptr, QsizeBytes );

   /* set HASH pointers to temp buffer */
   hashDataIn1_ptr = (DxUint8_t*)TempBuff1_ptr;
   hashDataIn2_ptr = (DxUint8_t*)TempBuff2_ptr;
   hashRes1_ptr = (CRYS_HASH_Result_t*)TempBuff3_ptr;
   hashRes2_ptr = hashRes1_ptr + 1;

   /*------------------------------- */
   /* Step 3. Create random prime Q  */
   /*------------------------------- */

   /* reverse copy seed S into HASH input buffers  */
   if( generateSeed != 1 )
   {
	   CRYS_COMMON_ReverseMemcpy( hashDataIn1_ptr, S_ptr, seedSizeBytes );

	   if( seedSizeBits != CRYS_COMMON_GetCounterEffectiveSizeInBits(hashDataIn1_ptr, (DxUint16_t)seedSizeBytes) )
		   return CRYS_DH_INVALID_SEED_SIZE_ERROR;
   }

   /* shift to bit position of MSbit of the seed (as big endian number) */
   shift = 8*seedSizeBytes - seedSizeBits;

   /* initialize isPrime, orderSizeInBlocks, and Q buffer */
   isPrime = DX_FALSE;

   /* Step 3.1. Try Q candidates     */
   /*--------------------------------*/
   while( isPrime != DX_TRUE )
   {
	   /* Step 3.1.1. Create random seed  S  */

	   if( generateSeed == 1 )
	   {
	       /* generation of random vector */
		   Error = CRYS_RND_GenerateVector( (DxUint16_t)seedSizeBytes, S_ptr );

		   if( Error != CRYS_OK )
		       goto EndWithError;

		   /* Set the MS bit of S and shift MSByte to provide exact size of seed in bits */
		   S_ptr[0] = (S_ptr[0] | 0x80) >> shift;

		   /* copy seed S into HASH input buffers  */
           /* reverse copy seed into hashDataIn1  */
		   CRYS_COMMON_ReverseMemcpy( hashDataIn1_ptr, S_ptr, seedSizeBytes );
	   }

	   else if( isFirst == 0 )

	       return  CRYS_DH_PASSED_INVALID_SEED_ERROR;

        /* set current pointer and size for copying HASH results into TempBuff3 */
	   current_ptr = Q_ptr;
	   currentSize = CRYS_HASH_SHA1_DIGEST_SIZE_IN_BYTES;
	   remainingSize = QsizeBytes;


		/* Step 3.1.2. Create Q candidate:  For i=0 to M1 do:
		                                      Q = Q + (SHA1(S+i) XOR SHA1(S+M1+i))*(2^(160*i)) */
       for( i = 0; i < M1; i++ )
       {
		    if( i != 0 )
            {
				/* increment hashDataIn1 by 1 starting from second cycle */
	            CRYS_COMMON_IncLsbUnsignedCounter(
	                                       (DxUint32_t*)hashDataIn1_ptr,
	                                       1 /*<< shift*/,
										   (DxUint8_t)(seedSizeBytes+3)/4 );
            }

            /* convert hashDataIn1_ptr to big endianness and set it into hashDataIn1_ptr */
			CRYS_COMMON_ReverseMemcpy( hashDataIn2_ptr, hashDataIn1_ptr, seedSizeBytes );

			/* calculate first HASH result */
		Error = CRYS_HASH(
		             CRYS_HASH_SHA1_mode,
                         hashDataIn2_ptr,
                         seedSizeBytes,
                         *hashRes1_ptr );

            if( Error != CRYS_OK )
               goto EndWithError;

		    /* copy hashDataIn1 into hashDataIn2 */
		    DX_VOS_FastMemCpy( hashDataIn2_ptr, hashDataIn1_ptr, seedSizeBytes );

			/* Calculate the hashDataIn2 by adding M1 to hashDataIn1 */
            CRYS_COMMON_IncLsbUnsignedCounter(
                                       (DxUint32_t*)hashDataIn2_ptr,
                                       M1 /*<< shift*/,
                                       (DxUint8_t)seedSizeWords );

			/* convert hashDataIn2_ptr to big endianness (in place) */
			CRYS_COMMON_ReverseMemcpy( hashDataIn2_ptr, hashDataIn2_ptr, seedSizeBytes );

             /* calculate  second HASH result */
		Error = CRYS_HASH(
		             CRYS_HASH_SHA1_mode,
                         hashDataIn2_ptr,
                         seedSizeBytes,
                         *hashRes2_ptr );

            if( Error != CRYS_OK )
               goto EndWithError;

            /* XOR HASH results */
            for( j = 0; j < CRYS_HASH_SHA1_DIGEST_SIZE_IN_WORDS; j++ )
            {
		(*hashRes1_ptr)[j] = (*hashRes1_ptr)[j] ^ (*hashRes2_ptr)[j];
            }

            /* set size for copying HASH result into Q buffer */
            if( remainingSize >= CRYS_DH_SEED_MIN_SIZE_IN_BYTES)
            {
		currentSize = CRYS_DH_SEED_MIN_SIZE_IN_BYTES;
            }
            else
		currentSize = remainingSize;

            /* reverse copy HASH result into current segment of Q (in big endian) */
            CRYS_COMMON_ReverseMemcpy( current_ptr,
                                       (DxUint8_t*)hashRes1_ptr + ( CRYS_DH_SEED_MIN_SIZE_IN_BYTES - currentSize ),
                                       currentSize );

            /* increment the current_ptr */
            current_ptr += currentSize;
            remainingSize -=  currentSize;
            isFirst = 0;

       } /* end of for() loop */

       /* update Q, by setting the first and last bits equal to 1 */
       Q_ptr[0] |= 0x01;         /* LS bit - little endian */
       current_ptr[-1] |= 0x80;  /* MS bit - little endian */


       /* Step 3.2. Perform primality tests on Q: 8 Miller-Rabin and 1 Lucas tests (X9.42-2001) */
       /*---------------------------------------------------------------------------------------*/

	   Error = LLF_PKI_RSA_primeTestCall(
			                              (DxUint32_t *)Q_ptr,
			                              QsizeBytes/4,
			                              8 /*rabinTestsCount*/,
			                              &isPrime,
			                              TempBuff2_ptr,
			                              CRYS_DH_PRIME_TEST_MODE );

        if( Error != CRYS_OK )
           goto EndWithError;

   } /* END of while() loop */


   /* End of function */

   return Error;

EndWithError:

    DX_VOS_MemSetZero( Q_ptr, QsizeBytes );
	DX_VOS_MemSetZero( S_ptr, seedSizeBytes );

    return Error;

} /* End of DX_DH_X942_FindPrimeQ */


/******************************************************************************************/
/**
 * @brief The function finds prime modulus P for key generation according to X9.42-2001.
 *
 *
 * @param[in]  modPSizeBits       - The  modulus (prime) P size in bits equal 256*n, where n >= 4.
 * @param[in]  QSizeBbytes        - The size of order of generator in bytes. Must be m >= 20 bytes and
 *                                  multiple of 4 bytes. According to ANSI X9.30-1: size = 20.
 * @param[in]  orderQSizeBits     - The size of order of generator in bits. Must be m >= 160 and
 *                                  multiple of 32 bits. According to ANSI X9.30-1: m = 160.
 * @param[in]  seedSizeBits       - The  seed size in bits (the size must be:  seedSizeBits >= 160,
 *                                  seedSizeBits <= modPSizeBits - 1 (the last required by implementation).
 * @param[out] P_ptr              - The prime modulus P of structure P = j*Q + 1, where Q is prime
 *                                  and j is an integer.The buffer must be aligned to 4 bytes.
 * @param[out] Q_ptr              - The pointer to the order Q of generator. The buffer must be aligned to 4 bytes.
 * @param[out] S_ptr              - The random seed used for generation of primes. The buffer must be aligned to 4 bytes.
 * @param[out] pgenCounter_ptr    - The pointer to counter of tries to generate the primes.
 * @param[in]  TempBuff1_ptr      - The temp buffer of size not less than max modulus size, aligned to 4 bytes.
 * @param[in]  TempBuff2_ptr      - The large temp buffer (aligned to 4 bytes) of size:
 *                                    - on HW platform not less than 8*CRYS_DH_MAX_MOD_BUFFER_SIZE_IN_WORDS.
 *                                    - on SW platform not less than 41*CRYS_DH_MAX_MOD_BUFFER_SIZE_IN_WORDS.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a predefined error code.
 *
 *
 */
static CRYSError_t CRYS_DH_X942_FindPrimeP(
                          DxUint32_t          modPsizeBits,        /*in */
                          DxUint32_t          orderQsizeBits,      /*in */
                          DxUint32_t          seedSizeBits,        /*in */
                          DxUint8_t          *P_ptr,               /*out*/
                          DxUint8_t          *Q_ptr,               /*out*/
                          DxUint8_t          *S_ptr,               /*out*/
                          DxUint32_t         *pgenCounter_ptr,     /*out*/
                          DxUint32_t         *TempBuff1_ptr,       /*in */
                          DxUint32_t         *TempBuff2_ptr )      /*in - large buffer */
{

   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* mod size in bytes and in words */
   DxUint32_t  modSizeBytes, modSizeWords;

   /* seed size in bytes and words */
   DxUint32_t seedSizeBytes;

   /* mod size in 160 bits blocks (rounded up) */
   DxUint32_t  L1;

   /* order sizes: M1 - in 160-bit blocks (rounded up) */
   DxUint32_t  orderSizeBytes, M1;

   /* flag of first hash calculating */
   DxUint8_t  isFirst = 1;

   /* primality flag (if prime, then isPrime = 1, else 0 ) */
   DxUint8_t  isPrime;

   /* HASH input and result pointers */
   DxUint8_t  *hashDataIn_ptr;
   CRYS_HASH_Result_t   *hashRes_ptr;

    /* current data pointer and size */
   DxUint8_t  *current_ptr;
   DxUint32_t  currentSize, remainingSize;

   CRYS_COMMON_CmpCounter_t  cmpRes;

   /* loop counter and carry */
   DxUint32_t i, carry;

   /* temp buffers pointers */
   DxUint32_t  *TempBuff3_ptr, *TempBuff4_ptr, *TempBuff5_ptr;

   /* FUNCTION  LOGIC */

	Error = CRYS_OK;


   /* --------------------------------- */
   /* Step 1. Check input parameters    */
   /*---------------------------------- */

   /* check pointers: modP, generator and tempBuff. Note: other pointers may be NULL  */
   if( P_ptr == DX_NULL ||
       Q_ptr == DX_NULL ||
       S_ptr == DX_NULL ||
       pgenCounter_ptr == DX_NULL ||
       TempBuff1_ptr == DX_NULL ||
       TempBuff2_ptr == DX_NULL )
   {
	   return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
   }

   /* --------------------------------- */
   /*  Step 2.  Initializations         */
   /* --------------------------------- */

   /* mod sizes in bytes */
   modSizeBytes = (modPsizeBits + 7) / 8;
   modSizeWords = (modPsizeBits + 31) / 32;
   /* mod size in 160 bit blocks */
   L1 = (modPsizeBits + CRYS_DH_SEED_MIN_SIZE_IN_BITS - 1) / CRYS_DH_SEED_MIN_SIZE_IN_BITS;

   /* order size: M1 - in 160-bit blocks (rounded up) */
   M1 = (orderQsizeBits + CRYS_DH_SEED_MIN_SIZE_IN_BITS - 1) / CRYS_DH_SEED_MIN_SIZE_IN_BITS;
   orderSizeBytes = (orderQsizeBits + 7) / 8;

   /* seedSize in bytes */
   seedSizeBytes = (seedSizeBits + 7) / 8;

   /* zeroing of P  */
   DX_VOS_MemSetZero( P_ptr, modSizeBytes + 2 );

   /* temp buffers pointers */
   TempBuff3_ptr = TempBuff2_ptr + modSizeWords + 2;
   TempBuff4_ptr = TempBuff3_ptr + 2*modSizeWords + 2;
   TempBuff5_ptr = TempBuff4_ptr + 2*modSizeWords + 2;

   /*------------------------------------------------------ */
   /* Step 3.   Create random prime P = (Q*J + 1)           */
   /*------------------------------------------------------ */

   /* set pgenCounter 0 */
   *pgenCounter_ptr = 0;

   /* set HASH pointers to temp buffer */
   hashDataIn_ptr = (DxUint8_t*)TempBuff1_ptr;
   hashRes_ptr = (CRYS_HASH_Result_t*)TempBuff4_ptr;   /* used as temp buffer */



    /* Calculating R = seed + 2*M1 , where  R is set in hashDataIn:
       copy the seed into hashDataIn_ptr ( big endian );
       set other bytes to 0; add M1 */

    CRYS_COMMON_ReverseMemcpy( hashDataIn_ptr, S_ptr, seedSizeBytes );
    DX_VOS_MemSetZero( hashDataIn_ptr + seedSizeBytes, modSizeBytes - seedSizeBytes );

    CRYS_COMMON_IncLsbUnsignedCounter(
                               (DxUint32_t*)hashDataIn_ptr,
                               2*M1 /*<< shift*/,
                               (DxUint8_t)(seedSizeBytes+3)/4 );

   /* initialize isPrime, orderSizeInBlocks, and Q buffer */
   isPrime = DX_FALSE;

   /* Step 3.1. Main loop - try P candidates */
   /*----------------------------------------*/
   while( isPrime != DX_TRUE )
   {
	   PLAT_WatchDogReset();
	   /* Step 3.1. Create P candidate:
		             For i=0 to L1 do:  P = P + SHA1(R+i) *(2^(160*i)) */

	   /* set current P pointer and size for copying HASH result into modulus P */
	   current_ptr = P_ptr;
	   currentSize = seedSizeBytes;
	   remainingSize = (modPsizeBits + 7) / 8;


       for( i = 0; i < L1; i++ )
       {
            /* Adding 1 to hashDataIn excluding the first hashing operation */
            if( isFirst != 1 )
            {
	            CRYS_COMMON_IncLsbUnsignedCounter(
	                                       (DxUint32_t*)hashDataIn_ptr,
	                                       1 /*<< shift*/,
	                                       (DxUint8_t)(seedSizeBytes+3)/4 );
            }

	    /* set 0 to isFirst */
		    isFirst = 0;

			/* reverse copy hashDataIn into temp3 buffer */
			CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)TempBuff3_ptr, hashDataIn_ptr, seedSizeBytes );

            /* calculate HASH result */
		Error = CRYS_HASH(
		             CRYS_HASH_SHA1_mode,
                         (DxUint8_t*)TempBuff3_ptr,
                         seedSizeBytes,
                         *hashRes_ptr );

            if( Error != CRYS_OK )
               goto EndWithError;

            /* set size for copying HASH result into P buffer */
            if( remainingSize >= CRYS_DH_SEED_MIN_SIZE_IN_BYTES)
            {
		currentSize = CRYS_DH_SEED_MIN_SIZE_IN_BYTES;
            }

            else
		currentSize = remainingSize;

            /* reverse copy HASH result into P (converted to little endian) */
            CRYS_COMMON_ReverseMemcpy( current_ptr,
                                       (DxUint8_t*)hashRes_ptr + ( CRYS_DH_SEED_MIN_SIZE_IN_BYTES - currentSize ),
                                       currentSize );

            /* increment the current_ptr */
            current_ptr += currentSize;
            remainingSize -=  currentSize;


       } /* end of j - loop */
       /*-----------------------------------------------------------------------*/

       /* update P candidate (TempBuff3_ptr) by setting the MS bit equal to 1 */
       current_ptr[- 1] |= 0x80;     /* in little endianness this is MS-byte */


       /* Step 3.2. Set P = P - (P mod 2*Q) + 1  */
       /*----------------------------------------*/

       /* set TempBuff3 = 2*Q. Note: Result size is large by 1 byte (and word), than Q size */
       carry = CRYS_COMMON_AddTwoLsbUint8Vectors(
                                           Q_ptr,
                                           Q_ptr,
                                           orderSizeBytes,
                                           (DxUint8_t*)TempBuff3_ptr );
       if( carry != 0 )
	   {
		   /* set 1 to one word after orderSizeBytes/4 words of TempBuff3 */
		   TempBuff3_ptr[orderSizeBytes / 4] = 1;
	   }
	   else
           TempBuff3_ptr[orderSizeBytes / 4] = 0;


       /* calculate TempBuff4 = P mod 2*Q */
       DX_VOS_MemSetZero( (DxUint8_t*)TempBuff4_ptr , modSizeBytes );

       LLF_PKI_RSA_Call_Div(
	            (DxUint32_t*)P_ptr,         /*numerator P*/
	            modSizeWords,               /*P_size in words*/
	            TempBuff3_ptr,              /*divider */
	            (orderSizeBytes+3)/4 + 1,   /*divider_size in words*/
	            TempBuff4_ptr,              /*ModRes_ptr*/
	            TempBuff2_ptr,              /*DivRes_ptr*/
	            TempBuff5_ptr               /*TempBuff_ptr - 2*N_size */ );

       /* subtract: P = P - TempBuff4 */
       CRYS_COMMON_SubtractUintArrays(
                                      (DxUint32_t*)P_ptr,
                                      TempBuff4_ptr,
                                      modSizeWords,
                                      (DxUint32_t*)P_ptr );

       /* add 1 to P */
       CRYS_COMMON_IncLsbUnsignedCounter(  (DxUint32_t*)P_ptr,
	                                       1,
	                                       (DxUint8_t)(modSizeBytes+3)/4 );

       /* check: if P > 2^(L-1), then perform step 3.3. */
       /*-----------------------------------------------*/

       /* set TempBuff5 = 2^(L-1): Note: L = modPsizeBits is multiple of 8 */
       DX_VOS_MemSetZero( (DxUint8_t*)TempBuff4_ptr , modSizeBytes );
       ((DxUint8_t*)TempBuff4_ptr)[modSizeBytes - 1] = 0x80;

       /* compare */
       cmpRes = CRYS_COMMON_CmpLsbUnsignedCounters(
                                       P_ptr,
                                       (DxUint16_t)modSizeBytes,
                                       (DxUint8_t*)TempBuff4_ptr,
                                       (DxUint16_t)modSizeBytes );


       /* Step 3.3. If P is not diverted, then perform primality tests on P:
                    8 Rabin-Miller and 1 Lucas tests (X9.42-2001)              */
       /*----------------------------------------------------------------------*/

       if( cmpRes == CRYS_COMMON_CmpCounter1GraterThenCounter2)
       {
		   Error = LLF_PKI_RSA_primeTestCall(
	                              (DxUint32_t *)P_ptr,
	                              modSizeWords,
	                              8 /*rabinTestsCount*/,
	                              (DxInt8_t*)&isPrime,
	                              TempBuff2_ptr, /* size as defined in function header */
	                              CRYS_DH_PRIME_TEST_MODE );

		   if( Error != CRYS_OK )
			goto EndWithError;
       }

       /* if pgenCounter >= 4096*N then return "generation is fail" */
       if( *pgenCounter_ptr >= 4096*(modPsizeBits + 1024 - 1)/1024 )
       {
		Error = CRYS_DH_PRIME_P_GENERATION_FAILURE_ERROR;
		goto EndWithError;
       }

       /* update pgenCounter_ptr */
	   *pgenCounter_ptr += 1;


   } /* END of while(isPrime != DX_TRUE)*/

   /* correction of pgenCounter */
   *pgenCounter_ptr -= 1;

   /* End of function */
   return Error;

EndWithError:

    DX_VOS_MemSetZero( P_ptr , modSizeBytes );

    return Error;

}/* End of DX_DH_X942_FindPrimeP */




/******************************************************************************************/
/**
 * @brief The function creates generator of GF(P) subgroup for key generation according to X9.42-2001.
 *
 *
 * @param[out] P_ptr              - The prime modulus P of structure P = j*Q + 1, where Q is prime
 *                                  and j is an integer.The buffer must be aligned to 4 bytes.
 * @param[in]  modPSizeBits       - The  modulus (prime) P size in bytes must be multiple of 4 bytes.
 * @param[out] Q_ptr              - The pointer to the order Q of generator. The buffer must be aligned to 4 bytes.
 * @param[in]  orderSizeBits      - The size of order of generator in bytes. Must be multiple of 4 bytes.
 * @param[out] G_ptr              - The generator of GF(P) subgroup. The buffer must be aligned to 4 bytes.
 *                                  size of buffer not less than modPSize in bytes.
 * @param[in]  tempBuff1_ptr      - The temp buffer of size not less than DH max modulus size, aligned to 4 bytes.
 * @param[in]  expTempBuff_ptr    - The temp buffer of defined structure.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a predefined error code.
 *
 *
 */
static CRYSError_t CRYS_DH_X942_CreateGenerator(
                          DxUint8_t           *P_ptr,               /*in */
                          DxUint32_t           modSizeBits,         /*in */
                          DxUint8_t           *Q_ptr,               /*in */
                          DxUint32_t           orderSizeBits,       /*in */
                          DxUint8_t           *G_ptr,               /*out*/
                          DxUint32_t          *tempBuff1_ptr,       /*in */
                          CRYS_DHKG_ExpTemp_t *expTempBuff_ptr )    /*in */
{

   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* modulus and order sizes in words */
   DxUint32_t modSizeBytes, modSizeWords, orderSizeWords;

   DxUint32_t J_effectiveSizeBits;

   /* compare flag */
   DxInt8_t  compFlag;


   /* INITIALIZATIONS */

   Error = CRYS_OK;
   modSizeBytes = modSizeBits  / 8;
   modSizeWords = modSizeBits  / 32;
   orderSizeWords = (orderSizeBits + 31) / 32;


   /* FUNCTION  LOGIC */

   /*-------------------------------------*/
   /* Step 1. Calculate J = (P - 1)/Q     */
   /*-------------------------------------*/

   /*  copy modulus into TempBuff1  */
   DX_VOS_FastMemCpy( (DxUint8_t*)expTempBuff_ptr->PubKey.n, (DxUint8_t*)P_ptr, modSizeBytes );
   DX_VOS_MemSetZero( (DxUint8_t*)expTempBuff_ptr->PubKey.n + modSizeBytes, CRYS_DH_MAX_MOD_SIZE_IN_BYTES - modSizeBytes );
   /* copy order Q into aligned buffer */
   DX_VOS_FastMemCpy( (DxUint8_t*)expTempBuff_ptr->TempBuff, Q_ptr, (orderSizeBits + 7)/8 );
   DX_VOS_MemSetZero( (DxUint8_t*)expTempBuff_ptr->TempBuff + (orderSizeBits + 7)/8, CRYS_DH_MAX_MOD_SIZE_IN_BYTES - (orderSizeBits + 7)/8 );

   /* subtract: P - 1 */
   CRYS_COMMON_DecrLsbUnsignedCounter( expTempBuff_ptr->PubKey.n, 1, (DxUint8_t)modSizeWords );

   /* divide */
   LLF_PKI_RSA_Call_Div(
                expTempBuff_ptr->PubKey.n,            /*numerator B*/
                modSizeWords,                         /*B_size in words*/
                expTempBuff_ptr->TempBuff,            /*Q - divider*/
                orderSizeWords,                       /*n_size in words*/
                expTempBuff_ptr->PubKey.e,            /*ModRes_ptr*/
                tempBuff1_ptr,                        /*DivRes_ptr*/
                expTempBuff_ptr->PrimeData.DataIn );  /*TempBuff_ptr: len=2*modSize (using also PrimeData_ptr->DataOut buff) */

   /* calculate actual size of J in bits */
   J_effectiveSizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits(
                                            (DxUint8_t*)tempBuff1_ptr,
                                            (DxUint16_t)modSizeWords * 4 );

   /*---------------------------------------------------------------*/
   /* Step 2. Generate random G : 1 < G < (P-1)  and                */
   /*         set it into DataIn buffer, other bytes of buffer = 0  */
   /*---------------------------------------------------------------*/
   /* cleaning of temp buffer */
   DX_VOS_MemSetZero( (DxUint8_t*)&expTempBuff_ptr->PrimeData, sizeof(CRYS_DHPrimeData_t) );

   /* generating rnd vector */
   Error = CRYS_RND_GenerateVectorInRange( modSizeBits, (DxUint8_t*)expTempBuff_ptr->PubKey.n /*P-1*/,
                                           (DxUint8_t*)expTempBuff_ptr->PrimeData.DataIn /*RND*/ );
   if( Error != CRYS_OK )

     goto End;

   /*----------------------------------------------------*/
   /* Step 3. Initialization of PubKey and PrivData      */
   /*         structures for exponentiation              */
   /*----------------------------------------------------*/

   /* cleaning of temp buffer */
   DX_VOS_MemSetZero( (DxUint8_t*)&expTempBuff_ptr->PubKey, sizeof(CRYS_DHPubKey_t) );

   /* set modulus in DH_PubKey structure for exponentiation G^J mod P */
   DX_VOS_FastMemCpy( (DxUint8_t*)expTempBuff_ptr->PubKey.n, (DxUint8_t*)P_ptr, modSizeBytes );
   expTempBuff_ptr->PubKey.nSizeInBits = modSizeBits;
   /* set exponent J and its size */
   DX_VOS_FastMemCpy( (DxUint8_t*)expTempBuff_ptr->PubKey.e, (DxUint8_t*)tempBuff1_ptr, (J_effectiveSizeBits+7)/8 );
   expTempBuff_ptr->PubKey.eSizeInBits = J_effectiveSizeBits;

   /*  initialize the H value in LLF of PubKey for exponentiation  */
   Error = LLF_PKI_RSA_InitPubKeyDb( &expTempBuff_ptr->PubKey );

   if( Error != CRYS_OK )

     goto End;


   /*-----------------------------------------------------------*/
   /* Step 4. Calculate G = G ^ J mod P , if G == 1, change     */
   /*         G (DataIn) and repeat exponentiation              */
   /*-----------------------------------------------------------*/

   compFlag = 0;  /* 0 - means G == 1 */

   while( compFlag == 0 )
   {
       /* exponentiation DataOut = DataIn ^ Exp mod P */
       Error = LLF_PKI_RSA_ExecPubKeyExp( &expTempBuff_ptr->PubKey, &expTempBuff_ptr->PrimeData );

	   if( Error != CRYS_OK )

	       return Error;

	   /* set 1 to temp2 register */
	   DX_VOS_MemSetZero( (DxUint8_t*)tempBuff1_ptr, modSizeBytes );
	   tempBuff1_ptr[0] = 1;

	   /* compare DataOut to 1: */
	   compFlag = DX_VOS_MemCmp( expTempBuff_ptr->PrimeData.DataOut, tempBuff1_ptr, modSizeBytes );

	   if( compFlag == 0 )  /* if G == 1 */
	   {
	      /* change DataIn (by adding 1) for trying next G value  */
	  CRYS_COMMON_IncLsbUnsignedCounter( expTempBuff_ptr->PrimeData.DataIn, 1, (DxUint8_t)modSizeWords );
	   }
   }

   /* copy generator into output */
   DX_VOS_FastMemCpy( G_ptr, (DxUint8_t*)expTempBuff_ptr->PrimeData.DataOut, modSizeBytes );

   /* End of function */
End:
    return Error;

}/* End of CRYS_DH_X942_CreateGenerator */

#endif /*#if defined(!CRYS_NO_HASH_SUPPORT && ! CRYS_NO_PKI_SUPPORT && ! CRYS_NO_DH_SUPPORT*/



/******************************************************************************************/
/************************         Public Functions           ******************************/
/******************************************************************************************/


/******************************************************************************************/
/**
 * @brief The function generates a DH (DLP) domain parameters in GF(P) (see X9.42-2001)
 *
 *
 * @param[in]  modPSizeBits       - The  modulus (prime) P size in bits equal 256*n, where n >= 4.
 * @param[in]  orderQSizeBits     - The size of order of generator in bits. According to ANSI X9.43:
 *                                  m must be multiple of 32 bits and m >= 160. According to ANSI X9.30-1:
 *                                  m = 160 bit. Because Q is prime factor of P-1, it must be <= (P-1)/2,
 *                                  therefore orderQSizeBits <= modPSizeBits.
 * @param[in]  seedSizeBits       - The  seed size in bits. Requirements:
 *                                  seedSizeBits >= orderQSizeBits and seedSizeBits <= modPSizeBits ( the
 *                                  last is required by our implementation).
 * @param[out] modP_ptr           - The prime modulus P of structure P = J*Q + 1, where Q is prime
 *                                  and j is an integer. Size of the buffer for output generated value must
 *                                  be not less, than modulus size.
 * @param[out] orderQ_ptr         - The pointer to the order Q of generator. Size of the buffer for output
 *                                  generated value must be not less, than order size.
 * @param[out] generatorG_ptr     - The pointer to the generator of multiplicative subgroup in GF(P).
 *                                  Size of the buffer for output generated value must be not less,
 *                                  than modulus size. If the pointer == NULL, the function not puts this
 *                                  parameter out (in this case generGsizeBytes_ptr must be set to NULL also,
 *                                  otherwise the function returns an error).
 * @param [in/out]generGsizeBytes_ptr - The pointer to the one-word buffer, containing the generator size value (in bytes).
 *                                  If output of generator is needed, the user must set the size value equaled to size
 *                                  of allocated buffer, and the function returns the actual size of generator in bytes.
 * @param[out] factorJ_ptr        - The pointer to buffer for integer factor J. If the pointer == NULL, the function
 *                                  not puts this parameter out. In this case JsizeBytes_ptr must be set to NULL also,
 *                                  otherwise the function returns an error. Size of the buffer must be not less,
 *                                  than ( modPSizesBytes - orderQSizeBytes + 1 ).
 * @param [in/out] JsizeBytes_ptr  - The pointer to the size of integer factor J. If the pointer == NULL,
 *                                  the function not puts this parameter out. If output of the factor J is needed, the
 *                                  user must set the J size value equal to the size of allocated buffer, and the
 *                                  function returns the actual size of J in bytes.
 * @param [in/out] seedS_ptr       - The random seed used for generation of primes. If the pointer == NULL,
 *                                  the function not puts this parameter out. Size of the buffer for output
 *                                  generated value must be not less, than passed seed size (see above).
 * @param[in] generateSeed        - The flag, defining whether the seed generated randomly by the function
 *                                  (generateSeed = 1), or it is passed by the input (generateSeed = 0).
 * @param[out] pgenCounter_ptr    - The pointer to counter of tries to generate the primes. If the pointer == NULL,
 *                                  the function not puts this parameter out.
 * @param[out] DHKGBuff_ptr       - The temp buffer for internal calculations. The buffer is defined as structure.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure - a predefined error code.
 *
 *     Note:  1. Input and Output vectors are in big endianness (high most bit is left most one).
 *            2. For reliability of checking of input parameters, in case that the user don't wont output of
 *               some parameters (generator or factorJ), he must set both - a pointer to appropriate buffer and a
 *               pointer to its size equaled to NULL for these parameters, otherwise the function returns an error.
 *            2. In case of any error the function may clean the output buffers.
 *
 */
CEXPORT_C CRYSError_t CRYS_DH_CreateDomainParams(
                          DxUint32_t          modPsizeBits,             /*in */
                          DxUint32_t          orderQsizeBits,           /*in */
                          DxUint32_t          seedSizeBits,             /*in */
                          DxUint8_t          *modP_ptr,                 /*out*/
                          DxUint8_t          *orderQ_ptr,               /*out*/
                          DxUint8_t          *generatorG_ptr,           /*out*/
						  DxUint32_t         *generGsizeBytes_ptr,      /*in/out*/
                          DxUint8_t          *factorJ_ptr,              /*out*/
                          DxUint32_t         *JsizeBytes_ptr,           /*in/out*/
                          DxUint8_t          *seedS_ptr,                /*in/out*/
                          DxInt8_t            generateSeed,             /*in*/
                          DxUint32_t         *pgenCounter_ptr,          /*out*/
                          CRYS_DHKGData_t    *DHKGbuff_ptr              /*in */ )
{

   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* pointers to temp buffers for candidates to order Q, modulus P, seed S, generator G */
   DxUint8_t  *Q_ptr, *P_ptr, *G_ptr, *J_ptr, *S_ptr;

   /* tries counter */
   DxUint32_t  pgenCounter;

   /* size of modulus and order ( in bytes) and generator (in bits) */
   DxUint32_t  modSizeBytes, orderSizeBytes, generatorSizeBits;

    /* temp buffer pointers */
   DxUint32_t  *TempBuff1_ptr, *TempBuff2_ptr;


   /* FUNCTION  LOGIC */

	Error = CRYS_OK;


   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DH_UNSUPPORTED( modPsizeBits, orderQsizeBits, seedSizeBits,
                             modP_ptr, orderQ_ptr, generatorG_ptr, generGsizeBytes_ptr,
							 factorJ_ptr,JsizeBytes_ptr,
                             seedS_ptr, generateSeed, pgenCounter_ptr, DHKGbuff_ptr,
                             Q_ptr, P_ptr, G_ptr, J_ptr, S_ptr, pgenCounter,
                             modSizeBytes, generatorSizeBits, orderSizeBytes,
                             TempBuff1_ptr, TempBuff2_ptr, Error, Error, Error, Error,
                             Error, Error, Error, Error, Error, Error);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT
   #ifndef CRYS_NO_DH_SUPPORT

   /* Step 1. Check input parameters */
   /*------------------------------- */

   /* check pointers: modP, orderQ and temp buffer. Note: other pointers may be NULL, if not used  */
   if( modP_ptr == DX_NULL ||
	   orderQ_ptr == DX_NULL ||
	   seedS_ptr == DX_NULL ||
       DHKGbuff_ptr == DX_NULL )
   {
	   return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
   }

   /* check sizes */
   if( modPsizeBits < CRYS_DH_MIN_VALID_KEY_SIZE_VALUE_IN_BITS ||
       modPsizeBits % 256 != 0 ||
       modPsizeBits > CRYS_DH_MAX_VALID_KEY_SIZE_VALUE_IN_BITS )
   {
	   return CRYS_DH_INVALID_MODULUS_SIZE_ERROR;	  }

   modSizeBytes = (modPsizeBits + 7)/ 8; /* calculate mod size in bytes */
   orderSizeBytes = (orderQsizeBits + 7)/ 8; /* calculate order size in bytes */

   if ( orderQsizeBits < CRYS_DH_SEED_MIN_SIZE_IN_BITS ||
        orderQsizeBits >  (modPsizeBits - 1)   ||
        orderQsizeBits % 32 != 0 )
   {
	   return CRYS_DH_INVALID_ORDER_SIZE_ERROR;
   }

   if( seedSizeBits < orderQsizeBits  ||    /* according to X9.42-2001 */
	   seedSizeBits > modPsizeBits )         /* our limitation of buffer size */
   {
	   return CRYS_DH_INVALID_SEED_SIZE_ERROR;
   }

   /* check generator G pointers and buffer size */
   if( (generatorG_ptr == DX_NULL && generGsizeBytes_ptr != DX_NULL) ||
	   (generatorG_ptr != DX_NULL && generGsizeBytes_ptr == DX_NULL) ||
	   ((generGsizeBytes_ptr != DX_NULL) && (*generGsizeBytes_ptr < modSizeBytes)) )
   {
	  return CRYS_DH_INVALID_GENERATOR_PTR_OR_SIZE_ERROR;
   }
   /* check J-factor pointers and buffer size */
   if( (factorJ_ptr == DX_NULL && JsizeBytes_ptr != DX_NULL) ||
	   (factorJ_ptr != DX_NULL && JsizeBytes_ptr == DX_NULL) ||
	   ((JsizeBytes_ptr != DX_NULL) && (*JsizeBytes_ptr < (modSizeBytes - orderSizeBytes + 1))) )
   {
	  return CRYS_DH_INVALID_J_FACTOR_PTR_OR_SIZE_ERROR;
   }

   /* check generateSeed parameter */
   if( generateSeed != 0 && generateSeed != 1 )
   {
		return CRYS_DH_INVALID_ARGUMENT_OPERATION_MODE_ERROR;
   }


   /* --------------------------------- */
   /*  Step 2.  Initializations         */
   /* --------------------------------- */

   /* clean DHKGbuff_ptr */
   DX_VOS_MemSetZero(DHKGbuff_ptr, sizeof(CRYS_DHKGData_t));

   /* set Q, S and G- pointers on DHKGbuff_ptr->PrimData temp buffers */
   Q_ptr = (DxUint8_t*)DHKGbuff_ptr->TempBuff2;
   P_ptr = (DxUint8_t*)DHKGbuff_ptr->TempBuff3;
   G_ptr = (DxUint8_t*)DHKGbuff_ptr->TempBuff4;
   J_ptr = (DxUint8_t*)DHKGbuff_ptr->TempBuff5;
   S_ptr = J_ptr;

   /* set 32-bit temp pointers on KGData and PrimData temp buffers */
   TempBuff1_ptr = DHKGbuff_ptr->TempBuff1;
   TempBuff2_ptr = (DxUint32_t*)&(DHKGbuff_ptr->ExpTemps);

   if( generateSeed == 0 )
       DX_VOS_FastMemCpy( S_ptr, seedS_ptr, (seedSizeBits+7)/8 );


   /*------------------------------------------------------------------- */
   /* Step 1. Find random prime Q and its Seed S according to ANSI X9.42 */
   /*------------------------------------------------------------------- */

   Error = CRYS_DH_X942_FindPrimeQ(
                              orderQsizeBits,            /*in */
                              seedSizeBits,              /*in */
                              generateSeed,              /*in */
                              Q_ptr,                     /*out*/
                              S_ptr,                     /*in/out*/
                              TempBuff1_ptr,             /*in */
                              TempBuff2_ptr );           /*in */

   if( Error != CRYS_OK )

      goto EndWithError;


   /*------------------------------------------------------ */
   /* Step 2.   Create random prime P = (Q*J + 1)           */
   /*------------------------------------------------------ */

   Error = CRYS_DH_X942_FindPrimeP(
                          modPsizeBits,        /*in */
                          orderQsizeBits,      /*in */
                          seedSizeBits,        /*in */
                          P_ptr,               /*out*/
                          Q_ptr,               /*out*/
                          S_ptr,               /*in */
                          &pgenCounter,        /*out*/
                          TempBuff1_ptr,       /*in */
                          TempBuff2_ptr );     /*in */

    if( Error != CRYS_OK )

       goto EndWithError;


   /*------------------------------------------------------ */
   /* Step 3.   Create generator of GF(P) subgroup          */
   /*------------------------------------------------------ */
   if( generatorG_ptr != DX_NULL )
   {
		Error = CRYS_DH_X942_CreateGenerator(
						  P_ptr,                                       /*in */
						  modPsizeBits,                                /*in */
						  Q_ptr,                                       /*in */
						  orderQsizeBits,                              /*in */
						  G_ptr,                                       /*out*/
						  TempBuff1_ptr,                               /*in */
						  &DHKGbuff_ptr->ExpTemps                      /*in */);

		if( Error != CRYS_OK )

		   goto EndWithError;

		/* calculate size of generator and output it in big endianness */
	    generatorSizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits( G_ptr, (DxUint16_t)modSizeBytes );
	    *generGsizeBytes_ptr = (generatorSizeBits + 7)/ 8;
		CRYS_COMMON_ReverseMemcpy( generatorG_ptr, G_ptr, *generGsizeBytes_ptr );
   }

   /* reverse output of result parameters (in big endianness) */
   CRYS_COMMON_ReverseMemcpy( modP_ptr, P_ptr, modSizeBytes );

   /* copy generated seed into output */
   if( generateSeed == 1 )
		DX_VOS_FastMemCpy( seedS_ptr, S_ptr, (seedSizeBits + 7)/8 );

   if( orderQ_ptr != DX_NULL )
		CRYS_COMMON_ReverseMemcpy( orderQ_ptr, Q_ptr, orderQsizeBits / 8 );

   /* if factorJ_ptr != DX_NULL, then calculate this factor and its size. J = (P-1) / Q */
   if( factorJ_ptr != DX_NULL )
   {
	   LLF_PKI_RSA_Call_Div(
	                    (DxUint32_t*)P_ptr,                 /*numerator B*/
	                    modPsizeBits / 32,                  /*B_size in words*/
	                    (DxUint32_t*)Q_ptr,                 /*divider N */
	                    orderQsizeBits / 32,                /*N_size in words*/
	                    TempBuff1_ptr,                      /*ModRes_ptr*/
	                    (DxUint32_t*)J_ptr,                 /*DivRes_ptr*/
	                    TempBuff2_ptr                       /*TempBuff_ptr (size >= 2*N_Size) */ );

      /* calculate size of J in bits */
	  *JsizeBytes_ptr = CRYS_COMMON_GetCounterEffectiveSizeInBits(
                              J_ptr, (DxUint16_t)(modPsizeBits / 8) );

      /* calculate size of J in bytes */
	  *JsizeBytes_ptr = (*JsizeBytes_ptr + 7) / 8;

      /* reverse copy result to factorJ_ptr buffer */
	  CRYS_COMMON_ReverseMemcpy( factorJ_ptr,  J_ptr, *JsizeBytes_ptr );
   }

   /*  if pgenCounter_ptr != DX_NULL put out pgenCounter */
   if( pgenCounter_ptr != DX_NULL )
		*pgenCounter_ptr = pgenCounter;

   goto End;

   /* End of function */

EndWithError:

   /* cleaning output buffers used also in internal computations */
   DX_VOS_MemSetZero( modP_ptr,   (modPsizeBits+7)/8 );
   DX_VOS_MemSetZero( generatorG_ptr, *generGsizeBytes_ptr );

   if( orderQ_ptr != DX_NULL )
	   DX_VOS_MemSetZero( orderQ_ptr, (orderQsizeBits+7)/8 );

   if( factorJ_ptr != DX_NULL )
	   DX_VOS_MemSetZero( factorJ_ptr, *JsizeBytes_ptr );

   if( generateSeed == 1 )
	   DX_VOS_MemSetZero( seedS_ptr, (seedSizeBits + 7)/8 );


End:
   /* cleaning of temp buffer */
   DX_VOS_MemSetZero( DHKGbuff_ptr, sizeof(DHKGbuff_ptr) );

	return Error;

  #endif /*CRYS_NO_DH_SUPPORT*/
  #endif /*CRYS_NO_PKI_SUPPORT*/
  #endif /*CRYS_NO_HASH_SUPPORT*/

}


/******************************************************************************************/
/**
* @brief The function checks the obtained DH domain parameters according X9.42-2001.
*
*        There may be 3 case of checking:
*        1. Checking of primes only ( modulus P and order Q according to passed seed S and pgenCounter).
*           In this case all pointers and sizes of said parameters must be passed (not NULL), but generator
*           G pointer and it size must be both set to NULL.
*        2. Checking of generator G only in assuming that primes parameters P, Q are valid. In ths case
*           the user must to pass the P,Q,G pointers and sizes. The seed S pointer and size must be both
*           set to NULL, otherwise the function returns an error.
*        3. Checking all domain parameters. In this case all input parameters must be passed to the function.
*
*        If any of checked domain parameters is not compliant to X9.42-2001 standard and our implementation
*        limitation, the function returns an error according to CRYS_DH_error.h file.
*
*        NOTE:  Detailed requirements to all used parameters are described above in CRYS_DH_CreateDomainParams
*               functions API.
*
* @param[out] modP_ptr           - The prime modulus P. Must be of structure P = j*Q + 1,
*                                  where Q is prime and j is an integer.
* @param[in]  modPSizeBits       - The  modulus (prime) P size in bits equal 256*n, where n >= 4.
* @param[out] orderQ_ptr         - The pointer to the order Q of generator.
* @param[in]  orderQSizeBytes    - The size of order of generator in bytes. According to ANSI X9.43:
*                                  m must be multiple of 32 bits and m >= 160. According to ANSI X9.30-1:
*                                  m = 160 bit. Because Q is prime factor of P-1, it must be <= (P-1)/2.
*                                  In our implementation required, that orderQSize <= modPSizeBytes/2.
* @param[in]  generatorG_ptr     - The pointer to the generator of multiplicative subgroup in GF(P).
* @param[in]  generatorSizeBytes - The size of generator in bytes (must be set if generator will be checked).
* @param[in]  seedS_ptr          - The random seed used for generation of primes (must be set if
*                                  primes will be checked).
* @param[in]  seedSizeBits      - The seed size in bits. If the seed is used, then its size
*                                  must be: seedSizeBits >= orderQSizeBits and seedSizeBits <= modPSizeBits ( the
*                                  last is required by our implementation).
* @param[in]  pgenCounter        - The counter of tries to generate the primes (must be set if primes
*                                  will be checked).
* @param[out] TempBuff_ptr       - The first temp buffer of defined structure.
* @param[in]  DHKGbuff_ptr       - The second temp buffer for internal calculations (defined type).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure or if one or more domain
*                       parameters are invalid the function returns a predefined error code.
*
*     Note:  Input vectors are in big endianness.
*
*/
CEXPORT_C CRYSError_t CRYS_DH_CheckDomainParams(
                          DxUint8_t               *modP_ptr,           /*in */
                          DxUint32_t               modPsizeBytes,      /*in */
                          DxUint8_t               *orderQ_ptr,         /*in */
                          DxUint32_t               orderQsizeBytes,    /*in */
                          DxUint8_t               *generatorG_ptr,     /*in */
                          DxUint32_t               generatorSizeBytes, /*in */
                          DxUint8_t               *seedS_ptr,          /*in */
                          DxUint32_t               seedSizeBits,      /*in */
                          DxUint32_t               pgenCounter,        /*in */
                          CRYS_DHKG_CheckTemp_t   *TempBuff_ptr,       /*in */
                          CRYS_DHKGData_t         *DHKGbuff_ptr        /*in */ )
{
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* pointers to temp buffers for calculating order Q, modulus P, seed S, generator G, H-value H_ptr */
   DxUint8_t  *Q_ptr, *P_ptr ;

   /* size of modulus in bits and in words */
   DxUint32_t  modPsizeBits, modPsizeWords;

   /* size  order Q (in bits) */
   DxUint32_t  orderQsizeBits;

   /* counter of trying to generate modulus P; pgenCounter*/
   DxUint32_t  pgenCounter1;


   /* FUNCTION  LOGIC */

	Error = CRYS_OK;


   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DH_UNSUPPORTED( modP_ptr, modPsizeBytes, orderQ_ptr, orderQsizeBytes,
                             generatorG_ptr, generatorSizeBytes, seedS_ptr,
                             seedSizeBits, pgenCounter, TempBuff_ptr, DHKGbuff_ptr,
                             Q_ptr, P_ptr, modPsizeBits, modPsizeWords, orderQsizeBits,
                             pgenCounter1, Error, Error, Error, Error, Error, Error, Error,
                             Error, Error, Error, Error, Error, Error, Error, Error,  Error, Error );

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT
   #ifndef CRYS_NO_DH_SUPPORT

   /*------------------------------- */
   /* Step 1. Check input pointers   */
   /*------------------------------- */

   /* check pointers: modP, generator and tempBuff. Note: other pointers may be NULL  */
   if( modP_ptr == DX_NULL ||
       orderQ_ptr == DX_NULL ||
       TempBuff_ptr == DX_NULL ||
       DHKGbuff_ptr == DX_NULL )

	   return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;

   /* check modulus and order sizes */
   if( modPsizeBytes < CRYS_DH_MIN_VALID_KEY_SIZE_VALUE_IN_BITS / 8 ||
       modPsizeBytes % 32 != 0 ||
       modPsizeBytes > CRYS_DH_MAX_MOD_SIZE_IN_BYTES )
   {
	   return CRYS_DH_INVALID_MODULUS_SIZE_ERROR;	  }

   if ( orderQsizeBytes < CRYS_DH_SEED_MIN_SIZE_IN_BITS / 8 ||
        orderQsizeBytes % 4 != 0 ||
		orderQsizeBytes > modPsizeBytes )
   {
	   return CRYS_DH_INVALID_ORDER_SIZE_ERROR;
   }

   /* Seed pointer and size checking:
      If pointer or size of seed are illegal, then output an error.
      Note: In case that primes checking is not needed, the seed pointer and size must be
	        set to NULL  and are legal */
   if( (seedSizeBits == 0 && seedS_ptr != DX_NULL ) ||
	   (seedSizeBits != 0 && seedS_ptr == DX_NULL ) )

	   return CRYS_DH_CHECK_SEED_SIZE_OR_PTR_NOT_VALID_ERROR;

   /* Generator pointer and size checking:
      If pointer or size of generator are illegal, then output an error.
      Note: In case that generator checking is not needed, its pointer and size are equaled to NULL */
   if( (generatorSizeBytes == 0 && generatorG_ptr != DX_NULL ) ||
	   (generatorSizeBytes != 0 && generatorG_ptr == DX_NULL ) )

	   return CRYS_DH_CHECK_GENERATOR_SIZE_OR_PTR_NOT_VALID_ERROR;


   /* --------------------------------- */
   /*  Step 2.  Initializations         */
   /* --------------------------------- */

   /* clean TempBuff_ptr */
   DX_VOS_MemSetZero(TempBuff_ptr, sizeof(CRYS_DHKG_CheckTemp_t));

   /* calculate P and Q size in bits */
   modPsizeWords = (modPsizeBytes + 3) / 4;

   /* set Q, P and G- pointers on DHKGbuff_ptr->PrimData temp buffers */
   Q_ptr = (DxUint8_t*)TempBuff_ptr;
   P_ptr = Q_ptr + 4*modPsizeWords;

   if( seedS_ptr != DX_NULL )
   {
	   /*--------------------------------------------- */
	   /* Step 3. Calculate and check primes sizes     */
	   /*--------------------------------------------- */
	   /* convert P and Q to little endianness for calculating their sizes in bits */
	   CRYS_COMMON_ReverseMemcpy( P_ptr, (DxUint8_t*)modP_ptr, modPsizeBytes );
	   CRYS_COMMON_ReverseMemcpy( Q_ptr, (DxUint8_t*)orderQ_ptr, orderQsizeBytes );

	   modPsizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits( P_ptr, (DxUint16_t)modPsizeBytes );
	   orderQsizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits( Q_ptr, (DxUint16_t)orderQsizeBytes );

	   /*------------------------------------------------------------------- */
	   /* Step 4. Generate random primes P,Q for given seed Seed S according */
	   /*         to ANSI X9.42 for comparing with input parameters          */
	   /*         The called CreateDomainParams also checks sizes of input   */
	   /*         parameters                                                 */
	   /*------------------------------------------------------------------- */

	   Error =  CRYS_DH_CreateDomainParams(
							  modPsizeBits,                    /*in */
							  orderQsizeBits,                  /*in */
							  seedSizeBits,                    /*in */
							  P_ptr,                           /*out*/
							  Q_ptr,                           /*out*/
							  DX_NULL /*generatorG_ptr*/,      /*out*/
							  0 /*generatorSize */,            /*out*/
							  DX_NULL /*generatorSize_ptr*/,   /*out*/
							  DX_NULL /*JsizeBytes_ptr*/,      /*out*/
							  seedS_ptr,                       /*in/out*/
							  DX_FALSE /*generateSeed*/,       /*in*/
							  &pgenCounter1,                   /*out*/
							  DHKGbuff_ptr                     /*in */ );

	   if( Error != CRYS_OK )

		  goto End;

	   /* -------------------------------------------------------------------*/
	   /* Step 5. Compare generated primes with input, if one of compares   */
	   /*         is not "equal", the output error                          */
	   /* -------------------------------------------------------------------*/

	   if( DX_VOS_MemCmp( modP_ptr, P_ptr, modPsizeBytes ) != 0 )
	   {
			Error = CRYS_DH_CHECK_DOMAIN_PRIMES_NOT_VALID_ERROR;
			goto End;
	   }

	   else if( DX_VOS_MemCmp( orderQ_ptr, Q_ptr, orderQsizeBytes ) != 0 )
	   {
			Error = CRYS_DH_CHECK_DOMAIN_PRIMES_NOT_VALID_ERROR;
			goto End;
	   }

	   /* compare pgen counters */
	   else if( pgenCounter != pgenCounter1 )
	   {
			Error = CRYS_DH_CHECK_DOMAIN_PRIMES_NOT_VALID_ERROR;
			goto End;
	   }

	   else if( Error != CRYS_OK )

		  goto End;
   }

   /*-----------------------------------------------------------------*/
   /* Step 4. Check generator using the function for checking of      */
   /*    the public key because both perform identical operations     */
   /*    with appropriate parameters. In this case:                   */
   /*    if G > P-2, or G < 2, or G^Q != 1, then output an error      */
   /*-----------------------------------------------------------------*/

   if( generatorG_ptr != DX_NULL )
   {
		Error = CRYS_DH_CheckPubKey(
							  modP_ptr,                                   /*in */
							  modPsizeBytes,                              /*in */
							  orderQ_ptr,                                 /*in */
							  orderQsizeBytes,                            /*in */
							  generatorG_ptr,                             /*in */
							  generatorSizeBytes,                         /*in */
							 &DHKGbuff_ptr->ExpTemps                      /*in */ );

	   /* Set error code according to checked parameter issue */
	   if( Error == CRYS_DH_INVALID_PUBLIC_KEY_SIZE_ERROR ||
		   Error == CRYS_DH_INVALID_PUBLIC_KEY_ERROR )
	   {
			Error =  CRYS_DH_CHECK_GENERATOR_NOT_VALID_ERROR;
	   }
   }

End:
   /* cleaning of temp buffers */
   DX_VOS_MemSetZero( DHKGbuff_ptr, sizeof(CRYS_DHKGData_t) );
   DX_VOS_MemSetZero( TempBuff_ptr, sizeof(CRYS_DHKG_CheckTemp_t) );

    /* End of function */

	return Error;

  #endif /*CRYS_NO_DH_SUPPORT*/
  #endif /*CRYS_NO_PKI_SUPPORT*/
  #endif /*CRYS_NO_HASH_SUPPORT*/

}


/******************************************************************************************/
/**
 * @brief The function checks the obtained DH public key according to its domain
 *        parameters (see X9.42-2001)
 *
 * @param[out] modP_ptr           - The modulus (prime) P.
 * @param[in]  modPSizeBytes      - The modulus size in bytes.
 * @param[in]  orderQ_ptr         - The pointer to prime order Q of generator,
 * @param[in]  orderQSizeBytes    - The size of order of generator in bytes.
 * @param[in]  pubKey_ptr         - The pointer to public key to be validated .
 * @param[in]  pubKeySizeBytes    - The public key size in bytes.
 * @param[in]  TempBuff_ptr       - The temp buffer for internal calculations.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure or if one or more domain
 *                       parameters are invalid the function returns a predefined error code.
 *
 *              Assuming: The DH domain parameters are valid.
 */
CEXPORT_C CRYSError_t CRYS_DH_CheckPubKey(
                          DxUint8_t              *modP_ptr,             /*in */
                          DxUint32_t              modPsizeBytes,        /*in */
                          DxUint8_t              *orderQ_ptr,           /*in */
                          DxUint32_t              orderQsizeBytes,      /*in */
                          DxUint8_t              *pubKey_ptr,           /*in */
                          DxUint32_t              pubKeySizeBytes,      /*in */
                          CRYS_DHKG_ExpTemp_t    *tempBuff_ptr          /*in */)
{

   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;


   /* size in bits of modulus P and order Q and public key */
   DxUint32_t  modPsizeBits;
   DxUint32_t  orderQsizeBits;
   DxUint32_t  pubKeySizeBits;

   /* comparing result */
   DxInt_t  cmpRes;
   CRYS_COMMON_CmpCounter_t cmpCounters;


   /* FUNCTION  LOGIC */

	Error = CRYS_OK;


   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_DH_UNSUPPORTED( modP_ptr, modPsizeBytes, orderQ_ptr, orderQsizeBytes,
                             pubKey_ptr, pubKeySizeBytes, tempBuff_ptr,
                             modPsizeBits, orderQsizeBits, cmpRes, cmpCounters,
                             pubKeySizeBits,  Error, Error, Error, Error, Error,
                             Error, Error, Error, Error, Error, Error, Error, Error,
                             Error, Error, Error, Error, Error, Error, Error, Error, Error );

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT
   #ifndef CRYS_NO_DH_SUPPORT

   /*------------------------------- */
   /* Step 1. Check input pointers   */
   /*------------------------------- */

   /* check pointers: modP, generator and tempBuff. Note: other pointers may be NULL  */
   if( modP_ptr == DX_NULL ||
       orderQ_ptr == DX_NULL ||
       pubKey_ptr == DX_NULL ||
       tempBuff_ptr == DX_NULL  )
   {
	   return CRYS_DH_INVALID_ARGUMENT_POINTER_ERROR;
   }

   /*----------------------------------------------------------- */
   /* Step 2. Calculate and check the sizes of modulus and order */
   /*----------------------------------------------------------- */
   /* preliminary check */
   if( modPsizeBytes > CRYS_DH_MAX_VALID_KEY_SIZE_VALUE_IN_BITS / 8 )

		return CRYS_DH_INVALID_MODULUS_SIZE_ERROR;

   if( orderQsizeBytes > modPsizeBytes )

		return CRYS_DH_INVALID_ORDER_SIZE_ERROR;

   /* temporary convert P, Q, pubKey to little endianness and calculate
      their sizes in bits  */

   DX_VOS_MemSetZero( tempBuff_ptr, sizeof(CRYS_DHKG_ExpTemp_t) );

   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)tempBuff_ptr->PubKey.n, modP_ptr, modPsizeBytes );

   modPsizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits(
                              (DxUint8_t*)tempBuff_ptr->PubKey.n, (DxUint16_t)modPsizeBytes );

   /* temporary reverse copy order into e-buffer for calculating its size */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)tempBuff_ptr->PubKey.e, orderQ_ptr, orderQsizeBytes );

   orderQsizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits(
                              (DxUint8_t*)tempBuff_ptr->PubKey.e, (DxUint16_t)orderQsizeBytes );

   /* reverse copy public key into data-in buffer */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)tempBuff_ptr->PrimeData.DataIn, pubKey_ptr, pubKeySizeBytes );

   pubKeySizeBits = CRYS_COMMON_GetCounterEffectiveSizeInBits(
                              (DxUint8_t*)tempBuff_ptr->PrimeData.DataIn, (DxUint16_t)pubKeySizeBytes );

   /* check sizes */
   if( modPsizeBits < CRYS_DH_MIN_VALID_KEY_SIZE_VALUE_IN_BITS ||
       modPsizeBits % 256 != 0 ||
       modPsizeBits > CRYS_DH_MAX_VALID_KEY_SIZE_VALUE_IN_BITS )
   {
	   Error = CRYS_DH_INVALID_MODULUS_SIZE_ERROR;
	   goto End;
   }

   if ( orderQsizeBits < CRYS_DH_SEED_MIN_SIZE_IN_BITS ||
        orderQsizeBits % 32 != 0 )
   {
	   Error = CRYS_DH_INVALID_ORDER_SIZE_ERROR;
	   goto End;
   }

   if ( pubKeySizeBits > modPsizeBits ||
        pubKeySizeBits <= 1 )
   {
	   Error = CRYS_DH_INVALID_PUBLIC_KEY_SIZE_ERROR;
	   goto End;
   }

   /*----------------------------------------------------------- */
   /* Step 2. Check value of public key:   pubKey < P-1          */
   /*         Note: pubKey > 1 already is checked above          */
   /*----------------------------------------------------------- */

    /* decrement modulus in temp buffer n (in little endianness). Note: the modulus is odd */
    ((DxUint8_t*)tempBuff_ptr->PubKey.n)[0] -= 1;

    /* compare with pub key saved in temp buff e */
    cmpCounters = CRYS_COMMON_CmpLsbUnsignedCounters(
                                          (DxUint8_t*)tempBuff_ptr->PubKey.e, /* counter1 - pubKey */
                                          (DxUint16_t)pubKeySizeBytes,
                                          (DxUint8_t*)tempBuff_ptr->PubKey.n, /* counter2 - (P-1) */
                                          (DxUint16_t)modPsizeBytes );

    if( cmpCounters != CRYS_COMMON_CmpCounter2GraterThenCounter1 )
    {
	Error = CRYS_DH_INVALID_PUBLIC_KEY_ERROR;

	    goto End;
    }

   /*----------------------------------------------------*/
   /* Step 4. Initialization of PubKey and PrivData      */
   /*         structures for exponentiation              */
   /*----------------------------------------------------*/

   /* increment (revert) modulus in temp buffer n (in little endianness) */
   ((DxUint8_t*)tempBuff_ptr->PubKey.n)[0] += 1;

   /* set modulus and exponent sizes in DH_PubKey structure  */
   tempBuff_ptr->PubKey.nSizeInBits = modPsizeBits;
   tempBuff_ptr->PubKey.eSizeInBits = orderQsizeBits;

   /*  initialize the H value in LLF of PubKey for exponentiation  */
   Error = LLF_PKI_RSA_InitPubKeyDb( &tempBuff_ptr->PubKey );

   if( Error != CRYS_OK )

     goto End;


   /*-----------------------------------------------------------*/
   /* Step 3. Calculate Res = Key ^ Q mod P , if Res == 1,      */
   /*         then key is valid, else non valid                 */
   /*-----------------------------------------------------------*/

   /* exponentiation DataOut = DataIn ^ exp mod n */
   Error = LLF_PKI_RSA_ExecPubKeyExp( &tempBuff_ptr->PubKey, &tempBuff_ptr->PrimeData );

   if( Error != CRYS_OK )

       goto End;


   /* set 1 to PubKey_ptr->n buffer (used as temp buffer) */
   DX_VOS_MemSetZero( (DxUint8_t*)tempBuff_ptr->PubKey.n, modPsizeBytes );
   tempBuff_ptr->PubKey.n[0] = 1;

   /* compare DataOut to 1: */
   cmpRes = DX_VOS_MemCmp( tempBuff_ptr->PrimeData.DataOut, tempBuff_ptr->PubKey.n, modPsizeBytes );

   if( cmpRes != 0 )  /* if Res != 1 */
   {
       return CRYS_DH_INVALID_PUBLIC_KEY_ERROR;
   }

   /* End of function */
End:

  /* clean temp buffers */
  DX_VOS_MemSetZero( tempBuff_ptr, sizeof(CRYS_DHKG_ExpTemp_t) );

  return Error;

  #endif /*CRYS_NO_DH_SUPPORT*/
  #endif /*CRYS_NO_PKI_SUPPORT*/
  #endif /*CRYS_NO_HASH_SUPPORT*/

}
