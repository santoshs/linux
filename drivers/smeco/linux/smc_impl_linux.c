/*
*   Smeco implementation specific for Linux Kernel.
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

Version:       4    19-Dec-2011     Heikki Siikaluoma
Status:        draft
Description :  Linux Kernel module features implemented

Version:       1    17-Nov-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#include "smc_common_includes.h"

#include "smc.h"
#include "smc_trace.h"
#include <linux/random.h>
#include <asm/io.h>

    /* INTC-BB interrupt handler */
static irqreturn_t smc_linux_interrupt_handler_intcbb(int irq, void *dev_id );
static irqreturn_t smc_linux_interrupt_handler_int_genout(int irq, void *dev_id );
static irqreturn_t smc_linux_interrupt_handler_int_resource(int irq, void *dev_id );

/* =============================================================
 * SMC Memory management platform specific implementations
 */



/* Changed to macros in h file
 * TODO Cleanup
void smc_shm_cache_invalidate(void* start_address, void* end_address)
{
    SMC_TRACE_PRINTF_INFO("smc_shm_cache_invalidate: 0x%08X-0x%08X", (uint32_t)start_address, (uint32_t)end_address);

    SMC_TRACE_PRINTF_WARNING("smc_shm_cache_clean: NOT IMPLEMENTED");
}

void smc_shm_cache_clean(void* start_address, void* end_address)
{
    SMC_TRACE_PRINTF_INFO("smc_shm_cache_clean: 0x%08X-0x%08X", (uint32_t)start_address, (uint32_t)end_address);

    SMC_TRACE_PRINTF_WARNING("smc_shm_cache_clean: NOT IMPLEMENTED");
}
*/

/* =============================================================
 * SMC Signal function platform specific implementation
 */


/*
 * Interrupt handler
 */
static irqreturn_t smc_linux_interrupt_handler_intcbb(int irq, void *dev_id )
{
    /*smc_signal_t* signal = NULL;*/

    SMC_TRACE_PRINTF_SIGNAL_RECEIVE("smc_linux_interrupt_handler_intcbb: IRQ: 0x%02X (%d), Device 0x%08X", (uint32_t)irq, irq, (uint32_t)dev_id);

    /* TODO Check lock --> Create common lock to smc.c */

        /* Get the appropriate signal from array */
    smc_signal_handler_t* signal_handler = smc_signal_handler_get( (uint32_t)irq, SMC_SIGNAL_TYPE_INTCBB );

    /* TODO Unlock if locked */

    assert( signal_handler != NULL );

    if( signal_handler->smc_channel != NULL )
    {
        smc_channel_interrupt_handler( signal_handler->smc_channel );

        SMC_TRACE_PRINTF_SIGNAL("smc_linux_interrupt_handler_intcbb: IRQ: 0x%02X (%d) handled", (uint32_t)irq, irq);
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_linux_interrupt_handler_intcbb: No channel initialized for IRQ: 0x%02X (%d)", (uint32_t)irq, irq);
    }

    return IRQ_HANDLED;
}

static irqreturn_t smc_linux_interrupt_handler_int_genout(int irq, void *dev_id )
{
    /*smc_signal_t* signal = NULL;*/

    SMC_TRACE_PRINTF_SIGNAL_RECEIVE("smc_linux_interrupt_handler_int_genout: IRQ: 0x%02X (%d), Device 0x%08X", (uint32_t)irq, irq, (uint32_t)dev_id);

    /* TODO Check lock --> Create common lock to smc.c */

        /* Get the appropriate signal from array */
    smc_signal_handler_t* signal_handler = smc_signal_handler_get( (uint32_t)irq, SMC_SIGNAL_TYPE_INT_WGM_GENOUT );

    /* TODO Unlock if locked */

    assert( signal_handler != NULL );

    if( signal_handler->smc_channel != NULL )
    {
        smc_channel_interrupt_handler( signal_handler->smc_channel );

        SMC_TRACE_PRINTF_SIGNAL("smc_linux_interrupt_handler_int_genout: IRQ: 0x%02X (%d) handled", (uint32_t)irq, irq);
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_linux_interrupt_handler_int_genout: No channel initialized for IRQ: 0x%02X (%d)", (uint32_t)irq, irq);
    }

    return IRQ_HANDLED;
}

static irqreturn_t smc_linux_interrupt_handler_int_resource(int irq, void *dev_id )
{
    smc_signal_t* signal = NULL;
    int irq_spi = irq-SMC_APE_IRQ_OFFSET_INTCSYS_SPI;

    SMC_TRACE_PRINTF_SIGNAL_RECEIVE("smc_linux_interrupt_handler_int_resource: IRQ: %d -> SPI %d, Device 0x%08X", irq, irq_spi, (uint32_t)dev_id);

    /* TODO Check lock --> Create common lock to smc.c */

        /* Get the appropriate signal from array */
    smc_signal_handler_t* signal_handler = smc_signal_handler_get( (uint32_t)irq_spi, SMC_SIGNAL_TYPE_INT_RESOURCE );

    /* TODO Unlock if locked */

    if( signal_handler != NULL )
    {
        signal = signal_handler->signal;

        if( signal_handler->smc_channel != NULL )
        {
            smc_channel_interrupt_handler( signal_handler->smc_channel );

            SMC_TRACE_PRINTF_SIGNAL("smc_linux_interrupt_handler_int_resource: IRQ: ID %d SPI %d handled", irq, irq_spi);
        }
        else
        {
            SMC_TRACE_PRINTF_WARNING("smc_linux_interrupt_handler_int_resource: No channel initialized for IRQ: 0x%02X (%d)", (uint32_t)irq, irq);
        }
    }
    else
    {
        SMC_TRACE_PRINTF_WARNING("smc_linux_interrupt_handler_int_resource: No IRQ handler initialized for IRQ: ID %d SPI %d", irq, irq_spi);
    }

    /* Pull down the IRQ since there is GOP001 used in MOdem Side (the peripheral address is given) */
    if( signal != NULL && signal->peripheral_address != NULL )
    {
        /* TODO FIX: The SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM only valid for SPI 193 - 198 */
        uint32_t genios = (1UL << ( signal->interrupt_id - SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM) );

        SMC_TRACE_PRINTF_SIGNAL("smc_linux_interrupt_handler_int_resource: Clear signal %d with gop001 CLEAR value 0x%08X",
        signal->interrupt_id, genios);

            // TODO Use GOP001 STR variable name for CLEAR
        __raw_writel( genios, ((void __iomem *)(signal->peripheral_address + 8 )) );

        __raw_readl( ((void __iomem *)signal->peripheral_address) );
    }


    return IRQ_HANDLED;
}

smc_signal_t* smc_signal_create( uint32_t signal_id, uint32_t signal_type )
{
    smc_signal_t* signal = (smc_signal_t*)SMC_MALLOC( sizeof( smc_signal_t ) );

    assert(signal!=NULL);

    signal->interrupt_id       = signal_id;
    signal->signal_type        = signal_type;
    signal->peripheral_address = NULL;
    signal->address_remapped   = FALSE;

    if( signal->signal_type == SMC_SIGNAL_TYPE_INTGEN ||
        signal->signal_type == SMC_SIGNAL_TYPE_INT_RESOURCE )
    {
        /* TODO Peripheral address configurable */
        uint32_t p_address = SMC_PERIPHERAL_ADDRESS_MODEM_GOP_INTGEN_1+SMC_ADDRESS_APE_OFFSET_TO_MODEM ;

        signal->peripheral_address = ioremap(p_address, sizeof(GOP001_STR));
        signal->address_remapped = TRUE;

        SMC_TRACE_PRINTF_SIGNAL("smc_signal_create: signal: 0x%08X, ID: (%d) 0x%08X type 0x%08X, remapped peripheral address from 0x%08X is 0x%08X",
                    (uint32_t)signal, signal->interrupt_id, signal->interrupt_id, signal->signal_type,
                    p_address, (uint32_t)signal->peripheral_address);
    }

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_create: signal: 0x%08X, ID: (%d) 0x%08X, type 0x%08X",
            (uint32_t)signal, signal->interrupt_id, signal->interrupt_id, signal->signal_type);

    return signal;
}

smc_signal_t*  smc_signal_copy( smc_signal_t* signal )
{
    smc_signal_t* copy_signal = NULL;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_copy: copy from 0x%08X", (uint32_t)signal);

    if( signal )
    {
        copy_signal = smc_signal_create( signal->interrupt_id, signal->signal_type );

        copy_signal->peripheral_address = signal->peripheral_address;
        copy_signal->address_remapped = signal->address_remapped;
    }

    return copy_signal;
}

void smc_signal_destroy( smc_signal_t* signal )
{
    SMC_TRACE_PRINTF_SIGNAL("smc_signal_destroy: signal: 0x%08X", (uint32_t)signal);

    if( signal != NULL )
    {
        if( signal->peripheral_address != NULL && signal->address_remapped==TRUE )
        {
            iounmap( signal->peripheral_address );
            signal->peripheral_address = NULL;
        }
    }

    SMC_FREE( signal );
    signal = NULL;

    /* Free the signal handler */
    /*void free_irq(unsigned int irq, void *dev_id);*/
}

uint8_t smc_signal_raise( smc_signal_t* signal )
{
    uint8_t ret_value = SMC_OK;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_raise: signal: 0x%08X", (uint32_t)signal);

    if( signal->signal_type == SMC_SIGNAL_TYPE_INTGEN )
    {
        if( signal->peripheral_address != NULL )
        {
            uint32_t genios = (1UL << ((signal->interrupt_id-SMC_MODEM_INTGEN_L2_FIRST) + SMC_MODEM_INTGEN_L2_OFFSET));

            SMC_TRACE_PRINTF_SIGNAL("smc_signal_raise: SMC_SIGNAL_TYPE_INTGEN: Raise signal %d with gop001 set value 0x%08X",
            signal->interrupt_id, genios);

                // TODO Use GOP001 STR variable names
            __raw_writel( genios, ((void __iomem *)(signal->peripheral_address+4)) );

            __raw_readl( ((void __iomem *)signal->peripheral_address) );
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_signal_raise: signal: 0x%08X, SMC_SIGNAL_TYPE_INTGEN, invalid peripheral address NULL",
                    (uint32_t)signal);
        }
    }
    else
    {

        /* TODO Write to register */

        SMC_TRACE_PRINTF_ERROR("smc_signal_raise: Signal type 0x%08X NOT IMPLEMENTED", signal->signal_type);
     }

    return ret_value;
}

uint8_t smc_signal_acknowledge( smc_signal_t* signal )
{
    uint8_t ret_value = SMC_OK;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_acknowledge: signal: 0x%08X", (uint32_t)signal);

    return ret_value;
}

uint8_t smc_signal_handler_register( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel )
{
    uint8_t       ret_value = SMC_OK;
    int           result    = 0;

    const char*   device_name  = NULL;
    void*         dev_id    = NULL;

    unsigned long flags     = 0x00; /* 0x00, IRQF_SHARED, IRQ_TYPE_PRIO, IRQF_SAMPLE_RANDOM | IRQF_DISABLED */

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: 0x%08X: 0x%02X (%d), type 0x%08X for channel: 0x%08X", (uint32_t)signal, signal->interrupt_id, signal->interrupt_id, signal->signal_type, (uint32_t)smc_channel);

    smc_signal_handler_create_and_add( smc_instance, signal, smc_channel);

    if( signal->signal_type == SMC_SIGNAL_TYPE_INTCBB )
    {
        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: 0x%08X: is SMC_SIGNAL_TYPE_INTCBB", (uint32_t)signal);
        result = request_irq( (unsigned int)signal->interrupt_id, smc_linux_interrupt_handler_intcbb, flags, device_name, dev_id );

        if( result )
        {
            SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INTCBB 0x%08X: request_irq FAILED, result %d", (uint32_t)signal, result);
        }
        else
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INTCBB 0x%08X: request_irq SUCCESS", (uint32_t)signal);
        }

#if(SMCTEST==TRUE)
        if( result )
        {
            /* Ignore in test mode, linux kernel perhaps not fully initialized */
            result = 0;
        }
#endif
    }
    else if( signal->signal_type == SMC_SIGNAL_TYPE_INT_WGM_GENOUT )
    {
        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: 0x%08X: is SMC_SIGNAL_TYPE_INT_WGM_GENOUT", (uint32_t)signal);

        result = request_irq( (unsigned int)signal->interrupt_id, smc_linux_interrupt_handler_int_genout, flags, device_name, dev_id );

        if( result )
        {
            SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_WGM_GENOUT 0x%08X: request_irq FAILED, result %d", (uint32_t)signal, result);
        }
        else
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_WGM_GENOUT 0x%08X: request_irq SUCCESS", (uint32_t)signal);
        }

#if(SMCTEST==TRUE)
        if( result )
        {
            /* Ignore in test mode, linux kernel perhaps not fully initialized */
            result = 0;
        }
#endif
    }
    else if( signal->signal_type == SMC_SIGNAL_TYPE_INT_RESOURCE )
    {
        struct resource*        res             = NULL;
        struct platform_device* platform_device = NULL;

        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: 0x%08X: is SMC_SIGNAL_TYPE_INT_RESOURCE", (uint32_t)signal);

        if( smc_instance->smc_parent_ptr != NULL )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: parent object 0x%08X found, extract the platform device", smc_instance->smc_parent_ptr);
            platform_device = (struct platform_device*)smc_instance->smc_parent_ptr;
        }

        if( platform_device )
        {
            int     irq_spi = (int)signal->interrupt_id;
            int     res_id  = 0;
            uint8_t found   = FALSE;

            /* TODO Check if change to: platform_get_resource_byname(pdev, IORESOURCE_IRQ, "cmd_irq"); */

            while( (res = platform_get_resource(platform_device, IORESOURCE_IRQ, res_id)) )
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: check res id %d where IRQ ID %d...", res_id, res->start);

                if( (res->start-SMC_APE_IRQ_OFFSET_INTCSYS_SPI) == irq_spi )
                {
                    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: res id %d: IRQ ID %d match to SPI %d...", res_id, res->start,irq_spi);
                    found = TRUE;
                    break;
                }

                res_id++;
            }

            if( found )
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: found res 0x%08X, IRQ start is %d",
                        (uint32_t)signal, res, res->start );

                /*flags = IRQF_SHARED | IRQF_DISABLED;*/
                flags = IRQF_DISABLED;

                device_name  = dev_name(&platform_device->dev);
                dev_id    = platform_device;

                result = request_irq( res->start, smc_linux_interrupt_handler_int_resource, flags, device_name, dev_id );

                if( result )
                {
                    SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: request_irq FAILED, result %d", (uint32_t)signal, result);
                }
                else
                {
                    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: request_irq SUCCESS", (uint32_t)signal);
                }
            }
            else
            {
                SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: platform_get_resource with irq id %d FAILED", (uint32_t)signal, signal->interrupt_id);
            }
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: No device, unable to get IRQ %d", (uint32_t)signal, signal->interrupt_id);
        }

#if(SMCTEST==TRUE)
        if( result )
        {
            /* Ignore in test mode, linux kernel perhaps not fully initialized */
            result = 0;
        }
#endif
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: 0x%08X type 0x%08X not supported, no handler initialized", (uint32_t)signal, signal->signal_type);
        result = -1;
    }

    if( result )
    {
        SMC_TRACE_PRINTF_ERROR("smc_signal_handler_register: signal: 0x%08X: 0x%02X (%d) request for channel: 0x%08X FAILED", (uint32_t)signal, signal->interrupt_id, signal->interrupt_id, (uint32_t)smc_channel);
        ret_value = SMC_ERROR;
    }
    else
    {
        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: signal: 0x%08X: 0x%02X (%d) for channel: 0x%08X successfully registered", (uint32_t)signal, signal->interrupt_id, signal->interrupt_id, (uint32_t)smc_channel);
    }

    return ret_value;
}

/* =============================================================
 * SMC locking function platform specific implementation
 * The cock struct defined in smc_conf_platform.h
 *
 */

smc_lock_t* smc_lock_create( void )
{
    smc_lock_t* lock = (smc_lock_t*)SMC_MALLOC( sizeof( smc_lock_t ) );

    spin_lock_init(&lock->mr_lock);

    lock->flags = 0x00000000;

    SMC_TRACE_PRINTF_INFO("smc_lock_create: lock 0x%08X...", (uint32_t)lock);

    return lock;
}


/* Changed to macros: TODO Cleanup
void smc_lock( smc_lock_t* lock )
{
    SMC_TRACE_PRINTF_LOCK("smc_lock: lock 0x%08X...", (uint32_t)lock);
    spin_lock( &(lock->mr_lock) );
}

void smc_unlock( smc_lock_t* lock )
{
    spin_unlock( &(lock->mr_lock) );
    SMC_TRACE_PRINTF_LOCK("smc_unlock: unlock 0x%08X...", (uint32_t)lock);
}

void smc_lock_irq( smc_lock_t* lock )
{
    SMC_TRACE_PRINTF_LOCK("smc_lock_irq: lock 0x%08X...", (uint32_t)lock);

    spin_lock_irqsave( &(lock->mr_lock), lock->flags);

}

void smc_unlock_irq( smc_lock_t* lock )
{
    spin_unlock_irqrestore(&lock->mr_lock, lock->flags);

    SMC_TRACE_PRINTF_LOCK("smc_unlock_irq: lock 0x%08X...", (uint32_t)lock);
}
*/


void smc_lock_destroy( smc_lock_t* lock )
{
    SMC_TRACE_PRINTF_INFO("smc_lock_destroy: lock 0x%08X...", (uint32_t)lock);

    if( lock != NULL )
    {
        SMC_FREE( lock );
        lock = NULL;
    }
}


/* =============================================================
 * SMC semaphore function platform specific implementations
 * The semaphore struct is defined in smc_conf_platform.h
 *
 */

smc_semaphore_t* smc_semaphore_create( void )
{
    smc_semaphore_t* sem = (smc_semaphore_t*)SMC_MALLOC( sizeof( smc_semaphore_t ) );

    /* TODO Semaphore mutex must be created before SMC is taken into use */

    SMC_TRACE_PRINTF_INFO("smc_semaphore_create: semaphore 0x%08X...", (uint32_t)sem);

    return sem;
}


/* =============================================================
 * Miscellaneous Linux Kernel implementations.
 */
uint32_t rand(void)
{
    uint32_t random;

    get_random_bytes(&random, sizeof(random));

    return random;
}

/*
 * Linux Kernel data trace function to print data array contents.
 */

void smc_printf_data_linux_kernel(int length, uint8_t* data)
{
    int i = 0;
    int row_len = 16;

    printk(KERN_DEBUG "\n");

    for( i = 0; i < length; i++ )
    {
        if(i%row_len == 0)
        {
            printk(KERN_DEBUG "0x%02X", data[i]);
        }
        else
        {
            printk(KERN_DEBUG " 0x%02X", data[i]);
        }

        if( i > 0 && ( i%(row_len) == (row_len-1) || i >= length-1 ))
        {
            printk(KERN_DEBUG "\n");
        }
    }
}

/*
 * SMC initialization function if called.
 */
uint8_t smc_module_initialize( smc_conf_t* smc_instance_conf )
{
    uint8_t ret_value = SMC_OK;

    return ret_value;
}

/* EOF */
