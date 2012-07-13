/*
 * iccom_drv_com.c
 *	 Inter Core Communication driver common function file.
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
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/completion.h>
#include "log_kernel.h"
#include "iccom_drv.h"
#include "iccom_drv_common.h"
#include "iccom_drv_private.h"

static iccom_fatal_info g_iccom_fatal;			  /* fatal information */
static struct list_head g_iccom_list_recv;		  /* queue header */
static spinlock_t g_iccom_lock_recv_list;		  /* spinlock for receive */
static spinlock_t g_iccom_lock_fatal;			  /* spinlock for fatal */
extern struct completion g_iccom_async_completion;  /* completion for asynchronous */

/******************************************************************************/
/* Function   : iccom_create_handle											  */
/* Description: create a ICCOM handle										  */
/******************************************************************************/
iccom_drv_handle *iccom_create_handle(
	int				 type
) {
	iccom_drv_handle   *handle;
	struct completion  *completion;
	unsigned int		alloc_size;
	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);

	/* calculate allocation size */
	alloc_size = sizeof(*handle) + sizeof(*completion);
	if (ICCOM_TYPE_USER == type) {
		alloc_size += sizeof(*completion);
	}

	/* allocate memory of ICCOM handle */
	handle = kmalloc(alloc_size, GFP_KERNEL);
	if (NULL != handle) {
		memset(handle, 0, alloc_size);
		completion = (struct completion *)(handle + 1);
		if (ICCOM_TYPE_KERNEL == type) {
			handle->sync_completion = completion;
			handle->async_completion = &g_iccom_async_completion;

			init_completion(handle->sync_completion);
		} else {
			handle->sync_completion = completion;
			handle->async_completion = (completion + 1);

			init_completion(handle->sync_completion);
			init_completion(handle->async_completion);
		}
	} else {
		MSG_ERROR("[ICCOMK]ERR| handle allocate error.\n");
	}
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
	return handle;
}

/******************************************************************************/
/* Function   : iccom_destroy_handle										  */
/* Description: destroy a ICCOM handle										  */
/******************************************************************************/
void iccom_destroy_handle(
	iccom_drv_handle   *handle
)
{
	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);
	memset(handle, 0, sizeof(*handle));
	kfree(handle);
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
}

/******************************************************************************/
/* Function   : iccom_init_recv_queue										  */
/* Description: initialize receive queue									  */
/******************************************************************************/
void iccom_init_recv_queue(
	void
)
{
	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);

	memset(&g_iccom_list_recv, 0, sizeof(g_iccom_list_recv));

	INIT_LIST_HEAD(&g_iccom_list_recv);

	spin_lock_init(&g_iccom_lock_recv_list);
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
}

/******************************************************************************/
/* Function   : iccom_put_recv_queue										  */
/* Description: put data to receive queue									  */
/******************************************************************************/
int iccom_put_recv_queue(
	struct completion	*completion,
	int					eicr_result,
	void				*recv_data,
	unsigned long		recv_size
)
{
	int				 ret;
	iccom_recv_queue   *p_queue_data;
	unsigned long	   flag;

	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);
	ret = SMAP_OK;

	/* allocate memory of receive queue data */
	p_queue_data = kmalloc(sizeof(*p_queue_data), GFP_ATOMIC);
	if (NULL == p_queue_data) {
		ret = SMAP_MEMORY;
	} else {
		/* set receive data information */
		p_queue_data->recv_data	 = recv_data;
		p_queue_data->recv_size	 = recv_size;
		p_queue_data->completion	= completion;
		p_queue_data->eicr_result   = eicr_result;
		spin_lock_irqsave(&g_iccom_lock_recv_list, flag);

		list_add_tail(&p_queue_data->queue_header, &g_iccom_list_recv);

		spin_unlock_irqrestore(&g_iccom_lock_recv_list, flag);
	}

	MSG_MED("[ICCOMK]OUT|[%s] ret = %d\n", __func__, ret);
	return ret;
}

/******************************************************************************/
/* Function   : iccom_get_recv_queue										  */
/* Description: get data from receive queue									  */
/******************************************************************************/
int iccom_get_recv_queue(
	struct completion  *completion,
	iccom_recv_queue  **queue
) {
	int ret;
	iccom_recv_queue   *p_queue_data;
	unsigned long	   flag;

	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);

	ret = SMAP_NG;
	if (NULL != queue) {
		*queue = NULL;
		spin_lock_irqsave(&g_iccom_lock_recv_list, flag);
		/* search the list */
		list_for_each_entry(p_queue_data, &g_iccom_list_recv, queue_header) {
			if ((NULL == completion) ||   /* target is not specified */
			   (completion == p_queue_data->completion)) {	/* match */
				*queue = p_queue_data;
				break;
			}
		}
		spin_unlock_irqrestore(&g_iccom_lock_recv_list, flag);
		if (NULL != *queue) {
			ret = SMAP_OK;
		}
	} else {
		MSG_ERROR("[ICCOMK]ERR| parameter error.\n");
	}
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
	return ret;
}

/******************************************************************************/
/* Function   : iccom_delete_recv_queue										  */
/* Description: delete data from receive queue								  */
/******************************************************************************/
void iccom_delete_recv_queue(
	iccom_recv_queue   *queue
) {
	unsigned long	   flag;

	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);
	spin_lock_irqsave(&g_iccom_lock_recv_list, flag);
	list_del(&queue->queue_header);
	spin_unlock_irqrestore(&g_iccom_lock_recv_list, flag);
	kfree(queue);
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
}

/******************************************************************************/
/* Function   : iccom_init_fatal											  */
/* Description: initialize fatal information								  */
/******************************************************************************/
void iccom_init_fatal(
	void
)
{
	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);
	memset(&g_iccom_fatal, 0, sizeof(g_iccom_fatal));

	spin_lock_init(&g_iccom_lock_fatal);
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
}

/******************************************************************************/
/* Function   : iccom_put_fatal												  */
/* Description: put fatal information										  */
/******************************************************************************/
int iccom_put_fatal(
	iccom_fatal_info   *fatal_info
) {
	int				 ret;
	unsigned long	 flag;

	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);
	ret = SMAP_OK;
	spin_lock_irqsave(&g_iccom_lock_fatal, flag);
	/* mismatch fatal information */
	if (fatal_info->handle != g_iccom_fatal.handle) {
		if ((NULL == fatal_info->handle) ||	/* delete fatal information */
			(NULL == g_iccom_fatal.handle)) {  /* fatal information none */
			memcpy(&g_iccom_fatal,
					fatal_info,
					sizeof(g_iccom_fatal));
		} else {
			MSG_ERROR("[ICCOMK]ERR| already entry.\n");
			ret = SMAP_ALREADY_EXIST;
		}
	}
	spin_unlock_irqrestore(&g_iccom_lock_fatal, flag);
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
	return ret;
}

/******************************************************************************/
/* Function   : iccom_get_fatal												  */
/* Description: get fatal information										  */
/******************************************************************************/
int iccom_get_fatal(
	iccom_fatal_info  **fatal_info
)
{
	int				 ret;
	unsigned long	 flag;

	MSG_MED("[ICCOMK]IN |[%s]\n", __func__);
	ret = SMAP_OK;
	if (NULL != fatal_info) {
		spin_lock_irqsave(&g_iccom_lock_fatal, flag);
		if (NULL != g_iccom_fatal.handle) {   /* fatal information is set */
			*fatal_info = &g_iccom_fatal;
			ret = SMAP_OK;
		} else {
			MSG_ERROR("[ICCOMK]ERR| no entry.\n");
			ret = SMAP_NG;
		}
		spin_unlock_irqrestore(&g_iccom_lock_fatal, flag);
	} else {
		MSG_ERROR("[ICCOMK]ERR| parameter error.\n");
		ret = SMAP_NG;
	}
	MSG_MED("[ICCOMK]OUT|[%s]\n", __func__);
	return ret;
}
