/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
 *   protected by copyright, international treaties and various patents.   *
 *   Any copy or reproduction of this Software as permitted below, must    *
 *   include this Copyright Notice as well as any other notices provided   *
 *   under such license.                                                   *
 *                                                                         *
 *   This program shall be governed by, and may be used and redistributed  *
 *   under the terms and conditions of the GNU Lesser General Public       *
 *   License, version 2.1, as published by the Free Software Foundation.   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY liability and WARRANTY; without even the implied      *
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, please write to the          *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST API Configurate header file						 					
*																			
* Filename:					    sst_config.h															
*																			
* Project, Target, subsystem:	SST 6.0, API, Configurate
* 
* Created:						06.12.2007														
*
* Modified:						06.12.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						
**********************************************************************************/

#ifndef _DX_SST_CONFIG_H_
    #define _DX_SST_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
	/*----------- Global Includes ------------------------------------------------*/

	/*----------- Global defines -------------------------------------------------*/
	/************************************************************************/
	/* General configurable defines                                         */
	/************************************************************************/

	/*! \brief 
		SST Semaphore maximum waiting time in msec**/
	#define SST_SEMAPHORE_WAITING_TIME_IN_MSEC						(3000L)

    /*! \brief SST Application ID Max size in bytes **/
    #define SST_APP_ID_MAX_SIZE_IN_BYTES                            (32UL)
	
	/************************************************************************/
	/* Authentication Layer configurable defines                            */
	/************************************************************************/
	
	/*! \brief 
		The maximal number of authenticators that can be open per session.*/
    #define SST_MAX_AUTHS_PER_SESSION	    (5UL)

	/*! \brief 
		The maximal number of sessions that can be open simultaneously.*/
    #define SST_MAX_OPEN_SESSIONS	        (5UL)

	/*! \brief 
		The maximal number of challenges that can be pending simultaneously.*/
    #define SST_MAX_PENDING_CHALLENGES	    (3UL)
    

    /*! \brief SST challenge ID for opening a PWD auth' **/
    #define SST_PWD_AUTH_CHALLENGE_ID        ((SSTChallengeId_t)0xBADABADA)

    #define SST_AUTH_PWD_MAX_SIZE_IN_BYTES	    (128UL)
	/************************************************************************/
	/* DataBase configurable defines                                        */
	/************************************************************************/

	/*! \brief 
		Number of maximum trials to create a random record handle if is duplicated**/
	#define SST_NUM_OF_MAX_TRIALS_OF_RND_HANDLE_WHEN_DUPLICATE		(10UL)

	/*! \brief 
		SST DB order - max number of keys per node
		This value is determined based on the size of memory page (order 8 -> 512 Bytes) 
		This value has a direct effect on the size of the DB, and search time. **/
	#define SST_DB_ORDER											(8UL)
	
	/*! \brief 
		SST Max number of entries in the DB Deleted items list. 
		The actual size of the list is SST_MAX_NUM_OF_ENTRY_IN_DB_DEL_LIST * 8 Bytes
		Changing this value will result in change of the size of the global parameters 
		of the SST**/
	#define SST_MAX_NUM_OF_ENTRY_IN_DB_DEL_LIST						(64UL)
	
	/*! \brief 
		SST Max number of MACs stored on stack when performing an altering operation in the DB. 
		The actual stored on stack is SST_MAX_NUM_OF_MACS_IN_MAC_STACK * 20 Bytes
		If this size is to small a SST_RC_ERROR_MEM_ALLOC will be returned**/
	#define SST_MAX_NUM_OF_MACS_IN_MAC_STACK						(10UL)

	#ifndef DX_NVS_RAM
		/*! \brief 
			SST Max size of data main file\memory must be smaller than 4G, 
            because of comparison limitation.
            The size must also be a multiple of 16 due to HASH/AES calculations.**/
		#define SST_NVS_MAIN_MAX_SIZE_IN_BYTES					(0x7ffffff0UL)	
		
		/*! \brief 
			SST Max size of data Transaction file\memory must be smaller than 4G, 
			because of comparison limitation.
            The size must also be a multiple of 16 due to HASH/AES calculations.**/
		#define SST_NVS_TXN_MAX_SIZE_IN_BYTES				     (0x7ffffff0UL/*24*1024UL*/) /* change according to Renesas request */
	#else
		/*! \brief 
			SST Max size of data main file\memory to align with RAM definition**/
		#define SST_NVS_MAIN_MAX_SIZE_IN_BYTES					(0x100000UL)	
		
		/*! \brief 
			SST Max size of data Transaction file\memory to align with RAM definition**/
		#define SST_NVS_TXN_MAX_SIZE_IN_BYTES				(0x100000UL)
	#endif
	/************************************************************************/
	/* Indexing defines				                                        */
	/************************************************************************/

	/*! \brief 
		Maximum number of available collisions (strings) in the index object
		Collisions are when different strings are hashed to the same digest
		Since collisions are rare, changing this parameter will have small 
		effect on indexing in the DB**/
	#define SST_MAX_STRINGS_COLLISION_OBJ_COUNT                   (5UL)

	/*! \brief 
		Maximum number of strings per record
		Note the actual number of strings could be larger in case of a hash 
		collision using different strings**/
	#define SST_MAX_STRINGS_PER_RECORD                           (10UL)


	/*! \brief 
         The maximal size of the main NVS overhead that is required by the SST       
         to perform a persistent operation.                                          
         When performing persistent operations (ones that change the DB) the user    
         must take into consideration that the main NVS should have at least         
         this amount of free space in addition to the size of the relevant object.   
         Note: this overhead takes into consideration a maximum quantity of 100K     
               objects in the SST. (please contact Discretix if your requirements differ)**/
    #define SST_NVS_MAIN_OVERHEAD_SIZE_IN_BYTES    (7*1024UL)


	/*! \brief     
     The maximal size of the transaction NVS overhead that is required by the    
     SST to perform a persistent operation.                                      
     When performing persistent operations (ones that change the DB) the user    
     must take into consideration that the transaction NVS should have at least  
     this amount of free space in addition to the size of the relevant object.   
     Note: this overhead takes into consideration a maximum quantity of 100K     
           objects in the SST. (please contact Discretix if your requirements differ)**/
//    #define SST_NVS_TXN_OVERHEAD_SIZE_IN_BYTES    (100*1024UL)
    #define SST_NVS_TXN_OVERHEAD_SIZE_IN_BYTES    (10*1024UL)/* change according to Renesas request */


	/*----------- Debug defines -------------------------------------------------*/
	/**************************************************************************/
	/*  
	* SST_ASSERT_ON_NULL_POINTERS	-	controls asserts if null pointers 
	*									are passed to the SST
	*
	* SST_ASSERT_ON_WORKSPACE_ERROR -	controls asserts if workspace 
	*									parameters are incorrect, for example 
	*									workspace size is insufficient or 
	*									workspace buffer is not word aligned
	*
	* SST_DEBUG_MODE_ENABLED		-	controls the internal SST debug		
	*									definitions, for example asserts in 
	*									not supported functionalities or if 
	*									an unidentified RC was returned
	*
	* SST_TESTING_MODE_ENABLED		-	controls internal SST testing modules
	* 
	* SST_RUN_TIME_VAR_SIZE_PRINT	-	controls printing of size of all global 
	*									variables in the SST, arranged by SST
	*									layers.
	*
	**************************************************************************/

	/*#define SST_DEBUG_MODE_ENABLED*/	

	#ifdef SST_DEBUG_MODE_ENABLED
		#define SST_ASSERT_ON_NULL_POINTERS
		#define SST_ASSERT_ON_WORKSPACE_ERROR
		#define SST_TESTING_MODE_ENABLED
	#endif


#ifdef __cplusplus
}
#endif

#endif  /* _DX_SST_CONFIG_H_ */
