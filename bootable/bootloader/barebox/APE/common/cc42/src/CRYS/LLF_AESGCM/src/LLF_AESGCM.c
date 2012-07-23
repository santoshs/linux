
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  07 Apr. 2011
   *  Last modified   :  %modify_time% 27 Apr. 2011
   */
  /** @file
   *  \brief The file includes LLF functions, performing AESGCM operations on hard ware.
   *
   *  \version LLF_AESGCM.c#1:csrc:1
   *  \author  R.Levin
   *  \remarks Copyright (C) 2011 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

/************* Include Files ****************/
#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_AES.h"
#include "LLF_AES_HwDefs.h"
#include "LLF_AES_Restrict.h"
#include "CRYS_AESGCM.h"
#include "LLF_AESGCM_HwDefs.h"
#include "LLF_AESGCM.h"
#include "LLF_AESGCM_error.h"

/* canceling the lint warning:
    Suspicious pointer-to-pointer conversion (area too small) */
/*lint --e{826} */

/************************ Defines ************************************/

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


/************************ Enums **************************************/

/************************** Typedefs *********************************/

/* The macros reverses order of bytes in 32 bit word x */
#define REV32(x)  \
	((CRYS_COMMON_ROT32(x) & 0xff00ff00UL) >> 8) | \
	((CRYS_COMMON_ROT32(x) & 0x00ff00ffUL) << 8)


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

/************************* Private functions prototypes *************************/


/********************* Public Functions ******************************/

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_INit function calculates Initial GCTR counter value value J0
*        according to NIST 800-38D.
*
* @param[in] Context_ptr   - The pointer to AESGCM context.
* @param[in] key_ptr       - The pointer to gCM key.
* @param[in] keySizeID     - The key size ID;
* @param[in] IV_ptr	       - A pointer to initial value (nonce) - unique value assigned to all data,
*                            passed into GCM.
* @param[in] ivSize        - The size of the user passed IV (in bytes).
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value defined in LLF_AESGCM_error.h
*/
CRYSError_t LLF_AESGCM_Init(
							 AESGCMContext_t       *Context_ptr,   /*GCM context*/
							 DxUint8_t             *key_ptr,       /*AESGCM key*/
							 CRYS_AESGCM_KeySize_t  keySizeID,     /*size of key in bytes*/
							 DxUint8_t             *iv_ptr,        /*initial value*/
							 DxUint32_t             ivSize )       /*size of IV in bytes*/

{
	   /* The return error identifiers */
   CRYSError_t Error;

   /* AES restricted Context (size 5 words) */
   AESResrtictContext_t    AesRestrContext;

   DxUint32_t regVal, virtualHwBaseAddr;
   DxUint32_t keySizeBytes;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;


    /* ............  Check AES version register .............................*/
	/* --------------------------------------------------------------------- */

	/* get virtual HW base address  */
	/* ---------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
	                      LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
	                      &virtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
		goto Unmap;

    /* read AES engine version */
    CRYS_PLAT_SYS_ReadRegister( virtualHwBaseAddr + HW_VERSION_REG_ADDR, regVal );

    /*extract the customer code from the version*/
    regVal = (( regVal & 0xF00 ) >> 0x8);

    /*Old engines does not support AES flags register but do support all AES key types*/
    if((regVal == LLF_AES_CUST_CODE_ONE_VAL) ||(regVal == LLF_AES_CUST_CODE_TWO_VAL))
        regVal = LLF_AES_HW_HLAG_192_256_VAL;
    else
        CRYS_PLAT_SYS_ReadRegister( virtualHwBaseAddr + LLF_AES_HW_FLAGS_ADDR, regVal );

    if( !(regVal & LLF_AES_HW_HLAG_192_256_VAL) && Context_ptr->LLF.KeySizeBytes > 16 )
        Error = LLF_AES_HW_INTERNAL_ERROR_3;

	/* .... un mapping the physical ...... */
Unmap:
	DX_VOS_MemUnMap( &virtualHwBaseAddr,             /* virtual address - in */
                    LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

	if( Error != CRYS_OK )
		return Error;

   /* -------------------------------------------------------------------- */
   /* .................  Init processing  ................................ */
   /* -------------------------------------------------------------------- */

   switch( keySizeID )
   {
	 case CRYS_AESGCM_Key128BitSize:
		keySizeBytes = 16;
		break;

     case CRYS_AESGCM_Key192BitSize:
		keySizeBytes = 24;
		break;

	 case CRYS_AESGCM_Key256BitSize:
		keySizeBytes = 32;
		break;
   }


   /* set key into Context */
   DX_VOS_FastMemCpy( Context_ptr->LLF.Key, key_ptr, keySizeBytes );
   Context_ptr->LLF.KeySizeBytes = keySizeBytes;

   /*  calculate GHAH subkey H = ECB(key,0)           */
   /* ----------------------------------------------- */

   /* get AES semaphore */
   LLF_AES_RESTR_StartInit();

   /* perform restricted AES operation on ECB mode */
   LLF_AES_RESTR_Init(
                   Context_ptr->LLF.Key,
                   keySizeBytes>>2,
                   DX_NULL /*iv_ptr*/,
                   CRYS_AES_ECB_mode,
                   CRYS_AES_Encrypt,
                   &AesRestrContext );

   LLF_AES_RESTR_BlockExec(
			   &Context_ptr->LLF.Ctr[0],    /*DataIn = 128 zero bits*/
				   &Context_ptr->H[0],         /*DataOut*/
				   1/*size in AES blocks*/,
	               &AesRestrContext );

   LLF_AES_RESTR_Finish( DX_NULL, /*v_ptr*/
			 &AesRestrContext );

    /* give AES semaphore */
   LLF_AES_RESTR_EndFinish();


   /*   calculate initial counter value  CTR = J0  and set it in LLF Context;
    because now allowed only IV size == 12 bytes, then J0 = IV || 31b'0 || 1 */
   DX_VOS_FastMemCpy( &Context_ptr->LLF.Ctr[0], iv_ptr, ivSize );
   /* increment J0 = J0 + 1*/
   Context_ptr->LLF.Ctr[3] = 0x02000000;

   return Error;

}/* END OF CRYS_AESGCM_Init */

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_GHASH function calculates the GHASH MAC value on block of
*        Additional data according to NIST 800-38D 6.4.
*
* @param[in] GcmContext_ptr - The pointer to AESGCM context.
* @param[in] data_ptr - The pointer to input data - bytes array.
* @param[in] size - The input data size in bytes;
* @param[in] size -  The size of the input data in bytes.
*
* @return CRYSError_t - On success CRYS_OK is returned, on failure a
*                       value defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_GHASH(
							  AESGCMContext_t *Context_ptr,
							  DxUint8_t       *DataIn_ptr,
							  DxUint32_t       DataInSize )
 {
   /* LOCAL DECLARATIONS */

   CRYSError_t Error = CRYS_OK;

   /* the virtual address definition */
   DxUint32_t virtualHwBaseAddr;

   /* remaining data size */
   DxUint32_t remainSize, i;

   /* temp buffer for aligned block of data */
   DxUint32_t  temp[4] = {0};


   /* Initializations */

   /* remaining size and alignment of input data */
   remainSize = DataInSize & 0xF;
   DataInSize -= remainSize;


	/* FUNCTION LOGIC */

	/* ............... getting the hardware semaphore ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	{
		goto Return;
	}

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
	                      LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
	                      &virtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
		goto ReturnAndReleaseSemaphore;
	}

    /*----------------------------------------------------------*/
    /* ..............    GHASH processing    .................. */
    /*----------------------------------------------------------*/

	/* initialization of GHASH operation - for all operations, besides DIN->AES->DOUT */
    /*-------------------------------------------------------------------------------*/
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CLK_ENABLE_ADDR, 0x1UL );

    /* set crypto control according to operation mode (flow) */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
                                 LLF_AESGCM_HW_CRYPTO_FLOW_HASH /*0x7*/);


    /* -------------------- GHASH Init ------------------ */

    /* set HASH control to GHASH mode */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CONTROL_ADDR,
                                 LLF_AESGCM_HW_HASH_CTL_GHASH_VAL );

    /* if this is a first GHASH operation, then set Start GHASH register to prevent
       XOR with prvious Digest result, else load the previouse Mac (Digest) result */
    if( Context_ptr->DataSizes[0] == 0 )
    {
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_START_GCM_REG_ADDR, 1UL );
    }
    else
    {
		CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_DIGEST0_REG_ADDR,
	                                       &Context_ptr->Mac[0], CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );
    }

	/* load GHASH subkeys */
	CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_SUBKEY0_REG_ADDR,
                                       &Context_ptr->H[0], CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );


	/* --------------------------------------------------------------------- */
	/* ............   process the full blocks of the Data  ................. */
	/* --------------------------------------------------------------------- */

    /* set Read and Write alignments according to the data alignment */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR, 0x0UL );

    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR,
          (((DxUint32_t)DataIn_ptr & CRYS_COMMON_BusAlignerMask))    |
          ((((DxUint32_t)DataIn_ptr + DataInSize)&CRYS_COMMON_BusAlignerMask)<<CRYS_COMMON_BusAlignerShift) );

	/* ????? Wait Crypto busy */
	LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

    /*  CPU-CPU transaction */
    if( DataInSize < CRYS_AESGCM_BLOCK_SIZE_IN_BYTES*LLF_AESGCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION )
    {
	    DxUint32_t *in_ptr =  (DxUint32_t*)((DxUint32_t)DataIn_ptr & 0xFFFFFFFC);
	    DxUint32_t numWords = DataInSize >> 2;

		/* if non aligned pointer, then insert the first not full word and update the pointer */
	    if( (DxUint32)DataIn_ptr & 3UL )
	    {
		/* insert first not full word and update the pointer */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR, *in_ptr );
		in_ptr++;

		numWords--;
	    }

	    /* insert the full words */
	    for(i = 0; i < numWords; i++ )
	    {
			CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR, *in_ptr );
		in_ptr++;

			/* ???  Wait Crypto busy //Change to: Wait HASH busy  ?  */
			if( (i & 3 ) == 3 )
				LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );
	    }

		/* if non aligned pointer, then insert the last not full word  */
	    if( (DxUint32)DataIn_ptr & 3UL ) /* non aligned pointer */
	    {
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + HW_WRITE_ALIGN_LAST_REG_ADDR, 1 );
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR, *in_ptr );
			/* ???  Wait Crypto busy //Change to: Wait HASH busy  ?  */
			LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );
	    }

    }

    else   /* DMA transaction */
    {
	    /* write single LLI words */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + HW_SRC_LLI_WORD0_REG_ADDR, (DxUint32_t)DataIn_ptr );
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + HW_SRC_LLI_WORD1_REG_ADDR,
		                          FIRST_ENTRY | LAST_ENTRY | DataInSize );

		/*?? polling on busy bits DIN and FIFO_IN */
		LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_DIN_MEM_DMA_BUSY_REG_ADDR );
		LLF_AESGCM_HW_WAIT_ON_REGISTER_VALUE( virtualHwBaseAddr, HW_FIFO_IN_EMPTY_REG_ADDR, 0x1UL );

		/* ???  Wait Crypto busy //Change to: Wait HASH busy    */
		LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );
    }

	/* update the input pointer */
	DataIn_ptr += DataInSize;


	/* --------------------------------------------------------------------- */
	/* ....................   process remaining Data   ..................... */
	/* --------------------------------------------------------------------- */
    if( remainSize )
    {
	    /* copy remaining data into zeroed temp buffer */
	    DX_VOS_FastMemCpy( (DxUint8_t*)temp, (DxUint8_t*)DataIn_ptr, remainSize );

	    /* set CRYPTO_CTL again in order to change alignments */
	    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
	                                 LLF_AESGCM_HW_CRYPTO_FLOW_HASH /*7*/);

	    /*  set the Read and Write alignments according to the temp buffer alignment = 0 */
	    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR, 0x0UL );
	    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR,  0x0UL );

	    /* process the temp buffer */
		CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
	                                       temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

		/* ???  Wait Crypto busy //may be change to: Wait HASH busy    */
		LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );
    }

	/* --------------------------------------------------------------------- */
	/* ....................   save Digest value in the context   ........... */
	/* --------------------------------------------------------------------- */
	CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_DIGEST0_REG_ADDR,
                                      &Context_ptr->Mac[0], CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

	/* ................. end of the function ................................. */
	/* ----------------------------------------------------------------------- */

	/* close the Hardware clock */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CLK_ENABLE_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &virtualHwBaseAddr,             /* virtual address - in */
                    LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

Return:

   return Error;

 }/* END OF LLF_AESGCM_GHASH */


/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_TextDataExec function processes the GCM authenticated
*        encryption/decryption on Text data.
*
* @param[in] virtualHwBaseAddr -  The virtual base address of HW.
* @param[in] GcmContext_ptr - The AESGCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value
*                       defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_TextDataExec(
                         DxUint32_t           virtualHwBaseAddr,
                         AESGCMContext_t     *Context_ptr,
						 DxUint8_t           *DataIn_ptr,
						 DxUint32_t           DataInSize,
						 DxUint8_t           *DataOut_ptr)
{
	/* LOCAL DECLARATIONS */

	CRYSError_t Error = CRYS_OK;

	DxUint32_t aesControl, remainSize, cryptoFlow;
	DxUint32_t temp[4] = {0};


	/* FUNCTION LOGIC */

	/* ........................ local initializations .......................... */
	/* ------------------------------------------------------------------------- */

    remainSize = DataInSize & 0xF;
    DataInSize = DataInSize - remainSize;

    /*----------------------------------------------------------*/
    /* ..............    Text Data processing   ............... */
    /*----------------------------------------------------------*/


	/*       initialization of AES operation                    */
    /*----------------------------------------------------------*/

    aesControl = LLF_AES_HW_AES_CTL_KEY_0_GCTR_MODE_VAL;

    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR, 0x1UL );
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CLK_ENABLE_ADDR, 0x1UL );

    /* set crypto and AES controls according to operation and encryption modes  */
    if( Context_ptr->EncryptMode == CRYS_AESGCM_Decrypt )
	    cryptoFlow = LLF_AESGCM_HW_CRYPTO_FLOW_AES_AND_HASH /*0x3*/;
    else
	    cryptoFlow = LLF_AESGCM_HW_CRYPTO_FLOW_AES_TO_HASH_AND_DOUT;

    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR, cryptoFlow );

    /* on CTR mode AES control always is on Encrypt */
    aesControl |= LLF_AES_HW_AES_CTL_ENCRYPT_VAL;

	/* set the KEY size value */
	switch( Context_ptr->LLF.KeySizeBytes )
	{
	case 16:
		aesControl |= LLF_AES_HW_AES_CTL_KEY128_VAL;
		break;

	case 24:
		aesControl |= LLF_AES_HW_AES_CTL_KEY192_VAL;
		break;

	case 32:
		aesControl |= LLF_AES_HW_AES_CTL_KEY256_VAL;
		break;

	}/* end of setting the KEY size switch case */


    /* ..................... AES Init  ................. */
    /* ------------------------------------------------- */

	/* Redundant ???  Wait AES busy  */
	LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, LLF_AES_HW_AES_BUSY_ADDR );

	/* setting the AES Control register */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CTL_ADDR, aesControl );

	/* load GCTR registers */
	CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_CTR_0_ADDR_0,
                                       &Context_ptr->LLF.Ctr[0], CRYS_AES_BLOCK_SIZE_IN_WORDS );

	/* load AES Key registers */
	CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0,
                                       &Context_ptr->LLF.Key[0], Context_ptr->LLF.KeySizeBytes >> 2 );

	/* wait AES busy - to create Extend key  */
	LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, LLF_AES_HW_AES_BUSY_ADDR );

    /* ................. GHASH Init...................... */
    /* -------------------------------------------------- */

    /* set HASH control to GHASH mode */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CONTROL_ADDR,
                                 LLF_AESGCM_HW_HASH_CTL_GHASH_VAL /*0x10*/ );

    /* if this is a first GHASH operation, then set Start GHASH register to prevent
       XOR with previous Mac (Digest) result, else load the previous Mac result */
    if( Context_ptr->DataSizes[0] == 0 && Context_ptr->DataSizes[1] == 0 )
    {
	    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_START_GCM_REG_ADDR, 1UL );
    }
    else
    {
	    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_START_GCM_REG_ADDR, 0UL );
		CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_DIGEST0_REG_ADDR,
	                                       &Context_ptr->Mac[0], CRYS_AES_BLOCK_SIZE_IN_WORDS );
    }

	/* load GHASH subkeys */
	CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_SUBKEY0_REG_ADDR,
                                       &Context_ptr->H[0], CRYS_AES_BLOCK_SIZE_IN_WORDS );

	/* Wait Crypto busy */
	LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

	/* --------------------------------------------------------------------- */
	/* ............   process the full blocks of the Data  ................. */
	/* --------------------------------------------------------------------- */

    if( DataInSize )
    {
	  if( DataInSize < CRYS_AES_BLOCK_SIZE_IN_BYTES*LLF_AESGCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION )
	  {
		DxUint32_t i;

	    /* set Read and Write alignments to 0 */
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR, 0UL );
		CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR, 0UL );

		/*  CPU-CPU transaction */
	    for( i = 0; i < (DataInSize >> 4); i++ )
	    {

		    /* copy data into aligned buffer and process the buffer */
		    DX_VOS_FastMemCpy( temp, DataIn_ptr, CRYS_AES_BLOCK_SIZE_IN_BYTES );

			CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
		                                       temp, CRYS_AES_BLOCK_SIZE_IN_WORDS );
			/* Wait Crypto busy  */
			LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

			CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
		                                      temp, CRYS_AES_BLOCK_SIZE_IN_WORDS );

		    /* output the result Data */
		    DX_VOS_FastMemCpy( DataOut_ptr, temp, CRYS_AES_BLOCK_SIZE_IN_BYTES );

			DataIn_ptr += CRYS_AES_BLOCK_SIZE_IN_BYTES;
			DataOut_ptr += CRYS_AES_BLOCK_SIZE_IN_BYTES;
	    }
	  }

	  else   /*  DMA transaction */
      {
	      /* set Read and Write alignments according to the data alignment */
		  CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR ,
		                             ((DxUint32_t)DataOut_ptr & CRYS_COMMON_BusAlignerMask));

	      CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR,
	            (((DxUint32_t)DataIn_ptr & CRYS_COMMON_BusAlignerMask))    |
	            ((((DxUint32_t)DataIn_ptr + DataInSize)&CRYS_COMMON_BusAlignerMask)<<CRYS_COMMON_BusAlignerShift) );

	      /* write DST single LLI words */
		  CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DST_LLI_WORD0_ADDR,
		                               (DxUint32_t)DataOut_ptr );
		  CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DST_LLI_WORD1_ADDR,
		                               FIRST_ENTRY | LAST_ENTRY | DataInSize );

	      /* write SRC single LLI words */
		  CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_SRC_LLI_WORD0_ADDR,
		                               (DxUint32_t)DataIn_ptr );
		  CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_SRC_LLI_WORD1_ADDR,
		                               FIRST_ENTRY | LAST_ENTRY | DataInSize );

		  /* polling on busy bits DIN and FIFO_IN */
		  LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_DIN_MEM_DMA_BUSY_REG_ADDR );
		  LLF_AESGCM_HW_WAIT_ON_REGISTER_VALUE( virtualHwBaseAddr, HW_FIFO_IN_EMPTY_REG_ADDR, 0x1UL );

		  /* Wait Crypto busy */
		  LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

		  /* update the pointers */
		  DataIn_ptr  += DataInSize;
		  DataOut_ptr += DataInSize;
      }
    }


	/* --------------------------------------------------------------------- */
	/* ....................   process remaining Data   ..................... */
	/* --------------------------------------------------------------------- */
    if( remainSize )
    {
	    /* zeroe temp buffer for next operations */
	    temp[0] = 0; temp[1] = 0; temp[2] = 0; temp[3] = 0;

	    /* copy remaining data into zeroed temp buffer */
	    DX_VOS_FastMemCpy( temp, (DxUint8_t*)DataIn_ptr, remainSize );

        /* write into remaining bytes register 0, because all data now is a full block */
        CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_REMAINING_BYTES_REG_ADDR, 0 );

	    /* process the temp buffer on Decrypt mode */
	    /* --------------------------------------- */
        if( Context_ptr->EncryptMode == CRYS_AES_Decrypt )
        {
			/* --------    Perform GHASH on Cypher text  ----------- */

		    /* set CRYPTO_CTL again in order to change alignments */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
		                                 LLF_AESGCM_HW_CRYPTO_FLOW_HASH );

		    /*  set the Read and Write alignments according to the temp buffer alignment = 0 */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR, 0x0UL );
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR, 0x0UL );

			CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                               temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

			LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

			/* --------    Perform GCTR on Cypher text  ----------- */

		    /* set CRYPTO_CTL again in order to change alignments */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
		                                 LLF_AESGCM_HW_CRYPTO_FLOW_AES );

			CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                               temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

			LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

			CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                               temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );
		    /* output the result Data */
		    DX_VOS_FastMemCpy( DataOut_ptr, temp, remainSize );
        }

	    /* process the temp buffer on Encrypt mode */
	    /* --------------------------------------- */
	    else
	    {
			/* --------    Perform GCTR on Plain text  ----------- */

		    /* set CRYPTO_CTL again in order to change alignments */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
		                                 LLF_AESGCM_HW_CRYPTO_FLOW_AES );

		    /*  set the Read and Write alignments according to the temp buffer alignment = 0 */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR, 0x0UL );
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR, 0x0UL );

			CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                               temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

			LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

			CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                               temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

		    /* output the result Data */
		    DX_VOS_FastMemCpy( DataOut_ptr, temp, remainSize );

			/* --------    Perform GHASH on Cypher text  ----------- */

			/* zero padding of Cypher text */
		    DX_VOS_MemSetZero( (DxUint8_t*)&temp[0] + remainSize, CRYS_AESGCM_BLOCK_SIZE_IN_BYTES - remainSize );


		    /* set CRYPTO_CTL again in order to change alignments */
		    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
		                                 LLF_AESGCM_HW_CRYPTO_FLOW_HASH );

	        /* write remaining bytes register */
	        CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_REMAINING_BYTES_REG_ADDR, 0 );

			CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                               temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

			LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );
	    }
    }


	return Error;

}/* END OF LLF_AESGCM_TextDataExec */

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_BlockTextData function processes the GCM authenticated
*        encryption/decryption on block of Text data.
*
* @param[in] GcmContext_ptr - The AESGCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value
*                       defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_TextData(
                         AESGCMContext_t     *Context_ptr,
						 DxUint8_t           *DataIn_ptr,
						 DxUint32_t           DataInSize,
						 DxUint8_t           *DataOut_ptr)
{
	/* LOCAL DECLARATIONS */

	CRYSError_t Error;

	/* the virtual address definition */
	DxUint32_t virtualHwBaseAddr;


	/* FUNCTION LOGIC */

    /* ............... getting the hardware semaphore ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	goto Return;

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &virtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	goto ReturnAndReleaseSemaphore;


    /*----------------------------------------------------------*/
    /* ..............    Text Data processing   ............... */
    /*----------------------------------------------------------*/

	Error =	LLF_AESGCM_TextDataExec( virtualHwBaseAddr,
                                     Context_ptr,
						             DataIn_ptr,
						             DataInSize,
						             DataOut_ptr );

    if( Error )
	goto ReturnWithError;


	/* .........   save the Digest and CTR values in the context   ......... */
	/* --------------------------------------------------------------------- */

	CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_DIGEST0_REG_ADDR,
                                      &Context_ptr->Mac[0], CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );

	CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AES_HW_AES_CTR_0_ADDR_0,
                                       &Context_ptr->LLF.Ctr[0], CRYS_AES_BLOCK_SIZE_IN_WORDS );

	/* ----------------------------------------------------------------------- */
	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */
ReturnWithError:
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CLK_ENABLE_ADDR , 0x0UL );
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &virtualHwBaseAddr,             /* virtual address - in */
                    LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

Return:

	return Error;

}/* END OF LLF_AESGCM_TextData */

/*********************************************************************************************/
/**
* @brief The LLF_AESGCM_Finish function processes the GCM authenticated
*        encryption/decryption on last block of the Text data.
*
* @param[in] GcmContext_ptr - The AESGCM context allocated by the CCM.
* @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
* @param[in] DataInSize -  The size of the buffer the user shall operate on.
* @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

* @return CRYSError_t - On success CRYS_OK is returned, on failure a value
*                       defined in LLF_AESGCM_error.h
*/
CRYSError_t  LLF_AESGCM_Finish(
                         AESGCMContext_t     *Context_ptr,
						 DxUint8_t           *DataIn_ptr,
						 DxUint32_t           DataInSize,
						 DxUint8_t           *DataOut_ptr )
{
    /* LOCAL DECLARATIONS */

    CRYSError_t Error;
    DxUint32_t temp[4];

    DxUint32_t *ptr;
    DxUint32_t  virtualHwBaseAddr;

    /* ..... local initializations ....... */


    /* FUNCTION LOGIC */

	/* ............... getting the hardware semaphore ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	{
		goto Return;
	}

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
	                      LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
	                      &virtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
		goto ReturnAndReleaseSemaphore;
	}


	/* set Finish indication into Context.LLF */
	Context_ptr->LLF.IsFinish = 1;

    /* process TextData  */
    Error =  LLF_AESGCM_TextDataExec(
                         virtualHwBaseAddr,
                         Context_ptr,
						 DataIn_ptr,
						 DataInSize,
						 DataOut_ptr);
    if( Error )
	goto ReturnWithError;


	/* --------------------------------------------------------------------- */
	/* ............  Finish operations: GHASH of Sizes info   .............. */
	/* --------------------------------------------------------------------- */

	ptr = (DxUint32_t*)&Context_ptr->DataSizes[0];

    /* convert Data Sizes to big endian and set in temp block */
    temp[0] = REV32(ptr[1]);
    temp[1] = REV32(ptr[0]);
    temp[2] = REV32(ptr[3]);
    temp[3] = REV32(ptr[2]);

	/* set crypto flow DIN-HASH-CPU */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
                                 LLF_AESGCM_HW_CRYPTO_FLOW_HASH /*0x7*/);

    /*  set the Read and Write alignments according to the temp buffer alignment = 0 */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR, 0x0UL );
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR, 0x0UL );

    /* write remaining bytes */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_REMAINING_BYTES_REG_ADDR,
                                 CRYS_AES_BLOCK_SIZE_IN_BYTES );

    /* process the DataSizes (temp) buffer */
	CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR, temp,
	                                   CRYS_AES_BLOCK_SIZE_IN_WORDS );

	/* Wait Crypto busy  */
	LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

	/* read the Digest */
	CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + LLF_AESGCM_HW_GHASH_DIGEST0_REG_ADDR,
                                      temp, CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );


	/* encrypt the GHASH Didgest by GCTR         */
    /* ----------------------------------------- */

	/* set crypto flow DIN-AES-CPU */
    CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_CRYPTO_CTL_ADDR,
                                 LLF_AESGCM_HW_CRYPTO_FLOW_AES /*0x1*/);

    /* set last word of CTR to J0 value */
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CTR_0_ADDR_3, 0x01000000UL );

    /* encrypt the temp buffer */
	CRYS_PLAT_SYS_WriteRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR, temp,
	                                   CRYS_AES_BLOCK_SIZE_IN_WORDS );

	/* Wait Crypto busy  */
	LLF_AESGCM_HW_WAIT_ON_REGISTER( virtualHwBaseAddr, HW_CRYPTO_BUSY_REG_ADDR );

	/* read the GHASH */
	CRYS_PLAT_SYS_ReadRegistersBlock( virtualHwBaseAddr + HW_DIN_BUFFER_REG_ADDR,
                                      &Context_ptr->Mac[0], CRYS_AESGCM_BLOCK_SIZE_IN_WORDS );


	/* ----------------------------------------------------------------------- */
	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */
ReturnWithError:
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AESGCM_HW_HASH_CLK_ENABLE_ADDR , 0x0UL );
	CRYS_PLAT_SYS_WriteRegister( virtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &virtualHwBaseAddr,             /* virtual address - in */
                    LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

Return:

	return Error;

}/* END OF LLF_AESGCM_Finish */
