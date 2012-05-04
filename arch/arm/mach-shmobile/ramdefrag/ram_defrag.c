/*
 * arch/arm/mach-shmobile/ramdefrag/ram_defrag.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

/* Main processing of RAM defragmentation (Core part) */
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/memory.h>
#include <linux/compaction.h>
#include <linux/page-flags.h>

#include "ram_defrag_internal.h"

/*#define DEBUG_RAMDEFRAG */

#ifdef DEBUG_RAMDEFRAG
	#define DEFRAG_PRINTK(fmt, arg...)  printk(fmt,##arg)
#else
	#define DEFRAG_PRINTK(fmt, arg...)
#endif
const unsigned int max_banks = 0x0C;
const unsigned int max_page_in_bank = 0x4000;

/*
 * get_ram_banks_status: Get status of RAM banks
 * return: A bit mask which corresponds to status of RAM banks
 *		- If certain bank is used, correlative bit status in
 *		 return value will be set to 1
 *		- If certain bank is freed, correlative bit status in
 *		 return value will be cleared to 0
 *		- Bit 8 to bit 31 are "Don't Care" which
 *		correspond to non-existent banks, will be set to 1
 *		- Bit status is least significant
 *		(bit 0 corresponds to bank 0, bit 1 corresponds to bank 1, etc.)
 */
unsigned int get_ram_banks_status(void)
{
	unsigned int i;
	unsigned int j;
	unsigned int begin;
	unsigned int end;
	unsigned int status;
	i = 0;
	j = 0;
	begin = 0;
	end = 0;
	status = 0xFFFFFFFF;
	DEFRAG_PRINTK("%s\n", __func__);
	for (i = 0; i < max_banks; i++) {
		begin = i * max_page_in_bank;
		end = ((i + 1) * max_page_in_bank) - 1;
		for (j = begin; j <= end; j++) {
			if (page_check(mem_map + j) == 1)
				break;
		}
		if (j > end)
			status &= ~(1 << i);
	}
	/* Bank 0 to Bank 3 are used as default */
	status = (status << 4) | 0x0F;
	return status;
}
EXPORT_SYMBOL_GPL(get_ram_banks_status);

/*
 * page_check: Check whether page is "use" or "free"
 * @page: Page needs to check status
 * return:
 *	0: Page is "freed"
 *	1: Page is "used"
 *	EINVAL: invalid argument
 */
static int page_check(struct page *page)
{
	int mapcount;
	int pagecount;
	mapcount = 0;
	pagecount = 0;
	DEFRAG_PRINTK("%s\n", __func__);
	if (page != NULL) {
		mapcount = atomic_read(&page->_mapcount);
		pagecount = atomic_read(&page->_count);
		if ((mapcount >= 0) || (page->mapping != NULL)
			|| (pagecount > 0)
			|| (page->flags & PAGE_FLAGS_CHECK_AT_FREE)) {
			return 1;	/* Page is "used" */
		}
		return 0;		/* Page is "freed" */
	} else {
		return -EINVAL;
	}
}

#ifdef CONFIG_COMPACTION

/*
 * defrag: Execute RAM defragmentation
 * return:
 *		0: Normal operation.
 */
int defrag()
{
	int ret;
	ret = 0;
	DEFRAG_PRINTK("%s\n", __func__);
	/* Do defragmentation on all nodes */
	ret = compact_nodes();
	if (ret == 3) { /* If compaction is finished */
		ret = 0;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(defrag);

#endif /*CONFIG_COMPACTION*/


