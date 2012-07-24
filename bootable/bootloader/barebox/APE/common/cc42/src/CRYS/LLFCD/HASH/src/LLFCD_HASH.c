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
#include "LLF_HASH_HwDefs.h"
#include "LLFCD_HASH.h"
#include "log_output.h"



  /*
   *  Object % LLFCD_HASH_H %
   *  State           :  %state%
   *  Creation date   :  Tue Nov 23 17:28:52 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief the file containing the functions of the CPU depended functions releated
   *         to the DES module.
   *
   *  \version LLFCD_HASH.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public functions ******************************************************/
/**
 * @brief This function operates the update execution of all of the blocks except the first
 *        and last ones ( not including the padding ).
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_HASH_Updat calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the HASH with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_HASH_Update(
              DxUint8_t            *DataIn_ptr,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{

	/*It is assum that both DataIn_ptr is aligned        */
	/* pointer for the aligned input data*/
	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)DataIn_ptr;

	/*local loop variable*/
	DxUint32_t i = 0;
        PRINTF("LLFCD_HASH_Update function starts\n\n");

	/* STEP 1: loop of writing block to to input wait on busy bit		*/
	/* Since in this mode we only write blocks to the machine there		*/
	/* is no realy difference between preloop,loop and post loop		*/
	/* code (the loop is till the data ends)*/
	while( DataInSize )
	{

	    /* STEP 1.1: load the next block to the machine */
	    for( i = 0 ; i < CRYS_HASH_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
	          CRYS_PLAT_SYS_WriteRegister(
	          VirtualHwBaseAddr + LLF_HASH_HW_DIN_ADDR,
	          DataIn32Bit_ptr[i] );
	    }

		/* STEP 1.2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + CRYS_HASH_BLOCK_SIZE_IN_WORDS;

		/* STEP 1.3 HASH busy bit */
		LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT(VirtualHwBaseAddr);
		DataInSize = DataInSize - CRYS_HASH_BLOCK_SIZE_IN_BYTES;
	}
       PRINTF("Input block insertion completed\n");
}


/************************************************************************************************/
/**
 * @brief This function operates the update execution of all of the blocks except the first
 *        and last ones ( not including the padding ).
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_HASH_Updat calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the HASH with this function.( does not include the
 *                          first and finish operations )
 *
 * @param[in] VirtualHwBaseAddr - The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void LLFCD_HASH_SHA2_Update(
              DxUint8_t            *DataIn_ptr,
              DxUint32_t            DataInSize,
              DxUint32_t           VirtualHwBaseAddr )
{

	/*It is assum that both DataIn_ptr is aligned        */
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
	    for( i = 0 ; i < CRYS_HASH_SHA2_BLOCK_SIZE_IN_WORDS ; i++ )
	    {
	          CRYS_PLAT_SYS_WriteRegister(
	          VirtualHwBaseAddr + LLF_HASH_HW_DIN_ADDR,
	          DataIn32Bit_ptr[i] );
	    }

		/* STEP 1.2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + CRYS_HASH_SHA2_BLOCK_SIZE_IN_WORDS;

		/* STEP 1.3 HASH busy bit */
		LLF_HASH_HW_WAIT_ON_HASH_BUSY_BIT(VirtualHwBaseAddr);
		DataInSize = DataInSize - CRYS_HASH_SHA2_BLOCK_SIZE_IN_BYTES;
	}
}
