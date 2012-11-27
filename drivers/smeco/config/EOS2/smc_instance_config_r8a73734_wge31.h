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

/*
 * Select between EOS2 ES10/ES20
 * There is different flagging between APE and Modem
 *
 */

#ifdef SMECO_MODEM

  #define SMC_CPU_NAME    "Modem"

    /* Modem ES10/ES20 definitions from makefile */

  #if defined EOS2_ASIC && (EOS2_ASIC == EOS2_ASIC_ES10)
      #define SMC_IN_EOS2_ES10
  #else
      #undef SMC_IN_EOS2_ES10
  #endif

#elif( defined(SMECO_LINUX_KERNEL) )

  #define SMC_CPU_NAME    "APE"


  /* APE Linux Kernel ES10/ES20 definitions from kernel config + makefile */

  #ifdef SMC_CONFIG_USE_EOS2_ES10
      #define SMC_IN_EOS2_ES10
  #else
      #undef SMC_IN_EOS2_ES10
  #endif

#else
  #error "Error in EOS2 configuration"
#endif


#define SMC_EOS_ASIC_ES10   0x10
#define SMC_EOS_ASIC_ES20   0x20


    /* =============================================================================
     * SMC specific configurations for EOS2 ES1.0/ES2.0
     */

    /* ---- ES1.0 ---- */
#define SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES10  "SH-Mobile-R8A73734-EOS2-ES10"
#define SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES10         "WGEModem-3.1-EOS2-ES10"

#define SMC_CONF_GLOBAL_SHM_START_ES10                       0x43B00000
    /* SHM area for SMC Control Instance */
#define SMC_CONF_CONTROL_SHM_START_OFFSET_ES10               (0)
#define SMC_CONF_CONTROL_SHM_START_ES10                      (SMC_CONF_GLOBAL_SHM_START_ES10 + SMC_CONF_CONTROL_SHM_START_OFFSET_ES10)
#define SMC_CONF_CONTROL_SHM_SIZE_ES10                       (1024*200)                  /* 200kB */
    /* SHM Area for L2MUX */
#define SMC_CONF_L2MUX_SHM_START_OFFSET_ES10                 (SMC_CONF_CONTROL_SHM_SIZE_ES10 + 64)
#define SMC_CONF_L2MUX_SHM_START_ES10                        (SMC_CONF_GLOBAL_SHM_START_ES10 + SMC_CONF_L2MUX_SHM_START_OFFSET_ES10)
#define SMC_CONF_L2MUX_SHM_SIZE_ES10                         (1024*1024*4 + 1024*512)    /* 4.5MB */



    /* ---- ES2.0 ---- */
#define SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES20  "SH-Mobile-R8A73734-EOS2-ES20"
#define SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2_ES20         "WGEModem-3.1-EOS2-ES20"

//#define SMC_CONF_GLOBAL_SHM_START_ES20                       0x43B00000
//#define SMC_CONF_GLOBAL_SHM_END_ES20                         0x43FFFFFF

#define SMC_CONF_GLOBAL_SHM_START_ES20                       0x44001000
#define SMC_CONF_GLOBAL_SHM_END_ES20                         0x44800FFF     /* 7FFFFF -> 8192 kB -> 8 MB, NOTE: Use always the last address */

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


#ifdef SMC_IN_EOS2_ES10

    /* ===========================================================================
     * SHM Configuration is based on memory mapping for ES10
     *
     *
     * SHM Address    Modem Address   Owner
     * -------------------------------------------------------------------------
     * 0x4000_0000    0x0800_0000     L23 Code
     * 0x4100_0000    0x0900_0000     L1  Code
     * 0x4200_0000    0x0A00_0000     L23 Data
     * 0x4300_0000    0x0B00_0000     L1  Data
     * 0x43B0_0000    0x0BB0_0000     Shared Memory starts 0x500000 == 5242880 bytes
     * 0x4400_0000    0xC400_0000     Linux SW
     *
     * In Linux side the SHM is ioremapped from physical address (no static value possible)
     */

    //#define SMC_CONF_GLOBAL_SHM_START       0x43B00000

#else  /* #ifdef SMC_IN_EOS2_ES10 (Currently only two different EOS2 ASIC definitions) */

    /*
     * SMC configuration names
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
#endif

    /* =============================================================================
     * COMMON Configuration for EOS2 ES1.0/ES2.0
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


#define SMC_WAKEUP_USE_EXTERNAL_IRQ_MODEM         /* If defined uses ext irq in modem side (modem wakes APE)*/

/* #define SMC_WAKEUP_USE_EXTERNAL_IRQ_APE */     /* If defined the APE SMC initializes the WUP signal (modem wakes APE)*/

#if( defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_APE ) || defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_MODEM ) )

        /* Configuration for Modem and APE*/
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_ID                65                                /* IRQ ID in the APE side */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_TYPE              SMC_SIGNAL_TYPE_INT_WGM_GENOUT
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_SENSE_DEFAULT     SMC_SIGNAL_SENSE_RISING_EDGE      /* Default value if the configuration negotiation fails */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_SENSE             SMC_SIGNAL_SENSE_FALLING_EDGE     /* SMC_SIGNAL_SENSE_FALLING_EDGE / SMC_SIGNAL_SENSE_RISING_EDGE */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_ID_FROM_MODEM     0                                 /* Modem side interrupt genio to wakeup APE */
    #define SMC_APE_WAKEUP_EXTERNAL_IRQ_REGISTER_HANDLER  FALSE

    #define SMC_APE_WAKEUP_WAKELOCK_TIMEOUT_MSEC          2000

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

//#define SMC_BUFFER_MESSAGE_OUT_OF_MDB_MEM       /* If defined, the message is buffered when out of MDB memory */

#endif  /* #ifndef SMC_INSTANCE_CONFIG_R8A73734_WGE31_H */

/* EOF */
