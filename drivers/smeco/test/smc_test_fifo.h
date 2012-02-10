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

#ifndef SMC_TEST_FIFO_H
#define SMC_TEST_FIFO_H


typedef struct
{
    void *ptr;
    int   length;
    int   flags;
} fifo_test_sample_t;

#define MAX_FIFO_SIZE          32
#define MAX_SAMPLES           (MAX_FIFO_SIZE+1)
#define TEST_CASE_FACTOR       100


#define SMC_TEST_CREATE_FIFO( name, length )       \
uint8_t _##name##_fifoa_[ sizeof(smc_fifo_t) +     \
(length)*sizeof(smc_fifo_cell_t) ];                \
smc_fifo_t *name = (smc_fifo_t *)_##name##_fifoa_

uint8_t smc_test_case_function_fifo( uint8_t* test_input_data, uint16_t test_input_data_len );


#endif /* End Of File */

