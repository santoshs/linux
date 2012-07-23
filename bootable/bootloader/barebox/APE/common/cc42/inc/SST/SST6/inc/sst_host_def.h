/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 /*! \file 
**********************************************************************************
* Title:                            Discretix SST API header file
*
* Filename:                         sst_api_def.h
*
* Project, Target, subsystem:       SST 6.0, API
* 
* Created:                          01.07.2007
*
* Modified:
* \Author                           Ira Boguslavsky
*
* \Remarks                        
**********************************************************************************/
#ifndef _SST_HOST_API_DEF_H_
#define _SST_HOST_API_DEF_H_

/*defines maximum size of authentication password on the SST on SEP*/
#define	SST_AUTH_PWD_MAX_SIZE_IN_BYTES						(2048UL)

/*defines authentication RSA public key modulus size*/
#define	SST_AUTH_PK_MODULU_SIZE_IN_BYTES					(256UL)

/*defines authentication AES size*/
#define	SST_AUTH_AES_KEY_SIZE_IN_BYTES						(32UL)


/*defines the overhead size of OUT parameters in buffer including op code*/
#define SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			(4UL)

/*defines the overhead size of IN parameters in buffer including op code and return code*/
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES	(8UL)

/*defines MAX size of AES key for Key management operation*/
#define SST_HOST_KM_AES_MAX_SIZE_IN_BYTES							(32UL)

/*defines MAX size of DES key for Key management operation*/
#define SST_HOST_KM_DES_MAX_SIZE_IN_BYTES							(24UL)

/*MACRO for checking return code form driver*/
#define SST_HOST_DRIVER_ERROR_CHECK(errorRC,exitMark)				\
		if (DX_SUCCESS != errorRC)									\
		{	errorRC = SST_RC_ERROR_SEP;								\
			goto exitMark;}

/*MACRO for checking return code form SST driver*/
#define SST_HOST_RETURN_CODE_CHECK(sstRC,exitMark)					\
		if ((SST_RC_OK != sstRC) && (SST_RC_ERROR_PARTIALLY_COMPLETED != sstRC ))\
			{	goto exitMark;}

/************************************************************************/
/*		parameters buffer size per SST operation						*/
/************************************************************************/

/*Init-Terminate*/
#define SST_HOST_IF_API_PARAM_BUFF_INIT_SIZE_IN_BYTES						\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ (2*sizeof(DxNvsMemoryId)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_INIT_SIZE_IN_BYTES				\
		SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES

#define SST_HOST_IF_API_PARAM_BUFF_TERMINATE_SIZE_IN_BYTES					\
		SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_TERMINATE_SIZE_IN_BYTES			\
		SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES

/*Version get*/
#define SST_HOST_IF_API_PARAM_BUFF_VERSION_GET_SIZE_IN_BYTES				\
		SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_VERSION_GET_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTVersion_t))

/*Authentication*/
#define SST_HOST_IF_API_PARAM_BUFF_AUTH_PWD_CREATE_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_AUTH_PWD_CREATE_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

#define SST_HOST_IF_API_PARAM_BUFF_AUTH_PUB_KEY_RSA_CREATE_SIZE_IN_BYTES	\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + (2*sizeof(DxByte_t*))						\
		+ sizeof(DxUint32_t) + sizeof(SSTHandle_t))
		
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_AUTH_PUB_KEY_RSA_CREATE_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTHandle_t))

/*continue...*/

/*Data Operations*/

/*Data Iterators*/

/*Key management*/
/*SST_AESKeyInsert*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_AES_INSERT_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_AES_INSERT_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_AESKeyGenerate*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_AES_GEN_SIZE_IN_BYTES					\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_AES_GEN_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_AESKeyRead*/	
#define SST_HOST_IF_API_PARAM_BUFF_KM_AES_READ_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTSessionId_t) + sizeof(SSTHandle_t)						\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_AES_READ_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(DxUint32_t)) 

/*SST_DESKeyInsert*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_DES_INSERT_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_DES_INSERT_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_DESKeyGenerate*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_DES_GEN_SIZE_IN_BYTES					\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_DES_GEN_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_DESKeyRead*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_DES_READ_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTSessionId_t) + sizeof(SSTHandle_t)						\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_DES_READ_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(DxUint32_t)) 

/*SST_HMACKeyInsert*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_HMAC_INSERT_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_HMAC_INSERT_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_HMACKeyGenerate*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_HMAC_GEN_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (2*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_HMAC_GEN_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_HMACKeyRead*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_HMAC_READ_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTSessionId_t) + sizeof(SSTHandle_t)						\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_HMAC_READ_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(DxUint32_t)) 

/*SST_RSAKeyInsert*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_RSA_INSERT_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (5*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ (3*sizeof(DxByte_t*)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_RSA_INSERT_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_RSAKeyGenerate*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_RSA_GEN_SIZE_IN_BYTES					\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (3*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_RSA_GEN_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_RSAKeyRead*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_RSA_READ_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTSessionId_t) + sizeof(SSTHandle_t)						\
		+ (3*sizeof(DxByte_t*)) + (4*sizeof(DxUint32_t)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_RSA_READ_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ (3*sizeof(DxUint32_t)))

/*SST_CRTRSAKeyInsert*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_CRTRSA_INSERT_SIZE_IN_BYTES			\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (9*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ (7*sizeof(DxByte_t*)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_CRTRSA_INSERT_SIZE_IN_BYTES	\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_CRTRSAKeyGenerate*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_CRTRSA_GEN_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTTxnId_t) + sizeof(SSTSessionId_t)						\
		+ (3*sizeof(DxUint32_t)) + (2*sizeof(SSTHandle_t))					\
		+ sizeof(DxByte_t*))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_CRTRSA_GEN_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ sizeof(SSTHandle_t))

/*SST_CRTRSAKeyRead*/
#define SST_HOST_IF_API_PARAM_BUFF_KM_CRTRSA_READ_SIZE_IN_BYTES				\
		(SST_HOST_IF_API_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES					\
		+ sizeof(SSTSessionId_t) + sizeof(SSTHandle_t)						\
		+ (7*sizeof(DxByte_t*)) + (8*sizeof(DxUint32_t)))
#define SST_HOST_IF_API_RESULT_PARAM_BUFF_KM_CRTRSA_READ_SIZE_IN_BYTES		\
		(SST_HOST_IF_API_RESULT_PARAM_BUFF_OVERHEAD_SIZE_IN_BYTES			\
		+ (7*sizeof(DxUint32_t)))
/************************************************************************/
/* ?? check these definitions                                            */
/************************************************************************/
#define SST_MAX_BUFFER_SIZE                      (0x3000UL)

#define SST_VERSION_MAX_PARAM_LEN_IN_BYTES       (96UL)

#define SST_AES_KEY_LENGTH_IN_WORDS              (16UL)

#define SST_ONE_WORD_IN_BYTES                    (4UL)

/* The maximum size of the AES KEY in words and bytes */
#define CRYS_AES_KEY_MAX_SIZE_IN_WORDS 8
#define CRYS_AES_KEY_MAX_SIZE_IN_BYTES (CRYS_AES_KEY_MAX_SIZE_IN_WORDS * sizeof(DxUint32_t))

/* The key size in words on the DES machine */
#define CRYS_DES_KEY_SIZE_IN_WORDS 2
#define CRYS_DES_KEY_SIZE_IN_BYTES ( CRYS_DES_KEY_SIZE_IN_WORDS * sizeof(DxUint32_t) )

/*! brief  the size of 1DES key in bytes**/
#define SST_VCRYS_1DES_KEY_SIZE_IN_BYTES		CRYS_DES_KEY_SIZE_IN_BYTES

/*! brief  the size of 3DES key in bytes**/
#define SST_VCRYS_3DES_KEY_SIZE_IN_BYTES		(CRYS_DES_KEY_SIZE_IN_BYTES*3)	

/* The HMAC key size after padding for MD5, SHA1, SHA256 */
#define CRYS_HMAC_KEY_SIZE_IN_BYTES 64

/*! brief  the maximum size of HMAC key in bytes**/
#define SST_VCRYS_HMAC_512_KEY_SIZE_IN_BYTES	CRYS_HMAC_KEY_SIZE_IN_BYTES

/* The HMAC key size after padding for SHA384, SHA512 */
#define CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES 128

/*! brief  the maximum size of HMAC key in bytes**/
#define SST_VCRYS_HMAC_1024_KEY_SIZE_IN_BYTES	CRYS_HMAC_SHA2_1024BIT_KEY_SIZE_IN_BYTES


/************************************************************************/
/************************************************************************/
/************************************************************************/

#endif
