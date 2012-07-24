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
#include "DX_VOS_Sem.h"
#include "CRYS.h"
#include "LLF_AES_Restrict.h"
#include "CRYS_RND_error.h"
#include "CRYS_RND_local.h"
#include "LLF_RND.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
//#include "CRYS_SELF_TEST_Local.h"
#include "log_output.h"
#include "DX_VOS_Stdio.h"

/*
*  Object %name    : % CRYS_RND.c
*  State           :  %state%
*  Creation date   :  18 May 2010
*/
/** @file
*  \brief CRYS RND module varsion compliant to NIST 800-90 standard.
*         Based on CTR DRBG Block Cipher (AES)
*
*  \version CRYS_RND.c#1:csrc:6
*  \author R.Levin
*/

/************************ Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/*********************************** Enums ******************************/
/*********************************Typedefs ******************************/

/**************** Global Data to be read by RNG function ****************/

    /* global buffer for RNG working State */
extern CRYS_RND_State_t  CRYS_RND_WorkingState;


/*------------------------------------------------------*/
/* const data from the spec - uses in the LLF functions */
/*------------------------------------------------------*/

/*   A. Data for case Security Strength = 128 bit.
        Note: Key1 byffer is common for 128 and 256 bits */
#ifdef BIG__ENDIAN
const DxUint32_t Key1[8] =
{0x00010203,0x04050607,0x08090A0B,0x0C0D0E0F,
 0x10111213,0x14151617,0x18191A1B,0x1C1D1E1F};
const DxUint32_t InitialMac128[2][CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS] =
{ {0xc6a13b37,0x878f5b82,0x6f4f8162,0xa1c8d879},
  {0x9503e3a2,0x245a2be4,0x3c9874ed,0xfe1bed9e} };

#else
const DxUint32_t Key1[8] =
{0x03020100,0x07060504,0x0B0A0908,0x0F0E0D0C,
 0x13121110,0x17161514,0x1B1A1918,0x1F1E1D1C};
const DxUint32_t InitialMac128[2][CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS]=
{ {0x373ba1c6,0x825b8f87,0x62814f6f,0x79d8c8a1},
  {0xA2e30395,0xE42b5a24,0xED74983c,0x9Eed1bfe} };
#endif

/*   B. Data for case Security Strength = 256 bit.
        Note: Key1 buffer is common for 128 and 256 bits */
#ifdef BIG__ENDIAN
const DxUint32_t InitialMac256[3][CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS] =
{
  {0xF29000B6,0x2A499FD0,0xA9F39A6A,0xDD2E7780},
  {0x9DBA41A7,0x77F3B46A,0x37B7AAAE,0x49D6DF8D},
  {0x2F7A3C60,0x0708D124,0xACD3C5DE,0x3B658447} };
#else
const DxUint32_t InitialMac256[3][CRYS_RND_BASIC_BLOCK_SIZE_IN_WORDS]=
{
  {0xB60090F2,0xD09F492A,0x6A9AF3A9,0x80772EDD},
  {0xA741BA9D,0x6AB4F377,0xAEAAB737,0x8DDFD649},
  {0x603C7A2F,0x24D10807,0xDEC5D3AC,0x4784653B} };

#endif


/* importing the semaphore used for the access to the hardware machine */
#ifndef CRYS_NO_GLOBAL_DATA
extern DxVosSem SemRndId;       /* semaphore for TRNG */
extern DxVosSem SemHwAccessId;  /* semaphore for AES */
#endif



/************************************************************************************/
/***********************           Private functions            *********************/
/************************************************************************************/


/****************************************************************************************/
/**
 * @brief The function performs NIST 800-90, 10.2.1.2. algorithm of Update function.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 * @param[in] providedData_ptr - The pointer to provided data buffer. The size of data
 *                               must be exactly of size of Seed.
 * @param[in/out] seed_ptr - The pointer to the Seed = (Key || V) buffer.
 * @param[in] skipSetUp - Flag, if set, then first two steps of algorithm sould be skipped.
 * @param[in/out] AESRestrContext_ptr - The pointer to restricted AES context.
 *
 *   Note: Updated result (Key||V) are in Seed buffer of the State.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
static CRYSError_t DX_RND_Update(
						CRYS_RND_State_t     *State,               /*in/out*/
						DxUint32_t           *providedData_ptr,    /*in*/
						DxUint32_t           *seed_ptr,            /*in/out - Key,V*/
						DxUint8_t             skipSetUp,           /*in*/
						AESResrtictContext_t *AESRestrContext_ptr )    /*in/out*/
 {

	/* LOCAL DECLARATIONS */

    CRYSError_t  Error = CRYS_OK;

    DxUint16_t keySizeWords;

	DxUint32_t temp[4]= {0};

    /* size of seed */
    DxUint16_t seedSizeInWords;


	/* FUNCTION LOGIC */

    /* Initializations */

    keySizeWords = State->KeySizeWords;

    /* seed size in AES blocks */
    seedSizeInWords = keySizeWords + CRYS_AES_BLOCK_SIZE_IN_WORDS;

	/*----------------------------------------------------------------- */
	/*    NIST 800-90, 10.2.1.2. Algorithm of Update function           */
	/*  Where: output performed into StateSeed buffer without using of   */
	/*        temp buffer                                               */
	/*----------------------------------------------------------------- */

    /* if not set skipSetUp flag, then init AES on CTR mode and perform one
       dummy encrypt for incrementing IV */
    if( !skipSetUp )
    {
	    /* Init AES operation on CTR mode */
	Error = LLF_AES_RESTR_Init(
                       &seed_ptr[0],     /*calculated key*/
                       keySizeWords,
                       &seed_ptr[keySizeWords],   /*IV*/
                       CRYS_AES_CTR_mode,
                       CRYS_AES_Encrypt,
                       AESRestrContext_ptr ); /*AES restrict context*/

		if( Error != CRYS_OK )
		return Error;

	     /* 2.2.  Dummy encrypt for increment the IV: [1]2.1. V = (V+1) mod 2^outLenBits */
        Error = LLF_AES_RESTR_BlockExec(
                        temp,             /*in_ptr*/
                        temp,             /*out_ptr*/
                        1,                /*sizeInBlocks*/
                        AESRestrContext_ptr );  /*AES restrict context*/

		if( Error != CRYS_OK )
		return Error;
    }

    /* 2.2. Encrypt the SEED on AES CTR mode */
    Error = LLF_AES_RESTR_BlockExec(
                        providedData_ptr,     /*in_ptr*/
                        seed_ptr,             /*out_ptr*/
                        seedSizeInWords/CRYS_AES_BLOCK_SIZE_IN_WORDS, /*sizeInBlocks*/
                        AESRestrContext_ptr );  /*AES restrict context*/

	if( Error != CRYS_OK )
	return Error;

    /* close AES clocks */
	Error = LLF_AES_RESTR_Finish(
                       DX_NULL,            /*v_ptr - not need to save*/
                       AESRestrContext_ptr );  /*AES restrict context*/

	return Error;

} /* End of LLF_RND_Update */



/****************************************************************************************/
/**
 * @brief The function performs NIST 800-90, 10.2.1.4.2. algorithm of
 *        Seed Derivation function.
 *
 * @param[in/out] State - The pointer to the internal State buffer.
 * @param[in] entropySourceBuff_ptr - The pointer to entropy source buffer (see below).
 *                                    The buffer includes also auxiliary buffer.
 * @param[in] entropySourceSizeWords - The size of entropy source = actual size of
 *            input data to process.
 * @param[in] additionalInput_ptr - The pointer to additional input buffer (see below).
 * @param[in] additionalInputSizeWords - The size of additional input in words.
 * @param[out] output_ptr - The pointer to the output buffer buffer.
 * @param[in] outputSizeWords - The size output data in words; must be multiple of
 *                              AES block size.
 * @param[in/out] AESRestrContext_ptr - The pointer to restricted AES context.
 *
 *    Note:
 *       1. The entropySourceBuff_ptr buffer consists from the following parts:
 *              AuxBuff1[2] || Entropy[24]. The size of the Entropy buffer is the
 *              maximal allowed Entropy size.
 *       2. The size of Entropy part must be:
 *                           - on Slow mode: 192 bit for AES-128, 384 bit for AES-256;
 *                           - on Fast mode: 384 bit for AES-128, 768 bit for AES-256.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
static CRYSError_t DX_RND_DF(
						CRYS_RND_State_t     *State,                    /*in/out*/
						DxUint32_t           *entropySourceBuff_ptr,    /*in*/
						DxUint32_t            entropySourceSizeWords,   /*in*/
						DxUint32_t           *additionalInput_ptr,      /*in*/
						DxUint32_t            additionalInputSizeWords, /*in*/
						DxUint32_t           *output_ptr,               /*out*/
						DxUint16_t            outputSizeWords,          /*in*/
						AESResrtictContext_t *AESRestrContext_ptr )     /*in/out*/
 {

	/* LOCAL DECLARATIONS */

    CRYSError_t  Error = CRYS_OK;


    /* AES key size in words (defining also security strength) and its ID */
    DxUint16_t keySizeWords = State->KeySizeWords;

	/* temp buffers for AES calculations and temp pointer */
	DxUint32_t temp[4], tempSeed[CRYS_RND_SEED_MAX_SIZE_WORDS], *t_ptr;

	/* pointer to precomputed initial MAC vectors (two-dimensional*/
	DxUint32_t *initMac_ptr;

	DxUint32 L, N;
    /* loop counter and temp variables */
    DxInt16_t i;
    DxInt16_t nBlocks, remainWords;

   PRINTF("Entered DX_RND_DF\n");

	/* FUNCTION LOGIC */

    /* Initializations */

    keySizeWords = State->KeySizeWords;

	/*----------------------------------------------------------------- */
	/* [1]: NIST 800-90, 10.2.1.4.2. Block_Cipher_df Process.           */
	/*      Algorithm of Seed Derivation function                       */
	/*----------------------------------------------------------------- */

    /* if entropy is given, then set t_ptr on entropy buffer, else on temp buffer */
    if( entropySourceSizeWords > 0 )
		t_ptr = entropySourceBuff_ptr;
	else
	t_ptr = temp;

    /* set L, N values in bytes */
    L = 4*(entropySourceSizeWords + additionalInputSizeWords); /* L */
    N = 4*outputSizeWords /* N */;

    /* convert said words L,N, Pad[0] to big endian */
  #ifndef BIG__ENDIAN
    L = CRYS_COMMON_REVERSE32(L);
    N = CRYS_COMMON_REVERSE32(N);
  #endif


    /***********************************************************************/
    /* [1] 8: First loop: compression of  seed source material             */
    /***********************************************************************/
    for( i = 0; i < outputSizeWords / CRYS_AES_BLOCK_SIZE_IN_WORDS; i++ )
    {
	    DxInt8_t j;

	    /* if entropy is given, then set t_ptr on entropy buffer, else on temp buffer */
	    if( entropySourceSizeWords > 0 )
	    {
		entropySourceBuff_ptr[0] = L;
		entropySourceBuff_ptr[1] = N;
	    }
		else
		{
			temp[0] = L;
			temp[1] = N;
		}

	    /* set pointer to initial precomputed MAC value */
	    if( keySizeWords == 4 )
		initMac_ptr = (DxUint32_t*)&InitialMac128[i][0];
	    else
		initMac_ptr = (DxUint32_t*)&InitialMac256[i][0];

		/*   [1]8.1: Initialization of AES engine on CBC-MAC mode
		             with Key1 = 0x00010203..., IV = initMac[i]  */
	Error = LLF_AES_RESTR_Init(
                       (DxUint32_t*)&Key1[0],     /*predefined  key 000203...*/
                       keySizeWords,
                       initMac_ptr,   /*IV*/
                       CRYS_AES_MAC_mode,
                       CRYS_AES_Encrypt,
                       AESRestrContext_ptr );  /*AES restricted context*/

		if( Error != CRYS_OK ) {
                PRINTF("Failed in LLF_AES_RESTR_Init\n");
		return Error;}

	    /*------------------------------------------------------*/
	    /*-----------  Process entropy buffer ------------------*/
	    /*------------------------------------------------------*/
	    if( entropySourceSizeWords > 0 )
	    {
		    /* count of full AES blocks in Entropy source + Auxiliary1 buffer and remaining words */
		    nBlocks = (DxInt16_t)((2 + entropySourceSizeWords) >> 2);
		    remainWords = (DxInt16_t)((2 + entropySourceSizeWords) - nBlocks*CRYS_AES_BLOCK_SIZE_IN_WORDS);

		    /* 2. A: Calculate IV = MAC(Key, IV=i) on full blocks of entropy source (without output) */
		    /*---------------------------------------------------------------------------------------*/
	        Error = LLF_AES_RESTR_BlockExec(
	                        entropySourceBuff_ptr,  /*in_ptr*/
	                        temp,                   /*out_ptr*/
	                        nBlocks,                /*sizeInBlocks*/
	                        AESRestrContext_ptr );  /*AES restricted context*/

			if( Error != CRYS_OK ) {
                        PRINTF("Failed in LLF_AES_RESTR_BlockExec\n");
			return Error;}

		    /* if remain words of entropy are present, then copy them into temp buffer
		       and fill buffer by additional data to full AES block */
		    DX_VOS_FastMemCpy( temp, &entropySourceBuff_ptr[2 + entropySourceSizeWords - remainWords],
		                       remainWords*sizeof(DxUint32_t) );
	    }
	    else
	    {
		    /* set not processed words L,N into temp buffer and set remain size accordingly */
		    remainWords = 2;
	    }

	    /* 2. B: Continue calculation of  IV = MAC(Key, IV=i) on remainWords and
	             additional data (without output)
           --------------------------------------------------------------------*/
	    if( additionalInputSizeWords > 0 )
	    {
	        /* fill temp buffer by additional data to full AES block */
		    DX_VOS_FastMemCpy( &temp[remainWords], additionalInput_ptr,
		                       (CRYS_AES_BLOCK_SIZE_IN_WORDS-remainWords)*sizeof(DxUint32_t) );

	        /* set pointer t_ptr to next word of additional data to process */
	        t_ptr = &additionalInput_ptr[remainWords];

		    /* count of full AES blocks in additional input plus previous remaining words */
		    nBlocks = (DxInt16_t)((remainWords + additionalInputSizeWords) >> 2 );
		    /* new value of remainWords for additional data */
		    remainWords = (DxInt16_t)((remainWords + additionalInputSizeWords) - nBlocks*CRYS_AES_BLOCK_SIZE_IN_WORDS);

		    /* 2. C: Calculate IV = MAC(Key, IV=i) on full blocks of additional data (without output) */
		    /*----------------------------------------------------------------------------------------*/
		    for( j = 0; j < nBlocks; j++ )
		    {
		        Error = LLF_AES_RESTR_BlockExec(
	                        temp,       /*in_ptr*/
	                        temp,       /*out_ptr*/
	                        1,          /*sizeInBlocks*/
	                        AESRestrContext_ptr );  /*AES restricted context*/

				if( Error != CRYS_OK )
				return Error;

	            /* copy next block of additional data into temp and increment the pointer */
	            if( j < nBlocks - 1 )
	            {
		            DX_VOS_FastMemCpy( temp, t_ptr, CRYS_AES_BLOCK_SIZE_IN_BYTES );
		            t_ptr += CRYS_AES_BLOCK_SIZE_IN_WORDS;
	            }
	            else
	            {
		            DX_VOS_FastMemCpy( temp, t_ptr, sizeof(DxUint32_t)*remainWords );
		            t_ptr += remainWords;
	            }
		    }
	    }

        /* Set padding into temp buffer. Note: remaining words of seed material data are copied
           into temp buffer yet */
      #ifndef BIG__ENDIAN
        temp[remainWords] = 0x00000080;
      #else
        temp[remainWords] = 0x80000000;
      #endif
        DX_VOS_MemSetZero( &temp[remainWords+1], (CRYS_AES_BLOCK_SIZE_IN_WORDS - remainWords-1)*sizeof(DxUint32_t) );


	    /* 2. D: Calculate and output IV = MAC(Key, IV=i) on remaining additional
	          data with padding by  0x80,0x0 ....
	     -------------------------------------------------------------------*/
        Error = LLF_AES_RESTR_BlockExec(
                        temp,                   /*in_ptr*/
                        &tempSeed[i*CRYS_AES_BLOCK_SIZE_IN_WORDS], /*out_ptr*/
                        1,                /*sizeInBlocks*/
                        AESRestrContext_ptr );  /*AES restricted context*/

		if( Error != CRYS_OK )
		return Error;

    } /* End of [1] 8: First loop: compression of  seed source material */


    /***********************************************************************/
    /* [1] 9,10: Encryption of previous output (seed material) data
             Let now the Key = leftmost bits of output_ptr buffer
             and the IV=X[0] = next outlen bits. Initialize AES with these
             parameters on AES-CBC mode */
    /***********************************************************************/

    /* Init AES on CBC mode */
	Error = LLF_AES_RESTR_Init(
                       &tempSeed[0],  /*calculated key*/
                       keySizeWords,
                       &tempSeed[keySizeWords],   /*IV*/
                       CRYS_AES_CBC_mode,
                       CRYS_AES_Encrypt,
                       AESRestrContext_ptr );  /*AES restricted context*/

	if( Error != CRYS_OK )
	return Error;

    /* ---------------------------------------------------------------------------------
       [1] 12: Calculate new encrypted seed:
         For i from 1 to nBlocs:
            X[i] = ECB_EncryptOneBlock(Key,X[i-1]); output = concatenation of X[i].
       This is equal to:
         Set IV = X[0], Input = 0000...
              out = CBC_EncryptOneBloc(Key,IV,input=0, nBlocks);
      ---------------------------------------------------------------------------------- */

    /* set 0 into tempSeed buffer because it is not needed now */
    DX_VOS_MemSetZero( tempSeed, outputSizeWords*sizeof(DxUint32_t) );

    Error = LLF_AES_RESTR_BlockExec(
                        tempSeed,   /*in_ptr = 00000...*/
                        output_ptr, /*out_ptr*/
                        outputSizeWords/CRYS_AES_BLOCK_SIZE_IN_WORDS, /*sizeInBlocks*/
                        AESRestrContext_ptr );  /*AES restricted context*/

	if( Error != CRYS_OK )
	return Error;

    /* close AES clocks */
	Error = LLF_AES_RESTR_Finish(
	                       DX_NULL,            /*v_ptr - not need to save*/
	                       AESRestrContext_ptr ); /*AES restricted context*/

	return Error;

} /* END of LLF_RND_DF */


/****************************************************************************************/
/**
 * @brief The function performs: NIST 800-90, 10.2.1.3.2  Instantiate function or
 *        NIST 800-90, 10.2.1.4.2 Reseeding function according to given flag.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @param[in] isInstantiate - The flag defining which algorithm to perform:
 *                            0 - Instantiate; 1 - Reseeding.
 * @param[in/out] tempBuff_ptr - The temp buffer for specific operations
 *                               on entropy estimator.
 *                         Note: for projects, which not use entropy estimator (e.g.
 *                               SW projects), the pointer may be set to NULL.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 CRYSError_t DX_RND_InstantiateOrReseed(
                        CRYS_RND_State_t     *State,           /*in/out*/
                        CRYS_RND_Params_t    *trngParams,      /*in/out*/
						DxInt8_t              isInstantiate,   /*in*/
						CRYS_RND_EntropyEstimatData_t *entrEstimBuff_ptr )   /*in/out*/

 {

	/* LOCAL DECLARATIONS */

    CRYSError_t  Error = CRYS_OK;

    DxUint16_t keySizeWords, entropySizeWords;

	/* Entropy temp buffer consists from the following parts:
        AuxBuff1[2] || Entropy[24]. The actual size of Entropy source must be:
              - on Slow mode: 192 bit for AES-128, 384 bit for AES-256;
              - on Fast mode: 384 bit for AES-128, 768 bit for AES-256.  */
    DxUint32_t EntropyTemp[CRYS_RND_ENTROPY_TEMP_BUFFER_MAX_SIZE_WORDS] = {0};

    /* seedTemp buffer */
    DxUint32_t SeedTemp[CRYS_RND_SEED_MAX_SIZE_WORDS ] = {0};

    /* restricted AES context */
	AESResrtictContext_t  AESRestrContext;


    /* ............. check parameters ............... */
    /*------------------------------------------------*/

	if( State == DX_NULL )
	return LLF_RND_STATE_PTR_INVALID_ERROR;

    /* For Reseeding check, that Instantiate was done */
    if( isInstantiate == 0 /*Reseeding*/ &&
        !(State->StateFlag & CRYS_RND_Instantiated) )

	return CRYS_RND_INSTANTIATION_NOT_DONE_ERROR;


	/* FUNCTION LOGIC */


    /* Initializations */
    /*-----------------*/

    /* Initial operations for starting TRNG and AES_RESTRICTED operations:
       (gets semaphore for RND and AES engine on specific platforms) */
    Error = LLF_AES_RESTR_StartInit();
    if( Error != CRYS_OK )
	goto End;

    Error = LLF_RND_Init();
    if( Error != CRYS_OK )
	goto End;

    /* key size */
    keySizeWords = State->KeySizeWords;

	/* Set Entropy source size according to performed operation, security
	   strength and TRNG mode:
	     - for reseeding: Size = Key size;
	     - for instantiation: Size = Key size + keySizeWords/2,
	       note: in the last case the Size includes random Nonce
	     - if TRNG mod = Fast then entropy source size must be multiplied
	       by 2, because the source contains only 50% of true entropy */
	if( isInstantiate == 0 )
		entropySizeWords = keySizeWords;         /*Reseeding*/
	else
	{
		/*Instantiation:  size = (3/2)*keySize */
		entropySizeWords = keySizeWords + (keySizeWords >> 1);
	    /* Set instantiation, sample loss and time exceeding  bits of the state = 0 */
		State->StateFlag &= ~( CRYS_RND_Instantiated |
		                       CRYS_RND_InstantReseedLossSamples |
		                       CRYS_RND_InstantReseedTimeExceed );
	}

    /* if RND KAT or KAT of Entropy estimator should be performed */
    if( State->StateFlag & CRYS_RND_KAT_mode )
    {
	    /* Copy known entropy to the context */
		DX_VOS_FastMemCpy( &EntropyTemp[2], CRYS_RND_WorkingState.Entropy,
						   sizeof(DxUint32_t)*entropySizeWords );
    }

    else
    {
	    /* in working mode get size of needed TRNG source for receiving required Entropy:
	       in - EntropySize, out - Entropy Source Size */
	    LLF_RND_GetEntropySourceSize( trngParams, &entropySizeWords /*in/out*/);

	    /* Get entropy (including random Nonce) from TRNG and set
	       it into Entropy Temp buffer. Update the needed size of
	       TRNG source for receiving required entropy    */
	    /*--------------------------------------------------------*/

	    Error = LLF_RND_GetTrngSource( State,            /*in*/
	                                   trngParams,       /*in/out*/
	                                   entropySizeWords, /*in*/
	                                  &EntropyTemp[2],   /*out*/
	                                  &AESRestrContext,  /*in/out*/
	                                   entrEstimBuff_ptr ); /*in*/

	    if(  Error != CRYS_OK )
		goto EndWithError;

    }

	/*--------------------------------------------------------------------- */
	/*   [1] NIST 800-90: 10.2.1.3.2  Instantiate or  10.2.1.4.2 Reseeding  */
	/*--------------------------------------------------------------------- */

#ifndef CRYS_RND_NOT_USE_DERIVATION_FUNCTION
    /* 2.1. if Derivation Function is used, call it */
    Error = DX_RND_DF(
					State,                     /*in*/
				EntropyTemp,               /*in buffer - data starts from word 2*/
					entropySizeWords,          /*in - size of entropy */
				    State->AdditionalInput,    /*in - additional data*/
				    State->AddInputSizeWords,  /*in - size of additional data in words*/
					SeedTemp,                  /*out - seed material*/
                    keySizeWords + CRYS_AES_BLOCK_SIZE_IN_WORDS,  /*in  - *Seed size needed*/
                    &AESRestrContext );        /*AES restricted context*/

    if(  Error != CRYS_OK )
	goto EndWithError;

#else  /*  if Derivation Function is not used, perform XOR operation */
     {
		DxUint16_t i;

	    DX_VOS_FastMemCpy( SeedTemp, EntropyTemp, sizeof(DxUint32_t)*entropySizeWords );

	    for( i = 0; i < State->AddInputSizeWords; i++  )
	    {
			SeedTemp[i] = SeedTemp[i] ^ State->AdditionalInput[i];
	    }
     }

#endif

	/* 3,4: Set Key = 0x00000... and IV = 0x0000... into Seed buffer */
    if( isInstantiate == 1 )
    {
	DX_VOS_MemSetZero( State->Seed, sizeof(DxUint32_t)*(keySizeWords + CRYS_AES_BLOCK_SIZE_IN_WORDS) );
    }

    /* 2.2. Call Update for Additional data */
    Error = DX_RND_Update(
					    State,              /*in/out*/
					    SeedTemp,           /*in - provided data*/
					    State->Seed,        /*in/out - Key||V inside the state: */
						0,                  /*in - skipSetUp*/
						&AESRestrContext ); /*AES restricted context*/

    if(  Error != CRYS_OK )
	goto EndWithError;

    /* [1] 6:  Reset State parameters           */
    /*------------------------------------------*/

    State->ReseedCounter = 1;

    /* in Instantiate mode set Instantiation flag = 1
       and previous value flag as "invalid" */
    if( isInstantiate == 1 )
    {
	State->StateFlag |= CRYS_RND_Instantiated;
	    State->StateFlag &= ~CRYS_RND_PreviousIsValid;
    }

    /* Reset additional input  */
    State->AddInputSizeWords = 0;
   DX_VOS_MemSetZero( State->AdditionalInput, sizeof(State->AdditionalInput) );

    /* check StateFlag and return Error if operations was not done with full entropy */
    if( State->StateFlag & CRYS_RND_InstantReseedLossSamples )
    {
	Error = CRYS_RND_TRNG_LOSS_SAMPLES_ERROR;
        goto End;
    }

    else if( State->StateFlag & CRYS_RND_InstantReseedTimeExceed )
    {
	Error = CRYS_RND_TRNG_TIME_EXCEED_ERROR;
        goto End;
    }

	/* ................. end of function ........... */
	/* --------------------------------------------- */
EndWithError:
    /* In case of error clean the State buffer */
  DX_VOS_MemSetZero( &State, sizeof(State) );

End:
    /* Ending of RND and AES_RESTRICTED operations (releasing the RND and AES
       semaphores on specific platforms) */
    LLF_AES_RESTR_EndFinish();
    LLF_RND_Finish();

	return Error;

} /* End of DX_RND_InstantiateOrReseed function */


/****************************************************************************************/
/**
 * @brief The function performs NIST 800-90, 10.2.1.5.2 algorithm of Generate function.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 * @param[in] out_ptr - The pointer to provided data buffer. The size of data
 *                       must be exactly of size of Seed.
 * @param[in] outSizeBytes - The required size of random data in bytes.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t DX_RND_Generate(
						CRYS_RND_State_t *State,           /*in/out*/
						DxUint8_t        *out_ptr,         /*out*/
						DxUint16_t        outSizeBytes )   /*in*/
 {

	/* LOCAL DECLARATIONS */

    CRYSError_t  Error = CRYS_OK;

    DxUint16_t keySizeWords, seedSizeWords;

    DxUint32_t AdditionalTemp[8];
    DxUint32_t temp[CRYS_RND_SEED_MAX_SIZE_WORDS];
    DxUint16_t countBlocks, remainBytes;
    DxUint32_t i;
    /* restricted AES context */
	AESResrtictContext_t  AESRestrContext;
   PRINTF("Entered DX_RND_Generate\n");

    /* FUNCTION LOGIC */

    /* ............. check parameters ............... */
    /*------------------------------------------------*/

	if( State == DX_NULL ){
       PRINTF("State == DX_NULL \n");
	return LLF_RND_STATE_PTR_INVALID_ERROR;}

    /* Check, that instantiation was done */
    if( !(State->StateFlag & CRYS_RND_Instantiated) ){
         DX_VOS_Printf("State->StateFlag =%d",State->StateFlag);
        DX_VOS_Printf("CRYS_RND_Instantiated = %d\n",CRYS_RND_Instantiated);
	return CRYS_RND_INSTANTIATION_NOT_DONE_ERROR;		}

    /* [1] 1: check Reseed counter of the State.
       Note: In [1] reseedCounter must be less than 2^48. In our implementation
       supplied more severe limitation of this parameter(counter < 2^32)that may
       only increase security */
    if( State->ReseedCounter >= CRYS_RND_MAX_RESEED_COUNTER ){
       PRINTF("State->ReseedCounter =%d",State->ReseedCounter);
	return CRYS_RND_RESEED_COUNTER_OVERFLOW_ERROR;}


	/* FUNCTION LOGIC */


    /* Initializations */
    /*-----------------*/

    /* Initial operation for starting AES_RESTRICTED operations, used in algorithm:
       (gets semaphore for AES engine on specific platforms) */
    Error = LLF_AES_RESTR_StartInit();

    if(  Error != CRYS_OK ){
       PRINTF("Failed in LLF_AES_RESTR_StartInit\n");
	goto End;}

    /* set seed, key size and keyID */
    keySizeWords = State->KeySizeWords;
    seedSizeWords = keySizeWords + CRYS_AES_BLOCK_SIZE_IN_WORDS;

    /* required count of full output blocks and remaining size in bytes */
    countBlocks = outSizeBytes / CRYS_AES_BLOCK_SIZE_IN_BYTES;
    remainBytes = outSizeBytes - countBlocks * CRYS_AES_BLOCK_SIZE_IN_BYTES;
    if( remainBytes > 0 )
	countBlocks++;

    /* zeroing temp buffers */
    DX_VOS_MemSetZero( AdditionalTemp, sizeof(DxUint32_t)*8 );
    DX_VOS_MemSetZero( temp, sizeof(DxUint32_t)*CRYS_RND_SEED_MAX_SIZE_WORDS);


	/*----------------------------------------------------------------- */
	/*   [1] NIST 800-90, 10.2.1.5.2. CTR_DRBG Generate Process         */
	/*----------------------------------------------------------------- */

    /* [1] 2:  If additional input valid, then call Derivation and Update functions */
    if( State->AddInputSizeWords  > 0 )
    {
	    /* if additional input size < seed len, then padded by 0-s */
	    if( State->AddInputSizeWords < seedSizeWords )
	    {
		DX_VOS_MemSetZero( &State->AdditionalInput[State->AddInputSizeWords],
		                    sizeof(DxUint32_t)*(seedSizeWords - State->AddInputSizeWords) );
	    }

  #ifndef CRYS_RND_NOT_USE_DERIVATION_FUNCTION

	    /* 2.1. Derivation Function call. If prediction resistance */
	    Error = DX_RND_DF(
						State,                     /*in*/
						DX_NULL,                   /*in - entropy source not needed */
						0,                         /*in - entropySourseSize*/
						State->AdditionalInput,    /*in - AdditionalInput*/
						State->AddInputSizeWords,  /*in - AddInputSizeWords*/
						State->AdditionalInput,    /*out - recalculated additional data*/
						seedSizeWords,             /*outputSizeWords - should be of seed size*/
						&AESRestrContext );        /*restricted AES context*/

	    if(  Error != CRYS_OK ){
                PRINTF("Failed in DX_RND_DF\n");
		goto End;}
  #endif

	    /* 2.2. Call Update with recalculated additional (provided) data */
	    Error = DX_RND_Update(
						    State,                  /*in/out*/
							State->AdditionalInput, /*in - provided data*/
							State->Seed,            /*in/out - Key||V*/
						    0,                      /*in - skipSetUp*/
						    &AESRestrContext );     /*restricted AES context*/

	    if(  Error != CRYS_OK ){
               PRINTF("Failed in DX_RND_Update\n");
		goto End;}

    }
    else   /* 2.3. Set AdditionalInput = 000000.... into temp buffer */
    {
	DX_VOS_MemSetZero( State->AdditionalInput, sizeof(State->AdditionalInput) );
    }


    /*------------------------------------------------------------------------------*/
    /* [1] 4: Calculation of random: In loop {V = V+1; out = AES_ECB(Key,CTR=V)}    */
    /*        Note: This algorithm is equaled to out = AES_CTR(Key,dataIn=00000...) */
    /*------------------------------------------------------------------------------*/

    /* Set one block = 0 in AdditionalTemp buffer because it is not used yet */
    DX_VOS_MemSetZero( &AdditionalTemp[CRYS_AES_BLOCK_SIZE_IN_WORDS], CRYS_AES_BLOCK_SIZE_IN_BYTES );

    /* increment counter V = V+1 */
	CRYS_COMMON_IncMsbUnsignedCounter(  &State->Seed[keySizeWords],
		                             1/*val*/, 4 /*size in words*/ );

	/*   Initialization of AES engine with Key on CTR mode */
	Error = LLF_AES_RESTR_Init(
                       &State->Seed[0],   /*calculated key*/
                       keySizeWords,
                       &State->Seed[keySizeWords],/*CTR=V*/
                       CRYS_AES_CTR_mode,
                       CRYS_AES_Encrypt,
                       &AESRestrContext );  /*AES special buffer*/

    if(  Error != CRYS_OK ){
        PRINTF("Failed in LLF_AES_RESTR_Init\n");
	goto End;
    }

    /* 4.2.  Call AES Block execution on CTR mode */
    for( i = 0; i < countBlocks; i++ )
    {
	    /* Generate one block of size 16 bytes of random data */
	    /*----------------------------------------------------*/
PerformGeneration:

             Error = LLF_AES_RESTR_BlockExec(
                        &AdditionalTemp[CRYS_AES_BLOCK_SIZE_IN_WORDS], /*in_ptr = 00000...*/
                        &AdditionalTemp[0], /*out_ptr*/
                        1, /*sizeInBlocks*/
                        &AESRestrContext );  /*AES special buffer*/

		if( Error != CRYS_OK ) {
                PRINTF("Failed in LLF_AES_RESTR_BlockExec\n");
		goto End;}

       /*  Perform CPRNGT - continuous test on each block: */
       /*--------------------------------------------------*/

	   if( !(State->StateFlag & CRYS_RND_KAT_mode) ) /* working mode */
	   {
		   /*  If isPreviousValid == 0, then save first result
		       in the State buffer for comparing and not output this value */
		   if( !(State->StateFlag & CRYS_RND_PreviousIsValid) )
		   {
			    /* save output into global buffer */
			    DX_VOS_FastMemCpy( State->PreviousRandValue, AdditionalTemp,
			                       CRYS_AES_IV_COUNTER_SIZE_IN_BYTES );
		        /* set previous valid */
			    State->StateFlag |= CRYS_RND_PreviousIsValid;

			    goto PerformGeneration;
		   }

		   else
		   {
			    /* CTRNGT: compare current with previous value */
			    if( DX_VOS_MemCmp( State->PreviousRandValue, AdditionalTemp,
			                       CRYS_AES_BLOCK_SIZE_IN_BYTES ) == 0 )
			    {
			    #ifndef CRYS_NO_FIPS_SUPPORT
				DX_GLOBAL_FIPS_MODE|= DX_CRYS_FIPS_MODE_ERROR_STATE;
			    #endif
				Error =  LLF_RND_CPRNG_TEST_FAIL_ERROR;
				goto End;
			    }

			    /* update previous */
			    DX_VOS_FastMemCpy( State->PreviousRandValue, AdditionalTemp,
			                       CRYS_AES_BLOCK_SIZE_IN_BYTES );
		   }
	   }

	    /* output the generated data */
	    /*---------------------------*/
	if( remainBytes > 0 && i == countBlocks-1 )
		DX_VOS_FastMemCpy( out_ptr, AdditionalTemp, remainBytes );
	else
		DX_VOS_FastMemCpy( out_ptr, AdditionalTemp, CRYS_AES_BLOCK_SIZE_IN_BYTES );

	/* increment output pointer */
	out_ptr += CRYS_AES_IV_COUNTER_SIZE_IN_BYTES;

    }

	/* increment counter V = V+1 */
	CRYS_COMMON_IncMsbUnsignedCounter(  &State->Seed[keySizeWords],
		                             countBlocks/*val*/, 4 /*size in words*/ );

    /*------------------------------------------*/
    /* [1] 6:    Update Key,V in the State      */
    /*------------------------------------------*/
    Error = DX_RND_Update(
					    State,                   /*in/out*/
					    State->AdditionalInput,  /*in - saved additional input */
						State->Seed,             /*in/out - Key||V*/
                        1,                       /*skipSetUp*/
                        &AESRestrContext );      /*AES special buffer*/

    if(  Error != CRYS_OK ){
              PRINTF("Failed in DX_RND_Update2\n");
		goto End;}

    /* [1] 6:    Increment Reseed counter       */
    /*------------------------------------------*/
    State->ReseedCounter++;

    /* Reset additional input  */
    State->AddInputSizeWords = 0;
    DX_VOS_MemSetZero( State->AdditionalInput, sizeof(State->AdditionalInput) );

	/* ................. end of function ........... */
	/* --------------------------------------------- */

End:
    /* End of AES_RESTRICTED operations  */
    LLF_AES_RESTR_EndFinish();

	return Error;

} /* End of DX_RND_Generate */


/****************************************************************************************/
/*****************************       Public Functions      ******************************/
/****************************************************************************************/


/*****************************************************************************************
 * @brief The CRYS_RND_GenerateVector function generates a random vector,
 *			 using the FIPS-PUB [SP800-90].
 *
 *        The random function is based on the AES.
 *
 *
 * @RndSize[in] - The size of random bytes that is required.
 *
 * @Output_ptr[in,out] The output vector.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CEXPORT_C CRYSError_t CRYS_RND_GenerateVector( DxUint16_t  RndSize,
											   DxUint8_t  *Output_ptr )
{
   /* FUNCTION DECLERATIONS */

   /* error identifier definition */
   CRYSError_t Error;


   /* FUNCTION LOGIC */

   /* ................... initializing local variables ..................... */
   /* ---------------------------------------------------------------------- */

   /* initialize the error identifier to O.K */
   Error = CRYS_OK;


   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RND_UNSUPPORTED( RndSize , Output_ptr , Error ,
                              Error , Error , Error ,
                              Error , Error , Error , Error );

#ifndef CRYS_NO_AES_RESTRICT_SUPPORT

   /* ................... checking parameters validity ..................... */
   /* ---------------------------------------------------------------------- */

   if( Output_ptr == DX_NULL )
     return CRYS_RND_DATA_OUT_POINTER_INVALID_ERROR;

   if (RndSize == 0)
	return CRYS_OK ; /* because of PSS*/

   /* ................... executing the random operation ................... */
   /* ---------------------------------------------------------------------- */
   Error = DX_RND_Generate( &CRYS_RND_WorkingState, Output_ptr, RndSize);


   return Error;

#endif /*CRYS_NO_AES_SUPPORT*/

}/* END OF CRYS_RND_GenerateVector */



/* -----------------------------------------------------------------------------
 * @brief The function performs instantiation of RNG and creates new
 *        internal State (including Seed) of RNG.
 *
 *        It implements the CTR_DRBG_Instantiate function of 9.1 [1].
 *	  This function must be called at least once per system reset (boot) and
 *        required before any random generation can be produced.
 *
 * @param[in/out] entrEstimBuff_ptr - The temp buffer for specific operations
 *                               on entropy estimator.
 *                         Note: for projects, which not use entropy estimator (e.g.
 *                               SW projects), the pointer may be set to NULL.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CEXPORT_C CRYSError_t CRYS_RND_Instantiation( CRYS_RND_EntropyEstimatData_t  *entrEstimBuff_ptr/*in/out*/ )
{

   /* error identifier definition */
 CRYSError_t Error = CRYS_OK;
   /* TRNG parameters structure */
   CRYS_RND_Params_t  TrngParams;

   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   RETURN_IF_RND_UNSUPPORTED( entrEstimBuff_ptr , TrngParams.TrngMode , Error , Error ,
                            Error , Error , Error , Error , Error , Error );

#ifndef CRYS_NO_AES_RESTRICT_SUPPORT

    /* check parameters */
    if( entrEstimBuff_ptr == DX_NULL )
	return CRYS_RND_ESTIMATOR_BUFFER_PTR_INVALID_ERROR;

	/* set users TRNG parameters into working State and TrngParams structures */
    Error = LLF_RND_SetUserRngParameters( &CRYS_RND_WorkingState, &TrngParams );

    if( Error != CRYS_OK )
	return Error;

	/* calling the DX_RND_InstantiateOrReseed function on Instantiation mode */
	Error = DX_RND_InstantiateOrReseed( &CRYS_RND_WorkingState,
	                                    &TrngParams,
	                                     1/*isInstantiate*/,
                                         entrEstimBuff_ptr );

#endif /*CRYS_NO_AES_SUPPORT*/

   return Error;

}

#ifndef _INTERNAL_CRYS_ONE_SEED
/* -----------------------------------------------------------------------------
 * @brief The CRYS_RND_UnInstantiation cleans the unused RNG State for security goals.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
/*CEXPORT_C CRYSError_t CRYS_RND_UnInstantiation(void)
{
	CRYSError_t Error = CRYS_OK;

    RETURN_IF_RND_UNSUPPORTED( Error , Error , Error , Error , Error ,
                              Error , Error , Error , Error , Error );

    DX_VOS_MemSetZero( &CRYS_RND_WorkingState, sizeof(CRYS_RND_WorkingState) );

    return Error;
}*/


/* ------------------------------------------------------------/
 * @brief The function performs reseeding of RNG Seed, and performs:
 *	     1. Mixing of additional entropy into the working state.
 *		 2. Mixing additional input provided by the user called additional input buffer.
 *
 *        The function implements the CTR_DRBG_Reseeding function of 9.2 [1].
 *	  This function must be called if reseed counter > reseed interval,
 *        in our implementation it is 2^32-1.
 *
 * @param[in/out] entrEstimBuff_ptr - The temp buffer for specific operations
 *                               on entropy estimator.
 *                         Note: for projects, which not use entropy estimator (e.g.
 *                               SW projects), the pointer may be set to NULL.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
//CEXPORT_C CRYSError_t CRYS_RND_Reseeding(  CRYS_RND_EntropyEstimatData_t  *entrEstimBuff_ptr/*in/out*/ )
//{
   /* FUNCTION DECLERATIONS */

   /* error identifier definition */
  // CRYSError_t Error;

   /* TRNG parameters structure */
   //CRYS_RND_Params_t  TrngParams;

   /* FUNCTION LOGIC */

   /* ............... if not supported exit .............................. */
   /* -------------------------------------------------------------------- */

   //RETURN_IF_RND_UNSUPPORTED( entrEstimBuff_ptr , TrngParams.TrngMode , Error , Error ,
     //                         Error , Error , Error , Error , Error ,  Error );

 //#ifndef CRYS_NO_AES_RESTRICT_SUPPORT

    /* check parameters */
   // if( entrEstimBuff_ptr == DX_NULL )
    //	return CRYS_RND_ESTIMATOR_BUFFER_PTR_INVALID_ERROR;

	/* set users TRNG parameters into working State and TrngParams structures */
    //Error = LLF_RND_SetUserRngParameters( &CRYS_RND_WorkingState, &TrngParams );

    //if( Error != CRYS_OK )
    //	return Error;

   /* calling the DX_RND_InstantiateOrReseed function on reset mode */
   //Error = DX_RND_InstantiateOrReseed( &CRYS_RND_WorkingState,
//	                                    &TrngParams,
  //                                      0/*isInstantiate*/,
    //                                    entrEstimBuff_ptr );

   //return Error;

//#endif /*CRYS_NO_AES_SUPPORT*/

//}/* END OF CRYS_RND_Reseeding */

#endif /*_INTERNAL_CRYS_ONE_SEED*/


/**********************************************************************************************************/
/**
 * @brief The CRYS_RND_Init initializes the module
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CEXPORT_C CRYSError_t CRYS_RND_Init( void )
{
   /* FUNCTION DECLERATIONS */

   CRYSError_t Error = CRYS_OK;

   /* FUNCTION LOGIC */

   return Error;

}/* END OF CRYS_RND_Init */


#ifndef _INTERNAL_CRYS_ONE_SEED
/**
* @brief The LLF_RND_SetAdditionalInput - This function load the AdditionaInput
*											from user to the global State buffer, sets
*											its Size and Valid flag to 1.
*
* @param[in/out] State - The pointer to the internal State buffer of DRNG.
* @param[in] AdditonalInput_ptr - The pointer to Additional input buffer.
* @param[in] AdditonalInputSize - The size of Additional input in words - must be up to 12 words.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                        value MODULE_* as defined in ...
*/
//CEXPORT_C CRYSError_t CRYS_RND_AddAdditionalInput(
//										DxUint8_t *AdditonalInput_ptr,
//										DxUint16_t AdditonalInputSizeBytes )
//{

	/* The return error identifiers */
//	CRYSError_t Error;


	/* ............... local initializations ............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
//	Error = CRYS_OK;

	/* FUNCTION LOGIC */

//	if ((AdditonalInput_ptr == DX_NULL) &&
//		(AdditonalInputSizeBytes != 0))
//		return CRYS_RND_ADDITIONAL_INPUT_BUFFER_NULL;

	/* check Additional Input size - must be less than 12 words*/
//	if( AdditonalInputSizeBytes > sizeof(CRYS_RND_WorkingState.AdditionalInput) )
//		return CRYS_RND_ADDITIONAL_INPUT_SIZE_ERROR;

//	DX_VOS_MemSetZero( CRYS_RND_WorkingState.AdditionalInput, sizeof(CRYS_RND_WorkingState.AdditionalInput));

//	if (AdditonalInput_ptr != DX_NULL)
		/* Copy the data from user to the global buffer: AdditionalInput */
//		DX_VOS_FastMemCpy( CRYS_RND_WorkingState.AdditionalInput,
//		AdditonalInput_ptr,
//		AdditonalInputSizeBytes );

	/* Set the AdditionalInput flag to indicate that data written to the buffer
	and the size of the data */
//	CRYS_RND_WorkingState.AddInputSizeWords = AdditonalInputSizeBytes / sizeof(DxUint32_t);

	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

//	return Error;
//}

/**********************************************************************************************************/
/**
 * @brief The CRYS_RND_EnterKatMode function sets KAT mode bit into StateFlag
 *        of global CRYS_RND_WorkingState structure.
 *
 *   The user must call this function before calling functions performing KAT tests.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
//CEXPORT_C void CRYS_RND_EnterKatMode( void )
//{
   /* FUNCTION LOGIC */

  // CRYS_RND_WorkingState.StateFlag |= CRYS_RND_KAT_mode;

   //return;

//}/* END OF CRYS_RND_EnterKatMode  */

/**********************************************************************************************************/
/**
 * @brief The CRYS_RND_DisableKatMode function sets KAT mode bit into StateFlag
 *        of global CRYS_RND_WorkingState structure.
 *
 *   The user must call this function before calling functions performing KAT tests.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
//CEXPORT_C void CRYS_RND_DisableKatMode( void )
//{
   /* FUNCTION LOGIC */

  // CRYS_RND_WorkingState.StateFlag &= ~CRYS_RND_KAT_mode;

   //return;

//}/* END OF CRYS_RND_DisableKatMode  */



/**********************************************************************************************************/
/*
 * @brief The CRYS_RND_GenerateVectorInRange function generates a random vector Rand in range:
 *            1 < RandVect < MaxVect,   using the FIPS-PUB 186-2 standard appendix 3 :
 *        The function performs the following:
 *        1.  Check input parameters.
 *        2.  If maxVect != 0 (maxVect is provided), then calculate required size of random
 *            equaled to actual bit size of MaxVector, else set it = rndSizeInBits.
 *        3.  Calls the CRYS_RND_GenerateVector() function for generating random vector
 *            RndVect of required size.
 *        4.  If maxVect is provided, then:
 *             4.1. Sets all high bits of RndVect, greatest than MSBit of MaxVector, to 0.
 *             4.2. If size of random vector > 16 bytes, then:
 *                     4.2.1. Compares high 16 bytes of randVect to maxVect and low limit
 *                     4.2.2. If condition 1 < randVect16 < maxVect16 is not satisfied,
 *                            then generate new high 16 bytes rndVect16 and go to step 4.2.1.
 *             4.3. Compares full value of RndVect with MaxVector and with 1 . If condition
 *                 1 < RandVect < MaxVector is not satisfied, then go to step 3, else go to 6.
 *        5. Else if maxVect is not provided, then set MSBit of rndVect to 1.
 *        6. Output the result and Exit.
 *
 *         Note: Random and Max vectors are given as sequence of bytes, where LSB is most left byte
 *               and MSB = most right one.
 *
 * @rndSize[in]         - The maximal size (in bits) of random vector if maxVect is given or exact size
 *                        of required random vector.
 * @maxVect_ptr[in]     - The pointer to vector defines high limit of random vector.
 * @rndVect_ptr[in,out] - The output random vector.
 * @return CRYSError_t  - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CEXPORT_C CRYSError_t CRYS_RND_GenerateVectorInRange(DxUint32_t  rndSizeInBits,
											  DxUint8_t *maxVect_ptr, DxUint8_t *rndVect_ptr )
{
   /* FUNCTION DECLARATIONS */

	CRYSError_t Error;
	DxInt32_t   i, k, extraBytes;
	DxInt8_t    shift;
	DxUint8_t   mask, flag0, flag1;
    DxInt32_t   rndSizeInBytes, checkingSizeBytes;
	DxInt32_t   maxVectSizeBits, maxVectSizeBytes;
	CRYS_COMMON_CmpCounter_t CompRes;
    DxUint8_t   smin;
	/* total size of generated random data */
	DxInt32_t   totalRandSize, roundedSize;
        CRYS_RND_EntropyEstimatData_t entrEstimBuff_ptr;


   /* FUNCTION LOGIC */

   /* Initializations */
	Error = CRYS_OK;
	maxVectSizeBytes = 0;

   /* the lower limit of the random vector value */
   smin = 1;
   PRINTF("Entered CRYS_RND_GenerateVectorInRange \n");

   /*----------------------------------------------------------------*/
   /* calculation of actual size of random (excluding leading zeros) */
   /*----------------------------------------------------------------*/

	/* user passed size of random in bytes */
   rndSizeInBytes = rndSizeInBits / 8;
   if( rndSizeInBits % 8 )
		rndSizeInBytes++;

   if( maxVect_ptr != DX_NULL )
   {
          PRINTF("Entered maxVect_ptr!=NULL\n");
	   /* calculate actual size of MaxVector in bits*/
	   maxVectSizeBits = CRYS_COMMON_GetBytesCounterEffectiveSizeInBits( maxVect_ptr, (DxUint16_t)rndSizeInBytes );

	   maxVectSizeBytes = maxVectSizeBits / 8;
	   if( maxVectSizeBits % 8 )
		   maxVectSizeBytes++;

	   /* calculate count of extra 0-bytes in maxVector */
	   extraBytes = rndSizeInBytes - maxVectSizeBytes;

	   /* intermediate checking size */
	   if( maxVectSizeBytes > CRYS_AES_BLOCK_SIZE_IN_BYTES )
		   checkingSizeBytes = CRYS_AES_BLOCK_SIZE_IN_BYTES;
	   else
		   /* intermediate checking size */
		   checkingSizeBytes = maxVectSizeBytes;
   }
   /* case that maxVect not provided and rndVect must have exact size = rndSizeInBits */
   else
   {
	   extraBytes = 0;
	   /* intermediate checking size */
	   checkingSizeBytes = rndSizeInBytes;
   }

  /* calculate count of extra bits for shifting */
   shift = (DxInt8_t)(8 - (rndSizeInBits & 7))&7;
   mask = 0xFF >> shift;

   /* size of rand vector rounded up in AES blocks */
   roundedSize = CRYS_AES_BLOCK_SIZE_IN_BYTES * ((rndSizeInBytes + CRYS_AES_BLOCK_SIZE_IN_BYTES - 1) / CRYS_AES_BLOCK_SIZE_IN_BYTES);

   /* initialization of total size of random data, generated in each call of this function.
      Total size should be less, than 2^28 (according to CRYS requirements) */
   totalRandSize = 0;

   /* main loop for generating random number    */
   /*-------------------------------------------*/
   k = 0;

   while( k < 0xFFFF )
   {
	   /* init checking flags */
	   flag0 = 0;
	   flag1 = 0;
            /* Instantiation process*/
	//  Error = CRYS_RND_Instantiation((CRYS_RND_EntropyEstimatData_t *)&entrEstimBuff_ptr);
           Error = CRYS_RND_Instantiation(&entrEstimBuff_ptr);
	   if( Error != CRYS_OK ) {
		PRINTF("Failed CRYS_RND_Instantiation\n");
		   goto End;
		}

	   /* generate full size random vector */
	   Error = DX_RND_Generate( &CRYS_RND_WorkingState, rndVect_ptr, (DxUint16_t)rndSizeInBytes );

	   if( Error != CRYS_OK )   {
                 PRINTF("Failed DX_RND_Generate \n");
		   goto End;}

       /* count of generated random blocks of size 16 bytes each one */
	   totalRandSize += roundedSize;

       /* set non significant bytes to 0 */
	   for ( i = 0; i < extraBytes; i++ )
	   {
		   rndVect_ptr[rndSizeInBytes-1 - i] = 0;
	   }

	   /* mask the non significant high bits */
       rndVect_ptr[rndSizeInBytes - extraBytes - 1] &= mask;

	   /* in case, that maxVect is provided, check 1 < rndVect < maxVect,
	      else generate new random */
	   if( maxVect_ptr != DX_NULL )
	   {
		  /* step1 check high part of random */
		   if( checkingSizeBytes == CRYS_AES_BLOCK_SIZE_IN_BYTES )
		   {
			   while( 1 )
			   {
				   CompRes = CRYS_COMMON_CmpLsbUnsignedCounters(
					                  rndVect_ptr + rndSizeInBytes - CRYS_AES_BLOCK_SIZE_IN_BYTES,
					                  CRYS_AES_BLOCK_SIZE_IN_BYTES,
									  maxVect_ptr  + rndSizeInBytes - CRYS_AES_BLOCK_SIZE_IN_BYTES,
									  CRYS_AES_BLOCK_SIZE_IN_BYTES );

				   if( CompRes == CRYS_COMMON_CmpCounter2GraterThenCounter1 )
				   {
					   flag0 = 1;
					   goto CheckMinimal;  /* random is found */
				   }

				   else if ( CompRes == CRYS_COMMON_CmpCounter1AndCounter2AreIdentical )
					   break; /* go to check full size */
                                   /* Instantiation process*/
	                          Error = CRYS_RND_Instantiation(&entrEstimBuff_ptr);
	                          if( Error != CRYS_OK ) {
		                  PRINTF("Failed CRYS_RND_Instantiation\n");
		                  goto End;
		                  }

				   /* generate new 16 random high bytes */
				   Error = DX_RND_Generate( &CRYS_RND_WorkingState,
					                         rndVect_ptr + rndSizeInBytes - CRYS_AES_BLOCK_SIZE_IN_BYTES,
											 CRYS_AES_BLOCK_SIZE_IN_BYTES );

				   if( Error != CRYS_OK ) {
                                         PRINTF("Failed in DX_RND_Generate\n");
					   goto End;}


				   /* set non significant bytes of random to 0 */
				   for ( i = 0; i < extraBytes; i++ )
				   {
					   rndVect_ptr[rndSizeInBytes - 1 - i] = 0;
				   }

				   /* mask the non significant high bits */
				   rndVect_ptr[rndSizeInBytes - extraBytes - 1] &= mask;
			   }
		   }

		   /* check full size relating to max vector */
		   CompRes = CRYS_COMMON_CmpLsbUnsignedCounters( rndVect_ptr, (DxUint16_t)rndSizeInBytes,
			                                             maxVect_ptr, (DxUint16_t)maxVectSizeBytes );

		   if ( CompRes == CRYS_COMMON_CmpCounter2GraterThenCounter1 )
			   flag0 = 1;

CheckMinimal:
                  PRINTF("Entered CheckMinimal\n");
		   /* check minimal value */
		   CompRes = CRYS_COMMON_CmpLsbUnsignedCounters( rndVect_ptr, (DxUint16_t)rndSizeInBytes,
			                                             &smin, 1 );

		   if( CompRes == CRYS_COMMON_CmpCounter1GraterThenCounter2 )
			   flag1 = 1;

		   if( flag0 && flag1 )
			   goto End;
	   }

	   else  /* in case, that maxVector is not provided, set MSBit of rndVect to 1 */
	   {
		   rndVect_ptr[rndSizeInBytes - extraBytes - 1] |= (0x80 >> shift);
		   goto End;
	   }

	   /* increment loop counter */
	   k++;
   }

	/* if all tries are Fail, then return the Error */
    Error = CRYS_RND_CAN_NOT_GENERATE_RAND_IN_RANGE;

 End:

    return Error;

} /* End of CRYS_RND_GenerateVectorInRange function */

#endif /*_INTERNAL_CRYS_ONE_SEED*/


/*************************************************************************************************/
/*******                     Functions for backward compatibility                  ***************/
/*************************************************************************************************/

#ifdef CRYS_RND_AES_OLD_128BIT_ONLY

/**********************************************************************************************************/
/**
 * @brief The CRYS_RND_ResetSeed function performs Instantiate operation.
 *
 *        This function needed for backward compatibility with some projects.
 *        It is recommended to call directly the CRYS_RND_Instantiation function.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
//CEXPORT_C CRYSError_t CRYS_RND_ResetSeed( void )
//{
   /* FUNCTION DECLERATIONS */

   /* error identifier definition */
  // CRYSError_t Error;

  //	Error = CRYS_RND_Instantiation();

   //return Error;

//}/* END OF CRYS_RND_ResetSeed */


/**********************************************************************************************************/
/**
 * @brief The CRYS_RND_StartResetSeed is a dummy function needed
 *        for backward compatibility with some projects
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
//CEXPORT_C CRYSError_t CRYS_RND_StartResetSeed( void )
//{
   /* FUNCTION DECLARATIONS */

  // return CRYS_OK;

//}/* END OF CRYS_RND_StartResetSeed */

#endif
