/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "CRYS.h"

/************************ Defines ******************************/

/* the CRYS release definitions */

#define LLF_C2_RELEASE_TYPE        CRYS_DEFS_CF6_ENGINE_TYPE
#define LLF_C2_MAGOR_VERSION_NUM     6
#define LLF_C2_MINOR_VERSION_NUM     1
#define LLF_C2_SUB_VERSION_NUM       0
#define LLF_C2_INTERNAL_VERSION_NUM  0

/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/**
* @brief This Api returnes the LLF C2 version.
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

void  LLF_C2_GetVersion(CRYS_ComponentVersion_t *version_ptr)
{
    /* LOCAL DECLERATIONS */

    /* FUNCTION LOGIC */

    /* .............. seting the CRYS version .................. */

    version_ptr->compName[0] = 'C';
    version_ptr->compName[1] = '2';
    version_ptr->compName[2] = 0;
    version_ptr->compName[3] = 0;

    version_ptr->type     = LLF_C2_RELEASE_TYPE;
    version_ptr->major    = LLF_C2_MAGOR_VERSION_NUM;
    version_ptr->minor    = LLF_C2_MINOR_VERSION_NUM;
    version_ptr->sub      = LLF_C2_SUB_VERSION_NUM;
    version_ptr->internal = LLF_C2_INTERNAL_VERSION_NUM;

    return;

}/* END OF LLF_C2_GetVersion */