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

Version:       27   03-Jul-2012     Andrey Derkach
Status:        draft
Description :  FIFO polling function updated to keep polling forever since it is 
               used only for XFile and core dump transfers

Version:       12   04-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Merged SHM OFFSET rule changes, code cleanup

Version:       10   04-Jan-2012     Heikki Siikaluoma
Status:        draft
Description :  Code improvements, clean up, SMC ID field and instance array removed

Version:       3    08-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  Platform independent code implemented

Version:       1    19-Oct-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#include "smc_common_includes.h"

#include "smc.h"
#include "smc_trace.h"
#include "smc_mdb.h"

#if(SMCTEST==TRUE)
  #include "smc_test.h"
#endif

    /*
     * Global variables
     */

uint8_t                smc_initialized          = FALSE;
uint8_t                signal_handler_count     = 0;
smc_signal_handler_t** signal_handler_ptr_array = NULL;

#ifdef SMC_PERFORM_VERSION_CHECK

    smc_version_check_t    g_smc_version_requirements[SMC_VERSION_REQUIREMENT_COUNT] =
    {
        SMC_VERSION_REQUIREMENT_ARRAY
    };

#endif

    /*
     * Local static variables
     */
static smc_lock_t* g_local_lock_smc_channel            = NULL;
static smc_lock_t* g_local_lock_signal_handler         = NULL;
static smc_lock_t* g_local_lock_smc_channel_sync       = NULL;
static smc_lock_t* g_local_lock_smc_fifo_buffer_flush  = NULL;
static smc_lock_t* g_local_lock_smc_fifo_buffer        = NULL;

static inline smc_lock_t* get_local_lock_smc_channel(void)
{
    if( g_local_lock_smc_channel == NULL ) g_local_lock_smc_channel = smc_lock_create();
    return g_local_lock_smc_channel;
}

static inline smc_lock_t* get_local_lock_signal_handler(void)
{
    if( g_local_lock_signal_handler == NULL ) g_local_lock_signal_handler = smc_lock_create();
    return g_local_lock_signal_handler;
}

static inline smc_lock_t* get_local_lock_smc_channel_sync(void)
{
    if( g_local_lock_smc_channel_sync == NULL ) g_local_lock_smc_channel_sync = smc_lock_create();
    return g_local_lock_smc_channel_sync;
}

static inline smc_lock_t* get_local_lock_smc_fifo_buffer_flush(void)
{
    if( g_local_lock_smc_fifo_buffer_flush == NULL ) g_local_lock_smc_fifo_buffer_flush = smc_lock_create();
    return g_local_lock_smc_fifo_buffer_flush;
}

static inline smc_lock_t* get_local_lock_smc_fifo_buffer(void)
{
    if( g_local_lock_smc_fifo_buffer == NULL ) g_local_lock_smc_fifo_buffer = smc_lock_create();
    return g_local_lock_smc_fifo_buffer;
}

    /*
     * Local static functions
     */
static uint8_t  smc_channel_configure_shm( smc_channel_t* smc_channel, smc_shm_config_t* smc_shm_conf, uint8_t is_master);
static uint8_t* smc_instance_calculate_next_free_shm_address( smc_t* smc_instance, uint32_t bytes_consumed);
static uint8_t  smc_channel_handle_sync( smc_channel_t* smc_channel, uint32_t sync_flag, uint32_t sync_msg );
static uint8_t  smc_channel_buffer_fifo_message(smc_channel_t* channel, void* data, uint32_t data_length, smc_user_data_t* userdata);

static uint8_t  smc_channel_buffer_fifo_flush( smc_channel_t* channel );
static uint8_t  smc_send_negotiable_configurations( smc_t* smc_instance );
static uint8_t  smc_send_channels_initialized_message(smc_t* smc_instance);
static void     smc_channel_execute_fixed_config( smc_channel_t* smc_channel_target, smc_channel_runtime_fixed_conf_t* configuration, smc_user_data_t* userdata_resp);

static uint8_t  smc_handle_loopback_data_message( smc_channel_t* smc_channel, smc_loopback_data_t* loopback_data, smc_user_data_t* userdata);

    /* TODO Each channel should have own timer
     * FIFO timer cb function */
static void     smc_fifo_timer_expired(uint32_t timer_data);

    /*
     * Local static inline functions
     */
static inline void*    smc_allocate_local_ptr     ( const smc_channel_t* smc_channel, uint32_t size, smc_user_data_t* userdata );
static inline void     smc_free_local_ptr         ( const smc_channel_t* smc_channel, void* ptr, smc_user_data_t* userdata );
static inline uint32_t smc_local_address_translate( const smc_channel_t* smc_channel, uint32_t ptr, uint32_t flags );
static inline uint8_t  smc_handle_fifo_event      (smc_channel_t* channel, SMC_CHANNEL_EVENT fifo_event);

    /*
     * Extern functions
     */
extern uint8_t  smc_module_initialize( smc_conf_t* smc_instance_conf );


    /*
     * Array of all SMC instances created
     */
static smc_t** g_smc_instance_array       = NULL;
static uint8_t g_smc_instance_array_count = 0;


    /*
     * Function implementations
     */


static void smc_instance_add( smc_t* smc_instance )
{
    smc_t**     old_array  = NULL;
    smc_lock_t* local_lock = NULL;

    local_lock = get_local_lock_smc_channel();  /* use the same lock as the channel */
    SMC_LOCK_IRQ( local_lock );

    SMC_TRACE_PRINTF_DEBUG("smc_instance_add: add instance 0x%08X, count %d", (uint32_t)smc_instance, g_smc_instance_array_count);

    if( g_smc_instance_array )
    {
        old_array = g_smc_instance_array;
    }

    g_smc_instance_array_count++;

    g_smc_instance_array = (smc_t**)SMC_MALLOC( sizeof(smc_t*) * g_smc_instance_array_count );

    if( old_array )
    {
        int i = 0;

        for( i = 0; i < g_smc_instance_array_count-1; i++ )
        {
            g_smc_instance_array[i] = old_array[i];
        }

        g_smc_instance_array[g_smc_instance_array_count-1] = smc_instance;

        SMC_FREE( old_array );
        old_array = NULL;
    }
    else
    {
        g_smc_instance_array[0] = smc_instance;
    }

    SMC_TRACE_PRINTF_DEBUG("smc_instance_add: instance 0x%08X added, count %d", (uint32_t)smc_instance, g_smc_instance_array_count);

    SMC_UNLOCK_IRQ( local_lock );
}

static void smc_instance_remove( smc_t* smc_instance )
{
    smc_lock_t* local_lock = NULL;

    local_lock = get_local_lock_smc_channel();  /* Use the same lock as the channel */
    SMC_LOCK_IRQ( local_lock );

    SMC_TRACE_PRINTF_DEBUG("smc_instance_remove: remove instance 0x%08X, count %d", (uint32_t)smc_instance, g_smc_instance_array_count);

    if( g_smc_instance_array != NULL && g_smc_instance_array_count > 0 )
    {
        smc_t** old_array      = NULL;
        uint8_t instance_found = 0;
        uint8_t instance_index = 0;

        for( int i = 0; i < g_smc_instance_array_count; i++ )
        {
            if( g_smc_instance_array[i] == smc_instance )
            {
                instance_found = 1;
                instance_index = i;
                break;
            }
        }

        old_array = g_smc_instance_array;

        if( instance_found > 0 )
        {
            g_smc_instance_array_count--;

            if( g_smc_instance_array_count > 0 )
            {
                g_smc_instance_array = (smc_t**)SMC_MALLOC( sizeof(smc_t*) * g_smc_instance_array_count );
            }
            else
            {
                SMC_FREE( g_smc_instance_array );
                g_smc_instance_array       = NULL;
                g_smc_instance_array_count = 0;
            }

            if( old_array && g_smc_instance_array)
            {
                uint8_t temp_index = 0;

                for(int i = 0; i < g_smc_instance_array_count+1; i++ )
                {
                    if( i != instance_index )
                    {
                        g_smc_instance_array[temp_index] = old_array[i];
                        temp_index++;
                    }
                }

                SMC_FREE(old_array);
                old_array = NULL;
            }
        }
    }

    SMC_TRACE_PRINTF_DEBUG("smc_instance_remove: instance 0x%08X removed, count %d", (uint32_t)smc_instance, g_smc_instance_array_count);

    SMC_UNLOCK_IRQ( local_lock );
}


char* smc_get_version( void )
{
    return SMC_SW_VERSION;
}

char* smc_version_to_str(uint32_t version)
{
    char* version_str = NULL;
    int   str_len     = 3+3+1;
    char* temp_str    = NULL;
    int   version_ind = 0;

    uint8_t  v1 = (version>>24)&0xFF;
    uint8_t  v2 = (version>>16)&0xFF;
    uint16_t v3 =  version&0xFFFF;

    SMC_TRACE_PRINTF_INFO("smc_version_to_str: convert version 0x%08X to string...", version);

    if( v1 > 9 )
    {
        str_len++;
    }

    if( v2 > 9 )
    {
        str_len++;
    }

    if( v3 > 9 )
    {
        str_len++;

        if( v3 > 99 )
        {
            str_len++;

            if( v3 > 999 )
            {
                str_len++;
            }
        }
    }

    version_str = (char*)SMC_MALLOC_IRQ(str_len);

    temp_str = smc_utoa( v1 );
    strcpy( version_str, temp_str );
    version_ind += strlen(temp_str);
    SMC_FREE( temp_str );

    strcpy( (version_str+version_ind), "." );
    version_ind += 1;

    temp_str = smc_utoa( v2 );
    strcpy( (version_str + version_ind), temp_str );
    version_ind += strlen(temp_str);
    SMC_FREE( temp_str );

    strcpy( (version_str+version_ind), "." );
    version_ind += 1;

    temp_str = smc_utoa( v3 );
    strcpy( (version_str + version_ind), temp_str );
    version_ind += strlen(temp_str);
    SMC_FREE( temp_str );

    SMC_TRACE_PRINTF_INFO("smc_version_to_str: version 0x%08X converted to '%s'...", version, version_str);

    return version_str;
}

uint32_t smc_version_to_int(char* version)
{
    uint32_t version_int        = 0x00000000;
    int      level              = 0;
    int      version_ind        = 0;
    int      str_len            = strlen( version );
    char     version_number[10];

    SMC_TRACE_PRINTF_INFO("smc_version_to_int: convert version '%s' to unsigned integer, len = %d..", version, str_len);

    memset(version_number, 0, 10);

    for(int i = 0; i < str_len; i++)
    {
        char ch = version[i];

        if( ch != '.' || ( level<=1 && version_ind>2) || ( level==2 && version_ind>4) )
        {
            if( ch >= 48 && ch <= 57 )
            {
                version_number[version_ind] = ch;
                version_ind++;
            }

            if(( level<=1 && version_ind>2) || ( level==2 && version_ind>4))
            {
                level++;

                if( level > 2 )
                {
                    break;
                }
            }
        }
        else
        {
            int vers = smc_atoi( version_number );

            if( level == 0 )
            {
                version_int += ( (vers&0xFF) << 24 );
            }
            else if( level == 1 )
            {
                version_int += ( (vers&0xFF) << 16 );
            }
            else
            {
                version_int += ( vers&0xFFFF );
            }

            memset(version_number, 0, 10);
            version_ind = 0;
            level++;
        }

        if( level > 2 )
        {
            break;
        }
    }

    if( strlen( version_number ) > 0 )
    {
        int vers = smc_atoi( version_number );

        if( level == 0 )
        {
            version_int += ( (vers&0xFF) << 24 );
        }
        else if( level == 1 )
        {
            version_int += ( (vers&0xFF) << 16 );
        }
        else
        {
            version_int += ( vers&0xFFFF );
        }
    }

    SMC_TRACE_PRINTF_INFO("smc_version_to_int: version '%s' converted to 0x%08X", version, version_int);

    return version_int;
}

int smc_atoi(char* a)
{
    int number  = 0;
    int str_len = strlen( a );
    int iPow    = 1;

    for(int i = str_len-1; i >= 0; i--)
    {
       char ch = a[i];

       if( ch >= 48 && ch <= 57 )
       {
           number += ( ch-48 ) * iPow;
           iPow *= 10;
       }
    }

    return number;
}

char* smc_utoa(uint32_t i)
{
    static char int_digits[]    = "0123456789";
    char*       int_str         = NULL;
    int         iNrIndx         = 0;
    char        temp[9];
    uint32_t    version_counter = i;
    int         divider_counter = 10;

    memset(temp, 0, 9);

    while( divider_counter >= 0 )
    {
        uint8_t mod = 0;
        uint32_t iPow = 1;

        for( int x = 0; x < divider_counter; x++)
        {
            iPow *= 10;
        }

        mod = (version_counter / iPow);

        if( !(mod==0 && divider_counter>0) )
        {
            temp[iNrIndx] = int_digits[mod];
            iNrIndx++;
            version_counter -= (mod*iPow);
        }

        divider_counter--;
    }

    int_str = (char*)SMC_MALLOC_IRQ(strlen(temp)+1);

    strcpy(int_str, temp);

    return int_str;
}

/*
 * General initialization function
 * This is not required to use.
 */
uint8_t smc_initialize( smc_conf_t* smc_instance_conf )
{
    if( !smc_initialized )
    {
        uint8_t return_value = SMC_ERROR;

        SMC_TRACE_PRINTF_VERSION("Version %s", SMC_SW_VERSION);

        RD_TRACE_SEND1(TRA_SMC_INIT, 2, &return_value);

        SMC_TRACE_PRINTF_DEBUG("smc_initialize: starting...");

#if(SMCTEST==TRUE)
        smc_test_handler_register();
#endif

        if( smc_instance_conf == NULL )
        {
            SMC_TRACE_PRINTF_WARNING("smc_initialize: SMC initialization is NULL");
        }

            /* Call the module specific SMC initialization */
        return_value = smc_module_initialize( smc_instance_conf );

        smc_initialized = TRUE;

        SMC_TRACE_PRINTF_DEBUG("smc_initialize: completed by return value 0x%02X", return_value);
        return return_value;
    }
    else
    {
        return SMC_OK;
    }
}

/*
 * Allocates local memory for data.
 */
static inline void* smc_allocate_local_ptr( const smc_channel_t* smc_channel, uint32_t size, smc_user_data_t* userdata )
{
    void* ptr = NULL;

    if( smc_channel->smc_receive_data_allocator_cb && !SMC_IS_LOOPBACK_MSG(userdata->flags) )
    {
        SMC_TRACE_PRINTF_INFO("smc_allocate_local_ptr: Channel %d (0x%08X) allocating ptr using callback 0x%08X", smc_channel->id, (uint32_t)smc_channel, (uint32_t)smc_channel->smc_receive_data_allocator_cb);

        ptr = smc_channel->smc_receive_data_allocator_cb( smc_channel, size, userdata );
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_allocate_local_ptr: Channel %d (0x%08X) allocating from OS %s", smc_channel->id, (uint32_t)smc_channel, SMC_IS_LOOPBACK_MSG(userdata->flags)?"(Loopback message)":"");
        ptr = (void*)SMC_MALLOC_IRQ( size );
    }

    SMC_TRACE_PRINTF_INFO("smc_allocate_local_ptr: Channel %d (0x%08X) allocated ptr 0x%08X", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr);

    return ptr;
}

/**
 * Frees pointer that was locally allocated in the channel operations.
 */
void smc_channel_free_ptr_local(const  smc_channel_t* smc_channel, void* ptr, smc_user_data_t* userdata)
{
    smc_free_local_ptr( smc_channel, ptr, userdata );
}

/*
 * Deallocates local pointer from proper source.
 */
static inline void smc_free_local_ptr(const  smc_channel_t* smc_channel, void* ptr, smc_user_data_t* userdata)
{
    SMC_TRACE_PRINTF_INFO("smc_free_local_ptr: channel %d (0x%08X): free ptr 0x%08X", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr);

    /* First Check if in MDB, then use loopback, CB or OS */
    if( SMC_MDB_ADDRESS_IN_POOL_OUT( ptr, smc_channel->smc_mdb_info ))
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: channel %d (0x%08X): deallocating ptr 0x%08X from MDB OUT...", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr);

        SMC_LOCK_IRQ( ((smc_channel_t*)smc_channel)->lock_mdb );

#ifdef SMC_XTILESS_CORE_DUMP_ENABLED
        if( userdata->userdata3 == 0xF1F0F1F0 )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: channel %d (0x%08X): not deallocating ptr 0x%08X located MDB OUT (XFILE dumping SMC SHM)...", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr);
            return;
        }
#endif

        smc_mdb_free( smc_channel, ptr );

        SMC_UNLOCK_IRQ( ((smc_channel_t*)smc_channel)->lock_mdb );

        // TODO This is temporary implementation for L2MUX->GPDS mem free, make more robust check
        if( smc_channel->smc_send_data_deallocator_cb != NULL && userdata->userdata2 > 0)
        {
            SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: channel %d (0x%08X): deallocating ptr 0x%08X from using CB and userdata2 0x%08X...", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr, userdata->userdata2);

            smc_channel->smc_send_data_deallocator_cb( smc_channel, ptr, userdata );
        }
    }
    else if( SMC_IS_LOOPBACK_MSG(userdata->flags) )
    {
        uint8_t is_in_mdb = ( SMC_MDB_ADDRESS_IN_POOL_OUT( ptr, smc_channel->smc_mdb_info ) ||  SMC_MDB_ADDRESS_IN_POOL_IN( ptr, smc_channel->smc_mdb_info ));

        if( is_in_mdb )
        {
            SMC_TRACE_PRINTF_ASSERT("smc_free_local_ptr: ERROR *** loopback ptr re-used directly from MDB due to zero copy");
            assert(0);
        }

        SMC_TRACE_PRINTF_LOOPBACK("smc_free_local_ptr: Channel %d: deallocating ptr 0x%08X used in loopback data...",
                                smc_channel->id, (uint32_t)ptr);
        SMC_FREE( ptr );

        ptr = NULL;
    }
    else if( SMC_FIFO_DATA_PTR_IS_LOCAL(userdata->flags) )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: Channel %d: deallocating local ptr 0x%08X using OS...", smc_channel->id, (uint32_t)ptr );

        SMC_FREE( ptr );
        ptr = NULL;
    }
    else if( smc_channel->smc_send_data_deallocator_cb != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: Channel %d: deallocating ptr 0x%08X using deallocator callback 0x%08X...",
                        smc_channel->id, (uint32_t)ptr, (uint32_t)smc_channel->smc_send_data_deallocator_cb);

        smc_channel->smc_send_data_deallocator_cb( smc_channel, ptr, userdata );
    }
#ifndef SMC_FREE_LOCAL_PTR_OS_NOT_NEEDED
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: Channel %d: deallocating ptr 0x%08X using OS...", smc_channel->id, (uint32_t)ptr );

        SMC_FREE( ptr );
        ptr = NULL;
    }
#endif
}

static inline uint32_t smc_local_address_translate( const smc_channel_t* smc_channel, uint32_t ptr, uint32_t flags)
{
    uint32_t          new_ptr          = ptr;
    smc_shm_config_t* shm_conf_channel = smc_channel->smc_shm_conf_channel;

    if( shm_conf_channel->remote_cpu_memory_offset_type == SMC_SHM_OFFSET_MDB_OFFSET )
    {
        smc_mdb_channel_info_t* smc_mdb_pool_info = smc_channel->smc_mdb_info;

        if( !SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ(flags) && !SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP(flags) )
        {
            if( !SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB(flags) )
            {
                if( SMC_MDB_ADDRESS_IN_POOL_OUT( ptr, smc_mdb_pool_info ))
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_local_address_translate: channel %d: SHM address to offset translation required for address 0x%08X (remove MDB OUT offset 0x%08X)",
                            smc_channel->id, ptr, smc_mdb_pool_info->pool_out);
                    new_ptr = ptr - (uint32_t)smc_mdb_pool_info->pool_out;
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address to offset translation performed, offset 0x%08X", smc_channel->id, new_ptr);
                }
                else if( SMC_MDB_ADDRESS_IN_POOL_IN( (ptr + smc_channel->mdb_in), smc_mdb_pool_info ))
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_local_address_translate: channel %d: SHM offset to address translation required for offset 0x%08X (add MDB IN offset 0x%08X)",
                            smc_channel->id, ptr, smc_mdb_pool_info->pool_in);
                    new_ptr = ptr + (uint32_t)smc_mdb_pool_info->pool_in;
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM offset to address translation performed, new address from 0x%08X is 0x%08X", smc_channel->id, ptr, new_ptr);
                }
                else
                {
                    SMC_TRACE_PRINTF_ASSERT("smc_local_address_translate: channel %d (0x%08X) address translation failed, pointer 0x%08X is not in any MDB pool: IN 0x%08X / OUT 0x%08X",
                            smc_channel->id, (uint32_t)smc_channel, ptr,
                            (uint32_t)smc_mdb_pool_info->pool_in,
                            (uint32_t)smc_mdb_pool_info->pool_out);

                    assert(0);
                }
            }
            else
            {
                if( SMC_MDB_ADDRESS_IN_POOL_IN( ptr, smc_channel->smc_mdb_info ))
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_local_address_translate: channel %d: SHM address to offset translation required for address 0x%08X (FREE from MDB IN)", smc_channel->id, ptr);
                    new_ptr = ptr - (uint32_t)smc_channel->mdb_in;
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address to offset translation performed, new address from 0x%08X is 0x%08X", smc_channel->id, ptr, new_ptr);
                }
                else if( SMC_MDB_ADDRESS_IN_POOL_OUT( (ptr + smc_channel->mdb_out), smc_channel->smc_mdb_info ))
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_local_address_translate: channel %d: SHM offset to address translation required for offset 0x%08X (FREE from MDB OUT)", smc_channel->id, ptr);
                    new_ptr = ptr + (uint32_t)smc_channel->mdb_out;
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM offset to address translation performed, address 0x%08X", smc_channel->id, new_ptr);
                }
                else
                {
                    SMC_TRACE_PRINTF_ASSERT("smc_local_address_translate: channel %d (0x%08X) address translation failed, pointer 0x%08X is not in any MDB pool",
                                                smc_channel->id, (uint32_t)smc_channel, ptr);
                    assert(0);
                }
            }
        }
        else
        {
            SMC_TRACE_PRINTF_INFO("smc_local_address_translate: Channel %d: SHM address translation not performed for SYNC message", smc_channel->id);
        }
    }
    else if( shm_conf_channel->remote_cpu_memory_offset_type != SMC_SHM_OFFSET_EQUAL )
    {
        if( !SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ(flags) && !SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP(flags) )
        {
            SMC_TRACE_PRINTF_INFO("smc_local_address_translate: Channel %d: SHM address translation (type: 0x%02X by %d) required for address 0x%08X",
            smc_channel->id, shm_conf_channel->remote_cpu_memory_offset_type,
            shm_conf_channel->remote_cpu_memory_offset, ptr);

            if( shm_conf_channel->remote_cpu_memory_offset_type == SMC_SHM_OFFSET_ADD )
            {
                new_ptr = ptr + shm_conf_channel->remote_cpu_memory_offset;
            }
            else if( ptr - shm_conf_channel->remote_cpu_memory_offset > 0 )
            {
                new_ptr = ptr - shm_conf_channel->remote_cpu_memory_offset;
            }
            else
            {
                SMC_TRACE_PRINTF_ASSERT("smc_local_address_translate: address translation failed: Invalid configuration");
                assert(0);
            }

            SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address translation performed, new address from 0x%08X is 0x%08X",
                    smc_channel->id, new_ptr, ptr);
        }
        else
        {
            SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address translation not performed for SYNC message", smc_channel->id);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address translation not performed, offset is equal", smc_channel->id);
    }

    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: return address 0x%08X", smc_channel->id, new_ptr);
    return new_ptr;
}

static void smc_fifo_timer_expired(uint32_t data)
{
    smc_channel_t* smc_channel = (smc_channel_t*)data;
    SMC_TRACE_PRINTF_TIMER("smc_fifo_timer_expired: channel %d (0x%08X)", smc_channel->id, (uint32_t)smc_channel);

        /* Invoke the SMC FIFO release check */
    if( smc_channel != NULL )
    {
        uint8_t           forward_event = FALSE;
        SMC_CHANNEL_EVENT new_event     = SMC_SEND_FIFO_FULL;

        SMC_LOCK_IRQ( smc_channel->lock_read );

        if( smc_fifo_peek( smc_channel->fifo_out, smc_channel->smc_shm_conf_channel->use_cache_control ) > 0 )
        {
            SMC_CHANNEL_STATE_CLEAR_FIFO_FULL( smc_channel->state );
            new_event = SMC_SEND_FIFO_HAS_FREE_SPACE;
            forward_event = TRUE;
        }
        else
        {
                // Restart the timer
            if( smc_timer_start( smc_channel->fifo_timer, (void*)smc_fifo_timer_expired, (uint32_t)smc_channel ) != SMC_OK )
            {
                SMC_TRACE_PRINTF_ERROR("smc_fifo_timer_expired: FIFO timer restart failed");
            }
        }

        SMC_UNLOCK_IRQ( smc_channel->lock_read );

        if( forward_event )
        {
            smc_handle_fifo_event(smc_channel, new_event);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_fifo_timer_expired: invalid channel pointer <NULL>");
    }

    SMC_TRACE_PRINTF_TIMER("smc_fifo_timer_expired: completed");
}

static inline uint8_t smc_handle_fifo_event(smc_channel_t* channel, SMC_CHANNEL_EVENT fifo_event)
{
    uint8_t           ret_val           = SMC_OK;
    uint8_t           forward_event     = FALSE;
    uint8_t           flush_fifo_buffer = FALSE;
    SMC_CHANNEL_EVENT new_event         = fifo_event;

    SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d: FIFO event 0x%02X", channel->id, fifo_event);

    //SMC_LOCK_IRQ( channel->lock_read );
    SMC_LOCK_TX_BUFFER( channel->lock_tx_queue );

    if( fifo_event == SMC_SEND_FIFO_FULL )
    {
#ifdef SMECO_MODEM
        SMC_TRACE_PRINTF_ASSERT("Channel %d: SMC FIFO 0x%08X full: Remote host does not read messages, remote CPU probably halted",
            channel->id, (uint32_t)channel->fifo_out);
        assert(0);
#endif
        if( !SMC_CHANNEL_STATE_IS_FIFO_FULL( channel->state ) )
        {
            SMC_CHANNEL_STATE_SET_FIFO_FULL( channel->state );

            // TODO FIFO Stop state needed
            if( !SMC_CHANNEL_STATE_SEND_IS_DISABLED( channel->state ) )
            {
                new_event = SMC_STOP_SEND_LOCAL;
                forward_event = TRUE;
            }

            /* Start a timer to check when there is item available */

            if( smc_timer_start( channel->fifo_timer, (void*)smc_fifo_timer_expired, (uint32_t)channel ) != SMC_OK )
            {
                SMC_TRACE_PRINTF_ERROR("smc_handle_fifo_event: FIFO timer start failed");
            }
        }
    }
    else if( fifo_event == SMC_SEND_FIFO_HAS_FREE_SPACE )
    {
        if( SMC_CHANNEL_STATE_IS_FIFO_FULL( channel->state ) )
        {
            flush_fifo_buffer = TRUE;
        }
    }

    //SMC_UNLOCK_IRQ( channel->lock_read );

    if( forward_event )
    {
        if( channel->smc_event_cb )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d, forward the event 0x%02X", channel->id, new_event);
            channel->smc_event_cb( channel, new_event, NULL );
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_handle_fifo_event: channel %d has no event callback registered", channel->id);
            ret_val = SMC_ERROR;
        }
    }

    SMC_UNLOCK_TX_BUFFER( channel->lock_tx_queue );

    /* TODO Change the lock to the loch_state and move to the FREE space section */
    if( flush_fifo_buffer )
    {
        /* Flush the buffered items */
        ret_val = smc_channel_buffer_fifo_flush( channel );

        if( ret_val == SMC_OK )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d: FIFO buffer successfully flushed", channel->id);

            if( smc_fifo_peek( channel->fifo_out, channel->smc_shm_conf_channel->use_cache_control ) > 0 )
            {
                SMC_CHANNEL_STATE_CLEAR_FIFO_FULL( channel->state );

                new_event     = SMC_RESUME_SEND_LOCAL;
                forward_event = TRUE;
            }
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_handle_fifo_event: channel %d: FIFO buffer flush failed", channel->id);

            /* Continue the FIFO full state */
            ret_val = SMC_OK;
        }
    }


    SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d: completed by return value 0x%02X", channel->id, ret_val);

    return ret_val;
}

static inline uint8_t smc_handle_mdb_memory_event(smc_channel_t* channel, SMC_CHANNEL_EVENT mem_event, uint8_t can_flush_fifo)
{
    uint8_t           ret_val       = SMC_OK;
    uint8_t           forward_event = FALSE;
    //uint8_t           flush_fifo = FALSE;
    SMC_CHANNEL_EVENT new_event     = mem_event;

    SMC_TRACE_PRINTF_DEBUG("smc_handle_mdb_memory_event: channel %d: mem event 0x%02X", channel->id, mem_event);

    if( mem_event == SMC_SEND_MDB_OUT_OF_MEM )
    {
        if( !SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( channel->state ))
        {
            SMC_CHANNEL_STATE_SET_MDB_OUT_OF_MEM( channel->state );

            // TODO Mem out needed --> If channel not open no event needed just set the state
            if( !SMC_CHANNEL_STATE_SEND_IS_DISABLED( channel->state )  && channel->smc_event_cb)
            {
                //new_event = SMC_STOP_SEND_LOCAL;
                //forward_event = TRUE;
                channel->smc_event_cb( channel, SMC_STOP_SEND_LOCAL, NULL );
            }
        }
    }
    else if( mem_event == SMC_SEND_MDB_HAS_FREE_MEM )
    {
        if( SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( channel->state ) )
        {
            SMC_CHANNEL_STATE_CLEAR_MDB_OUT_OF_MEM( channel->state );

            //flush_fifo = can_flush_fifo;

            if( SMC_CHANNEL_STATE_SEND_IS_DISABLED( channel->state ) && channel->smc_event_cb)
            {
                //new_event = SMC_RESUME_SEND_LOCAL;
                channel->smc_event_cb( channel, SMC_RESUME_SEND_LOCAL, NULL );
                //forward_event = TRUE;
            }
        }
    }

    /*
    if( forward_event )
    {
        if( channel->smc_event_cb )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_handle_mdb_memory_event: channel %d, forward the event 0x%02X", channel->id, new_event);
            channel->smc_event_cb( channel, new_event, NULL );
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_handle_mdb_memory_event: channel %d has no event callback registered", channel->id);
            ret_val = SMC_ERROR;
        }
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_handle_mdb_memory_event: channel %d, no event forwarded", channel->id);
    }
    */

    SMC_TRACE_PRINTF_DEBUG("smc_handle_mdb_memory_event: channel %d, completed by return value 0x%02X", channel->id, ret_val);

    return ret_val;
}


/*
 * SMC basic send function.
 * All user data fields are null.
 */
uint8_t smc_send(smc_t* smc_instance, uint8_t channel_id, void* data, uint32_t data_length)
{
    smc_channel_t*  smc_channel = smc_instance->smc_channel_ptr_array[channel_id];
    smc_user_data_t userdata;

    assert(smc_instance!=NULL);

    userdata.flags     = 0x00000000;
    userdata.userdata1 = 0x00000000;
    userdata.userdata2 = 0x00000000;
    userdata.userdata3 = 0x00000000;
    userdata.userdata4 = 0x00000000;
    userdata.userdata5 = 0x00000000;

    return smc_send_ext(smc_channel, data, data_length, &userdata);
}

/* ============================================================
 * SMC extended send, the main send function.
 *
 * TODO Change the extended name.
 */
uint8_t smc_send_ext(smc_channel_t* channel, void* data, uint32_t data_length, smc_user_data_t* userdata)
{
    uint8_t return_value = SMC_ERROR;

    assert(  channel != NULL );
    assert( userdata != NULL );

    SMC_TRACE_PRINTF_DEBUG("smc_send_ext: Starting send SMC 0x%08X Channel %d (0x%08X): Data 0x%08X, length %d, flags 0x%08X", (uint32_t)channel->smc_instance, channel->id, (uint32_t)channel, (uint32_t)data, data_length, userdata->flags);

    RD_TRACE_SEND4(TRA_SMC_MESSAGE_SEND, 1, &channel->id,
                                         4, &data,
                                         4, &data_length,
                                         4, &userdata->flags);
    if( channel->fifo_out )
    {
        if( SMC_CHANNEL_STATE_IS_READY_TO_SEND( channel->state )   ||
            SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ(userdata->flags) ||
            SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP(userdata->flags)   )
        {
            void* mdb_ptr = NULL;

            if( data_length > 0 )
            {
                if( SMC_COPY_SCHEME_SEND_IS_COPY( channel->copy_scheme ) )
                {
                    if( SMC_MDB_ADDRESS_IN_POOL_OUT( data, channel->smc_mdb_info ))
                    {
                        SMC_TRACE_PRINTF_DEBUG("smc_send_ext: channel %d: MDB copy NOT required, data 0x%08X in MDB OUT area",
                                channel->id, (uint32_t)data);

                        mdb_ptr = data;
#ifdef SMC_XTILESS_CORE_DUMP_ENABLED
                        if( userdata->userdata4 == 0xF1F0F1F0 )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_send_ext: channel %d: MDB copy NOT required, data 0x%08X is XFILE DUMP",
                                    channel->id, (uint32_t)data);

                            userdata->userdata3 = 0xF1F0F1F0;
                        }
#endif
                    }
                    else
                    {
                        SMC_TRACE_PRINTF_INFO("smc_send_ext: channel %d: MDB copy required, data 0x%08X NOT in MDB OUT area",
                                channel->id, (uint32_t)data);

                        /* ========================================
                         * Critical section begins
                         *
                         */
                        SMC_LOCK_IRQ( channel->lock_mdb );

                        mdb_ptr = smc_mdb_alloc(channel, data_length);
                        
                        SMC_UNLOCK_IRQ( channel->lock_mdb );
                        /*
                         * Critical section ends
                         * ========================================
                         */

                        if (mdb_ptr != NULL)
                        {
                            assert(data != NULL );
                            /* NOTE: Calls directly memcpy, if smc_mdb_copy changed -> take it in use */
                            /*smc_mdb_copy(mdb_ptr, data, data_length);*/
                            memcpy( mdb_ptr, data, data_length );

                            if( channel->smc_shm_conf_channel->use_cache_control )
                            {
                                SMC_SHM_CACHE_CLEAN( mdb_ptr, ((void*)(((uint32_t)mdb_ptr)+data_length)) );
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_INFO("smc_send_ext: Channel %d: No cache control required", channel->id);
                            }

                            SMC_TRACE_PRINTF_DEBUG("smc_send_ext: Channel %d: MDB copy performed from address 0x%08X to 0x%08X", channel->id, (uint32_t)data, (uint32_t)mdb_ptr);
                            SMC_TRACE_PRINTF_INFO_DATA(data_length, mdb_ptr);

                            smc_free_local_ptr( channel, data, userdata );
                        }
                        else  /* if(mdb_ptr != NULL)  */
                        {
#ifdef SMC_XTILESS_CORE_DUMP_ENABLED
                            if( userdata->userdata4 == 0xF1F0F1F0 )
                            {
                                int32_t  fifo_item_count = 0;
                                uint32_t trials          = 0;

                                SMC_TRACE_PRINTF_ERROR("smc_send_ext: XTI DUMP SEND: Channel %d: MDB allocation failed, not enough SHM memory to allocate %d bytes", channel->id, data_length);

                                    /* Buffer the original pointer */
                                return_value = smc_channel_buffer_fifo_message(channel, data, data_length, userdata);

                                if( return_value != SMC_OK )
                                {
                                    SMC_TRACE_PRINTF_ERROR("smc_send_ext: XTI DUMP SEND: Channel %d MDB out of memory and unable to buffer the message", channel->id);
                                }

                                    /* Check if there is data in the FIFO (e.g. interrupts are disabled)  */
                                do
                                {
                                    volatile uint32_t delay = 0;

                                    fifo_item_count = smc_fifo_peek( channel->fifo_in, channel->smc_shm_conf_channel->use_cache_control );

                                    if( trials++ < 7 && fifo_item_count == 0 )
                                    {
                                        delay = 150000;

                                        while( delay > 0 ) delay--;
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                                while( fifo_item_count == 0);

                                if( fifo_item_count > 0 )
                                {
                                    SMC_TRACE_PRINTF_ERROR("smc_send_ext: XTI DUMP SEND: FIFO IN has %d items to read, read FIFO and continue", fifo_item_count);

                                    smc_channel_interrupt_handler( channel );

                                    SMC_LOCK_TX_BUFFER( channel->lock_tx_queue );
                                    return_value = smc_handle_mdb_memory_event(channel, SMC_SEND_MDB_HAS_FREE_MEM, TRUE);
                                    SMC_UNLOCK_TX_BUFFER( channel->lock_tx_queue );

                                    SMC_TRACE_PRINTF_ERROR("smc_send_ext: XTI DUMP SEND: FIFO IN has %d items to read, flush the FIFO...", fifo_item_count);
                                    return_value = smc_channel_buffer_fifo_flush( channel );
                                }
                                else
                                {
                                    SMC_TRACE_PRINTF_WARNING("smc_send_ext: XTI DUMP SEND: FIFO IN is empty, create out of memory event");
                                    SMC_LOCK_TX_BUFFER( channel->lock_tx_queue );
                                    return_value = smc_handle_mdb_memory_event(channel, SMC_SEND_MDB_OUT_OF_MEM, FALSE);
                                    SMC_UNLOCK_TX_BUFFER( channel->lock_tx_queue );
                                }
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_send_ext: Channel %d: MDB allocation failed, not enough SHM memory to allocate %d bytes", channel->id, data_length);
                                SMC_LOCK_TX_BUFFER( channel->lock_tx_queue );
                                return_value = smc_handle_mdb_memory_event(channel, SMC_SEND_MDB_OUT_OF_MEM, FALSE);
                                SMC_UNLOCK_TX_BUFFER( channel->lock_tx_queue );
                            }
#else
                            SMC_LOCK_TX_BUFFER( channel->lock_tx_queue );

#ifdef SMC_BUFFER_MESSAGE_OUT_OF_MDB_MEM
                            {
                                /*
                                 * Buffer the message
                                 */
                                if( !SMC_FIFO_DATA_PTR_IS_BUFFERED( userdata->flags ) )
                                {
                                    void* data_local = NULL;

                                    channel->dropped_packets_mdb_out++;

                                    if( SMC_FIFO_DATA_PTR_IS_LOCAL( userdata->flags ) )
                                    {
                                        SMC_TRACE_PRINTF_DEBUG("smc_send_ext: already local ptr 0x%08X for FIFO buffer", (uint32_t)data);
                                        data_local = data;
                                    }
                                    else
                                    {
                                        data_local = SMC_MALLOC_IRQ(data_length);

                                        SMC_TRACE_PRINTF_DEBUG("smc_send_ext: allocated 0x%08X for FIFO buffer", (uint32_t)data_local);

                                        memcpy( data_local, data, data_length );

                                        channel->fifo_buffer_copied_total += data_length;

                                        SMC_FIFO_SET_DATA_PTR_IS_LOCAL( userdata->flags );
                                    }

                                    return_value = smc_channel_buffer_fifo_message(channel, data_local, data_length, userdata);

                                    if( return_value != SMC_OK )
                                    {
                                        SMC_TRACE_PRINTF_WARNING("smc_send_ext: Channel %d: MDB allocation failed, not enough SHM memory to allocate %d bytes, unable to buffer the message", channel->id, data_length);
                                    }
                                    else
                                    {
                                        SMC_TRACE_PRINTF_WARNING("smc_send_ext: Channel %d: MDB allocation failed, not enough SHM memory to allocate %d bytes, message was buffered", channel->id, data_length);
                                    }
                                }
                                else
                                {
                                    SMC_TRACE_PRINTF_DEBUG("smc_send_ext: ptr 0x%08X already in FIFO buffer", (uint32_t)data);
                                    /* Always return error if buffered message then the message stays in buffer */
                                    return_value = SMC_ERROR;
                                }
                            }
#else
                            channel->dropped_packets_mdb_out++;
#endif

                            if( !SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( channel->state ) )
                            {
                                 smc_handle_mdb_memory_event(channel, SMC_SEND_MDB_OUT_OF_MEM, FALSE);
                            }

                            SMC_UNLOCK_TX_BUFFER( channel->lock_tx_queue );
#endif
                        }
                    }
                }
                else
                {
                    SMC_TRACE_PRINTF_INFO("smc_send_ext: Channel %d: Send copy scheme is not to copy, MDB copy not performed", channel->id);
                    mdb_ptr = data;
                }
            }
            else
            {
                SMC_TRACE_PRINTF_INFO("smc_send_ext: Channel %d: MDB copy NOT required, no data send (internal msg)", channel->id);
                mdb_ptr = data;
            }

            if( mdb_ptr || data_length==0)
            {
                smc_fifo_cell_t cell;

                    /* Check that the data pointer is not dedicated to other use */
                    /* TODO Move event data to userdata field 1 */
                if( SMC_INTERNAL_MESSAGE_DATA_ADDRESS_TRANSLATE( userdata->flags ) )
                {
                    SMC_TRACE_PRINTF_SEND_PACKET("send %d bytes of data from SHM 0x%08X", data_length, (uint32_t)mdb_ptr);
                    SMC_TRACE_PRINTF_SEND_PACKET_DATA(data_length, mdb_ptr);

                        /* SHM Address translation */
                    mdb_ptr = (void*)smc_local_address_translate( channel, (uint32_t)mdb_ptr, userdata->flags );

                    SMC_TRACE_PRINTF_DEBUG("smc_send_ext: channel %d: translated address is 0x%08X", channel->id, (uint32_t)mdb_ptr);
                }
                else
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_send_ext: channel %d: MDB address not translated, mdb ptr: 0x%08X, message flags 0x%08X",
                                                        channel->id, (uint32_t)mdb_ptr, userdata->flags);

                    RD_TRACE_SEND2(TRA_SMC_EVENT_SEND, 1, &channel->id,
                                                       4, &mdb_ptr);
                }

                    /* Send the pointer to FIFO */
                SMC_TRACE_PRINTF_INFO("smc_send_ext: SMC 0x%08X Channel %d: Put data to FIFO...", (uint32_t)channel->smc_instance, channel->id);

                cell.data        = (uint32_t)mdb_ptr;
                cell.length      = data_length;
                cell.flags       = userdata->flags;
                cell.userdata1   = userdata->userdata1;
                cell.userdata2   = userdata->userdata2;
                cell.userdata3   = userdata->userdata3;
                cell.userdata4   = userdata->userdata4;
                cell.userdata5   = userdata->userdata5;

                /* ========================================
                 * Critical section begins
                 *
                 */
                SMC_LOCK_IRQ( channel->lock_write );

                return_value = smc_fifo_put_cell( channel->fifo_out, &cell, channel->smc_shm_conf_channel->use_cache_control );

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                if( return_value == SMC_OK && SMC_TRACE_HISTORY_DATA_SEND_ENABLED(channel->trace_features) )
                {
                    if( channel->smc_history_data_sent == NULL )
                    {
                        SMC_TRACE_PRINTF_HISTORY("smc_send_ext: create history array for channel %d, size %d", channel->id, channel->smc_history_len_sent);
                        channel->smc_history_data_sent = smc_history_data_array_create( channel->smc_history_len_sent );
                    }

                    if( channel->smc_history_data_sent != NULL )
                    {
                        channel->smc_history_index_sent++;

                        if( channel->smc_history_index_sent >= channel->smc_history_len_sent )
                        {
                            channel->smc_history_index_sent = 0;
                        }

                        SMC_TRACE_PRINTF_HISTORY("smc_send_ext: channel %d: save history to index %d", channel->id, channel->smc_history_index_sent);

                        channel->smc_history_data_sent[channel->smc_history_index_sent].channel_id        = channel->id;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].history_data_type = SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_SEND;

                        channel->smc_history_data_sent[channel->smc_history_index_sent].timestamp         = SMC_TIMESTAMP_GET;


                        if( channel->smc_shm_conf_channel->remote_cpu_memory_offset_type == SMC_SHM_OFFSET_MDB_OFFSET )
                        {
                            channel->smc_history_data_sent[channel->smc_history_index_sent].shm_offset        = (int32_t)channel->smc_mdb_info->pool_out;
                        }
                        else
                        {
                            channel->smc_history_data_sent[channel->smc_history_index_sent].shm_offset        = 0;
                        }

                        channel->smc_history_data_sent[channel->smc_history_index_sent].data_ptr_shm      = cell.data;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].data_ptr          = (uint32_t)data;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].data_length       = cell.length;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].userdata_flags    = cell.flags;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].userdata1         = cell.userdata1;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].userdata2         = cell.userdata2;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].userdata3         = cell.userdata3;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].userdata4         = cell.userdata4;
                        channel->smc_history_data_sent[channel->smc_history_index_sent].userdata5         = cell.userdata5;
                    }
                    else
                    {
                        SMC_TRACE_PRINTF_HISTORY("smc_send_ext: history array not created");
                    }
                }
#endif

                SMC_UNLOCK_IRQ( channel->lock_write );
                /*
                 * Critical section ends
                 * ========================================
                 */

                if( return_value == SMC_OK )
                {
                        /* Send the signal to remote */
                    assert( channel->signal_remote != NULL );
                    smc_signal_raise( channel->signal_remote );
                }
                else if( return_value == SMC_FIFO_ERROR_FIFO_FULL )
                {
                        /* The MDB address must be translated back */
                    if( SMC_INTERNAL_MESSAGE_DATA_ADDRESS_TRANSLATE( userdata->flags ) )
                    {
                        mdb_ptr = (void*)smc_local_address_translate( channel, (uint32_t)mdb_ptr, userdata->flags );
                    }

                    return_value = smc_channel_buffer_fifo_message(channel, mdb_ptr, data_length, userdata);

                    if( return_value != SMC_OK )
                    {
                        SMC_TRACE_PRINTF_ERROR("smc_send_ext: Channel %d FIFO is full and unable to buffer the message", channel->id);
                    }
                    else
                    {
                        SMC_TRACE_PRINTF_WARNING("smc_send_ext: Channel %d FIFO is full but the message was buffered", channel->id);
                    }

                    return_value = smc_handle_fifo_event( channel, SMC_SEND_FIFO_FULL );
                }
                else
                {
                    SMC_TRACE_PRINTF_ASSERT("smc_send_ext: Channel %d unhandled FIFO ERROR 0x%02X", channel->id, return_value);
                    assert(0);
                }
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_send_ext: Channel %d: MDB ptr is NULL", channel->id);
            }
        }
        else
        {
            SMC_TRACE_PRINTF_DEBUG("smc_send_ext: Channel %d is not synchronized with remote CPU, buffering the message...", channel->id);
            return_value = smc_channel_buffer_fifo_message(channel, data, data_length, userdata);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_send_ext: FIFO OUT not initialized for channel %d", channel->id);
        return_value = SMC_ERROR;
    }

    SMC_TRACE_PRINTF_DEBUG("smc_send_ext: Completed send SMC 0x%08X Channel %d (0x%08X): Data 0x%08X, length %d, flags 0x%08X by return value %d",
            (uint32_t)channel->smc_instance, channel->id, (uint32_t)channel, (uint32_t)data, data_length, userdata->flags, return_value);


    RD_TRACE_SEND4(TRA_SMC_MESSAGE_SEND_END, 1, &channel->id,
                                             4, &data,
                                             4, &data_length,
                                             4, &userdata->flags);

    return return_value;
}

void smc_fifo_poll( const smc_channel_t* smc_channel )
{
    /* This function is only called in polling mode when XFile or core dump are transferred */

    int32_t fifo_item_count = 0;

    /* Keep polling FIFO until item appears there */
    do
    {
        volatile uint32_t delay = 150000;

        fifo_item_count = smc_fifo_peek( smc_channel->fifo_in, smc_channel->smc_shm_conf_channel->use_cache_control );

        if( fifo_item_count > 0 )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_fifo_poll: FIFO IN 0x%08X has %d items -> read FIFO", (uint32_t)smc_channel->fifo_in, fifo_item_count);

            smc_channel_interrupt_handler( (smc_channel_t*)smc_channel );
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_fifo_poll: FIFO IN is empty");
        }

        while(delay > 0 ) delay--;
    }                            
    while( fifo_item_count == 0);

}

/*
 * Function to send SMC event to remote host.
 */
uint8_t smc_send_event(smc_channel_t* channel, SMC_CHANNEL_EVENT event)
{
    smc_user_data_t userdata;

    userdata.flags     = SMC_MSG_FLAG_CHANNEL_EVENT_USER;
    userdata.userdata1 = 0x00000000;
    userdata.userdata2 = 0x00000000;
    userdata.userdata3 = 0x00000000;
    userdata.userdata4 = 0x00000000;
    userdata.userdata5 = 0x00000000;

    SMC_TRACE_PRINTF_INFO("smc_send_event: Channel %d: Sending event %d to remote host...", channel->id, event);

    return smc_send_ext( channel, (void*)event, 0, &userdata);
}

uint8_t smc_send_event_ext(smc_channel_t* channel, SMC_CHANNEL_EVENT event, smc_user_data_t* userdata)
{
    smc_user_data_t userdata_event;

    userdata_event.flags     = SMC_MSG_FLAG_CHANNEL_EVENT_USER;
    userdata_event.userdata1 = userdata->userdata1;
    userdata_event.userdata2 = userdata->userdata2;
    userdata_event.userdata3 = userdata->userdata3;
    userdata_event.userdata4 = userdata->userdata4;
    userdata_event.userdata5 = userdata->userdata5;

    SMC_TRACE_PRINTF_EVENT_SEND("smc_send_event_ext: Channel %d: Sending event %d to remote host...", channel->id, event);

    return smc_send_ext( channel, (void*)event, 0, &userdata_event);
}

/**
 * Sends indication to remote host that the channels are initialized.
 */
static uint8_t smc_send_channels_initialized_message(smc_t* smc_instance)
{
    uint8_t         return_value = SMC_ERROR;
    smc_user_data_t userdata;
    smc_channel_t*  smc_channel  = NULL;

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_send_channels_initialized_message: starting...");

#ifdef SMC_CHANNEL_ID_FOR_CONFIGURATION
    /* Use the config channel  */
    smc_channel = SMC_CHANNEL_GET(smc_instance, SMC_CHANNEL_ID_FOR_CONFIGURATION);

#else
    SMC_TRACE_PRINTF_ERROR("smc_send_channels_initialized_message: no channel selection implemented");
#endif

    if( smc_channel != NULL )
    {
        SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_send_channels_initialized_message: send message using channel %d", smc_channel->id);

        userdata.flags     = SMC_MSG_FLAG_REMOTE_CHANNELS_INIT;
        userdata.userdata1 = 0x00000000;
        userdata.userdata2 = 0x00000000;
        userdata.userdata3 = 0x00000000;
        userdata.userdata4 = 0x00000000;
        userdata.userdata5 = 0x00000000;

        if( smc_send_ext(smc_channel, NULL, 0, &userdata) != SMC_OK )
        {
            SMC_TRACE_PRINTF_WARNING("smc_send_channels_initialized_message: message send failed");
            return_value = SMC_ERROR;
        }
        else
        {
            return_value = SMC_OK;
        }
    }
    else
    {
        SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_send_channels_initialized_message: no channels available to send message");
        return_value = SMC_ERROR;
    }

    return return_value;
}

/**
 * Send ping to the remote CPU.
 */
uint8_t smc_channel_send_ping( smc_channel_t* smc_channel, uint8_t wait_reply )
{
    uint8_t         return_value   = SMC_ERROR;
    smc_user_data_t userdata;
    uint32_t*       ping_reply_var = (uint32_t*)SMC_MALLOC( sizeof( uint32_t) );

    *ping_reply_var = 0x00000000;

    userdata.flags     = SMC_MSG_FLAG_PING_REQ;
    userdata.userdata1 = 0x00000000;
    userdata.userdata2 = 0x00000000;
    userdata.userdata3 = 0x00000000;
    userdata.userdata4 = 0x00000000;
    userdata.userdata5 = (int32_t)ping_reply_var;

    RD_TRACE_SEND2(TRA_SMC_PING_SEND_REQ, 1, &smc_channel->id,
                                          4, &userdata.userdata5);

    if( smc_send_ext(smc_channel, NULL, 0, &userdata) != SMC_OK )
    {
        SMC_TRACE_PRINTF_WARNING("smc_channel_send_ping: PING send failed");
    }
    else
    {
        if( wait_reply )
        {
            uint32_t  timer_counter  = 0;

            SMC_TRACE_PRINTF_DEBUG("smc_channel_send_ping: wait PING reply...");

            while( *ping_reply_var == 0x00000000 )
            {
                SMC_SLEEP_MS(2);

                if( ++timer_counter > 0xFFFF )
                {
                    SMC_TRACE_PRINTF_WARNING("smc_channel_send_ping: no ping reply received within timeout");
                    break;
                }
            }

            if( *ping_reply_var == 0x00000000 )
            {
                SMC_TRACE_PRINTF_WARNING("smc_channel_send_ping: PING reply not received");
                return_value = SMC_ERROR;
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_send_ping: PING reply received 0x%08X==0x%08X", (uint32_t)ping_reply_var, *ping_reply_var);
                return_value = SMC_OK;
            }
        }
        else
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_send_ping: not waiting PING reply");
            return_value = SMC_OK;
        }
    }

    userdata.userdata5 = 0;

    SMC_FREE( ping_reply_var );

    ping_reply_var = NULL;

    return return_value;
}

/**
 * Sends one configuration request to remote CPU.
 */
uint8_t smc_channel_send_config( smc_channel_t* smc_channel, uint32_t configuration_id, uint32_t configuration_value, uint8_t wait_reply )
{
    uint8_t         return_value = SMC_ERROR;
    smc_user_data_t userdata;
    uint32_t*       reply_var    = (uint32_t*)SMC_MALLOC( sizeof( uint32_t) );

    *reply_var = 0x00000000;

    userdata.flags     = SMC_MSG_FLAG_CONFIG_REQ;
    userdata.userdata1 = configuration_id;
    userdata.userdata2 = configuration_value;
    userdata.userdata3 = 0x00000000;
    userdata.userdata4 = 0x00000000;
    userdata.userdata5 = (int32_t)reply_var;

    if( smc_send_ext(smc_channel, NULL, 0, &userdata) != SMC_OK )
    {
        SMC_TRACE_PRINTF_WARNING("smc_channel_send_config: Configuration request send failed");
        return_value = SMC_ERROR;
    }
    else
    {
        if( wait_reply )
        {
            /* Wait the response */

            /* TODO Get the reply value from the remote CPU for given configuration */

            uint32_t  timer_counter  = 0;

            SMC_TRACE_PRINTF_DEBUG("smc_channel_send_config: wait reply...");

            while( *reply_var == 0x00000000 )
            {
                SMC_SLEEP_MS(2);

                if( ++timer_counter > 0xFFFF )
                {
                    SMC_TRACE_PRINTF_WARNING("smc_channel_send_config: no reply received within timeout");
                    break;
                }
            }

            if( *reply_var == 0x00000000 )
            {
                SMC_TRACE_PRINTF_WARNING("smc_channel_send_config: reply not received");
                return_value = SMC_ERROR;
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_send_config: reply received");
                return_value = SMC_OK;
            }
        }
        else
        {
            return_value = SMC_OK;
        }
    }

    return return_value;
}

uint8_t smc_channel_send_fixed_config( smc_channel_t* smc_channel, smc_channel_t* smc_channel_target )
{
    uint8_t         return_value = SMC_ERROR;
    smc_user_data_t userdata;
    uint32_t        shm_offset;
    smc_channel_runtime_fixed_conf_t* configuration = NULL;

    userdata.flags     = SMC_MSG_FLAG_CONFIG_SHM_REQ;
    userdata.userdata1 = 0x00000000;
    userdata.userdata2 = 0x00000000;
    userdata.userdata3 = 0x00000000;
    userdata.userdata4 = 0x00000000;
    userdata.userdata5 = 0x00000000;

    shm_offset = ((int32_t)smc_channel_target->smc_instance->smc_shm_conf->remote_cpu_memory_offset);

    configuration = smc_channel_runtime_fixes_conf_create();

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_fixed_config: channel %d configuring channel %d, config ptr 0x%08X", smc_channel->id, smc_channel_target->id, (uint32_t)configuration);

    /* Create the configuration based on the target channel */
    configuration->channel_id        = smc_channel_target->id;

        /* NOTE that FIFO and MDB directions (in/out) must be opposite to remote host*/

    configuration->fifo_in_size      = smc_channel_target->fifo_size_out;
    configuration->fifo_out_size     = smc_channel_target->fifo_size_in;

    configuration->shm_start_address = (uint32_t)smc_channel_target->smc_shm_conf_channel->shm_area_start_address - shm_offset;
    configuration->shm_size          = smc_channel_target->smc_shm_conf_channel->size;

    configuration->mdb_in_size       = smc_channel_target->mdb_size_out;
    configuration->mdb_out_size      = smc_channel_target->mdb_size_in;


    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_config: New FIFO config: FIFO in size %d, FIFO out size %d",
            configuration->fifo_in_size, configuration->fifo_out_size);

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_config: New MDB config: MDB start 0x%08X, size %d,  MDB in size %d, MDB out size %d",
            configuration->shm_start_address, configuration->shm_size, configuration->mdb_in_size, configuration->mdb_out_size);


    if( smc_send_ext(smc_channel, (uint8_t*)configuration, sizeof(smc_channel_runtime_fixed_conf_t), &userdata) != SMC_OK )
    {
        SMC_TRACE_PRINTF_WARNING("smc_channel_send_config: Configuration request send failed");
        return_value = SMC_ERROR;
    }
    else
    {
        return_value = SMC_OK;
    }

    return return_value;
}

/**
 * Performs the Fixed configuration message.
 *
 */
static void smc_channel_execute_fixed_config( smc_channel_t* smc_channel_target, smc_channel_runtime_fixed_conf_t* configuration, smc_user_data_t* userdata_resp)
{
    uint8_t           return_value    = SMC_ERROR;
    uint32_t          configs_changed = SMC_FIXED_CONFIG_NO_CHANGES;
    uint32_t          shm_offset;
    smc_shm_config_t* smc_shm_conf    = NULL;
    uint8_t           config_changed  = FALSE;

    smc_shm_conf = smc_channel_target->smc_shm_conf_channel;
    shm_offset   = ((int32_t)smc_channel_target->smc_instance->smc_shm_conf->remote_cpu_memory_offset);

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: configuring channel %d, config ptr 0x%08X...", smc_channel_target->id, (uint32_t)configuration);

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: Old FIFO config: FIFO in size %d, FIFO out size %d",
            smc_channel_target->fifo_size_in, smc_channel_target->fifo_size_out);

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: New FIFO config: FIFO in size %d, FIFO out size %d",
            configuration->fifo_in_size, configuration->fifo_out_size);

    if( !config_changed )
    {
        config_changed = (smc_channel_target->fifo_size_in!=configuration->fifo_in_size ||
                          smc_channel_target->fifo_size_out != configuration->fifo_out_size);
    }

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: Old MDB config: MDB start 0x%08X (0x%08X), size %d,  MDB in size %d, MDB out size %d",
            (uint32_t)smc_shm_conf->shm_area_start_address-shm_offset, (uint32_t)smc_shm_conf->shm_area_start_address, smc_shm_conf->size, smc_channel_target->mdb_size_in, smc_channel_target->mdb_size_out);

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: New MDB config: MDB start 0x%08X (0x%08X), size %d,  MDB in size %d, MDB out size %d",
            configuration->shm_start_address, configuration->shm_start_address+shm_offset, configuration->shm_size, configuration->mdb_in_size, configuration->mdb_out_size);

    if( !config_changed )
    {
        config_changed = ( ((uint32_t)smc_shm_conf->shm_area_start_address)!=(configuration->shm_start_address+shm_offset) ||
                          smc_shm_conf->size!=configuration->shm_size ||
                          smc_channel_target->mdb_size_in!=configuration->mdb_in_size ||
                          smc_channel_target->mdb_size_out!=configuration->mdb_out_size);
    }

    /* Check if something changed */
    /* TODO Currently only APE sends the configuration,
     * if config is possible to both directions the logic should be implemented here
     */

    if( config_changed )
    {
        /* TODO Check if use MDB offset setup parameter (now adding offset) */
        uint8_t* config_shm_start = (uint8_t*)(configuration->shm_start_address + shm_offset);

        SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: config for the channel %d is changed, updating channel configuration...", smc_channel_target->id);

        /* Put new values and configure */

        /* TODO Check if needed to disable channels after the current shared area */

        if( smc_channel_target->fifo_size_in != configuration->fifo_in_size )
        {
            configs_changed += SMC_FIXED_CONFIG_CHANGE_FIFO_IN;
            smc_channel_target->fifo_size_in  = configuration->fifo_in_size;
        }

        if( smc_channel_target->fifo_size_out != configuration->fifo_out_size )
        {
            configs_changed += SMC_FIXED_CONFIG_CHANGE_FIFO_OUT;
            smc_channel_target->fifo_size_out = configuration->fifo_out_size;
        }

        if( smc_shm_conf->shm_area_start_address != config_shm_start )
        {
            configs_changed += SMC_FIXED_CONFIG_CHANGE_SHM_START;
            smc_shm_conf->shm_area_start_address = config_shm_start;
        }

        if( smc_shm_conf->size != configuration->shm_size )
        {
            configs_changed += SMC_FIXED_CONFIG_CHANGE_SHM_SIZE;
            smc_shm_conf->size = configuration->shm_size;
        }

        if( smc_channel_target->mdb_size_in != configuration->mdb_in_size )
        {
            configs_changed += SMC_FIXED_CONFIG_CHANGE_MDB_IN;
            smc_channel_target->mdb_size_in = configuration->mdb_in_size;
        }

        if( smc_channel_target->mdb_size_out != configuration->mdb_out_size )
        {
            configs_changed += SMC_FIXED_CONFIG_CHANGE_MDB_OUT;
            smc_channel_target->mdb_size_out = configuration->mdb_out_size;
        }

        return_value = smc_channel_configure_shm(smc_channel_target, smc_shm_conf, smc_channel_target->smc_instance->is_master);

        if( return_value == SMC_OK )
        {
            SMC_CHANNEL_STATE_CLEAR_SYNC_SENT( smc_channel_target->state );

            /*SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: SHM config for the channel %d succeeded, synchronizing channel...", smc_channel_target->id );*/

            SMC_TRACE_PRINTF_STARTUP("Channel %d configuration with remote host negotiated, local changes:%s%s%s%s%s%s", smc_channel_target->id,
                                ((configs_changed&SMC_FIXED_CONFIG_CHANGE_FIFO_IN)==SMC_FIXED_CONFIG_CHANGE_FIFO_IN)?" [FIFO in]":"",
                                ((configs_changed&SMC_FIXED_CONFIG_CHANGE_FIFO_OUT)==SMC_FIXED_CONFIG_CHANGE_FIFO_OUT)?" [FIFO out]":"",
                                ((configs_changed&SMC_FIXED_CONFIG_CHANGE_SHM_START)==SMC_FIXED_CONFIG_CHANGE_SHM_START)?" [SHM start]":"",
                                ((configs_changed&SMC_FIXED_CONFIG_CHANGE_SHM_SIZE)==SMC_FIXED_CONFIG_CHANGE_SHM_SIZE)?" [SHM size]":"",
                                ((configs_changed&SMC_FIXED_CONFIG_CHANGE_MDB_IN)==SMC_FIXED_CONFIG_CHANGE_MDB_IN)?" [MDB in]":"",
                                ((configs_changed&SMC_FIXED_CONFIG_CHANGE_MDB_OUT)==SMC_FIXED_CONFIG_CHANGE_MDB_OUT)?" [MDB out]":"");


            return_value = smc_channel_handle_sync( smc_channel_target, SMC_MSG_FLAG_SYNC_SEND_REQ, SMC_SYNC_MSG_FIFO_REQ );

            if( return_value != SMC_OK )
            {
                SMC_TRACE_PRINTF_ERROR("smc_channel_execute_fixed_config: channel %d synchronization failed", smc_channel_target->id );
            }
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_channel_execute_fixed_config: Channel %d SHM configuration failed", smc_channel_target->id );
        }

        SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: config for the channel %d is completed %s", smc_channel_target->id, (return_value==SMC_OK)?"successfully":"but it failed");
    }
    else
    {
        SMC_TRACE_PRINTF_STARTUP("Channel %d configuration negotiation request received, configuration match with remote host", smc_channel_target->id);

        SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_execute_fixed_config: config for the channel %d is not changed", smc_channel_target->id);
        return_value   = SMC_OK;
    }

        /* Set the response value */
    userdata_resp->userdata2 = smc_channel_target->id;
    userdata_resp->userdata3 = configs_changed;
    userdata_resp->userdata4 = return_value;
}

/**
 * Sends shared memory configuration.
 */
uint8_t smc_channel_send_config_shm( smc_channel_t* smc_channel, uint8_t wait_reply)
{
    uint8_t         return_value = SMC_ERROR;
    smc_user_data_t userdata;
    uint32_t*       reply_var    = (uint32_t*)SMC_MALLOC( sizeof( uint32_t) );

    *reply_var = 0x00000000;

    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_config_shm: channel %d (%s)", smc_channel->id, wait_reply?"WAIT REPLY":"NO REPLY WAIT");

    userdata.flags     = SMC_MSG_FLAG_CONFIG_SHM_REQ;
    userdata.userdata1 = smc_channel->fifo_out->length;
    userdata.userdata2 = smc_channel->fifo_in->length;
    userdata.userdata3 = (int32_t)smc_channel->smc_shm_conf_channel->shm_area_start_address - ((int32_t)smc_channel->smc_instance->smc_shm_conf->remote_cpu_memory_offset);
    userdata.userdata4 = (int32_t)smc_channel->smc_shm_conf_channel->size;
    userdata.userdata5 = (int32_t)reply_var;

    if( smc_send_ext(smc_channel, NULL, 0, &userdata) != SMC_OK )
    {
        SMC_TRACE_PRINTF_ERROR("smc_channel_send_config: Configuration request send failed");
        return_value = SMC_ERROR;
    }
    else
    {
        if( wait_reply )
        {
            /* Wait the response */
            /* TODO Get the reply value from the remote CPU for given configuration */
            uint32_t  timer_counter  = 0;

            SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_config_shm: wait reply...");

            while( *reply_var == 0x00000000 )
            {
                SMC_SLEEP_MS(2);

                if( ++timer_counter > 0xFFFF )
                {
                    SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_config_shm: no reply received within timeout");
                    break;
                }
            }

            if( *reply_var == 0x00000000 )
            {
                SMC_TRACE_PRINTF_WARNING("smc_channel_send_config_shm: reply not received");
                return_value = SMC_ERROR;
            }
            else
            {
                SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_send_config_shm: reply received");
                return_value = SMC_OK;
            }
        }
        else
        {
            return_value = SMC_OK;
        }
    }

    return return_value;
}

uint8_t smc_send_crash_info( char* crash_message )
{
    smc_lock_t* local_lock = NULL;

    SMC_TRACE_PRINTF_ERROR("smc_send_crash_info: %s", crash_message);

    /* Get SMC instances and send to all */

    local_lock = get_local_lock_smc_channel();  /* Use the same lock as the channel */
    SMC_LOCK_IRQ( local_lock );

    if( g_smc_instance_array != NULL && g_smc_instance_array_count > 0 )
    {
        for( int i = 0; i < g_smc_instance_array_count; i++ )
        {
            uint8_t channel_count = 0;

            smc_t* smc = g_smc_instance_array[i];

            SMC_TRACE_PRINTF_ERROR("smc_send_crash_info: crash message %s to SMC instance 0x%08X", crash_message, (uint32_t)smc);

                /* Send until at least one channel succeeds */
            channel_count = smc->smc_channel_list_count;

            for(int x = 0; x < channel_count; x++ )
            {
                smc_channel_t* smc_channel = smc->smc_channel_ptr_array[x];

                SMC_TRACE_PRINTF_ERROR("smc_send_crash_info: crash message %s to SMC instance 0x%08X channel id %d in index %d ", crash_message, (uint32_t)smc, smc_channel->id, x);

                if( smc_send_crash_indication( smc_channel, crash_message ) == SMC_OK )
                {
                    SMC_TRACE_PRINTF_ERROR("smc_send_crash_info: crash message %s to SMC instance 0x%08X channel id %d succeeded", crash_message, (uint32_t)smc, smc_channel->id);
                    break;
                }
                else
                {
                    SMC_TRACE_PRINTF_ERROR("smc_send_crash_info: crash message %s to SMC instance 0x%08X channel id %d FAILED", crash_message, (uint32_t)smc, smc_channel->id);
                }
            }
        }
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_send_crash_info: unable to send crash message %s, no valid SMC instances found", crash_message);
    }

    SMC_UNLOCK_IRQ( local_lock );

    return SMC_OK;
}

/**
 * Send crash indication to the remote CPU.
 */
uint8_t smc_send_crash_indication( smc_channel_t* smc_channel, char* crash_message )
{
    uint8_t         ret_val  = SMC_OK;
    smc_user_data_t userdata;

    #define CRASH_INFO_PREFIX  " crash: "

    int iCpuNameLen      = strlen(SMC_CPU_NAME);
    int iCrashPrefixLen  = strlen(CRASH_INFO_PREFIX);
    int iCrashMessageLen = iCpuNameLen + iCrashPrefixLen;
    int iIndex = 0;

    char* crash_data_message = NULL;

    if( crash_message != NULL )
    {
        iCrashMessageLen += strlen(crash_message) + 1;

        crash_data_message = (char*)SMC_MALLOC(iCrashMessageLen);

        strcpy(crash_data_message+iIndex, SMC_CPU_NAME);
        iIndex += iCpuNameLen;
        strcpy(crash_data_message+iIndex, CRASH_INFO_PREFIX);
        iIndex += iCrashPrefixLen;
        strcpy(crash_data_message+iIndex, crash_message);
        strcpy(crash_data_message+iCrashMessageLen-1, "\0");
    }
    else
    {
        iCrashMessageLen += 14;

        crash_data_message = (char*)SMC_MALLOC(iCrashMessageLen);

        strcpy(crash_data_message+iIndex, SMC_CPU_NAME);
        iIndex += iCpuNameLen;
        strcpy(crash_data_message+iIndex, CRASH_INFO_PREFIX);
        iIndex += iCrashPrefixLen;
        strcpy(crash_data_message+iIndex, "<NULL>\0");
        crash_message = crash_data_message;
    }

    SMC_TRACE_PRINTF_ERROR("smc_send_crash_indication: channel %d sending crash info '%s' to remote CPU...", smc_channel->id, crash_message);

    userdata.flags     = SMC_MSG_FLAG_REMOTE_CPU_CRASH;
    userdata.userdata1 = 0;
    userdata.userdata2 = 0;
    userdata.userdata3 = 0;
    userdata.userdata4 = 0;
    userdata.userdata5 = 0;

    if( smc_send_ext(smc_channel, (uint8_t*)crash_data_message, strlen(crash_data_message)+1, &userdata) != SMC_OK )
    {
        if( crash_data_message != NULL )
        {
            SMC_FREE(crash_data_message);
        }

        SMC_TRACE_PRINTF_ERROR("smc_send_crash_indication: Failed to send crash information");
        ret_val = SMC_ERROR;
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_send_crash_indication: crash information successfully sent");
    }

    return ret_val;
}


/*
 * Disable/Enable channel data receive mode.
 * When receive mode is disabled the incoming data is buffered to FIFO and MDB.
 * After enabling the receive mode the data is flushed from FIFO and MDB right away.
 *
 */
uint8_t smc_channel_enable_receive_mode( smc_channel_t* smc_channel, uint8_t enable_receive)
{
    uint8_t ret_val = SMC_OK;

    /* ========================================
     * Critical section begins
     *
     */

    SMC_LOCK_IRQ( smc_channel->lock_read );

    if( (enable_receive == TRUE && SMC_CHANNEL_STATE_RECEIVE_IS_DISABLED( smc_channel->state )) )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_channel_enable_receive_mode: channel %d (0x%08X) %s receive",
                            smc_channel->id, (uint32_t)smc_channel, ((enable_receive==TRUE)?"ENABLE":"DISABLE"));

        SMC_CHANNEL_STATE_CLEAR_RECEIVE_IS_DISABLED( smc_channel->state );

            /* Flush the FIFO if messages buffered */
        smc_channel_interrupt_handler(smc_channel);
    }
    else if( (enable_receive == FALSE && !SMC_CHANNEL_STATE_RECEIVE_IS_DISABLED( smc_channel->state )) )
    {

        SMC_TRACE_PRINTF_DEBUG("smc_channel_enable_receive_mode: channel %d (0x%08X) %s receive",
                    smc_channel->id, (uint32_t)smc_channel, ((enable_receive==TRUE)?"ENABLE":"DISABLE") );


        SMC_CHANNEL_STATE_SET_RECEIVE_IS_DISABLED( smc_channel->state );

        /* TODO Indicate the remote CPU */

        /* TODO Different lock object to enable the receive handling (also without IRQ) */
        /* If state changed disable -> enable call the interrupt in the loop (user might disable the receive during this) */
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_channel_enable_receive_mode: channel %d (0x%08X) is already %s",
                            smc_channel->id, (uint32_t)smc_channel, ((enable_receive==TRUE)?"ENABLED":"DISABLED"));
    }

    SMC_UNLOCK_IRQ( smc_channel->lock_read );
    /*
     * Critical section ends
     * ========================================
     */

    SMC_TRACE_PRINTF_DEBUG("smc_channel_enable_receive_mode: channel %d (0x%08X) %s %s",
                smc_channel->id, (uint32_t)smc_channel, ((enable_receive==TRUE)?"ENABLED":"DISABLED"),
                ((ret_val==SMC_OK)?"OK":"FAILED"));

    return ret_val;
}


/* =======================================================================================0
 * SMC interrupt handler to receive message from remote.
 * This is the first function receiving data from remote.
 * In this function the data is routed to user's receive callback function.
 */
void smc_channel_interrupt_handler( smc_channel_t* smc_channel )
{
    if( smc_channel != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: channel %d (0x%08X)", smc_channel->id, (uint32_t)smc_channel);

            /*
             * Check that channel has not disabled the data receiving
             * NOTE THE FLAG DOES NOT WORK HERE
             */
        if( SMC_CHANNEL_STATE_RECEIVE_IS_DISABLED( smc_channel->state ) )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: channel %d (0x%08X), receive is disabled", smc_channel->id, (uint32_t)smc_channel);
            return;
        }

        if( smc_channel->fifo_in != NULL )
        {
            int32_t fifo_count = SMC_FIFO_READ_TO_EMPTY;

                /* Read the data from FIFO -> Read until empty */
            do
            {
                smc_fifo_cell_t celldata;
                smc_user_data_t userdata;

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                uint32_t*  data_ptr_history = NULL;
#endif

                /* ========================================
                 * Critical section begins
                 *
                 */
                SMC_LOCK_IRQ( smc_channel->lock_read );

                fifo_count = smc_fifo_get_cell( smc_channel->fifo_in, &celldata, smc_channel->smc_shm_conf_channel->use_cache_control );

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                if( SMC_TRACE_HISTORY_DATA_RECEIVE_ENABLED(smc_channel->trace_features) )
                {
                    if( smc_channel->smc_history_data_received == NULL )
                    {
                        SMC_TRACE_PRINTF_HISTORY("smc_channel_interrupt_handler: create receive history array for channel %d, size %d", smc_channel->id, smc_channel->smc_history_len_received);
                        smc_channel->smc_history_data_received = smc_history_data_array_create( smc_channel->smc_history_len_received );
                    }

                    if( smc_channel->smc_history_data_received != NULL )
                    {
                        smc_channel->smc_history_index_received++;

                        if( smc_channel->smc_history_index_received >= smc_channel->smc_history_len_received )
                        {
                            smc_channel->smc_history_index_received = 0;
                        }

                        SMC_TRACE_PRINTF_HISTORY("smc_channel_interrupt_handler: channel %d: save receive history to index %d", smc_channel->id, smc_channel->smc_history_index_received);

                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].channel_id        = smc_channel->id;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].history_data_type = SMC_TRACE_HISTORY_DATA_TYPE_MESSAGE_RECEIVE;

                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].timestamp         = SMC_TIMESTAMP_GET;


                        if( smc_channel->smc_shm_conf_channel->remote_cpu_memory_offset_type == SMC_SHM_OFFSET_MDB_OFFSET )
                        {
                            smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].shm_offset = (int32_t)smc_channel->smc_mdb_info->pool_in;
                        }
                        else
                        {
                            smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].shm_offset = 0;
                        }

                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].data_ptr_shm   = celldata.data;
                        //smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].data_ptr       = (uint32_t)data;

                        data_ptr_history = &smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].data_ptr;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].data_length    = celldata.length;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].userdata_flags = celldata.flags;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].userdata1      = celldata.userdata1;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].userdata2      = celldata.userdata2;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].userdata3      = celldata.userdata3;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].userdata4      = celldata.userdata4;
                        smc_channel->smc_history_data_received[smc_channel->smc_history_index_received].userdata5      = celldata.userdata5;

                        SMC_TRACE_PRINTF_HISTORY("smc_channel_interrupt_handler: channel %d: receive history saved to index %d", smc_channel->id, smc_channel->smc_history_index_received);
                    }
                    else
                    {
                        SMC_TRACE_PRINTF_HISTORY("smc_channel_interrupt_handler: history array not created");
                    }
                }
#endif

                SMC_UNLOCK_IRQ( smc_channel->lock_read );
                /*
                 * Critical section ends
                 * ========================================
                 */

                userdata.flags     = celldata.flags;
                userdata.userdata1 = celldata.userdata1;
                userdata.userdata2 = celldata.userdata2;
                userdata.userdata3 = celldata.userdata3;
                userdata.userdata4 = celldata.userdata4;
                userdata.userdata5 = celldata.userdata5;

                if( fifo_count != SMC_FIFO_EMPTY )
                {
                    //uint8_t is_loopback_msg = SMC_IS_LOOPBACK_MSG(celldata.flags);

                    SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: channel %d (0x%08X) Read data 0x%08X, length %d, flags 0x%08X, FIFO count %d",
                            smc_channel->id, (uint32_t)smc_channel, celldata.data, celldata.length, celldata.flags, fifo_count);

                    RD_TRACE_SEND4(TRA_SMC_MESSAGE_RECV, 1, &smc_channel->id,
                                                         4, &celldata.data,
                                                         4, &celldata.length,
                                                         4, &celldata.flags);

                        /* Check if internal message */
                    if( SMC_FIFO_IS_INTERNAL_MESSAGE( celldata.flags ) && !SMC_IS_LOOPBACK_MSG(celldata.flags) )
                    {
                        if( SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ");
#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                            *data_ptr_history = (uint32_t)celldata.data;
#endif
                            smc_channel_handle_sync( smc_channel, SMC_MSG_FLAG_SYNC_INFO_REQ, celldata.data );
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP");
#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                            *data_ptr_history = (uint32_t)celldata.data;
#endif
                            smc_channel_handle_sync( smc_channel, SMC_MSG_FLAG_SYNC_INFO_RESP, celldata.data );
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB( celldata.flags ) )
                        {
                            void* data = NULL;

                            SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB");

                                /* Do the address translation because the remote send the data ptr to free as it gets it */
                            data = (void*)smc_local_address_translate( smc_channel, celldata.data, celldata.flags );

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                            *data_ptr_history = (uint32_t)data;
#endif

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler(ch %d): SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB 0x%08X (from 0x%08X)",
                            smc_channel->id, (uint32_t)data, (uint32_t)celldata.data);

                            smc_free_local_ptr( smc_channel, data, &userdata );

                            SMC_LOCK_TX_BUFFER( smc_channel->lock_tx_queue );
                            //SMC_LOCK( smc_channel->lock_tx_queue );
                            if( SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( smc_channel->state ) )
                            {
                                smc_handle_mdb_memory_event(smc_channel, SMC_SEND_MDB_HAS_FREE_MEM, TRUE);

                                SMC_UNLOCK_TX_BUFFER( smc_channel->lock_tx_queue );
                                //SMC_UNLOCK( smc_channel->lock_tx_queue );
#ifdef SMC_BUFFER_MESSAGE_OUT_OF_MDB_MEM
                                //if(flush_fifo)
                                {
                                    SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: Flush FIFO buffer");

                                    if( smc_channel_buffer_fifo_flush( smc_channel ) == SMC_OK )
                                    {
                                        SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: FIFO buffer flushed");
                                    }
                                    else
                                    {
                                        SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: FIFO buffer flush failed");
                                    }
                                }
#endif
                            }
                            else
                            {
                                SMC_UNLOCK_TX_BUFFER( smc_channel->lock_tx_queue );
                                //SMC_UNLOCK( smc_channel->lock_tx_queue );
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_CHANNEL_EVENT_USER( celldata.flags ) )
                        {
                            RD_TRACE_SEND2(TRA_SMC_EVENT_RECV, 1, &smc_channel->id,
                                                               4, &celldata.data);

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CHANNEL_EVENT_USER: 0x%08X", (uint32_t)celldata.data);

                            if( smc_channel->smc_event_cb )
                            {
                                SMC_LOCK_TX_BUFFER( smc_channel->lock_tx_queue );
                                smc_channel->smc_event_cb( smc_channel, (SMC_CHANNEL_EVENT)celldata.data, NULL );
                                SMC_UNLOCK_TX_BUFFER( smc_channel->lock_tx_queue );
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: No callback registered for channel %d to receive events", smc_channel->id);
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_REQ( celldata.flags ) )
                        {
                            smc_user_data_t userdata_version;

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_REQ");

                            userdata_version.flags     = SMC_MSG_FLAG_VERSION_INFO_RESP;
                            userdata_version.userdata1 = smc_version_to_int(smc_get_version());
                            userdata_version.userdata2 = 0;
                            userdata_version.userdata3 = 0;
                            userdata_version.userdata4 = 0;
                            userdata_version.userdata5 = 0;

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_REQ, send version 0x%08X", userdata_version.userdata1);

                            if( smc_send_ext( smc_channel, NULL, 0, &userdata_version) != SMC_OK )
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_REQ send failed");
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_RESP( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_RESP");

                            if( smc_channel->smc_event_cb )
                            {
                                    /* Read the version from user data field 1 */
                                uint32_t* version_info = (uint32_t*)SMC_MALLOC_IRQ( sizeof( uint32_t ) );

                                *version_info = celldata.userdata1;

                                SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_VERSION_RESP, received version 0x%08X", *version_info);

                                smc_channel->version_remote = celldata.userdata1;

                                #ifdef SMC_PERFORM_VERSION_CHECK
                                    if( smc_version_check( smc_version_to_int(smc_get_version()), smc_channel->version_remote, smc_channel->smc_instance->is_master ) != SMC_OK )
                                    {
                                        SMC_TRACE_PRINTF_WARNING("Local SMC version is not same as remote version");
                                    }
                                #endif /* #ifdef SMC_PERFORM_VERSION_CHECK */

                                smc_channel->smc_event_cb( smc_channel, SMC_VERSION_INFO_REMOTE, (void*)version_info );

                                if( version_info != NULL )
                                {
                                    SMC_FREE( version_info );
                                    version_info = NULL;
                                }
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: No callback registered for channel %d to receive events", smc_channel->id);
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_PING_REQ( celldata.flags ) )
                        {
                            smc_user_data_t userdata_ping;

                            RD_TRACE_SEND2(TRA_SMC_PING_RECV_REQ, 1, &smc_channel->id,
                                                                  4, &celldata.userdata5);

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_PING_REQ, reply_var=0x%08X", celldata.userdata5);

                            userdata_ping.flags     = SMC_MSG_FLAG_PING_RESP;
                            userdata_ping.userdata1 = 0;
                            userdata_ping.userdata2 = 0;
                            userdata_ping.userdata3 = 0;
                            userdata_ping.userdata4 = 0;
                            userdata_ping.userdata5 = celldata.userdata5;

                            RD_TRACE_SEND2(TRA_SMC_PING_SEND_RESP, 1, &smc_channel->id,
                                                                   4, &userdata_ping.userdata5);

                            if( smc_send_ext( smc_channel, NULL, 0, &userdata_ping) != SMC_OK )
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: SMC_MSG_FLAG_PING_RESP send failed");
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_PING_RESP( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_PING_RESP, reply_var = 0x%08X", celldata.userdata5);

                            RD_TRACE_SEND2(TRA_SMC_PING_RECV_RESP, 1, &smc_channel->id,
                                                                   4, &celldata.userdata5);

                            if( celldata.userdata5 != 0 )
                            {
                                uint32_t* reply_var = (uint32_t*)celldata.userdata5;
                                *reply_var = SMC_MESSAGE_REPLY_FLAG_VALUE;
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_PING_RESP: No reply variable defined");
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_REQ( celldata.flags ) )
                        {
                            smc_user_data_t userdata_resp;
                            uint32_t        configuration_id    = celldata.userdata1;
                            uint32_t        configuration_value = celldata.userdata2;

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_REQ");

                            userdata_resp.flags     = SMC_MSG_FLAG_CONFIG_RESP;
                            userdata_resp.userdata1 = configuration_id;
                            userdata_resp.userdata2 = configuration_value;
                            userdata_resp.userdata3 = (uint32_t)smc_conf_request_received( smc_channel, configuration_id, configuration_value);

                            userdata_resp.userdata4 = 0x00000000;
                            userdata_resp.userdata5 = celldata.userdata5;

                            RD_TRACE_SEND3(TRA_SMC_CONFIG_REQ_RECV, 1, &smc_channel->id,
                                                                    4, &configuration_id,
                                                                    4, &configuration_value);

                            if( smc_send_ext( smc_channel, NULL, 0, &userdata_resp) != SMC_OK )
                            {
                                SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: SMC_MSG_FLAG_CONFIG_RESP send failed");
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_RESP( celldata.flags ) )
                        {
                            uint32_t        configuration_id               = celldata.userdata1;
                            uint32_t        configuration_value_requested  = celldata.userdata2;
                            uint32_t        configuration_resp             = celldata.userdata3;

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_RESP");

                            RD_TRACE_SEND3(TRA_SMC_CONFIG_RESP_RECV, 1, &smc_channel->id,
                                                                     4, &configuration_id,
                                                                     4, &configuration_resp);

                            smc_conf_response_received( smc_channel, configuration_id, configuration_value_requested, configuration_resp );

                            if( celldata.userdata5 != 0 )
                            {
                                uint32_t* reply_var = (uint32_t*)celldata.userdata5;
                                *reply_var = SMC_MESSAGE_REPLY_FLAG_VALUE; /**/
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ( celldata.flags ) )
                        {
                            smc_user_data_t userdata_resp;

                            SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ: channel %d configuring other channel...", smc_channel->id);

                            userdata_resp.userdata1 = 0x00000000;
                            userdata_resp.userdata2 = 0x00000000;
                            userdata_resp.userdata3 = 0x00000000;   /* Changed config status */
                            userdata_resp.userdata4 = 0x00000000;   /* OK / FAIL */

                            if( celldata.length > 0 && celldata.data != 0)
                            {
                                /* TODO Put the configuration into the response */
                                void*                             data               = (void*)smc_local_address_translate( smc_channel, celldata.data, celldata.flags );
                                smc_channel_runtime_fixed_conf_t* configuration      = NULL;
                                smc_channel_t*                    smc_channel_target = NULL;
                                smc_user_data_t                   userdata_free;

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                                *data_ptr_history = (uint32_t)data;
#endif

                                if( smc_channel->smc_shm_conf_channel->use_cache_control )
                                {
                                    SMC_SHM_CACHE_INVALIDATE( data, ((void*)(((uint32_t)data) + celldata.length)) );
                                }

                                configuration      = (smc_channel_runtime_fixed_conf_t*)data;
                                smc_channel_target = SMC_CHANNEL_GET( smc_channel->smc_instance, configuration->channel_id );

                                SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ: channel %d configuring other channel: Configuration data found, target channel is %d", smc_channel->id, smc_channel_target->id);

                                smc_channel_execute_fixed_config(smc_channel_target, configuration, &userdata_resp);

                                    /* Free the MDB SHM data PTR from remote */
                                userdata_free.flags     = SMC_MSG_FLAG_FREE_MEM_MDB;
                                userdata_free.userdata1 = userdata.userdata1;
                                userdata_free.userdata2 = userdata.userdata2;
                                userdata_free.userdata3 = userdata.userdata3;
                                userdata_free.userdata4 = userdata.userdata4;
                                userdata_free.userdata5 = userdata.userdata5;

                                if( smc_send_ext(smc_channel, data, 0, &userdata_free) != SMC_OK )
                                {
                                    SMC_TRACE_PRINTF_ERROR("smc_channel_interrupt_handler: channel %d: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ, MDB memory 0x%08X free from remote failed", smc_channel->id, (uint32_t)data);
                                }
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_REQ: channel %d configuring other channel: Invalid message data len 0", smc_channel->id);

                                userdata_resp.userdata4 = SMC_ERROR;
                            }

                            userdata_resp.flags     = SMC_MSG_FLAG_CONFIG_SHM_RESP;
                            userdata_resp.userdata5 = celldata.userdata5;

                            if( smc_send_ext( smc_channel, NULL, 0, &userdata_resp) != SMC_OK )
                            {
                                SMC_TRACE_PRINTF_ERROR("smc_channel_interrupt_handler: SMC_MSG_FLAG_CONFIG_SHM_RESP send failed");
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_RESP( celldata.flags ) )
                        {
                            smc_channel_t* smc_channel_target = NULL;
                            SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CONFIG_SHM_RESP for channel %d configuring other channel", smc_channel->id);

                            smc_channel_target = SMC_CHANNEL_GET( smc_channel->smc_instance, celldata.userdata2 );
                            smc_channel_fixed_config_response( smc_channel, smc_channel_target, &userdata );

                            if( celldata.userdata5 != 0 )
                            {
                                uint32_t* reply_var = (uint32_t*)celldata.userdata5;
                                *reply_var = SMC_MESSAGE_REPLY_FLAG_VALUE;
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CHANNELS_INIT( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CHANNELS_INIT");

                            SMC_INSTANCE_STATUS_INIT_SET_REMOTE( smc_channel->smc_instance->init_status );

                            if( SMC_INSTANCE_CAN_SEND_FIXED_CONFIG( smc_channel->smc_instance->init_status ) )
                            {
                                SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: channel %d: SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CHANNELS_INIT, send negotiable configurations...", smc_channel->id);

                                    /* Send negotiable parameters using all channels */
                                if( smc_send_negotiable_configurations( smc_channel->smc_instance ) != SMC_OK )
                                {
                                    SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: channel %d: smc_send_negotiable_configurations failed", smc_channel->id);
                                }
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CHANNELS_INIT, unable to send negotiable configurations yet");
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_TRACE_ACTIVATE_REQ( celldata.flags ) )
                        {

                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CPU_CRASH( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_CRASH_INFO("************************************************************");
                            /*SMC_TRACE_PRINTF_CRASH_INFO("*** Remote CPU crashed:");*/

                            if( celldata.length > 0 && celldata.data != 0)
                            {
                                void* data = (void*)smc_local_address_translate( smc_channel, celldata.data, celldata.flags );

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                                *data_ptr_history = (uint32_t)data;
#endif
                                if( smc_channel->smc_shm_conf_channel->use_cache_control )
                                {
                                    SMC_SHM_CACHE_INVALIDATE( data, ((void*)(((uint32_t)data) + celldata.length)) );
                                }

                                    /* TODO Add different errors with assert */
                                char* crash_data = (char*)data;

                                SMC_TRACE_PRINTF_CRASH_INFO("*** %s", crash_data);
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_ERROR("Remote CPU crashed, no crash info available");
                            }

                            SMC_TRACE_PRINTF_CRASH_INFO("************************************************************");
                        }
                        else
                        {
                            SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: channel %d: unsupported internal msg flag used 0x%08X", smc_channel->id, celldata.flags);
                        }
                    }
                    else
                    {
                            /* Not an internal message */
                        void* received_data_ptr = NULL;
                        void* data              = (void*)smc_local_address_translate( smc_channel, celldata.data, celldata.flags );

                        SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: Translated address is 0x%08X (%s)", data, SMC_IS_LOOPBACK_MSG(celldata.flags)?"LB":"Not LB");
                        SMC_TRACE_PRINTF_INFO_DATA( celldata.length, data );

                        assert( smc_channel->smc_receive_cb != NULL );

                        if( celldata.length > 0 )
                        {
                            if( SMC_COPY_SCHEME_RECEIVE_IS_COPY( smc_channel->copy_scheme ) )
                            {
                                SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: channel %d: copy scheme detected, check address 0x%08X", smc_channel->id, (uint32_t)data);

                                if( SMC_MDB_ADDRESS_IN_POOL_IN( data, smc_channel->smc_mdb_info ))
                                {
                                    smc_user_data_t userdata_free;

                                    userdata_free.flags     = SMC_MSG_FLAG_FREE_MEM_MDB;
                                    userdata_free.userdata1 = userdata.userdata1;
                                    userdata_free.userdata2 = userdata.userdata2;
                                    userdata_free.userdata3 = userdata.userdata3;
                                    userdata_free.userdata4 = userdata.userdata4;
                                    userdata_free.userdata5 = userdata.userdata5;

                                    SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: channel %d: MDB copy required, data in MDB IN area", smc_channel->id);

                                    /* ========================================
                                     * Critical section begins
                                     *
                                     */
                                    SMC_LOCK_IRQ( smc_channel->lock_read );

                                        /* Use the allocator to get memory for the data */
                                    received_data_ptr = smc_allocate_local_ptr( smc_channel, celldata.length, &userdata );

                                    /* TODO If no memory --> Disable the receiving
                                     * TODO Check if possible to put item back in the FIFO/MDB to be reused
                                     * */

                                    assert( received_data_ptr != NULL );

                                    if( smc_channel->smc_shm_conf_channel->use_cache_control )
                                    {
                                        SMC_SHM_CACHE_INVALIDATE( data, ((void*)(((uint32_t)data) + celldata.length)) );
                                    }
                                    else
                                    {
                                        SMC_TRACE_PRINTF_RECEIVE("smc_channel_interrupt_handler: channel %d: No cache control required", smc_channel->id);
                                    }

                                    SMC_TRACE_PRINTF_RECEIVE_PACKET("received %d bytes of data to SHM 0x%08X", celldata.length, (uint32_t)data);
                                    SMC_TRACE_PRINTF_RECEIVE_PACKET_DATA(celldata.length, data);

                                    /* NOTE: Calls directly memcpy, if smc_mdb_copy changed -> take it in use */
                                    /*smc_mdb_copy( received_data_ptr, data, celldata.length );*/
                                    memcpy( received_data_ptr, data, celldata.length );

                                    if( smc_channel->smc_shm_conf_channel->use_cache_control )
                                    {
                                            /* Clean the new data from cache */
                                        SMC_SHM_CACHE_CLEAN( received_data_ptr, ((void*)(((uint32_t)received_data_ptr) + celldata.length)) );
                                    }

                                    SMC_UNLOCK_IRQ( smc_channel->lock_read );
                                    /*
                                     * Critical section ends
                                     * ========================================
                                     */

                                    SMC_TRACE_PRINTF_RECEIVE("smc_channel_interrupt_handler: channel %d: MDB copy performed from 0x%08X to 0x%08X", smc_channel->id, (uint32_t)data, (uint32_t)received_data_ptr);
                                    SMC_TRACE_PRINTF_RECEIVE_DATA(celldata.length, received_data_ptr);

                                        /* Free the MDB SHM data PTR from remote */
                                    if( smc_send_ext(smc_channel, data, 0, &userdata_free) != SMC_OK )
                                    {
                                        SMC_TRACE_PRINTF_ERROR("smc_channel_interrupt_handler: channel %d: MDB memory 0x%08X free from remote failed", smc_channel->id, (uint32_t)data);
                                    }
                                }
                                else
                                {
                                    SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: channel %d: MDB copy not performed, data 0x%08X not in MDB IN area", smc_channel->id, (uint32_t)data);
                                    received_data_ptr = data;
                                }
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: channel %d (0x%08X) copy scheme is not copy in receive", smc_channel->id, (uint32_t)smc_channel);

                                SMC_TRACE_PRINTF_RECEIVE_PACKET("received %d bytes of data to SHM 0x%08X (0-copy)", celldata.length, (uint32_t)data);
                                SMC_TRACE_PRINTF_RECEIVE_PACKET_DATA(celldata.length, data);

                                received_data_ptr = data;
                            }
                        }
                        else
                        {
                            received_data_ptr = data;
                        }

                        RD_TRACE_SEND5(TRA_SMC_MESSAGE_RECV_TO_CB, 1, &smc_channel->id,
                                                                   4, &celldata.data,
                                                                   4, &received_data_ptr,
                                                                   4, &celldata.length,
                                                                   4, &celldata.flags);
#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
                        *data_ptr_history = (uint32_t)received_data_ptr;
#endif

                        if( !SMC_IS_LOOPBACK_MSG(celldata.flags) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: Deliver data to SMC channel callback...");

                            smc_channel->smc_receive_cb( received_data_ptr, celldata.length, &userdata, smc_channel);
                        }
                        else
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: Deliver data to SMC loopback data handler...");

                            if( smc_handle_loopback_data_message(smc_channel, (smc_loopback_data_t*)received_data_ptr, &userdata ) != SMC_OK )
                            {
                                SMC_TRACE_PRINTF_ERROR("smc_channel_interrupt_handler: smc_handle_loopback_data_message failed");
                            }
                        }

                        RD_TRACE_SEND4(TRA_SMC_MESSAGE_RECV, 1, &smc_channel->id,
                                                             4, &celldata.data,
                                                             4, &celldata.length,
                                                             4, &celldata.flags);
                    }
                }
                else
                {
                    SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: channel %d (0x%08X) FIFO 0x%08X is empty", smc_channel->id, (uint32_t)smc_channel, (uint32_t)smc_channel->fifo_in);
                }
            } while( fifo_count > SMC_FIFO_READ_TO_EMPTY );
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_channel_interrupt_handler: channel %d (0x%08X) FIFO IN is not configured (NULL)", smc_channel->id, (uint32_t)smc_channel);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_channel_interrupt_handler: receiver channel is NULL, interrupt is not handled further");
    }

    SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: channel (0x%08X) completed", (uint32_t)smc_channel);
}

/*
 * Creates SMC instance based on given SMC configuration.
 *
 * NOTE: Every item value in configuration structure (except function ptrs)
 *       must be copied to SMC instance because the configuration
 *       might be destroyed after the configuration.
 */
smc_t* smc_instance_create(smc_conf_t* smc_instance_conf)
{
    return smc_instance_create_ext(smc_instance_conf, NULL);
}

smc_t* smc_instance_create_ext(smc_conf_t* smc_instance_conf, void* parent_object)
{
    smc_t* smc = (smc_t*)SMC_MALLOC( sizeof(smc_t) );

    assert( smc != NULL );
    assert( smc_instance_conf != NULL );

    SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: created SMC instance 0x%08X based on configuration in 0x%08X",
            (uint32_t)smc, (uint32_t)smc_instance_conf);

    smc->cpu_id_remote          = smc_instance_conf->cpu_id_remote;
    smc->cpu_id_local           = smc_instance_conf->cpu_id_local;
    smc->is_master              = smc_instance_conf->is_master;
    smc->smc_channel_list_count = 0;
    smc->smc_channel_ptr_array  = NULL;
    smc->smc_instance_conf      = smc_instance_conf;
    smc->smc_parent_ptr         = parent_object;
    smc->init_status            = SMC_INSTANCE_STATUS_INIT_NONE;

    smc_instance_add( smc );

    if( smc_instance_conf->smc_shm_conf != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: SHM configuration found, copying to instance...");
        smc->smc_shm_conf = smc_shm_config_copy( smc_instance_conf->smc_shm_conf );

        if( smc->smc_shm_conf != NULL )
        {
            if( smc->smc_shm_conf->shm_area_start_address != NULL )
            {
                uint32_t physical_addr = (uint32_t)smc->smc_shm_conf->shm_area_start_address;

                smc->smc_shm_conf->shm_area_start_address = SMC_SHM_IOREMAP( smc->smc_shm_conf->shm_area_start_address, smc->smc_shm_conf->size );

                if( ((uint32_t)smc->smc_shm_conf->shm_area_start_address - physical_addr) != 0 )
                {
                        /* Store the offset for debugging purposes only if the shm is in virtual memory */
                    smc->smc_shm_conf->remote_cpu_memory_offset = ((uint32_t)smc->smc_shm_conf->shm_area_start_address - physical_addr);
                }

                SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: Remapped SHM start address from 0x%08X is 0x%08X, size is %d, offset to physical is %d (0x%08X)",
                        (uint32_t)smc_instance_conf->smc_shm_conf->shm_area_start_address , (uint32_t)smc->smc_shm_conf->shm_area_start_address,
                        smc->smc_shm_conf->size, (int32_t)smc->smc_shm_conf->remote_cpu_memory_offset, (uint32_t)smc->smc_shm_conf->remote_cpu_memory_offset);

                SMC_TRACE_PRINTF_STARTUP("Shared memory size %d bytes, memory area: 0x%08X - 0x%08X (physical: 0x%08X - 0x%08X)", smc->smc_shm_conf->size,
                                        (uint32_t)smc->smc_shm_conf->shm_area_start_address, ((uint32_t)smc->smc_shm_conf->shm_area_start_address + smc->smc_shm_conf->size),
                                        ((uint32_t)smc->smc_shm_conf->shm_area_start_address-smc->smc_shm_conf->remote_cpu_memory_offset),
                                        (((uint32_t)smc->smc_shm_conf->shm_area_start_address + smc->smc_shm_conf->size)-smc->smc_shm_conf->remote_cpu_memory_offset) );
            }
            else
            {
                SMC_TRACE_PRINTF_WARNING("smc_instance_create_ext: SHM start address is NULL, SHM not available");
            }

            SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: SHM configuration copied, modify first free address to 0x%08X...", (uint32_t)smc->smc_shm_conf->shm_area_start_address);
            smc->smc_memory_first_free = smc->smc_shm_conf->shm_area_start_address;
        }
        else
        {
            SMC_TRACE_PRINTF_WARNING("smc_instance_create_ext: SHM configuration copy failed");
            smc->smc_memory_first_free = NULL;
        }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_instance_create_ext: SHM configuration not set");
        smc->smc_shm_conf = NULL;
        smc->smc_memory_first_free = NULL;
    }

        /* Create the SMC channels */
    if( smc_instance_conf->smc_channel_conf_count > 0 )
    {
        int i = 0;

        SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: found %d channel configurations, initializing SMC channels...", smc_instance_conf->smc_channel_conf_count);

        for(i = 0; i < smc_instance_conf->smc_channel_conf_count; i++ )
        {
            smc_channel_t* channel = smc_channel_create( smc_instance_conf->smc_channel_conf_ptr_array[i] );

            SMC_TRACE_PRINTF_INFO("smc_instance_create_ext: new SMC channel in index %d created (0x%08X), add to channel array...",
                        i, (uint32_t)channel);

            smc_add_channel(smc, channel, smc_instance_conf->smc_channel_conf_ptr_array[i]);
        }

        SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: channels successfully configured");

        SMC_INSTANCE_STATUS_INIT_SET_LOCAL( smc->init_status );

            /* Send message to remote that channels are initialized */
        if( smc_send_channels_initialized_message( smc ) != SMC_OK )
        {
            SMC_TRACE_PRINTF_WARNING("smc_instance_create_ext: smc_send_channels_initialized_message failed");
        }

        if( SMC_INSTANCE_CAN_SEND_FIXED_CONFIG( smc->init_status ) )
        {
            SMC_TRACE_PRINTF_RUNTIME_CONF_SHM("smc_instance_create_ext: SMC_FIFO_IS_INTERNAL_MESSAGE_REMOTE_CHANNELS_INIT, send negotiable configurations...");

                /* Send negotiable parameters using all channels */
            if( smc_send_negotiable_configurations( smc ) != SMC_OK )
            {
                SMC_TRACE_PRINTF_WARNING("smc_instance_create_ext: smc_send_negotiable_configurations failed");
            }
        }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_instance_create_ext: no channels found from configuration");
    }

    SMC_TRACE_PRINTF_DEBUG("smc_instance_create_ext: new SMC instance 0x%08X created", (uint32_t)smc);

#ifdef SMC_TRACE_INFO_ENABLED

    smc_instance_dump(smc);

#endif

    return smc;
}

void smc_instance_destroy( smc_t* smc_instance )
{
    if( smc_instance )
    {
        SMC_TRACE_PRINTF_STARTUP("Closing SMC instance with %d channels...", smc_instance->smc_channel_list_count);

        /*SMC_TRACE_PRINTF_DEBUG("smc_instance_destroy: destroying channels (%d)...", smc_instance->smc_channel_list_count);*/

        for(int i = 0; i < smc_instance->smc_channel_list_count; i++)
        {
            smc_channel_destroy( smc_instance->smc_channel_ptr_array[i] );
        }

        SMC_FREE( smc_instance->smc_channel_ptr_array );
        smc_instance->smc_channel_ptr_array = NULL;

        if( smc_instance->smc_shm_conf != NULL )
        {
            SMC_TRACE_PRINTF_STARTUP("Deallocating Shared memory area: size %d bytes, memory area: 0x%08X - 0x%08X (physical: 0x%08X - 0x%08X)", smc_instance->smc_shm_conf->size,
                                                    (uint32_t)smc_instance->smc_shm_conf->shm_area_start_address, ((uint32_t)smc_instance->smc_shm_conf->shm_area_start_address + smc_instance->smc_shm_conf->size),
                                                    ((uint32_t)smc_instance->smc_shm_conf->shm_area_start_address-smc_instance->smc_shm_conf->remote_cpu_memory_offset),
                                                    (((uint32_t)smc_instance->smc_shm_conf->shm_area_start_address + smc_instance->smc_shm_conf->size)-smc_instance->smc_shm_conf->remote_cpu_memory_offset) );

            SMC_SHM_IOUNMAP( smc_instance->smc_shm_conf->shm_area_start_address );

            SMC_FREE( smc_instance->smc_shm_conf );
            smc_instance->smc_shm_conf = NULL;
        }
        else
        {
            SMC_TRACE_PRINTF_STARTUP("Shared memory area was not configured");
        }

        smc_instance_remove( smc_instance );

        SMC_FREE( smc_instance );
        smc_instance = NULL;

        SMC_TRACE_PRINTF_STARTUP("SMC instance closed");
    }
}

/*
 * SMC Instance restart.
 * Returns pointer to new instance.
 */

smc_t* smc_instance_restart( smc_t* smc_instance )
{
    SMC_TRACE_PRINTF_DEBUG("smc_instance_restart: restarting SMC instance 0x%08X...", (uint32_t)smc_instance);

    if( smc_instance != NULL )
    {
        smc_t*       smc_instance_new  = NULL;
        smc_conf_t*  smc_instance_conf = smc_instance->smc_instance_conf;
        void*        smc_parent_ptr    = smc_instance->smc_parent_ptr;

        SMC_TRACE_PRINTF_DEBUG("smc_instance_restart: destroying instance 0x%08X...", (uint32_t)smc_instance);
        smc_instance_destroy( smc_instance );

        smc_instance_new = smc_instance_create_ext( smc_instance_conf, smc_parent_ptr );

        SMC_TRACE_PRINTF_DEBUG("smc_instance_restart: return new SMC instance 0x%08X", (uint32_t)smc_instance_new);

        return smc_instance_new;
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_instance_restart: unable to restart NULL instance");
        return NULL;
    }
}

/*
 * Calculates next valid free SHM address based on bytes consumed.
 */
static uint8_t* smc_instance_calculate_next_free_shm_address( smc_t* smc_instance, uint32_t bytes_consumed)
{
    uint8_t* address = smc_instance->smc_memory_first_free;

    address += bytes_consumed;

    /* TODO Align if necessary */


    SMC_TRACE_PRINTF_INFO("smc_instance_calculate_next_free_shm_address: New free SHM address is 0x%08X", (uint32_t)address);

    return address;
}

/*
 * Create SMC channel based on configuration.
 *
 * NOTE: Every item value in configuration structure
 *       (except function ptrs and memory address references)
 *       must be copied to SMC channel because the configuration
 *       might be destroyed after the configuration.
 */
smc_channel_t* smc_channel_create( smc_channel_conf_t* smc_channel_conf )
{
    smc_channel_t* channel = (smc_channel_t*)SMC_MALLOC( sizeof( smc_channel_t ) );

    assert( channel != NULL );

        /* Setup the configuration */
    channel->id                            = smc_channel_conf->channel_id;
    channel->priority                      = smc_channel_conf->priority;
    channel->smc_instance                  = NULL;
    channel->copy_scheme                   = smc_channel_conf->copy_scheme; /*SMC_COPY_SCHEME_COPY_IN_SEND + SMC_COPY_SCHEME_COPY_IN_RECEIVE;*/    /* double-copy scheme is default*/
    channel->protocol                      = smc_channel_conf->protocol;
    channel->stop_counter                  = 0;
    channel->smc_shm_conf_channel          = NULL;
    channel->trace_features                = smc_channel_conf->trace_features;
    channel->version_remote                = 0x00000000;

        /* Initialize callback functions */
    channel->smc_receive_cb                = (smc_receive_data_callback)smc_channel_conf->smc_receive_data_cb;
    channel->smc_send_data_deallocator_cb  = (smc_send_data_deallocator_callback)smc_channel_conf->smc_send_data_deallocator_cb;

    channel->smc_receive_data_allocator_cb = (smc_receive_data_allocator_callback)smc_channel_conf->smc_receive_data_allocator_cb;
    channel->smc_event_cb                  = (smc_event_callback)smc_channel_conf->smc_event_cb;

        /*
         * FIFOs are created when channel is added to SMC instance and SHM is ready
         */
    channel->fifo_out   = NULL;
    channel->fifo_in    = NULL;

    if( smc_channel_conf->fifo_full_check_timeout_usec > 0 )
    {
        channel->fifo_timer = smc_timer_create( smc_channel_conf->fifo_full_check_timeout_usec );
    }
    else
    {
        channel->fifo_timer = NULL;
        SMC_TRACE_PRINTF_WARNING("smc_channel_create: timeout for FIFO full check not set");
    }

    channel->fifo_size_in  = smc_channel_conf->fifo_size_in;
    channel->fifo_size_out = smc_channel_conf->fifo_size_out;

    channel->mdb_size_in   = smc_channel_conf->mdb_size_in;
    channel->mdb_size_out  = smc_channel_conf->mdb_size_out;

        /*
         * MDBs are created when channel is added to SMC instance and SHM is ready
         */
    channel->mdb_out      = NULL;
    channel->mdb_in       = NULL;
    channel->smc_mdb_info = NULL;

        /* Initialize locks */
    channel->lock_write    = smc_lock_create();
    channel->lock_read     = smc_lock_create();
    channel->lock_mdb      = smc_lock_create();
    channel->lock_tx_queue = smc_lock_create();

        /* Initialize Signals */
    channel->signal_remote = smc_signal_copy( smc_channel_conf->signal_remote );
    channel->signal_local  = smc_signal_copy( smc_channel_conf->signal_local );

        /*
         * Set the default values for non-configurable items
         */
    channel->state = 0x00000000;

        /* Set default flags */
    SMC_CHANNEL_STATE_CLEAR_SYNCHRONIZED(channel->state);

        /*
         * Initialize FIFO buffer
         */
    channel->fifo_buffer_item_count = 0;
    channel->fifo_buffer_flushing   = FALSE;
    channel->fifo_buffer            = NULL;

#ifdef SMC_SEND_USE_SEMAPHORE
    channel->send_semaphore         = smc_semaphore_create();
#else
    channel->send_semaphore         = NULL;
#endif

    channel->dropped_packets_mdb_out     = 0;
    channel->dropped_packets_fifo_buffer = 0;
    channel->send_packets_fifo_buffer    = 0;
    channel->fifo_buffer_copied_total    = 0;
    SMC_TRACE_PRINTF_DEBUG("smc_channel_create: channel %d: 0x%08X created", channel->id, (uint32_t)channel);

    return channel;
}

/**
 * Configures shared memory area for specified SMC channel.
 */
static uint8_t smc_channel_configure_shm( smc_channel_t* smc_channel, smc_shm_config_t* smc_shm_conf, uint8_t is_master)
{
    uint8_t ret_val = SMC_ERROR;
    uint32_t bytes_consumed = 0;

    SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: Start SHM configuration...", smc_channel->id);

    assert( smc_channel != NULL );
    assert( smc_shm_conf != NULL );

    if( smc_shm_conf->shm_area_start_address != NULL )
    {
        assert( smc_shm_conf->size > 0 );

        smc_channel->smc_shm_conf_channel = smc_shm_conf;

        SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: SHM configuration found, start address 0x%08X, bytes available %d, create FIFOs", smc_channel->id, (uint32_t)smc_channel->smc_shm_conf_channel->shm_area_start_address,
                                                                                                            smc_channel->smc_shm_conf_channel->size);

        ret_val = SMC_OK;   /* Changed to error if something goes wrong */

        for(int i = 0; i < 2; i++)
        {
            if( (i == 0 && is_master) || (i==1 && !is_master))
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: Initializing FIFO OUT size %d...", smc_channel->id, smc_channel->fifo_size_out);

                smc_channel->fifo_out = (smc_fifo_t*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);

                // TODO Cleanup smc_fifo_init_out( smc_channel->fifo_out, smc_channel_conf->fifo_size_out, smc_shm_conf->use_cache_control);
                smc_fifo_init_out( smc_channel->fifo_out, smc_channel->fifo_size_out, smc_shm_conf->use_cache_control);

                // TODO Cleanup bytes_consumed += smc_fifo_calculate_required_shared_mem(smc_channel_conf->fifo_size_out);
                bytes_consumed += smc_fifo_calculate_required_shared_mem(smc_channel->fifo_size_out);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: FIFO OUT 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->fifo_out, bytes_consumed);
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: Initializing FIFO IN size %d...", smc_channel->id, smc_channel->fifo_size_in);

                smc_channel->fifo_in = (smc_fifo_t*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);

                // TODO Cleanup smc_fifo_init_in( smc_channel->fifo_in, smc_channel_conf->fifo_size_in, smc_shm_conf->use_cache_control );
                smc_fifo_init_in( smc_channel->fifo_in, smc_channel->fifo_size_in, smc_shm_conf->use_cache_control );

                // TODO Cleanup bytes_consumed += smc_fifo_calculate_required_shared_mem(smc_channel_conf->fifo_size_in);
                bytes_consumed += smc_fifo_calculate_required_shared_mem(smc_channel->fifo_size_in);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: FIFO IN 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->fifo_in, bytes_consumed);
            }
        }

        smc_channel->smc_mdb_info = smc_mdb_channel_info_create();

        SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: Creating MDBs...", smc_channel->id);

        for(int i = 0; i < 2; i++)
        {
            if( (i == 0 && is_master) || (i==1 && !is_master))
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: Initializing MDB OUT size %d...", smc_channel->id, smc_channel->mdb_size_out);

                if( smc_channel->mdb_out != NULL )
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: MDB OUT already initialized", smc_channel->id);
                }

                /* TODO Remove MDB out pointer from channel */

                smc_channel->mdb_out                      = (uint8_t*)smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed;
                smc_channel->smc_mdb_info->pool_out       = (void*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);
                // TODO Cleanup smc_channel->smc_mdb_info->total_size_out = smc_channel_conf->mdb_size_out;
                smc_channel->smc_mdb_info->total_size_out = smc_channel->mdb_size_out;

                if( smc_mdb_create_pool_out( smc_channel->smc_mdb_info->pool_out, smc_channel->smc_mdb_info->total_size_out) != SMC_OK )
                {
                    SMC_TRACE_PRINTF_ASSERT("smc_channel_configure_shm: MDB pool out create failed");
                    assert(0);
                }

                // TODO Cleanup bytes_consumed += smc_mdb_calculate_required_shared_mem(smc_channel_conf->mdb_size_out);
                bytes_consumed += smc_mdb_calculate_required_shared_mem(smc_channel->mdb_size_out);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: MDB OUT 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->mdb_out, bytes_consumed);
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: Initializing MDB IN size %d...", smc_channel->id, smc_channel->mdb_size_in);

                if( smc_channel->mdb_in != NULL )
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: MDB IN already initialized", smc_channel->id);

                    /* TODO Destroy current MDB data*/
                }

                /* TODO Remove MDB in pointer from channel */
                smc_channel->mdb_in                      = (uint8_t*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);
                smc_channel->smc_mdb_info->pool_in       = (void*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);

                // TODO Cleanup smc_channel->smc_mdb_info->total_size_in = smc_channel_conf->mdb_size_in;
                smc_channel->smc_mdb_info->total_size_in = smc_channel->mdb_size_in;

                bytes_consumed += smc_mdb_calculate_required_shared_mem(smc_channel->smc_mdb_info->total_size_in);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: MDB IN 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->mdb_in, bytes_consumed);
            }
        }

        /* Check that the consumed memory does not exceed the size */
        SMC_TRACE_PRINTF_STARTUP("Channel %d: shared memory starts from 0x%08X size %d bytes (Cache control %s)",
                smc_channel->id, (uint32_t)smc_channel->smc_shm_conf_channel->shm_area_start_address, bytes_consumed,
                smc_shm_conf->use_cache_control?"enabled":"disabled");

        SMC_CHANNEL_STATE_SET_SHM_CONFIGURED(smc_channel->state);
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_channel_configure_shm: SHM start address is NULL");
        ret_val = SMC_ERROR;
    }

    SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: completed by return value 0x%02X, bytes left in CH SHM %d", smc_channel->id, ret_val, (smc_channel->smc_shm_conf_channel->size-bytes_consumed));

    return ret_val;
}

/*
 * Returns SMC Channel of specified SMC instance with specified SMC channel id.
 * If the SMC channel is not found, function returns NULL.
 */
smc_channel_t* smc_channel_get( const smc_t* smc_instance, uint8_t smc_channel_id )
{
    smc_channel_t* smc_channel = NULL;

    if( smc_instance != NULL && smc_instance->smc_channel_ptr_array && smc_channel_id < smc_instance->smc_channel_list_count )
    {
        smc_channel = smc_instance->smc_channel_ptr_array[smc_channel_id];
    }

    return smc_channel;
}

void smc_channel_destroy( smc_channel_t* smc_channel )
{
    if( smc_channel )
    {
        SMC_TRACE_PRINTF_STARTUP("Closing SMC Channel %d...", smc_channel->id);

        if( smc_channel->mdb_out != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X: destroy MDB INFO 0x%08X...", (uint32_t)smc_channel, (uint32_t)smc_channel->smc_mdb_info);
            smc_mdb_info_destroy(smc_channel->smc_mdb_info);
        }

        if( smc_channel->fifo_timer != NULL )
        {
            smc_timer_destroy( smc_channel->fifo_timer );
        }

        if( smc_channel->lock_write != NULL )
        {
            smc_lock_destroy(smc_channel->lock_write);
        }

        if( smc_channel->lock_read != NULL )
        {
            smc_lock_destroy(smc_channel->lock_read);
        }

        if( smc_channel->lock_mdb != NULL )
        {
            smc_lock_destroy(smc_channel->lock_mdb);
        }

        if( smc_channel->lock_tx_queue != NULL )
        {
            smc_lock_destroy(smc_channel->lock_tx_queue);
        }

        if( smc_channel->signal_remote != NULL )
        {
            smc_signal_destroy(smc_channel->signal_remote);
        }

        if( smc_channel->signal_local != NULL )
        {
                /* Local interrupt has IRQ handler registered */
            smc_signal_handler_unregister( smc_channel->smc_instance, smc_channel->signal_local, smc_channel );

            smc_signal_destroy(smc_channel->signal_local);
        }

        if( smc_channel->smc_shm_conf_channel )
        {
            smc_shm_conf_destroy(smc_channel->smc_shm_conf_channel);
        }

        if( smc_channel->fifo_buffer != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X: destroy FIFO buffer 0x%08X...", (uint32_t)smc_channel, (uint32_t)smc_channel->fifo_buffer);
            SMC_FREE( smc_channel->fifo_buffer );
            smc_channel->fifo_buffer = NULL;
        }

        if( smc_channel->send_semaphore != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X: destroy send semaphore 0x%08X...", (uint32_t)smc_channel, (uint32_t)smc_channel->send_semaphore);
            SMC_FREE( smc_channel->send_semaphore );
            smc_channel->send_semaphore = NULL;
        }

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED
        if( smc_channel->smc_history_data_sent != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X: destroy sent data history buffer 0x%08X...", (uint32_t)smc_channel, (uint32_t)smc_channel->smc_history_data_sent);
            SMC_FREE( smc_channel->smc_history_data_sent );
            smc_channel->smc_history_data_sent = NULL;
        }

        if( smc_channel->smc_history_data_received != NULL )
        {
            SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X: destroy received data history buffer 0x%08X...", (uint32_t)smc_channel, (uint32_t)smc_channel->smc_history_data_received);
            SMC_FREE( smc_channel->smc_history_data_received );
            smc_channel->smc_history_data_received = NULL;
        }
#endif

        /* Finally destroy the smc channel pointer */

        SMC_FREE( smc_channel );

        smc_channel = NULL;

        SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X completed", (uint32_t)smc_channel);
    }
}

/**
 * Adds channel to the channel list of smc instance.
 */
uint8_t smc_add_channel(smc_t* smc_instance, smc_channel_t* smc_channel, smc_channel_conf_t* smc_channel_conf)
{
    uint8_t           ret_val           = SMC_OK;
    smc_channel_t**   old_channel_array = NULL;
    smc_shm_config_t* channel_shm       = NULL;
    smc_lock_t*       local_lock        = NULL;

    assert( smc_instance != NULL );
    assert( smc_channel_conf != NULL );

    SMC_TRACE_PRINTF_DEBUG("smc_add_channel: channel 0x%08X to SMC instance 0x%08X starts...", (uint32_t)smc_channel, (uint32_t)smc_instance);

    local_lock = get_local_lock_smc_channel();
    SMC_LOCK_IRQ( local_lock );

    smc_instance->smc_channel_list_count++;

    if( smc_instance->smc_channel_ptr_array )
    {
        SMC_TRACE_PRINTF_INFO("smc_add_channel: Store old array pointer 0x%08X", (uint32_t)smc_instance->smc_channel_ptr_array);
        old_channel_array = smc_instance->smc_channel_ptr_array;
    }

    smc_instance->smc_channel_ptr_array = (smc_channel_t**)SMC_MALLOC( sizeof(smc_channel_t*) * smc_instance->smc_channel_list_count );

    assert( smc_instance->smc_channel_ptr_array != NULL );

    if( old_channel_array )
    {
        int i = 0;

        SMC_TRACE_PRINTF_INFO("smc_add_channel: old channel found, add them to new array...");
        for( i = 0; i < smc_instance->smc_channel_list_count-1; i++ )
        {
            smc_instance->smc_channel_ptr_array[i] = old_channel_array[i];
            smc_instance->smc_channel_ptr_array[i]->id = i;
        }

        smc_instance->smc_channel_ptr_array[smc_instance->smc_channel_list_count-1] = smc_channel;
        smc_channel->id = smc_instance->smc_channel_list_count-1;

        SMC_FREE( old_channel_array );
        old_channel_array = NULL;
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_add_channel: First channel ptr 0x%08X added to array pointer 0x%08X", (uint32_t)smc_channel, (uint32_t)smc_instance->smc_channel_ptr_array);
        smc_instance->smc_channel_ptr_array[0] = smc_channel;
        smc_channel->id = 0;
    }

    smc_channel->smc_instance = smc_instance;

    if( smc_instance->smc_shm_conf && smc_instance->smc_shm_conf->shm_area_start_address != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_add_channel: Configure SHM for channel 0x%08X...", (uint32_t)smc_channel);

        channel_shm = smc_shm_config_copy(smc_instance->smc_shm_conf);

        channel_shm->size = smc_channel_calculate_required_shared_mem(smc_channel_conf);
        SMC_TRACE_PRINTF_INFO("smc_add_channel: Channel requires %d bytes of SHM", channel_shm->size);

            /* Check that there is enough SHM memory for channel */
        if( channel_shm->size <= smc_instance_get_free_shm(smc_instance) )
        {
                /* Get the SHM start address for the channel */
            channel_shm->shm_area_start_address = smc_instance->smc_memory_first_free;

            if( smc_channel_configure_shm( smc_channel, channel_shm, smc_instance->is_master ) == SMC_OK )
            {
                smc_instance->smc_memory_first_free = smc_instance_calculate_next_free_shm_address( smc_instance, channel_shm->size );
            }
            else
            {
                SMC_TRACE_PRINTF_ASSERT("smc_add_channel: smc_channel_configure_shm failed");
                assert(0);
            }
        }
        else
        {
            int iFreeShm = smc_instance_get_free_shm(smc_instance);
            SMC_TRACE_PRINTF_ASSERT("smc_add_channel: Not enough shared memory for SMC channel %d. SHM requested: %d, SHM free %d -> needs %d more", smc_channel->id,
                    channel_shm->size, iFreeShm, (channel_shm->size-iFreeShm) );
            assert(0);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_add_channel: SMC instance have no valid SHM for channel 0x%08X", (uint32_t)smc_channel);
    }

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED

    smc_channel->smc_history_index_sent     = 0;
    smc_channel->smc_history_index_received = 0;

    if( SMC_TRACE_HISTORY_DATA_SEND_ENABLED(smc_channel->trace_features) )
    {
        SMC_TRACE_PRINTF_HISTORY("smc_add_channel: channel %d: send history is enabled", smc_channel->id);

        smc_channel->smc_history_len_sent      = SMC_TRACE_HISTORY_DATA_ARRAY_SIZE;
        smc_channel->smc_history_data_sent     = smc_history_data_array_create( smc_channel->smc_history_len_sent );
    }
    else
    {
        SMC_TRACE_PRINTF_HISTORY("smc_add_channel: channel %d: send history is disabled", smc_channel->id);

        smc_channel->smc_history_len_sent      = 0;
        smc_channel->smc_history_data_sent     = NULL;
    }

    if( SMC_TRACE_HISTORY_DATA_RECEIVE_ENABLED(smc_channel->trace_features) )
    {
        SMC_TRACE_PRINTF_HISTORY("smc_add_channel: channel %d: receive history is enabled", smc_channel->id);
        smc_channel->smc_history_len_received  = SMC_TRACE_HISTORY_DATA_ARRAY_SIZE;
        smc_channel->smc_history_data_received = smc_history_data_array_create( smc_channel->smc_history_len_sent );
    }
    else
    {
        SMC_TRACE_PRINTF_HISTORY("smc_add_channel: channel %d: receive history is disabled", smc_channel->id);
        smc_channel->smc_history_len_received  = 0;
        smc_channel->smc_history_data_received = NULL;
    }

#endif /* #ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED */

        /*
         * Before sync initialize receiver interrupt because the SMC channel hierachy is ready
         * NOTE: Do any channel initialization before this because it is possible that IRQ comes right after
         *       the signal handler is registered
         */
    if( smc_channel->signal_local )
    {
        SMC_TRACE_PRINTF_INFO("smc_add_channel: Initializing local interrupt for channel %d...", smc_channel->id);
        assert(smc_channel->signal_local != NULL );

        ret_val = smc_signal_handler_register( smc_instance, smc_channel->signal_local, smc_channel);

        assert(ret_val == SMC_OK);
    }

    SMC_UNLOCK_IRQ( local_lock );

        /*
         * Finally send the sync message to remote
         */
    ret_val = smc_channel_handle_sync( smc_channel, SMC_MSG_FLAG_SYNC_SEND_REQ, SMC_SYNC_MSG_FIFO_REQ );

    SMC_TRACE_PRINTF_DEBUG("smc_add_channel: New channel count is %d, returning value 0x%02X", smc_instance->smc_channel_list_count, ret_val);
    return ret_val;
}

smc_conf_t* smc_instance_get_conf( smc_t* smc_instance )
{
    SMC_TRACE_PRINTF_ASSERT("smc_instance_get_conf: NOT IMPLEMENTED");
    assert(0);
    return NULL;
}

smc_channel_conf_t* smc_channel_get_conf( smc_channel_t* smc_channel )
{
    SMC_TRACE_PRINTF_ASSERT("smc_channel_get_conf: NOT IMPLEMENTED");
    assert(0);
    return NULL;
}

/*
 * Returns the current amount of free SHM of specified SMC instance
 */
uint32_t smc_instance_get_free_shm( smc_t* smc_instance )
{
    uint32_t  free_mem = 0;

    assert( smc_instance != NULL );
    assert( smc_instance->smc_shm_conf != NULL );

    free_mem = smc_instance->smc_shm_conf->size - ( ((uint32_t)smc_instance->smc_memory_first_free) - ((uint32_t)smc_instance->smc_shm_conf->shm_area_start_address));

    return free_mem;
}

/*
 * Calculates required shared memory in bytes for one channel based on the channel configuration
 */
uint32_t smc_channel_calculate_required_shared_mem( smc_channel_conf_t* smc_channel_conf )
{
    uint32_t required_mem = 0;

    required_mem += smc_fifo_calculate_required_shared_mem(smc_channel_conf->fifo_size_out);
    required_mem += smc_fifo_calculate_required_shared_mem(smc_channel_conf->fifo_size_in);

    required_mem += smc_mdb_calculate_required_shared_mem(smc_channel_conf->mdb_size_out);
    required_mem += smc_mdb_calculate_required_shared_mem(smc_channel_conf->mdb_size_in);

    SMC_TRACE_PRINTF_INFO("smc_channel_calculate_required_shared_mem: Channel requires %d bytes of shared memory", required_mem);

    return required_mem;
}

/*
 * Change the SMC Channel priority.
 * Returns the old priority.
 */
uint8_t smc_channel_change_priority( smc_t* smc_instance, uint8_t smc_channel_id, uint8_t new_priority )
{
    uint8_t old_priority = SMC_CHANNEL_PRIORITY_LOWEST;

    smc_channel_t* channel = smc_channel_get( smc_instance, smc_channel_id );

    if( channel != NULL )
    {
        old_priority = channel->priority;
        channel->priority = new_priority;
    }

    return old_priority;
}

/*
 * Returns signal from signal array.
 */
smc_signal_handler_t* smc_signal_handler_get( uint32_t signal_id, uint32_t signal_type )
{
    SMC_TRACE_PRINTF_SIGNAL("smc_signal_get: search signal handler for signal id %d, type 0x%08X", signal_id, signal_type);

    for(int i = 0; i < signal_handler_count; i++ )
    {
        smc_signal_t* signal = signal_handler_ptr_array[i]->signal;

        if( ((signal->interrupt_id - signal_id) | (signal->signal_type - signal_type))==0 )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_get: return signal handler 0x%08X", (uint32_t)signal_handler_ptr_array[i]);
            return signal_handler_ptr_array[i];
        }
    }

    SMC_TRACE_PRINTF_ERROR("smc_signal_handler_get: Handler for signal %d type 0x%08X not set", signal_id, signal_type);

    return NULL;
}

smc_signal_handler_t* smc_signal_handler_create_and_add( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel )
{
    smc_signal_handler_t* signal_handler = NULL;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_create_and_add(ch: %d): starts...", (smc_channel!=NULL)?smc_channel->id:-1);

    signal_handler = (smc_signal_handler_t*)SMC_MALLOC( sizeof( smc_signal_handler_t ) );

    assert( signal_handler != NULL );

    signal_handler->signal       = signal;
    signal_handler->smc_instance = smc_instance;
    signal_handler->smc_channel  = smc_channel;

    smc_signal_add_handler( signal_handler );

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_create_and_add(ch: %d): created 0x%08X", (smc_channel!=NULL)?smc_channel->id:-1, (uint32_t)signal_handler);

    return signal_handler;
}

uint8_t smc_signal_add_handler( smc_signal_handler_t* signal_handler )
{
    smc_signal_handler_t** old_ptr_array = NULL;

    smc_lock_t* local_lock = get_local_lock_signal_handler();
    SMC_LOCK_IRQ( local_lock );

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_add_handler: add handler 0x%08X, current count %d", (uint32_t)signal_handler, signal_handler_count);

    assert( signal_handler!=NULL );

        /* Check if handler exists */
    if( signal_handler_ptr_array )
    {
        for(int i = 0; i < signal_handler_count; i++ )
        {
            if( signal_handler_ptr_array[i]->signal->interrupt_id == signal_handler->signal->interrupt_id &&
                signal_handler_ptr_array[i]->signal->signal_type == signal_handler->signal->signal_type )
            {
                SMC_TRACE_PRINTF_ASSERT("smc_signal_add_handler: Handler for interrupt %d type 0x%08X already exists",
                        signal_handler->signal->interrupt_id, signal_handler->signal->signal_type);
                assert(0);
            }
        }

        old_ptr_array = signal_handler_ptr_array;
    }

    signal_handler_count++;

    signal_handler_ptr_array = (smc_signal_handler_t**)SMC_MALLOC( sizeof(signal_handler_ptr_array) * signal_handler_count );

    if( old_ptr_array )
    {
        for(int i = 0; i < signal_handler_count; i++ )
        {
            signal_handler_ptr_array[i] = old_ptr_array[i];
        }

        SMC_FREE(old_ptr_array);
        old_ptr_array = NULL;
    }

    signal_handler_ptr_array[signal_handler_count-1] = signal_handler;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_add_handler: completed, signal handler count is %d", signal_handler_count);

    SMC_UNLOCK_IRQ( local_lock );

    return SMC_OK;
}

void smc_signal_handler_remove_and_destroy( smc_signal_handler_t* signal_handler )
{
    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_remove_and_destroy: remove handler 0x%08X", (uint32_t)signal_handler );

    if(signal_handler != NULL )
    {
        smc_signal_remove_handler( signal_handler );

        SMC_FREE( signal_handler );
        signal_handler = NULL;

        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_remove_and_destroy: completed");
    }
    else
    {
        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_remove_and_destroy: handler is NULL");
    }
}

void smc_signal_remove_handler( smc_signal_handler_t* signal_handler )
{
    SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: remove handler 0x%08X, current count %d", (uint32_t)signal_handler, signal_handler_count);

    if(signal_handler != NULL )
    {
        smc_signal_handler_t** old_ptr_array = NULL;

        uint8_t     handler_found = 0;
        uint8_t     signal_index  = 0;

        smc_lock_t* local_lock = get_local_lock_signal_handler();
        SMC_LOCK_IRQ( local_lock );

            /* Check if handler exists */
        if( signal_handler_ptr_array )
        {
            for(int i = 0; i < signal_handler_count; i++ )
            {
                if( signal_handler_ptr_array[i]->signal->interrupt_id == signal_handler->signal->interrupt_id &&
                    signal_handler_ptr_array[i]->signal->signal_type == signal_handler->signal->signal_type )
                {
                    SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: Handler for interrupt %d type 0x%08X found from index %d",
                            signal_handler->signal->interrupt_id, signal_handler->signal->signal_type, i);

                    handler_found = 1;
                    signal_index  = i;
                    break;
                }
            }

            old_ptr_array = signal_handler_ptr_array;
        }

        if( handler_found > 0 )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: handler found, removing from the list...");

            signal_handler_count--;

            if( signal_handler_count > 0 )
            {
                signal_handler_ptr_array = (smc_signal_handler_t**)SMC_MALLOC( sizeof(signal_handler_ptr_array) * signal_handler_count );
            }
            else
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: handler count is 0, set global list 0x%08X to NULL", (uint32_t)signal_handler_ptr_array);
                signal_handler_ptr_array = NULL;
                signal_handler_count = 0;
                /* The original pointer is freed later */
            }

            if( old_ptr_array )
            {
                uint8_t temp_index = 0;

                for(int i = 0; i < signal_handler_count+1; i++ )
                {
                    if( i != signal_index )
                    {
                        SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: add existing signal handler from index %d -> %d", i, temp_index);
                        signal_handler_ptr_array[temp_index] = old_ptr_array[i];
                        temp_index++;
                    }
                }

                SMC_FREE(old_ptr_array);
                old_ptr_array = NULL;
            }
        }
        else
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: signal handler not found (%d)", handler_found);
        }

        SMC_TRACE_PRINTF_SIGNAL("smc_signal_remove_handler: completed, signal handler count is %d", signal_handler_count);

        SMC_UNLOCK_IRQ( local_lock );
    }
}

static uint8_t smc_channel_handle_sync( smc_channel_t* smc_channel, uint32_t sync_flag, uint32_t sync_msg )
{
    uint8_t ret_val = SMC_OK;
    uint32_t old_state = smc_channel->state;

    smc_lock_t* local_lock = get_local_lock_smc_channel_sync();
    SMC_LOCK_IRQ( local_lock );

    SMC_TRACE_PRINTF_DEBUG("smc_channel_handle_sync(ch %d, 0x%08X): handle sync msg 0x%08X, state 0x%08X", smc_channel->id,(uint32_t)smc_channel,
                                    sync_msg,
                                    smc_channel->state);
    switch(sync_flag)
    {
        case SMC_MSG_FLAG_SYNC_SEND_REQ:
        {
            smc_user_data_t userdata;

            userdata.flags     = SMC_MSG_FLAG_SYNC_INFO_REQ;
            userdata.userdata1 = 0x00000000;
            userdata.userdata2 = 0x00000000;
            userdata.userdata3 = 0x00000000;
            userdata.userdata4 = 0x00000000;
            userdata.userdata5 = 0x00000000;

            SMC_TRACE_PRINTF_DEBUG("smc_channel_handle_sync(ch %d, 0x%08X): <==== Sending SYNC REQ...", smc_channel->id,(uint32_t)smc_channel);

            ret_val = smc_send_ext(smc_channel, (void*)sync_msg, 0, &userdata);

            SMC_CHANNEL_STATE_SET_SYNC_SENT( smc_channel->state );

            break;
        }
        case SMC_MSG_FLAG_SYNC_INFO_REQ:
        {
            uint32_t message = SMC_SYNC_MSG_FIFO_RESP;
            smc_user_data_t userdata;

            userdata.flags     = SMC_MSG_FLAG_SYNC_INFO_RESP;
            userdata.userdata1 = 0x00000000;
            userdata.userdata2 = 0x00000000;
            userdata.userdata3 = 0x00000000;
            userdata.userdata4 = 0x00000000;
            userdata.userdata5 = 0x00000000;

            if( sync_msg == SMC_SYNC_MSG_FIFO_REQ )
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_handle_sync(ch %d, 0x%08X): ===> Received REQ flag with correct message 0x%08X, sending response...", smc_channel->id,(uint32_t)smc_channel, sync_msg);
                ret_val = smc_send_ext(smc_channel, (void*)message, 0, &userdata);

                SMC_CHANNEL_STATE_CLEAR_SYNC_SENT( smc_channel->state );
                SMC_CHANNEL_STATE_CLEAR_SYNC_RECEIVED(smc_channel->state);
                SMC_CHANNEL_STATE_SET_SYNCHRONIZED(smc_channel->state);
            }
            else
            {
                SMC_TRACE_PRINTF_WARNING("smc_channel_handle_sync(ch %d, 0x%08X): Received REQ flag with invalid message 0x%08X, sending request...", smc_channel->id,(uint32_t)smc_channel, sync_msg);
                message        = SMC_SYNC_MSG_FIFO_REQ;
                userdata.flags = SMC_MSG_FLAG_SYNC_INFO_REQ;

                ret_val = smc_send_ext(smc_channel, (void*)message, 0, &userdata);

                SMC_CHANNEL_STATE_SET_SYNC_SENT( smc_channel->state );
            }

            break;
        }
        case SMC_MSG_FLAG_SYNC_INFO_RESP:
        {
            if( !SMC_CHANNEL_STATE_SET_SYNC_SENT( smc_channel->state ) )
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_handle_sync(ch %d, 0x%08X): ====> Received RESP without REQ, setting state synchronized anyway", smc_channel->id,(uint32_t)smc_channel);
            }

            SMC_CHANNEL_STATE_CLEAR_SYNC_SENT( smc_channel->state );
            SMC_CHANNEL_STATE_CLEAR_SYNC_RECEIVED(smc_channel->state);
            SMC_CHANNEL_STATE_SET_SYNCHRONIZED(smc_channel->state);

            break;
        }
        default:
        {
            ret_val = SMC_ERROR;
        }
    }

    SMC_TRACE_PRINTF_INFO("smc_channel_handle_sync(ch %d, 0x%08X): ==== sync msg 0x%08X handled: new state 0x%08X (%s), return value 0x%02X", smc_channel->id,(uint32_t)smc_channel,
            sync_msg, smc_channel->state,
            SMC_CHANNEL_STATE_IS_SYNCHRONIZED(smc_channel->state)?"IN SYNC":"NOT IN SYNC", ret_val);

    if( SMC_CHANNEL_STATE_IS_READY_TO_SEND( smc_channel->state ) && !SMC_CHANNEL_STATE_IS_READY_TO_SEND(old_state))
    {
        smc_user_data_t userdata_version;

            /* Send the event to user if callback initialized */
        if( smc_channel->smc_event_cb )
        {
            SMC_TRACE_PRINTF_INFO("smc_channel_handle_sync(ch %d, 0x%08X): send SMC_CHANNEL_READY_TO_SEND event to user...", smc_channel->id,(uint32_t)smc_channel);
            smc_channel->smc_event_cb(smc_channel, SMC_CHANNEL_READY_TO_SEND, NULL);
        }
        else
        {
            SMC_TRACE_PRINTF_INFO("smc_channel_handle_sync(ch %d, 0x%08X): event cb not initialized by the user: SMC_CHANNEL_READY_TO_SEND not sent", smc_channel->id,(uint32_t)smc_channel);
        }

        SMC_TRACE_PRINTF_DEBUG("smc_channel_handle_sync(ch %d, 0x%08X): ==================== CHANNEL %d IS READY TO SEND AND RECEIVE DATA ====================", smc_channel->id, (uint32_t)smc_channel, smc_channel->id);

            /* Send version info req */
        userdata_version.flags     = SMC_MSG_FLAG_VERSION_INFO_REQ;
        userdata_version.userdata1 = 0x00000000;
        userdata_version.userdata2 = 0x00000000;
        userdata_version.userdata3 = 0x00000000;
        userdata_version.userdata4 = 0x00000000;
        userdata_version.userdata5 = 0x00000000;

        SMC_TRACE_PRINTF_DEBUG("smc_channel_handle_sync(ch %d, 0x%08X): Send SMC_MSG_FLAG_VERSION_INFO_REQ event...", smc_channel->id,(uint32_t)smc_channel);

        if( smc_send_ext(smc_channel, NULL, 0, &userdata_version) != SMC_OK )
        {
            SMC_TRACE_PRINTF_WARNING("smc_channel_handle_sync(ch %d, 0x%08X): SMC_MSG_FLAG_VERSION_INFO_REQ event send failed", smc_channel->id,(uint32_t)smc_channel);
        }

            /* Flush the FIFO buffer */
        smc_channel_buffer_fifo_flush( smc_channel );
    }

    SMC_UNLOCK_IRQ( local_lock );

    return ret_val;
}


/**
 * Returns TRUE if all channels of specified SMC instance are synchronized with the remote.
 *
 */
uint8_t smc_is_all_channels_synchronized(smc_t* smc_instance)
{
    uint8_t all_in_sync = FALSE;

    if( smc_instance != NULL && smc_instance->smc_instance_conf != NULL)
    {
            /* Checks also that all configs are initialized to SMC instance */
        if( smc_instance->smc_channel_list_count == smc_instance->smc_instance_conf->smc_channel_conf_count )
        {
            for(int i = 0; i < smc_instance->smc_channel_list_count; i++ )
            {
                smc_channel_t* smc_channel = smc_instance->smc_channel_ptr_array[i];

                all_in_sync = SMC_CHANNEL_STATE_IS_SYNCHRONIZED(smc_channel->state);

                if( !all_in_sync )
                {
                    break;
                }
            }
        }
    }

    return all_in_sync;
}

static uint8_t smc_send_negotiable_configurations(smc_t* smc_instance)
{
    uint8_t ret_value = SMC_ERROR;

    for(int i = 0; i < smc_instance->smc_channel_list_count; i++ )
    {
        smc_channel_t* smc_channel = smc_instance->smc_channel_ptr_array[i];

        SMC_TRACE_PRINTF_STARTUP("Channel %d negotiating configuration...", smc_channel->id);

            /* Invoke device specific configuration request */
        ret_value = smc_conf_request_initiate(smc_channel);

        if( ret_value != SMC_OK )
        {
            SMC_TRACE_PRINTF_WARNING("smc_send_negotiable_configurations: channel %d negotiating failed", smc_channel->id);
        }
        else
        {
            /* Set lock */
            SMC_INSTANCE_STATUS_SET_FIXED_CONFIG_SENT(smc_instance->init_status);
        }
    }

    return ret_value;
}

/*
 * Buffers one FIFO message.
 */
static uint8_t smc_channel_buffer_fifo_message(smc_channel_t* channel, void* data, uint32_t data_length, smc_user_data_t* userdata)
{
    uint8_t ret_value = SMC_ERROR;

    SMC_TRACE_PRINTF_DEBUG("smc_channel_buffer_fifo_message: channel 0x%08X: buffering FIFO message 0x%08X...", (uint32_t)channel, (uint32_t)data);

    if( channel->fifo_buffer_flushing )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_channel_buffer_fifo_message: channel is flushing, unable to add items");
        return SMC_ERROR;
    }
    else
    {
        smc_lock_t* local_lock = get_local_lock_smc_fifo_buffer();

        SMC_LOCK( local_lock );

        if( channel->fifo_buffer == NULL )
        {
            int i = 0;
                /* Allocate the whole buffer here, buffer is freed when flushed */
            SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_message: allocate buffer for %d items...", SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX);
            channel->fifo_buffer = SMC_MALLOC( SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX * sizeof( smc_fifo_cell_t ) );

            assert( channel->fifo_buffer != NULL );

            for(i = 0; i < SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX; i++ )
            {
                channel->fifo_buffer[i].data = 0;
                channel->fifo_buffer[i].length = 0;
                channel->fifo_buffer[i].flags = 0;
                channel->fifo_buffer[i].userdata1 = 0;
                channel->fifo_buffer[i].userdata2 = 0;
                channel->fifo_buffer[i].userdata3 = 0;
                channel->fifo_buffer[i].userdata4 = 0;
                channel->fifo_buffer[i].userdata5 = 0;
            }

            channel->fifo_buffer_item_count = 0;
        }
        else
        {
            channel->fifo_buffer_item_count++;

            if( channel->fifo_buffer_item_count >= SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX )
            {
                channel->fifo_buffer_item_count = 0;
            }

            // Check if there is item in current position
            smc_fifo_cell_t fifo_cell = channel->fifo_buffer[channel->fifo_buffer_item_count];

            if( fifo_cell.data != 0 || fifo_cell.length > 0 || fifo_cell.flags != 0 )
            {
                smc_user_data_t userdata_free;

                userdata_free.flags     = fifo_cell.flags;
                userdata_free.userdata1 = fifo_cell.userdata1;
                userdata_free.userdata2 = fifo_cell.userdata2;
                userdata_free.userdata3 = fifo_cell.userdata3;
                userdata_free.userdata4 = fifo_cell.userdata4;
                userdata_free.userdata5 = fifo_cell.userdata5;


                smc_free_local_ptr( channel, (void*)fifo_cell.data, &userdata_free );

                channel->fifo_buffer[channel->fifo_buffer_item_count].data = 0;
                channel->fifo_buffer[channel->fifo_buffer_item_count].length = 0;
                channel->fifo_buffer[channel->fifo_buffer_item_count].flags = 0;

                channel->dropped_packets_fifo_buffer++;
            }
        }

        if( channel->fifo_buffer_item_count < SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX )
        {
            channel->fifo_buffer[channel->fifo_buffer_item_count].data      = (uint32_t)data;
            channel->fifo_buffer[channel->fifo_buffer_item_count].length    = data_length;
            channel->fifo_buffer[channel->fifo_buffer_item_count].flags     = userdata->flags;

            SMC_FIFO_SET_DATA_PTR_IS_BUFFERED( channel->fifo_buffer[channel->fifo_buffer_item_count].flags );

            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata1 = userdata->userdata1;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata2 = userdata->userdata2;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata3 = userdata->userdata3;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata4 = userdata->userdata4;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata5 = userdata->userdata5;

            //channel->fifo_buffer_item_count++;
            //SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_message: FIFO buffer has now %d items", channel->fifo_buffer_item_count);

            ret_value = SMC_OK;
        }
        else
        {
            SMC_TRACE_PRINTF_WARNING("smc_channel_buffer_fifo_message: FIFO buffer is full");
            ret_value = SMC_ERROR_BUFFER_FULL;
        }

        SMC_UNLOCK( local_lock );
    }

    SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_message: completed by return value 0x%02X", ret_value);

    return ret_value;
}

/*
 * Flushes the FIFO buffer.
 */
static uint8_t smc_channel_buffer_fifo_flush( smc_channel_t* channel )
{
    uint8_t ret_val = SMC_ERROR;

    //smc_lock_t* local_lock = get_local_lock_smc_fifo_buffer_flush();

    smc_lock_t* local_lock = get_local_lock_smc_fifo_buffer();

    SMC_LOCK( local_lock );

    SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_flush: channel %d (0x%08X)...", channel->id, (uint32_t)channel);

    //if( channel->fifo_buffer != NULL && channel->fifo_buffer_item_count > 0 )
    if( channel->fifo_buffer != NULL )
    {
        uint8_t  flush_success = SMC_OK;
        uint32_t counter = 0;

        SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d (0x%08X), flushing %d items from the buffer...", channel->id, (uint32_t)channel, channel->fifo_buffer_item_count);

        //for(int i = 0; i < channel->fifo_buffer_item_count; i++ )

        int index = channel->fifo_buffer_item_count++;

        if( index >= SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX )
        {
            index = 0;
        }

        while(counter < SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX )
        {
            smc_fifo_cell_t fifo_cell = channel->fifo_buffer[index];

            if( fifo_cell.data != 0 || fifo_cell.length > 0 || fifo_cell.flags != 0 )
            {
                smc_user_data_t userdata;

                userdata.flags     = fifo_cell.flags;
                userdata.userdata1 = fifo_cell.userdata1;
                userdata.userdata2 = fifo_cell.userdata2;
                userdata.userdata3 = fifo_cell.userdata3;
                userdata.userdata4 = fifo_cell.userdata4;
                userdata.userdata5 = fifo_cell.userdata5;

                SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d: sending item from index %d...", channel->id, index);

                if( smc_send_ext( channel, (void*)fifo_cell.data, fifo_cell.length, &userdata ) == SMC_OK )
                {
                    SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d: item successfully send from index %d", channel->id, index);

                    /* "Mark" data as sent */
                    channel->fifo_buffer[index].data   = 0;
                    channel->fifo_buffer[index].length = 0;
                    channel->fifo_buffer[index].flags  = 0;

                    channel->send_packets_fifo_buffer++;
                }
                else
                {
                    SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d: item send from index %d failed", channel->id, index);
                    flush_success = SMC_ERROR;
                    break;
                }
            }
            else
            {
                SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d: item index %d was already sent or never set", channel->id, index);
            }

            if( flush_success == SMC_OK )
            {
                index++;

                if( index >= SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX )
                {
                    index = 0;
                }
            }
            else
            {
                break;
            }

            counter++;
        }

        if( flush_success == SMC_OK )
        {
            SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d (0x%08X) buffer successfully flushed", channel->id, (uint32_t)channel);

            channel->fifo_buffer_item_count = 0;
            /* Do not clear the buffer */
            //SMC_FREE( channel->fifo_buffer );
            //channel->fifo_buffer = NULL;
            ret_val = SMC_OK;
        }
    }
    else
    {
        SMC_TRACE_PRINTF_FIFO_BUFFER("smc_channel_buffer_fifo_flush: channel %d (0x%08X): FIFO buffer is empty", channel->id, (uint32_t)channel);
        ret_val = SMC_OK;
    }

    SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_flush: channel %d (0x%08X) completed by return value 0x%02X", channel->id, (uint32_t)channel, ret_val);

    SMC_UNLOCK( local_lock );

    return ret_val;
}

#if(SMC_CONTROL!=TRUE)
/*
 * Dummy implementation if the SMC Control Instance is not built in.
 * The real implementation is in the platform level code
 */
smc_t* smc_instance_get_control( void )
{
    SMC_TRACE_PRINTF_ERROR("smc_instance_get_control: SMC Control Instance is not available");
    return NULL;
}
#endif

void smc_instance_dump(smc_t* smc_instance)
{
    if( !smc_instance )
    {
        SMC_TRACE_PRINTF("smc_instance_dump: SMC instance is NULL");
        return;
    }

    SMC_TRACE_PRINTF_ALWAYS("SMC: ");
    SMC_TRACE_PRINTF_ALWAYS("SMC: Instance: 0x%08X v.%s (%s) CPU ID 0x%02X, Remote CPU ID: %d, channel count %d", (uint32_t)smc_instance, SMC_SW_VERSION,
                                                                             smc_instance->is_master?"Master":"Slave",
                                                                             smc_instance->cpu_id_local,
                                                                             smc_instance->cpu_id_remote,
                                                                             smc_instance->smc_channel_list_count);



    if( smc_instance->smc_shm_conf )
    {
        SMC_TRACE_PRINTF_ALWAYS("SMC:  - SHM starts from address 0x%08X, size %d, bytes used %d, bytes left %d, Cache control (%s)",
                (uint32_t)smc_instance->smc_shm_conf->shm_area_start_address,
                smc_instance->smc_shm_conf->size,
                (smc_instance->smc_shm_conf->size-smc_instance_get_free_shm(smc_instance)),
                smc_instance_get_free_shm(smc_instance),
                smc_instance->smc_shm_conf->use_cache_control?"ENABLED":"DISABLED");
    }
    else
    {
        SMC_TRACE_PRINTF_ALWAYS("SMC:  - <SHM not initialized>");
    }

    if( smc_instance->smc_channel_list_count > 0 && smc_instance->smc_channel_ptr_array != NULL)
    {
        int32_t mem_offset = 0;

        if( smc_instance->smc_shm_conf )
        {
            mem_offset = (int32_t)smc_instance->smc_shm_conf->remote_cpu_memory_offset;
        }

        for( int i = 0; i < smc_instance->smc_channel_list_count; i++ )
        {
            smc_channel_t* channel = smc_instance->smc_channel_ptr_array[i];

            SMC_TRACE_PRINTF_ALWAYS("SMC:  ");
            SMC_TRACE_PRINTF_ALWAYS("SMC:  - CH %d: priority: 0x%02X, protocol: 0x%02X, %s (0x%08X), Locks W: 0x%08X, R: 0x%08X, MDB: 0x%08X, TX: 0x%08X",
                                                                             channel->id,
                                                                             channel->priority,
                                                                             channel->protocol,
                                                                             SMC_CHANNEL_STATE_IS_SYNCHRONIZED(channel->state)?"IN SYNC":"NOT SYNC",
                                                                             channel->state,
                                                                             (uint32_t)channel->lock_write,
                                                                             (uint32_t)channel->lock_read,
                                                                             (uint32_t)channel->lock_mdb,
                                                                             (uint32_t)channel->lock_tx_queue);

            SMC_TRACE_PRINTF_ALWAYS("SMC:     TX: %s (lock sem counter %d), RX: %s, traces: 0x%02X, remote version %d.%d.%d",
                    SMC_CHANNEL_STATE_SEND_IS_DISABLED(channel->state)?"--DISABLED--":"Enabled",
                    channel->stop_counter,
                    SMC_CHANNEL_STATE_RECEIVE_IS_DISABLED(channel->state)?"--DISABLED--":"Enabled",
                    channel->trace_features,
                    SMC_VERSION_MAJOR(channel->version_remote),
                    SMC_VERSION_MINOR(channel->version_remote),
                    SMC_VERSION_REVISION(channel->version_remote));

            SMC_TRACE_PRINTF_ALWAYS("SMC:     Out of MDB mem send %d times, Fifo buffer delivered %d, lost %d, buffered %d bytes",
                    channel->dropped_packets_mdb_out, channel->send_packets_fifo_buffer,channel->dropped_packets_fifo_buffer,
                    channel->fifo_buffer_copied_total);

            /* Dump the FIFO, SIGNAL and MDB data */

            if( channel->fifo_out != NULL )
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - FIFO OUT:");
                  smc_fifo_dump( "SMC:        ", channel->fifo_out, mem_offset );
            }
            else
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - <FIFO OUT is not initialized>");
            }

            SMC_TRACE_PRINTF_ALWAYS("SMC:    - Signal OUT:");
            smc_signal_dump("SMC:        ", channel->signal_remote);

            if( channel->smc_mdb_info != NULL )
            {
                smc_mdb_info_dump( "SMC:    ", channel->smc_mdb_info, mem_offset, TRUE);
            }
            else
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - <MDB is not initialized>");
            }

            SMC_TRACE_PRINTF_ALWAYS("SMC:  ");

            if( channel->fifo_in != NULL )
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - FIFO IN:");
                  smc_fifo_dump( "SMC:        ", channel->fifo_in, mem_offset );
            }
            else
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - <FIFO IN  is not initialized>");
            }

            SMC_TRACE_PRINTF_ALWAYS("SMC:    - Signal IN:");
                    smc_signal_dump("SMC:        ", channel->signal_local);


            if( channel->smc_mdb_info != NULL )
            {
                smc_mdb_info_dump( "SMC:    ", channel->smc_mdb_info, mem_offset, FALSE);
            }
            else
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - <MDB is not initialized>");
            }

#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED

            if( SMC_TRACE_HISTORY_DATA_SEND_ENABLED(channel->trace_features) )
            {
                uint8_t found_items = FALSE;
                uint16_t counter    = 0;
                uint16_t index      = channel->smc_history_index_sent;

                SMC_TRACE_PRINTF_ALWAYS("SMC:    - Send data history (max %d items)", channel->smc_history_len_sent);
                SMC_TRACE_PRINTF_ALWAYS("SMC:        IDX  Timestamp  SHM Ptr     DataPtr     Length  Flags      Userdata1  Userdata2  Userdata3  Userdata4  Userdata5");

                while( counter < channel->smc_history_len_sent )
                {
                    if( channel->smc_history_data_sent[index].history_data_type != SMC_TRACE_HISTORY_DATA_TYPE_NONE )
                    {
                        SMC_TRACE_PRINTF_ALWAYS("SMC:    %5d  %11d  0x%08X  0x%08X  %6d  0x%08X  0x%08X 0x%08X 0x%08X 0x%08X 0x%08X", (counter+1),
                                channel->smc_history_data_sent[index].timestamp,
                                channel->smc_history_data_sent[index].data_ptr_shm + channel->smc_history_data_sent[index].shm_offset,
                                channel->smc_history_data_sent[index].data_ptr,
                                channel->smc_history_data_sent[index].data_length,
                                channel->smc_history_data_sent[index].userdata_flags,
                                channel->smc_history_data_sent[index].userdata1,
                                channel->smc_history_data_sent[index].userdata2,
                                channel->smc_history_data_sent[index].userdata3,
                                channel->smc_history_data_sent[index].userdata4,
                                channel->smc_history_data_sent[index].userdata5);

                        found_items = TRUE;
                    }

                    if( index > 0 )
                    {
                        index--;
                    }
                    else
                    {
                        index = channel->smc_history_len_sent-1;
                    }

                    counter++;
                }

                if( !found_items )
                {
                    SMC_TRACE_PRINTF_ALWAYS("SMC:       - <No data sent>");
                }

            }
            else
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - <Send data history is not recorded>");
            }

            if( SMC_TRACE_HISTORY_DATA_RECEIVE_ENABLED(channel->trace_features) )
            {
                uint16_t counter    = 0;
                uint16_t index      = channel->smc_history_index_received;
                uint8_t found_items = FALSE;

                SMC_TRACE_PRINTF_ALWAYS("SMC:    - Received data history (max %d items)", channel->smc_history_len_received);
                SMC_TRACE_PRINTF_ALWAYS("SMC:        IDX  Timestamp  SHM Ptr     DataPtr     Length  Flags      Userdata1  Userdata2  Userdata3  Userdata4  Userdata5");

                while( counter < channel->smc_history_len_received )
                {
                    if( channel->smc_history_data_received[index].history_data_type != SMC_TRACE_HISTORY_DATA_TYPE_NONE )
                    {
                        SMC_TRACE_PRINTF_ALWAYS("SMC:    %5d  %11d  0x%08X  0x%08X  %6d  0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X", (counter+1),
                                channel->smc_history_data_received[index].timestamp,
                                channel->smc_history_data_received[index].data_ptr_shm + channel->smc_history_data_sent[index].shm_offset,
                                channel->smc_history_data_received[index].data_ptr,
                                channel->smc_history_data_received[index].data_length,
                                channel->smc_history_data_received[index].userdata_flags,
                                channel->smc_history_data_received[index].userdata1,
                                channel->smc_history_data_received[index].userdata2,
                                channel->smc_history_data_received[index].userdata3,
                                channel->smc_history_data_received[index].userdata4,
                                channel->smc_history_data_received[index].userdata5);

                        found_items = TRUE;
                    }

                    if( index > 0 )
                    {
                        index--;
                    }
                    else
                    {
                        index = channel->smc_history_len_sent-1;
                    }

                    counter++;
                }

                if( !found_items )
                {
                    SMC_TRACE_PRINTF_ALWAYS("SMC:       - <No data received>");
                }

            }
            else
            {
                SMC_TRACE_PRINTF_ALWAYS("SMC:    - <Receive data history is not recorded>");
            }
#endif

        }
    }
    else
    {
        SMC_TRACE_PRINTF_ALWAYS("SMC:  - <No SMC Channels initialized>");
    }
}

void smc_mdb_info_dump( char* indent, struct _smc_mdb_channel_info_t* smc_mdb_info, int32_t mem_offset, uint8_t out_mdb )
{
    if( smc_mdb_info != NULL )
    {
        void*    pool = NULL;
        uint32_t size = 0;

        if( out_mdb==TRUE )
        {
            pool = smc_mdb_info->pool_out;
            size = smc_mdb_info->total_size_out;
        }
        else
        {
            pool = smc_mdb_info->pool_in;
            size = smc_mdb_info->total_size_in;
        }

        SMC_TRACE_PRINTF_ALWAYS("%s- MDB %s: size %d bytes, SHM offset 0x%08X", indent, (out_mdb==TRUE)?"OUT":"IN", size, (uint32_t)mem_offset);
        SMC_TRACE_PRINTF_ALWAYS("%s      Memory area : 0x%08X - 0x%08X (PHY-ADDR: 0x%08X - 0x%08X)", indent,
                    (uint32_t)(pool), (uint32_t)pool+size,
                    ((uint32_t)pool-mem_offset), ((uint32_t)pool+size-mem_offset));
    }
    else
    {
        SMC_TRACE_PRINTF_ALWAYS("%s- <MDB %s is not initialized>", indent, (out_mdb==TRUE)?"OUT":"IN");
    }
}

smc_loopback_data_t* smc_loopback_data_create( uint32_t size_of_message_payload )
{
    smc_loopback_data_t* data = NULL;

    data = (smc_loopback_data_t*)SMC_MALLOC( sizeof( smc_loopback_data_t ) + ((size_of_message_payload-1)*sizeof(uint8_t)) );

    data->round_trip_counter   = 0;
    data->loopback_data_length = size_of_message_payload;
    data->timestamp            = 0;
    data->loopback_rounds_left = 0;

    memset( data->loopback_data, 0, size_of_message_payload );

    return data;
}

uint8_t smc_send_loopback_data_message( smc_channel_t* smc_channel, uint32_t loopback_data_len, uint32_t loopback_rounds )
{
    smc_user_data_t userdata;
    uint8_t         ret_val = SMC_OK;
    uint32_t        message_len = 0;
    smc_loopback_data_t* loopback_data = NULL;

    SMC_TRACE_PRINTF_LOOPBACK("smc_send_loopback_data_message: channel %d:", smc_channel->id);

    loopback_data = smc_loopback_data_create( loopback_data_len );

    loopback_data->round_trip_counter   = 0;
    loopback_data->loopback_rounds_left = loopback_rounds;

    for(int i = 0; i < loopback_data->loopback_data_length; i++ )
    {
        loopback_data->loopback_data[i] = (uint8_t)(i&0xFF);
    }

    /* TODO Put the checksum in four last bytes */

    userdata.flags     = SMC_MSG_FLAG_LOOPBACK_DATA_REQ;
    userdata.userdata1 = 0x00000000;
    userdata.userdata2 = 0x00000000;
    userdata.userdata3 = 0x00000000;
    userdata.userdata4 = 0x00000000;
    userdata.userdata5 = 0x00000000;

    message_len = sizeof( smc_loopback_data_t ) + loopback_data->loopback_data_length - 1;

    RD_TRACE_SEND5(TRA_SMC_LOOPBACK_START, 1, &smc_channel->id,
                                           4, &loopback_data->loopback_data,
                                           4, &loopback_data->loopback_data_length,
                                           4, &loopback_data->round_trip_counter,
                                           4, &loopback_data->loopback_rounds_left);


    SMC_TRACE_PRINTF_LOOPBACK("smc_send_loopback_data_message: channel %d: send loopback data 0x%08X (len %d)", smc_channel->id, (uint32_t)loopback_data, message_len );
    SMC_TRACE_PRINTF_LOOPBACK("smc_send_loopback_data_message: channel %d: userdata: 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X", smc_channel->id,
            userdata.flags, userdata.userdata1, userdata.userdata2, userdata.userdata3, userdata.userdata4, userdata.userdata5);
    SMC_TRACE_PRINTF_LOOPBACK("smc_send_loopback_data_message: channel %d: loopback data: round trips = %d, lb rounds left %d, payload = 0x%08X, len %d", smc_channel->id,
            loopback_data->round_trip_counter, loopback_data->loopback_rounds_left, (uint32_t)loopback_data->loopback_data, loopback_data->loopback_data_length );

    SMC_TRACE_PRINTF_LOOPBACK_DATA(loopback_data->loopback_data_length, loopback_data->loopback_data);

    ret_val = smc_send_ext(smc_channel, (uint8_t*)loopback_data, message_len, &userdata);

    if( ret_val != SMC_OK )
    {
        SMC_TRACE_PRINTF_ERROR("smc_send_loopback_data_message: send failed");
    }

    SMC_TRACE_PRINTF_LOOPBACK("smc_send_loopback_data_message: channel %d completed", smc_channel->id);

    return ret_val;
}

static uint8_t smc_handle_loopback_data_message( smc_channel_t* smc_channel, smc_loopback_data_t* loopback_data, smc_user_data_t* userdata)
{
    uint8_t ret_val    = SMC_OK;
    uint8_t is_request = SMC_FIFO_IS_INTERNAL_MESSAGE_LOOPBACK_DATA_REQ( userdata->flags );
    uint32_t  lb_data_len = 0;

    SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback data 0x%08X", is_request?"Request":"Response", smc_channel->id, (uint32_t)loopback_data);

    if( loopback_data != NULL )
    {
        lb_data_len = sizeof( smc_loopback_data_t ) + loopback_data->loopback_data_length - 1;

        //SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback data found, payload length %d", is_request?"Request":"Response", smc_channel->id, loopback_data->loopback_data_length);
        SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: send loopback data 0x%08X (len %d)", is_request?"Request":"Response", smc_channel->id, (uint32_t)loopback_data, lb_data_len);
        SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: userdata: 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X", is_request?"Request":"Response", smc_channel->id,
                userdata->flags, userdata->userdata1, userdata->userdata2, userdata->userdata3, userdata->userdata4, userdata->userdata5);
        SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback data: round_trip_counter = %d, lb rounds left %d, payload = 0x%08X, len %d", is_request?"Request":"Response", smc_channel->id,
                loopback_data->round_trip_counter, loopback_data->loopback_rounds_left, (uint32_t)loopback_data->loopback_data, loopback_data->loopback_data_length );

        SMC_TRACE_PRINTF_LOOPBACK_DATA(loopback_data->loopback_data_length, loopback_data->loopback_data);
    }
    else
    {
        SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback is null", is_request?"Request":"Response", smc_channel->id);
        ret_val = SMC_ERROR;
    }

    if( is_request || ( loopback_data!=NULL && loopback_data->loopback_rounds_left > 0) )
    {
        /* Send the same data back, put loopback roundtrip information to the userdata */
        smc_user_data_t userdata_resp;
        smc_loopback_data_t* loopback_data_send = NULL;

        userdata_resp.userdata1 = userdata->userdata1;
        userdata_resp.userdata2 = userdata->userdata2;
        userdata_resp.userdata3 = userdata->userdata3;
        userdata_resp.userdata4 = ret_val;
        userdata_resp.userdata5 = userdata->userdata5;

        if( is_request )
        {
            userdata_resp.flags     = SMC_MSG_FLAG_LOOPBACK_DATA_RESP;
        }
        else
        {
            /* Update the round trip information when response */
            loopback_data->round_trip_counter++;
            loopback_data->loopback_rounds_left--;

                /* Send as a request */
            userdata_resp.flags     = SMC_MSG_FLAG_LOOPBACK_DATA_REQ;
        }

        if( loopback_data != NULL )
        {
            if( SMC_MDB_ADDRESS_IN_POOL_IN( loopback_data, smc_channel->smc_mdb_info ) )
            {
                smc_user_data_t userdata_free;

                SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback ptr 0x%08X, in MDB IN pool, create a copy", is_request?"Request":"Response", smc_channel->id, (uint32_t)loopback_data);

                loopback_data_send = smc_loopback_data_create( lb_data_len );

                loopback_data_send->round_trip_counter   = loopback_data->round_trip_counter;
                loopback_data_send->loopback_data_length = loopback_data->loopback_data_length;
                loopback_data_send->timestamp            = loopback_data->timestamp;
                loopback_data_send->loopback_rounds_left = loopback_data->loopback_rounds_left;

                for(int i = 0; i < loopback_data->loopback_data_length; i++ )
                {
                    loopback_data_send->loopback_data[i] = loopback_data->loopback_data[i];
                }

                /* The original pointer in sender's MDB must be freed */

                userdata_free.flags     = SMC_MSG_FLAG_FREE_MEM_MDB;
                userdata_free.userdata1 = userdata->userdata1;
                userdata_free.userdata2 = userdata->userdata2;
                userdata_free.userdata3 = userdata->userdata3;
                userdata_free.userdata4 = userdata->userdata4;
                userdata_free.userdata5 = userdata->userdata5;

                SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message: channel %d, free the original data 0x%08X from SHM...", smc_channel->id, (uint32_t)loopback_data);

                    /* Free the MDB SHM data PTR from remote */
                if( smc_send_ext( smc_channel, (void*)loopback_data, 0, &userdata_free) != SMC_OK )
                {
                    SMC_TRACE_PRINTF_ERROR("smc_handle_loopback_data_message: channel %d: MDB memory 0x%08X free from remote failed",
                            smc_channel->id, (uint32_t)loopback_data);
                }
            }
            else
            {
                loopback_data_send = loopback_data;
            }
        }

        SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: send loopback response data 0x%08X, data len %d", is_request?"Request":"Response", smc_channel->id, (uint32_t)loopback_data_send, lb_data_len);

        if( userdata_resp.flags == SMC_MSG_FLAG_LOOPBACK_DATA_RESP )
        {
            RD_TRACE_SEND5(TRA_SMC_LOOPBACK_SEND_RESP, 1, &smc_channel->id,
                                                       4, &loopback_data_send->loopback_data,
                                                       4, &loopback_data_send->loopback_data_length,
                                                       4, &loopback_data_send->round_trip_counter,
                                                       4, &loopback_data_send->loopback_rounds_left);
        }
        else
        {
            RD_TRACE_SEND5(TRA_SMC_LOOPBACK_SEND_REQ, 1, &smc_channel->id,
                                                      4, &loopback_data_send->loopback_data,
                                                      4, &loopback_data_send->loopback_data_length,
                                                      4, &loopback_data_send->round_trip_counter,
                                                      4, &loopback_data_send->loopback_rounds_left);
        }

        if( smc_send_ext( smc_channel, (uint8_t*)loopback_data_send, lb_data_len, &userdata_resp) != SMC_OK )
        {
            SMC_TRACE_PRINTF_WARNING("smc_handle_loopback_data_message: SMC_MSG_FLAG_LOOPBACK_DATA_RESP send failed");
        }
    }
    else
    {
        if( loopback_data != NULL )
        {
            SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback completed, result %s, loopback rounds %d", is_request?"Request":"Response", smc_channel->id, userdata->userdata4?"OK":"FAILED", loopback_data->round_trip_counter);
        }
        else
        {
            SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: loopback completed, result %s, loopback data was NULL (ERROR)", is_request?"Request":"Response", smc_channel->id, userdata->userdata4?"OK":"FAILED");
        }
    }

    SMC_TRACE_PRINTF_LOOPBACK("smc_handle_loopback_data_message (%s): channel %d: completed with return value 0x%02X", is_request?"Request":"Response", smc_channel->id, ret_val);

    return ret_val;
}


#ifdef SMC_PERFORM_VERSION_CHECK

/**
 * Checks that the local and remote SMC version are compliant.
 *
 */
uint8_t smc_version_check(uint32_t version_local, uint32_t version_remote, uint8_t local_is_master)
{
    uint8_t ret_val = SMC_OK;

    SMC_TRACE_PRINTF_DEBUG("smc_version_check: version_local=0x%08X, version_remote=0x%08X, is_master=%s",
            version_local, version_remote, local_is_master?"TRUE":"FALSE");

    if( version_local != version_remote )
    {
        /* Different versions detected checking compliance */

        for(int i = 0; i < SMC_VERSION_REQUIREMENT_COUNT; i++ )
        {
            smc_version_check_t version_req = g_smc_version_requirements[i];

            SMC_TRACE_PRINTF_DEBUG("smc_version_check: version requirement: 0x%08X, level 0x%02X (%s)",
                        version_req.version_requirement,
                        version_req.version_requirement_level,
                        version_req.version_requirement_reason );

            if( version_local >= version_req.version_requirement )
            {
                uint8_t version_req_match = FALSE;

                SMC_TRACE_PRINTF_DEBUG("smc_version_check: version requirement 0x%08X is valid", version_req.version_requirement);

                if( version_remote >= version_req.version_requirement )
                {
                    /* Remote meets the same version requirement */
                    /* Currently no other checkings done */
                    version_req_match = TRUE;
                }

                if( !version_req_match  )
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_version_check: version requirement 0x%08X does not match for remote", version_req.version_requirement);

                    if( version_req.version_requirement_level == SMC_VERSION_REQUIREMENT_LEVEL_WARNING )
                    {
                        /* Just a warning */
                        SMC_TRACE_PRINTF_WARNING("smc_version_check: remote CPU SMC version %d.%d.%d is not the required version %d.%d.%d (requirement: %s)",
                                SMC_VERSION_MAJOR(version_remote), SMC_VERSION_MINOR(version_remote), SMC_VERSION_REVISION(version_remote),
                                SMC_VERSION_MAJOR(version_req.version_requirement), SMC_VERSION_MINOR(version_req.version_requirement), SMC_VERSION_REVISION(version_req.version_requirement),
                                version_req.version_requirement_reason);
                    }
                    else if( version_req.version_requirement_level == SMC_VERSION_REQUIREMENT_LEVEL_ERROR )
                    {
                        SMC_TRACE_PRINTF_ERROR("smc_version_check: remote CPU SMC version %d.%d.%d is not the required version %d.%d.%d (requirement: %s)",
                                                        SMC_VERSION_MAJOR(version_remote), SMC_VERSION_MINOR(version_remote), SMC_VERSION_REVISION(version_remote),
                                                        SMC_VERSION_MAJOR(version_req.version_requirement), SMC_VERSION_MINOR(version_req.version_requirement), SMC_VERSION_REVISION(version_req.version_requirement),
                                                        version_req.version_requirement_reason);
                    }
                    else if( version_req.version_requirement_level == SMC_VERSION_REQUIREMENT_LEVEL_ASSERT )
                    {
                        /* Currently no assert */

                        SMC_TRACE_PRINTF_ASSERT("smc_version_check: *** FATAL ERROR *** remote CPU SMC version %d.%d.%d is not the required version %d.%d.%d (requirement: %s)",
                                                                                SMC_VERSION_MAJOR(version_remote), SMC_VERSION_MINOR(version_remote), SMC_VERSION_REVISION(version_remote),
                                                                                SMC_VERSION_MAJOR(version_req.version_requirement), SMC_VERSION_MINOR(version_req.version_requirement), SMC_VERSION_REVISION(version_req.version_requirement),
                                                                                version_req.version_requirement_reason);

                        /*assert(0);*/
                        break;
                    }

                    ret_val = SMC_ERROR;    /* Always return error if versions do not match */
                }
                else
                {
                    SMC_TRACE_PRINTF_DEBUG("smc_version_check: version requirement 0x%08X is ok in remote", version_req.version_requirement);

                    ret_val = SMC_OK;
                }
            }
        }
    }
    else
    {
        ret_val = SMC_OK;
    }

    return ret_val;
}

#endif /* #ifdef SMC_PERFORM_VERSION_CHECK */


/*
 * History data functions
 */
#ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED

smc_message_history_data_t* smc_history_data_array_create( uint16_t array_len )
{
    SMC_TRACE_PRINTF_HISTORY("smc_history_data_array_create: creating history array size of %d...", array_len);

    if( array_len > 0 )
    {
        int i = 0;
        smc_message_history_data_t* history_array = (smc_message_history_data_t*)SMC_MALLOC( sizeof( smc_message_history_data_t ) * array_len );

        for( i = 0; i < array_len; i++ )
        {
            history_array[i].channel_id        = 0;
            history_array[i].history_data_type = SMC_TRACE_HISTORY_DATA_TYPE_NONE;
            history_array[i].fill2             = 0;
            history_array[i].fill1             = 0;

            history_array[i].timestamp         = 0;
            history_array[i].shm_offset        = 0;
            history_array[i].data_ptr_shm      = 0;
            history_array[i].data_ptr          = 0;
            history_array[i].data_length       = 0;

            history_array[i].userdata_flags    = 0;
            history_array[i].userdata1         = 0;
            history_array[i].userdata2         = 0;
            history_array[i].userdata3         = 0;

            history_array[i].userdata4         = 0;
            history_array[i].userdata5         = 0;

        }

        SMC_TRACE_PRINTF_HISTORY("smc_history_data_array_create: history array size of %d created", array_len);

        return history_array;
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_history_data_array_create: invalid history array size %d", array_len);
        return NULL;
    }
}


#endif /* #ifdef SMC_HISTORY_DATA_COLLECTION_ENABLED */

/* EOF */

