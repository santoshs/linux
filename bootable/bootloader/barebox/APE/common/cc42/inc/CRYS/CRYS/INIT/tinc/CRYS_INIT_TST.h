

#ifndef CRYS_INIT_TST_H
#define CRYS_INIT_TST_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "CRYS_error.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %CRYS_INIT_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:08:02 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version CRYS_INIT_error.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/
#define 	 ENV_FUSE_SEL_ADDR_REG_ADDR 		 0xc00080ACUL
#define 	 ENV_FUSES_REG_ADDR 		         0xc0008300UL 
#define      HOST_CC_SW_RST                      0xc0000BECUL

#define      HOST_OTP_SIZE_IN_WORDS			     0x1CUL

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/************************ Public Functions ******************************/

/**
 * @brief This function initializes the CRYS SW for the testing platform.
 *
 *  The function should be called before the CRYS_INIT function.
 *
 * @param[in] - HwBaseAddress the hardware base address.
 
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

void  CRYS_TST_Init( void );

#ifdef __cplusplus
}
#endif

#endif



