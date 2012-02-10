/*  SMC L2MUX Specific Configuration header for Linux Kernel
*   Copyright © Renesas Mobile Corporation 2012. All rights reserved
*
*   This material, including documentation and any related source code
*   and information, is protected by copyright controlled by Renesas.
*   All rights are reserved. Copying, including reproducing, storing,
*   adapting, translating and modifying, including decompiling or
*   reverse engineering, any or all of this material requires the prior
*   written consent of Renesas. This material also contains
*   confidential information, which may not be disclosed to others
*   without the prior written consent of Renesas.
*/
#if 0
/*
Change history:

Version:       3    25-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  L2MUX specific code added in this file.

Version:       1    04-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#ifndef SMC_CONF_L2MUX_LINUX_H
#define SMC_CONF_L2MUX_LINUX_H

#include <linux/if_mhi.h>
#include <linux/mhi.h>
#include <linux/l2mux.h>
#include <linux/phonet.h>
#include <net/phonet/pn_dev.h>

#include "smc_linux.h"


#ifndef PN_DEV_HOST
    #define PN_DEV_HOST 0x00
#endif

#define SMC_CONF_L2MUX_LOCAL_CPU_ID  0xA9
#define SMC_CONF_L2MUX_REMOTE_CPU_ID 0xC0

#define SMC_L2MUX_QUEUE_COUNT        3

    /* L2MUX Queue mapping */
#define SMC_L2MUX_QUEUE_1_PHONET     0
#define SMC_L2MUX_QUEUE_2_MHI        1
#define SMC_L2MUX_QUEUE_3_MHDP       2

    /* L2MUX control header size */
#define SMC_L2MUX_HEADER_SIZE        4

#if( SMCTEST == TRUE )
    #define SMC_TEST_PHONET_DEVICE  0x70   /* Phonet messages to this device are routed to SMC test handler */
#endif


#endif

/* EOF */

