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
** Copyright (C) 2011-2012 Renesas Mobile Corp.                              **
** All rights reserved.                                                      **
** ************************************************************************* */

/* ****************************** DESCRIPTION ****************************** **
** Time measurement functions(use CMT1 module)                               **
**                                                                           **
** ************************************************************************* */
#ifndef _TIMER_API_H_
#define _TIMER_API_H_

/* *********************** HEADER (INCLUDE) SECTION ************************ */
#include "types.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
typedef enum{
    CMT1_CHANNEL0 = 0,      /* CH0 */
    CMT1_CHANNEL1 = 1,      /* CH1 */
    CMT1_CHANNEL2 = 2,      /* CH2 */
    CMT1_MAX_CHANNEL
}CMT1_CHANNEL;

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */
void timer_init( void );
void timer_start( CMT1_CHANNEL ch, unsigned short timeout_ms );
void timer_stop( CMT1_CHANNEL ch );
BOOL is_timeout( CMT1_CHANNEL ch );


/* ********************************** END ********************************** */
#endif /* _TIMER_API_H_ */
