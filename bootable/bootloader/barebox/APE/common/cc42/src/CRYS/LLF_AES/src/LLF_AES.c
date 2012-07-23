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
#include "cc_hw_interface.h"
#include "log_output.h"


#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
	//#include "CRYS_EXT_IF_OTF.h"
#endif

#include "CRYS_AES_error.h"

#include "LLF_COMMON.h"
#include "LLF_AES.h"
#include "LLF_AES_error.h"
#include "LLF_AES_HwDefs.h"

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

/* importing the base address of the CRYPTO Cell . this value must be initialized in the
   CRYS_Init function */

extern DxUint32_t PLAT_CryptoCellBaseAddr;
extern DxUint32_t CRYS_COMMON_BusAlignerMask;
extern DxUint32_t CRYS_COMMON_BusAlignerShift;

#ifndef CRYS_NO_GLOBAL_DATA
/* importing the semaphore used for the access to the hardware machine */
extern DxVosSem SemHwAccessId;
#endif

/*	This variable holds the RKEK size in the registers,
	Value 0 - than the RKEK is only 128 bits and there for RKEK operations can be done only with 128 bits.
	Value 1 - than the RKEK is 256 bits and all AES key types can be used
   */
static DxUint32_t LLF_AES_RkekSize;

/******************************************************************************************/
/*************         External and private function prototypes           *****************/
/******************************************************************************************/



/******************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockInit function initializes the hardware to operate on AES
 *        at the user selected modes.
 */
  CRYSError_t  LLF_AES_BlockInit (
                       AESContext_t  *WorkingContextID_ptr ,
                       DxUint32_t       VirtualHwBaseAddr ,
                       DxUint8_t       *DataOut_ptr,
                       DxUint8_t       *DataIn_ptr);

/******************************************************************************************/
/**
 * @brief The low level LLF_AES_AlignerInit function initializes the hardware aligner to operate on AES
 *        at the user selected modes.
 */
  void  LLF_AES_AlignerInit (
                       AESContext_t  *WorkingContextID_ptr ,
                       DxUint32_t       VirtualHwBaseAddr ,
                       DxUint32_t       Mask ,
                       DxUint32_t       Shift ,
                       DxUint8_t       *DataOut_ptr,
                       DxUint8_t       *DataIn_ptr,
					   DxUint32_t       DataInSize );


/*******************************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockExec executes the input of the data into HW and output the
 *        result in CPU to CPU mode.
 *
 */
CRYSError_t  LLF_AES_BlockExec(
                    AESContext_t          *WorkingContextID_ptr,
                    DxUint32_t             VirtualHwBaseAddr,
                    DxUint8_t             *dataIn_ptr,
                    DxUint8_t             *dataOut_ptr,
                    DxUint32_t             dataInSize );

/****************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockFinish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.
 */
CRYSError_t  LLF_AES_BlockFinish(
                         AESContext_t    *WorkingContextID_ptr,    /*in */
                         DxUint32_t       VirtualHwBaseAddr,       /* in */
                         DxUint8_t       *DataOut_ptr );           /* in / out */

/****************************************************************************************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones with the direct DMA on ECB,CBC and CTR modes.
 *
 */
CRYSError_t LLF_AES_DMA_BlockECBandCBCandCTRModes(
			              DxUint8_t            *DataIn_ptr,
			              DxUint8_t            *DataOut_ptr ,
			              DxUint32_t            DataInSize,
			              DxUint32_t           VirtualHwBaseAddr );

/****************************************************************************************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones with the direct DMA on all AES MAC modes.
 *
 */
CRYSError_t LLF_AES_DMA_BlockMacMode(
			              DxUint8_t            *DataIn_ptr,
			              DxUint32_t            DataInSize,
			              DxUint32_t            VirtualHwBaseAddr );


/****************************************************************************************/
/**
 * @brief This function operates the block execution with the multi LLI DMA
 *        on ECB,CBC and CTR modes.
 *
 */
CRYSError_t LLF_AES_BlockEcbCbcCtrModesSepMLLI(
					              DxUint32_t  *DataInLliTab_ptr,        /* the data in LLI table - in */
					              DxUint32_t   InLliTabSize,            /* the in LLI table size - in */
					              DxUint32_t  *DataOutLliTab_ptr,	    /* the data out LLI table - in */
					              DxUint32_t   OutLliTabSize,           /* the out LLI table size - in */
					              DxUint32_t   DataInSize,              /* the data size to load - in */
					              DxUint32_t   VirtualHwBaseAddr);      /* the virtual address - in */

/****************************************************************************************/
/**
 * @brief This function operates the block execution with the multi LLI DMA on all AES MAC modes.
 *
 */
CRYSError_t	LLF_AES_BlockMacModesSepMLLI(
					              DxUint32_t  *DataInLliTab_ptr,        /* the data in LLI table - in */
					              DxUint32_t   InLliTabSize,            /* the in LLI table size - in */
					              DxUint32_t   DataInSize,              /* the data size to load - in */
					              DxUint32_t   VirtualHwBaseAddr);      /* the virtual address - in */

/*******************************************************************************************************/
/**
 * @brief The LLF_AES_ReadOutputBlock function reads and puts block the data out.
 *
 *
 */
void  LLF_AES_ReadOutputBlock(
		                    DxUint32_t           **dataOut32Bit_ptr,
		                    DxUint32_t            *dataSizeWords_ptr,
		                    DxUint32_t             VirtualHwBaseAddr );




/******************************************************************************************/
/*************       External and private functions for special AES modes     *************/
/******************************************************************************************/


/****************************************************************************************/
/**
 * @brief The low level LLF_AES_Xcbc_Init performs specific initializations required
 *        on the used HW or SW platform for XCBC or CMAC modes using ordinary AES_CBC
 *        feature of HW.
 *
 *        On the lite platform currently this function does nothing.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 extern CRYSError_t LLF_AES_Xcbc_Init( AESContext_t  *WorkingContext_ptr );


/****************************************************************************************/
/**
 * @brief The low level LLF_AES_Xts_Init performs specific initializations required
 *        on the used HW or SW platform for XTS mode.
 *
 *        On the lite platform currently this function does nothing.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 extern CRYSError_t LLF_AES_Xts_Init( AESContext_t  *WorkingContext_ptr );




/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_ExecBlockXts processes specific operations for XTS
 *        Block operation.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 extern void  LLF_AES_ExecBlockXts(
                           DxUint32_t **dataIn32Bit_ptr,
                           DxUint32_t **dataOut32Bit_ptr,
                           DxUint32_t  *countWordsToLoad_ptr,
                           DxUint32_t  *countWordsToRead_ptr,
                           DxUint32_t   sizeNotMod4,
                           DxUint32_t   VirtualHwBaseAddr )   ;


/***********************************************************************************************/
/************************           Public Functions               *****************************/
/***********************************************************************************************/


/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_InitInit performs specific initializations required
 *        on the used HW or SW platform for necessary modes .
 *
 *        On this platform this function does nothing.
 *
 * @param[in] WorkingContext_ptr - The AES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_InitInit( AESContext_t  *WorkingContext_ptr )
{
   /* LOCAL DECLARATIONS */

   CRYSError_t Error;


   /* FUNCTION LOGIC */

   Error = CRYS_OK;


   /* call specific LLF_AES_Finish functions according to current AES mode */

   switch( WorkingContext_ptr->OperationMode )
   {
#ifndef CRYS_NO_AES_XCBC_CMAC_MODES_SUPPORT
		case CRYS_AES_XCBC_MAC_mode:

		   Error = LLF_AES_Xcbc_Init( WorkingContext_ptr );

		   break;
#endif

#ifndef CRYS_NO_AES_XTS_MODE_SUPPORT
		case CRYS_AES_XTS_mode:

		   Error = LLF_AES_Xts_Init( WorkingContext_ptr );

		   break;
#endif

		default:

		   break;
   }

   /* .................... end of function .................................... */
   /* ------------------------------------------------------------------------- */

   return Error;

}


/***********************************************************************************************/
/**
 * @brief The low level LLF_AES_Block function operates the AES engine in the hardware.
 *        It calls the following low level functions:
 *
 *        1. The function gets a semaphore to protect the access to the hardware
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address
 *           to the registers virtual address.
 *        3. The function calls the LLF_AES_BlockInit to initialize the AES hardware.
 *        4. The function calls the LLF_AES_BlockExecFirstOperation to execute the first operation.
 *        5. The function calls the level 3 implemented block operation if there is more then 2
 *           block to work on.
 *           If there is 2 block left or less then the function will continue to step 6.
 *
 *             If the operation mode is ECB CBC or CTR the LLFCD_AES_BlockECBandCBCandCTRModes function is called
 *             If the operation mode is MAC, XCBC_MAC or CMAC the LLFCD_AES_BlockMacMode function is called.
 *
 *           These functions as described are level 3 functions implemented according to the major
 *            CPUs in assembler in order to achieve improved performances.
 *
 *        6. If there are 2 blocks to work on the function will call the LLF_AES_BlockSingle function.
 *           If only one block is left it will continue to step 7.
 *
 *        7. The function calls the LLF_AES_BlockFinish function to execute the last block and to save
 *           the IV temporary values for the next block operation and to shut down the hardware machine.
 *        8. The function calls a VOS system call to unmap the registers virtual address mapping.
 *        9. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *       10. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_AES_Block( AESContext_t    *WorkingContextID_ptr,
                              DxUint8_t       *DataIn_ptr,
                              DxUint32_t       DataInSize,
                              DxUint8_t       *DataOut_ptr )
 {
	/* LOCAL DECLARATIONS */

	/* The return error identifier */
	CRYSError_t Error;

	/* the virtual address definition */
	DxUint32_t VirtualHwBaseAddr;

    /* is ssdma mode operation */
    DxUint8_t IsDmaMode;


	/* FUNCTION LOGIC */


	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* set the DMA mode if we exceed the minimum blocks required */

	if ( (DataInSize / CRYS_AES_BLOCK_SIZE_IN_BYTES) >= LLF_AES_MIN_AES_BLOCKS_FOR_DMA_OPERATION)

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

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
                          LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}

	/* .............. initializing the hardware .............................. */
	/* ----------------------------------------------------------------------- */

	/* remaining size of data (set indication for preventing specific operations on last block) */
    if( WorkingContextID_ptr->NotAddBlocks == 0 )
		WorkingContextID_ptr->remainSize = 0xFFFFFFFF;

	Error = LLF_AES_BlockInit ( WorkingContextID_ptr,     /* the working context id - in */
	                           VirtualHwBaseAddr,         /* the virtual base address - in */
	                           DataOut_ptr,               /* the output buffer - in */
	                           DataIn_ptr);               /* the input buffer - in */

	if( Error != CRYS_OK )
	{
		goto End;
	}

   /* ----------------------------------------------------------------------- */
   /* .............. direct CPU state machine ............................... */
   /* ----------------------------------------------------------------------- */

   if( !IsDmaMode )
   {
        /* for all modes, besides AES-CTR */
        if( WorkingContextID_ptr->OperationMode != CRYS_AES_CTR_mode )
        {
	        /* align the IN_OUT buffers of AES machine to the given in-out addresses */
	        LLF_AES_AlignerInit( WorkingContextID_ptr,     /* the working context id - in */
	                             VirtualHwBaseAddr,          /* the virtual base address - in */
	                             0x3,                        /*CPU always work with 0x3*/
	                             CRYS_COMMON_BusAlignerShift,/*CPU always work with 0x3*/
	                             DataOut_ptr,                /* the output buffer - in */
	                             DataIn_ptr,                 /* the input buffer - in */
								 DataInSize );              /* the data size bytes - in */

		    Error = LLF_AES_BlockExec(
					             WorkingContextID_ptr,         /* the working context id - in */
					             VirtualHwBaseAddr,            /* the virtual base address - in */
					             DataIn_ptr,                   /* the data in updated pointer - in / out */
					             DataOut_ptr,                  /* the data out updated pointer - in / out */
					             DataInSize );                 /* the number of blocks loaded - out */

		    if( Error != CRYS_OK )
		    {
			goto End;
		    }
        }

        /* for CTR mode process the full and non full blocks separately */
        else
        {
		    DxUint32_t temp[CRYS_AES_BLOCK_SIZE_IN_WORDS + 1];
		    DxUint32_t remainSize, firstOpSize ;

		    remainSize = DataInSize & 0xF;
		    firstOpSize = DataInSize - remainSize;

	        /* execute full blocks: for CTR if Size > 0, for other modes - always */
	        if( firstOpSize > 0 )
	        {
		        /* align the IN_OUT buffers of AES machine to given in-out addresses */
		        LLF_AES_AlignerInit ( WorkingContextID_ptr,     /* the working context id - in */
		                            VirtualHwBaseAddr,          /* the virtual base address - in */
		                            0x3,                        /*CPU always work with 0x3*/
		                            CRYS_COMMON_BusAlignerShift,/*CPU always work with 0x3*/
		                            DataOut_ptr,                /* the output buffer - in */
		                            DataIn_ptr,                 /* the input buffer - in */
									firstOpSize );              /* the data size bytes - in */

			    Error = LLF_AES_BlockExec(
					              WorkingContextID_ptr,         /* the working context id - in */
					              VirtualHwBaseAddr,            /* the virtual base address - in */
					              DataIn_ptr,                   /* the data in updated pointer - in / out */
					              DataOut_ptr,                  /* the data out updated pointer - in / out */
					              firstOpSize );                /* the number of blocks loaded - out */

			    if( Error != CRYS_OK )
			    {
				goto End;
			    }
	        }


	       /*  executing not full block in CTR mode */
		   if( remainSize > 0 )
		   {
		        LLF_AES_AlignerInit(
		                            WorkingContextID_ptr,        /* the working context id - in */
		                            VirtualHwBaseAddr,           /* the virtual base address - in */
		                            0x3,                         /*CPU always work with 0x3 */
		                            CRYS_COMMON_BusAlignerShift, /*CPU always work with 0x3 */
					                (DxUint8_t*)&temp[0],        /* the data in pointer - in */
					                (DxUint8_t*)&temp[0],        /* the data out pointer - in */
									CRYS_AES_BLOCK_SIZE_IN_BYTES ); /* the data size bytes - in */

	            /* set remain data into temp buffer with the same alignment */
				DX_VOS_FastMemCpy( (DxUint8_t*)&temp[0], DataIn_ptr + firstOpSize, remainSize );

			    Error = LLF_AES_BlockExec(
					              WorkingContextID_ptr,            /* the working context id - in */
					              VirtualHwBaseAddr,               /* the virtual base address - in */
					              (DxUint8_t*)&temp[0],            /* the data in pointer - in */
					              (DxUint8_t*)&temp[0],            /* the data out pointer - in */
					              CRYS_AES_BLOCK_SIZE_IN_BYTES );  /* the number of blocks loaded - out */

			DX_VOS_FastMemCpy(DataOut_ptr + firstOpSize, (DxUint8_t*)&temp[0], remainSize );
		   }
        }

   }/* end of CPU-CPU mode case */


   /* ----------------------------------------------------------------------- */
   /* ..............   DMA machine operation ................................ */
   /* ----------------------------------------------------------------------- */
	else
	{
        LLF_AES_AlignerInit( WorkingContextID_ptr,     /* the working context id - in */
                            VirtualHwBaseAddr,          /* the virtual base address - in */
                            CRYS_COMMON_BusAlignerMask, /*DMA works according to the hw configuartion*/
                            CRYS_COMMON_BusAlignerShift,/*DMA works according to the hw configuartion*/
                            DataOut_ptr,                /* the output buffer - in */
                            DataIn_ptr,                 /* the input buffer - in */
							DataInSize );               /* the data size bytes - in */
       LLF_Alignment_Pointer(DataIn_ptr,DataOut_ptr,DataInSize);

	/* calling the appropriate function according to the mode */
	switch( WorkingContextID_ptr->OperationMode )
	{
		case CRYS_AES_CBC_mode:
		case CRYS_AES_ECB_mode:
		case CRYS_AES_CTR_mode:
		case CRYS_AES_XTS_mode:

			LLF_AES_DMA_BlockECBandCBCandCTRModes(
						              DataIn_ptr,             /* the data in updated pointer - in */
						              DataOut_ptr,            /* the data out updated pointer - in */
						              DataInSize,             /* the data size to load - in */
						              VirtualHwBaseAddr);     /* the virtual address - in */

				break;

		case CRYS_AES_MAC_mode:
            case CRYS_AES_XCBC_MAC_mode:
            case CRYS_AES_CMAC_mode:

			LLF_AES_DMA_BlockMacMode(
						              DataIn_ptr,             /* the data in updated pointer - in */
						              DataInSize,             /* the data size to load - in */
						              VirtualHwBaseAddr);     /* the virtual address - in */

				break;

		default:

		return LLF_AES_HW_INTERNAL_ERROR_2;

      }/* end of SSDMA state machine case */

   }/* end of operation mode switch case */

   /* ................. finishing the block operation ....................... */
   /* ----------------------------------------------------------------------- */

	Error = LLF_AES_BlockFinish(
                         WorkingContextID_ptr,           /* the working context id - in */
                         VirtualHwBaseAddr,              /* the virtual base address - in */
                         DataOut_ptr );                  /* the data out updated pointer - in / out */

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

 }/* END OF LLF_AES_Block */




/*************************************************************************************************/
/**
 * @brief The low level LLF_AES_InitSk initializes the SK using the HASH on the SEED.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM\
 * @param[in] - seedSizeInBytes the size of the seed in bytes
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_InitSk( DxUint8_t *seed_PTR , DxUint16_t rkekSize )
{
	 /* LOCAL DECLARATIONS */
    DxUint32_t    VirtualHwBaseAddr;
    DxUint32_t    RegVal;

	/* FUNCTION LOGIC */

	/* to avoid compilers warnings */
	seed_PTR = seed_PTR;
	rkekSize = rkekSize;

	DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,       /* low address - in */
	             LLF_AES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
	             &VirtualHwBaseAddr );           /* The virtual address - out */

	/* Get AES control register */
	CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + HW_VERSION_REG_ADDR, RegVal );

	/*extract the costumer code from the version*/
	RegVal = (( RegVal & 0xF00 ) >> 0x8);

	/*Old engines does not support AES flags register*/
	if(RegVal == LLF_AES_CUST_CODE_ONE_VAL)
	{
	    LLF_AES_RkekSize = DX_FALSE;
	}
	else if (RegVal == LLF_AES_CUST_CODE_TWO_VAL)
	{     /* support 256 bit secret key*/
	    LLF_AES_RkekSize = DX_TRUE;
	}
	else
	{   /*New engine holds the HW configuration in the HW flags register */
	  /* Get AES control register */
	  CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_FLAGS_ADDR, RegVal );
	   /*set the correct value for the RKEK size */

	  if(RegVal & LLF_AES_HW_HLAG_LRKEK_VAL)
	  {
	    LLF_AES_RkekSize = DX_TRUE;
	  }
	  else
	  {
	    LLF_AES_RkekSize = DX_FALSE;
	  }
	}

   return CRYS_OK;

}/* END OF LLF_AES_InitSk */


/*********************************************************************************************************/
/**
 * @brief The LLF_AES_Finish function is used as finish operation of AES on special modes.
 *
 *    The LLF_AES_Finish function perform AES operations on last block of data.
 *    The function calls appropriate specific LLF AES functions according to AES mode.
 *
 *      1.	Calculates remaining size for specific modes.
 *      2.	If DataSize is enough large, the function calls LLF_AES_Block function for
 *          data size minus remaining size .
 *      3.  Calls appropriate LLF_AES_Finish___ functions according to AES mode.
 *      4.  Exits.
 *
 * @param[in] WorkingContext_ptr - a pointer to the AES context buffer allocated by the user that
 *                            should be the same context that was used on the previous call
 *                            of this session.
 * @param[in] DataIn_ptr -  The pointer to the input buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
 * @return CRYSError_t      - On success CRYS_OK is returned, on failure a
 *                            value MODULE_* CRYS_AES_error.h
 */
 CRYSError_t  LLF_AES_Finish(
                              AESContext_t    *WorkingContext_ptr,
                              DxUint8_t       *DataIn_ptr,
                              DxUint32_t       DataInSize,
                              DxUint8_t       *DataOut_ptr )
 {
   /* FUNCTION DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* local variables and pointers */


   /******************** FUNCTION LOGIC ************************************/

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* on all modes, besides CMAC with total data size = 0 (indication NotAddBlock = 0xFFFFFFFF),
      if DataInSize = 0 or DataIn_ptr = 0, return Error;  */
   if( !(WorkingContext_ptr->OperationMode  == CRYS_AES_CMAC_mode &&
	   WorkingContext_ptr->NotAddBlocks == 0xFFFFFFFF /*means this is first data processing*/) )
   {
		if( DataInSize == 0 )
			return CRYS_AES_DATA_IN_SIZE_ILLEGAL;

		if( DataIn_ptr == DX_NULL  )
			return CRYS_AES_DATA_IN_POINTER_INVALID_ERROR;

   }
   /* temporary XCBC mode finish with size 0 and not multiple of 16 bytes is not allowed */
   if( WorkingContext_ptr->OperationMode  == CRYS_AES_XCBC_MAC_mode &&
       (DataInSize & 0x0F) > 0 )
      return  CRYS_AES_DATA_IN_SIZE_ILLEGAL;

   /* set indication, that the current block is the last */
   WorkingContext_ptr->NotAddBlocks = 1;

   /* remaining size of data (exact value of last block) */
   WorkingContext_ptr->remainSize = DataInSize;

   /* call LLF_AES_Block for final data block */
   Error = LLF_AES_Block(
	                      WorkingContext_ptr,         /* the working context - in */
						  DataIn_ptr,                 /* the input data buffer - in */
						  DataInSize,                 /* the data in size - in */
						  DataOut_ptr );              /* the output data buffer i/o */


   /* on XCBC and CMAC modes output the final MAC result */
   if( WorkingContext_ptr->OperationMode  == CRYS_AES_XCBC_MAC_mode ||
       WorkingContext_ptr->OperationMode  == CRYS_AES_CMAC_mode )
   {
	   DX_VOS_FastMemCpy( DataOut_ptr, WorkingContext_ptr->AESContextIV,
	                      CRYS_AES_BLOCK_SIZE_IN_BYTES );
   }


   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   return Error;



}/* END OF LLF_AES_Finish */




/***************************************************************************************************/
/*******************          Private Functions                      *******************************/
/***************************************************************************************************/

/*******************************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockInit function initializes the hardware to operate on AES at the modes
 *        the user selected.
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
 *             on operation modes ECB, CBC , MAC : data_in_ptr % 4 value.
 *             on operation modes CTR: 0.
 *        6. Set the AES_CONTROL register to the value that is stored in the received context:
 *             WorkingContextIDD_ptr-> AESContextKeySize and the
 *             WorkingContextIDD_ptr-> EncryptDecryptFlag and the
 *             WorkingContextIDD_ptr-> OperationMode.
 *        7. If the operation mode (WorkingContextIDD_ptr-> OperationMode ) is CBC or MAC then
 *           load the IV counters (WorkingContextIDD_ptr-> AESContextIV ) to the AES_IV_0 - AES_IV_3 registers.
 *        8. If the secret key mode not enabled (WorkingContextIDD_ptr-> is_secret_key == 0 )
 *           load the AES_KEY_X registers per the following rules:
 *
 *               a) If the key size is 128 bit (WorkingContextIDD_ptr-> AESContextKeySize == 0 ) then
 *                  load the registers : AES_KEY_0 to AES_KEY_3
 *
 *               b) If the key size is 192 bit (WorkingContextIDD_ptr-> AESContextKeySize == 1 ) then
 *                  load the registers : AES_KEY_0 to AES_KEY_5
 *
 *               c) If the key size is 256 bit
 *                      (WorkingContextIDD_ptr-> AESContextKeySize == 2 ) then load the registers :
 *                      AES_KEY_0 to AES_KEY_7
 *
 *        9. If the secret key is enabled (WorkingContextIDD_ptr->is_secret_key == 1)
 *           perform the following steps:
 *
 *            a) set the AES_SK register with the value 0x1 - the loading of the secret key is starting.
 *
 *            b) Wait until the AES_BUSY register is ready ( the value is 0 ).
 *
 *            c) clear the AES_SK register.
 *
 *       10. Wait until the AES_BUSY register is ready ( the value is 0 ).
 *           On this stage the initialization of the hardware to operate the AES machine is ended.
 *       11. Exit the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] DataOut_ptr       -  The pointer to the users data output buffer .
 * @param[in] DataIn_ptr        -  The pointer to the users data input buffer .
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
	CRYSError_t  LLF_AES_BlockInit (
			                       AESContext_t    *WorkingContextID_ptr,
			                       DxUint32_t       VirtualHwBaseAddr,
			                       DxUint8_t       *DataOut_ptr,
			                       DxUint8_t       *DataIn_ptr )
{
	/* LOCAL DECLARATIONS */

	/* the AES control value */
	DxUint32_t aesControlVal;

    /*buffer for AES HW flags */
    DxUint32_t    RegVal;

	/* key ID and number of key registers to load value */
	DxUint8_t numOfKeyRegsToLoad;
    CRYS_AES_KeySize_t keySizeID;



	/* FUNCTION LOGIC */


	/* ................. setting the AES control variable ...................... */
	/* ------------------------------------------------------------------------- */
     DataIn_ptr = DataIn_ptr;
	/* ....... setting the AES control register according to the values in the AES context */

	/* building the AES control value */
	aesControlVal = 0;

	/*Choose key 0 for all working modes, key 1 is used only for the tunneling mode */
	aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_SELECT_VAL;

    /* set keySize ID */
    if( WorkingContextID_ptr->OperationMode != CRYS_AES_XTS_mode )
    {
	   keySizeID = WorkingContextID_ptr->AESContextKeySize;
    }

    else
    {
	if(WorkingContextID_ptr->AESContextKeySize == CRYS_AES_Key256BitSize )
		keySizeID = CRYS_AES_Key128BitSize;
	else if(WorkingContextID_ptr->AESContextKeySize == CRYS_AES_Key512BitSize )
		keySizeID = CRYS_AES_Key256BitSize;
	else
		return LLF_AES_HW_INTERNAL_ERROR_3;
    }

   /* check SK size if secret key is used */
	if( WorkingContextID_ptr->is_secret_key )
	{
		switch( keySizeID )
		{
			case CRYS_AES_Key128BitSize:
				/* This size is always supported */
				break;
			case CRYS_AES_Key192BitSize:
			case CRYS_AES_Key256BitSize:
				/* These sizes supported only if the RKEK is 256 bits length */
				if(LLF_AES_RkekSize == DX_FALSE)
				{
					return LLF_AES_HW_INTERNAL_ERROR_9;
				}
				break;

			default:

			    return LLF_AES_HW_INTERNAL_ERROR_9;
		}
	}

	/* loading the decrypt-encrypt mode */
	if( WorkingContextID_ptr->EncryptDecryptFlag == CRYS_AES_Encrypt )
	{
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_ENCRYPT_VAL;
	}
	else
	{
	aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_DECRYPT_VAL;
	}

    /* set AES control operation bit according to mode */
	switch( WorkingContextID_ptr->OperationMode )
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

	case CRYS_AES_XCBC_MAC_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_XCBC_MODE_VAL;
		break;

	  case CRYS_AES_CMAC_mode:
	      aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_CMAC_MODE_VAL;
		break;

	case CRYS_AES_CTR_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_CTR_MODE_VAL;
			break;

	case CRYS_AES_XTS_mode:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY_0_XTS_MODE_VAL;
			break;

	default:
		return LLF_AES_HW_INTERNAL_ERROR_4;

	}/* end of loading the mode switch case */

    /* Get AES version register */
    CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + HW_VERSION_REG_ADDR, RegVal );

    /*extract the customer code from the version*/
    RegVal = (( RegVal & 0xF00 ) >> 0x8);

    /*Old engines does not support AES flags register but do support all AES key types*/
    if((RegVal == LLF_AES_CUST_CODE_ONE_VAL) ||(RegVal == LLF_AES_CUST_CODE_TWO_VAL))
    {
        RegVal = LLF_AES_HW_HLAG_192_256_VAL;
    }
    else
    {
        CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_FLAGS_ADDR, RegVal );
    }

	/* loading the KEY size value */
	switch( keySizeID )
	{
	case CRYS_AES_Key128BitSize:
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY128_VAL;
		numOfKeyRegsToLoad = 4;
		break;

	case CRYS_AES_Key192BitSize:
            if(!(RegVal & LLF_AES_HW_HLAG_192_256_VAL))
            {
                return LLF_AES_HW_INTERNAL_ERROR_3;
            }
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY192_VAL;
		numOfKeyRegsToLoad = 6;
		break;

	case CRYS_AES_Key256BitSize:
            if(!(RegVal & LLF_AES_HW_HLAG_192_256_VAL))
            {
                return LLF_AES_HW_INTERNAL_ERROR_3;
            }
		aesControlVal |= LLF_AES_HW_AES_CTL_KEY256_VAL;
		numOfKeyRegsToLoad = 8;
		break;

	default:

	return LLF_AES_HW_INTERNAL_ERROR_3;

	}/* end of setting the KEY size switch case */

	/* ................. general registers initializations ..................... */
	/* ------------------------------------------------------------------------- */

	/* ...... enabling the AES clock */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR ,1UL);

	/* .... setting the CRYPTO_CTL register to AES mode - the machine is now configured to work on AES */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR ,
                                LLF_AES_HW_CRYPTO_CTL_AES_MODE_VAL );

	/* .................. AES registers initialization .......................... */
	/* -------------------------------------------------------------------------- */

	/* setting the register AES control register with the calculated value */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CTL_ADDR ,
                                aesControlVal );

	/* .............. loading the IV registers on CBC, MAC, XTS modes ........... */

    if(WorkingContextID_ptr->OperationMode == CRYS_AES_CBC_mode  ||
       WorkingContextID_ptr->OperationMode == CRYS_AES_MAC_mode  ||
       WorkingContextID_ptr->OperationMode == CRYS_AES_CMAC_mode ||
       WorkingContextID_ptr->OperationMode == CRYS_AES_XCBC_MAC_mode ||
       WorkingContextID_ptr->OperationMode == CRYS_AES_XTS_mode )
	{
	/* loading the registers */
		CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0 ,
		                                   WorkingContextID_ptr->AESContextIV ,
		                                    CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );
	}

	/* ....... if the secret key is enabled trigger the AES_SK register ...... */

	if( WorkingContextID_ptr->is_secret_key)
	{
	#ifdef CRYS_AES_RKEK2_SUPPORT
			/* set the crypto key selection - is_secret key has the same value as the register for RKEK1 or 2 */
			CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_CRYPTOKEY_SEL_REG_ADDR, (DxUint32_t)(WorkingContextID_ptr->is_secret_key >> 1));
    #endif
	/* setting TRUE to the secret SK register */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_SK_ADDR, 0x1UL );


	}/* end of initializing the SK mode */

	else
	/* .............. loading the users key on non secret key mode ........... */
	{
        /* setting FALSE to the secret SK register */
	/* no need to write the LLF_AES_HW_AES_SK_ADDR in case of the no secret key */
	/*CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_SK_ADDR, 0x0UL );*/

	/* loading the Key registers */
		CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_KEY_0_ADDR_0 ,
		                                   WorkingContextID_ptr->AESContextKey , numOfKeyRegsToLoad );

	}/* End of loading the key registers on non secret mode */

	/* ................ waiting until the HW machine is enabled */
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

	/*  if CMAC mode and the block is the last, then set CMAC init register for calculation special keys.
	Note: HW init for last block on XCBC mode temporary is not implemented and performed by SW */
	if( WorkingContextID_ptr->OperationMode == CRYS_AES_CMAC_mode &&
        WorkingContextID_ptr->remainSize != 0xFFFFFFFF )
	{
		CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_CMAC_INIT_REG_ADDR , 0x1);

		/* ................ waiting until the HW machine is enabled ............*/
		LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );
	}

	/* .. on XCBC mode load key2 and key3 into CTR and previous IV registers ...*/
	else if( WorkingContextID_ptr->OperationMode == CRYS_AES_XCBC_MAC_mode )
	{
		CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_CTR_ADDR_0 ,
		                                   WorkingContextID_ptr->SpecificData.XCBC_CMAC_Data.derivedKey2,
		                                   CRYS_AES_BLOCK_SIZE_IN_WORDS );

		CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_PREV_AES_IV_ADDR ,
		                                   WorkingContextID_ptr->SpecificData.XCBC_CMAC_Data.derivedKey3,
		                                   CRYS_AES_BLOCK_SIZE_IN_WORDS );
	}

	/* .... loading the CTR registers on CTR mode (after setting the key) ..... */
	if( WorkingContextID_ptr->OperationMode == CRYS_AES_CTR_mode )
	{
		CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_AES_CTR_ADDR_0 ,
		                                   WorkingContextID_ptr->AESContextIV , CRYS_AES_BLOCK_SIZE_IN_WORDS );
	}

    /* set the remaining bytes */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_REMAINING_BYTES_REG_ADDR,
	                             WorkingContextID_ptr->remainSize );


	return CRYS_OK;

}/* END OF LLF_AES_BlockInit */


/*******************************************************************************************************/
/**
 * @brief The low level LLF_AES_AlignerInit sets alignments for AES operation.
 *
  *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] Mask              -  The mask for alignment setting, depended on count bits for alignment.
 * @param[in] Shift             -  The bits-shifting value.
 * @param[in] DataOut_ptr       -  The pointer to the users data output buffer .
 * @param[in] DataIn_ptr        -  The pointer to the users data input buffer .
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void  LLF_AES_AlignerInit (AESContext_t    *WorkingContextID_ptr ,
                           DxUint32_t       VirtualHwBaseAddr ,
                           DxUint32_t       Mask ,
                           DxUint32_t       Shift ,
                           DxUint8_t       *DataOut_ptr,
                           DxUint8_t       *DataIn_ptr,
						   DxUint32_t       DataSize )
{
    /* set CRYPTO_CTL register to AES mode again in order to change alignments */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR ,
                                 LLF_AES_HW_CRYPTO_CTL_AES_MODE_VAL );

	/* ....... setting the alignment mode */

	switch( WorkingContextID_ptr->OperationMode )
	{
	/* On mode ECB , CBC and CTR the write alignment is according to the data in pointer
                           the read alignment is according to the data out pointer */
	case CRYS_AES_ECB_mode:
	case CRYS_AES_CBC_mode:
	case CRYS_AES_CTR_mode:
	case CRYS_AES_XTS_mode:

		CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR ,((DxUint32_t)DataOut_ptr & Mask));
            CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR ,
                                  (((DxUint32_t)DataIn_ptr & Mask)) | ((((DxUint32_t)DataIn_ptr + DataSize)& Mask)<<Shift) );

        break;

	     /* On MAC the machine does not work with the data out buffer directly
	        therefore the read alignment is 0 the write alignment is according to the data in pointer */
	     case CRYS_AES_MAC_mode:
		 case CRYS_AES_XCBC_MAC_mode:
		 case CRYS_AES_CMAC_mode:

             CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DOUT_READ_ALIGN_ADDR ,0x0UL );
             CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_WRITE_ALIGN_ADDR ,
                              (((DxUint32_t)DataIn_ptr & Mask)) | ((((DxUint32_t)DataIn_ptr + DataSize)& Mask)<<Shift) );

         break;

	default:

		return ;

	}/* end of operation mode switch */
}



/*******************************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockExec executes the input of the data into HW and output the
 *        result in CPU to CPU mode.
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
 *        4) On ECB,CBC and CTR if the output data is not aligned read the non alined bytes and if the
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
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t  LLF_AES_BlockExec(
                    AESContext_t          *WorkingContextID_ptr,
                    DxUint32_t             VirtualHwBaseAddr,
                    DxUint8_t             *dataIn_ptr,
                    DxUint8_t             *dataOut_ptr,
                    DxUint32_t             dataInSize )
{
	/* LOCAL DECLARATIONS */

	/* the alignment variable */
	DxUint32_t inAlignment, outAlignment;
	DxUint32_t outStartAlignBits, outEndAlignBits;

	/* pointer for the aligned data*/
	DxUint32_t *dataIn32Bit_ptr;
	DxUint32_t *dataOut32Bit_ptr;

	/* count of words and size of data, whicn is not mudulo of 4 bytes */
    DxUint32_t countWordsToLoad, countWordsToRead, sizeNotMod4;

	/* first and last words of output buffer before processing */
	DxUint32_t firstWord, lastWord;

	/* mask for correction of output words */
	DxUint32_t mask;


	/* FUNCTION LOGIC */

	/* .............................. initializing the local variables ....................... */
	/* --------------------------------------------------------------------------------------- */

	/* setting the data in and out alignments */
	inAlignment  = ((DxUint32_t)dataIn_ptr)  & 0x03;
	outAlignment = ((DxUint32_t)dataOut_ptr) & 0x03;

    /* count of words (with consideration of alignment), remaining to
       load into HW and to read from */
    countWordsToLoad = (dataInSize + inAlignment + 3) / 4;
    countWordsToRead = (dataInSize + outAlignment + 3) / 4;


	/* set 32-bit input and output pointers to appropriate nearest lesser addresses */
	dataIn32Bit_ptr  = (DxUint32_t*)(dataIn_ptr - inAlignment);
	dataOut32Bit_ptr = (DxUint32_t*)(dataOut_ptr - outAlignment);

	/* set output alignments in bits */
	outStartAlignBits = outAlignment << 3;
	outEndAlignBits = ((outAlignment + dataInSize) & 3) << 3;

	/* size of data, which is not modulo of AES Block (16 bytes) */
    sizeNotMod4 = dataInSize & 0xF;

    /* save the first and last aligned down words of output buffer */
    firstWord = dataOut32Bit_ptr[0];
    lastWord  = dataOut32Bit_ptr[countWordsToLoad - 1];


	/* if input data size == 0: for AES-CMAC or AES-XCBC_MAC modes do */
	/* special processing, for other modes - return an error          */
	/* -------------------------------------------------------------- */
	if( dataInSize == 0 )
	{
	    /* Note: temporary total data size 0 is allowed only on CMAC mode */
		if( WorkingContextID_ptr->OperationMode == CRYS_AES_CMAC_mode )
		{
			/* write LLF_AES_CMAC_SIZE0_KICK_REG_ADDR register to start calculation */
			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_CMAC_SIZE0_KICK_REG_ADDR, 0x1UL );;

	        /* polling on AES_Busy == 0  */
	        LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

			return CRYS_OK;
		}

		else
		    return CRYS_AES_DATA_IN_SIZE_ILLEGAL;
	}


	/* write the first not aligned word into DIN                      */
	/* -------------------------------------------------------------- */
	if( inAlignment != 0 )
    {
	/* set the aligned bytes of first input word to the hardware DIN */
	CRYS_PLAT_SYS_WriteRegister(
								VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
								*dataIn32Bit_ptr );

		/* increment input pointer */
		dataIn32Bit_ptr++;

		/* decrease count of words */
		countWordsToLoad--;
    }

#ifndef  CRYS_NO_AES_XTS_MODE_SUPPORT
    /*-----------------------------*/
    /*      XTS mode               */
    /*-----------------------------*/
    if( WorkingContextID_ptr->OperationMode == CRYS_AES_XTS_mode )
    {
        LLF_AES_ExecBlockXts(
                   &dataIn32Bit_ptr,
                   &dataOut32Bit_ptr,
                   &countWordsToLoad,
                   &countWordsToRead,
                   sizeNotMod4,
                   VirtualHwBaseAddr );

    } /* end XTS */

    /*-----------------------------------*/
    /*  other AES modes (besides XTS )   */
    /*-----------------------------------*/
    else
#endif
    {
	    /*----------------------------------------------------------------*/
	    /* main loop: load data in HW and read result                     */
	    /*----------------------------------------------------------------*/
	    while( 1 )
	    {
			    if( countWordsToLoad > CRYS_AES_BLOCK_SIZE_IN_WORDS ||
			        (countWordsToLoad == CRYS_AES_BLOCK_SIZE_IN_WORDS && sizeNotMod4 == 0) )
			    {
				    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
				                                       dataIn32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );

				countWordsToLoad -= CRYS_AES_BLOCK_SIZE_IN_WORDS;
				dataIn32Bit_ptr  += CRYS_AES_BLOCK_SIZE_IN_WORDS;
			    }

			    else
			    {
				    if( countWordsToLoad > 0 )
				    {
					    CRYS_PLAT_SYS_WriteRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
					                                       dataIn32Bit_ptr , countWordsToLoad - 1 );

					/* set the WRITE_ALIGN_LAST register */
	                    if( sizeNotMod4 != 0 )
						CRYS_PLAT_SYS_WriteRegister(
							           VirtualHwBaseAddr + LLF_AES_HW_WRITE_ALIGN_LAST_REG_ADDR , 0x1UL );

						CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
						                             dataIn32Bit_ptr[countWordsToLoad - 1] );

				        dataIn32Bit_ptr  += countWordsToLoad;
					countWordsToLoad = 0;
				    }
			    }

	        /* polling on AES_Busy == 0  */
	        LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

		    /* if AES mode is not one of MAC modes, then read current output block */
		    if( WorkingContextID_ptr->OperationMode != CRYS_AES_MAC_mode   &&
		        WorkingContextID_ptr->OperationMode != CRYS_AES_CMAC_mode  &&
		        WorkingContextID_ptr->OperationMode != CRYS_AES_XCBC_MAC_mode )
		    {
			    /* polling on isDoutFifoEmpty == 0 */
			    LLF_AES_HW_WAIT_ON_DOUT_FIFO_EMPTY_BIT( VirtualHwBaseAddr );

			    /* read the current block  */
		        LLF_AES_ReadOutputBlock(
				                    &dataOut32Bit_ptr,
				                    &countWordsToRead,
				                    VirtualHwBaseAddr );
		    }

		   /* if all data is processed, then exit from the function */
	       if( countWordsToLoad == 0 )
			goto EndLoop;
	}
     }

EndLoop:
     if( WorkingContextID_ptr->OperationMode != CRYS_AES_MAC_mode   &&
         WorkingContextID_ptr->OperationMode != CRYS_AES_CMAC_mode  &&
         WorkingContextID_ptr->OperationMode != CRYS_AES_XCBC_MAC_mode )
	 {

	    /*-------------------------------------------------------------*/
	    /* if present additional word, then read it into output buffer */
	    /*-------------------------------------------------------------*/

	    if( countWordsToRead == 1 )
	    {
		/* set the READ_LAST_DATA register */
		CRYS_PLAT_SYS_WriteRegister(
			           VirtualHwBaseAddr + LLF_AES_HW_READ_LAST_DATA_ADDR , 0x1UL );

		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
		                            dataOut32Bit_ptr[0] );

	        dataOut32Bit_ptr++;
	        countWordsToRead--;
	    }


	    /*----------------------------------------------------------------------------------*/
	    /* if data is not aligned, then correct the first and last output words, if needed  */
	    /*----------------------------------------------------------------------------------*/

	    /* restore count of words and set aligned output pointer to begin of the buffer */
	    countWordsToRead = (dataInSize + outAlignment + 3) / 4;
	    dataOut32Bit_ptr -= countWordsToRead;

	    /* correct the first word of output */
		if( outStartAlignBits != 0 )
		{
			mask = 0xFFFFFFFF << outStartAlignBits;
			dataOut32Bit_ptr[0] = (dataOut32Bit_ptr[0] & mask ) | (firstWord & ~mask);
		}

	    /* correct the last word of output */
	    if( outEndAlignBits != 0 )
	    {
		mask = 0xFFFFFFFF << outEndAlignBits;
			dataOut32Bit_ptr[countWordsToRead - 1] =
			       (dataOut32Bit_ptr[countWordsToRead - 1] & ~mask ) | (lastWord & mask);
	    }
	 }

	return CRYS_OK;

}/* END OF LLF_AES_BlockExec */



/*******************************************************************************************************/
/**
 * @brief The LLF_AES_ReadOutputBlock function reads and puts block the data out.
 *
 *
 * @param[out] dataOut32Bit_ptr  -  The pointer to the aligned pointer to current position on output buffer.
 * @param[in] dataSizeWords_ptr  -  The pointer to the size of data in words (including not full words).
 * @param[in] VirtualHwBaseAddr  -  The hardware virtual base address.
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void  LLF_AES_ReadOutputBlock(
		                    DxUint32_t           **dataOut32Bit_ptr,
		                    DxUint32_t            *dataSizeWords_ptr,
		                    DxUint32_t             VirtualHwBaseAddr )
{

	/* LOCAL DECLARATIONS */

	DxUint32_t i;



	/* FUNCTION LOGIC */


    /* read HW result and download it into output buffer */
    /*---------------------------------------------------*/

    if( *dataSizeWords_ptr >= 4 )
    {
        /* download 4 words from HW directly into output buffer */
		 CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	                                      *dataOut32Bit_ptr , CRYS_AES_BLOCK_SIZE_IN_WORDS );

        *dataOut32Bit_ptr += CRYS_AES_BLOCK_SIZE_IN_WORDS;
        *dataSizeWords_ptr -= CRYS_AES_BLOCK_SIZE_IN_WORDS;
    }
    else
    {
	volatile DxUint32_t temp;

	 /* download valid words from HW directly into output buffer */
		 CRYS_PLAT_SYS_ReadRegistersBlock( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	                                      *dataOut32Bit_ptr , *dataSizeWords_ptr );

         /* dummy read for complete 4 words */
         for( i = 0; i < 4 - *dataSizeWords_ptr; i++ )

		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR, temp );

         *dataOut32Bit_ptr += *dataSizeWords_ptr;
         *dataSizeWords_ptr =  0;
    }

    return;

}  /* End of LLF_AES_ReadOutputBlock */



/*********************************************************************************************************/
/**
 * @brief The low level LLF_AES_BlockFinish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.
 *
 * @param[in] WorkingContextID_ptr - The AES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataOut_ptr   -  The pointer to the users data output buffer at the exact position.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_AES_BlockFinish(
                         AESContext_t    *WorkingContextID_ptr,    /*in */
                         DxUint32_t       VirtualHwBaseAddr,       /* in */
                         DxUint8_t       *DataOut_ptr )            /* in / out */

{
	/* LOCAL DECLARATIONS */

	/* loop variable */
	DxUint8_t i;


	/* FUNCTION LOGIC */


	/* STEP 1 : On all MAC, CBC and XTS modes store the updated IV on the context */
	/* -------------------------------------------------------------------------- */
	if( WorkingContextID_ptr->OperationMode == CRYS_AES_MAC_mode    ||
        WorkingContextID_ptr->OperationMode == CRYS_AES_XCBC_MAC_mode ||
        WorkingContextID_ptr->OperationMode == CRYS_AES_CMAC_mode   ||
	WorkingContextID_ptr->OperationMode == CRYS_AES_CBC_mode    ||
	WorkingContextID_ptr->OperationMode == CRYS_AES_XTS_mode )

	{
	for( i = 0 ; i < CRYS_AES_IV_COUNTER_SIZE_IN_WORDS ; i++ )
	{
		/* read the MAC result from the Hardware and save it in the context*/
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_IV_0_ADDR_0 + (i * 4) ,
                                     WorkingContextID_ptr->AESContextIV[i]);
	}
	}/* end of storing the IV on the context on MAC modes */


	/* STEP 2 : On MAC modes save the previous IV counter  */
	/* -------------------------------------------------------------- */

    /* On MAC mode output data the block */
    if( WorkingContextID_ptr->OperationMode == CRYS_AES_MAC_mode  )
	{
         /* copy the MAC result from context IV to the out put buffer */
	     DX_VOS_FastMemCpy( DataOut_ptr , (DxUint8_t *)WorkingContextID_ptr->AESContextIV , CRYS_AES_IV_COUNTER_SIZE_IN_BYTES );
	}


	/* STEP 3 : On CTR mode store the updated CTR on the context */
	/* also reset skip register if enabled                       */
	/* ---------------------------------------------------------------- */

	if( WorkingContextID_ptr->OperationMode == CRYS_AES_CTR_mode )
	{
	for( i = 0 ; i < CRYS_AES_IV_COUNTER_SIZE_IN_WORDS ; i++ )
	{
		/* read the updated counter from the Hardware */
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CTR_ADDR_0 + (i * 4) ,
                                        (WorkingContextID_ptr->AESContextIV)[i]);
	}

	}/* end of storing the counter on the context on CTR modes */


	/* STEP 4 : Clear crypto CTL reg */
	/* --------------------------------- */

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR , 0x0UL );

	/* STEP 5 : Close the Hardware Clock */
	/* --------------------------------- */

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );


	return CRYS_OK;

}/* END OF LLF_AES_BlockFinish */



/*********************************************************************************************************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones with the direct DMA on ECB,CBC and CTR modes.
 *
 *        This function should be operated in ECB or CBC mode.
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exact data size to operate the AES with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t LLF_AES_DMA_BlockECBandCBCandCTRModes(
							              DxUint8_t            *DataIn_ptr,
							              DxUint8_t            *DataOut_ptr ,
							              DxUint32_t            DataInSize,
							              DxUint32_t            VirtualHwBaseAddr )
{
  /* LOCAL DECLARATION */

  /* initialize the error identifier as OK */
  CRYSError_t Error = CRYS_OK;


  /* FUNCTION LOGIC */

  /* ................. executing the DMA ........................... */

	/* initialize the destination LLI address */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DST_LLI_WORD0_ADDR, (DxUint32_t)DataOut_ptr );

	/* initialize the destination size and set the first & last LLI word */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_DST_LLI_WORD1_ADDR, 
	                               DataInSize |
	                               (1UL << LLF_AES_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS) | 
	                               (1UL << LLF_AES_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS) ); 

	/* initialize the source LLI address */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_SRC_LLI_WORD0_ADDR, (DxUint32_t)DataIn_ptr );

	/* initialize the source size and set the first & last LLI word - this triggers the operation */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_SRC_LLI_WORD1_ADDR, 
	                               DataInSize |
	                               (1UL << LLF_AES_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS) | 
	                               (1UL << LLF_AES_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS) );

  /* STEP2 : wait on the AES machine and DMA */
/*DX_VOS_Printf("DataSize =%x\n",DataInSize);
 DX_VOS_Printf("DataSize =%d\n",DataInSize);
  Error = CC_Hw_InterFace_Map_And_Start_DMA(DataOut_ptr,DataInSize,VirtualHwBaseAddr,DST_LLI);
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
	LLF_AES_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr );

	/* wait for the hardware machine to be ready */
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

	/* wait wait on the DMA DOUT  */
	LLF_AES_HW_WAIT_ON_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr );


err_end:
  return Error;


}/* END OF LLF_AES_DMA_BlockECBandCBCModes */


/*********************************************************************************************************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones with the direct DMA on all AES MAC modes.
 *
 *        This function should be operated in MAC mode.
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataInSize  - The exact data size to operate the AES with this function.( does not include the
 *                          first and finish operations )
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t LLF_AES_DMA_BlockMacMode(
					              DxUint8_t            *DataIn_ptr,
					              DxUint32_t            DataInSize,
					              DxUint32_t            VirtualHwBaseAddr )
{
  /* LOCAL DECLERATION */

  /* initialize the error identifier as OK */
  CRYSError_t Error = CRYS_OK;


  /* FUNCTION LOGIC */

  /* .................... initialize local variables ...................... */

  /* STEP1 : initialize the LLI */
  /* TBD - currently supporting only the SINGLE LLI - need to support multi LLI - using the VOS */

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_SRC_LLI_WORD0_ADDR , DataIn_ptr );

	/* initialize the source size and set the first & last LLI word - this triggers the operation */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_SRC_LLI_WORD1_ADDR ,
	                               DataInSize |
	                               (1UL << LLF_AES_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS) |
	                               (1UL << LLF_AES_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS) );

	/* wait on the DMA DIN */
	LLF_AES_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr );

  /* STEP2 : wait until the AES machine busy */

  /* wait for the hardware machine to be ready */
  LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );



  return Error;

}/* END OF LLF_AES_DMA_BlockMacMode */


/****************************************************************************************/
/**
 * @brief These functions are used by the rnd for time optimizations.
 *
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 * @param[in] DinDout_ptr - The pointer to the input-output buffer passed by the user.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void LLF_AES_OptAesBlock(DxUint32_t VirtualHwBaseAddr,
                           DxUint32_t *DinDout_ptr)
 {
    DxUint32_t i;

     /* load the DATA_IN registers */
    for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
    {
          CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR + (4 * i),
                                      DinDout_ptr[i] );
    }
    /* wait for AES operation to end */
	LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

    /* loading the words to the out buffer */
	for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS; i++ )
	{
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR + (4 * i) ,
                                       DinDout_ptr[i]);
    }

    return;
}

/****************************************************************************************/
/**
 * @brief These functions are used by the rnd for time optimizations.
 *
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
void LLF_AES_CloseAesEngine(DxUint32_t VirtualHwBaseAddr)
{
	/* Clear crypto CTL reg */
	/* --------------------------------- */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_CRYPTO_CTL_ADDR , 0x0UL );

	/* STEP 6 : Close the Hardware Clock */
	/* --------------------------------- */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_AES_HW_AES_CLK_ENABLE_ADDR , 0x0UL );
    return;
}
