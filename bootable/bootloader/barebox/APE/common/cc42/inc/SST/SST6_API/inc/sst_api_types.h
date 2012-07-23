/*! \file
**********************************************************************************
* Title:						Discretix SST API Type Definitions Header File
*
* Filename:						sst_api_types.h
*
* Project, Target, subsystem:	SST 6.0, Stubs, API
*
* Created:						11.03.2007
*
* Modified:						07.06.2007
*
* \Author						Raviv Levi
*
* \Remarks
*           Copyright (C) 2006 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/

#ifndef _DX_SST_API_INTERNAL_TYPES_H_
    #define _DX_SST_API_INTERNAL_TYPES_H_

    #include "sst_aa.h"

    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (sst_api_types.h)
    #else
        #if defined _DX_SST_API_NO_EXTERN_
            #define EXTERN
        #else
            #define EXTERN extern
        #endif /* _DX_SST_API_NO_EXTERN_*/
    #endif /*EXTERN*/

    /*----------- Local type definitions -----------------------------------------*/
    /*! \brief SST Meta data definition
              (each module has specific knowledge of its meta data structure) **/
    typedef struct
    {
        SSTAAMetaDataPlaceHolder_t aaMetaData;
        DxBool_t                   isObjectEncrypted;
    }SSTMetaData_t;


    typedef struct
	{
	    DxBool_t                isActive;
        SSTMode_t               sstMode;
		DxUint32_t				sstInitCount;

        #if defined SST_API_DEBUG_MODE_ENABLED
            // Debug related run time variables.
        #endif //SST_API_DEBUG_MODE_ENABLED

	} SSTAPIRuntimeVars_t;

	/*----------- Package global variables --------------------------------------*/
	#if defined SST_API_DEBUG_MODE_ENABLED
		//EXTERN int _gParam;
	#endif


    #undef EXTERN

#endif  /* _DX_SST_API_INTERNAL_TYPES_H_ */
