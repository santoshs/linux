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
* Title:						Discretix SST Version header file						 					
*																			
* Filename:						sst_version.h 															
*																			
* Project, Target, subsystem:	SST 6.0, Version
* 
* Created:						27.03.2007														
*
* Modified:						07.06.2007										
*
* \Author						Raviv levi														
*																			
* \Remarks						
**********************************************************************************/

#ifndef _SST_VERSION_H
    #define _SST_VERSION_H
#ifdef __cplusplus
extern "C" {
#endif  
    #include "DX_VOS_BaseTypes.h"
	#include "sst_types.h"
    /************************ Defines ******************************/

    /* the SST release version definitions */

    #define SST_RELEASE_TYPE         'A'
    #define SST_MAJOR_VERSION_NUM     6
    #define SST_MINOR_VERSION_NUM     2
    #define SST_SUB_VERSION_NUM       5
    #define SST_INTERNAL_VERSION_NUM  0

    /************************ Enums ********************************/

    /************************ Typedefs  ****************************/

    


    /************************ Public Functions **********************/


        /*SST_VersionGet*/
        /*!
        \brief    This function returns the SST version.
        
         @param[in] version_ptr - a pointer to the version structure.
            
            The release version number is handled by the following:

            D/B/R X1.X2.X3.X4

            The first letter defines the maturity of the release : 

            D – development , an internal release that passed maximim the unit tests.

            B – beta release , a release that usually passes the ATP by the QA.

            R – a formal release , passed all of the QA tests.



            X1 – primary version – on our case it will be set as 6 ( when this version is updated all 
                                                                     of the lower ones X2,X3,X4 are reset to 0 ).

            X2 – minor version – this number should be updated if a major feature is added or the API 
                                 interface has changed. ( when this version is updated X3,X4 are reset ).

            X3 – sub versoion – on each new release passed to the QA this number should be incremented ( X4 is reset )

            X4 – internal RD version , on each commit this number should be incremented , when the release is
                 passed to the QA its reset and X3 is incremented. On each release send to customer this value should be 0.



            Note that releases passed to the customer have only 3 digits on the release R X1.X2.X3

        **/

    DxError_t SST_VersionGet(SSTVersion_t *version_ptr);
#ifdef __cplusplus
}
#endif
#endif /* _SST_VERSION_H */
