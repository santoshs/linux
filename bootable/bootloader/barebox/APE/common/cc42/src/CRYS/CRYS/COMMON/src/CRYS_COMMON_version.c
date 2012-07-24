/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

/************* Include Files ****************/

/* .................. CRYS level includes ............. */

#include "DX_VOS_Mem.h"
#include "CRYS.h"

/* ................. low level includes ............... */

#ifndef CRYS_NO_AES_SUPPORT
#include "LLF_AES_version.h"
#endif

#ifndef CRYS_NO_DES_SUPPORT
#include "LLF_DES_version.h"
#endif

#ifndef CRYS_NO_HASH_SUPPORT
#include "LLF_HASH_version.h"
#endif

#if !defined(CRYS_NO_HASH_SUPPORT) && !defined(CRYS_NO_PKI_SUPPORT)
#include "LLF_PKI_version.h"
#endif

#if !defined(CRYS_NO_HASH_SUPPORT) && !defined(CRYS_NO_ECPKI_SUPPORT)
#include "LLF_ECPKI_version.h"
#endif

#ifndef CRYS_NO_RC4_SUPPORT
#include "LLF_RC4_version.h"
#endif

#ifndef CRYS_NO_RND_SUPPORT
#include "LLF_RND_version.h"
#endif

/************************ Defines ******************************/

/* the CRYS release definitions */

#define CRYS_RELEASE_TYPE         'A'
#define CRYS_MAGOR_VERSION_NUM     6
#define CRYS_MINOR_VERSION_NUM     3
#define CRYS_SUB_VERSION_NUM       4
#define CRYS_INTERNAL_VERSION_NUM  1

/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/**
 * @brief This Api returnes the CRYS version.
 *
 * The version containes the following:
 *
 * component string - a string describing the nature of the release.
 * release type : 'D' - development , 'A' - alpha (passed to the Q.A) ,
 *                'B' - beta , 'R' - release , after Q.A testing.
 *
 * major , minor , sub , internal - the release digits.
 *
 * each component : CRYS , LLF machines receives this database.
 *
 * @param[in] version_ptr - a pointer to the version structure.
 *
 */

 void  CRYS_GetVersion(CRYS_Version_t *version_ptr)
 {
    /* LOCAL DECLERATIONS */

    /* FUNCTION LOGIC */

    /* ............... Check input parameter ................... */

    if(version_ptr == (CRYS_Version_t *)DX_NULL)
	{
		return;
	}
    /* .............. clearing the version db .................. */

    DX_VOS_MemSet( version_ptr , 0 , sizeof(CRYS_Version_t) );

    /* .............. seting the CRYS version .................. */

    version_ptr->CRYS_Version.compName[0] = 'C';
    version_ptr->CRYS_Version.compName[1] = 'R';
    version_ptr->CRYS_Version.compName[2] = 'Y';
    version_ptr->CRYS_Version.compName[3] = 'S';

    version_ptr->CRYS_Version.type     = CRYS_RELEASE_TYPE;
    version_ptr->CRYS_Version.major    = CRYS_MAGOR_VERSION_NUM;
    version_ptr->CRYS_Version.minor    = CRYS_MINOR_VERSION_NUM;
    version_ptr->CRYS_Version.sub      = CRYS_SUB_VERSION_NUM;
    version_ptr->CRYS_Version.internal = CRYS_INTERNAL_VERSION_NUM;

    /* setting the engine versions */

    /* .................... getting the low level engines versions ................. */
    /* if an engine does not exist then we shall return on the spesific engine
       version 0.0.0.0 and CRYS_NOT_SUPPORTED type */

    /* .............. setting the AES version .................... */

    #ifndef CRYS_NO_AES_SUPPORT
    LLF_AES_GetVersion(&version_ptr->LLF_AES_Version);
    #else
    version_ptr->LLF_AES_Version.compName[0] = 'A';
    version_ptr->LLF_AES_Version.compName[1] = 'E';
    version_ptr->LLF_AES_Version.compName[2] = 'S';
    version_ptr->LLF_AES_Version.compName[3] =  '\0';
    version_ptr->LLF_AES_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #endif /* CRYS_NO_AES_SUPPORT */

    /* .............. setting the DES version .................... */

    #ifndef CRYS_NO_DES_SUPPORT
    LLF_DES_GetVersion(&version_ptr->LLF_DES_Version);
    #else
    version_ptr->LLF_DES_Version.compName[0] = 'D';
    version_ptr->LLF_DES_Version.compName[1] = 'E';
    version_ptr->LLF_DES_Version.compName[2] = 'S';
    version_ptr->LLF_DES_Version.compName[3] =  '\0';
    version_ptr->LLF_DES_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #endif /* CRYS_NO_DES_SUPPORT */

    /* .............. setting the HASH version .................... */

    #ifndef CRYS_NO_HASH_SUPPORT
    LLF_HASH_GetVersion(&version_ptr->LLF_HASH_Version);
    #else
    version_ptr->LLF_HASH_Version.compName[0] = 'H';
    version_ptr->LLF_HASH_Version.compName[1] = 'A';
    version_ptr->LLF_HASH_Version.compName[2] = 'S';
    version_ptr->LLF_HASH_Version.compName[3] = 'H';
    version_ptr->LLF_HASH_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #endif /* CRYS_NO_HASH_SUPPORT */

    /* .............. setting the PKI version .................... */

    #if defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_PKI_SUPPORT)
    version_ptr->LLF_PKI_Version.compName[0] = 'P';
    version_ptr->LLF_PKI_Version.compName[1] = 'K';
    version_ptr->LLF_PKI_Version.compName[2] = 'I';
    version_ptr->LLF_PKI_Version.compName[3] = '\0';
    version_ptr->LLF_PKI_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #else
    LLF_PKI_GetVersion(&version_ptr->LLF_PKI_Version);
    #endif /* defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_PKI_SUPPORT) */

    /* .............. setting the ECPKI version .................... */

    #if defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_ECPKI_SUPPORT)
    version_ptr->LLF_ECPKI_Version.compName[0] = 'E';
    version_ptr->LLF_ECPKI_Version.compName[1] = 'C';
    version_ptr->LLF_ECPKI_Version.compName[2] = 'P';
    version_ptr->LLF_ECPKI_Version.compName[3] = 'K';
    version_ptr->LLF_ECPKI_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #else
    LLF_ECPKI_GetVersion(&version_ptr->LLF_ECPKI_Version);
    #endif /* defined(CRYS_NO_HASH_SUPPORT) || defined(CRYS_NO_ECPKI_SUPPORT) */

    /* .............. setting the RC4 version .................... */

    #ifdef CRYS_NO_RC4_SUPPORT
    version_ptr->LLF_ECPKI_Version.compName[0] = 'R';
    version_ptr->LLF_ECPKI_Version.compName[1] = 'C';
    version_ptr->LLF_ECPKI_Version.compName[2] = '4';
    version_ptr->LLF_ECPKI_Version.compName[3] = '\0';
    version_ptr->LLF_ECPKI_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #else
    LLF_RC4_GetVersion(&version_ptr->LLF_RC4_Version);
    #endif /* defined(CRYS_NO_RC4_SUPPORT) */


	/* .............. setting the RND version .................... */
    #ifdef CRYS_NO_RND_SUPPORT
    version_ptr->LLF_ECPKI_Version.compName[0] = 'R';
    version_ptr->LLF_ECPKI_Version.compName[1] = 'N';
    version_ptr->LLF_ECPKI_Version.compName[2] = 'D';
    version_ptr->LLF_ECPKI_Version.compName[3] = '\0';
    version_ptr->LLF_ECPKI_Version.type        =  CRYS_DEFS_NOT_SUPPORTED;
    #else
    LLF_RND_GetVersion(&version_ptr->LLF_RND_Version);
    #endif /* defined(CRYS_NO_RND_SUPPORT) */

    return;

 }/* END OF CRYS_GetVersion */
