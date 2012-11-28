/*
*   Common SMC instance configuration for SMC control channel
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

Version:       1    25-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_CONFIG_CONTROL_R8A73734_WGE31_H
#define SMC_CONFIG_CONTROL_R8A73734_WGE31_H


/*
 * EOS2 Contains two different configurations for ES1.0 and ES2.0
 * NOTE: The channel configuration are currently the same
 */

#define SMC_CONFIG_NAME_EOS2_ES10                    "EOS2-ES10-SH-Mobile-R8A73734-WGEModem 3.1 for SMC control"
#define SMC_CONFIG_NAME_EOS2_ES20                    "EOS2-ES20-SH-Mobile-R8A73734-WGEModem 3.1 for SMC control"


#define SMC_CONF_COUNT_CONTROL          2

#define SMC_CONF_CHANNEL_COUNT_CONTROL  1


/**
 * SMC instance configurations for SMC control instance.
 */

static smc_instance_conf_channel_t smc_instance_conf_control_channels[SMC_CONF_CHANNEL_COUNT_CONTROL] =
{
    {
            .name                = "CTRL1",
            .protocol            = 0x00,
            .fifo_size_master    = 30,      /*  */
            .fifo_size_slave     = 30,
            .mdb_size_master     = 1024*50,
            .mdb_size_slave      = 1024*50,

                /*
                 * Master side signal configuration
                 */
            .signal_id_master_to_slave     = 46,             /* INTGEN C2_L2_CPU_Int_Gen_Ch11 */
            .signal_type_master_to_slave   = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

            .signal_id_master_from_slave   = 193,             /* WGM_Modem_interrupts[0] --> INTC-SYS, SPI 193 == Modem IntGen OUT 0 */
            .signal_type_master_from_slave = 0x03000006,      /* SMC_SIGNAL_TYPE_INT_RESOURCE         */

                /*
                 * Slave side signal configuration
                 */
            .signal_id_slave_to_master     = 0,              /* WGM_Modem_interrupts[0] --> INTC-SYS, Modem IntGen OUT 0 == SPI 193  */
            .signal_type_slave_to_master   = 0x03000005,     /* SMC_SIGNAL_TYPE_INTGEN_OUT    */

            .signal_id_slave_from_master   = 46,             /* INTGEN C2_L2_CPU_Int_Gen_Ch11 */
            .signal_type_slave_from_master = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

            .priority                            = SMC_CHANNEL_PRIORITY_HIGHEST,
            .copy_scheme_master                  = (SMC_COPY_SCHEME_COPY_IN_SEND),        /* No copy in Kernel receive --> directly to SKB */
            .copy_scheme_slave                   = (SMC_COPY_SCHEME_COPY_IN_SEND+SMC_COPY_SCHEME_COPY_IN_RECEIVE),

            .fifo_full_check_timeout_usec_master = 1000,    /* Linux kernel timer supports only min 1ms timer */
            .fifo_full_check_timeout_usec_slave  = 500,

            .trace_features_master               = SMC_TRACE_HISTORY_DATA_TYPE_NONE,
            .trace_features_slave                = SMC_TRACE_HISTORY_DATA_TYPE_NONE,
            .wake_lock_flags_master              = SMC_CHANNEL_WAKELOCK_NONE,
            .wake_lock_flags_slave               = SMC_CHANNEL_WAKELOCK_NONE,
     }
};

static smc_instance_conf_t smc_instance_conf_control[SMC_CONF_COUNT_CONTROL] =
{
    /**
     * SMC instance config for EOS2 SMC control instance between APE5R SH-Mobile and WGEModem3.1
     * ES1.0/ES2.0 configurations
     */

    /* ES1.0 configuration */
    {
        .name                         = SMC_CONFIG_NAME_EOS2_ES10,
        .user_name                    = SMC_CONFIG_USER_CONTROL,
        .master_name                  = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES10,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES10,

        .master_cpu_version_major     = 1,
        .master_cpu_version_minor     = 0,
        .slave_cpu_version_major      = 1,
        .slave_cpu_version_minor      = 0,

        .shm_start_address            = SMC_CONF_CONTROL_SHM_START_ES10,
        .shm_size                     = SMC_CONF_CONTROL_SHM_SIZE_ES10,
        .shm_use_cache_control_master = FALSE,
        .shm_use_cache_control_slave  = TRUE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_MDB_OFFSET,    /* Data location is transferred as an offset not as a pointer */
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_CONTROL,
        .channel_config_array         = smc_instance_conf_control_channels,
    },

    /* ES2.0 configuration */
    {
        .name                         = SMC_CONFIG_NAME_EOS2_ES20,
        .user_name                    = SMC_CONFIG_USER_CONTROL,
        .master_name                  = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES20,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES20,

        .master_cpu_version_major     = 2,
        .master_cpu_version_minor     = 0,
        .slave_cpu_version_major      = 2,
        .slave_cpu_version_minor      = 0,

        .shm_start_address            = SMC_CONF_CONTROL_SHM_START_ES20,
        .shm_size                     = SMC_CONF_CONTROL_SHM_SIZE_ES20,
        .shm_use_cache_control_master = FALSE,
        .shm_use_cache_control_slave  = TRUE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_MDB_OFFSET,    /* Data location is transferred as an offset not as a pointer */
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_CONTROL,
        .channel_config_array         = smc_instance_conf_control_channels,
    }
};


#endif
