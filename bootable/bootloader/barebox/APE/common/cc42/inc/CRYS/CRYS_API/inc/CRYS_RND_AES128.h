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
#ifdef CRYS_RND_AES_OLD_128BIT_ONLY   
 
 
#ifndef CRYS_RND_H
#define CRYS_RND_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"
#include "CRYS_Defs.h"
#include "LLF_RND_EngineInfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object CRYS_RND.h
   *  State           :  %state%
   *  Creation date   :  Thu Dec 16 17:11:32 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains the CRYS APIs used for random number generation.
   *
   *  \version CRYS_RND.h#1:incl:15
   *  \author adams
   */




/************************ Defines ******************************/

/** 
@brief - a definition describing the low level Engine type ( SW , Hardware , Etc )
*/

#define CRYS_RND_ENGINE_TYPE  LLF_RND_ENGINE_TYPE


/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/* ------------------------------------------------------------
 * @brief The CRYS_RND_GenerateVector function generates a random vector,
 *			 using the FIPS-PUB [SP800-90].
 *
 *        The random function is based on the AES. The code is written according to the:
 *			 CryptoCore: Random Number Generators System Specification (Revision 1.3)
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
													
/**************************************************************************************/
/**
 * @brief The CRYS_RND_ResetSeed resets the SEED generated by the low level 
 *        ( hardware or OPerating system service on software ).  
 * This is Dummy function for backward compatibility
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_ResetSeed( void );


/**********************************************************************************************************/
/**
 * @brief The CRYS_RND_StartResetSeed start the hardware bit random bit collection for the seeding/instantiation 
 *        operation)
 * 
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_StartResetSeed( void );												


/*************************************************************************************
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


/*************************************************************************************
 * @brief The CRYS_RND_Reseeding is used for:
 * 			 1. Mixing of additional entropy into the working state.
 *			 2. Mixing additional input provided by the user called additional input buffer.        
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_Reseeding( void );

/* -----------------------------------------------------------------------------
 * @brief The CRYS_RND_Instantiation initialize the RNG. Its required before any
 *					output can be produced. It is initiated in the CRYS_RND_Init phase.
 *					This function must be called at least once per system reset.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CIMPORT_C CRYSError_t CRYS_RND_Instantiation( void );


#ifdef __cplusplus
}
#endif

#endif

#endif /*CRYS_RND_AES_OLD_128BIT_ONLY*/
