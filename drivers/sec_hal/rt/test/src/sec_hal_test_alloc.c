/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                         **
** All rights reserved.                                                      **
** ************************************************************************* */

/* *********************** HEADER (INCLUDE) SECTION ************************ */
#include "sec_hal_test_alloc.h"
#include "sec_hal_rt_cmn.h"
#include "sec_hal_rt_trace.h"

#ifndef SEC_HAL_TEST_ISOLATION
#include <linux/ioport.h>
#include <linux/errno.h>
#include <asm/io.h>
#else
#include <errno.h>
#endif /* SEC_HAL_TEST_ISOLATION */


/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
/*!
 * RAM allocation area definitions
 */
/* head address of a RAM work area */
#define RAM_MEM_AREA_ADDR_START         SEC_HAL_MEM_RAM_MSG_AREA_ADDR_START
/* size of a RAM work area */
#define RAM_MEM_AREA_SIZE               SEC_HAL_MEM_RAM_MSG_AREA_SIZE
/* size of a single memory block */
#define RAM_MEM_AREA_BLOCK_SIZE         32
/* number of max blocks possible */
#define RAM_MEM_AREA_MAX_BLOCKS         (RAM_MEM_AREA_SIZE/RAM_MEM_AREA_BLOCK_SIZE)


/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */
/** Message Area Management Table */
typedef struct _sec_hal_msg_area_t
{
    void* addr;        /* the head address of the message area */
    uint8_t size;      /* the size of allocation */
    uint8_t allocated; /* status of the block */
} sec_hal_msg_area_t;

/* Keep message area information */
static sec_hal_msg_area_t g_sec_hal_msg_area_alloc_info[RAM_MEM_AREA_MAX_BLOCKS];

/* Message area block allocation count */
static uint32_t g_sec_hal_msg_area_block_cnt = 0;


/* ***************************** CODE SECTION ****************************** */
void sec_hal_alloc_memset(uint8_t *buff, uint8_t data, uint32_t cnt);
void sec_hal_alloc_memset(uint8_t *buff, uint8_t data, uint32_t cnt)
{
    while(cnt > 0)
    {
        *buff++ = (uint32_t)data;
        cnt--;
    }
}

/* ****************************************************************************
** Function name      : sec_hal_msg_area_init
** Description        : Initialize msg area memory blocks.
**                    : SEC_HAL_ICRAM0_MEM_AREA_BLOCK_SIZE defines the size.
** Parameters         : buf_ptr     start address
**                      buf_sz      size of the block
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
int sec_hal_msg_area_init(void *buf_ptr, unsigned int buf_sz)
{
    int ret = 0;
    uint32_t size = 0, block_cnt = 0, index = 0;
    void* base_ptr = NULL;

    SEC_HAL_TRACE_ENTRY

#ifdef SEC_HAL_TEST_ISOLATION
    base_ptr = buf_ptr;
    size = buf_sz;
#else /* SEC_HAL_TEST_ISOLATION */
    if(!request_mem_region((resource_size_t)RAM_MEM_AREA_ADDR_START,
                           (resource_size_t)RAM_MEM_AREA_SIZE,
                           "sec_hal icram msg area"))
    {
        ret = -ENODEV;
        goto e1;
    }
    base_ptr = ioremap_nocache((unsigned long)RAM_MEM_AREA_ADDR_START,
                               (unsigned long)RAM_MEM_AREA_SIZE);
    size = RAM_MEM_AREA_SIZE;
#endif /* SEC_HAL_TEST_ISOLATION */

    if(NULL == base_ptr)
    {
        ret = -EINVAL;
        goto e2;
    }
    
    block_cnt = size/RAM_MEM_AREA_BLOCK_SIZE;
    if(RAM_MEM_AREA_MAX_BLOCKS < block_cnt)
    {
        ret = -E2BIG;
        goto e2;
    }

    SEC_HAL_TRACE_INT("block_cnt", block_cnt)
    g_sec_hal_msg_area_block_cnt = block_cnt;

    /* slice memory to a proper size blocks */
    while(index < block_cnt)
    {
        g_sec_hal_msg_area_alloc_info[index].addr = base_ptr;
        g_sec_hal_msg_area_alloc_info[index].size = 0;
        g_sec_hal_msg_area_alloc_info[index].allocated = FALSE;
        base_ptr += RAM_MEM_AREA_BLOCK_SIZE;
        index++;
    }

    SEC_HAL_TRACE_EXIT
    return 0;

#ifdef SEC_HAL_TEST_ISOLATION
e2: /* NOP */
e1: /* NOP */
#else
e2: iounmap(base_ptr);
e1: release_mem_region(RAM_MEM_AREA_ADDR_START, RAM_MEM_AREA_SIZE);
#endif /* SEC_HAL_TEST_ISOLATION */
    SEC_HAL_TRACE_EXIT
    return ret;
}

/* ****************************************************************************
** Function name      : sec_hal_msg_area_exit
** Description        : Release msg area memory blocks.
** Parameters         :
** Return value       :
** ***************************************************************************/
void sec_hal_msg_area_exit(void)
{
    SEC_HAL_TRACE_ENTRY

#ifndef SEC_HAL_TEST_ISOLATION
    if(sec_hal_msg_area_alloc_count())
    {
        SEC_HAL_TRACE("!allocations still active, possible memleak!");
    }

    iounmap((void*)RAM_MEM_AREA_ADDR_START);
    release_mem_region(RAM_MEM_AREA_ADDR_START, RAM_MEM_AREA_SIZE);
#endif

    SEC_HAL_TRACE_EXIT
}

/* ****************************************************************************
** Function name      : sec_hal_msg_area_memcpy
** Description        : Copy a memory block from src to dst
** Parameters         : dst         destination memory
**                    : src         source memory
**                    : size        size of source buffer.
** Return value       : destination buffer
** ***************************************************************************/
unsigned long sec_hal_mem_msg_area_memcpy(void *dst, const void *src, unsigned long sz)
{
    char* dst8 = (char*)dst;
    char* src8 = (char*)src;

    while (sz--)
    {
        *dst8++ = *src8++;
    }

    return (unsigned long)dst;
}

/* ****************************************************************************
** Function name      : sec_hal_msg_area_calloc
** Description        : Allocate a memory block
** Parameters         : n           number of required memory.
**                      size        size of the required mem area.
** Return value       : Not NULL    top address of allocated message area
**                      NULL        message area could not be allocated
** ***************************************************************************/
void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz)
{
    int32_t index = 0; /* index for seek */
    int32_t block_index = 0; /* block index for seek */
    void* msg_area_addr = NULL; /* address of allocated memory */
    uint32_t real_size = 0; /* needed space for all objs */
    uint32_t block_cnt = 0; /* required number of mem blocks */
    uint32_t found = FALSE; /* loop end condition */

    SEC_HAL_TRACE_ENTRY

    real_size = n*sz;
    SEC_HAL_TRACE_INT("real_size", real_size)
    if (0 == real_size)
    {
        SEC_HAL_TRACE_EXIT_INFO("!!n*sz is zero, aborting!!")
        return NULL;
    }

    block_cnt = (real_size+RAM_MEM_AREA_BLOCK_SIZE-1)/RAM_MEM_AREA_BLOCK_SIZE;
    SEC_HAL_TRACE_INT("block_cnt", block_cnt)
    if (block_cnt > g_sec_hal_msg_area_block_cnt)
    {
        SEC_HAL_TRACE_EXIT_INFO("!!too big block count, aborting!!")
        return NULL;
    }

    /* seek big enough unallocated mem area */
    while (index < g_sec_hal_msg_area_block_cnt)
    {
        if (FALSE == g_sec_hal_msg_area_alloc_info[index].allocated)
        {
            found = TRUE;
            block_index = block_cnt - 1;
            while (block_index > 0 &&
                   (index+block_index) < g_sec_hal_msg_area_block_cnt)
                {
                found = (found &&
                         FALSE == g_sec_hal_msg_area_alloc_info[index+block_index].allocated);
                block_index--;
                }

            /* check if the loop can be ended */
            if (TRUE == found)
            {
                break;
            }
        }
        index++;
    }

    /* return ptr to first block, update allocation info & zero initialize */
    if (TRUE == found &&
        index < g_sec_hal_msg_area_block_cnt)
    {
        /* allocated found message area */
        msg_area_addr = g_sec_hal_msg_area_alloc_info[index].addr;
        g_sec_hal_msg_area_alloc_info[index].size = block_cnt;
        g_sec_hal_msg_area_alloc_info[index].allocated = TRUE;
        sec_hal_alloc_memset(msg_area_addr, 0, RAM_MEM_AREA_BLOCK_SIZE);
        block_cnt--;
    }
    else
    {
        SEC_HAL_TRACE("!!big enough block not found!!")
    }

    /* also update allocation info for rest of the blocks & zero initialize */
    while (TRUE == found &&
           block_cnt > 0 &&
           (index + block_cnt) < g_sec_hal_msg_area_block_cnt)
    {
        g_sec_hal_msg_area_alloc_info[index+block_cnt].allocated = TRUE;
        sec_hal_alloc_memset(g_sec_hal_msg_area_alloc_info[index+block_cnt].addr,
                0, RAM_MEM_AREA_BLOCK_SIZE);
        block_cnt--;
    }

    SEC_HAL_TRACE_EXIT
    return msg_area_addr; /* return allocated memory address */
}

/* ****************************************************************************
** Function name      : sec_hal_msg_area_free
** Description        : Free allocated message area for the secure service message.
** Parameters         : address   address of message area should be freed.
** Return value       : None
** ***************************************************************************/
void sec_hal_mem_msg_area_free(void *address)
{
    int32_t index = 0;  /* index for seek */
    int32_t block_index = 0; /* block index for seek */

    SEC_HAL_TRACE_ENTRY

    if(NULL == address)
    {
        SEC_HAL_TRACE_EXIT_INFO("!!NULL address, aborting!!")
        return;
    }

    while (index < g_sec_hal_msg_area_block_cnt)
    {
        if(g_sec_hal_msg_area_alloc_info[index].addr == address)
        {
            block_index = g_sec_hal_msg_area_alloc_info[index].size - 1;

            /* free allocated message area */
            g_sec_hal_msg_area_alloc_info[index].size = 0;
            g_sec_hal_msg_area_alloc_info[index].allocated = FALSE;

            /* free rest of the blocks */
            while(0 < block_index &&
                  (index+block_index) < g_sec_hal_msg_area_block_cnt)
            {
                g_sec_hal_msg_area_alloc_info[index+block_index].size = 0;
                g_sec_hal_msg_area_alloc_info[index+block_index].allocated = FALSE;
                block_index--;
            }
            break; /* terminate the seek-loop */
        }
        index++;/* seek next mem block */
    }

    SEC_HAL_TRACE_EXIT
}

/* ****************************************************************************
** Function name      : sec_hal_msg_area_alloc_count
** Description        : Returns the number of allocated blocks.
** Return value       : The number of allocated blocks
** ***************************************************************************/
unsigned int sec_hal_msg_area_alloc_count(void)
{
    uint32_t index = 0, count = 0;

    while (index < g_sec_hal_msg_area_block_cnt)
    {
        if (TRUE == g_sec_hal_msg_area_alloc_info[index].allocated)
        {
            count++;
        }
        index++;
    }

    return count;
}

/* ******************************** END ************************************ */

