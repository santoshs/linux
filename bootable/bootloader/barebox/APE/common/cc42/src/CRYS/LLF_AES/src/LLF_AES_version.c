/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS.h"

/************************ Defines ******************************/

/* the CRYS release definitions */

#define LLF_AES_RELEASE_TYPE       CRYS_DEFS_CF6_ENGINE_TYPE
#define LLF_AES_MAGOR_VERSION_NUM     6
#define LLF_AES_MINOR_VERSION_NUM     1
#define LLF_AES_SUB_VERSION_NUM       1
#define LLF_AES_INTERNAL_VERSION_NUM  0
/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/**
 * @brief This Api returnes the LLF AES version.
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

 void  LLF_AES_GetVersion(CRYS_ComponentVersion_t *version_ptr)
 {
    /* LOCAL DECLARATIONS */

    /* FUNCTION LOGIC */

  #ifndef CRYS_AES_CF_VERSION_USED
    return  LLF_AES_HW_VERSION_NOT_CORRECT_ERROR;
  #endif

    /* .............. seting the CRYS version .................. */

    version_ptr->compName[0] = 'A';
    version_ptr->compName[1] = 'E';
    version_ptr->compName[2] = 'S';
    version_ptr->compName[3] = 0;

    version_ptr->type     = LLF_AES_RELEASE_TYPE;
    version_ptr->major    = LLF_AES_MAGOR_VERSION_NUM;
    version_ptr->minor    = LLF_AES_MINOR_VERSION_NUM;
    version_ptr->sub      = LLF_AES_SUB_VERSION_NUM;
    version_ptr->internal = LLF_AES_INTERNAL_VERSION_NUM;

    return;

 }/* END OF LLF_AES_GetVersion */
