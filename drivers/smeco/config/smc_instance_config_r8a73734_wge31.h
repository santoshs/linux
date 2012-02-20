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

Version:       1    03-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_INSTANCE_CONFIG_R8A73734_WGE31_H
#define SMC_INSTANCE_CONFIG_R8A73734_WGE31_H


/*
 * SMC configuration names
 */

#define SMC_CONFIG_MASTER_NAME_SH_MOBILE_APE5R_EOS2  "SH-Mobile-APE5R-EOS2"
#define SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2      "WGEModem-3.1-EOS2"


/* ===========================================================================
 * SHM Configuration is based on memory mapping
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

#define SMC_CONF_GLOBAL_SHM_START       0x43B00000

    /* Put the SMC Control instance in the beginning of the SHM area */

    /* SHM area for SMC Control Instance */
#define SMC_CONF_CONTROL_SHM_START      SMC_CONF_GLOBAL_SHM_START
#define SMC_CONF_CONTROL_SHM_SIZE      (1024*200)                  /* 200kB */

    /* SHM Area for L2MUX */
#define SMC_CONF_L2MUX_SHM_START       (SMC_CONF_GLOBAL_SHM_START + SMC_CONF_CONTROL_SHM_SIZE + 64)
#define SMC_CONF_L2MUX_SHM_SIZE        (1024*1024*4 + 1024*512)    /* 4.5MB */



    /* Modem side offset */
#define SMC_CONF_SHM_OFFSET_TO_MODEM    (-1*0x38000000)

/*
 *  Peripheral addresses and configurations
 *  TODO Make configuration product specific --> e.g SH R8A73734
 */


    /* Modem side peripherals */
#define SMC_MODEM_INTGEN_L2_OFFSET                   16
#define SMC_MODEM_INTGEN_L2_FIRST                    35          /* C2_L2_CPU_Int_Gen_Ch0 */
#define SMC_PERIPHERAL_ADDRESS_MODEM_GOP_INTGEN_1    0x07C00040

    /* APE side peripherals */
#define SMC_ADDRESS_APE_OFFSET_TO_MODEM              0xDC000000

#define SMC_APE_IRQ_OFFSET_INTCSYS_SPI               32         /* INTC-SYS SPI ID starts from 32 -> 255 */
#define SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM            193        /* TODO Valid only with SPI 193-198 (NOT 221 and 222)*/

#endif
