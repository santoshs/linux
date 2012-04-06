/*
 * Function        : share memory management for SH Mobile
 *
 * Copyright (C) 2011-2012 Renesas Electronics Corporation
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
 * Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/proc_fs.h>

#include <media/sh_mobile_appmem.h>
#include <rtapi/system_memory.h>

#define DEV_NAME      "composer"


/******************************************************/
/* define local variables for App Memory handling     */
/******************************************************/
static int         app_share_initialize;
static spinlock_t  app_share_lock;
static LIST_HEAD(app_share_top);
static int         debug;

#define printk_dbg(level, fmt, arg...) \
	do { \
		if (level <= debug) \
			printk(KERN_INFO DEV_NAME ": %s: " fmt, \
				__func__, ## arg); \
	} while (0)

#define printk_err(fmt, arg...) \
	do { \
		printk(KERN_ERR DEV_NAME ":E %s: " fmt, __func__, ## arg); \
	} while (0)

/**************************************************/
/* implementation for app memory handling         */
/**************************************************/

/**************************************************/
/* name : sh_mobile_appmem__init                  */
/* function: initialize local variable            */
/**************************************************/
static void sh_mobile_appmem_dump_appshare_list(void)
{
	unsigned long flags;
	struct list_head *list;

	printk_dbg(1, "list currently registered appshare.\n");
	spin_lock_irqsave(&app_share_lock, flags);

	list_for_each(list, &app_share_top)
	{
		struct appmem_handle *mem = NULL;

		mem = list_entry((void *)list, struct appmem_handle, list);

		printk_dbg(1, "  memhandle:%p app_id:0x%x offset:0x%x key:%s"
			"size:0x%x vaddr:%p rtaddr:0x%lx ref_count:%d\n",
			mem->memhandle, mem->app_id, mem->offset, mem->key,
			mem->size, mem->vaddr, mem->rtaddr, mem->ref_count);
	}

	spin_unlock_irqrestore(&app_share_lock, flags);
}

/**************************************************/
/* name : sh_mobile_appmem__init                  */
/* function: initialize local variable            */
/**************************************************/
static void sh_mobile_appmem__init(void)
{
	if (app_share_initialize == 0) {
		app_share_initialize = 1;
		spin_lock_init(&app_share_lock);
	}
}

/***************************************************/
/* name : sh_mobile_appmem_alloc                   */
/* function: allocate app share memory in kernel   */
/* return: handle of app share memory information. */
/*         NULL if found error.                    */
/***************************************************/
struct appmem_handle *sh_mobile_appmem_alloc(int size, char *key)
{
	struct appmem_handle *mem = NULL;
	void *handle = NULL;
	void *memhandle = NULL;
	unsigned char *vadr = NULL;
	unsigned int offset, rt_adr;
	int  rc;

	sh_mobile_appmem__init();

	printk_dbg(1, "size:0x%x key:%s\n", size, key);

	handle = system_memory_info_new();
	if (handle == NULL) {
		printk_err("system_memory_info_new error\n");
		goto err_exit;
	}

	{
		system_mem_ap_open  op;
		op.handle      = handle;
		op.aparea_size = RT_MEMORY_APAREA_SIZE(size);
		op.cache_kind  = RT_MEMORY_NONCACHE;
		rc = system_memory_ap_open(&op);
		if (rc != 0) {
			printk_err("system_memory_ap_open " \
				"error: return by %d\n", rc);
			goto err_exit;
		}
		memhandle = op.apmem_handle;
	}

	{
		system_mem_ap_alloc  aloc;
		aloc.handle       = handle;
		aloc.alloc_size   = size;
		aloc.apmem_handle = memhandle;
		rc = system_memory_ap_alloc(&aloc);
		if (rc != 0) {
			printk_err("system_memory_ap_open " \
				"error: return by %d\n", rc);
			goto err_exit;
		}
		vadr = (unsigned char *)aloc.apmem_apaddr;
	}

	{
		system_mem_ap_share_mem_offset apmem_ofs;

		apmem_ofs.handle       = handle;
		apmem_ofs.apmem_handle = memhandle;
		apmem_ofs.apmem_apaddr = (unsigned int)vadr;
		rc = system_memory_ap_share_mem_offset(&apmem_ofs);
		if (rc != 0) {
			printk_err("system_memory_ap_share_mem_offset " \
			"error: return by %d\n", rc);
			goto err_exit;
		}
		offset = apmem_ofs.apmem_offset;
	}

	{
		system_mem_ap_change_rtaddr adr;

		adr.handle       = handle;
		adr.cache_kind   = RT_MEMORY_NONCACHE;
		adr.apmem_handle = memhandle;
		adr.apmem_apaddr = (unsigned int)vadr;
		rc = system_memory_ap_change_rtaddr(&adr);
		if (rc != 0) {
			printk_err("system_memory_ap_change_rtaddr " \
			"error: return by %d\n", rc);
			goto err_exit;
		}
		rt_adr = adr.apmem_rtaddr;
	}

	mem = kmalloc(sizeof(*mem), GFP_KERNEL);

	if (mem == NULL) {
		printk_err("kmalloc error\n");
		goto err_exit;
	} else {
		system_mem_ap_get_apmem_id  apmem_id;
		unsigned long flags;

		apmem_id.handle       = handle;
		apmem_id.apmem_handle = memhandle;

		mem->memhandle = memhandle;
		mem->app_id    = system_memory_ap_get_apmem_id(&apmem_id);
		mem->offset    = offset;
		mem->key       = key;

		/* following information is reserved */
		INIT_LIST_HEAD(&mem->list);
		mem->size      = size;
		mem->vaddr     = vadr   - offset;
		mem->rtaddr    = rt_adr - offset;
		mem->ref_count = 1;

		printk_dbg(2, "open appshare memory.\n");
		printk_dbg(2, "  memhandle:%p app_id:0x%x offset:0x%x key:%s"
			"size:0x%x vaddr:%p rtaddr:0x%lx ref_count:%d\n",
			mem->memhandle, mem->app_id, mem->offset, mem->key,
			mem->size, mem->vaddr, mem->rtaddr, mem->ref_count);

		spin_lock_irqsave(&app_share_lock, flags);
		list_add_tail(&mem->list, &app_share_top);
		spin_unlock_irqrestore(&app_share_lock, flags);
	}

err_exit:
	if (mem == NULL) {
		/* free resources */
		if (vadr) {
			system_mem_ap_free fre;

			fre.handle       = handle;
			fre.apmem_handle = memhandle;
			fre.apmem_apaddr = (unsigned int)vadr;
			rc = system_memory_ap_free(&fre);
			if (rc != 0) {
				printk_err("system_memory_ap_free " \
					"error: return by %d\n", rc);
			}
			vadr = NULL;
		}
		if (memhandle) {
			system_mem_ap_close clo;

			clo.handle       = handle;
			clo.apmem_handle = memhandle;
			rc = system_memory_ap_close(&clo);
			if (rc != 0) {
				printk_err("system_memory_ap_close " \
					"error: return by %d\n", rc);
			}
			memhandle = NULL;
		}
	}
	if (handle) {
		system_mem_info_delete del;
		del.handle = handle;

		system_memory_info_delete(&del);
	}
	if (debug) {
		/* display debug information. */
		sh_mobile_appmem_dump_appshare_list();
	}

	return mem;
}
EXPORT_SYMBOL(sh_mobile_appmem_alloc);


/***************************************************/
/* name : sh_mobile_appmem_share                   */
/* function: mapping app share memory in kernel    */
/* return: handle of app share memory information. */
/*         NULL if found error.                    */
/***************************************************/
struct appmem_handle *sh_mobile_appmem_share(int appid, char *key)
{
	struct appmem_handle *mem = NULL;
	void *handle        = NULL;
	void *memhandle     = NULL;
	unsigned char *vadr = NULL;
	unsigned int rt_adr;
	int  rc;

	sh_mobile_appmem__init();

	printk_dbg(1, "appid:%d key:%s\n", appid, key);
	/* search already created. */

	{
		unsigned long flags;
		struct list_head *list;

		spin_lock_irqsave(&app_share_lock, flags);

		list_for_each(list, &app_share_top)
		{
			struct appmem_handle *tmp = NULL;

			tmp = list_entry((void *)list,
				struct appmem_handle, list);
			if (appid == tmp->app_id) {
				/* already handle created. */

				tmp->ref_count = tmp->ref_count + 1;

				mem = tmp;
				break;
			}
		}
		spin_unlock_irqrestore(&app_share_lock, flags);

		if (mem != NULL) {
			/* app share already opend */
			printk_dbg(2, "app_id:0x%x already opend. mem:%p\n",
				appid, mem);
			goto normal_exit;
		}
	}

	handle = system_memory_info_new();
	if (handle == NULL) {
		printk_err("system_memory_info_new error\n");
		goto err_exit;
	}

	{
		system_mem_ap_share_area area;

		area.handle = handle;
		area.apmem_id = appid;
		rc = system_memory_ap_share_area(&area);
		if (rc != 0) {
			printk_err("system_memory_ap_share_area " \
				"error: return by %d\n", rc);
			goto err_exit;
		}
		memhandle = area.apmem_handle;
	}

	{
		system_mem_ap_share_mem  mem;

		mem.handle = handle;
		mem.apmem_handle = memhandle;
		mem.apmem_offset = 0;
		rc = system_memory_ap_share_mem(&mem);
		if (rc != 0) {
			printk_err("system_memory_ap_share_mem " \
				"error: return by %d\n", rc);
			goto err_exit;
		}
		vadr = (unsigned char *)mem.apmem_apaddr;
	}

	{
		system_mem_ap_change_rtaddr adr;

		adr.handle = handle;
		adr.cache_kind = RT_MEMORY_NONCACHE;
		adr.apmem_handle = memhandle;
		adr.apmem_apaddr = (unsigned int)vadr;
		rc = system_memory_ap_change_rtaddr(&adr);
		if (rc != 0) {
			printk_err("system_memory_ap_share_mem_offset " \
				"error: return by %d\n", rc);
			goto err_exit;
		}
		rt_adr = adr.apmem_rtaddr;
	}


	mem = kmalloc(sizeof(*mem), GFP_KERNEL);

	if (mem == NULL) {
		printk_err("kmalloc error\n");
		goto err_exit;
	} else {
		unsigned long flags;

		mem->memhandle = memhandle;
		mem->app_id    = appid;
		mem->offset    = 0;
		mem->key       = key;

		/* following information is reserved */
		INIT_LIST_HEAD(&mem->list);
		mem->size      = 0;
		mem->vaddr     = vadr;
		mem->rtaddr    = rt_adr;
		mem->ref_count = 1;

		printk_dbg(2, "share appshare memory.\n");
		printk_dbg(2, "  memhandle:%p app_id:0x%x offset:0x%x key:%s"
			"size:0x%x vaddr:%p rtaddr:0x%lx ref_count:%d\n",
			mem->memhandle, mem->app_id, mem->offset, mem->key,
			mem->size, mem->vaddr, mem->rtaddr, mem->ref_count);

		spin_lock_irqsave(&app_share_lock, flags);
		list_add_tail(&mem->list, &app_share_top);
		spin_unlock_irqrestore(&app_share_lock, flags);
	}

err_exit:
	if (mem == NULL) {
		/* free resources */
		if (memhandle) {
			system_mem_ap_close clo;

			clo.handle = handle;
			clo.apmem_handle = memhandle;
			rc = system_memory_ap_close(&clo);
			if (rc != 0) {
				printk_err("system_memory_ap_close " \
					"error: return by %d\n", rc);
			}
			memhandle = NULL;
		}
	}
	if (handle) {
		system_mem_info_delete     del;
		del.handle = handle;

		system_memory_info_delete(&del);
	}
normal_exit:
	if (debug) {
		/* display debug information. */
		sh_mobile_appmem_dump_appshare_list();
	}
	return mem;
}
EXPORT_SYMBOL(sh_mobile_appmem_share);


/***************************************************/
/* name : sh_mobile_appmem_getRTaddress            */
/* function: get RT address of offset.             */
/* return: virtual address to access app share.    */
/*         NULL if found error.                    */
/***************************************************/
unsigned long sh_mobile_appmem_getRTaddress(\
	struct appmem_handle *appmem, unsigned char *vadr)
{
	int rc = -1;
	system_mem_ap_change_rtaddr rtaddr;

	printk_dbg(1, "appmem:%p vadr:%p\n", appmem, vadr);

	if (appmem == NULL) {
		printk_err("appmem handle passed NULL.\n");
		goto err_exit;
	}

	if (vadr == NULL) {
		printk_err("appmem handle passed NULL.\n");
		goto err_exit;
	}

	if (appmem->size) {
		int offset = vadr - appmem->vaddr;
		int min = appmem->offset;
		int max = appmem->offset + appmem->size;
		/* appmem handle created by sh_mobile_appmem_alloc */
		if (offset < min || offset >= max) {
			printk_err("vadr %p out of valid range" \
				"(%p - %p)\n", vadr, \
				appmem->vaddr+min, appmem->vaddr+max);
			goto err_exit;
		}
		rtaddr.apmem_rtaddr = appmem->rtaddr + offset;
		rc = 0;
	} else {
		void *handle;
		system_mem_info_delete   del;

		handle = system_memory_info_new();
		if (handle == NULL) {
			printk_err("system_memory_info_new error\n");
			goto err_exit;
		}

		/* set handle */
		rtaddr.handle = handle;
		del.handle    = handle;

		/* set ap_change_rtaddr parameter */
		rtaddr.cache_kind   = RT_MEMORY_NONCACHE;
		rtaddr.apmem_handle = appmem->memhandle;
		rtaddr.apmem_apaddr = (unsigned long)vadr;
		rc = system_memory_ap_change_rtaddr(&rtaddr);
		if (rc != 0) {
			printk_err("system_memory_ap_change_rtaddr " \
				"error: return by %d\n", rc);
		}
		system_memory_info_delete(&del);
	}
err_exit:
	if (rc == 0)
		return rtaddr.apmem_rtaddr;
	else
		return 0;
}
EXPORT_SYMBOL(sh_mobile_appmem_getRTaddress);


/***************************************************/
/* name : sh_mobile_appmem_getaddress              */
/* function: get virtual address of offset.        */
/* return: virtual address to access app share.    */
/*         NULL if found error.                    */
/***************************************************/
unsigned char *sh_mobile_appmem_getaddress(\
	struct appmem_handle *appmem, int offset)
{
	int rc;
	unsigned char *vadr = NULL;

	printk_dbg(1, "appmem:%p offset:0x%x\n", appmem, offset);

	if (appmem == NULL) {
		printk_err("appmem handle passed NULL.\n");
		goto err_exit;
	}

	if (appmem->size) {
		int min = appmem->offset;
		int max = appmem->offset + appmem->size;
		/* appmem handle created by sh_mobile_appmem_alloc */
		if (offset < min || offset >= max) {
			printk_err("offset 0x%x out of valid range" \
				"(0x%x - 0x%x)\n", offset, min, max);
			goto err_exit;
		}
		vadr = appmem->vaddr + offset;
	} else {
		void *handle;
		system_mem_ap_share_mem  mem;
		system_mem_info_delete   del;

		handle = system_memory_info_new();
		if (handle == NULL) {
			printk_err("system_memory_info_new error\n");
			goto err_exit;
		}

		/* set handle */
		mem.handle = handle;
		del.handle = handle;

		/* set share_mem parameter */
		mem.apmem_handle = appmem->memhandle;
		mem.apmem_offset = offset;
		rc = system_memory_ap_share_mem(&mem);
		if (rc != 0) {
			printk_err("system_memory_ap_share_mem " \
				"error: return by %d\n", rc);
		} else {
			/* update vadr */
			vadr = (unsigned char *)mem.apmem_apaddr;
		}

		system_memory_info_delete(&del);
	}
err_exit:
	return vadr;
}
EXPORT_SYMBOL(sh_mobile_appmem_getaddress);


/***************************************************/
/* name : sh_mobile_appmem_free                    */
/* function: release app share handle.             */
/* return: 0  if success.                          */
/*         -EINVAL if found error.                 */
/***************************************************/
int sh_mobile_appmem_free(struct appmem_handle *appmem)
{
	/* remove list */
	unsigned long flags;
	int      rc = -EINVAL;
	void     *handle = NULL;

	printk_dbg(1, "appmem:%p\n", appmem);

	if (appmem == NULL) {
		printk_err("appmem handle passed NULL.\n");
		goto err_exit;
	}

	spin_lock_irqsave(&app_share_lock, flags);
	if (appmem->ref_count > 0) {
		/* decrement reference count */
		appmem->ref_count--;
	}
	if (appmem->ref_count != 0) {
		/* set return code */
		rc = 0;
	}
	spin_unlock_irqrestore(&app_share_lock, flags);

	printk_dbg(2, "appmem:%p ref_count:%d\n", appmem, appmem->ref_count);
	if (rc == 0) {
		/* reference count not equal 0 */
		printk_dbg(2, "not free appmem:%p (appid:%d).\n",
			appmem, appmem->app_id);
		goto err_exit;
	}

	list_del_init(&appmem->list);

	handle = system_memory_info_new();
	if (handle == NULL) {
		printk_err("system_memory_info_new error\n");
		goto err_exit;
	}

	if (appmem->size) {
		/* need free */
		system_mem_ap_free fre;

		fre.handle       = handle;
		fre.apmem_handle = appmem->memhandle;
		fre.apmem_apaddr = (unsigned int)appmem->vaddr + appmem->offset;
		rc = system_memory_ap_free(&fre);
		if (rc != 0) {
			printk_err("system_memory_ap_free " \
				"error: return by %d\n", rc);
			rc = -EINVAL;
			/* fall through, assume memory free */
		}
		appmem->vaddr = NULL;
		appmem->size  = 0;
	}
	{
		system_mem_ap_close clo;

		clo.handle       = handle;
		clo.apmem_handle = appmem->memhandle;
		rc = system_memory_ap_close(&clo);
		if (rc != 0) {
			printk_err("system_memory_ap_close " \
				"error: return by %d\n", rc);
			rc = -EINVAL;
			/* fall through, assume close success */
		}
		appmem->memhandle = NULL;
	}

	printk_dbg(2, "free appshare memory.\n");
	printk_dbg(2, "  app_id:0x%x key:%s\n", appmem->app_id, appmem->key);
	kfree(appmem);
err_exit:
	if (handle) {
		system_mem_info_delete   del;
		del.handle = handle;
		system_memory_info_delete(&del);
	}
	if (debug) {
		/* display debug information. */
		sh_mobile_appmem_dump_appshare_list();
	}
	return rc;
}
EXPORT_SYMBOL(sh_mobile_appmem_free);


/***************************************************/
/* name : sh_mobile_appmem_debugmode               */
/* function: set debug mode flag of app share.     */
/* return: -                                       */
/***************************************************/
void sh_mobile_appmem_debugmode(int mode)
{
	/* not implemented. */
	debug = mode;
}
EXPORT_SYMBOL(sh_mobile_appmem_debugmode);
