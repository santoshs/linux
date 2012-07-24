
  /*
   *  Object %LLF_RC4_version.c    : %
   *  State           :  %state%
   *  Creation date   :  05 June 2007
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes all of the common variables and function needed to operate
   *         all of the CRYS functionality.
   *
   *  \version LLF_RC4_version.c.#1:csrc:1
   *  \author R.Levin (using as templet file created by adams)
   *  \remarks Copyright (C) 2007 by Discretix Technologies Ltd.
   *           All Rights reserved
   */

/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS.h"


/************************ Defines ************************************/

/* the CRYS release definitions */

#define LLF_RC4_RELEASE_TYPE       CRYS_DEFS_CC_6_ENGINE_TYPE
#define LLF_RC4_MAGOR_VERSION_NUM     6
#define LLF_RC4_MINOR_VERSION_NUM     1
#define LLF_RC4_SUB_VERSION_NUM       0
#define LLF_RC4_INTERNAL_VERSION_NUM  0

/************************ Enums ***************************************/


/************************ Typedefs ************************************/

/************************ Global Data *********************************/

/******************** Private function prototype **********************/

/************************ Public Functions ****************************/

/**
 * @brief This Api returnes the LLF RC4 version.
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

 void  LLF_RC4_GetVersion(CRYS_ComponentVersion_t *version_ptr)
 {
    /* LOCAL DECLARATIONS */

    /* FUNCTION LOGIC */

    /* .............. seting the CRYS version .................. */

    version_ptr->compName[0] = 'R';
    version_ptr->compName[1] = 'C';
    version_ptr->compName[2] = '4';
    version_ptr->compName[3] =  0;

    version_ptr->type     = LLF_RC4_RELEASE_TYPE;
    version_ptr->major    = LLF_RC4_MAGOR_VERSION_NUM;
    version_ptr->minor    = LLF_RC4_MINOR_VERSION_NUM;
    version_ptr->sub      = LLF_RC4_SUB_VERSION_NUM;
    version_ptr->internal = LLF_RC4_INTERNAL_VERSION_NUM;

    return;

 }/* END OF LLF_RC4_GetVersion */
