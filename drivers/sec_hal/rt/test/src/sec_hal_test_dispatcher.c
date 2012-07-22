/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                         **
** All rights reserved.                                                      **
** ************************************************************************* */


/* ************************ HEADER (INCLUDE) SECTION *********************** */
#include "sec_hal_rt.h"
#include "sec_hal_rt_trace.h"
#include "sec_hal_rt_cmn.h"
#include "sec_serv_api.h"
#include <stdarg.h>

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
/* local macros */
#define DATA_SIZE_MAX 50
#define PPFLAGS_COUNT 10

/* volatile data */
static uint32_t g_status = SEC_SERV_STATUS_OK;
static uint32_t g_disp_status = SEC_ROM_RET_OK;
static uint32_t g_cert_address = 0x00;
static uint32_t g_data_address = 0x00;
static char g_simlock_unlock_codes[5][17];
static uint32_t g_rpc_handler;
static uint32_t g_coma_params[4] = {0};
static uint32_t g_memcpy_params[3] = {0};
static uint32_t g_a3sp_state_info[2] = {};

/* const data */
static const uint8_t k_auth_data[DATA_SIZE_MAX] =
    {0,1,2,3,4,5,6,7,8,9,
     10,11,12,13,14,15,16,17,18,19,
     20,21,22,23,24,25,26,27,28,29,
     30,31,32,33,34,35,36,37,38,39,
     40,41,42,43,44,45,46,47,48,49};
static const uint8_t k_mac_address[SEC_HAL_MAC_SIZE] =
    {0xa1,0xb2,0xc3,0xd4,0xe5,0xf6};
static const char k_imei[SEC_HAL_MAX_IMEI_SIZE] =
    {'a','b','c','d','e','f','g','h','1','2','3','4','5','6','7',0};
static const char k_keyinfo[SEC_HAL_KEY_INFO_SIZE] =
    {'A','B','C','D','E','F','G','H',
     'I','J','K','L','M','N','O','P',
     'Q','R','S','T','X','Y','0','1',
     '2','3','4','5','6','7','8','9'};
static const sec_hal_rat_band_t k_rat_band = {0x0f,{0x01,0x02,0x03,0x04,0x05}};
static const sec_hal_pp_flag_t k_pp_flags[PPFLAGS_COUNT] =
    {{0x01,0x02},
     {0x03,0x04},
     {0x05,0x06},
     {0x07,0x08},
     {0x09,0x10},
     {0x11,0x12},
     {0x13,0x14},
     {0x15,0x16},
     {0x17,0x18},
     {0x19,0x20}};
static const uint32_t k_default_timeout_value = 30000;
static const uint32_t k_objid = 12;
static const char k_correct_siml_unlock_code[5][17] =
	{"ONE","TWO","THREE","FOUR","FIVE"};


/* ****************************** CODE SECTION ***************************** */
void test_set_status(uint32_t status)
{
    g_status = status;
}

void test_set_disp_status(uint32_t status)
{
    g_disp_status = status;
}

int test_assert_random_number(int size, uint8_t* random_number)
{
    int i = 0, ret = 1;
    while(i<size && i<DATA_SIZE_MAX && ret)
    {
        ret = (k_auth_data[i] == random_number[i]);
        i++;
    }
    return ret;
}

int test_assert_key_info(uint8_t* key_info)
{
    int i = 0, ret = 1;
    while(i<SEC_HAL_KEY_INFO_SIZE && ret)
    {
        ret = (k_keyinfo[i]==key_info[i]);
        i++;
    }
    return ret;
}

int test_assert_cert_address(uint32_t *address)
{
    return (address == (uint32_t*)g_cert_address);
}

int test_assert_data_address(uint32_t *address)
{
    return (address == (uint32_t*)g_data_address);
}

int test_assert_objid(uint32_t objid)
{
    return (objid == k_objid);
}

int test_assert_mac_address(uint8_t* mac_address)
{
    int i = 0, ret = 1;
    while(i<SEC_HAL_MAC_SIZE && ret)
    {
        ret = (k_mac_address[i]==mac_address[i]);
        i++;
    }
    return ret;
}

int test_assert_imei(char* imei)
{
    int i = 0, ret = 1;
    while(i<SEC_HAL_MAX_IMEI_SIZE && ret)
    {
        ret = (k_imei[i]==imei[i]);
        i++;
    }
    return ret;
}

int test_assert_auth_data(uint8_t* input_buf,
                          uint8_t input_len,
                          uint8_t* output_buf)
{
    /* output should be two times the input */
    int i = 0, ret = 1;
    while(i<input_len && ret) /* first half */
    {
        ret = (input_buf[i]==output_buf[i]);
        i++;
    }
    i = 0;
    while(i<input_len && ret)/* second half */
    {
        ret = (input_buf[i]==output_buf[input_len+i]);
        i++;
    }
    return ret;
}

int test_assert_rat_band(sec_hal_rat_band_t* rat_band)
{
    int i = 0, ret = 1;
    ret = (k_rat_band.rats == rat_band->rats);
    while(i<SEC_HAL_MAX_BANDS && ret)
    {
        ret = (k_rat_band.bands[i] == rat_band->bands[i]);
        i++;
    }
    return ret;
}

int test_assert_pp_flags_count(int count)
{
    return (PPFLAGS_COUNT == count);
}

int test_assert_pp_flags(uint8_t* input_buf)
{
    int i = 0, ret = 1;
    uint8_t* own_buffer = (uint8_t*) k_pp_flags;
    while(i<(PPFLAGS_COUNT*sizeof(sec_hal_pp_flag_t)) && ret)
    {
        ret = (own_buffer[i] == input_buf[i]);
        i++;
    }
    return ret;
}

int test_assert_exp_time(uint32_t exp_time)
{
    return (k_default_timeout_value == exp_time);
}

int test_assert_rpc_handler(void* func_ptr)
{
    return (g_rpc_handler == (uint32_t)func_ptr);
}

int test_assert_coma_params(uint32_t param0, uint32_t param1, uint32_t param2, uint32_t param3)
{
    return (g_coma_params[0] == param0 &&
            g_coma_params[1] == param1 &&
            g_coma_params[2] == param2 &&
            g_coma_params[3] == param3);
}

int test_assert_a3sp_state_params(uint32_t param0, uint32_t param1)
{
    return (g_a3sp_state_info[0] == param0 &&
            g_a3sp_state_info[1] == param1);
}

int test_assert_memcpy_params(uint32_t param0, uint32_t param1, uint32_t param2)
{
    return (g_memcpy_params[0] == param0 &&
            g_memcpy_params[1] == param1 &&
            g_memcpy_params[2] == param2);
}

static const char* k_callback_hash = "5a36b50698fc9cec36087c93980d6580aa494d1b";
static int is_callback_req(const char* data, uint32_t* offs)
{
	int i = 0, ret = 1, len = strlen(k_callback_hash);
	while(ret && len > i){ret = (k_callback_hash[i] == data[i]); i++;}
	if(offs){*offs = (uint32_t)len;}
	return ret;
}
typedef uint32_t (*callback)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef struct
{
  uint32_t id;
  uint32_t p1;
  uint32_t p2;
  uint32_t p3;
  uint32_t p4;
} callback_args_t;

/* ****************************************************************************
** ***************************************************************************/
uint32_t sec_dispatcher(uint32_t service_id, uint32_t flags, ...);
uint32_t sec_dispatcher(uint32_t service_id, uint32_t flags, ...)
{
    uint32_t ret_status = g_disp_status;
    sec_msg_t* sec_msg_out_msg;
    sec_msg_t* sec_msg_in_msg;
    sec_msg_handle_t out_handle, in_handle;
    va_list list;

    SEC_HAL_TRACE_ENTRY
    va_start(list,flags);

    /*uint32_t spare_param = */ va_arg(list, uint32_t); /* spare not used. */
    sec_msg_out_msg = va_arg(list, sec_msg_t*);
    sec_msg_in_msg = va_arg(list, sec_msg_t*);
    sec_msg_open(&out_handle, SEC_HAL_MEM_PHY2VIR_FUNC(sec_msg_out_msg));
    sec_msg_open(&in_handle, SEC_HAL_MEM_PHY2VIR_FUNC(sec_msg_in_msg));

    switch(service_id)
    {
        case SEC_SERV_CERTIFICATE_REGISTER:
        {
            SEC_HAL_TRACE("SEC_SERV_CERTIFICATE_REGISTER")
            /* store cert address */
            sec_msg_param_read32(&in_handle, &g_cert_address);
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
        } break;
        case SEC_SERV_MAC_ADDRESS_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_MAC_ADDRESS_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    SEC_HAL_MAC_SIZE == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write macaddr */
            sec_msg_param_write(&out_handle,
                                k_mac_address,
                                SEC_HAL_MAC_SIZE, 0);
        } break;
        case SEC_SERV_IMEI_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_IMEI_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    SEC_HAL_MAX_IMEI_SIZE == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write imei */
            sec_msg_param_write(&out_handle,
                                k_imei,
                                SEC_HAL_MAX_IMEI_SIZE, 0);
        } break;
        case SEC_SERV_DEVICE_AUTH_DATA_REQUEST:
        {
            uint32_t offset = 0, ret = 0;
            uint8_t buffer[256] = {0};
            uint32_t input_data_size = 0x00;
            uint32_t auth_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_DEVICE_AUTH_DATA_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* read input data */
            sec_msg_param_read(&in_handle, buffer, input_data_size);
            memcpy(buffer+input_data_size, buffer, input_data_size);
            /* read auth data size */
            sec_msg_param_read32(&in_handle, &auth_data_size);

            if(is_callback_req(buffer, &offset) && g_rpc_handler)
            {
                callback cb = (callback)g_rpc_handler;
                callback_args_t args;
                memcpy(&args, buffer+offset, sizeof(callback_args_t));
                if(SEC_SERV_RPC_ALLOC == args.id)
                {
                    ret = (*cb)(SEC_SERV_RPC_ALLOC, args.p1, args.p2, args.p3, args.p4);
                    if(!args.p4) /*to enable out of mem testing*/
                    {
                        (*cb)(SEC_SERV_RPC_FREE, ret, args.p2, args.p3, args.p4);
                    }
                }
                else if(SEC_SERV_RPC_PROT_DATA_ALLOC == args.id)
                {
                    ret = (*cb)(SEC_SERV_RPC_PROT_DATA_ALLOC, args.p1, args.p2, args.p3, args.p4);
                    if(!args.p4) /*to enable out of mem testing*/
                    {
                        (*cb)(SEC_SERV_RPC_PROT_DATA_FREE, ret, args.p2, args.p3, args.p4);
                    }
                }
                else{ret = 0;}
            }
            else
            {
                ret = (auth_data_size == 2*input_data_size);
            }

            /* write status */
            sec_msg_param_write32(&out_handle,
                    ret ? g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write data */
            sec_msg_param_write(&out_handle, buffer, auth_data_size, 0);
        } break;
        case SEC_SERV_DEVICE_AUTH_DATA_SIZE_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_DEVICE_AUTH_DATA_SIZE_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
            /* write data size */
            sec_msg_param_write32(&out_handle, 2*input_data_size, 0);
        } break;
        case SEC_SERV_KEY_INFO_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_KEY_INFO_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    SEC_HAL_KEY_INFO_SIZE == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write data size */
            sec_msg_param_write(&out_handle,
                                k_keyinfo,
                                SEC_HAL_KEY_INFO_SIZE, 0);
        } break;
        case SEC_SERV_RANDOM_DATA_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_RANDOM_DATA_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
            /* write data */
            sec_msg_param_write(&out_handle,
                                k_auth_data,
                                (input_data_size<DATA_SIZE_MAX?input_data_size:DATA_SIZE_MAX), 0);
        } break;
        case SEC_SERV_RAT_BAND_INFO_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_RAT_BAND_INFO_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    sizeof(uint32_t)*(1+SEC_HAL_MAX_BANDS) == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write data */
            sec_msg_param_write32(&out_handle, k_rat_band.rats, 0);
            sec_msg_param_write(&out_handle,
                                k_rat_band.bands,
                                sizeof(uint32_t)*SEC_HAL_MAX_BANDS, 0);
        } break;
        case SEC_SERV_PP_FLAGS_SIZE_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_PP_FLAGS_SIZE_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    sizeof(sec_hal_pp_flag_t) == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write data */
            sec_msg_param_write32(&out_handle,
                    PPFLAGS_COUNT*sizeof(sec_hal_pp_flag_t), 0);
        } break;
        case SEC_SERV_PP_FLAGS_REQUEST:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_PP_FLAGS_REQUEST")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    PPFLAGS_COUNT*sizeof(sec_hal_pp_flag_t) == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write data */
            sec_msg_param_write(&out_handle, k_pp_flags,
                    PPFLAGS_COUNT*sizeof(sec_hal_pp_flag_t), 0);
        } break;
        case SEC_SERV_INTEGRITY_CHECK:
        {
            uint32_t input_data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_INTEGRITY_CHECK")
            /* read input data size */
            sec_msg_param_read32(&in_handle, &input_data_size);
            /* write status */
            sec_msg_param_write32(&out_handle,
                    sizeof(uint32_t) == input_data_size ?
                            g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write data */
            sec_msg_param_write32(&out_handle, k_default_timeout_value, 0);
        } break;
        case SEC_SERV_PROT_DATA_REGISTER:
        {
            uint32_t data_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_PROT_DATA_REGISTER")
            /* store cert address & data address */
            sec_msg_param_read32(&in_handle, &g_cert_address);
            /* store data address */
            sec_msg_param_read32(&in_handle, &g_data_address);
            /* store data size */
            sec_msg_param_read32(&in_handle, &data_size);
            /* write status */
            sec_msg_param_write32(&out_handle, g_data_address ?
                    (data_size?g_status:SEC_SERV_STATUS_INVALID_INPUT) :
                    g_status, 0);
            /* write status */
            sec_msg_param_write32(&out_handle, k_objid, 0);
        } break;
        case SEC_SERV_SIMLOCK_CHECK_LOCKS:
        {
            int index = 0;
            uint32_t status = 0x00;
            uint32_t level_status = 0x00;
            uint32_t param_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_SIMLOCK_CHECK_LOCKS")
            /* read codes and write status */
            while (index < 5 && SEC_SERV_STATUS_OK == status)
            {
                sec_msg_param_size_get(&in_handle, (void*)&param_size);
                status = sec_msg_param_read(&in_handle, g_simlock_unlock_codes[index++], param_size);
            }
            sec_msg_param_write32(&out_handle,
                    (SEC_SERV_STATUS_OK == status &&
                     strlen(g_simlock_unlock_codes[0]) &&
                     strlen(g_simlock_unlock_codes[1]) &&
                     strlen(g_simlock_unlock_codes[2]) &&
                     strlen(g_simlock_unlock_codes[3]) &&
                     strlen(g_simlock_unlock_codes[4])) ?
                    g_status : SEC_SERV_STATUS_INVALID_INPUT, 0);
            /* write simlock level status bitmask */
            index = 0;
            while (index < 5)
            {
                level_status |= (!strcmp(g_simlock_unlock_codes[index],
                                         k_correct_siml_unlock_code[index]) ?
                                         0x0 : 0x1<<index);
                index++;
            }
            sec_msg_param_write32(&out_handle, level_status, 0);
            memset(g_simlock_unlock_codes, 0, sizeof(char)*17*5);/*empty code array */
        } break;
        case SEC_SERV_SIMLOCK_CHECK_ONE_LOCK:
        {
            uint8_t level = 0;
            uint32_t status = 0x00;
            uint32_t param_size = 0x00;
            SEC_HAL_TRACE("SEC_SERV_SIMLOCK_CHECK_ONE_LOCK")
            /* read level */
            status = sec_msg_param_read8(&in_handle, &level);
            if (level <= 5 && SEC_SERV_STATUS_OK == status)
            {
                /* read code */
                sec_msg_param_size_get(&in_handle, (void*)&param_size);
                status = sec_msg_param_read(&in_handle,
                         g_simlock_unlock_codes[level-1], param_size);
            }
            if (SEC_SERV_STATUS_OK == status)
            {
                status = (!strcmp(g_simlock_unlock_codes[level-1],
                                  k_correct_siml_unlock_code[level-1]) ?
                                  g_status : SEC_SERV_STATUS_FAIL );
            }
            /* write status */
            sec_msg_param_write32(&out_handle, status, 0);
            memset(g_simlock_unlock_codes, 0, sizeof(char)*17*5);/*empty code array */
        } break;
        case SEC_SERV_SIMLOCK_GET_STATE:
        {
            SEC_HAL_TRACE("SEC_SERV_SIMLOCK_GET_STATE")
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
            /* write simlock level status bitmask */
            sec_msg_param_write32(&out_handle, 0x07, 0);
        } break;
        case SEC_SERV_RUNTIME_INIT:
        {
            SEC_HAL_TRACE("SEC_SERV_RUNTIME_INIT")
            ret_status = SEC_ROM_RET_OK;
        } break;
        case SEC_SERV_SELFTEST:
        {
            SEC_HAL_TRACE("SEC_SERV_SELFTEST")
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
            /* write selftest result */
            sec_msg_param_write32(&out_handle, g_status, 0);
        } break;
        case SEC_SERV_RPC_ADDRESS:
        {
            SEC_HAL_TRACE("SEC_SERV_RPC_ADDRESS")
            /* read input (phys)address */
            sec_msg_param_read32(&in_handle, &g_rpc_handler);
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
            /* write rpc result */
            sec_msg_param_write32(&out_handle, g_status, 0);
        } break;
        case SEC_SERV_COMA_ENTRY:
        {
            SEC_HAL_TRACE("SEC_SERV_COMA_ENTRY")
            /* read input params */
            sec_msg_param_read32(&in_handle, &g_coma_params[0]);
            sec_msg_param_read32(&in_handle, &g_coma_params[1]);
            sec_msg_param_read32(&in_handle, &g_coma_params[2]);
            sec_msg_param_read32(&in_handle, &g_coma_params[3]);
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
        } break;
        case SEC_SERV_POWER_OFF:
        {
            SEC_HAL_TRACE("SEC_SERV_POWER_OFF")
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
        } break;
        case SEC_SERV_MEMCPY_REQUEST:
        {
            SEC_HAL_TRACE("SEC_SERV_MEMCPY_REQUEST")
            /* read input params */
            sec_msg_param_read32(&in_handle, &g_memcpy_params[0]);
            sec_msg_param_read32(&in_handle, &g_memcpy_params[1]);
            sec_msg_param_read32(&in_handle, &g_memcpy_params[2]);
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
        }break;
        case SEC_SERV_COMA_CPU_OFF:
        {
            SEC_HAL_TRACE("SEC_SERV_COMA_CPU_OFF")
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
        }break;
        case SEC_SERV_PUBLIC_CC42_KEY_INIT:
        {
            SEC_HAL_TRACE("SEC_SERV_PUBLIC_CC42_KEY_INIT")
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
        }break;
        case SEC_SERV_A3SP_STATE_INFO:
        {
            SEC_HAL_TRACE("SEC_SERV_A3SP_STATE_INFO")
			sec_msg_param_read32(&in_handle, &g_a3sp_state_info[0]);
			g_a3sp_state_info[1] = g_a3sp_state_info[0] + 1;
            /* write status */
            sec_msg_param_write32(&out_handle, g_status, 0);
			/* write 'allowed' */
			sec_msg_param_write32(&out_handle, g_a3sp_state_info[1], 0);
        }break;
        default:
        {
            SEC_HAL_TRACE("SSA_SEC_PUB_DISPATCHER:default")
            ret_status = SEC_ROM_RET_NON_SUPPORTED_SERV;
        } break;
    }

    sec_msg_close(&in_handle);
    sec_msg_close(&out_handle);
    va_end(list);

    SEC_HAL_TRACE_EXIT
    return ret_status;
}

/* ******************************** END ************************************ */

