/*
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

Version:       1    19-Oct-2011     Heikki Siikaluoma
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

#if(SMC_L2MUX_IF==TRUE)
#include "smc_instance_config_l2mux.h"  /* For testing configuration management */
#endif

#ifdef SMECO_LINUX_ANDROID
    /* Nothing for Android at the moment */
#elif defined SMECO_LINUX_KERNEL

#else

#include "pn_const.h"

#if (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L2)

#include "pn.h"
#include "pn_name.h"

#elif (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L1)

#include "mmsgr.h"

#endif

#endif

modem_test_group_id_t smc_test_handler_group_id        = SMC_TEST_GROUP_ID;
modem_test_group_id_t smc_test_handler_group_id_remote = SMC_TEST_GROUP_ID_REMOTE;



static uint8_t smc_test_case_function_configure_shared_mem( uint8_t* test_input_data, uint16_t test_input_data_len );


    /*
     * SMC instance creation for test cases
     */
static smc_t* g_smc_instance1 = NULL;

static smc_t* get_smc_instance_1( uint8_t is_master, uint32_t shm_start_address );

static smc_channel_conf_t* smc_test_case_create_channel_conf_1(uint8_t is_master);
static void  smc_receive_data_callback_channel_conf_1(void*   data,
                                                      int32_t data_length,
                                                      const struct _smc_user_data_t* userdata,
                                                      const struct _smc_channel_t* channel);

static void  smc_test_case_channel_1_deallocator_callback(smc_channel_t* smc_channel, void* ptr, struct _smc_user_data_t* userdata);
static void* smc_test_case_channel_1_allocator_callback(smc_channel_t* smc_channel, uint32_t size, struct _smc_user_data_t* userdata);
static void  smc_test_case_channel_1_event_callback(smc_channel_t* smc_channel, SMC_CHANNEL_EVENT event);

static smc_channel_conf_t* smc_test_case_create_channel_conf_2(uint8_t is_master);
static uint8_t* smc_test_case_create_shm_conf_local( smc_conf_t* smc_conf, uint8_t* shm_mem_start_addr );


uint8_t smc_test_handler_send_message(uint16_t test_case_id, uint8_t* test_input_data, uint16_t test_input_data_len);



    /* Test configuration function used as testcase */
uint8_t smc_test_case_function_configure   ( uint8_t* test_input_data, uint16_t test_input_data_len );
uint8_t smc_test_case_function_smc_instance( uint8_t* test_input_data, uint16_t test_input_data_len );
uint8_t smc_test_case_function_remote_event( uint8_t* test_input_data, uint16_t test_input_data_len );

    /* Signal test case */
static uint8_t smc_test_case_function_signal( uint8_t* test_input_data, uint16_t test_input_data_len );

    /* Configuration instance test case */
static uint8_t smc_test_case_function_create_configuration( uint8_t* test_input_data, uint16_t test_input_data_len );

    /* Shared memory test case */
static uint8_t smc_test_case_function_shm( uint8_t* test_input_data, uint16_t test_input_data_len );

    /* ========================================================
     * SMC Test Case functions
     * First index is 0x00
     * Index is the test case ID inside SMC test handler
     */
smc_test_case_function smc_test_cases[] =
{
    smc_test_case_function_configure,               /* 0x00 */
    smc_test_case_function_fifo,                    /* 0x01 in smc_test_fifo.c */
    smc_test_case_function_smc_instance,            /* 0x02 */
    smc_test_case_function_messaging,               /* 0x03 */
    smc_test_case_function_signal,                  /* 0x04 */
    smc_test_case_function_mdb,                     /* 0x05 in smc_test_mdb.c */
    smc_test_case_function_remote_event,            /* 0x06 */
    smc_test_case_function_create_configuration,    /* 0x07 */
    smc_test_case_function_shm,                     /* 0x08 */
    0
};


/*
 * SMC Test Handler ID (in modem BFAT):
 * #define SMC_TEST_HDLR   31  (0x1F)
 */
modem_test_status_t smc_test_handler(modem_test_req_str_t *req)
{
    modem_test_status_t test_status = MODEM_TEST_FAILED;

    (void)modem_test_set_timeout(2000);

    SMC_TEST_TRACE_PRINTF_INFO("Testcase ID 0x%02X, INP Data len %d, data from 0x%08X", req->case_ID, req->inp_data_length, (uint32_t)req->inp_data);
    SMC_TEST_TRACE_PRINTF_INFO_DATA(req->inp_data_length, req->inp_data);

    if( req->case_ID < ((uint16_t)(sizeof(smc_test_cases)/sizeof(smc_test_case_function))) )
    {
        if( smc_test_cases[req->case_ID] != NULL )
        {
            int test_ret = smc_test_cases[req->case_ID]( req->inp_data, req->inp_data_length );

            if( test_ret == SMC_OK )
            {
                SMC_TEST_TRACE_PRINTF_INFO("Test Case 0x%02X passed", req->case_ID);
                test_status = MODEM_TEST_PASSED;
            }
            else
            {
                SMC_TEST_TRACE_PRINTF_INFO("Test Case 0x%02X FAILED!", req->case_ID);
                test_status = MODEM_TEST_FAILED;
            }
        }
        else
        {
            SMC_TEST_TRACE_PRINTF_INFO("Test Case 0x%02X function invalid (NULL)", req->case_ID);
            test_status = MODEM_TEST_FAILED;
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("Test Case 0x%02X DOES NOT EXIST", req->case_ID);
        test_status = MODEM_TEST_FAILED;
    }

    return test_status;
}

uint8_t smc_test_handler_start(uint16_t test_case_id, uint16_t test_data_input_len, uint8_t* test_data_input)
{
    modem_test_status_t status;
    modem_test_req_str_t* req = (modem_test_req_str_t*)SMC_MALLOC(sizeof(modem_test_req_str_t));

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_handler_start: initializing...");

    req->case_ID = test_case_id;
    req->inp_data_length = test_data_input_len;
    req->inp_data = test_data_input;

    status = smc_test_handler(req);

    SMC_FREE(req);

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_handler_start: completed by status 0x%02X", status);

    if( status == MODEM_TEST_PASSED )
    {
        return SMC_OK;
    }
    else
    {
        return SMC_ERROR;
    }
}


/**
 * SMC test case for configuring the test environment.
 *
 * Test Input Data format header:
 *  byte[0]  == config message
 *
 * -> Message specific data might require more input data
 */
uint8_t smc_test_case_function_configure( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t  test_status = SMC_ERROR;
    uint16_t test_input_len_required = 1;

    if( test_input_data_len >= test_input_len_required )
    {
        uint8_t data_index = 0;

        uint8_t config_msg =  test_input_data[data_index];
        data_index++;
        test_input_data_len--;

        switch(config_msg)
        {
            case SMC_TEST_CONFIG_SET_SHARED_SDRAM:
            {
                test_status = smc_test_case_function_configure_shared_mem( (uint8_t*)(test_input_data+data_index), test_input_data_len);
                break;
            }
            default:
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_configure: unknown configure message 0x%02X", config_msg);
                test_status = SMC_ERROR;
            }
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_configure: not enough test input data (received %d, expected %d)",
                                    test_input_data_len, test_input_len_required);
        test_status = SMC_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_configure: completed with return value 0x%02X", test_status);

    return test_status;
}

static uint8_t smc_test_case_function_signal( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t  test_status = SMC_ERROR;

    uint16_t test_input_len_required = 9;

    if( test_input_data_len >= test_input_len_required )
    {
        uint8_t  data_index  = 0;
        uint32_t signal_id   = 0;
        uint32_t signal_type = 0;
        uint8_t  test_case   = test_input_data[data_index++];

        signal_id   = SMC_BYTES_TO_32BIT( (test_input_data+data_index) );
        data_index+= 4;
        signal_type = SMC_BYTES_TO_32BIT( (test_input_data+data_index) );

        switch(test_case)
        {
            case 0x00:
            {
                smc_signal_t* signal = NULL;
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: Create signal 0x%02X (%d), type 0x%08X...", signal_id, signal_id, signal_type);

                signal = smc_signal_create( signal_id, signal_type );

                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: Raising signal 0x%08X...", (uint32_t)signal);

                test_status = smc_signal_raise( signal );

                smc_signal_destroy( signal );

                break;
             }
             case 0x01:
             {
                 smc_signal_t* signal = NULL;
                 SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: Create signal 0x%02X (%d), type 0x%08X...", signal_id, signal_id, signal_type);

                 signal = smc_signal_create( signal_id, signal_type );

                 SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: Registering handler for signal 0x%08X...", (uint32_t)signal);

                     /* Register without any SMC obejcts */
                 test_status = smc_signal_handler_register( NULL, signal, NULL);

                 SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: Signal handler registered");

                 break;
             }
             default:
             {
                 SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: Invalid test case 0x%02X", test_case);
                 test_status = SMC_ERROR;
             }
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: not enough test input data (received %d, expected %d)",
                                    test_input_data_len, test_input_len_required);
        test_status = SMC_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_signal: completed with return value 0x%02X", test_status);

    return test_status;
}

static smc_t* get_smc_instance_1( uint8_t is_master, uint32_t shm_start_address )
{
    if( !g_smc_instance1 )
    {
            /* ====================================================
             * Create SMC configuration
             */
        smc_channel_conf_t* smc_channel_conf_1 = NULL;
        smc_conf_t*         smc_conf           = smc_conf_create();

        smc_conf->cpu_id_local  = SMC_CPU_ID_LOCAL;
        smc_conf->cpu_id_remote = SMC_CPU_ID_REMOTE;
        smc_conf->is_master     = is_master;

        SMC_TEST_TRACE_PRINTF_INFO("get_smc_instance_1: Creating channel configurations...");

        smc_channel_conf_1 = smc_test_case_create_channel_conf_1(is_master);

        smc_conf_add_channel_conf(smc_conf, smc_channel_conf_1);


            /* ====================================================
             * Create the SHM
             * NOTE: Must be after channel configuration
             */
        SMC_TEST_TRACE_PRINTF_INFO("get_smc_instance_1: Creating SHM configuration...");
        smc_test_case_create_shm_conf_local( smc_conf, (uint8_t*)shm_start_address );

        g_smc_instance1 = smc_instance_create(smc_conf);
    }

    return g_smc_instance1;
}

/*
 * Send message ptr deallocator callback
 */
static void smc_test_case_channel_1_deallocator_callback(smc_channel_t* smc_channel, void* ptr, struct _smc_user_data_t* userdata)
{
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_channel_1_deallocator_callback: Not deallocating message PTR 0x%08X from channel 0x%08X because test case inpout data used", (uint32_t)ptr, (uint32_t)smc_channel);
}

static void* smc_test_case_channel_1_allocator_callback(smc_channel_t* smc_channel, uint32_t size, struct _smc_user_data_t* userdata)
{
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_channel_1_allocator_callback: Allocating data size of %d for channel 0x%08X", size, (uint32_t)smc_channel);

    return (void*)SMC_MALLOC(size);
}

static void  smc_test_case_channel_1_event_callback(smc_channel_t* smc_channel, SMC_CHANNEL_EVENT event)
{
    SMC_TEST_TRACE_PRINTF_INFO("=====================================================================");
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_channel_1_event_callback: Event %d from channel 0x%08X", event, (uint32_t)smc_channel);
    SMC_TEST_TRACE_PRINTF_INFO("=====================================================================");
}

static smc_channel_conf_t* smc_test_case_create_channel_conf_1(uint8_t is_master)
{
    smc_channel_conf_t* smc_channel_conf_1 = smc_channel_conf_create();

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_channel_conf_1: Starting...");

        /* Set the channel configuration */
    smc_channel_conf_1->priority      = SMC_CHANNEL_PRIORITY_HIGHEST;

    if( is_master )
    {
        smc_channel_conf_1->fifo_size_in  = 25;
        smc_channel_conf_1->fifo_size_out = 20;
        smc_channel_conf_1->mdb_size_in   = 120*1024;   /* 120 kB */
        smc_channel_conf_1->mdb_size_out  = 100*1024;   /* 100 kB */
    }
    else
    {
        smc_channel_conf_1->fifo_size_in  = 20;
        smc_channel_conf_1->fifo_size_out = 25;
        smc_channel_conf_1->mdb_size_in   = 100*1024;   /* 100 kB */
        smc_channel_conf_1->mdb_size_out  = 120*1024;   /* 120 kB */
    }

        /* Configure the callbacks */
    smc_channel_conf_1->smc_receive_data_cb           = (void*)smc_receive_data_callback_channel_conf_1;
    smc_channel_conf_1->smc_send_data_deallocator_cb  = (void*)smc_test_case_channel_1_deallocator_callback;
    smc_channel_conf_1->smc_receive_data_allocator_cb = (void*)smc_test_case_channel_1_allocator_callback;
    smc_channel_conf_1->smc_event_cb                  = (void*)smc_test_case_channel_1_event_callback;

    /* Configure the signals */

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_channel_conf_1: Configuring signals...");
#ifdef SMECO_MODEM
#if (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L2)

    smc_channel_conf_1->signal_remote = smc_signal_create(C1_L1_CPU_Int_Gen_Ch2, SMC_SIGNAL_TYPE_INTGEN);
    smc_channel_conf_1->signal_local  = smc_signal_create(C2_L2_CPU_Int_Gen_Ch2, SMC_SIGNAL_TYPE_INTGEN);

#elif (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L1)

    smc_channel_conf_1->signal_remote = smc_signal_create(C2_L2_CPU_Int_Gen_Ch2, SMC_SIGNAL_TYPE_INTGEN);
    smc_channel_conf_1->signal_local  = smc_signal_create(C1_L1_CPU_Int_Gen_Ch2, SMC_SIGNAL_TYPE_INTGEN);

#endif
#else

#endif

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_channel_conf_1: Completed");

    return smc_channel_conf_1;
}

static smc_channel_conf_t* smc_test_case_create_channel_conf_2(uint8_t is_master)
{
    smc_channel_conf_t* smc_channel_conf_2 = smc_channel_conf_create();

        // Set channel configuration
    smc_channel_conf_2->priority      = SMC_CHANNEL_PRIORITY_LOWEST;

    if( is_master )
    {
        // Set channel configuration
        smc_channel_conf_2->fifo_size_in  = 30;
        smc_channel_conf_2->fifo_size_out = 40;
        smc_channel_conf_2->mdb_size_in   = 150*1024;   /* 150 kB */
        smc_channel_conf_2->mdb_size_out  = 200*1024;   /* 200 kB */
    }
    else
    {
        // Set channel configuration
        smc_channel_conf_2->fifo_size_in  = 40;
        smc_channel_conf_2->fifo_size_out = 30;
        smc_channel_conf_2->mdb_size_in   = 200*1024;   /* 200 kB */
        smc_channel_conf_2->mdb_size_out  = 150*1024;   /* 150 kB */
    }

    return smc_channel_conf_2;
}

static uint8_t* smc_test_case_create_shm_conf_local( smc_conf_t* smc_conf, uint8_t* shm_mem_start_addr )
{
    uint8_t* shm_address = NULL;

        /* =========================================================
         * Create the Shared memory allocating it from dynamic pool
         */
    smc_conf->smc_shm_conf = smc_shm_conf_create();

    smc_conf->smc_shm_conf->size = 0;

    for( int i = 0; i < smc_conf->smc_channel_conf_count; i++)
    {
        smc_conf->smc_shm_conf->size += smc_channel_calculate_required_shared_mem(smc_conf->smc_channel_conf_ptr_array[i]);
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_shm_conf_local: %d bytes of SHM is needed", smc_conf->smc_shm_conf->size);

    if( shm_mem_start_addr > 0 )
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_shm_conf_local: Slave using SHM from 0x%08X created by master", (uint32_t)shm_mem_start_addr);
        shm_address = shm_mem_start_addr;
    }
    else
    {
        shm_address = (uint8_t*)SMC_MALLOC( smc_conf->smc_shm_conf->size );

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_shm_conf_local: Master allocated SHM from 0x%08X", (uint32_t)shm_address);
    }

    smc_conf->smc_shm_conf->shm_area_start_address = shm_address;

#ifdef SMECO_MODEM
    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_create_shm_conf_local: Modem uses cache control");
    smc_conf->smc_shm_conf->use_cache_control = TRUE;
#endif

    return shm_address;
}

uint8_t smc_test_case_function_remote_event( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t  test_status             = SMC_ERROR;
    uint16_t test_input_len_required = 2;

    if( test_input_data_len >= test_input_len_required )
    {
        uint8_t channel_id      = test_input_data[0];
        SMC_CHANNEL_EVENT event = (SMC_CHANNEL_EVENT)test_input_data[1];

            /* Expecting that the SMC instance is created */
        smc_t* smc = get_smc_instance_1(TRUE, 0x00);

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_remote_event: Sending event %d to remote using channel %d", test_input_data[1], test_input_data[0]);

        test_status = smc_send_event( smc_channel_get(smc, channel_id), event);

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_remote_event: Sending event %d to remote using channel %d completed", test_input_data[1], test_input_data[0]);
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_remote_event: not enough test input data (received %d, expected %d)",
                                    test_input_data_len, test_input_len_required);
        test_status = SMC_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_remote_event: completed with return value 0x%02X", test_status);

    return test_status;
}

uint8_t smc_test_case_function_smc_instance( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t  test_status = SMC_ERROR;

    uint16_t test_input_len_required = 2;

    if( test_input_data_len >= test_input_len_required )
    {
        uint8_t data_index = 0;

        uint8_t test_case =  test_input_data[data_index];
        data_index++;

        switch(test_case)
        {
            case 0x00:
            {
                uint8_t  test_case_data[6];
                uint32_t shm_address        = 0x00000000;
                uint8_t  is_master          = TRUE;
                uint32_t shm_start_address  = 0x00000000;
                smc_t*   smc                = NULL;
                uint8_t  start_remote_side  = TRUE;

                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: Test case 0x%02X, creating SMC instance...", test_case);

#ifdef SMECO_MODEM
#if (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L1)
                is_master = FALSE;

                    /* We need the SHM address from test input data */
                if( test_input_data_len >= test_input_len_required+4 )
                {
                    shm_start_address = SMC_BYTES_TO_32BIT( (test_input_data+2) );

                    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: shm_start_address==0x%08X", (uint32_t)shm_start_address);
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance:not enough test input data, SHM pointer is missing for slave SMC");
                    return SMC_ERROR;
                }
#else
                /* L2 specific code */
                if( test_input_data[data_index] == 0x01 )
                {
                    start_remote_side = FALSE;
                }

                data_index++;

#endif
#endif
                smc = get_smc_instance_1(is_master, shm_start_address);

#ifdef SMECO_MODEM
#if (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L2)

                if( start_remote_side )
                {
                    shm_address = (uint32_t)smc->smc_shm_conf->shm_area_start_address;

                        /* L2 sends message to L1 to create SMC instance */
                    test_case_data[0] = 0x00;
                    test_case_data[1] = 0x00;

                    test_case_data[2] = (shm_address>>24)&0xFF;
                    test_case_data[3] = (shm_address>>16)&0xFF;
                    test_case_data[4] = (shm_address>>8)&0xFF;
                    test_case_data[5] = shm_address&0xFF;

                    if( smc_test_handler_send_message(0x02, test_case_data, 6) == SMC_OK )
                    {
                        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: smc_test_handler_send_message to L1 OK");
                    }
                    else
                    {
                        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: smc_test_handler_send_message to L1 FAILED");
                    }
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: The remote side SMC instance is not started, use separate test message");
                }
#endif
#endif
                smc_instance_dump( smc );

                test_status = SMC_OK;
                break;
            }
            case 0x01:
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: Test case 0x%02X, Sending message...", test_case);

                if( g_smc_instance1 )
                {
                    /* Use the default */

                    uint8_t  smc_channel_id = 0;
                    uint32_t data_length    = (test_input_data_len-data_index);
                    uint8_t* data           = (test_input_data + data_index);
                    smc_user_data_t userdata;

                    userdata.flags     = 0x00000000;
                    userdata.userdata1 = 0x00000000;
                    userdata.userdata2 = 0x00000000;
                    userdata.userdata3 = 0x00000000;
                    userdata.userdata4 = 0x00000000;
                    userdata.userdata5 = 0x00000000;

                    test_status = smc_send_ext(SMC_CHANNEL_GET(g_smc_instance1, smc_channel_id), (void*)data, data_length, &userdata);
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: SMC instance not initialized, unable to send message");
                    test_status = SMC_ERROR;
                }

                break;
            }
            case 0x02:
            {
                uint8_t smc_instance_id = test_input_data[1];

                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: Test case 0x%02X, Dumping SMC data in index %d...", test_case, smc_instance_id);

                    /* TODO Make own SMC array inside the test module */
                if( g_smc_instance1 )
                {
                    smc_instance_dump( g_smc_instance1 );
                    test_status = SMC_OK;
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: Test case 0x%02X failed: No SMC instance created", test_case);
                    test_status = SMC_ERROR;
                }

                break;
            }
            case 0x03:
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: Test case 0x%02X, Sending message with userdata...", test_case);

                if( g_smc_instance1 )
                {
                        /* Use the default smc channel*/
                    uint8_t  required_data_len = 7;
                    uint8_t  smc_channel_id    = 0;
                    uint32_t data_length       = (test_input_data_len-data_index);

                    if( data_length >= required_data_len )
                    {
                        uint8_t* data = NULL;
                        smc_user_data_t userdata;

                        userdata.flags     = test_input_data[data_index++];
                        userdata.userdata1 = test_input_data[data_index++];
                        userdata.userdata2 = test_input_data[data_index++];
                        userdata.userdata3 = test_input_data[data_index++];
                        userdata.userdata4 = test_input_data[data_index++];
                        userdata.userdata5 = test_input_data[data_index++];

                            /* Put the rest of the data to the message part */
                        data = (test_input_data + data_index);
                        data_length    = (test_input_data_len-data_index);

                        test_status = smc_send_ext(SMC_CHANNEL_GET(g_smc_instance1, smc_channel_id), (void*)data, data_length, &userdata);
                    }
                    else
                    {
                        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: Not enough message data for test case 0x%02X, required %d, received %d",
                                test_case, (test_input_data_len-data_index+required_data_len), test_input_data_len);
                        test_status = SMC_ERROR;
                    }
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: SMC instance not initialized, unable to send message");
                    test_status = SMC_ERROR;
                }

                break;
            }
            case 0x04:
            {
                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, Destroying SMC instance...", test_case);

                if( g_smc_instance1 )
                {
                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, Destroying SMC instance 1: 0x%08X", test_case, (uint32_t)g_smc_instance1);

                    smc_instance_destroy( g_smc_instance1 );

                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, SMC instance 1 destroyed", test_case);

                    g_smc_instance1 = NULL;

                    test_status = SMC_OK;
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: SMC instance not initialized, unable to send message");
                    test_status = SMC_ERROR;
                }

                break;
            }
            case 0x05:
            {
                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, Start SMC instance in remote side...", test_case);

#ifdef SMECO_MODEM
#if (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L2)
                if( g_smc_instance1 )
                {
                    uint8_t test_case_data[6];
                    uint32_t shm_address = 0x00000000;

                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, Startimg SMC instance 1 0x%08X in remote side", (uint32_t)g_smc_instance1);

                        /* Get the SHM and send to the remote */
                    shm_address = (uint32_t)g_smc_instance1->smc_shm_conf->shm_area_start_address;

                        /* L2 sends message to L1 to create SMC instance */
                    test_case_data[0] = 0x00;
                    test_case_data[1] = 0x00;

                    test_case_data[2] = (shm_address>>24)&0xFF;
                    test_case_data[3] = (shm_address>>16)&0xFF;
                    test_case_data[4] = (shm_address>>8)&0xFF;
                    test_case_data[5] = shm_address&0xFF;

                    if( smc_test_handler_send_message(0x02, test_case_data, 6) == SMC_OK )
                    {
                        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: smc_test_handler_send_message to L1 OK");
                    }
                    else
                    {
                        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: smc_test_handler_send_message to L1 FAILED");
                    }

                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, SMC instance 1 started in remote");

                    test_status = SMC_OK;
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: SMC instance not initialized, unable to send message");
                    test_status = SMC_ERROR;
                }
#else
                SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: This test is supported only in L2");
                test_status = SMC_ERROR;
#endif
#endif

                break;
            }
            case 0xFD:
            {
                uint8_t test_input[4];

                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, Sending PN message to other CPU...", test_case);

                test_input[0] = 0xFE;   /* Send the FIFO dump message */
                test_input[1] = 0x00;
                test_input[2] = 0xBB;
                test_input[3] = 0xCC;

                test_status = smc_test_handler_send_message(0x02, test_input, 4);

                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, PN message sent to other CPU", test_case);
                break;
            }
            case 0xFE:
            {
                /* TODO Move this test to more propriate place */
                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, Dump FIFOs...", test_case);

                if( g_smc_instance1 )
                {
                    smc_fifo_t* p_fifo = SMC_CHANNEL_GET(g_smc_instance1, 0)->fifo_out;

                    smc_fifo_dump_data(p_fifo);

                    p_fifo = SMC_CHANNEL_GET(g_smc_instance1, 0)->fifo_in;

                    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: FIFO IN");
                    smc_fifo_dump_data(p_fifo);
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_smc_instance: SMC instance not initialized, unable to send message");
                    test_status = SMC_ERROR;
                }
                break;
            }
            case 0xFF:
            {
                uint8_t is_master = TRUE;
                smc_t* smc_2 = NULL;
                smc_channel_conf_t* smc_channel_conf_1_master = NULL;
                smc_t* smc_1 = NULL;
                smc_conf_t* smc_conf_2 = NULL;

                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_smc_instance: Test case 0x%02X, creating SMC instance...", test_case);

#ifdef SMECO_MODEM
#if (MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L1)
                is_master = FALSE;
#endif
#endif
                /* Other SMC instance for Single CPU test case  */
                /* NOTE: If the instance is created the master/slave or SHM values are not modified */
                smc_1 = get_smc_instance_1(is_master, 0);

                    /* ====================================================
                     * Create seconc SMC configuration
                     */
                smc_conf_2 = smc_conf_create();
                smc_conf_2->is_master = !is_master;

                    // Use the same channel configuration but master
                smc_channel_conf_1_master = smc_test_case_create_channel_conf_1(is_master);
                smc_conf_add_channel_conf(smc_conf_2, smc_channel_conf_1_master);


                /* Just one channel
                smc_channel_conf_t* smc_channel_conf_2_master = smc_test_case_create_channel_conf_2(TRUE);
                smc_conf_add_channel_conf(smc_conf_2, smc_channel_conf_2_master);
                */

                    /* ====================================================
                     * Create the Shared memory
                     */

                    // Use the same shared memory area with SMC 1
                smc_conf_2->smc_shm_conf = smc_shm_conf_create();

                smc_conf_2->smc_shm_conf->shm_area_start_address = smc_1->smc_shm_conf->shm_area_start_address;
                smc_conf_2->smc_shm_conf->size                   = smc_1->smc_shm_conf->size;

                    // Create the second SMC instance
                smc_2 = smc_instance_create(smc_conf_2);

                smc_2->cpu_id_local  = SMC_CPU_ID_REMOTE;
                smc_2->cpu_id_remote = SMC_CPU_ID_LOCAL;

                smc_instance_dump( smc_2 );
                break;
            }
            default:
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: unknown test case 0x%02X", test_case);
                test_status = SMC_ERROR;
            }
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: not enough test input data (received %d, expected %d)",
                                    test_input_data_len, test_input_len_required);
        test_status = SMC_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_smc_instance: completed with return value 0x%02X", test_status);

    return test_status;
}

static uint8_t smc_test_case_function_configure_shared_mem( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t test_status = SMC_ERROR;
    uint8_t input_len_required = 10;

    if( test_input_data_len >= input_len_required )
    {
        smc_shm_config_t shm_config;

        uint32_t address = SMC_BYTES_TO_32BIT( test_input_data );
        shm_config.shm_area_start_address = (uint8_t*)address;
        shm_config.size                   = SMC_BYTES_TO_32BIT((test_input_data+4));
        shm_config.use_cache_control      = test_input_data[8];
        shm_config.cache_line_len         = test_input_data[9];

        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_configure_shared_mem: SHM start: 0x%08X, length: %d",
                (uint32_t)shm_config.shm_area_start_address,
                shm_config.size);

        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_configure_shared_mem: Not implemented");
        test_status = SMC_ERROR;
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_configure_shared_mem: not enough input data %d, required %d", test_input_data_len, input_len_required);
        test_status = SMC_ERROR;
    }

    return test_status;
}

void smc_test_handler_register( void )
{
    SMC_TEST_TRACE_PRINTF_INFO("Register SMC test handler (Group ID 0x%04X)...", smc_test_handler_group_id);

    (void)modem_test_handler_register( smc_test_handler_group_id, (uint16_t)SMC_TEST_HDLR,
                                       (modem_test_handler_ptr_t)smc_test_handler );

    SMC_TEST_TRACE_PRINTF_INFO("SMC test handler successfully registered");
}

os_task_token smc_os_task_create(const char* task_name, os_prior task_priority, os_entrypoint_t task_function)
{
#ifdef SMECO_LINUX_ANDROID
    SMC_TEST_TRACE_PRINTF_ERROR( "smc_create_os_task: NOT IMPLEMENTED IN ANDROID");
    return 0;
#elif defined SMECO_LINUX_KERNEL
    SMC_TEST_TRACE_PRINTF_ERROR( "smc_create_os_task: NOT IMPLEMENTED IN LINUX KERNEL");
    return 0;
#else
    uint32_t stack_size = 512;

    if( os_task_id_get( task_name ) == OS_NO_TASK )
    {
        os_task_token task_id ;
        void*         stack_ptr  = os_aligned_block_alloc_no_wait(stack_size, 8);

        assert( stack_ptr != NULL );

        SMC_TEST_TRACE_PRINTF_INFO( "smc_create_os_task: Creating task '%s'...", task_name);

        task_id = os_task_create( task_name,
                                  task_priority,
                                  task_function,
                                  stack_ptr,
                                  stack_size,
                                  15 );
        return task_id;
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO( "smc_create_os_task: Task '%s' already exists", task_name);
        return OS_NO_TASK;
    }
#endif
}

uint8_t smc_os_task_start(const char* task_name, os_prior task_priority, os_entrypoint_t task_function, uint8_t* test_input_data, uint16_t test_input_data_len, os_task_token* new_task_id)
{
#ifdef SMECO_LINUX_ANDROID
    SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: NOT IMPLEMENTED IN ANDROID");
    return SMC_ERROR;
#elif defined SMECO_LINUX_KERNEL
    SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: NOT IMPLEMENTED IN LINUX KERNEL");
    return SMC_ERROR;
#else
    int task_id = OS_NO_TASK;

    SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: creating dynamic task with name '%s'...", task_name);

    task_id = smc_os_task_create( task_name, task_priority, task_function );

    if( task_id != OS_NO_TASK )
    {
        SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: Task '%s' with id %d created, starting task...", task_name, task_id);

        *new_task_id = task_id;

        os_task_resume( task_id );

        SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: Task '%s' with id %d successfully started", task_name, task_id);

        if( test_input_data != NULL && test_input_data_len > 0 )
        {
            SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: sending test input data to receiver task...");

                // Send configuration message to receiver task
            os_msg_send_no_wait( task_id, test_input_data );
        }
        else
        {
            SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: test start message not send, msg NULL or empty");
        }

        SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: completed");

        return SMC_OK;
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO( "smc_os_task_start: failed: task with name '%s' already exists", task_name);
        return SMC_ERROR;
    }
#endif
}


/* ======================================================
 * SMC FIFO Callback functions
 */
static void smc_receive_data_callback_channel_conf_1( void*   data,
                                                      int32_t data_length,
                                                      const struct _smc_user_data_t* userdata,
                                                      const struct _smc_channel_t* channel)
{
    SMC_TEST_TRACE_PRINTF_INFO("==============================================================================================");
    SMC_TEST_TRACE_PRINTF_INFO( "smc_receive_data_callback_channel_0: Handling a message 0x%08X (len %d, flags 0x%08X) from SMC 0x%08X channel %d",
            (uint32_t)data, data_length, userdata->flags, (uint32_t)channel->smc_instance, channel->id);

    SMC_TRACE_PRINTF_INFO_DATA(data_length, data);

    SMC_TEST_TRACE_PRINTF_INFO( "smc_receive_data_callback_channel_0: Userdata of the message 0x%08X: 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X",
            (uint32_t)data, userdata->userdata1, userdata->userdata2, userdata->userdata3, userdata->userdata4, userdata->userdata5);

    SMC_TEST_TRACE_PRINTF_INFO("==============================================================================================");
}



#ifdef SMECO_LINUX_ANDROID

    /* ====================================================
     * Android Specific functions
     */
uint8_t smc_test_handler_send_message(uint16_t test_case_id, uint8_t* test_input_data, uint16_t test_input_data_len)
{
    uint8_t return_value = SMC_ERROR;

    SMC_TRACE_PRINTF_ERROR( "smc_test_handler_send_message: NOT IMPLEMENTED IN ANDROID");
    return return_value;
}

#elif defined SMECO_LINUX_KERNEL
    /* ====================================================
     * Linux Kernel Specific functions
     */
uint8_t smc_test_handler_send_message(uint16_t test_case_id, uint8_t* test_input_data, uint16_t test_input_data_len)
{
    uint8_t return_value = SMC_ERROR;

    SMC_TRACE_PRINTF_ERROR( "smc_test_handler_send_message: NOT IMPLEMENTED IN LINUX KERNEL");
    return return_value;
}

#else




/*
 * Sends message to other CPUs SMC test server
 */
uint8_t smc_test_handler_send_message(uint16_t test_case_id, uint8_t* test_input_data, uint16_t test_input_data_len)
{
    uint8_t return_value = SMC_ERROR;


    uint16_t message_length = 0;

    uint8_t media = 0x00;
    uint8_t receiver_device_id  = PN_DEV_MODEM_HOST_2;  /* Mexe Test Server is in L2*/
    uint8_t sender_device_id  = 0x00;

    uint8_t resource_id = 0x00;        /* Mexe Test Server */
    uint8_t receiver_object = 0x00;
    uint8_t sender_object = 0x00;
    uint8_t tid = 0x00;

#if(MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L2)
    /* ====================================================
     * L2 sends to L1 using phonet
     */

    /*

Detailed PMD:
   0  (8) Media.......................: 0x00               => PN_MEDIA_ROUTING_REQ
   1  (6) Receiver Device.............: 0x60               => PN_DEV_MODEM_HOST_2
   2  (6) Sender Device...............: 0x10               => PN_DEV_PC
   3  (8) Resource....................: 0x91               => PN_MODEM_TEST
   4 (16) Length......................: 0x0012             => 18
   6 (10) Receiver Object.............: 0x000              => 0
   7 (10) Sender Object...............: 0x001              => 1
   8  (8) Transaction ID..............: 0x01               => 1
   9  (8) Message ID..................: 0x80               => MODEM_TEST_RUN_REQ
  10 (16) Test group ID...............: 0x8001             => MODEM_TEST_GRP_MEXE_L23
  12 (16) MeXe test handler ID........: 0x001F             => 0x001F
Warning: No constant is defined for this value
  14 (16) Test case ID................: 0x0002             => 0x0002
  16  (8) Attribute...................: 0x00               => 00000000  MODEM_TEST_ATTR_NONE
  17  (8) Filler......................: 0x00               => 8 bit padding
  18 (16) Input data length or zero...: 0x0002             => 2
  20  (8) Input data[1]...............: 0x01               => 0x01
  21  (8) Input data[2]...............: 0x02               => 0x02
  22 (16) Filler......................: 0x0000             => Alignment to 32 bit boundary

    */

    message_length = 12 + 2 + test_input_data_len;

    pn_msg* pn_message = (pn_msg*)SMC_MALLOC(message_length + 6);

    pn_message->media    = 0x00;
    pn_message->receiver = receiver_device_id;
    pn_message->sender   = PN_DEV_MODEM_HOST_2;
    pn_message->function = PN_MODEM_TEST;
    pn_message->length   = message_length;
    pn_message->data[0]  = 0x00;
    pn_message->data[1]  = 0x01;
    pn_message->data[2]  = 0x00;
    pn_message->data[3]  = 0x80;

    /* This is MODEM_TEST_GRP_MEXE_L23
    pn_message->data[4]  = 0x80;
    pn_message->data[5]  = 0x01;
    */

    /* This is MODEM_TEST_GRP_MEXE_L1 */
    pn_message->data[4]  = 0x00;
    pn_message->data[5]  = 0x01;

    pn_message->data[6]  = 0x00;    /* SMC test */
    pn_message->data[7]  = 0x1F;    /* SMC test */

    pn_message->data[8]  = ((test_case_id>>8)&0xFF);
    pn_message->data[9]  = test_case_id&0xFF;
    pn_message->data[10] = 0x00;
    pn_message->data[11] = 0x00;
    pn_message->data[12] = ((test_input_data_len>>8)&0xFF);
    pn_message->data[13] = test_input_data_len&0xFF;

    if( test_input_data_len > 0 )
    {
        for(int i = 0; i < test_input_data_len; i++)
        {
            pn_message->data[i+14] = test_input_data[i];
        }
    }

    SMC_TRACE_PRINTF_INFO( "smc_test_handler_send_message: Sending Phonet message 0x%08X...", (uint32_t)pn_message);
    SMC_TRACE_PRINTF_INFO_DATA(message_length+6, pn_message);

    if( pn_msg_send(pn_message) == PN_OK )
    {
        SMC_TRACE_PRINTF_INFO( "smc_test_handler_send_message: Phonet message 0x%08X successfully sent", pn_message);
        return_value = SMC_OK;
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR( "smc_test_handler_send_message: Phonet message 0x%08X send failed", pn_message);
        SMC_FREE( pn_message );
        return_value = SMC_ERROR;

    }


#elif(MEXE_TARGET_IMAGE == MEXE_TARGET_IMAGE_L1)

    /* ====================================================
     * L1 sends to L2 using MMSGR
     */

    uint8_t* message = NULL;

    sender_device_id  = PN_DEV_MODEM_HOST_3;


    mmsgr_msg_send(resource_id, (MMSGR_ISI_MSG_STR*)message);
#endif

    return return_value;
}

#endif

static uint8_t smc_test_case_function_create_configuration( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t test_status = SMC_OK;

#if(SMC_L2MUX_IF==TRUE)


    char*   config_user_name = SMC_CONFIG_USER_L2MUX;   /* Get a configuration for the L2MUX */
    smc_instance_conf_t* smc_instance_configuration = NULL;

    /* Select configuration "user name" based on platform */
#ifdef SMECO_LINUX_ANDROID
    char* smc_name = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2;
#elif defined SMECO_LINUX_KERNEL
    char* smc_name = SMC_CONFIG_MASTER_NAME_SH_MOBILE_R8A73734_EOS2;
#elif defined SMECO_MODEM
    char* smc_name = SMC_CONFIG_SLAVE_NAME_MODEM_WGEM31_EOS2;
#else
    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: NOT IMPLEMENTED FOR THIS PRODUCT");
#endif

    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: starting, Test input data (len %d):", test_input_data_len);
    SMC_TEST_TRACE_PRINTF_DEBUG_DATA(test_input_data_len, test_input_data);

        /* Get the configuration by the name */
    smc_instance_configuration = smc_instance_conf_get_l2mux( config_user_name, smc_name );

    if( smc_instance_configuration != NULL )
    {
        int         i            = 0;
        smc_conf_t* smc_conf     = NULL;
        smc_t*      smc_instance = NULL;

        SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: Configuration for config user %s by SMC name %s found",
                config_user_name, smc_name);

        /* Create SMC instance based on the configuration */

        SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: channel count is %d (array size %d)",
                smc_instance_configuration->channel_config_count, sizeof( smc_instance_configuration->channel_config_array ));

        smc_conf = smc_conf_create_from_instance_conf( smc_name, smc_instance_configuration );

        SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: SMC configuration 0x%08X created, creating instance...", (uint32_t)smc_conf );

        smc_instance = smc_instance_create(smc_conf);

        SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: SMC instance created");

        test_status = SMC_OK;
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_ERROR("smc_test_handler_configuration: Configuration for config user %s by name %s not found", config_user_name, smc_name);
        test_status = SMC_ERROR;
    }
#else
    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_handler_configuration: L2MUX is not built in");
    test_status = SMC_ERROR;
#endif

    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_handler_configuration: completed with return value 0x%02X", test_status);
    return test_status;
}

static uint8_t smc_test_case_function_shm( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t  test_status = SMC_ERROR;

    uint16_t test_input_len_required = 5;

    if( test_input_data_len >= test_input_len_required )
    {
        uint8_t  remap_address = 0;      /* Valid only in Linux */
        uint32_t data_index   = 0;
        uint8_t  test_case    = test_input_data[data_index++];
        uint32_t shm_data_len = 0;
        uint32_t shm_address  = SMC_BYTES_TO_32BIT( (test_input_data+data_index) );

        data_index += 4;

        if( (test_case&0x10)==0x10 )
        {
            remap_address = 1;
            test_case     = test_case - 0x10;
        }
        else
        {
            remap_address = 0;
        }

        switch(test_case)
        {
            case 0x00:
            {
                int i = 0;
                shm_data_len = test_input_data_len - data_index;

                SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: write %d bytes of data to SHM address 0x%08X...", shm_data_len, shm_address);
                SMC_TEST_TRACE_PRINTF_DEBUG_DATA(shm_data_len, (test_input_data+data_index) );

                if( remap_address == 1 )
                {
#if defined SMECO_LINUX_KERNEL

                    shm_address = ioremap(shm_address,shm_data_len);
                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: ioremapped address 0x%08X", shm_address);
#endif
                }

                /* Linux __raw_writel(2, ((void __iomem *)((uint32_t)remapped_mdm_io))); */

                for(i = 0; i < shm_data_len; i++ )
                {
                    *((uint8_t*)shm_address + i ) = test_input_data[data_index+i];
                }

                if( remap_address == 1 )
                {
#if defined SMECO_LINUX_KERNEL
                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: unmap address 0x%08X", shm_address);
                    iounmap(shm_address);
#endif
                }

                test_status = SMC_OK;

                break;
             }
             case 0x01:
             {
                 test_input_len_required = 8;

                 if( test_input_data_len >= test_input_len_required )
                 {
                     uint8_t* data_read_from_shm = NULL;

                     shm_data_len = SMC_BYTES_TO_32BIT( (test_input_data+ data_index) );
                     data_index += 4;

                     SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: read %d bytes of data from SHM address 0x%08X...", shm_data_len, shm_address);

                     if( remap_address == 1 )
                     {
#if defined SMECO_LINUX_KERNEL
                         shm_address = ioremap(shm_address,shm_data_len);
                         SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: ioremapped address 0x%08X", shm_address);
#endif
                     }

                     SMC_TEST_TRACE_PRINTF_DEBUG_DATA(shm_data_len, (uint8_t*)shm_address );

                     if( remap_address == 1 )
                     {
#if defined SMECO_LINUX_KERNEL
                         SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: unmap address 0x%08X", shm_address);
                         iounmap(shm_address);
#endif
                     }

                 }
                 else
                 {
                     SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_shm: not enough test input data for read (received %d, expected %d)",
                                                         test_input_data_len, test_input_len_required);
                 }

                 test_status = SMC_OK;

                 break;
             }
             default:
             {
                 SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_shm: Invalid test case 0x%02X", test_case);
                 test_status = SMC_ERROR;
             }
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_shm: not enough test input data (received %d, expected %d)",
                                    test_input_data_len, test_input_len_required);
        test_status = SMC_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_shm: completed with return value 0x%02X", test_status);

    return test_status;
}

/* EOF */

