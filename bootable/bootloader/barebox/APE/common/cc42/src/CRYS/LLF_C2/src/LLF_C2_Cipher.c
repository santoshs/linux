
  /*
   *  Object % LLF_C2_Cipher.c   : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief: This file contains LLF functions for performing C2 Cipher operations
   *          on ECB and CBC modes ).
   *
   *  \version LLF_C2_Cipher.c#1:csrc:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/********************** Include Files *****************************/

#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"

#include "DX_VOS_Sem.h"
#ifdef CRYS_SEP_SIDE_WORK_MODE
		#include "sep_sync.h"
#endif /*CRYS_SEP_SIDE_WORK_MODE*/

#include "LLF_C2.h"

#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#include "LLF_C2_error.h"
#include "log_output.h"
#include "LLF_C2_HwDefs.h"
#include "cc_hw_interface.h"
#include "log_output.h"


/************************ Defines **********************************/

/************************ Enums ************************************/

/************************ Typedefs *********************************/

/************************ Global Data ******************************/
#ifndef CRYS_NO_GLOBAL_DATA
/* importing the semaphore used for the access to the hardware machine */
extern DxVosSem SemHwAccessId;
#endif

extern DxUint32_t CRYS_COMMON_BusAlignerShift;
extern DxUint32_t CRYS_COMMON_BusAlignerMask;

extern DxUint32_t PLAT_CryptoCellBaseAddr;

/******************* Private function prototype ********************/
void  LLF_C2_BlockExecFirstOperation (DxUint32_t   VirtualHwBaseAddr,
                                      DxUint32_t   **DataIn_ptr,
                                      DxUint32_t   inAlignment,
                                      DxUint32_t   *CountWordsToLoad);



void LLF_C2_BlockExec(DxUint32_t      **DataIn_ptr,
                      DxUint32_t      **DataOut_ptr ,
                      DxUint32_t      *CountWordsToLoad,
                      DxUint32_t      inAlignment,
                      DxUint32_t      outAlignment,
                      DxUint32_t      VirtualHwBaseAddr );

void LLF_C2_BlockFinish(DxUint32_t **DataOut_ptr ,
                        DxUint32_t outAlignment,
                        DxUint32_t inAlignment,
                        DxUint32_t dataInSize,
                        DxUint32_t VirtualHwBaseAddr);

CRYSError_t LLF_C2_DMA_Block(DxUint8_t   *DataIn_ptr,
							 DxUint8_t   *DataOut_ptr ,
							 DxUint32_t   DataInSize,
							 DxUint32_t   VirtualHwBaseAddr ) ;

void  LLF_C2_AlignerInit (DxUint32_t       VirtualHwBaseAddr ,
                           DxUint32_t       Mask ,
                           DxUint32_t       Shift ,
                           DxUint8_t       *DataOut_ptr,
                           DxUint8_t       *DataIn_ptr,
						   DxUint32_t       DataSize );
/********************************************************************************************/
/****************************** Public Functions ********************************************/
/********************************************************************************************/




/*****************************************************************************************************/
/*
 * @brief This function is used to initialize the C2 Cipher secret constant
 *
 *        The function executes the following major steps:
 *
 *
 *         1. Activate HW semaphore
 *         2.  load SBox
 *	   3.Release HW semaphore
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                       value MODULE_* CRYS_C2_error.h
 */

CEXPORT_C CRYSError_t  LLF_C2_SBoxInit(DxUint8_t *aSecretConstant,
                                       DxUint32_t aConstantSize)
{
  DxUint32_t ind;
  CRYSError_t Error;

  /* the virtual address definition */
  DxUint32_t VirtualHwBaseAddr;

  if (aConstantSize != LLF_HW_SEC_CONST_SIZE)
      return LLF_C2_SEC_CONST_SIZE_ERROR;

  Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

  if( Error != CRYS_OK )
  {
	return Error;
  }

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap(PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_C2_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}

	/* .............. initializing the hardware secure constant .............................. */
	/* ----------------------------------------------------------------------- */

	/*  enabling the C2 clock */
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HW_C2_CLK_ENABLE_REG_ADDR , 0x1UL );

    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_CRYPTO_CTL_REG_ADDR , 0x00010  );

    for(ind = 0; ind < LLF_HW_SEC_CONST_SIZE; ind ++)
       CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_LOAD_SBOX_DATA_REG_ADDR, aSecretConstant[ind]);

    LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

    /*  disabling the C2 clock */
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HW_C2_CLK_ENABLE_REG_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_C2_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

  return CRYS_OK;
}

/**
 * @brief The low level LLF_C2_CipherBlock function operates the C2 engine .
 *
 *    The low level LLF_C2Cipher_Block function initializes needed hardware flags and registers,
 *    and start C2 engine which actually perform the C2 Cipher cryptographic algorithm according to
 *    user defined operation mode (ECB, CBC) and encrypt-decrypt mode.
 *  1.	 Checks the operation mode (EBC, C-CBC) and encrypt-decrypt mode (encrypt, decrypt)
 *  2. Activate HW semaphore
 *  3.	Resume the engine
 *  4.	Initializes hardware flags and registers using contextBuff (defined in paragraph 5.1.1.1)
 *  5.	LLF C2 engine enc/dec operations
 *      •   Direct Data Load Mode
 *           i.	Load data block to be processed into DIN
 *          ii.	poll DOUT for data availability
 *          iii.	read the processed data from DOUT
 *      •   DMA Mode
 *          i.  set up DMA engine
 *         ii.  poll DMA engine for DOUT completion
 *  6.	Suspend the engine
 *  7.	Read flags and registers to contextBuff (defined in paragraph 5.1.1.1) to use it in next block operation.
 *  8.	Release HW semaphore
 *
 * @param[in] WorkingContextID_ptr - The C2Cipher context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_C2_CipherBlock(
                              C2CipherContext_t  *WorkingContextID_ptr,
                              DxUint8_t          *DataIn_ptr,
                              DxUint32_t          DataInSize,
                              DxUint8_t          *DataOut_ptr )
 {

	/* LOCAL DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;

	/* the virtual address definition */
	DxUint32_t VirtualHwBaseAddr;

	/*local loop variable*/
	DxUint32_t i = 0;

	DxUint8_t ind = 0;
    /* is ssdma mode operation */
    DxUint8_t IsDmaMode;


	/* the alignment variable */
	DxUint32_t inAlignment, outAlignment;

	/* pointer for the aligned data*/
	DxUint32_t *dataIn32Bit_ptr;
	DxUint32_t *dataOut32Bit_ptr;

    DxUint32_t countWordsToLoad;


	/* FUNCTION LOGIC */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

       PRINTF("Entered LLF_C2_CipherBlock \n");
	/* set the DMA mode if we exceed the minimum blocks required */

	if ( (DataInSize /CRYS_C2_BLOCK_SIZE_IN_BYTES) >= LLF_C2_MIN_C2_BLOCKS_FOR_DMA_OPERATION)

       IsDmaMode = DX_TRUE;

    else

       IsDmaMode = DX_FALSE;


    /* ............... getting the hardware semaphore ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	{
	goto Return;
	}

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap(PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_C2_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}

	/* .............. initializing the hardware .............................. */
	/* ----------------------------------------------------------------------- */

	/*  enabling the C2 clock */
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HW_C2_CLK_ENABLE_REG_ADDR , 0x1UL );

    LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_CRYPTO_CTL_REG_ADDR , 0x00010  );

    LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

    /*generate resume state */
    CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_SUSPEND_RESUME_REG_ADDR, 0x1UL);

    /*perform resume operation: restore engine state from context buffer*/
    for(ind = 0; ind < 5; ind++)
       CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_RESUME_WDATA_REG_ADDR, WorkingContextID_ptr->LLF.contextBuff[ind]);

   /* ----------------------------------------------------------------------- */
   /* .............. direct CPU state machine ............................... */
   /* ----------------------------------------------------------------------- */

   if( !IsDmaMode )
   {

    inAlignment  = ((DxUint32_t)DataIn_ptr)  & 0x03;
	outAlignment = ((DxUint32_t)DataOut_ptr) & 0x03;

    countWordsToLoad = (inAlignment + DataInSize - DataInSize % 4 + 3 ) / 4;

    dataIn32Bit_ptr  = (DxUint32_t*)(DataIn_ptr);
	dataOut32Bit_ptr = (DxUint32_t*)(DataOut_ptr);

	LLF_C2_AlignerInit ( VirtualHwBaseAddr,
                         0x3,
                         0x3,
                         DataOut_ptr,
                         DataIn_ptr,
						 DataInSize );

	/* .............. executing the first operation .......................... */

    LLF_C2_BlockExecFirstOperation (VirtualHwBaseAddr,
                                    &dataIn32Bit_ptr,
                                    inAlignment,
                                    &countWordsToLoad);

	/* .............. executing the block operation .......................... */

    if(countWordsToLoad > 0)

         LLF_C2_BlockExec(&dataIn32Bit_ptr,
                          &dataOut32Bit_ptr,
                          &countWordsToLoad,
                          inAlignment,
                          outAlignment,
		              VirtualHwBaseAddr);


    /* ................. finishing the block operation ....................... */
   LLF_C2_BlockFinish(&dataOut32Bit_ptr,
                      outAlignment,
	                  inAlignment,
                      DataInSize,
                      VirtualHwBaseAddr);


   }/* end of the direct mode (CPU mode ) case */


   /* ----------------------------------------------------------------------- */
   /* .............. SSDMA state machine .................................... */
   /* ----------------------------------------------------------------------- */
	else
	{

	    LLF_C2_AlignerInit ( VirtualHwBaseAddr,
                              CRYS_COMMON_BusAlignerMask,
                              CRYS_COMMON_BusAlignerShift,
                              DataOut_ptr,
                              DataIn_ptr,
							  DataInSize );

               LLF_Alignment_Pointer(DataIn_ptr,DataOut_ptr,DataInSize); 

		LLF_C2_DMA_Block(DataIn_ptr,             /* the data in updated pointer - in */
						 DataOut_ptr ,           /* the data out updated pointer - in */
						 DataInSize,             /* the data size to load - in */
						 VirtualHwBaseAddr);     /* the virtual address - in */


      }/* end of SSDMA state machine case */



	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

    /*generate suspend state */
    CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_SUSPEND_RESUME_REG_ADDR,0x0UL);

    /*perform suspend operation: save engine state to context buffer*/
    for(ind = 0; ind < 5; ind++)
       CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_HW_C2_SUSPEND_RDATA_REG_ADDR, WorkingContextID_ptr->LLF.contextBuff[ind]);

    /*check that engine finished all operations*/
    LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

   /*  disabling the C2 clock */
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HW_C2_CLK_ENABLE_REG_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_C2_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

	Return:

	return Error;


   return CRYS_OK;

 }/* END OF LLF_C2_CipherBlock */


/********************************************************************************************/
 /**
 * @brief The low level LLF_C2_Init initializes the spesific low level initializatios
 *        required on the spesific platform.
 *  The function initialized registers of C2 engine and save them in contextBuff
 *
 * 1.	Activate HW semaphore
 * 2.	Initialize C2 engine registers
 * 3.	Suspend the engine
 * 4.	Read engine state registers and save them in contextBuff (defined in paragraph 5.1.1.1 of C2 SDD)
 * 5.	Release HW semaphore
 *
  * @param[in] WorkingContextID_ptr - A pointer to the context buffer allocated by the user that
 *                            is used for the C2 Cipher operations.
 *
 * @param[in] Key_ptr -  A pointer to the users key buffer.
 *
 * @param[in] EncryptDecryptMode - This flag determains if the C2 shall perform an Encrypt
 *                                 operation [0] or a Decrypt operation [1].
 *
 * @param[in] OperationMode - The operation mode : ECB or CBC.
 *
 * @param[in] CBC_ResetInterval -  CBC chain breaking interval. The value not used in ECB mode .
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_C2_CipherInit(C2CipherContext_t          *WorkingContextID_ptr,
                               CRYS_C2_Key_t               Key_ptr,
                               CRYS_C2_EncryptMode_t       EncryptDecryptFlag,
                               CRYS_C2_OperationMode_t     OperationMode,
                               DxUint32_t                  CBC_ResetInterval)
{
    CRYSError_t Error;

    /* the virtual address definition */
	DxUint32_t VirtualHwBaseAddr;

	/* the C2 control reg value */
	DxUint32_t c2ControlVal;

	/* the Key  reg value */
	DxUint32_t tempKey;

	DxUint32_t ind;

	/* FUNCTION LOGIC */

	c2ControlVal = 0;
	tempKey =0;
    /* ............... getting the hardware semaphore ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	{
	goto Return;
	}

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */
        PRINTF("Entered LLF_C2_CipherInit\n");
	Error = DX_VOS_MemMap(PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_C2_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}

	/*  enabling the C2 clock */
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HW_C2_CLK_ENABLE_REG_ADDR , 0x1UL );
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_CRYPTO_CTL_REG_ADDR , 0x00010  );

    c2ControlVal |= OperationMode;

    switch (EncryptDecryptFlag)
    {
	case CRYS_C2_Encrypt:

		break;
        case CRYS_C2_Decrypt:
		c2ControlVal |= 0x2UL;
		break;

	default:
	    Error = LLF_C2_ENC_DEC_MODE_ERROR;
	    goto ReturnAndUnMap;
    }

    /* configure ControlRegister to  needed mode EBC/C-CBC, ENC/DEC  */
    CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_CONTROL_REG_REG_ADDR, c2ControlVal);



	 /*write RST_KEY_AFTR_X_BLCKS  register, needed only in C_CBC mode*/
    if(OperationMode == CRYS_C2_CBC_mode)
       CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_RST_KEY_AFTR_X_BLCKS_REG_ADDR,CBC_ResetInterval);

   // Check how to write from Key_ptr
	DX_VOS_FastMemCpy(&tempKey, Key_ptr, sizeof(tempKey));
    CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_KEY0_REG_ADDR,tempKey);

    /*write KEY1 registre*/
    tempKey = 0;

    DX_VOS_FastMemCpy((&tempKey) , Key_ptr +sizeof(DxUint32_t), 3);//we need to copy only 3 dytes
    CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_KEY1_REG_ADDR,tempKey);


    /*save current engine state*/

    /*check that engine finished all operations*/
    LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

    /*generate suspend state */
    CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HW_C2_SUSPEND_RESUME_REG_ADDR,0x0UL);

    /*perform suspend operation: save engine state to context buffer*/
    for(ind = 0; ind < 5; ind++)
    {
      CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_HW_C2_SUSPEND_RDATA_REG_ADDR, WorkingContextID_ptr->LLF.contextBuff[ind]);
    }

    /*check that engine finished all operations*/
    LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);


ReturnAndUnMap:

    /*  disabling the C2 clock */
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HW_C2_CLK_ENABLE_REG_ADDR , 0x0UL );

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_C2_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

	Return:

	return Error;


   return CRYS_OK;


}/* END OF LLF_C2_InitInit */




void  LLF_C2_BlockExecFirstOperation (DxUint32_t   VirtualHwBaseAddr,
                                      DxUint32_t   **DataIn_ptr,
                                      DxUint32_t   inAlignment,
                                      DxUint32_t   *CountWordsToLoad)
{

   /* The first 2 words will be loaded to the hardware */

      DxUint32_t *dataIn32Bit_ptr =  *DataIn_ptr;
      DxUint8_t i;
      DxUint32_t dataInTemp = 0;
	  DxUint32_t temp;

	/* write the first not aligned word into DIN                      */
	/* -------------------------------------------------------------- */
	if( inAlignment != 0 )
    {
     for(i = 0; i < 4 - inAlignment; i++)

		    ((DxUint8_t*)&dataInTemp)[i + inAlignment] =((DxUint8_t*)dataIn32Bit_ptr)[i];

	/* set the aligned bytes of first input word to the hardware DIN */
	CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR, dataInTemp);

		/* increment input pointer */
       temp = (DxUint32_t)dataIn32Bit_ptr;
	   temp  += (4 - inAlignment);
	   dataIn32Bit_ptr = (DxUint32_t*)temp;

       /* decrease count of words */
	   *CountWordsToLoad -=1;
    }

    /* load the DATA_IN registers */
    for( i = 0 ; i < CRYS_C2_BLOCK_SIZE_IN_WORDS ; i++ )
    {
	    if(*CountWordsToLoad > 1 )
	    {
	       CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR, dataIn32Bit_ptr[i]);
		   *CountWordsToLoad -= 1;
		 }

		 else
		 {
		    if(inAlignment)
		      CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_C2_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );

			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,dataIn32Bit_ptr[i]);
		    *CountWordsToLoad = 0;
		    LLF_C2_HW_WAIT_ON_DOUT_FIFO_EMPTY_BIT( VirtualHwBaseAddr );
		 }
	  }

	   //update DataIn_ptr for following write data operations
	  *DataIn_ptr =  dataIn32Bit_ptr + CRYS_C2_BLOCK_SIZE_IN_WORDS;

      LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

}/* END OF LLF_C2_BlockExecFirstOperation */


void LLF_C2_BlockExec(DxUint32_t      **DataIn_ptr,
                      DxUint32_t      **DataOut_ptr ,
                      DxUint32_t      *CountWordsToLoad,
                      DxUint32_t      inAlignment,
                      DxUint32_t      outAlignment,
                      DxUint32_t      VirtualHwBaseAddr )
{

	DxUint32_t *DataIn32Bit_ptr = *DataIn_ptr;

	DxUint32_t *DataOut32Bit_ptr = *DataOut_ptr;

	/*local loop variable*/
	DxUint32_t i = 0;
	DxUint32_t dataOutTemp = 0;


	/* STEP 1: Read first C2 block from DOUT */
    if(outAlignment!=0)//read first not aligned word to temp buff
    {

       CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,dataOutTemp);

       for(i = 0; i < 4 - outAlignment; i++)

		   ((DxUint8_t*)DataOut32Bit_ptr)[i] = ((DxUint8_t*)&dataOutTemp)[i + outAlignment];
    }
    else
       CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,DataOut32Bit_ptr[0]);

    //read secon word of first C2 block
	CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,DataOut32Bit_ptr[1]);


	/* STEP 2: increment the output  buffer pointer */
	DataOut32Bit_ptr = DataOut32Bit_ptr + CRYS_C2_BLOCK_SIZE_IN_WORDS;


	/* STEP 3: loop of writing block to to input wait on busy bit	*/
	/* and read output from DOUT						*/
	/* Since this function should leave the last block in			*/
	/* the machine for the finish function to read the loop is		*/
	/* while the data size is more than one C2 block			*/

	while(*CountWordsToLoad > CRYS_C2_BLOCK_SIZE_IN_WORDS )
	{

	    /* STEP 3.1: load the next block to the machine */
	    for( i = 0 ; i < CRYS_C2_BLOCK_SIZE_IN_WORDS ; i++ )
	    {

	          CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,DataIn32Bit_ptr[i] );
	    }

		/* STEP 3.2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + CRYS_C2_BLOCK_SIZE_IN_WORDS;
        *CountWordsToLoad -= CRYS_C2_BLOCK_SIZE_IN_WORDS;
		/* STEP 3.3 C2 busy bit */
		LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

		/* STEP 3.4 read block from machine */
	    for( i = 0 ; i < CRYS_C2_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
		    CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR ,
		                               DataOut32Bit_ptr[i]);
	    }

		/* STEP 3.5: increment the output buffer pointer */
		DataOut32Bit_ptr = DataOut32Bit_ptr + CRYS_C2_BLOCK_SIZE_IN_WORDS;
	}

	/* STEP 4: post loop phase, write last block to machine */
    for( i = 0 ; i < CRYS_C2_BLOCK_SIZE_IN_WORDS ; i++ )
    {
	   if(*CountWordsToLoad > 1)
		 {
		     CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR, *DataIn32Bit_ptr);

		     *CountWordsToLoad -= 1;
			 DataIn32Bit_ptr  += 1;
		 }

		 else
		 {
		    if(inAlignment)
		       CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_C2_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );

			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,*DataIn32Bit_ptr);
		    *CountWordsToLoad = 0;
		    DataIn32Bit_ptr  += 1;
		    LLF_C2_HW_WAIT_ON_DOUT_FIFO_EMPTY_BIT( VirtualHwBaseAddr );
		 }
    }

	/* STEP 5: C2 busy bit */
	LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);
	*DataOut_ptr = DataOut32Bit_ptr;

}

void LLF_C2_BlockFinish(DxUint32_t **DataOut_ptr ,
                        DxUint32_t outAlignment,
                        DxUint32_t inAlignment,
                        DxUint32_t dataInSize,
                        DxUint32_t VirtualHwBaseAddr)
{

	/*local loop variable*/
	DxUint32_t i = 0;
	DxUint32_t dataOutTemp = 0;
	DxUint32_t *DataOut32Bit_ptr = *DataOut_ptr;
    DxUint32_t temp = 0;

	/*  if we read first not aligned C2 block from DOUT */
    if((outAlignment!=0) && (dataInSize == CRYS_C2_BLOCK_SIZE_IN_BYTES))
    {
      //read first not aligned word to temp buff
       CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,dataOutTemp);

       for(i = 0; i < 4 - outAlignment; i++)

		   ((DxUint8_t*)DataOut32Bit_ptr)[i] = ((DxUint8_t*)&dataOutTemp)[i + outAlignment];
    }
    else //it is not the first block or it is aligned block
       CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR,DataOut32Bit_ptr[0]);

    CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR, DataOut32Bit_ptr[1]);


    DataOut32Bit_ptr += CRYS_C2_BLOCK_SIZE_IN_WORDS;

    //if output data buffer is not aligned read last data buff
    if( outAlignment != 0 )
    {
	   temp = (DxUint32_t)DataOut32Bit_ptr;
	   temp  -=  outAlignment;
	   DataOut32Bit_ptr = (DxUint32_t*)temp;

        /* set READ_ALIGN_LAST register */
	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_READ_LAST_DATA_ADDR, 0x1 );

	    /* read the last word in temp buffer */
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_C2_HW_DIN_DOUT_ADDR, dataOutTemp );

		/* put out the non aligned bytes of the result */
	    for(i = 0; i < outAlignment; i++)
	    {
		    ((DxUint8_t*)(DataOut32Bit_ptr))[i] = ((DxUint8_t*)&dataOutTemp)[i];
	    }
     }
}

CRYSError_t LLF_C2_DMA_Block(DxUint8_t   *DataIn_ptr,
							 DxUint8_t   *DataOut_ptr ,
							 DxUint32_t   DataInSize,
							 DxUint32_t   VirtualHwBaseAddr
							  )
{
  /* LOCAL DECLARATION */

  /* initialize the error identifier as OK */
  CRYSError_t Error = CRYS_OK;

  DxUint32_t DataIn;
  DxUint32_t DataOut;


  /* FUNCTION LOGIC */


   DataIn = (DxUint32_t) DataIn_ptr;
   DataOut= (DxUint32_t)DataOut_ptr;


  /* ................. executing the DMA ........................... */

	/* initialize the destination LLI address */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DST_LLI_WORD0_ADDR, DataOut );

	/* initialize the destination size and set the first & last LLI word */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DST_LLI_WORD1_ADDR, 
	                               DataInSize |
	                               (1UL << LLF_C2_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS) | 
	                              (1UL << LLF_C2_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS) ); 

	/* initialize the source LLI address */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_SRC_LLI_WORD0_ADDR, (DxUint32_t)DataIn );

	/* initialize the source size and set the first & last LLI word - this triggers the operation */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_SRC_LLI_WORD1_ADDR, 
	                               DataInSize |
	                               (1UL << LLF_C2_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS) | 
	                               (1UL << LLF_C2_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS) );
  
  /* STEP2 : wait on the C2 machine and DMA */
/*  Error = CC_Hw_InterFace_Map_And_Start_DMA(DataOut_ptr,DataInSize,VirtualHwBaseAddr,DST_LLI);
  if(Error != CRYS_OK)
        {
          PRINTF("\n CC_Hw_InterFace_Map_And_Start_DMA for DST  failed \n ");
          goto err_end ;
        }

  Error = CC_Hw_InterFace_Map_And_Start_DMA(DataIn_ptr,DataInSize,VirtualHwBaseAddr,SRC_LLI);
  if(Error != CRYS_OK)
  {
     PRINTF("\n CC_Hw_InterFace_Map_And_Start_DMA for SRC DMA failed \n ");
     goto err_end ;
  }
*/

	/* STEP2 */
	/* wait on the DMA DIN */
	LLF_C2_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr );

	/* wait for the hardware machine to be ready */
	LLF_C2_WAIT_ON_REGISTER(VirtualHwBaseAddr,LLF_HW_C2_BUSY_REG_ADDR);

	/* wait on the DMA DOUT  */
	LLF_C2_HW_WAIT_ON_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr );
err_end:

  return Error;

}/* END OF LLF_C2_DMA_Block */


void  LLF_C2_AlignerInit (DxUint32_t       VirtualHwBaseAddr ,
                           DxUint32_t       Mask ,
                           DxUint32_t       Shift ,
                           DxUint8_t       *DataOut_ptr,
                           DxUint8_t       *DataIn_ptr,
						   DxUint32_t       DataSize )
{

	 CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_CRYPTO_CTL_REG_ADDR , 0x00010  );

	/* ....... setting the alignment mode */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DOUT_READ_ALIGN_ADDR ,((DxUint32_t)DataOut_ptr & Mask));
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_C2_HW_DIN_WRITE_ALIGN_ADDR ,
                                  (((DxUint32_t)DataIn_ptr & Mask)) | ((((DxUint32_t)DataIn_ptr)& Mask)<<Shift) );


    return ;

}
