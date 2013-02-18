/*
 * drivers/sec_hal/sec_hal_icram0.c
 *
 * Copyright (c) 2013, Renesas Mobile Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/errno.h>
#include <asm/io.h>


#define FALSE 0
#define TRUE  1
#define BLOCKCOUNT 128
#define ICRAM1_SIZE 4096
#define ICRAM1_ADDRESS 0xE63A0000
#define MEM_MSG_AREA_CLEAR(ptr) mem_msg_area_clear(ptr)


struct mem_alloc_info {
	void* virt_addr; /* the start address of the message area */
	uint8_t size;/* the size of allocation */
	uint8_t allocated; /* status of the block */
};

struct mem_msg_area {
	void* virt_baseptr; /* stores ioremap output */
	unsigned long phys_start; /* phys start addr */
	unsigned long phys_size; /* phys size */
	unsigned long offset; /* offset between physical and virtual addresses */
	/* blocks to-be-allocated for out & in msgs */
	struct mem_alloc_info msg_blocks[BLOCKCOUNT];
};


static struct mem_msg_area g_msg_area_smc0;
static DEFINE_SPINLOCK(g_msg_area_spinlock_smc0);


static inline
void mem_msg_area_clear(struct mem_msg_area *ptr)
{
	int i = 0;
	if (ptr) {
		ptr->virt_baseptr = NULL;
		ptr->phys_start = 0;
		ptr->phys_size = 0;
		for (; i < BLOCKCOUNT; i++) {
			ptr->msg_blocks[i].virt_addr = NULL;
			ptr->msg_blocks[i].size = 0;
			ptr->msg_blocks[i].allocated = FALSE;
		}
	}
}

unsigned long sec_hal_mem_msg_area_memcpy(void *dst, const void *src,
		unsigned long sz)
{
	__u8* dst8 = (__u8*)dst;
	__u8* src8 = (__u8*)src;

	while (sz--) {
		*dst8++ = *src8++;
	}

	return (unsigned long)dst;
}

unsigned long sec_hal_mem_msg_area_write(void *dst, const void *src,
		unsigned long sz)
{
	__u8* dst8 = (__u8*)dst;
	__u8* src8 = (__u8*)src;

	while (sz--) {
		*dst8++ = *src8++;
	}

	return (unsigned long)dst;
}

unsigned long sec_hal_mem_msg_area_read(void *dst, const void *src,
		unsigned long sz)
{
	__u8* dst8 = (__u8*)dst;
	__u8* src8 = (__u8*)src;

	while (sz--) {
		*dst8++ =  *src8++;
	}

	return (unsigned long)dst;
}

static inline
void sec_hal_mem_msg_area_memset(void *buff, unsigned char data,
		unsigned int cnt)
{
	__u8* ptr = (__u8*)buff;

	while (cnt > 0) {
		*ptr++ = data;
		cnt--;
	}
}

#if (!defined(BLOCKCOUNT) && !BLOCKCOUNT)
#error !!local macro not defined, can cause div by zero exception!!
#endif
/* **********************************************************************
** Function name      : sec_hal_mem_msg_area_calloc
** Description        :
** Return             : virtual address if success, NULL otherwise.
** *********************************************************************/
void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz)
{
	unsigned int block_sz, block_cnt, block_ind, index = 0;
	int found = FALSE;
	void* vaddr = NULL;

	if (0 == (n*sz))
		return NULL;

	block_sz = (g_msg_area_smc0.phys_size)/BLOCKCOUNT;
	block_cnt = ((n*sz)+block_sz-1)/block_sz;

	if (block_cnt > BLOCKCOUNT)
		return NULL;

	spin_lock(&g_msg_area_spinlock_smc0);
	/* critical section starting, do not 'call' anything that may sleep.*/

	/* seek big enough unallocated mem area */
	while (index < BLOCKCOUNT) {
		if (FALSE == g_msg_area_smc0.msg_blocks[index].allocated) {
			found = TRUE;
			block_ind = block_cnt - 1;

			while (block_ind > 0 && (index+block_ind) < BLOCKCOUNT) {
				found = (found && FALSE == g_msg_area_smc0.msg_blocks[index+block_ind].allocated);
				block_ind--;
			}
			if (TRUE == found)
				break; /* check if the loop can be ended */
		}
		index++;
	}

	/* return ptr to first block, update allocation info & zero initialize */
	if (TRUE == found && index < BLOCKCOUNT) {
		/* allocated found message area */
		vaddr = g_msg_area_smc0.msg_blocks[index].virt_addr;
		g_msg_area_smc0.msg_blocks[index].size = block_cnt;
		g_msg_area_smc0.msg_blocks[index].allocated = TRUE;
		sec_hal_mem_msg_area_memset(vaddr, 0, block_sz);
		block_cnt--;
	}

	/* also update allocation info for rest of the blocks & zero initialize */
	while (TRUE == found && block_cnt > 0 && (index+block_cnt) < BLOCKCOUNT) {
		g_msg_area_smc0.msg_blocks[index+block_cnt].allocated = TRUE;
		sec_hal_mem_msg_area_memset(
				g_msg_area_smc0.msg_blocks[index+block_cnt].virt_addr,
				0, block_sz);
		block_cnt--;
	}

	/* critical section ending. */
	spin_unlock(&g_msg_area_spinlock_smc0);

	return vaddr; /* return allocated(or not) memory address */
}

/* **********************************************************************
** Function name      : sec_hal_mem_msg_area_free
** Description        :
** Return             :
** *********************************************************************/
void sec_hal_mem_msg_area_free(void *vaddr)
{
	unsigned int block_ind, index = 0;

	if (NULL == vaddr)
		return;

	spin_lock(&g_msg_area_spinlock_smc0);
	/* critical section starting, do not 'call' anything that may sleep.*/

	while (index < BLOCKCOUNT) {
		if (g_msg_area_smc0.msg_blocks[index].virt_addr == vaddr) {
			block_ind = g_msg_area_smc0.msg_blocks[index].size - 1;

			/* free allocated message area */
			g_msg_area_smc0.msg_blocks[index].size = 0;
			g_msg_area_smc0.msg_blocks[index].allocated = FALSE;

			/* free rest of the blocks */
			while (0 < block_ind && (index+block_ind) < BLOCKCOUNT) {
				g_msg_area_smc0.msg_blocks[index+block_ind].size = 0;
				g_msg_area_smc0.msg_blocks[index+block_ind].allocated = FALSE;
				block_ind--;
			}
			break; /* terminate the seek-loop */
		}
		index++;/* seek next mem block */
	}

	/* critical section ending. */
	spin_unlock(&g_msg_area_spinlock_smc0);
}

/* **********************************************************************
** Function name      : sec_hal_virt_to_icram_phys
** Description        :
** Return             :
** *********************************************************************/
unsigned long sec_hal_virt_to_icram_phys(unsigned long vaddr)
{
	unsigned long paddr;
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
	paddr = virt_to_phys((void *)vaddr);
#else
	paddr = vaddr - g_msg_area_smc0.offset;
#endif
	return paddr;
}

/* **********************************************************************
** Function name      : sec_hal_icram_phys_to_virt
** Description        :
** Return             :
** *********************************************************************/
unsigned long sec_hal_icram_phys_to_virt(unsigned long paddr)
{
	unsigned long vaddr;
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
	vaddr = phys_to_virt((phys_addr_t) paddr);
#else
	vaddr = paddr + g_msg_area_smc0.offset;
#endif
	return vaddr;
}


/* **********************************************************************
** Function name      : sec_hal_icram0_init
** Description        :
** Return             :
** *********************************************************************/
int sec_hal_icram0_init(void)
{
	int ret = 0;
	unsigned int block_sz, index = 0;

#if 0
    if (!request_mem_region(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE), "msg area")) {
        ret = -ENODEV;
        goto e1;
    }
#endif

	if (g_msg_area_smc0.virt_baseptr != NULL) {
		ret = -EINVAL;
		goto e1;
	}

#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
	g_msg_area_smc0.virt_baseptr = kmalloc(UL(ICRAM1_SIZE), GFP_KERNEL);
#else
	g_msg_area_smc0.virt_baseptr = ioremap_nocache(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE));
#endif /* CONFIG_ARM_SEC_HAL_TEST_DISPATCHER */
	g_msg_area_smc0.phys_start = UL(ICRAM1_ADDRESS);
	g_msg_area_smc0.phys_size = UL(ICRAM1_SIZE);
    g_msg_area_smc0.offset = (unsigned long)(g_msg_area_smc0.virt_baseptr - g_msg_area_smc0.phys_start);

	if (g_msg_area_smc0.virt_baseptr == NULL) {
		ret = -EINVAL;
		goto e2;
	}

	block_sz = (g_msg_area_smc0.phys_size)/BLOCKCOUNT;
	/* initialize msg area alloc blocks */
	for (; index < BLOCKCOUNT; index++) {
		g_msg_area_smc0.msg_blocks[index].virt_addr = ((__u8*)g_msg_area_smc0.virt_baseptr + block_sz*index);
		g_msg_area_smc0.msg_blocks[index].size = 0;
		g_msg_area_smc0.msg_blocks[index].allocated = FALSE;
	}

	return ret;

e2:	/*release_mem_region(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE));*/
e1:	MEM_MSG_AREA_CLEAR(&g_msg_area_smc0); /* leave mem struct as 'unallocated' */
	return ret;
}


