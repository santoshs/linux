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

Version:       10   06-Jun-2012     Heikki Siikaluoma
Status:        draft
Description :  New SHM SDRAM memory mapping for the EOS2 ES2.0

Version:       1    03-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_INSTANCE_CONFIG_R8A73734_WGE31_H
#define SMC_INSTANCE_CONFIG_R8A73734_WGE31_H

#ifdef SMECO_MODEM
  #define SMC_CPU_NAME    "Modem"
#elif( defined(SMECO_LINUX_KERNEL) )
  #define SMC_CPU_NAME    "APE"
#else
  #error "Error in EOS2 configuration"
#endif

#define SMC_EOS_ASIC_ES20   0x20


    /* =============================================================================
     * SMC specific configurations for EOS2 ES2.0
     */

    /* ---- ES2.0 ---- */
#define SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES20  "SH-Mobile-R8A73734-EOS2-ES20"
#define SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES20         "WGEModem-3.1-EOS2-ES20"

//#define SMC_CONF_GLOBAL_SHM_START_ES20                       0x43B00000
//#define SMC_CONF_GLOBAL_SHM_END_ES20                         0x43FFFFFF

#define SMC_CONF_GLOBAL_SHM_START_ES20                       0x45001000
#define SMC_CONF_GLOBAL_SHM_END_ES20                         0x457FFBFF     /* 7FFFFF -> 8192 kB -> 8 MB, NOTE: Use always the last address */

    /* SHM area for SMC Control Instance */
#define SMC_CONF_CONTROL_SHM_START_OFFSET_ES20               (0)
#define SMC_CONF_CONTROL_SHM_START_ES20                      (SMC_CONF_GLOBAL_SHM_START_ES20 + SMC_CONF_CONTROL_SHM_START_OFFSET_ES20)
#define SMC_CONF_CONTROL_SHM_SIZE_ES20                       (1024*200)                  /* 200kB */

    /* SHM Area for L2MUX */
#define SMC_CONF_L2MUX_SHM_START_OFFSET_ES20                 (SMC_CONF_CONTROL_SHM_SIZE_ES20 + 64)
#define SMC_CONF_L2MUX_SHM_START_ES20                        (SMC_CONF_GLOBAL_SHM_START_ES20 + SMC_CONF_L2MUX_SHM_START_OFFSET_ES20)
#define SMC_CONF_L2MUX_SHM_SIZE_ES20                         (1024*1856*2 + 1024*256*4 + 1024*30)    /* 4.5MB */


#define SMC_CHANNEL_ID_FOR_CONFIGURATION                     0   /* Channel to used for configure others (FIFO, SHM), if not defined, no config.
                                                                    FIFO+SHM configuration of this channel is not allowed to change without parallel changes to the other device
                                                                    Currently only first channel (0) is supported.
                                                                    The master (APE) only initiate the runtime configuration and its config is used
                                                                  */

     /*
      *	 SMC configuration names
      */

    /* ===========================================================================
     * SHM Configuration is based on memory mapping for ES20
     *
     *
     * SHM Address    Modem Address   Owner
     * -------------------------------------------------------------------------
     * 0x4000_0000    0x0800_0000     L23 Code
     * 0x4100_0000    0x0900_0000     L1  Code
     * 0x4200_0000    0x0A00_0000     L23 Data/L1 Data
     * 0x43FF_0000                    Stack space       to 0x4400_0000 size 64 kB
     * 0x4400_0000    0x0C00_0000     Reserved          to 0x477F_FFFF size 56 MB
     *    0x4400_0000                 Malloc space      to 0x4500_0000 size 16 MB
     *
     * 0x4800_0000    -----           Modem is not able to the area starting from this address
     * 0x4C00_0000    0x1400_0000     Non-Secure Spinlock
     * 0x4C00_1000    0x1400_1000     Shared Memory 0x4C80_0FFF
     * 0x4C80_1000    0x1480_1000     Crash log
     * 0x4C90_1000    0x1490_1000     STM Trace buffer
     *
     * In Linux side the SHM is ioremapped from physical address (no static value possible)
     */

    //#define SMC_CONF_GLOBAL_SHM_START       0x4C001000

    /* =============================================================================
     * COMMON Configuration for EOS2 ES2.0
     */

#define SMC_CONF_PM_APE_HOST_ACCESS_REQ_ENABLED     TRUE

    /* Modem side offset */
#define SMC_CONF_SHM_OFFSET_TO_MODEM                (-1*0x38000000)

/* =================================================
 * Peripheral addresses and configurations
 *
 */


    /*
     * Modem side peripherals
     */
#define SMC_MODEM_INTGEN_L2_OFFSET                   16
#define SMC_MODEM_INTGEN_L2_FIRST                    35          /* C2_L2_CPU_Int_Gen_Ch0 */
#define SMC_PERIPHERAL_ADDRESS_MODEM_GOP_INTGEN_1    0x07C00040


    /*
     * APE side peripherals
     */
#define SMC_ADDRESS_APE_OFFSET_TO_MODEM              0xDC000000

#define SMC_APE_IRQ_OFFSET_INTCSYS_SPI               32         /* INTC-SYS SPI ID starts from 32 -> 255 */
#define SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM            193        /* TODO Valid only with SPI 193-198 (NOT 221 and 222)*/

#define SMC_APE_IRQ_OFFSET_IRQ_SPI                   512        /* APE IRQ offset (#define IRQPIN_IRQ_BASE 512 in irqs.h )*/


#define SMC_WPMCIF_EPMU_BASE                         0xE6190000
#define SMC_WPMCIF_EPMU_ACC_CR                       (SMC_WPMCIF_EPMU_BASE + 0x0004)    /* Used for checking that modem is up */
    /* Register values of the EPMU ACC CR */
#define SMC_WPMCIF_EPMU_ACC_CR_MODEM_SLEEP_REQ       0x00000000
#define SMC_WPMCIF_EPMU_ACC_CR_MODEM_ACCESS_REQ      0x00000002
#define SMC_WPMCIF_EPMU_ACC_CR_MODEM_ACCESS_OK       0x00000003

#define SMC_HPB_BASE                                 0xE6000000                         /* Used to retrieve EOS2 ASIC version */
#define SMC_CCCR                                     (SMC_HPB_BASE + 0x101C)

#if( SMC_RUNTIME_TRACES_ENABLED == TRUE )
#define SMC_APE_RDTRACE_ENABLED         /* If defined, APE handles runtime trace activation for modem stype rdtraces */
#else
#undef SMC_APE_RDTRACE_ENABLED
#endif

#define SMC_APE_WAKEUP_WAKELOCK_USE                             /* If defined, the APE uses wakelock, otherwise there is no wakelocks at all */
#define SMC_APE_WAKEUP_WAKELOCK_USE_TIMER                       /* If defined, the APE uses timer in wakelock while receiving packets from modem */
#define SMC_APE_WAKEUP_WAKELOCK_TIMEOUT_MSEC           400      /* Wakelock timeout in milliseconds in APE IRQ (old configs: 2000/200) */

#define SMC_WAKEUP_USE_EXTERNAL_IRQ_MODEM                       /* If defined uses ext irq in modem side (modem wakes APE)*/

/* #define SMC_WAKEUP_USE_EXTERNAL_IRQ_APE */                   /* If defined the APE SMC initializes the WUP signal (modem wakes APE)*/

#if( defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_APE ) || defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_MODEM ) )

        /* Configuration for Modem and APE*/
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_ID                65                                /* IRQ ID in the APE side */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_TYPE              SMC_SIGNAL_TYPE_INT_WGM_GENOUT
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_SENSE_DEFAULT     SMC_SIGNAL_SENSE_RISING_EDGE      /* Default value if the configuration negotiation fails */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_SENSE             SMC_SIGNAL_SENSE_FALLING_EDGE     /* SMC_SIGNAL_SENSE_FALLING_EDGE / SMC_SIGNAL_SENSE_RISING_EDGE */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_ID_FROM_MODEM     0                                 /* Modem side interrupt genio to wakeup APE */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_REGISTER_HANDLER  FALSE


    #define SMC_VERSION_REQUIREMENT_EXTERNAL_IRQ_POLARITY  { SMC_VERSION_TO_INT(0,0,36), SMC_VERSION_REQUIREMENT_LEVEL_WARNING, "Modem->APE wakeup signal polarity change" }
    #define SMC_VERSION_REQUIREMENT_MHDP_SHM_CHANGE_0_0_37 { SMC_VERSION_TO_INT(0,0,37), SMC_VERSION_REQUIREMENT_LEVEL_ASSERT, "MHDP Channel Shared Memory size change" }

#else
    #define SMC_VERSION_REQUIREMENT_EXTERNAL_IRQ_POLARITY  {0,0,""}

#endif /* #if( defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_APE ) || defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_MODEM ) ) */



    /*
     * SMC Version requirements
     */
#define SMC_VERSION_REQUIREMENT_ARRAY  SMC_VERSION_REQUIREMENT_EXTERNAL_IRQ_POLARITY, SMC_VERSION_REQUIREMENT_MHDP_SHM_CHANGE_0_0_37
#define SMC_VERSION_REQUIREMENT_COUNT  2

/* #define SMC_BUFFER_MESSAGE_OUT_OF_MDB_MEM */      /* If defined, the message is buffered when out of MDB memory */


/* ======================================
 * Target specific product configurations
 */
#ifdef SMECO_MODEM
    /*
     * MODEM specific configuration
     */

  /* #define SMC_BUFFER_MESSAGE_OUT_OF_FIFO_ITEMS */       /* If defined, the message is buffered when FIFO is full */

#elif( defined(SMECO_LINUX_KERNEL) )
    /*
     * APE Linux Kernel specific configuration
     */

  #define SMC_BUFFER_MESSAGE_OUT_OF_FIFO_ITEMS        /* If defined, the message is buffered when FIFO is full */

  #define SMC_SUPPORT_SKB_FRAGMENT_UL                 /* If defined, the SKB fragments are supported in uplink. NOTE: enable also the SMC_DMA_TRANSFER_ENABLED with this for highmem config */
  #define SMC_DMA_TRANSFER_ENABLED                    /* If defined, the DMA transfer feature is enabled for channel configuration  */

#endif  /* End of target specific configuration */

#endif  /* #ifndef SMC_INSTANCE_CONFIG_R8A73734_WGE31_H */

/* EOF */
