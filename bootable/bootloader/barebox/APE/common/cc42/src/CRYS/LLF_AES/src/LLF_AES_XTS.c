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

/* special value for GF(2^128) multiplication used in XTS algorithm: 0x87 = 135 */
#define CRYS_AES_XTS_GF_128_FDBK   0x87

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

/******************************************************************************************/
/*************           Private functions prototypes      ********************************/
/******************************************************************************************/

/******************************************************************************************/
/*************             External function prototypes             ***********************/
/******************************************************************************************/


extern void  LLF_AES_ReadOutputBlock(
		                    DxUint32_t           **dataOut32Bit_ptr,
		                    DxUint32_t            *dataSizeWords_ptr,
		                    DxUint32_t             VirtualHwBaseAddr );



/****************************************************************************************/
/****************************************************************************************/
/*                        Public Functions                                              */
/****************************************************************************************/
/****************************************************************************************/


/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_Xts_Init initializes the specific low level initializations required
 *        on the specific platform.
 *
 *        On the lite platform currently this function does nothing.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_Xts_Init( AESContext_t  *WorkingContextID_ptr )
{
   /* LOCAL DECLARATIONS */

   CRYSError_t                Error;
   CRYS_AES_KeySize_t         KeySizeID;
   CRYS_AES_EncryptMode_t     EncryptDecryptFlag;

   /* FUNCTION LOGIC */

   Error = CRYS_OK;

   PLAT_LOG_DEV_PRINT(( 5 , " Start LLF_AES_XTS_InitInit: \n " ));

   /* save keyID and encrypt flag */
   KeySizeID = WorkingContextID_ptr->AESContextKeySize;
   EncryptDecryptFlag = WorkingContextID_ptr->EncryptDecryptFlag;

   /* set temporary ECB encrypt operation mode  */
   WorkingContextID_ptr->OperationMode = CRYS_AES_ECB_mode;
   WorkingContextID_ptr->EncryptDecryptFlag = CRYS_AES_Encrypt;

   /* set key size according to next AES-ECB operation */
   if( WorkingContextID_ptr->AESContextKeySize == CRYS_AES_Key256BitSize )
	    WorkingContextID_ptr->AESContextKeySize = CRYS_AES_Key128BitSize;
   else
		WorkingContextID_ptr->AESContextKeySize = CRYS_AES_Key256BitSize;


   /* encrypt the Tweak value and save it in the same IV buffer */
   Error = LLF_AES_Block( WorkingContextID_ptr,
					     (DxUint8_t *)(WorkingContextID_ptr->AESContextIV),  /* input: Tweak = sector number */
					      CRYS_AES_BLOCK_SIZE_IN_BYTES ,
					     (DxUint8_t*)(WorkingContextID_ptr->AESContextIV) ); /* output: encrypted Tweak */

   /* reset operation mode and keySizeID*/
   WorkingContextID_ptr->OperationMode = CRYS_AES_XTS_mode;
   WorkingContextID_ptr->AESContextKeySize = KeySizeID;
   WorkingContextID_ptr->EncryptDecryptFlag = EncryptDecryptFlag;

   /* set Data_Key into AESContextKey buffer */
   DX_VOS_FastMemCpy( WorkingContextID_ptr->AESContextKey, WorkingContextID_ptr->SpecificData.XTS_Data.AES_XTS_Key2,
                      CRYS_AES_KEY_MAX_SIZE_IN_BYTES / 2 );

   /* clean Key2 buffer for security goals */
   DX_VOS_MemSetZero( WorkingContextID_ptr->SpecificData.XTS_Data.AES_XTS_Key2, CRYS_AES_KEY_MAX_SIZE_IN_BYTES / 2 );


   /* .................... end of function .................................... */
   /* ------------------------------------------------------------------------- */

   return Error;

}/* END OF LLF_AES_XTS_InitInit */



/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_ExecBlockXts processes specific operations for XTS
 *        Block operation.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  void  LLF_AES_ExecBlockXts(
                           DxUint32_t **dataIn32Bit_ptr,
                           DxUint32_t **dataOut32Bit_ptr,
                           DxUint32_t  *countWordsToLoad_ptr,
                           DxUint32_t  *countWordsToRead_ptr,
                           DxUint32_t   sizeNotMod4,
                           DxUint32_t   VirtualHwBaseAddr )
{
	/* LOCAL DECLARATIONS */

	DxUint32_t  isAesBusy;
	DxUint32_t  isDoutFifoEmpty;

	/* FUNCTION LOGIC */
	isAesBusy = 0;

	/* main loop of processing XTS */
	while( 1 )
	{
		if( isAesBusy == 0 )
		{
			if( *countWordsToLoad_ptr > CRYS_AES_BLOCK_SIZE_IN_WORDS ||
					(*countWordsToLoad_ptr == CRYS_AES_BLOCK_SIZE_IN_WORDS && sizeNotMod4 == 0) )
			{
				CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
			                                       *dataIn32Bit_ptr, CRYS_AES_BLOCK_SIZE_IN_WORDS );

				*countWordsToLoad_ptr -= CRYS_AES_BLOCK_SIZE_IN_WORDS;
				*dataIn32Bit_ptr  += CRYS_AES_BLOCK_SIZE_IN_WORDS;
			}

			else
			{
				if( *countWordsToLoad_ptr > 0 )
				{
					CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
				                                       *dataIn32Bit_ptr, *countWordsToLoad_ptr - 1 );

					/* set the WRITE_ALIGN_LAST register */
					if( sizeNotMod4 != 0 )
						CRYS_PLAT_SYS_WriteRegister(
						           VirtualHwBaseAddr + LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );

					CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
					                             (*dataIn32Bit_ptr)[*countWordsToLoad_ptr - 1] );

					*dataIn32Bit_ptr  += *countWordsToLoad_ptr;
					*countWordsToLoad_ptr = 0;
				}
			}
		}

		/* read the machine states */
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_BUSY_ADDR, isAesBusy );
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_BUSY_ADDR, isAesBusy );
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_FIFO_OUT_EMPTY_REG_ADDR, isDoutFifoEmpty );

		/* if DOUT FIFO is not empty, then read DOUT and output current block of data */
		if( isDoutFifoEmpty == 0 )
		{
			LLF_AES_ReadOutputBlock(
		                    dataOut32Bit_ptr,
		                    countWordsToRead_ptr,
		                    VirtualHwBaseAddr );

			/* if the block is one BeforeLast, then break the loop */
			if( *countWordsToLoad_ptr == 0 )
				break;
		}

	} /* end while() */

	/* ---------------------------------------------------------------------*/
	/* if data size is not multiple of AES_BLOCK_SIZE, then read last block */
	/* ---------------------------------------------------------------------*/
	if( sizeNotMod4 != 0 )
	{
		/* polling on DoutFifoEmpty == 0 */
		LLF_AES_HW_WAIT_ON_DOUT_FIFO_EMPTY_BIT( VirtualHwBaseAddr );

		/* read the last block  */
		LLF_AES_ReadOutputBlock(
		                    dataOut32Bit_ptr,
		                    countWordsToRead_ptr,
		                    VirtualHwBaseAddr );
	}

	/* end of execution */
	return;

} /* end XTS */
