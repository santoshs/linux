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

Version:       10   31-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Signal type configuration added.

Version:       9    11-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Structure for the SMC instance configuration.

Version:       8    21-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Change common name for platform specific configuration header

Version:       7    14-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Linux Kernel object target included

Version:       3    08-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  Platform independent code implemented

Version:       1    19-Oct-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#ifndef SMC_CONF_H
#define SMC_CONF_H

    /* Product configurations */
#include "smc_instance_config_r8a73734_wge31.h"

#include "smc_conf_platform.h"    /* Platform specific configuration header */
#include "smc_memory_mgmt.h"


/**
 * Configuration structure for one logical SMC Channel.
 */
typedef struct
{
    uint8_t           channel_id;
    uint8_t           priority;                         /* The priority of the channel, the highest priority is 0 */

    uint32_t          fifo_size_in;                     /* The size of the FIFO receiving data (remote) */
    uint32_t          fifo_size_out;                    /* The size of the FIFO sending data (local) */
    uint32_t          mdb_size_in;                      /* The size of the MDB for receiving data (remote) */
    uint32_t          mdb_size_out;                     /* The size of the MDB for sending data (local) */
    smc_signal_t*     signal_remote;                    /* The signal used to indicate remote that data has been sent */
    smc_signal_t*     signal_local;                     /* The signal that indicates that data is received from remote side */

    void*             smc_receive_data_cb;              /* Function pointer for data receiving      */
    void*             smc_receive_data_allocator_cb;    /* Function pointer for memory allocation   */
    void*             smc_send_data_deallocator_cb;     /* Function pointer for memory deallocation */
    void*             smc_event_cb;                     /* Function pointer for receiving events    */

} smc_channel_conf_t;


/**
 * Configuration structure for one SMC instance.
 */
typedef struct
{
    uint8_t              cpu_id_remote;                 /* ID of the remote CPU */
    uint8_t              cpu_id_local;                  /* ID of the CPU used locally */
    uint8_t              is_master;                     /* If set the SMC instance is master */
    uint8_t              smc_channel_conf_count;        /* Count of SMC channels in smc_channel_conf_ptr_array */
    smc_channel_conf_t** smc_channel_conf_ptr_array;

    smc_shm_config_t*    smc_shm_conf;                  /* Global shared memory configuration (in SMC instance level) */

} smc_conf_t;

/**
 * Common configuration structures for SMC instance.
 * Used in the product configuration.
 */

typedef struct _smc_instance_conf_channel_t
{
    char*    name;
    uint32_t protocol;

    uint32_t fifo_size_master;
    uint32_t fifo_size_slave;

    uint32_t mdb_size_master;         /* Size of the MDB for data from master to slave */
    uint32_t mdb_size_slave;          /* Size of the MDB for data from slave to master */


        /*
         * Signal configuration for master side
         */
    uint32_t signal_id_master_to_slave;             /* Master is raising the signal */
    uint32_t signal_type_master_to_slave;

    uint32_t signal_id_master_from_slave;           /* Master is waiting the signal */
    uint32_t signal_type_master_from_slave;

    // TODO Clean: uint32_t signal_id_to_master;     /* ID of the signal from slave to master - master is waiting signal */
    // TODO Clean uint32_t signal_type_to_master;   /* Type of the signal from slave to master  */

        /*
         * Signal configuration in slave side
         */
    uint32_t signal_id_slave_to_master;             /* Slave is raising the signal */
    uint32_t signal_type_slave_to_master;

    uint32_t signal_id_slave_from_master;           /* Slave is waiting the signal */
    uint32_t signal_type_slave_from_master;

    // TODO Clean uint32_t signal_id_to_slave;      /* ID of the signal from master to slave - slave is waiting signal */
    // TODO Clean uint32_t signal_type_to_slave;    /* Type of the signal from master to slave */

    uint8_t  priority;

} smc_instance_conf_channel_t;


typedef struct _smc_instance_conf_t
{
    char*    name;                  /* Unique name of the configuration */
    char*    user_name;             /* Name of the SMC user (e.g. L2MUX) */
    char*    master_name;           /* Master CPU name */
    char*    slave_name;            /* Slave CPU name */

    uint32_t shm_start_address;
    uint32_t shm_size;

        /* Share memory configuration */
    uint8_t  shm_use_cache_control_master;  /* Master cache control */
    uint8_t  shm_use_cache_control_slave;   /* Slave cache control */

    uint8_t  shm_cache_line_len_master;
    uint8_t  shm_cache_line_len_slave;

    SMC_SHM_OFFSET_TYPE shm_memory_offset_type_master_to_slave; /* Offset type in master point of view */
    uint32_t shm_cpu_memory_offset;                             /* Offset between master and slave */

    uint8_t  channel_config_count;
    smc_instance_conf_channel_t* channel_config_array;

} smc_instance_conf_t;


/*
 * SMC instance configuration API functions.
 */
smc_conf_t*          smc_conf_create( void );
void                 smc_conf_destroy( smc_conf_t* smc_conf );

/*
 * SMC Channel configuration API functions.
 */
smc_channel_conf_t*  smc_channel_conf_create( void );

void                 smc_channel_conf_destroy( smc_channel_conf_t* smc_channel_conf);
void                 smc_conf_add_channel_conf(smc_conf_t* smc_conf, smc_channel_conf_t* smc_channel_conf);

smc_conf_t*          smc_conf_create_from_instance_conf( char* smc_cpu_name, smc_instance_conf_t* smc_instance_conf);
smc_channel_conf_t*  smc_channel_conf_create_from_instance_conf( smc_instance_conf_channel_t* smc_instance_conf_channel, uint8_t is_master );

smc_instance_conf_t* smc_instance_conf_get_from_list(smc_instance_conf_t* smc_instance_conf_array, int config_count, char* smc_user_name, char* smc_name);

#endif /* EOF */

