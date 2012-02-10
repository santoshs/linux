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

#ifndef SMC_INSTANCE_CONFIG_CONTROL_H
#define SMC_INSTANCE_CONFIG_CONTROL_H

#define SMC_CONFIG_USER_CONTROL                      "SMC_CONTROL"
#define SMC_CONFIG_NAME_EOS2                         "EOS2-SH-Mobile-APE5R-WGEModem 3.1 for SMC control"

/* ===========================================================================
 * SHM Configuration is based on memory mapping
 * TODO ---- Common SHM area config for products
 *
 * SHM Address    Modem Address   Owner
 * -------------------------------------------------------------------------
 * 0x4000_0000    0x0800_0000     L23 Code
 * 0x4100_0000    0x0900_0000     L1  Code
 * 0x4200_0000    0x0A00_0000     L23 Data
 * 0x4300_0000    0x0B00_0000     L1  Data
 * 0x43B0_0000    0x0BB0_0000     Shared Memory starts
 * 0x4400_0000    0xC400_0000     Linux SW
 *
 */


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

            .signal_id_master_from_slave   = 193,             /* WGM_Modem_interrupts[0] --> INTC-SYS, SPI 193 == Modem IntGen OUT 0 */
            .signal_type_master_from_slave = 0x03000006,      /* SMC_SIGNAL_TYPE_INT_RESOURCE         */

                /*
                 * Slave side signal configuration
                 */
            .signal_id_slave_to_master     = 0,              /* WGM_Modem_interrupts[0] --> INTC-SYS, Modem IntGen OUT 0 == SPI 193  */
            .signal_type_slave_to_master   = 0x03000005,     /* SMC_SIGNAL_TYPE_INTGEN_OUT    */

            .signal_id_slave_from_master   = 46,             /* INTGEN C2_L2_CPU_Int_Gen_Ch11 */
            .signal_type_slave_from_master = 0x03000001,     /* SMC_SIGNAL_TYPE_INTGEN */

            .priority            = SMC_CHANNEL_PRIORITY_HIGHEST
     }
};

static smc_instance_conf_t smc_instance_conf_control[SMC_CONF_COUNT_CONTROL] =
{
    /**
     * SMC instance config for EOS2 SMC control instance between APE5R SH-Mobile and WGEModem3.1
     *
     */
    {
        .name                         = SMC_CONFIG_NAME_EOS2,
        .user_name                    = SMC_CONFIG_USER_CONTROL,
        .master_name                  = SMC_CONFIG_MASTER_NAME_SH_MOBILE_APE5R_EOS2,
        .slave_name                   = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2,
        .shm_start_address            = SMC_CONF_CONTROL_SHM_START,
        .shm_size                     = 1024*200,    /* 200kB */
        .shm_use_cache_control_master = FALSE,
        .shm_use_cache_control_slave  = FALSE,
        .shm_memory_offset_type_master_to_slave = SMC_SHM_OFFSET_MDB_OFFSET,    /* Offset is transfered not pointer */
        .shm_cpu_memory_offset        = 0,

        .channel_config_count         = SMC_CONF_CHANNEL_COUNT_CONTROL,
        .channel_config_array         = smc_instance_conf_control_channels,
    }
};

smc_instance_conf_t* smc_instance_conf_get_control( char* smc_user_name, char* config_name );


#endif
