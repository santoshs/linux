/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Additions Header file 				 					
*																			
* Filename:						itzam_data_dx_add.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						05.06.2006														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_DATA_DX_ADD_H_
    #define _DX_SST_ITZAM_DATA_DX_ADD_H_

	#include "DX_VOS_BaseTypes.h"
	#include "NVS.h"
	#include "itzam.h"
	#include "itzam_dx_add_types.h"
	#include "sst_vcrys.h"

/*----------- Global type definitions ----------------------------------------*/

/*----------- Extern definition ----------------------------------------------*/

/*----------- Global variable declarations -----------------------------------*/

/*----------- Global constant definitions ------------------------------------*/

/*----------- Global function prototypes -------------------------------------*/
/************************************************************************/
/*					NVS wrapper function								*/                                              
/************************************************************************/

/*
perform read operation to file\flash.
In debug mode this function will also accumulates the number of bytes 
read in total.

@param memoryId		[in] the non volatile memory id - to this memory the NVS driver is accessing.
@param offset		[in] offset, in bytes, from the base address in the flash to be read
@param buffer_ptr	[in] Pointer to the destination buffer
@param buffLen		[in] Number of bytes to read from the memory
@param retState_ptr [in/out] Pointer to state of read from NVS

@return Number of bytes that were read, if not successful returns 0
*/
DxUint32_t itzam_read(DxNvsMemoryId_t	memoryId,  
					  DxUint32_t	    offset,       
					  char*			    buffer_ptr,        
					  DxUint32_t	    buffLen,
                      itzam_state*      retState_ptr);

/*
perform write operation to file\flash.
In debug mode this function will also accumulates the number of bytes 
wrote in total.

@param memoryId		[in] the non volatile memory id - to this memory the NVS driver is accessing.
@param offset		[in] offset, in bytes, from the base address in the flash to be read
@param buffer_ptr	[in] Pointer to the source buffer
@param buffLen		[in] Number of bytes to write to memory
@param retState_ptr [in/out] Pointer to state of write to NVS

@return Number of bytes that were written, if not successful returns 0
* 
*
* Note: The implementation of this function includes argument retState_ptr that can be missing 
*       in some calls of this function. Those calls are not in use now. If those calls will be
*	    in use in the future This issue will be repair. 
*
*/
DxUint32_t itzam_write(DxNvsMemoryId_t	memoryId,  
					   DxUint32_t		offset,       
					   char*			buffer_ptr,        
					   DxUint32_t		buffLen,
                       itzam_state*     retState_ptr);


/*retrieve the size of the used main memory \ the offset to the end of the used memory*/
#define SST_ITZAM_DATA_MAIN_MEMORY_SIZE_GET()	(g_SstItzamPermanentVar.btreeDataFile.m_header.m_used_size)

/*retrieve the size of the used main memory \ the offset to the end of the used memory*/
#define SST_ITZAM_DATA_SECONDARY_MEMORY_SIZE_GET()	(g_SstItzamPermanentVar.btreeTransactionFile.m_header.m_used_size)

/************************************************************************/
/* DELETE LIST FUNCTIONES                                               */
/************************************************************************/


/*add an entry to the del-list */
itzam_state itzam_datafile_dellist_insert_entry(itzam_datafile *datafile, 
												itzam_ref		entryPosition,
												DxUint32_t		size);

/*remove an entry from the dellist*/
itzam_state itzam_datafile_dellist_remove_entry(itzam_datafile *datafile, 
												itzam_ref		entryPosition);

/*read the dellist header*/
itzam_state itzam_datafile_dellist_read_header(	itzam_datafile *datafile);

/*write to the file a section of the dellist to a given offset*/
itzam_state itzam_datafile_dellist_write(		itzam_datafile *datafile,
												DxUint32_t		offset,
												DxUint32_t		size);

/*create the dellist*/
itzam_state itzam_datafile_dellist_create(		itzam_datafile	*datafile, 
											    itzam_ref		entryPosition,
											    DxUint32_t		size);

/*expend the existing dellist*/
itzam_state itzam_datafile_dellist_expend(		itzam_datafile *datafile, 
												itzam_ref		entryPosition,
												DxUint32_t		size);


/*write dellist header to the file*/
itzam_state itzam_datafile_dellist_write_header(itzam_datafile	*datafile);

/*itzam_datafile_dellist_check_entry*/
itzam_state itzam_datafile_dellist_check_entry(itzam_datafile* datafile, itzam_int length,DxBool_t* isAvailable_ptr);

void flush_transaction(	itzam_datafile 	*datafile, 
			BOOL			ClearTransaction );

#endif  /* _DX_SST_ITZAM_DATA_DX_ADD_H_ */
