/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 #ifndef __FVOS_ERROR_H__
#define __FVOS_ERROR_H__


/*
   *  Object %name    : FVOS_Error.h
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief FVOS h file (API and structures )
   *
   *  \version 
   *  \author yevgenys
   */


/*------------------------------
    DEFINES
--------------------------------*/


#define FVOS_OK                               0

/* timeout error code */
#define FVOS_TIMEOUT_ERR                      1

/* access denied error */
#define FVOS_ACCESS_DENIED_ERR                2

/* access to message shared area denied */
#define FVOS_MESSAGE_AREA_ACCESS_ERR          3

/* allocation memory failure */
#define FVOS_ALLOCATE_FAIL_ERR                4

/* not enough memory for operation */
#define FVOS_MEMORY_NOT_AVAILABLE_ERR         5

/* illegal flow id */
#define FVOS_ILLEGAL_FLOW_ID_ERR              6

/* SEP_GPR3 == 1 */
#define FVOS_SEP_IS_DISABLED_ERR 		          7

/* the shareed area allocated is not sufficient in size */
#define FVOS_SHARED_AREA_UNSUFFICENT_SIZE_ERR 8

/* failure of the sys call (i.e. poll ,..) */
#define FVOS_SYS_CALL_FAIL_ERR                9

/* the value if the GPR2 register is not sync between SEP and HOST */
#define FVOS_SEP_HOST_NOT_SYNCHRONIC_REG_VAL  10

/* error occurred during creation of the symetric tables */
#define FVOS_BUILD_SYM_TABLE_ERR              11

/* error occurred during creation of the symetric tables */
#define FVOS_CC_INIT_SECOND_CALL_ERR          12

/* error occurred during creation of the symetric tables */
#define FVOS_CC_POLLING_STATUS_ERR    		  13

#endif
