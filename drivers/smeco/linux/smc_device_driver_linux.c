/*
*   Smeco device driver implementation for Linux Kernel.
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

Version:       3    17-Jan-20112    Heikki Siikaluoma
Status:        draft
Description :  Improvements, test features added

Version:       1    23-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#include "smc_common_includes.h"

#include "smc.h"
#include "smc_trace.h"

#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_phonet.h>

#include "smc_linux.h"

#if(SMC_CONTROL==TRUE)

  #define SMC_CONF_COUNT_CONTROL  1

  extern smc_device_driver_config dev_config_control;

#else
  #define SMC_CONF_COUNT_CONTROL  0
#endif

#if(SMC_L2MUX_IF==TRUE)
  #include "smc_conf_l2mux_linux.h"

  #define SMC_CONF_COUNT_L2MUX  1

    /*
     * The Device Configuration variables for L2MUX usage
     */
  extern smc_device_driver_config dev_config_l2mux;
#else
      /* No L2MUX included */
  #define SMC_CONF_COUNT_L2MUX  0
#endif

MODULE_AUTHOR("Renesas Mobile Europe / MeXe");
MODULE_DESCRIPTION("SMeCo - SMC Network Device Driver");
MODULE_LICENSE("Dual BSD/GPL");


#if( SMCTEST == TRUE )
    /*#define SMC_TEST_PHONET_DEVICE  0x70*/   /* Phonet messages to this device are routed to SMC test handler */
    static uint8_t smc_test_linux_start_by_phonet_msg(uint32_t phonet_msg_len, uint8_t* phonet_msg);

        /*
         * For test purposes the smeco.ko can be instantiated in the driver
         * NOTE: In this case there should be no instantiation in the kernel starup
         */
    static int       instantiate  = 0;

    module_param(instantiate, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    MODULE_PARM_DESC(instantiate, "Instantiate SMC Platform Driver");

    /**
     * IRQ Resources from the R-Mobile HW Manual
     */
    static struct resource smc0_resources[] =
    {
        [0] = {
                  .start = gic_spi(193),
                  .end   = gic_spi(193),
                  .flags = IORESOURCE_IRQ | IRQ_TYPE_LEVEL_LOW,
              },
        [1] = {
                  .start = gic_spi(194),
                  .end   = gic_spi(194),
                  .flags = IORESOURCE_IRQ | IRQ_TYPE_LEVEL_LOW,
              },
        [2] = {
                  .start = gic_spi(195),
                  .end   = gic_spi(195),
                  .flags = IORESOURCE_IRQ,
              },
        [3] = {
                  .start = gic_spi(196),
                  .end   = gic_spi(196),
                  .flags = IORESOURCE_IRQ,
              },
    };


    static struct platform_device smc_netdevice0 =
    {
      .name          = "smc_net_device",
      .id            = 0,
      .resource      = smc0_resources,
      .num_resources = ARRAY_SIZE(smc0_resources),
    };

    static struct platform_device *devices[] __initdata =
    {
        &smc_netdevice0,
    };

#endif



    /*
     * Net Device configuration array of variables defined above.
     * The first device has id 0, the second has id 1, etc.
     *
     */

#define SMC_DEVICE_DRIVER_CONF_COUNT    (SMC_CONF_COUNT_CONTROL + SMC_CONF_COUNT_L2MUX)

smc_device_driver_config* smc_device_conf[SMC_DEVICE_DRIVER_CONF_COUNT] =
{
#if(SMC_L2MUX_IF==TRUE)
    &dev_config_l2mux,
#endif

#if(SMC_CONTROL==TRUE)
    &dev_config_control,
#endif

};


    /*
     * Device methods implemented by SMC
     */
static int smc_net_device_driver_open(struct net_device* device);
static int smc_net_device_driver_close(struct net_device* device);
static int smc_net_device_driver_xmit(struct sk_buff *skb, struct net_device *device);
uint16_t   smc_net_device_driver_select_queue(struct net_device *device, struct sk_buff *skb);
static int smc_net_device_driver_ioctl(struct net_device *device, struct ifreq *ifr, int cmd);
static int smc_net_device_driver_set_mtu(struct net_device *device, int new_mtu);

    /*
     * Net Device function implementation declarations.
     */
static const struct net_device_ops smc_net_device_ops =
{
    .ndo_open           = smc_net_device_driver_open,
    .ndo_stop           = smc_net_device_driver_close,
    .ndo_select_queue   = smc_net_device_driver_select_queue,
    .ndo_start_xmit     = smc_net_device_driver_xmit,
    .ndo_do_ioctl       = smc_net_device_driver_ioctl,
    .ndo_change_mtu     = smc_net_device_driver_set_mtu,
};


/*
 * Network device configuration getter.
 */
static smc_device_driver_config* smc_get_device_driver_config(int platform_device_id)
{
    SMC_TRACE_PRINTF_WARNING("smc_get_device_driver_config: search configuration for device id %d...", platform_device_id);

    if( platform_device_id < SMC_DEVICE_DRIVER_CONF_COUNT )
    {
        SMC_TRACE_PRINTF_WARNING("smc_get_device_driver_config: configuration for device id %d found", platform_device_id);

        return smc_device_conf[platform_device_id];
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_get_device_driver_config: configuration for device id %d not found", platform_device_id);
        return NULL;
    }
}

static int smc_net_device_driver_open(struct net_device* device)
{
    int                       ret_val  = SMC_DRIVER_OK;
    smc_device_driver_priv_t* smc_priv = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_open: Device 0x%08X...", (uint32_t)device);

    smc_priv = netdev_priv(device);

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_open: SMC priv: 0x%08X, SMC version %s...", (uint32_t)smc_priv, SMC_SW_VERSION);

    if( smc_priv != NULL && smc_priv->smc_dev_config != NULL )
    {
        smc_conf_t* smc_instance_conf = smc_priv->smc_dev_config->smc_conf();

        if( smc_instance_conf != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_open: SMC priv 0x%08X: read SMC configuration 0x%08X and create SMC instance, use parent 0x%08X...",
                    (uint32_t)smc_priv, (uint32_t)smc_instance_conf, smc_priv->platform_device);

            smc_priv->smc_instance = smc_instance_create_ext(smc_instance_conf, smc_priv->platform_device);

            if( smc_priv->smc_instance != NULL )
            {
                SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_open: SMC priv 0x%08X: SMC instance 0x%08X created", (uint32_t)smc_priv, (uint32_t)smc_priv->smc_instance);

                SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_open: netif_tx_start_all_queues...");
                netif_tx_start_all_queues(device);

                ret_val = SMC_DRIVER_OK;
            }
            else
            {
                SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_open: SMC instance creation failed");
                ret_val = SMC_DRIVER_ERROR;
            }
         }
         else
         {
             SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_open: SMC configuration not initialized");
             ret_val = SMC_DRIVER_ERROR;
         }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_open: invalid SMC device configuration");
        ret_val = SMC_DRIVER_ERROR;
    }

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_open: Completed by return value %d", ret_val);

    return ret_val;
}

static int smc_net_device_driver_close(struct net_device* device)
{
    int ret_val = SMC_DRIVER_OK;
    smc_device_driver_priv_t* smc_priv = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: Device 0x%08X...", (uint32_t)device);

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: net device 0x%08X: stop all queues...", (uint32_t)device);
    netif_tx_stop_all_queues(device);

    smc_priv = netdev_priv(device);

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: SMC priv: 0x%08X...", (uint32_t)smc_priv);

    if( smc_priv->smc_instance != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: SMC priv: 0x%08X: Destroying SMC instance 0x%08X...",
                (uint32_t)smc_priv, (uint32_t)smc_priv->smc_instance);

        smc_instance_destroy( smc_priv->smc_instance );
        smc_priv->smc_instance = NULL;

        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: SMC priv: 0x%08X: SMC instance destroyed", (uint32_t)smc_priv);
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: SMC priv: 0x%08X: SMC instance not initialized", (uint32_t)smc_priv);
    }


    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_close: Completed by return value %d", ret_val);

    return ret_val;
}

static int smc_net_device_driver_xmit(struct sk_buff* skb, struct net_device* device)
{
    int                       ret_val      = SMC_DRIVER_OK;
    smc_device_driver_priv_t* smc_net_dev  = NULL;
    smc_t*                    smc_instance = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: device 0x%08X, protocol 0x%04X, queue %d...", (uint32_t)device, skb->protocol, skb->queue_mapping );
    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: SKB Data (len %d):", skb->len);
    SMC_TRACE_PRINTF_DEBUG_DATA( skb->len , skb->data );

#if( SMCTEST == TRUE )
        /* Check if message for the test interface */
    if( skb->protocol == htons(ETH_P_PHONET) )
    {
        if( skb->len > 1 && skb->data[0] == SMC_TEST_PHONET_DEVICE )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: Message is routed to SMC test handler...");

            if( smc_test_linux_start_by_phonet_msg( skb->len, skb->data ) == SMC_OK )
            {
                SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: SMC test OK");
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: SMC test FAILED");
            }

            /* Just drop the packet */
            goto DROP_PACKET;
        }
        else
        {
            SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: Message is not for SMC test handler");
        }
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: Message protocol is not ETH_P_PHONET (0x%02X)", htons(ETH_P_PHONET));
    }
#endif


    if (skb->len < PHONET_MIN_MTU)
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: protocol %d, packet too short, len %d, required %d",skb->protocol, skb->len, PHONET_MIN_MTU);
        goto DROP_PACKET;
    }

        /* 32-bit alignment */
    if ((skb->len & 3) && skb_pad(skb, 4 - (skb->len & 3)))
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: protocol %d, packet 32-bit alignment failed", skb->protocol);
        goto DROP_PACKET;
    }

    smc_net_dev = netdev_priv(device);

    smc_instance = smc_net_dev->smc_instance;

    if( smc_instance != NULL )
    {
        if( skb->queue_mapping < smc_instance->smc_channel_list_count )
        {
            SMC_TRACE_PRINTF_INFO("smc_net_device_driver_xmit: stop the subqueue to allow queue in upper layer");
            netif_stop_subqueue(device, skb->queue_mapping);

            SMC_TRACE_PRINTF_INFO("smc_net_device_driver_xmit: deliver to upper layer TX function...");
            ret_val = smc_net_dev->smc_dev_config->skb_tx_function(skb, device);

            if( unlikely(ret_val) )
            {
                SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_xmit: protocol %d, L2MUX SKB TX failed", skb->protocol);
                goto DROP_PACKET;
            }
            else
            {
                smc_channel_t*  smc_channel = NULL;
                smc_user_data_t userdata;

                SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: send data using SMC 0x%08X channel id %d...", (uint32_t)smc_instance, skb->queue_mapping);

                    /* Select the channel by the Queue */
                smc_channel = SMC_CHANNEL_GET(smc_instance, skb->queue_mapping );

                userdata.flags     = 0x00000000;
                /*userdata.userdata1 = *(uint32_t*)skb->data;*/     /* TODO L2MUX specific code to be removed from common XMIT */
                userdata.userdata1 = 0x00000000;
                userdata.userdata2 = 0x00000000;
                userdata.userdata3 = 0x00000000;
                userdata.userdata4 = 0x00000000;
                userdata.userdata5 = 0x00000000;

                /*skb_pull(skb, SMC_L2MUX_HEADER_SIZE);*/           /* TODO L2MUX specific code to be removed from common XMIT */


                if( smc_net_dev->smc_dev_config && smc_net_dev->smc_dev_config->driver_modify_send_data )
                {
                    SMC_TRACE_PRINTF_ERROR("smc_net_device_driver_xmit: upper layer wants to modify send packet");
                    smc_net_dev->smc_dev_config->driver_modify_send_data(skb, &userdata);
                }

                /* TODO Check fragmentation */
                /* DPRINTK("NB_FRAGS = %d total size: %d frags size: %d\n", skb_shinfo(skb)->nr_frags, skb->len, skb->data_len); */

                if( smc_send_ext(smc_channel, skb->data, skb->len, &userdata) != SMC_OK )
                {
                    SMC_TRACE_PRINTF_ERROR("smc_net_device_driver_xmit: protocol %d, smc_send_ext failed", skb->protocol);

                    /* TODO Check if not wakeup the sub queue because of the send failure */
                    SMC_TRACE_PRINTF_INFO("smc_net_device_driver_xmit: wake up the subqueue to allow message sending");
                    netif_wake_subqueue(device, skb->queue_mapping);

                    goto DROP_PACKET;
                }
                else
                {
                        /* Update statistics */
                    device->stats.tx_packets++;
                    device->stats.tx_bytes += skb->len;

                        /* Free the message data */
                    dev_kfree_skb(skb);

                    SMC_TRACE_PRINTF_INFO("smc_net_device_driver_xmit: wake up the subqueue to allow message sending");
                    netif_wake_subqueue(device, skb->queue_mapping);

                    ret_val = SMC_DRIVER_OK;
                    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: Completed by return value %d", ret_val);
                }
            }
        }
        else
        {
            SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_xmit: SMC instance has no channel for queue %d", skb->queue_mapping);
            goto DROP_PACKET;
        }

        return ret_val;
     }
     else
     {
         SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_xmit: SMC instance not initialized for the device");
     }

    /* --- DROP the PACKET ---- */
DROP_PACKET:
    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_xmit: Packet dropped");

    device->stats.tx_dropped++;
    dev_kfree_skb(skb);

    return ret_val;
}

uint16_t smc_net_device_driver_select_queue(struct net_device *device, struct sk_buff *skb)
{
    smc_device_driver_priv_t* smc_net_dev  = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_select_queue: device 0x%08X, protocol 0x%04X...", (uint32_t)device, skb->protocol);

    smc_net_dev = netdev_priv(device);

    if( smc_net_dev && smc_net_dev->smc_dev_config && smc_net_dev->smc_dev_config->driver_select_queue)
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_select_queue: delivering to upper layer queue selection handler...");
        return smc_net_dev->smc_dev_config->driver_select_queue( device, skb );
    }
    else
    {
        SMC_TRACE_PRINTF_ASSERT("smc_net_device_driver_select_queue: no queue selection handler in upper layer");
        assert(0);
        return 0;
    }
}

static int smc_net_device_driver_ioctl(struct net_device* device, struct ifreq* ifr, int cmd)
{
    int                       ret_val      = SMC_DRIVER_OK;
    smc_device_driver_priv_t* smc_net_dev  = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_ioctl: starts device 0x%08X, command %d (0x%08X)...", (uint32_t)device, cmd, cmd);

    smc_net_dev = netdev_priv(device);

    if( smc_net_dev && smc_net_dev->smc_dev_config && smc_net_dev->smc_dev_config->driver_ioctl)
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_ioctl: delivering to upper layer IOCTL handler...");
        ret_val = smc_net_dev->smc_dev_config->driver_ioctl(device, ifr, cmd );
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_ioctl: upper layer does not have IOCTL handler");
        switch(cmd)
        {
        /*
            case SIOCPNGETOBJECT:
            {
                SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_ioctl: SIOCPNGETOBJECT");
                break;
            }
            */
            default:
            {
                SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_ioctl: Unsupported command %d", cmd);
                break;
            }
        }
    }

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_ioctl: Completed by return value %d", ret_val);

    return ret_val;
}

static int smc_net_device_driver_set_mtu(struct net_device *device, int new_mtu)
{
    int ret_val = SMC_DRIVER_OK;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_set_mtu: device 0x%08X, old MTU %d new MTU %d...", (uint32_t)device, device->mtu, new_mtu);

    /*if ((new_mtu < MHI_MIN_MTU) || (new_mtu > MHI_MAX_MTU))*/
    if( FALSE )
    {
        SMC_TRACE_PRINTF_WARNING("smc_net_device_driver_set_mtu: device 0x%08X, invalid MTU %d", (uint32_t)device, new_mtu);

        ret_val =  SMC_DRIVER_ERROR;
    }
    else
    {
        device->mtu = new_mtu;
        ret_val = SMC_DRIVER_OK;
    }

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_set_mtu: Completed by return value %d", ret_val);

    return ret_val;
}

static void smc_net_device_driver_setup(struct net_device* device)
{
    smc_device_driver_priv_t* smc_net_dev  = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_setup: device 0x%08X starts...", (uint32_t)device);

    smc_net_dev = netdev_priv(device);

    if( smc_net_dev && smc_net_dev->smc_dev_config && smc_net_dev->smc_dev_config->driver_setup)
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_ioctl: delivering to upper layer IOCTL handler...");
        smc_net_dev->smc_dev_config->driver_setup( device );
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_setup: device 0x%08X creating default configuration...", (uint32_t)device);

        device->features        = NETIF_F_SG /* Frags to be tested by MHDP team  | NETIF_F_HW_CSUM | NETIF_F_FRAGLIST*/;
        device->type            = 0;
        device->flags           = IFF_POINTOPOINT | IFF_NOARP;
        device->mtu             = 1024;
        device->hard_header_len = 4;
        device->dev_addr[0]     = 0x00;
        device->addr_len        = 1;
        device->tx_queue_len    = 500;
    }

        /* These are the mandatory values */
    device->netdev_ops      = &smc_net_device_ops;
    device->destructor      = free_netdev;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_driver_setup: completed");
}


/*
 * SMC Platform driver implementation.
 *
 */

static int __devinit smc_net_platform_device_probe(struct platform_device* platform_device)
{
    int                ret_val = SMC_DRIVER_OK;
    struct net_device* ndevice = NULL;
    smc_device_driver_config* dev_conf = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: SMC v.%s: platform device 0x%08X: id %d, name %s num resources %d...",
            SMC_SW_VERSION, (uint32_t)platform_device, platform_device->id, platform_device->name, platform_device->num_resources);

    dev_conf = smc_get_device_driver_config(platform_device->id);

    if( !dev_conf )
    {
        ret_val = SMC_DRIVER_ERROR;
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: platform device 0x%08X: allocate net device using '%s' configuration...",
                (uint32_t)platform_device, dev_conf->name);

        ndevice = alloc_netdev_mq( sizeof(smc_device_driver_priv_t), SMC_NET_DEVICE_NAME_FORMAT, smc_net_device_driver_setup, dev_conf->queue_count );

        SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: net device is 0x%08X, queue count %d", (uint32_t)ndevice, dev_conf->queue_count);

        if( ndevice )
        {
            smc_device_driver_priv_t* smc_net_dev = NULL;

            SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: net device 0x%08X: get SMC priv...", (uint32_t)ndevice);

            smc_net_dev = netdev_priv( ndevice );

            SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: net device 0x%08X: SMC priv 0x%08X", (uint32_t)ndevice, (uint32_t)smc_net_dev);

                /* Initialize SMC Net Device */
            smc_net_dev->net_dev         = ndevice;
            smc_net_dev->platform_device = platform_device;
                /* SMC instance is created while the device is opened */
            smc_net_dev->smc_instance    = NULL;
            smc_net_dev->smc_dev_config  = dev_conf;

                /* Link the config to priv */
            dev_conf->device_driver_priv = smc_net_dev;

            SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: net device 0x%08X: stop all queues...", (uint32_t)ndevice);
            netif_tx_stop_all_queues(ndevice);

            SET_NETDEV_DEV(smc_net_dev->net_dev, &platform_device->dev);

            SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: register net device 0x%08X (0x%08X)...", (uint32_t)ndevice, (uint32_t)smc_net_dev->net_dev);
            ret_val = register_netdev(smc_net_dev->net_dev);

            if( ret_val >= 0 )
            {
                ret_val = SMC_DRIVER_OK;

                SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: net device 0x%08X OK", (uint32_t)ndevice);
            }
            else
            {
                dev_err(&platform_device->dev, "netdev register failed (%d)\n", ret_val);
                free_netdev(smc_net_dev->net_dev);
                SMC_TRACE_PRINTF_ERROR("smc_net_platform_device_probe: net device register failed (%d)", ret_val);
                ret_val = SMC_DRIVER_ERROR;
            }
        }
        else
        {
            dev_err(&platform_device->dev, "No memory for netdev\n");
            SMC_TRACE_PRINTF_ERROR("smc_net_platform_device_probe: no memory for net device");
            ret_val = SMC_DRIVER_ERROR;
        }
    }

    SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_probe: completed by return value 0x%02X", ret_val);

    return ret_val;
}

static int smc_net_device_remove( smc_device_driver_priv_t* smc_device_priv )
{
    int                ret_val = SMC_DRIVER_OK;
    struct net_device* device  = smc_device_priv->net_dev;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_remove: unregister net device 0x%08X...", (uint32_t)device);
    unregister_netdev( device );

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_remove: Destroy SMC instance 0x%08X in priv 0x%08X...", (uint32_t)smc_device_priv->smc_instance, (uint32_t)smc_device_priv);

    smc_instance_destroy( smc_device_priv->smc_instance );
    smc_device_priv->smc_instance = NULL;

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_remove: free net device 0x%08X...", (uint32_t)device);
    free_netdev(device);

    SMC_TRACE_PRINTF_DEBUG("smc_net_device_remove: completed by return value 0x%02X", ret_val);

    return ret_val;
}

static int smc_net_platform_device_remove(struct platform_device* platform_device)
{
    int ret_val = SMC_DRIVER_OK;
    int i       = 0;

    SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_remove: platform device: 0x%08X id %d...", (uint32_t)platform_device, platform_device->id);

    for( i = 0; i < SMC_DEVICE_DRIVER_CONF_COUNT; i++ )
    {
        smc_device_driver_config* dev_config = smc_get_device_driver_config( platform_device->id );

        if( dev_config != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_remove: platform device 0x%08X unregister net device in config 0x%08X...", (uint32_t)platform_device, (uint32_t)dev_config);

            smc_net_device_remove( dev_config->device_driver_priv );

                /* Unlink the priv from config */
            dev_config->device_driver_priv = NULL;

            /* Do no destroy the device driver config since it is from the stack */
        }
    }


    SMC_TRACE_PRINTF_DEBUG("smc_net_platform_device_remove: completed by return value 0x%02X", ret_val);

    return ret_val;
}

static struct platform_driver smc_platform_device_driver = {
    .probe    = smc_net_platform_device_probe,
    .remove   = smc_net_platform_device_remove,
    .driver   = {
        .name = SMC_PLATFORM_DRIVER_NAME,
        .owner = THIS_MODULE,
    },

};

static int __init smc_platform_device_driver_init(void)
{
    int ret_val = SMC_DRIVER_OK;

    SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_init: starts...");

    SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_init: register driver...");
    ret_val = platform_driver_register(&smc_platform_device_driver);

#if( SMCTEST == TRUE )

    if( instantiate == 1 )
    {
        if( ret_val == SMC_DRIVER_OK )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_init: Instantiate SMC platform driver...");
            platform_add_devices(devices, ARRAY_SIZE(devices));

            SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_init: SMC platform driver instantiated");
        }
        else
        {
            SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_init: SMC platform driver register failed, unable to instantiate");
        }
    }

#endif

    SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_init: completed by return value 0x%02X", ret_val);

    return ret_val;
}

static void __exit smc_platform_device_driver_exit(void)
{
    SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_exit: starts...");

    platform_driver_unregister(&smc_platform_device_driver);

    SMC_TRACE_PRINTF_DEBUG("smc_platform_device_driver_exit: completed");
}

module_init(smc_platform_device_driver_init);
module_exit(smc_platform_device_driver_exit);


#if( SMCTEST == TRUE )

#include "smc_test.h"

/*
 * SMC Test Interface functions for Linux Kernel
 */
uint8_t smc_test_linux_start(uint16_t test_case_id, uint16_t test_data_input_len, uint8_t* test_data_input)
{
    SMC_TRACE_PRINTF_INFO("smc_test_linux_start: Test case id 0x%04X, test data len %d...", test_case_id, test_data_input_len);

    return smc_test_handler_start(test_case_id, test_data_input_len, test_data_input);
}

EXPORT_SYMBOL(smc_test_linux_start);

static uint8_t smc_test_linux_start_by_phonet_msg(uint32_t phonet_msg_len, uint8_t* phonet_msg)
{
    uint8_t ret_val = SMC_OK;

    SMC_TRACE_PRINTF_DEBUG("smc_test_linux_start_by_phonet_msg: data len %d...", phonet_msg_len);
    SMC_TRACE_PRINTF_DEBUG_DATA( phonet_msg_len , phonet_msg );

    /* Remove the  header 7 bytes */

    if( phonet_msg_len > 7 )
    {
        uint32_t counter = 7;

            /* Use just one byte */
        uint16_t test_case_id  = (uint16_t)phonet_msg[counter++];
        uint16_t test_data_len = (uint16_t)(phonet_msg_len-counter);
        uint8_t* test_data     = (uint8_t*)(phonet_msg+counter);

        SMC_TRACE_PRINTF_DEBUG("smc_test_linux_start_by_phonet_msg: smc_test_linux_start: invoke test case 0x%02X with data len %d",
                test_case_id, test_data_len);
        ret_val = smc_test_linux_start( test_case_id, test_data_len, test_data);

        SMC_TRACE_PRINTF_DEBUG("smc_test_linux_start_by_phonet_msg: smc_test_linux_start completed by return value 0x%02X", ret_val );
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_test_linux_start_by_phonet_msg: not enough data for the test");
        ret_val = SMC_ERROR;
    }

    return ret_val;
}



#endif

/* EOF */
