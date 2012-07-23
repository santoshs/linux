
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



/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS.h"


#ifdef DX_RND_TEST_MODE
  /* Global buffer for user given RNG parameters */
  extern CRYS_RND_Params_t UserRngParameters;
#endif

/****************************************************************************************/
/**
 * @brief The function gets user provided parameters of RNG HW.
 *
 *   This implementation is in user competence. Temporary a pseudo function
 *   is implemented for testing goals. To use this implementation the user must define
 *   compilation flag "DX_RND_TEST_MODE", otherwise
 *
 *   Note: In temporary implementation assumed, that users parameters are placed
 *         in global structure UserRngParameters (now placed in ATP tests).
 *
 * @param[in] KeySizeWords - The key size: 4 or 8 words according to security
 *                           strength 128 bits or 256 bits;
 * @param[in] TrngMode -  TRNG mode: 0 - Fast mode, 1 - Slow mode.
 * @param[in] RingOscillatorLength - Ring oscillator length level: should be set
 *            as 2 bits value: 0,1,2,3.
 * @param[in] SampleCount - The sampling count - count of RND blocks of RNG HW
 *            output, required for needed entropy accumulation:
 *              - in "slow" mode a possible values are 4095 to 65535, in steps of 4096;
 *              - in "fast" mode, sampling counter limit is set to a low value -
 *                typically 1 or 2.
 * @param[in] CollectionCount - Count of RND vectors for entropy source compression
 *                              using AES algorithm. Typical values are 60 to 400.
 *                              Relevant for Fast mode only.
 * @param[in] MaxAllowedTrngClocksCount - Maximal allowed clocks count (time)
 *                              for generation of one TRNG block.
 *
 * @return CRYSError_t - CRYS_OK
 */
 CRYSError_t  LLF_RND_GetRngParams(
                            DxUint32_t  *KeySizeWords,
							DxUint32_t  *TrngMode,
							DxUint32_t  *RingOscillatorLength,
							DxUint32_t  *SampleCount,
							DxUint32_t  *CollectionCount,
							DxUint32_t  *MaxAllowedTrngClocksCount )
{

	/* FUNCTION LOGIC */


#ifdef DX_RND_TEST_MODE
    /* 1. The AES Key size in words */
    *KeySizeWords = UserRngParameters.KeySizeWords ;

    /* 2. The TRNG operation mode flag:  0 - Fast, 1 - Slow */
    *TrngMode = UserRngParameters.TrngMode;

    /*The ring oscillator length level */
    *RingOscillatorLength = UserRngParameters.RingOscillatorLength;

	/* sampling and collection counts */
	*SampleCount = UserRngParameters.SampleCount;
	*CollectionCount = UserRngParameters.CollectionCount;
	*MaxAllowedTrngClocksCount = UserRngParameters.MaxAllowedTrngClocksCount;

#else
	/*-------------------------------------------*/
	/* Users implementation of this function     */
	/*                                           */
	/*  Note: For using default implementation   */
	/*        with constant values - comment the */
    /*        #error message                     */
	/*-------------------------------------------*/

    //#error "LLF_RND_GetRngParams is porting function - must be implemented by user"

    /* Default implementation for compilation goals */
    /*----------------------------------------------*/
    /* 1. The AES Key size in words */
    *KeySizeWords = 4 ;
    /* 2. The TRNG operation mode flag:  0 - Fast, 1 - Slow */
    *TrngMode = CRYS_RND_Fast;

    /*The ring oscillator length level */
    *RingOscillatorLength = 0;

	/* sampling and collection counts */
	*SampleCount = 200;
	*CollectionCount = 5;
	/* Max allowed clock count for generation of 1 TRNG block */
	*MaxAllowedTrngClocksCount = 5000000;
#endif

    return CRYS_OK;

} /* End of LLF_RND_GetRngParams */
