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

    /*
     * Local static variables
     */
static smc_lock_t* g_local_lock_smc_instance           = NULL;
static smc_lock_t* g_local_lock_smc_channel            = NULL;
static smc_lock_t* g_local_lock_signal_handler         = NULL;
static smc_lock_t* g_local_lock_smc_channel_sync       = NULL;
static smc_lock_t* g_local_lock_smc_fifo_buffer_flush  = NULL;
static smc_lock_t* g_local_lock_smc_fifo_buffer        = NULL;

static inline smc_lock_t* get_local_lock_smc_instance(void)
{
    if( g_local_lock_smc_instance == NULL ) g_local_lock_smc_instance = smc_lock_create();
    return g_local_lock_smc_instance;
}

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
static uint8_t  smc_channel_configure_shm( smc_channel_t* smc_channel, smc_channel_conf_t* smc_channel_conf, smc_shm_config_t* smc_shm_conf, uint8_t is_master);
static uint8_t* smc_instance_calculate_next_free_shm_address( smc_t* smc_instance, uint32_t bytes_consumed);
static uint8_t  smc_channel_handle_sync( smc_channel_t* smc_channel, uint32_t sync_flag, uint32_t sync_msg );
static uint8_t  smc_channel_buffer_fifo_message(smc_channel_t* channel, void* data, uint32_t data_length, smc_user_data_t* userdata);
static uint8_t  smc_channel_buffer_fifo_flush( smc_channel_t* channel );


    /* TODO Each channel should have own timer
     * FIFO timer cb function */
static void     smc_fifo_timer_expired(uint32_t timer_data);

    /*
     * Local inline functions
     */
static inline void*    smc_allocate_local_ptr     ( const smc_channel_t* smc_channel, uint32_t size, smc_user_data_t* userdata );
static inline void     smc_free_local_ptr         ( const smc_channel_t* smc_channel, void* ptr, smc_user_data_t* userdata );
static inline uint32_t smc_local_address_translate( const smc_channel_t* smc_channel, uint32_t ptr, uint32_t flags );
static inline uint8_t  smc_handle_fifo_event      (smc_channel_t* channel, SMC_CHANNEL_EVENT fifo_event);

extern uint8_t  smc_module_initialize( smc_conf_t* smc_instance_conf );


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

    SMC_TRACE_PRINTF_DEBUG("smc_version_to_str: convert version 0x%08X to string...", version);

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

    SMC_TRACE_PRINTF_DEBUG("smc_version_to_str: version 0x%08X converted to '%s'...", version, version_str);

    return version_str;
}

uint32_t smc_version_to_int(char* version)
{
    uint32_t version_int        = 0x00000000;
    int      level              = 0;
    int      version_ind        = 0;
    int      str_len            = strlen( version );
    char     version_number[10];

    SMC_TRACE_PRINTF_DEBUG("smc_version_to_int: convert version '%s' to unsigned integer, len = %d..", version, str_len);

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

    SMC_TRACE_PRINTF_DEBUG("smc_version_to_int: version '%s' converted to 0x%08X", version, version_int);

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

    if( smc_channel->smc_receive_data_allocator_cb )
    {
        SMC_TRACE_PRINTF_INFO("smc_allocate_local_ptr: Channel %d (0x%08X) allocating ptr using callback 0x%08X", smc_channel->id, (uint32_t)smc_channel, (uint32_t)smc_channel->smc_receive_data_allocator_cb);

        ptr = smc_channel->smc_receive_data_allocator_cb( smc_channel, size, userdata );
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_allocate_local_ptr: Channel %d (0x%08X) allocating from OS", smc_channel->id, (uint32_t)smc_channel);
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

    /* First Check if in MDB, then use CB or OS */

    //if( smc_mdb_address_check( smc_channel, ptr, SMC_MDB_OUT) == SMC_OK )
    if( SMC_MDB_ADDRESS_IN_POOL_OUT( ptr, smc_channel->smc_mdb_info ))
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: channel %d (0x%08X): deallocating ptr 0x%08X from MDB OUT...", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr);

        SMC_LOCK_IRQ( ((smc_channel_t*)smc_channel)->lock_mdb );

        smc_mdb_free( smc_channel, ptr );

        SMC_UNLOCK_IRQ( ((smc_channel_t*)smc_channel)->lock_mdb );

        // TODO This is temporary implementation for L2MUX->GPDS mem free, make more robust check

        if( smc_channel->smc_send_data_deallocator_cb != NULL && userdata->userdata2 > 0)
        {
            SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: channel %d (0x%08X): deallocating ptr 0x%08X from using CB and userdata2 0x%08X...", smc_channel->id, (uint32_t)smc_channel, (uint32_t)ptr, userdata->userdata2);

            smc_channel->smc_send_data_deallocator_cb( smc_channel, ptr, userdata );
        }
    }
    else if( smc_channel->smc_send_data_deallocator_cb != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: Channel %d: deallocating ptr 0x%08X using deallocator callback 0x%08X...",
                        smc_channel->id, (uint32_t)ptr, (uint32_t)smc_channel->smc_send_data_deallocator_cb);

        smc_channel->smc_send_data_deallocator_cb( smc_channel, ptr, userdata );
    }
    else
    {
        SMC_TRACE_PRINTF_DEBUG("smc_free_local_ptr: Channel %d: deallocating ptr 0x%08X using OS...", smc_channel->id, (uint32_t)ptr );

        SMC_FREE( ptr );
        ptr = NULL;
    }
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
                //if( smc_mdb_address_check( smc_channel, (void*)ptr, SMC_MDB_OUT) == SMC_OK )
                if( SMC_MDB_ADDRESS_IN_POOL_OUT( ptr, smc_mdb_pool_info ))
                {
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address to offset translation required for address 0x%08X (remove MDB OUT offset 0x%08X)",
                            smc_channel->id, ptr, smc_mdb_pool_info->pool_out);
                    new_ptr = ptr - (uint32_t)smc_mdb_pool_info->pool_out;
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address to offset translation performed, offset 0x%08X", smc_channel->id, new_ptr);
                }
                //else if(smc_mdb_address_check( smc_channel, (void*)(ptr + smc_channel->mdb_in), SMC_MDB_IN) == SMC_OK)
                else if( SMC_MDB_ADDRESS_IN_POOL_IN( (ptr + smc_channel->mdb_in), smc_mdb_pool_info ))
                {
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM offset to address translation required for offset 0x%08X (add MDB IN offset 0x%08X)",
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
                //if(smc_mdb_address_check(smc_channel, (void*)ptr, SMC_MDB_IN) == SMC_OK)
                if( SMC_MDB_ADDRESS_IN_POOL_IN( ptr, smc_channel->smc_mdb_info ))
                {
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address to offset translation required for address 0x%08X (FREE from MDB IN)", smc_channel->id, ptr);
                    new_ptr = ptr - (uint32_t)smc_channel->mdb_in;
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM address to offset translation performed, new address from 0x%08X is 0x%08X", smc_channel->id, ptr, new_ptr);
                }
                //else if(smc_mdb_address_check(smc_channel, (void*)(ptr + smc_channel->mdb_out), SMC_MDB_OUT) == SMC_OK)
                else if( SMC_MDB_ADDRESS_IN_POOL_OUT( (ptr + smc_channel->mdb_out), smc_channel->smc_mdb_info ))
                {
                    SMC_TRACE_PRINTF_INFO("smc_local_address_translate: channel %d: SHM offset to address translation required for offset 0x%08X (FREE from MDB OUT)", smc_channel->id, ptr);
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
    uint8_t           ret_val       = SMC_OK;
    uint8_t           forward_event = FALSE;
    SMC_CHANNEL_EVENT new_event     = fifo_event;

    SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d: FIFO event 0x%02X", channel->id, fifo_event);

    SMC_LOCK_IRQ( channel->lock_read );

    if( fifo_event == SMC_SEND_FIFO_FULL )
    {
#ifdef SMECO_MODEM
        /* TODO Modem side timer implementation */
        SMC_TRACE_PRINTF_ASSERT("Channel %d: SMC FIFO 0x%08X full: Remote host does not read messages, remote CPU probably halted",
            channel->id, (uint32_t)channel->fifo_out);
        assert(0);
#endif
        if( !SMC_CHANNEL_STATE_IS_FIFO_FULL( channel->state ) )
        {
            SMC_CHANNEL_STATE_SET_FIFO_FULL( channel->state );
            new_event = SMC_STOP_SEND;
            forward_event = TRUE;

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
            /* If there is buffered items push them to the FIFO first */
            ret_val = smc_channel_buffer_fifo_flush( channel );

            if( ret_val == SMC_OK )
            {
                SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d: FIFO buffer successfully flushed", channel->id);

                if( smc_fifo_peek( channel->fifo_out, channel->smc_shm_conf_channel->use_cache_control ) > 0 )
                {
                    SMC_CHANNEL_STATE_CLEAR_FIFO_FULL( channel->state );
                    new_event = SMC_RESUME_SEND;
                    forward_event = TRUE;
                }
            }
            else
            {
                /* Continue the FIFO full state */
                ret_val = SMC_OK;
            }
        }
    }

    SMC_UNLOCK_IRQ( channel->lock_read );

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

    SMC_TRACE_PRINTF_DEBUG("smc_handle_fifo_event: channel %d: completed by return value 0x%02X", channel->id, ret_val);

    return ret_val;
}

static inline uint8_t smc_handle_mdb_memory_event(smc_channel_t* channel, SMC_CHANNEL_EVENT mem_event)
{
    uint8_t           ret_val       = SMC_OK;
    uint8_t           forward_event = FALSE;
    SMC_CHANNEL_EVENT new_event     = mem_event;

    SMC_TRACE_PRINTF_DEBUG("smc_handle_mdb_memory_event: channel %d: mem event 0x%02X", channel->id, mem_event);

    SMC_LOCK_IRQ( channel->lock_write );

    if( mem_event == SMC_SEND_MDB_OUT_OF_MEM )
    {
        if( !SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( channel->state ) )
        {
            SMC_CHANNEL_STATE_SET_MDB_OUT_OF_MEM( channel->state );
            new_event = SMC_STOP_SEND;
            forward_event = TRUE;
        }
    }
    else if( mem_event == SMC_SEND_MDB_HAS_FREE_MEM )
    {
        if( SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( channel->state ) )
        {
            SMC_CHANNEL_STATE_CLEAR_MDB_OUT_OF_MEM( channel->state );
            new_event = SMC_RESUME_SEND;
            forward_event = TRUE;
        }
    }

    SMC_UNLOCK_IRQ( channel->lock_write );

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
                    //uint8_t result = smc_mdb_address_check(channel, data, SMC_MDB_OUT);
                    //if (result == SMC_OK)
                    if( SMC_MDB_ADDRESS_IN_POOL_OUT( data, channel->smc_mdb_info ))
                    {
                        SMC_TRACE_PRINTF_INFO("smc_send_ext: channel %d: MDB copy NOT required, data 0x%08X in MDB OUT area",
                                channel->id, (uint32_t)data);

                        mdb_ptr = data;
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
                            smc_mdb_copy(mdb_ptr, data, data_length);

                            if( channel->smc_shm_conf_channel->use_cache_control )
                            {
                                SMC_SHM_CACHE_CLEAN( mdb_ptr, ((void*)(((uint32_t)mdb_ptr)+data_length)) );
                            }
                            else
                            {
                                SMC_TRACE_PRINTF_INFO("smc_send_ext: Channel %d: No cache control required", channel->id);
                            }

                            SMC_TRACE_PRINTF_INFO("smc_send_ext: Channel %d: MDB copy performed from address 0x%08X to 0x%08X", channel->id, (uint32_t)data, (uint32_t)mdb_ptr);
                            SMC_TRACE_PRINTF_INFO_DATA(data_length, mdb_ptr);

                            smc_free_local_ptr( channel, data, userdata );
                        }
                        else
                        {
                            SMC_TRACE_PRINTF_ERROR("smc_send_ext: Channel %d: MDB allocation failed, out of SHM memory", channel->id);

                                /* Buffer the original pointer */
                            return_value = smc_channel_buffer_fifo_message(channel, data, data_length, userdata);

                            if( return_value != SMC_OK )
                            {
                                SMC_TRACE_PRINTF_ERROR("smc_send_ext: Channel %d MDB out of memory and unable to buffer the message", channel->id);
                            }

                            return_value = smc_handle_mdb_memory_event(channel, SMC_SEND_MDB_OUT_OF_MEM);
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
                }
                else
                {
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
                SMC_TRACE_PRINTF_ERROR("smc_send_ext: Channel %d: MDB ptr is NULL", channel->id);
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

    SMC_TRACE_PRINTF_DEBUG("smc_send_event: Channel %d: Sending event %d to remote host...", channel->id, event);

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

    SMC_TRACE_PRINTF_DEBUG("smc_send_event_ext: Channel %d: Sending event %d to remote host...", channel->id, event);

    return smc_send_ext( channel, (void*)event, 0, &userdata_event);
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
            // HS_TEST while( fifo_count >= SMC_FIFO_READ_TO_EMPTY )
            do
            {
                smc_fifo_cell_t celldata;
                smc_user_data_t userdata;

                /* ========================================
                 * Critical section begins
                 *
                 */
                SMC_LOCK_IRQ( smc_channel->lock_read );

                fifo_count = smc_fifo_get_cell( smc_channel->fifo_in, &celldata, smc_channel->smc_shm_conf_channel->use_cache_control );

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
                    SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: channel %d (0x%08X) Read data 0x%08X, length %d, flags 0x%08X, FIFO count %d",
                            smc_channel->id, (uint32_t)smc_channel, celldata.data, celldata.length, celldata.flags, fifo_count);

                    RD_TRACE_SEND4(TRA_SMC_MESSAGE_RECV, 1, &smc_channel->id,
                                                         4, &celldata.data,
                                                         4, &celldata.length,
                                                         4, &celldata.flags);

                        /* Check if internal message */
                    if( SMC_FIFO_IS_INTERNAL_MESSAGE( celldata.flags ) )
                    {
                        if( SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_REQ");
                            smc_channel_handle_sync( smc_channel, SMC_MSG_FLAG_SYNC_INFO_REQ, celldata.data );
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP( celldata.flags ) )
                        {
                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_SYNC_RESP");
                            smc_channel_handle_sync( smc_channel, SMC_MSG_FLAG_SYNC_INFO_RESP, celldata.data );
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB( celldata.flags ) )
                        {
                            void* data = NULL;

                            SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB");

                                /* Do the address translation because the remote send the data ptr to free as it gets it */
                            data = (void*)smc_local_address_translate( smc_channel, celldata.data, celldata.flags );

                            SMC_TRACE_PRINTF_DEBUG("smc_channel_interrupt_handler(ch %d): SMC_FIFO_IS_INTERNAL_MESSAGE_FREE_MEM_MDB 0x%08X",
                            smc_channel->id, (uint32_t)data);

                            smc_free_local_ptr( smc_channel, data, &userdata );

                            if( SMC_CHANNEL_STATE_IS_MDB_OUT_OF_MEM( smc_channel->state ) )
                            {
                                smc_handle_mdb_memory_event(smc_channel, SMC_SEND_MDB_HAS_FREE_MEM);
                            }
                        }
                        else if( SMC_FIFO_IS_INTERNAL_MESSAGE_CHANNEL_EVENT_USER( celldata.flags ) )
                        {
                            RD_TRACE_SEND2(TRA_SMC_EVENT_RECV, 1, &smc_channel->id,
                                                               4, &celldata.data);

                            SMC_TRACE_PRINTF_EVENT_RECEIVED("smc_channel_interrupt_handler: SMC_FIFO_IS_INTERNAL_MESSAGE_CHANNEL_EVENT_USER: 0x%08X", (uint32_t)celldata.data);

                            if( smc_channel->smc_event_cb )
                            {
                                smc_channel->smc_event_cb( smc_channel, (SMC_CHANNEL_EVENT)celldata.data, NULL );
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
                            userdata_version.userdata1 = smc_version_to_int(smc_get_version());;
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
                        else
                        {
                            SMC_TRACE_PRINTF_WARNING("smc_channel_interrupt_handler: channel %d: unsupported internal msg flag used 0x%08X", smc_channel->id, celldata.flags);
                        }
                    }
                    else
                    {
                        /* Not sn internal message */

                        void* received_data_ptr = NULL;

                        void* data = (void*)smc_local_address_translate( smc_channel, celldata.data, celldata.flags );

                        SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: Translated address is 0x%08X", data);
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

                                    smc_mdb_copy( received_data_ptr, data, celldata.length );

                                    if( smc_channel->smc_shm_conf_channel->use_cache_control )
                                    {
                                            /* Clean the new data from cache */
                                        SMC_SHM_CACHE_CLEAN( received_data_ptr, ((void*)(((uint32_t)received_data_ptr) + celldata.length)) );
                                    }

                                    // TODO CHECK LOCK
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
                                SMC_TRACE_PRINTF_INFO("smc_channel_interrupt_handler: channel %d (0x%08X) copy scheme is not copy in receive", smc_channel->id, (uint32_t)smc_channel);

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

                            /* Deliver data to callback */
                        smc_channel->smc_receive_cb( received_data_ptr, celldata.length, &userdata, smc_channel);


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
            }   /* while( fifo_count >= IPC2_FIFO_READ_TO_EMPTY ) */
            while( fifo_count > SMC_FIFO_READ_TO_EMPTY );
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

    SMC_TRACE_PRINTF_DEBUG("smc_instance_create: created SMC instance 0x%08X based on configuration in 0x%08X",
            (uint32_t)smc, (uint32_t)smc_instance_conf);

    smc->cpu_id_remote          = smc_instance_conf->cpu_id_remote;
    smc->cpu_id_local           = smc_instance_conf->cpu_id_local;
    smc->is_master              = smc_instance_conf->is_master;
    smc->smc_channel_list_count = 0;
    smc->smc_channel_ptr_array  = NULL;
    smc->smc_instance_conf      = smc_instance_conf;
    smc->smc_parent_ptr         = parent_object;

    if( smc_instance_conf->smc_shm_conf != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_instance_create: SHM configuration found, copying to instance...");
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

                SMC_TRACE_PRINTF_DEBUG("smc_instance_create: Remapped SHM start address from 0x%08X is 0x%08X, size is %d, offset to physical is %d (0x%08X)",
                        (uint32_t)smc_instance_conf->smc_shm_conf->shm_area_start_address , (uint32_t)smc->smc_shm_conf->shm_area_start_address,
                        smc->smc_shm_conf->size, (int32_t)smc->smc_shm_conf->remote_cpu_memory_offset, (uint32_t)smc->smc_shm_conf->remote_cpu_memory_offset);

                SMC_TRACE_PRINTF_STARTUP("Shared memory size %d bytes, memory area: 0x%08X - 0x%08X (physical: 0x%08X - 0x%08X)", smc->smc_shm_conf->size,
                                        (uint32_t)smc->smc_shm_conf->shm_area_start_address, ((uint32_t)smc->smc_shm_conf->shm_area_start_address + smc->smc_shm_conf->size),
                                        ((uint32_t)smc->smc_shm_conf->shm_area_start_address-smc->smc_shm_conf->remote_cpu_memory_offset),
                                        (((uint32_t)smc->smc_shm_conf->shm_area_start_address + smc->smc_shm_conf->size)-smc->smc_shm_conf->remote_cpu_memory_offset) );
            }
            else
            {
                SMC_TRACE_PRINTF_WARNING("smc_instance_create: SHM start address is NULL, SHM not available");
            }

            SMC_TRACE_PRINTF_DEBUG("smc_instance_create: SHM configuration copied, modify first free address to 0x%08X...", (uint32_t)smc->smc_shm_conf->shm_area_start_address);
            smc->smc_memory_first_free = smc->smc_shm_conf->shm_area_start_address;
        }
        else
        {
            SMC_TRACE_PRINTF_WARNING("smc_instance_create: SHM configuration copy failed");
            smc->smc_memory_first_free = NULL;
        }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_instance_create: SHM configuration not set");
        smc->smc_shm_conf = NULL;
        smc->smc_memory_first_free = NULL;
    }

        /* Create the SMC channels */
    if( smc_instance_conf->smc_channel_conf_count > 0 )
    {
        int i = 0;

        SMC_TRACE_PRINTF_DEBUG("smc_instance_create: found %d channel configurations, initializing SMC channels...", smc_instance_conf->smc_channel_conf_count);

        for(i = 0; i < smc_instance_conf->smc_channel_conf_count; i++ )
        {
            smc_channel_t* channel = smc_channel_create( smc_instance_conf->smc_channel_conf_ptr_array[i] );

            SMC_TRACE_PRINTF_INFO("smc_instance_create: new SMC channel in index %d created (0x%08X), add to channel array...",
                        i, (uint32_t)channel);

            smc_add_channel(smc, channel, smc_instance_conf->smc_channel_conf_ptr_array[i]);

        }

        SMC_TRACE_PRINTF_DEBUG("smc_instance_create: channels successfully configured");
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_instance_create: no channels found from configuration");
    }

    SMC_TRACE_PRINTF_DEBUG("smc_instance_create: new SMC instance 0x%08X created", (uint32_t)smc);

#ifdef SMC_TRACE_INFO_ENABLED

    smc_instance_dump(smc);

#endif

    return smc;
}

void smc_instance_destroy( smc_t* smc_instance )
{
    if( smc_instance )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_instance_destroy: destroying channels (%d)...", smc_instance->smc_channel_list_count);

        for(int i = 0; i < smc_instance->smc_channel_list_count; i++)
        {
            smc_channel_destroy( smc_instance->smc_channel_ptr_array[i] );
        }

        SMC_FREE( smc_instance->smc_channel_ptr_array );
        smc_instance->smc_channel_ptr_array = NULL;

        /* TODO Check if destroy SHM */

        SMC_FREE( smc_instance );
        smc_instance = NULL;

        SMC_TRACE_PRINTF_DEBUG("smc_instance_destroy: completed");
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
        SMC_TRACE_PRINTF_WARNING("smc_channel_create: timeout for FIFO full check not set");
    }

        /*
         * MDBs are created when channel is added to SMC instance and SHM is ready
         */
    channel->mdb_out  = NULL;
    channel->mdb_in   = NULL;

        /* Initialize locks */
    channel->lock_write = smc_lock_create();
    channel->lock_read  = smc_lock_create();
    channel->lock_mdb   = smc_lock_create();

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

    SMC_TRACE_PRINTF_DEBUG("smc_channel_create: 0x%08X created", (uint32_t)channel);

    return channel;
}

static uint8_t smc_channel_configure_shm( smc_channel_t* smc_channel, smc_channel_conf_t* smc_channel_conf, smc_shm_config_t* smc_shm_conf, uint8_t is_master)
{
    uint8_t ret_val = SMC_ERROR;
    uint32_t bytes_consumed = 0;

    assert( smc_channel != NULL );
    assert( smc_shm_conf != NULL );

    if( smc_shm_conf->shm_area_start_address != NULL )
    {
        assert( smc_shm_conf->size > 0 );

        smc_channel->smc_shm_conf_channel = smc_shm_conf;

        SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: SHM configuration found, start address 0x%08X, bytes available %d", smc_channel->id, (uint32_t)smc_channel->smc_shm_conf_channel->shm_area_start_address,
                                                                                                            smc_channel->smc_shm_conf_channel->size);
        SMC_TRACE_PRINTF_INFO("smc_channel_configure_shm: Channel %d: Creating FIFOs...", smc_channel->id);

        ret_val = SMC_OK;   /* Change if something goes wrong */

        for(int i = 0; i < 2; i++)
        {
            if( (i == 0 && is_master) || (i==1 && !is_master))
            {
                SMC_TRACE_PRINTF_INFO("smc_channel_configure_shm: Channel %d: Initializing FIFO OUT size %d...", smc_channel->id, smc_channel_conf->fifo_size_out);

                smc_channel->fifo_out = (smc_fifo_t*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);

                smc_fifo_init_out( smc_channel->fifo_out, smc_channel_conf->fifo_size_out, smc_shm_conf->use_cache_control);

                bytes_consumed += smc_fifo_calculate_required_shared_mem(smc_channel_conf->fifo_size_out);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: FIFO OUT 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->fifo_out, bytes_consumed);
            }
            else
            {
                SMC_TRACE_PRINTF_INFO("smc_channel_configure_shm: Channel %d: Initializing FIFO IN size %d...", smc_channel->id, smc_channel_conf->fifo_size_in);

                smc_channel->fifo_in = (smc_fifo_t*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);

                smc_fifo_init_in( smc_channel->fifo_in, smc_channel_conf->fifo_size_in, smc_shm_conf->use_cache_control );

                bytes_consumed += smc_fifo_calculate_required_shared_mem(smc_channel_conf->fifo_size_in);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: FIFO IN 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->fifo_in, bytes_consumed);
            }
        }

        smc_channel->smc_mdb_info = smc_mdb_channel_info_create();

        SMC_TRACE_PRINTF_INFO("smc_channel_configure_shm: Channel %d: Creating MDBs...", smc_channel->id);

        for(int i = 0; i < 2; i++)
        {
            if( (i == 0 && is_master) || (i==1 && !is_master))
            {
                SMC_TRACE_PRINTF_INFO("smc_channel_configure_shm: Channel %d: Initializing MDB OUT size %d...", smc_channel->id, smc_channel_conf->mdb_size_out);

                /* TODO Remove MDB out pointer from channel */

                smc_channel->mdb_out                      = (uint8_t*)smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed;
                smc_channel->smc_mdb_info->pool_out       = (void*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);
                smc_channel->smc_mdb_info->total_size_out = smc_channel_conf->mdb_size_out;

                if( smc_mdb_create_pool_out( smc_channel->smc_mdb_info->pool_out, smc_channel->smc_mdb_info->total_size_out) != SMC_OK )
                {
                    SMC_TRACE_PRINTF_ASSERT("smc_channel_configure_shm: MDB pool out create failed");
                    assert(0);
                }

                bytes_consumed += smc_mdb_calculate_required_shared_mem(smc_channel_conf->mdb_size_out);

                SMC_TRACE_PRINTF_DEBUG("smc_channel_configure_shm: Channel %d: MDB OUT 0x%08X, bytes consumed %d", smc_channel->id, (uint32_t)smc_channel->mdb_out, bytes_consumed);
            }
            else
            {
                SMC_TRACE_PRINTF_INFO("smc_channel_configure_shm: Channel %d: Initializing MDB IN size %d...", smc_channel->id, smc_channel_conf->mdb_size_in);

                /* TODO Remove MDB in pointer from channel */
                smc_channel->mdb_in                      = (uint8_t*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);
                smc_channel->smc_mdb_info->pool_in       = (void*)(smc_channel->smc_shm_conf_channel->shm_area_start_address + bytes_consumed);
                smc_channel->smc_mdb_info->total_size_in = smc_channel_conf->mdb_size_in;

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
        SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X...", (uint32_t)smc_channel);

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

        if( smc_channel->signal_remote != NULL )
        {
            smc_signal_destroy(smc_channel->signal_remote);
        }

        if( smc_channel->signal_local != NULL )
        {
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

        SMC_FREE( smc_channel );

        smc_channel = NULL;

        SMC_TRACE_PRINTF_DEBUG("smc_channel_destroy: channel 0x%08X completed", (uint32_t)smc_channel);
    }
}

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

                /* Configure the channel */
            if( smc_channel_configure_shm( smc_channel, smc_channel_conf, channel_shm, smc_instance->is_master ) == SMC_OK )
            {
                    /* Update the SMC instance SHM free address */
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
            SMC_TRACE_PRINTF_ASSERT("smc_add_channel: Not enough memory for SMC channel SHM: %d, free %d", channel_shm->size, smc_instance_get_free_shm(smc_instance) );
            assert(0);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_add_channel: SMC instance have no valid SHM for channel 0x%08X", (uint32_t)smc_channel);
    }

        /*
         * Before sync initialize receiver interrupt because the SMC channel hierachy is ready
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
    SMC_TRACE_PRINTF_INFO("smc_signal_get: search signal handler for signal id %d, type 0x%08X", signal_id, signal_type);

    for(int i = 0; i < signal_handler_count; i++ )
    {
        if( signal_handler_ptr_array[i]->signal->interrupt_id == signal_id && signal_handler_ptr_array[i]->signal->signal_type == signal_type )
        {
            SMC_TRACE_PRINTF_INFO("smc_signal_get: return signal handler 0x%08X", (uint32_t)signal_handler_ptr_array[i]);
            return signal_handler_ptr_array[i];
        }
    }

    SMC_TRACE_PRINTF_ASSERT("smc_signal_handler_get: Handler for signal %d type %0x08X not set", signal_id, signal_type);

    return NULL;
}

smc_signal_handler_t* smc_signal_handler_create_and_add( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel )
{
    smc_signal_handler_t* signal_handler = NULL;

    SMC_TRACE_PRINTF_INFO("smc_signal_handler_create_and_add(ch: %d): starts...", (smc_channel!=NULL)?smc_channel->id:-1);

    signal_handler = (smc_signal_handler_t*)SMC_MALLOC( sizeof( smc_signal_handler_t ) );

    assert( signal_handler != NULL );

    signal_handler->signal       = signal;
    signal_handler->smc_instance = smc_instance;
    signal_handler->smc_channel  = smc_channel;

    smc_signal_add_handler( signal_handler );

    SMC_TRACE_PRINTF_INFO("smc_signal_handler_create_and_add(ch: %d): created 0x%08X",
       (smc_channel!=NULL)?smc_channel->id:-1, (uint32_t)signal_handler);

    return signal_handler;
}

uint8_t smc_signal_add_handler( smc_signal_handler_t* signal_handler )
{
    smc_signal_handler_t** old_ptr_array = NULL;

    smc_lock_t* local_lock = get_local_lock_signal_handler();
    SMC_LOCK_IRQ( local_lock );

    SMC_TRACE_PRINTF_INFO("smc_signal_add_handler: add handler 0x%08X, current count %d", (uint32_t)signal_handler, signal_handler_count);

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

    SMC_TRACE_PRINTF_INFO("smc_signal_add_handler: completed, signal handler count is %d", signal_handler_count);

    SMC_UNLOCK_IRQ( local_lock );

    return SMC_OK;
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

            SMC_TRACE_PRINTF_INFO("smc_channel_handle_sync(ch %d, 0x%08X): <==== Sending SYNC REQ...", smc_channel->id,(uint32_t)smc_channel);

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
                SMC_TRACE_PRINTF_INFO("smc_channel_handle_sync(ch %d, 0x%08X): ===> Received REQ flag with correct message 0x%08X, sending response...", smc_channel->id,(uint32_t)smc_channel, sync_msg);
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
                SMC_TRACE_PRINTF_INFO("smc_channel_handle_sync(ch %d, 0x%08X): ====> Received RESP without REQ, setting state synchronized anyway", smc_channel->id,(uint32_t)smc_channel);
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
                /* Allocate the whole buffer here, buffer is freed when flushed */
            SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_message: allocate buffer for %d items...", SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX);
            channel->fifo_buffer = SMC_MALLOC( SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX * sizeof( smc_fifo_cell_t ) );

            assert( channel->fifo_buffer != NULL );

            channel->fifo_buffer_item_count = 0;
        }

        if( channel->fifo_buffer_item_count < SMC_CHANNEL_FIFO_BUFFER_SIZE_MAX )
        {
            channel->fifo_buffer[channel->fifo_buffer_item_count].data      = (uint32_t)data;
            channel->fifo_buffer[channel->fifo_buffer_item_count].length    = data_length;
            channel->fifo_buffer[channel->fifo_buffer_item_count].flags     = userdata->flags;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata1 = userdata->userdata1;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata2 = userdata->userdata2;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata3 = userdata->userdata3;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata4 = userdata->userdata4;
            channel->fifo_buffer[channel->fifo_buffer_item_count].userdata5 = userdata->userdata5;

            channel->fifo_buffer_item_count++;

            SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_message: FIFO buffer has now %d items", channel->fifo_buffer_item_count);

            ret_value = SMC_OK;
        }
        else
        {
            SMC_TRACE_PRINTF_WARNING("smc_channel_buffer_fifo_message: FIFO buffer is full");
            ret_value = SMC_ERROR_BUFFER_FULL;
        }

        /*smc_unlock_irq( local_lock );*/
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
    uint8_t ret_val = SMC_OK;

    smc_lock_t* local_lock = get_local_lock_smc_fifo_buffer_flush();

    SMC_LOCK( local_lock );

    SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_flush: channel 0x%08X...", (uint32_t)channel);

    if( channel->fifo_buffer != NULL && channel->fifo_buffer_item_count > 0 )
    {
        uint8_t flush_success = SMC_OK;

        SMC_TRACE_PRINTF_DEBUG("smc_channel_buffer_fifo_flush: channel 0x%08X, flushing %d items from the buffer...", (uint32_t)channel, channel->fifo_buffer_item_count);

        for(int i = 0; i < channel->fifo_buffer_item_count; i++ )
        {
            smc_fifo_cell_t fifo_cell = channel->fifo_buffer[i];

            if( fifo_cell.data != 0 || fifo_cell.length > 0 || fifo_cell.flags != 0 )
            {
                smc_user_data_t userdata;

                userdata.flags     = fifo_cell.flags;
                userdata.userdata1 = fifo_cell.userdata1;
                userdata.userdata2 = fifo_cell.userdata2;
                userdata.userdata3 = fifo_cell.userdata3;
                userdata.userdata4 = fifo_cell.userdata4;
                userdata.userdata5 = fifo_cell.userdata5;

                if( smc_send_ext( channel, (void*)fifo_cell.data, fifo_cell.length, &userdata ) == SMC_OK )
                {
                    /* "Mark" data as sent */
                    fifo_cell.data   = 0;
                    fifo_cell.length = 0;
                    fifo_cell.flags  = 0;
                }
                else
                {
                    SMC_TRACE_PRINTF_WARNING("smc_channel_buffer_fifo_flush: smc send failed");
                    flush_success = SMC_ERROR;
                    break;
                }
            }
            else
            {
                SMC_TRACE_PRINTF_DEBUG("smc_channel_buffer_fifo_flush: item was already sent");
            }
        }

        if( flush_success == SMC_OK )
        {
            SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_flush: channel 0x%08X buffer successfully flushed", (uint32_t)channel);

            channel->fifo_buffer_item_count = 0;

            SMC_FREE( channel->fifo_buffer );
            channel->fifo_buffer = NULL;
            ret_val = SMC_OK;
        }
    }
    else
    {
        SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_flush: channel 0x%08X: FIFO buffer is empty", (uint32_t)channel);
    }

    SMC_TRACE_PRINTF_INFO("smc_channel_buffer_fifo_flush: channel 0x%08X completed by return value 0x%02X", (uint32_t)channel, ret_val);

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

    SMC_TRACE_PRINTF("SMC: ");
    SMC_TRACE_PRINTF("SMC: Instance: 0x%08X (%s) CPU ID 0x%02X, Remote CPU ID: %d, channel count %d", (uint32_t)smc_instance,
                                                                             smc_instance->is_master?"Master":"Slave",
                                                                             smc_instance->cpu_id_local,
                                                                             smc_instance->cpu_id_remote,
                                                                             smc_instance->smc_channel_list_count);

    if( smc_instance->smc_shm_conf )
    {
        SMC_TRACE_PRINTF("SMC:  - SHM starts from address 0x%08X, size %d, bytes used %d, bytes left %d, Cache control (%s)",
                (uint32_t)smc_instance->smc_shm_conf->shm_area_start_address,
                smc_instance->smc_shm_conf->size,
                (smc_instance->smc_shm_conf->size-smc_instance_get_free_shm(smc_instance)),
                smc_instance_get_free_shm(smc_instance),
                smc_instance->smc_shm_conf->use_cache_control?"ENABLED":"DISABLED");
    }
    else
    {
        SMC_TRACE_PRINTF("SMC:  - <SHM not initialized>");
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

            SMC_TRACE_PRINTF("SMC:  ");
            SMC_TRACE_PRINTF("SMC:  - CH %d: priority: 0x%02X, protocol: 0x%02X, %s (0x%08X), Locks W: 0x%08X, R: 0x%08X, MDB: 0x%08X",
                                                                             channel->id,
                                                                             channel->priority,
                                                                             channel->protocol,
                                                                             SMC_CHANNEL_STATE_IS_SYNCHRONIZED(channel->state)?"IN SYNC":"NOT SYNC",
                                                                             channel->state,
                                                                             (uint32_t)channel->lock_write,
                                                                             (uint32_t)channel->lock_read,
                                                                             (uint32_t)channel->lock_mdb);

            /* Dump the FIFO SIGNAL and MDB data */

            if( channel->fifo_out != NULL )
            {
                SMC_TRACE_PRINTF("SMC:    - FIFO OUT:");
                  smc_fifo_dump( "SMC:        ", channel->fifo_out, mem_offset );
            }
            else
            {
                SMC_TRACE_PRINTF("SMC:    - <FIFO OUT is not initialized>");
            }

            if( channel->signal_remote != NULL )
            {
                SMC_TRACE_PRINTF("SMC:    - Signal OUT 0x%08X", (uint32_t)channel->signal_remote);
            }
            else
            {
                SMC_TRACE_PRINTF("SMC:    - <Signal OUT is not initialized>");
            }


            if( channel->smc_mdb_info != NULL )
            {
                //SMC_TRACE_PRINTF("SMC:    - MDB  OUT: 0x%08X", (uint32_t)channel->mdb_out );

                smc_mdb_info_dump( "SMC:    ", channel->smc_mdb_info, mem_offset, TRUE);
            }
            else
            {
                SMC_TRACE_PRINTF("SMC:    - <MDB is not initialized>");
            }

            SMC_TRACE_PRINTF("SMC:  ");

            if( channel->fifo_in != NULL )
            {
                SMC_TRACE_PRINTF("SMC:    - FIFO IN:");
                  smc_fifo_dump( "SMC:        ", channel->fifo_in, mem_offset );
            }
            else
            {
                SMC_TRACE_PRINTF("SMC:    - <FIFO IN  is not initialized>");
            }

            if( channel->signal_local != NULL )
            {
                SMC_TRACE_PRINTF("SMC:    - Signal IN 0x%08X", (uint32_t)channel->signal_local);
            }
            else
            {
                SMC_TRACE_PRINTF("SMC:    - <Signal IN is not initialized>");
            }

            if( channel->smc_mdb_info != NULL )
            {
                //SMC_TRACE_PRINTF("SMC:    - MDB  IN:  0x%08X", (uint32_t)channel->mdb_in );
                smc_mdb_info_dump( "SMC:    ", channel->smc_mdb_info, mem_offset, FALSE);
            }
            else
            {
                SMC_TRACE_PRINTF("SMC:    - <MDB is not initialized>");
            }
        }
    }
    else
    {
        SMC_TRACE_PRINTF("SMC:  - <No SMC Channels initialized>");
    }
}


void smc_mdb_info_dump( char* indent, struct _smc_mdb_channel_info_t* smc_mdb_info, int32_t mem_offset, uint8_t out_mdb )
{
    /*
    typedef struct _smc_mdb_channel_info_t
    {
        void*    pool_in;
        void*    pool_out;
        uint32_t total_size_in;
        uint32_t total_size_out;

    } smc_mdb_channel_info_t;
    */

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

        SMC_TRACE_PRINTF("%s- MDB %s: size %d bytes, SHM offset 0x%08X", indent, (out_mdb==TRUE)?"OUT":"IN", size, (uint32_t)mem_offset);
        SMC_TRACE_PRINTF("%s    Memory area: 0x%08X - 0x%08X (PHY-ADDR: 0x%08X - 0x%08X)", indent,
                    (uint32_t)(pool), (uint32_t)pool+size,
                    ((uint32_t)pool-mem_offset), ((uint32_t)pool+size-mem_offset));

    }
    else
    {
        SMC_TRACE_PRINTF("%s- <MDB %s is not initialized>", indent, (out_mdb==TRUE)?"OUT":"IN");
    }
}

/* EOF */

