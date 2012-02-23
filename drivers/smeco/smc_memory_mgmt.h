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

Version:       6    03-Feb-2012     Jussi Pellinen
Status:        draft
Description:   Offset type SMC_SHM_OFFSET_MDB_OFFSET added

Version:       5    02-Dec-2011     Heikki Siikaluoma
Status:        draft
Description:   Code clean up

Version:       1    19-Oct-2011     Heikki Siikaluoma
Status:        draft
Description:   File created
-------------------------------------------------------------------------------
*/
#endif


#ifndef SMC_MEMORY_MGMT_H
#define SMC_MEMORY_MGMT_H

    /* Remote SHM memory offset policy */
typedef enum
{
    SMC_SHM_OFFSET_EQUAL = 0x00,
    SMC_SHM_OFFSET_ADD,
    SMC_SHM_OFFSET_SUBTRACT,
    SMC_SHM_OFFSET_MDB_OFFSET
} SMC_SHM_OFFSET_TYPE;

typedef struct
{
    uint8_t*            shm_area_start_address;            /* Start address of the SHM */
    uint32_t            size;                              /* Size of SHM in bytes */

    uint8_t             use_cache_control;                 /* If TRUE the user is responsible of cache clean/invalidate */
    uint8_t             cache_line_len;                    /* Length of cache line in bytes */
    uint8_t             reserved;                          /* Fill */
    SMC_SHM_OFFSET_TYPE remote_cpu_memory_offset_type;     /* SHM memory offset policy of the remote CPU. Uses constants SMC_SHM_OFFSET_xxx */

    uint32_t            remote_cpu_memory_offset;          /* Offset to remote CPU SHM physical address */

} smc_shm_config_t;


    /*
     * Shared memory API functions
     */

smc_shm_config_t* smc_shm_conf_create ( void );
void              smc_shm_conf_destroy( smc_shm_config_t* smc_shm_conf);

smc_shm_config_t* smc_shm_config_copy ( smc_shm_config_t* source_shm_config );

    /*
     * Platform specific shared memory API functions
     * Implementation is in the directory
     * - /modem
     * - /linux
     */
/* Cache controls are changed to macros
 * TODO Cleanup
void smc_shm_cache_invalidate(void* start_address, void* end_address);
void smc_shm_cache_clean     (void* start_address, void* end_address);
*/

/* Remove these */
#define smc_shm_cache_invalidate  SMC_SHM_CACHE_INVALIDATE
#define smc_shm_cache_clean  SMC_SHM_CACHE_CLEAN

#endif /* EOF */

