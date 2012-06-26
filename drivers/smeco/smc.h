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

#define SMC_SW_VERSION  "0.0.30"

#define SMC_ERROR   0
#define SMC_OK      1

    /* Additional SMC error return values */
#define SMC_ERROR_BUFFER_FULL  2
#define SMC_ERROR_NOT_READY    3


#include "smc_conf.h"
#include "smc_fifo.h"
#include "smc_memory_mgmt.h"


    /*
     * SMC internal configuration values
     */
#define SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX    20

    /*
     * SMC Channel Common Priorities
     */
#define SMC_CHANNEL_PRIORITY_HIGHEST        0x00
#define SMC_CHANNEL_PRIORITY_DEFAULT        0x7F
#define SMC_CHANNEL_PRIORITY_LOWEST         0xFF

    /* TODO Make this channel specific and configurable */
#define SMC_TIMER_PERIOD_FIFO_FULL_CHECK_USEC 500   /* 500 usec period */

    /*
     * SMC Channel internal message flags
     * The MSB bit is set for SMC internal messages.
     */
#define SMC_MSG_FLAG_MASK                   0x80000000        /* Mask to filter if FIFO contains internal message */
#define SMC_MSG_FLAG_SYNC_INFO_REQ          0x80000001        /* Used in channel synchronization */
#define SMC_MSG_FLAG_SYNC_INFO_RESP         0x80000002        /* Used in channel synchronization */
#define SMC_MSG_FLAG_SYNC_SEND_REQ          0x80000004        /* Used in channel synchronization */
#define SMC_MSG_FLAG_FREE_MEM_MDB           0x80000008        /* If flag is set, the FIFO message is for freeing memory in MDB */
#define SMC_MSG_FLAG_CREATE_CHANNEL         0x80000010        /* If flag is set, the FIFO message is for creating new SMC channel */
#define SMC_MSG_FLAG_CHANNEL_EVENT_USER     0x80000020        /* If flag is set, the FIFO message is an event from remote host to user */
#define SMC_MSG_FLAG_VERSION_INFO_REQ       0x80000040        /* Version info request */
#define SMC_MSG_FLAG_VERSION_INFO_RESP      0x80000080        /* Version info response */

#define SMC_FIFO_IS_INTERNAL_MESSAGE( flag )                    (((flag)&SMC_MSG_FLAG_MASK)==SMC_MSG_FLAG_MASK)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ( flag )           (((flag)&SMC_MSG_FLAG_SYNC_INFO_REQ)==SMC_MSG_FLAG_SYNC_INFO_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP( flag )          (((flag)&SMC_MSG_FLAG_SYNC_INFO_RESP)==SMC_MSG_FLAG_SYNC_INFO_RESP)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB( flag )       (((flag)&SMC_MSG_FLAG_FREE_MEM_MDB)==SMC_MSG_FLAG_FREE_MEM_MDB)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CREATE_CHANNEL( flag )     (((flag)&SMC_MSG_FLAG_FREE_MEM_MDB)==SMC_MSG_FLAG_FREE_MEM_MDB)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_CHANNEL_EVENT_USER( flag ) (((flag)&SMC_MSG_FLAG_CHANNEL_EVENT_USER)==SMC_MSG_FLAG_CHANNEL_EVENT_USER)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_REQ( flag )        (((flag)&SMC_MSG_FLAG_VERSION_INFO_REQ)==SMC_MSG_FLAG_VERSION_INFO_REQ)
#define SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_RESP( flag )       (((flag)&SMC_MSG_FLAG_VERSION_INFO_RESP)==SMC_MSG_FLAG_VERSION_INFO_RESP)


    /* This macro defines internal messages which data pointer must be translated */
#define SMC_INTERNAL_MESSAGE_DATA_ADDRESS_TRANSLATE( flag )   ( !SMC_FIFO_IS_INTERNAL_MESSAGE(flag) || SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB(flag) )

    /*
     * SMC Channel state flags
     */
#define SMC_CHANNEL_STATE_SYNCHRONIZED      0x00000001      /* If set the channel is synchronized with remote */
#define SMC_CHANNEL_STATE_SYNC_MSG_SENT     0x00000002      /* If set the synchronize msg has been sent to remote */
#define SMC_CHANNEL_STATE_SYNC_MSG_RECEIVED 0x00000004      /* If set the synchronize msg has been received from remote */
#define SMC_CHANNEL_STATE_SHM_CONFIGURED    0x00000008      /* If set the shared memory of the channel is initialized */
#define SMC_CHANNEL_STATE_RECEIVE_DISABLED  0x00000010      /* If set the channel does not receive any data (data is buffered to MDB)*/
#define SMC_CHANNEL_STATE_MDB_OUT_OF_MEM    0x00000020      /* If set the channel's OUT MDB is out of memory*/
#define SMC_CHANNEL_STATE_FIFO_FULL         0x00000040      /* If set the channel's out FIFO is FULL */
#define SMC_CHANNEL_STATE_SEND_DISABLED     0x00000080      /* If set the channel does not send any data (upper layer tx buffers are stopped )*/

    /* Defines the flags to be set before SMC send is possible */
#define SMC_CHANNEL_STATE_READY_TO_SEND     (SMC_CHANNEL_STATE_SYNCHRONIZED | SMC_CHANNEL_STATE_SHM_CONFIGURED)

    /*
     * SMC Misc defines
     */
#define SMC_SYNC_MSG_FIFO_REQ               0x108F1F0       /* Synchronization request  message put in the FIFO */
#define SMC_SYNC_MSG_FIFO_RESP              0xF1F0104       /* Synchronization response message put in the FIFO */

    /*
     * Copy Scheme bits
     */
#define SMC_COPY_SCHEME_COPY_IN_SEND        0x01
#define SMC_COPY_SCHEME_COPY_IN_RECEIVE     0x02

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

    /*
     * SMC Macros for common usage
     */
#define SMC_CHANNEL_GET( smc_instance, channel_id )    ( smc_instance->smc_channel_ptr_array[channel_id] )

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
    SMC_STOP_SEND,                          /* Local or remote channel requests to stop sending data */
    SMC_RESUME_SEND,                        /* Local or remote channel requests to continue sending data */
    SMC_RECEIVE_STOPPED,                    /* The remote channel has stopped to receive data */
    SMC_RECEIVE_RESUMED,                    /* The remote channel has started to receive data again */
    SMC_RESET,                              /* Remote channel requests reset */
    SMC_CLOSED,                             /* Remote SMC instance is closed */
    SMC_VERSION_INFO_REMOTE                 /* Received version information from the remote channel */

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
    uint8_t                             stop_counter;                    /* Start / stop counter to handle multiple start/stop events for the channel */

    struct _smc_t*                      smc_instance;                    /* Pointer to SMC instance the channel belongs to */

    uint32_t                            state;                           /* Indicates the current status of the SMC Channel */

    smc_fifo_t*                         fifo_out;                        /* FIFO for outgoing data */
    smc_fifo_t*                         fifo_in;                         /* FIFO for incoming data */

    smc_timer_t*                        fifo_timer;                      /* Timer to use when out FIFO is full */

    smc_lock_t*                         lock_write;                      /* Lock for write operations */
    smc_lock_t*                         lock_read;                       /* Lock for read operations  */
    smc_lock_t*                         lock_mdb;                        /* Lock for MDB operations  */

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

} smc_channel_t;


    /*
     * SMC instance structure
     */
typedef struct _smc_t
{
    uint8_t           cpu_id_remote;          /* ID of remote CPU */
    uint8_t           cpu_id_local;           /* ID of the CPU used locally */
    uint8_t           is_master;              /* Master flag */
    uint8_t           smc_channel_list_count; /* Count of channels initialized to channel pointer array */

    smc_channel_t**   smc_channel_ptr_array;  /* Array of pointers to SMC channels */

    smc_conf_t*       smc_instance_conf;      /* Configuration for the whole instance */

    smc_shm_config_t* smc_shm_conf;           /* Shared memory configuration for the SMC instance */
    uint8_t*          smc_memory_first_free;  /* Points to end of the SHM area that is in use of the SHM configuration */
    void*             smc_parent_ptr;         /* Optional pointer to parent object, can be used in platform specific implementations */

} smc_t;


    /*
     * SMC user data structure to use to send additional
     * data to remote CPU.
     */
typedef struct _smc_user_data_t
{
    uint32_t flags;
     int32_t userdata1;
     int32_t userdata2;
     int32_t userdata3;
     int32_t userdata4;
     int32_t userdata5;

} smc_user_data_t;


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
smc_semaphore_t* smc_semaphore_create( void );


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
smc_t*  smc_instance_get_control( void );

    /*
     * Misc functions
     */
char*    smc_get_version   ( void );
uint32_t smc_version_to_int( char* version );
char*    smc_version_to_str( uint32_t version );
int      smc_atoi          ( char* a );
char*    smc_utoa          ( uint32_t i );


#endif

/* EOF */
