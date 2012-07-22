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
** Copyright (C) 2012 Renesas Electronics Corp.                              **
** All rights reserved.                                                      **
** ************************************************************************* */
#ifndef SEC_DISPATCH_H
#define SEC_DISPATCH_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <inttypes.h>
#endif



uint32_t sec_dispatcher(uint32_t appl_id, uint32_t flags, uint32_t spare_param, uint32_t out_msg_addr, uint32_t in_msg_addr );

#endif /* SEC_DISPATCH_H */
