/*! \file 
**********************************************************************************	
* Title:						Discretix SST utility API header file						 					
*																			
* Filename:					    sst_api_utility.h															
*																			
* Project, Target, subsystem:	SST 6.0, Utility, API
* 
* Created:						31.08.2007														
*
* Modified:						31.08.2007										
*
* \Author						Miri S														
*																			
* \Remarks						
*           Copyright (C) 2008 by Discretix Technologies Ltd. All Rights reserved.
**********************************************************************************/


#ifndef _DX_SST_API_UTILITY_H_
#define _DX_SST_API_UTILITY_H_
#ifdef __cplusplus
extern "C" {
#endif

    /*----------- Global Includes ------------------------------------------------*/
#include "sst_types.h" 
#include "NVS.h"

#include "sst_vcrys.h"
#include "sst_aa.h"
#include "sst_vdb.h"
#include "sst_ix.h"
    /*----------- Global defines -------------------------------------------------*/  
    /*----------- Global function prototypes -------------------------------------*/
    
    /*SST_VCRYSReturnCodeMap*/
    /*!
    \brief * This function maps the VCRYS return code into 
    a corresponding API return code.

    @param aVcrysRc        [in]     error code comes from VCRYS.         
    **/        
    DxError_t SST_VCRYSReturnCodeMap (SSTVCRYSReturnCodes_t aVcrysRc);


    /*SST_AAReturnCodeMap*/
    /*!
    \brief This function maps the AA return code into 
    a corresponding SST return code.

    @param aAaRc        [in]     error code comes from AA.     
    **/    
    DxError_t SST_AAReturnCodeMap    (SSTAAReturnCodes_t	aAaRc);


    /*SST_VDBReturnCodeMap*/
    /*!
    \brief This function maps the VDB return code into 
    a corresponding SST return code.

    @param aVdbRc        [in]     error code comes from VDB.     
    **/    
    DxError_t SST_VDBReturnCodeMap   (SSTVDBReturnCodes_t	aVdbRc);


    /*SST_IXReturnCodeMap*/
    /*!
    \brief This function maps the IX return code into 
    a corresponding SST return code.

    @param aIxRc        [in]     error code comes from IX.     
    **/
    DxError_t SST_IXReturnCodeMap	 (SSTIXReturnCodes_t 	aIxRc);


    /*SST_WorkspaceCheck*/
    /*!
    \brief This function check if the workspace is legal
    regarding to size, alignment, multiple of word

    @param aWorkspace_ptr               [in]     
    A pointer to the workspace buffer to be used by the SST.

    @param aWorkspaceSizeInBytes        [in]     
    The size in bytes of the workspace buffer to be used by the SST.

    @param aInternalWorkspace_ptr       [in/out]     
    A pointer to the real aligned workspace buffer to be used by the SST.

    @param aInternalWorkspaceSize_ptr   [in/out]     
    The size in bytes of the real aligned workspace buffer to be used by the SST.

    @param aRequiredWorkspaceSize       [in]     
    Required workspace size in bytes
    **/
    DxError_t SST_WorkspaceCheck(  DxByte_t     *aWorkspace_ptr,
                                   DxUint32_t	aWorkspaceSizeInBytes,
                                   DxByte_t	    **aInternalWorkspace_ptr,
                                   DxUint32_t	*aInternalWorkspaceSize_ptr,
                                   DxUint32_t	aRequiredWorkspaceSize);

#ifdef __cplusplus
}
#endif       
#endif  /* _DX_SST_API_UTILITY_H_ */
