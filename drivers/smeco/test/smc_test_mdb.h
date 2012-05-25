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

#ifndef SMC_TEST_MDB_H
#define SMC_TEST_MDB_H

uint8_t smc_test_case_function_mdb( uint8_t* test_input_data, uint16_t test_input_data_len );
uint8_t smc_test_mdb_channel_test1( void );
uint8_t smc_test_mdb_address_check( void );
uint8_t smc_test_mdb_alloc_dealloc( void );
uint8_t smc_test_mdb_dealloc_wrong_pool( void );
uint8_t smc_test_mdb_copy_test( void ); 
uint8_t smc_test_mdb_perf( void );
uint8_t smc_test_mdb_map_test( uint32_t *map, uint32_t allocs, uint32_t deallocs, smc_channel_t* smc_channel );
void smc_test_mdb_random_alloc(void);

#endif
