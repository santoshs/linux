
#ifndef LLF_COMMON_ERROR_H
#define LLF_COMMON_ERROR_H

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
   *  Object % LLF_COMMON_error.h    : %
   *  State           :  %state%
   *  Creation date   :  Sun Nov 21 11:07:08 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief The error definitions of the LLF COMMON module
   *
   *  \version CRYS_COMMON_error.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/

#define LLF_COMMON_HW_VERSION_NOT_CORRECT_ERROR   	(LLF_COMMON_MODULE_ERROR_BASE + 0x0UL)

/* NOTE: Error numbers starting from  (LLF_COMMON_MODULE_ERROR_BASE + 0x11UL) are used 
         and defined in the CRYS_COMMON_TST.h file */

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

#ifdef __cplusplus
}
#endif

#endif

