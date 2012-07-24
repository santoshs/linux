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
#ifndef SEC_HAL_ALLOC_H_
#define SEC_HAL_ALLOC_H_

/* ************************ HEADER (INCLUDE) SECTION *********************** */
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <inttypes.h>
#endif


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
/* Operation completed successfully */
#define SEC_HAL_ALLOC_OK                        0x00000000

/* Incorrect parameters given to the function */
#define SEC_HAL_ALLOC_PARAM_ERROR               0x00000100

/* Too big allocation block requested */
#define SEC_HAL_ALLOC_TOO_BIG                   0x00000200

/* Initialization not done properly */
#define SEC_HAL_ALLOC_INIT_FAIL                 0x00000400


/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */

/* ************************** FUNCTION PROTOTYPES ************************** */
/** initialize msg area */
int sec_hal_msg_area_init(void *buf_ptr, unsigned int buf_sz);

/** release msg area */
void sec_hal_msg_area_exit(void);

/** copy memory */
unsigned long sec_hal_msg_area_memcpy(void *dst, const void *src, unsigned long sz);

/** allocate a message area for the secure service message */
void* sec_hal_msg_area_calloc(unsigned int n, unsigned int sz);

/** free allocated message area for the secure service message */
void sec_hal_msg_area_free(void *address);

/** returns the number of allocated blocks */
unsigned int sec_hal_msg_area_alloc_count(void);

/* ******************************** END ************************************ */

#endif /* SEC_HAL_ALLOC_H_ */
