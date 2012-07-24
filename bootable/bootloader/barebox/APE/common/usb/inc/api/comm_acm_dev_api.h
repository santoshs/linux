/*	comm_acm_dev_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __COMM_ACM_DEV_API_H__
#define __COMM_ACM_DEV_API_H__

#include "error_base.h"

#define COMM_ACM_DEV_STATUS_SUCCESS                 (0x00000000L)                               // success
#define COMM_ACM_DEV_STATUS_NOT_OPENED              (COMM_ACM_DEV_STATUS_BASE + 0x00000001L)    // device stack isn't open
#define COMM_ACM_DEV_STATUS_STILL_OPENED            (COMM_ACM_DEV_STATUS_BASE + 0x00000002L)    // device stack is still open
#define COMM_ACM_DEV_STATUS_ALREADY_OPENED          (COMM_ACM_DEV_STATUS_BASE + 0x00000003L)    // device stack has already opened
#define COMM_ACM_DEV_STATUS_NO_MEMORY               (COMM_ACM_DEV_STATUS_BASE + 0x00000004L)    // memory shortage

// (@htn)
#define USBDEV_COMTMOUT_TASK_PRIORITY               4
#define USBDEV_COMTMOUT_TASK_STACK_SIZE             1024

// Comm Class
int CommAcmDevOpen(void);
int CommAcmDevClose(void);

#endif /* __COMM_ACM_DEV_API_H__ */
