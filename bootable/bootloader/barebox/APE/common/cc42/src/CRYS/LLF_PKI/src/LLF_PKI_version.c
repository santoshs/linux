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
#include "CRYS_Defs.h"
#include "CRYS_version.h"

/************************ Defines ******************************/

/* the CRYS release definitions */

#define LLF_PKI_RELEASE_TYPE          CRYS_DEFS_CC6_PKA_ENGINE_TYPE
#define LLF_PKI_MAGOR_VERSION_NUM     6
#define LLF_PKI_MINOR_VERSION_NUM     0
#define LLF_PKI_SUB_VERSION_NUM       2
#define LLF_PKI_INTERNAL_VERSION_NUM  0

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

 void  LLF_PKI_GetVersion(CRYS_ComponentVersion_t *version_ptr)
 {
    /* LOCAL DECLERATIONS */

    /* FUNCTION LOGIC */

    /* .............. seting the CRYS version .................. */

    version_ptr->compName[0] = 'P';
    version_ptr->compName[1] = 'K';
    version_ptr->compName[2] = 'I';
    version_ptr->compName[3] = 0;

    version_ptr->type     = LLF_PKI_RELEASE_TYPE;
    version_ptr->major    = LLF_PKI_MAGOR_VERSION_NUM;
    version_ptr->minor    = LLF_PKI_MINOR_VERSION_NUM;
    version_ptr->sub      = LLF_PKI_SUB_VERSION_NUM;
    version_ptr->internal = LLF_PKI_INTERNAL_VERSION_NUM;

    return;

 }/* END OF LLF_PKI_GetVersion */
