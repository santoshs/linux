/*
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

Version:       40   20-Nov-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.40

Version:       39   02-Nov-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.39

Version:       38   16-Oct-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.38

Version:       37   26-Sep-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.37

Version:       36   13-Sep-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.36

Version:       35   22-Aug-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.35, modem to APE wakeup triggering polarity change
               This version requires both APE and modem to be same, version added to g_smc_version_requirements table.

Version:       34   13-Aug-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.34

Version:       32   29-Jun-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.32

Version:       31   19-Jun-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.31, Modem core dump transfer support

Version:       30   08-Jun-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.30

Version:       29   07-Jun-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.29, Linux Kernel IRQ handler sequence changed -> IRQ clear set before handler

Version:       28   25-May-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.28, new memory mapping for SHM area (modem<->APE)

Version:       27   15-May-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.27

Version:       26   09-May-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.26, EOS2 ES2.0 sleep wakeup modifications

Version:       25   26-Apr-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.25, EOS2 ES2.0 wakeup modifications

Version:       24   19-Apr-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.24, EOS2 ES2.0 wakeup modifications

Version:       23   23-Mar-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.23, Modem DMA DL

Version:       22   29-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.22

Version:       21   27-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.21

Version:       20   24-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.20

Version:       19   23-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.19

Version:       15   07-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.15

Version:       14   06-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.14

Version:       13   20-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.13

Version:       12   18-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.12

Version:       11   17-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Improvements 0.0.11

Version:       10   04-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Code improvements, clean up, SMC ID field and instance array removed

Version:       9    20-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Code improvements, implementation

Version:       8    12-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Code improvements and clean up

Version:       7    02-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Code improvements and clean up

Version:       3    08-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  Platform independent code implemented

Version:       2    21-Oct-2011     Heikki Siikaluoma
Status:        draft
Description :  Structure implementations

Version:       1    19-Oct-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_H
#define SMC_H

#define SMC_SW_VERSION  "0.0.40"

#define SMC_ERROR   0
#define SMC_OK      1

    /* Additional SMC error return values */
#define SMC_ERROR_BUFFER_FULL      2
#define SMC_ERROR_NOT_READY        3
#define SMC_ERROR_FIFO_FULL        4
#define SMC_ERROR_MDB_OUT_OF_MEM   5


#include "smc_conf.h"
#include "smc_fifo.h"
#include "smc_memory_mgmt.h"


    /*
     * SMC internal configuration values
     */

#define SMC_PERFORM_VERSION_CHECK                /* If defined, the version compliance check is performed */


    /*
     * SMC Channel Common Priorities
     */
#define SMC_CHANNEL_PRIORITY_HIGHEST          0x00
#define SMC_CHANNEL_PRIORITY_DEFAULT          0x7F
#define SMC_CHANNEL_PRIORITY_LOWEST           0xFF

#define SMC_TIMER_PERIOD_FIFO_FULL_CHECK_USEC 500   /* 500 usec period */

    /*
     * SMC Channel internal message flags
     * The MSB bit is set for SMC internal messages.
     */
#define SMC_MSG_FLAG_MASK                   0x80000000              /* Mask to filter if FIFO contains internal message */
#define SMC_MSG_FLAG_SYNC_INFO_REQ          0x80000001              /* Used in channel synchronization */
#define SMC_MSG_FLAG_SYNC_INFO_RESP         0x80000002              /* Used in channel synchronization */
#define SMC_MSG_FLAG_SYNC_SEND_REQ          0x80000004              /* Used in channel synchronization */
#define SMC_MSG_FLAG_FREE_MEM_MDB           0x80000008              /* If flag is set, the FIFO message is for freeing memory in MDB */
#define SMC_MSG_FLAG_CREATE_CHANNEL         0x80000010              /* If flag is set, the FIFO message is for creating new SMC channel */
#define SMC_MSG_FLAG_CHANNEL_EVENT_USER     0x80000020              /* If flag is set, the FIFO message is an event from remote host to user */
#define SMC_MSG_FLAG_VERSION_INFO_REQ       0x80000040              /* Version info request */
#define SMC_MSG_FLAG_VERSION_INFO_RESP      0x80000080              /* Version info response */
#define SMC_MSG_FLAG_PING_REQ               0x80000100              /* Ping request  */
#define SMC_MSG_FLAG_PING_RESP              0x80000200              /* Ping response */
#define SMC_MSG_FLAG_CONFIG_REQ             0x80000400              /* Configuration request for single configuration value */
#define SMC_MSG_FLAG_CONFIG_RESP            0x80000800              /* Configuration response for single configuration value */
#define SMC_MSG_FLAG_CONFIG_SHM_REQ         0x80001000              /* Shared Memory area configuration request for single channel */
#define SMC_MSG_FLAG_CONFIG_SHM_RESP        0x80002000              /* Shared Memory area configuration response for single channel */
#define SMC_MSG_FLAG_REMOTE_CHANNELS_INIT   0x80004000              /* Send to remote when all own channels are initialized (not necessarily sync'd) */
#define SMC_MSG_FLAG_LOOPBACK_DATA_REQ      0x80008000              /* Loopback data sent from remote */
#define SMC_MSG_FLAG_LOOPBACK_DATA_RESP     0x80010000              /* Response to the loopback data req, the data is received in remote side */
#define SMC_MSG_FLAG_TRACE_ACTIVATE_REQ     0x80020000              /* Trace activation/deactivation message */
#define SMC_MSG_FLAG_REMOTE_CPU_CRASH       0x80040000              /* Remote CPU has crashed */

#define SMC_MSG_FLAG_MASK_FIFO_PACKET_BUFFERED    0x20000000        /* If bit set the fifo packet is buffered */

#define SMC_FIFO_DATA_PTR_IS_BUFFERED( flag )                        (((flag)&SMC_MSG_FLAG_MASK_FIFO_PACKET_BUFFERED)==SMC_MSG_FLAG_MASK_FIFO_PACKET_BUFFERED)
#define SMC_FIFO_SET_DATA_PTR_IS_BUFFERED( flag )                    ((flag) |= SMC_MSG_FLAG_MASK_FIFO_PACKET_BUFFERED)
#define SMC_FIFO_CLEAR_DATA_PTR_IS_BUFFERED( flag )                  ((flag) &= ~SMC_MSG_FLAG_MASK_FIFO_PACKET_BUFFERED)


#define SMC_MSG_FLAG_MASK_LOCAL_DATA_PTR    0x40000000              /* If bit set the data pointer was local */

#define SMC_FIFO_DATA_PTR_IS_LOCAL( flag )                        (((flag)&SMC_MSG_FLAG_MASK_LOCAL_DATA_PTR)==SMC_MSG_FLAG_MASK_LOCAL_DATA_PTR)
#define SMC_FIFO_SET_DATA_PTR_IS_LOCAL( flag )                    ((flag) |= SMC_MSG_FLAG_MASK_LOCAL_DATA_PTR)
#define SMC_FIFO_CLEAR_DATA_PTR_IS_LOCAL( flag )                  ((flag) &= ~SMC_MSG_FLAG_MASK_LOCAL_DATA_PTR)


#define SMC_FIFO_IS_INTERNAL_MESSAGE( flag )                      (((flag)&SMC_MSG_FLAG_MASK)==SMC_MSG_FLAG_MASK)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ( flag )             (((flag)&SMC_MSG_FLAG_SYNC_INFO_REQ)==SMC_MSG_FLAG_SYNC_INFO_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP( flag )            (((flag)&SMC_MSG_FLAG_SYNC_INFO_RESP)==SMC_MSG_FLAG_SYNC_INFO_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB( flag )         (((flag)&SMC_MSG_FLAG_FREE_MEM_MDB)==SMC_MSG_FLAG_FREE_MEM_MDB)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CREATE_CHANNEL( flag )       (((flag)&SMC_MSG_FLAG_FREE_MEM_MDB)==SMC_MSG_FLAG_FREE_MEM_MDB)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CHANNEL_EVENT_USER( flag )   (((flag)&SMC_MSG_FLAG_CHANNEL_EVENT_USER)==SMC_MSG_FLAG_CHANNEL_EVENT_USER)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_REQ( flag )          (((flag)&SMC_MSG_FLAG_VERSION_INFO_REQ)==SMC_MSG_FLAG_VERSION_INFO_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_RESP( flag )         (((flag)&SMC_MSG_FLAG_VERSION_INFO_RESP)==SMC_MSG_FLAG_VERSION_INFO_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_PING_REQ( flag )             (((flag)&SMC_MSG_FLAG_PING_REQ)==SMC_MSG_FLAG_PING_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_PING_RESP( flag )            (((flag)&SMC_MSG_FLAG_PING_RESP)==SMC_MSG_FLAG_PING_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_REQ( flag )           (((flag)&SMC_MSG_FLAG_CONFIG_REQ)==SMC_MSG_FLAG_CONFIG_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_RESP( flag )          (((flag)&SMC_MSG_FLAG_CONFIG_RESP)==SMC_MSG_FLAG_CONFIG_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ( flag )       (((flag)&SMC_MSG_FLAG_CONFIG_SHM_REQ)==SMC_MSG_FLAG_CONFIG_SHM_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_RESP( flag )      (((flag)&SMC_MSG_FLAG_CONFIG_SHM_RESP)==SMC_MSG_FLAG_CONFIG_SHM_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CHANNELS_INIT( flag ) (((flag)&SMC_MSG_FLAG_REMOTE_CHANNELS_INIT)==SMC_MSG_FLAG_REMOTE_CHANNELS_INIT)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_REQ( flag )    (((flag)&SMC_MSG_FLAG_LOOPBACK_DATA_REQ)==SMC_MSG_FLAG_LOOPBACK_DATA_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_RESP( flag )   (((flag)&SMC_MSG_FLAG_LOOPBACK_DATA_RESP)==SMC_MSG_FLAG_LOOPBACK_DATA_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_TRACE_ACTIVATE_REQ( flag )   (((flag)&SMC_MSG_FLAG_TRACE_ACTIVATE_REQ)==SMC_MSG_FLAG_TRACE_ACTIVATE_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CPU_CRASH( flag )     (((flag)&SMC_MSG_FLAG_REMOTE_CPU_CRASH)==SMC_MSG_FLAG_REMOTE_CPU_CRASH)


#define SMC_IS_LOOPBACK_MSG( flag ) ( SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_REQ(flag) || SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_RESP(flag) )

    /* This macro defines internal messages which data pointer must be translated */
#define SMC_INTERNAL_MESSAGE_DATA_ADDRESS_TRANSLATE( flag )    (!SMC_FIFO_IS_INTERNAL_MESSAGE(flag) || \
                                                                 SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB(flag) || \
                                                                 SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ( flag ) || \
                                                                 SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_REQ( flag ) || \
                                                                 SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_RESP( flag ) || \
                                                                 SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CPU_CRASH( flag ) )

    /*
     * SMC instance status flags
     */
#define SMC_INSTANCE_STATUS_INIT_NONE               0x00
#define SMC_INSTANCE_STATUS_INIT_CHANNELS_LOCAL     0x01            /* Local channels are initialized */
#define SMC_INSTANCE_STATUS_INIT_CHANNELS_REMOTE    0x02            /* Remote channels are initialized */

#define SMC_INSTANCE_STATUS_FIXED_CONFIG_SENT       0x04            /* Fixed config is sent */

#define SMC_INSTANCE_STATUS_INIT_SET_LOCAL( status )          ((status) |= SMC_INSTANCE_STATUS_INIT_CHANNELS_LOCAL)
#define SMC_INSTANCE_STATUS_INIT_SET_REMOTE( status )         ((status) |= SMC_INSTANCE_STATUS_INIT_CHANNELS_REMOTE)
#define SMC_INSTANCE_STATUS_SET_FIXED_CONFIG_SENT( status )   ((status) |= SMC_INSTANCE_STATUS_FIXED_CONFIG_SENT)

#define SMC_INSTANCE_CAN_SEND_FIXED_CONFIG(status)            (((status&SMC_INSTANCE_STATUS_INIT_CHANNELS_LOCAL)==SMC_INSTANCE_STATUS_INIT_CHANNELS_LOCAL) && ((status&SMC_INSTANCE_STATUS_INIT_CHANNELS_REMOTE)==SMC_INSTANCE_STATUS_INIT_CHANNELS_REMOTE) && ((status&SMC_INSTANCE_STATUS_FIXED_CONFIG_SENT)!=SMC_INSTANCE_STATUS_FIXED_CONFIG_SENT))


#define SMC_FIXED_CONFIG_NO_CHANGES         0x00000000
#define SMC_FIXED_CONFIG_CHANGE_FIFO_IN     0x00000001
#define SMC_FIXED_CONFIG_CHANGE_FIFO_OUT    0x00000002
#define SMC_FIXED_CONFIG_CHANGE_MDB_IN      0x00000004
#define SMC_FIXED_CONFIG_CHANGE_MDB_OUT     0x00000008
#define SMC_FIXED_CONFIG_CHANGE_SHM_SIZE    0x00000010
#define SMC_FIXED_CONFIG_CHANGE_SHM_START   0x00000020

    /*
     * SMC Channel state flags
     */
#define SMC_CHANNEL_STATE_SYNCHRONIZED         0x00000001           /* If set the channel is synchronized with remote */
#define SMC_CHANNEL_STATE_SYNC_MSG_SENT        0x00000002           /* If set the synchronize msg has been sent to remote */
#define SMC_CHANNEL_STATE_SYNC_MSG_RECEIVED    0x00000004           /* If set the synchronize msg has been received from remote */
#define SMC_CHANNEL_STATE_SHM_CONFIGURED       0x00000008           /* If set the shared memory of the channel is initialized */
#define SMC_CHANNEL_STATE_RECEIVE_DISABLED     0x00000010           /* If set the channel does not receive any data (data is buffered to MDB)*/
#define SMC_CHANNEL_STATE_MDB_OUT_OF_MEM       0x00000020           /* If set the channel's OUT MDB is out of memory*/
#define SMC_CHANNEL_STATE_FIFO_FULL            0x00000040           /* If set the channel's out FIFO is FULL */
#define SMC_CHANNEL_STATE_SEND_DISABLED        0x00000080           /* If set the channel does not send any data (upper layer tx buffers are stopped )*/
#define SMC_CHANNEL_STATE_SEND_STOP_BY_REMOTE  0x00000100           /* If set the channel send disable was set by the remote */
#define SMC_CHANNEL_STATE_SEND_DISABLED_REMOTE 0x00000200           /* If set the remote channel is set to not send any data to us (upper layer tx buffers are stopped )*/
#define SMC_CHANNEL_STATE_SEND_DISABLED_XMIT   0x00000400           /* If set the transmit function has disabled the queue */

    /* Defines the flags to be set before SMC send is possible */
#define SMC_CHANNEL_STATE_READY_TO_SEND     (SMC_CHANNEL_STATE_SYNCHRONIZED | SMC_CHANNEL_STATE_SHM_CONFIGURED)

    /*
     * SMC Misc defines
     */
#define SMC_SYNC_MSG_FIFO_REQ                   0x108F1F0           /* Synchronization request  message put in the FIFO */
#define SMC_SYNC_MSG_FIFO_RESP                  0xF1F0104           /* Synchronization response message put in the FIFO */

    /*
     * Copy Scheme bits
     */
#define SMC_COPY_SCHEME_COPY_IN_SEND             0x01
#define SMC_COPY_SCHEME_COPY_IN_RECEIVE          0x02

#define SMC_COPY_SCHEME_SEND_IS_COPY( bits )     (((bits)&SMC_COPY_SCHEME_COPY_IN_SEND)==SMC_COPY_SCHEME_COPY_IN_SEND)
#define SMC_COPY_SCHEME_RECEIVE_IS_COPY( bits )  (((bits)&SMC_COPY_SCHEME_COPY_IN_RECEIVE)==SMC_COPY_SCHEME_COPY_IN_RECEIVE)

#define SMC_COPY_SCHEME_SEND_SET_COPY( bits )    ((bits) |= SMC_COPY_SCHEME_COPY_IN_SEND)
#define SMC_COPY_SCHEME_RECEIVE_SET_COPY( bits ) ((bits) |= SMC_COPY_SCHEME_COPY_IN_RECEIVE)

    /*
     * SMC Channel state bit read/set/clear macros
     */
#define SMC_CHANNEL_STATE_IS_READY_TO_SEND( state )          (((state)&SMC_CHANNEL_STATE_READY_TO_SEND)==SMC_CHANNEL_STATE_READY_TO_SEND)

#define SMC_CHANNEL_STATE_IS_SYNCHRONIZED( state )           (((state)&SMC_CHANNEL_STATE_SYNCHRONIZED)==SMC_CHANNEL_STATE_SYNCHRONIZED)
#define SMC_CHANNEL_STATE_SET_SYNCHRONIZED( state )          ((state) |= SMC_CHANNEL_STATE_SYNCHRONIZED)
#define SMC_CHANNEL_STATE_CLEAR_SYNCHRONIZED( state )        ((state) &= ~SMC_CHANNEL_STATE_SYNCHRONIZED)

#define SMC_CHANNEL_STATE_IS_SYNC_SENT( state )              (((state)&SMC_CHANNEL_STATE_SYNC_MSG_SENT)==SMC_CHANNEL_STATE_SYNC_MSG_SENT)
#define SMC_CHANNEL_STATE_SET_SYNC_SENT( state )             ((state) |= SMC_CHANNEL_STATE_SYNC_MSG_SENT)
#define SMC_CHANNEL_STATE_CLEAR_SYNC_SENT( state )           ((state) &= ~SMC_CHANNEL_STATE_SYNC_MSG_SENT)

#define SMC_CHANNEL_STATE_IS_SYNC_RECEIVED( state )          (((state)&SMC_CHANNEL_STATE_SYNC_MSG_RECEIVED)==SMC_CHANNEL_STATE_SYNC_MSG_RECEIVED)
#define SMC_CHANNEL_STATE_SET_SYNC_RECEIVED( state )         ((state) |= SMC_CHANNEL_STATE_SYNC_MSG_RECEIVED)
#define SMC_CHANNEL_STATE_CLEAR_SYNC_RECEIVED( state )       ((state) &= ~SMC_CHANNEL_STATE_SYNC_MSG_RECEIVED)

#define SMC_CHANNEL_STATE_IS_SHM_CONFIGURED( state )         (((state)&SMC_CHANNEL_STATE_SHM_CONFIGURED)==SMC_CHANNEL_STATE_SHM_CONFIGURED)
#define SMC_CHANNEL_STATE_SET_SHM_CONFIGURED( state )        ((state) |= SMC_CHANNEL_STATE_SHM_CONFIGURED)
#define SMC_CHANNEL_STATE_CLEAR_SHM_CONFIGURED( state )      ((state) &= ~SMC_CHANNEL_STATE_SHM_CONFIGURED)

#define SMC_CHANNEL_STATE_RECEIVE_IS_DISABLED( state )       (((state)&SMC_CHANNEL_STATE_RECEIVE_DISABLED)==SMC_CHANNEL_STATE_RECEIVE_DISABLED)
#define SMC_CHANNEL_STATE_SET_RECEIVE_IS_DISABLED( state )   ((state) |= SMC_CHANNEL_STATE_RECEIVE_DISABLED)
#define SMC_CHANNEL_STATE_CLEAR_RECEIVE_IS_DISABLED( state ) ((state) &= ~SMC_CHANNEL_STATE_RECEIVE_DISABLED)

#define SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( state )         (((state)&SMC_CHANNEL_STATE_MDB_OUT_OF_MEM)==SMC_CHANNEL_STATE_MDB_OUT_OF_MEM)
#define SMC_CHANNEL_STATE_SET_MDB_OUT_OF_MEM( state )        ((state) |= SMC_CHANNEL_STATE_MDB_OUT_OF_MEM)
#define SMC_CHANNEL_STATE_CLEAR_MDB_OUT_OF_MEM( state )      ((state) &= ~SMC_CHANNEL_STATE_MDB_OUT_OF_MEM)

#define SMC_CHANNEL_STATE_IS_FIFO_FULL( state )              (((state)&SMC_CHANNEL_STATE_FIFO_FULL)==SMC_CHANNEL_STATE_FIFO_FULL)
#define SMC_CHANNEL_STATE_SET_FIFO_FULL( state )             ((state) |= SMC_CHANNEL_STATE_FIFO_FULL)
#define SMC_CHANNEL_STATE_CLEAR_FIFO_FULL( state )           ((state) &= ~SMC_CHANNEL_STATE_FIFO_FULL)

#define SMC_CHANNEL_STATE_SEND_IS_DISABLED( state )          (((state)&SMC_CHANNEL_STATE_SEND_DISABLED)==SMC_CHANNEL_STATE_SEND_DISABLED)
#define SMC_CHANNEL_STATE_SET_SEND_IS_DISABLED( state )      ((state) |= SMC_CHANNEL_STATE_SEND_DISABLED)
#define SMC_CHANNEL_STATE_CLEAR_SEND_IS_DISABLED( state )    ((state) &= ~SMC_CHANNEL_STATE_SEND_DISABLED)

#define SMC_CHANNEL_STATE_IS_SET_STOP_SEND_FROM_REMOTE( state )   (((state)&SMC_CHANNEL_STATE_SEND_STOP_BY_REMOTE)==SMC_CHANNEL_STATE_SEND_STOP_BY_REMOTE)
#define SMC_CHANNEL_STATE_SET_STOP_SEND_FROM_REMOTE( state )      ((state) |= SMC_CHANNEL_STATE_SEND_STOP_BY_REMOTE)
#define SMC_CHANNEL_STATE_CLEAR_STOP_SEND_FROM_REMOTE( state )    ((state) &= ~SMC_CHANNEL_STATE_SEND_STOP_BY_REMOTE)

#define SMC_CHANNEL_STATE_IS_SEND_DISABLED_REMOTE( state )   (((state)&SMC_CHANNEL_STATE_SEND_DISABLED_REMOTE)==SMC_CHANNEL_STATE_SEND_DISABLED_REMOTE)
#define SMC_CHANNEL_STATE_SET_SEND_DISABLED_REMOTE( state )      ((state) |= SMC_CHANNEL_STATE_SEND_DISABLED_REMOTE)
#define SMC_CHANNEL_STATE_CLEAR_SEND_DISABLED_REMOTE( state )    ((state) &= ~SMC_CHANNEL_STATE_SEND_DISABLED_REMOTE)

#define SMC_CHANNEL_STATE_IS_SEND_DISABLED_XMIT( state )   (((state)&SMC_CHANNEL_STATE_SEND_DISABLED_XMIT)==SMC_CHANNEL_STATE_SEND_DISABLED_XMIT)
#define SMC_CHANNEL_STATE_SET_SEND_DISABLED_XMIT( state )      ((state) |= SMC_CHANNEL_STATE_SEND_DISABLED_XMIT)
#define SMC_CHANNEL_STATE_CLEAR_SEND_DISABLED_XMIT( state )    ((state) &= ~SMC_CHANNEL_STATE_SEND_DISABLED_XMIT)

    /* Check that it is possible to allow channel send */
#define SMC_CHANNEL_STATE_ALLOW_RESUME_SEND( state )         ( !SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM(state) && !SMC_CHANNEL_STATE_IS_FIFO_FULL(state) && !SMC_CHANNEL_STATE_IS_SET_STOP_SEND_FROM_REMOTE(state) && !SMC_CHANNEL_STATE_IS_SEND_DISABLED_XMIT(state))


#define SMC_SIGNAL_SENSE_RISING_EDGE              0x00     /* Default value */
#define SMC_SIGNAL_SENSE_FALLING_EDGE             0x01
#define SMC_SIGNAL_SENSE_BOTH_EDGE                0x02

    /* ===============================================
     * SMC Macros for common usage
     */

#define SMC_CHANNEL_GET( smc_instance, channel_id )    ( smc_instance->smc_channel_ptr_array[channel_id] )

    /* SMC history data collection flags */

#define SMC_TRACE_HISTORY_DATA_ARRAY_SIZE           20        /* Data array size is for both send and receive */

#define SMC_TRACE_HISTORY_DATA_TYPE_NONE            0x00
#define SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND    0x01
#define SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE 0x02

#define SMC_TRACE_HISTORY_DATA_SEND_ENABLED( flag )      (((flag)&SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND)==SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND )
#define SMC_TRACE_HISTORY_DATA_RECEIVE_ENABLED( flag )   (((flag)&SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE)==SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE )

#define SMC_MESSAGE_REPLY_FLAG_VALUE              0x01234567

#define SMC_VERSION_MAJOR(version)                ((version>>24)&0xFF)
#define SMC_VERSION_MINOR(version)                ((version>>16)&0xFF)
#define SMC_VERSION_REVISION(version)             ((version)&0xFFFF)

#define SMC_VERSION_TO_INT(major,minor,revision)  (((major&0xFF)<<24)+((minor&0xFF)<<16)+(revision&0xFF))

#define SMC_VERSION_REQUIREMENT_LEVEL_NONE        0x00
#define SMC_VERSION_REQUIREMENT_LEVEL_WARNING     0x01
#define SMC_VERSION_REQUIREMENT_LEVEL_ERROR       0x02
#define SMC_VERSION_REQUIREMENT_LEVEL_ASSERT      0x03

#define SMC_CHANNEL_WAKELOCK_NONE                 0x00      /* No wakelock at all */
#define SMC_CHANNEL_WAKELOCK_TIMER                0x01      /* Wakelock uses timer for releasing */
#define SMC_CHANNEL_WAKELOCK_MESSAGE              0x02      /* Wakelock is enabled but the message receiver releases that */

    /*
     * SMC specific types
     */

typedef enum
{
    SMC_CHANNEL_READY_TO_SEND = 0x00,       /* SMC Channel is in sync */
    SMC_CHANNEL_DISCONNECTED,               /* Remote channel disconneted */
    SMC_SEND_FIFO_FULL,                     /* FIFO is full */
    SMC_SEND_FIFO_HAS_FREE_SPACE,           /* FIFO has free space available */
    SMC_SEND_MDB_OUT_OF_MEM,                /* Out MDB is out of memory */
    SMC_SEND_MDB_HAS_FREE_MEM,              /* MDB has free memory available */
    SMC_STOP_SEND,                          /* Remote channel requests to stop sending data */
    SMC_RESUME_SEND,                        /* Remote channel requests to continue sending data */
    SMC_RECEIVE_STOPPED,                    /* The remote channel has stopped to receive data */
    SMC_RECEIVE_RESUMED,                    /* The remote channel has started to receive data again */
    SMC_RESET,                              /* Remote channel requests reset */
    SMC_CLOSED,                             /* Remote SMC instance is closed */
    SMC_VERSION_INFO_REMOTE,                /* Received version information from the remote channel */
    SMC_STOP_SEND_LOCAL,                    /* Local channel requests to stop sending data */
    SMC_RESUME_SEND_LOCAL                   /* Local channel requests to continue sending data */

} SMC_CHANNEL_EVENT;


struct _smc_channel_t;
struct _smc_user_data_t;

    /*
     * Prototype of the callback function for the user receive data from SMC channel
     */
typedef void ( *smc_receive_data_callback )( void*   data,
                                             int32_t data_length,
                                             const struct _smc_user_data_t* userdata,
                                             const struct _smc_channel_t* channel );

    /*
     * Prototype of the callback function for user to define allocator function for data to receive
     */
typedef void* ( *smc_receive_data_allocator_callback )( const struct _smc_channel_t* channel, uint32_t size, struct _smc_user_data_t* userdata );

    /*
     * Prototype of the callback function for user to define deallocator function
     */
typedef void ( *smc_send_data_deallocator_callback )( const struct _smc_channel_t* channel, void* data, struct _smc_user_data_t* userdata );

    /*
     * Prototype of the callback function for the use to get events from SMC or from remote host
     */
typedef void ( *smc_event_callback )( const struct _smc_channel_t* channel, SMC_CHANNEL_EVENT event, void* event_data );


    /*
     * SMC Channel structure
     */
typedef struct _smc_channel_t
{
    uint8_t                             id;                              /* Channel ID / the index in the channel array */
    uint8_t                             priority;                        /* The priority of the channel, the highest priority is 0 */
    uint8_t                             copy_scheme;                     /* Copy scheme bits for send and receive */
    uint8_t                             wake_lock_flags;                 /* Channel specific Wakeup lock policy */

    struct _smc_t*                      smc_instance;                    /* Pointer to SMC instance the channel belongs to */

    uint32_t                            state;                           /* Indicates the current status of the SMC Channel */

    smc_fifo_t*                         fifo_out;                        /* FIFO for outgoing data */
    smc_fifo_t*                         fifo_in;                         /* FIFO for incoming data */

    uint32_t                            fifo_size_in;                    /* The size of the FIFO receiving data (remote) */
    uint32_t                            fifo_size_out;                   /* The size of the FIFO sending data (local) */

    smc_timer_t*                        fifo_timer;                      /* Timer to use when out FIFO is full */

    smc_lock_t*                         lock_write;                      /* Lock for write operations */
    smc_lock_t*                         lock_read;                       /* Lock for read operations  */
    smc_lock_t*                         lock_mdb;                        /* Lock for MDB operations  */
    smc_lock_t*                         lock_tx_queue;                   /* Lock for TX queue control operations */

    uint32_t                            mdb_size_in;                     /* The size of the MDB for receiving data (remote) */
    uint32_t                            mdb_size_out;                    /* The size of the MDB for sending data (local) */

    uint8_t*                            mdb_out;                         /* MDB OUT area */
    uint8_t*                            mdb_in;                          /* MDB IN  area  */
    struct _smc_mdb_channel_info_t*     smc_mdb_info;                    /* MDB info containing pool data */

    smc_signal_t*                       signal_remote;                   /* Pointer to signal to raise for sending data to remote */
    smc_signal_t*                       signal_local;                    /* Pointer to signal to get to receive data from remote  */

    smc_shm_config_t*                   smc_shm_conf_channel;            /* Shared memory configuration used in channel (own ptr -> must be destroyed) */

    smc_receive_data_callback           smc_receive_cb;                  /* Callback function to receive data from remote CPU using this channel */
    smc_receive_data_allocator_callback smc_receive_data_allocator_cb;   /* If set, the channel use this function to allocate pointer for data outside of the MDB */
    smc_send_data_deallocator_callback  smc_send_data_deallocator_cb;    /* If set, the channel calls this to deallocate the send data not belonging to MDB */
    smc_event_callback                  smc_event_cb;                    /* If set, the channel send events to this callback functions */

    smc_fifo_cell_t*                    fifo_buffer;                     /* FIFO buffer. Used when FIFO is not ready */

    uint8_t                             fifo_buffer_item_count;          /* Count of items in the FIFO buffer */
    uint8_t                             fifo_buffer_flushing;            /* Flag indicating that the buffer flush is ongoing*/
    uint8_t                             protocol;
    uint8_t                             trace_features;                  /* Trace feature bits (msg send/receive history) */

    uint32_t                            version_remote;                  /* Version of the remote SMC channel */

    struct _smc_semaphore_t*            send_semaphore;                  /* Semaphore used in smc_send to prevent overflow */

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED

    struct _smc_message_history_data_t* smc_history_data_sent;
    struct _smc_message_history_data_t* smc_history_data_received;
    uint16_t                            smc_history_len_sent;
    uint16_t                            smc_history_len_received;
    uint16_t                            smc_history_index_sent;
    uint16_t                            smc_history_index_received;

#endif /* #ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED */

    uint32_t                            dropped_packets_mdb_out;
    uint32_t                            dropped_packets_fifo_buffer;
    uint32_t                            send_packets_fifo_buffer;
    uint32_t                            fifo_buffer_copied_total;
} smc_channel_t;


    /*
     * SMC instance structure
     */
typedef struct _smc_t
{
    uint8_t           cpu_id_remote;                /* ID of remote CPU */
    uint8_t           cpu_id_local;                 /* ID of the CPU used locally */
    uint8_t           is_master;                    /* Master flag */
    uint8_t           smc_channel_list_count;       /* Count of channels initialized to channel pointer array */

    smc_channel_t**   smc_channel_ptr_array;        /* Array of pointers to SMC channels */
    smc_conf_t*       smc_instance_conf;            /* Configuration for the whole instance */
    smc_shm_config_t* smc_shm_conf;                 /* Shared memory configuration for the SMC instance */

    uint8_t*          smc_memory_first_free;        /* Points to end of the SHM area that is in use of the SHM configuration */
    void*             smc_parent_ptr;               /* Optional pointer to parent object, can be used in platform specific implementations */

    uint8_t           wakeup_event_sense_local;     /* Wakeup event sense of this SMC end point */
    uint8_t           init_status;                  /* Initialization statuses of local and remote instance */
    uint8_t           fill2;
    uint8_t           fill1;

} smc_t;


    /*
     * SMC user data structure to use to send additional
     * data to remote CPU.
     */
typedef struct _smc_user_data_t
{
    uint32_t flags;
    int32_t  userdata1;
    int32_t  userdata2;
    int32_t  userdata3;
    int32_t  userdata4;
    int32_t  userdata5;

} smc_user_data_t;


#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
    /*
     * SMC structure for send/receive data history collection
     */

typedef struct _smc_message_history_data_t
{
    uint8_t  channel_id;
    uint8_t  history_data_type;
    uint8_t  fill2;
    uint8_t  fill1;

    uint32_t timestamp;
    int32_t  shm_offset;
    uint32_t data_ptr_shm;          /* Pointer in the SHM area */
    uint32_t data_ptr;              /* Currently no data content history collected */

    uint32_t data_length;
    int32_t  userdata_flags;
    int32_t  userdata1;
    int32_t  userdata2;

    int32_t  userdata3;
    int32_t  userdata4;
    int32_t  userdata5;



} smc_message_history_data_t;



#endif

    /*
     * SMC version check structure.
     */
typedef struct _smc_version_check_t
{
    uint32_t version_requirement;
    uint32_t version_requirement_level;
    char*    version_requirement_reason;

} smc_version_check_t;

    /*
     * SMC Data structure for loopback messaging.
     */
typedef struct _smc_loopback_data_t
{
    uint32_t round_trip_counter;
    uint32_t loopback_data_length;
    uint32_t timestamp;
    uint32_t loopback_rounds_left;

    uint8_t  loopback_data[1];

} smc_loopback_data_t;


    /*
     * Common SMC API function declarations.
     */

smc_t*                smc_instance_create( smc_conf_t * smc_instance_conf );
smc_t*                smc_instance_create_ext(smc_conf_t* smc_instance_conf, void* parent_object);
uint32_t              smc_instance_get_free_shm( smc_t* smc_instance );
void                  smc_instance_destroy( smc_t * smc_instance );

smc_channel_t*        smc_channel_create( smc_channel_conf_t* smc_channel_conf );
void                  smc_channel_destroy( smc_channel_t* smc_channel );
smc_channel_t*        smc_channel_get( const smc_t* smc_instance, uint8_t smc_channel_id );
smc_conf_t*           smc_instance_get_conf( smc_t* smc_instance );

smc_channel_conf_t*   smc_channel_get_conf( smc_channel_t* smc_channel );
uint8_t               smc_channel_enable_receive_mode( smc_channel_t* smc_channel, uint8_t enable_receive);
void                  smc_channel_interrupt_handler( smc_channel_t* smc_channel );
uint8_t               smc_add_channel(smc_t* smc_instance, smc_channel_t* smc_channel, smc_channel_conf_t* smc_channel_conf);

uint32_t              smc_channel_calculate_required_shared_mem( smc_channel_conf_t* smc_channel_conf );
uint8_t               smc_send(smc_t* smc_instance, uint8_t channel_id, void* data, uint32_t data_length);
uint8_t               smc_send_ext(smc_channel_t* channel, void* data, uint32_t data_length, smc_user_data_t* userdata);
uint8_t               smc_send_event(smc_channel_t* channel, SMC_CHANNEL_EVENT event);

uint8_t               smc_send_event_ext(smc_channel_t* channel, SMC_CHANNEL_EVENT event, smc_user_data_t* userdata);
uint8_t               smc_initialize( smc_conf_t* smc_instance_conf );
void                  smc_channel_free_ptr_local(const smc_channel_t* smc_channel, void* ptr, smc_user_data_t* userdata);
void                  smc_fifo_poll( const smc_channel_t* smc_channel );

uint8_t               smc_is_all_channels_synchronized(smc_t* smc_instance);
uint8_t               smc_send_loopback_data_message( smc_channel_t* smc_channel, uint32_t loopback_data_len, uint32_t loopback_rounds );
smc_loopback_data_t*  smc_loopback_data_create( uint32_t size_of_message_payload );
uint8_t               smc_send_negotiable_configurations( smc_t* smc_instance );
uint8_t               smc_send_channels_initialized_message(smc_t* smc_instance);

    /*
     * Structure holding signal handler information.
     */
typedef struct
{
    smc_signal_t*  signal;
    smc_t*         smc_instance;
    smc_channel_t* smc_channel;

} smc_signal_handler_t;

    /*
     * SMC channel configuration functions
     */
uint8_t        smc_channel_change_priority( smc_t* smc_instance, uint8_t smc_channel_id, uint8_t new_priority );

    /* Function dump information specified SMC instance (channels, FIFOs, etc.) */
void           smc_instance_dump( smc_t* smc_instance );
void           smc_mdb_info_dump( char* indent, struct _smc_mdb_channel_info_t* smc_mdb_info, int32_t mem_offset, uint8_t out_mdb );

    /*
     * SMC signal handler prototypes
     * Implementations are in the platform specific modules.
     */

#define SMC_SIGNAL_TYPE_NONE          0x01000000
#define SMC_SIGNAL_TYPE_REGISTER      0x02000000
#define SMC_SIGNAL_TYPE_INTERRUPT     0x03000000

#define SMC_SIGNAL_TYPE_PRIVATE_START 0x00000000

#define SMC_SIGNAL_TYPE_IS_NOT_SET( signal_type )     ( (signal_type&SMC_SIGNAL_TYPE_NONE)     == SMC_SIGNAL_TYPE_NONE )
#define SMC_SIGNAL_TYPE_IS_REGISTER( signal_type )    ( (signal_type&SMC_SIGNAL_TYPE_REGISTER) == SMC_SIGNAL_TYPE_REGISTER )
#define SMC_SIGNAL_TYPE_IS_INTERRUPT( signal_type )   ( (signal_type&SMC_SIGNAL_TYPE_INTERRUPT)== SMC_SIGNAL_TYPE_INTERRUPT )


smc_signal_t*         smc_signal_create                ( uint32_t signal_id, uint32_t signal_type );
smc_signal_t*         smc_signal_copy                  ( smc_signal_t* signal );
void                  smc_signal_destroy               ( smc_signal_t* signal );
uint8_t               smc_signal_raise                 ( smc_signal_t* signal );
uint8_t               smc_signal_acknowledge           ( smc_signal_t* signal );
uint8_t               smc_signal_handler_register      ( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel );
uint8_t               smc_signal_handler_unregister    ( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel );
smc_signal_handler_t* smc_signal_handler_create_and_add( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel );
uint8_t               smc_signal_add_handler           ( smc_signal_handler_t* signal_handler );
smc_signal_handler_t* smc_signal_handler_get           ( uint32_t signal_id, uint32_t signal_type );
void                  smc_signal_handler_remove_and_destroy( smc_signal_handler_t* signal_handler );
void                  smc_signal_remove_handler        ( smc_signal_handler_t* signal_handler );
void                  smc_signal_dump                  ( char* indent, smc_signal_t* signal );

    /*
     * SMC Local Locking function prototypes.
     * Implementations are in the platform specific modules.
     */
smc_lock_t* smc_lock_create ( void );
void        smc_lock_destroy( smc_lock_t* lock );

    /*
     * SMC semaphore function prototypes.
     * Implementations are in the platform specific modules.
     */
smc_semaphore_t* smc_semaphore_create ( void );
void             smc_semaphore_destroy( smc_semaphore_t* semaphore );


    /*
     * SMC timer function prototypes
     *
     */
smc_timer_t* smc_timer_create( uint32_t timer_usec );

typedef void ( *smc_timer_callback )( uint32_t timer_data );

uint8_t      smc_timer_start  ( smc_timer_t* timer, smc_timer_callback* timer_cb, uint32_t timer_data );
uint8_t      smc_timer_stop   ( smc_timer_t* timer );
uint8_t      smc_timer_destroy( smc_timer_t* timer );

    /*
     * SMC Control Instance API.
     * NOTE: If the SMC Control Instance is not included
     *       the API works but returns NULL
     *       The API implementation is in platform leve.
     */
smc_t*   smc_instance_get_control    ( void );

smc_t**  smc_instance_array_get      ( void );
uint8_t  smc_instance_array_count_get( void );
// TODO Cleanup smc_lock_t* get_local_lock_smc_channel_ext(void);

    /*
     * Misc functions
     */
char*    smc_get_version            ( void );
uint32_t smc_version_to_int         ( char* version );
char*    smc_version_to_str         ( uint32_t version );
int      smc_atoi                   ( char* a );
char*    smc_utoa                   ( uint32_t i );
uint8_t  smc_channel_send_ping            ( smc_channel_t* smc_channel, uint8_t wait_reply );
uint8_t  smc_channel_send_config          ( smc_channel_t* smc_channel, uint32_t configuration_id, uint32_t configuration_value, uint8_t wait_reply );
uint8_t  smc_send_crash_info              ( char* crash_message );
uint8_t  smc_send_crash_indication        ( smc_channel_t* smc_channel, char* crash_message );
uint8_t  smc_channel_send_config_shm      ( smc_channel_t* smc_channel, uint8_t wait_reply);
uint8_t  smc_channel_send_fixed_config    ( smc_channel_t* smc_channel, smc_channel_t* smc_channel_target );
void     smc_channel_fixed_config_response( smc_channel_t* smc_channel, smc_channel_t* smc_channel_target, smc_user_data_t* userdata_resp );


/*
 * Version history functions
 */
#ifdef SMC_PERFORM_VERSION_CHECK

uint8_t smc_version_check(uint32_t version_local, uint32_t version_remote, uint8_t local_is_master);

#endif /* SMC_PERFORM_VERSION_CHECK */


/*
 * History data functions
 */
#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED

smc_message_history_data_t* smc_history_data_array_create( uint16_t array_len );

#endif

#endif

/* EOF */
