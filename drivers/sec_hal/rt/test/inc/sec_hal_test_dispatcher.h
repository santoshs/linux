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

#include "sec_hal_rt.h"

void test_set_status(uint32_t status);
void test_set_disp_status(uint32_t status);

int test_assert_key_info(uint8_t* key_info);
int test_assert_random_number(int size, uint8_t* random_number);
int test_assert_cert_address(void *address);
int test_assert_data_address(void *address);
int test_assert_objid(uint32_t objid);
int test_assert_mac_address(uint8_t* mac_address);
int test_assert_imei(char* imei);
int test_assert_auth_data(uint8_t* input_buf,
                          uint8_t input_len,
                          uint8_t* output_buf);
int test_assert_rat_band(sec_hal_rat_band_t* rat_band);
int test_assert_pp_flags_count(int count);
int test_assert_pp_flags(uint8_t* input_buf);
int test_assert_exp_time(uint32_t exp_time);
int test_assert_rpc_handler(void* func_ptr);
int test_assert_coma_params(uint32_t param0,
                            uint32_t param1,
                            uint32_t param2,
                            uint32_t param3);
int test_assert_a3sp_state_params(uint32_t param0,
                                  uint32_t param1);
int test_assert_memcpy_params(uint32_t param0,
                              uint32_t param1,
                              uint32_t param2);

