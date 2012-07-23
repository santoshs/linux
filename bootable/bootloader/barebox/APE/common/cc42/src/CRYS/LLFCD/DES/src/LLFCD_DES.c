/*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Sun Feb 13 11:53:11 2005
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLFCD_BlockCTRModeNonAlign.c#1:csrc:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

#include "DX_VOS_Mem.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "LLF_DES_HwDefs.h"
#include "LLFCD_DES.h"




  /*
   *  Object % LLFCD_DES_H %
   *  State           :  %state%
   *  Creation date   :  Tue Nov 23 17:28:52 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief the file containing the functions of the CPU depended functions releated
   *         to the DES module.
   *
   *  \version LLFCD_DES.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/
/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones.
 *
 *        This function should be operated in ECB or CBC mode.
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
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

void LLFCD_DES_BlockECBandCBCModes(
              DxUint8_t            *DataIn_ptr,
              DxUint8_t            *DataOut_ptr ,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{
	/*It is assum that both DataIn_ptr and DataOut_ptr are aligned        */
	/*when this function is called (after LLF_DES_BlockExecFirstOperation) */
	/* pointer for the aligned input data*/
	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)DataIn_ptr;
	/* pointer for the aligned Output data*/
	DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)DataOut_ptr;

	/*local loop variable*/
	DxUint32_t i = 0;

	/* STEP 1: init phase read prevois block from machine */
    for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )
    {
		CRYS_PLAT_SYS_ReadRegister(
	    VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,
	    DataOut32Bit_ptr[i]);
    }

	/* STEP 2: increment the output buffer pointer */
	DataOut32Bit_ptr = DataOut32Bit_ptr + CRYS_DES_BLOCK_SIZE_IN_WORDS;

	/* STEP 3: loop of writing block to to input wait on busy bit	*/
	/* and read output from DOUT									*/
	/* Since this function should leave the last block in			*/
	/* the machine for the finish function to read the loop is		*/
	/* while the data size is more than one block					*/
	while(DataInSize > CRYS_DES_BLOCK_SIZE_IN_BYTES )
	{

	    /* STEP 3.1: load the next block to the machine */
	    for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
	          CRYS_PLAT_SYS_WriteRegister(
	         VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR,
	          DataIn32Bit_ptr[i] );
	    }

		/* STEP 3.2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + CRYS_DES_BLOCK_SIZE_IN_WORDS;

		/* STEP 3.3 DES busy bit */
		LLF_DES_HW_WAIT_ON_DES_BUSY_BIT(VirtualHwBaseAddr);
		/* STEP 3.4 read block from machine */
	    for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
		    CRYS_PLAT_SYS_ReadRegister(
		    VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,
		    DataOut32Bit_ptr[i]);
	    }

		/* STEP 3.5: increment the output buffer pointer */
		DataOut32Bit_ptr = DataOut32Bit_ptr + CRYS_DES_BLOCK_SIZE_IN_WORDS;

		DataInSize = DataInSize - CRYS_DES_BLOCK_SIZE_IN_BYTES;
	}


	/* STEP 4: post loop phase, write last block to machine */
    for( i = 0 ; i < CRYS_DES_BLOCK_SIZE_IN_WORDS ; i++ )
    {
	    CRYS_PLAT_SYS_WriteRegister(
	    VirtualHwBaseAddr + LLF_DES_HW_DIN_DOUT_ADDR ,
	    DataIn32Bit_ptr[i]);
    }

	/* STEP 5: DES busy bit */
	LLF_DES_HW_WAIT_ON_DES_BUSY_BIT(VirtualHwBaseAddr);



}
