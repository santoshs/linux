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

/*#define SMC_APE_LINUX_KERNEL_STM*/    /* If defined, the SMC traces are routed to STM -> Ntrace */


#ifdef SMC_APE_RDTRACE_ENABLED
    #error "APE Trace flag enabled - please disable"
    /* RD runtime trace activation to Linux Kernel */
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
#define SMC_TRACE_SIGNAL_RAISE_ENABLED
#define SMC_TRACE_FIFO_GET_ENABLED
#define SMC_TRACE_FIFO_PUT_ENABLED
#define SMC_TRACE_LOCK_ENABLED
#define SMC_TRACE_RECEIVE_PACKET_ENABLED
#define SMC_TRACE_EVENT_RECEIVED_ENABLED
#define SMC_TRACE_DMA_ENABLED
#define SMC_TRACE_TASKLET_ENABLED
*/






#ifdef SMC_TRACE_TASKLET_ENABLED
  #define SMC_TRACE_PRINTF_TASKLET(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"TASKLET: " __VA_ARGS__ )
#else
  #define SMC_TRACE_PRINTF_TASKLET(...)
#endif

#ifdef SMC_APE_LINUX_KERNEL_STM
  #define SMC_TRACE_PRINTF(format, arg...)         smc_printk( format,## arg )
  #define SMC_TRACE_PRINTF_DATA(length, data)      smc_printf_data_linux_kernel_stm( length, data )

  #define SMC_TRACE_PRINTF_STM(format, arg...)     smc_printk( "SMC: "format,## arg )
  #define SMC_TRACE_PRINTF_DATA_STM(prefix_text, data, length, max_print_len)  smc_printk_data(prefix_text, data, length, max_print_len)
#elif( SMC_TRACES_PRINTF==TRUE )

  #define SMC_TRACE_PRINTF(format, arg...)     printk(KERNEL_DEBUG_LEVEL format "\n",## arg )
  #define SMC_TRACE_PRINTF_DATA(length, data)  smc_printf_data_linux_kernel( length, data )
  #define SMC_TRACE_PRINTF_STM(format, arg...)
  #define SMC_TRACE_PRINTF_DATA_STM(prefix_text, data, length, max_print_len)
#else
  #define SMC_TRACE_PRINTF(...)
  #define SMC_TRACE_PRINTF_STM(...)
  #define SMC_TRACE_PRINTF_DATA(length, data)
  #define SMC_TRACE_PRINTF_DATA_STM(prefix_text, data, length, max_print_len)
#endif

    /* Show UI traces */
//#define SMC_TRACE_PRINTF_UI(...)                   printk(KERN_ALERT __VA_ARGS__ )
//#define SMC_TRACE_PRINTF_ALWAYS(...)               printk(KERN_ALERT __VA_ARGS__ )

#ifdef SMC_APE_LINUX_KERNEL_STM
  #define SMC_TRACE_PRINTF_UI(format, arg...)        printk(KERN_ALERT format "\n",## arg ); smc_printk( format,## arg )
#else
  #define SMC_TRACE_PRINTF_UI(format, arg...)        printk(KERN_ALERT format "\n",## arg )
#endif

#ifdef SMC_APE_LINUX_KERNEL_STM
  #define SMC_TRACE_PRINTF_ALWAYS(format, arg...)        smc_printk( format,## arg )
  #define SMC_TRACE_PRINTF_ALWAYS_ERROR(format, arg...)  printk(KERN_ALERT format,## arg ); smc_printk( format,## arg )
  #define SMC_TRACE_PRINTF_ALWAYS_DATA(length, data)     smc_printk_data("SMC", data, length, 100)
#else
  #define SMC_TRACE_PRINTF_ALWAYS(format, arg...)        printk(KERN_ALERT format,## arg )
  #define SMC_TRACE_PRINTF_ALWAYS_ERROR(format, arg...)  printk(KERN_ALERT format,## arg )
  #define SMC_TRACE_PRINTF_ALWAYS_DATA(length, data)     smc_printf_data_linux_kernel( length, data )
#endif


#ifdef SMC_APE_LINUX_KERNEL_STM
  void smc_printk(const char *fmt, ...);
  void smc_vprintk(const char *fmt, va_list args);
  void smc_printk_data(const char* prefix_text, const uint8_t* data, int data_len, int max_print_len);
#endif /* #ifdef SMC_APE_LINUX_KERNEL_STM */

#endif
