/*
*   SMC Common includes for Linux Kernel.
*   Copyright © Renesas Mobile Corporation 2011. All rights reserved
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

Version:       4    29-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Linux Kernel timer functions for SMC

Version:       1    22-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_COMMON_INCLUDES_PLATFORM_H
#define SMC_COMMON_INCLUDES_PLATFORM_H

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

  /* Linux kernel specific definitions */
#define TRUE              -1
#define FALSE              0
#define SMC_MALLOC(size)     kmalloc(size, GFP_KERNEL)
#define SMC_MALLOC_IRQ(size) kmalloc(size, GFP_ATOMIC)
#define SMC_FREE(p)        kfree(p)

#ifndef CHAR_BIT
    #define CHAR_BIT       8
#endif

#ifndef NULL
    #define NULL           0x00000000
#endif

#ifndef SMCTEST
    #define SMCTEST        FALSE
#endif



#define assert( param )    BUG_ON( !(param) )

uint32_t rand(void);

void     smc_printf_data_linux_kernel(int length, uint8_t* data);

#endif

