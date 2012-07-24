/*! \file 
**********************************************************************************	
* Title:						Discretix SST VDB Configurations Header File						 	
*																			
* Filename:						sst_vdb_config.h 															
*																			
* Project, Target, subsystem:	SST 6.0 Virtual DataBase
* 
* Created:						05.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/


#ifndef _DX_SST_VDB_CONFIG_H_
    #define _DX_SST_VDB_CONFIG_H_

	#include "sst_config.h"

	/*Type of lock mode in the ITZAM: ITZAM_FILE_LOCK_NONE or ITZAM_FILE_LOCK_OS or ITZAM_FILE_LOCK_FILE*/
	#define SST_VDB_DB_LOCK_MODE							ITZAM_FILE_LOCK_NONE  
	
	/*Name of the Comparator function used by the ITZAM DB*/
	#define SST_VDB_DB_COMPARATOR_FUNCTION					SST_VDBRecordHandleComparator

	/*Name of the Transaction semaphore*/
	#define SST_VDB_TRANSACTION_SEM_NAME					"SSTVDBSemaphoreName"

	/*Semaphore maximum waiting time in msec*/
	#define SST_VDB_TRANSACTION_SEM_WAITING_TIME			SST_SEMAPHORE_WAITING_TIME_IN_MSEC
		
	/*Number of maximum trials to create a random record handle if is duplicated*/
	#define SST_VDB_NUM_OF_TRIALS_RND_DUPLICATE_HANDLE		SST_NUM_OF_MAX_TRIALS_OF_RND_HANDLE_WHEN_DUPLICATE
			
#endif  /* _DX_SST_VDB_CONFIG_H_ */
