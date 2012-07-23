
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  16 Sept. 2008
   *  Last modified   :  %modify_time% 29 Oct. 2008
   */
  /** @file
   *  \brief The file includes CC6 LLF AESCCM functions, performing AESCCM
   *         operations on CC-hardware.
   *
   *  \version LLF_AESCCM.c#1:csrc:1
   *  \author  R.Levin
   *  \remarks Copyright (C) 2008 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

/************* Include Files ****************/
#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"

#include "DX_VOS_Sem.h"
#ifdef CRYS_SEP_SIDE_WORK_MODE
/* SEP side needs a basic synchronixzation between the interrupts */
#include "sep_sync.h"
#endif /*CRYS_SEP_SIDE_WORK_MODE*/

#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
#include "CRYS_EXT_IF_OTF.h"
#endif

#include "LLF_COMMON.h"
#include "LLF_AES.h"
#include "LLF_AES_error.h"
#include "LLF_AES_HwDefs.h"
#include "LLFCD_AES.h"

#include "LLF_AESCCM_HwDefs.h"
#include "LLF_AESCCM.h"
#include "LLF_AESCCM_error.h"



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


/******************************************************************************************/
/*************           Private functions prototypes      ********************************/
/******************************************************************************************/


/****************************************************************************************/
/**
 * @brief This function is used for initialization of AES_CCM operation of block of data.
 *
 */
CRYSError_t  LLF_AESCCM_BlockInit(
		                       AESCCM_Context_t    *WorkingContextID_ptr,
		                       DxUint32_t           VirtualHwBaseAddr,
		                       DxUint8_t           *DataOut_ptr,
		                       DxUint8_t           *DataIn_ptr,
		                       DxUint32_t           DataInSize );

/****************************************************************************************/
/**
 * @brief This function performs AESCCM (CTR + MAC) operations on block of data.
 *
 */
  CRYSError_t  LLF_AESCCM_Block(
	                              AESCCM_Context_t    *WorkingContextID_ptr,
	                              DxUint8_t           *DataIn_ptr,
	                              DxUint32_t           DataInSize,
	                              DxUint8_t           *DataOut_ptr );



/****************************************************************************************/
/**
 * @brief This function finishess AESCCM operation on block of data.
 *
 */
/****************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockFinish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.
 */
 CRYSError_t  LLF_AESCCM_BlockFinish(
                         AESCCM_Context_t    *WorkingContextID_ptr,    /* in */
                         DxUint32_t           VirtualHwBaseAddr,       /* in */
                         DxUint8_t           *DataIn_ptr ,             /* in/out */
                         DxUint8_t           *DataOut_ptr ,            /* in/out */
                         DxUint32_t           DataInSize,              /* in */
                         DxUint8_t            DataInAlignment,         /* in */
                         DxUint8_t            DataOutAlignment,        /* in */
                         DxUint32_t           NumOfLoadedWords,
                         DxUint32_t           NumOfReadedWords,
                         DxUint8_t            allDataLoaded,
                         DxUint8_t            IsSsdmaMode );           /* in */



/****************************************************************************************/
/**
 * @brief The low level LLF_AESCCM_BlockExecFirstOperation executes the first hardware operation in order to
 *        solve alignment issues and to prepare the next block in the hardware.
 */

 CRYSError_t  LLF_AESCCM_BlockExecFirstOperation(AESCCM_Context_t    *WorkingContextID_ptr,
                                                 DxUint32_t             VirtualHwBaseAddr,
                                                 DxUint8_t            **DataIn_ptr,
                                                 DxUint32_t             DataInSize,
                                                 DxUint8_t            **DataOut_ptr,
                                                 DxUint32_t             *numOfWordsLoadedToHw_ptr,
                                                 DxUint32_t             *numOfWordsReadedFrHw_ptr,
                                                 DxUint8_t             *allDataLoaded);



/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_AlignerInit function initializes the hardware aligner to operate on AES
 *        at the user selected modes.
 */
void  LLF_AESCCM_AlignerInit (
		                       AESCCM_Context_t  *WorkingContextID_ptr ,
		                       DxUint32_t         VirtualHwBaseAddr ,
		                       DxUint32_t         Mask ,
		                       DxUint32_t         Shift ,
		                       DxUint8_t         *DataOut_ptr,
		                       DxUint8_t         *DataIn_ptr,
		                       DxUint32_t         DataInSize );


/*******************************************************************************************************/
/**
 * @brief The low level LLF_AESCCM_BlockProcessSmallData processeses the data not great than 32 bytes.
 *
 */

void  LLF_AESCCM_BlockProcessSmallData (
					                    AESCCM_Context_t      *WorkingContextID_ptr,
					                    DxUint32_t             VirtualHwBaseAddr,
					                    DxUint8_t             *DataIn_ptr,
					                    DxUint8_t             *DataOut_ptr,
					                    DxUint32_t             DataInSize );


/******************************************************************************************/
/*************             External function prototypes             ***********************/
/******************************************************************************************/


/****************************************************************************************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones with the direct DMA on ECB,CBC and CTR modes.
 *
 */
extern CRYSError_t LLF_AES_DMA_BlockECBandCBCandCTRModes(
						              DxUint8_t            *DataIn_ptr,
						              DxUint8_t            *DataOut_ptr ,
						              DxUint32_t            DataInSize,
						              DxUint32_t           VirtualHwBaseAddr );


/****************************************************************************************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones on CPU-CPU with the MAC modes.
 *
 */
extern CRYSError_t LLF_AES_DMA_BlockMacMode(
					              DxUint8_t            *DataIn_ptr,
					              DxUint32_t            DataInSize,
					              DxUint32_t            VirtualHwBaseAddr );

/****************************************************************************************/
#define AESCCM_DEBUG 0

/* debug function for reading IV */
#ifdef AESCCM_DEBUG
	void AESCCM_DebugReadIV( DxUint32_t       VirtualHwBaseAddr,
	                         AESCCM_Context_t *WorkingContextID_ptr)
	{
	   DxUint32_t i;

		LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );
		/* read the MAC from registers */
		for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
		{
			if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )

				CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0 + (4 * i),
	                                    WorkingContextID_ptr->CCM_IV[i] );
			else

				CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_1_ADDR_0 + (4 * i),
	                                    WorkingContextID_ptr->CCM_IV[i] );
		}
	}
#else
	#define AESCCM_DebugReadIV( VirtualHwBaseAddr, WorkingContextID_ptr)
#endif

/****************************************************************************************/
/****************************************************************************************/
/*                        Public Functions                                              */
/****************************************************************************************/
/****************************************************************************************/

/*********************************************************************************************/
/**
 * @brief The LLF_AESCCM_BlockAdata function processes the AESCCM-MAC operation on block of Additional data.
 *
 * @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in LLF_AESCCM_error.h
 */

  CRYSError_t  LLF_AESCCM_BlockAdata( AESCCM_Context_t    *WorkingContextID_ptr,
                                      DxUint8_t           *DataIn_ptr,
                                      DxUint32_t           DataInSize )
 {
   /* LOCAL DECLARATIONS */

   CRYSError_t Error;


   /* FUNCTION LOGIC */


   /* ........................ Call LLF_AESCCM_Block .......................... */
   /* ------------------------------------------------------------------------- */

   Error = LLF_AESCCM_Block( WorkingContextID_ptr,
						     DataIn_ptr,
						     DataInSize,
						     (DxUint8_t *)WorkingContextID_ptr->CCM_IV );


   return Error;

 }/* END OF LLF_AESCCM_BlockAdata */



  /*********************************************************************************************/
  /**
  * @brief The LLF_AESCCM_BlockTextData function processes the AESCCM-MAC operation on block of
  *        Text data.
  *
  * @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
  * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
  * @param[in] DataInSize -  The size of the buffer the user shall operate on.
  * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
  *
  * @return CRYSError_t - On success CRYS_OK is returned, on failure -
  *                       a value defined in LLF_AESCCM_error.h
  */

  CRYSError_t  LLF_AESCCM_BlockTextData(
	                                 AESCCM_Context_t    *WorkingContextID_ptr,
									 DxUint8_t           *DataIn_ptr,
									 DxUint32_t           DataInSize,
									 DxUint8_t           *DataOut_ptr)
  {
    /* LOCAL DECLARATIONS */

    CRYSError_t Error;

   /* FUNCTION LOGIC */

   /* executing the CBC_MAC on Adata */
   Error = LLF_AESCCM_Block( WorkingContextID_ptr,
						     DataIn_ptr,
						     DataInSize,
						     DataOut_ptr );
   return Error;


  }/* END OF LLF_AESCCM_BlockTextData */


  /*********************************************************************************************/
  /**
  * @brief The LLF_AESCCM_BlockLastTextData function processes the AESCCM-MAC operation on
  *        last block of Text data.
  *
  * @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM.
  * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
  * @param[in] DataInSize -  The size of the buffer the user shall operate on.
  * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
  * @param[in] DataOutSize - The size of the output buffer.

  * @return CRYSError_t - On success CRYS_OK is returned, on failure a value defined in LLF_AESCCM_error.h
  */
  CRYSError_t  LLF_AESCCM_BlockLastTextData(
	                                 AESCCM_Context_t    *WorkingContextID_ptr,
									 DxUint8_t           *DataIn_ptr,
									 DxUint32_t           DataInSize,
									 DxUint8_t           *DataOut_ptr,
									 DxUint32_t           DataOutSize )
{
	/* LOCAL DECLARATIONS */

	CRYSError_t Error;


    /* FUNCTION LOGIC */

    /* for preventing compiler warnings */
    DataOutSize = DataOutSize;



   /* FUNCTION LOGIC */


   /* executing the CBC_MAC or CBC_MAC and CTR on data according to data type */
   Error = LLF_AESCCM_Block( WorkingContextID_ptr,
						     DataIn_ptr,
						     DataInSize,
						     DataOut_ptr );
   return Error;

}/* END OF LLF_AESCCM_BlockLastTextData */



/***************************************************************************************/
/*******************          Private Functions              ***************************/
/***************************************************************************************/


/***************************************************************************************/
/**
 * @brief The low level LLF_AESCCM_Block function operates the AES engine in the hardware.
 *        It calls the following low level functions:
 *
 *        1. The function gets a semaphore to protect the access to the hardware
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address
 *           to the registers virtual address.
 *        3. The function calls the LLF_AES_BlockInit to initialize the AES hardware.
 *        4. The function calls the LLF_AES_BlockExecFirstOperation to execute the first operation.
 *        5. The function calls the LLFCD_AES_BlockECBandCBCandCTRModes function if there
 *            is more then 2 block to work on, else will continue to step 6.
 *        6. If there are 2 blocks to work on the function will call the LLF_AES_BlockSingle function.
 *           If only one block is left it will continue to step 7.
 *        7. The function calls the LLF_AES_BlockFinish function to execute the last block and to save
 *           the IV temporary values for the next block operation and to shut down the hardware machine.
 *        8. The function calls a VOS system call to un map the registers virtual address mapping.
 *        9. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *       10. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_AESCCM_Block(
                              AESCCM_Context_t    *WorkingContextID_ptr,
                              DxUint8_t           *DataIn_ptr,
                              DxUint32_t           DataInSize,
                              DxUint8_t           *DataOut_ptr )
 {
	/* LOCAL DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;

	/* The number of blocks left to load the HW after the first operation */
	DxUint32_t NumOfBlocksToLoadHw;

	/* the virtual address definition */
	DxUint32_t VirtualHwBaseAddr, i;

	/* the updated data out and data in position */
	DxUint8_t *DataInUpdatePos_ptr;
	DxUint8_t *DataOutUpdatePos_ptr;

	/* the number of blocks loaded in the first operation */
	DxUint32_t NumOfLoadedWords = 0, NumOfReadedWords = 0;

    /* is ssdma mode operation */
    DxUint8_t IsSsdmaMode;

    /* full data size */
    DxUint32_t  fullSize;
    DxUint8_t   allDataLoaded =0 ;

	/* FUNCTION LOGIC */


	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* initializing the local data buffer pointers to the values received by the user */
	DataInUpdatePos_ptr  = DataIn_ptr;
	DataOutUpdatePos_ptr = DataOut_ptr;


	/* set the DMA mode if we exceed the minimum blocks required or on CSI mode */

	if ( (DataInSize / 16) >= LLF_AESCCM_MIN_AES_BLOCKS_FOR_DMA_OPERATION ||
	     (DxUint32_t)DataIn_ptr >= DX_CSI || (DxUint32_t)DataOut_ptr >= DX_CSI )

       IsSsdmaMode = DX_TRUE;

    else

       IsSsdmaMode = DX_FALSE;


    /* ............... getting the hardware sempaphore .................... */
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
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}

	/* .............. initializing the hardware ............................ */
	/* --------------------------------------------------------------------- */

	fullSize = DataInSize + WorkingContextID_ptr->lastBlockSize;

	/* perform BlockInit operation for all modes, besides AESCCM mode */
	Error = LLF_AESCCM_BlockInit( WorkingContextID_ptr,      /* the working context id - in */
		                          VirtualHwBaseAddr,         /* the virtual base address - in */
		                          DataOut_ptr,               /* the output buffer - in */
		                          DataIn_ptr,                /* the input buffer - in */
		                          fullSize);                 /* the data size - in */
	if( Error != CRYS_OK )
		goto End;

    /* ---------------------------------------------------------------------- */
	/* for AData: process the data from context lastBlock buffer		      */
	/* ---------------------------------------------------------------------- */

    if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )
    {
		/* since the input and output  buffers are aligned set alignments to 0 */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR , 0x0UL );
        CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR , 0x0UL );

		/* load the DATA_IN registers */
		for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
		{
			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR + (4 * i),
	                                     WorkingContextID_ptr->lastBlock[i] );
		}

	    /* wait for the hardware machine to be ready */
	    LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

        if( DataInSize == 0 )
		goto Finish;
    }


   /* ----------------------------------------------------------------------- */
   /* .............. direct CPU access to input data ........................ */
   /* ----------------------------------------------------------------------- */

   if( !IsSsdmaMode )
	{
	   /* ...........  process small data (size <= 2 AES blocks) ............ */

	    if( DataInSize <= 2*CRYS_AES_BLOCK_SIZE_IN_BYTES )
	    {
	        /* process Adata */
	        if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )
	        {
			    LLF_AESCCM_BlockProcessSmallData(
							                    WorkingContextID_ptr,
							                    VirtualHwBaseAddr,
							                    DataIn_ptr,
							                    DX_NULL,
							                    DataInSize );
	        }

	        else /* process TextData */
	        {
			    LLF_AESCCM_BlockProcessSmallData(
							                    WorkingContextID_ptr,
							                    VirtualHwBaseAddr,
							                    DataIn_ptr,
							                    DataOut_ptr,
							                    DataInSize );
	        }

			DataInSize = 0;

		    goto Finish;
	    }


	    /* .............. executing the first operation ...................... */

        LLF_AESCCM_AlignerInit ( WorkingContextID_ptr,   /* the working context id - in */
                            VirtualHwBaseAddr,           /* the virtual base address - in */
                            0x3,                         /* CPU always work with 0x3*/
                            CRYS_COMMON_BusAlignerShift, /* CPU always work with 0x3*/
                            DataOut_ptr,                 /* the output buffer - in */
                            DataIn_ptr,                  /* the input buffer - in */
                            DataInSize  );               /* input data size */

	    Error = LLF_AESCCM_BlockExecFirstOperation ( WorkingContextID_ptr,
					              VirtualHwBaseAddr,                            /* the virtual base address - in */
					              &DataInUpdatePos_ptr,                         /* the data in updated pointer - in / out */
					              DataInSize,
					              &DataOutUpdatePos_ptr,                         /* the data size - in */
					              &NumOfLoadedWords,                            /* the number of blocks loaded - out */
					              &NumOfReadedWords,
					              &allDataLoaded );

	    if( Error != CRYS_OK )
		goto End;


	    /* .............. executing the block operation ........................ */

	    /* calculate the number of blocks left to load */

	    /*all data was loaded in first block operation*/
	    if(allDataLoaded)
	      goto Finish;

	    /*calc number of not loaded blocks and perform it in the loop*/
	    NumOfBlocksToLoadHw = (DataInSize - NumOfLoadedWords*4) / CRYS_AES_BLOCK_SIZE_IN_BYTES;

	    /*we have to remain one word to */
	    if((NumOfBlocksToLoadHw*CRYS_AES_BLOCK_SIZE_IN_BYTES + NumOfLoadedWords*4) == DataInSize)
	      NumOfBlocksToLoadHw--;



	    /* ....if the number of blocks left to load the hardware is more then 1 call the block functions
	       ....to operate a multi block operation */
	    if( NumOfBlocksToLoadHw > 0 )
	    {
	        /* Adata processing */
	    if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )
	    {
			LLFCD_AES_BlockMacMode(
							DataInUpdatePos_ptr,     /* the data in updated pointer - in */
				                NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_BYTES,  /* the data size to load - in */
							VirtualHwBaseAddr);      /* the virtual address - in */

			/* update the buffer in pointer - we can assumed there are aligned */
			DataInUpdatePos_ptr = DataInUpdatePos_ptr + (NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_BYTES);
			NumOfLoadedWords += NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_WORDS;
			NumOfReadedWords += NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_WORDS ;
	    }

            else  /* Text data processing */
            {

			LLFCD_AES_BlockECBandCBCandCTRModes(
					                DataInUpdatePos_ptr,    /* the data in updated pointer - in */
					                DataOutUpdatePos_ptr,   /* the data out updated pointer - in */
	                                NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_BYTES,  /* the data size to load - in */
					                VirtualHwBaseAddr);     /* the virtual address - in */

			/* update the buffer pointers - we can assumed there are aligned */
			DataInUpdatePos_ptr = DataInUpdatePos_ptr + (NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_BYTES);
			DataOutUpdatePos_ptr = DataOutUpdatePos_ptr + (NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_BYTES);
			NumOfLoadedWords += NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_WORDS;
			NumOfReadedWords += NumOfBlocksToLoadHw * CRYS_AES_BLOCK_SIZE_IN_WORDS ;

            }

	    }/* End of number of blocks grater then 1 case */



   }/* end of the direct mode (CPU mode ) case */


   /* ----------------------------------------------------------------------- */
   /* .............. SDMA  machine access ................................... */
   /* ----------------------------------------------------------------------- */
	else
	{
        LLF_AESCCM_AlignerInit ( WorkingContextID_ptr,   /* the working context id - in */
                            VirtualHwBaseAddr,           /* the virtual base address - in */
                            CRYS_COMMON_BusAlignerMask,  /* DMA works according to the hw configuartion*/
                            CRYS_COMMON_BusAlignerShift, /* DMA works according to the hw configuartion*/
                            DataOut_ptr,                 /* the output buffer - in */
                            DataIn_ptr,                  /* the input buffer - in */
                            DataInSize );

	/* calling the appropriate function according to the mode */

	        /* Adata processing */
	    if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )
	    {
			LLF_AES_DMA_BlockMacMode(
						              DataInUpdatePos_ptr,             /* the data in updated pointer - in */
						              DataInSize,                      /* the data size to load - in */
						              VirtualHwBaseAddr);              /* the virtual address - in */

			/* update the buffer pointers */
			if( (DxUint32_t)DataInUpdatePos_ptr < DX_CSI )
				DataInUpdatePos_ptr = DataInUpdatePos_ptr + DataInSize;

			if( (DxUint32_t)DataOutUpdatePos_ptr < DX_CSI )
				DataOutUpdatePos_ptr = DataOutUpdatePos_ptr + DataInSize;
	    }

            else  /* Text data processing */
            {
				LLF_AES_DMA_BlockECBandCBCandCTRModes(
						              DataInUpdatePos_ptr,        /* the data in updated pointer - in */
						              DataOutUpdatePos_ptr,       /* the data out updated pointer - in */
						              DataInSize,                 /* the data size to load - in */
						              VirtualHwBaseAddr);         /* the virtual address - in */

			/* update the buffer pointers */
			if( (DxUint32_t)DataInUpdatePos_ptr < DX_CSI )
				DataInUpdatePos_ptr = DataInUpdatePos_ptr + DataInSize;
            }
    }/* end of SDMA state machine case */



   /* ................. finishing the block operation ....................... */
   /* ----------------------------------------------------------------------- */
Finish:

	Error = LLF_AESCCM_BlockFinish(
                         WorkingContextID_ptr,           /* the working context id - in */
                         VirtualHwBaseAddr,              /* the virtual base address - in */
                         DataInUpdatePos_ptr,            /* the data in updated pointer - in / out */
                         DataOutUpdatePos_ptr,           /* the data out updated pointer - in / out */
                         DataInSize,                     /* the data size - in */
                         (DxUint32_t)DataIn_ptr & 0x03,  /* the data in alignment - in */
                         (DxUint32_t)DataOut_ptr & 0x03, /* the data out alignment - in */
                         NumOfLoadedWords,
                         NumOfReadedWords,
                         allDataLoaded,
                         IsSsdmaMode );                  /* The SSDMA mode - in */

	if( Error != CRYS_OK )
	{
	goto End;
	}



	/* ................. end of function ..................................... */
	/* ----------------------------------------------------------------------- */

End:

	/* if an error occurred close the Hardware clock */
	if( Error )
	{
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );
	}

	/* .... un mapping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_AES_HW_CRYPTO_ADDR_SPACE );  /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

Return:

	return Error;

 }/* END OF LLF_AESCCM_Block */



/***************************************************************************************/
/**
 * @brief The low level LLF_AES_CCM_BlockInit function initializes the hardware to
 *        operate AES on CCM mode.
 *
 *        The function operates the following steps:
 *
 *        1. Checking the CONTENT register. if the AES is not supported return an error code.
 *        2. Enable the clock of the AES engine by setting the register CLK_ENABLE with
 *           the value 0x1 (AES_EN bit).
 *        3. Set the CRYPTO_CTL register to the AES mode - value 0x1.
 *        4. Set the READ_ALIGN register to the value:
 *             on operation modes ECB, CBC: data_out_ptr % 4 value.
 *             on operation modes MAC, CTR: 0.
 *        5  Set the WRIT_ALIGN register to the value:
 *             for Adata (op.mode MAC): data_in_ptr % 4 value.
 *             for Text data (op. mode tunnel) CTR->MAC: 0.
 *        6. Set the all AES_CONTROL register fields.
 *        7. Loads the IV and CTR counters.
 *        8. Load the AES_KEY_0, AES_KEY_1 registers to the same value.
 *        9. Wait until the AES_BUSY register is ready ( the value is 0 ).
 *           On this stage the initialization of the hardware to operate the AES machine is ended.
 *       10. Exit the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] DataOut_ptr       -  The pointer to the users data output buffer .
 * @param[in] DataIn_ptr        -  The pointer to the users data input buffer .
 * @param[in] DataInSize        -  The size of the data.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AESCCM_BlockInit(
		                       AESCCM_Context_t    *WorkingContextID_ptr,
		                       DxUint32_t           VirtualHwBaseAddr,
		                       DxUint8_t           *DataOut_ptr,
		                       DxUint8_t           *DataIn_ptr,
		                       DxUint32_t           DataInSize )
{

	/* LOCAL DECLARATIONS */

	/* the AES control value */
	DxUint32_t aesControlVal, RegVal;

    /* the pointer to AES context, placed inside the AESCCM context */
    AESContext_t    *AESContext_ptr = (AESContext_t*)WorkingContextID_ptr;


	/* FUNCTION LOGIC */

	/* initializing the AES control value to 0 */
	aesControlVal = 0;
    DataIn_ptr = DataIn_ptr;
	/* ............. AESCCM control register initialization  ................... */
	/* ------------------------------------------------------------------------- */

	/* set AES control fields for AES CCM according to AESCCM data type operations */


	if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )
	{
		/* for additional data processing */
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_MAC_MODE_VAL | \
		                 AES_HW_AES_CTL_PAD_DATA_IN_VAL        | \
		                 LLF_AES_HW_AES_CTL_KEY_0_ENCRYPT_VAL;
	}

	else /*  for text data processing */
	{
	    /* set tunneling CTR->MAC  AES mode fields */
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_CTR_MODE_VAL     | \
		                 LLF_AES_HW_AES_CTL_KEY_1_MAC_MODE_VAL ;

	    /* set control flags dependent on encrypt-decrypt mode */
		if( WorkingContextID_ptr->CCM_EncryptMode == CRYS_AES_Encrypt )
		{
		    /*  for text data encryption */
		aesControlVal |= AES_HW_AES_CTL_TUNNEL_IS_ON_VAL           | \
		                 AES_HW_AES_CTL_PAD_DATA_IN_VAL            | \
		                 AES_HW_AES_CTL_OUT_MID_TUNNEL_DATA_VAL    | \
		                 AES_HW_AES_CTL_TUN_B0_ENCRYPT_VAL         | \
		                 AES_HW_AES_CTL_TUN_B1_USES_PADDED_DATA_IN ;
		}

	else
	{   /*  for text data decryption */
		aesControlVal |= AES_HW_AES_CTL_TUNNEL_IS_ON_VAL         | \
		                 AES_HW_AES_CTL_OUT_MID_TUNNEL_DATA_VAL  | \
		                 AES_HW_AES_CTL_TUNNEL_B1_PAD_EN_VAL     | \
		                 LLF_AES_HW_AES_CTL_KEY_0_DECRYPT_VAL ;
	}
	}


	/* ................. setting the common AES control fields ................. */
	/* ------------------------------------------------------------------------- */

	/* set the KEY size control field */
	switch( WorkingContextID_ptr->AESContext.AESContextKeySize )
	{
		case CRYS_AES_Key128BitSize:
			aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_KEY128_VAL | LLF_AES_HW_AES_CTL_KEY_1_KEY128_VAL;
			break;

		case CRYS_AES_Key192BitSize:
			aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_KEY192_VAL | LLF_AES_HW_AES_CTL_KEY_1_KEY192_VAL;;
			break;

		case CRYS_AES_Key256BitSize:
			aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_KEY256_VAL | LLF_AES_HW_AES_CTL_KEY_1_KEY256_VAL;;
			break;
        default:
            return LLF_AESCCM_INTRNAL_ERROR_1;
	}

    if (WorkingContextID_ptr->AESContext.is_secret_key)
    {
        /* Get AES control register - to find out the SK size is supported */
        CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_FLAGS_ADDR, RegVal );
        /*set the correct value for the RKEK size */

        switch( WorkingContextID_ptr->AESContext.AESContextKeySize )
		{
			case CRYS_AES_Key128BitSize:
				/* This size is always supported */
				break;
			case CRYS_AES_Key192BitSize:
			case CRYS_AES_Key256BitSize:
				/* These sizes supported only if the RKEK is 256 bits length */
				if(RegVal & LLF_AES_HW_HLAG_LRKEK_VAL)
				    break;
				else
                    return LLF_AES_HW_INTERNAL_ERROR_9;
			default:

			    return LLF_AES_HW_INTERNAL_ERROR_9;
		}

    }


	/* ................. general registers initializations ..................... */
	/* ------------------------------------------------------------------------- */

	/* ...... enabling the AES clock */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR ,1UL);

	/*  set the CRYPTO_CTL register to AES mode - the machine is now configured
	    to work on AES */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR ,
                                 LLF_AES_HW_CRYPTO_CTL_AES_MODE_VAL );


	/* .................. AES registers initialization .......................... */
	/* -------------------------------------------------------------------------- */

	/* setting the AES control register with the calculated value */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CTL_ADDR ,
                                 aesControlVal );

	/* only in case of (DataOut == fifo) write this control */
	if ((DxUint32_t)DataOut_ptr >= DX_CSI)
	{
		CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_FIFO_MODE_REG_ADDR, 1UL );
	}

	/* Loading key - either from secret key or from context */
	if( WorkingContextID_ptr->AESContext.is_secret_key)
	{
	#ifdef CRYS_AES_RKEK2_SUPPORT
		/* set the crypto key selection - is_secret key has the same value as the register for RKEK1 or 2 */
		CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_CRYPTOKEY_SEL_REG_ADDR, (DxUint32_t)(WorkingContextID_ptr->AESContext.is_secret_key >> 1));
    #endif
	/* setting TRUE to the secret SK register */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_SK_ADDR, 0x1UL );
	}/* end of initializing the SK mode */

	else
	{
	    /* loading the user's key into Key0 registers for MAC and CTR operations  */
        CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 ,
                                           AESContext_ptr->AESContextKey , CRYS_AESCCM_KEY_SIZE_WORDS );
	}

	/* waiting until the HW machine is enabled */
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

    /* .......................text data processing ...............................*/
    /* -------------------------------------------------------------------------- */
    if( WorkingContextID_ptr->CurrentDataType >= CRYS_AESCCM_FirstTextData )
    {
	    /* loading the user's key into Key1 registers for tunnel MAC operation */
	    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_KEY_1_ADDR_0 ,
	                                       AESContext_ptr->AESContextKey , CRYS_AESCCM_KEY_SIZE_WORDS );

		/* waiting until the HW machine is enabled */
		LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

	/* loading the CTR registers */
	    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_CTR_0_ADDR_0 ,
	                                       WorkingContextID_ptr->CCM_CTR , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );

	/* loading the IV1 registers for MAC operation */
	    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_1_ADDR_0 ,
	                                       WorkingContextID_ptr->CCM_IV , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );

	/* loading remaining size into HW */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_REMAINING_BYTES_ADDR ,
                                     DataInSize );
    }

    /* .......................... Adata processing ...............................*/
    /* -------------------------------------------------------------------------- */
    else
	{
	/* loading the IV0 register on MAC mode */
	    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0 ,
	                                       WorkingContextID_ptr->CCM_IV , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );

	/* loading remaining size into HW */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_REMAINING_BYTES_ADDR ,
                                     DataInSize  );
	}


	/* ................ waiting until the HW machine is enabled ...................*/
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

	return CRYS_OK;

}/* END OF LLF_AES_BlockInit */



/*******************************************************************************************************/
/**
 * @brief The low level LLF_AESCCM_BlockProcessSmallData processeses the data not great than 32 bytes.
 *
 *        The functions major steps:
 *
 *        1) Set the input data into aligned temp buffer and loads it into AES machine.
 *        3) Wait for the AES machine to be ready.
 *        2) Reads the result from the hardware and outputs it.
 *        3) Closes HW and exits.
 *
 * @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataOut_ptr   -  The pointer to the users data output buffer.
 * @param[in/out] DataIn_ptr    -  The pointer to the users data input buffer.
 * @param[in] DataInSize        -  The data in size.
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void  LLF_AESCCM_BlockProcessSmallData (
					                    AESCCM_Context_t      *WorkingContextID_ptr,
					                    DxUint32_t             VirtualHwBaseAddr,
					                    DxUint8_t             *DataIn_ptr,
					                    DxUint8_t             *DataOut_ptr,
					                    DxUint32_t             DataInSize )
{
	/* LOCAL DECLERATIONS */

	/* loop variables */
	DxUint8_t j;

	/* pointer for the aligned input data*/
	DxUint32_t *Data32Bit_ptr = DX_NULL;

	DxUint32_t countBlocksToLoad;


	/* FUNCTION LOGIC */

	/* ................... initializing the local variables .................... */
	/* ------------------------------------------------------------------------- */


    countBlocksToLoad = DataInSize / CRYS_AES_BLOCK_SIZE_IN_BYTES;

    if( DataInSize % CRYS_AES_BLOCK_SIZE_IN_BYTES > 0 )
	countBlocksToLoad++;

	/* since the input and output  buffers are aligned set alignments to 0 */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR , 0x0UL );
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR , 0x0UL );


	/* ------------------------------------------------------------------------- */
	/* set the input data into aligned temp buffer and load it into AES machine  */
	/* ------------------------------------------------------------------------- */

    for( j = 0 ; j < countBlocksToLoad; j++ )
    {
	    /* set aligned pointer to temp buffer */
	    Data32Bit_ptr = WorkingContextID_ptr->lastBlock;

	    if( DataIn_ptr !=  (DxUint8_t*)&WorkingContextID_ptr->lastBlock )
	    {
		    DX_VOS_MemSetZero( WorkingContextID_ptr->lastBlock, CRYS_AES_BLOCK_SIZE_IN_BYTES );
		    DX_VOS_FastMemCpy( WorkingContextID_ptr->lastBlock, DataIn_ptr,
		                       min(CRYS_AES_BLOCK_SIZE_IN_BYTES, DataInSize) );
	    }

        /* load the input data into DATA_IN registers */
	    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
	                                      Data32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );

	    /* wait HW */
		LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

		/*  for Text Data processing read and output the result */
		if( DataOut_ptr != DX_NULL )
		{
	        /* read the data from HW registers and write it into temp buffer  */
		    CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
		                                      Data32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );

		    /* output the result data */
		    DX_VOS_FastMemCpy( DataOut_ptr, WorkingContextID_ptr->lastBlock,
		                       min(CRYS_AES_BLOCK_SIZE_IN_BYTES, DataInSize) );
		}

	    /* update pointers and sizes */
	    if( countBlocksToLoad == 2 )
	    {
		DataIn_ptr += CRYS_AES_BLOCK_SIZE_IN_BYTES;
		DataInSize -= CRYS_AES_BLOCK_SIZE_IN_BYTES;

		if( DataOut_ptr != 0 )
			DataOut_ptr += CRYS_AES_BLOCK_SIZE_IN_BYTES;
	    }
    }

    return;

} /* END of LLF_AESCCM_BlockProcessSmallData */


/*******************************************************************************************************/
/**
 * @brief The low level LLF_AESCCM_BlockExecFirstOperation executes the first hardware operation in order to
 *        solve alignment issues and to prepare the next block in the hardware.
 *
 *        The functions major steps:
 *
 *        1) if the input data is not aligned load the non aligned bytes to the
 *           appropriate DATA_IN register (for all modes).
 *
 *        2) Load the first block from the input buffer to the hardware (DATA_IN).
 *
 *        3) Wait for the AES machine to be ready.
 *
 *        4) On ECB,CBC and CTR if the output data is not aligned read the non aligned bytes and if the
 *           data size is larger then 16 bytes read the entire data block and load the next one to
 *           the hardware.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataOut_ptr   -  The pointer to the users data output buffer it is updated to the
 *                                 exact position after the first operation.
 * @param[in/out] DataIn_ptr    -  The pointer to the users data input buffer it is updated to the
 *                                 exact position after the first operation.
 * @param[in] DataInSize        -  The data in size - the function needs to now if there only one block
 *                                 to operate.
 * @param[out] numOfInBlocksLoadedToHw_ptr - the function sets this parameter to the actual number of input
 *                                           blocks that were loaded to the hardware.
 *                                           This parameter will give the indication if to call the the low level
 *                                           block operation or jump to the finish operation.
 *
 *    Note: Because the the small data processed by LLF_AESCCM_BlockProcessSmallData(), in this
 *          function the DataInSize always is great than 32 bytes.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t  LLF_AESCCM_BlockExecFirstOperation (AESCCM_Context_t    *WorkingContextID_ptr,
                                                 DxUint32_t             VirtualHwBaseAddr,
                                                 DxUint8_t            **DataIn_ptr,
                                                 DxUint32_t             DataInSize,
                                                 DxUint8_t            **DataOut_ptr,
                                                 DxUint32_t             *numOfWordsLoadedToHw_ptr,
                                                 DxUint32_t             *numOfWordsReadedFrHw_ptr,
                                                 DxUint8_t             *allDataLoaded)


{
	/* LOCAL DECLARATIONS */


	/* the alignment variable */
	DxUint8_t data_in_alignment =0;
	DxUint8_t data_out_alignment =0;


	/* loop variable */
	DxUint8_t i;

	/* pointer for the aligned input data*/
	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t*) *DataIn_ptr;

	/* pointer for the aligned output data*/
	DxUint32_t *DataOut32Bit_ptr = (DxUint32_t*)*DataOut_ptr;

	DxUint32_t dataInOutTemp = 0;
	DxUint32_t temp =0, numWordsToLoad =0;

	/* FUNCTION LOGIC */

	/* .............................. initializing the local variables ....................... */
	/* --------------------------------------------------------------------------------------- */

	/* setting the data in alignment */
	data_in_alignment = (DxUint32_t)*DataIn_ptr & 0x03;

	/* setting the data in alignment */
	data_out_alignment = (DxUint32_t)*DataOut_ptr & 0x03;


	/* initialize the number of blocks loaded to the hardware to 0 */
	*numOfWordsLoadedToHw_ptr = 0;

	/* STEP 1: handling the case the data in is not aligned */
	/* --------------------------------------------------------------------------------------- */

	/* on this case we shall load the non aligned bytes to the appropriate non align DATA IN register */

	if( data_in_alignment != 0 )
    {
     for(i = 0; i < sizeof( DxUint32_t ) -  data_in_alignment; i++)

		((DxUint8_t*)&dataInOutTemp)[i +  data_in_alignment] =((DxUint8_t*)DataIn32Bit_ptr)[i];

	/* set the aligned bytes of first input word to the hardware DIN */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, dataInOutTemp);

		/* increment input pointer */
       temp = (DxUint32_t)DataIn32Bit_ptr;
	   temp  += (sizeof( DxUint32_t ) - data_in_alignment);
	   DataIn32Bit_ptr = (DxUint32_t*)temp;

       /* decrease count of words */
	   *numOfWordsLoadedToHw_ptr +=1;
    }



	/* STEP 2. load the first aligned words from the input buffer                    */
	/* ----------------------------------------------------------------------------- */


	/* The first 4 aligned words will be loaded to the hardware and the
	  input buffer pointer will be updated */

    /* load the DATA_IN registers */
    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
                                       DataIn32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );

    /* update the pointer to the new position */
    DataIn32Bit_ptr += CRYS_AES_BLOCK_SIZE_IN_WORDS ;

    /* increment the number of blocks loaded to the hardware */
    (*numOfWordsLoadedToHw_ptr)  = ((*numOfWordsLoadedToHw_ptr) + CRYS_AES_BLOCK_SIZE_IN_WORDS);

    /* wait for the hardware machine to be ready */
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );


    /* in the case of not aligned out buff read first not aligned bytes and write data to complete aes block size*/
	if(data_out_alignment != 0 && WorkingContextID_ptr->CurrentDataType >=CRYS_AESCCM_FirstTextData )
	{
	   dataInOutTemp = 0;

	   /*read not aligned bytes and copy it to output buff, aftre it output buff should be aligned*/
	   CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,dataInOutTemp);

       for(i = 0; i < sizeof( DxUint32_t ) - data_out_alignment; i++)

		   ((DxUint8_t*)DataOut32Bit_ptr)[i] = ((DxUint8_t*)&dataInOutTemp)[i + data_out_alignment];

	   /* increment output pointer */
	   temp = (DxUint32_t)DataOut32Bit_ptr;
	   temp  += (sizeof( DxUint32_t ) - data_out_alignment);
	   DataOut32Bit_ptr = (DxUint32_t*)temp;

		/*update number of readed from HW output words*/
	   *numOfWordsReadedFrHw_ptr = 1;

	   /*check if remaning size is bigger than crys aes block size*/
	   if((DataInSize - (*numOfWordsLoadedToHw_ptr)*sizeof(DxUint32_t)) > CRYS_AES_BLOCK_SIZE_IN_BYTES )
	   {

	   /*if ramaning size is bigger load next block to HW input*/

	  /* load the DATA_IN registers */
       CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
                                       DataIn32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );

       /* update the pointer to the new position */
       DataIn32Bit_ptr += CRYS_AES_BLOCK_SIZE_IN_WORDS ;

       /* increment the number of blocks loaded to the hardware */
       (*numOfWordsLoadedToHw_ptr)  = ((*numOfWordsLoadedToHw_ptr) + CRYS_AES_BLOCK_SIZE_IN_WORDS);

	   }

	   else
	   /*last data block *, we need to perform last block writting to HW*/
	   {
	      /* calc remaining sizes of data */
	     numWordsToLoad = (DataInSize  - (*numOfWordsLoadedToHw_ptr)*sizeof(DxUint32_t) + data_in_alignment)/4;

	     /*if remaning size of data is not multiply of words increment temp flag*/
	     if((DataInSize  - (*numOfWordsLoadedToHw_ptr)*sizeof(DxUint32_t) + data_in_alignment)%4)
	      temp = 1;

	     /* we have not multiply of word size last block, therefore last bytes should be copied
	        byte by byte from temp variable   */
		 if(temp)
	 {
		CRYS_PLAT_SYS_WriteRegistersBlock(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
					                            DataIn32Bit_ptr , numWordsToLoad);

			DataIn32Bit_ptr += numWordsToLoad;

			/* set the WRITE_ALIGN_LAST register */

			CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );

			dataInOutTemp =0;

			for(i = 0; i < (DataInSize  - (*numOfWordsLoadedToHw_ptr)*sizeof(DxUint32_t) + data_in_alignment)%4; i++)
	        {
		       ((DxUint8_t*)&dataInOutTemp)[i] = ((DxUint8_t*)(DataIn32Bit_ptr))[i];
	        }

			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, dataInOutTemp );
	 }

	 else
	 {
		CRYS_PLAT_SYS_WriteRegistersBlock(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
					                            DataIn32Bit_ptr , numWordsToLoad - 1);

			DataIn32Bit_ptr += (numWordsToLoad -1);

			/* set the WRITE_ALIGN_LAST register */

			CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );


			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, DataIn32Bit_ptr[0] );
	 }

	 /**/
	 *allDataLoaded = 1;

	   }
		/* wait for the hardware machine to be ready */
        LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );
	}
      /*update offset of 'in' and 'out' buffers*/
       *DataIn_ptr = (DxUint8_t*)DataIn32Bit_ptr;
       *DataOut_ptr = (DxUint8_t*)DataOut32Bit_ptr ;
	return CRYS_OK;

}/* END OF LLF_AESCCM_BlockExecFirstOperation */



/*********************************************************************************************************/
/**
 * @brief The low level LLF_AESCCM_BlockFinish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataOut_ptr   -  The pointer to the users data output buffer at the exact position.
 * @param[in/out] DataIn_ptr    -  The pointer to the users data input buffer at the exact position.
 * @param[in] DataInAlignment   -  The data in alignment - needed to know how many words to load
 *                                 from the input.
 * @param[in] DataOutAlignment  -  The data out alignment - needed to know how many bytes to load
 *                                 from the last word.
 * @param[i] DataInSize         -  The data in size - the function needs to now if there is only one block
 *                                 to operate. ( this parameter refers to all of the AES data )
 * @param[in] IsSsdmaMode       -  TRUE - the operation is with DMA (BYPASS).
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AESCCM_BlockFinish(
                         AESCCM_Context_t    *WorkingContextID_ptr,    /* in */
                         DxUint32_t           VirtualHwBaseAddr,       /* in */
                         DxUint8_t           *DataIn_ptr ,             /* in/out */
                         DxUint8_t           *DataOut_ptr ,            /* in/out */
                         DxUint32_t           DataInSize,              /* in */
                         DxUint8_t            InAlign,                 /* in */
                         DxUint8_t            OutAlign,                /* in */
                         DxUint32_t           LoadedSize,
                         DxUint32_t           ReadedSize,
                         DxUint8_t            allDataLoaded,
                         DxUint8_t            IsSsdmaMode )            /* in */

{
	/* LOCAL DECLARATIONS */

	/* loop variable */
	DxUint8_t i;

	/* the number of aligned words to load */
	DxUint8_t numWordsToLoad;
	DxUint32_t dataOutTemp = 0;
    DxUint32_t dataInTemp = 0;


	/* FUNCTION LOGIC */
	/*remove lint warning*/
    DataIn_ptr = DataIn_ptr;

    if( DataInSize == 0 )
	goto ReadIV;

    /* --------------------------------------------------------------------------- */
    /* If data input mode is not SSDMA and the block is the last                   */
    /* --------------------------------------------------------------------------- */
	if( !IsSsdmaMode )
	{

	/* since the output buffer is now aligned we can cast it to a 32 bit pointer type */
	DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)DataOut_ptr;

	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)DataIn_ptr;


	    /* remaining sizes of data  */
	    DxUint32_t remainSize;


        /* remaining sizes of data */
	    numWordsToLoad = (DataInSize  - LoadedSize*sizeof(DxUint32_t) + InAlign)/4;

	    /*if last block is not word multiply*/
	    if((DataInSize  - LoadedSize*sizeof(DxUint32_t) + InAlign)%4)
	      dataInTemp = 1;


        /* read 4 words of DIN_DOUT result stated in HW after previous functions,  */
        /* this will be performed for TextData operations only                     */
        /*-------------------------------------------------------------------------*/


        /*read the last block of prev loop iteration*/
        if( WorkingContextID_ptr->CurrentDataType >= CRYS_AESCCM_FirstTextData && (!allDataLoaded))
        {
          /*read last writen block from prev function*/
           CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
	                                       DataOut32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );


           DataOut32Bit_ptr += CRYS_AES_BLOCK_SIZE_IN_WORDS;
           ReadedSize +=CRYS_AES_BLOCK_SIZE_IN_WORDS;
        }



	 /*write last chunk of data*/
	 if(!allDataLoaded)
	 {

	   if(dataInTemp)/*we have not word multiply last chunk*/
	   {
			/*write all whole words*/
			CRYS_PLAT_SYS_WriteRegistersBlock(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
						                            DataIn32Bit_ptr , numWordsToLoad);

				DataIn32Bit_ptr += numWordsToLoad;

				/* set the WRITE_ALIGN_LAST register */

				CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );

				dataInTemp =0;
				/*copy last bytes to temp word and write it to HW*/
				for(i = 0; i < (DataInSize  - LoadedSize*sizeof(DxUint32_t) + InAlign)%4; i++)
		        {
			       ((DxUint8_t*)&dataInTemp)[i] = ((DxUint8_t*)(DataIn32Bit_ptr))[i];
		        }

				CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, dataInTemp );
	   }

	   else /*we have word multiply last chunk*/
	   {
			CRYS_PLAT_SYS_WriteRegistersBlock(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
						                            DataIn32Bit_ptr , numWordsToLoad - 1);

				DataIn32Bit_ptr += (numWordsToLoad -1);

				/* set the WRITE_ALIGN_LAST register */

				CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );



				CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, DataIn32Bit_ptr[0] );
	   }



		   /* wait for the hardware machine to be ready */
	       LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

	   }

	      /*read last output*/

		  if( WorkingContextID_ptr->CurrentDataType >= CRYS_AESCCM_FirstTextData )
          {
	          /*in bytes*/
	          remainSize = DataInSize - ReadedSize*4 + OutAlign;

	          /*number of whole words*/
	          remainSize = remainSize/4;

	         /*we have buffer that not multiply of words*/
	         if((DataInSize - ReadedSize*4 + OutAlign)%4)
	          {
	          /*read only whole words*/
	          CRYS_PLAT_SYS_ReadRegistersBlock(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	                                           DataOut32Bit_ptr, remainSize );

	          DataOut32Bit_ptr += remainSize;


	          /* set the READ_LAST_DATA register */
			  CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_READ_LAST_DATA_ADDR , 0x1UL );

	          CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, dataOutTemp);

	           for(i = 0; i < (DataInSize - ReadedSize*4 + OutAlign)%4; i++)
		       {
			       ((DxUint8_t*)(DataOut32Bit_ptr))[i] = ((DxUint8_t*)&dataOutTemp)[i];
		       }

            }

            else
            {
	           /*we have only whole words*/
	          CRYS_PLAT_SYS_ReadRegistersBlock(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	                                           DataOut32Bit_ptr, remainSize - 1 );

	          DataOut32Bit_ptr += (remainSize -1);

	          /* set the READ_LAST_DATA register */
			  CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_READ_LAST_DATA_ADDR , 0x1UL );

	          CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	                                           DataOut32Bit_ptr[0]);

            }

        }


}
ReadIV:

    /* STEP 3 : read IV (MAC) from HW and save it into AESCCM context:                   */
    /*            for Adata - read IV from AES tunnel 0, for TextData read from tunnel 1 */
	/* --------------------------------------------------------------------------------- */

    if( WorkingContextID_ptr->CurrentDataType <= CRYS_AESCCM_LastAdata )
    {
        CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0 ,
                                           WorkingContextID_ptr->CCM_IV , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );
    }
    else
    {
        CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_1_ADDR_0 ,
                                           WorkingContextID_ptr->CCM_IV , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );
    }

	/* save CTR in the context for TextData processing */
    if( WorkingContextID_ptr->CurrentDataType >= CRYS_AESCCM_FirstTextData )
    {

	    /* read CTR pointer from HW and save it into AESCCM context */
        CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_CTR_0_ADDR_0 ,
                                           WorkingContextID_ptr->CCM_CTR , CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );

    }


	/* STEP 4 : Clear crypto CTL register and close the Hardware Clock */
	/* --------------------------------------------------------------- */

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR , 0x0UL );

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );


	return CRYS_OK;


}/* END OF LLF_AESCCM_BlockFinish */



/***************************************************************************************/
/**
 * @brief The low level LLF_AES_AlignerInit sets alignments for AES operation.
 *
 *
 * @param[in] WorkingContextID_ptr - The AESCCM context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] Mask              -  The mask for alignment setting, depended on count bits for alignment.
 * @param[in] Shift             -  The bits-shifting value.
 * @param[in] DataOut_ptr       -  The pointer to the users data output buffer .
 * @param[in] DataIn_ptr        -  The pointer to the users data input buffer .
 * @param[in] DataInSize        -  The size of the input data in bytes.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void  LLF_AESCCM_AlignerInit (
		                       AESCCM_Context_t  *WorkingContextID_ptr ,
		                       DxUint32_t         VirtualHwBaseAddr ,
		                       DxUint32_t         Mask ,
		                       DxUint32_t         Shift ,
		                       DxUint8_t         *DataOut_ptr,
		                       DxUint8_t         *DataIn_ptr,
		                       DxUint32_t         DataInSize )
{

    /* set CRYPTO_CTL register to AES mode again in order to change alignments */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR ,
                                 LLF_AES_HW_CRYPTO_CTL_AES_MODE_VAL );

   /* setting the write (DIN) alignment register: set for all sizes and alignments */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR,
					            ((DxUint32_t)DataIn_ptr & Mask) |
					            (((((DxUint32_t)DataIn_ptr + DataInSize) & Mask) ) << Shift) );

   /* setting read (DOUT) alignment register */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR,
				(DxUint32_t)DataOut_ptr & Mask );
    return;
}
