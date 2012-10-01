/*
 * rtds_memory_drv_sub.c
 *	 RT domain shared memory driver function file.
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/outercache.h>

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/semaphore.h>
#include <linux/kernel.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/backing-dev.h>
#include <linux/gfp.h>
#include <linux/version.h>


#include "log_kernel.h"
#include "rtds_memory_drv.h"
#include "rtds_memory_drv_common.h"
#include "rtds_memory_drv_main.h"
#include "rtds_memory_drv_sub.h"
#include "rtds_memory_drv_private.h"
#include "system_memory.h"
#include "iccom_drv.h"

/*** define ***/
#if ((LINUX_VERSION_CODE) < 0x00030000)
#define RTDS_PTE_OFFSET		(-PTRS_PER_PTE)
#else
#define RTDS_PTE_OFFSET		(PTRS_PER_PTE)
#endif

#define RTDS_MEM_PROC_CNT_MAX 10

extern spinlock_t		g_rtds_memory_lock_recv_queue;
extern struct list_head	g_rtds_memory_list_rcv_event;
extern struct semaphore	g_rtds_memory_apmem_rttrig_sem;
extern struct completion	g_rtds_memory_completion;
extern void				*g_rtds_memory_iccom_handle;
extern spinlock_t		g_rtds_memory_lock_cache_all;
extern rtds_memory_section_info	g_rtds_memory_section_info;		/* Section info */
extern spinlock_t				g_rtds_memory_lock_mpro;				/* SpinLock(Mpro) */
extern struct list_head		g_rtds_memory_list_mpro;				/* List head(Mpro) */
extern struct semaphore		g_rtds_memory_mpro_sem;					/* Semaphore(Mpro) */

extern struct list_head		g_rtds_memory_list_shared_mem;
extern struct semaphore		g_rtds_memory_shared_mem;
extern struct semaphore		g_rtds_memory_phy_mem;

static struct vm_operations_struct	g_rtds_memory_vm_ops = {
			.close = rtds_memory_drv_close_vma,
};

static rtds_memory_phymem_table phy_mem   = {
	.phy_addr	   = 0,
	.map_size	   = 0,
	.rt_addr		= 0
};

extern struct list_head	g_rtds_memory_list_create_mem;
extern spinlock_t		g_rtds_memory_lock_create_mem;

/**** prototype ****/
static unsigned long rtds_memory_tablewalk(unsigned long virt_addr);

/*******************************************************************************
 * Function   : rtds_memory_drv_init_mpro
 * Description: This function initialises mpro and registers asynchronous event to RT domain.
 * Parameters : fp			- file descriptor
 * Returns	  : SMAP_OK		- Success
 *				SMAP_NG		- Fatal error
 *				Others		- System error/ result of RT domain
 *******************************************************************************/
int rtds_memory_drv_init_mpro(
	struct file			*fp
)
{
	int							ret;
	iccom_drv_send_cmd_param	send_cmd;
	rtds_memory_drv_data		*data_p = fp->private_data;
	struct {
		unsigned long info1;
		unsigned long info2;
	} send_data;		 /* Send data */

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |fp	[0x%08X]\n", (u32)fp);

	/* Set page gloval drectory address */
	data_p->mpro_control->mpro_data.pgd_phy_addr = virt_to_phys(current->mm->pgd);
	/* Set context ID */
	data_p->mpro_control->mpro_data.context_id = current->mm->context.id;

	MSG_MED("[RTDSK]   |pgd_phy_addr [0x%08X]\n", (u32)data_p->mpro_control->mpro_data.pgd_phy_addr);
	MSG_MED("[RTDSK]   |context_id   [0x%08X]\n", (u32)data_p->mpro_control->mpro_data.context_id);

	send_data.info1 = data_p->mpro_control->mpro_data.pgd_phy_addr;
	send_data.info2 = data_p->mpro_control->mpro_data.context_id;

	send_cmd.handle      = g_rtds_memory_iccom_handle;
	send_cmd.task_id     = TASK_MEMORY_SUB;
	send_cmd.function_id = EVENT_MEMORYSUB_INITAPPSHAREDMEM;
	send_cmd.send_mode   = ICCOM_DRV_SYNC;
	send_cmd.send_size   = sizeof(send_data);
	send_cmd.send_data   = (unsigned char *)(&send_data);
	send_cmd.recv_size   = 0;
	send_cmd.recv_data   = NULL;

	/* send EVENT_MEMORYSUB_INITAPPSHAREDMEM */
	ret = iccom_drv_send_command(&send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| iccom_drv_send_command error[EVENT_MEMORYSUB_INITAPPSHAREDMEM]\n");
		MSG_HIGH("[RTDSK]OUT|[%s]ret = %d\n", __func__, ret);
		return ret;
	}

	MSG_LOW("[RTDSK]   |Send [EVENT_MEMORYSUB_INITAPPSHAREDMEM]\n");

	/*
	 *   Register asynchronous event to RT domain
	 */

	/* send EVENT_MEMORY_OPENAPPMEMORY
	 * It does not change send_command parameter as follows.
	 *  send_cmd.handle	  = g_rtds_memory_iccom_handle;
	 *  send_cmd.recv_size   = 0;
	 *  send_cmd.recv_data   = NULL;
	 */
	send_cmd.task_id	 = TASK_MEMORY;
	send_cmd.function_id = EVENT_MEMORY_OPENAPPMEMORY;
	send_cmd.send_mode   = ICCOM_DRV_ASYNC;
	send_cmd.send_size   = 0;
	send_cmd.send_data   = NULL;

	ret = iccom_drv_send_command(&send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| iccom_drv_send_command error[EVENT_MEMORY_OPENAPPMEMORY]\n");
		MSG_HIGH("[RTDSK]OUT|[%s]ret = %d\n", __func__, ret);
		return ret;
	}

	MSG_LOW("[RTDSK]   |Send [EVENT_MEMORY_OPENAPPMEMORY]\n");


	/* send EVENT_MEMORY_CLOSEAPPMEMORY
	 * It does not change send_command parameter as follows.
	 *  send_cmd.handle	     = g_rtds_memory_iccom_handle;
	 *  send_cmd.task_id	 = TASK_MEMORY;
	 *  send_cmd.send_mode   = ICCOM_DRV_ASYNC;
	 *  send_cmd.send_size   = 0;
	 *  send_cmd.send_data   = NULL;
	 *  send_cmd.recv_size   = 0;
	 *  send_cmd.recv_data   = NULL;
	 */
	send_cmd.function_id = EVENT_MEMORY_CLOSEAPPMEMORY;
	ret = iccom_drv_send_command(&send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| iccom_drv_send_command error[EVENT_MEMORY_CLOSEAPPMEMORY]\n");
	} else {
		MSG_LOW("[RTDSK]   |Send [EVENT_MEMORY_CLOSEAPPMEMORY]\n");
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret = %d\n", __func__, ret);

	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_init_data
 * Description: This function initialises RTDS memory handle info.
 * Parameters : handle  - RTDS memory handle info
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_init_data(
	rtds_memory_drv_handle *handle
)
{

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	handle->var_app_addr	= 0;
	handle->var_rt_addr_nc  = KERNEL_ADDRESS_MASK(g_rtds_memory_section_info.var_address);
	handle->var_kernel_addr = (unsigned long)g_rtds_memory_section_info.kernel_var_addr;
	handle->var_addr_size   = g_rtds_memory_section_info.var_length;
	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)handle->var_app_addr);
	MSG_MED("[RTDSK]   |rt_addr[0x%08X]\n", (u32)handle->var_rt_addr_nc);
	MSG_MED("[RTDSK]   |kernel_addr[0x%08X]\n", (u32)handle->var_kernel_addr);
	MSG_MED("[RTDSK]   |var_size[0x%08X]\n", handle->var_addr_size);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}
/*******************************************************************************
 * Function   : rtds_memory_ioctl_init_data
 * Description: This function initialises RTDS memory handle info(ioctl).
 * Parameters : fp			-   File descriptor
 *				buffer		-   user data
 *				buf_size	-   user data size
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_NG		-   Fatal error
 *******************************************************************************/
int rtds_memory_ioctl_init_data(
	struct file		*fp,
	char __user		*buffer,
	size_t			buf_size
)
{
	int						ret = SMAP_OK;
	rtds_memory_init_info	init_info;
	rtds_memory_drv_data	*data_p = fp->private_data;
	unsigned long			app_addr;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Map variable area */
	ret = rtds_memory_open_shared_rtmem(g_rtds_memory_section_info.var_address,
										g_rtds_memory_section_info.var_length,
										RTDS_MEMORY_DRV_NONCACHE,
										&(data_p->mem_map),
										fp,
										&app_addr);

	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_open_shared_rtmem failed[%d]\n", ret);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = SMAP_NG\n", __func__);
		return SMAP_NG;

	}

	init_info.app_address	= app_addr;
	init_info.rt_address	= KERNEL_ADDRESS_MASK(g_rtds_memory_section_info.var_address);
	init_info.var_size		= g_rtds_memory_section_info.var_length;
	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)init_info.app_address);
	MSG_MED("[RTDSK]   |rt_addr[0x%08X]\n", (u32)init_info.rt_address);
	MSG_MED("[RTDSK]   |var_size[0x%08X]\n", init_info.var_size);

	data_p->var_app_addr	= app_addr;
	data_p->var_rt_nc_addr  = KERNEL_ADDRESS_MASK(g_rtds_memory_section_info.var_address);
	data_p->var_kernel_addr = (unsigned long)g_rtds_memory_section_info.kernel_var_addr;
	data_p->var_area_len	= g_rtds_memory_section_info.var_length;

	/* Copy into user space */
	if (copy_to_user(buffer, &init_info, buf_size)) {
		(void)rtds_memory_close_shared_rtmem(data_p->var_app_addr, data_p->var_area_len);
		MSG_ERROR("[RTDSK]ERR| copy_to_user failed\n");
		data_p->var_app_addr	= 0;
		data_p->var_rt_nc_addr  = 0;
		data_p->var_kernel_addr = 0;
		data_p->var_area_len	= 0;
		ret = SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_check_shared_apmem
 * Description: This function controls App shared memory by corresponding event.
 * Parameters : fp			-   File descriptor
 *				map_data	-   Memory mapping information
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_check_shared_apmem(
	struct file					*fp,
	rtds_memory_mapping_data	*map_data
)
{
	int								ret;
	unsigned long					flag;
	rtds_memory_app_memory_table	*mem_table = NULL;
	unsigned int					page_num;
	unsigned int					apmem_id;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Get semaphore */
	down(&g_rtds_memory_mpro_sem);

	spin_lock_irqsave(&g_rtds_memory_lock_mpro, flag);

	/* Check empty */
	if (0 != list_empty(&g_rtds_memory_list_mpro)) {
		spin_unlock_irqrestore(&g_rtds_memory_lock_mpro, flag);
		MSG_ERROR("[RTDSK]ERR| List is empty.\n");
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return;
	}

	/* Search and entry list */
	list_for_each_entry(mem_table, &g_rtds_memory_list_mpro, list_head_mpro)
		break;

	/* Delete list */
	list_del(&mem_table->list_head_mpro);
	spin_unlock_irqrestore(&g_rtds_memory_lock_mpro, flag);

	MSG_LOW("[RTDSK]   |event	 [0x%08X]\n", (u32)mem_table->event);
	MSG_LOW("[RTDSK]   |mem_size  [0x%08X]\n", (u32)mem_table->memory_size);

	switch (mem_table->event) {
	case RTDS_MEM_RT_CREATE_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_RT_CREATE_EVENT\n");

		/* Create page frame */
		page_num = RTDS_MEM_GET_PAGE_NUM(mem_table->memory_size);
		MSG_LOW("[RTDSK]   |page_num  [0x%08X]\n", page_num);

		/* Allocate page descriptor area */
		mem_table->pages = kmalloc(page_num * sizeof(struct page *), GFP_KERNEL);
		if (NULL == mem_table->pages) {
			MSG_ERROR("[RTDSK]ERR| kmalloc of page area failed.\n");
			rtds_memory_send_error_msg(mem_table, SMAP_MEMORY);
			MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
			return;
		}

		ret = rtds_memory_create_page_frame(page_num, mem_table->pages, NULL);
		if (SMAP_OK != ret) {
			kfree(mem_table->pages);
			MSG_ERROR("[RTDSK]ERR| rtds_memory_create_page_frame failed.\n");
			rtds_memory_send_error_msg(mem_table, ret);
			MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
			return;
		}

		/* Mapping */
		ret = rtds_memory_map_shared_apmem(fp, map_data, mem_table);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RTDSK]ERR| rtds_memory_map_shared_apmem ret[%d].\n", ret);
			rtds_memory_free_page_frame(page_num, mem_table->pages, NULL);
			kfree(mem_table->pages);
			rtds_memory_send_error_msg(mem_table, ret);
			MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
			return;
		}

		rtds_memory_drv_inv_cache(mem_table->rt_wb_addr, mem_table->memory_size);

		/* Create APP shared memory */
		ret = rtds_memory_send_open_shared_apmem(mem_table->rt_wb_addr,
													mem_table->rt_nc_addr,
													mem_table->memory_size,
													mem_table->app_addr,
													mem_table->error_code,
													0,
													&(mem_table->apmem_id));
		if (SMAP_OK != ret) {
			rtds_memory_free_page_frame(page_num, mem_table->pages, NULL);
			kfree(mem_table->pages);
			kfree(mem_table);
			MSG_ERROR("[RTDSK]ERR| rtds_memory_send_open_shared_apmem ret[%d].\n", ret);
			panic("Send error[%s][%d] err_code[%d]", __func__, __LINE__, ret);
		} else {

			down(&g_rtds_memory_shared_mem);
			list_add_tail(&(mem_table->list_head), &g_rtds_memory_list_shared_mem);
			up(&g_rtds_memory_shared_mem);
		}

		break;

	case RTDS_MEM_APP_DELETE_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_APP_DELETE_EVENT\n");

		break;

	case RTDS_MEM_RT_DELETE_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_RT_DELETE_EVENT\n");
		MSG_LOW("[RTDSK]   |apmem_id[0x%08X]\n", (u32)mem_table->apmem_id);

		/* Request RT domain to close shared memory */
		ret = rtds_memory_send_close_shared_apmem(mem_table->apmem_id);
		switch (ret) {
		case SMAP_OK:
			break;
		case SMAP_NG:
			MSG_MED("[RTDSK]   |Free API is not called.\n");
			break;
		default:
			MSG_ERROR("[RTDSK]ERR| rtds_memory_send_close_shared_apmem ret[%d].\n", ret);
			rtds_memory_free_page_frame(RTDS_MEM_GET_PAGE_NUM(mem_table->memory_size), mem_table->pages, NULL);
			kfree(mem_table->pages);
			kfree(mem_table);
			panic("Send error[%s][%d] err_code[%d]", __func__, __LINE__, ret);
		}

		/* Set delete apmem_id*/
		apmem_id = mem_table->apmem_id;

		/* Free memtable for Mpro event */
		kfree(mem_table);
		mem_table = NULL;

		/* Search for shared memory list */
		down(&g_rtds_memory_shared_mem);
		list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
			if (mem_table->apmem_id == apmem_id) {
				MSG_LOW("[RTDSK]   |found!apmem_id[0x%08X]\n", (u32)mem_table->apmem_id);
				break;
			}
		}

		if ((unsigned int)mem_table == (unsigned int)&g_rtds_memory_list_shared_mem) {
			MSG_ERROR("[RTDSK]ERR| List is not found.\n");
			up(&g_rtds_memory_shared_mem);
			panic("memory list error[%s][%d]", __func__, __LINE__);
			break;
		}

		list_del(&mem_table->list_head);
		up(&g_rtds_memory_shared_mem);

		MSG_LOW("[RTDSK]   |Delete process (RT trigger)\n");
		MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)mem_table->pages);
		MSG_MED("[RTDSK]   |rt_wb_addr[0x%08X]\n", (u32)mem_table->rt_wb_addr);
		MSG_MED("[RTDSK]   |rt_nc_addr[0x%08X]\n", (u32)mem_table->rt_nc_addr);

		/* Unmap write-back addr */
		if (0 != mem_table->rt_wb_addr) {
			/* Unmap */
			rtds_memory_do_unmap(mem_table->rt_wb_addr, mem_table->memory_size);
			mem_table->rt_wb_addr = 0;
		}

		/* Unmap non-cache addr */
		if (0 != mem_table->rt_nc_addr) {
			/* Unmap */
			rtds_memory_do_unmap(mem_table->rt_nc_addr, mem_table->memory_size);
			mem_table->rt_nc_addr = 0;
		}

		/* Free allocated page */
		page_num = RTDS_MEM_GET_PAGE_NUM(mem_table->memory_size);
		rtds_memory_free_page_frame(page_num, mem_table->pages, NULL);

		/* Release allocate memory */
		kfree(mem_table->pages);
		kfree(mem_table);

		break;

	case RTDS_MEM_MAP_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_MAP_EVENT\n");
		map_data->cache_kind	= RTDS_MEMORY_DRV_WRITEBACK;
		map_data->data_ent	  = false;
		map_data->mapping_flag  = RTDS_MEM_MAPPING_RTDOMAIN;
		map_data->mem_table	 = NULL;

		/* Map user space */
		ret = rtds_memory_do_map(fp,
								  &(mem_table->rt_wb_addr),
								  mem_table->memory_size,
								  mem_table->rt_nc_addr >> PAGE_SHIFT);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RTDSK]ERR| rtds_memory_do_map failed[%d]\n", ret);
			mem_table->error_code = RTDS_MEM_ERR_MAPPING;
		} else if (RTDS_MEM_ADDR_NG <= (mem_table->rt_wb_addr + mem_table->memory_size)) {
			/* Unmap */
			rtds_memory_do_unmap(mem_table->rt_wb_addr, mem_table->memory_size);
			mem_table->rt_wb_addr = 0;
			mem_table->error_code = RTDS_MEM_ERR_MAPPING;
		}

		up(&(mem_table->semaphore));

		break;

	case RTDS_MEM_UNMAP_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_UNMAP_EVENT\n");
		/* Unmap */
		rtds_memory_do_unmap(mem_table->rt_wb_addr, mem_table->memory_size);
		mem_table->rt_wb_addr = 0;
		up(&(mem_table->semaphore));
		break;

	case RTDS_MEM_MAP_PNC_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_MAP_PNC_EVENT\n");

		/* Mapping */
		ret = rtds_memory_map_shared_apmem(fp, map_data, mem_table);
		if (SMAP_OK == ret) {
			/* Create APP shared memory */
			ret = rtds_memory_send_open_shared_apmem(mem_table->rt_wb_addr, mem_table->rt_nc_addr,
													 mem_table->memory_size, 0, mem_table->error_code,
													 0, &(mem_table->apmem_id));
			if (SMAP_OK != ret) {
				panic("Send error[%s][%d] err_code[%d]", __func__, __LINE__, ret);
			}
		}

		up(&(mem_table->semaphore));

		break;

	case RTDS_MEM_UNMAP_PNC_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_UNMAP_PNC_EVENT\n");
		MSG_LOW("[RTDSK]   |rt_wb_addr[0x%08X]\n", (u32)mem_table->rt_wb_addr);
		MSG_LOW("[RTDSK]   |rt_nc_addr[0x%08X]\n", (u32)mem_table->rt_nc_addr);

		/* Request RT domain to close shared memory */
		ret = rtds_memory_send_close_shared_apmem(mem_table->apmem_id);
		switch (ret) {
		case SMAP_OK:
			break;
		case SMAP_NG:
			MSG_MED("[RTDSK]   |Free API is not called.\n");
			break;
		default:
			MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
			mem_table->error_code = ret;
			up(&(mem_table->semaphore));
			return;
		}

		/* Unmap write-back addr */
		if (0 != mem_table->rt_wb_addr) {
			/* Unmap */
			rtds_memory_do_unmap(mem_table->rt_wb_addr, mem_table->memory_size);
			mem_table->rt_wb_addr = 0;
		}

		/* Unmap non-cache addr */
		if (0 != mem_table->rt_nc_addr) {
			/* Unmap */
			rtds_memory_do_unmap(mem_table->rt_nc_addr, mem_table->memory_size);
			mem_table->rt_nc_addr = 0;
		}

		/* Release allocate memory */
		kfree(mem_table->pages);
		up(&(mem_table->semaphore));

		break;

	case RTDS_MEM_MAP_PNC_NMA_EVENT:
		MSG_MED("[RTDSK]   |RTDS_MEM_MAP_PNC_NMA_EVENT\n");
		map_data->cache_kind	= RTDS_MEMORY_DRV_WRITEBACK;
		map_data->data_ent	  = false;
		map_data->mapping_flag  = RTDS_MEM_MAPPING_APMEM;
		map_data->mem_table	 = mem_table;

		/* Mapping */
		ret = rtds_memory_do_map(fp,
								  &(mem_table->rt_wb_addr),
								  mem_table->memory_size,
								  0);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RTDSK]ERR| rtds_memory_do_map failed[%d]\n", ret);
			mem_table->error_code = RTDS_MEM_ERR_MAPPING;
		} else if (RTDS_MEM_ADDR_NG <= (mem_table->rt_wb_addr + mem_table->memory_size)) {
			/* Unmap */
			rtds_memory_do_unmap(mem_table->rt_wb_addr, mem_table->memory_size);
			mem_table->rt_wb_addr = 0;
			mem_table->error_code = RTDS_MEM_ERR_MAPPING;
		}

		up(&(mem_table->semaphore));

		break;

	default:
		break;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}


/*******************************************************************************
 * Function   : rtds_memory_open_shared_rtmem
 * Description: This function maps SHARED_RTMEM.
 * Parameters : phy_addr		-   Physical address
 *				map_size		-   Map size
 *				cache			-   cache type
 *				map_data		-   Memory mapping information
 *				fp				-   File pointer
 *				rt_addr			-   RT Logical address
 * Returns	  : SMAP_OK					-   Success
 *				RTDS_MEM_ERR_MAPPING	-   map error
 *******************************************************************************/
int rtds_memory_open_shared_rtmem(
	unsigned long				phy_addr,
	int							map_size,
	int							cache,
	rtds_memory_mapping_data	*map_data,
	struct file					*fp,
	unsigned long				*rt_addr
)
{
	int ret_code = SMAP_OK;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |phy_addr[0x%08X]\n", (u32)phy_addr);
	MSG_MED("[RTDSK]   |map_size[%d]\n", map_size);

	map_data->cache_kind	= cache;
	map_data->data_ent		= false;
	map_data->mapping_flag	= RTDS_MEM_MAPPING_RTMEM;

	down_write(&current->mm->mmap_sem);
	*rt_addr = do_mmap_pgoff(fp, 0, map_size, PROT_READ|PROT_WRITE,
			MAP_SHARED, phy_addr >> PAGE_SHIFT);
	up_write(&current->mm->mmap_sem);

	if (0 != (*rt_addr & RTDS_MEM_ADDR_ERR)) {
		ret_code = RTDS_MEM_ERR_MAPPING;
	}

	MSG_MED("[RTDSK]   |rt_addr[0x%08X]\n", (u32)*rt_addr);
	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_close_shared_rtmem
 * Description: This function unmaps SHARED_RTMEM.
 * Parameters : address		-   RT Logical address
 *				map_size	-   Map size
 * Returns	  : SMAP_OK		-   Success
 *******************************************************************************/
int rtds_memory_close_shared_rtmem(
	unsigned long	address,
	int				map_size
)
{
	int ret_code = SMAP_OK;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |address[0x%08X]\n", (u32)address);
	MSG_MED("[RTDSK]   |map_size[%d]\n", map_size);

	rtds_memory_do_unmap(address, (unsigned long)map_size);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_map_shared_memory
 * Description: This function performs memory mapping.
 * Parameters : map_data				-   Memory mapping information
 *				vm_area					-   VMA information
 * Returns	  : SMAP_OK					-   Success
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_map_shared_memory(
	rtds_memory_mapping_data	*map_data,
	struct vm_area_struct		*vm_area
)
{
	int				ret_code = SMAP_OK;
	int				i;
	unsigned long	vm_addr = vm_area->vm_start;
	unsigned int	page_num = RTDS_MEM_GET_PAGE_NUM(vm_area->vm_end - vm_area->vm_start);

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |page_num[0x%08X]\n", page_num);

	/* Setting CACHE attribute */
	switch (map_data->cache_kind) {
	case RTDS_MEMORY_DRV_WRITETHROUGH:
		vm_area->vm_page_prot = (vm_area->vm_page_prot & RTDS_MEM_CACHE_MASK) | RTDS_MEM_WRITE_THROUGH;
		break;
	case RTDS_MEMORY_DRV_NONCACHE:
		vm_area->vm_page_prot = (vm_area->vm_page_prot & RTDS_MEM_CACHE_MASK) | RTDS_MEM_NONCACHE;
		break;
	case RTDS_MEMORY_DRV_BUFFER_NONCACHE:
		vm_area->vm_page_prot = (vm_area->vm_page_prot & RTDS_MEM_CACHE_MASK) | RTDS_MEM_BUF_NONCACHE;
		break;
	default:
		vm_area->vm_page_prot = (vm_area->vm_page_prot & RTDS_MEM_CACHE_MASK) | RTDS_MEM_WRITE_BACK;
		break;
	}

	if (0 != map_data->data_ent) {
		vm_area->vm_ops = &g_rtds_memory_vm_ops;
	}

	switch (map_data->mapping_flag) {
	case RTDS_MEM_MAPPING_APMEM:

		MSG_MED("[RTDSK]   |RTDS_MEM_MAPPING_APMEM\n");
		for (i = 0; i < page_num; i++) {
			/* remap kernel memory to userspace() */
			ret_code = remap_pfn_range(vm_area,
										vm_addr,
										page_to_pfn(map_data->mem_table->pages[i]),
										PAGE_SIZE,
										vm_area->vm_page_prot);
			if (ret_code) {
				MSG_ERROR("[RTDSK]ERR| remap_pfn_range failed [%d]page.\n", i);
				MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
				return RTDS_MEM_ERR_MAPPING;
			}
			MSG_INFO("[RTDSK]   |pfn[0x%08X]\n", (u32)page_to_pfn(map_data->mem_table->pages[i]));
			/* mmu flush */
			rtds_memory_flush_mmu(vm_area, vm_addr);
			vm_addr += PAGE_SIZE;
		}
		break;

	case RTDS_MEM_MAPPING_RTMEM:
		/* continuous domain mapping */
		ret_code = remap_pfn_range(vm_area, vm_area->vm_start, vm_area->vm_pgoff,
				vm_area->vm_end - vm_area->vm_start, vm_area->vm_page_prot);
		if (ret_code) {
			MSG_ERROR("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
			return RTDS_MEM_ERR_MAPPING;
		}
		break;

	case RTDS_MEM_MAPPING_RTDOMAIN:
		/* continuous domain mapping */
		ret_code = remap_pfn_range(vm_area, vm_area->vm_start, vm_area->vm_pgoff,
				vm_area->vm_end - vm_area->vm_start, vm_area->vm_page_prot);

		if (ret_code) {
			MSG_ERROR("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
			return RTDS_MEM_ERR_MAPPING;
		}
		for (i = 0; i < page_num; i++) {
			rtds_memory_flush_mmu(vm_area, vm_addr);
			vm_addr += PAGE_SIZE;
		}
		break;

	default:
		panic("mapping_flag error[%s][%d]", __func__, __LINE__);
		break;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_open_kernel_shared_apmem
 * Description: This function creates App shared area(Kernel).
 * Parameters : rtds_memory_open_mem -   App shared memory create info
 * Returns	  : SMAP_OK				 -   Success
 *				SMAP_PARA_NG		 -   Parameter Error
 *				SMAP_MEMORY			 -   No memory
 *				RTDS_MEM_ERR_MAPPING -   Mapping error
 *******************************************************************************/
int rtds_memory_open_kernel_shared_apmem(
	rtds_memory_drv_open_mem_param	 *rtds_memory_open_mem
)
{
	int				ret;
	pgprot_t		protect = RTDS_MEMORY_PROTECT_WB;
	unsigned int	page_num = RTDS_MEM_GET_PAGE_NUM(rtds_memory_open_mem->mem_size);

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)rtds_memory_open_mem->mem_size);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)rtds_memory_open_mem->pages);
	MSG_MED("[RTDSK]   |page_num[0x%08X]\n", page_num);

	/* Parameter check */
	switch (rtds_memory_open_mem->app_cache) {
	case RTDS_MEMORY_DRV_WRITEBACK:
		protect = RTDS_MEMORY_PROTECT_WB;
		break;

	case RTDS_MEMORY_DRV_WRITETHROUGH:
		protect = RTDS_MEMORY_PROTECT_WT;
		break;

	case RTDS_MEMORY_DRV_NONCACHE:
		protect = RTDS_MEMORY_PROTECT_NC;
		break;

	case RTDS_MEMORY_DRV_BUFFER_NONCACHE:
		protect = RTDS_MEMORY_PROTECT_NCB;
		break;

	default:
		MSG_ERROR("[RTDSK]ERR| Cache type is illegal.\n");
		ret = SMAP_PARA_NG;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Allocate page descriptor area */
	rtds_memory_open_mem->pages = kmalloc(page_num * sizeof(struct page *), GFP_KERNEL);
	if (NULL == rtds_memory_open_mem->pages) {
		MSG_ERROR("[RTDSK]ERR| kmalloc of page area failed.\n");
		ret = SMAP_MEMORY;
		MSG_HIGH("[RTDSK]OUT|[%s] ret[%d]\n", __func__, ret);
		return ret;
	}

	/* Create page frame */
	ret = rtds_memory_create_page_frame(page_num, rtds_memory_open_mem->pages, NULL);
	if (SMAP_OK != ret) {
		kfree(rtds_memory_open_mem->pages);
		MSG_ERROR("[RTDSK]ERR| rtds_memory_create_page_frame failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* map an array of pages into virtually contiguous space */
	rtds_memory_open_mem->app_addr = (unsigned long)vmap(rtds_memory_open_mem->pages, page_num, VM_ALLOC, protect);
	if (0 == rtds_memory_open_mem->app_addr) {
		MSG_ERROR("[RTDSK]ERR| Mapping failed(vamalloc space).\n");
		ret = RTDS_MEM_ERR_MAPPING;
		/* free allocated page */
		rtds_memory_free_page_frame(page_num, rtds_memory_open_mem->pages, NULL);

		/* free allocated memory */
		kfree(rtds_memory_open_mem->pages);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	MSG_MED("[RTDSK]   |vmap.app_addr[0x%08X]\n", (u32)rtds_memory_open_mem->app_addr);

	/* Entry list to close App shared area */
	ret = rtds_memory_put_create_mem_list(rtds_memory_open_mem->app_addr,
											rtds_memory_open_mem->mem_size,
											rtds_memory_open_mem->pages,
											rtds_memory_open_mem->app_cache);
	if (SMAP_OK != ret) {
		vunmap((const void *)rtds_memory_open_mem->app_addr);
		/* free allocated page */
		rtds_memory_free_page_frame(page_num, rtds_memory_open_mem->pages, NULL);

		/* free allocated memory */
		kfree(rtds_memory_open_mem->pages);
		MSG_ERROR("[RTDSK]ERR| rtds_memory_put_create_mem_list failed[%d].\n", ret);
		MSG_HIGH("[RTDSK]OUT|[%s] ret[%d]\n", __func__, ret);
		return ret;
	}

	rtds_memory_drv_inv_cache(rtds_memory_open_mem->app_addr, rtds_memory_open_mem->mem_size);

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;

}
/*******************************************************************************
 * Function   : rtds_memory_close_kernel_shared_apmem
 * Description: This function close App shared memory.
 * Parameters : rtds_memory_close_mem   -   App shared memory destroy info
 * Returns	: SMAP_OK				 -   Success
 *			  SMAP_NG				 -   Fatal error
 *******************************************************************************/
int rtds_memory_close_kernel_shared_apmem(
	rtds_memory_drv_close_mem_param	*rtds_memory_close_mem
)
{
	int				ret = SMAP_OK;
	int				proc_cnt;
	unsigned int	page_num;
	unsigned int	mem_size;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)rtds_memory_close_mem->app_addr);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)rtds_memory_close_mem->pages);

	/* Unmap linear kernel address space set up */
	vunmap((const void *)rtds_memory_close_mem->app_addr);

	/* Get App shared memory size */
	proc_cnt = rtds_memory_get_create_mem_list(rtds_memory_close_mem->app_addr,
												*rtds_memory_close_mem->pages,
												&mem_size);
	switch (proc_cnt) {
	case 0:
		/* Page num */
		MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", mem_size);
		page_num = RTDS_MEM_GET_PAGE_NUM(mem_size);
		MSG_MED("[RTDSK]   |page_num[0x%08X]\n", page_num);

		/* Free page */
		rtds_memory_free_page_frame(page_num, rtds_memory_close_mem->pages, NULL);
		/* Release page descriptor area */
		kfree(rtds_memory_close_mem->pages);
		break;

	case -1:
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR| There is no list.\n");
		break;

	default:
		MSG_MED("[RTDSK]   |App shared memory has been used[%d]\n", proc_cnt);
		kfree(rtds_memory_close_mem->pages);
		break;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;

}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_open_apmem
 * Description: This function requests to open App shared memory(ioctl).
 * Parameters : fp			-   File descriptor
 *				buffer		-   user data
 *				buf_size	-   user data size
 *				map_data	-   mapping data
 * Returns	  : SMAP_OK					-   Success
 *				SMAP_NG					-   Fatal error
 *				SMAP_PARA_NG			-   Parameter error
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_ioctl_open_apmem(
	struct file					*fp,
	char __user					*buffer,
	size_t						buf_size,
	rtds_memory_mapping_data	*map_data
)
{
	int							ret = SMAP_PARA_NG;
	rtds_memory_drv_ioctl_mem	ioctl_mem;
	rtds_memory_apmem_info		apmem_info;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);
	MSG_MED("[RTDSK]   |map_data[0x%08X]\n", (u32)map_data);

	/* Parameter check */
	if (buf_size != sizeof(rtds_memory_drv_ioctl_mem)) {
		MSG_ERROR("[RTDSK]ERR| buf_size error [%d]\n", buf_size);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Copy user data into kernel space */
	ret = copy_from_user(&ioctl_mem, buffer, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_from_user failed[%d]\n", ret);
		ret = SMAP_NG;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)ioctl_mem.mem_size);
	MSG_MED("[RTDSK]   |app_cache[0x%08X]\n", (u32)ioctl_mem.app_cache);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)ioctl_mem.pages);

	/* Set parameter */
	apmem_info.app_addr		= 0;
	apmem_info.mem_size		= ioctl_mem.mem_size;
	apmem_info.app_cache	= ioctl_mem.app_cache;
	apmem_info.pages		= ioctl_mem.pages;

	/* Open App shared memory and get shared memory address and page descriptor info. */
	ret = rtds_memory_open_shared_apmem(fp, &apmem_info, map_data);
	if (SMAP_OK == ret) {
		MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)apmem_info.app_addr);
		MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)apmem_info.pages);
		ioctl_mem.app_addr	= apmem_info.app_addr;
		ioctl_mem.pages		= apmem_info.pages;
	} else {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_open_shared_apmem failed[%d]\n", ret);
	}

	ioctl_mem.err_code = ret;

	/* Copy into user space */
	ret = copy_to_user(buffer, &ioctl_mem, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_to_user failed[%d]\n", ret);
		ret = SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_open_shared_apmem
 * Description: This function create App shared memory by user trigger.
 * Parameters : fp			-   File descriptor
 *				mem_info	-   App shared memory info
 *				map_data	-   mapping data
 * Returns	  : SMAP_OK					-   Success
 *				SMAP_NG					-   Fatal error
 *				SMAP_PARA_NG			-   Parameter error
 *				SMAP_MEMORY				-   No memory
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_open_shared_apmem(
	struct file					*fp,
	rtds_memory_apmem_info		*mem_info,
	rtds_memory_mapping_data	*map_data
)
{
	int								ret = SMAP_MEMORY;
	rtds_memory_app_memory_table	*mem_table;
	unsigned int					page_num = RTDS_MEM_GET_PAGE_NUM(mem_info->mem_size);
	struct page						**k_pages;
	rtds_memory_create_queue		*create_list;
	unsigned long					flag;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (unsigned int)fp);
	MSG_MED("[RTDSK]   |mem_info[0x%08X]\n", (u32)mem_info);
	MSG_MED("[RTDSK]   |map_data[0x%08X]\n", (u32)map_data);
	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)mem_info->mem_size);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)mem_info->pages);

	/* leak check */
	rtds_memory_leak_check_page_frame();

	k_pages = kmalloc(page_num * sizeof(struct page *), GFP_KERNEL);
	if (NULL == k_pages) {
		MSG_ERROR("[RTDSK]ERR| kmalloc failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		kfree(k_pages);
		MSG_ERROR("[RTDSK]ERR| kmalloc failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	ret = copy_from_user(k_pages, mem_info->pages, page_num * sizeof(struct page *));
	if (0 != ret) {
		kfree(mem_table);
		kfree(k_pages);
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return ret;
	}

	create_list = kmalloc(sizeof(rtds_memory_create_queue), GFP_KERNEL);
	if (NULL == create_list) {
		kfree(k_pages);
		kfree(mem_table);
		MSG_ERROR("[RTDSK]ERR| kmalloc failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

	memset(create_list, 0, sizeof(rtds_memory_create_queue));
	create_list->pages		= k_pages;
	create_list->task_info	= current;
	create_list->app_cache	= mem_info->app_cache;
	list_add_tail(&create_list->queue_header, &g_rtds_memory_list_create_mem);

	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	/* Create page frame */
	ret = rtds_memory_create_page_frame(page_num, k_pages, create_list);
	if (SMAP_OK != ret) {

		spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
		list_del(&create_list->queue_header);
		kfree(k_pages);
		kfree(mem_table);
		kfree(create_list);
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

		MSG_ERROR("[RTDSK]ERR| rtds_memory_create_page_frame failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	create_list->page = *k_pages;
	ret = copy_to_user(mem_info->pages, k_pages, page_num * sizeof(struct page *));
	if (0 != ret) {
		rtds_memory_free_page_frame(page_num, k_pages, create_list);

		spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
		list_del(&create_list->queue_header);
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
		kfree(k_pages);
		kfree(mem_table);
		kfree(create_list);

		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR| copy_to_user failed[%d]\n", ret);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	mem_table->pages = mem_info->pages;
	MSG_MED("[RTDSK]   |mem_info->pages [0x%08X]\n", (u32)mem_info->pages);
	MSG_MED("[RTDSK]   |mem_table->pages [0x%08X]\n", (u32)mem_table->pages);

	/* Set map data */
	map_data->cache_kind	= mem_info->app_cache;
	map_data->data_ent		= true;
	map_data->mapping_flag	= RTDS_MEM_MAPPING_APMEM;
	map_data->mem_table		= mem_table;

	/* Map user space */
	ret = rtds_memory_do_map(fp, &(create_list->app_addr), mem_info->mem_size, 0);
	if (SMAP_OK != ret) {
		rtds_memory_free_page_frame(page_num, k_pages, create_list);
		spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
		list_del(&create_list->queue_header);
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
		kfree(k_pages);
		kfree(mem_table);
		kfree(create_list);
		MSG_ERROR("[RTDSK]ERR| rtds_memory_do_map failed[%d]\n", ret);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}
	MSG_MED("[RTDSK]   |app_addr [0x%08X]\n", (u32)mem_info->app_addr);

	mem_info->app_addr = create_list->app_addr;

	rtds_memory_drv_inv_cache(mem_info->app_addr, mem_info->mem_size);

	kfree(mem_table);

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;

}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_close_apmem(ioctl)
 * Description: This function close App shared memory(user trigger).
 * Parameters : buffer		-   user data
 *				buf_size	-   user data size
 * Returns	  : SMAP_OK				-   Success
 *				SMAP_NG				-   Fatal error
 *				SMAP_PARA_NG		-   Parameter error
 *******************************************************************************/
int rtds_memory_ioctl_close_apmem(
	char __user				*buffer,
	size_t					buf_size
)
{
	int							ret = SMAP_PARA_NG;
	rtds_memory_drv_ioctl_mem	ioctl_mem;
	int							proc_cnt = -1;
	rtds_memory_create_queue	*entry_p = NULL;
	rtds_memory_create_queue	*temp_p = NULL;
	unsigned long				flag;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);

	/* Parameter check */
	if (buf_size != sizeof(rtds_memory_drv_ioctl_mem)) {
		MSG_ERROR("[RTDSK]ERR| buf_size error [%d]\n", buf_size);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Copy user data into kernel space */
	ret = copy_from_user(&ioctl_mem, buffer, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_from_user failed[%d]\n", ret);
		ret = SMAP_NG;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)ioctl_mem.app_addr);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)ioctl_mem.pages);
	MSG_MED("[RTDSK]   |page [0x%08X]\n", (u32)*ioctl_mem.pages);

	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

	if (0 != list_empty(&g_rtds_memory_list_create_mem)) {
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|List is empty.\n");
		goto  out;
	}

	/* Search and entry list */
	list_for_each_entry(entry_p, &g_rtds_memory_list_create_mem, queue_header) {
		if (entry_p->page == *ioctl_mem.pages) {
			/* Prevention of the overflow */
			if (RTDS_MEM_PROC_CNT_MAX > proc_cnt) {
				proc_cnt++;
			}

			if (entry_p->app_addr == ioctl_mem.app_addr) {
				MSG_MED("[RTDSK]   |entry_p           [0x%08X]\n", (u32)entry_p);
				MSG_MED("[RTDSK]   |entry_p->app_addr [0x%08X]\n", (u32)entry_p->app_addr);
				MSG_MED("[RTDSK]   |entry_p->mem_size [0x%08X]\n", (u32)entry_p->mem_size);
				MSG_MED("[RTDSK]   |entry_p->page     [0x%08X]\n", (u32)entry_p->page);
				MSG_MED("[RTDSK]   |entry_p->app_cache[0x%08X]\n", (u32)entry_p->app_cache);
				temp_p = entry_p;
			}
		}
	}

	MSG_LOW("[RTDSK]   |temp entry_p[0x%08X] proc_cnt[%d]\n", (u32)temp_p, proc_cnt);
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	if (NULL == temp_p) {
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR| List is not found.\n");
		goto out;
	}

	if (!proc_cnt) {
		rtds_memory_free_page_frame(RTDS_MEM_GET_PAGE_NUM(temp_p->mem_size),
									temp_p->pages,
									temp_p);
	}

	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
	list_del(&temp_p->queue_header);
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
	kfree(temp_p->pages);
	kfree(temp_p);
	ret = SMAP_OK;

out:
	/* Set error code */
	ioctl_mem.err_code  = ret;

	/* Copy into user space */
	ret = copy_to_user(buffer, &ioctl_mem, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_to_user failed[%d]\n", ret);
		ret = SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_cache_flush
 * Description: This function flushes the cache.
 * Parameters : buffer			-   user data
 *				buf_size		-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_NG			-   Fatal error
 *				SMAP_PARA_NG	-   Parameter error
 *******************************************************************************/
int rtds_memory_ioctl_cache_flush(
	char __user				*buffer,
	size_t					buf_size
)
{
	int								ret_code = SMAP_PARA_NG;
	rtds_memory_drv_ioctl_cache		ioctl_cache;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);

	/* Parameter check. */
	if (buf_size != sizeof(ioctl_cache)) {
		MSG_ERROR("[RTDSK]ERR| buf_size error [%d]\n", buf_size);
		MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
		return ret_code;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_cache, buffer, buf_size);
	if (0 != ret_code) {
		ret_code = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR| copy_from_user failed[%d]\n", ret_code);
		MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
		return ret_code;
	}

	/* Flush L2 cache. */
	rtds_memory_drv_flush_cache(ioctl_cache.app_addr, ioctl_cache.mem_size);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_cache_clear
 * Description: This function clears the cache.
 * Parameters : buffer			-   user data
 *				buf_size		-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_NG			-   Fatal error
 *				SMAP_PARA_NG	-   Parameter error
 *******************************************************************************/
int rtds_memory_ioctl_cache_clear(
	char __user				*buffer,
	size_t					buf_size
)
{
	int								ret_code = SMAP_PARA_NG;
	rtds_memory_drv_ioctl_cache		ioctl_cache;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);

	/* Parameter check. */
	if (buf_size != sizeof(ioctl_cache)) {
		MSG_ERROR("[RTDSK]ERR| buf_size error [%d]\n", buf_size);
		MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
		return ret_code;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_cache, buffer, buf_size);
	if (0 != ret_code) {
		ret_code = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR| copy_from_user failed[%d]\n", ret_code);
		MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
		return ret_code;
	}

	/* Clear the L2 cache. */
	rtds_memory_drv_inv_cache(ioctl_cache.app_addr, ioctl_cache.mem_size);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_map_shared_apmem
 * Description: This function maps App shared memory.
 * Parameters : fp			-   file descrptor
 *				map_data	-   map data
 *				mem_table	-   memory table
 * Returns	  : SMAP_OK					-   Success
 *				SMAP_MEMORY				-   No memory
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_map_shared_apmem(
	struct file						*fp,
	rtds_memory_mapping_data		*map_data,
	rtds_memory_app_memory_table	*mem_table
)
{
	int ret = SMAP_OK;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (u32)fp);
	MSG_MED("[RTDSK]   |map_data[0x%08X]\n", (u32)map_data);
	MSG_MED("[RTDSK]   |mem_table[0x%08X]\n", (u32)mem_table);
	MSG_MED("[RTDSK]   |memory_size[0x%08X]\n", (u32)mem_table->memory_size);

	map_data->cache_kind	= RTDS_MEMORY_DRV_WRITEBACK;
	map_data->data_ent		= false;
	map_data->mapping_flag	= RTDS_MEM_MAPPING_APMEM;
	map_data->mem_table		= mem_table;

	/* Mapping */
	ret = rtds_memory_do_map(fp, &(mem_table->rt_wb_addr), mem_table->memory_size, 0);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_do_map failed[%d]\n", ret);
		mem_table->error_code = RTDS_MEM_ERR_MAPPING;
		goto out;
	}

	MSG_MED("[RTDSK]   |rt_wb_addr[0x%08X]\n", (u32)mem_table->rt_wb_addr);

	/* Address range check */
	if (RTDS_MEM_ADDR_NG <= (mem_table->rt_wb_addr + mem_table->memory_size)) {
		rtds_memory_do_unmap(mem_table->rt_wb_addr, mem_table->memory_size);
		mem_table->rt_wb_addr = 0;
		mem_table->error_code = SMAP_MEMORY;
		ret = SMAP_MEMORY;
		MSG_ERROR("[RTDSK]ERR| address range error\n");
		goto out;
	}


	/* RT cache type is non CACHE */

	if ((0 != (mem_table->rt_cache & RT_MEMORY_RTMAP_WBNC)) &&
		(SMAP_OK == mem_table->error_code)) {
		map_data->cache_kind	= RTDS_MEMORY_DRV_NONCACHE;
		map_data->data_ent		= false;
		map_data->mapping_flag	= RTDS_MEM_MAPPING_APMEM;

		/* Mapping */
		ret = rtds_memory_do_map(fp, &(mem_table->rt_nc_addr), mem_table->memory_size, 0);
		if (SMAP_OK != ret) {
			MSG_ERROR("[RTDSK]ERR| rtds_memory_do_map failed[%d]\n", ret);
			mem_table->error_code = RTDS_MEM_ERR_MAPPING;
			goto out;
		}

		MSG_MED("[RTDSK]   |rt_nc_addr[0x%08X]\n", (u32)mem_table->rt_nc_addr);

		/* Address range check */
		if (RTDS_MEM_ADDR_NG <= (mem_table->rt_nc_addr + mem_table->memory_size)) {
			rtds_memory_do_unmap(mem_table->rt_nc_addr, mem_table->memory_size);
			mem_table->rt_nc_addr = 0;
			mem_table->error_code = SMAP_MEMORY;
			ret = SMAP_MEMORY;
			MSG_ERROR("[RTDSK]ERR| address range error\n");
		}
	}

out:
	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;

}

/*******************************************************************************
 * Function   : rtds_memory_open_rttrig_shared_apmem
 * Description: This function open App shared memory(RT trigger).
 * Parameters : mem_size	-   Memory size
 *				rt_cache	-   cache type(RT domain)
 *				rt_trigger	-   RT trigger identifier
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_open_rttrig_shared_apmem(
	unsigned int	mem_size,
	unsigned int	rt_cache,
	unsigned int	rt_trigger
)
{
	int								ret = SMAP_OK;
	rtds_memory_app_memory_table	*mem_table;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)mem_size);
	MSG_MED("[RTDSK]   |rt_cache[0x%08X]\n", (u32)rt_cache);
	MSG_MED("[RTDSK]   |rt_trigger[0x%08X]\n", (u32)rt_trigger);

	/* Allocate memory table area */
	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		MSG_ERROR("[RTDSK]ERR| kmalloc failed\n");
		ret = SMAP_MEMORY;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Initialise and set parameter */
	memset(mem_table, 0, sizeof(*mem_table));
	mem_table->event		= RTDS_MEM_RT_CREATE_EVENT;
	mem_table->memory_size	= mem_size;
	mem_table->rt_cache		= rt_cache;
	mem_table->task_info	= current;
	mem_table->trigger		= RTDS_MEM_TRIGGER_RT;
	mem_table->app_addr		= rt_trigger;

	init_MUTEX_LOCKED(&(mem_table->semaphore));

	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_close_rttrig_shared_apmem
 * Description: This function destroys App shared memory by RT trigger.
 * Parameters : apmem_id	-   App shared memory ID
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_close_rttrig_shared_apmem(
	unsigned int	apmem_id
)
{
	int								ret = SMAP_OK;
	rtds_memory_app_memory_table	*mem_table;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |apmem_id [0x%08X]\n", (u32)apmem_id);

	/* Allocate memory table area */
	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		MSG_ERROR("[RTDSK]ERR| kmalloc failed\n");
		ret = SMAP_MEMORY;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Initialise and set parameter */
	memset(mem_table, 0, sizeof(*mem_table));
	mem_table->event		= RTDS_MEM_RT_DELETE_EVENT;
	mem_table->apmem_id		= apmem_id;
	mem_table->task_info	= current;
	mem_table->trigger		= RTDS_MEM_TRIGGER_RT;

	init_MUTEX_LOCKED(&(mem_table->semaphore));

	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_delete_shared_apmem
 * Description: This function deletes App shared memory.
 * Parameters : apmem_id	-   App shared memory ID
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_delete_shared_apmem(
	unsigned int	apmem_id
)
{

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |apmem_id [0x%08X]\n", apmem_id);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return SMAP_OK;
}

/*******************************************************************************
 * Function   : rtds_memory_send_open_shared_apmem
 * Description: This function requests RT Domain to create App shared memory.
 * Parameters : write_back_addr	-   write back addr
 *				non_cache_addr	-   non-cache addr
 *				mem_size		-   memory size
 *				rt_trigger		-   RT trigger identifier
 *				rt_err			-   RT error code
 *				app_trigger		-   App trigger identifier
 *				apmem_id		-   App shared memory ID
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_NG		-   Fatal error
 *				Others		-   System error/RT result
 *******************************************************************************/
int rtds_memory_send_open_shared_apmem(
	unsigned long	write_back_addr,
	unsigned long	non_cache_addr,
	unsigned long	mem_size,
	unsigned long	rt_trigger,
	unsigned long	rt_err,
	unsigned long	app_trigger,
	unsigned int	*apmem_id
)
{
	int							ret = SMAP_OK;
	unsigned long				write_through_addr = 0;
	iccom_drv_send_cmd_param	send_cmd;
	struct {
		unsigned long	wb_addr;
		unsigned long	wt_addr;
		unsigned long	nc_addr;
		unsigned long	mem_size;
		unsigned long	rt_trig;
		unsigned long	rt_error_code;
		unsigned long	app_tri;
	} send_data;

	struct {
		unsigned int	apmem_id;
		unsigned long	app_trig;
	} rcv_data;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |write_back_addr	[0x%08X]\n", (u32)write_back_addr);
	MSG_MED("[RTDSK]   |non_cache_addr	[0x%08X]\n", (u32)non_cache_addr);
	MSG_MED("[RTDSK]   |mem_size		[0x%08X]\n", (u32)mem_size);
	MSG_MED("[RTDSK]   |rt_trigger		[0x%08X]\n", (u32)rt_trigger);
	MSG_MED("[RTDSK]   |rt_err			[0x%08X]\n", (u32)rt_err);
	MSG_MED("[RTDSK]   |app_trigger		[0x%08X]\n", (u32)app_trigger);
	MSG_MED("[RTDSK]   |apmem_id		[0x%08X]\n", (u32)apmem_id);

	/* Set send data */
	send_data.wb_addr		= write_back_addr;
	send_data.wt_addr		= write_through_addr;   /* Write through addr is not used */
	send_data.nc_addr		= non_cache_addr;
	send_data.mem_size		= mem_size;
	send_data.rt_trig		= rt_trigger;
	send_data.rt_error_code	= rt_err;
	send_data.app_tri		= app_trigger;

	send_cmd.handle			= g_rtds_memory_iccom_handle;
	send_cmd.task_id		= TASK_MEMORY;
	send_cmd.function_id	= EVENT_MEMORY_CREATEAPPMEMORY;
	send_cmd.send_mode		= ICCOM_DRV_SYNC;
	send_cmd.send_size		= sizeof(send_data);
	send_cmd.send_data		= (unsigned char *)(&send_data);
	send_cmd.recv_size		= sizeof(rcv_data);
	send_cmd.recv_data		= (unsigned char *)(&rcv_data);

	/* send EVENT_MEMORY_CREATEAPPMEMORY */
	ret = iccom_drv_send_command(&send_cmd);
	if (SMAP_OK == ret) {
		*apmem_id = rcv_data.apmem_id;
		MSG_MED("[RTDSK]   |apmem_id		[0x%08X]\n", (u32)*apmem_id);
	} else {
		MSG_ERROR("[RTDSK]ERR|[%s]iccom send error[%d]\n", __func__, ret);
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_send_close_shared_apmem
 * Description: This function requests  RT Domain to destroy App shared memory.
 * Parameters : apmem_id	-   App shared memory ID
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_NG		-   Fatal error
 *				Others		-   System error/ RT result
 *******************************************************************************/
int rtds_memory_send_close_shared_apmem(
	unsigned int	apmem_id
)
{
	int							ret = SMAP_OK;
	iccom_drv_send_cmd_param	send_cmd;
	struct {
		unsigned int	apmem_id;
	} send_data;

	struct {
		unsigned int	apmem_id;
	} rcv_data;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |apmem_id [0x%08X]\n", apmem_id);

	/* Set send data */
	send_data.apmem_id = apmem_id;

	send_cmd.handle			= g_rtds_memory_iccom_handle;
	send_cmd.task_id		= TASK_MEMORY_SUB;
	send_cmd.function_id	= EVENT_MEMORYSUB_DELETEAPPSHAREDMEM;
	send_cmd.send_mode		= ICCOM_DRV_SYNC;
	send_cmd.send_size		= sizeof(send_data);
	send_cmd.send_data		= (unsigned char *)(&send_data);
	send_cmd.recv_size		= sizeof(rcv_data);
	send_cmd.recv_data		= (unsigned char *)(&rcv_data);

	/* send EVENT_MEMORYSUB_DELETEAPPSHAREDMEM */
	ret = iccom_drv_send_command(&send_cmd);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR|[%s]iccom send error[%d]\n", __func__, ret);
	}

	MSG_MED("[RTDSK]   |rcv_data[0x%08X]\n", (u32)rcv_data.apmem_id);
	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_send_error_msg
 * Description: This function nortifies RT Domain error.
 * Parameters : err_code	-   error code
 *				mem_table	-   memory table
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_send_error_msg(
	rtds_memory_app_memory_table	*mem_table,
	int								err_code
)
{
	int ret;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |err_code [%d]\n", err_code);

	ret = rtds_memory_send_open_shared_apmem(mem_table->rt_wb_addr,
												mem_table->rt_nc_addr,
												mem_table->memory_size,
												mem_table->app_addr,
												err_code,
												0,
												&(mem_table->apmem_id)
											);
	if (SMAP_OK == ret) {
		kfree(mem_table);
	} else {
		kfree(mem_table);
		panic("Send error[%s][%d] err_code[%d]", __func__, __LINE__, ret);
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

typedef struct {
	unsigned int	mem_size;
	unsigned int	rt_cache;
	unsigned int	rt_trigger;
} rcv_open_data;

typedef struct {
	unsigned int	apmem_id;
} rcv_close_data;

/*******************************************************************************
 * Function   : rtds_memory_rcv_comp_notice
 * Description: This function is callback to response asynchronous receive event.
 *			  Entry received event to queue.
 * Parameters : user_data		- user data(Not used)
 *				result_code		- result
 *				function_id		- function id
 *				data_addr		- receive data address
 *				data_len		- receive data size
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_rcv_comp_notice(
	void			*user_data,
	int				result_code,
	int				function_id,
	unsigned char	*data_addr,
	int				data_len
)
{
	int				ret;
	unsigned int	event = RTDS_MEM_DRV_EVENT_APMEM;
	rcv_open_data	open_data;
	rcv_close_data	close_data;
	rcv_open_data	*open_data_p = &open_data;
	rcv_close_data	*close_data_p = &close_data;
	bool			error_flag = false;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |user_data	[0x%08X]\n", (u32)user_data);
	MSG_MED("[RTDSK]   |result_code	[%d]\n", result_code);
	MSG_MED("[RTDSK]   |function_id	[%d]\n", function_id);
	MSG_MED("[RTDSK]   |data_addr	[0x%08X]\n", (u32)data_addr);
	MSG_MED("[RTDSK]   |data_len	[0x%08X]\n", (u32)data_len);

	/* Parameter check*/
	if ((NULL == data_addr) || (0 == data_len)) {
		MSG_ERROR("[RTDSK]ERR| rcv_data is illegal. addr[0x%08X]size[%x]\n",
		 (u32)data_addr, (u32)data_len);
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return;
	}

	/* initialise parameter */
	memset(&open_data, 0, sizeof(rcv_open_data));
	close_data.apmem_id = 0;

	switch (function_id) {
	case EVENT_MEMORY_OPENAPPMEMORY:
		MSG_MED("[RTDSK]   |EVENT_MEMORY_OPENAPPMEMORY\n");
		event = RTDS_MEM_DRV_EVENT_APMEM_OPEN;

		if (sizeof(rcv_open_data) == data_len) {
			/* Get mem_size, rt_cache, rt_trigger */
			open_data_p = (rcv_open_data *)data_addr;
		} else {
			error_flag = true;
		}

		break;

	case EVENT_MEMORY_CLOSEAPPMEMORY:
		MSG_MED("[RTDSK]   |EVENT_MEMORY_CLOSEAPPMEMORY\n");
		event = RTDS_MEM_DRV_EVENT_APMEM_CLOSE;

		if (sizeof(rcv_close_data) == data_len) {
			/* Get apmem_id */
			close_data_p = (rcv_close_data *)data_addr;
		} else {
			error_flag = true;
		}
		break;

	case EVENT_MEMORYSUB_DELETEAPPSHAREDMEM:
		MSG_MED("[RTDSK]   |EVENT_MEMORYSUB_DELETEAPPSHAREDMEM\n");
		break;

	default:
		MSG_ERROR("[RTDSK]ERR| No function id[%d]\n", function_id);
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return;
	}

	if (error_flag) {
		MSG_ERROR("[RTDSK]ERR| rcv_data is illegal. size[%x]\n", data_len);
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return;
	}

	/* Entry received event */
	ret = rtds_memory_put_recv_queue(event,
									open_data_p->mem_size,
									open_data_p->rt_cache,
									open_data_p->rt_trigger,
									close_data_p->apmem_id);
	if (SMAP_OK == ret) {
		/* Get semaphore to wakeup apmem thread */
		down(&g_rtds_memory_apmem_rttrig_sem);
		MSG_LOW("[RTDSK]   |Semaphore down(Apmem)\n");
		/* Wakeup thread */
		complete(&g_rtds_memory_completion);
	} else {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_put_recv_queue failed ret[%d]\n", ret);
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_put_recv_queue
 * Description: This function entries received event to list queue.
 *				Entry received event to queue.
 * Parameters : event			- event
 *				mem_size		- memory size
 *				rt_cache		- cache type of RT domain
 *				rt_trigger		- RT trigger
 *				apmem_id		- App shared memory ID
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_put_recv_queue(
	unsigned int		event,
	unsigned int		mem_size,
	unsigned int		rt_cache,
	unsigned int		rt_trigger,
	unsigned int		apmem_id
)
{
	int							ret = SMAP_OK;
	unsigned long				flag;
	rtds_memory_rcv_event_queue	*rcv_event_queue_p;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |event		[0x%08X]\n", event);
	MSG_MED("[RTDSK]   |mem_size	[0x%08X]\n", mem_size);
	MSG_MED("[RTDSK]   |rt_cache	[0x%08X]\n", rt_cache);
	MSG_MED("[RTDSK]   |rt_trigger	[0x%08X]\n", rt_trigger);
	MSG_MED("[RTDSK]   |apmem_id	[0x%08X]\n", apmem_id);


	/* Allocate memory(rtds_memory_rcv_event_queue) */
	rcv_event_queue_p = kmalloc(sizeof(*rcv_event_queue_p), GFP_KERNEL);
	if (NULL == rcv_event_queue_p) {
		MSG_ERROR("[RTDSK]ERR| kmalloc() failed\n");
		ret = SMAP_MEMORY;
	} else {
		/* Set data */
		rcv_event_queue_p->event		= event;
		rcv_event_queue_p->mem_size	 = mem_size;
		rcv_event_queue_p->rt_cache	 = rt_cache;
		rcv_event_queue_p->rt_trigger   = rt_trigger;
		rcv_event_queue_p->apmem_id	 = apmem_id;

		/* Spin lock*/
		spin_lock_irqsave(&g_rtds_memory_lock_recv_queue, flag);

		/* Add entry to list queue */
		list_add_tail(&rcv_event_queue_p->queue_header, &g_rtds_memory_list_rcv_event);

		/* Spin unlock*/
		spin_unlock_irqrestore(&g_rtds_memory_lock_recv_queue, flag) ;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_get_recv_queue
 * Description: This function gets received event from list queue.
 * Parameters : none
 * Returns	  : NotNULL		-   rtds_memory_rcv_event_queue
 *				NULL		-
 *******************************************************************************/
rtds_memory_rcv_event_queue *rtds_memory_get_recv_queue(
	void
)
{
	unsigned long				flag;
	rtds_memory_rcv_event_queue	*entry_p = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Spin lock*/
	spin_lock_irqsave(&g_rtds_memory_lock_recv_queue, flag);

	if (0 != list_empty(&g_rtds_memory_list_rcv_event)) {
		spin_unlock_irqrestore(&g_rtds_memory_lock_recv_queue, flag);
		MSG_ERROR("[RTDSK]ERR| List is empty.\n");
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return entry_p;
	}

	/* Search and entry list */
	list_for_each_entry(entry_p, &g_rtds_memory_list_rcv_event, queue_header) break;

	/* Spin unlock*/
	spin_unlock_irqrestore(&g_rtds_memory_lock_recv_queue, flag);

	MSG_MED("[RTDSK]   |entry_p = [0x%08X]\n", (u32)entry_p);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return entry_p;
}

/*******************************************************************************
 * Function   : rtds_memory_delete_recv_queue
 * Description: This function deletes received event from list queue.
 * Parameters : queue		   - received event queue info
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_delete_recv_queue(
	rtds_memory_rcv_event_queue *queue
)
{
	unsigned long	flag;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Spin lock*/
	spin_lock_irqsave(&g_rtds_memory_lock_recv_queue, flag);

	/* Delete queue from list */
	list_del(&queue->queue_header);

	/* Spin unlock*/
	spin_unlock_irqrestore(&g_rtds_memory_lock_recv_queue, flag);

	/* Free memory of queue */
	kfree(queue);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);

	return;
}

/*******************************************************************************
 * Function   : rtds_memory_put_mpro_list
 * Description: This function entrys mpro list queue.
 * Parameters : mem_table - memory table
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_put_mpro_list(
	rtds_memory_app_memory_table	*mem_table
)
{
	unsigned long	flag;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |mem_table	 [0x%08X]\n", (u32)mem_table);

	/* Spin lock*/
	spin_lock_irqsave(&g_rtds_memory_lock_mpro, flag);

	/* Add list of Mpro */
	list_add_tail(&(mem_table->list_head_mpro), &g_rtds_memory_list_mpro);

	/* Spin unlock */
	spin_unlock_irqrestore(&g_rtds_memory_lock_mpro, flag);

	/* Release Semaphore(Mpro) */
	up(&g_rtds_memory_mpro_sem);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);

	return;
}


/*******************************************************************************
 * Function   : rtds_memory_map_mpro
 * Description: This function gives a map demand to Mpro.
 * Parameters : phy_address		-   physical address
 *				mem_size		-   Memory size
 *				vir_address		-   RT domain logical address
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_map_mpro(
	unsigned long	phy_address,
	unsigned long	mem_size,
	unsigned long	*vir_address
)
{
	int								ret_code = SMAP_OK;
	rtds_memory_app_memory_table	*mem_table = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |phy_address[0x%08X]\n", (u32)phy_address);
	MSG_MED("[RTDSK]   |mem_size[%d]\n", (u32)mem_size);

	/* leak check */
	rtds_memory_leak_check_mpro();

	/* Allocate memory */
	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_MEMORY;
	}

	down(&g_rtds_memory_shared_mem);
	memset(mem_table, 0, sizeof(*mem_table));
	/* Set mem_table data */
	mem_table->event			= RTDS_MEM_MAP_EVENT;
	mem_table->rt_nc_addr		= phy_address;
	mem_table->memory_size		= mem_size;
	mem_table->app_cache		= RTDS_MEMORY_DRV_WRITEBACK;
	mem_table->task_info		= current;
	init_MUTEX_LOCKED(&(mem_table->semaphore));

	/* Add list of shared_mem */
	list_add_tail(&(mem_table->list_head), &g_rtds_memory_list_shared_mem);
	up(&g_rtds_memory_shared_mem);

	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);
	down(&(mem_table->semaphore));

	/* mapped address  */
	*vir_address	= mem_table->rt_wb_addr;
	ret_code		= mem_table->error_code;

	if (SMAP_OK != ret_code) {
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
	}

	MSG_MED("[RTDSK]   |*vir_address[0x%08X]\n", (u32)*vir_address);
	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);

	return ret_code;

}


/*******************************************************************************
 * Function   : rtds_memory_unmap_mpro
 * Description: This function gives a unmap demand to Mpro.
 * Parameters : vir_address		-   logical address
 *				mem_size		-   Memory size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_MEMORY		-   No memory
 *******************************************************************************/
int rtds_memory_unmap_mpro(
	unsigned long	vir_address,
	unsigned long	mem_size
)
{
	int								ret_code = SMAP_NG;
	rtds_memory_app_memory_table	*mem_table = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |vir_address[0x%08X]\n", (u32)vir_address);
	MSG_MED("[RTDSK]   |mem_size[%d]\n", (u32)mem_size);

	down(&g_rtds_memory_shared_mem);
	/* Search for shared memory list */
	list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
		if ((0 == mem_table->apmem_id) &&
			(vir_address == mem_table->rt_wb_addr) &&
			(mem_size == mem_table->memory_size)) {
			ret_code = SMAP_OK;
			break;
		}
	}
	up(&g_rtds_memory_shared_mem);

	/* not found */
	if (SMAP_OK != ret_code) {
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret_code);
		return ret_code;
	}

	/* Set mem_table data */
	mem_table->event		= RTDS_MEM_UNMAP_EVENT;
	mem_table->task_info	= current;
	init_MUTEX_LOCKED(&(mem_table->semaphore));

	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);
	down(&(mem_table->semaphore));

	ret_code = mem_table->error_code;

	down(&g_rtds_memory_shared_mem);
	list_del(&(mem_table->list_head));
	kfree(mem_table);
	up(&g_rtds_memory_shared_mem);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_map_pnc_mpro
 * Description: This function gives a map demand to Mpro.
 * Parameters : app_addr		-   AppDomain side logical address
 *				map_size		-   Map size
 *				pages			-   page descriptor
 *				rt_cache		-   RT cache type
 *				mem_info		-   memory info
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_MEMORY		-   No memory
 *				Others			-   System error/RT result
 *******************************************************************************/
int rtds_memory_map_pnc_mpro(
	unsigned int					app_addr,
	unsigned int					map_size,
	struct page						**pages,
	unsigned int					rt_cache,
	rtds_memory_drv_app_mem_info	*mem_info
)
{
	int								ret_code = SMAP_OK;
	rtds_memory_app_memory_table	*mem_table = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)app_addr);
	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)map_size);
	MSG_MED("[RTDSK]   |rt_cache[%d]\n", (u32)rt_cache);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)pages);

	/* leak check */
	rtds_memory_leak_check_mpro();

	/* Allocate memory */
	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_MEMORY;
	}

	down(&g_rtds_memory_shared_mem);
	memset(mem_table, 0, sizeof(*mem_table));
	/* Set mem_table data */
	mem_table->event		= RTDS_MEM_MAP_PNC_EVENT;
	mem_table->memory_size	= map_size;
	mem_table->rt_cache		= rt_cache;
	mem_table->pages		= pages;
	mem_table->app_addr		= app_addr;
	mem_table->task_info	= current;
	init_MUTEX_LOCKED(&(mem_table->semaphore));

	/* Add list of shared_mem */
	list_add_tail(&(mem_table->list_head), &g_rtds_memory_list_shared_mem);
	up(&g_rtds_memory_shared_mem);

	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);
	down(&(mem_table->semaphore));

	if (0 != mem_table->error_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
		ret_code = mem_table->error_code;
		MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
		return ret_code;
	}

	/* mapped address  */
	mem_info->rt_write_back_addr	= mem_table->rt_wb_addr;
	mem_info->rt_non_cache_addr		= mem_table->rt_nc_addr;
	mem_info->apmem_id				= mem_table->apmem_id;

	MSG_MED("[RTDSK]   |rt_write_back_addr[0x%08X]\n", (u32)mem_info->rt_write_back_addr);
	MSG_MED("[RTDSK]   |rt_non_cache_addr[0x%08X]\n", (u32)mem_info->rt_non_cache_addr);
	MSG_MED("[RTDSK]   |apmem_id[0x%08X]\n", (u32)mem_info->apmem_id);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);

	return ret_code;

}


/*******************************************************************************
 * Function   : rtds_memory_unmap_pnc_mpro
 * Description: This function gives a unmap demand to Mpro.
 * Parameters : address			-   APP shared_mem address
 *				apmem_id		-   App shared memory ID
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_MEMORY		-   No memory
 *				SMAP_PARA_NG	-   Parameter error
 *				Others			-   System error/RT result
 *******************************************************************************/
int rtds_memory_unmap_pnc_mpro(
	unsigned long	address,
	unsigned int	apmem_id
)
{
	int								ret_code = SMAP_PARA_NG;
	unsigned int					proc_cnt = 0;
	rtds_memory_app_memory_table	*mem_table = NULL;
	rtds_memory_app_memory_table	*entry_check_mem_table = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |address[0x%08X]\n", (u32)address);
	MSG_MED("[RTDSK]   |apmem_id[0x%08X]\n", (u32)apmem_id);

	down(&g_rtds_memory_shared_mem);
	/* Search for shared memory list */
	list_for_each_entry(entry_check_mem_table, &g_rtds_memory_list_shared_mem, list_head) {
		if (apmem_id == entry_check_mem_table->apmem_id) {
			proc_cnt++;
			MSG_LOW("[RTDSK]   |---\n");
			MSG_LOW("[RTDSK]   |mem_table[0x%08X]\n", (u32)entry_check_mem_table);
			MSG_LOW("[RTDSK]   |state[0x%08X]\n", (u32)entry_check_mem_table->task_info->state);
			MSG_LOW("[RTDSK]   |flags[0x%08X]\n", (u32)entry_check_mem_table->task_info->flags);
			MSG_LOW("[RTDSK]   |event[0x%08X]\n", (u32)entry_check_mem_table->event);
			MSG_LOW("[RTDSK]   |app_addr[0x%08X]\n", (u32)entry_check_mem_table->app_addr);
			MSG_LOW("[RTDSK]   |rt_wb_addr[0x%08X]\n", (u32)entry_check_mem_table->rt_wb_addr);
			MSG_LOW("[RTDSK]   |rt_nc_addr[0x%08X]\n", (u32)entry_check_mem_table->rt_nc_addr);
			MSG_LOW("[RTDSK]   |memory_size[0x%08X]\n", (u32)entry_check_mem_table->memory_size);
			MSG_LOW("[RTDSK]   |pages[0x%08X]\n", (u32)entry_check_mem_table->pages);
			MSG_LOW("[RTDSK]   |page[0x%08X]\n", (u32)*entry_check_mem_table->pages);
			MSG_LOW("[RTDSK]   |pid[%d]\n", (u32)entry_check_mem_table->task_info->pid);
			MSG_LOW("[RTDSK]   |tgid[%d]\n", (u32)entry_check_mem_table->task_info->tgid);
			MSG_LOW("[RTDSK]   |---\n");
			if (address == entry_check_mem_table->app_addr) {
				if ((entry_check_mem_table->task_info->tgid == current->tgid) ||
					(entry_check_mem_table->task_info->flags & PF_EXITPIDONE)) {
					MSG_LOW("[RTDSK]   |current tgid[%d]\n", (u32)current->tgid);
					proc_cnt--;
					mem_table = entry_check_mem_table;
					ret_code = SMAP_OK;
				} else {
					MSG_LOW("[RTDSK]   |[%s][%d] Not match.\n", __func__, __LINE__);
					MSG_LOW("[RTDSK]   |flags[0x%08X]\n", (u32)entry_check_mem_table->task_info->flags);
					MSG_LOW("[RTDSK]   |tgid[%d]\n", (u32)entry_check_mem_table->task_info->tgid);
					MSG_LOW("[RTDSK]   |current tgid[%d]\n", (u32)current->tgid);
				}
			}
		}
	}

	/* not found */
	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d] List is not found.\n", __func__, __LINE__);
		up(&g_rtds_memory_shared_mem);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret_code);
		return ret_code;
	}

	up(&g_rtds_memory_shared_mem);

	if (proc_cnt) {
		MSG_MED("[RTDSK]   |proc_cnt = %d\n", proc_cnt);
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret_code);
		return ret_code;
	}

	mem_table->event = RTDS_MEM_UNMAP_PNC_EVENT;

	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);
	down(&(mem_table->semaphore));

	ret_code = mem_table->error_code;

	down(&g_rtds_memory_shared_mem);
	list_del(&(mem_table->list_head));
	kfree(mem_table);
	up(&g_rtds_memory_shared_mem);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_map_pnc_nma_mpro
 * Description: This function gives a map demand to Mpro.
 * Parameters : map_size		-   Map size
 *				pages			-   page descriptor
 *				rt_addr_wb		-   RT domain logical address
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_map_pnc_nma_mpro(
	unsigned int			map_size,
	struct page				**pages,
	unsigned int			*rt_addr_wb
)
{
	int								ret_code = SMAP_OK;
	rtds_memory_app_memory_table	*mem_table = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)map_size);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)pages);

	/* leak check */
	rtds_memory_leak_check_mpro();

	/* Allocate memory */
	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_MEMORY;
	}

	down(&g_rtds_memory_shared_mem);
	memset(mem_table, 0, sizeof(*mem_table));
	/* Set mem_table data */
	mem_table->event			= RTDS_MEM_MAP_PNC_NMA_EVENT;
	mem_table->memory_size		= map_size;
	mem_table->pages			= pages;
	mem_table->task_info		= current;
	init_MUTEX_LOCKED(&(mem_table->semaphore));

	/* Add list of shared_mem */
	list_add_tail(&(mem_table->list_head), &g_rtds_memory_list_shared_mem);
	up(&g_rtds_memory_shared_mem);


	/* Add mpro list */
	rtds_memory_put_mpro_list(mem_table);
	down(&(mem_table->semaphore));

	/* mapped address  */
	*rt_addr_wb		= mem_table->rt_wb_addr;
	ret_code		= mem_table->error_code;

	if (SMAP_OK != ret_code) {
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
	}

	MSG_MED("[RTDSK]   |*rt_addr_wb[0x%08X]\n", (u32)*rt_addr_wb);
	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);

	return ret_code;

}


/*******************************************************************************
 * Function   : rtds_memory_ioctl_map_mpro
 * Description: This function gives a map demand to Mpro.
 * Parameters : buffer			-   user data
 *				cnt				-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *				SMAP_NG			-   Data error
 *******************************************************************************/
int rtds_memory_ioctl_map_mpro(
	char __user		*buffer,
	size_t			cnt
)
{
	int							ret_code = SMAP_OK;
	rtds_memory_drv_ioctl_rtmap	ioctl_rtmap;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Parameter check. */
	if (cnt != sizeof(ioctl_rtmap)) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_PARA_NG;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_rtmap, buffer, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	/* map demand to Mpro */
	ret_code = rtds_memory_map_mpro(ioctl_rtmap.phy_addr, ioctl_rtmap.mem_size, (unsigned long *)&ioctl_rtmap.app_addr);
	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	/* Copy into user space */
	ret_code = copy_to_user(buffer, &ioctl_rtmap, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_ioctl_unmap_mpro
 * Description: This function gives a unmap demand to Mpro.
 * Parameters : buffer			-   user data
 *				cnt				-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *				SMAP_NG			-   Data error
 *******************************************************************************/
int rtds_memory_ioctl_unmap_mpro(
	char __user		*buffer,
	size_t			cnt
)
{
	int								ret_code = SMAP_OK;
	rtds_memory_drv_ioctl_rtunmap	ioctl_rtunmap;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Parameter check. */
	if (cnt != sizeof(ioctl_rtunmap)) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_PARA_NG;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_rtunmap, buffer, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	/* unmap demand to Mpro */
	ret_code = rtds_memory_unmap_mpro(ioctl_rtunmap.app_addr, ioctl_rtunmap.mem_size);
	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_ioctl_map_pnc_mpro
 * Description: This function gives a map demand to Mpro.
 * Parameters : buffer			-   user data
 *				cnt				-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *				SMAP_MEMORY		-   No memory
 *				SMAP_NG			-   Data error
 *******************************************************************************/
int rtds_memory_ioctl_map_pnc_mpro(
	char __user		*buffer,
	size_t			cnt
)
{
	int								ret_code = SMAP_OK;
	int								pages_size = 0;
	struct page						**kernel_pages = NULL;
	rtds_memory_drv_ioctl_mem		ioctl_rtmap_pnc;
	rtds_memory_drv_app_mem_info	memory_info;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Parameter check. */
	if (cnt != sizeof(ioctl_rtmap_pnc)) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_PARA_NG;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_rtmap_pnc, buffer, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	/* Page descriptor information storage area */
	pages_size = (ioctl_rtmap_pnc.mem_size / PAGE_SIZE) * sizeof(struct page *);
	kernel_pages = kmalloc(pages_size, GFP_KERNEL);
	if (NULL == kernel_pages) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_MEMORY;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(kernel_pages, ioctl_rtmap_pnc.pages, pages_size);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		kfree(kernel_pages);
		return SMAP_NG;
	}

	memset(&memory_info, 0, sizeof(memory_info));

	/* map demand to Mpro */
	ret_code = rtds_memory_map_pnc_mpro(ioctl_rtmap_pnc.app_addr, ioctl_rtmap_pnc.mem_size,
										kernel_pages, ioctl_rtmap_pnc.rt_cache, &memory_info);
	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		kfree(kernel_pages);
		ioctl_rtmap_pnc.err_code = ret_code;
		ret_code = copy_to_user(buffer, &ioctl_rtmap_pnc, cnt);
		return SMAP_NG;
	}

	/* mapped info */
	ioctl_rtmap_pnc.rt_addr		= memory_info.rt_write_back_addr;
	ioctl_rtmap_pnc.rt_addr_nc	= memory_info.rt_non_cache_addr;
	ioctl_rtmap_pnc.pages		= kernel_pages;
	ioctl_rtmap_pnc.apmem_id	= memory_info.apmem_id;
	ioctl_rtmap_pnc.err_code	= ret_code;

	/* Copy into user space */
	ret_code = copy_to_user(buffer, &ioctl_rtmap_pnc, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		ret_code = SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_ioctl_unmap_pnc_mpro
 * Description: This function gives a unmap demand to Mpro.
 * Parameters : buffer			-   user data
 *				cnt				-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *				SMAP_NG			-   Data error
 *******************************************************************************/
int rtds_memory_ioctl_unmap_pnc_mpro(
	char __user		*buffer,
	size_t			cnt
)
{
	int							ret_code = SMAP_OK;
	rtds_memory_drv_ioctl_mem	ioctl_rtunmap_pnc;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Parameter check. */
	if (cnt != sizeof(ioctl_rtunmap_pnc)) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_PARA_NG;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_rtunmap_pnc, buffer, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	MSG_MED("[RTDSK]   |ioctl_rtunmap_pnc.pages[0x%08X]\n", (u32)ioctl_rtunmap_pnc.pages);

	/* unmap demand to Mpro */
	ret_code = rtds_memory_unmap_pnc_mpro(ioctl_rtunmap_pnc.app_addr, ioctl_rtunmap_pnc.apmem_id);
	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		ioctl_rtunmap_pnc.err_code = ret_code;
		ret_code = copy_to_user(buffer, &ioctl_rtunmap_pnc, cnt);
		return SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_ioctl_map_pnc_nma_mpro
 * Description: This function gives a map demand to Mpro.
 * Parameters : buffer			-   user data
 *				cnt				-   user data size
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *				SMAP_MEMORY		-   No memory
 *				SMAP_NG			-   Data error
 *******************************************************************************/
int rtds_memory_ioctl_map_pnc_nma_mpro(
	char __user		*buffer,
	size_t			cnt
)
{
	int								ret_code = SMAP_OK;
	int								pages_size = 0;
	struct page						**kernel_pages = NULL;
	rtds_memory_drv_ioctl_mem		ioctl_rtmap_pnc_nma;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Parameter check. */
	if (cnt != sizeof(ioctl_rtmap_pnc_nma)) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_PARA_NG;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(&ioctl_rtmap_pnc_nma, buffer, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_NG;
	}

	/* Page descriptor information storage area */
	pages_size = (ioctl_rtmap_pnc_nma.mem_size / PAGE_SIZE) * sizeof(struct page *);
	kernel_pages = kmalloc(pages_size, GFP_KERNEL);
	if (NULL == kernel_pages) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		return SMAP_MEMORY;
	}

	/* Copy user data into kernel space. */
	ret_code = copy_from_user(kernel_pages, ioctl_rtmap_pnc_nma.pages, pages_size);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		kfree(kernel_pages);
		return SMAP_NG;
	}

	/* map demand to Mpro */
	ret_code = rtds_memory_map_pnc_nma_mpro(ioctl_rtmap_pnc_nma.mem_size, kernel_pages,
										(unsigned int *)&ioctl_rtmap_pnc_nma.rt_addr);
	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		kfree(kernel_pages);
		ioctl_rtmap_pnc_nma.err_code = ret_code;
		ret_code = copy_to_user(buffer, &ioctl_rtmap_pnc_nma, cnt);
		return SMAP_NG;
	}

	ioctl_rtmap_pnc_nma.err_code	= ret_code;

	/* Copy into user space */
	ret_code = copy_to_user(buffer, &ioctl_rtmap_pnc_nma, cnt);
	if (0 != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		ret_code = SMAP_NG;
	}

	kfree(kernel_pages);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}


/*******************************************************************************
 * Function   : rtds_memory_drv_close_vma
 * Description: This function close vma.
 * Parameters : vm_area	  -   vm_area info
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_drv_close_vma(
	struct vm_area_struct	  *vm_area
)
{
	rtds_memory_app_memory_table	*mem_table;
	int								ret;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |mm      [0x%08X]\n", (u32)vm_area->vm_mm);
	MSG_MED("[RTDSK]   |vm_start[0x%08X]\n", (u32)vm_area->vm_start);
	MSG_MED("[RTDSK]   |vm_end  [0x%08X]\n", (u32)vm_area->vm_end);
	MSG_MED("[RTDSK]   |vm_flags[0x%08X]\n", (u32)vm_area->vm_flags);
	MSG_MED("[RTDSK]   |vm_pgoff[0x%08X]\n", (u32)vm_area->vm_pgoff);

	MSG_MED("[RTDSK]   |---cuurent task info---\n");
	MSG_MED("[RTDSK]   |state[0x%08X]\n", (u32)current->state);
	MSG_MED("[RTDSK]   |flags[0x%08X]\n", (u32)current->flags);
	MSG_MED("[RTDSK]   |mm[0x%08X]\n", (u32)current->mm);
	MSG_MED("[RTDSK]   |pid[%d]\n", (u32)current->pid);
	MSG_MED("[RTDSK]   |tgid[%d]\n", (u32)current->tgid);
	MSG_MED("[RTDSK]   |real_parent->pid[%d]\n", (u32)current->real_parent->pid);
	MSG_MED("[RTDSK]   |parent->pid[%d]\n", (u32)current->parent->pid);

	MSG_LOW("[RTDSK]   |---Mpro list---\n");
	down(&g_rtds_memory_shared_mem);
	list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
		MSG_LOW("[RTDSK]   |---\n");
		MSG_LOW("[RTDSK]   |mem_table[0x%08X]\n", (u32)mem_table);
		MSG_LOW("[RTDSK]   |apmem_id[%d]\n", mem_table->apmem_id);
		MSG_LOW("[RTDSK]   |app_address[0x%08X]\n", (u32)mem_table->app_addr);
		MSG_LOW("[RTDSK]   |rt_wb_addr[0x%08X]\n", (u32)mem_table->rt_wb_addr);
		MSG_LOW("[RTDSK]   |rt_nc_addr[0x%08X]\n", (u32)mem_table->rt_nc_addr);
		MSG_LOW("[RTDSK]   |task_info->state[0x%08X]\n", (u32)mem_table->task_info->state);
		MSG_LOW("[RTDSK]   |task_info->flags[0x%08X]\n", (u32)mem_table->task_info->flags);
		MSG_LOW("[RTDSK]   |task_info->mm[0x%08X]\n", (u32)mem_table->task_info->mm);
		MSG_LOW("[RTDSK]   |task_info->pid[%d]\n", (u32)mem_table->task_info->pid);
		MSG_LOW("[RTDSK]   |task_info->tgid[%d]\n", (u32)mem_table->task_info->tgid);
		MSG_LOW("[RTDSK]   |real_parent->pid[%d]\n", (u32)mem_table->task_info->real_parent->pid);
		MSG_LOW("[RTDSK]   |parent->pid[%d]\n", (u32)mem_table->task_info->parent->pid);
		MSG_LOW("[RTDSK]   |---\n");
	}
	up(&g_rtds_memory_shared_mem);

	/* check process status */
	if (current->flags & PF_EXITING) {

		/* check mpro space */
		down(&g_rtds_memory_shared_mem);
		list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
			MSG_MED("[RTDSK]   |state[0x%08X]\n", (u32)mem_table->task_info->state);
			MSG_MED("[RTDSK]   |flags[0x%08X]\n", (u32)mem_table->task_info->flags);
			if ((mem_table->task_info->tgid == current->tgid) &&
				(mem_table->app_addr == vm_area->vm_start)) {
				up(&g_rtds_memory_shared_mem);
				switch (mem_table->event) {
				case RTDS_MEM_MAP_PNC_EVENT:
				case RTDS_MEM_UNMAP_PNC_EVENT:
					ret = rtds_memory_unmap_pnc_mpro(mem_table->app_addr, mem_table->apmem_id);
					MSG_MED("[RTDSK]   |result rtds_memory_unmap_pnc_mpro [%d]\n", ret);
					break;
				default:
					ret = rtds_memory_unmap_mpro(mem_table->rt_wb_addr, mem_table->memory_size);
					MSG_MED("[RTDSK]   |result rtds_memory_unmap_mpro [%d]\n", ret);
					break;
				}
				down(&g_rtds_memory_shared_mem);
				break;
			}
		}
		up(&g_rtds_memory_shared_mem);

		/* check page frame */
		rtds_memory_close_apmem(vm_area->vm_start, (vm_area->vm_end - vm_area->vm_start));
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;

}

/*******************************************************************************
 * Function   : rtds_memory_close_apmem
 * Description: This function releases resorce of memory descriptor.
 * Parameters : app_addr	-	app logical address
 *				mem_size	-	memory size
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_close_apmem(
	unsigned int	app_addr,
	unsigned int	mem_size
)
{
	int							ret;
	unsigned long				flag;
	unsigned int				temp_mem_size;
	rtds_memory_create_queue	*entry_p = NULL;
	rtds_memory_create_queue	*temp_p = NULL;
	struct page					**pages = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)app_addr);
	MSG_MED("[RTDSK]   |mem_size[0x%08X]\n", (u32)mem_size);

	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
	list_for_each_entry(entry_p, &g_rtds_memory_list_create_mem, queue_header) {
		if ((current->tgid == entry_p->task_info->tgid) && (app_addr == entry_p->app_addr)) {
			MSG_MED("[RTDSK]   |create list is found.\n");
			MSG_MED("[RTDSK]   |addr[0x%08X]\n", (u32)entry_p->app_addr);
			MSG_MED("[RTDSK]   |pid[%d]\n", (u32)entry_p->task_info->pid);
			MSG_MED("[RTDSK]   |tgid[%d]\n", (u32)entry_p->task_info->tgid);
			MSG_MED("[RTDSK]   |page[0x%08X]\n", (u32)entry_p->page);
			MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)entry_p->pages);
			temp_p = entry_p;
			break;
		}
	}
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	if (NULL == temp_p) {
		MSG_MED("[RTDSK]   |create list is not found.\n");
		goto out;
	}

	/* check if current process is forked */
	if (temp_p->task_info->tgid != current->tgid) {
		MSG_MED("[RTDSK]   |current_tgid[%d],create_list_tgid[%d]\n", current->tgid, temp_p->task_info->tgid);
		goto out;
	}

	pages = kmalloc(RTDS_MEM_GET_PAGE_NUM(mem_size) * sizeof(struct page *), GFP_KERNEL);
	if (NULL == pages) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		goto out;
	}
	memcpy(pages, temp_p->pages, RTDS_MEM_GET_PAGE_NUM(mem_size) * sizeof(struct page *));

	ret = rtds_memory_get_create_mem_list(app_addr, *pages, &temp_mem_size);
	if (0 == ret) {
		MSG_MED("[RTDSK]   |Free page frame.\n");
		rtds_memory_free_page_frame(RTDS_MEM_GET_PAGE_NUM(temp_mem_size), pages, NULL);
	} else {
		MSG_MED("[RTDSK]   |rtds_memory_free_page_frame[%d].\n", ret);
	}

	kfree(pages);

out:
	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_create_page_frame
 * Description: This function creates page frames.
 * Parameters : page_num	-   number of page
 *				pages		-   page descriptor
 *				create_list	-   App shared memory create list
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY	-   No memory
 *******************************************************************************/
int rtds_memory_create_page_frame(
	unsigned int				page_num,
	struct page					**pages,
	rtds_memory_create_queue	*create_list
)
{
	int				ret = SMAP_OK;
	int				retry;
	int				page_cnt;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |page_num[0x%08X]\n", page_num);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)pages);
	MSG_MED("[RTDSK]   |create_list[0x%08X]\n", (u32)create_list);

#ifdef RTDS_MEM_ALLOC_FROM_HIGHMEM

	for (page_cnt = 0; page_cnt < page_num; page_cnt++) {
		for (retry = 0; retry < RTDS_MEM_ALLOC_PAGE_RETRY; retry++) {
			pages[page_cnt] = alloc_page(GFP_HIGHUSER|__GFP_NORETRY);
			if (NULL != pages[page_cnt]) {
				if (create_list) {
					create_list->mem_size += PAGE_SIZE;
					MSG_INFO("[RTDSK]   |create_list->mem_size[0x%08X]\n", (u32)create_list->mem_size);
				}
				break;
			}
			congestion_wait(BLK_RW_ASYNC, HZ/50);
		}

		if (NULL == pages[page_cnt]) {
			for (--page_cnt; page_cnt >= 0; page_cnt--)
				__free_page(pages[page_cnt]);
			memset(pages, 0, page_num * sizeof(struct page *));
			if (create_list)
				create_list->mem_size = 0;
			ret = SMAP_MEMORY;
			MSG_ERROR("[RTDSK]ERR| alloc_page failed.\n");
			break;
		}
		MSG_INFO("[RTDSK]   |c_pages[%d][0x%08X]\n", page_cnt, (u32)pages[page_cnt]);
	}

#else
	{
		int				cnt = page_num;
		int				page_loop = RTDS_MEM_ALLOC_PAGE_NUM;
		unsigned long	virt_addr = 0;

		while (0 < cnt) {
			if (cnt < RTDS_MEM_ALLOC_PAGE_NUM) {
				/* check if less than 4 page */
				page_loop = cnt;
				MSG_LOW("[RTDSK]   |rest page[%d] cnt[%d]\n", page_loop, cnt);
			}

			for (retry = 0; retry < RTDS_MEM_ALLOC_PAGE_RETRY; retry++) {
				/* Allocate physically-contiguous pages */
				virt_addr = (unsigned long)alloc_pages_exact(PAGE_SIZE*RTDS_MEM_ALLOC_PAGE_NUM,
															GFP_USER|__GFP_NORETRY);
				MSG_INFO("[RTDSK]   |-virt_addr[0x%08X]\n", (u32)virt_addr);
				if (0 != virt_addr) {
					break;
				}
				congestion_wait(BLK_RW_ASYNC, HZ/50);
			}

			/* Check allocated page frame address */
			if (0 != virt_addr) {
				for (page_cnt = 0; page_cnt < page_loop; page_cnt++) {
					MSG_INFO("[RTDSK]   |virt_addr[0x%08X]\n",
					(u32)(virt_addr + PAGE_SIZE*page_cnt));
					/* change from virtual address to page address */
					pages[(page_num - cnt) + page_cnt] = virt_to_page(virt_addr + PAGE_SIZE*page_cnt);
					MSG_INFO("[RTDSK]   |page[%x]=[0x%08X]\n",
					(page_num - cnt+page_cnt), (u32)(pages[page_num - cnt + page_cnt]));
				}
			} else {
				/* Free every 4 page */
				for (page_cnt = 0; page_cnt < (page_num - cnt); page_cnt += RTDS_MEM_ALLOC_PAGE_NUM) {
					free_pages_exact(page_address(pages[page_cnt]), PAGE_SIZE*RTDS_MEM_ALLOC_PAGE_NUM);
				}

				/* Initialise page descriptor */
				memset(pages, 0, page_num * sizeof(struct page *));

				MSG_ERROR("[RTDSK]ERR| alloc_pages_exact failed.\n");
				ret = SMAP_MEMORY;
				break;
			}

			/* Decrement 4 page */
			cnt -= RTDS_MEM_ALLOC_PAGE_NUM;

			if (0 > cnt) {
				MSG_LOW("[RTDSK]   |cnt[%d]\n", cnt);
				/* remove extra allocated page frame
				*  Now page_loop is 1~3 and page frame allocates every 4 page.
				*  So because (RTDS_MEM_ALLOC_PAGE_NUM - page_loop) is extra page, need to free pagas.
				*/
				for (page_cnt = 0; page_cnt < (RTDS_MEM_ALLOC_PAGE_NUM - page_loop); page_cnt++) {
					MSG_LOW("[RTDSK]   |free_page[0x%08X]\n",
					(u32)(virt_addr + PAGE_SIZE*(page_loop+page_cnt)));
					free_page(virt_addr + PAGE_SIZE*(page_loop+page_cnt));
				}
			}
		}
	}
#endif
	MSG_HIGH("[RTDSK]OUT|[%s] ret[%d]\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_free_page_frame
 * Description: This function is common process to free page.
 * Parameters : page_num	-   number of pages
 *			    pages		-   Page descriptor
 *				create_list	-   App shared memory create list
 * Returns	 : None
 *******************************************************************************/
void rtds_memory_free_page_frame(
	unsigned int				page_num,
	struct page					**pages,
	rtds_memory_create_queue	*create_list
)
{
	int cnt;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |page_num   [0x%08X]\n", page_num);
	MSG_MED("[RTDSK]   |pages      [0x%08X]\n", (u32)pages);
	MSG_MED("[RTDSK]   |create_list[0x%08X]\n", (u32)create_list);

	if (NULL == create_list) {
		/* Free page */
		for (cnt = 0; cnt < page_num; cnt++)
			__free_page(pages[cnt]);

	} else {
		for (cnt = page_num - 1; cnt >= 0; cnt--) {
			__free_page(pages[cnt]);
			create_list->mem_size -= PAGE_SIZE;
			MSG_INFO("[RTDSK]   |create_list->mem_size[0x%08X]\n", (u32)create_list->mem_size);
		}
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_do_map
 * Description: This function is common processing of Map.
 * Parameters : fp			- file descriptor
 *				addr		- address[out]
 *				size		- memory size
 *				pgoff		- page offset
 * Returns	  : SMAP_OK					-   Success
 *				RTDS_MEM_ERR_MAPPING	-   mapping error
 *******************************************************************************/
int rtds_memory_do_map(
	struct file		*fp,
	unsigned long	*addr,
	unsigned long	size,
	unsigned long	pgoff
)
{
	int ret = SMAP_OK;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (u32)fp);
	MSG_MED("[RTDSK]   |address[0x%08X]\n", (u32)addr);
	MSG_MED("[RTDSK]   |size[0x%08X]\n", (u32)size);
	MSG_MED("[RTDSK]   |pgoff[0x%08X]\n", (u32)pgoff);

	/* Mapping */
	down_write(&current->mm->mmap_sem);
	*addr =	 do_mmap_pgoff(fp,
							0,
							size,
							PROT_READ|PROT_WRITE,
							MAP_SHARED,
							pgoff);
	up_write(&current->mm->mmap_sem);

	MSG_MED("[RTDSK]   |do_mmap_pgoff_addr [0x%08X]\n", (u32)addr);

	/* Address check(PAGE alignment) */
	if ((0 != (*addr & RTDS_MEM_ADDR_ERR)) || (0 == *addr)) {
		MSG_ERROR("[RTDSK]ERR| mapping error\n");
		ret = RTDS_MEM_ERR_MAPPING;

		/* Unmap */
		rtds_memory_do_unmap(*addr, size);
		*addr = 0;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret[%d]\n", __func__, ret);
	return ret;
}


/*******************************************************************************
 * Function   : rtds_memory_do_unmap
 * Description: This function is common processing of UnMap.
 * Parameters : user_data	- user data(Not used)
 *				address		- unmap address
 *				size		- unmap size
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_do_unmap(
	unsigned long		address,
	unsigned long		size
)
{
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |address[0x%08X]\n", (u32)address);
	MSG_MED("[RTDSK]   |size[0x%08X]\n", (u32)size);

	down_write(&current->mm->mmap_sem);
	do_munmap(current->mm, address, size);
	up_write(&current->mm->mmap_sem);

	MSG_HIGH("[RTDSK]OUT |[%s]\n", __func__);
	return;
}


/*******************************************************************************
 * Function   : rtds_memory_flush_mmu
 * Description: This function is common processing of UnMap.
 * Parameters : *vm_area	- Memory region
 *				address		- Memory region start address
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_flush_mmu(
	struct vm_area_struct	*vm_area,
	unsigned long			address
)
{
	pgd_t   *pgd;
	pmd_t   *pmd;
	pte_t   *pte;
	unsigned long   phy_pgd;
	unsigned long   phy_pte;

	MSG_INFO("[RTDSK]IN |[%s]\n", __func__);
	MSG_INFO("[RTDSK]   |address[0x%08X]\n", (u32)address);

	pgd = pgd_offset(vm_area->vm_mm, address);
	pmd = pmd_offset(pgd, address);
	pte = pte_offset_map(pmd, address);

	phy_pgd = virt_to_phys(pgd);
	phy_pte = virt_to_phys(pte + RTDS_PTE_OFFSET);

	outer_flush_range(phy_pgd, phy_pgd + sizeof(*pgd));
	outer_flush_range(phy_pte, phy_pte + sizeof(*pte));

	MSG_INFO("[RTDSK]OUT |[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_share_kernel_shared_apmem
 * Description: This function shares AppDomain control shared area between processes.
 * Parameters : apmem_id	-   App shared memory ID
 *				mem_info	-   App shared memory info
 * Returns	  : SMAP_OK					-   Success
 *				SMAP_PARA_NG			-   Parameter error
 *				SMAP_MEMORY				-   No memory
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_share_kernel_shared_apmem(
	rtds_memory_drv_share_mem_param	 *rtds_memory_share_mem
)
{
	int								ret = SMAP_PARA_NG;
	unsigned long					flag;
	rtds_memory_app_memory_table	*mem_table = NULL;
	rtds_memory_app_memory_table	*tmp_mem_table = NULL;
	rtds_memory_create_queue		*entry_p = NULL;
	rtds_memory_create_queue		*tmp_entry_p = NULL;
	pgprot_t						protect;
	int								page_num;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |apmem_id[%d]\n", rtds_memory_share_mem->apmem_id);

	down(&g_rtds_memory_shared_mem);
	list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
		if (rtds_memory_share_mem->apmem_id == mem_table->apmem_id) {
			tmp_mem_table = mem_table;
			break;
		}
	}
	up(&g_rtds_memory_shared_mem);

	if (NULL == tmp_mem_table) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		ret = SMAP_MEMORY;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	memcpy(mem_table, tmp_mem_table, sizeof(*mem_table));

	/* Get App cache kind */
	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
	list_for_each_entry(entry_p, &g_rtds_memory_list_create_mem, queue_header) {
		if ((*(mem_table->pages) == entry_p->page) && (mem_table->app_addr == entry_p->app_addr)) {
			mem_table->app_cache = entry_p->app_cache;
			tmp_entry_p = entry_p;
			break;
		}
	}
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	if (NULL == tmp_entry_p) {
		kfree(mem_table);
		ret = RTDS_MEM_ERR_MAPPING;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	switch (mem_table->app_cache) {
	case RTDS_MEMORY_DRV_WRITEBACK:
		protect = RTDS_MEMORY_PROTECT_WB;
		break;
	case RTDS_MEMORY_DRV_WRITETHROUGH:
		protect = RTDS_MEMORY_PROTECT_WT;
		break;
	case RTDS_MEMORY_DRV_NONCACHE:
		protect = RTDS_MEMORY_PROTECT_NC;
		break;
	case RTDS_MEMORY_DRV_BUFFER_NONCACHE:
		protect = RTDS_MEMORY_PROTECT_NCB;
		break;
	default:
		kfree(mem_table);
		ret = RTDS_MEM_ERR_MAPPING;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	mem_table->app_addr		= 0;
	mem_table->error_code	= 0;
	mem_table->task_info	= current;
	mem_table->trigger		= RTDS_MEM_TRIGGER_SHARE_KERNEL;
	init_MUTEX_LOCKED(&(mem_table->semaphore));

	page_num = RTDS_MEM_GET_PAGE_NUM(mem_table->memory_size);
	mem_table->app_addr = (unsigned long)vmap(mem_table->pages, page_num, VM_ALLOC, protect);

	if (0 == mem_table->app_addr) {
		kfree(mem_table);
		ret = RTDS_MEM_ERR_MAPPING;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Entry list to close App shared area */
	ret = rtds_memory_put_create_mem_list(mem_table->app_addr,
											mem_table->memory_size,
											mem_table->pages,
											mem_table->app_cache);
	if (SMAP_OK != ret) {
		vunmap((const void *)mem_table->app_addr);
		kfree(mem_table);
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	rtds_memory_share_mem->app_addr		= mem_table->app_addr;
	rtds_memory_share_mem->rt_addr		= mem_table->rt_wb_addr;
	rtds_memory_share_mem->rt_addr_nc	= mem_table->rt_nc_addr;
	rtds_memory_share_mem->mem_size		= mem_table->memory_size;
	rtds_memory_share_mem->app_cache	= mem_table->app_cache;
	rtds_memory_share_mem->rt_cache		= mem_table->rt_cache;
	rtds_memory_share_mem->pages		= mem_table->pages;
	MSG_LOW("[RTDSK]   |app_addr[0x%08X]\n", (u32)rtds_memory_share_mem->app_addr);
	MSG_LOW("[RTDSK]   |rt_addr[0x%08X]\n", (u32)rtds_memory_share_mem->rt_addr);
	MSG_LOW("[RTDSK]   |rt_addr_nc[0x%08X]\n", (u32)rtds_memory_share_mem->rt_addr_nc);
	MSG_LOW("[RTDSK]   |mem_size[%d]\n", rtds_memory_share_mem->mem_size);
	MSG_LOW("[RTDSK]   |app_cache[%d]\n", rtds_memory_share_mem->app_cache);
	MSG_LOW("[RTDSK]   |rt_cache[%d]\n", rtds_memory_share_mem->rt_cache);
	MSG_LOW("[RTDSK]   |pages[0x%08X]\n", (u32)rtds_memory_share_mem->pages);

	down(&g_rtds_memory_shared_mem);
	list_add_tail(&(mem_table->list_head), &g_rtds_memory_list_shared_mem);
	up(&g_rtds_memory_shared_mem);

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_share_apmem
 * Description: This function shares AppDomain control shared area between processes(ioctl).
 * Parameters : fp			-   File descriptor
 *				buffer		-   user data
 *				buf_size	-   user data size
 *				map_data	-   mapping data
 * Returns	  : SMAP_OK					-   Success
 *				SMAP_NG					-   Fatal error
 *				SMAP_PARA_NG			-   Parameter error
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_ioctl_share_apmem(
	struct file					*fp,
	char __user					*buffer,
	size_t						buf_size,
	rtds_memory_mapping_data	*map_data
)
{
	int ret = SMAP_PARA_NG;
	rtds_memory_drv_ioctl_mem	   ioctl_mem;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);
	MSG_MED("[RTDSK]   |map_data[0x%08X]\n", (u32)map_data);

	/* Parameter check */
	if (buf_size != sizeof(rtds_memory_drv_ioctl_mem)) {
		MSG_ERROR("[RTDSK]ERR| buf_size error [%d]\n", buf_size);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Copy user data into kernel space */
	ret = copy_from_user(&ioctl_mem, buffer, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_from_user failed[%d]\n", ret);
		ret = SMAP_NG;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}
	MSG_LOW("[RTDSK]   |apmem_id[%d]\n", ioctl_mem.apmem_id);

	ret = rtds_memory_share_shared_apmem(fp, ioctl_mem.apmem_id, map_data);
	if (SMAP_OK == ret) {
		ioctl_mem.app_cache		= map_data->mem_table->app_cache;
		ioctl_mem.rt_cache		= map_data->mem_table->rt_cache;
		ioctl_mem.mem_size		= map_data->mem_table->memory_size;
		ioctl_mem.app_addr		= map_data->mem_table->app_addr;
		ioctl_mem.pages			= map_data->mem_table->pages;
		ioctl_mem.rt_addr		= map_data->mem_table->rt_wb_addr;
		ioctl_mem.rt_addr_nc	= map_data->mem_table->rt_nc_addr;
		MSG_LOW("[RTDSK]   |app_cache[%d]\n", ioctl_mem.app_cache);
		MSG_LOW("[RTDSK]   |rt_cache[%d]\n", ioctl_mem.rt_cache);
		MSG_LOW("[RTDSK]   |mem_size[%d]\n", ioctl_mem.mem_size);
		MSG_LOW("[RTDSK]   |app_addr[0x%08X]\n", (u32)ioctl_mem.app_addr);
		MSG_LOW("[RTDSK]   |pages[0x%08X]\n", (u32)ioctl_mem.pages);
		MSG_LOW("[RTDSK]   |rt_addr[0x%08X]\n", (u32)ioctl_mem.rt_addr);
		MSG_LOW("[RTDSK]   |rt_addr_nc[0x%08X]\n", (u32)ioctl_mem.rt_addr_nc);
	} else {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_share_shared_apmem failed[%d]\n", ret);
	}

	ioctl_mem.err_code = ret;

	/* Copy into user space */
	ret = copy_to_user(buffer, &ioctl_mem, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_to_user failed[%d]\n", ret);
		ret = SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_share_shared_apmem
 * Description: This function shares AppDomain control shared area between processes.
 * Parameters : fp			-   File descriptor
 *				apmem_id	-   App shared memory ID
 *				map_data	-   mapping data
 *				mem_info	-   App shared memory info
 * Returns	  : SMAP_OK					-   Success
 *				SMAP_NG					-   Fatal error
 *				SMAP_PARA_NG			-   Parameter error
 *				SMAP_MEMORY				-   No memory
 *				RTDS_MEM_ERR_MAPPING	-   Mapping error
 *******************************************************************************/
int rtds_memory_share_shared_apmem(
	struct file					*fp,
	unsigned int				apmem_id,
	rtds_memory_mapping_data	*map_data
)
{
	int								ret = SMAP_PARA_NG;
	unsigned long					flag;
	rtds_memory_app_memory_table	*mem_table = NULL;
	rtds_memory_app_memory_table	*tmp_mem_table = NULL;
	rtds_memory_create_queue		*entry_p = NULL;
	rtds_memory_create_queue		*tmp_entry_p = NULL;
	struct page						**k_pages;
	rtds_memory_create_queue		*create_list = NULL;


	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |apmem_id[%d]\n", apmem_id);

	down(&g_rtds_memory_shared_mem);
	list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
		if (apmem_id == mem_table->apmem_id) {
			tmp_mem_table = mem_table;
			break;
		}
	}
	up(&g_rtds_memory_shared_mem);

	if (NULL == tmp_mem_table) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	mem_table = kmalloc(sizeof(*mem_table), GFP_KERNEL);
	if (NULL == mem_table) {
		ret = SMAP_MEMORY;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	memcpy(mem_table, tmp_mem_table, sizeof(*mem_table));

	down(&g_rtds_memory_shared_mem);
	mem_table->app_addr		= 0;
	mem_table->error_code	= 0;
	mem_table->task_info	= current;
	mem_table->trigger		= RTDS_MEM_TRIGGER_SHARE_APP;
	init_MUTEX_LOCKED(&(mem_table->semaphore));

	map_data->data_ent		= true;
	map_data->mapping_flag	= RTDS_MEM_MAPPING_APMEM;
	map_data->mem_table		= mem_table;

	list_add_tail(&(mem_table->list_head), &g_rtds_memory_list_shared_mem);

	up(&g_rtds_memory_shared_mem);

	/* Get App cache kind */
	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
	list_for_each_entry(entry_p, &g_rtds_memory_list_create_mem, queue_header) {
		if ((*(mem_table->pages) == entry_p->page) && (tmp_mem_table->app_addr == entry_p->app_addr)) {
			mem_table->app_cache = entry_p->app_cache;
			map_data->cache_kind = mem_table->app_cache;
			tmp_entry_p = entry_p;
			break;
		}
	}
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	if (NULL == tmp_entry_p) {
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
		ret = RTDS_MEM_ERR_MAPPING;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	ret = rtds_memory_do_map(fp, &(mem_table->app_addr), mem_table->memory_size, 0);
	if (SMAP_OK != ret) {
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}
	MSG_MED("[RTDSK]   |app_addr [0x%08X]\n", (u32)mem_table->app_addr);

	k_pages = kmalloc(RTDS_MEM_GET_PAGE_NUM(mem_table->memory_size) * sizeof(struct page *), GFP_KERNEL);
	if (NULL == k_pages) {
		rtds_memory_do_unmap(mem_table->app_addr, mem_table->memory_size);
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
		ret = SMAP_MEMORY;
		MSG_ERROR("[RTDSK]ERR| kmalloc failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	create_list = kmalloc(sizeof(rtds_memory_create_queue), GFP_KERNEL);
	if (NULL == create_list) {
		kfree(k_pages);
		rtds_memory_do_unmap(mem_table->app_addr, mem_table->memory_size);
		down(&g_rtds_memory_shared_mem);
		list_del(&(mem_table->list_head));
		kfree(mem_table);
		up(&g_rtds_memory_shared_mem);
		ret = SMAP_MEMORY;
		MSG_ERROR("[RTDSK]ERR| kmalloc failed.\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	memcpy(k_pages, mem_table->pages, RTDS_MEM_GET_PAGE_NUM(mem_table->memory_size) * sizeof(struct page *));

	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

	create_list->mem_size	= mem_table->memory_size;
	create_list->page		= *k_pages;
	create_list->app_addr	= mem_table->app_addr;
	create_list->app_cache	= mem_table->app_cache;
	create_list->pages		= k_pages;
	create_list->task_info	= current;

	list_add_tail(&create_list->queue_header, &g_rtds_memory_list_create_mem);

	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}


/*******************************************************************************
 * Function   : rtds_memory_tablewalk
 * Description: This function translates the virtual address into the physical address.
 * Parameters : virt_addr	   - virtual address
 * Returns	  : phys_addr	   - physical address
 *******************************************************************************/
static
unsigned long rtds_memory_tablewalk(
	unsigned long		virt_addr
)
{
	pgd_t			*pgd;
	pmd_t			*pmd;
	pte_t			*pte;
	unsigned long	page_num;
	unsigned long	phys_addr;
	unsigned long	*page_addr;

	MSG_INFO("[RTDSK]IN |[%s]\n", __func__);
	MSG_INFO("[RTDSK]   |virt_addr[0x%08X]\n", (u32)virt_addr);

	if (VMALLOC_START <= virt_addr) {
		page_num = vmalloc_to_pfn((const void *)virt_addr);
		phys_addr = ((page_num << PAGE_SHIFT) | (virt_addr & (PAGE_SIZE-1)));
	} else if (PAGE_OFFSET <= virt_addr) {
		phys_addr = virt_to_phys((void *)virt_addr);
	} else {
		pgd = pgd_offset(current->mm, virt_addr);
		pmd = pmd_offset(pgd, virt_addr);
		pte = pte_offset_map(pmd, virt_addr);

		page_addr = (unsigned long *)((unsigned long)pte + (RTDS_PTE_OFFSET * sizeof(pte)));
		phys_addr = (0xFFFFF000 & (*page_addr)) | (0x00000FFF & virt_addr);
	}

	MSG_INFO("[RTDSK]OUT|[%s] phys_addr[0x%08X]\n", __func__, (u32)phys_addr);
	return phys_addr;
}

/*******************************************************************************
 * Function   : rtds_memory_flush_l2cache
 * Description: This function flushes L2 cache in the specified range.
 * Parameters : start_addr		- Start address
 *				end_addr		- End address
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_flush_l2cache(
	 unsigned long	start_addr,
	 unsigned long	end_addr
)
{
	unsigned long	flush_start = start_addr;
	unsigned long	flush_end   = 0;
	unsigned long	phys_addr   = 0;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |start_addr[0x%08X]\n", (u32)start_addr);
	MSG_MED("[RTDSK]   |end_addr[0x%08X]\n", (u32)end_addr);

	if (start_addr >= end_addr) {
		MSG_ERROR("[RTDSK]ERR| start_addr[0x%08X] >= end_addr[0x%08X].\n", (u32)start_addr, (u32)end_addr);
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return;
	}

	while (flush_start < end_addr) {
		/* Logical address is translated into physical address. */
		phys_addr = rtds_memory_tablewalk(flush_start);
		flush_end = (flush_start + PAGE_SIZE) & (~(PAGE_SIZE-1));
		if (flush_end > end_addr) {
			flush_end = end_addr;
		}

		/* Flush L2 cache in the specified range. */
		outer_flush_range(phys_addr, phys_addr + (flush_end - flush_start));
		flush_start = flush_end;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_inv_l2cache
 * Description: This function clears L2 cache in the specified range.
 * Parameters : start_addr		- Start address
 *				end_addr		- End address
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_inv_l2cache(
	 unsigned long	start_addr,
	 unsigned long	end_addr
)
{
	unsigned long	inv_start = start_addr;
	unsigned long	inv_end   = 0;
	unsigned long	phys_addr = 0;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |start_addr[0x%08X]\n", (u32)start_addr);
	MSG_MED("[RTDSK]   |end_addr[0x%08X]\n", (u32)end_addr);

	if (start_addr >= end_addr) {
		MSG_ERROR("[RTDSK]ERR| start_addr[0x%08X] >= end_addr[0x%08X].\n", (u32)start_addr, (u32)end_addr);
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return;
	}

	while (inv_start < end_addr) {
		/* Logical address is translated into physical address. */
		phys_addr = rtds_memory_tablewalk(inv_start);
		inv_end = (inv_start + PAGE_SIZE) & (~(PAGE_SIZE-1));
		if (inv_end > end_addr) {
			inv_end = end_addr;
		}

		/* Clear L2 cache in the specified range. */
		outer_inv_range(phys_addr, phys_addr + (inv_end - inv_start));
		inv_start = inv_end;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_flush_cache_all
 * Description: This function flushes all the cache.
 * Parameters : None
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_flush_cache_all(
	 void
)
{
	unsigned long flag;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	spin_lock_irqsave(&g_rtds_memory_lock_cache_all, flag);

	/* Flush all the L1 cache. */
	flush_cache_all();

	/* Flush all the L2 cache. */
	outer_flush_all();

	spin_unlock_irqrestore(&g_rtds_memory_lock_cache_all, flag);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}
/*******************************************************************************
 * Function   : rtds_memory_put_create_mem_list
 * Description: This function entries informatin of App shared area to list queue.
 * Parameters : app_addr		- App shared address
 *				mem_size		- memory size
 *				page			- page descriptor
 *				app_cache		- cache type of App domain
 * Returns	  : SMAP_OK			- Success
 *				SMAP_MEMORY		- No memory
 *******************************************************************************/
int rtds_memory_put_create_mem_list(
	unsigned int		app_addr,
	unsigned int		mem_size,
	struct page			**page,
	unsigned long		app_cache
)
{
	int							ret = SMAP_OK;
	unsigned long				flag;
	rtds_memory_create_queue	*create_queue;
	struct page					**k_pages;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |app_addr  [0x%08X]\n", (u32)app_addr);
	MSG_MED("[RTDSK]   |mem_size  [0x%08X]\n", (u32)mem_size);
	MSG_MED("[RTDSK]   |page      [0x%08X]\n", (u32)page);
	MSG_MED("[RTDSK]   |app_cache [0x%08X]\n", (u32)app_cache);

	/* Allocate memory(rtds_memory_rcv_event_queue) */
	create_queue = kmalloc(sizeof(rtds_memory_create_queue), GFP_KERNEL);
	if (NULL == create_queue) {
		MSG_ERROR("[RTDSK]ERR| kmalloc() failed\n");
		ret = SMAP_MEMORY;
	} else {
		k_pages = kmalloc(RTDS_MEM_GET_PAGE_NUM(mem_size) * sizeof(struct page *), GFP_KERNEL);
		if (NULL == k_pages) {
			MSG_ERROR("[RTDSK]ERR| kmalloc failed.\n");
			kfree(create_queue);
			ret = SMAP_MEMORY;
			MSG_HIGH("[RTDSK]OUT|[%s] ret[%d]\n", __func__, ret);
			return ret;
		}
		memcpy(k_pages, page, RTDS_MEM_GET_PAGE_NUM(mem_size) * sizeof(struct page *));

		/* Set data */
		create_queue->app_addr	= app_addr;
		create_queue->mem_size	= mem_size;
		create_queue->page		= *page;
		create_queue->app_cache	= app_cache;
		create_queue->task_info	= current;
		create_queue->pages		= k_pages;

		/* Spin lock*/
		spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

		/* Add entry to list queue */
		list_add_tail(&create_queue->queue_header, &g_rtds_memory_list_create_mem);

		/* Spin unlock*/
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret[%d]\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_get_create_mem_list
 * Description: This function gets memory size from list queue.
 * Parameters : app_addr	-   App address
 *				page		-   Page descriptor
 *				mem_size	-   memory size
 * Returns	  : number of shared process
 *******************************************************************************/
int rtds_memory_get_create_mem_list(
	unsigned int		app_addr,
	struct page			*page,
	unsigned int		*mem_size
)
{
	unsigned long				flag;
	rtds_memory_create_queue	*entry_p = NULL;
	rtds_memory_create_queue	*temp = NULL;
	int							proc_cnt = -1;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |app_addr  [0x%08X]\n", (u32)app_addr);
	MSG_MED("[RTDSK]   |page	  [0x%08X]\n", (u32)page);
	MSG_MED("[RTDSK]   |mem_size  [0x%08X]\n", (u32)mem_size);

	/* Spin lock*/
	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

	if (0 != list_empty(&g_rtds_memory_list_create_mem)) {
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
		MSG_ERROR("[RTDSK]ERR|List is empty.\n");
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return proc_cnt;
	}

	/* Search and entry list */
	list_for_each_entry(entry_p, &g_rtds_memory_list_create_mem, queue_header) {
		if (entry_p->page == page) {
			/* Prevention of the overflow */
			if (RTDS_MEM_PROC_CNT_MAX > proc_cnt) {
				proc_cnt++;
			}

			if ((entry_p->app_addr == app_addr) &&
				(entry_p->task_info->tgid == current->tgid)) {
				MSG_MED("[RTDSK]   |entry_p           [0x%08X]\n", (u32)entry_p);
				MSG_MED("[RTDSK]   |entry_p->app_addr [0x%08X]\n", (u32)entry_p->app_addr);
				MSG_MED("[RTDSK]   |entry_p->mem_size [0x%08X]\n", (u32)entry_p->mem_size);
				MSG_MED("[RTDSK]   |entry_p->page     [0x%08X]\n", (u32)entry_p->page);
				MSG_MED("[RTDSK]   |entry_p->app_cache[0x%08X]\n", (u32)entry_p->app_cache);
				*mem_size = entry_p->mem_size;
				temp = entry_p;
			}
		}
	}

	MSG_LOW("[RTDSK]   |temp entry_p[0x%08X]\n", (u32)temp);

	if (NULL == temp) {
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
		proc_cnt = -1;
		MSG_ERROR("[RTDSK]ERR| List is not found.\n");
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return proc_cnt;
	}

	/* Delete queue from list */
	list_del(&temp->queue_header);

	/* Spin unlock*/
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	kfree(temp->pages);
	kfree(temp);

	if (proc_cnt) {
		*mem_size = 0;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n proc_cnt[%d]", __func__, proc_cnt);
	return proc_cnt;
}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_get_memsize(ioctl)
 * Description: This function get App shared memory size.
 * Parameters : buffer		-   user data
 *				buf_size	-   user data size
 * Returns	  : SMAP_OK				-   Success
 *				SMAP_NG				-   Fatal error
 *				SMAP_PARA_NG		-   Parameter error
 *******************************************************************************/
int rtds_memory_ioctl_get_memsize(
	char __user				*buffer,
	size_t					buf_size
)
{
	int							ret = SMAP_PARA_NG;
	rtds_memory_drv_ioctl_mem	ioctl_mem;
	rtds_memory_create_queue	*entry_p = NULL;
	rtds_memory_create_queue	*temp = NULL;
	unsigned long				flag;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);

	/* Parameter check */
	if (buf_size != sizeof(rtds_memory_drv_ioctl_mem)) {
		MSG_ERROR("[RTDSK]ERR| buf_size error [%d]\n", buf_size);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Copy user data into kernel space */
	ret = copy_from_user(&ioctl_mem, buffer, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_from_user failed[%d]\n", ret);
		ret = SMAP_NG;
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	MSG_MED("[RTDSK]   |app_addr[0x%08X]\n", (u32)ioctl_mem.app_addr);
	MSG_MED("[RTDSK]   |pages[0x%08X]\n", (u32)ioctl_mem.pages);


	/* Spin lock */
	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

	if (0 != list_empty(&g_rtds_memory_list_create_mem)) {
		spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
		MSG_ERROR("[RTDSK]ERR| List is empty.\n");
		MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
		return SMAP_NG;
	}


	/* Get App shared memory size */
	list_for_each_entry(entry_p, &g_rtds_memory_list_create_mem, queue_header) {
		if ((entry_p->app_addr == ioctl_mem.app_addr) && (entry_p->page == ioctl_mem.pages[0])) {
			MSG_MED("[RTDSK]   |entry_p->app_addr[0x%08X]\n", (u32)entry_p->app_addr);
			MSG_MED("[RTDSK]   |entry_p->mem_size[0x%08X]\n", (u32)entry_p->mem_size);
			MSG_MED("[RTDSK]   |entry_p->page    [0x%08X]\n", (u32)entry_p->page);
			temp = entry_p;
			break;
		}
	}

	/* Spin unlock */
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
	if (NULL == temp) {
		MSG_ERROR("[RTDSK]ERR| List is not found.\n");
		ret = SMAP_PARA_NG;
	} else {
		ioctl_mem.mem_size = temp->mem_size;
	}
	/* Set error code */
	ioctl_mem.err_code  = ret;

	/* Copy into user space */
	ret = copy_to_user(buffer, &ioctl_mem, buf_size);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| copy_to_user failed[%d]\n", ret);
		ret = SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_get_pagesinfo(ioctl)
 * Description: This function get memory of page descriptor.
 * Parameters : buffer		-   user data
 *				buf_size	-   user data size
 * Returns	  : SMAP_OK				-   Success
 *				SMAP_NG				-   Fatal error
 *				SMAP_PARA_NG		-   Parameter error
 *******************************************************************************/
int rtds_memory_ioctl_get_pagesinfo(
	char __user				*buffer,
	size_t					buf_size
)
{
	int								ret = SMAP_PARA_NG;
	int								pages_size = 0;
	rtds_memory_drv_ioctl_mem		ioctl_mem;
	rtds_memory_app_memory_table	*mem_table = NULL;
	rtds_memory_app_memory_table	*tmp_mem_table = NULL;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |buffer[0x%08X]\n", (u32)buffer);
	MSG_MED("[RTDSK]   |buf_size[0x%08X]\n", (u32)buf_size);

	/* Parameter check */
	if (buf_size != sizeof(rtds_memory_drv_ioctl_mem)) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Copy user data into kernel space */
	ret = copy_from_user(&ioctl_mem, buffer, buf_size);
	if (0 != ret) {
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}
	MSG_LOW("[RTDSK]   |apmem_id[%d]\n", ioctl_mem.apmem_id);

	down(&g_rtds_memory_shared_mem);
	list_for_each_entry(mem_table, &g_rtds_memory_list_shared_mem, list_head) {
		if (ioctl_mem.apmem_id == mem_table->apmem_id) {
			tmp_mem_table = mem_table;
			break;
		}
	}
	up(&g_rtds_memory_shared_mem);

	if (NULL == tmp_mem_table) {
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
		return ret;
	}

	/* Copy into user space */
	pages_size = (ioctl_mem.mem_size / PAGE_SIZE) * sizeof(struct page *);
	ret = copy_to_user(ioctl_mem.pages, tmp_mem_table->pages, pages_size);
	if (0 != ret) {
		ret = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
	}

	MSG_HIGH("[RTDSK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_reg_kernel_phymem
 * Description: This function set a translation information.
 * Parameters : phy_addr		-   physical address
 *				map_size		-   map size
 *				rt_addr			-   RTDomain logical address
 * Returns	  : SMAP_OK				-   Success
 *******************************************************************************/
int rtds_memory_reg_kernel_phymem(
	unsigned long		phy_addr,
	unsigned long		map_size,
	unsigned long		rt_addr
){
	int ret_code = SMAP_OK;
	rtds_memory_phymem_table *phymem_table = NULL;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |phy_addr[0x%08X]\n", (u32)phy_addr);
	MSG_MED("[RTDSK]   |map_size[%d]\n", (u32)map_size);
	MSG_MED("[RTDSK]   |rt_addr [0x%08X]\n", (u32)rt_addr);

	down(&g_rtds_memory_phy_mem);
	phymem_table = &phy_mem;
	memset(phymem_table, 0, sizeof(*phymem_table));
	phymem_table->phy_addr	= phy_addr;
	phymem_table->map_size	= map_size;
	phymem_table->rt_addr	= rt_addr;
	up(&g_rtds_memory_phy_mem);

	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_unreg_kernel_phymem
 * Description: This function clear a translation information.
 * Parameters : phy_addr		-   physical address
 *				map_size		-   map size
 *				rt_addr			-   RTDomain logical address
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *******************************************************************************/
int rtds_memory_unreg_kernel_phymem(
	unsigned long		phy_addr,
	unsigned long		map_size,
	unsigned long		rt_addr
){
	int ret_code = SMAP_OK;
	rtds_memory_phymem_table *phymem_table = NULL;
	rtds_memory_phymem_table *tmp_phymem_table = NULL;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |phy_addr[0x%08X]\n", (u32)phy_addr);
	MSG_MED("[RTDSK]   |map_size[%d]\n", (u32)map_size);
	MSG_MED("[RTDSK]   |rt_addr [0x%08X]\n", (u32)rt_addr);

	down(&g_rtds_memory_phy_mem);
	tmp_phymem_table = &phy_mem;
	if (phy_addr == tmp_phymem_table->phy_addr) {
		phymem_table = tmp_phymem_table;
		MSG_LOW("[RTDSK]   |phymem exist.\n");
	}
	if (NULL == phymem_table) {
		up(&g_rtds_memory_phy_mem);
		MSG_HIGH("[RTDSK]   |phymem info. not found.\n");
		goto out;
	}
	if ((map_size != phymem_table->map_size) ||
		(rt_addr != phymem_table->rt_addr)) {
		up(&g_rtds_memory_phy_mem);
		ret_code = SMAP_PARA_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		MSG_LOW("[RTDSK]   |map_size[0x%08X] phymem_table->map_size[0x%08X]\n", (u32)map_size, (u32)phymem_table->map_size);
		MSG_LOW("[RTDSK]   |rt_addr [0x%08X] phymem_table->rt_addr [0x%08X]\n", (u32)rt_addr,  (u32)phymem_table->rt_addr);
		goto out;
	}
	phymem_table->phy_addr	= 0;
	phymem_table->map_size	= 0;
	phymem_table->rt_addr	= 0;
	up(&g_rtds_memory_phy_mem);
out:
	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_change_kernel_phymem_address
 * Description: This function change physical address to logical address
 * Parameters : phy_addr		-   physical address
 *				*rt_addr		-   RTDomain logical address
 * Returns	  : SMAP_OK			-   Success
 *				SMAP_PARA_NG	-   Parameter error
 *******************************************************************************/
int rtds_memory_change_kernel_phymem_address(
	unsigned long		phy_addr,
	unsigned long		*rt_addr
){
	int ret_code = SMAP_OK;
	rtds_memory_phymem_table *phymem_table = NULL;
	rtds_memory_phymem_table *tmp_phymem_table = NULL;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |phy_addr[0x%08X]\n", (u32)phy_addr);

	down(&g_rtds_memory_phy_mem);
	tmp_phymem_table = &phy_mem;
	if ((phy_addr >= tmp_phymem_table->phy_addr) &&
		(phy_addr < (tmp_phymem_table->phy_addr + tmp_phymem_table->map_size))) {
		phymem_table = tmp_phymem_table;
		MSG_LOW("[RTDSK]   |phy_addr OK");
	}
	if (NULL == phymem_table) {
		up(&g_rtds_memory_phy_mem);
		ret_code = SMAP_PARA_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		goto out;
	}
	*rt_addr	= phymem_table->rt_addr + (phy_addr - phymem_table->phy_addr);
	up(&g_rtds_memory_phy_mem);

	MSG_MED("[RTDSK]   |rt_addr [0x%08X]\n", (u32)*rt_addr);
out:
	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}

/*******************************************************************************
 * Function   : rtds_memory_ioctl_change_phymem_address(ioctl)
 * Description: This function change physical address to logical address
 * Parameters : buffer		-   user data
 *				cnt			-   user data size
 * Returns	  : SMAP_OK				-   Success
 *				SMAP_NG				-   Fatal error
 *				SMAP_PARA_NG		-   Parameter error
 *******************************************************************************/
int rtds_memory_ioctl_change_phymem_address(
	char __user			*buffer,
	size_t				cnt
){
	int ret_code = SMAP_OK;
	rtds_memory_drv_ioctl_phymem ioctl_phymem;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	if (cnt != sizeof(ioctl_phymem)) {
		ret_code = SMAP_PARA_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		goto out;
	}

	ret_code = copy_from_user(&ioctl_phymem, buffer, cnt);

	if (0 != ret_code) {
		ret_code = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		goto out;
	}

	ret_code = rtds_memory_change_kernel_phymem_address(
								ioctl_phymem.phy_addr,
				(unsigned long *)&ioctl_phymem.app_addr);

	if (SMAP_OK != ret_code) {
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		goto out;
	}

	ret_code = copy_to_user(buffer, &ioctl_phymem, cnt);

	if (0 != ret_code) {
		ret_code = SMAP_NG;
		MSG_ERROR("[RTDSK]ERR|[%s][%d]\n", __func__, __LINE__);
		goto out;
	}
out:
	MSG_HIGH("[RTDSK]OUT|[%s] ret_code = %d\n", __func__, ret_code);
	return ret_code;
}
/*******************************************************************************
 * Function   : rtds_memory_leak_check_page
 * Description: This function checks page frame leak.
 * Parameters : None
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_leak_check_page_frame(
		void
)
{
	unsigned long 				flag;
	rtds_memory_create_queue	*this_p = NULL;
	rtds_memory_create_queue	*next_p = NULL;
	rtds_memory_create_queue	*share_p = NULL;
	int							proc_cnt = -1;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);

	if (0 != list_empty(&g_rtds_memory_list_create_mem)) {
		MSG_LOW("[RTDSK]   | List is empty.\n");
		goto out;
	}

	list_for_each_entry_safe(this_p, next_p, &g_rtds_memory_list_create_mem, queue_header) {
		MSG_LOW("[RTDSK]   |---\n");
		MSG_LOW("[RTDSK]   |state[0x%08X]\n", (u32)this_p->task_info->state);
		MSG_LOW("[RTDSK]   |flags[0x%08X]\n", (u32)this_p->task_info->flags);
		MSG_LOW("[RTDSK]   |pid[%d]\n", (u32)this_p->task_info->pid);
		MSG_LOW("[RTDSK]   |app_addr[0x%08X]\n", (u32)this_p->app_addr);
		MSG_LOW("[RTDSK]   |page[0x%08X]\n", (u32)this_p->page);
		MSG_LOW("[RTDSK]   |pages[0x%08X]\n", (u32)this_p->pages);
		MSG_LOW("[RTDSK]   |mem_size[0x%08X]\n", (u32)this_p->mem_size);
		MSG_LOW("[RTDSK]   |app_cache[0x%08X]\n", (u32)this_p->app_cache);
		MSG_LOW("[RTDSK]   |---\n");
		if (this_p->task_info->flags & PF_EXITPIDONE) {
			/* check if process is shared */
			list_for_each_entry(share_p, &g_rtds_memory_list_create_mem, queue_header) {
				if (this_p->page == share_p->page) {
					if (RTDS_MEM_PROC_CNT_MAX > proc_cnt) {
						proc_cnt++;
					}
				}
			}
			spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);
			MSG_LOW("[RTDSK]   |page frame leak is found.proc_cnt[%d]\n", proc_cnt);

			if (!proc_cnt) {
				rtds_memory_free_page_frame(RTDS_MEM_GET_PAGE_NUM(this_p->mem_size),
											this_p->pages,
											this_p);
			}
			spin_lock_irqsave(&g_rtds_memory_lock_create_mem, flag);
			list_del(&this_p->queue_header);
			kfree(this_p->pages);
			kfree(this_p);
		}
	}

out:
	spin_unlock_irqrestore(&g_rtds_memory_lock_create_mem, flag);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;
}

/*******************************************************************************
 * Function   : rtds_memory_leak_check_mpro
 * Description: This function check the memory leak.
 * Parameters : None
 * Returns	  : None
 *******************************************************************************/
void rtds_memory_leak_check_mpro(
	void
)
{
	rtds_memory_app_memory_table	*mem_table = NULL;
	rtds_memory_app_memory_table	*next_p = NULL;
	int								ret;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	down(&g_rtds_memory_shared_mem);

	if (0 != list_empty(&g_rtds_memory_list_shared_mem)) {
		MSG_LOW("[RTDSK]   | List is empty.\n");
		goto out;
	}

	list_for_each_entry_safe(mem_table, next_p, &g_rtds_memory_list_shared_mem, list_head) {
		MSG_MED("[RTDSK]   |state[0x%08X]\n", (u32)mem_table->task_info->state);
		MSG_MED("[RTDSK]   |flags[0x%08X]\n", (u32)mem_table->task_info->flags);
		if (mem_table->task_info->flags & PF_EXITPIDONE) {
			up(&g_rtds_memory_shared_mem);
			switch (mem_table->event) {
			case RTDS_MEM_MAP_PNC_EVENT:
			case RTDS_MEM_UNMAP_PNC_EVENT:
				ret = rtds_memory_unmap_pnc_mpro(mem_table->app_addr, mem_table->apmem_id);
				MSG_MED("[RTDSK]   |result rtds_memory_unmap_pnc_mpro [%d]\n", ret);
				break;
			default:
				ret = rtds_memory_unmap_mpro(mem_table->rt_wb_addr, mem_table->memory_size);
				MSG_MED("[RTDSK]   |result rtds_memory_unmap_mpro [%d]\n", ret);
				break;
			}
			down(&g_rtds_memory_shared_mem);
		}
	}

out:
	up(&g_rtds_memory_shared_mem);

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);
	return;

}