
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_DES.c#1:csrc:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
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
//#include "CRYS_EXT_IF_OTF.h"
#endif
#include "LLF_COMMON.h"
#include "LLF_DES.h"
#include "LLF_DES_error.h"
#include "LLF_DES_HwDefs.h"
#include "LLFCD_DES.h"
#include "LLF_COMMON.h"
#include "cc_hw_interface.h"
#include "log_output.h"
/************************ Defines *********************************/

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


/*********************************************************************************************************/
/************************         Private Functions Prototypes              ******************************/
/*********************************************************************************************************/

/*********************************************************************************************************/
/**
 * @brief The low level LLF_DES_BlockInit function initializes the hardware to operate on DES at the modes
 *        the user selected.
 */
  CRYSError_t  LLF_DES_BlockInit (
                       DESContext_t  *WorkingContextID_ptr ,
                       DxUint32_t       VirtualHwBaseAddr,
                       DxUint8_t       *DataOut_ptr,
                       DxUint8_t       *DataIn_ptr );


/*********************************************************************************************************/
/**
 * @brief The low level LLF_DES_AlignerInit function initializes the hardware aligner to operate on DES at the modes
 *        the user selected.
 */
  void  LLF_DES_AlignerInit (
                       DxUint32_t       VirtualHwBaseAddr,
                       DxUint32_t       Mask,
                       DxUint32_t       Shift,
                       DxUint8_t       *DataOut_ptr,
                       DxUint8_t       *DataIn_ptr );


/*********************************************************************************************************/

/**
 * @brief The low level LLF_DES_BlockExecFirstOperation executes the first hardware operation in order to
 *        solve alignment issues and to prepare the next block in the hardware.
 */

CRYSError_t  LLF_DES_BlockExecFirstOperation (
                    DxUint32_t             VirtualHwBaseAddr,
                    DxUint8_t            **DataIn_ptr,
                    DxUint8_t            **DataOut_ptr,
                    DxUint32_t             DataInSize,
                    DxUint8_t             *numOfInBlocksLoadedToHw_ptr);


/*********************************************************************************************************/

/**
 * @brief The low level LLF_DES_BlockSingle executes a single block operation.
 *        It is used insted of the LLFCD block operation in the case that there is only on block to operate
 *        above the ones operated by the LLF_DES_BlockExecFirstOperation and the LLF_DES_BlockFinish
 *        finctions.
 */
CRYSError_t  LLF_DES_BlockSingle (
                    DxUint32_t             VirtualHwBaseAddr,
                    DxUint8_t            **DataIn_ptr,
                    DxUint8_t            **DataOut_ptr );

/*********************************************************************************************************/
/**
 * @brief The low level LLF_DES_BlockFinish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.
 */
CRYSError_t  LLF_DES_BlockFinish(
                         DESContext_t  *WorkingContextID_ptr,    /*in */
                         DxUint32_t       VirtualHwBaseAddr,       /* in */
                         DxUint8_t       *DataOut_ptr ,            /* in / out */
                         DxUint32_t       DataInSize,              /* in */
                         DxUint8_t        DataOutAlignment,		   /* in */
                         DxUint8_t        IsSsdmaMode );			   /* in */


/*********************************************************************************************************/

/**
 * @brief This function operates the block execution on the DMA or CSI modes.
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_DES_DMA_BlockECBandCBCModes(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr );


/*********************************************************************************************************/
/**
 * @brief This function operates the block execution on the DMA mode.
 */
CRYSError_t LLF_DES_DMA_BlockECBandCBCModesSepMLLI(
					              DxUint32_t  *DataInLliTab_ptr,        /* the data in LLI table - in */
					              DxUint32_t   InLliTabSize,            /* the in LLI table size - in */
					              DxUint32_t  *DataOutLliTab_ptr,	    /* the dataout LLI table - in */
					              DxUint32_t   OutLliTabSize,           /* the out LLI table size - in */
					              DxUint32_t   DataInSize,              /* the data size to load - in */
					              DxUint32_t   VirtualHwBaseAddr);      /* the virtual address - in */



/*********************************************************************************************************/
/************************              Pubic Functions                      ******************************/
/*********************************************************************************************************/


/************************************************************************************************/
/**
 * @brief The low level LLF_DES_Block function operates the DES engine in the hardware.
 *        It calls the following low level functions:
 *
 *        1. The function gets a semaphore to protect the access to the hardware
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address
 *           to the registers virtual address.
 *        3. The function calls the LLF_DES_BlockInit to initialize the DES hardware.
 *        4. If DataInSize is enough large or at least one of the data In-Out pointers is
 *           in CSI state, then calls the LLF_DES_DMA_BlockECBandCBCModes function and go
 *           to step 8.
 *        5. The function calls the LLF_DES_BlockExecFirstOperation to execute the first operation.
 *        6. The function calls the level 3 LLFCD_DES_BlockECBandCBCModes implemented block operation
 *           if there is more then 2 block to work on.
 *           If there is 2 block left or less then the function will continue to step 6.
 *
 *        7. If there are 2 blocks to work on the function will call the LLF_DES_BlockSingle function.
 *           If only one block is left it will continue to step 7.
 *
 *        8. The function calls the LLF_DES_BlockFinish function to execute the last block (if needed)
 *           and to save the IV temporary values for the next block operation and to shut down
 *           the hardware machine.
 *        9. The function calls a VOS system call to un map the registers virtual address mapping.
 *       10. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *       11. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The DES context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_DES_Block( DESContext_t    *WorkingContextID_ptr,
                              DxUint8_t       *DataIn_ptr,
                              DxUint32_t       DataInSize,
                              DxUint8_t       *DataOut_ptr )
 {
   /* LOCAL DECLERATIONS */

   /* The return error identifier */
   CRYSError_t Error;

   /* The number of blocks left to load the HW after the first operation */
   DxUint32_t NumOfBlocksToLoadHw;

   /* the virtual address definition */
   DxUint32_t VirtualHwBaseAddr;

   /* the updated data out and data in position */
   DxUint8_t *DataInUpdatePos_ptr;
   DxUint8_t *DataOutUpdatePos_ptr;

   /* the number of blocks loaded in the first operation */
   DxUint8_t NumOfInputBlockLoadedToHwInFirstOperation;

   /* is ssdma mode operation */
   DxUint8_t IsSsdmaMode;
   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;


   /* ............... getting the hardware sempaphre ..................... */
   /* -------------------------------------------------------------------- */

   Error = DX_VOS_SemWait( SemHwAccessId ,DX_INFINITE );

   if( Error != CRYS_OK )

     goto Return;

   /* ............... mapping the physical memory to the virtual one ...... */
   /* --------------------------------------------------------------------- */

   Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_DES_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

   if( Error != CRYS_OK )

     goto ReturnAndReleaseSemaphore;


   /* .............. initializing the hardware .............................. */
   /* ----------------------------------------------------------------------- */

   Error = LLF_DES_BlockInit ( WorkingContextID_ptr,      /* the working context id - in */
                               VirtualHwBaseAddr,         /* the virtual base address - in */
                               DataOut_ptr,               /* the output buffer - in */
                               DataIn_ptr);               /* the input buffer - in */

   if( Error != CRYS_OK )

     goto End;


   /* initializing the local data buffer pointers to the values received by the user */
   DataInUpdatePos_ptr  = DataIn_ptr;
   DataOutUpdatePos_ptr = DataOut_ptr;

   /* set the DMA mode on the following conditions :
      1) if we exceed the minimum blocks required we shall activate the DMA mode
      2) the mode can't be CTR since in is not supppoted by the HW and we need to
         calculate it after each block */

   if ( (DataInSize / CRYS_DES_BLOCK_SIZE_IN_BYTES) >= LLF_DES_MIN_DES_BLOCKS_FOR_DMA_OPERATION ||
        (DxUint32_t)DataIn_ptr  >= DX_CSI ||
        (DxUint32_t)DataOut_ptr >= DX_CSI )

        IsSsdmaMode = DX_TRUE;

   else

        IsSsdmaMode = DX_FALSE;



   /* .............. direct CPU state machine ............................... */
   /* ----------------------------------------------------------------------- */

   if( !IsSsdmaMode )
   {
       LLF_DES_AlignerInit (VirtualHwBaseAddr,         /* the virtual base address - in */
                           0x3,                        /* CPU mode always works with mask of 0x3*/
                           CRYS_COMMON_BusAlignerShift,/* CPU mode always works with mask of 0x3*/
                           DataOut_ptr,                /* the output buffer - in */
                           DataIn_ptr);                /* the input buffer - in */
		/* .............. executing the first operation .......................... */
		/* ----------------------------------------------------------------------- */

		Error = LLF_DES_BlockExecFirstOperation (
			          VirtualHwBaseAddr,                           /* the virtual base address - in */
				  &DataInUpdatePos_ptr,                        /* the data in updated pointer - in / out */
				  &DataOutUpdatePos_ptr,                       /* the data out updated pointer - in / out */
				  DataInSize ,                                 /* the data size - in */
				  &NumOfInputBlockLoadedToHwInFirstOperation); /* the number of blocks loaded - out */

		if( Error != CRYS_OK )

		goto End;

		/* .............. executing the block operation .......................... */
		/* ----------------------------------------------------------------------- */

		/* calculate the number of blocks left to load */
		NumOfBlocksToLoadHw =
		DataInSize / CRYS_DES_BLOCK_SIZE_IN_BYTES - NumOfInputBlockLoadedToHwInFirstOperation;

		/* if the number of blocks left to load the hw is 1 call the single block function */
		if( NumOfBlocksToLoadHw == 1 )
		{
		Error = LLF_DES_BlockSingle (
			    VirtualHwBaseAddr,                           /* the virtual base address - in */
				&DataInUpdatePos_ptr,                        /* the data in updated pointer - in / out */
				&DataOutUpdatePos_ptr );                     /* the data out updated pointer - in / out */

		if( Error != CRYS_OK )

			goto End;

		}/* end of 1 block case */

		/* ....if the number of blocks left to load the hardware is more then 1 call the block functions
	....to operate a multi block operation */
		if( NumOfBlocksToLoadHw > 1 )
		{

		LLFCD_DES_BlockECBandCBCModes(
			DataInUpdatePos_ptr,                                 /* the data in updated pointer - in */
			DataOutUpdatePos_ptr ,                               /* the data out updated pointer - in */
			NumOfBlocksToLoadHw * CRYS_DES_BLOCK_SIZE_IN_BYTES,  /* the data size to load - in */
			VirtualHwBaseAddr);                                  /* the virtual address - in */

		/* update the buffer pointers - we can assumed there are aligned */
		DataInUpdatePos_ptr = DataInUpdatePos_ptr + (NumOfBlocksToLoadHw * CRYS_DES_BLOCK_SIZE_IN_BYTES);
		DataOutUpdatePos_ptr = DataOutUpdatePos_ptr + (NumOfBlocksToLoadHw * CRYS_DES_BLOCK_SIZE_IN_BYTES);


		}/* End of number of blocks grater then 1 case */

   }

   /* ----------------------------------------------------------------------- */
   /* .............. SSDMA state machine .................................... */
   /* ----------------------------------------------------------------------- */
	else
	{
       LLF_DES_AlignerInit (VirtualHwBaseAddr,         /* the virtual base address - in */
                           CRYS_COMMON_BusAlignerMask,/* DMA mode works according to hardware type*/
                           CRYS_COMMON_BusAlignerShift,/* DMA mode works according to hardware type*/
                           DataOut_ptr,               /* the output buffer - in */
                           DataIn_ptr);               /* the input buffer - in */

              LLF_Alignment_Pointer(DataInUpdatePos_ptr,DataOutUpdatePos_ptr,DataInSize);
		LLF_DES_DMA_BlockECBandCBCModes(
			DataInUpdatePos_ptr,             /* the data in updated pointer - in */
			DataOutUpdatePos_ptr ,           /* the data out updated pointer - in */
			DataInSize,                      /* the data size to load - in */
			VirtualHwBaseAddr);              /* the virtual address - in */

			/* update the buffer pointers */
			if( (DxUint32_t)DataInUpdatePos_ptr < DX_CSI )
				DataInUpdatePos_ptr = DataInUpdatePos_ptr + DataInSize;

			if( (DxUint32_t)DataOutUpdatePos_ptr < DX_CSI )
				DataOutUpdatePos_ptr = DataOutUpdatePos_ptr + DataInSize;

   }/*......... end DMA machine .........*/

   /* ................. finishing the block operation ....................... */
   /* ----------------------------------------------------------------------- */

   Error = LLF_DES_BlockFinish(
                         WorkingContextID_ptr,           /* the working context id - in */
                         VirtualHwBaseAddr,              /* the virtual base address - in */
                         DataOutUpdatePos_ptr ,          /* the data out updated pointer - in / out */
                         DataInSize ,                    /* the data size - in */
                         (DxUint32_t)DataOut_ptr & 0x03, /* the data out alignemnt - in */
                         IsSsdmaMode );                  /* the flag defining the DMA mode */

     if( Error != CRYS_OK )

        goto End;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

   End:

   /* if an error occored close the Hardware clock */
   if( Error )

      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_CLK_ENABLE_ADDR , 0x0UL );


   /* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
   DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_DES_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

 ReturnAndReleaseSemaphore:

   /* release the hardware semaphore */
   DX_VOS_SemGive ( SemHwAccessId );

   Return:


   return Error;

 }/* END OF LLF_DES_Block */


/************************************************************************************************/
 /**
 * @brief The low level LLF_DES_InitInit initializes the spesific low level initializatios required
 *        on the spesific platform.
 *
 *        On the lite platform currently this function does nothing.
 *
 * @param[in] WorkingContextID_ptr - The DES context allocated by the CCM
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_DES_InitInit( DESContext_t  *WorkingContextID_ptr )
{
   /* LOCAL DECLERATIONS */

   /* FUNCTION LOGIC */

   /* just to avoid compilers warnings */
   WorkingContextID_ptr = WorkingContextID_ptr;

   return CRYS_OK;

}/* END OF LLF_DES_InitInit */




/************************ Private Functions ******************************/

/************************ Private Functions **********************/

/************************************************************************************************/
/**
 * @brief The low level LLF_DES_BlockInit function initializes the hardware to operate on DES at the modes
 *        the user selected.
 *
 *        The function operates the follwing steps:
 *
 *        1. Checking the CONTENT register. if the DES is not supported return an error code.
 *        2. Enable the clock of the DES engine by setting the register CLK_ENABLE with
 *           the value 0x2 (DES_EN bit).
 *        3. Set the CRYPTO_CTL register to the DES mode - value 0x1.
 *        4. Set the READ_ALIGN register to the value: data_out_ptr % 4 value.
 *        5. Set the WRITE_ALIGN register to the value: data_out_ptr % 4 value.
 *        6. Set the DES_CONTROL register to the value that is stored in the received context:
 *             WorkingContextIDD_ptr-> NumOfKeys with the number of keys ,
 *             WorkingContextIDD_ptr-> EncryptDecryptFlag and the
 *             WorkingContextIDD_ptr-> OperationMode.
 *        7. If the operation mode (WorkingContextIDD_ptr-> OperationMode ) is CBC then
 *           load the IV counters (WorkingContextIDD_ptr-> DESContextIV ) to the DES_IV_0 - DES_IV_1 registers.
 *        8. load the DES_KEY_X registers per the following rules:
 *
 *               a) If the number of keys is 1 (WorkingContextIDD_ptr-> NumOfKeys == 0 ) then
 *                  load the registers : DES_KEY_0 to DES_KEY_1
 *
 *               b) If the number of keys is 2 (WorkingContextIDD_ptr-> NumOfKeys == 1 ) then
 *                  load the registers : DES_DEY_0 to DES_KEY_2
 *
 *               c) If the number of keys is 3
 *                      (WorkingContextIDD_ptr-> NumOfKeys == 2 ) then load the registers :
 *                      DES_KEY_0 to DES_KEY_5
 *
 *        9. Wait until the DES_BUSY register is ready ( the value is 0 ).
 *           On this stage the initialization of the hardware to operate the DES machine is ended.
 *       10. Exit the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The DES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] DataOut_ptr       -  The pointer to the users data output buffer .
 * @param[in] DataIn_ptr        -  The pointer to the users data output buffer .

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  CRYSError_t  LLF_DES_BlockInit (
                       DESContext_t    *WorkingContextID_ptr ,
                       DxUint32_t       VirtualHwBaseAddr,
                       DxUint8_t       *DataOut_ptr,
                       DxUint8_t       *DataIn_ptr )
{
   /* LOCAL DECLARATIONS */

   /* the DES control value */
   DxUint32_t desControlVal;


   /* loop variable */
   DxUint8_t i;



   /* FUNCTION LOGIC */


   /* .................... initialize local variables ......................... */

	DataIn_ptr = DataIn_ptr;
   /* ................. general registers initializations ..................... */
   /* ------------------------------------------------------------------------- */

   /* ...... enabeling the DES clock */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_CLK_ENABLE_ADDR , 0x1UL );

   /* setting the CRYPTO_CTL register to DES mode - the machine is now configured to work on DES */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_CRYPTO_CTL_ADDR ,
                                LLF_DES_HW_CRYPTO_CTL_DES_MODE_VAL );

   /* .................. DES registers initialization .......................... */
   /* -------------------------------------------------------------------------- */

   /* ....... setting the DES control register according to the values in the DES context */

   /* building the DES control value */
   desControlVal = 0;

   /* loading the DEC mode */
   if( WorkingContextID_ptr->EncryptDecryptFlag == CRYS_DES_Encrypt )

     desControlVal |= LLF_DES_HW_DES_CTL_ENCRYPT_VAL;

   else

     desControlVal |= LLF_DES_HW_DES_CTL_DECRYPT_VAL;

   /* loading the KEY number value */
   switch( WorkingContextID_ptr->NumOfKeys )
   {
     case CRYS_DES_1_KeyInUse:

       desControlVal |= LLF_DES_HW_DES_CTL_1_KEY_VAL;
       break;

     case CRYS_DES_2_KeysInUse:

       desControlVal |= LLF_DES_HW_DES_CTL_2_KEYS_VAL;
       break;

     case CRYS_DES_3_KeysInUse:

       desControlVal |= LLF_DES_HW_DES_CTL_3_KEYS_VAL;
       break;

     default:

       return LLF_DES_HW_INTERNAL_ERROR_1;

   }/* end of setting the KEY size switch case */

   /* loading the operation mode */
   switch( WorkingContextID_ptr->OperationMode )
   {
     case CRYS_DES_ECB_mode:

       desControlVal |= LLF_DES_HW_DES_CTL_ECB_MODE_VAL;
       break;

     case CRYS_DES_CBC_mode:

       desControlVal |= LLF_DES_HW_DES_CTL_CBC_MODE_VAL;
       break;

     default:

       return LLF_DES_HW_INTERNAL_ERROR_2;

   }/* end of loading the mode switch case */

   /* setting the register DES control register with the calculated value */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_CTL_0_ADDR ,
                                desControlVal );

   /* setting the register DES control 1 register with dummy value to execute the control write */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_CTL_1_ADDR ,
                                0x0UL );

   /* only in case of (DataOut == fifo) write this control */
	if ((DxUint32_t)DataOut_ptr >= DX_CSI)
				CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_FIFO_MODE_REG_ADDR,
						  1UL );

   /* .............. loading the IV registers on CBC mode ........... */

   if( WorkingContextID_ptr->OperationMode == CRYS_DES_CBC_mode )
   {
      /* loading the registers loop */
      for( i = 0 ; i < CRYS_DES_IV_SIZE_IN_WORDS ; i++ )

         CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_IV_0_ADDR + (i * 4) ,
                                        WorkingContextID_ptr->DESContextIV[i] );

   }/* end of loading the IV counters on CBC mode */

   /* .............. loading the users keys ........... */

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_KEY_0_ADDR,
                                WorkingContextID_ptr->DESContextKey.key1[0] );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_KEY_1_ADDR,
                                WorkingContextID_ptr->DESContextKey.key1[1] );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_KEY_2_ADDR,
                                WorkingContextID_ptr->DESContextKey.key2[0] );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_KEY_3_ADDR,
                                WorkingContextID_ptr->DESContextKey.key2[1] );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_KEY_4_ADDR,
                                WorkingContextID_ptr->DESContextKey.key3[0] );

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_KEY_5_ADDR,
                                WorkingContextID_ptr->DESContextKey.key3[1] );

   /* ................ waiting until the HW machine is enabled */

   LLF_DES_HW_WAIT_ON_DES_BUSY_BIT( VirtualHwBaseAddr );

   return CRYS_OK;

}/* END OF LLF_DES_BlockInit */

/*********************************************************************************************************/
/**
 * @brief The low level LLF_DES_AlignerInit function initializes the hardware aligner to operate on DES at the modes
 *        the user selected.
 */
  void  LLF_DES_AlignerInit (DxUint32_t       VirtualHwBaseAddr,
							 DxUint32_t       Mask,
							 DxUint32_t       Shift,
							 DxUint8_t       *DataOut_ptr,
							 DxUint8_t       *DataIn_ptr )
{
    /* data alignement variables */
   DxUint32_t DataOutAlignemnt;
   DxUint32_t DataInAlignemnt;


    DataInAlignemnt  = (DxUint32_t)DataIn_ptr & Mask;
    DataOutAlignemnt  = (DxUint32_t)DataOut_ptr & Mask;
   /* set the read alignment register according to the CSI mode and data out pointer alignment  */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DIN_WRITE_ALIGN_ADDR,
	                            DataInAlignemnt | DataInAlignemnt << Shift );

   /* set the read alignment register according to the CSI mode and data out pointer alignment  */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DOUT_READ_ALIGN_ADDR,
				    DataOutAlignemnt );
}
/************************************************************************************************/
/**
 * @brief The low level LLF_DES_BlockExecFirstOperation executes the first hardware operation in order to
 *        solve alignment issues and to prepare the next block in the hardware.
 *
 *
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] DataOut_ptr       -  The pointer to the users data output buffer it is updated to the
 *                                 exact popsition after the first operation.
 * @param[in] DataIn_ptr        -  The pointer to the users data input buffer it is updated to the
 *                                 exact popsition after the first operation.
 * @param[in] DataInSize        -  The data in size - the function needs to now if there only one block
 *                                 to operate.
 * @param[out] numOfInBlocksLoadedToHw_ptr - the function sets this parameter to the actual number of input
 *                                           blocks that were loaded to the hardware.
 *                                           This parameter will give the indication if to call the the low level
 *                                           block operation or jump to the finish operation.
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t  LLF_DES_BlockExecFirstOperation (
                    DxUint32_t             VirtualHwBaseAddr,
                    DxUint8_t            **DataIn_ptr,
                    DxUint8_t            **DataOut_ptr,
                    DxUint32_t             DataInSize,
                    DxUint8_t             *numOfInBlocksLoadedToHw_ptr)
{
   /* LOCAL DECLARATIONS */

   /* a variable containing the input non aligned bytes */
   DxUint32_t non_aligned_bytes;

   /* the alignment variable */
   DxUint8_t data_in_alignment;
   DxUint8_t data_out_alignment;

   /* loop variable */
   DxUint8_t i;

   /* FUNCTION LOGIC */

   /* .............................. initializing the local variables ....................... */
   /* --------------------------------------------------------------------------------------- */

   /* setting the data in alignment */
   data_in_alignment = (DxUint32_t)*DataIn_ptr & 0x03;

   /* setting the data out alignment */
   data_out_alignment = (DxUint32_t)*DataOut_ptr & 0x03;

   /* initialize the number of blocks loaded to the hardware to 0 */
   *numOfInBlocksLoadedToHw_ptr = 0;


   /* STEP 1: handeling the case the data in is not aligned */
   /* ----------------------------------------------------- */

   /* on this case we shall load the non aligned bytes to the appropiate non align DATA IN register */
   if( data_in_alignment )
   {
      /* the calue we shall load to the ANALIGN register is :
         non alignment 1 : | dataIn[2] | dataIn[1] | DataIn[0] | X |
         non alignment 2 : | dataIn[1] | dataIn[0] | X | X |
         non alignment 3 : | dataIn[0] | X | X | X | */

      /* init the non aligned bytes variable to 0 */
      non_aligned_bytes = 0;

      /* load the non aligned bytes from the data in pointer to the
         non aligned variable. */
      for( i = 0 ; i < sizeof( DxUint32_t ) - data_in_alignment ; i++ )

          non_aligned_bytes |= ((DxUint32_t)( (*DataIn_ptr)[i] << (8 * i) ) );

      /* move the non aligned byte result to start from the MSB */
      non_aligned_bytes <<= (8 * data_in_alignment);

      /* set the data in pointer to the aligned value */
      *DataIn_ptr += (sizeof(DxUint32_t) - data_in_alignment);

      /* set the aligned bytes to the hardware register */
      CRYS_PLAT_SYS_WriteRegister(
          VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR,
          non_aligned_bytes );

   }/* end of handeling the non alignemnt case */

   /* STEP 2 : load the first aligned block from the input buffer */
   /* ----------------------------------------------------------- */

   /* The first 4 words will be loaded to the hardware and the input buffer pointer will be updated */
   {
      /* since the input buffer is now aligned we can cast it to a 32 bit pointer type */
      DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)*DataIn_ptr;

      /* load the DATA_IN registers */
      for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )

          CRYS_PLAT_SYS_WriteRegister(
          VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR + (4 * i),
          DataIn32Bit_ptr[i] );

      /* update the data in to the new position */
      *DataIn_ptr += (CRYS_DES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t));

      /* increment the number of block loaded to the hardware */
      (*numOfInBlocksLoadedToHw_ptr)++;

   }/* end of loading the first aligned block */

   /* STEP 3 : waiting untile the DES busy register is ready ............................. */
   /* ------------------------------------------------------------------------------------ */

   LLF_DES_HW_WAIT_ON_DES_BUSY_BIT( VirtualHwBaseAddr );

   /* STEP 5 : handeling the case the data out  is not aligned .......................... */
   /* ----------------------------------------------------------------------------------- */

   /* on this case we shall read the non aligned bytes from the first DATA_OUT register.
      then read the other 3 words.
      If the data size is larger then 1 block we shall load the the hardware 4 more aligned blocks.
      The data in and data out pointers will be incremented to there new position after this operation */
   if( data_out_alignment )
   {
      /* load the first aligned word */
      /* the value we shall read is :
         non alignment 1 : | dataOut[2] | dataOut[1] | dataOut[0] | X |
         non alignment 2 : | dataOut[1] | dataOut[0] | X | X |
         non alignment 3 : | dataOut[0] | X | X | X | */
      CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,non_aligned_bytes);

      /* push the non aligned bytes to the LSB */
      non_aligned_bytes >>= ( 8 * data_out_alignment );

      /* loading the non aligned bytes to the data out */
      for ( i = 0 ; i < sizeof(DxUint32_t) - data_out_alignment ; i++ )
      {
        (*DataOut_ptr)[i] = (DxUint8_t)(non_aligned_bytes & 0xFF);

        /* shifting the next byte */
        non_aligned_bytes >>= 8;

      }/* end of loading the non aligned bytes */

       /* set the data out pointer to the aligned value */
      *DataOut_ptr += (sizeof(DxUint32_t) - data_out_alignment);

      /* ..... if the data size is larger then one block then load the next 3 words and load to the hardware
               the next 4 words from the input data */
      if( DataInSize > CRYS_DES_BLOCK_SIZE_IN_BYTES )
      {
         /* since the output buffer is now aligned we can cast it to a 32 bit pointer type */
         DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)*DataOut_ptr;

         /* since the input buffer is now aligned we can cast it to a 32 bit pointer type */
         DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)*DataIn_ptr;


         /* loading the words to the out buffer */
         for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS - 1 ; i++ )

           CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,
                                       DataOut32Bit_ptr[i]);

         /* update the data out to the new position */
         *DataOut_ptr += ((CRYS_DES_BLOCK_SIZE_IN_WORDS - 1 )* sizeof(DxUint32_t));


         /* load the DATA_IN registers */
         for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )

            CRYS_PLAT_SYS_WriteRegister(
              VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR + (4 * i),
              DataIn32Bit_ptr[i] );

         /* update the data in to the new position */
         *DataIn_ptr += (CRYS_DES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t));

         /* increment the number of block loaded to the hardware */
         (*numOfInBlocksLoadedToHw_ptr)++;

         /* wait for the hardware machine to be ready */
         LLF_DES_HW_WAIT_ON_DES_BUSY_BIT( VirtualHwBaseAddr );

      }/* end of preparing the next data block in case the data size is larger then 1
          block */

   }/* end of handeling the data out alignment case */

   return CRYS_OK;

}/* END OF LLF_DES_BlockExecFirstOperation */



/************************************************************************************************/
/**
 * @brief The low level LLF_DES_BlockSingle executes a single block operation.
 *        It is used insted of the LLFCD block operation in the case that there is only on block to operate
 *        above the ones operated by the LLF_DES_BlockExecFirstOperation and the LLF_DES_BlockFinish
 *        finctions.
 *
 *
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataOut_ptr   -  The pointer to the users data output buffer it is updated to the
 *                                 exact popsition after the first operation.
 * @param[in/out] DataIn_ptr    -  The pointer to the users data input buffer it is updated to the
 *                                 exact popsition after the first operation.
 *
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_DES_BlockSingle (
                    DxUint32_t             VirtualHwBaseAddr,
                    DxUint8_t            **DataIn_ptr,
                    DxUint8_t            **DataOut_ptr )
{
   /* LOCAL DECLARATIONS */

   /* loop variable */
   DxUint8_t i;

   /* FUNCTION LOGIC */

   /* STEP 1 : reading the data stored in the HW to the data out buffer  */
   /* ------------------------------------------------------------------ */

   {
      /* since the output buffer is now aligned we can cast it to a 32 bit pointer type */
      DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)*DataOut_ptr;

      /* loading the words to the out buffer */
      for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )

         CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,
                                     DataOut32Bit_ptr[i]);

      /* updating the position of the output pointer to the next block position */
      *DataOut_ptr += (CRYS_DES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t));

   }/* END OF reading the data stored in the hardware */

   /* STEP 2 : loading the next block of DATA_IN from the input buffer  */
   /* ----------------------------------------------------------------- */

   {
      /* since the input buffer is now aligned we can cast it to a 32 bit pointer type */
      DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)*DataIn_ptr;

      /* load the DATA_IN registers */
      for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )

          CRYS_PLAT_SYS_WriteRegister(
          VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR + (4 * i),
          DataIn32Bit_ptr[i] );

      /* updating the position of the input pointer to the next block position */
      *DataIn_ptr += (CRYS_DES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t));

   }/* end of loading the data to DATA_IN  */

   /* wait for the hardware machine to be ready */
   LLF_DES_HW_WAIT_ON_DES_BUSY_BIT( VirtualHwBaseAddr );

   return CRYS_OK;

}/* END OF LLF_DES_BlockSingle */


/************************************************************************************************/
/**
 * @brief The low level LLF_DES_BlockFinish reads the last block from the DATA_OUT.
 *        and closes the hardware clock.
 *
 * @param[in] WorkingContextID_ptr - The DES context allocated by the CCM
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in/out] DataOut_ptr   -  The pointer to the users data output buffer at the exact position.
 * @param[in] DataOutAlignment  -  The data out alignemnt - needed to know how meny bytes to load
 *                                 from the last word.
 * @param[i] DataInSize         -  The data in size - the function needs to now if there only one block
 *                                 to operate.
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_DES_BlockFinish(
                         DESContext_t    *WorkingContextID_ptr,    /*in */
                         DxUint32_t       VirtualHwBaseAddr,       /* in */
                         DxUint8_t       *DataOut_ptr,             /* in / out */
                         DxUint32_t       DataInSize,              /* in */
                         DxUint8_t        DataOutAlignment,		   /* in */
                         DxUint8_t        IsSsdmaMode )			   /* in */
{
   /* LOCAL DECLARATIONS */

   /* loop variable */
   DxUint8_t i;

   /* FUNCTION LOGIC */

   /* STEP 1 : reading the data stored in the HW to the data out buffer       */
   /* If the operation was done via DMA the data is already in the out buffer */
   /* ----------------------------------------------------------------------- */
   if(!IsSsdmaMode)
   {
      /* since the output buffer is now aligned we can cast it to a 32 bit pointer type */
      DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)DataOut_ptr;

      /* the number of aligned woeds to load */
      DxUint8_t numOfAlignedWords;

      /* if the data is aligned or we have executed the DES operation on more then
         one block we can assum that in the hardware we have the entire 4 words
         of information.
         if the data is not aligned and this is the only block we need to read one aligned
         word less */
      if( DataOutAlignment == 0 || DataInSize > CRYS_DES_BLOCK_SIZE_IN_BYTES )

         numOfAlignedWords = CRYS_DES_BLOCK_SIZE_IN_WORDS;

      else

         numOfAlignedWords = CRYS_DES_BLOCK_SIZE_IN_WORDS - 1;

      /* loading the aligned words to the out buffer */
      for( i = 0 ; i < numOfAlignedWords ; i++ )

         CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,
                                     DataOut32Bit_ptr[i]);

      /* set the data out to the new position */
      DataOut_ptr += (numOfAlignedWords * sizeof(DxUint32_t));

      /* handeling the last aligned data if there is an alignment on the data out */
      if( DataOutAlignment )
      {
         /* the last aligned value variable */
         DxUint32_t lastDataValue;

         /* set the READ_LAST_DATA register */
         CRYS_PLAT_SYS_WriteRegister(
          VirtualHwBaseAddr + LLF_DES_HW_READ_LAST_DATA_ADDR , 0x1UL );

         /* read the last data value from the hardware */
         /* the value we shall read is :
         non alignment 1 : | X | dataOut[2] | dataOut[1] | dataOut[0] |
         non alignment 2 : | X | X | dataOut[1] | dataOut[0] |
         non alignment 3 : | X | X | X | dataOut[0] | */
         CRYS_PLAT_SYS_ReadRegister(
          VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR , lastDataValue );

         /* loading the result to the last bytes */
         for( i = 0 ; i < DataOutAlignment ; i++ )
         {
            DataOut_ptr[i] = lastDataValue & 0xFF;
            lastDataValue >>= 8;
         }

      }/* end of data alignment case */

   }/* END OF loading the DATA stored in the hardware (on NonSSDMA mode) */


   /* STEP 2 : On CBC mode store the updated IV on the context */
   /* -------------------------------------------------------- */

   /* This operation will enable us to send the CRYS_DES_Block call on the next data */
   if( WorkingContextID_ptr->OperationMode == CRYS_DES_CBC_mode )
   {
      for( i = 0 ; i < CRYS_DES_IV_SIZE_IN_WORDS ; i++ )

         /* read the result from the Hardware */
         CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_IV_0_ADDR + (i * 4) ,
                                     WorkingContextID_ptr->DESContextIV[i]);


   }/* end of storing the IV on the context on MAC modes */

   /* STEP 3 : Close the Hardware Clock */
   /* --------------------------------- */

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DES_CLK_ENABLE_ADDR , 0x0UL );


   return CRYS_OK;

}/* END OF LLF_DES_BlockFinish */


/************************************************************************************************/
/**
 * @brief This function operates the block execution on the DMA or CSI modes.
 *
 *        This function should be operated in ECB or CBC mode.
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_DES_Block calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_DES_Block calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the DES with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

CRYSError_t LLF_DES_DMA_BlockECBandCBCModes(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{
  /* LOCAL DECLARATION */

  /* initialize the error identifier as OK */
  CRYSError_t Error = CRYS_OK;


  /* FUNCTION LOGIC */

  /* .................... initialize local variables ...................... */

  /* .................... executing the DMA ............................... */

  if( (DxUint32_t)DataOut_ptr < DX_CSI )
  {
      /* initialize the destination LLI address */
      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DST_LLI_WORD0_ADDR, (DxUint32_t)DataOut_ptr );

	  /* initialize the destination size and set the first & last LLI word */
	  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_DST_LLI_WORD1_ADDR,
	                               DataInSize |
	                               (1UL << LLF_DES_HW_LLI_WORD1_FIRST_LLI_WORD_POS) |
	                               (1UL << LLF_DES_HW_LLI_WORD1_LAST_LLI_WORD_POS) );
  }
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
  else
  {
    //CSI_StartOutputOperation(DataInSize);
  }
#endif

  if( (DxUint32_t)DataIn_ptr < DX_CSI )
  {
      /* initialize the source LLI address */
	  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_SRC_LLI_WORD0_ADDR, (DxUint32_t)DataIn_ptr );

	  /* initialize the source size and set the first & last LLI word - this triggers the operation */
	  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_DES_HW_SRC_LLI_WORD1_ADDR,
	                               DataInSize |
	                               (1UL << LLF_DES_HW_LLI_WORD1_FIRST_LLI_WORD_POS) |
	                               (1UL << LLF_DES_HW_LLI_WORD1_LAST_LLI_WORD_POS) );
  }
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
  else /* on CSI mode */
  {
	   /*Error = (DxUint32_t)CSI_StartOperation( DataInSize );

	   if( Error != CRYS_OK )
		return Error;*/
  }
#endif

  if( (DxUint32_t)DataOut_ptr < DX_CSI)
  {
      /* STEP2 : wait on the DES mashine and DMA output */

	  /* wait for the hardware machine to be ready */
	  LLF_DES_HW_WAIT_ON_DES_BUSY_BIT( VirtualHwBaseAddr );

      /* wait for the DMA output machine to be ready */
      LLF_DES_HW_WAIT_ON_DMA_DST_BUSY_BIT( VirtualHwBaseAddr );

  }
#ifndef CRYS_NO_EXT_IF_MODE_SUPPORT
  else
  {
      /* wait for FIFO_IN_EMPTY before waiting to the hardware machine to be ready */
      LLF_COMMON_CSI_FIFO_IN_EMPTY_WAIT( VirtualHwBaseAddr );

      //LLF_COMMON_CSI_READ_FINISH_WAIT( );
  }
#endif

  return Error;

}/* END OF LLF_DES_DMA_BlockECBandCBCModes */
