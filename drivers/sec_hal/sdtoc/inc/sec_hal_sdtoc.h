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
#ifndef SEC_HAL_SDTOC_H
#define SEC_HAL_SDTOC_H


/* ************************ HEADER (INCLUDE) SECTION *********************** */
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <inttypes.h>
#endif

int sec_hal_sdtoc_area_init(unsigned long start, unsigned long size);
uint32_t sec_hal_sdtoc_read(uint32_t *input_data, void *output_data);
void * sdtoc_get_payload(const uint32_t object_id, uint32_t * const p_length);

/* ******************************** END ************************************ */

#endif /* SEC_HAL_SDTOC_H */
