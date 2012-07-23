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
 
 
#ifndef CRYS_CCM_H
#define CRYS_CCM_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_BaseTypes.h"
#include "CRYS_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:44:53 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_CCM.h#1:incl:12
   *  \author adams
   */

/************************ Defines ******************************/
            
 #define DX_HW_ENGINE 0x1
 #define DX_SW_ENGINE 0x2
    
 #define DX_MAX_RSA_CONTEXT_BUFFERS       1 
 #define DX_MAX_DES_CONTEXT_BUFFERS       1
 #define DX_MAX_AES_CONTEXT_BUFFERS       1
 #define DX_MAX_AESCCM_CONTEXT_BUFFERS    1
 #define DX_MAX_DH_CONTEXT_BUFFERS        1
 #define DX_MAX_ECDHGFp_CONTEXT_BUFFERS   1
 #define DX_MAX_HASH_CONTEXT_BUFFERS      1   /* this number is for TLS min value*/
 #define DX_MAX_SK_CONTEXT_BUFFERS        1
 #define DX_MAX_RC4_CONTEXT_BUFFERS       1
 #define DX_MAX_ECDSA_CONTEXT_BUFFERS     1 
 #define DX_MAX_C2_CIPHER_CONTEXT_BUFFERS 1 
 #define DX_MAX_C2HASH_CONTEXT_BUFFERS    1 

/************************ Enums ********************************/

/* The enum determining the type of context to be allocated using the CRYS_CCM_GetContext function or released
   using the CRYS_CCM_ReleaseContext function */

typedef enum ContextType_enum
{

	 DX_HASH_MD5_CONTEXT   ,
	 DX_HASH_SHA1_CONTEXT  ,
	 DX_HMAC_CONTEXT,
	 DX_DES_1KEY_CONTEXT   ,
	 DX_DES_2KEY_CONTEXT   ,
	 DX_DES_3KEY_CONTEXT   ,
	 DX_RSA_SIGN_CONTEXT   ,
	 DX_RSA_VERIFY_CONTEXT ,
	 DX_AES_CONTEXT,
	 DX_RC4_CONTEXT,
 	 DX_ECDSA_SIGN_CONTEXT,
	 DX_ECDSA_VERIFY_CONTEXT,
	 DX_C2_CIPHER_CONTEXT,
	 DX_C2_HASH_CONTEXT,
	 DX_OTF_CONTEXT,
	 DX_AESCCM_CONTEXT,
	 DX_AESGCM_CONTEXT,

	 ContextTypeLast= 0x7FFFFFFF,

}ContextType_t;

/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/
 
 /* ------------------------------------------------------------
 **
 * @brief This function does the following:
 *        1) activates a semaphore on the required context. 
 *        2) Allocates a free context managed by the context manager.
 *        3) copies the information from the users context to the allocated context.
 *        4) Decrypts the information in the context. 
 *
 * @param[in] UserContext_ptr - The users context pointer.
 * @param[in] Decrypt_flag - Weather or not to make a decrypt operation. e.g. in AES_Init a decrypt is not needed.
 * @param[out] CRYS_GlobalContext_ptr - The returned pointer of the allocated context.
 * @param[out] Type - The context type.
 *
 * @return CRYSError_t - On success CRYS_OK.
 */
 CRYSError_t CRYS_CCM_GetContext(void *UserContext_ptr,void ** CRYS_GlobalContext_ptr,ContextType_t Type,DxUint8_t Decrypt_flag);

#define CRYS_CCM_ReleaseContext(user,global,type) CRYS_OK

#define CRYS_CCM_Init() CRYS_OK

#define CRYS_CCM_Terminate() CRYS_OK

#ifdef __cplusplus
}
#endif

#endif



