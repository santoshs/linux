/*! \file 
**********************************************************************************	
* Title:						Discretix SST General Configuration Header File	 					
*																			
* Filename:						sst_general_config.h 															
*																			
* Project, Target, subsystem:	SST 6.0 General
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007											
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_SST_GENERAL_CONFIG_H_
    #define _DX_SST_SST_GENERAL_CONFIG_H_

    /*----------- Local type definitions -----------------------------------------*/
	#define SST_HANDLE_SIZE_IN_BITS						(32UL)
	#define SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS		(23UL)
    #define SST_HANDLE_SIZE_OF_TYPE_FIELD_IN_BITS		(8UL)
    #define SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS	(1UL)
	#define SST_HANDLE_SIZE_OF_DIGEST_FIELD_IN_BITS		(32UL)
	
	
	#if ((SST_HANDLE_SIZE_OF_TYPE_FIELD_IN_BITS +								\
	      SST_HANDLE_SIZE_OF_INT_EXT_FIELD_IN_BITS +							\
		  SST_HANDLE_SIZE_OF_USER_FIELD_IN_BITS) > SST_HANDLE_SIZE_IN_BITS)
		  
	   #error	ERROR - SST Record handle total size does not match fields size
	
	#endif


	/*----------- General flags ---------------------------------------------------*/


#endif  /* _DX_SST_SST_GENERAL_CONFIG_H_ */
