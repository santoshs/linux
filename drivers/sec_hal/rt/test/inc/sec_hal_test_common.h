/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                         **
** All rights reserved.                                                      **
** ************************************************************************* */


/* ************************ HEADER (INCLUDE) SECTION *********************** */
#include "sec_hal_rt_trace.h"

#define __ASSERT(exp, action) if(!(exp)){action;}
#define ASSERT(exp) SEC_HAL_TRACE_INT("asserting on line",__LINE__)\
  __ASSERT(exp, return __LINE__)


