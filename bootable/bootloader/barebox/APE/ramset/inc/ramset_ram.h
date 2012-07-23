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
** Copyright (C) 2012 Renesas Mobile Corp.	                                 **
** All rights reserved.                                                      **
** ************************************************************************* */

/* ****************************** DESCRIPTION ****************************** **
** Public ROM patch programs.                                                **
**                                                                           **
** ************************************************************************* */

#ifndef _RAMSET_RAM_H_
#define _RAMSET_RAM_H_

/* *********************** HEADER (INCLUDE) SECTION ************************ */
#include "string.h"
#include "common.h"
#include "log_output.h"
#include "sbsc.h"
#include "sysc.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
typedef unsigned long        uint32_t;

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */
typedef uint32_t (*cb_register_func)(uint32_t id, void (*func)(va_list ap));
typedef int (*cb_pbrom_uart_printf)(const char *format, ...);



/* ************************** FUNCTION PROTOTYPES ************************** */
uint32_t ramset_main(cb_register_func  ramset_cb_register_func, cb_pbrom_uart_printf ramset_cb_pbrom_uart_printf);
void set_ramset_callback(cb_register_func ramset_cb_register_func );



/* ********************************** END ********************************** */
#endif /* #ifndef _RAMSET_RAM_H_ */
