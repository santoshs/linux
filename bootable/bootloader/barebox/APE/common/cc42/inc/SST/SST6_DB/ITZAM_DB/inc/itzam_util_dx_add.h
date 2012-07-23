/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Utility Additions Header file				 					
*																			
* Filename:						itzam_util_dx_add.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB Utility
* 
* Created:						21.08.2007														
*
* Modified:						21.08.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_UTILITY_DX_ADD_H_
    #define _DX_SST_ITZAM_UTILITY_DX_ADD_H_

	#include "DX_VOS_BaseTypes.h"
	#include "itzam.h"

/*----------- Global type definitions ----------------------------------------*/

/*----------- Extern definition ----------------------------------------------*/

/*----------- Global variable declarations -----------------------------------*/

/*----------- Global constant definitions ------------------------------------*/

/*----------- Global function prototypes -------------------------------------*/
/************************************************************************/
/*              Utility functions                                       */
/************************************************************************/
/*!
\brief 
Set the DB pages pointer to an outside given address
@param pages_address	[in] Address of the DB pages
**/
itzam_state itzam_util_set_pages_pointer (DxByte_t *pages_address);

/* itzam_util_get_nvs_sizes */
/*!
    \brief 
    Retrieve the maximal size of the NVS and the currently used size
    according to the memory ID.

    @param memoryID	            [in]  identifier to the relevant memory.
    @param maxSizeInBytes_ptr	[out] a pointer to store the max size.
    @param usedSizeInBytes_ptr	[out] a pointer to store the used size.

    @notes: it is allowed to pass only one pointer and set the other to NULL.  
 **/
itzam_state itzam_util_get_nvs_sizes(DxNvsMemoryId_t memoryID, 
                                     DxUint32_t *maxSizeInBytes_ptr,
                                     DxUint32_t *usedSizeInBytes_ptr);

/* itzam_util_set_max_nvs_size */
/*!
    \brief 
    Set the maximal NVS size to be used by the memory identified by the memory ID.

    @param memoryID	            [in]  identifier to the relevant memory.
    @param newMaxSizeInBytes	[in]  the new maximal NVS size.
 **/
itzam_state itzam_util_set_max_nvs_size(DxNvsMemoryId_t memoryID,
                                        DxUint32_t newMaxSizeInBytes);

/* itzam_util_check_sufficient_nvs_space */
/*!
    \brief 
    check for sufficient space in the NVS for certain operation.

    @param recordSizeInbytes     [in]  the size of the record at hand.
    @param dataOpType	         [in]  the type of the data operation.
    @param isSpaceSufficient_ptr [out] a pointer to hold the result.
 **/
itzam_state itzam_util_check_sufficient_nvs_space(DxUint32_t recordSizeInbytes,
                                                  itzam_data_op_type dataOpType,
                                                  BOOL      *isMainSpaceSufficient_ptr,
                                                  BOOL      *isTxnSpaceSufficient_ptr);
#endif  /* _DX_SST_ITZAM_UTILITY_DX_ADD_H_ */
