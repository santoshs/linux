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
 
#ifndef CRYS_SELF_TEST_H
#define CRYS_SELF_TEST_H

#include "DX_VOS_BaseTypes.h"
#include "CRYS_RSA_Types.h"
#include "CRYS_ECPKI_Types.h"
#include "CRYS_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /** @file
   *  \brief Perform self tests for specified engines
   *
   *  \author adams
   */

/************************ Defines ******************************/

/* @brief the following defines enable us to select the engines we want to test
          these defines also describe the bit field of the result */
#define CRYS_SELF_TEST_AES_BIT				0x0001
#define CRYS_SELF_TEST_DES_BIT				0x0002
#define CRYS_SELF_TEST_HASH_BIT				0x0004
#define CRYS_SELF_TEST_HMAC_BIT				0x0008
#define CRYS_SELF_TEST_RSA_ENCDEC_BIT	    0x0010
#define CRYS_SELF_TEST_RND_TRNG_BIT	    	0x0020
#define CRYS_SELF_TEST_RND_DRBG_BIT	    	0x0040
#define CRYS_SELF_TEST_RND_BIT  CRYS_SELF_TEST_RND_TRNG_BIT
#define CRYS_SELF_TEST_ECC_BIT	    	    0x0080  /*ECDSA bit*/ 
#define CRYS_SELF_TEST_AESGCM_BIT    	    0x0100 
#define CRYS_SELF_TEST_ECC_ELGAMAL_BIT 	    0x0101 
#define CRYS_SELF_TEST_ECC_DH_BIT	   	    0x0102 
#define CRYS_SELF_TEST_RSA_SIGN_VERIF_BIT	0x0104
#define CRYS_SELF_TEST_DH_BIT	            0x0108
#define CRYS_SELF_TEST_C2_BIT	            0x0110
#define CRYS_SELF_TEST_RC4_BIT	            0x0111


#define CRYS_SELF_TEST_ALL_BIT     (CRYS_SELF_TEST_AES_BIT |  \
                                    CRYS_SELF_TEST_DES_BIT |  \
                                    CRYS_SELF_TEST_HASH_BIT | \
									CRYS_SELF_TEST_HMAC_BIT | \
                                    CRYS_SELF_TEST_RSA_ENCDEC_BIT | \
                                    CRYS_SELF_TEST_RND_TRNG_BIT | \
                                    CRYS_SELF_TEST_RND_DRBG_BIT |\
                                    CRYS_SELF_TEST_ECC_BIT | \
									CRYS_SELF_TEST_AESGCM_BIT | \
								    CRYS_SELF_TEST_ECC_ELGAMAL_BIT )

#define CRYS_SELF_TEST_NO_RND     (CRYS_SELF_TEST_AES_BIT |  \
                                    CRYS_SELF_TEST_DES_BIT |  \
                                    CRYS_SELF_TEST_HASH_BIT | \
									CRYS_SELF_TEST_HMAC_BIT | \
                                    CRYS_SELF_TEST_RSA_ENCDEC_BIT |\
                                    CRYS_SELF_TEST_ECC_BIT | \
									CRYS_SELF_TEST_AESGCM_BIT | \
									CRYS_SELF_TEST_ECC_ELGAMAL_BIT)

/************************ Structs  ******************************/

#if( CRYS_RSA_SIZE_IN_WORDS_OF_CRYSRSAPrivKey_t > 600 )
	#define CRYS_SELF_TEST_BUFFER_SIZE_IN_WORDS (CRYS_RSA_SIZE_IN_WORDS_OF_CRYSRSAPrivKey_t + sizeof(CRYS_EC_ELGAMAL_TempData_t)/4)
#else
	#define CRYS_SELF_TEST_BUFFER_SIZE_IN_WORDS  (600 + sizeof(CRYS_EC_ELGAMAL_TempData_t)/4)
#endif

typedef struct
{
   DxUint32_t temp1[CRYS_SELF_TEST_BUFFER_SIZE_IN_WORDS];
   
}CRYS_SelfTestData_t;   


/************************ Public Functions **********************/

/**
 * @brief This function executes the CRYS self test for the specified engines
 *
 * @param[in] EnginesSelect - Specify the engines to test according to the following values:\n
 *
 *                            CRYS_SELF_TEST_AES_BIT			\n
 *                            CRYS_SELF_TEST_DES_BIT			\n
 *                            CRYS_SELF_TEST_HASH_BIT			\n
 *							  CRYS_SELF_TEST_HMAC_BIT			\n
 *                            CRYS_SELF_TEST_RSA_ENCDEC_BIT     \n
 *                            CRYS_SELF_TEST_RND_TRNG_BIT    	\n
 *                            CRYS_SELF_TEST_RND_DRBG_BIT       \n
 *                            CRYS_SELF_TEST_AESGCM_BIT			\n
 *                            CRYS_SELF_TEST_ECC_ELGAMAL_BIT    \n
 *                            CRYS_SELF_TEST_ALL_BIT			\n
 *
 * @param[out] EnginesTestResult_ptr - a bit field describing the test results of the selected engines.
 *                                    The bit field is the same as described above.
 *                                    For each bit specified in the input: 0 - test passes, 1 - test failed. 
 *
 * @param[in] TempData_ptr - A pointer to a scratchpad buffer required for the self test.
 *							 The size of the buffer is specified in the typedef.
 *                            
 * @return CRYSError_t - On success the function returns the value CRYS_OK, and on failure a non-ZERO error.
 *
 * \note This function is useful mainly to test hardware implementations of the cryptographic engines upon 
 * system startup. This function is to fulfil the FIPS demands for power-up self tests.
 */

 CRYSError_t  CRYS_SelfTest(DxUint16_t EnginesSelect,
                            DxUint16_t *EnginesTestResult_ptr,
                            CRYS_SelfTestData_t *TempData_ptr);

#ifdef __cplusplus
}
#endif

#endif


