/*! \file 
**********************************************************************************	
* Title:						Discretix SST General Definitions Header File	 					
*																			
* Filename:						sst_general_def.h 															
*																			
* Project, Target, subsystem:	SST 6.0 General
* 
* Created:						29.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv Levi														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_GENERAL_DEF_H_
    #define _DX_SST_GENERAL_DEF_H_

	#include "sst_general_config.h"
	#include "sst_general_def_handle.h"
    #include "DX_VOS_Stdio.h"
	
    /*----------- Local type definitions -----------------------------------------*/
	
    /* Assert Macros.*/

#ifdef SST_DEBUG_MODE_WITH_ENDLESS_LOOP_ENABLED
	#define SST_ASSERT(arg)                                                                  \
	{if (DX_NULL == (arg))                                                                   \
	    {SST_DEBUG_PRINT("\n\nERROR - SST_ASSERT, File: %s, Line: %d \n\n",__FILE__,__LINE__); \
         while(DX_NULL == (arg));}}
#else
    #define SST_ASSERT(arg)
#endif
    
	#ifdef SST_ASSERT_ON_WORKSPACE_ERROR /* Configurable throught the sst_general_config.h file.*/
	    #define SST_DBG_WORKSPACE_ASSERT(arg)    SST_ASSERT(arg)
	#else /*SST_ASSERT_ON_NULL_POINTERS*/
	    #define SST_DBG_WORKSPACE_ASSERT(arg)
    #endif /*SST_ASSERT_ON_NULL_POINTERS*/
	
	
	/* General Macros*/
	/*return Boolean if size of workspace received is valid*/
	#define SST_WORKSPACE_SIZE_IN_BYTES_VALID(requiredSize,receivedSize)	\
			((requiredSize) <= (receivedSize)) 

    #define SST_BYTES_IN_WORD                       (sizeof(DxUint_t))
    #define SST_WORDS_TO_BYTES(sizeInWords)         (SST_BYTES_IN_WORD*(sizeInWords))
   	#define SST_BITS_IN_BYTE					    (DX_BITS_IN_BYTE)

	/*align argument to word (4 Bytes)*/
	/* Disable PCLINT Info Msg 778: Constant expression evaluates to 0 in operation '&' */ 
	#define SST_ALIGN_TO_WORD(arg)											\
			/*lint -save -e778*/											\
			((((DxUint32_t)(arg)) & (0x3)) ?								\
			(((((DxUint32_t)(arg))>>(2))+1)<<(2)) : ((DxUint32_t)(arg)))	\
			/*lint -restore */
    
    /* NULL validation macros*/
    /* Disable PCLINT Warning  506: Constant value Boolean */    
    /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */    
    #define SST_IS_NULL_PARAM(param1)  /*lint -save -e506 -e774*/           \
            SST_IS_NULL_4PARAMS((param1),(!DX_NULL),(!DX_NULL),(!DX_NULL))  \
            /*lint -restore */

    /* Takes 2 parameters and returns an ERROR if it is null.*/
    /* Disable PCLINT Warning  506: Constant value Boolean */    
    /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */    
    #define SST_IS_NULL_2PARAMS(param1,param2) /*lint -save -e506 -e774*/   \
            SST_IS_NULL_4PARAMS((param1),(param2),(!DX_NULL),(!DX_NULL))    \
            /*lint -restore */

    /* Takes 3 parameters and returns an ERROR if it is null.*/
    /* Disable PCLINT Warning  506: Constant value Boolean */    
    /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */    
    #define SST_IS_NULL_3PARAMS(param1,param2,param3) /*lint -save -e506 -e774*/  \
            SST_IS_NULL_4PARAMS((param1),(param2),(param3),(!DX_NULL))            \
            /*lint -restore */

    /* Takes 4 parameters and returns an ERROR if it is null.*/
    #define SST_IS_NULL_4PARAMS(param1,param2,param3,param4)           \
           ((DX_NULL == ((void*)(param1)))||                           \
            (DX_NULL == ((void*)(param2)))||                           \
            (DX_NULL == ((void*)(param3)))||                           \
            (DX_NULL == ((void*)(param4))))
            	
	#ifdef SST_DEBUG_MODE_ENABLED

		/*report fatal error and stop exe in debug mode*/
        #define SST_DBG_ASSERT(arg)                                                                         \
            /*lint -save -e722 -e731 */                                                                     \
            /* Disable PCLINT Info Msg 722: Suspicious use of ; */                                          \
            /* Disable PCLINT Info Msg 731: Boolean argument to equal/not equal */                          \
            {SST_ASSERT(arg)}                                                                               \
		    /*lint -restore */                                                                             

		/*check if the workspace size is sufficient*/
		#define SST_DBG_WORKSPACE_SIZE_IN_BYTES_VALID(requiredSize,receivedSize)	\
				((requiredSize) <= (receivedSize)) 

		/* Takes 1 parameter and returns an ERROR if it is null.*/
        #define SST_DBG_IS_NULL_PARAM(param1)                                                               \
            /*lint -save -e506 -e774*/                                                                      \
            /* Disable PCLINT Warning  506: Constant value Boolean */                                       \
            /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
            SST_IS_NULL_4PARAMS((void*)param1,((void*)(!DX_NULL)),((void*)(!DX_NULL)),((void*)(!DX_NULL)))  \
            /*lint -restore */

        /* Takes 2 parameters and returns an ERROR if it is null.*/
        #define SST_DBG_IS_NULL_2PARAMS(param1,param2)                                                      \
            /*lint -save -e506 -e774*/                                                                      \
            /* Disable PCLINT Warning  506: Constant value Boolean */                                       \
            /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
             SST_IS_NULL_4PARAMS((void*)param1,(void*)param2,((void*)(!DX_NULL)),((void*)(!DX_NULL)))       \
            /*lint -restore */

        /* Takes 3 parameters and returns an ERROR if it is null.*/
        #define SST_DBG_IS_NULL_3PARAMS(param1,param2,param3)                                               \
            /*lint -save -e506 -e774*/                                                                      \
            /* Disable PCLINT Warning  506: Constant value Boolean */                                       \
            /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
             SST_IS_NULL_4PARAMS((void*)param1,(void*)param2,(void*)param3,((void*)(!DX_NULL)))             \
            /*lint -restore */
                        
        /* Takes 4 parameters and returns an ERROR if it is null.*/
        #define SST_DBG_IS_NULL_4PARAMS(param1,param2,param3,param4)                                        \
                SST_IS_NULL_4PARAMS(param1,param2,param3,param4)

                        
	#else /*SST_DEBUG_MODE_ENABLED*/

        /*#error  ERROR - SST Debug phase, please use SST_DEBUG_MODE_ENABLED.*/
		/*return the (arg) not in debug mode (in debug: fatal error*/
        #define SST_DBG_ASSERT(arg)
		#define SST_DBG_WORKSPACE_SIZE_IN_BYTES_VALID(requiredSize,receivedSize)	(DX_TRUE)
        #define SST_DBG_IS_NULL_PARAM(param1)										\
                /*lint -save -e506 -e774*/                                          \
                /* Disable PCLINT Warning  506: Constant value Boolean */           \
                /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
                (DX_FALSE)                                                          \
                 /*lint -restore */
        #define SST_DBG_IS_NULL_2PARAMS(param1,param2)										\
                /*lint -save -e506 -e774*/                                          \
                /* Disable PCLINT Warning  506: Constant value Boolean */           \
                /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
                (DX_FALSE)                                                          \
                 /*lint -restore */
        #define SST_DBG_IS_NULL_3PARAMS(param1,param2,param3)										\
                /*lint -save -e506 -e774*/                                          \
                /* Disable PCLINT Warning  506: Constant value Boolean */           \
                /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
                (DX_FALSE)                                                          \
                 /*lint -restore */
        #define SST_DBG_IS_NULL_4PARAMS(param1,param2,param3,param4)										\
                /*lint -save -e506 -e774*/                                          \
                /* Disable PCLINT Warning  506: Constant value Boolean */           \
                /* Disable PCLINT Info Msg 774: Boolean within 'String' always evaluates to [True/False] */     \
                (DX_FALSE)                                                          \
                 /*lint -restore */

	#endif /*SST_DEBUG_MODE_ENABLED*/

#endif  /* _DX_SST_GENERAL_DEF_H_ */
