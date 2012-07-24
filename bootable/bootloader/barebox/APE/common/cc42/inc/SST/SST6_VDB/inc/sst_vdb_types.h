/*! \file 
**********************************************************************************	
* Title:						Discretix SST VDB Type Definitions Header File	 					
*																			
* Filename:						sst_vdb_types.h 															
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

#ifndef _DX_SST_VDB_TYPES_H_
    #define _DX_SST_VDB_TYPES_H_

	#include "CRYS.h"
	#include "DX_VOS_Sem.h"
	#include "itzam.h"
	#include "sst_vdb_def.h"


    #if defined EXTERN
        #error EXTERN ALREADY DEFINED !!! (vdb_types.h) 
    #else
        #if defined _DX_SST_VDB_NO_EXTERN_
            #define EXTERN 
        #else
            #define EXTERN extern
        #endif /* _DX_SST_VDB_NO_EXTERN_*/
    #endif /*EXTERN*/

    /*----------- Local type definitions -----------------------------------------*/
		
	/*Transaction variables*/
	typedef struct  
	{
		DxVosSem				sem;
		DxBool_t				inTransaction;
		SSTTxnId_t				id;
		DxUint32_t				pid;
	}SSTVDBTransactionVar_t;

	/*DB variables*/
	typedef struct  
	{
		DxBool_t				isOpen;
		DxBool_t				PassPhase1;
		itzam_btree				databaseHandle;
	}SSTVDBDBVar_t;

	/*CRYS variables*/
	typedef struct  
	{
		DxByte_t				skData[SST_VDB_SK_BUFFER_MAX_SIZE_IN_BYTES];
		DxUint32_t				skSize;
	}SSTVDBVcrysVar_t;

	/*Permanent runtime variables*/
	typedef struct  
	{	
		DxBool_t				isIntegrityCompromised;
		SSTVDBTransactionVar_t	transaction;
		SSTVDBDBVar_t			database;
        DxBool_t                useNvsCache;        
	}SSTVDBPermanentRuntimeVar_t;
	
	
	/*Non permanent runtime variables*/
	typedef struct  
	{	
		SSTVDBVcrysVar_t			vcrys;
	}SSTVDBNonPermanentRuntimeVar_t;
	/*----------- Package global variables --------------------------------------*/

#undef EXTERN

#endif  /* _DX_SST_VDB_TYPES_H_ */
