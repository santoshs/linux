
  /*
   *  Object %LLF_HASH_version.c    : %
   *  State           :  %state%
   *  Creation date   :  Mon Nov 22 10:22:57 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file containes all of the common variables and function needed to operate
   *         all of the CRYS functionality.
   *
   *  \version CRYS_COMMON_version.c.#1:csrc:1
   *  \author adams
   *  \remarks Copyright (C) 2004 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS.h"

/************************ Defines ******************************/

/* the CRYS release definitions */

#define LLF_HASH_RELEASE_TYPE       CRYS_DEFS_CC_6_ENGINE_TYPE
#define LLF_HASH_MAGOR_VERSION_NUM     6
#define LLF_HASH_MINOR_VERSION_NUM     0
#define LLF_HASH_SUB_VERSION_NUM       2
#define LLF_HASH_INTERNAL_VERSION_NUM  0

/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

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

 void  LLF_HASH_GetVersion(CRYS_ComponentVersion_t *version_ptr)
 {
    /* LOCAL DECLERATIONS */

    /* FUNCTION LOGIC */

    /* .............. seting the CRYS version .................. */

    version_ptr->compName[0] = 'H';
    version_ptr->compName[1] = 'A';
    version_ptr->compName[2] = 'S';
    version_ptr->compName[3] = 'H' ;

    version_ptr->type     = LLF_HASH_RELEASE_TYPE;
    version_ptr->major    = LLF_HASH_MAGOR_VERSION_NUM;
    version_ptr->minor    = LLF_HASH_MINOR_VERSION_NUM;
    version_ptr->sub      = LLF_HASH_SUB_VERSION_NUM;
    version_ptr->internal = LLF_HASH_INTERNAL_VERSION_NUM;

    return;

 }/* END OF LLF_DES_GetVersion */
