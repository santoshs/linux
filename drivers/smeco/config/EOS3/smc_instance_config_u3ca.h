/*
*   Common SMC instance configuration for Renesas U3CA
*   Copyright � Renesas Mobile Corporation 2013. All rights reserved
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
#if 0/*
Change history:

Version:       1    12-Feb-2013     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_INSTANCE_CONFIG_U3CA_H
#define SMC_INSTANCE_CONFIG_U3CA_H

#ifdef SMECO_MODEM

  #define SMC_CPU_NAME          "Modem"

  #define SDRAM_SMC_START_ADDR  0x45001000
  #define SDRAM_SMC_END_ADDR    0x457FFBFF

#elif( defined(SMECO_LINUX_KERNEL) )

  /*#include <mach/memory-XXXXX.h>*/          /* Common header file containing shared memory addresses */
    /* TODO Add proper memory mapping header file */

  #define SMC_CPU_NAME          "APE"

  #define SDRAM_SMC_START_ADDR  0x45001000
  #define SDRAM_SMC_END_ADDR    0x457FFBFF

#else
  #error "Error in EOS3 configuration"
#endif


    /* =============================================================================
     * SMC specific configurations for EOS3 U3CA
     */


    /* ---- EOS3 U3CA ES1 ---- */
#define SMC_CONFIG_MASTER_NAME_APE_EOS3_U3CA_ES1            "EOS3-U3CA-APE"
#define SMC_CONFIG_SLAVE_NAME_MODEM_WGEM50_EOS3_ES1         "WGEModem50-EOS3-U3CA"

#define SMC_CONF_GLOBAL_SHM_START_ES1                       SDRAM_SMC_START_ADDR
#define SMC_CONF_GLOBAL_SHM_END_ES1                         SDRAM_SMC_END_ADDR      /* 7FFFFF -> 8192 kB -> 8 MB, NOTE: Use always the last address */

    /* SHM area for SMC Control Instance */
#define SMC_CONF_CONTROL_SHM_START_OFFSET_ES1               (0)
#define SMC_CONF_CONTROL_SHM_START_ES1                      (SMC_CONF_GLOBAL_SHM_START_ES1 + SMC_CONF_CONTROL_SHM_START_OFFSET_ES1)
#define SMC_CONF_CONTROL_SHM_SIZE_ES1                       (1024*200)                  /* 200kB */

    /* SHM Area for L2MUX */
#define SMC_CONF_L2MUX_SHM_START_OFFSET_ES1                 (SMC_CONF_CONTROL_SHM_SIZE_ES1 + 64)
#define SMC_CONF_L2MUX_SHM_START_ES1                        (SMC_CONF_GLOBAL_SHM_START_ES1 + SMC_CONF_L2MUX_SHM_START_OFFSET_ES1)
#define SMC_CONF_L2MUX_SHM_SIZE_ES1                         (1024*1856*2 + 1024*256*4 + 1024*30)    /* 4.5MB */


#define SMC_CHANNEL_ID_FOR_CONFIGURATION                     0   /* Channel to used for configure others (FIFO, SHM), if not defined, no config.
                                                                    FIFO+SHM configuration of this channel is not allowed to change without parallel changes to the other device
                                                                    Currently only first channel (0) is supported.
                                                                    The master (APE) only initiate the runtime configuration and its config is used
                                                                  */

    /* =============================================================================
     * COMMON Configuration for EOS3 U3CA ES1
     */

#define SMC_CONF_PM_APE_HOST_ACCESS_REQ_ENABLED     TRUE

    /* Modem side offset */
#define SMC_CONF_SHM_OFFSET_TO_MODEM                (-1*0x20000000)


/* =================================================
 * Peripheral addresses and configurations
 *
 */

    /*
     * Modem side peripherals
     */
#define SMC_MODEM_INTGEN_L2_OFFSET                   0             /* Bit offset in the GOP register */
#define SMC_MODEM_INTGEN_L2_FIRST                    74            /* First IntGen: IRQ_CPU_Int_Gen_Ch0 */
#define SMC_PERIPHERAL_ADDRESS_MODEM_GOP_INTGEN_1    0x87024050     /* a_gop001_intgen_0 */

    /*
     * APE side peripherals
     */
#define SMC_ADDRESS_APE_OFFSET_TO_MODEM              0x59000000

#define SMC_APE_IRQ_OFFSET_INTCSYS_SPI               32             /* INTC-SYS SPI ID starts from 32 -> 255 */
#define SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM            193            /* TODO Valid only with SPI 193-198 (NOT 221 and 222)*/

#define SMC_APE_IRQ_OFFSET_IRQ_SPI                   512            /* APE IRQ offset (#define IRQPIN_IRQ_BASE 512 in irqs.h )*/


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
#else
    #define SMC_VERSION_REQUIREMENT_EXTERNAL_IRQ_POLARITY  {0,0,""}
#endif /* #if( defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_APE ) || defined( SMC_WAKEUP_USE_EXTERNAL_IRQ_MODEM ) ) */

    /* Version requirements */
#define SMC_VERSION_REQUIREMENT_MHDP_SHM_CHANGE_0_0_37 { SMC_VERSION_TO_INT(0,0,37), SMC_VERSION_REQUIREMENT_LEVEL_ASSERT, "MHDP Channel Shared Memory size change" }

    /*
     * SMC Version requirements
     */
#define SMC_VERSION_REQUIREMENT_ARRAY  SMC_VERSION_REQUIREMENT_EXTERNAL_IRQ_POLARITY, SMC_VERSION_REQUIREMENT_MHDP_SHM_CHANGE_0_0_37
#define SMC_VERSION_REQUIREMENT_COUNT  2

/* #define SMC_BUFFER_MESSAGE_OUT_OF_MDB_MEM */      /* If defined, the message is buffered when out of MDB memory */


/* ======================================
 * Target specific product configurations
 *
 */

#ifdef SMECO_MODEM
    /* -------------------------------------
     * MODEM specific configuration
     */

  /* #define SMC_BUFFER_MESSAGE_OUT_OF_FIFO_ITEMS */            /* If defined, the message is buffered when FIFO is full */
  #define SMC_HISTORY_DATA_COLLECTION_ENABLED                   /* If defined, the SMC data transfer collection is enabled in the modem side. This has some impact to throughput performance */

    /* Definition for Modem to wakeup APE when the channel interrupt does not do that */
  #define SYSC_PSTR_REG (*(volatile uint32_t *)0x80180080)
  #define SYSC_PSTR_C4_A3SM_A2SL ((1 << 16) | (1 << 20) | (1 << 19))

  #define SMC_MODEM_WAKEUP_APE()           {                                                                            \
                                                volatile uint32_t sysc_pstr_data;                                       \
                                                volatile uint32_t sysc_poll_delay;                                      \
                                                                                                                        \
                                                while (SYSC_PSTR_C4_A3SM_A2SL != ((sysc_pstr_data = SYSC_PSTR_REG) & SYSC_PSTR_C4_A3SM_A2SL))           \
                                                {                                                                                                       \
                                                    SMC_TRACE_PRINTF_INFO("smc_signal_raise: SYSC.PSTR C4/A3SM/A2SL is off (0x%08X)", sysc_pstr_data);  \
                                                    for (sysc_poll_delay = 0xFFF; sysc_poll_delay; sysc_poll_delay--);                                  \
                                                }                                                                                                       \
                                                SMC_TRACE_PRINTF_SIGNAL("smc_signal_raise: SYSC.PSTR C4ST C4+A3SM+A2SL is ON (0x%08X)", sysc_pstr_data);\
                                            }




    #define SMC_MODEM_GENIO_GOP001_REG    &a_gop001_intgen_0
    #define SMC_MODEM_INTGEN_IN_USE                         /* If defined, the intgen interrupts are enabled */

#elif( defined(SMECO_LINUX_KERNEL) )
    /*
     * APE Linux Kernel specific configuration
     */

  /*#define SMC_MODEM_WAKEUP_WAIT_TIMEOUT_MS       0*/ /* 5 */      /* Timeout in milliseconds to wait the modem to wake up. If 0, waits forever, if not defined no wakeup request from APE */

  #define SMC_BUFFER_MESSAGE_OUT_OF_FIFO_ITEMS                  /* If defined, the message is buffered when FIFO is full */

  #define SMC_SUPPORT_SKB_FRAGMENT_UL                           /* If defined, the SKB fragments are supported in uplink. NOTE: enable also the SMC_DMA_ENABLED with this for highmem config */
  #define SMC_DMA_ENABLED                                       /* If defined, the DMA initialization is enabled, this is required by the SKB fragment handling */
  /*#define SMC_DMA_TRANSFER_ENABLED*/                          /* If defined, the DMA transfer feature is enabled for channel configuration, requires SMC_DMA_ENABLED to be defined */

  #define SMC_DMA_TRANSFER_TIMEOUT_MS   500                     /* TODO Optimize */

  /* #define SMC_HISTORY_DATA_COLLECTION_ENABLED */             /* If defined, the SMC data transfer collection is enabled in the kernel side. This has some impact to throughput performance */


      /*
       * Device driver suspend and resume macros
       */
  #define SMC_PLATFORM_DEVICE_DRIVER_SUSPEND( device, wakeup_irq_sense )    {                                                                               \
                                                                                uint32_t signal_event_sense = 0x00000002;                                   \
                                                                                if( wakeup_irq_sense == SMC_SIGNAL_SENSE_RISING_EDGE ) {                    \
                                                                                    signal_event_sense = 0x00000002; }                                      \
                                                                                else if( wakeup_irq_sense == SMC_SIGNAL_SENSE_FALLING_EDGE ) {              \
                                                                                    signal_event_sense = 0x00000001; }                                      \
                                                                                else {                                                                      \
                                                                                    SMC_TRACE_PRINTF_ASSERT("SMC_PLATFORM_DEVICE_DRIVER_SUSPEND: invalid wakeup IRQ sense value"); \
                                                                                    assert(0);                                                              \
                                                                                }                                                                           \
                                                                                __raw_writel(0x00000001, 0xe61c2414);           /* PORT_SET */              \
                                                                                __raw_writel(signal_event_sense, 0xe61c1980);   /* CONFIG_0 - 1 = low level detect, 2 = high level detect */ \
                                                                                __raw_writel(0x00000001, 0xe61c1888);           /* WAKEN_SET0 */            \
                                                                            }

      /* Called from the smc_platform_device_driver_resume() */
  #define SMC_PLATFORM_DEVICE_DRIVER_RESUME( device )                       {                                                                               \
                                                                                __raw_writel(0x00000000, 0xe61c1980); /* CONFIG_02 - Disable Interrupt */               \
                                                                                __raw_writel(0x00000001, 0xe61c1884); /* WAKEN_STS0 - Disable WakeUp Request Enable */  \
                                                                            }


#endif  /* End of target specific configuration */

#endif  /* #ifndef SMC_INSTANCE_CONFIG_R8A73734_WGE31_H */

/* EOF */
