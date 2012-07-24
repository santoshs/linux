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
#include "sec_hal_test_alloc.h"
#include "sec_serv_api.h"
#include "sec_hal_test_common.h"
#include "sec_hal_test_dispatcher.h"
#include <stdio.h>
#include <stdlib.h>

#if(defined PUBLIC_ENVIRONMENT && !(defined SEC_HAL_TEST_ISOLATION))
#error !!PUBLIC_ENVIRONMENT alone is not enough to test allocators(_ISOLATION)!!
#endif

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
const char* k_cert_data = "CERTDATACERTDATACERTDATA";
const char* k_input_auth_data = "hereissomeauthdataforyou..";

#define ASSERT_KEYINFO(keyinfo) ASSERT(test_assert_key_info(keyinfo))
#define ASSERT_CERT_ADDR(addr) ASSERT(test_assert_cert_address(addr))
#define ASSERT_DATA_ADDR(addr) ASSERT(test_assert_data_address(addr))
#define ASSERT_OBJID(objid) ASSERT(test_assert_objid(objid))
#define ASSERT_CERT_RANDOM_NUMBER(size, number) ASSERT(test_assert_random_number(size, number))
#define ASSERT_MACADDR(macaddr) ASSERT(test_assert_mac_address(macaddr))
#define ASSERT_IMEI(im) ASSERT(test_assert_imei(im))
#define ASSERT_AUTH_DATA(input, len, output) ASSERT(test_assert_auth_data(input, len, output))
#define ASSERT_RAT_BAND(rat_band) ASSERT(test_assert_rat_band(rat_band))
#define ASSERT_PP_FLAGS_COUNT(count) ASSERT(test_assert_pp_flags_count(count))
#define ASSERT_PP_FLAGS(input) ASSERT(test_assert_pp_flags(input))
#define ASSERT_PER_TIMEOUT(timeout) ASSERT(test_assert_exp_time(timeout))
#define ASSERT_RPC_HANDLER(func_ptr) ASSERT(test_assert_rpc_handler((void*)func_ptr))
#define ASSERT_COMA_PARAMS(p1,p2,p3,p4) ASSERT(test_assert_coma_params(p1,p2,p3,p4))
#define ASSERT_A3SP_STATE_INFO(p1, p2) ASSERT(test_assert_a3sp_state_params(p1,p2))
#define ASSERT_MEMCPY_PARAMS(p1,p2,p3) ASSERT(test_assert_memcpy_params(p1,p2,p3))

int test_sanity();

/* ***************************** CODE SECTION ****************************** */
#if 0
uint32_t id_alloc(uint32_t data_type,
        uint32_t data_id,
        void** data_ptr,
        uint32_t* data_size)
{
    return 0x00;
}
uint32_t id_free(uint32_t data_type,
        uint32_t data_id,
        void* data_ptr)
{
    return 0x00;
}
#endif
uint32_t rpc_handler(uint32_t id, void* in_msg, void* out_msg)
{
    return 0x00;
}


SEC_HAL_TRACE_INIT
int main()
{
    int result = 0;

#ifdef SEC_HAL_TEST_ISOLATION
    uint8_t* heap = malloc(4*1024);
    if (!heap){ return -1; }
    sec_hal_msg_area_init(heap, 4*1024);
#endif /* SEC_HAL_TEST_ISOLATION */

    result = test_sanity();

#ifdef SEC_HAL_TEST_ISOLATION
    if (sec_hal_msg_area_alloc_count())
    {
        SEC_HAL_TRACE("Memory is leaking!! check your code!!")
        result = (result ? result : __LINE__);
    }
    free(heap);
#endif /* SEC_HAL_TEST_ISOLATION */

    return result;
}

int test_sanity()
{
    uint32_t count = 0x00;
    uint32_t size = 0x00;

    SEC_HAL_TRACE("STARTING SANITY CHECK.")

    {
        uint32_t periodic_timer = 0x00;
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_init(&periodic_timer));
        ASSERT(60000 == periodic_timer);
    }
    {   /* sec_hal_rt_install_rpc_handler */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_install_rpc_handler(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_install_rpc_handler(&rpc_handler));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_install_rpc_handler(&rpc_handler));
        test_set_disp_status(SEC_SERV_STATUS_OK);
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_install_rpc_handler(&rpc_handler));
        ASSERT_RPC_HANDLER(&rpc_handler);
    }

    /* Actual functional API. */
    /* Invalid, valid and out-of-bounds tests are executed. */

    {   /* sec_hal_key_info_get. */
        sec_hal_key_info_t key_info;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_key_info_get(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_key_info_get(&key_info));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_key_info_get(&key_info));
        test_set_disp_status(SEC_SERV_STATUS_OK);
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_key_info_get(&key_info));
        ASSERT_KEYINFO(key_info.key_info); /*asserted against known values*/
    }
    {   /* sec_hal_cert_register. */
        void* cert = (void*) k_cert_data;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_cert_register(NULL, 0, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_cert_register(cert, 0, NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_cert_register(cert, 24, NULL));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_cert_register(cert, 24, NULL));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_cert_register(cert, 24, NULL));
        ASSERT_CERT_ADDR(cert); /*asserted against stored values*/
    }
    {   /* sec_hal_data_cert_register. */
        void* cert = (void*) k_cert_data;
        uint32_t id = 0x00;
        uint8_t auth_data[64];
        void* auth_data_ptr = (void*) k_input_auth_data;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_data_cert_register(NULL, 0, NULL, 0, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_data_cert_register(auth_data_ptr, 0, NULL, 0, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_data_cert_register(auth_data_ptr, 24, NULL, 0, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_data_cert_register(auth_data_ptr, 24, auth_data, 0, NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_data_cert_register(auth_data_ptr, 24, auth_data, 20, &id));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_data_cert_register(auth_data_ptr, 24, auth_data, 20, &id));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_data_cert_register(auth_data_ptr, 24, (void*)k_cert_data, 20, NULL));
        ASSERT_CERT_ADDR(auth_data_ptr);
        ASSERT_DATA_ADDR((void*)k_cert_data);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_data_cert_register(cert, 24, cert, 20, &id));
        ASSERT_CERT_ADDR(cert); /*asserted against stored values*/
        ASSERT_DATA_ADDR(cert); /*asserted against stored values*/
        ASSERT_OBJID(id); /*asserted against known values*/
    }
#if 0
    {   /* sec_hal_random_number_get. */
        uint8_t random[100];
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_random_number_get(0, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_random_number_get(46, NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_random_number_get(46, random));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_random_number_get(46, random));
        test_set_disp_status(SEC_ROM_RET_OK);
        /* fails due to lack of ICRAM0, in use only 4*1024 */
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_random_number_get(8*1024, random));
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_random_number_get(46, random));
        ASSERT_CERT_RANDOM_NUMBER(46, random); /*asserted against known values*/
    }
#endif
    {   /* sec_hal_mac_address_get. */
        sec_hal_mac_address_t mac_addr;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_mac_address_get(0, NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_mac_address_get(0, &mac_addr));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_mac_address_get(0, &mac_addr));
        test_set_disp_status(SEC_SERV_STATUS_OK);
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_mac_address_get(0, &mac_addr));
        ASSERT_MACADDR(mac_addr.mac_address); /*asserted against known values*/
    }
    {   /* sec_hal_imei_get. */
        sec_hal_imei_t imei;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_imei_get(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_imei_get(&imei));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_imei_get(&imei));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_imei_get(&imei));
        ASSERT_IMEI(imei.imei); /*asserted against known values*/
    }
#if 0
    {   /* sec_hal_rat_band_get. */
        sec_hal_rat_band_t rat_band;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rat_band_get(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rat_band_get(&rat_band));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rat_band_get(&rat_band));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rat_band_get(&rat_band));
        ASSERT_RAT_BAND(&rat_band);
    }
    {   /* sec_hal_pp_flags_count_get. */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_pp_flags_count_get(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_pp_flags_count_get(&count));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_pp_flags_count_get(&count));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_pp_flags_count_get(&count));
        ASSERT_PP_FLAGS_COUNT(count);
    }
    {   /* sec_hal_pp_flags_get. */
        sec_hal_pp_flag_t sec_pp_flags[10];
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_pp_flags_get(0, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_pp_flags_get(count, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_pp_flags_get(0, sec_pp_flags));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_pp_flags_get(count, sec_pp_flags));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_pp_flags_get(count, sec_pp_flags));
        ASSERT_PP_FLAGS((uint8_t *)sec_pp_flags);
    }
#endif
    {   /* sec_hal_rt_simlock_levels_open. */
        char *valid_unlock_codes = "ONE\0TWO\0THREE\0FOUR\0FIVE";
        char *invalid_unlock_codes = "NOT\0ONE\0ONE\0ONE\0ONE";
        char *too_long_unlock_codes = "qwertyuiopasdfghjhglkh\0qwertyuiopasdfghjhglkh\0qwertyuiopasdfghjhglkh\0qwertyuiopasdfghjhglkh\0qwertyuiopasdfghjhglkh\0qwertyuiopasdfghjhglkh";
        uint32_t post_status;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_levels_open(0, NULL, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_levels_open(0x0F, NULL, NULL));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_levels_open(0x0F, valid_unlock_codes, NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_levels_open(0x0F, valid_unlock_codes, &post_status));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_levels_open(0x0F, valid_unlock_codes, &post_status));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_levels_open(0x1F, too_long_unlock_codes, &post_status));
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_simlock_levels_open(0x1F, valid_unlock_codes, &post_status));
        ASSERT(0x00 == post_status); /* all levels should be open */
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_simlock_levels_open(0x1F, invalid_unlock_codes, &post_status));
        ASSERT(0x1F == post_status); /* all levels should be closed */
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_simlock_levels_open(0x05, valid_unlock_codes, &post_status));
        ASSERT(0x1A == post_status); /* some of the levels should be open(1st and 3rd) */
    }
    {   /* sec_hal_rt_simlock_level_open. */
        char unlock_code_level3[SEC_HAL_MAX_SIMLOCK_CODE_LENGTH] = "THREE";
        char wrong_unlock_code[SEC_HAL_MAX_SIMLOCK_CODE_LENGTH] = "THEWRONG";
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_level_open(NULL, 0));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_level_open(unlock_code_level3, 0));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_level_open(unlock_code_level3, 2));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_level_open(unlock_code_level3, 2));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_level_open(unlock_code_level3, 2));/* fails due to wrong code for this particular level */
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_level_open(wrong_unlock_code, 3));/* fails due to wrong code for this particular level */
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_simlock_level_open(unlock_code_level3, 3));
    }
    {   /* sec_hal_rt_simlock_level_status_get. */
        uint32_t ll_status = 0x00;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_simlock_level_status_get(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_level_status_get(&ll_status));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_simlock_level_status_get(&ll_status));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_simlock_level_status_get(&ll_status));
        ASSERT(ll_status == 0x07);
    }
    {   /* sec_hal_rt_auth_data_size_get. */
        void* auth_data_ptr = (void*) k_input_auth_data;
        count = strlen((char*)auth_data_ptr);
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_size_get(auth_data_ptr, 0,  &size));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_size_get(NULL, count, &size));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_size_get(auth_data_ptr, count, NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_size_get(auth_data_ptr, count, &size));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_size_get(auth_data_ptr, count, &size));
        test_set_disp_status(SEC_ROM_RET_OK);
#ifdef SEC_HAL_TEST_ALLOC
        /* fails due to lack of ICRAM0, in use only 4*1024 */
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_size_get(auth_data_ptr, 8*1024, &size));
#endif /* SEC_HAL_TEST_ALLOC */
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_auth_data_size_get(auth_data_ptr, count, &size));
        ASSERT(2*count == size);
    }
    {   /* sec_hal_rt_auth_dev_data_get. */
        uint8_t auth_data[64];
        void* auth_data_ptr = (void*) k_input_auth_data;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_get(NULL, 0, NULL, 0));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_get(NULL, count, NULL, 0));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_get(auth_data_ptr, count, NULL, 0));
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_auth_data_get(auth_data_ptr, count, NULL, 2*count));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_get(auth_data_ptr, count, auth_data, 2*count));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_get(auth_data_ptr, count, auth_data, 2*count));
        test_set_disp_status(SEC_ROM_RET_OK);
#ifdef SEC_HAL_TEST_ALLOC
        /* fails due to lack of ICRAM0, in use only 4*1024 */
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_get(auth_data_ptr, 8*1024, auth_data, 2*count));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_auth_data_get(auth_data_ptr, count,  auth_data, 8*1024));
#endif /* SEC_HAL_TEST_ALLOC */
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_auth_data_get(auth_data_ptr, count, auth_data, 2*count));
        count = strlen((char*)auth_data_ptr);
        ASSERT_AUTH_DATA(auth_data_ptr, count, auth_data); /*(input,output)*/
    }
    {   /* sec_hal_periodic_integrity_check. */
        uint32_t periodic_timer = 0x00;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_periodic_integrity_check(NULL));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_periodic_integrity_check(&periodic_timer));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_periodic_integrity_check(&periodic_timer));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_periodic_integrity_check(&periodic_timer));
        ASSERT_PER_TIMEOUT(periodic_timer);
    }
    {   /* sec_hal_selftest. */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_selftest());
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_selftest());
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_selftest());
    }
    {   /* sec_hal_rt_coma_entry */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_coma_entry(1,2,3,4));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_coma_entry(5,6,7,8));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_coma_entry(9,10,11,12));
        ASSERT_COMA_PARAMS(9,10,11,12);
    }
    {   /* sec_hal_rt_coma_cpu_off */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_coma_cpu_off());
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_coma_cpu_off());
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_coma_cpu_off());
    }
    {   /* sec_hal_rt_poweroff */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_poweroff());
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_poweroff());
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_poweroff());
    }
    {   /* sec_hal_rt_public_cc42_key_init */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_public_cc42_key_init());
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_public_cc42_key_init());
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_public_cc42_key_init());
    }
    {   /* sec_hal_rt_a3sp_state_info */
		uint32_t allowed = 0x00;
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_a3sp_state_info(0, NULL));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_a3sp_state_info(1, &allowed));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_a3sp_state_info(1, &allowed));
		ASSERT_A3SP_STATE_INFO(1, allowed);
    }
    {   /* sec_hal_rt_memcpy */
        test_set_status(SEC_SERV_STATUS_FAIL);
        test_set_disp_status(SEC_ROM_RET_FAIL);
        ASSERT(SEC_HAL_RES_PARAM_ERROR == sec_hal_rt_memcpy(0x00,NULL,0));
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_memcpy(0x02, 0x0000FFDE, 7));
        test_set_status(SEC_SERV_STATUS_OK);
        ASSERT(SEC_HAL_RES_FAIL == sec_hal_rt_memcpy(0x02, 0x0000FFDE, 7));
        test_set_disp_status(SEC_ROM_RET_OK);
        ASSERT(SEC_HAL_RES_OK == sec_hal_rt_memcpy(0x10001000, 0xF000FFDE, 7));
        ASSERT_MEMCPY_PARAMS(0x10001000, 0xF000FFDE, 7);
    }

    SEC_HAL_TRACE("ENDING SANITY CHECK.")
    return 0;
}

/* ******************************** END ************************************ */
