/*
*   Functions for Smeco RD traces to use in Linux Kernel
*
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

Version:       2    14-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Linux Kernel only tracing set

Version:       1    08-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_TRACE_PLATFORM_H
#define SMC_TRACE_PLATFORM_H

//#if( SMC_TRACES_PRINTF==TRUE )
#ifdef SMC_APE_RDTRACE_ENABLED

    /* Experimental for RD runtime trace activation to Linux Kernel */
    #include "smc_trace_rdtrace.h"


#else
        /* No traces */
    #define RD_TRACE_SEND1(...)
    #define RD_TRACE_SEND2(...)
    #define RD_TRACE_SEND3(...)
    #define RD_TRACE_SEND4(...)
    #define RD_TRACE_SEND5(...)
#endif


#if( SMC_TRACES_PRINTF_KERN_ALERT == TRUE )
  #define KERNEL_DEBUG_LEVEL     KERN_ALERT
#else
  #define KERNEL_DEBUG_LEVEL     KERN_DEBUG
#endif

    /*
     *  Linux kernel side specific traces
     */
/*
#define SMC_TRACE_DEBUG_ENABLED
#define SMC_TRACE_INFO_ENABLED
#define SMC_TRACE_MDB_ENABLED
#define SMC_TRACE_RECEIVE_ENABLED
#define SMC_TRACE_TRANSMIT_ENABLED
#define SMC_TRACE_SIGNALS_ENABLED
#define SMC_TRACE_SIGNAL_RECEIVE_ENABLED
#define SMC_TRACE_FIFO_GET_ENABLED
#define SMC_TRACE_LOCK_ENABLED
#define SMC_TRACE_RECEIVE_PACKET_ENABLED
#define SMC_TRACE_EVENT_RECEIVED_ENABLED
*/


#if( SMC_TRACES_PRINTF==TRUE )
  #define SMC_TRACE_PRINTF(...)                printk(KERNEL_DEBUG_LEVEL __VA_ARGS__ )
  //#define SMC_TRACE_PRINTF(trace, ...)       printk(KERNEL_DEBUG_LEVEL "%s\n" trace, ##__VA_ARGS__ )
  #define SMC_TRACE_PRINTF_DATA(length, data)  smc_printf_data_linux_kernel( length, data )
#else
  #define SMC_TRACE_PRINTF(...)
  #define SMC_TRACE_PRINTF_DATA(length, data)
#endif

    /* Show UI traces */
#define SMC_TRACE_PRINTF_UI(...)                   printk(KERN_ALERT __VA_ARGS__ )
#define SMC_TRACE_PRINTF_ALWAYS(...)               printk(KERN_ALERT __VA_ARGS__ )
#define SMC_TRACE_PRINTF_ALWAYS_DATA(length, data) smc_printf_data_linux_kernel( length, data )
#endif
