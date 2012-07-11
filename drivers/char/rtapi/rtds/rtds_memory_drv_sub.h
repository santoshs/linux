/*
 * rtds_memory_drv_sub.h
 *	 RT domain shared memory device driver API function file.
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
#ifndef __RTDS_MEMORY_DRV_SUB_H__
#define __RTDS_MEMORY_DRV_SUB_H__

/* ******************************* CONSTANTS ******************************** */
/* CACHE Parameters */
#define RTDS_MEM_CACHE_MASK		(0xFFFFFFB3)
#define RTDS_MEM_WRITE_BACK		(0xC)
#define RTDS_MEM_WRITE_THROUGH	(0x8)
#define RTDS_MEM_NONCACHE		(0x0)
#define RTDS_MEM_BUF_NONCACHE	(0x4)

#define RTDS_MEMORY_PROTECT_WB	(0x183 | RTDS_MEM_WRITE_BACK)
#define RTDS_MEMORY_PROTECT_WT	(0x183 | RTDS_MEM_WRITE_THROUGH)
#define RTDS_MEMORY_PROTECT_NC	(0x183 | RTDS_MEM_NONCACHE)
#define RTDS_MEMORY_PROTECT_NCB	(0x183 | RTDS_MEM_BUF_NONCACHE)

/* triger identifier */
enum {
	RTDS_MEM_TRIGGER_RT = 1,
	RTDS_MEM_TRIGGER_APP,
	RTDS_MEM_TRIGGER_KERNEL,
	RTDS_MEM_TRIGGER_SHARE_APP,
	RTDS_MEM_TRIGGER_SHARE_KERNEL,
};

/* Error check parameters */
#define RTDS_MEM_ADDR_NG		(0x80000000)
#define RTDS_MEM_ADDR_ERR		(0x00000FFF)

/* Error */
#define RTDS_MEM_ERR_PARAM		(-2)
#define RTDS_MEM_ERR_MAPPING	(-4)
#define RTDS_MEM_ERR_NO_LIST	(-200)
#define RTDS_MEM_ERR_PAGE_MAP	(-300)

#define RTDS_MEM_ALLOC_PAGE_NUM		(4)
#define RTDS_MEM_ALLOC_PAGE_RETRY	(5)

#define RTDS_MEM_GET_PAGE_NUM(mem_size)	((mem_size + PAGE_SIZE - 1) / PAGE_SIZE)

#define RTDS_MEM_ALLOC_FROM_HIGHMEM

/* Receive event from RT domain */
enum {
	RTDS_MEM_DRV_EVENT_APMEM_OPEN = 0,
	RTDS_MEM_DRV_EVENT_APMEM_CLOSE,
	RTDS_MEM_DRV_EVENT_APMEM_DELETE,

	RTDS_MEM_DRV_EVENT_APMEM

};

/* Mapping flag */
enum {
	RTDS_MEM_MAPPING_APMEM = 1,
	RTDS_MEM_MAPPING_RTMEM,
	RTDS_MEM_MAPPING_RTDOMAIN
};

/* Mpro event */
enum {
	RTDS_MEM_RT_CREATE_EVENT = 1,
	RTDS_MEM_APP_DELETE_EVENT,
	RTDS_MEM_RT_DELETE_EVENT,
	RTDS_MEM_MAP_EVENT,
	RTDS_MEM_UNMAP_EVENT,
	RTDS_MEM_MAP_PNC_EVENT,
	RTDS_MEM_UNMAP_PNC_EVENT,
	RTDS_MEM_MAP_PNC_NMA_EVENT,

};

/* ******************************* STRUCTURE ******************************** */
typedef struct {
	struct list_head	queue_header;	/* queue head */
	unsigned int		event;			/* Receive event */
	unsigned int		mem_size;		/* Memory size */
	unsigned int		rt_cache;		/* RT domain cache type */
	unsigned int		rt_trigger;		/* RT trigger identifier */
	unsigned int		apmem_id;		/* App shared memory ID */
} rtds_memory_rcv_event_queue;


typedef struct {
	unsigned long  rt_write_back_addr;
	unsigned long  rt_non_cache_addr;
	unsigned long  apmem_id;
} rtds_memory_drv_app_mem_info;

typedef struct {
	struct list_head	queue_header;	/* queue head */
	unsigned int		mem_size;		/* Memory size */
	struct page			*page;			/* page descriptor */
	unsigned int		app_addr;		/* App address */
	unsigned long		app_cache;		/* App cahce type */
	struct page			**pages;		/* page descriptor */
	struct task_struct	*task_info;		/* Task info */
} rtds_memory_create_queue;

typedef struct {
	unsigned long   phy_addr;
	unsigned long   map_size;
	unsigned long   rt_addr;
} rtds_memory_phymem_table;

/* ******************************* PROTOTYPE ******************************** */

int rtds_memory_drv_init_mpro(
	struct file			*fp
);

void rtds_memory_init_data(
	rtds_memory_drv_handle *handle
);


int rtds_memory_ioctl_init_data(
	struct file		*fp,
	char __user		*buffer,
	size_t			buf_size
);

void rtds_memory_check_shared_apmem(
	struct file					*fp,
	rtds_memory_mapping_data	*map_data
);

int rtds_memory_open_shared_rtmem(
	unsigned long	phy_addr,
	int				map_size,
	int				cache,
	rtds_memory_mapping_data	*map_data,
	struct file		*fp,
	unsigned long	*rt_addr
);

int rtds_memory_close_shared_rtmem(
	unsigned long	address,
	int				map_size
);

int rtds_memory_map_shared_memory(
	rtds_memory_mapping_data	*map_data,
	struct vm_area_struct		*vm_area
);


int rtds_memory_open_kernel_shared_apmem(
	rtds_memory_drv_open_mem_param	 *rtds_memory_open_mem
);

int rtds_memory_close_kernel_shared_apmem(
	rtds_memory_drv_close_mem_param	*rtds_memory_close_mem
);

int rtds_memory_ioctl_open_apmem(
	struct file					*fp,
	char __user					*buffer,
	size_t						buf_size,
	rtds_memory_mapping_data	*map_data
);

int rtds_memory_open_shared_apmem(
	struct file					*fp,
	rtds_memory_apmem_info		*mem_info,
	rtds_memory_mapping_data	*map_data
);

int rtds_memory_map_shared_apmem(
	struct file						*fp,
	rtds_memory_mapping_data		*map_data,
	rtds_memory_app_memory_table	*mem_table
);

int rtds_memory_open_rttrig_shared_apmem(
	unsigned int	mem_size,
	unsigned int	rt_cache,
	unsigned int	rt_trigger
);

int rtds_memory_close_rttrig_shared_apmem(
	unsigned int	apmem_id
);

int rtds_memory_send_open_shared_apmem(
	unsigned long   write_back_addr,
	unsigned long   non_cache_addr,
	unsigned long   mem_size,
	unsigned long   rt_trigger,
	unsigned long   rt_err,
	unsigned long   app_trigger,
	unsigned int	*apmem_id
);

int rtds_memory_send_close_shared_apmem(
	unsigned int	apmem_id
);

int rtds_memory_delete_shared_apmem(
	unsigned int	apmem_id
);


void rtds_memory_rcv_comp_notice(
	void			*user_data,
	int				result_code,
	int				function_id,
	unsigned char	*data_addr,
	int				data_len
);

int rtds_memory_put_recv_queue(
	unsigned int	event,
	unsigned int	mem_size,
	unsigned int	rt_cache,
	unsigned int	rt_trigger,
	unsigned int	apmem_id
);

rtds_memory_rcv_event_queue *rtds_memory_get_recv_queue(
	void
);

void rtds_memory_delete_recv_queue(
	rtds_memory_rcv_event_queue	*queue
);

void rtds_memory_put_mpro_list(
	rtds_memory_app_memory_table   *mem_table
);

int rtds_memory_map_mpro(
	unsigned long phy_address,
	unsigned long mem_size,
	unsigned long *vir_address
);

int rtds_memory_unmap_mpro(
	unsigned long vir_address,
	unsigned long mem_size
);

int rtds_memory_map_pnc_mpro(
	unsigned int		app_addr,
	unsigned int		map_size,
	struct page			**pages,
	unsigned int		rt_cache,
	rtds_memory_drv_app_mem_info	*mem_info
);

int rtds_memory_unmap_pnc_mpro(
	unsigned long   address,
	unsigned int	apmem_id
);

int rtds_memory_map_pnc_nma_mpro(
	unsigned int		map_size,
	struct page			**pages,
	unsigned int		*rt_addr_wb
);

int rtds_memory_ioctl_map_mpro(
	char __user		*buffer,
	size_t			cnt
);

int rtds_memory_ioctl_unmap_mpro(
	char __user		*buffer,
	size_t			cnt
);

int rtds_memory_ioctl_map_pnc_mpro(
	char __user		*buffer,
	size_t			cnt
);

int rtds_memory_ioctl_unmap_pnc_mpro(
	char __user		*buffer,
	size_t			cnt
);

int rtds_memory_ioctl_map_pnc_nma_mpro(
	char __user		*buffer,
	size_t			cnt
);

void rtds_memory_drv_close_vma(
	struct vm_area_struct	*vm_area
);

int rtds_memory_create_page_frame(
	unsigned int	page_num,
	struct page		**pages
);

void rtds_memory_do_unmap(
	unsigned long	address,
	unsigned long	size
);

void rtds_memory_flush_mmu(
	struct vm_area_struct	*vm_area,
	unsigned long			address
);

void rtds_memory_free_page_frame(
	unsigned int		page_num,
	struct page			**pages
);

int rtds_memory_do_map(
	struct file		*fp,
	unsigned long	*addr,
	unsigned long	size,
	unsigned long	pgoff
);

int rtds_memory_share_kernel_shared_apmem(
	rtds_memory_drv_share_mem_param	 *rtds_memory_share_mem
);

int rtds_memory_share_shared_apmem(
	struct file						*fp,
	unsigned int					apmem_id,
	rtds_memory_mapping_data		*map_data
);

void rtds_memory_flush_l2cache(
	 unsigned long	  start_addr,
	 unsigned long	  end_addr
);

void rtds_memory_inv_l2cache(
	 unsigned long	  start_addr,
	 unsigned long	  end_addr
);

void rtds_memory_flush_cache_all(
	 void
);

int rtds_memory_put_create_mem_list(
	unsigned int		app_addr,
	unsigned int		mem_size,
	struct page			**page,
	unsigned long		app_cache
);

int rtds_memory_get_create_mem_list(
	unsigned int		app_addr,
	struct page			*page,
	unsigned int		*mem_size
);

int rtds_memory_reg_kernel_phymem(
	unsigned long			phy_addr,
	unsigned long			map_size,
	unsigned long			rt_addr
);

int rtds_memory_unreg_kernel_phymem(
	unsigned long   phy_addr,
	unsigned long   map_size,
	unsigned long   rt_addr
);

int rtds_memory_change_kernel_phymem_address(
	unsigned long  phy_addr,
	unsigned long  *rt_addr
);

int rtds_memory_ioctl_change_phymem_address(
	char __user   *buffer,
	size_t    buf_size
);

int rtds_memory_ioctl_close_apmem(
	char __user			*buffer,
	size_t				buf_size
);

int rtds_memory_ioctl_share_apmem(
	struct file				*fp,
	char __user				*buffer,
	size_t					buf_size,
	rtds_memory_mapping_data	*map_data
);

int rtds_memory_ioctl_cache_flush(
	char __user				*buffer,
	size_t					buf_size
);

int rtds_memory_ioctl_cache_clear(
	char __user				*buffer,
	size_t					buf_size
);

int rtds_memory_ioctl_get_memsize(
	char __user				*buffer,
	size_t					buf_size
);

int rtds_memory_ioctl_get_pagesinfo(
	char __user				*buffer,
	size_t					buf_size
);

void rtds_memory_send_error_msg(
	rtds_memory_app_memory_table	*mem_table,
	int								err_code
);

void rtds_memory_close_apmem(
	unsigned int		app_addr,
	unsigned int		mem_size
);

#endif /* __RTDS_MEMORY_DRV_SUB_H__ */
