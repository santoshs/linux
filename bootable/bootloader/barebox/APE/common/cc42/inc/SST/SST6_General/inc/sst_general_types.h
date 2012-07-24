/*! \file 
**********************************************************************************	
* Title:						Discretix SST General Type Definitions Header File	 					
*																			
* Filename:						sst_general_types.h 															
*																			
* Project, Target, subsystem:	SST 6.0 General
* 
* Created:						29.03.2007														
*
* Modified:					    07.06.2007										
*
* \Author						Einat Ron														
*																			
* \Remarks						Copyright (C) 2006 by Discretix Technologies Ltd.     			
*  								All Rights reserved											
**********************************************************************************/

#ifndef _DX_SST_GENERAL_TYPES_H_
    #define _DX_SST_GENERAL_TYPES_H_
        
    #include "DX_VOS_BaseTypes.h" 	
	#include "sst_general_config.h"
	

    /*----------- Local type definitions -----------------------------------------*/

	/*SST handle field related types*/
	/************************************************************************/
	/* SST Handle structure:                                                */
	/*  <Record Type field :8><I/E type :1><User defined part :23>          */
	/*  OR                                                                  */
    /*  <Record    Properties    field  :9><User defined part :23>          */
    /* Notes:                                                               */
	/*      1. The I/E (Internal/External) bit denotes whether the record   */
	/*         is an SST internal object or not                             */
	/*         (affects the type interpretation, 1 - Internal).             */
	/*      2. The user defined part can be randomly generated upon request.*/
	/*      3. Both structure are equivalent! the second one is used to     */
	/*         encapsulate the type and the internal / external fields.     */
	/************************************************************************/

    /*! \brief This field describes the object's type [8bits] **/
    typedef DxUint32_t SSTHandleTypeField_t;    

    /*! \brief This field indicates whether this object is 
               an SST internal object or not [1bit] **/
    typedef DxUint32_t SSTHandleIntExtField_t;    

    /*! \brief This field unifies the <type> and <I/E> fields information [9bits] **/
    typedef DxUint32_t SSTHandlePropertiesField_t;

    /*! \brief  This field is free for the user to fill and can be randomly 
                generated upon request [23bits] **/
    typedef DxUint32_t SSTHandleUserField_t;

	/*----------- Package global variables --------------------------------------*/


#endif  /* _DX_SST_GENERAL_TYPES_H_ */
