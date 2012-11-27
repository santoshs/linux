/*
*   Common SMC configuration for L2MUX:
*   This configuration file is for SMC between EOS2 devices APE R8A73734 and Modem WGE3.1
*
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

Version:       1    27-Sep-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_INSTANCE_CONFIG_L2MUX_H
#define SMC_INSTANCE_CONFIG_L2MUX_H

#define SMC_CONFIG_NAME_EOS2_ES10                    "EOS2-ES10-SH-Mobile-R8A7374-WGEModem 3.1 for L2MUX"
#define SMC_CONFIG_NAME_EOS2_ES20                    "EOS2-ES20-SH-Mobile-R8A7374-WGEModem 3.1 for L2MUX"


#define SMC_CONF_COUNT_L2MUX                         2
#define SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2            3
#define SMC_L2MUX_QUEUE_COUNT                        3



/**
 * SMC instance configurations for L2MUX
 */

static smc_instance_conf_channel_t smc_instance_conf_l2mux_channels[SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2] =
{
    {
            .name                = "ETH_P_PHONET",
            .protocol            = SMC_L2MUX_QUEUE_1_PHONET,
            .fifo_size_master    = 30,
            .fifo_size_slave     = 30,
            .mdb_size_master     = 1024*256,
            .mdb_size_slave      = 1024*256,

                /*
                 * Master side signal configuration
                 */
            .signal_id_master_to_slave     = 43,             /* INTGEN C2_L2_CPU_Int_Gen_Ch8 */
            .signal_type_master_to_slave   = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

            .signal_id_master_from_slave   = 194,             /* WGM_Modem_interrupts[1] --> INTC-SYS, SPI 194 == Modem IntGen OUT 1 */
            .signal_type_master_from_slave = 0x03000006,      /* SMC_SIGNAL_TYPE_INT_RESOURCE         */

                /*
                 * Slave side signal configuration
                 */
            .signal_id_slave_to_master     = 1,              /* WGM_Modem_interrupts[1] --> INTC-SYS, Modem IntGen OUT 1 == SPI 194  */
            .signal_type_slave_to_master   = 0x03000005,     /* SMC_SIGNAL_TYPE_INTGEN_OUT    */

            .signal_id_slave_from_master   = 43,             /* INTGEN C2_L2_CPU_Int_Gen_Ch8 */
            .signal_type_slave_from_master = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

            .priority                      = SMC_CHANNEL_PRIORITY_HIGHEST,
            .copy_scheme_master            = (SMC_COPY_SCHEME_COPY_IN_SEND),        /* No copy in Kernel receive --> directly to SKB */
            .copy_scheme_slave             = (SMC_COPY_SCHEME_COPY_IN_SEND+SMC_COPY_SCHEME_COPY_IN_RECEIVE),

            .fifo_full_check_timeout_usec_master = 1000,    /* Linux kernel timer supports only min 1ms timer */
            .fifo_full_check_timeout_usec_slave  = 500,
            .trace_features_master               = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
            .trace_features_slave                = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
            .wake_lock_flags_master              = SMC_CHANNEL_WAKELOCK_TIMER,
            .wake_lock_flags_slave               = SMC_CHANNEL_WAKELOCK_NONE,
     },
     {
             .name                = "ETH_P_MHI",
             .protocol            = SMC_L2MUX_QUEUE_2_MHI,
             .fifo_size_master    = 30,
             .fifo_size_slave     = 30,
             .mdb_size_master     = 1024*256,
             .mdb_size_slave      = 1024*256,

                 /*
                  * Master side signal configuration
                  */
             .signal_id_master_to_slave     = 44,             /* INTGEN C2_L2_CPU_Int_Gen_Ch9 */
             .signal_type_master_to_slave   = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

             .signal_id_master_from_slave   = 195,             /* WGM_Modem_interrupts[2] --> INTC-SYS, SPI 195 == Modem IntGen OUT 2 */
             .signal_type_master_from_slave = 0x03000006,      /* SMC_SIGNAL_TYPE_INT_RESOURCE         */

                 /*
                  * Slave side signal configuration
                  */
             .signal_id_slave_to_master     = 2,              /* WGM_Modem_interrupts[2] --> INTC-SYS, Modem IntGen OUT 2 == SPI 195 */
             .signal_type_slave_to_master   = 0x03000005,     /* SMC_SIGNAL_TYPE_INTGEN_OUT    */

             .signal_id_slave_from_master   = 44,             /* INTGEN C2_L2_CPU_Int_Gen_Ch9 */
             .signal_type_slave_from_master = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

             .priority                      = SMC_CHANNEL_PRIORITY_DEFAULT,
             .copy_scheme_master            = (SMC_COPY_SCHEME_COPY_IN_SEND),        /* No copy in Kernel receive --> directly to SKB */
             .copy_scheme_slave             = (SMC_COPY_SCHEME_COPY_IN_SEND+SMC_COPY_SCHEME_COPY_IN_RECEIVE),
             .fifo_full_check_timeout_usec_master = 1000,    /* Linux kernel timer supports only min 1ms timer */
             .fifo_full_check_timeout_usec_slave  = 500,
             .trace_features_master               = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
             .trace_features_slave                = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
             .wake_lock_flags_master              = SMC_CHANNEL_WAKELOCK_TIMER,
             .wake_lock_flags_slave               = SMC_CHANNEL_WAKELOCK_NONE,
     },

     {
             .name                = "ETH_P_MHDP",
             .protocol            = SMC_L2MUX_QUEUE_3_MHDP,


             .fifo_size_master    = 300,
             .fifo_size_slave     = 400,
             .mdb_size_master     = 1024*1600,          /* Master to slave MDB (Typically UL) */
             .mdb_size_slave      = 1024*1600,          /* Slave to master MDB (Typically DL) */

             /* New MHDP channel configuration from after sw 12w45 (approximately)
              * To support 100 x 16kB burst of data
             .fifo_size_master    = 200,
             .fifo_size_slave     = 200,
             .mdb_size_master     = 1024*1600,
             .mdb_size_slave      = 1024*1600,
             */

                 /*
                  * Master side signal configuration
                  */
             .signal_id_master_to_slave     = 45,             /* INTGEN C2_L2_CPU_Int_Gen_Ch10 */
             .signal_type_master_to_slave   = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

             .signal_id_master_from_slave   = 196,             /* WGM_Modem_interrupts[3] --> INTC-SYS, SPI 196 == Modem IntGen OUT 3*/
             .signal_type_master_from_slave = 0x03000006,      /* SMC_SIGNAL_TYPE_INT_RESOURCE         */

                 /*
                  * Slave side signal configuration
                  */
             .signal_id_slave_to_master     = 3,              /* WGM_Modem_interrupts[3] --> INTC-SYS, Modem IntGen OUT 3 == SPI 196 */
             .signal_type_slave_to_master   = 0x03000005,     /* SMC_SIGNAL_TYPE_INTGEN_OUT    */

             .signal_id_slave_from_master   = 45,             /* INTGEN C2_L2_CPU_Int_Gen_Ch10 */
             .signal_type_slave_from_master = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

             .priority                      = SMC_CHANNEL_PRIORITY_LOWEST,           /*  */
             .copy_scheme_master            = (SMC_COPY_SCHEME_COPY_IN_SEND),        /* No copy in Kernel receive --> directly to SKB */
             .copy_scheme_slave             = (SMC_COPY_SCHEME_COPY_IN_SEND),         /* No copy in Modem receive in L2_PRIORITY_LTE channel (delayed allocation)*/
             .fifo_full_check_timeout_usec_master = 1000,    /* Linux kernel timer supports only min 1ms timer */
             .fifo_full_check_timeout_usec_slave  = 500,
             .trace_features_master               = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
             .trace_features_slave                = (SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND+SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE),
             .wake_lock_flags_master              = SMC_CHANNEL_WAKELOCK_TIMER,
             .wake_lock_flags_slave               = SMC_CHANNEL_WAKELOCK_NONE,
     }
};

static smc_instance_conf_t smc_instance_conf_l2mux[SMC_CONF_COUNT_L2MUX] =
{
    /**
     * SMC instance config for EOS2 L2MUX between APE5R SH-Mobile and WGEModem3.1
     *
     */
    {
        .name                         = SMC_CONFIG_NAME_EOS2_ES10,
        .user_name                    = SMC_CONFIG_USER_L2MUX,
        .master_name                  = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES10,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES10,

        .master_cpu_version_major     = 1,
        .master_cpu_version_minor     = 0,
        .slave_cpu_version_major      = 1,
        .slave_cpu_version_minor      = 0,

        .shm_start_address            = SMC_CONF_L2MUX_SHM_START_ES10,
        .shm_size                     = SMC_CONF_L2MUX_SHM_SIZE_ES10,
        .shm_use_cache_control_master = TRUE,
        .shm_use_cache_control_slave  = TRUE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_MDB_OFFSET,
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2,
        .channel_config_array         = smc_instance_conf_l2mux_channels,
    },

    {
        .name                         = SMC_CONFIG_NAME_EOS2_ES20,
        .user_name                    = SMC_CONFIG_USER_L2MUX,
        .master_name                  = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES20,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES20,

        .master_cpu_version_major     = 2,
        .master_cpu_version_minor     = 0,
        .slave_cpu_version_major      = 2,
        .slave_cpu_version_minor      = 0,

        .shm_start_address            = SMC_CONF_L2MUX_SHM_START_ES20,
        .shm_size                     = SMC_CONF_L2MUX_SHM_SIZE_ES20,
        .shm_use_cache_control_master = TRUE,
        .shm_use_cache_control_slave  = TRUE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_MDB_OFFSET,
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2,
        .channel_config_array         = smc_instance_conf_l2mux_channels,
    }
};

#endif
