/*! \file 
**********************************************************************************	
* Title:						Discretix SST VCRYS Type Definitions Header File	 					
*																			
* Filename:						sst_vcrys_types.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual Cryptography Services
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_VCRYS_TYPES_H_
    #define _DX_SST_VCRYS_TYPES_H_

	#include "CRYS.h"
	#include "sst_vcrys_def.h"

    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (vcrys_types.h) 
    #else
        #if defined _DX_SST_VCRYS_NO_EXTERN_
            #define EXTERN 
        #else
            #define EXTERN extern
        #endif /* _DX_SST_VCRYS_NO_EXTERN_*/
    #endif /*EXTERN*/

    /*----------- Local type definitions -----------------------------------------*/
	/*Module MAC variables*/
	typedef struct  
	{
		CRYS_AESUserContext_t	contextID;
	}SSTVCRYSAESOperation_t;


	/*Module Internal Key variables*/
	typedef struct  
	{
		DxUint16_t	length;
		DxByte_t	key_ptr[SST_VCRYS_SK_MAX_SIZE_IN_BYTES];
	}SSTVCRYSInternalKey_t;

	/*VCRYS permanent runtime variables*/
	typedef struct  
	{			
		DxBool_t	isActive;
		
		SSTVCRYSInternalKey_t 	integrityKey;
		SSTVCRYSInternalKey_t 	encryptKey;
		DxBool_t				inOpEnc;
		DxBool_t				inOpDec;
		DxBool_t				inOpMac;
#ifdef DX_SST_DEBUG_REPRODUCE
		int						seed;
#endif
	}SSTVCRYSPermanentRuntimeVar_t;
	
	/*VCRYS non permanent runtime  variables*/
	typedef struct  
	{			
		SSTVCRYSAESOperation_t encryptOperation;
		SSTVCRYSAESOperation_t decryptOperation;
		SSTVCRYSAESOperation_t calcMACOperation;
	}SSTVCRYSNonPermanentRuntimeVar_t;

	/*----------- Package global variables --------------------------------------*/

#undef EXTERN

#endif  /* _DX_SST_VCRYS_TYPES_H_ */
