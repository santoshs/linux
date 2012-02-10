/*
*   Common SMC instance configuration for L2MUX
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

Version:       1    11-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_INSTANCE_CONFIG_L2MUX_H
#define SMC_INSTANCE_CONFIG_L2MUX_H

#define SMC_CONFIG_USER_L2MUX                        "L2MUX"

#define SMC_CONFIG_NAME_EOS2_WAKEUP                  "EOS2-Wakeup-SH-Mobile-APE5R-WGEModem 3.1 for L2MUX"


#define SMC_CONF_COUNT_L2MUX                         1

#define SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2_WAKEUP     3


/**
 * SMC instance configurations for L2MUX
 */

static smc_instance_conf_channel_t smc_instance_conf_l2mux_channels[SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2_WAKEUP] =
{
    {
            .name                = "ETH_P_PHONET",
            .protocol            = 0x00,
            .fifo_size_master    = 30,
            .fifo_size_slave     = 30,
            .mdb_size_master     = 1024*512,
            .mdb_size_slave      = 1024*512,

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

            .priority            = SMC_CHANNEL_PRIORITY_HIGHEST
     },
     {
             .name                = "ETH_P_MHI",
             .protocol            = 0x00,
             .fifo_size_master    = 30,
             .fifo_size_slave     = 30,
             .mdb_size_master     = 1024*512,
             .mdb_size_slave      = 1024*512,

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

             .priority            = SMC_CHANNEL_PRIORITY_DEFAULT
     },

     {
             .name                = "ETH_P_MHDP",
             .protocol            = 0x00,
             .fifo_size_master    = 30,
             .fifo_size_slave     = 30,
             .mdb_size_master     = 1024*512,
             .mdb_size_slave      = 1024*512,

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

             .priority            = SMC_CHANNEL_PRIORITY_LOWEST
     }
};

static smc_instance_conf_t smc_instance_conf_l2mux[SMC_CONF_COUNT_L2MUX] =
{
    /**
     * SMC instance config for EOS2 L2MUX between APE5R SH-Mobile and WGEModem3.1
     *
     */
    {
        .name                         = SMC_CONFIG_NAME_EOS2_WAKEUP,
        .user_name                    = SMC_CONFIG_USER_L2MUX,
        .master_name                  = SMC_CONFIG_MASTER_NAME_SH_MOBILE_APE5R_EOS2,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2,
        .shm_start_address            = SMC_CONF_L2MUX_SHM_START,
        .shm_size                     = (1024*1024*3 + 1024*512),    /* 3.5MB */
        .shm_use_cache_control_master = FALSE,
        .shm_use_cache_control_slave  = FALSE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_EQUAL,
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_L2MUX_EOS2_WAKEUP,
        .channel_config_array         = smc_instance_conf_l2mux_channels,
    }
};

smc_instance_conf_t* smc_instance_conf_get_l2mux( char* smc_user_name, char* config_name );


#endif
