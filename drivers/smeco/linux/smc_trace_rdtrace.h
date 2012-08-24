/*
*   Functions for Smeco RD traces to use in Linux Kernel
*
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

Version:       1    18-Jul-2012     Heikki Siikaluoma
Status:        draft
Description :  File created
-------------------------------------------------------------------------------
*/
#endif

#ifndef SMC_TRACE_RDTRACE_H
#define SMC_TRACE_RDTRACE_H


    /* Single trace line */
typedef struct _smc_rdtrace_t
{
    uint32_t           trace_id;
    char*              trace_text;
    char*              trace_var_format_str;
    char**             trace_var_name_array;
    uint32_t*          trace_var_size_array;

} smc_rdtrace_t;

typedef struct _smc_rdtrace_group_t
{
    uint32_t          trace_group_id;
    char*             trace_group_text;
    smc_rdtrace_t*    trace_item_array;


} smc_rdtrace_group_t;

#define SMC_TRACE_GROUP_MASK          0xF0000000
#define SMC_TRACE_ID_MASK             0x0FFFFFFF

    /* Trace groups available */
#define SMC_TRACE_GROUP               0x00000001    /* 0 */
#define SMC_TRACE_GROUP_MSG_SEND      0x00000002    /* 1 */
#define SMC_TRACE_GROUP_MSG_RECEIVE   0x00000004
#define SMC_TRACE_GROUP_EVENT         0x00000008
#define SMC_TRACE_GROUP_IRQ           0x00000010
#define SMC_TRACE_GROUP_FIFO          0x00000020
#define SMC_TRACE_GROUP_FIFO_STAT     0x00000040
#define SMC_TRACE_GROUP_LOCK          0x00000080
#define SMC_TRACE_GROUP_L2MUX         0x00000100
#define SMC_TRACE_GROUP_L2MUX_DL      0x00000200
#define SMC_TRACE_GROUP_L2MUX_UL      0x00000400


/* Trace IDs (same as used in modem) */

#define TRA_SMC_INIT                  0x00000000  /* Group 0 (MSB) in group index 0 */
#define TRA_SMC_CHANNEL_IN_SYNC       0x00000001
#define TRA_SMC_CHANNEL_VERSION_INFO  0x00000002

#define TRA_SMC_MESSAGE_SEND          0x10000000
#define TRA_SMC_MESSAGE_SEND_END      0x10000001

#define TRA_SMC_MESSAGE_RECV          0x20000000
#define TRA_SMC_MESSAGE_RECV_TO_CB    0x20000001
#define TRA_SMC_MESSAGE_RECV_END      0x20000002

#define TRA_SMC_EVENT_SEND            0x30000000
#define TRA_SMC_EVENT_RECV            0x30000001

#define TRA_SMC_IRQ_START             0x40000000
#define TRA_SMC_IRQ_END               0x40000001
#define TRA_SMC_SIGNAL_INTGEN         0x40000002
#define TRA_SMC_SIGNAL_INTGEN_OUT     0x40000003
#define TRA_SMC_SIGNAL_GOP001         0x40000004

#define TRA_SMC_FIFO_INIT             0x50000000
#define TRA_SMC_FIFO_PUT              0x50000001
#define TRA_SMC_FIFO_GET              0x50000002
#define TRA_SMC_FIFO_GET_EMPTY        0x50000003

#define TRA_SMC_FIFO_PUT_STATISTICS   0x60000000
#define TRA_SMC_FIFO_GET_STATISTICS   0x60000001

// 0x00000000 + (0x01<<group_id)

#define SMC_TRACE_ID_TO_GROUP_INDEX( trace_id )          ( ((uint8_t)(trace_id>>28))&0xF )
#define SMC_TRACE_ID_TO_GROUP_ID( trace_id )             (  0x00000000 + ( 0x01 << SMC_TRACE_ID_TO_GROUP_INDEX(trace_id) ) )
#define SMC_TRACE_GROUP_IS_ACTIVATED( trace_group_id )   ( (get_smc_trace_activation_group(0)&trace_group_id)==trace_group_id )

#define SMC_TRACE_GROUP_COUNT         7


#define RD_TRACE_SEND1(trace_id, sz1, ptr1)              if( SMC_TRACE_GROUP_IS_ACTIVATED( SMC_TRACE_ID_TO_GROUP_ID(trace_id)) ) { smc_rd_trace_send1(trace_id, ptr1); }
#define RD_TRACE_SEND2(trace_id, sz1, ptr1, sz2, ptr2)   if( SMC_TRACE_GROUP_IS_ACTIVATED( SMC_TRACE_ID_TO_GROUP_ID(trace_id)) ) { smc_rd_trace_send2(trace_id, ptr1, ptr2); }
#define RD_TRACE_SEND3(trace_id, sz1, ptr1, sz2, ptr2, sz3, ptr3)  if( SMC_TRACE_GROUP_IS_ACTIVATED( SMC_TRACE_ID_TO_GROUP_ID(trace_id)) ) { smc_rd_trace_send3(trace_id, ptr1, ptr2, ptr3); }
#define RD_TRACE_SEND4(trace_id, sz1, ptr1, sz2, ptr2, sz3, ptr3, sz4, ptr4)   if( SMC_TRACE_GROUP_IS_ACTIVATED( SMC_TRACE_ID_TO_GROUP_ID( trace_id )) ) { smc_rd_trace_send4(trace_id, ptr1, ptr2, ptr3, ptr4); }
#define RD_TRACE_SEND5(trace_id, sz1, ptr1, sz2, ptr2, sz3, ptr3, sz4, ptr4, sz5, ptr5) if( SMC_TRACE_GROUP_IS_ACTIVATED( SMC_TRACE_ID_TO_GROUP_ID( trace_id )) ) { smc_rd_trace_send5(trace_id, ptr1, ptr2, ptr3, ptr4, ptr5); }

uint32_t get_smc_trace_activation_group(uint8_t bit_group_id);

void smc_rd_trace_group_activate(uint32_t trace_group_id, uint8_t activate);

void smc_rd_trace_send1(uint32_t trace_id, uint32_t* ptr1);
void smc_rd_trace_send2(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2);
void smc_rd_trace_send3(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2, uint32_t* ptr3);
void smc_rd_trace_send4(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2, uint32_t* ptr3, uint32_t* ptr4);
void smc_rd_trace_send5(uint32_t trace_id, uint32_t* ptr1, uint32_t* ptr2, uint32_t* ptr3, uint32_t* ptr4, uint32_t* ptr5);

#endif
/* EOF */

