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

#include "CRYS_AES_error.h"

#include "LLF_AES.h"
#include "LLF_AES_error.h"
#include "LLF_AES_HwDefs.h"
#include "LLF_AES_Restrict.h"

/************************ Defines *********************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */

/* canceling the lint warning:
   suspicious pointer conversion */
/*lint --e{826} */

/* canceling the lint warning:
   Possible creation of out-of-bounds pointer */
/*lint --e{662} */
/*lint --e{661} */

/************************ Enums ************************************/

/************************ Typedefs *********************************/

/************************ Global Data ******************************/

#ifndef CRYS_NO_GLOBAL_DATA
/* importing the semaphore used for the access to the hardware machine */
extern DxVosSem SemHwAccessId;
#endif

/* importing the base address of the CRYPTO Cell . this value must be initialized in the
   CRYS_Init function */

extern DxUint32_t PLAT_CryptoCellBaseAddr;

/************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_StartInit function gets the AES HW semaphore.
 *
 *   The function must be called once before starting AES_RESTRICTED processing.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_RESTR_StartInit ( void )
{

    /* ............... getting the hardware semaphore ..................... */
	/* -------------------------------------------------------------------- */

	return DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

}/* END OF LLF_AES_RESTR_StartInit */

/************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_Init function initializes the hardware to operate
 *        AES at CBC-MAC or CTR modes
 *
 *
 * @param[in] key_ptr - The pointer to AES key.
 * @param[in] keySizeWords  -  The key size in words.
 * @param[in] iv_ptr        -  The pointer to the initial (chaining) value.
 * @param[in] aesMode       -  AES Operation mode.
 * @param[in] encrMode      -  AES encrypt-decrypt mode.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_RESTR_Init (
	                       DxUint32_t      *key_ptr,
	                       DxUint16_t       keySizeWords,
	                       DxUint32_t      *iv_ptr,
	                       CRYS_AES_OperationMode_t aesMode,
	                       CRYS_AES_EncryptMode_t   encrMode,
	                       AESResrtictContext_t *aesCont_ptr )
{
	/* LOCAL DECLARATIONS */

    CRYSError_t Error = CRYS_OK;

	/* the AES control value */
	DxUint32_t aesControlVal = LLF_AES_HW_AES_CTL_KEY_0_SELECT_VAL;

	DxUint32_t  ivOrCtr_ptr;

	DxUint32_t  virtualHwBaseAddr;

	/* FUNCTION LOGIC */

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &virtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	return Error;

    /*................  setting context parameters  ...............*/

    /* set virtual HW base address into context */
	aesCont_ptr->virtualHwBaseAddr = virtualHwBaseAddr;
    /* set AES mode and Encrypt-Decrypt mode into context */
	aesCont_ptr->aesMode = aesMode;
	aesCont_ptr->encryptMode = encrMode;

    /* set AES control operation bit  */
	switch( aesMode )
	{
	case CRYS_AES_ECB_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_ECB_MODE_VAL;
		break;

	case CRYS_AES_CBC_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_CBC_MODE_VAL;
			break;

	case CRYS_AES_MAC_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_MAC_MODE_VAL;
		break;

	case CRYS_AES_CTR_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_CTR_MODE_VAL;
			break;
	}

	/* set the Key size control bits */
	if( keySizeWords == 4)
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY128_VAL;
    else if( keySizeWords == 6 ) /* 6 words */
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY192_VAL;
    else /* 8 words */
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY256_VAL;

	/* loading the decrypt-encrypt mode */
	if( aesCont_ptr->encryptMode == CRYS_AES_Encrypt )
	{
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_ENCRYPT_VAL;
	}
	else
	{
	aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_DECRYPT_VAL;
	}


	/* ................. registers initializations ..................... */
	/* ----------------------------------------------------------------- */

	/* ...... enabling the AES clock */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR ,1UL);

	/* .... setting the CRYPTO_CTL register to AES mode */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR ,
                                LLF_AES_HW_CRYPTO_CTL_AES_MODE_VAL );

	/* setting the AES control register  */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CTL_ADDR ,
                                aesControlVal );

	/* .............. loading the IV registers on CBC, MAC ...................... */

	if( aesMode == CRYS_AES_CTR_mode )
		ivOrCtr_ptr =  LLF_AES_HW_AES_CTR_ADDR_0;
	else
		ivOrCtr_ptr = LLF_AES_HW_AES_IV_0_ADDR_0;

    if( aesMode == CRYS_AES_CBC_mode || aesMode == CRYS_AES_MAC_mode ||
        aesMode == CRYS_AES_CTR_mode )
	{
		if( iv_ptr != DX_NULL ) /* set IV from user passed pointer */
			CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + ivOrCtr_ptr ,
							   iv_ptr , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );
		else /* set IV = 0, this needed for saving memory in some algorithms */
		{
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + ivOrCtr_ptr +  0, 0x0UL );
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + ivOrCtr_ptr +  4, 0x0UL );
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + ivOrCtr_ptr +  8, 0x0UL );
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + ivOrCtr_ptr + 12, 0x0UL );
		}
	}
    /* if key_ptr == NULL, then set key = 0  */
	if( key_ptr == DX_NULL )
	{
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 +  0, 0x0UL );
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 +  4, 0x0UL );
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 +  8, 0x0UL );
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 + 12, 0x0UL );
	}

	else
	    /*  loading the users key */
		CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 ,
		                                   key_ptr , keySizeWords );

	/*  waiting until the HW machine is enabled */
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( virtualHwBaseAddr );

	/* write alignments 0 */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR ,0x0UL );
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR ,0x0UL );

	return Error;

}/* END OF LLF_AES_RESTR_Init */

/****************************************************************************************/
/**
 * @brief The function performs AES operations on full AES blocks of data.
 *
 * @param[in] in_ptr - The pointer to the input buffer.
 * @param[in] out_ptr - The pointer to the output buffer.
 * @param[in] sizeInBlocks - size of data in AES blocks.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_AES_RESTR_BlockExec(
                        DxUint32_t *in_ptr,
                        DxUint32_t *out_ptr,
                        DxUint32_t  sizeInBlocks,
                        AESResrtictContext_t *aesCont_ptr )
 {
	/* LOCAL DECLARATIONS */

    DxUint32_t i;
    CRYSError_t Error = CRYS_OK;
	DxUint32_t  virtualHwBaseAddr = aesCont_ptr->virtualHwBaseAddr;


	/* FUNCTION LOGIC */

    if( sizeInBlocks == 0 )
	return Error;

    for( i = 0 ; i < sizeInBlocks; i++ )
    {
        /* load 4 words of data in DIN_DOUT register */
        CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
                                           &in_ptr[4*i], CRYS_AES_BLOCK_SIZE_IN_WORDS );
	    /* wait for AES operation to end */
		LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( virtualHwBaseAddr );

	    /* loading the words to the out buffer */
	    if( aesCont_ptr->aesMode != CRYS_AES_MAC_mode )
		CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
	                                          &out_ptr[4*i], CRYS_AES_BLOCK_SIZE_IN_WORDS );
    }

    /* on AES_MAC mode output MAC result */
    if( aesCont_ptr->aesMode == CRYS_AES_MAC_mode && out_ptr != DX_NULL )
	CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0 ,
                                          out_ptr, CRYS_AES_BLOCK_SIZE_IN_WORDS );

    return Error;
} /* End of LLF_AES_RESTR_BlockExec */


/*********************************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_Finish reads the last block from the DATA_OUT
 *        and closes the hardware clock.
 *
 * @param[in] v_ptr         -  The pointer to the initial (chaining) or CTR value.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_RESTR_Finish(
	                       DxUint32_t           *v_ptr,         /*in*/
			   AESResrtictContext_t *aesCont_ptr )  /*in*/
{
	/* LOCAL DECLARATIONS */

    CRYSError_t Error = CRYS_OK;
	DxUint32_t  virtualHwBaseAddr = aesCont_ptr->virtualHwBaseAddr;
	CRYS_AES_OperationMode_t aesMode = aesCont_ptr->aesMode;

	/* FUNCTION LOGIC */

	/* On MAC, CBC output the updated IV */
	if( v_ptr != DX_NULL )
	{
	    /* On CTR mode store the updated CTR  */
		if( aesMode == CRYS_AES_MAC_mode || aesMode == CRYS_AES_CBC_mode  )

	        CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0, v_ptr,
	                                          CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );

		/* On CTR mode output the updated CTR  */
		if( aesMode == CRYS_AES_CTR_mode )

	        CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_CTR_ADDR_0, v_ptr,
	                                          CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );
	}

	/* Clear crypto CTL reg */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR , 0x0UL );

	/* Close the Hardware Clock */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &virtualHwBaseAddr,             /* virtual address - in */
                     LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */


    return Error;

}/* END OF LLF_AES_RESTR_Finish */

/************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_EndFinish function releases the AES HW semaphore.
 *
 *   The function must be called after finish AES_RESTRICTED processing.
 *
 * @return - no return value
 *
 */
void  LLF_AES_RESTR_EndFinish ( void )
{
	/* release the hardware AES semaphore */
	DX_VOS_SemGive ( SemHwAccessId );


}/* END OF LLF_AES_RESTR_EndFinish */




/*********************************************************************************************************/
/**
 * @brief The low level LLF_AES_RESTR_ReadIV gets the IV fron HW registers in MAC or CBC modes
 *
 * @param[in] iv_ptr         -  The pointer to the initial (chaining) value.
 * @param[in] aesCont_ptr   -  The pointer to restricted AES context used for specific operations.
 *                             In HW implementation it contains virtual Hw Base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void  LLF_AES_RESTR_ReadIV(
                       DxUint32_t           *iv_ptr,        /*in*/
			   AESResrtictContext_t *aesCont_ptr )  /*in*/
{
	/* LOCAL DECLARATIONS */

	/* FUNCTION LOGIC */

	/* output the updated IV */
    CRYS_PLAT_SYS_ReadRegistersBlock( aesCont_ptr->virtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0, iv_ptr,
                                      CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );

}/* END OF LLF_AES_RESTR_ReadIV */
