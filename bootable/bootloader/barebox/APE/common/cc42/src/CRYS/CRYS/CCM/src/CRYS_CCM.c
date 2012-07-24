/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

/* .............. CRYS level includes ................. */

#include "DX_VOS_Mem.h"
#include "DX_VOS_Sem.h"
#include "DX_VOS_Errors.h"
#include "CRYS.h"
#include "CRYS_CCM.h"
#include "CRYS_CCM_error.h"
#include "PLAT_SystemDep.h"


/************************ Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums ********************************/

/************************ Typedefs *****************************/

/************************ Global Data **************************/

/* **********************************************************************
 *
 * implementation when we are not encrypting and decrypting the received context
 * from the user , we are actually passing the internal context buffer of the context allocated
 * by the user - on this method there is no protection on the context and no usage of global data
 *
 * ********************************************************************** */

//#if defined(CRYS_NO_CONTEXT_ENCRYPTION_PROTECTION) || defined(CRYS_NO_AES_SUPPORT)

 /* ------------------------------------------------------------
 **
 * @brief This function does returnees the pointer on the internal context buffer.
 *
 * @param[in] UserContext_ptr - The users context pointer.
 * @param[in] Decrypt_flag - Weather or not to make a decrypt operation. e.g. in AES_Init a decrypt is not needed.
 * @param[out] CRYS_GlobalContext_ptr - The returned pointer of the allocated context.
 * @param[out] Type - The context type.
 *
 * @return CRYSError_t - On success CRYS_OK.
 */

 CRYSError_t CRYS_CCM_GetContext(void *UserContext_ptr,void ** CRYS_GlobalContext_ptr,ContextType_t Type,DxUint8_t AES_Dec_Enc_flag)
 {

    /* LOCAL DECLERATIONS */

    /* error identifier */
	CRYSError_t Error;

	/*The types of user contexts possibilities*/
  #ifndef CRYS_NO_HASH_SUPPORT
	CRYS_HASHUserContext_t    *HASH_user_context_ptr;
  #endif

  #ifndef CRYS_NO_HMAC_SUPPORT
	CRYS_HMACUserContext_t    *HMAC_user_context_ptr;
  #endif

  #ifndef CRYS_NO_AES_SUPPORT
	CRYS_AESUserContext_t     *AES_user_context_ptr;
  #endif

  #ifndef CRYS_NO_DES_SUPPORT
	CRYS_DESUserContext_t     *DES_user_context_ptr;
  #endif

  #ifndef CRYS_NO_PKI_SUPPORT
	CRYS_RSAPrivUserContext_t *RSAPriv_user_context_ptr;
	CRYS_RSAPubUserContext_t  *RSAPub_user_context_ptr;
  #endif

  #ifndef CRYS_NO_RC4_SUPPORT
    CRYS_RC4UserContext_t     *RC4_user_context_ptr;
  #endif

  #ifndef CRYS_NO_ECPKI_SUPPORT
	CRYS_ECDSA_SignUserContext_t    *ECDSA_sign_user_context_ptr;
	CRYS_ECDSA_VerifyUserContext_t  *ECDSA_verify_user_context_ptr;
  #endif /* CRYS_NO_ECPKI_SUPPORT */

  #ifndef CRYS_NO_C2_SUPPORT
	CRYS_C2CipherUserContext_t    *C2Cipher_user_context_ptr;
	CRYS_C2HASHUserContext_t      *C2HASH_user_context_ptr;
  #endif /* CRYS_NO_C2_SUPPORT */

  #ifndef CRYS_NO_OTF_SUPPORT
    CRYS_OTFUserContext_t     *OTF_user_context_ptr;
  #endif

  #ifndef CRYS_NO_AESCCM_SUPPORT
	CRYS_AESCCM_UserContext_t     *AESCCM_user_context_ptr;
  #endif

#ifndef CRYS_NO_AESGCM_SUPPORT
	CRYS_AESGCM_UserContext_t     *AESGCM_user_context_ptr;
#endif


	/* FUNCTION LOGIC */

	/* ............... initializing local variables ....................... */
	/* -------------------------------------------------------------------- */

    /*This variable is not used when the flag Encryption compilation flag is not defined*/
    AES_Dec_Enc_flag = AES_Dec_Enc_flag;

	/* initialize the error identifier to CRYS_OK */
	Error = CRYS_OK;

	/* switch case for setting the return pointer for the working context */
	switch(Type)
	{
      #ifndef CRYS_NO_HASH_SUPPORT
	   case DX_HASH_MD5_CONTEXT:
	   case DX_HASH_SHA1_CONTEXT:

			HASH_user_context_ptr = (CRYS_HASHUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = HASH_user_context_ptr->context_buff;
			break;

      #ifndef CRYS_NO_HMAC_SUPPORT
	   case DX_HMAC_CONTEXT:

			HMAC_user_context_ptr = (CRYS_HMACUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = HMAC_user_context_ptr->context_buff;
			break;
	  #endif
	  #endif

      #ifndef CRYS_NO_AES_SUPPORT
	   case DX_AES_CONTEXT:

			AES_user_context_ptr = (CRYS_AESUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = AES_user_context_ptr->context_buff;
			break;
      #endif

      #ifndef CRYS_NO_PKI_SUPPORT
	   case  DX_RSA_SIGN_CONTEXT:

			RSAPriv_user_context_ptr = (CRYS_RSAPrivUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = RSAPriv_user_context_ptr->context_buff;
			break;

	   case  DX_RSA_VERIFY_CONTEXT:

			RSAPub_user_context_ptr = (CRYS_RSAPubUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = RSAPub_user_context_ptr->context_buff;
			break;
	  #endif

	  #ifndef CRYS_NO_DES_SUPPORT
	   case  DX_DES_1KEY_CONTEXT:
	   case  DX_DES_2KEY_CONTEXT:
	   case  DX_DES_3KEY_CONTEXT:

			DES_user_context_ptr = (CRYS_DESUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = DES_user_context_ptr->context_buff;
			break;
      #endif

      #ifndef CRYS_NO_RC4_SUPPORT

	   case DX_RC4_CONTEXT:

			RC4_user_context_ptr = (CRYS_RC4UserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = RC4_user_context_ptr->context_buff;
			break;

	  #endif

      #ifndef CRYS_NO_ECPKI_SUPPORT

	   case  DX_ECDSA_SIGN_CONTEXT:

			ECDSA_sign_user_context_ptr = (CRYS_ECDSA_SignUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = ECDSA_sign_user_context_ptr->context_buff;
			break;

	   case  DX_ECDSA_VERIFY_CONTEXT:

			ECDSA_verify_user_context_ptr = (CRYS_ECDSA_VerifyUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = ECDSA_verify_user_context_ptr->context_buff;
			break;

     #endif /* CRYS_NO_ECPKI_SUPPORT */

     #ifndef CRYS_NO_C2_SUPPORT

	   case  DX_C2_CIPHER_CONTEXT:

			C2Cipher_user_context_ptr = (CRYS_C2CipherUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = C2Cipher_user_context_ptr->context_buff;
			break;

	   case  DX_C2_HASH_CONTEXT:

			C2HASH_user_context_ptr = (CRYS_C2HASHUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = C2HASH_user_context_ptr->context_buff;
			break;

    #endif /* CRYS_NO_C2_SUPPORT */

    #ifndef CRYS_NO_OTF_SUPPORT

     case DX_OTF_CONTEXT:

			OTF_user_context_ptr = (CRYS_OTFUserContext_t *)UserContext_ptr;
			*CRYS_GlobalContext_ptr = OTF_user_context_ptr->context_buff;
			break;

    #endif /* CRYS_NO_OTF_SUPPORT */

    #ifndef CRYS_NO_AESCCM_SUPPORT

	 case DX_AESCCM_CONTEXT:

		 AESCCM_user_context_ptr = (CRYS_AESCCM_UserContext_t *)UserContext_ptr;
		 *CRYS_GlobalContext_ptr = AESCCM_user_context_ptr->context_buff;
		 break;

    #endif /* CRYS_NO_AESCCM_SUPPORT */

    #ifndef CRYS_NO_AESGCM_SUPPORT

	 case DX_AESGCM_CONTEXT:

		 AESGCM_user_context_ptr = (CRYS_AESGCM_UserContext_t *)UserContext_ptr;
		 *CRYS_GlobalContext_ptr = AESGCM_user_context_ptr->context_buff;
		 break;

   #endif /* CRYS_NO_AESGCM_SUPPORT */


   default:
		return CRYS_CCM_CONTEXT_TYPE_ERROR ;

	}/* end of setting the returned context pointer - switch case */

	return Error;

 }/* END OF CRYS_CCM_GetContext */

//#endif /* defined(CRYS_NO_CONTEXT_ENCRYPTION_PROTECTION) || defined(CRYS_NO_AES_SUPPORT) */
