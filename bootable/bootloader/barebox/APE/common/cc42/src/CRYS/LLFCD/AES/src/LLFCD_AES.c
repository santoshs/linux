
/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

#ifndef LLFCD_AES_C
#define LLFCD_AES_C

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"
#include "DX_VOS_Sem.h"
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#include "LLF_AES.h"
#include "LLF_AES_error.h"
#include "LLF_AES_HwDefs.h"
#include "LLFCD_AES.h"


#ifdef __cplusplus
extern "C"
{
#endif




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

/************************ Enums ********************************/


/************************ Typedefs  ****************************/
#ifndef DX_REG_ACCESS_WITH_NO_PRINT
	#define LLFCD_AES_PLAT_SYS_WriteRegister   CRYS_PLAT_SYS_WriteRegister
	#define LLFCD_AES_PLAT_SYS_ReadRegister    CRYS_PLAT_SYS_ReadRegister
	#define LLFCD_AES_HW_WAIT_ON_AES_BUSY_BIT  LLF_AES_HW_WAIT_ON_AES_BUSY_BIT
#else
	#define LLFCD_AES_PLAT_SYS_WriteRegister   CRYS_PLAT_SYS_WriteRegister_NO_PRINT
	#define LLFCD_AES_PLAT_SYS_ReadRegister    CRYS_PLAT_SYS_ReadRegister_NO_PRINT
	#define LLFCD_AES_HW_WAIT_ON_AES_BUSY_BIT  LLF_AES_HW_WAIT_ON_AES_BUSY_BIT_NO_PRINT
#endif

/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones.
 *
 *        This function should be operated in ECB ,CBC and CTR mode.
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the AES with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_AES_BlockECBandCBCandCTRModes(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{


	/*It is assum that both DataIn_ptr and DataOut_ptr are aligned        */
	/*when this function is called (after LLF_AES_BlockExecFirstOperation) */
	/* pointer for the aligned input data*/
	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)DataIn_ptr;
	/* pointer for the aligned Output data*/
	DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)DataOut_ptr;

	/*local loop variable*/
	DxUint32_t i;

	/* STEP 1: init phase read prevois block from machine */
    for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
    {
		LLFCD_AES_PLAT_SYS_ReadRegister(
	    VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
	    DataOut32Bit_ptr[i]);
    }

	/* STEP 2: increment the output buffer pointer */
	DataOut32Bit_ptr = DataOut32Bit_ptr + CRYS_AES_BLOCK_SIZE_IN_WORDS;

	/* STEP 3: loop of writing block to to input wait on busy bit	*/
	/* and read output from DOUT									*/
	/* Since this function should leave the last block in			*/
	/* the machine for the finish function to read the loop is		*/
	/* while the data size is more than one block					*/
	while(DataInSize > CRYS_AES_BLOCK_SIZE_IN_BYTES )
	{

	    /* STEP 3.1: load the next block to the machine */
	    for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
	          LLFCD_AES_PLAT_SYS_WriteRegister(
	                         VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	                         DataIn32Bit_ptr[i] );
	    }

		/* STEP 3.2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + CRYS_AES_BLOCK_SIZE_IN_WORDS;

		/* STEP 3.3 AES busy bit */
		LLFCD_AES_HW_WAIT_ON_AES_BUSY_BIT(VirtualHwBaseAddr);

		/* STEP 3.4 read block from machine */
	    for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
		    LLFCD_AES_PLAT_SYS_ReadRegister(
						    VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
						    DataOut32Bit_ptr[i]);
	    }

		/* STEP 3.5: increment the output buffer pointer */
		DataOut32Bit_ptr = DataOut32Bit_ptr + CRYS_AES_BLOCK_SIZE_IN_WORDS;

		DataInSize = DataInSize - CRYS_AES_BLOCK_SIZE_IN_BYTES;
	}


	/* STEP 4: post loop phase, write last block to machine */
    for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
    {
	    LLFCD_AES_PLAT_SYS_WriteRegister(
					    VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
					    DataIn32Bit_ptr[i]);
    }

	/* STEP 5: AES busy bit */
	LLFCD_AES_HW_WAIT_ON_AES_BUSY_BIT(VirtualHwBaseAddr);

}

/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones.
 *
 *        This function should be operated in MAC mode.
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataInSize  - The exect data size to operate the AES with this function.( does not include the
 *                          first and finish operations )
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_AES_BlockMacMode(
              DxUint8_t            *DataIn_ptr,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{

	/*It is assum that both DataIn_ptr is aligned        */
	/*when this function is called (after LLF_AES_BlockExecFirstOperation) */
	/* pointer for the aligned input data*/
	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)DataIn_ptr;

	/*local loop variable*/
	DxUint32_t i = 0;

	/* STEP 1: loop of writing block to to input wait on busy bit		*/
	/* Since in this mode we only write blocks to the machine there		*/
	/* is no realy difference between preloop,loop and post loop		*/
	/* code (the loop is till the data ends)*/
	while( DataInSize )
	{

	    /* STEP 1.1: load the next block to the machine */
	    for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
	          LLFCD_AES_PLAT_SYS_WriteRegister(
	          VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR,
	          DataIn32Bit_ptr[i] );
	    }

		/* STEP 1.2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + CRYS_AES_BLOCK_SIZE_IN_WORDS;

		/* STEP 1.3 AES busy bit */
		LLFCD_AES_HW_WAIT_ON_AES_BUSY_BIT(VirtualHwBaseAddr);
		DataInSize = DataInSize - CRYS_AES_BLOCK_SIZE_IN_BYTES;
	}
}



#ifdef __cplusplus
}
#endif

#endif
