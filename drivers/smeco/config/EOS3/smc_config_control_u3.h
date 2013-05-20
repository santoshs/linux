/*
*   Common SMC instance configuration for SMC control channel
*   Copyright © Renesas Mobile Corporation 2013. All rights reserved
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

Version:       1    13-Feb-2013     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_CONFIG_CONTROL_EOS3_U3_H
#define SMC_CONFIG_CONTROL_EOS3_U3_H


/*
 * EOS3 configurations for U3 ES1
 * NOTE: The channel configuration are currently the same
 */

#define SMC_CONFIG_NAME_EOS3_U3_ES1                    "EOS3-U3-ES1-WGEModem31-SMC-control"

#define SMC_CONF_COUNT_CONTROL          1

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
            /*.signal_type_master_to_slave   = 0x03000000,*/       /* SMC_SIGNAL_TYPE_IRQ_NONE */


            .signal_id_master_from_slave   = 193,             /* WGM_Modem_interrupts[0] --> INTC-SYS, SPI 193 == Modem IntGen OUT 0 */
            .signal_type_master_from_slave = 0x03000006,      /* SMC_SIGNAL_TYPE_INT_RESOURCE         */
            /*.signal_type_master_from_slave = 0x03000000,*/       /* SMC_SIGNAL_TYPE_IRQ_NONE */

                /*
                 * Slave side signal configuration
                 */
            .signal_id_slave_to_master     = 0,              /* WGM_Modem_interrupts[0] --> INTC-SYS, Modem IntGen OUT 0 == SPI 193  */
            .signal_type_slave_to_master   = 0x03000005,     /* SMC_SIGNAL_TYPE_INTGEN_OUT    */
            /*.signal_type_slave_to_master   = 0x03000000,*/       /* SMC_SIGNAL_TYPE_IRQ_NONE */

            .signal_id_slave_from_master   = 46,             /* INTGEN C2_L2_CPU_Int_Gen_Ch11 */
            .signal_type_slave_from_master = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */
            /*.signal_type_slave_from_master = 0x03000000,*/       /* SMC_SIGNAL_TYPE_IRQ_NONESMC_SIGNAL_TYPE_IRQ_NONE */

            .priority                            = SMC_CHANNEL_PRIORITY_HIGHEST,
            .copy_scheme_master                  = (SMC_COPY_SCHEME_COPY_IN_SEND),        /* No copy in Kernel receive --> directly to SKB */
            .copy_scheme_slave                   = (SMC_COPY_SCHEME_COPY_IN_SEND+SMC_COPY_SCHEME_COPY_IN_RECEIVE),

            .fifo_full_check_timeout_usec_master = 1000,    /* Linux kernel timer supports only min 1ms timer */
            .fifo_full_check_timeout_usec_slave  = 500,

            .trace_features_master               = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
            .trace_features_slave                = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
            .wake_lock_flags_master              = SMC_CHANNEL_WAKELOCK_NONE,
            .wake_lock_flags_slave               = SMC_CHANNEL_WAKELOCK_NONE,
     }
};

static smc_instance_conf_t smc_instance_conf_control[SMC_CONF_COUNT_CONTROL] =
{
    /**
     * SMC instance config for EOS3 U3 SMC control instance
     */

    /* ES1 configuration */
    {
        .name                         = SMC_CONFIG_NAME_EOS3_U3_ES1,
        .user_name                    = SMC_CONFIG_USER_CONTROL,
        .master_name                  = SMC_CONFIG_MASTER_NAME_APE_EOS3_U3_ES1,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS3_ES1,

        .master_cpu_version_major     = 1,
        .master_cpu_version_minor     = 0,
        .slave_cpu_version_major      = 1,
        .slave_cpu_version_minor      = 0,

        .shm_start_address            = SMC_CONF_CONTROL_SHM_START_ES1,
        .shm_size                     = SMC_CONF_CONTROL_SHM_SIZE_ES1,
        .shm_use_cache_control_master = TRUE,
        .shm_use_cache_control_slave  = TRUE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_MDB_OFFSET,    /* Data location is transferred as an offset not as a pointer */
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_CONTROL,
        .channel_config_array         = smc_instance_conf_control_channels,
    }
};


#endif
