/*! \file 
**********************************************************************************	
* Title:						Discretix SST VDB Utility header file						 					
*																			
* Filename:						sst_vdb_util.h															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual DataBase Utility
* 
* Created:						21.08.2007														
*
* Modified:						21.08.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2007 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_VDB_UTILITY_H_
    #define _DX_SST_VDB_UTILITY_H_

	#include "sst_vdb.h"
	/*----------- Global defines -------------------------------------------------*/

    /*----------- Global macro definitions ---------------------------------------*/

    /*----------- Global type definitions ----------------------------------------*/

    /*----------- Extern definition ----------------------------------------------*/

    /*----------- Global variable declarations -----------------------------------*/
	
    /*----------- Global constant definitions ------------------------------------*/
	
    /*----------- Global function prototypes -------------------------------------*/
	/* SST_VDBUtilDBPagesPointerSet */
	/*!
	\brief 
	Set the DB pages pointer to an outside given address
	
	@param aDBPagesAddress [in] Address of the DB pages
	
	@return SST_VDB_RC_OK					On success
	@return	SST_VDB_RC_FAIL					Operation failed 
	@return SST_VDB_RC_ERROR_NULL_POINTER	At least one of the arguments is null
	@return SST_VDB_RC_ERROR_DB				DB operation failed
	**/
	SSTVDBReturnCodes_t SST_VDBUtilDBPagesPointerSet (DxByte_t *aDBPagesAddress);
	
    /* SST_VDBUtilNVSSizesGet */
    /*!
    \brief 
    Retrieve the maximal size of the NVS and the currently used size
    according to the memory ID.

    @param memoryID	            [in]  identifier to the relevant memory.
    @param maxSizeInBytes_ptr	[out] a pointer to store the max size.
    @param usedSizeInBytes_ptr	[out] a pointer to store the used size.

    @notes: it is allowed to pass only one pointer and set the other to NULL.  
    **/
    SSTVDBReturnCodes_t SST_VDBUtilNVSSizesGet(const DxNvsMemoryId_t memoryID, 
                                               DxUint32_t *maxSizeInBytes_ptr,
                                               DxUint32_t *usedSizeInBytes_ptr);

 

    /* SST_VDBUtilNVSMaxSizeSet */
    /*!
    \brief 
    Set the maximal NVS size to be used by the memory identified by the memory ID.

    @param memoryID	            [in]  identifier to the relevant memory.
    @param newMaxSizeInBytes	[in]  the new maximal NVS size
    **/
    SSTVDBReturnCodes_t SST_VDBUtilNVSMaxSizeSet(const DxNvsMemoryId_t memoryID, 
                                                 const DxUint32_t newMaxSizeInBytes);

    
    /* SST_VDBUtilNVSSufficientSpaceCheck */

    /*!
        \brief 
        check for sufficient space in the NVS for certain operation.

        @param recordSizeInbytes     [in]  the size of the record at hand.
        @param dataOpType	         [in]  the type of the data operation.
        @param isSpaceSufficient_ptr [out] a pointer to hold the result.
     **/
    SSTVDBReturnCodes_t SST_VDBUtilNVSSufficientSpaceCheck(DxUint32_t     recordSizeInbytes,
                                                           SSTVDBOpType_t dataOpType,
                                                           DxBool_t      *isMainSpaceSufficient_ptr,
                                                           DxBool_t      *isTxnSpaceSufficient_ptr);
#endif  /* _DX_SST_VDB_UTILITY_H_ */
