/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
 *   protected by copyright, international treaties and various patents.   *
 *   Any copy or reproduction of this Software as permitted below, must    *
 *   include this Copyright Notice as well as any other notices provided   *
 *   under such license.                                                   *
 *                                                                         *
 *   This program shall be governed by, and may be used and redistributed  *
 *   under the terms and conditions of the GNU Lesser General Public       *
 *   License, version 2.1, as published by the Free Software Foundation.   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY liability and WARRANTY; without even the implied      *
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, please write to the          *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CRYS_RND_AES_OLD_128BIT_ONLY   

#ifndef CRYS_RND_H
#define CRYS_RND_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "CRYS_AES.h"
#include "LLF_RND_EngineInfo.h"
#include "LLF_AES_Restrict.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
*  Object CRYS_RND.h
*  State           :  %state%
*  Creation date   :  Thu Dec 16 17:11:32 2004
*  Last modified   :  %modify_time% 18 Mar. 2010
*/
/** @file
*  \brief This file contains the CRYS APIs used for random number generation.
*
*  \version CRYS_RND.h#1:incl:15
*  \author adams, R.Levin
*  \remarks Copyright (C) 2005 by Discretix Technologies Ltd.
*           All Rights reserved
*/


/************************ Defines ******************************/

/* a definitions describing the TRNG Entropy estimator parameters: 
   width of bits prefix and correlation table size */
#define CRYS_RND_nb   8 
#define CRYS_RND_NB  (1 << CRYS_RND_nb)
/* size of buffer used in CRYS_Init (4) and in entropy estimator (512) */ 
#define CRYS_RND_TEMP_BUFF_SIZE_WORDS  (4+512)

#define CRYS_RND_ENGINE_TYPE  LLF_RND_ENGINE_TYPE

   /* maximal requested size counter (12 bits active) - maximal count 
      of generated random 128 bit blocks allowed per one request of 
      Generate function according NIST 800-90 it is (2^12 - 1) = 0x3FFFF */
#define CRYS_RND_REQUESTED_SIZE_COUNTER  0x3FFFF

/* AES output block size in words */
#define CRYS_RND_AES_BLOCK_SIZE_IN_WORDS  CRYS_AES_BLOCK_SIZE_IN_WORDS

/* Entropy max size, full size of Entropy temp buffer and max seed size  */
#define CRYS_RND_ENTROPY_MAX_SIZE_WORDS               24
#define CRYS_RND_ENTROPY_TEMP_BUFFER_MAX_SIZE_WORDS   (CRYS_RND_ENTROPY_MAX_SIZE_WORDS + 2)
#define CRYS_RND_SEED_MAX_SIZE_WORDS                  12

#define CRYS_RND_VECT_IN_RANGE_MAX_COUNT_OF_TRIES    100

/* Bit-fields of Instantiation steps in the StateFlag:
    - b'0: 0 - not instantiated, 1 - instantiated normally;
    - b'1: 1 - loss samples, 0 - no loss;
    - b'2: 1 - time exceeding, 0 - no time exceeding. 
    In case of sample loss or time exceed b`0 must be 0.
*/
#define CRYS_RND_NonInstantiated             0UL
#define CRYS_RND_Instantiated                1UL 
#define CRYS_RND_InstantReseedLossSamples    2UL 
#define CRYS_RND_InstantReseedTimeExceed     4UL  
	
/* The bit-field in the StateFlag, defining the workiong or KAT mode: 
     - b`8: 0 - working mode, 1 - KAT mode; */
#define CRYS_RND_WorkMode                 (0UL << 8)
#define CRYS_RND_KAT_mode                 (1UL << 8)

/* The bit-field in the StateFlag, defining that the previous generated random
   block is valid for comparison with current block or not */ 
#define CRYS_RND_PreviousIsValid          (1UL << 16)

/* Note: for backward compatibility defined an alias name for the StateFlag 
         (see CRYS_RND_State_t structure definition */
#define InstantiationIsDone  StateFlag;

/************************ Enums ********************************/

/* Definition of Fast or Slow mode of random generator (TRNG)*/
typedef  enum
{
	CRYS_RND_Fast  = 0,
	CRYS_RND_Slow  = 1,
	CRYS_RND_ModeLast = 0x7FFFFFFF,
} CRYS_RND_mode_t;


/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/* The internal state of DRBG mechanism based on AES CTR and CBC-MAC
   algorithms. It will be set as global data defined by the following 
   structure  */
typedef  struct  
{
   /* Seed buffer, consists from concatenated Key||V: max size 12 words */
   DxUint32_t  Seed[CRYS_RND_SEED_MAX_SIZE_WORDS];
 
   /* AdditionalInput buffer max size = seed max size words */
   DxUint32_t  AdditionalInput[CRYS_RND_SEED_MAX_SIZE_WORDS];
   
   /* Previous value for continuous test */
   DxUint32_t  PreviousRandValue[CRYS_AES_BLOCK_SIZE_IN_WORDS];
   
   /* reseed counter (32 bits active) - indicates number of requests for entropy 
       since instantiation or reseeding */
   DxUint32_t  ReseedCounter;  
  
  /* key size: 4 or 8 words according to security strength 128 bits or 256 bits*/
   DxUint16_t KeySizeWords; 
     
   /* State flag (see definition of StateFlag above), containing bit-fields, defining: 
    - b'0: instantiation steps:   0 - not done, 1 - done;
    - b'1: loss samples:  0 - not loss, 1 - loss;
    - b'2: time exceeded: 0 - not exceeded;
    - b'8: working or testing mode: 0 - working, 1 - KAT test;
    - b'16: flag defining is Previous random valid or not: 0 -not valid, 1 - valid. 
  */    
   DxUint32_t StateFlag; 
     
   DxUint16_t AddInputSizeWords; /* size of additional data set by user, words   */  
   
   /* TRNG entropy buffer */
   DxUint32_t Entropy[12];

} CRYS_RND_State_t;


/* The CRYS Random Generator Parameters  structure CRYS_RND_Params_t - 
   structure containing the user given Parameters */
typedef struct  CRYS_RND_Params_t
{
   /* key size: 4 or 8 words according to security strength 128 bits or 256 bits*/
   DxUint16_t KeySizeWords;
   
   /* parameters defining TRNG */   
   CRYS_RND_mode_t TrngMode;
   /* ring oscillator length level -  2 bits  */
   DxUint8_t  RingOscillatorLength;
   
   /* sampling interval: count of ring oscillator cycles between 
      consecutive bits sampling */
   DxUint32_t  SampleCount; 
   /* count of 128/192-bits blocks, required for 
      entropy accumulation: */   
   DxUint32_t  CollectionCount;
   /* parameter, defining maximal allowed time (clocks) for 
      generation of one random block EHR */ 
   DxInt32_t  MaxAllowedTrngClocksCount; 
   
}CRYS_RND_Params_t;

/* structure containing parameters and buffers for entropy estimator */
typedef struct
{ 
   /* #include the specific fields that are used by the low level */
   #include "LLF_RND_entr_estim_db_def.h"   

}CRYS_RND_EntropyEstimatData_t;


/*****************************************************************************/
/**********************        Public Functions      *************************/
/*****************************************************************************/



/* -----------------------------------------------------------------------------
 * @brief The function performs instantiation of RNG and creates new 
 *        internal State (including Seed) of RNG. 
 *
 *        It implements the CTR_DRBG_Instantiate function of 9.1 [1]. 
 *  	  This function must be called at least once per system reset (boot) and
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
CIMPORT_C CRYSError_t CRYS_RND_Instantiation( CRYS_RND_EntropyEstimatData_t  *entrEstimBuff_ptr/*in/out*/ );


/* -----------------------------------------------------------------------------
 * @brief The CRYS_RND_UnInstantiation cleans the unused RNG State for security goals.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_UnInstantiation(void);


/* ------------------------------------------------------------/
 * @brief The function performs reseeding of RNG Seed, and performs: 
 * 	     1. Mixing of additional entropy into the working state.
 *		 2. Mixing additional input provided by the user called additional input buffer.        
 *
 *        The function implements the CTR_DRBG_Reseeding function of 10.2.1.4.2 NIST SP [SP800-90]. 
 *  	  This function must be called if reseed counter > reseed interval,
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
CIMPORT_C CRYSError_t CRYS_RND_Reseeding( CRYS_RND_EntropyEstimatData_t  *entrEstimBuff_ptr/*in/out*/ );



/* ------------------------------------------------------------
 * @brief The CRYS_RND_GenerateVector function generates a random vector.
 *
 *        The random function is based on the AES. The code is written according to the:
 *		  10.2.1.5.2 NIST SP [SP800-90].
 *
 *
 * @RndSize[in] - The size of random bytes that is required.
 *
 * @Output_ptr[in,out] The output vector.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_GenerateVector(DxUint16_t RndSize, DxUint8_t *Output_ptr);



/**********************************************************************************************************/
/* 
 * @brief The CRYS_RND_GenerateVectorInRange function generates a random vector Rand in range:
 *            1 < RandVect < MaxVect,   using the CRYS_RND_GenerateVector function:
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
 *        5. If maxVect is not provided, then set MSBit of rndVect to 1.
 *        6. Output the result and Exit. 
 *
 *         Note: Random and Max vectors are given as sequence of bytes, where LSB is most left byte
 *               and MSB = most right one. 
 *
 * @rndSize[in]         - The maximal size of random vector (in bits).
 * @maxVect_ptr[in]     - The pointer to vector defines high limit of random vector.
 * @rndVect_ptr[in,out] - The output vector.
 * @return CRYSError_t  - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_GenerateVectorInRange(DxUint32_t  RndSizeInBits, 
											         DxUint8_t *MaxVect_ptr, DxUint8_t *RndVect_ptr );
											         
											         
/*************************************************************************************/
/**
 * @brief The CRYS_RND_AddAdditionalInput is used for:
 *		  Add additional input by the user.
 *		  This data is used to input additional data for random generation
 *	      reseeding operation or Instantiation.
 *		  The size must be 16 or 32 bytes.
 *
 * @AdditonalInput_ptr[in] - The AdditionalInput buffer.
 * 
 * AdditonalInputSize[in] - The size of the AdditionalInput buffer, can be 16 or 32 bytes.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_AddAdditionalInput( DxUint8_t *AdditonalInput_ptr,
												   DxUint16_t AdditonalInputSize);
													
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
CIMPORT_C void CRYS_RND_EnterKatMode( void );


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
CIMPORT_C void CRYS_RND_DisableKatMode( void );
													

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CRYS_RND_H */

#else /* #ifdef CRYS_RND_AES_OLD_128BIT_ONLY */
/*********************************************************************************/
/************** include old version CRYS_RND_AES_OLD_128BIT_ONLY  ****************/
/*********************************************************************************/

#include "CRYS_RND_AES128.h"
#endif 

