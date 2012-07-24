/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

/************* Include Files ****************/
#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"

#ifndef   CRYS_NO_GLOBAL_DATA
#include "DX_VOS_Sem.h"
#else
#ifndef CRYS_SEP_SIDE_WORK_MODE
/* working from HOST side does not require any synchronization */
#define DX_VOS_SemWait(semId,timeout)	CRYS_OK
#define DX_VOS_SemGive(semId)
#else
/* SEP side needs a basic synchronization between the interrupts */
#include "sep_sync.h"
#define DX_VOS_SemWait(semId,timeout)	DX_SEP_SemWait()
#define DX_VOS_SemGive(semId)			DX_SEP_SemGive()
#endif /*CRYS_SEP_SIDE_WORK_MODE*/
#endif /*CRYS_NO_GLOBAL_DATA*/

#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
//#include "CRYS_EXT_IF_OTF.h"
#endif

#include "CRYS_AES_error.h"
#include "LLF_COMMON.h"
#include "LLF_AES.h"
#include "LLF_AES_error.h"
#include "LLF_AES_HwDefs.h"
#include "LLFCD_AES.h"




/************************ Defines ************************************/


/************************ Enums **************************************/

/* canceling the lint warning:
    Suspicious pointer-to-pointer conversion (area too small) */
/*lint --e{826} */

/************************** Typedefs *********************************/

/************************ Global Data ********************************/

/* importing the base address of the CRYPTO Cell . this value must be initialized in the
   CRYS_Init function */

extern DxUint32_t PLAT_CryptoCellBaseAddr;
extern DxUint32_t CRYS_COMMON_BusAlignerMask;
extern DxUint32_t CRYS_COMMON_BusAlignerShift;

#ifndef CRYS_NO_GLOBAL_DATA
/* importing the semaphore used for the access to the hardware machine */
extern DxVosSem SemHwAccessId;
#endif

/* special values used in XCBC and CMAC modes for derived keys calculation */
static const DxUint32_t InitValue01_01[] = {0x01010101, 0x01010101, 0x01010101, 0x01010101};
static const DxUint32_t InitValue02_02[] = {0x02020202, 0x02020202, 0x02020202, 0x02020202};
static const DxUint32_t InitValue03_03[] = {0x03030303, 0x03030303, 0x03030303, 0x03030303};


/***************************************************************************************/
/*********           Private and external function prototypes                 **********/
/***************************************************************************************/
CRYSError_t  LLF_AES_Cmac_Finish( AESContext_t   *ContextID_ptr );
CRYSError_t  LLF_AES_Xcbc_Finish( AESContext_t   *ContextID_ptr );

/****************************************************************************************/
/****************************************************************************************/
/*                        Public Functions                                              */
/****************************************************************************************/
/****************************************************************************************/



/****************************************************************************************/
/**
 * @brief The low level LLF_AES_Xcbc_Init performs specific initializations required
 *        on the used HW or SW platform for necessary modes (currently only on XCBC is need).
 *
 *        On the lite platform currently this function does nothing.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_AES_Xcbc_Init( AESContext_t  *WorkingContext_ptr )
{
   /* LOCAL DECLARATIONS */

   CRYSError_t Error;


   /* FUNCTION LOGIC */

   Error = CRYS_OK;



   /* ------------------------------------------------------------------------- */
   /* .... Calculate derived keys K1, K2, K3 and set they into AES context .....*/
   /* ------------------------------------------------------------------------- */

    /* set temporary operation mode to ECB for permitting output of result
       from AES block operation */
    WorkingContext_ptr->OperationMode = CRYS_AES_ECB_mode ;

	/* Calculate derived key K1 and set they into AES context  */
    Error = LLF_AES_Block( WorkingContext_ptr,
                            (DxUint8_t*)InitValue01_01,
                            CRYS_AES_BLOCK_SIZE_IN_BYTES ,
                            (DxUint8_t*)(WorkingContext_ptr->SpecificData.XCBC_CMAC_Data.derivedKey1) );

    if( Error != CRYS_OK )
	return Error;

    /* calculate K2 */
    Error = LLF_AES_Block( WorkingContext_ptr,
                            (DxUint8_t*)InitValue02_02,
                            CRYS_AES_BLOCK_SIZE_IN_BYTES ,
                            (DxUint8_t*)(WorkingContext_ptr->SpecificData.XCBC_CMAC_Data.derivedKey2) );

    if( Error != CRYS_OK )
	return Error;

    /* calculate K3 */
    Error = LLF_AES_Block( WorkingContext_ptr,
                            (DxUint8_t*)InitValue03_03,
                            CRYS_AES_BLOCK_SIZE_IN_BYTES ,
                            (DxUint8_t*)(WorkingContext_ptr->SpecificData.XCBC_CMAC_Data.derivedKey3) );


    if( Error != CRYS_OK )
	return Error;

    /* set AESContextkey = K1 instead K */
    DX_VOS_FastMemCpy( (DxUint8_t*)(WorkingContext_ptr->AESContextKey),
                       (DxUint8_t*)(WorkingContext_ptr->SpecificData.XCBC_CMAC_Data.derivedKey1),
                        CRYS_AES_BLOCK_SIZE_IN_BYTES );

    /* restore operation mode */
    WorkingContext_ptr->OperationMode = CRYS_AES_XCBC_MAC_mode;


   /* .................... end of function .................................... */
   /* ------------------------------------------------------------------------- */

   return Error;

}/* END OF LLF_AES_Xcbc_Init */


/*********************************************************************************************************/
/**
* @brief The LLF_AES_Cmac_Finish function finishes the AES operation on XCBC and CMAC modes.
*
*      The algorithm is following:
*      1.	Derive from AES-context saved data: M[n], SizeOfLastBlock, E[n-1], K, K1, K2.
*          Initializes variables and arrays according to passed operation mode, in particular
*          sets  DataInSize = 1.
*      2.	If operation mode = AES_XCBC_MAC, then:
*              Sets pointers to operation keys Kop = K1, K1op = K2, K2op  = K3,
*          If operation mode = AES_CMAC, then:
*              Sets pointers to operation keys Kop = K, K1op = K1, K2op = K2.
*      3.	If(SizeOfLastBlock = 128 bits) then:   M[n] = M[n] XOR K1op.
*      4.	If(SizeOfLastBlock < 128 bits), then:
*              a)  Pad M[n] with a single "1" bit, followed by the number of "0" bits required
*                  to  increase M[n]'s block size to 128 bits.
*              b)  M[n] = M[n] XOR K2op.
*      5.	Calls  LLF_AES_CBC_encrypt function for encrypting M[n] with key Kop, yielding  E[n].
*      6.   Output the authenticator value  - 128  bits of E[n].
*      7.   Exits.
*
* @param[in] ContextID_ptr - a pointer to the AES context buffer allocated by the user, that
*                            should be the same context that was used on the previous call
*                            of this session.
* @return CRYSError_t      - On success CRYS_OK is returned, on failure a
*                            value MODULE_* CRYS_AES_error.h
*/

//CRYSError_t  LLF_AES_Cmac_Finish( AESContext_t   *ContextID_ptr )
//{
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
//	CRYSError_t Error;

	/* local variables and pointers */
/*	DxUint32_t  SizeOfLastBlock = ContextID_ptr->remainSize;

	if(ContextID_ptr->NotAddBlocks != 0xFFFFFFFF && SizeOfLastBlock == 0)
	  return CRYS_AES_DATA_IN_SIZE_ILLEGAL;
*/
	/* set last block flag to context */
  //  ContextID_ptr->NotAddBlocks = 1;



	/* call the AES_Block operation with K2op_ptr as DataIn and K1op_ptr as DataOut pointers */
/*	Error = LLF_AES_Block( ContextID_ptr,
	                       (DxUint8_t*)&ContextID_ptr->lastBlock,
	                       SizeOfLastBlock,
		                   (DxUint8_t*)&ContextID_ptr->lastBlock);


*/	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

//	return Error;

//}/* END OF LLF_AES_Cmac_Finish */




//CRYSError_t  LLF_AES_Xcbc_Finish( AESContext_t   *ContextID_ptr )
//{
	/* FUNCTION DECLARATIONS */

	/* The return error identifier */
//	CRYSError_t Error;

//	DxInt32_t  i;

	/* local variables and pointers */
//	DxUint32_t  SizeOfLastBlock;

//	DxUint32_t  *K1op_ptr, *K2op_ptr; /* pointers to keys */

//	DxUint8_t   *LastBlock_ptr; /* pointer to last block data XORed with previous IV */

	/******************** FUNCTION LOGIC ************************************/

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
//	Error = CRYS_OK;

	/* setting pointers to appropriate context buffers and calculating expanded key on XCBC modes*/

//	K1op_ptr = ContextID_ptr->SpecificData.XCBC_CMAC_Data.derivedKey2;
//	K2op_ptr = ContextID_ptr->SpecificData.XCBC_CMAC_Data.derivedKey3;
	/* setting pointer to last block */
//	LastBlock_ptr = (DxUint8_t*)(ContextID_ptr->lastBlock);

	/* last block size */
//	SizeOfLastBlock = ContextID_ptr->lastBlockSize;


//	if( SizeOfLastBlock == CRYS_AES_BLOCK_SIZE_IN_BYTES )
//	{
		/* perform XOR: M[n] = M[n] XOR K1op and output temporary the result in K2op_ptr buffer */
/*		for( i = 0; i < CRYS_AES_BLOCK_SIZE_IN_WORDS; i++ )

			((DxUint32_t*)K2op_ptr)[i] = (((DxUint32_t*)LastBlock_ptr)[i]) ^ (K1op_ptr[i]);
	}

	if( SizeOfLastBlock < CRYS_AES_BLOCK_SIZE_IN_BYTES )
	{
*/		/* pad the block with one bit 1 and other bits 0. Note: because all remaining bytes
		of the block are already 0, we need set only one bit  */
//		LastBlock_ptr[SizeOfLastBlock] = 0x80;

		/* perform XOR: M[n] = M[n] XOR K2op and output temporary the result in K2op_ptr buffer */
/*		for( i = 0; i < CRYS_AES_BLOCK_SIZE_IN_WORDS; i++ )

			((DxUint32_t*)K2op_ptr)[i] = (((DxUint32_t*)LastBlock_ptr)[i]) ^ (K2op_ptr[i]);
	}
*/
	/* set temporary context operation mode to MAC  for permitting output of result from AES block operation */
//	ContextID_ptr->OperationMode = CRYS_AES_MAC_mode ;

	/* set remain size in the context */
//	ContextID_ptr->remainSize = CRYS_AES_BLOCK_SIZE_IN_BYTES;

	/* call the AES_Block operation with K2op_ptr as DataIn and K1op_ptr as DataOut pointers */
/*	Error = LLF_AES_Block( ContextID_ptr,
	                       (DxUint8_t*)K2op_ptr,
	                       CRYS_AES_BLOCK_SIZE_IN_BYTES,
		                   (DxUint8_t*)K1op_ptr );


*/	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

//	return Error;

//}/* END OF LLF_AES_Xcbc_Finish */



//CRYSError_t  LLF_AES_XcbcCmac_Finish( AESContext_t   *ContextID_ptr )
//{
	/* FUNCTION DECLARATIONS */
/*
	if(ContextID_ptr->OperationMode == CRYS_AES_XCBC_MAC_mode)
	   return LLF_AES_Xcbc_Finish(ContextID_ptr);
	else
	   return LLF_AES_Cmac_Finish(ContextID_ptr );
}*//* END OF LLF_AES_XcbcCmac_Finish */
