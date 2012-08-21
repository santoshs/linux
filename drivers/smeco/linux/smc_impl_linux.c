/*
*   Smeco implementation specific for Linux Kernel.
*   Copyright � Renesas Mobile Corporation 2011. All rights reserved
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

Version:       14   07-Jun-2012     Heikki Siikaluoma
Status:        draft
Description :  Resource IRQ handler changed: First clears the interrupt then handles the request

Version:       8    29-Feb-2012     Heikki Siikaluoma
Status:        draft
Description :  Linux Kernel SMC timer functions added

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
#include <linux/wakelock.h>
#include <asm/io.h>

static irqreturn_t smc_linux_interrupt_handler_intcbb(int irq, void *dev_id );          /* INTC-BB interrupt handler */
static irqreturn_t smc_linux_interrupt_handler_int_genout(int irq, void *dev_id );      /* GENIO interrupt handler */
static irqreturn_t smc_linux_interrupt_handler_int_resource(int irq, void *dev_id );    /* SPI resource interrupt handler */

static smc_lock_t* g_local_lock_sleep_control = NULL;

static struct wake_lock* wakelock = NULL;
static uint8_t wake_lock_initialized = FALSE;

static inline smc_lock_t* get_local_lock_sleep_control(void)
{
    if( g_local_lock_sleep_control == NULL ) g_local_lock_sleep_control = smc_lock_create();
    return g_local_lock_sleep_control;
}

static inline struct wake_lock* get_wake_lock()
{
    if( wakelock==NULL )
    {
        SMC_TRACE_PRINTF_ALWAYS("get_wake_lock: initialize");
        wakelock = (struct wake_lock*)SMC_MALLOC( sizeof( struct wake_lock ) );

        wake_lock_init(wakelock, WAKE_LOCK_SUSPEND, "smc_wakelock");
        wake_lock_initialized = TRUE;
     }

    return wakelock;
}

/* =============================================================
 * SMC Interrupt platform specific implementations
 */

#ifdef SMC_WAKEUP_USE_EXTERNAL_IRQ_APE

void smc_init_external_wakeup_irqc( smc_t* smc_instance );

static irqreturn_t smc_linux_interrupt_handler_int_genout_wakeup(int irq, void *dev_id );

static irqreturn_t smc_linux_interrupt_handler_int_genout_wakeup(int irq, void *dev_id )
{
    smc_signal_handler_t* signal_handler = NULL;
    uint32_t              signal_type = SMC_SIGNAL_TYPE_INT_WGM_GENOUT;


    wake_lock( get_wake_lock() );
    SMC_TRACE_PRINTF_SIGNAL("smc_linux_interrupt_handler_int_genout_wakeup: IRQ: 0x%02X (%d), Device 0x%08X", (uint32_t)irq, irq, (uint32_t)dev_id);

    /* TODO Check lock --> Create common lock to smc.c */

    RD_TRACE_SEND2(TRA_SMC_IRQ_START, 4, &irq,
                                      4, &signal_type );


    /* TODO Unlock if locked */
    /*
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

    */

    RD_TRACE_SEND2(TRA_SMC_IRQ_END, 4, &irq,
                                    4, &signal_type );

    wake_unlock( get_wake_lock() );

    return IRQ_HANDLED;
}

void smc_register_wakeup_irq( smc_t* smc_instance, uint32_t signal_id, uint32_t signal_type)
{

#if(SMC_APE_WAKEUP_EXTERNAL_IRQ_REGISTER_HANDLER==TRUE)

    //uint32_t      signal_id    = 0;     // EXT
    int           result       = 0;
    const char*   device_name  = NULL;
    void*         dev_id       = NULL;
    unsigned long flags        = 0x00;

    int           irq_offset   = SMC_APE_IRQ_OFFSET_INTCSYS_SPI;

    struct resource*        res             = NULL;
    struct platform_device* platform_device = NULL;

    smc_signal_t* signal = smc_signal_create( signal_id, signal_type );

    smc_init_external_wakeup_irqc( smc_instance );

    if( signal_type == SMC_SIGNAL_TYPE_INT_IRQC )
    {
        irq_offset = SMC_APE_IRQ_OFFSET_IRQ_SPI;
    }


    SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: signal: 0x%08X", (uint32_t)signal);

    if( smc_instance->smc_parent_ptr != NULL )
    {
        SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: parent object 0x%08X found, extract the platform device", smc_instance->smc_parent_ptr);
        platform_device = (struct platform_device*)smc_instance->smc_parent_ptr;
    }

    if( platform_device )
    {
        int     irq_spi = (int)signal->interrupt_id;
        int     res_id  = 0;
        uint8_t found   = FALSE;

        while( (res = platform_get_resource(platform_device, IORESOURCE_IRQ, res_id)) )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: check res id %d where IRQ ID %d...", res_id, res->start);

            if( (res->start-irq_offset) == irq_spi )
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: res id %d: IRQ ID %d match to SPI %d...", res_id, res->start,irq_spi);
                found = TRUE;
                break;
            }

            res_id++;
        }

        if( found )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: signal: 0x%08X: found res 0x%08X, IRQ start is %d",
                    (uint32_t)signal, res, res->start );

            /*flags = IRQF_SHARED | IRQF_DISABLED;*/
            flags = IRQF_DISABLED;

            device_name  = dev_name(&platform_device->dev);
            dev_id    = platform_device;

            result = request_irq( res->start, smc_linux_interrupt_handler_int_genout_wakeup, flags, device_name, dev_id );

            if( result )
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: signal: 0x%08X: request_irq FAILED, result %d", (uint32_t)signal, result);
            }
            else
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: signal: 0x%08X: request_irq SUCCESS", (uint32_t)signal);
            }

            enable_irq_wake(res->start);
        }
        else
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: signal: SMC_SIGNAL_TYPE_INT_WGM_GENOUT: NO Resource found");
        }
    }
    else
    {
        SMC_TRACE_PRINTF_SIGNAL("smc_register_wakeup_irq: signal: SMC_SIGNAL_TYPE_INT_WGM_GENOUT: NO Platform device");
    }
#else

    smc_init_external_wakeup_irqc( smc_instance );

#endif
}

void smc_init_external_wakeup_irqc( smc_t* smc_instance )
{

    #define SMC_PRI_STS0         0xE61C2410     /* Status i Event Generator block 2 */

    #define SMC_PRI_SET0         0xE61C2414     /* Event Generator block 2 */
    #define SMC_PRI_SET0_VALUE   0x00000001

    #define SMC_CONFIG0          0xE61C1980     /**/
    //#define SMC_CONFIG0_VALUE    0x00000001
    #define SMC_CONFIG0_VALUE    0x00000008     /* Rising edge*/

    #define SMC_WAKEN_STS0       0xE61C1884
    #define SMC_WAKEN_SET0       0xE61C1888
    #define SMC_WAKEN_SET0_VALUE 0x00000001

    #define SMC_WUPMMSK          0xE6180030

    SMC_TRACE_PRINTF_SIGNAL("smc_init_external_wakeup_irqc: initialize...");

    uint32_t reg_val  = SMC_SHM_READ32(SMC_PRI_STS0);
    uint32_t new_val = reg_val | SMC_PRI_SET0_VALUE;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: SMC_PRI_SET0 = 0x%08X, write 0x%08X", reg_val, new_val);

    SMC_SHM_WRITE32(SMC_PRI_SET0, new_val);
    SMC_SHM_READ32(SMC_PRI_SET0);
    reg_val = SMC_SHM_READ32(SMC_PRI_STS0);

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: SMC_PRI_SET0 is now 0x%08X", reg_val);

    // -------

    reg_val  = SMC_SHM_READ32(SMC_CONFIG0);
    new_val = reg_val | SMC_CONFIG0_VALUE;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: SMC_CONFIG0 = 0x%08X, write 0x%08X", reg_val, new_val);

    SMC_SHM_WRITE32(SMC_CONFIG0, new_val);
    reg_val = SMC_SHM_READ32(SMC_CONFIG0);

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: SMC_CONFIG0 is now 0x%08X", reg_val);

    // -------

    reg_val  = SMC_SHM_READ32(SMC_WAKEN_STS0);
    new_val = reg_val | SMC_WAKEN_SET0_VALUE;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: SMC_WAKEN_SET0 = 0x%08X, write 0x%08X", reg_val, new_val);

    SMC_SHM_WRITE32(SMC_WAKEN_SET0, new_val);
    SMC_SHM_READ32(SMC_WAKEN_SET0);
    reg_val = SMC_SHM_READ32(SMC_WAKEN_STS0);



    // -------

    reg_val = SMC_SHM_READ32(SMC_WUPMMSK);
    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_register: SMC_WUPMMSK is 0x%08X", reg_val);

    SMC_TRACE_PRINTF_SIGNAL("smc_init_external_wakeup_irqc: completed");

}

#endif


/*
 * Interrupt handler
 */
static irqreturn_t smc_linux_interrupt_handler_intcbb(int irq, void *dev_id )
{
    smc_signal_handler_t* signal_handler = NULL;
    uint32_t              signal_type    = SMC_SIGNAL_TYPE_INTCBB;

    wake_lock( get_wake_lock() );

    SMC_TRACE_PRINTF_SIGNAL_RECEIVE("smc_linux_interrupt_handler_intcbb: IRQ: 0x%02X (%d), Device 0x%08X", (uint32_t)irq, irq, (uint32_t)dev_id);



    RD_TRACE_SEND2(TRA_SMC_IRQ_START, 4, &irq,
                                      4, &signal_type );


    /* TODO Check lock --> Create common lock to smc.c */

        /* Get the appropriate signal from array */
    signal_handler = smc_signal_handler_get( (uint32_t)irq, SMC_SIGNAL_TYPE_INTCBB );

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

    RD_TRACE_SEND2(TRA_SMC_IRQ_END, 4, &irq,
                                    4, &signal_type );

    wake_lock_timeout( get_wake_lock(), msecs_to_jiffies(SMC_APE_WAKEUP_WAKELOCK_TIMEOUT_MSEC) );

    return IRQ_HANDLED;
}

static irqreturn_t smc_linux_interrupt_handler_int_genout(int irq, void *dev_id )
{
    smc_signal_handler_t* signal_handler = NULL;
    uint32_t              signal_type = SMC_SIGNAL_TYPE_INT_WGM_GENOUT;


    wake_lock( get_wake_lock() );
    SMC_TRACE_PRINTF_SIGNAL_RECEIVE("smc_linux_interrupt_handler_int_genout: IRQ: 0x%02X (%d), Device 0x%08X", (uint32_t)irq, irq, (uint32_t)dev_id);

    /* TODO Check lock --> Create common lock to smc.c */

    RD_TRACE_SEND2(TRA_SMC_IRQ_START, 4, &irq,
                                      4, &signal_type );

        /* Get the appropriate signal from array */
    signal_handler = smc_signal_handler_get( (uint32_t)irq, SMC_SIGNAL_TYPE_INT_WGM_GENOUT );

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

    RD_TRACE_SEND2(TRA_SMC_IRQ_END, 4, &irq,
                                    4, &signal_type );

    wake_lock_timeout( get_wake_lock(), msecs_to_jiffies(SMC_APE_WAKEUP_WAKELOCK_TIMEOUT_MSEC) );

    return IRQ_HANDLED;
}

static irqreturn_t smc_linux_interrupt_handler_int_resource(int irq, void *dev_id )
{
    smc_signal_handler_t* signal_handler = NULL;
    smc_signal_t*         signal         = NULL;
    uint32_t              signal_type    = SMC_SIGNAL_TYPE_INT_RESOURCE;
    int                   irq_spi        = irq-SMC_APE_IRQ_OFFSET_INTCSYS_SPI;

    wake_lock( get_wake_lock() );

    SMC_TRACE_PRINTF_SIGNAL_RECEIVE("smc_linux_interrupt_handler_int_resource: IRQ: %d -> SPI %d, Device 0x%08X", irq, irq_spi, (uint32_t)dev_id);

    RD_TRACE_SEND2(TRA_SMC_IRQ_START, 4, &irq,
                                      4, &signal_type );

    /* TODO Check lock --> Create common lock to smc.c */

        /* Get the appropriate signal from array */
    signal_handler = smc_signal_handler_get( (uint32_t)irq_spi, SMC_SIGNAL_TYPE_INT_RESOURCE );

    /* TODO Unlock if locked */

    if( signal_handler != NULL )
    {
        signal = signal_handler->signal;

        /* Clear IRQ first the handle the request to avoid missing irqs */
        /* Pull down the IRQ since there is GOP001 used in Modem Side (the peripheral address is given) */
        if( signal != NULL && signal->peripheral_address != 0 )
        {
            /* TODO FIX: The SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM only valid for SPI 193 - 198 */
            uint32_t genios = (1UL << ( signal->interrupt_id - SMC_APE_IRQ_OFFSET_INTCSYS_TO_WGM) );
            smc_gop001_t* gop001 = (smc_gop001_t*)signal->peripheral_address;

            SMC_TRACE_PRINTF_SIGNAL("smc_linux_interrupt_handler_int_resource: Clear signal %d with gop001 CLEAR value 0x%08X",
            signal->interrupt_id, genios);

            SMC_HOST_ACCESS_WAKEUP( get_local_lock_sleep_control(), SMC_MODEM_WAKEUP_WAIT_TIMEOUT );

            /*
                // TODO Use GOP001 STR variable name for CLEAR --> Clean up
            genios |= __raw_readl( ((void __iomem *)signal->peripheral_address + 8) );
            __raw_writel( genios, ((void __iomem *)(signal->peripheral_address + 8 )) );
            __raw_readl( ((void __iomem *)signal->peripheral_address + 8) );
            */

            genios |= SMC_SHM_READ32( &gop001->clear );
            SMC_SHM_WRITE32( &gop001->clear, genios );
            SMC_SHM_READ32( &gop001->clear );

            SMC_HOST_ACCESS_SLEEP( get_local_lock_sleep_control() );
        }

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

    RD_TRACE_SEND2(TRA_SMC_IRQ_END, 4, &irq,
                                    4, &signal_type );

    wake_lock_timeout( get_wake_lock(), msecs_to_jiffies(SMC_APE_WAKEUP_WAKELOCK_TIMEOUT_MSEC) );

    return IRQ_HANDLED;
}

smc_signal_t* smc_signal_create( uint32_t signal_id, uint32_t signal_type )
{
    smc_signal_t* signal = (smc_signal_t*)SMC_MALLOC( sizeof( smc_signal_t ) );

    assert(signal!=NULL);

    signal->interrupt_id       = signal_id;
    signal->signal_type        = signal_type;
    signal->peripheral_address = 0;
    signal->address_remapped   = FALSE;

    if( signal->signal_type == SMC_SIGNAL_TYPE_INTGEN ||
        signal->signal_type == SMC_SIGNAL_TYPE_INT_RESOURCE )
    {
        /* TODO Peripheral address configurable */
        uint32_t p_address = SMC_PERIPHERAL_ADDRESS_MODEM_GOP_INTGEN_1+SMC_ADDRESS_APE_OFFSET_TO_MODEM ;

        signal->peripheral_address = (uint32_t)ioremap(p_address, sizeof(smc_gop001_t));
        signal->address_remapped   = TRUE;

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
        /* Unregister */

        if( signal->peripheral_address != 0 && signal->address_remapped==TRUE )
        {
            iounmap( (void*)signal->peripheral_address );
            signal->peripheral_address = 0;
        }
    }

    SMC_FREE( signal );
    signal = NULL;

    /* Free the signal handler */
    /*void free_irq(unsigned int irq, void *dev_id);*/
}

uint8_t smc_signal_raise( smc_signal_t* signal )
{
    SMC_TRACE_PRINTF_SIGNAL("smc_signal_raise: signal: 0x%08X", (uint32_t)signal);

    if( signal->signal_type == SMC_SIGNAL_TYPE_INTGEN )
    {
        if( signal->peripheral_address != 0 )
        {
            uint8_t ret_value = SMC_OK;
            uint32_t genios = (1UL << ((signal->interrupt_id-SMC_MODEM_INTGEN_L2_FIRST) + SMC_MODEM_INTGEN_L2_OFFSET));
            smc_gop001_t* gop001 = (smc_gop001_t*)signal->peripheral_address;

            SMC_TRACE_PRINTF_SIGNAL("smc_signal_raise: SMC_SIGNAL_TYPE_INTGEN: Raise signal %d with gop001 set value 0x%08X",
            signal->interrupt_id, genios);

            RD_TRACE_SEND2(TRA_SMC_SIGNAL_INTGEN, 4, &signal,
                                                  4, &signal->interrupt_id );

            SMC_HOST_ACCESS_WAKEUP( get_local_lock_sleep_control(), SMC_MODEM_WAKEUP_WAIT_TIMEOUT );

            /*
                // TODO Use GOP001 STR variable names

            genios |= __raw_readl( ((void __iomem *)(signal->peripheral_address+4)) );
            __raw_writel( genios, ((void __iomem *)(signal->peripheral_address+4)) );
            __raw_readl( ((void __iomem *)(signal->peripheral_address+4)) );
                */

            genios |= SMC_SHM_READ32( &gop001->set );
            SMC_SHM_WRITE32( &gop001->set, genios );
            SMC_SHM_READ32( &gop001->set );

            SMC_HOST_ACCESS_SLEEP( get_local_lock_sleep_control() );

            return ret_value;
        }
        /* TODO Clean up
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_signal_raise: signal: 0x%08X, SMC_SIGNAL_TYPE_INTGEN, invalid peripheral address NULL",
                    (uint32_t)signal);
        }
        */
    }
    /* TODO Cleanup
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_signal_raise: Signal type 0x%08X NOT IMPLEMENTED", signal->signal_type);
     }
     */

    SMC_TRACE_PRINTF_ERROR("smc_signal_raise: Signal type 0x%08X FAILED", signal->signal_type);

    return SMC_ERROR;
}

uint8_t smc_signal_acknowledge( smc_signal_t* signal )
{
    uint8_t ret_value = SMC_OK;

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_acknowledge: signal: 0x%08X", (uint32_t)signal);

    return ret_value;
}

uint8_t smc_signal_handler_register( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel )
{
    uint8_t       ret_value    = SMC_OK;
    int           result       = 0;
    const char*   device_name  = NULL;
    void*         dev_id       = NULL;
    unsigned long flags        = 0x00; /* 0x00, IRQF_SHARED, IRQ_TYPE_PRIO, IRQF_SAMPLE_RANDOM | IRQF_DISABLED */

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



                /*
                { // DEBUGGING WAKEUP

                uint32_t wupsysmsk = SMC_SHM_READ32(0xE618002C);
                SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: WUPSMSK=0x%08X", wupsysmsk);

                //wupsysmsk = 0xFFFFFFFF;
                //SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: set WUPSMSK=0x%08X...", wupsysmsk);
                //SMC_SHM_WRITE32(0xE618002C, wupsysmsk);
                //wupsysmsk = SMC_SHM_READ32(0xE618002C);

                //SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: WUPSMSK is now 0x%08X", wupsysmsk);


                uint32_t WUPSCR = SMC_SHM_READ32(0xE6180264);

                SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: WUPSCR is 0x%08X", WUPSCR);

                WUPSCR = 0xFFFFFFFF;

                SMC_SHM_WRITE32(0xE6180264, WUPSCR);
                WUPSCR = SMC_SHM_READ32(0xE6180264);

                SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: WUPSMSK is now 0x%08X", wupsysmsk);

                SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: enable_irq_wake %d", res->start);
                //SMC_TRACE_PRINTF_ALWAYS("smc_signal_handler_register: WUPSFAC=0x%08X WUPSMSK=0x%08X", SMC_SHM_READ32(0xE6180098), SMC_SHM_READ32(0xE618002C));
                }   // EOF DEBUGGING
                */

                enable_irq_wake(res->start);
                //device_init_wakeup(&platform_device->dev, 1);
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

/**
 * Unregister signal and remove signal handler.
 */
uint8_t smc_signal_handler_unregister( smc_t* smc_instance, smc_signal_t* signal, smc_channel_t* smc_channel )
{
    uint8_t               ret_value      = SMC_OK;
    smc_signal_handler_t* signal_handler = NULL;

    if( signal != NULL )
    {
        void*         dev_id       = NULL;

        signal_handler = smc_signal_handler_get(signal->interrupt_id, signal->signal_type);

        if( signal->signal_type == SMC_SIGNAL_TYPE_INTCBB )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: signal: 0x%08X: is SMC_SIGNAL_TYPE_INTCBB", (uint32_t)signal);

            free_irq((int)signal->interrupt_id, dev_id);

        }
        else if( signal->signal_type == SMC_SIGNAL_TYPE_INT_WGM_GENOUT )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: signal: 0x%08X: is SMC_SIGNAL_TYPE_INT_WGM_GENOUT", (uint32_t)signal);

            free_irq((int)signal->interrupt_id, dev_id);
        }
        else if( signal->signal_type == SMC_SIGNAL_TYPE_INT_RESOURCE )
        {
            struct resource*        res             = NULL;
            struct platform_device* platform_device = NULL;

            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: signal: 0x%08X: is SMC_SIGNAL_TYPE_INT_RESOURCE", (uint32_t)signal);

            if( smc_instance->smc_parent_ptr != NULL )
            {
                SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: parent object 0x%08X found, extract the platform device", smc_instance->smc_parent_ptr);
                platform_device = (struct platform_device*)smc_instance->smc_parent_ptr;
            }

            if( platform_device )
            {
                int     irq_spi = (int)signal->interrupt_id;
                int     res_id  = 0;
                uint8_t found   = FALSE;

                while( (res = platform_get_resource(platform_device, IORESOURCE_IRQ, res_id)) )
                {
                    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: check res id %d where IRQ ID %d...", res_id, res->start);

                    if( (res->start-SMC_APE_IRQ_OFFSET_INTCSYS_SPI) == irq_spi )
                    {
                        SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: res id %d: IRQ ID %d match to SPI %d...", res_id, res->start,irq_spi);
                        found = TRUE;
                        break;
                    }

                    res_id++;
                }

                if( found )
                {
                    dev_id    = platform_device;

                    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: found res 0x%08X, IRQ start is %d",
                            (uint32_t)signal, res, res->start );

                    free_irq( res->start, dev_id );
                }
                else
                {
                    SMC_TRACE_PRINTF_ERROR("smc_signal_handler_unregister: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: platform_get_resource with irq id %d FAILED", (uint32_t)signal, signal->interrupt_id);
                }
            }
            else
            {
                SMC_TRACE_PRINTF_ERROR("smc_signal_handler_unregister: signal: SMC_SIGNAL_TYPE_INT_RESOURCE 0x%08X: No device, unable to get IRQ %d", (uint32_t)signal, signal->interrupt_id);
            }
        }
        else
        {
            SMC_TRACE_PRINTF_ERROR("smc_signal_handler_unregister: signal: 0x%08X type 0x%08X not supported", (uint32_t)signal, signal->signal_type);
            ret_value      = SMC_ERROR;
        }

        if( signal_handler != NULL )
        {
            SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: remove signal handler...");
            smc_signal_handler_remove_and_destroy( signal_handler );
        }
    }

    SMC_TRACE_PRINTF_SIGNAL("smc_signal_handler_unregister: completed");

    return ret_value;
}


void smc_signal_dump( char* indent, smc_signal_t* signal )
{
    if( signal != NULL )
    {
        SMC_TRACE_PRINTF_ALWAYS("%sSignal 0x%08X: ID %d, type 0x%08X, address: 0x%08X (%s)", indent, (uint32_t)signal,
                                                                        signal->interrupt_id,
                                                                        signal->signal_type,
                                                                        (uint32_t)signal->peripheral_address,
                                                                        signal->address_remapped?"remapped":"not remapped");
    }
    else
    {
        SMC_TRACE_PRINTF_ALWAYS("%s<Signal not initialized>", indent);
    }
}


/* =============================================================
 * SMC locking function platform specific implementation
 * The lock struct defined in smc_conf_platform.h
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
 * SMC timer functions platform specific implementations
 * The timer struct is defined in smc_conf_platform.h
 *
 */

smc_timer_t* smc_timer_create( uint32_t timer_usec )
{
    smc_timer_t* timer = (smc_timer_t*)SMC_MALLOC( sizeof( smc_timer_t ) );

    assert(timer != NULL );

    timer->period_us      = timer_usec;
    timer->timer_data     = 0;
    timer->smc_timer_list = NULL;
    timer->prev_jiffies   = 0;

    SMC_TRACE_PRINTF_TIMER("smc_timer_create: timer 0x%08X created, timeout is %u usec", (uint32_t)timer, timer_usec);

    return timer;
}

uint8_t smc_timer_start( smc_timer_t* timer, smc_timer_callback* timer_cb, uint32_t timer_data )
{
    uint8_t ret_val  = SMC_OK;

    unsigned long period  = (timer->period_us * HZ)/(1000*1000);
    //unsigned long jiff    = jiffies;

    timer->timer_data = timer_data;

    if( timer->smc_timer_list == NULL )
    {
        SMC_TRACE_PRINTF_TIMER("smc_timer_start: timer 0x%08X CB 0x%08X create new timer list...", (uint32_t)timer, (uint32_t)timer_cb);
        //struct timer_list tmr_lst = TIMER_INITIALIZER( timer_cb, (jiff + period), ((uint32_t)timer) );
        //timer->smc_timer_list = &tmr_lst;

        timer->smc_timer_list = (struct timer_list*)SMC_MALLOC( sizeof(struct timer_list) );
        timer->smc_timer_list->data     = (unsigned long)timer;
        timer->smc_timer_list->function = (void*)timer_cb;
        timer->prev_jiffies             = jiffies;
        timer->smc_timer_list->expires  = timer->prev_jiffies + period;

        init_timer( timer->smc_timer_list );

        add_timer( timer->smc_timer_list );

        //wait_event_interruptible(data->wait, !data->loops);
    }
    else
    {
        SMC_TRACE_PRINTF_TIMER("smc_timer_start: timer 0x%08X CB 0x%08X: timer list created, modify it...", (uint32_t)timer, (uint32_t)timer_cb);

        timer->prev_jiffies = jiffies;
        mod_timer( timer->smc_timer_list, (timer->prev_jiffies + period) );
    }

    SMC_TRACE_PRINTF_TIMER("smc_timer_start: timer 0x%08X CB 0x%08X period %d us, expires: %lu , HZ: %d...",
                (uint32_t)timer, (uint32_t)timer_cb, timer->period_us, timer->smc_timer_list->expires, HZ);

    return ret_val;
}

uint8_t smc_timer_stop( smc_timer_t* timer )
{
    uint8_t ret_val = SMC_OK;

    SMC_TRACE_PRINTF_TIMER("smc_timer_stop: timer 0x%08X...", (uint32_t)timer);

    if( timer!= NULL && timer->smc_timer_list != NULL )
    {
        SMC_TRACE_PRINTF_DEBUG("smc_timer_stop: delete timer list 0x%08X...", (uint32_t)timer->smc_timer_list);
        del_timer( timer->smc_timer_list );
        SMC_TRACE_PRINTF_DEBUG("smc_timer_stop: free timer list ptr 0x%08X...", (uint32_t)timer->smc_timer_list);
        SMC_FREE( timer->smc_timer_list );
        timer->smc_timer_list = NULL;
    }
    else
    {
        SMC_TRACE_PRINTF_TIMER("smc_timer_stop: invalid timer parameter NULL");
        ret_val = SMC_ERROR;
    }

    return ret_val;
}

uint8_t smc_timer_destroy( smc_timer_t* timer )
{
    uint8_t ret_val = SMC_OK;

    SMC_TRACE_PRINTF_TIMER("smc_timer_destroy: timer 0x%08X...", (uint32_t)timer);

    if( timer!= NULL )
    {
        if( timer->smc_timer_list != NULL )
        {
            SMC_TRACE_PRINTF_TIMER("smc_timer_destroy: delete timer list 0x%08X...", (uint32_t)timer->smc_timer_list);
            del_timer( timer->smc_timer_list );
            SMC_TRACE_PRINTF_TIMER("smc_timer_destroy: free timer list ptr 0x%08X...", (uint32_t)timer->smc_timer_list);
            SMC_FREE( timer->smc_timer_list );
            timer->smc_timer_list = NULL;
        }

        SMC_TRACE_PRINTF_TIMER("smc_timer_destroy: free timer 0x%08X...", (uint32_t)timer);
        SMC_FREE( timer );
        timer = NULL;
    }
    else
    {
        SMC_TRACE_PRINTF_ERROR("smc_timer_destroy: invalid timer parameter NULL");
        ret_val = SMC_ERROR;
    }

    return ret_val;
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

    printk(KERNEL_DEBUG_LEVEL "\n");

    for( i = 0; i < length; i++ )
    {
        if(i%row_len == 0)
        {
            printk("0x%02X", data[i]);
        }
        else
        {
            printk(" 0x%02X", data[i]);
        }

        if( i > 0 && ( i%(row_len) == (row_len-1) || i >= length-1 ))
        {
            printk("\n");
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

/*
 * Reading ASIC version from the EOS2 ES10/ES20
 * TODO Own module required
 */
uint16_t smc_asic_version_get(void)
{
    uint8_t cpu_version = 0x00;
    unsigned int cccr, major, minor;

    cccr = readl(SMC_CCCR);

    major = ((cccr & 0xF0) >> 4) + 1;
    minor = cccr & 0x0F;

    cpu_version = ((major&0xFF)<<4) + (minor&0xFF);

    SMC_TRACE_PRINTF_DEBUG("ASIC version Renesas R-Mobile U2 ES%d.%d (0x%08X / 0x%04X)\n", major, minor, cccr, cpu_version);

    return cpu_version;
}



/* EOF */
