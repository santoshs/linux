#ifndef __RAMSET_ROM_H__
#define __RAMSET_ROM_H__
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
** Public ROM patch programs.                                                **
**                                                                           **
** ************************************************************************* */

/* *********************** HEADER (INCLUDE) SECTION ************************ */
#include "types.h"
#include "common.h"
#include "libc.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
/*!
 * Callback function indexes
 */
#define RAMSET_EMMC_BEFORE_LOADER_ID   1
#define RAMSET_USB_BEFORE_LOADER_ID    2
#define RAMSET_SOFT_RESET_ID           3
#define RAMSET_LOAD_IMAGE_ID           4
#define RAMSET_EMMC_AFTER_LOADER_ID    5
#define RAMSET_USB_AFTER_LOADER_ID     6
#define RAMSET_MAX_CALLBACK_ID         7

#define RAMSET_OK                      0                /* RAMSet successful */
#define RAMSET_NG                      0xFFFFFFFF       /* RAMSet error */

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */
typedef struct
{
    void (*func)(va_list ap);
} RAMSET_CALLBACK_ENTRY_STR;

// static RAMSET_CALLBACK_ENTRY_STR ramset_callback_table[RAMSET_MAX_CALLBACK_ID];

/* ************************** FUNCTION PROTOTYPES ************************** */
uint32_t ramset_callback_register(uint32_t patch_id, void (*func)(va_list ap));
void ramset_callback_execute(uint32_t patch_id, ...);

/* ********************************** END ********************************** */
#endif /* #ifndef __RAMSET_ROM_H__ */
