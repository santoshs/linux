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
** Copyright (C) 2012 Renesas Electronics Corp.                              **
** All rights reserved.                                                      **
** ************************************************************************* */
#include "sec_dispatch.h"
#include "sec_hal_rt_trace.h"
#include "sec_hal_rt_cmn.h"

/*#include <stdarg.h>*/
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
#include <linux/uaccess.h>
#include <asm/cacheflush.h>


#define SEC_ROM_ICACHE_ENABLE_MASK              0x00000001
#define SEC_ROM_DCACHE_ENABLE_MASK              0x00000002
#define SEC_ROM_IRQ_ENABLE_MASK                 0x00000004
#define SEC_ROM_FIQ_ENABLE_MASK                 0x00000008
#define SEC_ROM_UL2_CACHE_ENABLE_MASK           0x00000010

#define MODE_USER                               0x10
#define MODE_FIQ                                0x11
#define MODE_IRQ                                0x12
#define MODE_SVC                                0x13
#define MODE_ABORT                              0x17
#define MODE_UNDEF                              0x1B
#define MODE_SYSTEM                             0x1F
#define MODE_MONITOR                            0x16
#define MODE_CLR_MASK                           0x1F
#define ARM_THUMB_MODE_MASK                     0x20
#define FIQ_IRQ_MASK                            0xC0
#define FIQ_MASK                                0x40
#define IRQ_MASK                                0x80

struct mem_alloc_info
{
	void* virt_addr;   /* the start address of the message area */
	uint8_t size;      /* the size of allocation */
	uint8_t allocated; /* status of the block */
};
#define BLOCKCOUNT 128
struct mem_msg_area
{
	void* virt_baseptr;        /* stores ioremap output */
	unsigned long phys_start;  /* phys start addr */
	unsigned long phys_size;   /* phys size */
	unsigned long offset;      /* offset between physical and virtual addresses */
	uint32_t * valist_ptr;  /* pointer for valist data */
	uint32_t * valist_ptr_ptr;  /* pointer pointer */
	uint32_t valist_size; /* size for valist data */
	/* blocks to-be-allocated for out & in msgs */
	struct mem_alloc_info msg_blocks[BLOCKCOUNT];
};

/* ****************************************************************************
 * STATIC WRITABLE data of the device, the only 'global' writable data.
 * ***************************************************************************/
struct cli_dev_data
{
	struct class* class;
	struct cdev cdev;
	struct semaphore sem;
#ifdef CONFIG_ARM_SEC_HAL_RPC_HANDLER
	rpc_queue_t rpc_read_waitq;
	rpc_queue_t rpc_write_waitq;
#endif /* CONFIG_ARM_SEC_HAL_RPC_HANDLER */
	unsigned int wdt_upd; /* initial wdt value, received in rt_init */
	struct mem_msg_area icram0; /* memory (icram0) allocation information */
};

extern struct cli_dev_data g_cli_dev_data;

uint32_t hw_sec_rom_pub_bridge(uint32_t appl_id, uint32_t flags, va_list *);

/* ****************************************************************************
** Function name      : sec_dispatcher
** Description        :
** Parameters         :
** Return value       : uint32
**                      ==0 operation successful
**                      failure otherwise.
** ***************************************************************************/
uint32_t sec_dispatcher(uint32_t appl_id, uint32_t flags, uint32_t spare_param, uint32_t out_msg_addr, uint32_t in_msg_addr )
{
    va_list ap;
    uint32_t return_value;
    uint32_t pub_cpsr;
    unsigned long  phys_ptr;
    uint32_t * tmp;
    SEC_HAL_TRACE_ENTRY

    /* Instead of valist use pointer in ICRAM */
    tmp = g_cli_dev_data.icram0.valist_ptr;
/*    *tmp = flags;
    tmp++;*/
    *tmp = spare_param;
    tmp++;
    *tmp = out_msg_addr;
    tmp++;
    *tmp = in_msg_addr;

    // Read current CPSR
    __asm__ __volatile__("MRS %0, CPSR" : "=r"(pub_cpsr));

    // FIQ won't be enabled in secure mode if FIQ is currently disabled
    if (pub_cpsr & FIQ_MASK) {
        flags &= ~SEC_ROM_FIQ_ENABLE_MASK;
    }

    // IRQ won't be enabled in secure mode if IRQ is currently disabled
    if (pub_cpsr & IRQ_MASK) {
        flags &= ~SEC_ROM_IRQ_ENABLE_MASK;
    }

    *g_cli_dev_data.icram0.valist_ptr_ptr = sec_hal_virt_to_icram_phys(g_cli_dev_data.icram0.valist_ptr);

#if 0
    printk("flags 0x%x\n",flags);
    printk("&flags 0x%x\n",&flags);
    printk("g_cli_dev_data.icram0.valist_ptr 0x%x\n",g_cli_dev_data.icram0.valist_ptr);
    printk("*g_cli_dev_data.icram0.valist_ptr 0x%x\n",*g_cli_dev_data.icram0.valist_ptr);
    printk("g_cli_dev_data.icram0.valist_ptr_ptr 0x%x\n",g_cli_dev_data.icram0.valist_ptr_ptr);
    printk("*g_cli_dev_data.icram0.valist_ptr_ptr 0x%x\n",*g_cli_dev_data.icram0.valist_ptr_ptr);
#endif
    printk("calling bridge 0x%x\n",&flags);

    return_value = hw_sec_rom_pub_bridge(appl_id, flags, g_cli_dev_data.icram0.valist_ptr_ptr); // 'Address of va_list' convention is inherited from previous projects
    va_end(ap);

    SEC_HAL_TRACE_EXIT
    return return_value;
}


/* MMU, dispatcher args should be already as physical addresses. */

void* hw_mmu_physical_address_get(void* arg)
{
    /*return ((void*) SEC_HAL_MEM_VIR2PHY_FUNC(arg));*/
#if 0
	printk("hw_mmu_physical_address_get param 0x%x \n",arg);
	printk("hw_mmu_physical_address_get phys_addr 0x%x \n",sec_hal_virt_to_icram_phys(arg));
#endif
    return ((void*) sec_hal_virt_to_icram_phys(arg));
/*    return ((void*) __pa(arg));*/
}


#define HW_ARM_DCACHE_OP_CLEAN                0  ///< Clean D cache(s)
#define HW_ARM_DCACHE_OP_INVALIDATE           1  ///< Invalidate D cache(s)
#define HW_ARM_DCACHE_OP_CLEAN_INVALIDATE     2  ///< Clean & invalidate D cache(s)
#define HW_ARM_DCACHE_LEVEL_POU               0  ///< Operate up to point of unification level (meaning L1 in R_Mobile_U)
/* D-Cache */
void hw_arm_dcache_maintenance(uint32_t op, uint32_t lvl)
{
    switch(op)
    {
        case HW_ARM_DCACHE_OP_CLEAN:
        case HW_ARM_DCACHE_OP_INVALIDATE:
        case HW_ARM_DCACHE_OP_CLEAN_INVALIDATE:
        default: break;
    }
}


/* L2-Cache */
void hw_arm_l2_cache_area_clean(void * virt_addr, int32_t size)
{
    //SEC_HAL_MEM_CACHE_CLEAN_FUNC(virt_addr, size);
    __cpuc_flush_dcache_area(virt_addr, size);
}

unsigned long sec_hal_virt_to_icram_phys(unsigned long virt_addr)
{
    unsigned long phys_addr;
    phys_addr = virt_addr - g_cli_dev_data.icram0.offset;
#if 0
    printk(" sec_hal_virt_to_icram_phys, virt: %x, phys; %x\n",virt_addr,phys_addr);
#endif
    return phys_addr;
}

/* ******************************** END ************************************ */
