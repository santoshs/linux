/*
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

Version:       1    04-Dec-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#include "smc_common_includes.h"

#include "smc_conf.h"
#include "smc_trace.h"
#include "smc_fifo.h"
#include "smc.h"
#include "smc_memory_mgmt.h"
#include "smc_test.h"

    /* ===============================
     * DMA test cases for Linux Kernel
     */
#ifdef SMECO_LINUX_KERNEL

#include "smc_linux.h"
#include <linux/sh_dma.h>
#include <linux/dmaengine.h>


smc_device_driver_priv_t* g_smc_net_dev_priv = NULL;

static int     smc_test_dma_request_start(smc_device_driver_priv_t* smc_net_dev_priv, const void *buf, unsigned int buf_size);
static uint8_t smc_test_dma_filter(struct dma_chan *chan, void *slave);
static void    smc_test_dma_transfer_complete_cb(void *param);
static void    smc_test_dma_request_release(void);


/**
 * Initializes the SMC device to used in DMA test cases.
 */
void set_smc_device_driver_priv(smc_device_driver_priv_t* smc_net_dev_priv)
{
    g_smc_net_dev_priv = smc_net_dev_priv;
}


uint8_t smc_test_case_dma( uint8_t* test_input_data, uint32_t test_input_data_len )
{
    uint8_t test_status = SMC_ERROR;

    if( g_smc_net_dev_priv != NULL )
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: DMA test invoke from device '%s'", g_smc_net_dev_priv->net_dev->name);

        if( smc_test_dma_request_start( g_smc_net_dev_priv, (const void*)test_input_data, (unsigned int)test_input_data_len ) == SMC_DRIVER_OK )
        {
            SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: smc_test_dma_request_start succeeded");
            test_status = SMC_OK;
        }
        else
        {
            SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: smc_test_dma_request_start failed");
            test_status = SMC_ERROR;
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_dma: The SMC device is not set, unable to run DMA test");
    }

    return test_status;
}

static int smc_test_dma_request_start(smc_device_driver_priv_t* smc_net_dev_priv, const void *buf, unsigned int buf_size)
{
    int ret_value = SMC_DRIVER_OK;

    struct dma_chan*      dma_channel = NULL;
    dma_cap_mask_t        mask;
    struct sh_dmae_slave* dma_user_fn_param = NULL;

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_request_start: Data 0x%08X, len %d", (uint32_t)buf, buf_size);

    /* Initialize the DMA transfer */

    //dma_user_fn_param = kzalloc( sizeof(struct sh_dmae_slave), GFP_KERNEL);
    //dma_user_fn_param->slave_id = ;

    dma_cap_zero(mask);
    dma_cap_set(DMA_SLAVE, mask);

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_request_start: dma_request_channel dma_user_fn_param=0x%08X", (uint32_t)dma_user_fn_param);

    dma_channel = dma_request_channel(mask, (dma_filter_fn)smc_test_dma_filter, dma_user_fn_param);

    if( dma_channel != NULL )
    {
        struct scatterlist *tx_sg = NULL;
        int nent = 0;
        struct dma_async_tx_descriptor* tx_desc = NULL;
        dma_cookie_t tx_cookie;

        SMC_TEST_TRACE_PRINTF_INFO( "smc_test_dma_request_start: DMA channel 0x%08X received", (uint32_t)dma_channel );

        tx_sg = kmalloc(sizeof(struct scatterlist),GFP_KERNEL);

        sg_init_table(tx_sg, 1);
        sg_set_page(tx_sg, virt_to_page(buf), PAGE_SIZE, offset_in_page(buf));

        SMC_TEST_TRACE_PRINTF_INFO( "smc_test_dma_request_start: dma_map_sg( tx_sq=0x%08X )", (uint32_t)tx_sg);

        nent = dma_map_sg(&smc_net_dev_priv->platform_device->dev, tx_sg, 1, DMA_TO_DEVICE);

        //sg_dma_address(tx_sg) = (sg_dma_address(tx_sg) & ~(PAGE_SIZE  - 1));

        SMC_TEST_TRACE_PRINTF_INFO( "smc_test_dma_request_start: call device_prep_slave_sg(tx_sg=0x%08X, nent=%d) ...", (uint32_t)tx_sg, nent );

        tx_desc = dma_channel->device->device_prep_slave_sg(dma_channel, tx_sg, nent, DMA_TO_DEVICE, DMA_PREP_INTERRUPT|DMA_CTRL_ACK);

        if( tx_desc != NULL )
        {
            tx_desc->callback = smc_test_dma_transfer_complete_cb;
            tx_desc->callback_param = dma_channel;

            SMC_TEST_TRACE_PRINTF_INFO( "smc_test_dma_request_start: call tx_desc->tx_submit()..." );

            tx_cookie = tx_desc->tx_submit(tx_desc);

            SMC_TEST_TRACE_PRINTF_INFO( "smc_test_dma_request_start: call dma_async_issue_pending()..." );

            dma_async_issue_pending(dma_channel);
        }
        else
        {
            SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_request_start: failed in device_prep_slave_sg");
            ret_value = SMC_DRIVER_ERROR;
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_request_start: failed to get DMA channel");
        ret_value = SMC_DRIVER_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_ERROR( "smc_test_dma_request_start: completed by ret val %d", ret_value);

    return ret_value;
}


static uint8_t smc_test_dma_filter(struct dma_chan *chan, void *slave)
{
    struct sh_dmae_slave *param = slave;

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_filter: Channel 0x%08X, Slave 0x%08X", (uint32_t)chan, (uint32_t)slave);


    chan->private = param;


    return TRUE;
}

static void smc_test_dma_transfer_complete_cb(void *param)
{
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer_complete_cb: param = 0x%08X", (uint32_t)param);
}



static void smc_test_dma_request_release( void )
{
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_request_release: invoked");

}

#else   /* #ifdef SMECO_LINUX_KERNEL */

uint8_t smc_test_case_dma( uint8_t* test_input_data, uint32_t test_input_data_len )
{
    uint8_t test_status = SMC_ERROR;


    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_dma: Test not supported");

    return test_status;
}

#endif


/* EOF */

