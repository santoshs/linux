/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
#ifndef LLF_RND_H
#define LLF_RND_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_RND.h"
#include "LLF_RND_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:48 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AES.h#1:incl:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2010 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

/************************ Defines ******************************/

/* size of Entropy block to read from TRNG */
#define LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS          6
#define LLF_RND_HW_TRNG_EHR_WIDTH_IN_BITS          (32*LLF_RND_HW_TRNG_EHR_WIDTH_IN_WORDS)
#define LLF_RND_ENTROPY_ACCUMULATED_IN_BLOCK_BITS  64
#define LLF_RND_MINIMAL_SAMPLE_COUNT_FOR_FAST_MODE 60

/* relation between TRNG source size and required entropy size, contained 
   in the source, when TRNG is in "Fast" mode */
#define LLF_RND_SOURCE_AND_ENTROPY_SIZES_RELATION  2

/****************************** Enums ***********************************/

/**************************** Typedefs **********************************/

/***************************** Structs  *********************************/

/**************************** Global Data   *****************************/

/************************ Public Functions ******************************/

/*******************************************************************************************/
/**
 * @brief The LLF_RND_GetRngBytes returns size of random source needed for collection
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
                                 CRYS_RND_Params_t* trngParams, /*in*/         
                                 DxUint16_t   *entropySizeWords_ptr );   /*in/out*/


/****************************************************************************************/
/**
 * @brief The function gets user provided parameters of RNG HW. 
 *
 *   This implementation is in user competence. Temporary a pseudo function 
 *   is implemented for testing goals. 
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
 *                              using AES algorithm. Typical values are 200 to 1700, 
 *                              in steps of 100. Relevant for Fast mode only.
 * @param[in] MaxAllowedTrngClocksCount - Maximal allowed clocks count (time) 
 *                              for generation of one TRNG block.  
 *
 * @return CRYSError_t - no return value  
 */
 CRYSError_t  LLF_RND_GetRngParams( 
                            DxUint32_t  *KeySizeWords,
							DxUint32_t  *TrngMode,
							DxUint32_t  *RingOscillatorLength,
							DxUint32_t  *SampleCount,
							DxUint32_t  *CollectionCount,
							DxUint32_t  *MaxAllowedTrngClocksCount );


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
                                           CRYS_RND_Params_t  *trngParams_ptr  ); 
 
 
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
                          DxUint32_t            virtualHwBaseAddr );
                          

/**********************************************************************************************************/
/**
 * @brief The LLF_RND_TurnOffTrng stops the hardware random bits collection.
 *
 * @param[in] virtualHwBaseAddr - The HW registers virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void LLF_RND_TurnOffTrng( DxUint32_t  virtualHwBaseAddr /*in*/ );


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
						DxUint32_t        virtualHwBaseAddr /*in*/ );
						
/****************************************************************************************/
/**
 * @brief The function gets the block of entropy source from RNG. 
 *
 *        -  In Slow mode the function reads from HW one 198/128 bits of entropy
 *           (depending onEHR width) and outputs them "AS IS".
 *        -  In Fast mode the function reads entropy source from HW, compresses it 
 *           in one 128 bits block using AES-MAC so, that it contains at least 64 
 *           bits of entropy and sets this block to output.
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
		                DxUint32_t         virtualHwBaseAddr );       /*in*/
						
 
/****************************************************************************************/
/**
 * @brief The function performs initial step of RND processing (gets HW RND semaphore
 *        on appropriate LLF platforms).  
 *        
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_RND_Init( void );


/****************************************************************************************/
/**
 * @brief The function finishes random processing (gets HW RND semaphore
 *        on appropriate LLF platforms).  
 *        
 * @return - no return  value.
 */
 void LLF_RND_Finish( void );


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
 CRYSError_t LLF_RND_StartTrngHW( CRYS_RND_State_t   *State       /*out*/ );


/*******************************************************************************/
/**
 * @brief The LLF_RND_GetTrngSource reads random source of needed size from TRNG.
 *
 *        The function is used in ATP and Self, Instantiation and Reseeding functions.
 *
 * @param[in/out] State - The pointer to the internal State buffer of DRNG.
 * @param[in/out] trngParams - The pointer to structure, containing TRNG parameters.
 * @rndSize[in] - The size of random vector that is required in words.
 * @output_ptr[in,out] The output vector.
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
                                   DxUint32_t           *output_ptr,    /*out*/
						           AESResrtictContext_t *AESRestrContext_ptr,   /*in/out*/
						           CRYS_RND_EntropyEstimatData_t *entrEstimData_ptr /*in/out*/ );  

/*******************************************************************************/
/**
 * @brief The LLF_RND_StartTrngAndGetSource initializes RND and AES engines 
 *        and reads random source of needed size from TRNG.
 *
 *        The function is used in SelfTest function.
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
						           CRYS_RND_EntropyEstimatData_t *entrEstimData_ptr /*in/out*/ );

#ifdef __cplusplus
}
#endif

#endif
