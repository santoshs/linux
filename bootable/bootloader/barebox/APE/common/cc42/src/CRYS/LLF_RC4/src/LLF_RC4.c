
  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  26 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief RC4cription of this module
   *
   *  \version LLF_RC4.c#1:csrc:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

#include "DX_VOS_Sem.h"
#ifdef CRYS_SEP_SIDE_WORK_MODE
/* SEP side needs a basic synchronixzation between the interrupts */
#include "sep_sync.h"
#endif /*CRYS_SEP_SIDE_WORK_MODE*/
#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "LLF_COMMON.h"
#include "CRYS.h"
#include "LLF_RC4.h"
#include "LLF_RC4_error.h"
#include "LLF_RC4_HwDefs.h"
#include "LLFCD_RC4.h"
#include "cc_hw_interface.h"
#include "log_output.h"

//#include "CC_Hw_InterFace.h"



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

/************************ Enums ******************************/


/************************ Typedefs ******************************/


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
 * @brief The low level LLF_RC4_StreamInit function initializes the hardware to operate RC4 .
 *
 */
 CRYSError_t  LLF_RC4_StreamInit (
                                   DxUint32_t       VirtualHwBaseAddr,
			                       RC4Context_t    *WorkingContextID_ptr,
			                       DxUint8_t       *DataIn_ptr,
			                       DxUint8_t       *DataOut_ptr,
			                       DxUint32_t       DataInSize );

/*********************************************************************************************************/
/**
 * @brief The low level LLF_RC4_AlignerInit function initializes the hardware to operate RC4 .
 *
 */
void  LLF_RC4_AlignerInit (DxUint32_t       VirtualHwBaseAddr,
                            DxUint32_t       Mask,
                            DxUint32_t       Shift,
			                DxUint8_t       *DataIn_ptr,
			                DxUint8_t       *DataOut_ptr,
			                DxUint32_t       DataInSize );



/************************************************************************************************/
/**
 * @brief The low level LLF_RC4_StreamRemainData function finishes the RC4 stream process.
 *
 */
CRYSError_t  LLF_RC4_StreamRemainingData(
                           DxUint32_t       VirtualHwBaseAddr,       /*in*/
	                       DxUint8_t       *DataIn_ptr,              /*in*/
	                       DxUint32_t       DataInSize,              /*in*/
	                       DxUint8_t       *DataOut_ptr );           /*out*/


/************************************************************************************************/
/**
 * @brief The low level LLF_RC4_StreamFinish function finishes the RC4 stream process.
 *
 */
CRYSError_t  LLF_RC4_StreamFinish(
                           DxUint32_t       VirtualHwBaseAddr,       /*in*/
                           RC4Context_t    *WorkingContextID_ptr );  /*in*/


/*********************************************************************************************************/
/**
 * @brief This function operates the RC4 stream operation on the direct access mode.
 *
 */
CRYSError_t  LLF_RC4_StreamDirect(
                                   DxUint32_t      VirtualHwBaseAddr,
                                   DxUint8_t      *DataIn_ptr,
                                   DxUint8_t      *DataOut_ptr,
                                   DxUint32_t      DataInSize );


/*********************************************************************************************************/
/**
 * @brief This function operates the RC4 Stream operation on full words of data on the DMA mode.
 *
 */
CRYSError_t LLF_RC4_StreamDma(
              DxUint32_t           VirtualHwBaseAddr,
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t            inStartAlign,
              DxUint32_t            outStartAlign );



/*********************************************************************************************************/
/************************              Public Functions                     ******************************/
/*********************************************************************************************************/


/*********************************************************************************************************/

/**
 * @brief This function is used to initialize the RC4 machine engine and to save its state in the context.
 *        The function is called from the CRYS_RC4_Init function.
 *
 * @param[in] ContextID_ptr - A pointer to the RC4 context buffer that is allocated by the user
 *                            and is used for the RC4 machine operation.
 * @param[in] Key_ptr  - A pointer to the user's key buffer.
 * @param[in] KeySize  - The size of the KEY in bytes.
 *
 * @return CRYSError_t - On success the value CRYS_OK is returned,
 *                       and on failure a value from LLF_RC4_error.h
 */

CRYSError_t   LLF_RC4_Init( RC4Context_t    *ContextID_ptr,
                            DxUint8_t	    *Key_ptr,
                            DxUint32_t		 KeySizeInBytes)
{

   /* DECLARATIONS */

   /* loop variable */
   DxUint16_t   i;



   /* FUNCTION LOGIC */

   /* local variables for HW init */

   CRYSError_t Error;
   DxUint32_t  KeyWord;
   DxUint32_t  VirtualHwBaseAddr;


   /* initializing the Error to O.K */
   Error = CRYS_OK;

   //PRINTF("LLF_RC4_Init function entered\n\n");
   /* ............... getting the hardware semaphore ..................... */
   /* -------------------------------------------------------------------- */

   Error = DX_VOS_SemWait( SemHwAccessId ,DX_INFINITE );

   if( Error != CRYS_OK )

	goto Return;

   /* ............... mapping the physical memory to the virtual one ...... */
   /* --------------------------------------------------------------------- */

   Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_RC4_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

   if( Error != CRYS_OK )

	goto ReturnAndReleaseSemaphore;


   /* checking the key size according to LLF and hardware requirements */
   if( KeySizeInBytes > LLF_RC4_MAX_KEY_SIZE_IN_BYTES || KeySizeInBytes < LLF_RC4_MIN_KEY_SIZE_IN_BYTES )

	return LLF_RC4_KEY_SIZE_IS_INVALID_FOR_HW_ERROR;


   /* .......................... initializations .............................. */
   /* ------------------------------------------------------------------------- */
  //PRINTF("Writing to Register\n\n");
   /* ...... enabeling the RC4 clock */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_CLK_ENABLE_ADDR , 1UL );
  //PRINTF("Writing to register completed\n");

   /* initialization of temp buffer for key loading loop */
   KeyWord = 0;

   /* Step 1. Loading the RC4 key */

   for( i = 0; i < LLF_RC4_MAX_KEY_SIZE_IN_BYTES; i++ )
   {
        DxUint32_t  shift;
        shift =  i % 4;

        if( i < KeySizeInBytes )

		 KeyWord |=  (DxUint32_t)Key_ptr[i] << 8*shift;

	    if( shift == 3 )
	    {
		 CRYS_PLAT_SYS_WriteRegister(
		           VirtualHwBaseAddr + LLF_RC4_HW_RC4_KEY_REG_ADDR , KeyWord );

		 KeyWord = 0;
	    }
   }

   CRYS_PLAT_SYS_WriteRegister(VirtualHwBaseAddr + LLF_RC4_HW_RC4_RST_KEY_PTR_REG_ADDR , 0x1 );

   /* load key size */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_KEY_LENGTH_REG_ADDR ,
                                KeySizeInBytes - 1 );

   /* Step 2. Start HW init operation */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_INIT_REG_ADDR , 1 );
   //PRINTF("Waiting started\n");
   /* wait on RC4 ready bit  */
   LLF_RC4_HW_WAIT_ON_RC4_READY_BIT( VirtualHwBaseAddr );
   //PRINTF("Waiting on bust bit completed\n\n");


#ifndef LLF_RC4_NO_RESUME_HW_MEMORY
   {
       DxUint32_t  suspendAnswer = 0;


       /* Step 3. Set suspend request register to "request" state (1) and wait for suspend answer */
	   while( suspendAnswer != LLF_RC4_HW_SUSPEND_ANSWER_VAL )
	   {
                   //PRINTF("Waiting on busy bit2\n");
		   LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT(VirtualHwBaseAddr);
                   //PRINTF("Waiting on busy bit completed\n");
		   /* write suspend request */
			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SUSPEND_REQUEST_REG_ADDR,
			                             LLF_RC4_HW_SUSPEND_REQUEST_VAL );

		    /* read suspend answer */
		    CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SUSPEND_ANSWER_REG_ADDR,
	                                    suspendAnswer );
                   //PRINTF("Reading suspend answer completed\n");
	   }

	   /* Step 4. Save  HW RAM (S-box and S[I], I, S[J], J registers)  into Context */
	   for( i = 0 ; i < LLF_RC4_RAM_SIZE_IN_WORDS ; i++ )
	   {
                //PRINTF("Waiting on busy bit3\n");
	        LLF_RC4_HW_WAIT_ON_RC4_DATA_READY_BIT( VirtualHwBaseAddr );
                //PRINTF("Waiting on busy bit completed3\n");

	        CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SPECIAL_REGISTER_REG_ADDR,
	                                    ((DxUint32_t*)ContextID_ptr->LLF.RC4_HW_Mem)[i] );
             //   PRINTF("Reading HW RAM into context completed\n");
	   }

	   /* Step 5 : Close the Hardware Clock */
	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_CLK_ENABLE_ADDR , 0UL );

	   /* Step 6. Set IsSboxInitialized flag  */
	   ContextID_ptr->IsSboxInitialized = 1;

   }

#endif /*  !LLF_RC4_NO_RESUME_HW_MEMORY */


/* ................. end of Hw function .................................. */
/* ----------------------------------------------------------------------- */


   /* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
   DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_RC4_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

  ReturnAndReleaseSemaphore:

   /* release the hardware semaphore */
   DX_VOS_SemGive ( SemHwAccessId );
   //PRINTF("LLF_RC4_Init function ended\n\n");

  Return:

   return Error;


}/* END OF RC4_Init */




/************************************************************************************************/
/**
 * @brief The low level LLF_RC4_Stream function operates the RC4 engine in the hardware.
 *
 *        It calls the following low level functions:
 *
 *        1. The function gets a semaphore to protect the access to the hardware
 *           from any other process. The semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address
 *           to the registers virtual address.
 *        3. The function calls the LLF_RC4_StreamInit to initialize the RC4 hardware.
 *        4. According to predefined flags and data size, the function calls appropriate
 *           LLF_RC4_Stream private function:
 *           - if DataSize < LLF_RC4_MIN_RC4_BLOCKS_FOR_DMA_OPERATION  the function
 *             calls the LLF_RC4_StreamDirect() function,
 *           - else calls the LLF_RC4_StreamDma() function,
 *           - if data size is not multiple of 4 bytes, then calls LLF_RC4_StreamRemainingData()
 *             function.
 *        5. On the end the function calls the LLF_RC4_StreamFinish function to save
 *           the RC4 HW specific memory  for next stream operations and to shut down the RC4 machine.
 *        6. The function also calls a VOS system to un map the registers virtual address mapping.
 *        7. Releases the "crypto hardware busy" semaphore acquired in paragraph 1.
 *        8. Exits the function.
 *
 *
 * @param[in] WorkingContextID_ptr - The RC4 context allocated by the CCM
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  CRYSError_t  LLF_RC4_Stream(
                               RC4Context_t    *WorkingContextID_ptr,
                               DxUint8_t       *DataIn_ptr,
                               DxUint8_t       *DataOut_ptr,
                               DxUint32_t       DataInSize )
 {
   /* LOCAL DECLARATIONS */

   /* The return error identifier */
   CRYSError_t Error;

    /* the virtual address definition */
   DxUint32_t VirtualHwBaseAddr;

   /* count of bytes of input data which not complete a 32-bit word */
   DxUint32_t  countOfExtraBytes;

   /* remaining data buffer */
   DxUint32_t   remainingData;


   /* FUNCTION LOGIC */

   /* ............... local initializations .............................. */
   /* -------------------------------------------------------------------- */

   /* initializing the Error to O.K */
   Error = CRYS_OK;


   /* ............... getting the hardware sempaphre ..................... */
   /* -------------------------------------------------------------------- */
   //PRINTF("LLF_RC4_Stream function starts\n\n");
#ifndef CRYS_SEP_SIDE_WORK_MODE
   Error = DX_VOS_SemWait( SemHwAccessId ,DX_INFINITE );
   if( Error != CRYS_OK )

     goto Return;
#endif
   /* ............... mapping the physical memory to the virtual one ...... */
   /* --------------------------------------------------------------------- */

   Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_RC4_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

   if( Error != CRYS_OK )

     goto ReturnAndReleaseSemaphore;


   /* .............. initializing the hardware .............................. */
   /* ----------------------------------------------------------------------- */
   Error = LLF_RC4_StreamInit (
                                VirtualHwBaseAddr,         /* the virtual base address - in */
                                WorkingContextID_ptr,      /* the working context id - in */
                                DataIn_ptr,                /* the input buffer - in */
                                DataOut_ptr,               /* the output buffer - in */
                                DataInSize );              /* size of input data */
	if( Error != CRYS_OK )

       goto End;

   /* count of extra bytes (last bytes, which not composes a full word) */
   countOfExtraBytes = DataInSize % 4;

   if( DataInSize > 0 )
   {
	   /* set the Direct mode if we not exceed the minimum data size required for DMA */
	   if( DataInSize < LLF_RC4_MIN_DATA_SIZE_BYTES_FOR_DMA_OPERATION || DataInSize < 12 )
	   {
		    /* ..... if there is data for StreamDirect operation, then process it .... */
		    /* ----------------------------------------------------------------------- */
		    if( DataInSize - countOfExtraBytes > 0 )
		    {
                LLF_RC4_AlignerInit (VirtualHwBaseAddr,         /* the virtual base address - in */
                                    0x3,                /* the input buffer - in */
                                    CRYS_COMMON_BusAlignerShift,/* the input buffer - in */
                                    DataIn_ptr,                /* the input buffer - in */
                                    DataOut_ptr,               /* the output buffer - in */
                                    DataInSize );              /* size of input data */


				Error = LLF_RC4_StreamDirect (
			          VirtualHwBaseAddr,                  /* the virtual base address - in */
				  DataIn_ptr,                         /* the data in updated pointer - in / out */
				  DataOut_ptr,                        /* address of the data out updated pointer - in/out */
				  DataInSize - countOfExtraBytes);    /* the number of blocks loaded - out */

				if( Error != CRYS_OK )

				goto End;

			    /* updating data size and pointers */
			    DataIn_ptr  += DataInSize - countOfExtraBytes;
			    DataOut_ptr += DataInSize - countOfExtraBytes;
			    DataInSize = countOfExtraBytes;

		    }

		   /* .. if there is remaining data then process it   ....................... */
		   /* ----------------------------------------------------------------------- */

		   if( DataInSize != 0 )
		   {
		        /* copy remainig data into 32-bits buffer */
		        DX_VOS_FastMemCpy( (DxUint8_t*)&remainingData, DataIn_ptr, DataInSize );
		        Error = LLF_RC4_StreamRemainingData(
		                         VirtualHwBaseAddr,               /* the virtual base address - in */
							 (DxUint8_t*)&remainingData,      /* the data in updated pointer - in */
							 DataInSize,                      /* the data size to load - in */
							 DataOut_ptr );                   /* the data out updated pointer - in */

		        if( Error != CRYS_OK )

		             goto End;
		   }

	   }

	   else /* set the DMA mode if we exceed the minimum data size required */
	   {
            LLF_RC4_AlignerInit (VirtualHwBaseAddr,         /* the virtual base address - in */
                                 CRYS_COMMON_BusAlignerMask, /* the input buffer - in */
                                 CRYS_COMMON_BusAlignerShift, /* the input buffer - in */
                                 DataIn_ptr,                /* the input buffer - in */
                                 DataOut_ptr,               /* the output buffer - in */
                                 DataInSize );              /* size of input data */
			/* ......... executing the stream operation on single DMA mode ........... */
			/* ----------------------------------------------------------------------- */
                       LLF_Alignment_Pointer(DataIn_ptr,DataOut_ptr,DataInSize);
			LLF_RC4_StreamDma(
			        VirtualHwBaseAddr,               /* the virtual address - in */
				DataIn_ptr,                      /* the data in updated pointer - in */
				DataOut_ptr,                     /* the data out updated pointer - in */
				DataInSize /*- countOfExtraBytes*/,     /* the data size to load - in */
				(DxUint32_t)DataIn_ptr &0x3,     /* the data input alignment - in */
				(DxUint32_t)DataOut_ptr &0x3 );  /* the data output alignment - in */

	   }
   }

   /* ................. finishing the block operation ....................... */
   /* ----------------------------------------------------------------------- */

   Error = LLF_RC4_StreamFinish(
                         VirtualHwBaseAddr,               /* the virtual base address - in */
                         WorkingContextID_ptr );          /* the working context id - in */

     if( Error != CRYS_OK )

        goto End;

   /* ................. end of function ..................................... */
   /* ----------------------------------------------------------------------- */

  End:

   /* if an error occurred close the Hardware clock */
   if( Error )

      CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_CLK_ENABLE_ADDR , 0x0UL );

   /* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
   DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_RC4_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

  ReturnAndReleaseSemaphore:

#ifndef CRYS_SEP_SIDE_WORK_MODE
   /* release the hardware semaphore */
   DX_VOS_SemGive ( SemHwAccessId );
  Return:
#endif
    //PRINTF("LLF_RC4_Stream function ended\n\n");
   return Error;

 }/* END OF LLF_RC4_Stream */






/************************************************************************************************/
/*****************          Private Functions                   *********************************/
/************************************************************************************************/


/**
 * @brief The LLF_RC4_StreamInit function initializes the hardware to perform RC4 operation.
 *
 *        The function operates the following steps:
 *
 *        1. Checking the HW VERSION register, if the version is not right return an error code.
 *        2. Enables the clock of the RC4 engine by setting the register CLK_ENABLE with
 *           the value 0xb (RC4_EN bit).
 *        3. Set the CRYPTO_CTL register to the RC4 mode - value 0xb.
 *        4. Set the READ_ALIGN register to the value: data_out_ptr % 4.
 *        5. Set the WRITE_ALIGN register to the value: data_in_ptr % 4 | (data_in_ptr % 4 << 2)
 *        6. Resume HW RAM (set the S-box and I, J registers) from the RC4 Context.
 *        7. Wait until the RC4_DATA_READY register is ready ( the value is 1 ).
 *           On this stage the initialization of the hardware to operate the RC4 machine is ended.
 *        8. Exit the function.
 *
 *
 * @param[in] VirtualHwBaseAddr    - The hardware virtual base address.
 * @param[in] WorkingContextID_ptr - The RC4 context allocated by the CCM.
 * @param[in] DataIn_ptr           - The pointer to the users data output buffer.
 * @param[in] DataOut_ptr          - The pointer to the users data output buffer.
 * @param[in] DataInSize           - The data size to operate the RC4 with this function.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
  CRYSError_t  LLF_RC4_StreamInit (
                                   DxUint32_t       VirtualHwBaseAddr,
			                       RC4Context_t    *WorkingContextID_ptr,
			                       DxUint8_t       *DataIn_ptr,
			                       DxUint8_t       *DataOut_ptr,
			                       DxUint32_t       DataInSize )
{

   /* LOCAL DECLARATIONS */



   /* FUNCTION LOGIC */

  DataIn_ptr = DataIn_ptr;
  DataOut_ptr = DataOut_ptr;
  DataInSize = DataInSize;

  //PRINTF("LLF_RC4_StreamInit function starts\n\n");
   /* ................. general registers initializations ..................... */
   /* ------------------------------------------------------------------------- */

   /*  enabling the RC4 clock */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_CLK_ENABLE_ADDR , 0x1UL );

   /* setting the CRYPTO_CTL register to RC4 mode - the machine is now configured to work on RC4 */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_CRYPTO_CTL_ADDR ,
                                LLF_RC4_HW_CRYPTO_CTL_RC4_MODE_VAL );

   /*--------------------------------------------------------------------------------*/
   /* If not defined LLF_RC4_NO_RESUME_HW_MEMORY flag resume HW RC4 RAM from Context */
   /*--------------------------------------------------------------------------------*/

 #ifndef LLF_RC4_NO_RESUME_HW_MEMORY
   {
        /* loop variable */
        DxUint8_t i;


	   /* .. Resume HW RAM ( set S-box and S[I], I, S[J], J registers from the RC4 Context) ..*/
	   /* ----------------------------------------------------------------------------------- */

	   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_RESUME_REQUEST_REG_ADDR,
	                                LLF_RC4_HW_RESUME_REQUEST_VAL );

	   for( i = 0; i < LLF_RC4_RAM_SIZE_IN_WORDS; i++ )
	   {
			LLF_RC4_HW_WAIT_ON_RC4_DATA_READY_BIT( VirtualHwBaseAddr );

			CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SPECIAL_REGISTER_REG_ADDR,
			                             ((DxUint32_t*)WorkingContextID_ptr->LLF.RC4_HW_Mem)[i] );

	   }

	   /* .... waiting until the HW machine is enabled ...... */
       LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr );
   }


 #endif /*  !LLF_RC4_NO_RESUME_HW_MEMORY */

  //PRINTF("LLF_RC4_StreamInit function ended\n\n");
   return CRYS_OK;

}/* END OF LLF_RC4_StreamInit */

void   LLF_RC4_AlignerInit (DxUint32_t       VirtualHwBaseAddr,
			                DxUint32_t       Mask,
			                DxUint32_t       Shift,
			                DxUint8_t       *DataIn_ptr,
			                DxUint8_t       *DataOut_ptr,
			                DxUint32_t       DataInSize )
{

   //PRINTF("LLF_RC4_AlignerInit function starts\n\n");
   /* setting the write (DIN) alignment register: set for data size as if it is 0 modulo 4 */
   CRYS_PLAT_SYS_WriteRegister(
             VirtualHwBaseAddr + LLF_RC4_HW_DIN_WRITE_ALIGN_ADDR,
             ((DxUint32_t)DataIn_ptr & Mask) |
             (((((DxUint32_t)DataIn_ptr + DataInSize) & Mask) ) << Shift) );

   /* setting read (DOUT) alignment register */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DOUT_READ_ALIGN_ADDR,
				(DxUint32_t)DataOut_ptr & Mask );

   //PRINTF("LLF_RC4_AlignerInit function ended\n\n");
    return;
}


/************************************************************************************************/
/**
 * @brief The low level LLF_RC4_StreamFinish function finishes the RC4 stream process.
 *
 *        The function saves the RC4 machine specific RAM (S-box and S[i],I,S[J],J  registers)
 *        into Context and closes the HW clocks.
 *
 * @param[in] VirtualHwBaseAddr -  The hardware virtual base address.
 * @param[in] WorkingContextID_ptr - The RC4 context allocated by the CCM.
  *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_RC4_StreamFinish(
                           DxUint32_t       VirtualHwBaseAddr,       /*in*/
                           RC4Context_t    *WorkingContextID_ptr )   /*in*/
{

 #ifndef LLF_RC4_NO_RESUME_HW_MEMORY

   /* LOCAL DECLARATIONS */

   /* loop index */
   DxUint32_t  i;

   /* suspend answer buffer */
   DxUint32_t suspendAnswer = 0;


   /* FUNCTION LOGIC */

   //PRINTF("LLF_RC4_StreamFinish function started\n\n");
   /* Step 1. Set suspend request register to "request" state ( = 1) and wait for suspend answer ( = 2) */

   while( suspendAnswer != LLF_RC4_HW_SUSPEND_ANSWER_VAL )
   {
	   LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT(VirtualHwBaseAddr);
	    /* write suspend request */
		CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SUSPEND_REQUEST_REG_ADDR,
		                             LLF_RC4_HW_SUSPEND_REQUEST_VAL );

	    /* read suspend answer */
	    CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SUSPEND_ANSWER_REG_ADDR,
                                    suspendAnswer );
   }


   /* Step 2. Save  HW S-box RAM and I, J values in the Context */

   for( i = 0 ; i < LLF_RC4_RAM_SIZE_IN_WORDS ; i++ )
   {
        LLF_RC4_HW_WAIT_ON_RC4_DATA_READY_BIT( VirtualHwBaseAddr );

        CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_SPECIAL_REGISTER_REG_ADDR,
                                    ((DxUint32_t*)WorkingContextID_ptr->LLF.RC4_HW_Mem)[i] );
   }

#endif  /* LLF_RC4_NO_RESUME_HW_MEMORY */

   /* Step 3 : Close the Hardware Clock */

   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_RC4_CLK_ENABLE_ADDR , 0x0UL );

  //PRINTF("LLF_RC4_StreamFinish function ended\n\n");

   return CRYS_OK;

}/* END OF LLF_RC4_StreamFinish */




/************************************************************************************************/
/**
 * @brief The low level LLF_RC4_StreamDirect executes a stream operation on direct access mode.
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 * @param[in] DataIn_ptr - The input data pointer.
 * @param[in] DataOut_ptr - The output data pointer.
 * @param[in] DataInSize  - The data size to operate the RC4 with this function.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_RC4_StreamDirect(
                                   DxUint32_t      VirtualHwBaseAddr,
                                   DxUint8_t      *DataIn_ptr,
                                   DxUint8_t      *DataOut_ptr,
                                   DxUint32_t      DataInSize )
{
   /* LOCAL DECLARATIONS */

   /* loop variable */
   DxUint8_t i;

   /* number of words (including not full non aligned first and last words) to input and to output */
   DxUint32_t  numWordsToInput;
   DxUint32_t  numWordsToOutput;

   /* alignment values for start and end addresses of input and output data */
   DxUint32_t  inStartAlign, outStartAlign;

   /* word to load into mashine registers */
   DxUint32_t  oneWordBuffer = 0;

   //PRINTF("LLF_RC4_StreamDirect function entered\n\n");
   /* INITIALIZATIONS */

   /* set start and end alignment values */
   inStartAlign  = (DxUint32_t)DataIn_ptr & 0x3;
   outStartAlign = (DxUint32_t)DataOut_ptr & 0x3;


   /* FUNCTION LOGIC */


   /* calculate number of words to input in mashine registers (full and not full words) */
   numWordsToInput = ( inStartAlign + DataInSize - DataInSize % 4 + 3 ) / 4;
   numWordsToOutput = ( outStartAlign + DataInSize - DataInSize % 4 + 3 ) / 4;


   /* Step 1. If input is not aligned, then perform RC4 operation on first non aligned bytes of data  */
   if( inStartAlign != 0 )
   {
	    /* set non aligned first input bytes into oneWordBuffer 32-bit buffer (for prventing access to not allowed
	       memory regions if to use 32-bit pointer */
	    for(i = 0; i < 4 - inStartAlign; i++)

		    ((DxUint8_t*)&oneWordBuffer)[i + inStartAlign] = DataIn_ptr[i];

	    /* RC4 operation on first word: write non aligned bytes of data into mashine DIN_DOUT register */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR, oneWordBuffer );

        /* wait on  RC4_BUSY_BIT */
        LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr );

        /* decrement number of words and remining data size */
		numWordsToInput -=  1;
		DataInSize =  DataInSize - (4 - inStartAlign);

		/* increment data input pointer */
		DataIn_ptr += 4 - inStartAlign;
   }

   /* Step 2. If output is not aligned and we have data to input, then
              insert next input word and output non aligned valid bytes of result */
   /* --------------------------------------------------------------------------------- */
   if( outStartAlign != 0 && numWordsToInput > 0)
   {
	    /* RC4 operation on current word: write word into mashine DIN_DOUT register
	       Note: Now the input pointer is aligned! */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR, ((DxUint32_t*)DataIn_ptr)[0] );

        /* wait on  RC4_BUSY_BIT */
        LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr );

        /* read  first word of result into word buffer */
	CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR, oneWordBuffer );

        /* output non aligned valid bytes of result */
			/* put out the first non aligned bytes of the result */
	    for(i = 0; i < 4 - outStartAlign; i++)
	    {
		    DataOut_ptr[i] = ((DxUint8_t*)&oneWordBuffer)[i + outStartAlign];
	    }

		/* increment data input and output pointers */
		DataIn_ptr += 4;
		DataOut_ptr += 4 - outStartAlign;

       /* update remaining data size */
		numWordsToInput -= 1;
		numWordsToOutput -= 1;

   }

   /* Step 3. If number of words remaining to input > 0, then call LLFCD_RC4_Stream
              function for performing RC4 operation on numWordsToInput words of data.
              Note: Now both the input and output pointers are aligned !                */
   /* --------------------------------------------------------------------------------- */
   if( numWordsToInput > 0 )
   {
	   LLFCD_RC4_StreamDirect(
                         DataIn_ptr,
                         DataOut_ptr,
                         4*(numWordsToInput),
                         VirtualHwBaseAddr );

		/* increment data input and output pointers */
		DataIn_ptr += 4*(numWordsToInput);
		DataOut_ptr += 4*(numWordsToInput);

       /* update remaining data size */
        numWordsToOutput -= numWordsToInput;
		numWordsToInput = 0;
   }


    /* Step 6. If the output word is the last, then set READ_ALIGN_LAST register
               and read result in word buffer */
    /* ----------------------------------------------------------------------------- */
    if( outStartAlign != 0 )
    {
        /* set READ_ALIGN_LAST register */
	    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_READ_ALIGN_LAST_REG_ADDR, 0x1 );

	    /* read the last word in temp buffer */
		CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR, oneWordBuffer );

		/* put out the non aligned bytes of the result */
	    for(i = 0; i < outStartAlign; i++)
	    {
		    DataOut_ptr[i] = ((DxUint8_t*)&oneWordBuffer)[i];
	    }
    }

    //PRINTF("LLF_RC4_StreamDirect function ended\n\n");
   /*..............   End of function ............ */

   return CRYS_OK;

}/* END OF LLF_RC4_StreamDirect */



/************************************************************************************************/
/**
 * @brief This function operates the RC4 Stream operation on full words of data on the DMA mode.
 *
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 * @param[in] DataIn_ptr  - The input data pointer.
 * @param[in] DataOut_ptr - The output data pointer.
 * @param[in] DataInSize  - The data size to operate the RC4 with this function.
 *
 * @param[in] inStartAlign  - The input start alignment.
 * @param[in] outStartAlign - The output start alignment.
 *                            NOTE: the data size must be multiple of 4 bytes.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t LLF_RC4_StreamDma(
              DxUint32_t           VirtualHwBaseAddr,
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t            inStartAlign,
              DxUint32_t            outStartAlign )
{
  /* LOCAL DECLARATION */

  /* initialize the error identifier as OK */
  CRYSError_t Error = CRYS_OK;


   /* FUNCTION LOGIC */

   /*                initializations             */
   /* ------------------------------------------ */

  /* prevent compiler warnings */
  inStartAlign = inStartAlign;
  outStartAlign = outStartAlign;

  PRINTF("LLF_RC4_StreamDma function started\n\n");


  /*  initialize the LLI */
  //commented by Ipsita

  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DST_LLI_WORD0_ADDR , (DxUint32_t)DataOut_ptr );

  /* initialize the RC4 destination size and set the first & last LLI word */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DST_LLI_WORD1_ADDR ,
                               DataInSize  |
                               (DxUint32_t)(1 << LLF_RC4_HW_DST_LLI_WORD1_FIRST_LLI_WORD_POS) |
                               (DxUint32_t)(1 << LLF_RC4_HW_DST_LLI_WORD1_LAST_LLI_WORD_POS) );


  /* initialize the source LLI address */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_SRC_LLI_WORD0_ADDR , (DxUint32_t)DataIn_ptr );

  /* initialize the source size and set the first & last LLI word - this triggers the operation */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_SRC_LLI_WORD1_ADDR ,
                               DataInSize  |
                               (DxUint32_t)(1 << LLF_RC4_HW_SRC_LLI_WORD1_FIRST_LLI_WORD_POS) |
                               (DxUint32_t)(1 << LLF_RC4_HW_SRC_LLI_WORD1_LAST_LLI_WORD_POS) );

 //#ifdef DX_ARM_INTEGRATOR_DEBUG
  //  LLF_COMMON_DebugDelay( 1000 );
 //#endif

  /* wait on the DMA DIN busy */
  LLF_RC4_HW_WAIT_ON_DIN_MEM_DMA_BUSY_BIT( VirtualHwBaseAddr );

  /* wait for the hardware machine to be ready */
  LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr );

  /* wait on the DMA DOUT busy */
  LLF_RC4_HW_WAIT_ON_DOUT_MEM_DMA_BUSY_BIT( VirtualHwBaseAddr );

  PRINTF("LLF_RC4_StreamDma function ended\n\n");

  return Error;
  //CC_Hw_InterFace_Map_And_Start_DMA(DataIn_ptr, DataOut_ptr, DataInSize);
  //CC_Hw_InterFace_Wait_for_RC4_to_Finish();
  //CC_Hw_InterFace_unMap();


}/* END OF LLF_RC4_StreamDma */

/************************************************************************************************/
/**
 * @brief The LLF_RC4_StreamRemainData function performs the RC4 operation on last not full word of data.
 *
 *
 * @param[in] VirtualHwBaseAddr    - The hardware virtual base address.
 * @param[in] WorkingContextID_ptr - The RC4 context allocated by the CCM.
 * @param[in] DataIn_ptr           - The pointer to the users data output buffer.
 * @param[in] DataOut_ptr          - The pointer to the users data output buffer.
 * @param[in] DataInSize           - The data size to operate the RC4 with this function.
 *                                   NOTE: the data size must be less, than 4 bytes.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
CRYSError_t  LLF_RC4_StreamRemainingData(
                           DxUint32_t       VirtualHwBaseAddr,       /*in*/
	                       DxUint8_t       *DataIn_ptr,              /*in*/
	                       DxUint32_t       DataInSize,              /*in*/
	                       DxUint8_t       *DataOut_ptr )            /*out*/
{

     /* LOCAL DECLARATIONS */

   /* buffers for input-output values  */
   DxUint32_t  outWordBuffer[2] = { 0, 0 };

   /* count words to input and to output */
   DxUint32_t  countWordsToInput;


   /* FUNCTION LOGIC */

   /* calculate count words to input  */
   countWordsToInput = ( DataInSize + 3 ) / 4;

   PRINTF("LLF_RC4_StreamRemainingData function entered\n\n");
   /*------------------------------------------------------------------------*/
   /* reset DIN and DOUT aligning registers according to output end aligning */
   /*------------------------------------------------------------------------*/

   /* setting the CRYPTO_CTL register to RC4 mode - the machine is now configured to work on RC4 */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_CRYPTO_CTL_ADDR ,
                                LLF_RC4_HW_CRYPTO_CTL_RC4_MODE_VAL );

   CRYS_PLAT_SYS_WriteRegister(
             VirtualHwBaseAddr + LLF_RC4_HW_DIN_WRITE_ALIGN_ADDR, 0UL | ((DataInSize & 0x3 ) << CRYS_COMMON_BusAlignerShift) );



   /* setting read (DOUT) alignment register */
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DOUT_READ_ALIGN_ADDR, 0UL );


   /*------------------------------------------------------------------------*/
   /*                   process the data                                     */
   /*------------------------------------------------------------------------*/

   /* if count words to input = 2 process the first word */
   if( countWordsToInput > 0 )
   {
        /* because the current input word is the last set WRITE_ALIGN_LAST register */
        CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_WRITE_ALIGN_LAST_REG_ADDR, 1UL );

	    /* write the word into mashine DIN_DOUT register */
	    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR, ((DxUint32_t*)DataIn_ptr)[0] );

	    /* wait on CRYPTO_BUSY_BIT */
	    LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT( VirtualHwBaseAddr );

        /* read the current result in output word buffer */
        CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR, outWordBuffer[0] );
   }

   /* put the result into output buffer */
   DX_VOS_FastMemCpy( DataOut_ptr, (DxUint8_t*)outWordBuffer, DataInSize );


	/* END OF FUNCTION */
  PRINTF("LLF_RC4_StreamRemainingData function ended\n\n");
  return CRYS_OK;
}
