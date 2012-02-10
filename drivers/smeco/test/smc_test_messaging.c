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

Version:       1    11-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#include "smc_common_includes.h"

#include "smc_conf.h"
#include "smc_fifo.h"
#include "smc.h"
#include "smc_test.h"
#include "smc_test_fifo.h"
#include "smc_trace.h"

    /* Local messaging test case functions */
static uint8_t smc_start_messaging_test_single_cpu(uint8_t* test_input_data, uint16_t test_input_data_len);

uint8_t smc_test_case_function_messaging( uint8_t* test_input_data, uint16_t test_input_data_len )
{
    uint8_t  test_result = SMC_ERROR;
    uint16_t test_input_len_required = 2;

    SMC_TEST_TRACE_PRINTF_INFO("Messaging test case invoked, input data len %d", test_input_data_len);

    if( test_input_data_len >= test_input_len_required )
    {
        uint16_t data_index = 0;
        uint8_t  test_case  = test_input_data[data_index++];

        switch( test_case )
        {
            case 0x00:
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_messaging: smc_start_messaging_test_single_cpu test case starting...");

                test_result = smc_start_messaging_test_single_cpu((test_input_data+data_index), test_input_data_len-data_index);

                break;
            }
            case 0x01:
            {
                SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_messaging: Send using SMC control insttance...");

                smc_t* smc_instance_control = smc_instance_get_control();

                if( smc_instance_control != NULL )
                {
                    uint8_t smc_channel_id = 0;

                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_messaging (SMC Control): Sending data using channel %d...", smc_channel_id);

                    uint32_t        data_length    = (test_input_data_len-data_index);
                    uint8_t*        data           = (test_input_data + data_index);
                    smc_user_data_t userdata;

                    userdata.flags     = 0x00000000;
                    userdata.userdata1 = 0x00000000;
                    userdata.userdata2 = 0x00000000;
                    userdata.userdata3 = 0x00000000;
                    userdata.userdata4 = 0x00000000;
                    userdata.userdata5 = 0x00000000;

                    SMC_TEST_TRACE_PRINTF_DEBUG("smc_test_case_function_messaging: sending %d bytes of data...", data_length);
                    SMC_TEST_TRACE_PRINTF_DEBUG_DATA(data_length, data);

                    test_result = smc_send_ext(SMC_CHANNEL_GET(smc_instance_control, smc_channel_id), (void*)data, data_length, &userdata);
                }
                else
                {
                    SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_messaging: SMC control channel is not available");
                    test_result = SMC_ERROR;
                }

                break;
            }
            default:
            {
                SMC_TEST_TRACE_PRINTF_ERROR("smc_test_case_function_messaging: Invalid test case 0x%02X", test_case);
                test_result = SMC_ERROR;
                break;
            }
        }
    }
    else
    {
        SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_messaging: not enough test input data (received %d, expected %d)",
                                            test_input_data_len, test_input_len_required);
        test_result = SMC_ERROR;
    }

    SMC_TEST_TRACE_PRINTF_INFO("smc_test_case_function_messaging: Test completed with return value 0x%02X", test_result);

    return test_result;
}

static uint8_t smc_start_messaging_test_single_cpu(uint8_t* test_input_data, uint16_t test_input_data_len)
{
    uint8_t test_result = SMC_ERROR;
    
    SMC_TEST_TRACE_PRINTF_INFO( "smc_start_messaging_test_single_cpu: START Messaging test single CPU, (Input data len %d)...", test_input_data_len);
    SMC_TEST_TRACE_PRINTF_INFO_DATA(test_input_data_len, test_input_data);
    
    SMC_TEST_TRACE_PRINTF_INFO( "smc_start_messaging_test_single_cpu: NOT IMPLEMENTED");

    SMC_TEST_TRACE_PRINTF_INFO( "smc_start_messaging_test_single_cpu: Messaging test single CPU completed by return value 0x%02X", test_result);
    
    return test_result;
}

