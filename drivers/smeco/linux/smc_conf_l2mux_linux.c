/*
*   Smeco device driver L2MUX configuration module for Linux Kernel.
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

Version:       1    09-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#include "smc_common_includes.h"

#include <linux/phonet.h>
#include <linux/l2mux.h>
#include <net/phonet/pn_dev.h>

#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_phonet.h>


#include "smc.h"
#include "smc_trace.h"

#include "smc_conf_l2mux_linux.h"

#include "smc_instance_config_l2mux.h"

#if( SMCTEST == TRUE )
  #include "smc_test.h"
#endif

/*
 * Start / stop lock mechanism
 */
static smc_lock_t* g_local_lock_smc_start_stop = NULL;

static inline smc_lock_t* get_local_lock_smc_start_stop(void)
{
    if( g_local_lock_smc_start_stop == NULL ) g_local_lock_smc_start_stop = smc_lock_create();
    return g_local_lock_smc_start_stop;
}

/*
 * Callback functions for SMC
 */
static void  smc_receive_data_callback_channel_l2mux(void*   data,
                                                      int32_t data_length,
                                                      const struct _smc_user_data_t* userdata,
                                                      const struct _smc_channel_t* channel);

static void  smc_event_callback_l2mux(smc_channel_t* smc_channel, SMC_CHANNEL_EVENT event, void* event_data);

static void  smc_deallocator_callback_l2mux(smc_channel_t* smc_channel, void* ptr, struct _smc_user_data_t* userdata);

static smc_conf_t* smc_device_create_conf_l2mux(void);

static int      l2mux_net_device_driver_ioctl(struct net_device* device, struct ifreq* ifr, int cmd);
static uint16_t l2mux_net_device_driver_select_queue( struct net_device *device, struct sk_buff *skb );
static int      l2mux_modify_send_data( struct sk_buff *skb, smc_user_data_t* smc_user_data );
static void     l2mux_layer_device_driver_setup(struct net_device* dev);

/*
 * The one and only L2MUX device configuration
 */
smc_device_driver_config dev_config_l2mux =
{
    .name                    = "L2MUX",
    .queue_count             = SMC_L2MUX_QUEUE_COUNT,
    .smc_conf                = smc_device_create_conf_l2mux,
    .skb_tx_function         = l2mux_skb_tx,
    .skb_rx_function         = l2mux_skb_rx,
    .driver_ioctl            = l2mux_net_device_driver_ioctl,
    .driver_select_queue     = l2mux_net_device_driver_select_queue,
    .driver_modify_send_data = l2mux_modify_send_data,
    .driver_setup            = l2mux_layer_device_driver_setup,
    .device_driver_priv      = NULL,
};

#if 0
/* If L2MUX is not available use these dummy functions */
/* TODO FLAG OUT */
int l2mux_skb_tx( struct sk_buff *skb, struct net_device *device )
{
    SMC_TRACE_PRINTF_DEBUG("l2mux_skb_tx: DUMMY L2MUX function invoked");
    return 0;
}

int l2mux_skb_rx( struct sk_buff *skb, struct net_device *device )
{
    SMC_TRACE_PRINTF_DEBUG("l2mux_skb_rx: DUMMY L2MUX function invoked");
    return 0;
}

#endif

/*
 * Returns the specified L2MUX Configuration initialized in the smc_instance_config_l2mux.h.
 */
smc_instance_conf_t* smc_instance_conf_get_l2mux( char* smc_user_name, char* config_name )
{
    return smc_instance_conf_get_from_list(smc_instance_conf_l2mux, SMC_CONF_COUNT_L2MUX, smc_user_name, config_name);
}

smc_t* get_smc_instance_l2mux( void )
{
    smc_t*  smc_instance = NULL;

    if( dev_config_l2mux.device_driver_priv != NULL )
    {
        SMC_TRACE_PRINTF_INFO("get_smc_instance_l2mux: SMC Net Device is configured properly, retrieving SMC instance...");
        smc_instance = dev_config_l2mux.device_driver_priv->smc_instance;
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("get_smc_instance_l2mux: SMC Net Device is not configured properly. SMC instance is not available");
    }

    SMC_TRACE_PRINTF_DEBUG("get_smc_instance_l2mux: Return the SMC Instance 0x%08X...", (uint32_t)smc_instance);
    return smc_instance;
}


/*
 * Creates configuration for SMC between CA9 and MODEM
 */
static smc_conf_t* smc_device_create_conf_l2mux(void)
{
    smc_conf_t*          smc_conf                = NULL;
    smc_channel_conf_t*  smc_channel_conf        = NULL;
    smc_instance_conf_t* smc_instance_conf       = NULL;
    int                  i                       = 0;

        /* Select the SMC configuration */
    char* smc_cpu_name = NULL;
    uint8_t asic_version = smc_asic_version_get();

    if( asic_version == SMC_EOS_ASIC_ES10 )
    {
        smc_cpu_name = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES10;
    }
    else
    {
        smc_cpu_name = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2_ES20;
    }

    SMC_TRACE_PRINTF_STARTUP("L2MUX configuration '%s' for ASIC version 0x%02X (PM Host Access Req %s)", smc_cpu_name, asic_version,
            SMC_CONF_PM_APE_HOST_ACCESS_REQ_ENABLED?"enabled":"disabled");

    SMC_TRACE_PRINTF_DEBUG("smc_device_create_conf_l2mux: start...");

    smc_instance_conf = smc_instance_conf_get_l2mux( SMC_CONFIG_USER_L2MUX, smc_cpu_name );

    smc_conf = smc_conf_create_from_instance_conf( smc_cpu_name, smc_instance_conf );

    SMC_TRACE_PRINTF_DEBUG("smc_device_create_conf_l2mux: initializing callback functions for %d channels...", smc_conf->smc_channel_conf_count);

    for( i = 0; i < smc_conf->smc_channel_conf_count; i++ )
    {
        smc_channel_conf = smc_conf->smc_channel_conf_ptr_array[i];

        smc_channel_conf->smc_receive_data_cb           = (void*)smc_receive_data_callback_channel_l2mux;
        smc_channel_conf->smc_send_data_deallocator_cb  = (void*)smc_deallocator_callback_l2mux;
        smc_channel_conf->smc_receive_data_allocator_cb = NULL;
        smc_channel_conf->smc_event_cb                  = (void*)smc_event_callback_l2mux;
    }

    SMC_TRACE_PRINTF_DEBUG("smc_device_create_conf_l2mux: completed, return SMC instance configuration 0x%08X", (uint32_t)smc_conf);

    return smc_conf;
}

static void smc_deallocator_callback_l2mux(smc_channel_t* smc_channel, void* ptr, struct _smc_user_data_t* userdata)
{
	SMC_TRACE_PRINTF_DEBUG("smc_deallocator_callback_l2mux: do not deallocate SKB data 0x%08X", (uint32_t)ptr);
}

static void  smc_receive_data_callback_channel_l2mux(void*   data,
                                                     int32_t data_length,
                                                     const struct _smc_user_data_t* userdata,
                                                     const struct _smc_channel_t* channel)
{
    struct net_device* device = NULL;

    assert( channel!=NULL );

    SMC_TRACE_PRINTF_INFO("smc_receive_data_callback_channel_l2mux: Data 0x%08X from SMC 0x%08X: channel %d, len %d",
                            (uint32_t)data, (uint32_t)channel->smc_instance, channel->id, data_length);

    SMC_TRACE_PRINTF_DEBUG_DATA(data_length, data);

    device = dev_config_l2mux.device_driver_priv->net_dev;

    if( device != NULL )
    {
        SMC_TRACE_PRINTF_INFO("smc_receive_data_callback_channel_l2mux: net device is 0x%08X", (uint32_t)device);

        if( unlikely( !netif_running(device) ) )
        {
            SMC_TRACE_PRINTF_WARNING("smc_receive_data_callback_channel_l2mux: Device 0x%08X not running, drop RX packet", (uint32_t)device);
            device->stats.rx_dropped++;
        }
        else
        {
            struct sk_buff *skb = NULL;

            /* ========================================
             * Critical section begins
             *
             */
            SMC_LOCK( channel->lock_read );

            skb = netdev_alloc_skb( device, data_length + SMC_L2MUX_HEADER_SIZE );

            if( unlikely(!skb) )
            {
                SMC_TRACE_PRINTF_ERROR("smc_receive_data_callback_channel_l2mux: No memory for RX skb");
                device->stats.rx_dropped++;

                SMC_UNLOCK( channel->lock_read );
                /*
                 * Critical section ends
                 * ========================================
                 */
            }
            else
            {
                smc_device_driver_priv_t* smc_net_dev = NULL;
                char* skb_data_buffer = NULL;
                
                skb_data_buffer = skb_put(skb, data_length);

                SMC_TRACE_PRINTF_DEBUG("smc_receive_data_callback_channel_l2mux: Copy Message data 0x%08X into the SKB buffer 0x%08X (0x%08X)...",
                        (uint32_t)data, (uint32_t)skb->data, (uint32_t)skb_data_buffer);


                /* TODO Cache control */

                memcpy( skb_data_buffer, data, data_length);

                /* Ensure the cache cleanup */
                SMC_TRACE_PRINTF_DEBUG("smc_receive_data_callback_channel_l2mux clean 0x%08X->0x%08X (len %d)",
                        (uint32_t)skb_data_buffer, (uint32_t)(skb_data_buffer+data_length), data_length );

                __raw_readl( ((void __iomem *)(skb_data_buffer+data_length)) );

                SMC_TRACE_PRINTF_DEBUG("smc_receive_data_callback_channel_l2mux: free the original data 0x%08X", (uint32_t)data);

                /* TODO Create common free function for freeing RECEIVE data -> detects there if data is from remote */

                if( SMC_COPY_SCHEME_RECEIVE_IS_COPY( channel->copy_scheme ) )
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_receive_data_callback_channel_l2mux: free the original data 0x%08X from local mem", (uint32_t)data);
                    smc_channel_free_ptr_local( (smc_channel_t*)channel, data, (smc_user_data_t*)userdata );
                }
                else
                {
                    smc_user_data_t userdata_free;

                    userdata_free.flags     = SMC_MSG_FLAG_FREE_MEM_MDB;
                    userdata_free.userdata1 = userdata->userdata1;
                    userdata_free.userdata2 = userdata->userdata2;
                    userdata_free.userdata3 = userdata->userdata3;
                    userdata_free.userdata4 = userdata->userdata4;
                    userdata_free.userdata5 = userdata->userdata5;

                    SMC_TRACE_PRINTF_DEBUG("smc_receive_data_callback_channel_l2mux: free the original data 0x%08X from SHM...", (uint32_t)data);

                        /* Free the MDB SHM data PTR from remote */
                    if( smc_send_ext( (smc_channel_t*)channel, data, 0, &userdata_free) != SMC_OK )
                    {
                        SMC_TRACE_PRINTF_ERROR("smc_receive_data_callback_channel_l2mux: channel %d: MDB memory 0x%08X free from remote failed",
                                channel->id, (uint32_t)data);
                    }
                }

                /* TODO Implement the fragments for SKB ( hsi_logical_skb_to_msg in hsi_locigal.c ) */

                skb_push(skb, SMC_L2MUX_HEADER_SIZE);

                SMC_TRACE_PRINTF_INFO("smc_receive_data_callback_channel_l2mux: Push L2MUX header 0x%08X into the SKB 0x%08X",
                        userdata->userdata1, (uint32_t)skb->data);

                *(uint32_t*)(skb->data) = userdata->userdata1;

                    /* Put the metadata */
                skb->dev        = device;
                /* TODO Check to put protocol ETH_xxx from channel information */
                /*skb->protocol = eth_type_trans(skb, dev);*/
                skb->ip_summed  = CHECKSUM_UNNECESSARY;

                smc_net_dev = netdev_priv( device );

                device->stats.rx_packets++;

                SMC_UNLOCK( channel->lock_read );
                /*
                 * Critical section ends
                 * ========================================
                 */

                SMC_TRACE_PRINTF_INFO("smc_receive_data_callback_channel_l2mux: Deliver SKB (length %d) to upper layer RX ...", skb->len);
                SMC_TRACE_PRINTF_INFO_DATA( skb->len , skb->data );

                smc_net_dev->smc_dev_config->skb_rx_function( skb, device );
            }
        }
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_receive_data_callback_channel_l2mux: unable to get proper net device");
    }

    SMC_TRACE_PRINTF_DEBUG("smc_receive_data_callback_channel_l2mux: completed");
}

static void smc_event_callback_l2mux(smc_channel_t* smc_channel, SMC_CHANNEL_EVENT event, void* event_data)
{
    assert(smc_channel != NULL );

    switch(event)
    {
        case SMC_CHANNEL_READY_TO_SEND:
        {
            SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_event_callback_l2mux: channel id %d: SMC_CHANNEL_READY_TO_SEND", smc_channel->id);

            SMC_TRACE_PRINTF_STARTUP("Channel %d is synchronized with the remote", smc_channel->id);

            break;
        }
        case SMC_STOP_SEND:
        {
            struct net_device* device     = NULL;
            smc_lock_t*        local_lock = get_local_lock_smc_start_stop();

            SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_event_callback_l2mux: channel id %d: SMC_STOP_SEND", smc_channel->id);

            SMC_LOCK( local_lock );

            assert( smc_channel->stop_counter < 0xFF );

            smc_channel->stop_counter++;

                /* Get the net device and close */
            device = dev_config_l2mux.device_driver_priv->net_dev;

            if( device != NULL )
            {
                struct netdev_queue* queue     = NULL;
                uint16_t             queue_ind = smc_channel->protocol;

                SMC_TRACE_PRINTF_DEBUG("smc_event_callback_l2mux: channel id %d: netif_tx_stop_queue %d...", smc_channel->id, queue_ind);

                /* Get the QUEUE and stop it */
                queue = netdev_get_tx_queue(device, queue_ind);
                netif_tx_stop_queue(queue);

                SMC_CHANNEL_STATE_SET_SEND_IS_DISABLED( smc_channel->state );

                //netif_tx_stop_all_queues(device);
            }
            else
            {
                SMC_TRACE_PRINTF_ASSERT("mc_event_callback_l2mux: channel id %d: Net device not found", smc_channel->id);
                assert(0);
            }

            SMC_UNLOCK( local_lock );

            break;
        }
        case SMC_RESUME_SEND:
        {
            struct net_device* device     = NULL;
            smc_lock_t*        local_lock = get_local_lock_smc_start_stop();

            SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_event_callback_l2mux: channel id %d: SMC_RESUME_SEND", smc_channel->id);

            SMC_LOCK( local_lock );

            if( smc_channel->stop_counter > 0 ) smc_channel->stop_counter--;

            if( smc_channel->stop_counter==0 )
            {
                    /* Get the net device and close */
                device = dev_config_l2mux.device_driver_priv->net_dev;

                if( device != NULL )
                {
                    struct netdev_queue* queue     = NULL;
                    uint16_t             queue_ind = smc_channel->protocol;

                    SMC_TRACE_PRINTF_DEBUG("smc_event_callback_l2mux: channel id %d: netif_tx_start_queue %d...", smc_channel->id, queue_ind);

                    queue = netdev_get_tx_queue(device, queue_ind);
                    netif_tx_start_queue(queue);

                    SMC_CHANNEL_STATE_CLEAR_SEND_IS_DISABLED( smc_channel->state );
                }
                else
                {
                    SMC_TRACE_PRINTF_ASSERT("mc_event_callback_l2mux: channel id %d: Net device not found", smc_channel->id);
                    assert(0);
                }
            }
            else
            {
                SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_event_callback_l2mux: channels %d lock counter is %d, not starting queue %d", smc_channel->id, smc_channel->stop_counter, smc_channel->protocol);
            }

            SMC_UNLOCK( local_lock );

            break;
        }
        case SMC_VERSION_INFO_REMOTE:
        {
            uint32_t version           = *(uint32_t*)event_data;
            char*    str_version       = smc_version_to_str(version);
            char*    str_version_local = smc_get_version();
            uint32_t version_local     = smc_version_to_int( str_version_local );

            if( version_local <= 0 )
            {
                version_local = version_local;  /* Remove warning */
            }

            SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_l2mux_event_handler: channel id %d: SMC_VERSION_INFO_REMOTE, version is 0x%08X -> v.%s, local version: 0x%08X -> v.%s",
                    smc_channel->id, version, str_version, version_local, str_version_local);

            SMC_TRACE_PRINTF_STARTUP("Channel %d version info: local v.%s, remote v.%s", smc_channel->id,str_version_local, str_version);

            /* TODO Implement version compare and decide if it is possible to communicate */

            SMC_FREE( str_version );

            break;
        }
        default:
        {
            SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_event_callback_l2mux: channel id %d: event %d, no actions", smc_channel->id, event);
        }
    }
}

static int l2mux_net_device_driver_ioctl(struct net_device* device, struct ifreq* ifr, int cmd)
{
    int ret_val = SMC_DRIVER_OK;

    SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: starts device 0x%08X, command %d (0x%08X)...", (uint32_t)device, cmd, cmd);

    switch(cmd)
    {
        case SIOCPNGAUTOCONF:
        {
            struct if_phonet_req *req = (struct if_phonet_req *)ifr;
            uint8_t address = 0x00;

            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCPNGAUTOCONF");

            req->ifr_phonet_autoconf.device = PN_DEV_HOST;

            address = 0x60;
            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCPNGAUTOCONF: Add route 0x%02X...", address);
            phonet_route_add(device, address);

            address = 0x44;
            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCPNGAUTOCONF: Add route 0x%02X...", address);
            phonet_route_add(device, address);

            address = 0x64;
            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCPNGAUTOCONF: Add route 0x%02X...", address);
            phonet_route_add(device, address);

            break;
        }
        case SIOCCONFIGTYPE:
        {
            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCCONFIGTYPE");
            break;
        }
        case SIOCCONFIGSUBTYPE:
        {
            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCCONFIGSUBTYPE");
            break;
        }
        case SIOCPNGETOBJECT:
        {
            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCPNGETOBJECT");
            break;
        }
#if(SMCTEST==TRUE)
        case SIOCDEV_RUN_TEST:
        {
            struct ifreq_smc_test* if_req_smc_test = (struct ifreq_smc_test *)ifr;

            SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCDEV_RUN_TEST: test case 0x%02X, test data length = %d, test data 0x%08X",
                    if_req_smc_test->if_test_case ,if_req_smc_test->if_test_data_len, (uint32_t)if_req_smc_test->if_test_data);

            if(if_req_smc_test->if_test_data_len > 0 && if_req_smc_test->if_test_data != NULL )
            {
                SMC_TRACE_PRINTF_DEBUG_DATA( if_req_smc_test->if_test_data_len, if_req_smc_test->if_test_data );
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCDEV_RUN_TEST: No test input data found");
            }

            if( smc_test_handler_start(if_req_smc_test->if_test_case, if_req_smc_test->if_test_data_len, if_req_smc_test->if_test_data) == SMC_OK )
            {
                SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: SIOCDEV_RUN_TEST: test 0x%02X OK", if_req_smc_test->if_test_case);

                if_req_smc_test->if_test_result = 0xABCD;
                ret_val = SMC_DRIVER_OK;
            }
            else
            {
                SMC_TRACE_PRINTF_ERROR("l2mux_net_device_driver_ioctl: SIOCDEV_RUN_TEST: test 0x%02X FAILED", if_req_smc_test->if_test_case);
                if_req_smc_test->if_test_result = 0xCDEF;
                ret_val = SMC_DRIVER_ERROR;
            }

            break;
        }
#endif
        default:
        {
            SMC_TRACE_PRINTF_WARNING("l2mux_net_device_driver_ioctl: unsupported ioctl command 0x%04X", cmd);
            break;
        }
    }

    SMC_TRACE_PRINTF_DEBUG("l2mux_net_device_driver_ioctl: completed by return value %d", ret_val);

    return ret_val;
}

static uint16_t l2mux_net_device_driver_select_queue( struct net_device *device, struct sk_buff *skb )
{
    uint16_t subqueue = 0;

    SMC_TRACE_PRINTF_TRANSMIT("l2mux_net_device_driver_select_queue: device 0x%08X, protocol 0x%04X...", (uint32_t)device, skb->protocol);

    if (skb->protocol == htons(ETH_P_PHONET))
    {
        SMC_TRACE_PRINTF_TRANSMIT("l2mux_net_device_driver_select_queue: protocol ETH_P_PHONET");
        subqueue = SMC_L2MUX_QUEUE_1_PHONET;
    }
    else if (skb->protocol == htons(ETH_P_MHI))
    {
        SMC_TRACE_PRINTF_TRANSMIT("l2mux_net_device_driver_select_queue: protocol ETH_P_MHI");

            /* No audio, return medium */
        subqueue = SMC_L2MUX_QUEUE_1_PHONET;
    }
    else if (skb->protocol == htons(ETH_P_MHDP))
    {
        SMC_TRACE_PRINTF_TRANSMIT("l2mux_net_device_driver_select_queue: protocol ETH_P_MHDP");
        subqueue = SMC_L2MUX_QUEUE_3_MHDP;
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("l2mux_net_device_driver_select_queue: unsupported protocol device 0x%08X, 0x%04X", (uint32_t)device, skb->protocol);
    }

    SMC_TRACE_PRINTF_TRANSMIT("l2mux_net_device_driver_select_queue: completed: subqueue 0x%04X", subqueue);

    return subqueue;
}

static int l2mux_modify_send_data( struct sk_buff *skb, smc_user_data_t* smc_user_data )
{
    SMC_TRACE_PRINTF_TRANSMIT("l2mux_modify_send_data: get L2MUX header from SKB and put it into SMC user data...");

    smc_user_data->userdata1 = *(uint32_t*)skb->data;

    skb_pull(skb, SMC_L2MUX_HEADER_SIZE);

    return SMC_DRIVER_OK;
}

static void l2mux_layer_device_driver_setup(struct net_device* device)
{
    SMC_TRACE_PRINTF_DEBUG("l2mux_layer_device_driver_setup: modify net device for L2MUX usage...");

    device->features        = NETIF_F_SG /* Frags to be tested by MHDP team  | NETIF_F_HW_CSUM | NETIF_F_FRAGLIST*/;
    device->type            = ARPHRD_MHI;
    device->flags           = IFF_POINTOPOINT | IFF_NOARP;
    device->mtu             = MHI_MAX_MTU;
    device->hard_header_len = 4;
    device->dev_addr[0]     = PN_MEDIA_MODEM_HOST_IF;
    device->addr_len        = 1;
    device->tx_queue_len    = 500;

}

/* EOF */
