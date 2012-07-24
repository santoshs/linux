/*
**********************************************************************************	
* Title:						Discretix SST DB Itzam Additions Header file 				 					
*																			
* Filename:						itzam_dx_add_def.h			
*																			
* Project, Target, subsystem:	SST 6.0 Itzam DB
* 
* Created:						13.05.2007														
*
* Modified:						07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_ITZAM_DX_ADD_DEF_H_
    #define _DX_SST_ITZAM_DX_ADD_DEF_H_

	#include "DX_VOS_BaseTypes.h"
	#include "itzam.h"
    #include "itzam_dx_add_config.h"

	/*----------- Global defines -------------------------------------------------*/
	#if ((defined SST_DEBUG_MODE_ENABLED) && (!defined SST_DB_ITZAM_DEBUG_MODE_ENABLED))
		#define SST_DB_ITZAM_DEBUG_MODE_ENABLED
	#endif

	#if ((defined SST_TESTING_MODE_ENABLED) && (!defined SST_DB_ITZAM_TESTING_MODE_ENABLED))
		#define SST_DB_ITZAM_TESTING_MODE_ENABLED
	#endif
	/*----------------------------------------------------------------------------*/

	#define SST_ITZAM_PAGE_DATA_BUFFER_SIZE_IN_BYTES	((sizeof(itzam_btree_page_header)) + \
														((sizeof(itzam_record_info))*(SST_ITZAM_DB_ORDER+1)) + \
														((sizeof(itzam_ref))*(SST_ITZAM_DB_ORDER+1)) + \
														((sizeof(itzam_link_info))*(SST_ITZAM_DB_ORDER+2)))

	#define SST_ITZAM_DATAFILE_HEADER_SIZE_IN_BYTES		(sizeof(itzam_datafile_header))
	#define SST_ITZAM_DATA_FILE_BLOCK_SIZE							(16UL)

	/*----------- Global macro definitions ---------------------------------------*/

	/*basic block size operation: round up \ down and reminder*/
	#define ITZAM_ENCRYPT_BLOCK_ROUNDED_UP_SIZE(size)	\
													(((size) & (SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK)) ? \
													((((size)>>(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT))+1)\
													<<(SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_NUM_SHIFT)) : (size)) 
	#define ITZAM_MAC_BLOCK_ROUNDED_DOWN_SIZE(size)	(((size) & (SST_VCRYS_MAC_BLOCK_MASK)) ? \
													(((size)>>(SST_VCRYS_MAC_BLOCK_NUM_SHIFT))\
													<<(SST_VCRYS_MAC_BLOCK_NUM_SHIFT)) : (size))
	#define ITZAM_ENCRYPT_BLOCK_REMINDER_SIZE(size) ((size) & (SST_VCRYS_ENCRYPT_DECRYPT_BLOCK_MASK))


	/*Error handling*/
    #define ITZAM_ERROR_DBG_EVENT_SEND(functionName,returnCode)	\
            SST_DBG_EVENT_SEND(SST_DBG_EVENT_DB_ITZAM_##functionName##_FAIL,returnCode)

	/*finds the free page index in the page list, 
	if non exist it will return SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME */
	#define SST_ITZAM_RUN_TIME_FREE_PAGE_INDEX_GET(pageIndex) \
			pageIndex = 0; \
			while ((pageIndex<SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME)&&(DX_TRUE == g_SstItzamPermanentVar.pageListHeader[pageIndex].isInUse))\
				{pageIndex++;}

	/*finds the index of a given page in the page list, 
	if non exist it will return SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME*/
	#define SST_ITZAM_RUN_TIME_PAGE_INDEX_GET(pageIndex,page)  \
			pageIndex = 0; \
			while ((page != &(g_SstItzamPermanentVar.pageListHeader[pageIndex].pointersBuffer))&&((pageIndex<SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME)&&(&(g_SstItzamPermanentVar.pageListHeader[pageIndex].pointersBuffer))))\
				{pageIndex++;}

	/*initialize the run time variables*/
	#define SST_ITZAM_RUN_TIME_VAR_INIT(pageIndex)								\
			g_SstItzamPermanentVar.nPageInUse = 0;										\
			for (pageIndex=0; pageIndex<SST_ITZAM_NUM_OF_PAGE_IN_RUN_TIME; pageIndex++) \
				{g_SstItzamPermanentVar.pageListHeader[pageIndex].pointersBuffer.m_data = \
				g_SstItzamPermanentVar.pageList_ptr+(pageIndex*SST_ITZAM_PAGE_DATA_BUFFER_SIZE_IN_BYTES); \
				set_page_pointers((&(g_SstItzamPermanentVar.pageListHeader[pageIndex].pointersBuffer))); \
				g_SstItzamPermanentVar.pageListHeader[pageIndex].isInUse = DX_FALSE;}


	/*initialize the pages pointer*/
	#ifndef SST_ON_SEP
		#define SST_ITZAM_RUN_TIME_VAR_PAGE_POINTER_INIT()									\
				g_SstItzamPermanentVar.pageList_ptr = (DxByte_t*)g_SstItzamNonPermanentVar.pageList;
	#else
		#define SST_ITZAM_RUN_TIME_VAR_PAGE_POINTER_INIT()
	#endif

	/*initialize an entry in the del-list*/
	#define SST_ITZAM_DELLIST_ENTRY_INIT(entryIndex) \
			g_SstItzamNonPermanentVar.datafileDelList[entryIndex].m_where  = ITZAM_NULL_REF; \
			g_SstItzamNonPermanentVar.datafileDelList[entryIndex].m_length = 0

	/*gets the size of the corresponding memory*/
	#define SST_ITZAM_DATA_MEMORY_BY_ID_GET(memoryID,where)								\
		if (memoryID == g_SstItzamPermanentVar.btreeDataFile.m_file)						\
			{where = (itzam_ref)SST_ITZAM_DATA_MAIN_MEMORY_SIZE_GET();}					\
		else if (memoryID == g_SstItzamPermanentVar.btreeTransactionFile.m_file)			\
			{where = (itzam_ref)SST_ITZAM_DATA_SECONDARY_MEMORY_SIZE_GET();}


	/*******************/
	/*Debug definitions*/
	/*******************/

	#ifdef SST_DB_ITZAM_DEBUG_MODE_ENABLED

		#define SST_ITZAM_DBG_ASSERT(returnCode)						\
				SST_DEBUG_PRINT("\n\nERROR - SST_DB_FATAL_ERROR, Return Code: %d",returnCode); \
				SST_ASSERT(0)

		#define SST_ITZAM_DBG_MEM_ROUNDED_UP(size)						\
				SST_ITZAM_MEMORY_BLOCK_SIZE_ROUNDED_UP(size)
                
        #ifdef SST_DB_ITZAM_STATISTICS_ENABLED
            #define SST_ITZAM_DBG_READ_STATISTICS_INCREMENT(tempValue)		\
                    incrementReadDebugData(tempValue);						\
                    incrementReadDebugByOp(tempValue)
            #define SST_ITZAM_DBG_WRITE_STATISTICS_INCREMENT(tempValue)		\
                    incrementWriteDebugData(tempValue);						\
                    incrementWriteDebugByOp(tempValue)
        #else      
            #define SST_ITZAM_DBG_READ_STATISTICS_INCREMENT(tempValue)
            #define SST_ITZAM_DBG_WRITE_STATISTICS_INCREMENT(tempValue)        
        #endif
		/* Check for sufficient workspace */
		#define SST_ITZAM_DBG_WORKSPACE_SIZE_IN_BYTES_CHECK(functionName,requiredSize,receivedSize)	\
				{if (requiredSize > receivedSize)						\
					{ITZAM_ERROR_HANDLE(functionName,ITZAM_DX_ERROR_INSUFFICIENT_SCRATCH_BUFFER)}}
		

	#else
		#define SST_ITZAM_DBG_ASSERT(returnCode)
		#define SST_ITZAM_DBG_MEM_ROUNDED_UP(size)							(size)
		#define SST_ITZAM_DBG_READ_STATISTICS_INCREMENT(tempValue)
		#define SST_ITZAM_DBG_WRITE_STATISTICS_INCREMENT(tempValue)
		#define SST_ITZAM_DBG_WORKSPACE_SIZE_IN_BYTES_CHECK(functionName,requiredSize,receivedSize)
	#endif

	#ifdef SST_RUN_TIME_VAR_SIZE_PRINT
		#define SST_DB_ITZAM_RUN_TIME_VAR_SIZE_PRINT()	\
				SST_DEBUG_PRINT("DB_ITZAM global size in bytes:%d\n", sizeof(g_SstItzamPermanentVar)); \
				SST_DEBUG_PRINT("DB_ITZAM page size in bytes:%d\n", sizeof(itzam_page_place_holder))
	#else
		#define SST_DB_ITZAM_RUN_TIME_VAR_SIZE_PRINT()
	#endif
/*----------------------------------------------------------------------------*/

#endif  /* _DX_SST_ITZAM_DX_ADD_DEF_H_ */
