/*
*   Common SMC configuration for L2MUX.
*   This includes various configuration files for different products.
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

#ifndef SMC_CONFIG_L2MUX_H
#define SMC_CONFIG_L2MUX_H

#define SMC_CONFIG_USER_L2MUX       "L2MUX"


    /* L2MUX Queue mapping for protocols */
#define SMC_L2MUX_QUEUE_1_PHONET     0
#define SMC_L2MUX_QUEUE_2_MHI        1
#define SMC_L2MUX_QUEUE_3_MHDP       2


    /*
     * Product specific configuration header files
     *
     * TODO THE PRODUCT SPECIFIC BUILD FLAG NEEDED
     */


/*#include "smc_config_l2mux_r8a73734_wge31.h"*/

#if( SMC_CURRENT_PRODUCT_CONFIG == SMC_CONFIG_EOS2 )
    #include "smc_config_l2mux_r8a73734_wge31.h"
#elif( SMC_CURRENT_PRODUCT_CONFIG == SMC_CONFIG_EOS3 )
    #include "smc_config_l2mux_u3ca.h"
#elif( SMC_CURRENT_PRODUCT_CONFIG == SMC_CONFIG_EOS3_WGE31 )
    #include "smc_config_l2mux_u3.h"
#else
    #error "Invalid SMC product configuration"
#endif



    /*
     * Common L2MUX configuration function prototypes.
     */

smc_instance_conf_t* smc_instance_conf_get_l2mux( char* smc_user_name, char* config_name );

#endif
