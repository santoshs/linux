
  /*
   *  Object %LLF_AESGCM_version.c    : %
   *  State           :  %state%
   *  Creation date   :  21 March 2011
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief This file contains a function, getting the current version of LLF AESGCM.
   *
   *  \version LLF_AESGCM_version.c.#1:csrc:1
   *  \author R.levin
   *  \remarks Copyright (C) 2011 by Discretix Technologies Ltd.
   *           All Rights reserved
   */



/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS_Defs.h"
#include "CRYS_version.h"

/************************ Defines ******************************/

/* the CRYS release definitions */

#define LLF_AESGCM_RELEASE_TYPE       CRYS_DEFS_CC_6_ENGINE_TYPE
#define LLF_AESGCM_MAGOR_VERSION_NUM     6
#define LLF_AESGCM_MINOR_VERSION_NUM     0
#define LLF_AESGCM_SUB_VERSION_NUM       0
#define LLF_AESGCM_INTERNAL_VERSION_NUM  1

/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/**
 * @brief This API returns the LLF AESGCM version.
 *
 * The version contains the following:
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

 void  LLF_AESGCM_GetVersion(CRYS_ComponentVersion_t *version_ptr)
 {
    /* LOCAL DECLERATIONS */

    /* FUNCTION LOGIC */

    /* .............. seting the CRYS version .................. */

    version_ptr->compName[0] = 'A';
    version_ptr->compName[1] = 'G';
    version_ptr->compName[2] = 'C';
    version_ptr->compName[3] = 'M';

    version_ptr->type     = LLF_AESGCM_RELEASE_TYPE;
    version_ptr->major    = LLF_AESGCM_MAGOR_VERSION_NUM;
    version_ptr->minor    = LLF_AESGCM_MINOR_VERSION_NUM;
    version_ptr->sub      = LLF_AESGCM_SUB_VERSION_NUM;
    version_ptr->internal = LLF_AESGCM_INTERNAL_VERSION_NUM;

    return;

 }/* END OF LLF_AESGCM_GetVersion */
