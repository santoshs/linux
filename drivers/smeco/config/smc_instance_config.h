/*
*   Common SMC instance configuration file.
*   This file includes the configuration files for different products
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

#ifndef SMC_INSTANCE_CONFIG_H
#define SMC_INSTANCE_CONFIG__H


#define SMC_MODEM_WAKEUP_WAIT_TIMEOUT_MS       0 /* 5 */   /* Timeout in milliseconds to wait the modem to wake up. If 0, waits forever */

    /*
     * Product specific SMC instance header files
     *
     * TODO THE PRODUCT SPECIFIC BUILD FLAG NEEDED
     */

#include "smc_instance_config_r8a73734_wge31.h"

#endif
