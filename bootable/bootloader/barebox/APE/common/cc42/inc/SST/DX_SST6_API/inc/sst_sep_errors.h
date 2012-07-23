/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 /*! \file 
**********************************************************************************	
* Title:						Discretix SST SEP Errors API header file						 					
*																			
* Filename:					    sst_sep_errors.h															
*																			
* Project, Target, subsystem:	SST 6.0, Stubs, API
* 
* Created:						07.11.2007														
*
* Modified:						07.11.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						
**********************************************************************************/


#ifndef _DX_SST_SEP_ERRORS_H_
    #define _DX_SST_SEP_ERRORS_H_
#ifdef __cplusplus
extern "C" {
#endif
    	
	/*----------- SST SEP Error definitions----------------------------------*/
    
	/*! \brief SST SEP API Return Codes base							      **/
	#define SST_RC_ERROR_SEP_BASE	            (0x00001000UL)
    

	/*! \brief Operation failed -   SST on SEP Communication protocol error	  **/
	#define SST_RC_ERROR_SEP					(SST_RC_ERROR_BASE +			\
												 SST_RC_ERROR_SEP_BASE +		\
												 0x0)

	/*! \brief Operation failed -   SST on SEP error authenticator password 
		size not valid  **/
	#define SST_RC_ERROR_SEP_AUTH_PWD_SIZE_INVALID	(SST_RC_ERROR_BASE +		\
													 SST_RC_ERROR_SEP_BASE +	\
													 0x1)
    
    	/*! \brief Operation failed -   SST on SEP error, SEP is disabled        **/ 
	#define SST_RC_ERROR_SEP_DISABLED	(SST_RC_ERROR_BASE +		            \
                                         SST_RC_ERROR_SEP_BASE +            	\
										 0x2)
										 
	/*! \brief Operation failed -   SST on SEP error authenticator application id 
		size not valid  **/
	#define SST_RC_ERROR_SEP_AUTH_APP_ID_SIZE_INVALID	(SST_RC_ERROR_BASE +		\
														 SST_RC_ERROR_SEP_BASE +	\
    													 0x3)

		/*! \brief Operation failed -   SST on SEP error ,SST_INIT failed due to SEP 
		logic  **/
	#define SST_RC_ERROR_SEP_INIT_FAILED	(SST_RC_ERROR_BASE +		\
											 SST_RC_ERROR_SEP_BASE +	\
											 0x4)
    
    	/*! \brief Operation failed -   SST on SEP error ,SST_TransactionEnd failed due to SEP 
		logic  **/
    #define SST_RC_ERROR_SEP_TRANSACTION_END_FAILED	(SST_RC_ERROR_BASE +		\
													 SST_RC_ERROR_SEP_BASE +	\
													 0x5)
	
    	/*! \brief Operation failed -   SST on SEP error ,SST_OpenAuth failed due to SEP 
		logic  **/
    #define SST_RC_ERROR_SEP_OPEN_AUTH_FAILED	(SST_RC_ERROR_BASE +		\
													 SST_RC_ERROR_SEP_BASE +	\
													 0x6)
    

#ifdef __cplusplus
}
#endif       
#endif  /* _DX_SST_SEP_ERRORS_H_ */


