/*
*   Common Smeco header used in Linux Kernel.
*
*   Copyright © Renesas Mobile Corporation 2011. All rights reserved
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

Version:       3    23-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Linux Net Device driver implementation

Version:       1    08-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_LINUX_H
#define SMC_LINUX_H

#define SMC_DRIVER_OK     0
#define SMC_DRIVER_ERROR  (-EINVAL)


#define SMC_PLATFORM_DRIVER_NAME    "smc_net_device"
    /*
     * The  device name format Smeco uses.
     * The postfix '%d' of the name is the interface number kernel creates for the driver, e.g "smc0", "smc1"
     */
#define SMC_NET_DEVICE_NAME_FORMAT   "smc%d"

    /*
     * Upper layer TX/RX callback function prototypes
     */
typedef int ( *upper_layer_skb_tx )( struct sk_buff *skb, struct net_device *device );
typedef int ( *upper_layer_skb_rx )( struct sk_buff *skb, struct net_device *device );

    /* IOCTL function for upper layers */
typedef int ( *upper_layer_net_device_driver_ioctl )( struct net_device* device, struct ifreq* ifr, int cmd );

    /* Queue selection handler function for upper layers */
typedef uint16_t ( *upper_layer_net_device_driver_select_queue )( struct net_device *device, struct sk_buff *skb );

    /* SKB packet and SMC user data modification function for upper layer
     * This is called right before sending data to other CPU
     */

typedef int (* upper_layer_modify_send_data )( struct sk_buff *skb, smc_user_data_t* smc_user_data );

typedef void (* upper_layer_device_driver_setup )(struct net_device* dev);

    /* SMC Configuration creation function prototype */
typedef smc_conf_t* ( *smc_device_create_conf )(void);


    /*
     * Private IO Control commands
     * #define SIOCDEVPRIVATE   0x89F0 to 89FF (Max +15)
     * #define SIOCPROTOPRIVATE 0x89E0 to 89EF
     */
/*#define SIOCPNGETOBJECT     (SIOCPROTOPRIVATE + 0)*/


    /*
     * Net Device specific configuration
     */
typedef struct _smc_device_driver_config
{
    char*              name;
    int                queue_count;

        /* SMC Configuration function */
    smc_device_create_conf smc_conf;

        /* TX and RX functions */
    upper_layer_skb_tx skb_tx_function;
    upper_layer_skb_rx skb_rx_function;

    upper_layer_net_device_driver_ioctl driver_ioctl;

    upper_layer_net_device_driver_select_queue driver_select_queue;

    upper_layer_modify_send_data driver_modify_send_data;

    upper_layer_device_driver_setup driver_setup;

        /* Link to the device */
    struct _smc_device_driver_priv_t* device_driver_priv;

} smc_device_driver_config;


    /* SMC Network Device Driver object*/
typedef struct _smc_device_driver_priv_t
{
    struct net_device*        net_dev;
    struct platform_device*   platform_device;

        /* Single SMC instance in every device */
    smc_t*                    smc_instance;

    smc_device_driver_config* smc_dev_config;

} smc_device_driver_priv_t;


/*
 * Net device driver interface request structure for commands
 */

#include "linux/if.h"

#define SIOCDEV_SEND_DATA     (SIOCDEVPRIVATE + 0x04)
#define SIOCDEV_RUN_TEST      (SIOCDEVPRIVATE + 0x05)       /* Run SMC tests (valid only when test module is built in)*/
#define SIOCDEV_STATUS        (SIOCDEVPRIVATE + 0x06)       /* Return the status of the specified device*/

struct ifreq_smc
{
    union
    {
      char ifrn_name[IFNAMSIZ];        /* if name, e.g. "smc0" */
    } ifr_ifrn;

    uint32_t if_data_len;
    uint8_t* if_data;
};

struct ifreq_smc_test
{
    union
    {
      char ifrn_name[IFNAMSIZ];        /* if name, e.g. "smc0" */
    } ifr_ifrn;

    uint32_t if_test_case;
    uint32_t if_test_data_len;
    uint8_t* if_test_data;
    uint32_t if_test_result;
};

#endif
