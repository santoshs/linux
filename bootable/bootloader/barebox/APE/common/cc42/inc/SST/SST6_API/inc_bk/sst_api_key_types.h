/*! \file
**********************************************************************************
* Title:						Discretix SST API Key Type Definitions Header File
*
* Filename:						sst_api_key_types.h
*
* Project, Target, subsystem:	SST 6.0, Stubs, API
*
* Created:						13.03.2007
*
* Modified:						07.06.2007
*
* \Author						Raviv Levi
*
* \Remarks
*           Copyright (C) 2006 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/

#ifndef _DX_SST_API_KEY_TYPES_H_
    #define _DX_SST_API_KEY_TYPES_H_

	#include "sst_vcrys_ext.h"

    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (sst_api_key_types.h)
    #else
        #if defined _DX_SST_API_NO_EXTERN_
            #define EXTERN
        #else
            #define EXTERN extern
        #endif /* _DX_SST_API_NO_EXTERN_*/
    #endif /*EXTERN*/

    /*----------- Local type definitions -----------------------------------------*/

	typedef struct
	{
		SSTAESKeyType_t type;
		DxByte_t		data[16];
	}SSTKeyAES128Bit_t;

	typedef struct
	{
		SSTAESKeyType_t type;
		DxByte_t		data[24];
	}SSTKeyAES192Bit_t;

	typedef struct
	{
		SSTAESKeyType_t type;
		DxByte_t		data[32];
	}SSTKeyAES256Bit_t;

	typedef struct
	{
		SSTAESKeyType_t type;
		DxByte_t		data[64];
	}SSTKeyAES512Bit_t;

	typedef struct
	{
		SSTDESKeyType_t type;
		DxByte_t		data[SST_VCRYS_1DES_KEY_SIZE_IN_BYTES];
	}SSTKey1DES_t;

	typedef struct
	{
		SSTDESKeyType_t type;
		DxByte_t		data[SST_VCRYS_2DES_KEY_SIZE_IN_BYTES];
	}SSTKey2DES_t;

	typedef struct
	{
		SSTDESKeyType_t type;
		DxByte_t		data[SST_VCRYS_3DES_KEY_SIZE_IN_BYTES];
	}SSTKey3DES_t;

	typedef struct
	{
		DxUint32_t		size;
		DxByte_t		data[SST_VCRYS_HMAC_512_KEY_SIZE_IN_BYTES];
	}SSTKeyHMAC512Bit_t;

	typedef struct
	{
		DxUint32_t		size;
		DxByte_t		data[SST_VCRYS_HMAC_1024_KEY_SIZE_IN_BYTES];
	}SSTKeyHMAC1024Bit_t;


	#define SST_KEY_AES_MAX_BUFFER_SIZE_IN_BYTES	SST_ALIGN_TO_WORD(sizeof(SSTKeyAES512Bit_t))
	#define SST_KEY_DES_MAX_BUFFER_SIZE_IN_BYTES	SST_ALIGN_TO_WORD(sizeof(SSTKey3DES_t))
	#define SST_KEY_HMAC_MAX_BUFFER_SIZE_IN_BYTES	SST_ALIGN_TO_WORD(sizeof(SSTKeyHMAC1024Bit_t))
	#define SST_KEY_RSA_MAX_BUFFER_SIZE_IN_BYTES	SST_ALIGN_TO_WORD(sizeof(SSTKeyRSA_t))



	/*----------- Package global variables --------------------------------------*/
	#if defined SST_API_DEBUG_MODE_ENABLED
		//EXTERN int _gParam;
	#endif


#undef EXTERN

#endif  /* _DX_SST_API_KEY_TYPES_H_ */
