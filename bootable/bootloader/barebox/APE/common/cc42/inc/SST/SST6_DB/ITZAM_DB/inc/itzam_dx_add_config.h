/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Additions Header file 				 					
*																			
* Filename:						itzam_dx_add_config.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						05.06.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_DX_ADD_CONFIG_H_
    #define _DX_SST_ITZAM_DX_ADD_CONFIG_H_

	#include "sst_config.h"

	/*----------- Global defines -------------------------------------------------*/
	/*Max number of pages in running time*/
	#define SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME						(3UL)
	
	/*SST DB order - max number of keys per node*/
	#define SST_ITZAM_DB_ORDER										SST_DB_ORDER
	
	/*Max number of entries in the DB Deleted items list*/
	#define SST_ITZAM_MAX_SIZE_OF_DEL_LIST							SST_MAX_NUM_OF_ENTRY_IN_DB_DEL_LIST
	
	/*Max number of MACs stored on stack*/
	#define SST_ITZAM_MAC_STACK_SIZE								SST_MAX_NUM_OF_MACS_IN_MAC_STACK

    /* The maximal overhead in bytes for main and transaction */
    #define ITZAM_NVS_MAIN_OVERHEAD_SIZE_IN_BYTES  SST_NVS_MAIN_OVERHEAD_SIZE_IN_BYTES
    #define ITZAM_NVS_TXN_OVERHEAD_SIZE_IN_BYTES   SST_NVS_TXN_OVERHEAD_SIZE_IN_BYTES

    /* The maximal size in bytes for main and transaction */
    #define ITZAM_NVS_MAIN_MAX_SIZE_IN_BYTES  SST_NVS_MAIN_MAX_SIZE_IN_BYTES
    #define ITZAM_NVS_TXN_MAX_SIZE_IN_BYTES   SST_NVS_TXN_MAX_SIZE_IN_BYTES

     /* The default maximal NVS size in bytes for each memory*/
     #define ITZAM_NVS_MAIN_DEFAULT_MAX_SIZE_IN_BYTES               ITZAM_NVS_MAIN_MAX_SIZE_IN_BYTES
 
     /* The default maximal NVS size in bytes for each memory*/
     #define ITZAM_NVS_TXN_DEFAULT_MAX_SIZE_IN_BYTES                ITZAM_NVS_TXN_MAX_SIZE_IN_BYTES

	/*----------- Global macro definitions ---------------------------------------*/

	
#endif  /* _DX_SST_ITZAM_DX_ADD_CONFIG_H_ */
