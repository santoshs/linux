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
 
 #ifndef __KMNG_DEFS_H__
#define __KMNG_DEFS_H__


/*
   *  Object %name    : SEPDriver.h
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief  Key Management h file (API and structures )
   *
   *  \version 
   *  \author yevgenys
   */
#include "CRYS_RSA_Types.h"
#include "CRYS_AES.h"

/*------------------------------
         DEFINES
--------------------------------*/

/**********************************************************/
/* Internal encrypted KeyData  - size information         */ 
/**********************************************************/
#define KMNG_CONFIG_INFO_SIZE_IN_BYTES				24 
#define KMNG_KEY_INDEX_SIZE_IN_BYTES				4
#define KMNG_HASH_PSW_SIZE_IN_BYTES					32
#define KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES			CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES  /* 8 */

/* Maximum keys sizes
 *********************/
#define KMNG_SYM_MAX_KEY_SIZE_IN_BYTES				64
#define KMNG_DES_MAX_KEY_SIZE_IN_BYTES				24
#define KMNG_AES_MAX_KEY_SIZE_IN_BYTES				32
#define KMNG_HMAC_MAX_KEY_SIZE_IN_BYTES				64

/* Must be the sizeof(CRYS_RSAUserPubKey_t) */
#define KMNG_RSA_PUB_STRUCT_SIZE_IN_BYTES			sizeof(CRYS_RSAUserPubKey_t)
/* Must be the sizeof(CRYS_RSAUserPrivKey_t) */
#define KMNG_RSA_PRV_STRUCT_SIZE_IN_BYTES			sizeof(CRYS_RSAUserPrivKey_t)

#define KMNG_RSA_MAX_KEY_PAIR_SIZE_IN_BYTES		  ( KMNG_RSA_PUB_STRUCT_SIZE_IN_BYTES + \
                                                     KMNG_RSA_PRV_STRUCT_SIZE_IN_BYTES )

/* DH key max size in bytes  = 1040 = ModSize+GenSize+QSize+Esize+Dsize+Seedsize+ 6*sizeOfWord = 5*256+20 + 6*4  */
#define KMNG_DH_MAX_MODULUS_SIZE_IN_BYTES			256
#define KMNG_DH_MAX_KEY_SIZE_IN_BYTES			    (5*KMNG_DH_MAX_MODULUS_SIZE_IN_BYTES + 20 + 6*4)

/* Specific user key data size */
#define KMNG_USER_SPECIFIC_KEY_DATA_SIZE             8
#define KMNG_USER_SPECIFIC_KEY_DATA_SIZE_IN_WORDS    (KMNG_USER_SPECIFIC_KEY_DATA_SIZE / 4)

/* maximum length of password for a key in the key ring */
#define KMNG_KEY_PASSWORD_MAX_LENGTH_IN_BYTES   128   


/*******************************************************************/
/*     Key data sizes definitions used in HOST                     */
/*******************************************************************/

/* ------------- Overhead for internal use = 60 bytes = 24 + 4 + 32  --------- */
#define KMNG_KEY_OVERHEAD_SIZE_IN_BYTES		(  KMNG_CONFIG_INFO_SIZE_IN_BYTES  \
										     + KMNG_KEY_INDEX_SIZE_IN_BYTES  \
										     + KMNG_HASH_PSW_SIZE_IN_BYTES )

/*---------------  Ring key size (already rounded)  ---------------------------*/
#define KMNG_WRAPPED_RING_KEY_SIZE_IN_BYTES		   (  CRYS_AES_KEY_MAX_SIZE_IN_BYTES \
													+ KMNG_KEY_OVERHEAD_SIZE_IN_BYTES \
													+ KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES )


/* ----------------- Symmetric keys data sizes -------------------*/

/* decrypted key data size, NOT rounded  */
#define KMNG_SYM_KEY_DATA_SIZE_IN_BYTES		  ( KMNG_SYM_MAX_KEY_SIZE_IN_BYTES + \
                                                KMNG_KEY_OVERHEAD_SIZE_IN_BYTES )

/* encrypted key buffer size: added 1 AES_WRAP_BLOCK for WRAP and 1 AES_WRAP_BLOCK 
  for rounding to AES_WRAP_BLOCK in run time */
#define KMNG_SYM_ENCR_KEY_BUFF_SIZE_IN_BYTES   ( KMNG_SYM_KEY_DATA_SIZE_IN_BYTES + \
                                                 2*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES )


/* ----------------- RSA key data sizes --------------------------*/

/* decrypted key data size, NOT rounded  */
#define KMNG_RSA_KEY_DATA_SIZE_IN_BYTES  	   ( KMNG_RSA_MAX_KEY_PAIR_SIZE_IN_BYTES + \
                                                 KMNG_KEY_OVERHEAD_SIZE_IN_BYTES )

/* encrypted key buffer size: added 1 AES_WRAP_BLOCK for WRAP and 1 AES_WRAP_BLOCK 
  for rounding to AES_WRAP_BLOCK in run time */
#define KMNG_RSA_ENCR_KEY_BUFF_SIZE_IN_BYTES	( KMNG_RSA_KEY_DATA_SIZE_IN_BYTES + \
                                                  2*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES)


/* ----------------- DH key data sizes ----------------------------*/

/* decrypted key data size, NOT rounded  */
#define KMNG_DH_KEY_DATA_SIZE_IN_BYTES		( KMNG_DH_MAX_KEY_SIZE_IN_BYTES + \
                                              KMNG_KEY_OVERHEAD_SIZE_IN_BYTES )

/* encrypted key buffer size: added 1 AES_WRAP_BLOCK for WRAP and 1 AES_WRAP_BLOCK 
  for rounding to AES_WRAP_BLOCK in run time */
#define KMNG_DH_ENCR_KEY_BUFF_SIZE_IN_BYTES   ( KMNG_DH_KEY_DATA_SIZE_IN_BYTES + \
                                                2*CRYS_AES_WRAP_BLOCK_SIZE_IN_BYTES )



/*******************************************************************/
/*     Key data sizes definitions used in SEP                      */
/*******************************************************************/
 
/*--------------------  AES Key  ----------------------------------*/													
/* length of the aes wrapped key = 64 bytes = 32+24+8+0 */
/* The size of the AES context for Active functions:
	size of the AES max key = 32 bytes
	24 bytes for the configuration information
	16 bytes for wrap AES and rounding */
#define KMNG_AES_WRAP_KEY_LEN              (KMNG_AES_MAX_KEY_SIZE_IN_BYTES + \
											KMNG_CONFIG_INFO_SIZE_IN_BYTES + \
											2*KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES)
 

/*--------------------  DES Key  ----------------------------------*/
/* length of the des wrapped key = 56 bytes = 24+24+8+0 */
/* The size of the AES context for Active functions:
	size of the DES max key = 24 bytes
	24 bytes for the configuration information
	16 bytes for wrap AES and rounding */
#define KMNG_DES_WRAP_KEY_LEN              (KMNG_DES_MAX_KEY_SIZE_IN_BYTES + \
											KMNG_CONFIG_INFO_SIZE_IN_BYTES + \
											2*KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES)


/*-------------------- HMAC Key  ----------------------------------*/
/* length of the hmac context == 96 bytes = 64+24+8+8 */
/* The size of the HMAC context for Active functions:
	size of the HMAC key = 64 bytes
	24 bytes for the configuration information
	16 bytes for wrap AES and rounding */
#define KMNG_HMAC_WRAP_KEY_LEN                 (KMNG_HMAC_MAX_KEY_SIZE_IN_BYTES + \
												KMNG_CONFIG_INFO_SIZE_IN_BYTES + \
												2*KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES)


/*--------------------  RSA Key  ----------------------------------*/
/* length of the rsa context = 2264 = 2232+24+8 */
/* The size of the RSA context for Active functions:
	size of the RSA keys: pub:544 + priv:1688 = 2232 bytes
	24 bytes for the configuration information
	16 bytes for wrap AES and rounding */
#define KMNG_RSA_WRAP_KEY_LEN                  (KMNG_RSA_PRV_STRUCT_SIZE_IN_BYTES + \
												KMNG_CONFIG_INFO_SIZE_IN_BYTES + \
												2*KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES)
												

/*--------------------  DH Key  ----------------------------------*/
/* length of the unpadded DH max key buffer = 1072 = 1040 + 24 + 8 */
/* The size of the DH context for Active functions:
	size of the DH key: 2*(mod size + 4) = 2*(256 + 4) = 520 bytes
	24 bytes for the configuration information
	16 bytes for wrap AES and rounding */
#define KMNG_DH_WRAP_KEY_LEN                   ( 2*KMNG_DH_MAX_MODULUS_SIZE_IN_BYTES + 8 + \
												KMNG_CONFIG_INFO_SIZE_IN_BYTES + \
												2*KMNG_AES_WRAP_BLOCK_SIZE_IN_BYTES)



/*******************************************************************/
/*    Different key data buffers for key activate and CRYS_KMNG    */
/*******************************************************************/         

/* Defines the AES Wrap key buffer */
typedef DxUint8_t KMNG_AES_WrappedKey_t[KMNG_AES_WRAP_KEY_LEN];
/* Defines the DES Wrap key buffer */
typedef DxUint8_t KMNG_DES_WrappedKey_t[KMNG_DES_WRAP_KEY_LEN];
/* Defines the HMAC Wrap key buffer */
typedef DxUint8_t KMNG_HMAC_WrappedKey_t[KMNG_HMAC_WRAP_KEY_LEN];
/* Defines the RSA Wrap key buffer */
typedef DxUint8_t KMNG_RSA_WrappedKey_t[KMNG_RSA_WRAP_KEY_LEN];
/* Defines the DH Wrap key buffer */
typedef DxUint8_t KMNG_DH_WrappedKey_t[KMNG_DH_WRAP_KEY_LEN];   

/* Defines the user specific key data buffer */
typedef DxUint32_t KMNG_UserSpecificKeyData_t[KMNG_USER_SPECIFIC_KEY_DATA_SIZE_IN_WORDS];

/*--------------------  Predefined values  --------------------------*/
/* This define is for Activate function - Global Secret key is 128 bits */
#define KMNG_GLOBAL_SECRET_KEY_FOR_AES_SIZE			CRYS_AES_Key128BitSize

#define KMNG_EXTRA_BYTES_FOR_AES_MAC 8

/* Defines for values of key usage parameter */
#define KMNG_KEY_USAGE_SIGNING       1
#define KMNG_KEY_USAGE_STORAGE       2
#define KMNG_KEY_USAGE_WRAPPING      4
#define KMNG_KEY_USAGE_HMAC_VERIFY   8
#define KMNG_KEY_USAGE_LEGACY       15    /* all usages are permitted */
#define KMNG_KEY_USAGE_ODRM        (1UL<<31)

/* Defines for key restriction parameter values  */
#define KMNG_KEY_RESTRICTION_EXPORTABLE  1
#define KMNG_KEY_RESTRICTION_NON_EXPORTABLE  0
 
/* Valid Token for Key Ring */
#define KMNG_VALID_KEY_RING_TOKEN 0x12345678

/* The offset where to copy the key - after the configuration information */
#define KMNG_LLF_CONTEXT_KEY_OFFSET_BYTES				KMNG_CONFIG_INFO_SIZE_IN_BYTES

/*-------------------------------
           STRUCTURES
---------------------------------*/
/* enum for deciding the operation on the RSA key */
typedef enum
{
  KMNG_RsaKeyOperationEncrypt,
  KMNG_RsaKeyOperationDecrypt,

  KMNG_RsaKeyOperationTypeLast       = 0x7FFFFFFF,

}KMNG_RsaKeyOperationType_t;

/* enum of the key type */
typedef enum
{
  KMNG_KeyTypeAES = 0,
  KMNG_KeyTypeDES = 1,
  KMNG_KeyTypeHMAC = 3,
  KMNG_KeyTypeRSAPriv = 4,
  KMNG_KeyTypeRSAPub  = 5,
  KMNG_KeyTypeRSAPair = 6,
  KMNG_KeyTypeDH = 7,  

  KMNG_KeyTypeNumOfOptions,

  KMNG_KeyTypeLast= 0x7FFFFFFF,

}KMNG_KeyType_t;



/* enum of the key configuration */
typedef struct _KMNG_KeyConfiguration_t
{
  /* type */
  KMNG_KeyType_t  type;
  
  /* key size */
  DxUint32_t  keySizeBytes; 
  
  /* usage */
  DxUint32_t usage;    
  
  /* restriction */
  DxUint32_t restriction;
  
  /* user specific key data */
  KMNG_UserSpecificKeyData_t   UserSpecificKeyData;
  
}KMNG_KeyConfiguration_t;



/**********************************************/
/* structures defining the contexts for flows */
/**********************************************/

typedef struct _KMNG_UserKeyGeneralData_t
{
  /* valid flag */
  DxUint32_t                    validFlag;
  /* the actual length of the data in the keyData field */
  DxUint32_t                    keyDataLen;
}KMNG_UserKeyGeneralData_t;

/*--------------*/

typedef  struct _KMNG_SYMKeyData_t
{
  /* general data of the key */
  KMNG_UserKeyGeneralData_t     generalData;  
  /* data for encrypted key data */
  DxUint8_t                     encryptedKeyBuffer[KMNG_SYM_ENCR_KEY_BUFF_SIZE_IN_BYTES];
  
}KMNG_SYMKeyData_t;

/*--------------*/

typedef  struct _KMNG_RSAKeyData_t
{
  /* general data of the key */
  KMNG_UserKeyGeneralData_t     generalData;  
  /* data for encrypted key data */
  DxUint8_t                     encryptedKeyBuffer[KMNG_RSA_ENCR_KEY_BUFF_SIZE_IN_BYTES];
}KMNG_RSAKeyData_t;

/*--------------*/

typedef  struct _KMNG_DHKeyData_t
{
  /* general data of the key */
  KMNG_UserKeyGeneralData_t     generalData;  
  /* data for encrypted key data */
  DxUint8_t                     encryptedKeyBuffer[KMNG_DH_ENCR_KEY_BUFF_SIZE_IN_BYTES];
  
}KMNG_DHKeyData_t;

/***********************************************/


/*   Key ring structure */
typedef struct _KMNG_KeyRing_t
{
  /* The valid tag for valid Key Ring */
  DxUint32_t		   ValidTag;

  /* maximum number of the symmetric keys in the ring */
  DxUint32_t          maxSymKeysNum;
  
  /* maximum number of the RSA keys in the ring */
  DxUint32_t          maxRSAKeysNum;
  
  /* maximum number of the DH keys in the ring */
  DxUint32_t          maxDHKeysNum;
  
  /* number of symmetric keys currently in the key ring */
  DxUint32_t          numActiveSymKeys;
  
  /* number of RSA keys currently in the key ring */
  DxUint32_t          numActiveRSAKeys;
  
  /* number of DH keys currently in the key ring */
  DxUint32_t          numActiveDHKeys;
  
  /* protection key of the key ring */
  KMNG_SYMKeyData_t   protectionKey;
  
  /* holds the start address where the array of symmetric keys begins */
  DxUint32_t          startSymKeysAddr;
  
  /* holds the start address where the array of RSA keys begins */
  DxUint32_t          startRSAKeysAddr;
  
  /* holds the start address where the array of DH keys begins */
  DxUint32_t          startDHKeysAddr;
  
  
}KMNG_KeyRing_t;


/* the version structure definition */
typedef struct
{
   char compName[4];
   char type;
   DxUint32_t major;
   DxUint32_t minor;
   DxUint32_t sub;
   DxUint32_t internal;   

}KMNG_ComponentVersion_t;

typedef struct
{
   KMNG_ComponentVersion_t KMNG_Version;
}KMNG_Version_t;

                            
 
#endif /* __KMNG_DEFS_H__ */
