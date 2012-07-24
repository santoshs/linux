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
#include "CRYS_RSA_error.h"
#include "CRYS_RSA_Local.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_SELF_TEST_Local.h"

/* .............. LLF level includes ................. */

#if !defined(CRYS_NO_HASH_SUPPORT) && !defined(CRYS_NO_PKI_SUPPORT)

#include "LLF_PKI_RSA.h"
#include "LLF_PKI_EngineInfo.h"

#endif /* !defined(CRYS_NO_HASH_SUPPORT) && !defined(CRYS_NO_PKI_SUPPORT) */

/************************ Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums ************************************/

/************************ Typedefs *********************************/

/************************ Global Data ******************************/

/*
 For debugging the RSA_KG module define the following flags in project properties
 and perform the following:
   1. Define LLF_PKI_PKA_DEBUG.
   2. For finding the bad random vactors (P,Q,P1pR,P2pR,P1qR,P2qR):
      define RSA_KG_FIND_BAD_RND flag, perform test and save (from memory) the finded bad vectors.
   3. For repeat the testing of finded bad vectors, write they as HW initialization of
      the following buffers: P=>RSA_KG_debugPvect, Q=>RSA_KG_debugQvect - in the CRYS_RSA_KG.c file,
      and P1pR=>rBuff1, P2pR=>rBuff1, P1qR=>rBuff3, P2qR=>rBuff4 in the LLF_PKI_GenKeyX931FindPrime.c file.
      Define the flag RSA_KG_NO_RND instead previously defined RSA_KG_FIND_BAD_RND flag and
      perform the test.
   4. For ordinary ATP or other tests (without debug) undefine all the named flags.
*/

#ifdef LLF_PKI_PKA_DEBUG
#if (defined RSA_KG_FIND_BAD_RND || defined RSA_KG_NO_RND)
	DxUint8_t   RSA_KG_debugPvect[CRYS_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BYTES] =
	{ 0x78,0x71,0xDF,0xC5,0x36,0x98,0x12,0x21,0xCA,0xAC,0x48,0x22,0x01,0x94,0xF7,0x1A,
	  0x1C,0xBF,0x82,0xE9,0x8A,0xE4,0x2C,0x84,0x43,0x46,0xCF,0x6D,0x60,0xFB,0x5B,0xD3};
	DxUint8_t   RSA_KG_debugQvect[CRYS_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BYTES] =
	{ 0x46,0x13,0x9F,0xBA,0xBC,0x8E,0x21,0x13,0x35,0x8C,0x2C,0x2D,0xA8,0xD6,0x59,0x78,
	  0x8A,0x14,0x17,0x5F,0xA5,0xEC,0x22,0xD5,0x87,0xF9,0x99,0x45,0x1B,0x38,0xA3,0xF0};
#endif
#endif


/************* Private function prototype **************************/

CRYSError_t CRYS_RSA_GenerateVectorInRangeX931( DxUint32_t  RndSizeInBits,
                                                DxUint8_t  *RndVect_ptr,
                                                DxUint8_t  *temp_ptr );


/************************ Public Functions ******************************/


/***********************************************************************************************/
#ifndef _INTERNAL_CRYS_NO_RSA_KG_SUPPORT
/**
   @brief CRYS_RSA_KG_GenerateKeyPair generates a Pair of public and private keys on non CRT mode.

   @param[in] PubExp_ptr - The pointer to the public exponent (public key)
   @param[in] PubExpSizeInBytes - The public exponent size in bytes.
   @param[in] KeySize  - The size of the key, in bits. Supported sizes are:
                            - for PKI without PKA HW: all 256 bit multiples between 512 - 2048;
                            - for PKI with PKA: HW all 32 bit multiples between 512 - 2112;
   @param[out] UserPrivKey_ptr - A pointer to the private key structure.
                           This structure is used as input to the CRYS_RSA_PRIM_Decrypt API.
   @param[out] UserPubKey_ptr - A pointer to the public key structure.
                           This structure is used as input to the CRYS_RSA_PRIM_Encrypt API.
   @param[in] KeyGenData_ptr - a pointer to a structure required for the KeyGen operation.

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_SIZE
*/
CEXPORT_C CRYSError_t CRYS_RSA_KG_GenerateKeyPair(
	                                    DxUint8_t             *PubExp_ptr,
                                        DxUint16_t             PubExpSizeInBytes,
                                        DxUint32_t             KeySize,
                                        CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,
                                        CRYS_RSAUserPubKey_t  *UserPubKey_ptr,
                                        CRYS_RSAKGData_t      *KeyGenData_ptr )
{
   /* LOCAL INITIALIZATIONS AND DECLERATIONS */

   /* the error identifier */
   CRYSError_t Error;

   /* the pointers to the key structures */
   CRYSRSAPubKey_t  *PubKey_ptr;
   CRYSRSAPrivKey_t *PrivKey_ptr;

   /* a temp definition to solve a problem on release mode on VC++ */
   volatile DxUint32_t dummy = PubExpSizeInBytes;

   /* Data for Conditional test, after Key - Generation */
   const DxUint8_t Data_ptr[]={'D','i','s','c','r','e','t','i','x'};
   DxUint16_t DataSize=sizeof(Data_ptr);


   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RSA_UNSUPPORTED( PubExp_ptr , PubExpSizeInBytes , KeySize ,
                              UserPrivKey_ptr , UserPubKey_ptr , KeyGenData_ptr ,
                              Error , PubKey_ptr ,
                              PrivKey_ptr , dummy , dummy , dummy ,
                              dummy,dummy,dummy,dummy,dummy,dummy,dummy,dummy,dummy,dummy);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT

   /* ................ initializaions and local declarations ............ */
   /* ------------------------------------------------------------------- */

   /* to avoid compilers warnings */
   dummy = dummy;

   /* initialize the error identifier to O.K */
   Error = CRYS_OK;


   /* ................. checking the validity of the pointer arguments ....... */
   /* ------------------------------------------------------------------------ */

   /* ...... checking the key database handle pointer .................... */
   if( PubExp_ptr == DX_NULL )

      return CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR;

   /* ...... checking the validity of the exponent pointer ............... */
   if( UserPrivKey_ptr == DX_NULL )

      return CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR;

   /* ...... checking the validity of the modulus pointer .............. */
   if( UserPubKey_ptr == DX_NULL )

      return CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR;

   /* ...... checking the validity of the keygen data .................. */
   if( KeyGenData_ptr == DX_NULL )

      return CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID;

   /* ...... checking the required key size ............................ */
   if( ( KeySize < CRYS_RSA_MIN_VALID_KEY_SIZE_VALUE_IN_BITS ) ||
       ( KeySize > CRYS_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS ) ||
       ( KeySize % CRYS_RSA_VALID_KEY_SIZE_MULTIPLE_VALUE_IN_BITS ) )

      return CRYS_RSA_INVALID_MODULUS_SIZE;

   /* set the public and private key structure pointers */
   PubKey_ptr  = ( CRYSRSAPubKey_t *)UserPubKey_ptr->PublicKeyDbBuff;
   PrivKey_ptr = ( CRYSRSAPrivKey_t *)UserPrivKey_ptr->PrivateKeyDbBuff;



   /* ................ clear all input structures ............................. */
   /* ------------------------------------------------------------------------- */

   DX_VOS_MemSet( UserPrivKey_ptr , 0 , sizeof(CRYS_RSAUserPrivKey_t) );
   DX_VOS_MemSet( UserPubKey_ptr  , 0 , sizeof(CRYS_RSAUserPubKey_t) );
   DX_VOS_MemSet( KeyGenData_ptr  , 0 , sizeof(CRYS_RSAKGData_t) );

   /* ................ loading the public exponent to the structure .......... */
   /* ------------------------------------------------------------------------- */

   /* loading the buffers to start from LS word to MS word */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PubKey_ptr->e , PubExp_ptr , min(PubExpSizeInBytes , sizeof( PubKey_ptr->e )) );

   /* .......... initializing the effective counters size in bits .......... */
   PubKey_ptr->eSizeInBits = CRYS_COMMON_GetCounterEffectiveSizeInBits( (DxUint8_t*)PubKey_ptr->e,PubExpSizeInBytes );

   /* if the size in bits is 0 - return error */
   if( PubKey_ptr->eSizeInBits == 0 )
   {
      Error = CRYS_RSA_INVALID_EXPONENT_SIZE;
      goto End;
   }

   /* verifying the exponent has legal value (currently only 0x3,0x11 and 0x10001) */
   if( PubKey_ptr->e[0] != 0x3  &&
       PubKey_ptr->e[0] != 0x11 &&
       PubKey_ptr->e[0] != 0x010001 )
   {
       Error = CRYS_RSA_INVALID_EXPONENT_VAL;
       goto End;
   }

   /* .......... initialize the public key on the private structure ............... */
   DX_VOS_FastMemCpy( PrivKey_ptr->PriveKeyDb.NonCrt.e , PubKey_ptr->e , 4*((PubExpSizeInBytes+3)/4) );
   PrivKey_ptr->PriveKeyDb.NonCrt.eSizeInBits = PubKey_ptr->eSizeInBits;

   /* .......... initializing the key size in bits ......................... */

   /* this initialization is required for the low level function (LLF) - indicates the required
      size of the key to be found */
   PubKey_ptr->nSizeInBits  = KeySize;
   PrivKey_ptr->nSizeInBits = KeySize;

   /* .......... set the private mode to non CRT .............................. */
   /* ------------------------------------------------------------------------- */

   /* set the mode to non CRT */
   PrivKey_ptr->OperationMode = CRYS_RSA_NoCrt;

   /* set the key source as internal */
   PrivKey_ptr->KeySource = CRYS_RSA_InternalKey;

   /* ................ executing the key generation ........................... */
   /* ------------------------------------------------------------------------- */

   /* on the SSDMA platform it the random generation is executed by the PIC - it is not required */
 #if LLF_PKI_ENGINE_TYPE != CRYS_DEFS_SSDMA_ENGINE_TYPE

   /* generate the random */

#if ( (!defined RSA_KG_FIND_BAD_RND && !defined RSA_KG_NO_RND) || defined RSA_KG_FIND_BAD_RND )
   Error = CRYS_RSA_GenerateVectorInRangeX931( KeySize / 2, (DxUint8_t*)KeyGenData_ptr->KGData.p,
                                               (DxUint8_t*)PrivKey_ptr->n /*as temp buff*/);
   if( Error != CRYS_OK )

     goto End;

   Error = CRYS_RSA_GenerateVectorInRangeX931( KeySize / 2, (DxUint8_t*)KeyGenData_ptr->KGData.q,
                                               (DxUint8_t*)PrivKey_ptr->n /*as temp buff*/ );
   if( Error != CRYS_OK )

     goto End;
#endif

#if defined RSA_KG_FIND_BAD_RND

   DX_VOS_FastMemCpy( RSA_KG_debugPvect, (DxUint8_t*)KeyGenData_ptr->KGData.p, KeySize / (2*8) );
   DX_VOS_FastMemCpy( RSA_KG_debugQvect, (DxUint8_t*)KeyGenData_ptr->KGData.q, KeySize / (2*8) );

#endif

#if defined RSA_KG_NO_RND

   DX_VOS_FastMemCpy( (DxUint8_t*)KeyGenData_ptr->KGData.p, RSA_KG_debugPvect, KeySize / (2*8) );
   DX_VOS_FastMemCpy( (DxUint8_t*)KeyGenData_ptr->KGData.q, RSA_KG_debugQvect, KeySize / (2*8) );

#endif


  #ifdef BIG__ENDIAN
	/* for big endiannes machine reverse bytes order in words according to Big Endian  */
	CRYS_COMMON_INVERSE_UINT32_IN_ARRAY( KeyGenData_ptr->KGData.p, (KeySize/2+31)/32 );
	CRYS_COMMON_INVERSE_UINT32_IN_ARRAY( KeyGenData_ptr->KGData.q, (KeySize/2+31)/32 );
  #endif

   /* clean the n-buffer */
   DX_VOS_MemSetZero( PrivKey_ptr->n, 4*CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS );

 #endif /* LLF_PKI_ENGINE_TYPE != CRYS_DEFS_SSDMA_ENGINE_TYPE */

   /* ................ execute the low level keygen ........................... */
   Error = LLF_PKI_RSA_GenerateKeyPair( PubKey_ptr,
                                        PrivKey_ptr,
                                        KeyGenData_ptr );

   /* on failure exit the function */
   if( Error != CRYS_OK )

     goto End;

   /* ................ initialize the low level key structures ................ */
   /* ------------------------------------------------------------------------- */

   Error = LLF_PKI_RSA_InitPubKeyDb( PubKey_ptr );

   if( Error != CRYS_OK )

     goto End;

   Error = LLF_PKI_RSA_InitPrivKeyDb( PrivKey_ptr );

   if( Error != CRYS_OK )

     goto End;


   /* ............... START : Conditional test for Key-Generation ............. */
   /* ------------------------------------------------------------------------- */

 #ifndef CRYS_NO_RSA_SELF_TEST_SUPPORT

	/* Clean and fill the data buffer */
	DX_VOS_MemSet(KeyGenData_ptr->PrimData.DataIn, 0, CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*4);
	DX_VOS_MemSet(KeyGenData_ptr->PrimData.DataOut, 0, CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*4);
	DX_VOS_FastMemCpy(KeyGenData_ptr->PrimData.DataIn, Data_ptr, DataSize);

	/* Execute the encryption
	**************************/
	Error= LLF_PKI_RSA_ExecPubKeyExp(PubKey_ptr,
									(CRYS_RSAPrimeData_t *)&KeyGenData_ptr->PrimData );
	if( Error != CRYS_OK )
	{
		goto End;
	}
		/* if the result is the same as the original data set the test as failure */
		if( (DX_VOS_MemCmp( KeyGenData_ptr->PrimData.DataOut,
							Data_ptr,
							sizeof(Data_ptr) ))==0 )
		{
			DX_VOS_MemSet(PubKey_ptr, 0, sizeof(CRYS_RSAUserPubKey_t ) );
			DX_VOS_MemSet(PrivKey_ptr, 0, sizeof(CRYS_RSAUserPrivKey_t ) );
			#ifndef CRYS_NO_FIPS_SUPPORT
			DX_GLOBAL_FIPS_MODE|=DX_CRYS_FIPS_MODE_ERROR_STATE;
			#endif
			Error = CRYS_RSA_KEY_GEN_CONDITIONAL_TEST_FAIL_ERROR;
			goto End;
		}

	/* Clean and fill the data buffer */
	DX_VOS_FastMemCpy(KeyGenData_ptr->PrimData.DataIn, KeyGenData_ptr->PrimData.DataOut, CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*4);
	DX_VOS_MemSet(KeyGenData_ptr->PrimData.DataOut, 0, CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS*4);

	/* Execute the decryption
	**************************/
	Error = LLF_PKI_RSA_ExecPrivKeyExp( PrivKey_ptr,
										(CRYS_RSAPrimeData_t *)&KeyGenData_ptr->PrimData);
	if( Error != CRYS_OK )
	{
		return Error;
	}

	/* if the result is not the same as the original data set the test as failure */
		if(DX_VOS_MemCmp( KeyGenData_ptr->PrimData.DataOut,
						  Data_ptr,
						  DataSize ) )
		{
			DX_VOS_MemSet(PubKey_ptr, 0, sizeof(CRYS_RSAUserPubKey_t ) );
			DX_VOS_MemSet(PrivKey_ptr, 0, sizeof(CRYS_RSAUserPrivKey_t ) );
			#ifndef CRYS_NO_FIPS_SUPPORT
			DX_GLOBAL_FIPS_MODE|=DX_CRYS_FIPS_MODE_ERROR_STATE;
			#endif
			Error = CRYS_RSA_KEY_GEN_CONDITIONAL_TEST_FAIL_ERROR;
			goto End;
		}

	 /* Test Passed - return Error = CRYS_OK */

#endif /* CRYS_NO_RSA_SELF_TEST_SUPPORT */

   /* ............... END : Conditional test for Key-Generation ............... */
   /* ------------------------------------------------------------------------- */


   /* ................ set the key valid tags ................................. */
   /* ------------------------------------------------------------------------- */
   UserPrivKey_ptr->valid_tag = CRYS_RSA_PRIV_KEY_VALIDATION_TAG;
   UserPubKey_ptr->valid_tag  = CRYS_RSA_PUB_KEY_VALIDATION_TAG;

   End:

   /* clear the KG data structure */
   DX_VOS_MemSet ( KeyGenData_ptr , 0 , sizeof(CRYS_RSAKGData_t) );

   return Error;

   #endif /* !CRYS_NO_HASH_SUPPORT */
   #endif /* !CRYS_NO_PKI_SUPPORT */

}/* END OF CRYS_RSA_KG_GenerateKeyPair */


/***********************************************************************************************/
/**
   @brief CRYS_RSA_KG_GenerateKeyPairCRT generates a Pair of public and private keys on CRT mode.

   @param[in] PubExp_ptr - The pointer to the public exponent (public key)
   @param[in] PubExpSizeInBytes - The public exponent size in bits.
   @param[in] KeySize  - The size of the key, in bits. Supported sizes are:
                            - for PKI without PKA HW: all 256 bit multiples between 512 - 2048;
                            - for PKI with PKA: HW all 32 bit multiples between 512 - 2112;
   @param[out] UserPrivKey_ptr - A pointer to the private key structure.
                           This structure is used as input to the CRYS_RSA_PRIM_Decrypt API.
   @param[out] UserPubKey_ptr - A pointer to the public key structure.
                           This structure is used as input to the CRYS_RSA_PRIM_Encryped API.
   @param[in] KeyGenData_ptr - a pointer to a structure required for the KeyGen operation.

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_SIZE
*/

CEXPORT_C CRYSError_t CRYS_RSA_KG_GenerateKeyPairCRT(DxUint8_t               *PubExp_ptr,
                                           DxUint16_t               PubExpSizeInBytes,
                                           DxUint32_t               KeySize,
                                           CRYS_RSAUserPrivKey_t *UserPrivKey_ptr,
                                           CRYS_RSAUserPubKey_t  *UserPubKey_ptr,
                                           CRYS_RSAKGData_t      *KeyGenData_ptr )
{
   /* LOCAL INITIALIZATIONS AND DECLERATIONS */

   /* the error identifier */
   CRYSError_t Error;

   /* the pointers to the key structures */
   CRYSRSAPubKey_t  *PubKey_ptr;
   CRYSRSAPrivKey_t *PrivKey_ptr;

   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RSA_UNSUPPORTED( PubExp_ptr , PubExpSizeInBytes , KeySize ,
                              UserPrivKey_ptr , UserPubKey_ptr , KeyGenData_ptr ,
                              Error , PubKey_ptr ,
                              PrivKey_ptr , PrivKey_ptr , PrivKey_ptr , PrivKey_ptr ,
                              Error,Error,Error,Error,Error,Error,Error,Error,Error,Error);

   #ifndef CRYS_NO_HASH_SUPPORT
   #ifndef CRYS_NO_PKI_SUPPORT

   /* ................ initializations and local declarations ............ */
   /* ------------------------------------------------------------------- */

   /* initialize the error identifier to O.K */
   Error = CRYS_OK;


   /* ................. checking the validity of the pointer arguments ....... */
   /* ------------------------------------------------------------------------ */

   /* ...... checking the key database handle pointer .................... */
   if( PubExp_ptr == DX_NULL )

      return CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR;

   /* ...... checking the validity of the exponent pointer ............... */
   if( UserPrivKey_ptr == DX_NULL )

      return CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR;

   /* ...... checking the validity of the modulus pointer .............. */
   if( UserPubKey_ptr == DX_NULL )

      return CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR;

   /* ...... checking the validity of the keygen data .................. */
   if( KeyGenData_ptr == DX_NULL )

      return CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID;

   /* ...... checking the required key size ............................ */
   if( ( KeySize < CRYS_RSA_MIN_VALID_KEY_SIZE_VALUE_IN_BITS ) ||
       ( KeySize > CRYS_RSA_MAX_VALID_KEY_SIZE_VALUE_IN_BITS ) ||
       ( KeySize % CRYS_RSA_VALID_KEY_SIZE_MULTIPLE_VALUE_IN_BITS ) )

      return CRYS_RSA_INVALID_MODULUS_SIZE;

   /* set the public and private key structure pointers */
   PubKey_ptr  = ( CRYSRSAPubKey_t *)UserPubKey_ptr->PublicKeyDbBuff;
   PrivKey_ptr = ( CRYSRSAPrivKey_t *)UserPrivKey_ptr->PrivateKeyDbBuff;


   /* ................ clear all input structures ............................. */
   /* ------------------------------------------------------------------------- */

   DX_VOS_MemSet( UserPrivKey_ptr , 0 , sizeof(CRYS_RSAUserPrivKey_t) );
   DX_VOS_MemSet( UserPubKey_ptr  , 0 , sizeof(CRYS_RSAUserPubKey_t) );
   DX_VOS_MemSet( KeyGenData_ptr  , 0 , sizeof(CRYS_RSAKGData_t) );

   /* ................ loading the public exponent to the structure .......... */
   /* ------------------------------------------------------------------------- */

   /* loading the buffers to start from LS word to MS word */
   CRYS_COMMON_ReverseMemcpy( (DxUint8_t*)PubKey_ptr->e , PubExp_ptr , min(PubExpSizeInBytes , sizeof( PubKey_ptr->e )) );

   /* .......... initializing the effective counters size in bits .......... */
   PubKey_ptr->eSizeInBits = CRYS_COMMON_GetCounterEffectiveSizeInBits( (DxUint8_t*)PubKey_ptr->e,PubExpSizeInBytes );

   /* if the size in bits is 0 - return error */
   if( PubKey_ptr->eSizeInBits == 0 )
   {
      Error = CRYS_RSA_INVALID_EXPONENT_SIZE;
      goto End;
   }

   /* verifing the exponent has legal value (currently only 0x3,0x11 and 0x10001) */
   switch(PubKey_ptr->eSizeInBits)
   {
       case 2:
           if( PubKey_ptr->e[0] != 0x3)
           {
               Error = CRYS_RSA_INVALID_EXPONENT_VAL;
               goto End;
           }
           break;
       case 5:
           if( PubKey_ptr->e[0] != 0x11)
           {
               Error = CRYS_RSA_INVALID_EXPONENT_VAL;
               goto End;
           }
           break;
       case 17:
           if(PubKey_ptr->e[0] != 0x010001)
           {
               Error = CRYS_RSA_INVALID_EXPONENT_VAL;
               goto End;
           }
           break;
       default:
           Error = CRYS_RSA_INVALID_EXPONENT_SIZE;
           goto End;
   }
   /* .......... initializing the key size in bits ......................... */

   /* this initialization is required for the low level function (LLF) - indicates the required
      size of the key to be found */
   PubKey_ptr->nSizeInBits  = KeySize;
   PrivKey_ptr->nSizeInBits = KeySize;

   /* .......... set the private mode to CRT .................................. */
   /* ------------------------------------------------------------------------- */

   /* set the mode to CRT */
   PrivKey_ptr->OperationMode = CRYS_RSA_Crt;

   /* set the key source as internal */
   PrivKey_ptr->KeySource = CRYS_RSA_InternalKey;

   /* ................ executing the key generation ........................... */
   /* ------------------------------------------------------------------------- */

   /* ................ generate the prime1 and prime2 random numbers .......... */

   /* generate the random */
   Error = CRYS_RSA_GenerateVectorInRangeX931( KeySize / 2, (DxUint8_t*)KeyGenData_ptr->KGData.p,
                                               (DxUint8_t*)PrivKey_ptr->n /*as temp buff*/);
   if( Error != CRYS_OK )

     goto End;

   Error = CRYS_RSA_GenerateVectorInRangeX931( KeySize / 2, (DxUint8_t*)KeyGenData_ptr->KGData.q,
                                               (DxUint8_t*)PrivKey_ptr->n /*as temp buff*/);
   if( Error != CRYS_OK )

     goto End;

  #ifdef BIG__ENDIAN
	/* for big endianness machine reverse bytes order according to Big Endian words */
	CRYS_COMMON_INVERSE_UINT32_IN_ARRAY( KeyGenData_ptr->KGData.p, (KeySize/2+31)/32 );
	CRYS_COMMON_INVERSE_UINT32_IN_ARRAY( KeyGenData_ptr->KGData.q, (KeySize/2+31)/32 );
  #endif

   /* clean the n-buffer */
   DX_VOS_MemSetZero( PrivKey_ptr->n, 4*CRYS_RSA_MAXIMUM_MOD_BUFFER_SIZE_IN_WORDS );

   /* ................ execute the low level key gen ........................... */
   Error = LLF_PKI_RSA_GenerateKeyPair( PubKey_ptr,
                                        PrivKey_ptr,
                                        KeyGenData_ptr );

   /* on failure exit the function */
   if( Error != CRYS_OK )

     goto End;

   /* ................ set the vector sizes ................................... */
   /* ------------------------------------------------------------------------- */

   PrivKey_ptr->PriveKeyDb.Crt.PSizeInBits =
      CRYS_COMMON_GetCounterEffectiveSizeInBits((DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.P, (DxUint16_t)(KeySize / 16) );

   PrivKey_ptr->PriveKeyDb.Crt.QSizeInBits =
      CRYS_COMMON_GetCounterEffectiveSizeInBits((DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.Q, (DxUint16_t)(KeySize / 16) );

   PrivKey_ptr->PriveKeyDb.Crt.dPSizeInBits =
      CRYS_COMMON_GetCounterEffectiveSizeInBits((DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.dP, (DxUint16_t)(KeySize / 16) );

   PrivKey_ptr->PriveKeyDb.Crt.dQSizeInBits =
      CRYS_COMMON_GetCounterEffectiveSizeInBits((DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.dQ, (DxUint16_t)(KeySize / 16) );

   PrivKey_ptr->PriveKeyDb.Crt.qInvSizeInBits =
      CRYS_COMMON_GetCounterEffectiveSizeInBits((DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.qInv, (DxUint16_t)(KeySize / 16) );

   /* ................ initialize the low level key structures ................ */
   /* ------------------------------------------------------------------------- */

   Error = LLF_PKI_RSA_InitPubKeyDb( PubKey_ptr );

   if( Error != CRYS_OK )

     goto End;

   Error = LLF_PKI_RSA_InitPrivKeyDb( PrivKey_ptr );

   if( Error != CRYS_OK )

     goto End;

   /* ................ set the key valid tags ................................. */
   /* ------------------------------------------------------------------------- */

   UserPrivKey_ptr->valid_tag = CRYS_RSA_PRIV_KEY_VALIDATION_TAG;
   UserPubKey_ptr->valid_tag  = CRYS_RSA_PUB_KEY_VALIDATION_TAG;

   End:

   /* clear the KG data structure */
   DX_VOS_MemSet ( KeyGenData_ptr , 0 , sizeof(CRYS_RSAKGData_t) );

   return Error;

   #endif /* !CRYS_NO_HASH_SUPPORT */
   #endif /* !CRYS_NO_PKI_SUPPORT */

}/* END OF CRYS_RSA_KG_GenerateKeyPairCRT */


/**********************************************************************************************************/
/*
 * @brief The CRYS_RSA_GenerateVectorInRangeX931 function generates a random vector in range:
 *            MinVect < RandVect < MaxVect, where:
 *            MinVect = sqwRoot(2) * 2^(RndSizeInBits-1),  MaxVect = 2^RndSizeInBits.
 *
 *        This function is used in PKI RSA for random generation according to ANS X9.31 standard.
 *        If PKI_RSA is not supported, the function does nothing.
 *
 *        Functions algorithm::
 *
 *        1.  Calls the CRYS_RND_GenerateVector() function for generating random vector
 *            RndVect of size (RndSizeInBits, rounded up to bytes).
 *        2.  Zeroes extra bits of RndVect and sets MSbit to 1 (i.e. performs condition, related to MaxVect) .
 *        3.  Compares RndVect^2 with MinVect. If condition is not satisfied,
 *            then generates new random and checks it again, etc. (in loop).
 *
 *     ASSUMINGS: 1. RndVect_ptr and temp_ptr buffers and their sizes are aligned to 32-bit words.
 *                   Size of buffer for RndVect_ptr must be not less, than RndSizeInBits rounded up to
 *                   full words.
 *                2. RndSizeInBits is not less, than 32 bit.
 *                3. Bytes order of output vector is set from LSB to MSB as bytes stream.
 *                4. temp_ptr buffer must be aligned and its size must be not less, than 2*RndSize (in words).
 *
 * @RndSizeInBits[in]  - The size of random vectore that is required.
 * @RndVect_ptr[out]   - The output buffer of size not less, than RndSizeInBits, rounded up to words.
 * @temp_ptr[in]       - The pointer to temp buffer of size 2*(size of random, rounded to words).
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* as defined in ...
 */
CRYSError_t CRYS_RSA_GenerateVectorInRangeX931( DxUint32_t  RndSizeInBits,
                                                DxUint8_t  *RndVect_ptr,
                                                DxUint8_t  *temp_ptr )
{
   /* FUNCTION DECLARATIONS */

	CRYSError_t Error;

#ifdef CRYS_NO_PKI_SUPPORT
    /* prevent compiler warnings */
    RndSizeInBits = RndSizeInBits; RndVect_ptr = RndVect_ptr; temp_ptr = temp_ptr;

#else

	DxUint32_t  i, j;
	DxUint32_t  shift;
    DxUint32_t  RndSizeInBytes;
	DxUint32_t  RndSizeInWords;
    DxUint32_t  MSWord;

  /* define size of block random data according to used type of generator */
  #ifndef DX_USE_RND_HASH
    #define KG_RND_BLOCK_SIZE_IN_WORDS   CRYS_AES_BLOCK_SIZE_IN_WORDS
  #else
    #define KG_RND_BLOCK_SIZE_IN_WORDS   CRYS_HASH_SHA1_DIGEST_SIZE_IN_WORDS
  #endif

    /* temp buffer for random block */
    DxUint32_t  RndBlock[KG_RND_BLOCK_SIZE_IN_WORDS];


    /* MaxWord = MS word of 2^31 * sqwRoot(2), rounded down */
    DxUint32_t  MaxWord = 0xB504F333;


   /* FUNCTION LOGIC */

    /* Initializations */

	Error = CRYS_OK;

    /* size of Rnd in bytes and words */
	RndSizeInBytes = (RndSizeInBits + 7) / 8;
	RndSizeInWords = (RndSizeInBytes + 3) / 4;


   /* ................ executing the Rnd generation ........................... */
   /* ------------------------------------------------------------------------- */

   /* calculate Mask for extra bits in random S */
   shift = (8*RndSizeInBytes - RndSizeInBits);

   /* initialize number of rnd word, used from new rnd block */
   j = 0;

   /* main loop of random generation of not shifted vector */
   for(i = 0; i < 0x100; i++)
   {
	   /* generate the random vector of size = RndSizeInBytes */
		Error = CRYS_RND_GenerateVector( (DxUint16_t)RndSizeInBytes, (DxUint8_t*)RndVect_ptr );

		if( Error != CRYS_OK )
			goto EndWithError;

        /* Set 1 to MSbit of not shifted random S */
		RndVect_ptr[RndSizeInBytes - 1] |= 0x80;

		/* compare MS blocks of random data and Min vector. If MSblock not meets conditions,
		   then regenerate it, else generate full new random data */

		while(1)
		{
		    /* copy 4 MS bytes from random */
		    DX_VOS_FastMemCpy( &MSWord, &RndVect_ptr[RndSizeInBytes - 4], 4 );

		  #ifdef BIG__ENDIAN
				/* reverse bytes order in the word according to Big Endian */
				MSWord = CRYS_COMMON_REVERSE32( MSWord );
		  #endif

		    /* compare 4 MS bytes of RndVect to MaxWord */
            if( MSWord > MaxWord+1 )

                goto END;   /* the random is generated */

            /* compare MSWord+1 to MaxWord */
            if( MSWord+1 < MaxWord )
            {
			    /* generate new random MS block */
			    if( j == 0 )
			    {
					Error = CRYS_RND_GenerateVector(
					            (DxUint16_t)KG_RND_BLOCK_SIZE_IN_WORDS*4, (DxUint8_t*)RndBlock );

					if( Error != CRYS_OK )
						goto EndWithError;
			    }

		        /* copy new word into MS word of the random */
			    DX_VOS_FastMemCpy( RndVect_ptr+RndSizeInBytes-4, (DxUint8_t*)RndBlock + 4*j, 4 );

			    j++;   /* update the j */
			    j = j % KG_RND_BLOCK_SIZE_IN_WORDS;

		        /* Set 1 to MSbit of random S */
				RndVect_ptr[RndSizeInBytes - 1] |= 0x80;
            }

            else  /* compare square of full RndVect to 2^(RndSizeInBits-1) */
            {
                /* mask for not relevant bytes in MS word */
                DxUint32_t mask;

		  #ifdef BIG__ENDIAN
				/* reverse bytes order according to Big Endian for RMul operation */
				CRYS_COMMON_INVERSE_UINT32_IN_ARRAY( (DxUint32_t*)RndVect_ptr, (RndSizeInBytes+3)/4 );
		      #endif

		/* save MS word of RndVect_ptr buffer (it may include some bytes not belonging to RndVect) */
		MaxWord = ((DxUint32_t*)RndVect_ptr)[RndSizeInWords - 1];

		mask = 0xFFFFFFFF >> 8*(4*RndSizeInWords - RndSizeInBytes);

		/* mask the non relevant bytes in the MS word */
		((DxUint32_t*)RndVect_ptr)[RndSizeInWords - 1] &= mask;

		/* calculate Rnd^2 */
		Error = LLF_PKI_RSA_CallRMul( (DxUint32_t*)RndVect_ptr, RndSizeInBits,
                                   (DxUint32_t*)RndVect_ptr, (DxUint32_t*)temp_ptr );

				if( Error != CRYS_OK )
					goto EndWithError;

				/* restore the MS word */
		((DxUint32_t*)RndVect_ptr)[RndSizeInWords - 1] = MaxWord;


		  #ifdef BIG__ENDIAN
				/* reverse backward bytes order according bytes stream */
				CRYS_COMMON_INVERSE_UINT32_IN_ARRAY( (DxUint32_t*)RndVect_ptr, (RndSizeInBytes+3)/4 );

			    /* compare Rnd^2 to 2^(RndSizeInBits-1) for Big endian bytes order */
			    if( temp_ptr[2*RndSizeInBytes-4] > 0x80  )
				goto END;   /* the random is generated */

			  #else
			    /* compare Rnd^2 to 2^(RndSizeInBits-1) for little endian bytes order */
			    if( temp_ptr[2*RndSizeInBytes-1] > 0x80  )
				goto END;   /* the random is generated */
		      #endif

			break;  /* goto generation of full new random */
            }
		}
   }

   return CRYS_RSA_CAN_NOT_GENERATE_RAND_IN_RANGE;

  END:

   /* shift right the random S according to exact bit-size */
   CRYS_COMMON_ShiftRightVector( RndVect_ptr, RndSizeInBytes, (DxInt8_t)shift );

  EndWithError:

#endif   /*CRYS_NO_PKI_SUPPORT*/

   return Error;

} /* End of CRYS_RSA_GenerateVectorInRangeX931 function */

#endif /*_INTERNAL_CRYS_NO_RSA_KG_SUPPORT*/
