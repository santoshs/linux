
#ifndef LLF_RND_version_H
#define LLF_RND_version_H

/*
 * All the includes that are needed for code using this module to
 * compile correctly should be #included here.
 */
#include "CRYS_version.h"


#ifdef __cplusplus
extern "C"
{
#endif

  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Tue Mar 22 15:20:01 2005
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version LLF_AES_version.h#1:incl:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */




/************************ Defines ******************************/

/************************ Enums ********************************/


/************************ Typedefs  ****************************/


/************************ Structs  ******************************/


/************************ Public Variables **********************/


/************************ Public Functions **********************/

/**
 * @brief This Api returnes the LLF HASH version.
 *
 * The version containes the following:
 *
 * component string - a string describing the nature of the release.
 * release type : 0 / SW 
 *
 * major , minor , sub , internal - the release digits.
 * 
 * each component : CRYS , LLF machines receives this database. 
 *
 * @param[in] version_ptr - a pointer to the version structure.
 *
 */

 void  LLF_RND_GetVersion(CRYS_ComponentVersion_t *version_ptr);


#ifdef __cplusplus
}
#endif

#endif

