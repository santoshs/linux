/* *********************************************************************** **
**                               Renesas                                   **
** *********************************************************************** */

/* *************************** COPYRIGHT INFORMATION ********************* **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2010-2012 Renesas Electronics Corp.                       **
** All rights reserved.                                                    **
** *********************************************************************** */


/* ************************ HEADER (INCLUDE) SECTION ********************* */
#include "sec_hal_rt.h"
#include "sec_hal_toc.h"
#include "toc_data.h"
#include "sec_hal_rt_trace.h"
#include <linux/string.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/completion.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <asm/io.h>


void * toc_get_payload(DATA_TOC_ENTRY * toc_root,const uint32_t object_id, uint32_t * const p_length)
    {
    void * rv = 0;
    uint32_t toc_index;
    SEC_HAL_TRACE_ENTRY();

    if(toc_root == NULL)
        {
        SEC_HAL_TRACE("!!toc_root is NULL, aborting!!");
        return rv;
        }

    for(toc_index = 0;
        toc_index < TOC_MAX_ENTRIES;
        toc_index++)
        {
        if (toc_root[toc_index].object_id == object_id)
            {
            rv = (void*)(((ptrdiff_t)toc_root) + toc_root[toc_index].start);

            if (p_length)
                {
                * p_length = toc_root[toc_index].length;
                }
            break;
            }

        if (toc_root[toc_index].object_id == TOC_END_MARKER)
            {
            break;
            }
        }
    SEC_HAL_TRACE_EXIT();
    return rv;
    }


const void * toc_put_payload(DATA_TOC_ENTRY * toc_root,
                               const uint32_t     object_id,
                               const void * const data,
                               const uint32_t     length,
                               const void ** const p_handle,
                               const uint32_t     reserve,
                               const uint32_t     offset)
    {
    SEC_HAL_TRACE_ENTRY();
    // Mutex

    DATA_TOC_ENTRY * item = get_new_toc_entry(toc_root,object_id);

    if (!item) 
        {
        return 0;
        }

    if (item->object_id == TOC_END_MARKER)
        {
        void *slot;
        slot = toc_get_free_slot(toc_root,length);
        memcpy(slot,data,length);
        item->object_id = object_id;
        item->start     = (uint32_t)((ptrdiff_t)slot - (ptrdiff_t)toc_root);
        item->length    = length;
        item->spare     = 0;
        }
    else
        {
        SEC_HAL_TRACE("!!object_id exists, aborting!!");
        return 0;
        }

    if (p_handle)
        {
        *p_handle = item;
        }

    SEC_HAL_TRACE_EXIT();
    }




// Prototype doxymented above
DATA_TOC_ENTRY * get_new_toc_entry(DATA_TOC_ENTRY * toc_root, uint32_t object_id)
    {
    uint32_t iterator;

    SEC_HAL_TRACE_ENTRY();

    for(iterator = 0;
        iterator < TOC_MAX_ENTRIES;
        iterator++)
        {

        if ((toc_root[iterator].object_id == TOC_END_MARKER) ||
            (toc_root[iterator].object_id == object_id))
            { // Note: data areas of duplicates are lost. This is to keep the implementation simple.
            return toc_root + iterator;
            }
        }

    SEC_HAL_TRACE_EXIT();

    return 0;
    }

void * toc_get_free_slot(DATA_TOC_ENTRY * toc_root, uint32_t size)
    {
    uint32_t iterator;
    uint32_t max_offset = TOC_MAX_ENTRIES * 16;
    void *slot;

    SEC_HAL_TRACE_ENTRY();

    for(iterator = 0;
        iterator < TOC_MAX_ENTRIES;
        iterator++)
        {
        SEC_HAL_TRACE("iterator: 0x%08x",iterator);
        if(toc_root[iterator].object_id == TOC_END_MARKER)
            {
            break;
            SEC_HAL_TRACE("end marker reached");
            }
        if(max_offset<=toc_root[iterator].start)
            {
            max_offset=toc_root[iterator].start + toc_root[iterator].length;
            SEC_HAL_TRACE("max_offset: 0x%08x",max_offset);
            }
        }

    slot = (void*)toc_root;

    slot = slot + max_offset;

    SEC_HAL_TRACE("max_offset: 0x%08x",max_offset);
    SEC_HAL_TRACE("toc_root: 0x%08x",toc_root);
    SEC_HAL_TRACE("return value: 0x%08x",slot);
    SEC_HAL_TRACE_EXIT();
    return slot;
    }


int toc_initialize(DATA_TOC_ENTRY * toc_root, uint32_t size)
    {
    uint32_t iterator;

    void *tmp;

    SEC_HAL_TRACE_ENTRY();

    memset(toc_root,TOC_END_MARKER,size);

    SEC_HAL_TRACE_EXIT();

    return 0;
    }


// EOF

/* ******************************** END ********************************** */

