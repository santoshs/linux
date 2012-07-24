/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
 #ifndef _FVOS_TYPES_H_
#define _FVOS_TYPES_H_


/*
   *  Object %name    : FVOS_API.h
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

typedef unsigned long         FVOS_Error_t; 

typedef struct _FVOS_FLOW_BufferData_t
{
  /* buffer pointer */
  DxUint8_t*   buffer_ptr;
  
  /* buffer size */
  DxUint32_t   bufferSize;
  
}FVOS_FLOW_BufferData_t; 
 
#endif
