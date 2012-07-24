/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "CRYS_COMMON_error.h"
#include "CRYS.h"

/************************ Defines ******************************/
#ifdef DX_USE_RND_HASH
#define BUFF0_SIZE_IN_WORDS 5 /* used for the PRNG key state : size CRYS_HASH_RESULT_SIZE_IN_WORDS */
#else
#define BUFF2_SIZE_IN_WORDS 4 /* 128 bits */
#endif
#define BUFF1_SIZE_IN_WORDS 4 /* used for the AES SK key state : size CRYS_AES_SECRET_KEY_SIZE_IN_WORDS */

/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

#if !defined(CRYS_NO_GLOBAL_DATA) || defined(CRYS_SEP_SIDE_WORK_MODE)


#ifndef CRYS_NO_FIPS_SUPPORT
/* global variable for storing the FIPS mode - whether it is set and whether error occured */
DxUint8_t DX_GLOBAL_FIPS_MODE=0;
/* global variable for storing whether the self test of an engines ahs already been executed */
DxUint16_t	DX_GLOBAL_FIPS_SF_STATUS = 0;
#endif

  #ifndef CRYS_RND_AES_OLD_128BIT_ONLY
	/*************** For new RND_AES   *************************/
    /* global buffer for RNG working State */
	CRYS_RND_State_t  CRYS_RND_WorkingState = {0x0};
  #else
	/************ For old RND_AES 128 bit  *********************/
	/* 128 bit buffer to stores K for PRNG generation */
	DxUint32_t K_Buffer[BUFF2_SIZE_IN_WORDS]={0};

	/* 128 bit buffer to stores V for PRNG generation */
	DxUint32_t V_Buffer[BUFF2_SIZE_IN_WORDS]={0};

	/* 256 bits buffer to store an additional input from the user */
	DxUint32_t AdditionalInput[BUFF2_SIZE_IN_WORDS*2]={0};

	/* Global Flag to indicate the state and size of the Additiona Input buffer */
	DxUint16_t  AI_FLAG=0;
  #endif

	static DxUint32_t buff1[BUFF1_SIZE_IN_WORDS];

#endif /* !defined(CRYS_NO_GLOBAL_DATA) || defined(CRYS_SEP_SIDE_WORK_MODE) */



/******************** Private function prototype ************************/

/************************ Public Functions ******************************/

#if !defined(CRYS_NO_GLOBAL_DATA) || defined(CRYS_SEP_SIDE_WORK_MODE) || defined(CRYS_RND_AES_OLD_128BIT_ONLY)

/**
 * @brief CRYS_COMMON_GetGlobalData get the global random key hidden inside the function
 *	the global data implemented for now are random key buffer and AES secret key buffer
 *
 * When no_rtos is declared then we allow a global data. The random key/AES secret key are hidden as static inside the function
 *
 *
 * @param[in] Globalid	   select the buffer
 * @param[in] GlobalDataSizeWords	   - the global data buffer size needed in words - this value must be a predetermined value
 * @param[out] GlobalData_ptr - Pointer to the global buffer returned. The buffer must be at least GlobalDataSizeWords size
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure an Error as defined in VOS_error
 */
CRYSError_t CRYS_COMMON_GetGlobalData(DxUint16_t Globalid, DxUint32_t *GlobalData_ptr, DxUint16_t GlobalDataSizeWords)
{
    /* LOCAL DECLEARUINS */

    /* FUNCTION LOGIC */

	/* buffers handeled by VOS */
	switch(Globalid)
	{
		#ifdef DX_USE_RND_HASH
		case 0:
			if(GlobalDataSizeWords > BUFF0_SIZE_IN_WORDS)
				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;
			DX_VOS_FastMemCpy((DxUint8_t *)GlobalData_ptr, (DxUint8_t *)buff0, GlobalDataSizeWords*sizeof(DxUint32_t));
			break;
		#endif
		case 1:
			if(GlobalDataSizeWords > BUFF1_SIZE_IN_WORDS)
				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;
			DX_VOS_FastMemCpy((DxUint8_t *)GlobalData_ptr, (DxUint8_t *)buff1, GlobalDataSizeWords*sizeof(DxUint32_t));
			break;
		default:
			return CRYS_COMMON_GLOBAL_DATA_INPUT_ID_ERROR;

	}/* end of switch */

	return CRYS_OK;

}/* END OF CRYS_COMMON_GetGlobalData */

/**************************************************************************************************************/
/** @brief CRYS_COMMON_StoreGlobalData store the global random key into the global buffer hidden inside the function
*	the global data implemented for now are random key buffer and AES secret key buffer
*
*
* @param[in] Globalid	   - random key / AES secret key
* @param[in] GlobalDataSizeWords	   - the global data buffer size needed in words - this value must be a predetermined value
* @param[in] GlobalData_ptr - Pointer to the global buffer to be saved. The buffer must be at least GlobalDataSizeWords size
*
*   Return Value:
*
***************************************************************************************/

CRYSError_t CRYS_COMMON_StoreGlobalData(DxUint16_t Globalid, DxUint32_t *GlobalData_ptr, DxUint16_t GlobalDataSizeWords)
{
    /* LOCAL DECLEARUINS */

    /* FUNCTION LOGIC */

	switch(Globalid)
	{
		#ifdef DX_USE_RND_HASH
		case 0:

			if(GlobalDataSizeWords > BUFF0_SIZE_IN_WORDS)
				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;

			DX_VOS_FastMemCpy((DxUint8_t *)buff0, (DxUint8_t *)GlobalData_ptr, GlobalDataSizeWords*sizeof(DxUint32_t));
			break;
		#endif

		case 1:

			if(GlobalDataSizeWords > BUFF1_SIZE_IN_WORDS)
				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;

			DX_VOS_FastMemCpy((DxUint8_t *)buff1, (DxUint8_t *)GlobalData_ptr, GlobalDataSizeWords*sizeof(DxUint32_t));
			break;

		default:
			return CRYS_COMMON_GLOBAL_DATA_INPUT_ID_ERROR;

	}/* end of switch */

	return CRYS_OK;

}/* END OF CRYS_COMMON_StoreGlobalData */

#else /* !defined(CRYS_NO_GLOBAL_DATA) || defined(CRYS_SEP_SIDE_WORK_MODE) */

#include "DX_VOS_File.h"

 /************************ Public Functions ******************************/

/**
 * @brief VOS_GetGlobalData get the global random key / AES key hidden inside the function (in a file)
 *	the global data implemented for now are random key buffer and AES secret key buffer
 *
 * When no_rtos is declared then we allow a global data. The random key/AES secret key are hidden as static inside the function
 *
 *
 * @param[in] Globalid	   - random key / AES secret key
 * @param[in] GlobalDataSizeWords	   - the global data buffer size needed in words - this value must be a predetermined value
 * @param[out] GlobalData_ptr - Pointer to the global buffer returned. The buffer must be at least GlobalDataSizeWords size
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure an Error as defined in VOS_error
 */
CRYSError_t CRYS_COMMON_GetGlobalData(DxUint16_t Globalid, DxUint32_t *GlobalData_ptr, DxUint16_t GlobalDataSizeWords)
{
	CRYSError_t Error = CRYS_OK, Error2 = CRYS_OK;
	DxVosFile *GlobalFile_ptr = DX_NULL;

	switch(Globalid)
	{
		#ifdef DX_USE_RND_HASH
		case 0:

			if(GlobalDataSizeWords > BUFF0_SIZE_IN_WORDS)
				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;

			/*TBD - change the absulute path*/
			Error = DX_VOS_FOpen(GlobalFile_ptr, "c:\\randkeyFile", "r+b" ); /*'r' opens the existing file for read/write*/

			if( (GlobalFile_ptr == DX_NULL) || (Error != DX_SUCCESS) )

				return Error;

			Error = DX_VOS_FRead(GlobalFile_ptr, GlobalData_ptr, GlobalDataSizeWords*sizeof(DxUint32_t));

			if(Error != DX_SUCCESS)
				goto END_WITH_ERROR;

			break;
			#endif
		case 1:

			if(GlobalDataSizeWords > BUFF1_SIZE_IN_WORDS)

				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;

			/*TBD - change the absulute path*/
			Error = DX_VOS_FOpen(GlobalFile_ptr, "c:\\aesSecretkeyFile", "r+b" ); /*'r' opens the existing file for read/write*/

			if( (GlobalFile_ptr == DX_NULL) || (Error != DX_SUCCESS) )

				return Error;

			Error = DX_VOS_FRead(GlobalFile_ptr, GlobalData_ptr, GlobalDataSizeWords*sizeof(DxUint32_t));

			if(Error != DX_SUCCESS)
				goto END_WITH_ERROR;

			break;

		default:

			Error = CRYS_COMMON_GLOBAL_DATA_INPUT_ID_ERROR;
			goto END_WITH_ERROR;
	}
    END_WITH_ERROR:

	Error2 = DX_VOS_FClose(GlobalFile_ptr);
    if(Error2 != DX_SUCCESS)
        return Error2;

	return Error;

}/* END OF VOS_GetGlobalData */

/****************************************************************************************************************/
/** @brief VOS_StoreGlobalData store the global random key into the global buffer hidden inside the function
*	the global data implemented for now are random key buffer and AES secret key buffer
*
*
* @param[in] Globalid	   - random key / AES secret key
* @param[in] GlobalDataSizeWords	   - the global data buffer size needed in words - this value must be a predetermined value
* @param[in] GlobalData_ptr - Pointer to the global buffer to be saved. The buffer must be at least GlobalDataSizeWords size
*
*   Return Value:
*
***************************************************************************************/

CRYSError_t CRYS_COMMON_StoreGlobalData(DxUint16_t Globalid, DxUint32_t *GlobalData_ptr, DxUint16_t GlobalDataSizeWords)
{
	CRYSError_t Error = CRYS_OK, Error2 = CRYS_OK;
	DxVosFile *GlobalFile_ptr = DX_NULL;
	DxUint8_t ind;

	switch(Globalid)
	{
		#ifdef DX_USE_RND_HASH
		case 0:
			if(GlobalDataSizeWords > BUFF0_SIZE_IN_WORDS)
				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;

			/*TBD - change the absulute path*/
			Error = DX_VOS_FOpen(GlobalFile_ptr, "c:\\randkeyFile", "w+b" ); /*'w' = destroys the former file content*/

			if( (GlobalFile_ptr == DX_NULL) || (Error != DX_SUCCESS) )

				return Error;

			/*if all the written data to the file is zero than the file will be removed*/
			/*only in random key option - make the random number not zero*/
			for(ind=0;ind<GlobalDataSizeWords;ind++)
			{
				if(GlobalData_ptr[ind] != 0)
					break;
			}
			if(ind == GlobalDataSizeWords)
				GlobalData_ptr[ind-1] = 1;		/*Make it non-zero*/

			Error = DX_VOS_FWrite(GlobalFile_ptr, GlobalData_ptr, GlobalDataSizeWords*sizeof(DxUint32_t));

			if(Error != DX_SUCCESS)
				goto END_WITH_ERROR;

			break;
			#endif

		case 1:

			if(GlobalDataSizeWords > BUFF1_SIZE_IN_WORDS)

				return CRYS_COMMON_GLOBAL_DATA_INPUT_SIZE_ERROR;

			/*TBD - change the absulute path*/
            /*Only on AES secret key - if the file already exist - don't */
			Error = DX_VOS_FOpen(GlobalFile_ptr, "c:\\aesSecretkeyFile", "w+b"); /*'w' = destroys the former file content*/

			if( (GlobalFile_ptr == DX_NULL) || (Error != DX_SUCCESS) )

				return Error;

			/*if all the written data to the file is zero than the file will be removed*/
			/*Return Error*/
			for(ind=0;ind<GlobalDataSizeWords;ind++)
			{
				if(GlobalData_ptr[ind] != 0)
					break;
			}
			if(ind == GlobalDataSizeWords)
				return CRYS_COMMON_GLOBAL_DATA_ZERO_ERROR;

			Error = DX_VOS_FWrite(GlobalFile_ptr, GlobalData_ptr, GlobalDataSizeWords*sizeof(DxUint32_t));

			if(Error != DX_SUCCESS)
				goto END_WITH_ERROR;

			break;

		default:
			Error = CRYS_COMMON_GLOBAL_DATA_INPUT_ID_ERROR;
			goto END_WITH_ERROR;
	}

    END_WITH_ERROR:

	Error2 = DX_VOS_FClose(GlobalFile_ptr);
    if(Error2 != DX_SUCCESS)
        return Error2;

	return Error;

}/* END OF VOS_StoreGlobalData */

#endif /* !defined(CRYS_NO_GLOBAL_DATA) || defined(CRYS_SEP_SIDE_WORK_MODE) || defined CRYS_RND_AES_OLD_128BIT_ONLY */
