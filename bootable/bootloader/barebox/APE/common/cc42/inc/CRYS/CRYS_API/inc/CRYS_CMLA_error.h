/***********************************************************************************  
 * Copyright 2009  Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
   /*
   *  Object % CRYS_CMLA_error.h    : %
   *  State           :  %state%
   *  Creation date   :  28 sept 2006
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This module containes the definitions of the CRYS CMLA errors.
   *
   *  \version CRYS_CMLA_error.h#1:incl:1
   *  \author R.Levin
   */


#ifndef CRYS_CMLA_ERROR_H
#define CRYS_CMLA_ERROR_H

#include "CRYS_error.h"

#ifdef __cplusplus
extern "C"
{
#endif




/************************ Defines ******************************/

/* The CRYS CMLA module errors */
#define CRYS_CMLA_IS_NOT_SUPPORTED                            (CRYS_CMLA_MODULE_ERROR_BASE + 0x00UL)

/* The CRYS CMLA KDF function errors */
#define CRYS_CMLA_KDF_INVALID_SEED_PTR_ERROR                  (CRYS_CMLA_MODULE_ERROR_BASE + 0x01UL)
#define CRYS_CMLA_KDF_INVALID_KEK_PTR_ERROR                   (CRYS_CMLA_MODULE_ERROR_BASE + 0x02UL)
#define CRYS_CMLA_KDF_INVALID_TEMP_BUFF_PTR_ERROR             (CRYS_CMLA_MODULE_ERROR_BASE + 0x03UL)

/* The CRYS CMLA Wrap and Unwrap functions errors */
#define CRYS_CMLA_WRAP_ILLEGAL_KEK_PTR_ERROR                  (CRYS_CMLA_MODULE_ERROR_BASE + 0x10UL)
#define CRYS_CMLA_WRAP_ILLEGAL_KEY_PTR_ERROR                  (CRYS_CMLA_MODULE_ERROR_BASE + 0x11UL)
#define CRYS_CMLA_WRAP_ILLEGAL_WRAPPED_KEY_PTR_ERROR          (CRYS_CMLA_MODULE_ERROR_BASE + 0x12UL)

#define CRYS_CMLA_UNWRAP_ILLEGAL_KEK_PTR_ERROR                (CRYS_CMLA_MODULE_ERROR_BASE + 0x20UL)
#define CRYS_CMLA_UNWRAP_ILLEGAL_WRAPPED_KEY_PTR_ERROR        (CRYS_CMLA_MODULE_ERROR_BASE + 0x21UL)
#define CRYS_CMLA_UNWRAP_ILLEGAL_KEY_PTR_ERROR                (CRYS_CMLA_MODULE_ERROR_BASE + 0x22UL)

/* The CRYS CMLA RSA Encrypt and Decrypt functions errors */
#define CRYS_CMLA_RSA_ENCRYPT_ILLEGAL_PUB_KEY_PTR_ERROR       (CRYS_CMLA_MODULE_ERROR_BASE + 0x30UL)
#define CRYS_CMLA_RSA_ENCRYPT_ILLEGAL_MESSAGE_PTR_ERROR       (CRYS_CMLA_MODULE_ERROR_BASE + 0x31UL)
#define CRYS_CMLA_RSA_ENCRYPT_ILLEGAL_ENCR_MESSAGE_PTR_ERROR  (CRYS_CMLA_MODULE_ERROR_BASE + 0x32UL)
#define CRYS_CMLA_RSA_ENCRYPT_ILLEGAL_TEMP_BUFF_PTR_ERROR     (CRYS_CMLA_MODULE_ERROR_BASE + 0x32UL)
#define CRYS_CMLA_RSA_ENCRYPT_MESSAGE_INTEGER_TOO_LONG_ERROR  (CRYS_CMLA_MODULE_ERROR_BASE + 0x32UL)
#define CRYS_CMLA_RSA_ENCRYPT_ILLEGAL_MODULUS_SIZE_ERROR      (CRYS_CMLA_MODULE_ERROR_BASE + 0x33UL)

#define CRYS_CMLA_RSA_DECRYPT_ILLEGAL_PRIV_KEY_PTR_ERROR      (CRYS_CMLA_MODULE_ERROR_BASE + 0x40UL)
#define CRYS_CMLA_RSA_DECRYPT_ILLEGAL_ENCR_MESSAGE_PTR_ERROR  (CRYS_CMLA_MODULE_ERROR_BASE + 0x41UL)
#define CRYS_CMLA_RSA_DECRYPT_ILLEGAL_DECR_MESSAGE_PTR_ERROR  (CRYS_CMLA_MODULE_ERROR_BASE + 0x42UL)
#define CRYS_CMLA_RSA_DECRYPT_ILLEGAL_TEMP_BUFF_PTR_ERROR     (CRYS_CMLA_MODULE_ERROR_BASE + 0x42UL)
#define CRYS_CMLA_RSA_DECRYPT_DATA_INTEGER_TOO_LONG_ERROR     (CRYS_CMLA_MODULE_ERROR_BASE + 0x42UL)
#define CRYS_CMLA_RSA_DECRYPT_ILLEGAL_MODULUS_SIZE_ERROR      (CRYS_CMLA_MODULE_ERROR_BASE + 0x43UL)


/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/************************ Public Variables *********************/

/************************ Public Functions *********************/

#ifdef __cplusplus
}
#endif

#endif


