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
 
 
#ifndef CRYS_TEST_FLAGS_H
#define CRYS_TEST_FLAGS_H

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 16:56:19 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A file containing the flags for all of the module testing.
   *
   *  \version CRYS_test_flags.h#1:incl:1
   *  \author adams
   */



/************************ Defines ******************************/

/*  test flag definitions - used to activate a specific module test */

#define CRYS_AES_MODULE_TEST                      0
#define CRYS_AES_SEP_MODULE_TEST                  0
#define CRYS_AES_MODULE_WRAP_TEST                 0
#define CRYS_AES_XTS_MODULE_TEST                  0
#define CRYS_AES_CSI_MODULE_TEST                  0
#define CRYS_AESCCM_MODULE_TEST                   0
#define CRYS_DES_MODULE_TEST                      0	
#define CRYS_DES_SEP_MODULE_TEST                  0
#define CRYS_DES_CSI_MODULE_TEST                  0		
#define CRYS_HASH_MODULE_TEST                     0
#define CRYS_HASH_SEP_MODULE_TEST                 0
#define CRYS_HASH_CSI_MODULE_TEST                 0
#define CRYS_HMAC_MODULE_TEST                     0
#define CRYS_HMAC_SEP_MODULE_TEST                 0
#define CRYS_HMAC_CSI_MODULE_TEST                 0
#define CRYS_RND_MODULE_TEST                      0
#define CRYS_RSA_PRIM_MODULE_TEST                 0
#define CRYS_RSA_KG_MODULE_TEST                   0
#define CRYS_RSA_INV_MOD_WORD_MODULE_TEST         0
#define CRYS_RSA_PSS_OAEP_ENC_DEC_MODULE_TEST     0
#define CRYS_RSA_PSS_SIGN_VER_MODULE_TEST         0
#define CRYS_RSA_SEP_PSS_SIGN_VER_MODULE_TEST     0
#define CRYS_RSA_PKCS1_VER15_SIGN_VER_MODULE_TEST 0
#define CRYS_RSA_PKCS1_VER15_ENC_DEC_MODULE_TEST  0
#define CRYS_DH_PKCS3_MODULE_TEST                 0
#define CRYS_DH_ANSIX_942_MODULE_TEST             0
#define CRYS_DH_ANSIX_942_KG_MODULE_TEST          0
#define CRYS_RC4_MODULE_TEST                      0
#define CRYS_RC4_SEP_MODULE_TEST                  0
#define CRYS_C2_MODULE_TEST                       0
#define CRYS_KDF_MODULE_TEST                      0
#define CRYS_OTF_MODULE_TEST                      0
#define CRYS_SELF_TEST_MODULE_TEST                0

#ifndef CRYS_NO_SST_SUPPORT 
#define CRYS_SST_MODULE_TEST							0
#define CRYS_SST_RSA_KG_MODULE_TEST						0
#define CRYS_SST_RSA_PRIM_MODULE_TEST					0
#define CRYS_SST_RSA_PKCS1_VER15_ENC_DEC_MODULE_TEST	0
#define CRYS_SST_RSA_PKCS1_VER15_SIGN_VER_MODULE_TEST	0
#define CRYS_SST_RSA_PSS_OAEP_ENC_DEC_MODULE_TEST		0
#define CRYS_SST_RSA_PSS_SIGN_VER_MODULE_TEST			0
#endif

#define CRYS_AES_DSM_SIM_MODULE_TEST          0
#define CRYS_DES_DSM_SIM_MODULE_TEST    	  0
#define CRYS_HASH_DSM_SIM_MODULE_TEST    	  0
#define CRYS_PKI_PRIM_DSM_SIM_MODULE_TEST     0
#define CRYS_RND_DSM_SIM_MODULE_TEST          0

/* low level module tests */
#define LLF_PKI_PKA_MODULE_TEST               0
#define LLF_PKI_MMUL_MODULE_TEST              0 
#define LLF_PKI_EXP_MODULE_TEST               0
#define LLF_PKI_HCALC_MODULE_TEST             0
#define LLF_PKI_MON_EXP_MODULE_TEST           0
#define LLF_PKI_RMUL_MODULE_TEST              0
#define LLF_CRT_PKI_EXP_MODULE_TEST           0
#define LLF_PKI_DIVIDE_MODULE_TEST            0
#define LLF_PKI_X931_KEYGEN_FIND_PRIME_TEST   0
#define LLF_C2_MODULE_TEST                    0

/* ECC low level module tests */

/* LLF PKI_UTIL modules tests */
#define LLF_PKI_UTIL_MMUL_MODULE_TEST         0
#define LLF_PKI_UTIL_EXP_MODULE_TEST          0
#define LLF_PKI_UTIL_DIVIDE_MODULE_TEST       0

#ifndef CRYS_NO_ECPKI_SUPPORT
	
/* CRYS_ECPKI modules tests. 
   Note: For using these tests the CRYS_TST_ENABLE_CALL_LLF 
   compilation flag must be defined in project properties */
#define CRYS_ECPKI_KG_MODULE_TEST             0
#define CRYS_ECDH_MODULE_TEST                 0
#define CRYS_ECDSA_MODULE_TEST                0
#define CRYS_ECDSA_SEP_MODULE_TEST            0
#define CRYS_ECPKI_ELGAMAL_MODULE_TEST        0

/* LLF ECPKI modules tests */
#define LLF_ECPKI_ARITHMETIC_TEST             0
#define LLF_ECPKI_FULL_TEST		              0

#endif /*CRYS_NO_ECPKI_SUPPORT*/

#ifndef CRYS_NO_CMLA_SUPPORT

#define CRYS_CMLA_MODULE_TEST                 0

#endif /*CRYS_NO_CMLA_SUPPORT*/


/* Note: in order to operate PSS functionality on other Hash mode than SHA-1 - 
 * CRYS_RSA_SIGN_USE_TEMP_SALT & DEBUG_OAEP_SEED must be zero*/
 /*NOTE: these 2 defines are connected and dependent*/
#define CRYS_RSA_HASH_MODE_4_TEST /*CRYS_RSA_HASH_MD5_mode*/ CRYS_RSA_HASH_SHA1_mode /*CRYS_RSA_After_MD5_mode*/ /*CRYS_RSA_After_SHA1_mode*/ /*CRYS_RSA_HASH_NO_HASH_mode*/
#define AFTER_HASH_MODE_ACTIVE 0

/* a flag enabling printings */
#define CRYS_ENABLE_LOG_PRINTINGS 	  0
#define CRYS_ENABLE_SYS_TST_PRINTINGS 0
    
/* these flags control the printings of the Develop group and the testing group */
/* all of the printings below the level are enabled .
   setting the max level to 0 will deactivate all of the printings */   
#define CRYS_LOG_DEV_MAX_LEVEL_ENABLED 0
#define CRYS_LOG_TST_MAX_LEVEL_ENABLED 0

/* setting printings on the access to the registers */
#define CRYS_TST_HARDWARE_REG_PRINT_MODE     0
#define CRYS_TST_HARDWARE_REG_SIM_PRINT_MODE 0

/*When using a known Salt for PSS Sign Verify*/
#define CRYS_RSA_SIGN_USE_TEMP_SALT 0

/*When using a known Salt for PSS Encrypt Decrypt*/
#define DEBUG_OAEP_SEED 0

/*When using a constant random for DH*/
#define CRYS_DH_DEBUG_CONST_RANDOM_MODE 0

/*For testing in Symbian OS - whether to test using threads or simply running the tests*/
#define SYMBIAN_FLAG_MULTITASKING_TEST 0

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif




