/*
   *  Object %name    : % LLFCD_RC4.c
   *  State           :  %state%
   *  Creation date   :  12 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLFCD_RC4.c#1:csrc:1
   *  \author R.Levin
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

#include "DX_VOS_Mem.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "LLF_RC4.h"
#include "LLF_RC4_HwDefs.h"
#include "LLFCD_RC4.h"

#include "log_output.h"



/************************ Defines ******************************/

/************************ Enums ********************************/

/************************ Typedefs  ****************************/

/************************ Structs  *****************************/

/******************** Public functions *************************/



/**
 * @brief This function operates the RC4 stream operation on aligned input data with when the
 *        size is a multiple of 4 bytes.
 *
 *        In order to improve the performance this function may be implemented also in assembler.
 *        Therefore do not change the arguments order !! .
 *
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_RC4_SteramDirect calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_RC4_SteramDirect calling the function should update it after the execution.
 *
 * @param[in] DataInSize  - The exect data size to operate the RC4 with this function (does not include the
 *                          first and finish operations). NOTE: the size must be a multiple of 4 bytes.
 *
 * @param[in] VirtualHwBaseAddr - The HW base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 *
 *	NOTE:  It is assumed that both DataIn_ptr and DataOut_ptr are aligned when this function is called.
 *
 */

void LLFCD_RC4_StreamDirect(
                      DxUint8_t            *DataIn_ptr,
                      DxUint8_t            *DataOut_ptr ,
                      DxUint32_t            DataInSize,
                      DxUint32_t            VirtualHwBaseAddr )
{

	/* pointer for the aligned input data*/
	DxUint32_t *DataIn32Bit_ptr = (DxUint32_t *)DataIn_ptr;
	/* pointer for the aligned Output data*/
	DxUint32_t *DataOut32Bit_ptr = (DxUint32_t *)DataOut_ptr;

        //PRINTF("LLFCD_RC4_StreamDirect function starts\n\n");
	/*  Loop of writing words to to DIN, waiting on busy bit and reading the output from DOUT */

	while( DataInSize > 0 )
	{
	    /* STEP 1: load the next word to the machine */

	CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR,
	                                 *DataIn32Bit_ptr );

		/* STEP 2: increment the input buffer pointer */
		DataIn32Bit_ptr = DataIn32Bit_ptr + 1;

		/* STEP 3: wait CRYPTO busy bit */
		LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT(VirtualHwBaseAddr);

		/* STEP 4 read block from machine */

	    CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_RC4_HW_DIN_DOUT_ADDR,
		                            *DataOut32Bit_ptr );


		/* STEP 5: increment the output buffer pointer */
		DataOut32Bit_ptr = DataOut32Bit_ptr + 1;

		DataInSize = DataInSize - CRYS_RC4_HW_BLOCK_SIZE_IN_BYTES;
	}


	/* STEP 6: wait on CRYPTO busy bit */

	LLF_RC4_HW_WAIT_ON_CRYPTO_BUSY_BIT(VirtualHwBaseAddr);
        //PRINTF("LLFCD_RC4_StreamDirect function ends\n\n");

}
