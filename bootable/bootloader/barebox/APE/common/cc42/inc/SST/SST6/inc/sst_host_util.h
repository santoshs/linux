/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 /*! \file 
**********************************************************************************
* Title:                            Discretix DX API header file
*
* Filename:                         util.h
*
* Project, Target, subsystem:       SST 6.0, UTIL
* 
* Created:                          26.07.2007
*
* Modified:
* \Author                           Ira Boguslavsky
*
* \Remarks                        
**********************************************************************************/
#ifndef _SST_HOST_UTIL_H_
#define _SST_HOST_UTIL_H_

#define DX_1_WORD_PARAM                         (1UL)
#define DX_2_WORDS_PARAMS                       (2UL)
#define DX_3_WORDS_PARAMS                       (3UL)
#define DX_4_WORDS_PARAMS                       (4UL)
#define DX_5_WORDS_PARAMS                       (5UL)
#define DX_6_WORDS_PARAMS                       (6UL)
#define DX_7_WORDS_PARAMS                       (7UL)
#define DX_8_WORDS_PARAMS                       (8UL)
#define DX_9_WORDS_PARAMS                       (9UL)
#define DX_10_WORDS_PARAMS                      (10UL)
#define DX_11_WORDS_PARAMS                      (11UL)
#define DX_12_WORDS_PARAMS                      (12UL)
#define DX_13_WORDS_PARAMS                      (13UL)
#define DX_14_WORDS_PARAMS                      (14UL)
#define DX_15_WORDS_PARAMS                      (15UL)

#define DX_OP_CODE_PARAM_SIZE_IN_BYTES          (1UL)

typedef enum
{
    DX_BIG_ENDIAN      = 0,
    DX_LITTLE_ENDIAN   = 1,

    SSTEndianessLast  = 0x7FFFFFFF,

}SSTEndianess_t;


/* Define endianess flag*/
#define DX_ENDIANESS_FLAG                       DX_BIG_ENDIAN


/************************************************************************/
/*Simulator definitions*/
#ifdef SST_SEP_SIMULATOR_ENV
	//#include "sst_utility.h"
	//DxByte_t g_DBPagesPointer[SST_UTIL_DB_PAGES_MAMEORY_SIZE_IN_BYTES];
#endif
/************************************************************************/



/* NULL validation macros*/
/* Disable PCLINT Warning  506: Constant value Boolean */    
/* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */
#define DX_IS_NULL_PARAM(param1)  /*lint -save -e506 -e774*/  \
    DX_IS_NULL_4PARAMS((param1),(!DX_NULL),(!DX_NULL),(!DX_NULL))
/*lint -restore */

/* Takes 2 parameters and returns an ERROR if it is null.*/
/* Disable PCLINT Warning  506: Constant value Boolean */    
/* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */
#define DX_IS_NULL_2PARAMS(param1,param2) /*lint -save -e506 -e774*/  \
    DX_IS_NULL_4PARAMS((param1),(param2),(!DX_NULL),(!DX_NULL))
/*lint -restore */

/* Takes 3 parameters and returns an ERROR if it is null.*/
/* Disable PCLINT Warning  506: Constant value Boolean */    
/* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */
#define DX_IS_NULL_3PARAMS(param1,param2,param3) /*lint -save -e506 -e774*/  \
    DX_IS_NULL_4PARAMS((param1),(param2),(param3),(!DX_NULL))    
/*lint -restore */

/* Takes 4 parameters and returns an ERROR if it is null.*/
#define DX_IS_NULL_4PARAMS(param1,param2,param3,param4)           \
    ((DX_NULL == ((void*)(param1)))||                          \
    (DX_NULL == ((void*)(param2)))||                           \
    (DX_NULL == ((void*)(param3)))||                           \
    (DX_NULL == ((void*)(param4))))

#define DX_ALIGN_TO_WORD(arg)   ((((DxUint32_t)(arg)) & (0x3)) ? \
    (((((DxUint32_t)(arg))>>(2))+1)<<(2)) : ((DxUint32_t)(arg)))

#define DX_BYTES_TO_WORD(arg)   /* remove lint warnings of preprocess boolean evaluation */ \
        /*lint -save -e778*/ \
        ((((DxUint32_t)(arg)) & (0x3)) ? \
		((((DxUint32_t)(arg))>>(2))+1) : (((DxUint32_t)(arg))>>(2))) \
		/*lint -restore*/

/*
* 
Description: This function implements a Native protocol sequence. It sends input
parameters to SeP and passes output parameters to Host.
Note:        It sends only one buffer.
*/

/************************ Public Functions **********************/

/**
* @brief SST_SendOneMsgGetResponse implements a Native protocol sequence. It sends input
*        parameters to SeP and passes output parameters to Host.
* Note:  It sends only one buffer.
*
*
* @param[in]     aInputBufferParam_ptr              - Pointer to the input buffer which will be sent to SeP
* @param[in]     aInputBufferParamLengthInBytes     - Actual & maximal input buffer length 
* @param[in/out] aOutputBuffer_ptr                  - Pointer to the output buffer, which will be filled by SeP
* @param[out]    aOutputBufferParamLengthInBytes    - Actual & maximal output buffer length 
*
* Note:  aInputBufferParamLengthInBytes also the maximum input bufer length
*        aOutputBufferParamLengthInBytes also the maximum output bufer length
*
* @return DxError_t - On success the value DX_SUCCESS is returned, 
*                     On failure the following values:
*
*                     NVS_MEMORY_ID_NOT_EXIST_ERROR - the memory device does not exist.
*                     NVS_MEMORY_ACCESS_FAILURE - failing to access the memory.
*/
DxError_t SST_SendOneMsgGetResponse( DxUint32_t *aInputBufferParam_ptr,
									DxUint32_t  aInputBufferParamLengthInBytes,
									DxUint32_t *aOutputBuffer_ptr,
									DxUint32_t  aOutputBufferParamLengthInBytes);


#endif/*_SST_HOST_UTIL_H_*/
