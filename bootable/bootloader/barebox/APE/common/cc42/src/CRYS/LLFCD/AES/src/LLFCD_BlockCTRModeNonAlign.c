
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



/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "PLAT_SystemDep.h"
#include "CRYS_COMMON_Math.h"
#include "CRYS.h"
#include "LLF_AES_HwDefs.h"
#include "LLFCD_AES.h"

/************************ Defines ******************************/


/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/


/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/**
 * @brief This function operates the block execution of all of the blocks except the first
 *        and last ones.
 *
 *        This function should be operated in CTR mode on non aligned data.
 *
 *        In order to improved the performance this function is implemented also in assembler.
 *        There for do not change the arguments order !! .
 *
 * @param[in] IVCounter_ptr - the IV needed for the CTR operation.
 * @param[in] DataIn_ptr - The exact position of the input data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataOut_ptr - The exact position of the output data - note it is not updated in the function
 *                         The LLF_AES_Block calling the function should update it after the execution.
 * @param[in] DataInSizeAndBaseAddr[2]  - a buffer containing 2 parametrs due to limitation of the number of arguments
 *                                        we can pass on the assembly function.
 *
 *                                      1)  first word : The exect data size to operate the AES with this function.
 *                                         ( does not include the first and finish operations )
 *                                      2)  second word : The base address.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void  LLFCD_AES_BlockCTRModeNonAlign(
                         DxUint32_t           *IVCounter_ptr,
                         DxUint8_t            *DataIn_ptr,
                         DxUint32_t            DataInSizeAndBaseAddr[2],
                         DxUint8_t            *DataOut_ptr )
{

   /* FUNCTION DECLERATIONS */

   /* the virtual address */
   DxUint32_t VirtualHwBaseAddr;

   /* the number of blocks to load to the hardware */
   DxUint32_t NumOfBlocksToLoadHw;

   /* the temp buffer of ohaneling the non aligned data ( in and out ) */
   DxUint32_t tempAlignedInBuff[CRYS_AES_BLOCK_SIZE_IN_WORDS];
   DxUint32_t tempAlignedOutBuff[CRYS_AES_BLOCK_SIZE_IN_WORDS];

   /* loop variable */
   DxUint32_t i,k;

   /* FUNCTION LOGIC */

   /* .................... initialize local variables ......................... */
   /* ------------------------------------------------------------------------- */

   NumOfBlocksToLoadHw = DataInSizeAndBaseAddr[0] / CRYS_AES_BLOCK_SIZE_IN_BYTES;
   VirtualHwBaseAddr   = DataInSizeAndBaseAddr[1];

   /* .................... executing the CTR .................................. */
   /* ------------------------------------------------------------------------- */

   /* loop for executing the CTR */
   for( k = 0 ; k < NumOfBlocksToLoadHw ; k++)
   {
      /* ............ calculating and loading the CTR to the output data ........... */

      /* load to the temp input buffer the information in the data in */
      DX_VOS_FastMemCpy( (DxUint8_t *)tempAlignedInBuff , DataIn_ptr , sizeof(tempAlignedInBuff) );

      /* read the data from the hardware to the output temp buffer and execute a XOR with the
         input data */
      for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
      {
         /* read the result from the Hardware */
         CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR ,
                                     tempAlignedOutBuff[i]);

         /* executing XOW with the input data */
         tempAlignedOutBuff[i] ^= tempAlignedInBuff[i];

      }/* end of preparing the output data block */

      /* copy the results to the out put buffer */
      DX_VOS_FastMemCpy( DataOut_ptr , (DxUint8_t *)tempAlignedOutBuff , sizeof(tempAlignedOutBuff) );

      /* updating the position of the input & output pointers to the next block position */
      DataIn_ptr += (CRYS_AES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t));
      DataOut_ptr += (CRYS_AES_BLOCK_SIZE_IN_WORDS * sizeof(DxUint32_t));

      /* ............ loading the next counter to the hardware ......................... */

      /* load the DATA_IN registers */
      for( i = 0 ; i < CRYS_AES_BLOCK_SIZE_IN_WORDS ; i++ )
      {
        CRYS_PLAT_SYS_WriteRegister(
          VirtualHwBaseAddr + LLF_AES_HW_DIN_DOUT_ADDR + (4 * i),
          IVCounter_ptr[i] );
      }

      /* increneting the counter by 1 */
      CRYS_COMMON_IncMsbUnsignedCounter(
           IVCounter_ptr,                         /* the counter - in / out */
           1,                                     /* the value - in */
           CRYS_AES_IV_COUNTER_SIZE_IN_WORDS );   /* the counter size */

      /* wait on the AES BUSY */
      LLF_AES_HW_WAIT_ON_AES_BUSY_BIT( VirtualHwBaseAddr );

   }/* end of loop for handeling the CTR */

   return;

}/* END OF LLFCD_AES_BlockCTRModeNonAlign */
