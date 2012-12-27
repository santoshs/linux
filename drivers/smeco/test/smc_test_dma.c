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
#include "smc_mdb.h"

    /* ===============================
     * DMA test cases for Linux Kernel
     */
#ifdef SMECO_LINUX_KERNEL

#include "smc_linux.h"
#include <linux/sh_dma.h>
#include <linux/dmaengine.h>
#include <asm/memory.h>

smc_device_driver_priv_t* g_smc_net_dev_priv = NULL;

static int     smc_test_dma_transfer(struct dma_chan* dma_channel, void* target_address, void* source_address, uint32_t length);
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
        dma_cap_mask_t     mask;
        struct dma_chan*   dma_channel = NULL;

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: DMA test invoke from device '%s'", g_smc_net_dev_priv->net_dev->name);

        dma_cap_zero(mask);
        dma_cap_set(DMA_SLAVE, mask);

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: request DMA channel...");

        dma_channel = dma_request_channel(mask, (dma_filter_fn)smc_test_dma_filter, NULL);

        if( dma_channel )
        {
            smc_user_data_t userdata;

                /* Use SMC channel 0 */
            smc_channel_t* smc_channel = SMC_CHANNEL_GET(g_smc_net_dev_priv->smc_instance, 0);

            void* source_address = smc_mdb_alloc(smc_channel, test_input_data_len);

            memcpy(source_address, test_input_data, test_input_data_len);

            SMC_SHM_CACHE_CLEAN( source_address, ((void*)(((uint32_t)source_address)+test_input_data_len)) );

            void* target_address = kmalloc(test_input_data_len, GFP_KERNEL);

            SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: start %d bytes DMA transfer from 0x%08X to 0x%08X...", test_input_data_len, (uint32_t)source_address, (uint32_t)target_address);

            if( smc_test_dma_transfer( dma_channel, target_address, source_address, test_input_data_len) == SMC_DRIVER_OK )
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: %d bytes DMA transfer succeeded, source (0x%08X)/mdb (0x%08X)/target (0x%08X) data:", test_input_data_len, test_input_data, source_address, target_address);
                SMC_TEST_TRACE_PRINTF_INFO_DATA( test_input_data_len, test_input_data );
                SMC_TEST_TRACE_PRINTF_INFO_DATA( test_input_data_len, source_address );

                SMC_SHM_CACHE_INVALIDATE( data, ((void*)(((uint32_t)target_address) + test_input_data_len)) );

                SMC_TEST_TRACE_PRINTF_INFO_DATA( test_input_data_len, target_address );

                test_status = SMC_OK;
            }
            else
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: DMA transfer failed");
                test_status = SMC_ERROR;
            }

            SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: release DMA channel...");
            dma_release_channel(dma_channel);

            SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: release allocated memory...");
            kfree(target_address);

            SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: release MDB memory...");
            smc_channel_free_ptr_local( smc_channel, source_address, &userdata);
         }
         else
         {
             SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_dma: DMA channel request failed");
         }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_dma: The SMC device is not set, unable to run DMA test");
    }

    return test_status;
}

static int smc_test_dma_transfer(struct dma_chan* dma_channel, void* target_address, void* source_address, uint32_t length)
{
    int                             ret_value         = SMC_DRIVER_OK;
    struct dma_async_tx_descriptor *dma_tx_descriptor = NULL;

    enum dma_ctrl_flags             flags;
    dma_addr_t                      dma_source_addr;
    dma_addr_t                      dma_target_addr;
    struct dma_device*              device            = dma_channel->device;
    struct page*                    page_source       = NULL;

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: Transfer %d bytes from 0x%08X to 0x%08X", length, source_address, target_address);

    if( device == NULL )
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_transfer: invalid DMA device NULL");
        return SMC_DRIVER_ERROR;
    }

    if( dma_channel == NULL )
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_transfer: invalid DMA channel NULL");
        return SMC_DRIVER_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: DMA map: PAGE_OFFSET=0x%08X, highmemory=0%08X", PAGE_OFFSET, high_memory);

        /* Map DMA source and target */
    if( source_address < high_memory )
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: map DMA source from 0x%08X, len %d...", source_address, length);

            /* DMA_FROM_DEVICE == from SHM / DMA_TO_DEVICE == to SHM*/
        dma_source_addr = dma_map_single(device->dev, source_address, length, DMA_FROM_DEVICE);     /* DMA_TO_DEVICE */
    }
    else
    {
            /* SHM in highmem*/
        unsigned long offset_source       = 0;
        unsigned long page_source_address = 0;

        page_source = virt_to_page(source_address);

        //get_page(page_source);

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: highmem usage: Virtual address of page 0x%08X == 0x%08X, PAGE_MASK==0x%08X", (uint32_t)page_source, (uint32_t)page_address(page_source), PAGE_MASK);

        page_source_address = kmap_high( page_source );
        offset_source = ((unsigned long)source_address) & ~PAGE_MASK;

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: highmem usage: virtual address of page 0x%08X from kmap_high == 0x%08X", (uint32_t)page_source, page_source_address );

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: %d bytes from virtual address 0x%08X:", length, ((uint32_t)page_source_address)+offset_source);
        SMC_TEST_TRACE_PRINTF_INFO_DATA( length, ((uint32_t)page_source_address)+offset_source );

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: map DMA source from 0x%08X (highmem), len %d, page=0x%08X (address 0x%08x), offset 0x%08X...",
                source_address, length, (uint32_t)page_source, page_source_address, offset_source);

        dma_source_addr = dma_map_page(device->dev, page_source, offset_source, length, DMA_FROM_DEVICE);     /* DMA_TO_DEVICE / DMA_FROM_DEVICE */
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: map DMA target from 0x%08X, len %d...", target_address, length);
    dma_target_addr = dma_map_single(device->dev, target_address, length, DMA_BIDIRECTIONAL);   /* Force invalidate*/

        /* Set control flags:
         * (src buffers are freed by the DMAEngine code with dma_unmap_single())
         * src buffers are freed by ourselves
         * dst buffers are freed by ourselves
         */
    //flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT | DMA_COMPL_SKIP_DEST_UNMAP | DMA_COMPL_SRC_UNMAP_SINGLE;
    flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT | DMA_COMPL_SKIP_DEST_UNMAP; //| DMA_COMPL_SKIP_SRC_UNMAP;

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: prepare DMA memcpy...");
    dma_tx_descriptor = device->device_prep_dma_memcpy(dma_channel, dma_target_addr, dma_source_addr, length, flags);

    if( dma_tx_descriptor )
    {
        static int        timeout = 1000;      /* 1 sec timeout */
        struct completion cmp;
        dma_cookie_t      cookie;
        enum dma_status   status;
        unsigned long     tmo = msecs_to_jiffies(timeout);

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: DMA descriptor OK, initialize completion...");

        init_completion(&cmp);
        dma_tx_descriptor->callback = smc_test_dma_transfer_complete_cb;
        dma_tx_descriptor->callback_param = &cmp;
        cookie = dma_tx_descriptor->tx_submit(dma_tx_descriptor);

        if( dma_submit_error(cookie) )
        {
            SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_transfer: DMA submit error cookie");
        }

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: start DMA transfer...");
        dma_async_issue_pending(dma_channel);

        tmo    = wait_for_completion_timeout(&cmp, tmo);
        status = dma_async_is_tx_complete(dma_channel, cookie, NULL, NULL);

        if (tmo == 0)
        {
            SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_transfer: DMA transfer failed by timeout");
            ret_value = SMC_DRIVER_ERROR;
        }
        else if (status != DMA_SUCCESS)
        {
            SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_transfer: DMA transfer failed by error %d", status);
            ret_value = SMC_DRIVER_ERROR;
        }

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: unmap target DMA handle...");
        dma_unmap_single(device->dev, dma_target_addr, length, DMA_BIDIRECTIONAL);  /**/

        if( page_source != NULL && source_address >= high_memory )
        {
            SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: unmap highmemory virtual address before page unmap...");
            kunmap_high( page_source );
        }

        /*
        if( page_source != NULL && source_address >= high_memory )
        {
            SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: unmap source DMA handle...");
            dma_unmap_page(device->dev, dma_source_addr, length, DMA_FROM_DEVICE);
        }
        */

        page_source = NULL;
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_dma_transfer: DMA descriptor NULL");
        ret_value = SMC_DRIVER_ERROR;
    }

    if( page_source != NULL && source_address >= high_memory )
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: unmap highmemory virtual address...");
        kunmap_high( page_source );
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer: Transfer of %d bytes from 0x%08X to 0x%08X completed by ret_value %d", length, source_address, target_address, ret_value);
    return ret_value;
}


/**
 * OBSOLETE FUNCTION CURRENTLY
 */
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
    //struct sh_dmae_slave *param = slave;

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_filter: Channel 0x%08X, Slave 0x%08X", (uint32_t)chan, (uint32_t)slave);


    //chan->private = param;


    return TRUE;
}

static void smc_test_dma_transfer_complete_cb(void *param)
{
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_dma_transfer_complete_cb: param = 0x%08X -> complete", (uint32_t)param);

    complete(param);


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

