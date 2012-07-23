/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

/************* Include Files ****************/
#include "DX_VOS_Sem.h"
#ifdef CRYS_SEP_SIDE_WORK_MODE
/* SEP side needs a basic synchronixzation between the interrupts */
//#include "sep_sync.h"
#endif /*CRYS_SEP_SIDE_WORK_MODE*/
#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON_math.h"
#include "CRYS.h"
#include "CRYS_RND_local.h"
#include "CRYS_RND_error.h"
#include "LLF_RND.h"
#include "LLF_RND_error.h"
#include "LLF_AES_Restrict.h"
#include "LLF_AES_HwDefs.h"
#include "LLF_RND_HwDefs.h"


  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  09 March 2010
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AES.c#1:csrc:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2010 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/********************************* Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */


/*********************************** Enums ******************************/
/*********************************Typedefs ******************************/

/**************** Global Data to be read by RNG function ****************/

#ifdef RND_TEST_TRNG_WITH_ESTIMATOR
#ifndef CRYS_SEP_SIDE_WORK_MODE
    extern DxUint32_t GlobalRndSequence[];
#endif
#endif

/* importing the base address of the CRYPTO Cell. This value must be initialized in the
   CRYS_Init function */
extern DxUint32_t PLAT_CryptoCellBaseAddr;

/* importing the semaphore used for the access to the hardware machine */
#ifndef CRYS_NO_GLOBAL_DATA
extern DxVosSem SemRndId;       /* semaphore for TRNG */
extern DxVosSem SemHwAccessId;  /* semaphore for AES */
#endif

/* definitions used in the Entropy Estimator functions */
#define S(a,n) ((DxUint32_t)((a) * (1<<(n)))) /* a scaled by n: a \times 2^n */
#define U(a,n) ((DxUint32_t)(a) >> (n)) /* unscale unsigned: a / 2^n */
#define SQR(x) ((DxInt64_t)(x)*(x))

/* macros for updating histogram for any separate bit;
   where x represents cw  or e1 */
#define   LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, x ) \
	h_ptr[x & 0xff]++;  \
	ec_ptr[x & 0x7f] = ((ec_ptr[x & 0x7f] & 1) == ((x & 0xff) >> 7)) ? ec_ptr[x & 0x7f] + 2 : ec_ptr[x & 0x7f] ^ 1; \
	x >>= 1;

/* Entropy estimation histogram width (prefix size + 1) */
#define LLF_RND_nb   CRYS_RND_nb
#define LLF_RND_NB   CRYS_RND_NB
#define halfNB   (LLF_RND_NB / 2)


/* test variables */
#ifdef RND_TEST_TRNG_WITH_ESTIMATOR
	DxUint32_t SamplesTotal, SamplesLoss;
#endif



/************************************************************************************/
/***********************           Private functions            *********************/
/************************************************************************************/
 /****************************************************************************/
/***********         Functions used for Entropy estimation      *************/
/****************************************************************************/
/*****************************************************************************/


/* approximation of entropy  */
/**
 * @brief The function approximates the entropy for separate prefix
 *        ae = n * log2(n/m).
 *
 *    Implementation according A.Klimov algorithm uses approximation by
 *    polynomial: ae = (n-m)*(A1 + A2*x + A3*x^2), where x = (n-m)/n <= 0.5 .
 *    The coefficients are defined above in this file.
 *
 * @param[in] n - The summ of  0-bits and 1-bits in the test.
 * @param[in] m - The maximal from the two above named counts.
 *
 * @return - result value of entropy ae.
 */
static DxUint32_t ae(DxUint32_t n, DxUint32_t m)
{
	/* logarithm calculation constants */
	#define A1 1.4471280
	#define A2 0.6073851
	#define A3 0.9790318

    DxUint32_t d = n-m,
	    x = S(d,16) / n,                 /* x; 16 */
	    a = S(A3,14) * x,                /* x*A3; 30 */
	    b = U(S(A2,30) + a, 16) * x,     /* x*(A2 + x*A3); 30 */
	    c = (S(A1,30) + b),              /* (A1 + x*(A2 + x*A3)); 30 */
	    r = d * U(c,14);                 /* result: 16 bits scaled */

  return r;

}


/*****************************************************************************/
/**
 * @brief The function calculates a histogram of 0-s and 1-s distribution
 *        depending on forgouing bits combination - prefix.
 *
 *     Implementation according A.Klimov algorithm modified by A.Ziv
 *
 * @param[in]  h_ptr - The pointer to the histogramm h buffer.
 * @param[in]  ec_ptr - The pointer to the histogramm equality counter (ec) buffer.
 * @param[in]  r_ptr - The pointer to Entropy source.
 * @param[in]  nr    - The size of Entropy source in words.
 * @param[in/out] pref_ptr - The pointer to last saved prefix.
 * @param[in]  snp_ptr   - The pointer to the flag defining whether the new prefix should be set.
 *
 * @return CRYSError_t - no return value
 */
void LLF_RND_HistogramUpdate(
			                DxUint32_t   *h_ptr,   /* in/out */
			                DxUint32_t   *ec_ptr,  /* in/out */
	                        DxUint32_t   *r_ptr,   /* in - input sequence */
	                        DxInt32_t     nr,      /* in - input sequence size in words */
	                        DxUint32_t   *pref_ptr,/* in - pointer to last saved prefix */
	                        DxInt32_t    *snp_ptr )/* in/out - pointer to "Set New Prefix" flag */
{
	DxUint32_t  i, j = 0;
	DxUint32_t  cW; /*current word of secuence*/


	/* FUNCTION  LOGIC  */

	/*------------------------------------------------------*/
	/* update for first word of sequence: begin new prefix  */
	/*------------------------------------------------------*/
	if (*snp_ptr)
	{
        cW = r_ptr[0];

		/* 25 sequences are purely from new bits */
		for (i = 0; i < 5; i++)
		{
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		}

		*snp_ptr = 0;
		*pref_ptr = cW;
		j = 1;
	}

	/*-----------------------------------------------------------------------*/
	/* update for remaining words of sequence: continue with previous prefix */
	/*-----------------------------------------------------------------------*/
	for( ; j < nr; j++ )
	{
		DxUint32_t e1;

        /*current word of random secuence*/
        cW = r_ptr[j];
        /* concatenation of previous saved prefix and new bits */
        e1 = (cW << 7) | *pref_ptr;

		/* first 7 sequences are combined from previous prefix and new bits  */
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );
	    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, e1 );

		/* next 25 sequences are purely from new bits */
		for (i = 0; i < 5; i++)
		{
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		    LLF_RND_UpdateHistOneBit( h_ptr, ec_ptr, cW );
		}

		*pref_ptr = cW;
	}

} /* End of LLF_RND_HistogramUpdate() */


/*****************************************************************************/
/**
 * @brief The function calculates estimation of entropy, generated by TRNG and
 *        used for control the TRNG work.
 *
 *   Implementation based on algorithm developed by A.Klimov.
 *
 * @param[in] entrEstimData - The pointer to the structure containing estimation buffers.
 * @param[out] e_ptr - The pointer to count of accumulated Entropy (bits multiplied by 2^16).
 *
 * @return CRYSError_t - no return value
 */
CRYSError_t LLF_RND_EntropyEstimate(
                        CRYS_RND_EntropyEstimatData_t *entrEstimData, /*in/out*/
                        DxUint64_t  *e_ptr )   /* out - result Entropy size */
{
  CRYSError_t Error = CRYS_OK;
  DxUint64_t t = 0;              /* total entropy */
  DxUint32_t i, ac = 0;          /* number of active prefixes */

  /* pointers to histogram buffer */
  DxUint32_t *h  = &entrEstimData->LLF.h[0];    /* histogram */
  DxUint32_t *ec = &entrEstimData->LLF.ec[0];   /* equality counter */


  /*-------------  calculate entropy -----------------*/

  for(i = 0; i < halfNB; ++i)
  {
    DxUint32_t n = h[i] + h[i+halfNB], m = max(h[i], h[i+halfNB]);

    /* check that n < 2^16, else return overflow error */
    if( n > (1<<16))
	return LLF_RND_TRNG_ENTR_ESTIM_SIZE_EXCEED_ERROR;

    if(n != m) /* if active prefix */
    {
      DxUint64_t n2, pp, od, od2, od2n, var;


	  /* increment count of active prefixes */
      ++ac;

      pp = SQR(m) + SQR(n-m);       /* related to theoretical "autocorrelation" probability */
      n2 = SQR(n);

      /* value, related to observed deviation of autocorrelation */
      if((ec[i]/2)*n > pp)
	od = (ec[i]/2)*n - pp;
      else
		od = pp - (ec[i]/2)*n;

      /* theoretical variance of B(n, pp): alwais > 0 */
      var = pp*(n2 - pp);

      /* if  n*od^2 < var then accumulate entropy, else return Error;
         note: that this condition may be True only if od < 2^32 */
      if( od < ((DxUint64_t)1 << 32) )
      {
		od2 = SQR(od);

		/* scale variables */
		if( od2 > ((DxUint64_t)1 << 48) )
		{
			od2 /= 1UL << 16;
			var /= 1UL << 16;
		}

		od2n = od2 * n;

		if( od2n < var )
			t += ae(n, m);
      }
    }
  }

  /* output entropy size value with 16 bits fixed point */
  *e_ptr = ac > 3 ? t : 0;

  return Error;

} /* End of LLF_RND_EntropyEstimate */


/*******************************************************************************************/
/**
 * @brief The LLF_RND_GetEntropySourceSize returns size of random source needed for collection
 *        required entropy .
 *
 *        The function returns size of source needed for required entropy.
 *
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @entropySizeWords[in/out] - The pointer to size of random source. The user sets
 *                    size of entropy that is required and the function returns
 *                    the actual size of source needed for this count of entropy.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_RND_GetEntropySourceSize(
                                 CRYS_RND_Params_t    *trngParams, /*in*/
                                 DxUint16_t   *entropySizeWords_ptr )   /*in/out*/
{
   /* LOCAL DECLARATIONS */


   /* FUNCTION LOGIC */

   if( trngParams->TrngMode  == CRYS_RND_Fast)
		*entropySizeWords_ptr = *entropySizeWords_ptr * LLF_RND_SOURCE_AND_ENTROPY_SIZES_RELATION;

    return CRYS_OK;

 }/* END of LLF_RND_GetEntropySourceSize */


/****************************************************************************************/
/**
 * @brief The function  turns the HW RNG clocks ON.
 *
 *   Note: All needed TRNG parameters are saved in the internal State structure.
 *
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @param[in] virtualHwBaseAddr - The HW registers virtual base address.
 *
 * @return CRYSError_t - no return value
 */
 void LLF_RND_TurnOnTrng( CRYS_RND_Params_t    *trngParams, /*in/out*/
                          DxUint32_t            virtualHwBaseAddr )
 {
    /* LOCAL DECLARATIONS */
    DxUint32_t  val;


	/* FUNCTION LOGIC */

  //		DX_RND_TEST_MODE

	/*----------------------------------------------------------------- */
	/* Check the current status of the RNG clocks and if it is          */
	/* enabled, then exit, else turn the RNG on                         */
	/*----------------------------------------------------------------- */
	CRYS_PLAT_SYS_ReadRegister( virtualHwBaseAddr + LLF_RNG_CLK_ENABLE_REG_ADDR/*0x0850UL*/, val );

	if( val == LLF_RND_HW_RND_CLK_ENABLE_VAL )
	{
		DxUint32_t RingOscillatorLength, SampleCount, TrngDebugControl;

		/* check TRNG settings */
		CRYS_PLAT_SYS_ReadRegister( virtualHwBaseAddr + LLF_RND_HW_TRNG_CONFIG_REG_ADDR /*0x10C*/, RingOscillatorLength );
		CRYS_PLAT_SYS_ReadRegister( virtualHwBaseAddr + LLF_SAMPLE_CNT1_REG_ADDR /*0x10C*/, SampleCount );
		CRYS_PLAT_SYS_ReadRegister( virtualHwBaseAddr + LLF_RND_HW_TRNG_DEBUG_CONTROL_REG_ADDR /*0x10C*/, TrngDebugControl );

		if( (RingOscillatorLength & LLF_RND_HW_RING_OSCILLATOR_LENGTH_MASK) == (trngParams->RingOscillatorLength & LLF_RND_HW_RING_OSCILLATOR_LENGTH_MASK) &&
		     SampleCount == trngParams->SampleCount &&
		     TrngDebugControl == LLF_RND_HW_TRNG_DEBUG_CONTROL_Value_ON_FAST_MODE )
		{
			return;
		}
		else
		{
		    /* disable the RND source  */
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RND_SOURCE_ENABLE_REG_ADDR, /* 0x012CUL */
			                        LLF_RND_HW_RND_SRC_DISABLE_VAL );
		    /* close the Hardware clock */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RNG_CLK_ENABLE_REG_ADDR ,
		                                LLF_RND_HW_RND_CLK_DISABLE_VAL );
		}
	}

	/* enable the RNG clock  */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RNG_CLK_ENABLE_REG_ADDR, /*0x0850UL*/
	                          LLF_RND_HW_RND_CLK_ENABLE_VAL );

    /* for Fast TRNG: VNC_BYPASS + TRNG_CRNGT_BYPASS + AUTO_CORRELATE_BYPASS */
    if( trngParams->TrngMode == CRYS_RND_Fast )
    {
	/* set TRNG config register to predefined length of ring oscillator */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RND_HW_TRNG_CONFIG_REG_ADDR /*0x10C*/,
	                                 trngParams->RingOscillatorLength & LLF_RND_HW_RING_OSCILLATOR_LENGTH_MASK );

		/* set RNG sampling control register to predefined count of clocks for sampling random bits */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_SAMPLE_CNT1_REG_ADDR/*0x130*/,
	                                 trngParams->SampleCount );

		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RND_HW_TRNG_DEBUG_CONTROL_REG_ADDR/*0x138*/,
								 LLF_RND_HW_TRNG_DEBUG_CONTROL_Value_ON_FAST_MODE/*0x0000000E*/);
    }
    /* for Slow mode - set the user passed HW parameters */
    else
    {
	/* set TRNG config register to predefined length of ring oscillator */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RND_HW_TRNG_CONFIG_REG_ADDR /*0x10C*/,
	                                 trngParams->RingOscillatorLength & LLF_RND_HW_RING_OSCILLATOR_LENGTH_MASK );

	/* set RNG sampling control register to predefined count of clocks for sampling random bits */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_SAMPLE_CNT1_REG_ADDR/*0x130*/,
	                                 trngParams->SampleCount );
    }

	/* enable the RNG source  */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RND_SOURCE_ENABLE_REG_ADDR/*0x012CUL*/,
	                             LLF_RND_HW_RND_SRC_ENABLE_VAL );

    return ;

} /* End of LLF_RND_TurnOnTrng */


/**********************************************************************************************************/
/**
 * @brief The LLF_RND_TurnOffTrng stops the hardware random bits collection.
 *
 * @param[in] virtualHwBaseAddr - The HW registers virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void LLF_RND_TurnOffTrng( DxUint32_t  virtualHwBaseAddr /*in*/ )
{

	/* LOCAL DECLARATIONS */


	/* FUNCTION LOGIC */

   /* ..............   set the hardware registers   ......................... */
   /* ----------------------------------------------------------------------- */

   /* disable the RND source  */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RND_SOURCE_ENABLE_REG_ADDR, /* 0x012CUL */
	                        LLF_RND_HW_RND_SRC_DISABLE_VAL );

   /* close the Hardware clock */
   CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_RNG_CLK_ENABLE_REG_ADDR ,
                                LLF_RND_HW_RND_CLK_DISABLE_VAL );

	return;

}/* END OF LLF_RND_TurnOffTrng*/



/****************************************************************************************/
/**
 * @brief The function reads 192 or 128 bits block of entropy from RNG according
 *        to EHR register size.
 *
 *      Assumed: RNG engine is initialized and turned On previously.
 *
 * @param[in] State - The pointer to the internal State buffer of RNG.
 * @param[out] out_ptr - The pointer to output buffer.
 * @param[in] countBlocks - The count of EHR blocks to generate.
 * @param[in] virtualHwBaseAddr - The HW registers virtual base address.
 * @param[in] trngMode - The flag defining TRNG mod: 0 - "Fast", 1 - "Slow".
 * @param[in] snp - The flag, defining that this is a first EHR read and a
 *                  new prefix in entropy estimator must be set.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 CRYSError_t LLF_RND_TrngReadBlocks(
						CRYS_RND_State_t *State,            /*in*/
						DxUint32_t       *out_ptr,          /*out*/
						DxUint32_t         countBlocks,      /*in*/
						CRYS_RND_mode_t   trngMode,         /*in*/
                        DxInt32_t         snp,              /*in*/
						DxUint32_t        virtualHwBaseAddr /*in*/ )

 {

	/* LOCAL DECLARATIONS */

    CRYSError_t Error = CRYS_OK;
    DxUint32_t  reg_val = 0x0, i;

    /* FUNCTION LOGIC */
#ifdef CRYS_SEP_SIDE_WORK_MODE
	/*DSM simulation workaround */
	CRYS_PLAT_SYS_ReadRegister( HW_SEP_OVERRIDE_REG_ADDR,reg_val);
#endif
	if(!(reg_val & DX_SEP_HW_RESET_SEED_OVERRIDE_FLAG))
	{

		/* in Fast mode check random samplings loss */
		if( trngMode == CRYS_RND_Fast )
		{
		   #ifdef RND_TEST_TRNG_WITH_ESTIMATOR
			SamplesTotal++;
		   #endif

			/* for not first Block (snp==0) check, that EHR buffer was full previously,
			   means - sampling loss */
			CRYS_PLAT_SYS_ReadRegister( (virtualHwBaseAddr) + LLF_RND_HW_TRND_BUSY_ADDR , reg_val );

			if( reg_val == 1 && snp == 0 )
			{
		   #ifdef RND_TEST_TRNG_WITH_ESTIMATOR
				SamplesLoss++;
		   #endif
		        /* set LossSamples bit */
                State->StateFlag = State->StateFlag | CRYS_RND_InstantReseedLossSamples;
			}
		}
	}

	/* read random dsts from EHR register */
	for( i = 0; i < countBlocks; i++ )
	{
		/* wait for RND ready */
		LLF_RND_HW_WAIT_VALID_EHR_BIT(virtualHwBaseAddr);

		/* load the random data from HW to the output buffer */
		CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + HW_EHR_DATA_ADDR_0_REG_ADDR,
		       out_ptr + i*LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS, LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS );

	}

	return Error;

 }


/****************************************************************************************/
/**
 * @brief The function gets the block of entropy source from RNG.
 *
 *        -  In Slow mode the function reads from HW one 198 or 128 bits block of entropy
 *           and outputs them "AS IS".
 *        -  In Fast mode the function reads needed size of the entropy source from HW,
 *           compresses it in one 128 bits block using AES-MAC so, that it contains at least 64
 *           bits of entropy and sets this block into output.
 *
 * @param[in] State - The pointer to the internal State buffer of RNG.
 * @param[in/out] entrEstimData - The pointer to structure, containing buffers and
 *                                data of entropy estimator.
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @param[out] out_ptr - The pointer to output buffer.
 * @param[in/out] AESRestrContext_ptr - The pointer to restricted AES context.
 * @param[in] virtualHwBaseAddr - The HW registers virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 CRYSError_t LLF_RND_TrngRead(
						CRYS_RND_State_t  *State,         /*in*/
                        CRYS_RND_EntropyEstimatData_t *entrEstimData, /*in/out*/
					    CRYS_RND_Params_t *trngParams,    /*in/out*/
						DxUint32_t        *out_ptr,       /*out*/
		                AESResrtictContext_t *AESRestrContext_ptr,    /*in/out*/
		                DxUint32_t         virtualHwBaseAddr )        /*in*/
 {

	/* LOCAL DECLARATIONS */

    CRYSError_t Error = CRYS_OK;

    DxUint32_t i;
    DxUint32_t countEhrBlocks;

    /* current and summary entropy sizes */
    DxUint64_t  es = 0, e = 0;

    /* last saved prefix in random bits sequence */
    DxUint32_t pref = 0;

    DxUint32_t wordsCountInLoop, loopCount, aesBlocksToCompress;


	/*DSM simulation workaround */
	DxUint32_t reg_val = 0x0;

	/* FUNCTION LOGIC */

    /* Initialization */


   /* ----------------------------------------------------------------- */
   /* In slow mode read and output EHR size bits block.                 */
   /* Note: assumed, that CTRNG test was performed yet inside the HW    */
   /* ----------------------------------------------------------------- */
   if( trngParams->TrngMode == CRYS_RND_Slow )
   {
		Error = LLF_RND_TrngReadBlocks( State, out_ptr, 1/*count of EHR blocks*/,
		                                CRYS_RND_Slow,  1/*snp*/, virtualHwBaseAddr );
		return Error;
   }

   /* ---------------------------------------------------------- */
   /* In Fast mode reads the needed count of 128-bits blocks     */
   /* and compress them to one 128 bit block, so that it         */
   /* should contain at least 64 bit of Entropy. If Sample Loss  */
   /* or generation time exceedeng occurred an appropriate value */
   /* in the State->InstantiationIsDone variable should be set   */
   /* ---------------------------------------------------------- */

   /* clear entropy estimation buffer  */
   DX_VOS_MemSetZero( entrEstimData, sizeof(CRYS_RND_EntropyEstimatData_t) );

  /* calculate count of EHR blocks which may be written into P-temp buffer  */
   countEhrBlocks = (sizeof(entrEstimData->LLF.p)/4) / LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS;

   /* correction of countEhrBlocks according to given CollectionCount value and EHR register width.
      In case EHR size = 6 words we need even countEhrBlocks to perform AES compression */
   if( countEhrBlocks > trngParams->CollectionCount )
   {
		countEhrBlocks = trngParams->CollectionCount;

	    /* for EHR register width = 6 words we need even countEhrBlocks and may round up it value */
	    if( LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS == 6 && (countEhrBlocks & 1) )
		countEhrBlocks++;
   }
   else
   {
	    /* for EHR register width = 6 words we need round down countEhrBlocks to even value */
	    if( LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS == 6 && (countEhrBlocks & 1) )
		countEhrBlocks--;
   }

   /* variables, precalculated for increasing performance: count of words of entropy,
      readed in one cycle of loop, count of cycles in the loop and count of AES blocks in the entropy source */
   wordsCountInLoop = LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS*countEhrBlocks;
   loopCount = (trngParams->CollectionCount + countEhrBlocks - 1)/countEhrBlocks;
   aesBlocksToCompress = LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS*countEhrBlocks/CRYS_AES_BLOCK_SIZE_IN_WORDS;

   /* ------------------------------------------------------ */
   /*  read random bits and compress loop:                   */
   /*  in each loop processed countEhrBlocks of EHR blocks   */
   /* ------------------------------------------------------ */
   while ( 1 )
   {
       /* flag "Set New Prefix", defining that the the new prefix must be set */
	   DxInt32_t snp = 1;

	   for( i = 0 ; i < loopCount; i++ )
	   {
	        /* 1. Read bits into temp p-buffer and compress the data by AES-MAC */
	        /*------------------------------------------------------------------*/

            /* if working mode */
	        if( (State->StateFlag & CRYS_RND_KAT_mode) == 0 )
	        {
				Error = LLF_RND_TrngReadBlocks( State, entrEstimData->LLF.p,
				                               countEhrBlocks,
				                               CRYS_RND_Fast,
				                               snp, virtualHwBaseAddr );

			    /* check TRNG Error and reset TRNG with incremented Samling interval */
				if( Error != CRYS_OK )
					return Error;

		      #ifdef  RND_TEST_TRNG_WITH_ESTIMATOR
		        /* copy generated entropy source for testing in ATP */
		        DX_VOS_FastMemCpy( &GlobalRndSequence[wordsCountInLoop*i], entrEstimData->LLF.p, 4*wordsCountInLoop );
	          #endif

	        }

	      #ifdef  RND_TEST_TRNG_WITH_ESTIMATOR
            else /* if KAT test mode */
            {
			    /* copy Known entropy source for processing KAT TRNG test */
			    DX_VOS_FastMemCpy( entrEstimData->LLF.p, &GlobalRndSequence[wordsCountInLoop*i], 4*wordsCountInLoop );
            }
          #endif

          #ifndef CRYS_NO_RND_ENTROPY_ESTIMATOR_SUPPORT
	        /* calculate statistics for entropy estimation */
			LLF_RND_HistogramUpdate(
		                    entrEstimData->LLF.h,
		                    entrEstimData->LLF.ec,
	                        entrEstimData->LLF.p,  /*in - input sequence*/
	                        wordsCountInLoop, /* in - input size in words */
	                        &pref,      /* in - pointer to last saved prefix */
	                        &snp );     /* in - set new prefix if = 1 */
	      #endif

			/* compression of entropy source by AES MAC */
			LLF_AES_RESTR_MacBlockExec( virtualHwBaseAddr, entrEstimData->LLF.p, aesBlocksToCompress );
	#if 0
	        Error = LLF_AES_RESTR_BlockExec(
	                        entrEstimData->LLF.p,     /*in_ptr*/
	                        DX_NULL,                  /*out_ptr*/
	                        aesBlocksToCompress,      /*sizeInBlocks*/
	                        AESRestrContext_ptr );    /*AES restricted context*/

			if( Error != CRYS_OK )
			return Error;
	#endif
		    trngParams->MaxAllowedTrngClocksCount -= trngParams->SampleCount*wordsCountInLoop;

		    if(  trngParams->MaxAllowedTrngClocksCount < 0x0 )
		    {
		        /* zero instaniation bit b'0 and set TimeExceed bit */
                State->StateFlag = (State->StateFlag & ~CRYS_RND_Instantiated) | CRYS_RND_InstantReseedTimeExceed;
			    break;
		    }
	   } /* end for(...) loop */


     #ifndef CRYS_NO_RND_ENTROPY_ESTIMATOR_SUPPORT
	   /* calculate entropy estimation */
	   /*------------------------------*/
	   Error = LLF_RND_EntropyEstimate( entrEstimData, &e /*Entropy Size, accumulated in current cycle*/ );

	   if( Error )
		   return Error;

	   es += e; /*summarized enthropy*/

	 #else
	   break;  /* break if Estimator not supported */
	 #endif

       /* if time is exceeded then break while loop */
       if( State->StateFlag & CRYS_RND_InstantReseedTimeExceed )
	   break;

	#ifdef CRYS_SEP_SIDE_WORK_MODE
		/*DSM simulation workaround */
		CRYS_PLAT_SYS_ReadRegister( HW_SEP_OVERRIDE_REG_ADDR,reg_val);
	#endif

	    /* end the loop in case DX_SEP_HW_RESET_SEED_OVERRIDE_FLAG != 0 */
	if( (reg_val & DX_SEP_HW_RESET_SEED_OVERRIDE_FLAG) )
			break;

	#ifdef  RND_TEST_TRNG_WITH_ESTIMATOR
		/* if defined entropy estimator test mode, then exit to allow
		   output and testing gathered original ring oscillator source */
		break;
	#else
	   /* if enough entropy was accumulated, then breake */
	   if( ((es - (es>>6))>>16) >= LLF_RND_ENTROPY_ACCUMULATED_IN_BLOCK_BITS )
			break;
	   else
	   {
		    /* clear histogram buffers */
		    DX_VOS_MemSetZero( entrEstimData->LLF.h, sizeof(entrEstimData->LLF.h) );
		    DX_VOS_MemSetZero( entrEstimData->LLF.ec, sizeof(entrEstimData->LLF.ec) );
	   }
	#endif

   } /************ End while(...) loop ************/


#ifndef CRYS_NO_RND_ENTROPY_ESTIMATOR_SUPPORT
   /* rescale total entropy size by 16 bits:
      Note: plus 2^15 - for more precize rounding to integer */
   entrEstimData->LLF.EstimEntropySizeBits = (es + (1 << 15)) / (1UL << 16);

   /* assume that final Entropy calculation error < entropy/256 */
   entrEstimData->LLF.EstimEntropySizeErrorInBits = entrEstimData->LLF.EstimEntropySizeBits >> 8;
#endif

   /* output result random source (AES-MAC) */
   LLF_AES_RESTR_ReadIV( out_ptr, AESRestrContext_ptr );

   return Error;

} /* End of LLF_RND_TrngRead */



/****************************************************************************************/
/*****************************       Public Functions      ******************************/
/****************************************************************************************/

/****************************************************************************************/
/**
 * @brief The function performs initial step of RND processing (gets HW RND semaphore
 *        on appropriate LLF platforms).
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_RND_Init( void )
 {

	/* FUNCTION LOGIC */


    /* ......... getting the hardware semaphores ............ */
	/* ------------------------------------------------------ */

#if !defined CRYS_NO_GLOBAL_DATA
	return DX_VOS_SemWait( SemRndId , DX_INFINITE);  /* RND semaphore */

#else
	return CRYS_OK;
#endif

} /* End of LLF_RND_Init function */


/****************************************************************************************/
/**
 * @brief The function finishes random processing (gets HW RND semaphore
 *        on appropriate LLF platforms).
 *
 * @return - no return  value.
 */
 void LLF_RND_Finish( void )
 {

	/* FUNCTION LOGIC */

#ifndef CRYS_NO_GLOBAL_DATA
	 DX_VOS_SemGive(SemRndId);  /* RND semaphore */
#endif

     return;

 } /* End of LLF_RND_Finish function */





/****************************************************************************************/
/**
 * @brief The function gets user given TRNG parameters, gets RND semaphore and
 *        turns the TRNG ON.  Then the function releases the RND semaphore to
 *        allow to other functions using the RND module.
 *
 *   Note: The function needed for starting TRNG entropy collection in "Slow" mode
 *         at Boot time for increasing performance. In "Fast" mode the function
 *         does nothing.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 *
 * @return CRYSError_t - no return value
 */
 CRYSError_t LLF_RND_StartTrngHW( CRYS_RND_State_t   *State )
 {
    /* LOCAL DECLARATIONS */

    CRYSError_t Error = CRYS_OK;
    DxUint32_t  virtualHwBaseAddr;
    CRYS_RND_Params_t  trngParams;


	/* FUNCTION LOGIC */

    /* get user parameters and set them into output structures */
    Error = LLF_RND_SetUserRngParameters( State, &trngParams );

	if( Error != CRYS_OK )
	return Error;

	/* in "Fast" mode exit */
	if( trngParams.TrngMode == CRYS_RND_Fast )
		return Error;


    /* ......... getting the hardware semaphores ............ */
	/* ------------------------------------------------------ */

#if !defined CRYS_NO_GLOBAL_DATA
	Error = DX_VOS_SemWait( SemRndId , DX_INFINITE);  /* RND semaphore */

	if( Error != CRYS_OK )
		return Error;
#endif

	/*  mapping the physical memory: set the virtual HW address into State   */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
                           LLF_RND_HW_CRYPTO_ADDR_SPACE,  /* 16 LS bit space - in */
                           &virtualHwBaseAddr );          /* The virtual address - out */

	if( Error != CRYS_OK )
	goto End;


    /* turn ON the TRNG for starting collection entropy */
    LLF_RND_TurnOnTrng( &trngParams, virtualHwBaseAddr );

End:
   #ifndef CRYS_NO_GLOBAL_DATA
	DX_VOS_SemGive(SemRndId);  /* RND semaphore */
   #endif

	/* end  of function */
	return Error;
 }

/*******************************************************************************/
/**
 * @brief The LLF_RND_GetTrngSource reads random source of needed size from TRNG.
 *
 *        The function is used in Self, Instantiation and Reseeding functions.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @rndSize[in] - The size of random vector that is required in words.
 * @out_ptr[in,out] The output vector.
 * @param[in/out] AESRestrContext_ptr - The pointer to restricted AES context.
 * @param[in/out] entrEstimData_ptr - The pointer to temp buffer for specific operations
 *                               on entropy estimator.
 *                         Note: for projects, which not use entropy estimator (e.g.
 *                               SW projects), the pointer may be set to NULL.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_RND_GetTrngSource( CRYS_RND_State_t     *State,         /*in/out*/
                                   CRYS_RND_Params_t    *trngParams,    /*in/out*/
                                   DxUint16_t            rndSizeWords,  /*in*/
                                   DxUint32_t           *out_ptr,       /*out*/
						           AESResrtictContext_t *AESRestrContext_ptr,   /*in/out*/
						           CRYS_RND_EntropyEstimatData_t *entrEstimData_ptr /*in/out*/ )

{
   /* LOCAL DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

	/* The block size and number of full blocks needed */
	DxUint16_t blockSize;
	DxUint16_t  numOfBlocks = 0;

	/* The number of remaining bytes needed */
	DxUint16_t  numOfRemainingWords;

	/* loop variable */
	DxInt16_t i;

    /* HW base address */
    DxUint32_t  virtualHwBaseAddr;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;



   /* .............. initializing the hardware .............................. */
   /* ----------------------------------------------------------------------- */

   /* set block size to read from TRNG without loss entropy:
      - 192 bit for Slow mode, 128 bit for Fast  */
   if( trngParams->TrngMode == CRYS_RND_Slow )
	   blockSize = LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS;
   else
	   blockSize = CRYS_AES_BLOCK_SIZE_IN_WORDS;


    /* calculate count of full blocks needed and remaining size  */
    /* --------------------------------------------------------- */
	if(rndSizeWords < blockSize)
	{
		 numOfBlocks = 0x0;
		 numOfRemainingWords = rndSizeWords;
	}
	else
	{
	  #ifndef CRYS_SEP_SIDE_WORK_MODE
		 /* The number of full blocks needed */
		 numOfBlocks = rndSizeWords / blockSize;

		 /* The number of remaining bytes needed */
		 numOfRemainingWords = rndSizeWords % blockSize;
	  #else
		while( rndSizeWords >= blockSize )
		{
			numOfBlocks++;
			rndSizeWords = rndSizeWords - blockSize;
		}
		numOfRemainingWords = rndSizeWords;
	  #endif
	}

    /*  memory mapping: get virtual HW base address */
	/* -------------------------------------------- */
	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &virtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	   return Error;

   /* set 128-bit AES key = 0 for MAC operations using temporary
      entrEstimData_ptr buffer */
   DX_VOS_MemSetZero( entrEstimData_ptr, CRYS_AES_BLOCK_SIZE_IN_BYTES );

   /* Initialize the AES machine on CBC-MAC mode, SecretKey is used  */
   /*----------------------------------------------------------------*/
   Error = LLF_AES_RESTR_Init(
                       (DxUint32_t*)entrEstimData_ptr, /*key_ptr */
                       4,             /*keySizeWords*/
                       DX_NULL,       /*IV*/
                       CRYS_AES_MAC_mode,
                       CRYS_AES_Encrypt,
                       AESRestrContext_ptr );  /*AES restricted context*/

	if( Error != CRYS_OK )
	return Error;

   /* turn the RNG  on   */
   /*--------------------*/
   LLF_RND_TurnOnTrng( trngParams, virtualHwBaseAddr );

   /* .............. read the RND source from TRNG .......................... */
   /* ----------------------------------------------------------------------- */

   /* fill the Output buffer with up to full blocks */
   for( i = 0 ; i < numOfBlocks ; i++ )
   {
	    Error = LLF_RND_TrngRead(
							State,               /*in*/
							entrEstimData_ptr,
							trngParams,          /*in/out*/
							out_ptr,             /*out*/
							AESRestrContext_ptr, /*in*/
							virtualHwBaseAddr ); /*in*/

	   if(  Error != CRYS_OK )
		goto End;

       if( State->StateFlag & CRYS_RND_InstantReseedTimeExceed )
       {
		   goto Finish;
       }

		out_ptr += blockSize;
   }

   if ( numOfRemainingWords > 0 )
   {
	   Error = LLF_RND_TrngRead(
							State,              /*in*/
							entrEstimData_ptr,
							trngParams,          /*in/out*/
							out_ptr,             /*out*/
							AESRestrContext_ptr, /*in*/
							virtualHwBaseAddr ); /*in*/

	   if(  Error != CRYS_OK )
		goto End;
   }

Finish:
   Error = LLF_AES_RESTR_Finish( DX_NULL,                /*output not need*/
                                 AESRestrContext_ptr );  /*AES restricted context*/

	if( Error != CRYS_OK )
	return Error;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */
End:
   /* turn the RNG off    */
   /*---------------------*/
   LLF_RND_TurnOffTrng( virtualHwBaseAddr );

   /* un mapping of the physical memory to the virtual one  */
   DX_VOS_MemUnMap( &virtualHwBaseAddr,             /* virtual address - in */
                    LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

return Error;

 }/* END of LLF_RND_GetTrngSource */


/*******************************************************************************/
/**
 * @brief The LLF_RND_StartTrngAndGetSource initializes RND and AES engines
 *        and reads random source of needed size from TRNG.
 *
 *        The function is used in ATP and SelfTest functions.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @rndSize[in] - The size of random vector that is required in words.
 * @out_ptr[in,out] The output vector.
 * @param[in/out] AESRestrContext_ptr - The pointer to restricted AES context.
 * @param[in/out] entrEstimData_ptr - The pointer to temp buffer for specific operations
 *                               on entropy estimator.
 *                         Note: for projects, which not use entropy estimator (e.g.
 *                               SW projects), the pointer may be set to NULL.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_RND_StartTrngAndGetSource(
                                   CRYS_RND_State_t     *State,         /*in/out*/
                                   CRYS_RND_Params_t    *trngParams,    /*in/out*/
                                   DxUint16_t            rndSizeWords,  /*in*/
                                   DxUint32_t           *out_ptr,       /*out*/
						           AESResrtictContext_t *AESRestrContext_ptr,   /*in/out*/
						           CRYS_RND_EntropyEstimatData_t *entrEstimData_ptr /*in/out*/ )

{
    /* LOCAL DECLARATIONS */

    CRYSError_t Error = CRYS_OK;


    /* FUNCTION LOGIC */

    /* Initial operations for starting TRNG and AES_RESTRICTED operations:
       (gets semaphore for RND and AES engine on specific platforms) */
    Error = LLF_AES_RESTR_StartInit();
    if( Error != CRYS_OK )
	goto End;

    Error = LLF_RND_Init();
    if( Error != CRYS_OK )
	goto End;

    /* get TRNG source */
    Error = LLF_RND_GetTrngSource(
                               State,         /*in/out*/
                               trngParams,    /*in/out*/
                               rndSizeWords,  /*in*/
                               out_ptr,       /*out*/
					           AESRestrContext_ptr,   /*in/out*/
					           entrEstimData_ptr /*in/out*/ );


End:
    /* Ending of the RND and AES_RESTRICTED operations (releasing the RND and AES
       semaphores on specific platforms) */
    LLF_AES_RESTR_EndFinish();
    LLF_RND_Finish();

	return Error;

 }/* END of LLF_RND_StartTrngAndGetSource */

/****************************************************************************************/
/**
 * @brief The function sets user provided parameters of TRNG into
 *        global State structure and into trngParams_ptr structure.
 *
 * @param[out] State - The pointer to RNG working state structure.
 * @param[out] trngParams_ptr - The pointer to structure containing parameters of HW TRNG.
 *
 * @return CRYSError_t - no return value
 */

 CRYSError_t LLF_RND_SetUserRngParameters( CRYS_RND_State_t   *State,
                                           CRYS_RND_Params_t  *trngParams_ptr  )
{
    /* DECLARATIONS   */

	CRYSError_t  Error = CRYS_OK;

	DxUint32_t           KeySizeWords;
	CRYS_RND_mode_t      TrngMode;
	DxUint32_t           RingOscillatorLength;
	DxUint32_t           SampleCount;
	DxUint32_t           CollectionCount;
	DxUint32_t           MaxAllowedTrngClocksCount;

	DxUint32_t  virtualHwAddress, regVal;

	/* FUNCTION LOGIC */

    Error =  LLF_RND_GetRngParams(
                             &KeySizeWords,
							 (DxUint32_t*)&TrngMode,
							 &RingOscillatorLength,
							 &SampleCount,
							 &CollectionCount,
							 &MaxAllowedTrngClocksCount );

	if( Error != CRYS_OK )
	goto ReturnError;

    /* 1. The AES Key size in words (defines also security strength) */
    State->KeySizeWords = (DxUint16_t)KeySizeWords;
    trngParams_ptr->KeySizeWords =  (DxUint16_t)KeySizeWords;

    /* 2. The TRNG operation mode flag:
          Slow - when the flag is set, Fast - when not set */
    trngParams_ptr->TrngMode = TrngMode;

    /* The ring oscillator length level - defined by 2-bits   */
    trngParams_ptr->RingOscillatorLength = RingOscillatorLength;

	/* sampling and collection counts (count of 192 or 128 -bits blocks),
	    required for entropy accumulation: */
	trngParams_ptr->SampleCount = SampleCount;
	trngParams_ptr->CollectionCount = CollectionCount;
	/* set maximal allowed clocks count (time) for generation of one TRNG block */
	trngParams_ptr->MaxAllowedTrngClocksCount = MaxAllowedTrngClocksCount;

	/*  get the virtual HW address                                           */
	/* --------------------------------------------------------------------- */
	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
                           LLF_RND_HW_CRYPTO_ADDR_SPACE,  /* 16 LS bit space - in */
                           &virtualHwAddress );           /* The virtual address - out */

	if( Error != CRYS_OK )
	goto ReturnError;


	/* Check that user passed key size is supported by AES module */
	CRYS_PLAT_SYS_ReadRegister( virtualHwAddress + LLF_AES_HW_FLAGS_ADDR , regVal );

    if( (regVal & LLF_AES_HW_HLAG_192_256_VAL) == 0 )
	Error = LLF_RND_AES_256_NOT_SUPPORTED_ERROR;

ReturnError:
    return Error;

} /* End of LLF_RND_SetUserRngParameters */
