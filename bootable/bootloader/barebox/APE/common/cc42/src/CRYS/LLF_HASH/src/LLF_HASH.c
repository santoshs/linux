
  /*
   *  Object %LLF_HASH.c    : %
   *  State           :  %state%
   *  Creation date   :  Mon Nov 29 14:10:40 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_HASH.c#1:csrc:4
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

#include "DX_VOS_Mem.h"

#include "DX_VOS_Sem.h"
#ifdef CRYS_SEP_SIDE_WORK_MODE
/* SEP side needs a basic synchronixzation between the interrupts */
#include "sep_sync.h"
#endif /*CRYS_SEP_SIDE_WORK_MODE*/
#include "DX_VOS_Memmap.h"
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
//#include "CRYS_EXT_IF_OTF.h"
#endif
#include "LLF_HASH.h"
#include "LLF_HASH_error.h"
#include "LLF_HASH_HwDefs.h"
#include "LLFCD_HASH.h"
#include "LLF_COMMON.h"
#include "log_output.h"


/************************ Defines ******************************/

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */

/* canceling the lint warning:
   suspisious pointer conversion */
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


/************************ Definitions ******************************/

/*********************************************************************************************/
/************************   Private function prototypes   ************************************/
/*********************************************************************************************/


/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_Init function initializes the hardware to operate on HASH at the modes
 *        the user selected.
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 CRYSError_t  LLF_HASH_Init (
                       HASHContext_t  *WorkingContextID_ptr ,
                       DxUint32_t       VirtualHwBaseAddr );

/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_UpdateExecFirstOperation executes the first hardware operation in order to
 *        solve alignment issues and to prepare the next block in the hardware.
 *
 *        The functions major steps:
 *
 *        1) load the first block from the previous update data block on the context.
 *
 *        2) Wait for the HASH machine to be ready.
 *
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

 CRYSError_t  LLF_HASH_UpdateExecFirstOperation ( HASHContext_t  *WorkingContextID_ptr,
									                    DxUint32_t      VirtualHwBaseAddr);


/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_UpdateExecAlignmentOperation solve alignment issues
 *		   for the next HASH bolcks.
 *        The functions major steps:
 *
 *        1) if the input data is not aligned load the non aligned bytes to the
 *           apropiate DATA_IN register.
 *
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataIn_ptr    -  The pointer to the users data input buffer it is updated
 *                                 to the exact position after the first operation.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

static CRYSError_t  LLF_HASH_UpdateExecAlignmentOperation (
                           DxUint32_t             VirtualHwBaseAddr,
                           DxUint8_t            **DataIn_ptr );



/*********************************************************************************************/
/**
 * @brief This function operates the HASH blocks execution on the DMA or CSI mode.
 *
 *
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *
 * @param[in] DataInSize  - The exect data size to operate the HASH with this function.( does not include reamin
 *                         (if any) for the next operation )
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t LLF_HASH_UpdateExecBlocksWithDMA(
              DxUint8_t            *DataIn_ptr,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr );


/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_UpdateFinish reads the result from the HASH_Hi registers
 *        loads it to the context , and closes the hardware clock it is called from the 2 main functions:
 *        LLF_HASH_Update , LLF_HASH_Finish
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_HASH_UpdateFinish(  HASHContext_t *WorkingContextID_ptr,
								DxUint32_t       VirtualHwBaseAddr);


/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_FinishUpdateLastData executes the last data and the padding.
 *        it is called from the LLF_Finish function
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

static CRYSError_t LLF_HASH_FinishUpdateLastData(	HASHContext_t *WorkingContextID_ptr,
								DxUint32_t VirtualHwBaseAddr );


/*********************************************************************************************************/
/**
 * @brief This function operates the HASH update execution with the multi LLI DMA.
 *
 *        This function operates the Multi LLI DMA and HASH mashine.
 *
 * @param[in] DataInLliTab_ptr - The pointer to input LLI table.
 * @param[in] InLliTabSize - The size of the input LLI table in words. Must be aligned and placed in SEP SRAM.
 * @param[in] DataInSize  - The data size to operate the AES with this function ( does not include the
 *                          first and finish operations ).
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  CRYSError_t LLF_HASH_UpdateExecBlocksWithMLLI(
					              DxUint32_t  *DataInLliTab_ptr,
					              DxUint32_t   InLliTabSize,
					              DxUint32_t   DataInSize,
					              DxUint32_t   VirtualHwBaseAddr);



/**********************************************************************************************/
/************************              Public Functions          ******************************/
/**********************************************************************************************/


/**
 * @brief The low level LLF_HASH_InitContext function initializes the low level context according
 *        to the current low level settings. This function is usually called from the CRYS_HASH_Init function
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t   LLF_HASH_InitContext(HASHContext_t  *WorkingContextID_ptr)
{

	/* initialize the counter bits */
	WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[0] = 0;
	WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[1] = 0;
	WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[2] = 0;
	WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[3] = 0;

	return CRYS_OK;
}


/**********************************************************************************************/
/**
 * @brief The low level LLF_HASH_Update function operates the HASH engine in the hardware.
 *        It calls the following low level functions:
 *
 *        The function flow:
 *
 *
 * @param[in] WorkingContextID_ptr -     The HASH context allocated by the CCM
 * @param DataIn_ptr a pointer to the buffer that stores the data to be
 *                       hashed .
 * @param DataInSize  The size of the data to be hashed in bytes.
 * @return CRYSError_t on success the function returns CRYS_OK else non ZERO error.
 *
 */

CRYSError_t LLF_HASH_Update( HASHContext_t  *WorkingContextID_ptr,
		                     DxUint8_t      *DataIn_ptr,
		                     DxUint32_t      DataInSize )
{
   /* LOCAL DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* The number of blocks left to load the HW after the first operation */
   DxUint32_t NumOfBlocksToLoadHw;

   /* the virtual address definition */
   DxUint32_t VirtualHwBaseAddr;

   /* the updated data in position */
   DxUint8_t *DataInUpdatePos_ptr;

   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */
   PRINTF("LLF_HASH_Update function entered \n\n");

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   /* initializing the local data buffer pointers to the values received by the user */
   DataInUpdatePos_ptr  = DataIn_ptr;

   /* ............... getting the hardware sempaphre ..................... */
   /* -------------------------------------------------------------------- */

   Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

   if( Error != CRYS_OK )

     goto Return;

   /* ............... mapping the physical memory to the virtual one ...... */
   /* --------------------------------------------------------------------- */

   Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_HASH_HW_CRYPTO_ADDR_SPACE,  /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

   if( Error != CRYS_OK )

     goto ReturnAndReleaseSemaphore;


   /* .............. initializing the hardware .............................. */
   /* ----------------------------------------------------------------------- */

   Error = LLF_HASH_Init ( WorkingContextID_ptr, /* the working context id - in */
                           VirtualHwBaseAddr );  /* the virtual base address - in */

   //PRINTF("LLF_HASH_Init function ended*****\n\n");

   if( Error != CRYS_OK )

     goto End;

   if( (DxUint32_t)DataIn_ptr < DX_CSI )
   {
	   /* .............. executing the first operation .......................... */
	   /* .... NOTE: there is only execution of the saved block in the context .. */
	   /* ----------------------------------------------------------------------- */

	   Error = LLF_HASH_UpdateExecFirstOperation (
	                  WorkingContextID_ptr,                        /* the working context id - in */
	                  VirtualHwBaseAddr);                           /* the virtual base address - in */
        //   PRINTF("LLF_HASH_UpdateExecFirstOperation function ended*****\n\n");

	   if( Error != CRYS_OK )

	     goto End;
   }

   /* .............. executing the block operation .......................... */
   /* ----------------------------------------------------------------------- */


   /* calculating the number of full blocks we can load the hardware */
	if(WorkingContextID_ptr->OperationModeBlockSizeInBytes == CRYS_HASH_BLOCK_SIZE_IN_BYTES)
	{
		NumOfBlocksToLoadHw = (DataInSize >> 6);
	}
	else
	{
		NumOfBlocksToLoadHw = (DataInSize >> 7);
	}
    /* aligned the data to multiplication of blocks */
    DataInSize = DataInSize - (DataInSize & (WorkingContextID_ptr->OperationModeBlockSizeInBytes - 1));
	/*....if there is many blocks of data or data transfering mode is CSI, use DMA (or CSI) to process the data */
   if( (NumOfBlocksToLoadHw >= LLF_HASH_MIN_HASH_BLOCKS_FOR_DMA_OPERATION ) ||
	   ((DxUint32_t)DataIn_ptr >= DX_CSI) )
   {
               PRINTF("Entered DMA mode\n");
		LLF_HASH_UpdateExecBlocksWithDMA(
					DataInUpdatePos_ptr,
			DataInSize,
			VirtualHwBaseAddr );
               PRINTF("LLF_HASH_UpdateExecBlocksWithDMA function ends\n\n");
   }

   /* if there no many data to process, use direct loading of data into HASH mashine */
   else if( NumOfBlocksToLoadHw > 0 )
   {
		/*set the alignment before start to process the reset of data*/
		Error = LLF_HASH_UpdateExecAlignmentOperation (
		              VirtualHwBaseAddr,                    /* the virtual base address - in */
		              &DataInUpdatePos_ptr );               /* the data in updated pointer - in / out */

                PRINTF("LLF_HASH_UpdateExecAlignmentOperation function ends\n\n");

		if( Error != CRYS_OK )

		  goto End;

		if(WorkingContextID_ptr->OperationMode != CRYS_HASH_SHA384_mode &&
		   WorkingContextID_ptr->OperationMode != CRYS_HASH_SHA512_mode )
		{
			LLFCD_HASH_Update(
		              DataInUpdatePos_ptr,											 /* the data in updated pointer - in */
		              DataInSize, /* the data size to load - in */
		              VirtualHwBaseAddr );											 /* the virtual address - in */

                       PRINTF("LLFCD_HASH_Update function completed\n");
		}
		else
		{

		#ifndef  LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
		/* in case of SHA-384 or SHA-512 */
		LLFCD_HASH_SHA2_Update(
			              DataInUpdatePos_ptr,											 /* the data in updated pointer - in */
			              DataInSize, /* the data size to load - in */
			              VirtualHwBaseAddr );
        #endif
		}


   }/* end of more then one hardware block to load */

   /* ................. finishing the update operation ..................... */
   /* ----------------------------------------------------------------------- */

   Error = LLF_HASH_UpdateFinish(
              WorkingContextID_ptr,           /* the working context id - in */
              VirtualHwBaseAddr);             /* the virtual base address - in */
   PRINTF("LLF_HASH_UpdateFinish completed\n\n");

   if( Error != CRYS_OK )

      goto End;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   End:

   /* if an error occored close the Hardware clock */
   if( Error )

      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CLK_ENABLE_ADDR , 0x0UL );


   /* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
   DX_VOS_MemUnMap( &VirtualHwBaseAddr,              /* virtual address - in */
                    LLF_HASH_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

   ReturnAndReleaseSemaphore:

   /* release the hardware semaphore */
   DX_VOS_SemGive ( SemHwAccessId );

   Return:

   return Error;

}/* END OF LLF_HASH_Update */


/************************************************************************************************/
/**
 * @brief The low level LLF_HASH_Finish executes the last data block stored in the previous update
 *        context buffer plus the padding data stored in the context.
 *
 * @param[in] WorkingContextID_ptr -     The HASH context allocated by the CCM
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_HASH_Finish( HASHContext_t *WorkingContextID_ptr )
{
   /* LOCAL DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* the virtual address definition */
   DxUint32_t VirtualHwBaseAddr;

   /* loop counter */
   DxUint8_t  i;


   /* FUNCTION LOGIC */

   /* initializing the Error to O.K */
   Error = CRYS_OK;

   //PRINTF("LLF_HASH_Finish function starts\n");

   /* ............... getting the hardware sempaphre ..................... */
   /* -------------------------------------------------------------------- */

   Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

   if( Error != CRYS_OK )

     goto Return;

   /* ............... mapping the physical memory to the virtual one ...... */
   /* --------------------------------------------------------------------- */

   Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_HASH_HW_CRYPTO_ADDR_SPACE,  /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

   if( Error != CRYS_OK )

     goto ReturnAndReleaseSemaphore;



   /* .............. initializing the hardware .............................. */
   /* ----------------------------------------------------------------------- */


   Error = LLF_HASH_Init ( WorkingContextID_ptr, /* the working context id - in */
                           VirtualHwBaseAddr );  /* the virtual base address - in */

   //PRINTF("LLF_HASH_Init function ended\n\n");

   if( Error != CRYS_OK )

     goto End;

   /* .............. executing the finish ................................... */
   /* ----------------------------------------------------------------------- */

   /* call LLF updating last data operation. We assume that HW supports padding */

   Error = LLF_HASH_FinishUpdateLastData( WorkingContextID_ptr , VirtualHwBaseAddr );

   if( Error != CRYS_OK )
   {
     goto End;
   }


   /* ................. finishing the operation ............................. */
   /* ----------------------------------------------------------------------- */

   Error = LLF_HASH_UpdateFinish(
              WorkingContextID_ptr,           /* the working context id - in */
              VirtualHwBaseAddr);             /* the virtual base address - in */

   //PRINTF("LLF_HASH_UpdateFinish function completed\n");

   if( Error != CRYS_OK )

      goto End;


	/* preparing the final result (big- or little-endian accroding to the operation mode */
   if( WorkingContextID_ptr->OperationMode != CRYS_HASH_MD5_mode )
   {
      DxUint32_t tempUint;

      for( i = 0 ; i < CRYS_HASH_SHA512_DIGEST_SIZE_IN_WORDS; i++ )
      {

          WorkingContextID_ptr->HASH_Result[i] =
               CRYS_COMMON_INVERSE_UINT32_BYTES( WorkingContextID_ptr->HASH_Result[i] );


	      if((WorkingContextID_ptr->OperationMode == CRYS_HASH_SHA384_mode || WorkingContextID_ptr->OperationMode == CRYS_HASH_SHA512_mode) && i%2)
	      {
			tempUint = WorkingContextID_ptr->HASH_Result[i - 1];
				WorkingContextID_ptr->HASH_Result[i - 1] = WorkingContextID_ptr->HASH_Result[i];
				WorkingContextID_ptr->HASH_Result[i] = tempUint;
	      }
      }
   }

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   End:

   /* if an error occored close the Hardware clock */
   if( Error )

      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CLK_ENABLE_ADDR , 0x0UL );


   /* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
   DX_VOS_MemUnMap( &VirtualHwBaseAddr,              /* virtual address - in */
                    LLF_HASH_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

   ReturnAndReleaseSemaphore:

   /* release the hardware semaphore */
   DX_VOS_SemGive ( SemHwAccessId );

   Return:

   return Error;

}/* END OF LLF_HASH_Finish */




/**********************************************************************************************/
/************************ Private Functions ***************************************************/
/**********************************************************************************************/


/**********************************************************************************************/
/**
 * @brief The low level LLF_HASH_Init function initializes the hardware to operate on HASH at the modes
 *        the user selected.
 *
 *        The function operates the follwing steps:
 *
 *        1. Checking the CONTENT register. if the HASH is not supported return an error code.
 *        2. Enable the clock of the HASH engine by setting the register CLK_ENABLE with
 *           the value 0x4 (HASH_EN bit).
 *        3. Set the CRYPTO_CTL register to the HASH mode - value 0x7.
 *        4. Wait until the HASH_BUSY register is ready ( the value is 0 ).
 *           On this stage the initialization of the hardware to operate the HASH machine is enabled.
 *        5. Set the HASH_CONTROL register to the value that is stored in the received context:
 *             WorkingContextIDD_ptr-> OperationMode.
 *        6. Wait until the HASH_BUSY register is ready ( the value is 0 ).
 *           On this stage the initialization of the hardware to operate the HASH machine is enabled.
 *        7. Initialize the HASH_H0 - HASH_H4 registes.
 *        9. Wait until the HASH_BUSY register is ready ( the value is 0 ).
 *           On this stage the initialization of the hardware to operate the HASH machine is ended.
 *       10. Exit the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  CRYSError_t  LLF_HASH_Init (
                       HASHContext_t  *WorkingContextID_ptr ,
                       DxUint32_t      VirtualHwBaseAddr )
{
   /* LOCAL DECLARATIONS */

   /* the HASH control value */
   DxUint32_t hashControlVal;

   /* the hash parameters register */
   DxUint32_t hashParam;
   DxUint32_t endianess;


   /* FUNCTION LOGIC */

   /* ................. check the version & content ........................... */
   /* ------------------------------------------------------------------------- */
   //PRINTF("LLF_HASH_Init function starts\n\n");


   /*check that the right SHA version is enabled */
   CRYS_PLAT_SYS_ReadRegister(VirtualHwBaseAddr + LLF_HASH_HW_HASH_PARAM ,
                              hashParam);

   //PRINTF("VirtualHwBaseAddr = %X", VirtualHwBaseAddr);
   //PRINTF("VirtualHwBaseAddr = %d", VirtualHwBaseAddr);
   /* SHA1 and SHA256 are default, check if the SHA512 is enabled , if needed */
   if( ( WorkingContextID_ptr->OperationMode == CRYS_HASH_SHA512_mode ||
         WorkingContextID_ptr->OperationMode == CRYS_HASH_SHA384_mode ) &&
         !(hashParam & LLF_HASH_HW_HASH_SHA2_PRS_VAL))
   {
		return  LLF_HASH_HW_SHA_VERSION_NOT_CORRECT_ERROR;
   }

   /* ................. general registers initializations ..................... */
   /* ------------------------------------------------------------------------- */

   /* ......... enabeling the HASH clock */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CLK_ENABLE_ADDR , 0x1UL );
   //PRINTF("Write Register*************\n\n");
   //PRINTF("VirtualHwBaseAddr value ====%u\n\n",VirtualHwBaseAddr);

   /* setting the CRYPTO_CTL register to HASH mode - the machine is now configured to work on HASH */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_CRYPTO_CTL_ADDR ,
                                LLF_HASH_HW_CRYPTO_CTL_HASH_MODE_VAL );

   /* initialize the data in alignment - data out is alwase aligned to 0 since it is not in use
      and data in is aligned in first operation */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_WRITE_ALIGN_ADDR ,0);
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DOUT_READ_ALIGN_ADDR ,0);
   //PRINTF(" Data aligning completed\n\n");

   /* .................. HASH registers initialization ......................... */
   /* -------------------------------------------------------------------------- */

   /* ....... setting the HASH control register according to the values in the HASH context */

   /* building the HASH control value and setting endianess through temp register*/
   hashControlVal = 0;

   /* set default endianess value 1 (littl)  */
   endianess = 1;

   switch(WorkingContextID_ptr->OperationMode)
   {
	    case CRYS_HASH_MD5_mode:
		hashControlVal |= LLF_HASH_HW_HASH_CTL_MD5_VAL;
		/* set endianess to 0 (big) */
		endianess = 0;
			break;

		case CRYS_HASH_SHA1_mode:
		hashControlVal |= LLF_HASH_HW_HASH_CTL_SHA1_VAL;
			break;

		case CRYS_HASH_SHA224_mode:
		case CRYS_HASH_SHA256_mode:
			hashControlVal |= LLF_HASH_HW_HASH_CTL_SHA256_VAL;
			break;

		case CRYS_HASH_SHA384_mode:
		case CRYS_HASH_SHA512_mode:

		  /* in case HASH mode is SHA512 or SHA384, and they are are not supported,
		     return an error */
		  #ifdef  LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
			return LLF_HASH_SHA512_SHA384_NOT_SUPPORTED_ERROR;

		  #else
			hashControlVal |= LLF_HASH_HW_HASH_CTL_SHA512_VAL;
			break;
		  #endif

		default:
			break;
   }

   /* setting the register HASH control register with the calculated value */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CONTROL_ADDR ,
                                hashControlVal );

   //PRINTF("Setting HASH control register completed\n\n");

   /* setting endianes mode to HW register */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_ENDIANESS_REG_ADDR ,
                                endianess );


   /* initializing the init HASH values from context */
   switch(WorkingContextID_ptr->OperationMode)
   {
        case CRYS_HASH_MD5_mode: /* order of writing H for MD5 is different from other modes */

               CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
							WorkingContextID_ptr->HASH_Result[0]);

               CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
							WorkingContextID_ptr->HASH_Result[3]);

               CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
							WorkingContextID_ptr->HASH_Result[2]);

               CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
							WorkingContextID_ptr->HASH_Result[1]);
			break;

		case CRYS_HASH_SHA1_mode:

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_LO ,
							WorkingContextID_ptr->HASH_Result[4]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[3]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[2]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[1]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[0]);

			break;


		case CRYS_HASH_SHA224_mode:
		case CRYS_HASH_SHA256_mode:

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H7_REG_LO ,
							WorkingContextID_ptr->HASH_Result[7]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H6_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[6]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H5_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[5]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[4]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[3]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
							WorkingContextID_ptr->HASH_Result[2]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[1]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[0]);


			   //PRINTF("Setting SHA256 mode registers completed\n\n");
			break;


	 #ifndef  LLF_HASH_SHA384_SHA512_NOT_SUPPORTED

		case CRYS_HASH_SHA384_mode:
		case CRYS_HASH_SHA512_mode:

		       CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H7_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[15]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H7_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[14]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H6_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[13]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H6_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[12]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H5_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[11]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H5_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[10]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[9]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[8]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[7]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[6]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[5]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[4]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[3]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[2]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[1]);

			   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[0]);
			break;
	   #endif

		default:
			break;
   }

   /* enable padding */
   CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HASH_HW_HASH_PAD_EN,
							   LLF_HASH_HW_HASH_PAD_EN_VAL);

   //PRINTF("Enabling padding completed\n\n");

   /* load the current length of message being processed  - load high and low word for all cases */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_0 ,
                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[0] );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_1 ,
                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[1] );

   //PRINTF("Loading curent length of message completed\n");

 #ifndef  LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
   if( WorkingContextID_ptr->OperationMode == CRYS_HASH_SHA384_mode ||
       WorkingContextID_ptr->OperationMode == CRYS_HASH_SHA512_mode )
   {
	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_2 ,
	                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[2] );

	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_3 ,
	                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[3] );
   }
 #endif

   //PRINTF("BUSY BIT wait starting\n");
   LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr );
   //PRINTF("BUSY BIT wait completed without execution\n");

   return CRYS_OK;

}/* END OF LLF_HASH_Init */




/**********************************************************************************************/
/**
 * @brief The low level LLF_HASH_UpdateExecFirstOperation executes the first hardware operation
 *        in order to solve HASH-blocks completeon and to prepare the next block in the hardware.
 *
 *        The functions major steps:
 *
 *        1) load the first block from the previous update data buffer from the context.
 *        2) Wait for the HASH machine to be ready.
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t  LLF_HASH_UpdateExecFirstOperation ( HASHContext_t       *WorkingContextID_ptr,
									                    DxUint32_t           VirtualHwBaseAddr )
{
   /* LOCAL DECLERATIONS */

   /* loop variable */
   DxUint8_t i;

   /* loop condition variable */
   DxUint16_t block_size;

   /* FUNCTION LOGIC */


   //PRINTF("LLF_HASH_UpdateExecFirstOperation function starts\n\n");
   /* STEP 1: loading the first packet from the previous update data buffer in the context
              we can assume that the entire block is full and aligned */
   /* --------------------------------------------------------------- */

   block_size = WorkingContextID_ptr->OperationModeBlockSizeInBytes / 4;

   for( i = 0 ; i < block_size ; i++ )

      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_ADDR,
                                   WorkingContextID_ptr->DataFromThePreviousUpdateBuff[i] );



   //PRINTF(" Loading first packet from prev data buf\n");
   /* clear the number of bytes in the buffer */
   WorkingContextID_ptr->NumOfBytesOnThePreviousUpdateBuff = 0;

   /* STEP 2: wiat for the HASH machine */
   /* --------------------------------------------------------------- */
   //PRINTF("Waiting on busy Bit in LLF_HASH_UpdateExecFirstOperation\n");
   LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr );
  // PRINTF("Completed on busy Bit in LLF_HASH_UpdateExecFirstOperation\n");



   return CRYS_OK;

}/* END OF LLF_HASH_UpdateExecFirstOperation */



/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_UpdateExecAlignmentOperation solve alignment issues
 *		   for the next HASH blocks.
 *
 *
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataIn_ptr    -  The pointer to the users data input buffer it is updated
 *                                 to the exact position after the first operation.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

static CRYSError_t  LLF_HASH_UpdateExecAlignmentOperation (
													DxUint32_t             VirtualHwBaseAddr,
													DxUint8_t		**DataIn_ptr)
{

   /* LOCAL DECLARATIONS */

   /* a variable containing the input non aligned bytes */
   DxUint32_t non_aligned_bytes;

   /* the alignment variable */
   DxUint8_t data_in_alignment;

   /* loop variable */
   DxUint8_t i;

   PRINTF("LLF_HASH_UpdateExecAlignmentOperation function starts\n\n");

   /* .............................. initializing the local variables ....................... */
   /* --------------------------------------------------------------------------------------- */

   /* setting the data in alignment */
   data_in_alignment = (DxUint32_t)*DataIn_ptr & 0x03;

   /* STEP 1: initializing the alignment and handeling the case data in is not aligned */
   /* -------------------------------------------------------------------------------- */

   /* setting the CRYPTO_CTL register to HASH mode - the machine is now configured to work on HASH - we need to
      initialize this once again in order to change the aligner configuration */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_CRYPTO_CTL_ADDR ,
                                LLF_HASH_HW_CRYPTO_CTL_HASH_MODE_VAL );

   /* initialize the data in alignment - data out is alwase aligned to 0 since it is not in use */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_WRITE_ALIGN_ADDR ,
                                data_in_alignment | data_in_alignment << CRYS_COMMON_BusAlignerShift );
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DOUT_READ_ALIGN_ADDR ,0);

   /* on this case we shall load the non aligned bytes to the appropiate non align DATA IN register */
   if( data_in_alignment )
   {
      /* the number of data bytes to load */
      DxUint8_t numOfDataBytesToLoad = sizeof( DxUint32_t ) - data_in_alignment;

      /* the value we shall load to the ANALIGN register is :
         non alignment 1 : | dataIn[2] | dataIn[1] | DataIn[0] | X |
         non alignment 2 : | dataIn[1] | dataIn[0] | X | X |
         non alignment 3 : | dataIn[0] | X | X | X | */

      /* init the non aligned bytes variable to 0 */
      non_aligned_bytes = 0;

      /* load the non aligned bytes from the data in pointer to the
         non aligned variable. */
      for( i = 0 ; i < numOfDataBytesToLoad ; i++ )

          non_aligned_bytes |= ((DxUint32_t)( (*DataIn_ptr)[i] << (8 * i) ) );

      /* move the non aligned byte result to start from the MSB */
      non_aligned_bytes <<= (8 * data_in_alignment);

      /* set the aligned bytes to the hardware register */
      CRYS_PLAT_SYS_WriteRegister(
          VirtualHwBaseAddr + LLF_HASH_HW_DIN_ADDR,
          non_aligned_bytes );

      /* set the data in pointer to the aligned value */
      *DataIn_ptr = *DataIn_ptr + numOfDataBytesToLoad;

   }/* end of handeling the non alignemnt case */

   return CRYS_OK;

}/* END OF LLF_HASH_UpdateExecAlignmentOperation */


/**********************************************************************************************/
/**
 * @brief The low level LLF_HASH_UpdateFinish reads the result from the HASH_Hi registers
 *        loads it to the context , and closes the hardware clock.
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
 CRYSError_t  LLF_HASH_UpdateFinish(HASHContext_t *WorkingContextID_ptr,    /* in */
					                      DxUint32_t     VirtualHwBaseAddr)       /* in */
{
   /* LOCAL DECLARATIONS */

     //PRINTF("LLF_HASH_UpdateFinish function started\n");
   /* FUNCTION LOGIC */

   /* STEP 1 : reading the data stored in the hin registers */
   /* ----------------------------------------------------- */

   /* reading the temprorary Hi values */
   switch(WorkingContextID_ptr->OperationMode)
   {
        case CRYS_HASH_MD5_mode:

                CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
							WorkingContextID_ptr->HASH_Result[0]);

                CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
							WorkingContextID_ptr->HASH_Result[3]);

                CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
							WorkingContextID_ptr->HASH_Result[2]);

                CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
							WorkingContextID_ptr->HASH_Result[1]);
             break;

		case CRYS_HASH_SHA1_mode:

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_LO ,
							WorkingContextID_ptr->HASH_Result[4]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[3]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[2]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[1]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[0]);

			   break;

		case CRYS_HASH_SHA224_mode:
		case CRYS_HASH_SHA256_mode:

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H7_REG_LO ,
							WorkingContextID_ptr->HASH_Result[7]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H6_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[6]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H5_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[5]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[4]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[3]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
							WorkingContextID_ptr->HASH_Result[2]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[1]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[0]);

			//   PRINTF("Fetching the HASH result from the registers completed\n\n");
			break;

#ifndef  LLF_HASH_SHA384_SHA512_NOT_SUPPORTED

		case CRYS_HASH_SHA384_mode:
		case CRYS_HASH_SHA512_mode:

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H7_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[15]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H7_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[14]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H6_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[13]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H6_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[12]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H5_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[11]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H5_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[10]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[9]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H4_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[8]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[7]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H3_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[6]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[5]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H2_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[4]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[3]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H1_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[2]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_HI ,
			                                WorkingContextID_ptr->HASH_Result[1]);

			   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_H0_REG_LO ,
			                                WorkingContextID_ptr->HASH_Result[0]);

			break;
#endif

		default:
			break;
   }

   /* load the current length of message being processed  - load high and low word for all cases */
   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_0 ,
                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[0] );

   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_1 ,
                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[1] );

#ifndef LLF_HASH_SHA384_SHA512_NOT_SUPPORTED
   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_2 ,
                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[2] );

   CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CUR_LEN_3 ,
                                WorkingContextID_ptr->TotalDataSizeProcessed_128Bit[3] );


 #endif

   /* STEP 2 : Do busy loop for the last block */
   /* --------------------------------- */
   LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr );
   //PRINTF("BUSY BIT loop completed\n\n");


   /* STEP 3 : Close the Hardware Clock */
   /* --------------------------------- */

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_HASH_CLK_ENABLE_ADDR , 0x0UL );

   return CRYS_OK;

}/* END OF LLF_HASH_UpdateFinish */



/*********************************************************************************************/
/**
 * @brief The low level LLF_HASH_FinishUpdateLastData executes the last data and triggers the padding in the hardware.
 *
 * @param[in] WorkingContextID_ptr - The HASH context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

static CRYSError_t LLF_HASH_FinishUpdateLastData( HASHContext_t *WorkingContextID_ptr,
                                                  DxUint32_t VirtualHwBaseAddr )
{
   /* LOCAL DECLERATIONS */

   /* a variable used to define the number of data words to load */
   DxUint32_t numOfWordsToLoad;

   /* the number non aligned bytes in the last word */
   DxUint32_t numNonAlignBytesInLastWord;

   /* loop variable */
   DxUint32_t i;


   /*     FUNCTION LOGIC   */


   /* setting the CRYPTO_CTL register to HASH mode - the machine is now configured to work on HASH -
      we need to initialize this once again in order to change the aligner configuration */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_CRYPTO_CTL_ADDR ,
                                LLF_HASH_HW_CRYPTO_CTL_HASH_MODE_VAL );

   /* if there is data in context previous update buffer, put it in the HASH mashine */

   if( WorkingContextID_ptr->NumOfBytesOnThePreviousUpdateBuff != 0 )
   {
	   /* calculate the number of rounded up data words left to load  */
	   numOfWordsToLoad = ( WorkingContextID_ptr->NumOfBytesOnThePreviousUpdateBuff / sizeof(DxUint32_t));

	   /* calculate number of non aligned bytes in last word */
	   numNonAlignBytesInLastWord = WorkingContextID_ptr->NumOfBytesOnThePreviousUpdateBuff & 0x3;

	   /* initialize the data in alignment: start=0, end = numNonAlignBytesInLastWord;
	      set also data out alignment to 0 since output is aligned */
	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_WRITE_ALIGN_ADDR ,
	                                numNonAlignBytesInLastWord << CRYS_COMMON_BusAlignerShift );

	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DOUT_READ_ALIGN_ADDR , 0 );



	   /* set numOfWordsToLoad words of data to the DATA_IN registers */
	   for( i = 0 ; i < numOfWordsToLoad; i++ )
	   {
	      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_ADDR,
	                                   WorkingContextID_ptr->DataFromThePreviousUpdateBuff[i] );
	   }

	   if( numNonAlignBytesInLastWord )
	   {
			/* set WRITE_ALIGN_LAST register before writing last word */
			CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HASH_HW_DIN_WRITE_ALIGN_LAST_ADDR, 0x1 );

			/* write last word of data */
			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_ADDR,
			                            WorkingContextID_ptr->DataFromThePreviousUpdateBuff[i] );
	   }
   }

   /* wait for the machine to signal the end of the HASH operation */
   LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr );

   /* do HW padding by writing 1 to HW_HASH_PAD_CFG register */
   CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HASH_HW_HASH_PAD_CFG,
							   LLF_HASH_HW_HASH_DO_PADDING_VAL);

   /* wait for the machine to signal the end of the HASH operation */
   LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr );

   /* disable padding */
   CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_HASH_HW_HASH_PAD_EN,
							   0UL);

   return CRYS_OK;

}/* END OF LLF_HASH_FinishUpdateLastData */



/*********************************************************************************************/
/**
 * @brief This function operates the HASH blocks execution on the DMA or CSI mode.
 *
 *
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *
 * @param[in] DataInSize  - The exect data size to operate the HASH with this function.( does not include reamin
 *                         (if any) for the next operation )
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t LLF_HASH_UpdateExecBlocksWithDMA(
              DxUint8_t            *DataIn_ptr,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{
	/* LOCAL DECLARATION */

	/* initialize the error identifier as OK */
	CRYSError_t Error = CRYS_OK;

	/* data alignement variables */
	DxUint32_t DataInAlignemnt;
         DxUint32_t  Data_P;
        int i;
        DxUint8_t *DataIn_ptr1;

	/* FUNCTION LOGIC */

	/* .................... initialize local variables ...................... */
	DataInAlignemnt  = (DxUint32_t)DataIn_ptr & CRYS_COMMON_BusAlignerMask;

       PRINTF("LLF_HASH_UpdateExecBlocksWithDMA function starts\n\n");



	/* STEP 1: initializing the alignment and handeling the case data in is not aligned */
	/* -------------------------------------------------------------------------------- */

	/* setting the CRYPTO_CTL register to HASH mode - the machine is now configured to work on HASH - we need to
      initialize this once again in order to change the aligner configuration */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_CRYPTO_CTL_ADDR ,
                                LLF_HASH_HW_CRYPTO_CTL_HASH_MODE_VAL );

	/* initialize the data in alignment - data out is alwase aligned to 0 since it is not in use */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DIN_WRITE_ALIGN_ADDR ,
	                             DataInAlignemnt | DataInAlignemnt << CRYS_COMMON_BusAlignerShift );

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_DOUT_READ_ALIGN_ADDR ,0);


	/* .................... executing the DMA ............................... */

	/* STEP2 : initialize the LLI */
   if( (DxUint32_t)DataIn_ptr < DX_CSI )
  {
      Data_P = (DxUint32_t)DataIn_ptr & 0xFFFF0000;
           DX_VOS_Printf("DataIn_ptr = %08x\n",DataIn_ptr);
           DX_VOS_Printf("Data_P = %08x\n",Data_P);
           //Datap_32 = (DxUint32_t)DataIn_ptr1;
           DataIn_ptr1 = Data_P +0x10000;
           DX_VOS_Printf("DataIn_ptr1 = %08x\n",DataIn_ptr1);
          for(i=0;i<DataInSize;i++){
          *(DataIn_ptr1+i*sizeof(DxUint8_t)) = *(DataIn_ptr+i*sizeof(DxUint8_t));

          }
	   /*CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_SRC_LLI_WORD0_ADDR ,
	                             (DxUint32_t)DataIn_ptr );*/
           CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_SRC_LLI_WORD0_ADDR ,
                                     (DxUint32_t)DataIn_ptr1 );
	   /* initialize the source size and set the first & last LLI word - this triggers the operation */
	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_HASH_HW_SRC_LLI_WORD1_ADDR ,
                               DataInSize  |
                               (1UL << LLF_HASH_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS) |
                               (1UL << LLF_HASH_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS) );
           PRINTF("Setting LLI WORD0 and WORD1 completed\n");
  }
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
  else /* on CSI mode */
  {
	   /*Error = (DxUint32_t)CSI_StartOperation( DataInSize );

	   if( Error != CRYS_OK )
		return Error;*/
  }
#endif

  /* STEP2 : wait on the DMA (or CSI) and HASH mashine busy */

    /* Temporary Debug delay - for debugging mode only */
 #ifdef DX_ARM_INTEGRATOR_DEBUG
     LLF_COMMON_DebugDelay( 100000 );
 #endif

  /* wait on the DMA DIN */
  if( (DxUint32_t)DataIn_ptr < DX_CSI )
  {
		LLF_HASH_HW_WAIT_ON_DMA_SRC_BUSY_BIT( VirtualHwBaseAddr );
  }
  else
  {
        /* wait for FIFO_IN_EMPTY before waiting to the hardware machine to be ready */
        LLF_COMMON_CSI_FIFO_IN_EMPTY_WAIT( VirtualHwBaseAddr );
  }

  /*hardware workaround to avoid falling between DMA done and busy engine*/
  LLF_HASH_HW_WAIT_FIFO_IN_BIT( VirtualHwBaseAddr );

  /* wait for the hardware machine to be ready */
  LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT( VirtualHwBaseAddr );


  return Error;


}/* END of LLF_HASH_UpdateExecBlocksWithDMA */
