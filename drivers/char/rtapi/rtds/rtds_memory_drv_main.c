/*
 * rtds_memory_drv_main.c
 *	 RT domain shared memory driver API function file.
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
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/semaphore.h>

#include "log_kernel.h"
#include "rtds_memory_drv.h"
#include "rtds_memory_drv_main.h"
#include "rtds_memory_drv_common.h"
#include "rtds_memory_drv_sub.h"
#include "iccom_drv.h"
#include "system_rtload_internal.h"

/* file operation */
static struct file_operations	g_rtds_memory_fops = {
								.owner			= THIS_MODULE,
								.open			= rtds_memory_drv_open,
								.release		= rtds_memory_drv_close,
								.mmap			= rtds_memory_drv_mapping,
								.unlocked_ioctl = rtds_memory_drv_ioctl
};

/* miscdevice info */
static struct miscdevice	g_rtds_memory_device;

/* Asynchronous thread info */
static struct task_struct	*g_rtds_memory_thread_info;

/* Section info */
rtds_memory_section_info	g_rtds_memory_section_info;

/* ICCOM driver handle */
void						*g_rtds_memory_iccom_handle;

struct completion			g_rtds_memory_completion;
struct semaphore			g_rtds_memory_apmem_rttrig_sem;
spinlock_t					g_rtds_memory_lock_recv_queue;
spinlock_t					g_rtds_memory_lock_cache_all;
struct list_head			g_rtds_memory_list_rcv_event;
static unsigned int			g_rtds_memory_mpro_startup_flag = RTDS_MEM_MPRO_INACTIVE;
spinlock_t					g_rtds_memory_lock_create_mem;
struct list_head			g_rtds_memory_list_create_mem;
struct list_head			g_rtds_memory_list_mpro;
struct semaphore			g_rtds_memory_mpro_sem;
spinlock_t					g_rtds_memory_lock_mpro;
struct list_head			g_rtds_memory_list_shared_mem;
struct semaphore			g_rtds_memory_shared_mem;
struct list_head			g_rtds_memory_list_map_rtmem;
spinlock_t					g_rtds_memory_lock_map_rtmem;
struct list_head			g_rtds_memory_list_reg_phymem;
struct semaphore			g_rtds_memory_phy_mem;
struct semaphore			g_rtds_memory_leak_sem;

/*******************************************************************************
 * Function   : rtds_memory_drv_open
 * Description: This function open RTDS MEMORY driver.
 * Parameters : inode   -   inode information
 *				fp		-   file descriptor
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_MEMORY -   No memory
 *				SMAP_NG		-   FatalError
 *******************************************************************************/
int rtds_memory_drv_open(
		struct inode	*inode,
		struct file		*fp
)
{
	rtds_memory_drv_data		*rtds_memory_data_p;
	rtds_memory_mpro_control	*mpro_control_p;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |inode_p[0x%08X]\n", (u32)inode);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (u32)fp);
	MSG_LOW("[RTDSK]   |major[%d]\n", imajor(inode));
	MSG_LOW("[RTDSK]   |minor[%d]\n", iminor(inode));

	/* Allocate memory for rtds_memory_mpro_control */
	mpro_control_p = kmalloc(sizeof(*mpro_control_p), GFP_KERNEL);
	if (NULL == mpro_control_p) {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_mpro_control NotMemory\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = SMAP_MEMORY\n", __func__);
		return SMAP_MEMORY;
	}

	/* Allocate memory for rtds_memory_drv_data */
	rtds_memory_data_p = kmalloc(sizeof(*rtds_memory_data_p), GFP_KERNEL);
	if (NULL == rtds_memory_data_p) {
		MSG_ERROR("[RTDSK]ERR| rtds_memory_drv_data NotMemory\n");
		MSG_HIGH("[RTDSK]OUT|[%s] ret = SMAP_MEMORY\n", __func__);
		kfree(mpro_control_p);
		return SMAP_MEMORY;
	}

	memset(mpro_control_p, 0, sizeof(*mpro_control_p));
	memset(rtds_memory_data_p, 0, sizeof(*rtds_memory_data_p));

	fp->private_data = rtds_memory_data_p;
	rtds_memory_data_p->mpro_control = mpro_control_p;

	MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_OK\n", __func__);

	return SMAP_OK;
}

/*******************************************************************************
 * Function   : rtds_memory_drv_close
 * Description: This function close RTDS MEMORY driver.
 * Parameters : inode   -   inode information
 *				fp		-   file descriptor
 * Returns	  : SMAP_OK		-   Success
 *
 *******************************************************************************/
int rtds_memory_drv_close(
		struct inode	*inode,
		struct file		*fp
)
{
	rtds_memory_drv_data		*rtds_memory_data_p = fp->private_data;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |inode_p[0x%08X]\n", (u32)inode);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (u32)fp);

	/* Release memory for rtds_memory_drv_data */
	if (rtds_memory_data_p) {
		kfree(rtds_memory_data_p->mpro_control);
		kfree(rtds_memory_data_p);
		fp->private_data = NULL;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_OK\n", __func__);

	return SMAP_OK;
}

/*******************************************************************************
 * Function   : rtds_memory_drv_ioctl
 * Description: This function controls I/O for RTDS MEMORY driver.
 * Parameters : inode   -   inode information
 *				fp		-   file descriptor
 * Returns	  : SMAP_OK		-   Success
 *				SMAP_NG		-   Fatal error
 *				SMAP_MEMORY -   No memory
 *
 *******************************************************************************/
long rtds_memory_drv_ioctl(
		struct file		*fp,
		unsigned int	cmd,
		unsigned long	arg
)
{
	int						ret = SMAP_OK;
	rtds_memory_drv_data	*rtds_memory_data_p = fp->private_data;
	rtds_memory_ioctl_data	ioctl_data;
	struct task_struct		*task = current;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (u32)fp);
	MSG_MED("[RTDSK]   |command[0x%08X]\n", (u32)cmd);
	MSG_MED("[RTDSK]   |argument[0x%08X]\n", (u32)arg);

	task->flags |= PF_FREEZER_SKIP;
	memset(&ioctl_data, 0, sizeof(ioctl_data));

	if (0 != arg) {
		/* Check validation of user space pointer */
		if (!access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd))) {
			ret = SMAP_NG;
			MSG_ERROR("[RTDSK]ERR| parameter error\n");
			MSG_HIGH("[RTDSK]OUT|[%s]ret = %d\n", __func__, ret);
			return ret;
		}

		/* Copy data from user space to kernel space */
		if (copy_from_user(&ioctl_data, (int __user *)arg, sizeof(ioctl_data))) {
			ret = SMAP_NG;
			MSG_ERROR("[RTDSK]ERR| copy_from_user error\n");
			MSG_HIGH("[RTDSK]OUT|[%s]ret = %d\n", __func__, ret);
			return ret;
		}
		MSG_LOW("[RTDSK]   |ioctl_data->size[%d]\n", ioctl_data.data_size);
	}

	switch (cmd) {
	case IOCTL_MEM_CMD_WR_SET_MPRO:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_SET_MPRO\n");

		if (RTDS_MEM_MPRO_ACTIVE == g_rtds_memory_mpro_startup_flag) {
			MSG_MED("[RTDSK]   |MPRO is already activated.\n");
			break;
		}

		/* Start Mpro */
		ret = rtds_memory_drv_init_mpro(fp);
		if (SMAP_OK == ret) {
			g_rtds_memory_mpro_startup_flag = RTDS_MEM_MPRO_ACTIVE;
			MSG_MED("[RTDSK]   |Start MPRO.\n");
			while (1) {
				rtds_memory_check_shared_apmem(fp, &(rtds_memory_data_p->mem_map));
			}
		} else {
			/* #MU2SYS2264 add -S- */
			MSG_ERROR("[RTDSK]ERR| rtds_memory_drv_init_mpro failed ret[%d]\n", ret);
			panic("[RTDSK]ERR|[%s][%d]rtds_memory_drv_init_mpro failed[%d]\n", __func__, __LINE__, ret);
			/* #MU2SYS2264 add -E- */
		}

		break;

	case IOCTL_MEM_CMD_WR_INIT:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_INIT\n");

		/* Initialise RTDS memory handle parameter */
		ret = rtds_memory_ioctl_init_data(fp, (void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_EXIT:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_EXIT\n");

		/* Unmap shared RT memory */
		if (0 != rtds_memory_data_p->var_app_addr) {
			(void)rtds_memory_close_shared_rtmem(rtds_memory_data_p->var_app_addr,
												  rtds_memory_data_p->var_area_len);
		}
		break;

	case IOCTL_MEM_CMD_WR_OPEN_MEMORY:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_OPEN_MEMORY\n");

		/* Create App shared memory */
		ret = rtds_memory_ioctl_open_apmem(fp,
											(void __user *)ioctl_data.data,
											ioctl_data.data_size,
											&(rtds_memory_data_p->mem_map)
											);
		break;

	case IOCTL_MEM_CMD_WR_CLOSE_MEMORY:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_CLOSE_MEMORY\n");

		/* Close App shared memory */
		ret = rtds_memory_ioctl_close_apmem((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_SHARE_MEMORY:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_SHARE_MEMORY\n");

		/* Share App shared memory */
		ret = rtds_memory_ioctl_share_apmem(fp,
											(void __user *)ioctl_data.data,
											ioctl_data.data_size,
											&(rtds_memory_data_p->mem_map)
											);
		break;

	case IOCTL_MEM_CMD_WR_FLUSH_CACHE:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_FLUSH_CACHE\n");

		ret = rtds_memory_ioctl_cache_flush((void __user *)ioctl_data.data,
											ioctl_data.data_size
											);
		break;

	case IOCTL_MEM_CMD_WR_CLEAR_CACHE:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_CLEAR_CACHE\n");

		ret = rtds_memory_ioctl_cache_clear((void __user *)ioctl_data.data,
											ioctl_data.data_size
											);
		break;

	case IOCTL_MEM_CMD_WR_RTMAP:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_RTMAP\n");

		ret = rtds_memory_ioctl_map_mpro((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_RTUNMAP:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_RTUNMAP\n");

		ret = rtds_memory_ioctl_unmap_mpro((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_RTMAP_PNC:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_RTMAP_PNC\n");

		ret = rtds_memory_ioctl_map_pnc_mpro((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_RTUNMAP_PNC:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_RTUNMAP_PNC\n");

		ret = rtds_memory_ioctl_unmap_pnc_mpro((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_RTMAP_PNC_NMA:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_RTMAP_PNC_NMA\n");

		ret = rtds_memory_ioctl_map_pnc_nma_mpro((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_GET_MEMSIZE:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_GET_MEMSIZE\n");

		/* Get App shared memory size */
		ret = rtds_memory_ioctl_get_memsize((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_GET_PAGESINFO:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_GET_PAGESINFO\n");

		/* Get memory of page descriptor */
		ret = rtds_memory_ioctl_get_pagesinfo((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	case IOCTL_MEM_CMD_WR_PHYMEM:
		MSG_MED("[RTDSK]   |IOCTL_MEM_CMD_WR_PHYMEM\n");
		ret = rtds_memory_ioctl_change_phymem_address((void __user *)ioctl_data.data, ioctl_data.data_size);
		break;

	default:
		MSG_ERROR("[RTDSK]ERR| No command\n");

		ret = SMAP_NG;
		break;
	}

	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| ioctl function error\n");
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret = %d\n", __func__, ret);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_drv_mapping
 * Description: This function does mapping.
 * Parameters : inode	-   inode information
 *				fp		-   file descriptor
 * Returns	: 0			-   Success(always)
 *
 *******************************************************************************/
int rtds_memory_drv_mapping(
		struct file				*fp,
		struct vm_area_struct	*vm_area
)
{
	int						ret;
	rtds_memory_drv_data	*rtds_memory_data_p = fp->private_data;
	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);
	MSG_MED("[RTDSK]   |fp[0x%08X]\n", (unsigned int)fp);
	MSG_MED("[RTDSK]   |vm_pgoff[0x%08X]\n", (unsigned int)vm_area->vm_pgoff);

	/* Mapping memory */
	ret = rtds_memory_map_shared_memory(&(rtds_memory_data_p->mem_map), vm_area);
	/*
	 * Always return 0, because this function is called by OS.
	 */
	MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_OK\n", __func__);
	return 0;
}

/*******************************************************************************
 * Function   : rtds_memory_thread_apmem_rttrig
 * Description: This function is asynchronous receive thread from RT domain.
 * Parameters : none
 * Returns	  : SMAP_OK -   Success
 *******************************************************************************/
int rtds_memory_thread_apmem_rttrig(
		void	*vp
)
{
	int							ret = SMAP_OK;
	rtds_memory_rcv_event_queue	*recv_queue;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	while (1) {
		/* Release semaphore */
		up(&g_rtds_memory_apmem_rttrig_sem);

		/* Wait for completion from RT Domain */
		wait_for_completion(&g_rtds_memory_completion);

		/* Get receive event */
		recv_queue = rtds_memory_get_recv_queue();
		if (NULL == recv_queue) {
			MSG_ERROR("[RTDSK]ERR| receive queue is NULL\n");
			panic("[RTDSK]ERR|[%s][%d]receive queue is NULL\n", __func__, __LINE__);
		} else {
			MSG_LOW("[RTDSK]   |event	 [0x%08X]\n", recv_queue->event);
			MSG_LOW("[RTDSK]   |mem_size  [0x%08X]\n", recv_queue->mem_size);
			MSG_LOW("[RTDSK]   |rt_cache  [0x%08X]\n", recv_queue->rt_cache);
			MSG_LOW("[RTDSK]   |rt_trigger[0x%08X]\n", recv_queue->rt_trigger);
			MSG_LOW("[RTDSK]   |apmem_id  [0x%08X]\n", recv_queue->apmem_id);

			switch (recv_queue->event) {
			case RTDS_MEM_DRV_EVENT_APMEM_OPEN:
				MSG_MED("[RTDSK]   |RTDS_MEM_DRV_EVENT_APMEM_OPEN\n");
				/* Open App shared memory */
				ret = rtds_memory_open_rttrig_shared_apmem(recv_queue->mem_size,
															recv_queue->rt_cache,
															recv_queue->rt_trigger);
				if (SMAP_OK != ret) {
					MSG_ERROR("[RTDSK]ERR| rtds_memory_open_rttrig_shared_apmem failed ret[%d]\n", ret);
					panic("[RTDSK]ERR|[%s][%d]rtds_memory_open_rttrig_shared_apmem failed[%d]\n", __func__, __LINE__, ret);
				}
				break;

			case RTDS_MEM_DRV_EVENT_APMEM_CLOSE:
				MSG_MED("[RTDSK]   |RTDS_MEM_DRV_EVENT_APMEM_CLOSE\n");
				/* Close App shared memory */
				ret = rtds_memory_close_rttrig_shared_apmem(recv_queue->apmem_id);
				if (SMAP_OK != ret) {
					MSG_ERROR("[RTDSK]ERR| rtds_memory_close_rttrig_shared_apmem failed ret[%d]\n", ret);
					panic("[RTDSK]ERR|[%s][%d]rtds_memory_close_rttrig_shared_apmem failed[%d]\n", __func__, __LINE__, ret);
				}
				break;

			case RTDS_MEM_DRV_EVENT_APMEM_DELETE:
				MSG_MED("[RTDSK]   |RTDS_MEM_DRV_EVENT_APMEM_DELETE\n");

				break;

			case RTDS_MEM_DRV_EVENT_FATAL:
				MSG_MED("[RTDSK]   |RTDS_MEM_DRV_EVENT_FATAL\n");
				rtds_memory_dump_mpro();
				break;

			default:
				MSG_ERROR("[RTDSK]ERR| No EVENT[0x%08X]\n", recv_queue->event);
				panic("[RTDSK]ERR| thread_apmem illegal event\n");
				break;
			}
			/* Delete receive event */
			rtds_memory_delete_recv_queue(recv_queue);
		}
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_OK\n", __func__);
	return ret;
}

/*******************************************************************************
 * Function   : rtds_memory_init_module
 * Description: This function initialise RTDS MEMORY driver.
 * Parameters : none
 * Returns	  : SMAP_OK -   Success
 *				SMAP_NG -   Fatal error
 *******************************************************************************/
int rtds_memory_init_module(
		void
)
{
	int							ret;
	iccom_drv_init_param		iccom_init;
	iccom_drv_cleanup_param		iccom_cleanup;
	get_section_header_param	section;
	system_rt_section_header	section_header;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Register device driver */
	g_rtds_memory_device.name	= RTDS_MEMORY_DRIVER_NAME;
	g_rtds_memory_device.fops	= &g_rtds_memory_fops;
	g_rtds_memory_device.minor	= MISC_DYNAMIC_MINOR;
	ret = misc_register(&g_rtds_memory_device);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| misc_register failed ret[%d]\n", ret);
		MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_NG\n", __func__);
		return SMAP_NG;
	}

	memset(&iccom_init, 0, sizeof(iccom_init));
	/* Set callback for asynchronous response */
	iccom_init.comp_notice = rtds_memory_rcv_comp_notice;

	/* Create ICCOM driver handle */
	g_rtds_memory_iccom_handle = iccom_drv_init(&iccom_init);
	if (NULL == g_rtds_memory_iccom_handle) {
		MSG_ERROR("[RTDSK]ERR| iccom_drv_init failed\n");
		ret = misc_deregister(&g_rtds_memory_device);
		if (0 != ret) {
			MSG_ERROR("[RTDSK]ERR| misc_deregister failed ret[%d]\n", ret);
		}

		MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_NG\n", __func__);

		return SMAP_NG;
	}

	/* Initialise section info */
	memset(&g_rtds_memory_section_info, 0, sizeof(g_rtds_memory_section_info));

	/* Initialise LIST head */
	memset(&g_rtds_memory_list_rcv_event, 0, sizeof(g_rtds_memory_list_rcv_event));
	memset(&g_rtds_memory_list_create_mem, 0, sizeof(g_rtds_memory_list_create_mem));
	memset(&g_rtds_memory_list_mpro, 0, sizeof(g_rtds_memory_list_mpro));
	memset(&g_rtds_memory_list_shared_mem, 0, sizeof(g_rtds_memory_list_shared_mem));
	memset(&g_rtds_memory_list_map_rtmem, 0, sizeof(g_rtds_memory_list_map_rtmem));
	memset(&g_rtds_memory_list_reg_phymem, 0, sizeof(g_rtds_memory_list_reg_phymem));

	INIT_LIST_HEAD(&g_rtds_memory_list_rcv_event);
	INIT_LIST_HEAD(&g_rtds_memory_list_create_mem);
	INIT_LIST_HEAD(&g_rtds_memory_list_mpro);
	INIT_LIST_HEAD(&g_rtds_memory_list_shared_mem);
	INIT_LIST_HEAD(&g_rtds_memory_list_map_rtmem);
	INIT_LIST_HEAD(&g_rtds_memory_list_reg_phymem);

	/* Initialise spin_lock */
	spin_lock_init(&g_rtds_memory_lock_recv_queue);
	spin_lock_init(&g_rtds_memory_lock_cache_all);
	spin_lock_init(&g_rtds_memory_lock_create_mem);
	spin_lock_init(&g_rtds_memory_lock_mpro);
	spin_lock_init(&g_rtds_memory_lock_map_rtmem);

	/* Initialise completion */
	init_completion(&g_rtds_memory_completion);

	/* Initialise semapore */
	init_MUTEX(&g_rtds_memory_apmem_rttrig_sem);
	init_MUTEX_LOCKED(&g_rtds_memory_mpro_sem);
	init_MUTEX(&g_rtds_memory_shared_mem);
	init_MUTEX(&g_rtds_memory_phy_mem);
	init_MUTEX(&g_rtds_memory_leak_sem);

	/* Get section info */
	section.section_header = &section_header;
	ret = sys_get_section_header(&section);
	if (SMAP_OK != ret) {
		MSG_ERROR("[RTDSK]ERR| sys_get_section_header failed[%d]\n", ret);
		iccom_cleanup.handle = g_rtds_memory_iccom_handle;
		iccom_drv_cleanup(&iccom_cleanup);
		ret = misc_deregister(&g_rtds_memory_device);
		if (0 != ret) {
			MSG_ERROR("[RTDSK]ERR| misc_deregister failed ret[%d]\n", ret);
		}
		MSG_HIGH("[RTDSK]OUT|[%s] ret = SMAP_NG\n", __func__);
		return SMAP_NG;
	}

	g_rtds_memory_section_info.var_address	= section_header.memmpl_address;
	g_rtds_memory_section_info.var_length	= section_header.memmpl_size;

	/* I/O remap */
	g_rtds_memory_section_info.kernel_var_addr = ioremap_nocache(g_rtds_memory_section_info.var_address, g_rtds_memory_section_info.var_length);
	if (NULL == g_rtds_memory_section_info.kernel_var_addr) {
		MSG_ERROR("[RTDSK]ERR| ioremap_nocache failed\n");
		g_rtds_memory_section_info.var_address = 0;
		g_rtds_memory_section_info.var_length  = 0;
		iccom_cleanup.handle = g_rtds_memory_iccom_handle;
		iccom_drv_cleanup(&iccom_cleanup);
		ret = misc_deregister(&g_rtds_memory_device);
		if (0 != ret) {
			MSG_ERROR("[RTDSK]ERR| misc_deregister failed ret[%d]\n", ret);
		}

		MSG_HIGH("[RTDSK]OUT|[%s] ret = SMAP_NG\n", __func__);
		return SMAP_NG;
	}

	MSG_MED("[RTDSK]   |addr(section)[0x%08X]\n", (u32)g_rtds_memory_section_info.var_address);
	MSG_MED("[RTDSK]   |length(section)[0x%08X]\n", (u32)g_rtds_memory_section_info.var_length);
	MSG_MED("[RTDSK]   |addr(kernel)[0x%08X]\n", (u32)g_rtds_memory_section_info.kernel_var_addr);

	/* Run asynchronous thread */
	g_rtds_memory_thread_info = kthread_run(rtds_memory_thread_apmem_rttrig,
											  NULL,
											  "rtds_memory_th_async");
	if (NULL == g_rtds_memory_thread_info) {
		MSG_ERROR("[RTDSK]ERR| kthread_run failed g_rtds_memory_thread_info[NULL]\n");

		g_rtds_memory_section_info.var_address = 0;
		g_rtds_memory_section_info.var_length  = 0;
		g_rtds_memory_section_info.kernel_var_addr = 0;
		iccom_cleanup.handle = g_rtds_memory_iccom_handle;
		iccom_drv_cleanup(&iccom_cleanup);
		ret = misc_deregister(&g_rtds_memory_device);
		if (0 != ret) {
			MSG_ERROR("[RTDSK]ERR| misc_deregister failed ret[%d]\n", ret);
		}

		MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_NG\n", __func__);

		return SMAP_NG;
	}

	MSG_HIGH("[RTDSK]OUT|[%s]ret = SMAP_OK\n", __func__);

	return SMAP_OK;
}

/*******************************************************************************
 * Function   : rtds_memory_exit_module
 * Description: This function exits RTDS MEMORY driver.
 * Parameters : none
 * Returns	  : none
 *
 *******************************************************************************/
void rtds_memory_exit_module(
		void
)
{
	int						ret;
	iccom_drv_cleanup_param	iccom_cleanup;

	MSG_HIGH("[RTDSK]IN |[%s]\n", __func__);

	/* Stop asynchronous thread */
	ret = kthread_stop(g_rtds_memory_thread_info);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| kthread_stop failed ret[%d]\n", ret);
	}

	/* Cleanup iccom driver handle */
	iccom_cleanup.handle = g_rtds_memory_iccom_handle;
	iccom_drv_cleanup(&iccom_cleanup);

	/* Unregister device driver */
	ret = misc_deregister(&g_rtds_memory_device);
	if (0 != ret) {
		MSG_ERROR("[RTDSK]ERR| misc_deregister failed ret[%d]\n", ret);
	}

	MSG_HIGH("[RTDSK]OUT|[%s]\n", __func__);

	return;
}
module_init(rtds_memory_init_module);
module_exit(rtds_memory_exit_module);
