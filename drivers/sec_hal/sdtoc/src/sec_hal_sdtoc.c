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


/* ************************ HEADER (INCLUDE) SECTION *********************** */
#include "sec_hal_rt.h"
#include "sec_hal_sdtoc.h"
#include "sdtoc_data.h"
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


SECURED_DATA_TOC_ENTRY * sdtoc_root =NULL;

/* ****************************************************************************
** Function name      : sec_hal_sdtoc_area_init
** Description        : sets the sdtoc_root
** Parameters         : IN/--- struct mem_msg_area *msg_area
                        IN/--- unsigned long start
                        IN/--- unsigned long size
** Return value       : int
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/

int sec_hal_sdtoc_area_init(unsigned long start, unsigned long size)
    {
    sdtoc_root = (SECURED_DATA_TOC_ENTRY *) ioremap_nocache(start, size);
    if(sdtoc_root == NULL)
        {
        return 1;
        }
    else
        {
        return 0;
        }
    
    }

/* ****************************************************************************
** Function name      : sec_hal_sdtoc_read
** Description        : read data from toc to the given address.
** Parameters         : IN/--- uint32_t *input_data, object id for reading the toc
                        IN/--- void *output_data, pointer for returning read data
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/

uint32_t sec_hal_sdtoc_read(uint32_t *input_data, void *output_data)
{
    uint32_t sec_hal_status = SEC_HAL_RES_OK;
    uint32_t toc_item_size = 0;
    uint32_t object_id = 0;
    char * data_ptr;

    SEC_HAL_TRACE_ENTRY

    if (NULL == input_data )
    {
        SEC_HAL_TRACE_EXIT_INFO("!!null input_data, aborting!!")
        sec_hal_status = SEC_HAL_RES_PARAM_ERROR;
    }
    else
    {
        if ( copy_from_user(&object_id, input_data, sizeof(uint32_t)) )
        {
            sec_hal_status = SEC_HAL_RES_FAIL;
        }
        else
        {
            data_ptr = (char *) sdtoc_get_payload(object_id,&toc_item_size);
            if ( toc_item_size == 0 || copy_to_user(output_data, data_ptr, toc_item_size) )
            {
                sec_hal_status = SEC_HAL_RES_FAIL;
            }
        }
    }

    SEC_HAL_TRACE_EXIT
    return sec_hal_status;
}

void * sdtoc_get_payload(const uint32_t object_id, uint32_t * const p_length)
{
#define SDTOC_ROOT 0x47FE0000UL

  void * rv = 0;
  uint32_t sdtoc_index;
/*  SECURED_DATA_TOC_ENTRY * sdtoc_root = (SECURED_DATA_TOC_ENTRY *)SDTOC_ROOT;*/

/*    SECURED_DATA_TOC_ENTRY * sdtoc_root = (SECURED_DATA_TOC_ENTRY *) phys_to_virt((phys_addr_t)SDTOC_ROOT);*/
/*    sdtoc_root = (SECURED_DATA_TOC_ENTRY *) ioremap_nocache(SDTOC_ROOT,
0x4000);*/

    if(sdtoc_root == NULL)
    {
        SEC_HAL_TRACE_EXIT_INFO("!!sdtoc_root is NULL, aborting!!")
        return rv;
    }
  for(sdtoc_index = 0;
      sdtoc_index < SDTOC_MAX_ENTRIES;
      sdtoc_index++) {

    if (sdtoc_root[sdtoc_index].object_id == object_id) {

      rv = (void*)(((ptrdiff_t)sdtoc_root) + sdtoc_root[sdtoc_index].start);

      if (p_length) {
        * p_length = sdtoc_root[sdtoc_index].length;
      }
      break;
    }

    if (sdtoc_root[sdtoc_index].object_id == SDTOC_END_MARKER) {
      break;
    }

  }
  return rv;
}

/* ******************************** END ************************************ */

