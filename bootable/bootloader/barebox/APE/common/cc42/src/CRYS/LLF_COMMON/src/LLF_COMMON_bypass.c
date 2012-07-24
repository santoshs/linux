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
#ifndef CRYS_NO_GLOBAL_DATA
#include "DX_VOS_Sem.h"
#else
#define DX_VOS_SemWait(semId,timeout)	CRYS_OK
#define DX_VOS_SemGive(semId)
#endif
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "LLF_COMMON_error.h"
#include "LLF_COMMON_HwDefs.h"

/************************ Defines ******************************/
/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */


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
/************* Private function prototype ****************/
CRYSError_t LLF_COMMON_Bypass_Activate_Dma_MEM_to_MEM(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr );
static CRYSError_t LLF_COMMON_Bypass_Activate_Dma_MEM_to_SRAM(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr );


/************************ Public Functions ******************************/

/**
 * @brief The low level LLF_COMMON_Bypass_Block function activates thye DMA to
 *        copy data in to data out without changing it..
 *          It calls the following low level functions:
 *
 *        1. The function gets a semaphore to protect the access to the hardware
 *           from any other process the semaphores name: "crypto hardware busy"
 *        2. The function calls a VOS system call to map the registers physical base address
 *           to the registers virtual address.
 *        3. The function configures the HW to be in bypass mode
 *        4. The function calls the LLF_COMMON_Bypass_Activate_Dma to to the DMA action.
 *        5. The function calls a VOS system call to un map the registers virtual address mapping.
 *        6. Release the "crypto hardware busy" semaphore acquired in paragraph 1.
 *
 *
 * @param[in] DataIn_ptr -  The pointer to the inpult buffer passed by the user.
 * @param[in] DataInSize -  The size of the buffer the user shall operate on.
 * @param[in] DataOut_ptr - The pointer to the output buffer passed by the user.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_COMMON_Bypass_Block(DxUint8_t       *DataIn_ptr,
                              DxUint32_t       DataInSize,
                              DxUint8_t       *DataOut_ptr )
 {
	/* LOCAL DECLERATIONS */

	/* The return error identifier */
	CRYSError_t Error;

	/* the virtual address definition */
	DxUint32_t VirtualHwBaseAddr;

	/* the updated data out and data in position */
	DxUint8_t *DataInUpdatePos_ptr;
	DxUint8_t *DataOutUpdatePos_ptr;


	/* FUNCTION LOGIC */

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

	/* initializing the local data buffer pointers to the values received by the user */
	DataInUpdatePos_ptr  = DataIn_ptr;
	DataOutUpdatePos_ptr = DataOut_ptr;

     /* ............... getting the hardware sempaphre ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	{
	return Error;
	}

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_COMMON_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}


	/* ................. general registers initializations ..................... */
	/* ------------------------------------------------------------------------- */


	/* .... setting the CRYPTO_CTL register to bypass mode  */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_CRYPTO_CTL_ADDR , LLF_COMMON_HW_CRYPTO_CTL_BYPASS_MODE_VAL);


	LLF_COMMON_Bypass_Activate_Dma_MEM_to_SRAM( DataInUpdatePos_ptr,             /* the data in updated pointer - in */
												DataOutUpdatePos_ptr ,           /* the data out updated pointer - in */
												DataInSize,                      /* the data size to load - in */
												VirtualHwBaseAddr);              /* the virtual address - in */



	/* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_COMMON_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

	return Error;

 }/* END OF LLF_COMMON_Bypass_Block */



/************************ Private Functions ******************************/
/**
 * @brief This function operates the block execution on the DMA mode. The source and destinations are external memories.
 *
 *
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *
 *
 * @param[in] DataInSize  - The exect data size to operate the pass with this function.
 *
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */


CRYSError_t LLF_COMMON_Bypass_Activate_Dma_MEM_to_MEM(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{
	/* LOCAL DECLERATION */

	/* initialize the error identifier as OK */
	CRYSError_t Error = CRYS_OK;


	/* data alignement variables */
	DxUint32_t DataOutAlignemnt = 0;
	DxUint32_t DataInAlignemnt = 0;

	/* FUNCTION LOGIC */

	/* .................... initialize local variables ...................... */

	/*	Since the  data size is written in words units and if there is alignment*/
	/*	we need to add one word for the the left bytes in the last word			*/

	DataInAlignemnt  = (DxUint32_t)DataIn_ptr & CRYS_COMMON_BusAlignerMask;
	DataOutAlignemnt  = (DxUint32_t)DataOut_ptr & CRYS_COMMON_BusAlignerMask;




	/* initialize the data in and out alignments */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DIN_WRITE_ALIGN_ADDR ,
                               DataInAlignemnt | DataInAlignemnt << CRYS_COMMON_BusAlignerShift );

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DOUT_READ_ALIGN_ADDR ,
                               DataOutAlignemnt | DataOutAlignemnt << CRYS_COMMON_BusAlignerShift );
	/* .................... executing the DMA ............................... */


	/* STEP1 : initialize the CF_IDMA_ADDR */
#ifdef DX_ARM_INTEGRATOR_DEBUG
  /* initialize the board memory select to MEM */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 1UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 1UL );

  /* coping to the board memory with access to the DMA the data */
  DX_VOS_FastMemCpy( (DxUint8_t*)(LLF_COMMON_HW_DMA_TMP_SRC_ADDR + DataInAlignemnt) , DataIn_ptr , DataInSize );

  /* initialize the board memory select to CRYPTO */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 0UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 0UL );

  /* initialize the destination LLI address */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DST_LLI_WORD0_ADDR , LLF_COMMON_HW_DMA_TMP_SRC_ADDR /* DataOut_ptr - DataOutAlignemnt */ );

#else
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DST_LLI_WORD0_ADDR ,
								DataOut_ptr - DataOutAlignemnt );
#endif

	/* STEP2 : initialize the CF_IDMA_LEN */
	/*currently only size is written*/
  /* initialize the destination size and set the first & last LLI word */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DST_LLI_WORD1_ADDR ,
                               (DataInSize + DataOutAlignemnt) |
                               (1UL << LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS) |
                               (1UL << LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS) );

	/* STEP3 : initialize the source address */
#ifdef DX_ARM_INTEGRATOR_DEBUG
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRC_LLI_WORD0_ADDR , LLF_COMMON_HW_DMA_TMP_SRC_ADDR  /*DataIn_ptr - DataInAlignemnt */ );
#else
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRC_LLI_WORD0_ADDR , (DataIn_ptr - DataInAlignemnt ));
#endif

	/* STEP4 : initialize the source size and set the first & last LLI word - this triggers the operation   */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRC_LLI_WORD1_ADDR ,
                               (DataInSize + DataInAlignemnt) |
                               (1UL << LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS) |
                               (1UL << LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS) );

	/* STEP6 : wait on the DMA */
    /*DMA WRITE (output) */
	LLF_COMMON_HW_WAIT_ON_MEM_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr );

#ifdef DX_ARM_INTEGRATOR_DEBUG
  /* initialize the board memory select to MEM */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 1UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 1UL );

   /* coping from the board memory back to the out buffer */
  DX_VOS_FastMemCpy( DataOut_ptr , (DxUint8_t*)(LLF_COMMON_HW_DMA_TMP_DST_ADDR + DataOutAlignemnt), DataInSize );

  /* initialize the board memory select to CRYPTO */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 0UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 0UL );

#endif
	return Error;

}/* END OF LLF_COMMON_Bypass_Activate_Dma */

/************************ Private Functions ******************************/
/**
 * @brief This function operates the block execution on the DMA mode. The source external memory.
 *		  The destination is local SRAM
 *
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *
 *
 * @param[in] DataInSize  - The exect data size to operate the pass with this function.
 *
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 *
 *
 *
 *	This function differs from the MEM to MEM function in the order of the configuartion.
 *  The source MEM is configured before the destination SRAM.
 *
 */


CRYSError_t LLF_COMMON_Bypass_Activate_Dma_MEM_to_SRAM(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{
	/* LOCAL DECLERATION */

	/* initialize the error identifier as OK */
	CRYSError_t Error = CRYS_OK;


	/* data alignement variables */
	DxUint32_t DataOutAlignemnt = 0;
	DxUint32_t DataInAlignemnt = 0;

	/* FUNCTION LOGIC */

	/* .................... initialize local variables ...................... */

	/*	Since the  data size is written in words units and if there is alignment*/
	/*	we need to add one word for the the left bytes in the last word			*/

	DataInAlignemnt  = (DxUint32_t)DataIn_ptr & CRYS_COMMON_BusAlignerMask;
	DataOutAlignemnt  = (DxUint32_t)DataOut_ptr & CRYS_COMMON_BusAlignerMask;


	/* initialize the data in and out alignments */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DIN_WRITE_ALIGN_ADDR ,
                               DataInAlignemnt | DataInAlignemnt << CRYS_COMMON_BusAlignerShift );

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DOUT_READ_ALIGN_ADDR ,
                               DataOutAlignemnt | DataOutAlignemnt << CRYS_COMMON_BusAlignerShift );
	/* .................... executing the DMA ............................... */


	/* STEP1 : initialize the destination SRAM address */
#ifdef DX_ARM_INTEGRATOR_DEBUG
  /* initialize the board memory select to MEM */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 1UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 1UL );

  /* coping to the board memory with access to the DMA the data */
  DX_VOS_FastMemCpy( (DxUint8_t*)(LLF_COMMON_HW_DMA_TMP_SRC_ADDR + DataInAlignemnt) , DataIn_ptr , DataInSize );

  /* initialize the board memory select to CRYPTO */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 0UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 0UL );

  /* initialize the destination LLI address */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DST_SRAM_ADDR , LLF_COMMON_HW_DMA_TMP_DST_ADDR /* DataOut_ptr - DataOutAlignemnt */ );

#else
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DST_SRAM_ADDR ,
								DataOut_ptr - DataOutAlignemnt );
#endif

	/* STEP2 : initialize the destination SRAM size */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_DST_SRAM_BYTES_LEN ,
                               (DataInSize + DataOutAlignemnt) );

    /* STEP3 : initialize the source address */
#ifdef DX_ARM_INTEGRATOR_DEBUG
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRC_LLI_WORD0_ADDR , LLF_COMMON_HW_DMA_TMP_SRC_ADDR  /*DataIn_ptr - DataInAlignemnt */ );
#else
    CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRC_LLI_WORD0_ADDR , (DataIn_ptr - DataInAlignemnt ));
#endif

	/* STEP4 : initialize the source size and set the first & last LLI word - this triggers the operation   */
  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRC_LLI_WORD1_ADDR ,
                               (DataInSize + DataInAlignemnt) |
                               (1UL << LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS) |
                               (1UL << LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS) );

	/* STEP5 : wait on the DMA */
    /*DMA WRITE (output) */
	LLF_COMMON_HW_WAIT_ON_SRAM_DMA_DEST_BUSY_BIT( VirtualHwBaseAddr );

#ifdef DX_ARM_INTEGRATOR_DEBUG
  /* initialize the board memory select to MEM */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 1UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 1UL );

   /* coping from the board memory back to the out buffer */
  DX_VOS_FastMemCpy( DataOut_ptr , (DxUint8_t*)(LLF_COMMON_HW_DMA_TMP_DST_ADDR + DataOutAlignemnt), DataInSize );

  /* initialize the board memory select to CRYPTO */
  CRYS_PLAT_SYS_WriteRegister( 0xC0008010UL , 0UL );
  CRYS_PLAT_SYS_WriteRegister( 0xC0008014UL , 0UL );

#endif
	return Error;

}/* END OF LLF_COMMON_Bypass_Activate_Dma */

/**
 * @brief The low level LLF_COMMON_SetCCToBypass function configures the CC/CF to bypass
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

  CRYSError_t  LLF_COMMON_SetCCToBypass( void )
 {
	/* LOCAL DECLERATIONS */

	/* The return error identifier */
	CRYSError_t Error;

	/* the virtual address definition */
	DxUint32_t VirtualHwBaseAddr;


	/* FUNCTION LOGIC */

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
	Error = CRYS_OK;

     /* ............... getting the hardware sempaphre ..................... */
	/* -------------------------------------------------------------------- */

	Error = DX_VOS_SemWait( SemHwAccessId , DX_INFINITE );

	if( Error != CRYS_OK )
	{
	return Error;
	}

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
                          LLF_COMMON_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
                          &VirtualHwBaseAddr );           /* The virtual address - out */

	if( Error != CRYS_OK )
	{
	goto ReturnAndReleaseSemaphore;
	}


	/* ................. general registers initializations ..................... */
	/* ------------------------------------------------------------------------- */


	/* .... setting the CRYPTO_CTL register to bypass mode  */
	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_CRYPTO_CTL_ADDR , LLF_COMMON_HW_CRYPTO_CTL_BYPASS_MODE_VAL);

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_FIFO_MODE_REG_ADDR,
						  1UL );

	/* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
                    LLF_COMMON_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

ReturnAndReleaseSemaphore:

	/* release the hardware semaphore */
	DX_VOS_SemGive ( SemHwAccessId );

	return Error;

 }/* END OF LLF_COMMON_Bypass_Block */
