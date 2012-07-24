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
 
 #ifndef LLF_PKI_RSA_EXTEND_H
#define LLF_PKI_RSA_EXTEND_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_RSA_Types.h"
#include "LLF_PKI_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  17 Jan. 2010
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This extension to RSA module contains additional APIs, 
   *         used in some projects.
   *
   *  \version LLF_PKI_RSA_Extend.h#1:incl:1
   *  \author R.Levin
   */

/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  ******************************/

/************************ Public Variables **********************/

/************************ Public Functions ******************************/


/*******************************************************************************************/
/**
 * @brief This function LLF_PKI_RSA_GeneratePQprimes generates pair of prime factors P,Q.
 *         
 * @param[in]  e_ptr - The pointer to the public exponent (public key).
 *                     Allowed values = 0x3, 0x11, 0x1001.
 * @param[in]  eSizeInBits - The public exponent size in bytes.
 * @param[in]  primeSizeInBits - size of prime factor in bits.
 * @param[out] p_ptr - the pointer to first prime factor buffer of size not less, than 
 *                     prime factor size in words.
 * @param[out] q_ptr - the pointer to second prime factor buffer of size not less, than 
 *                     prime factor size in words.
 * @param[in] testsCount - Count of Miller-Rabin tests. If testsCount = 0,
 *                         then count of tests will be set automatically
 * @param[in]  keyGenData_ptr - temp buffer of defined type (not need to be initialized.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in .
 */
CRYSError_t LLF_PKI_RSA_GeneratePQprimes( 
	                                 DxUint32_t       *e_ptr,                 /* in */
	                                 DxUint32_t        eSizeInBits,           /* in */
	                                 DxUint32_t        primeSizeInBits,       /* in */
	                                 DxUint32_t       *p_ptr,                 /* out */  
	                                 DxUint32_t       *q_ptr,                 /* out */
	                                 DxUint32_t        testsCount,            /* in */
	                                 CRYS_RSAKGData_t *keyGenData_ptr );      /* in */
                                      

/*******************************************************************************************/                                         
/**
   @brief LLF_PKI_RSA_CalculatePubKeyFromPQ calculates a public key using P and Q.
 
   @param[in] e_ptr - The pointer to the public exponent (public key). 
                      Allowed values = 0x3, 0x11, 0x010001.
   @param[in] eSizeInBits  - The public exponent size in bits.
   @param[in] keySizeBits  - The size of the key, in bits. Supported sizes are:
                              - for PKI without PKA HW: all 256 bit multiples between 512 - 2048;
                              - for PKI with PKA: HW all 32 bit multiples between 512 - 2112;
   @param[in] p_ptr - A pointer to the valid first prime factor in words (LSWord is the left most). 
   @param[in] q_ptr - A pointer to the valid second prime factor in words (LSWord is the left most). 
   @param[out] pubKey_ptr - A pointer to the public key structure. 
                           This structure is used as input to the CRYS_RSA_PRIM_Encrypt API.

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_SIZE
*/
CRYSError_t LLF_PKI_RSA_CalculatePubKeyFromPQ(
                                        DxUint32_t                *e_ptr,             /* in : public exponent – little endian bytes array (LSB – left most) */                                        
                                        DxUint16_t                 eSizeInBits,       /* in: size of public exponent */
                                        DxUint16                   keySizeBits,       /* in: size of key modulus in bits */
                                        DxUint32_t                *p_ptr,             /* in: first prime – little endian words array */
                                        DxUint32_t                *q_ptr,             /* in: second prime  – little endian words array */
                                        CRYSRSAPubKey_t           *pubKey_ptr );      /* out: user public key structure */


/*******************************************************************************************/                                         
/**
   @brief LLF_PKI_RSA_CalculatePrivKeyFromPQ calculates private key on non CRT and CRT modes.
 
   @param[in] e_ptr - The pointer to the public exponent (public key)
                      Allowed values = 0x3, 0x11, 0x010001.
   @param[in] eSizeInBits  - The public exponent size in bits.
   @param[in] keySizeBits  - The size of the key, in bits. Supported sizes are:
                              - for PKI without PKA HW: all 256 bit multiples between 512 - 2048;
                              - for PKI with PKA: HW all 32 bit multiples between 512 - 2112;
   @param[in] p_ptr - A pointer to the valid first prime factor in words (LSWord is the left most). 
   @param[in] q_ptr - A pointer to the valid second prime factor in words (LSWord is the left most). 
   @param[out] privKey_ptr - A pointer to the private key structure. 
                           This structure is used as input to the CRYS_RSA_PRIM_Decrypt API.

   @return CRYSError_t - CRYS_OK,
                         CRYS_RSA_INVALID_EXPONENT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PRIV_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_INVALID_PUB_KEY_STRUCT_POINTER_ERROR,
                         CRYS_RSA_KEY_GEN_DATA_STRUCT_POINTER_INVALID,
                         CRYS_RSA_INVALID_MODULUS_SIZE,
                         CRYS_RSA_INVALID_EXPONENT_SIZE
*/
CRYSError_t LLF_PKI_RSA_CalculatePrivKeyFromPQ(
                                        DxUint32_t                *e_ptr,              /* in : public exponent – little endian bytes array (LSB – left most) */                                        
                                        DxUint16_t                 eSizeInBits,        /* in: size of public exponent */
                                        DxUint16                   keySizeBits,        /* in: size of key modulus in bits */
                                        DxUint32_t                *p_ptr,              /* in: first prime – little endian words array */
                                        DxUint32_t                *q_ptr,              /* in: second prime  – little endian words array */
                                        CRYS_RSA_DecryptionMode_t  privKeyMode,        /* in: mode of RSA key: 0 – non CRT,  1 – CRT */
                                        CRYSRSAPrivKey_t          *privKey_ptr );      /* out: user private key structure*/
                          
                           

/************************ Private Functions **********************/

#ifdef __cplusplus
}
#endif

#endif
