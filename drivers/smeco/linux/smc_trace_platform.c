/*
*   Smeco implementation specific for Linux Kernel.
*   Copyright © Renesas Mobile Corporation 2012. All rights reserved
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

Version:       1    18-Jul-2011     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif


#include "smc_common_includes.h"

#include "smc.h"
#include "smc_trace.h"


//#if( SMC_TRACES_PRINTF==TRUE )
#ifdef SMC_APE_RDTRACE_ENABLED


char* var_smc_init[1] = {"osTaskId"};
uint32_t var_size_smc_init[1] = {0xFFFF};
    /* Trace group: SMC_TRACE_GROUP */
smc_rdtrace_t trace_smc[3] =
{
    { TRA_SMC_INIT, "initialize", "%s%s; %s:%d\n", &var_smc_init, &var_size_smc_init },
    { TRA_SMC_CHANNEL_IN_SYNC, "channel is synchronized with remote", "%s%s; ", NULL, NULL },
    { TRA_SMC_CHANNEL_VERSION_INFO, "channel version info", "%s%s; ", NULL, NULL }
};

char* var_smc_send[4] = {"channelId","msgPtr","msgLength","userdata"};
uint32_t var_size_smc_send[4] = {0xFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
smc_rdtrace_t trace_smc_send[2] =
{
    { TRA_SMC_MESSAGE_SEND,     "Send",      "%s%s; %s:%d; %s:0x%08X; %s:%d; %s:0x%08X\n", &var_smc_send, &var_size_smc_send },
    { TRA_SMC_MESSAGE_SEND_END, "Send: end", "%s%s; %s:%d; %s:0x%08X; %s:%d; %s:0x%08X\n", &var_smc_send, &var_size_smc_send }
};


char* var_smc_receive[4]    = {"channelId","msgPtr","msgLength","userdata"};
char* var_smc_receive_cb[5] = {"channelId","msgPtr","translatedMsgPtr","msgLength", "userdata"};
uint32_t var_size_smc_receive[4] = {0xFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
uint32_t var_size_smc_receive_cb[5] = {0xFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

smc_rdtrace_t trace_smc_receive[3] =
{
    { TRA_SMC_MESSAGE_RECV,       "Recv",               "%s%s; %s:%d; %s:0x%08X; %s:%d; %s:0x%08X\n", &var_smc_receive, &var_size_smc_receive },
    { TRA_SMC_MESSAGE_RECV_TO_CB, "Recv: put callback", "%s%s; %s:%d; %s:0x%08X; %s:0x%08X; %s:%d; %s:0x%08X\n", &var_smc_receive_cb, &var_size_smc_receive_cb },
    { TRA_SMC_MESSAGE_RECV_END,   "Recv: end",          "%s%s; %s:%d; %s:0x%08X; %s:%d; %s:0x%08X\n", &var_smc_receive, &var_size_smc_receive }
};


char* var_smc_event[2] = {"channelId","eventId"};
uint32_t var_size_smc_event[2] = {0xFF,0xFFFFFFFF};
smc_rdtrace_t trace_smc_event[2] =
{
    { TRA_SMC_EVENT_SEND, "Event send", "%s%s; %s:%d; %s:0x%08X\n", &var_smc_event, &var_size_smc_event },
    { TRA_SMC_EVENT_RECV, "Event recv", "%s%s; %s:%d; %s:0x%08X\n", &var_smc_event, &var_size_smc_event }
};


char* var_smc_irq[2] = {"irqId","irqType"};
char* var_smc_signal[2] = {"signalPtr","signalId"};
uint32_t var_size_smc_irq[2] = {0xFFFFFFFF,0xFFFFFFFF};
smc_rdtrace_t trace_smc_irq[5] =
{
    { TRA_SMC_IRQ_START,         "IRQ:Start",        "%s%s; %s:%d; %s:0x%08X\n", &var_smc_irq,    &var_size_smc_irq },
    { TRA_SMC_IRQ_END,           "IRQ:End",          "%s%s; %s:%d; %s:0x%08X\n", &var_smc_irq,    &var_size_smc_irq },
    { TRA_SMC_SIGNAL_INTGEN,     "Signal:IntGen",    "%s%s; %s:0x%08X; %s:%d\n", &var_smc_signal, &var_size_smc_irq },
    { TRA_SMC_SIGNAL_INTGEN_OUT, "Signal:IntGenOut", "%s%s; %s:0x%08X; %s:%d\n", &var_smc_signal, &var_size_smc_irq },
    { TRA_SMC_SIGNAL_GOP001,     "Signal:GOP001",    "%s%s; %s:0x%08X; %s:%d\n", &var_smc_signal, &var_size_smc_irq }
};


char* var_smc_fifo_init[1] = {"fifoSize"};
uint32_t var_size_smc_fifo_init[1] = {0xFFFFFFFF};

char* var_smc_fifo_put[5]  = {"fifoPtr","data","length","flags","itemsInFifo"};
char* var_smc_fifo_get[5]  = {"fifoPtr","data","length","flags","itemsLeftInFifo"};
uint32_t var_size_fifo_put_get[5] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
char* var_smc_fifo_get_empty[5]  = {"fifoPtr","readIndex","writeIndex","readCount","writeCount"};

smc_rdtrace_t trace_smc_fifo[4] =
{
    { TRA_SMC_FIFO_INIT,      "initialize",     "%s%s; %s:%d;\n", &var_smc_fifo_init, &var_size_smc_fifo_init },
    { TRA_SMC_FIFO_PUT,       "fifo put",       "%s%s; %s:0x%08X; %s:0x%08X; %s:%d; %s:0x%08X; %s:%d\n", &var_smc_fifo_put, &var_size_fifo_put_get },
    { TRA_SMC_FIFO_GET,       "fifo get",       "%s%s; %s:0x%08X; %s:0x%08X; %s:%d; %s:0x%08X; %s:%d\n", &var_smc_fifo_get, &var_size_fifo_put_get },
    { TRA_SMC_FIFO_GET_EMPTY, "fifo get empty", "%s%s; %s:0x%08X; %s:%d; %s:%d; %s:%d; %s:%d\n", &var_smc_fifo_get_empty, &var_size_fifo_put_get }
};

char* var_smc_fifo_stat[4] = {"fifoPtr","writeCount","readCount","itemsInFifo"};
uint32_t var_size_smc_fifo_stat[4] = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

smc_rdtrace_t trace_smc_fifo_stat[2] =
{
    { TRA_SMC_FIFO_PUT_STATISTICS, "fifo put stat", "%s%s; %s:0x%08X; %s:%d; %s:%d; %s:%d\n", &var_smc_fifo_stat, &var_size_smc_fifo_stat },
    { TRA_SMC_FIFO_GET_STATISTICS, "fifo get stat", "%s%s; %s:0x%08X; %s:%d; %s:%d; %s:%d\n", &var_smc_fifo_stat, &var_size_smc_fifo_stat }
};


smc_rdtrace_group_t  trace_group_smc[SMC_TRACE_GROUP_COUNT] =
{
    {SMC_TRACE_GROUP,             "SMC:",           &trace_smc},
    {SMC_TRACE_GROUP_MSG_SEND,    "SMC:",           &trace_smc_send},
    {SMC_TRACE_GROUP_MSG_RECEIVE, "SMC:",           &trace_smc_receive},
    {SMC_TRACE_GROUP_EVENT,       "SMC:",           &trace_smc_event},
    {SMC_TRACE_GROUP_IRQ,         "SMC:",           &trace_smc_irq},
    {SMC_TRACE_GROUP_FIFO,        "SMC FIFO:",      &trace_smc_fifo},
    {SMC_TRACE_GROUP_FIFO_STAT,   "SMC FIFO STAT:", &trace_smc_fifo_stat}
};


    /* Trace group activation */
static uint32_t smc_trace_activation_group_0 = 0x00000000;

uint32_t get_smc_trace_activation_group(uint8_t bit_group_id)
{
    /* TODO Add some more bit groups if required */

    return smc_trace_activation_group_0;
}

void smc_rd_trace_group_activate(uint32_t trace_group_id, uint8_t activate)
{
    if( activate )
    {
        smc_trace_activation_group_0 |= trace_group_id;
    }
    else
    {
        smc_trace_activation_group_0 &= ~trace_group_id;
    }
}

void smc_rd_trace_send1(uint32_t trace_id, uint32_t* ptr1)
{
    smc_rdtrace_group_t group = trace_group_smc[SMC_TRACE_ID_TO_GROUP_INDEX(trace_id)];

    if( group.trace_item_array != NULL )
    {
        smc_rdtrace_t trace = group.trace_item_array[trace_id&SMC_TRACE_ID_MASK];

        printk(trace.trace_var_format_str, group.trace_group_text, trace.trace_text,
               trace.trace_var_name_array[0], SMC_SHM_READ32(ptr1)&(trace.trace_var_size_array[0]) );

    }
}

void smc_rd_trace_send2(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2)
{
    smc_rdtrace_group_t group = trace_group_smc[SMC_TRACE_ID_TO_GROUP_INDEX(trace_id)];

    if( group.trace_item_array != NULL )
    {
        smc_rdtrace_t trace = group.trace_item_array[trace_id&SMC_TRACE_ID_MASK];

        printk(trace.trace_var_format_str, group.trace_group_text, trace.trace_text,
               trace.trace_var_name_array[0], SMC_SHM_READ32(ptr1)&(trace.trace_var_size_array[0]),
               trace.trace_var_name_array[1], SMC_SHM_READ32(ptr2)&(trace.trace_var_size_array[1]));
    }
}

void smc_rd_trace_send3(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2, uint32_t* ptr3)
{
    smc_rdtrace_group_t group = trace_group_smc[SMC_TRACE_ID_TO_GROUP_INDEX(trace_id)];

    if( group.trace_item_array != NULL )
    {
        smc_rdtrace_t trace = group.trace_item_array[trace_id&SMC_TRACE_ID_MASK];

        printk(trace.trace_var_format_str, group.trace_group_text, trace.trace_text,
               trace.trace_var_name_array[0], SMC_SHM_READ32(ptr1)&(trace.trace_var_size_array[0]),
               trace.trace_var_name_array[1], SMC_SHM_READ32(ptr2)&(trace.trace_var_size_array[1]),
               trace.trace_var_name_array[2], SMC_SHM_READ32(ptr3)&(trace.trace_var_size_array[2]));
    }
}

void smc_rd_trace_send4(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2, uint32_t* ptr3, uint32_t* ptr4)
{
    smc_rdtrace_group_t group = trace_group_smc[SMC_TRACE_ID_TO_GROUP_INDEX(trace_id)];

    if( group.trace_item_array != NULL )
    {
        smc_rdtrace_t trace = group.trace_item_array[trace_id&SMC_TRACE_ID_MASK];

        printk(trace.trace_var_format_str, group.trace_group_text, trace.trace_text,
               trace.trace_var_name_array[0], SMC_SHM_READ32(ptr1)&(trace.trace_var_size_array[0]),
               trace.trace_var_name_array[1], SMC_SHM_READ32(ptr2)&(trace.trace_var_size_array[1]),
               trace.trace_var_name_array[2], SMC_SHM_READ32(ptr3)&(trace.trace_var_size_array[2]),
               trace.trace_var_name_array[3], SMC_SHM_READ32(ptr4)&(trace.trace_var_size_array[3]));

    }
}

void smc_rd_trace_send5(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2, uint32_t* ptr3, uint32_t* ptr4, uint32_t* ptr5)
{
            smc_rdtrace_group_t group = trace_group_smc[SMC_TRACE_ID_TO_GROUP_INDEX(trace_id)];

            if( group.trace_item_array != NULL )
            {
                smc_rdtrace_t trace = group.trace_item_array[trace_id&SMC_TRACE_ID_MASK];

                printk(trace.trace_var_format_str, group.trace_group_text, trace.trace_text,
                       trace.trace_var_name_array[0], SMC_SHM_READ32(ptr1)&(trace.trace_var_size_array[0]),
                       trace.trace_var_name_array[1], SMC_SHM_READ32(ptr2)&(trace.trace_var_size_array[1]),
                       trace.trace_var_name_array[2], SMC_SHM_READ32(ptr3)&(trace.trace_var_size_array[2]),
                       trace.trace_var_name_array[3], SMC_SHM_READ32(ptr4)&(trace.trace_var_size_array[3]),
                       trace.trace_var_name_array[4], SMC_SHM_READ32(ptr5)&(trace.trace_var_size_array[4]));
            }
}

#endif
