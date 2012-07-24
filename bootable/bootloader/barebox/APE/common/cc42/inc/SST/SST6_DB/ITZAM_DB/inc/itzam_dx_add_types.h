/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Additions Header file 				 					
*																			
* Filename:						itzam_dx_add_types.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						09.05.2006														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_DX_ADD_TYPES_H_
    #define _DX_SST_ITZAM_DX_ADD_TYPES_H_

	#include "DX_VOS_BaseTypes.h"
	#include "itzam.h"
	#include "sst_vcrys.h"
	#include "itzam_dx_add_def.h"
	#include "itzam_dx_add_config.h"

    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (itzam_dx_add_types.h) 
    #else
        #if defined _DX_SST_ITZAM_NO_EXTERN_
            #define EXTERN 
        #else
            #define EXTERN extern
        #endif /* _DX_SST_ITZAM_NO_EXTERN_*/
    #endif /*EXTERN*/

/*----------- Local type definitions -----------------------------------------*/

	/*defines memory for one page*/
	typedef DxByte_t itzam_page_place_holder[SST_ITZAM_PAGE_DATA_BUFFER_SIZE_IN_BYTES];

	/*defines header for one page including the pointers to the page
	Its data and a BOOLean indicating if this page is in use*/
	typedef struct
	{
		itzam_btree_page	pointersBuffer;
		DxBool_t			isInUse;
	}itzam_page_list_header;

	/*defines run time variables for ITZAM operation*/
	typedef struct
	{
		/*page place holders*/
		DxUint32_t				nPageInUse;
		itzam_page_list_header	pageListHeader[SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME];
		DxByte_t				*pageList_ptr;
		
		/*page MAC descriptor - describes the data in a B-tree page to calculate MAC on*/
		SSTVCRYSMACDescriptorsArr_t pageMACDescriptor;

		/*B-tree MAC descriptor - describes the data in a B-tree struct to calculate MAC on*/
		SSTVCRYSMACDescriptorsArr_t btreeMACDescriptor;
		
		/*data file variables for B-tree and transaction including the NVS Memory ID*/
		itzam_datafile	btreeDataFile;
		itzam_datafile	btreeTransactionFile;

	}itzam_permanent_runtime_var;

	/*defines non permanent run time variables for ITZAM operation*/
	typedef struct
	{
		/*data file delete-list workspace*/
		itzam_dellist_entry	datafileDelList[SST_ITZAM_MAX_SIZE_OF_DEL_LIST];
		
		/*place holder for the DB pages*/
		#ifndef SST_ON_SEP
			itzam_page_place_holder pageList[SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME];
		#endif
	}itzam_nonpermanent_runtime_var;
		

	/*MAC stack*/
	typedef struct  
	{
		SSTVCRYSMACfield_t	stack[SST_ITZAM_MAC_STACK_SIZE];
		DxUint32_t			nMac;
	}itzam_mac_stack;

/*----------- Package global variables ---------------------------------------*/

	/*----------------------------------------------------------------------------*/

	#undef EXTERN

#endif  /* _DX_SST_ITZAM_DX_ADD_TYPES_H_ */
