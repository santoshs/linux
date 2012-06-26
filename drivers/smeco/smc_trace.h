/*
*   Functions for Smeco RD traces
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

Version:       5    22-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Common platform specific header file added

Version:       1    19-Oct-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_TRACE_H
#define SMC_TRACE_H

#include "smc_trace_platform.h"

#define SMC_RD_TRACE_PREFIX                   "SMC:"

/**
 * R&D common traces enabling
 */
#define SMC_TRACE_VERSION_INFO_ENABLED
#define SMC_TRACE_STARTUP_INFO_ENABLED
#define SMC_TRACE_ASSERT_ENABLED
#define SMC_TRACE_ERROR_ENABLED
#define SMC_TRACE_WARNING_ENABLED
/*#define SMC_TRACE_DEBUG_ENABLED*/
/*#define SMC_TRACE_INFO_ENABLED*/

/* -----------------------------------
 * Module specific traces.
 * These flags can be enabled also in the smc_trace_platform.h file
 */

/*
#define SMC_TRACE_FIFO_ENABLED
#define SMC_TRACE_FIFO_GET_ENABLED
#define SMC_TRACE_FIFO_PUT_ENABLED
#define SMC_TRACE_MDB_ENABLED
#define SMC_TRACE_SIGNALS_ENABLED
#define SMC_TRACE_SIGNAL_RECEIVE_ENABLED
#define SMC_TRACE_EVENT_SEND_ENABLED
#define SMC_TRACE_EVENT_RECEIVED_ENABLED
#define SMC_TRACE_LOCK_ENABLED
#define SMC_TRACE_TRANSMIT_ENABLED
#define SMC_TRACE_TRANSMIT_DATA_ENABLED
#define SMC_TRACE_RECEIVE_ENABLED
#define SMC_TRACE_SEND_PACKET_ENABLED
#define SMC_TRACE_RECEIVE_PACKET_ENABLED
#define SMC_TRACE_TIMER_ENABLED
*/

/**
 * ----------- R&D Trace macros begin ---------------
 */

#ifdef SMC_TRACE_VERSION_INFO_ENABLED
  #define SMC_TRACE_PRINTF_VERSION(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX" " __VA_ARGS__ )
#else
  #define SMC_TRACE_PRINTF_VERSION(...)
#endif

#ifdef SMC_TRACE_STARTUP_INFO_ENABLED
  #define SMC_TRACE_PRINTF_STARTUP(...)                 SMC_TRACE_PRINTF_UI( SMC_RD_TRACE_PREFIX" " __VA_ARGS__ )
#else
  #define SMC_TRACE_PRINTF_STARTUP(...)
#endif

#ifdef SMC_TRACE_FIFO_ENABLED
  #define SMC_TRACE_PRINTF_FIFO(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"FIFO: " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_FIFO_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_FIFO(...)
  #define SMC_TRACE_PRINTF_FIFO_DATA( length, data )
#endif

#ifdef SMC_TRACE_FIFO_GET_ENABLED
  #define SMC_TRACE_PRINTF_FIFO_GET(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"FIFO: get:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_FIFO_GET_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_FIFO_GET(...)
  #define SMC_TRACE_PRINTF_FIFO_GET_DATA( length, data )
#endif

#ifdef SMC_TRACE_FIFO_PUT_ENABLED
  #define SMC_TRACE_PRINTF_FIFO_PUT(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"FIFO: put:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_FIFO_PUT_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_FIFO_PUT(...)
  #define SMC_TRACE_PRINTF_FIFO_PUT_DATA( length, data )
#endif

#ifdef SMC_TRACE_TIMER_ENABLED
  #define SMC_TRACE_PRINTF_TIMER(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"TIMER:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_TIMER_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_TIMER(...)
  #define SMC_TRACE_PRINTF_TIMER_DATA( length, data )
#endif

#ifdef SMC_TRACE_EVENT_SEND_ENABLED
  #define SMC_TRACE_PRINTF_EVENT_SEND(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"EVENT: send:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_EVENT_SEND_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_EVENT_SEND(...)
  #define SMC_TRACE_PRINTF_EVENT_SEND_DATA( length, data )
#endif

#ifdef SMC_TRACE_EVENT_RECEIVED_ENABLED
  #define SMC_TRACE_PRINTF_EVENT_RECEIVED(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"EVENT: recv:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_EVENT_RECEIVED_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_EVENT_RECEIVED(...)
  #define SMC_TRACE_PRINTF_EVENT_RECEIVED_DATA( length, data )
#endif


#ifdef SMC_TRACE_LOCK_ENABLED
  #define SMC_TRACE_PRINTF_LOCK(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"LOCK: " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_LOCK_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_LOCK(...)
  #define SMC_TRACE_PRINTF_LOCK_DATA( length, data )
#endif

#ifdef SMC_TRACE_MDB_ENABLED
  #define SMC_TRACE_PRINTF_MDB(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"MDB:  " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_MDB_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_MDB(...)
  #define SMC_TRACE_PRINTF_MDB_DATA( length, data )
#endif

#ifdef SMC_TRACE_SIGNALS_ENABLED
  #define SMC_TRACE_PRINTF_SIGNAL(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"SIGNL:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_SIGNAL_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_SIGNAL(...)
  #define SMC_TRACE_PRINTF_SIGNAL_DATA( length, data )
#endif

#ifdef SMC_TRACE_SIGNAL_RECEIVE_ENABLED
  #define SMC_TRACE_PRINTF_SIGNAL_RECEIVE(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"SIGRC:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_SIGNAL_RECEIVE_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_SIGNAL_RECEIVE(...)
  #define SMC_TRACE_PRINTF_SIGNAL_RECEIVE_DATA( length, data )
#endif

#ifdef SMC_TRACE_TRANSMIT_ENABLED
  #define SMC_TRACE_PRINTF_TRANSMIT(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"XMIT: " __VA_ARGS__ )

  #ifdef SMC_TRACE_TRANSMIT_DATA_ENABLED
      #define SMC_TRACE_PRINTF_TRANSMIT_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
  #else
      #define SMC_TRACE_PRINTF_TRANSMIT_DATA( length, data )
  #endif
#else
  #define SMC_TRACE_PRINTF_TRANSMIT(...)
  #define SMC_TRACE_PRINTF_TRANSMIT_DATA( length, data )
#endif

#ifdef SMC_TRACE_RECEIVE_ENABLED
  #define SMC_TRACE_PRINTF_RECEIVE(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"RECV: " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_RECEIVE_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_RECEIVE(...)
  #define SMC_TRACE_PRINTF_RECEIVE_DATA( length, data )
#endif

#ifdef SMC_TRACE_SEND_PACKET_ENABLED
  #define SMC_TRACE_PRINTF_SEND_PACKET(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"SEND: " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_SEND_PACKET_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_SEND_PACKET(...)
  #define SMC_TRACE_PRINTF_SEND_PACKET_DATA( length, data )
#endif

#ifdef SMC_TRACE_RECEIVE_PACKET_ENABLED
  #define SMC_TRACE_PRINTF_RECEIVE_PACKET(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"RECV: " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_RECEIVE_PACKET_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_RECEIVE_PACKET(...)
  #define SMC_TRACE_PRINTF_RECEIVE_PACKET_DATA( length, data )
#endif


#ifdef SMC_TRACE_ERROR_ENABLED
  #define SMC_TRACE_PRINTF_ERROR(...)                 SMC_TRACE_PRINTF_ALWAYS( SMC_RD_TRACE_PREFIX"ERR:  " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_ERROR_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_ERROR(...)
  #define SMC_TRACE_PRINTF_ERROR_DATA( length, data )
#endif

#ifdef SMC_TRACE_WARNING_ENABLED
  #define SMC_TRACE_PRINTF_WARNING(...)               SMC_TRACE_PRINTF(SMC_RD_TRACE_PREFIX"WARN: " __VA_ARGS__)
#else
  #define SMC_TRACE_PRINTF_WARNING(...)
#endif

#ifdef SMC_TRACE_DEBUG_ENABLED
  #define SMC_TRACE_PRINTF_DEBUG(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"DEBUG:" __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_DEBUG_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_DEBUG(...)
  #define SMC_TRACE_PRINTF_DEBUG_DATA( length, data )
#endif

#ifdef SMC_TRACE_INFO_ENABLED
  #define SMC_TRACE_PRINTF_INFO(...)                 SMC_TRACE_PRINTF( SMC_RD_TRACE_PREFIX"INFO: " __VA_ARGS__ )
  #define SMC_TRACE_PRINTF_INFO_DATA( length, data ) SMC_TRACE_PRINTF_DATA(length, data)
#else
  #define SMC_TRACE_PRINTF_INFO(...)
  #define SMC_TRACE_PRINTF_INFO_DATA( length, data )
#endif

#ifdef SMC_TRACE_ASSERT_ENABLED
  #define SMC_TRACE_PRINTF_ASSERT(...)                 SMC_TRACE_PRINTF_ALWAYS(SMC_RD_TRACE_PREFIX"ASSERT: *** " __VA_ARGS__)
#else
  #define SMC_TRACE_PRINTF_ASSERT(...)
#endif

#endif /* EOF */

