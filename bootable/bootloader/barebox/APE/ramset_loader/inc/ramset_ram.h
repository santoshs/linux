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
#include "ramset_rom.h"
#include "rpc.h"
#include "types.h"
#include "emmc_std.h"
#include "emmc_hal.h"
#include "emmc_config.h"
#include "timer_api.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */
typedef uint32_t (*cb_register_func)(uint32_t id, void (*func)(va_list ap));
typedef int (*cb_pbrom_uart_printf)(const char *format, ...);



/* ************************** FUNCTION PROTOTYPES ************************** */
uint32_t ramset_main(cb_register_func  ramset_cb_register_func, cb_pbrom_uart_printf ramset_cb_pbrom_uart_printf);
void set_ramset_callback(cb_register_func ramset_cb_register_func );
void set_func_pointer(void);

/* Patch_point function. */
#if 0
void ramset_eMMC_Before_Loder(va_list ap);
void ramset_Usb_Before_Loder(va_list ap);
void ramset_Soft_Reset(va_list ap);
void ramset_eMMC_After_Loder(va_list ap);
void ramset_Usb_After_Loder(va_list ap);
#endif
void ramset_Load_Image(va_list ap); /*  Argument : st_load_image_param_t* load_image_p  */

/* ********************************** END ********************************** */
#endif /* #ifndef _RAMSET_RAM_H_ */
