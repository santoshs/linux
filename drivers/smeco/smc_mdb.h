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

Version:       3    27-Dec-2011     Janne Mahosenaho
Status:        draft
Description :  Alternative TLSF implementation added.
-------------------------------------------------------------------------------
Version:       1    01-Dec-2011     Jussi Pellinen
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#if !defined(__GNUC__)
#ifndef __inline__
#define __inline__ inline
#endif
#endif

#ifndef TLSF_USE_REF
#define TLSF_USE_REF 0
#endif

#ifndef SMC_MDB_H
#define SMC_MDB_H


#define SMC_MDB_IN  0
#define SMC_MDB_OUT 1

typedef struct
{
    int8_t   id;
    void*    pool_in;
    void*    pool_out;
    uint32_t total_size_in;
    uint32_t total_size_out;
} smc_mdb_channel_info_t;

typedef struct
{
    uint8_t channel_amount;
    smc_mdb_channel_info_t channel_info[1];
} smc_mdb_channel_info_set_t;


uint32_t smc_mdb_calculate_required_shared_mem( uint32_t mdb_size );
int32_t  smc_mdb_create( uint8_t channel_id, void* pool_address, uint32_t pool_size );
int32_t  smc_mdb_init( uint8_t channel_id, void* pool_address, uint32_t pool_size );
void     smc_mdb_destroy( uint8_t channel_id );
void     smc_mdb_all_destroy( void );
void*    smc_mdb_alloc( uint8_t channel_id, uint32_t size );
void     smc_mdb_free( uint8_t channel_id, void* ptr );
void     smc_mdb_copy( void* target_ptr, void* source_ptr, uint32_t length );
uint8_t  smc_mdb_address_check( uint8_t channel_id, void* ptr, uint8_t direction );
void   	 smc_mdb_channel_info_dump( void );
uint32_t smc_mdb_channel_frag_get( uint8_t channel_id );
uint32_t smc_mdb_channel_free_space_get( uint8_t channel_id );
uint32_t smc_mdb_channel_free_space_min_get( uint8_t channel_id );
uint32_t smc_mdb_channel_largest_free_block_get( uint8_t channel_id );

#if !TLSF_USE_REF
typedef void* tlsf_pool;
/* Debugging. */
typedef void (*tlsf_walker)(void* ptr, size_t size, int used, void* user);
void tlsf_walk_heap(tlsf_pool pool, tlsf_walker walker, void* user);
/* Returns nonzero if heap check fails. */
int tlsf_check_heap(tlsf_pool pool);

/* Returns internal block size, not original request size */
size_t tlsf_block_size(void* ptr);
/* Returns external block size, should be equal or greater 
 * than the original request size */
size_t tlsf_ptr_size(void * ptr);

/* Overhead of per-pool internal structures. */
size_t tlsf_overhead( void );
#endif

#endif



