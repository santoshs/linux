/* *********************************************************************** **
**                             Renesas                                     **
** *********************************************************************** */

/* ************************* COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of **
** Renesas and also is protected as an unpublished work under              **
** applicable Copyright laws. Recipient is to retain this program in       **
** confidence and is not permitted to use or make copies thereof other than**
** as permitted in a written agreement with Renesas.                       **
**                                                                         **
** Copyright (C) 2012 Renesas Electronics Corp.                            **
** All rights reserved.                                                    **
** *********************************************************************** */
#include "sec_hal_rt.h"
#include "sec_hal_rt_cmn.h"
#include "sec_hal_drm.h"
#include "sec_msg.h"
#include "sec_hal_rt_trace.h"
#include "sec_hal_dev_ioctl.h"
#include "sec_hal_dev_info.h"
#include "sec_hal_cmn.h"
#include "sec_hal_mdm.h"
#include "sec_hal_pm.h"
#include "sec_hal_toc.h"
#include "sec_hal_tee.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>

#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>

#include <asm/io.h>
#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>




/* ********************************************************************
 * LOCAL MACROS, static constants.
 * *******************************************************************/
#define FALSE 0
#define TRUE  1
#define DEFAULT_WDT_VALUE 60000
#define ICRAM1_SIZE 4096
#define ICRAM1_ADDRESS 0xE63A0000

#define SDTOC_SIZE 0x4000
#if 0
#define SDTOC_ADDRESS 0x46500000
#else
#define SDTOC_ADDRESS 0x47FE0000
#endif

#define PUBLIC_TOC_SIZE 4096
/*#define PUBLIC_TOC_ADDRESS 0x47FE0000*/


#ifndef gic_spi
#define gic_spi(param) 32 + param
#endif

void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz);
uint32_t sec_hal_reset_info_addr_register();
#ifdef SEC_STORAGE_SELFTEST_ENABLE
static uint32_t rpc_handler(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
#endif
int workaround_done = 0;

uint32_t secure_storage_pid;

/* **********************************************************************
 * MEM ALLOC information, defined here so that one spinlock is enough.
 * *********************************************************************/
struct mem_alloc_info
{
    void* virt_addr; /* the start address of the message area */
    uint8_t size;/* the size of allocation */
    uint8_t allocated; /* status of the block */
};
#define BLOCKCOUNT 128
struct mem_msg_area
{
    void* virt_baseptr; /* stores ioremap output */
    unsigned long phys_start; /* phys start addr */
    unsigned long phys_size; /* phys size */
    unsigned long offset; /* offset between physical and virtual addresses */
    /* blocks to-be-allocated for out & in msgs */
    struct mem_alloc_info msg_blocks[BLOCKCOUNT];
};
static inline void mem_msg_area_clear(struct mem_msg_area *ptr)
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
#define MEM_MSG_AREA_CLEAR(ptr) mem_msg_area_clear(ptr)

typedef struct
{
    struct list_head list;
    void* virt_addr; /* virtual address */
    TEEC_SharedMemory *shmem;
} shared_memory_node;

typedef struct
{
    struct list_head head;
} shared_memory_list;

/* **********************************************************************
 * RPC related data structures
 * *********************************************************************/
typedef struct
{
    struct list_head list;
    sd_rpc_params_t param;
} rpc_queue_item_t;
typedef struct
{
    struct list_head list_head;
    wait_queue_head_t waitq;
} rpc_queue_t;
static int
rpcq_init(rpc_queue_t* q)
{
    INIT_LIST_HEAD(&q->list_head);
    init_waitqueue_head(&q->waitq);
    return 0;
}

/* **********************************************************************
 * *********************************************************************/
static int
_rpcq_add(struct list_head* lst,
          sd_rpc_params_t* param_in)
{
    rpc_queue_item_t* new = kmalloc(sizeof(rpc_queue_item_t), GFP_KERNEL);
    if (!new){
        return -ENOMEM;
    }

    new->param = *param_in;

    list_add(&new->list, lst);
    return 0;
}
static int
rpcq_add_wakeup(rpc_queue_t* q,
                sd_rpc_params_t* param_in)
{
    _rpcq_add(&q->list_head, param_in);
    wake_up_interruptible(&q->waitq);
    return 0;
}

/* **********************************************************************
 * *********************************************************************/
static int
_rpcq_get(struct list_head* lst,
          sd_rpc_params_t* param_out)
{
    unsigned int tgid = (unsigned int)current->tgid;
    struct list_head *pos;
    rpc_queue_item_t *item;

    list_for_each(pos, lst) {
        item = list_entry(pos, rpc_queue_item_t, list);
        if (item->param.reserved2 == tgid) {
            *param_out = item->param;
            list_del(&item->list);
            kfree(item);
            return 1;
        }
    }

    return 0;
}
static int
rpcq_get_wait(rpc_queue_t* q,
              sd_rpc_params_t* item)
{
    return wait_event_interruptible(q->waitq, _rpcq_get(&q->list_head, item));
}


/* W/A to disable mem_prot in certain bootmodes */
static inline int
is_recovery(void)
{
	int i;
	char *c = &boot_command_line[0];

	if (c[0] && c[1] && c[2] && c[3]) {
		for (i=3; c[i]; i++)
			if (c[i-3] == 'a'
					&& c[i-2] == 'n'
					&& c[i-1] == 'd'
					&& c[i]   == 'r')
				return 0;
	} 

	return 1;
}


/* **********************************************************************
 * STATIC WRITABLE data of the device, the only 'global' writable data.
 * *********************************************************************/
struct device_data
{
    struct class* class;
    struct cdev cdev;
    struct semaphore sem;
    rpc_queue_t rpc_read_waitq;
    rpc_queue_t rpc_write_waitq;
    unsigned int wdt_upd; /* initial wdt value, received in rt_init */
    struct mem_msg_area icram0; /* memory (icram0) allocation information */
    struct platform_device *pdev;
} g_device_data =
{
    .wdt_upd = DEFAULT_WDT_VALUE,
    .icram0 = {.virt_baseptr = NULL},
};
static DEFINE_SPINLOCK(g_dev_spinlock); /* to protect above data struct */
/* CLIENT specific data */
struct client_data
{
    struct device_data *device;
    uint32_t drm_session_id;
    TEEC_Context *teec_context;
    TEEC_SharedMemory *next_teec_shmem;
    void * next_teec_shmem_buffer;
    shared_memory_list shmem_list;
};

/* initial DBGREG values, for Rnd.
 * Eventually should be read from general purpose register
 * which are accessible by hw debuggers. */
#define TDBG_SPI 81U
static uint32_t g_dbgreg1 = 0x20000000;
static uint32_t g_dbgreg2 = 0x00;
static uint32_t g_dbgreg3 = 0x00078077;
static int g_icram_mem_init_done = 0;

DATA_TOC_ENTRY * sdtoc_root =NULL;
DATA_TOC_ENTRY * public_toc_root =NULL;
uint8_t * public_id;
uint8_t * prcf;
uint32_t public_id_size;

int search_mem_node(struct list_head* lst, void * virt_addr)
    {
    struct list_head *pos;
    shared_memory_node *item;
    shared_memory_node *node_to_return = NULL;

    SEC_HAL_TRACE_ENTRY();
    SEC_HAL_TRACE("virt_addr: 0x%08x",virt_addr);

    list_for_each(pos, lst)
        {
        item = list_entry(pos, shared_memory_node, list);
        SEC_HAL_TRACE("item: 0x%08x",item);
        SEC_HAL_TRACE("item->virt_addr: 0x%08x",item->virt_addr);
        if (item->virt_addr == virt_addr)
            {
            SEC_HAL_TRACE("found node");
            node_to_return = item;
            break;
            }
        }

    SEC_HAL_TRACE_EXIT();
    return node_to_return;
    }


/* **********************************************************************
 * CLIENT INTERFACE RELATED FUNCTIONS.
 * *********************************************************************/
/* ----------------------------------------------------------------------
 * sec_hal_cli_open : allocate handle specific resources
 * --------------------------------------------------------------------*/
static int
sec_hal_cli_open(struct inode *inode,
                 struct file *filp)
{
    struct device_data *device;
    struct client_data *client;

    SEC_HAL_TRACE_ENTRY();

    device = container_of(inode->i_cdev, struct device_data, cdev);
    if (!device) {
        SEC_HAL_TRACE_EXIT_INFO(
            "failed to find cdev from container, aborting!");
        return -ENODEV;
    }
    if (nonseekable_open(inode, filp)) {
        SEC_HAL_TRACE_EXIT_INFO(
            "failed set node as 'nonseekable', aborting!");
        return -ENODEV;
    }
    client = kmalloc(sizeof(struct client_data), GFP_KERNEL);
    if (!client) {
        SEC_HAL_TRACE_EXIT_INFO(
            "failed to allocate memory, aborting!");
        return -ENODEV;
    }
    client->device = device;
    client->drm_session_id = 0;
    client->teec_context = NULL;
    client->next_teec_shmem = NULL;
    client->next_teec_shmem_buffer = NULL;

    INIT_LIST_HEAD(&(client->shmem_list.head));

    filp->private_data = client;

    SEC_HAL_TRACE_EXIT();
    return 0;
}
/* ----------------------------------------------------------------------
 * sec_hal_cli_release :
 * --------------------------------------------------------------------*/
static int
sec_hal_cli_release(struct inode *inode,
                    struct file *filp)
{

   struct client_data *client = filp->private_data;

    SEC_HAL_TRACE_ENTRY();

#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
    if (client->drm_session_id){
       sec_hal_drm_exit(1, &client->drm_session_id);
    }
#endif

/* In case of crash client mem_nodes need to be freed here */

    kfree(client);

    SEC_HAL_TRACE_EXIT();
    return 0;
}

#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
static inline int 
has_drm_session(struct client_data *client,
                uint32_t id)
{
    return (client->drm_session_id == id);
}


static inline uint32_t
_drm_enter(struct client_data *client,
           sd_ioctl_params_t *params)
{
    uint32_t id, ret;

    if (!has_drm_session(client, id)) {
        ret = sec_hal_drm_enter(current->pid, &id);
        if (SEC_HAL_RES_OK == ret) {
            client->drm_session_id = id;
            if (copy_to_user((void __user*)params->param0, &id, sizeof(uint32_t))) {
                ret = SEC_HAL_RES_FAIL;
            }
        }
        else {
            ret = SEC_HAL_RES_FAIL;
        }
    }
    else {
        ret = SEC_HAL_RES_FAIL;
    }

    return ret;
}

static inline uint32_t
_drm_exit(struct client_data *client,
          sd_ioctl_params_t *params)
{
    uint32_t id, ret;

    id = params->param0;
    if (has_drm_session(client, id)) {
        ret = sec_hal_drm_exit(1, &id);
        client->drm_session_id = 0;
    }
    else {
        ret = SEC_HAL_RES_FAIL;
    }

    return ret;
}

#define ENTIT_KEY_MAX_LEN 16
/* copy semantics wrapper. */
static inline uint32_t
_drm_set_entit_key(struct client_data *client,
                   sd_ioctl_params_t *params)
{
    uint32_t id, ret, key_len;
    uint8_t key[ENTIT_KEY_MAX_LEN] = {0};

    id = params->param0;
    key_len = params->param2;
    if (has_drm_session(client, id) &&
        ENTIT_KEY_MAX_LEN >= key_len) {
        if (copy_from_user(key, (const void __user*)params->param1, key_len)) {
            ret = SEC_HAL_RES_FAIL;
        }
        else {
            ret = sec_hal_drm_set_entit_key(id, key_len, key);
         }
    }
    else
    {
        ret = SEC_HAL_RES_FAIL;
    }

    return ret;
}

#define ECM_KEY_MAX_LEN 32
/* copy semantics wrapper. */
static inline uint32_t
_drm_derive_cw(struct client_data *client,
               sd_ioctl_params_t *params)
{
    uint32_t id, ret, ecm_len, flags = 0;
    uint8_t ecm[ECM_KEY_MAX_LEN] = {0};

    id = params->param0;
    ecm_len = params->param2;
    if (has_drm_session(client, id) &&
        ECM_KEY_MAX_LEN >= ecm_len) {
        if (copy_from_user(ecm, (const void __user*)params->param1, ecm_len)) {
            ret = SEC_HAL_RES_FAIL;
        }
        else {
            ret = sec_hal_drm_derive_cw(id, ecm_len, ecm, &flags);
            put_user(flags, (uint32_t __user*)params->param3);
         }
    }
    else
    {
        ret = SEC_HAL_RES_FAIL;
    }

    return ret;
}
#endif/* CONFIG_ARM_SEC_HAL_DRM_WVN*/

static inline uint32_t
pad_size(uint32_t sz)
{
    return ((sz+0xF) & ~0xF);
}
#define PTE_OFFSET -PTRS_PER_PTE 
static inline uint32_t
get_phys_addr(uint32_t virt_addr)
{
    uint32_t ret;
    if (PAGE_OFFSET <= virt_addr) {
        ret = (uint32_t)virt_to_phys((void*)virt_addr);
    }
    else {
        pgd_t *pgd;
        pmd_t *pmd;
        pte_t *pte;
        unsigned long *page_addr;
        pgd = pgd_offset(current->mm, virt_addr);
        pmd = pmd_offset(pgd, virt_addr);
        pte = pte_offset_map(pmd, virt_addr); 
        page_addr = (unsigned long*)((unsigned long)pte + (PTE_OFFSET*sizeof(pte)));
        ret = ((0xFFFFF000 & (*page_addr)) | (0x00000FFF & virt_addr));
    }
    //SEC_HAL_TRACE_HEX("ret", ret);
    return ret;
}

#define IV_SIZE 16

#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
/* copy semantics wrapper. */
static inline uint32_t
_drm_decrypt_video(struct client_data *client,
                   sd_ioctl_params_t *params)
{
    uint32_t id, ret, inl, outl = 0, phys_in = 0;
    uint8_t iv[IV_SIZE*2] = {0};
    void *iv_user = 0, *input = 0;

    id = params->param0;
    if (has_drm_session(client, id)) {
        iv_user = (void*)params->param1;
        inl = params->param3;

        if (iv_user && copy_from_user(iv, (const void __user*)iv_user, IV_SIZE)) {
            return SEC_HAL_RES_FAIL;
        }

        input = kmalloc(pad_size(inl), GFP_KERNEL);
        if (!input) {
            return -ENOMEM; 
        }

        if (copy_from_user(input, (const void __user*)params->param2, inl)) {
            kfree(input);
            return SEC_HAL_RES_FAIL;
        }

        phys_in = dma_map_single(&client->device->pdev->dev,
                input, pad_size(inl), DMA_TO_DEVICE);
        if (phys_in == 0) {
            SEC_HAL_TRACE("dma_map_single error: %d", phys_in);
            kfree(input);
            return -ENOMEM;
        }

        ret = sec_hal_drm_decrypt_video(id, iv, iv_user ? IV_SIZE : 0, inl,
                phys_in, params->param4,
                params->param5, &outl, iv+IV_SIZE);
        dma_unmap_single(&client->device->pdev->dev,
                phys_in, pad_size(inl), DMA_TO_DEVICE);
        kfree(input);

        if (SEC_HAL_RES_OK == ret) {
            if ((iv_user && copy_to_user((void __user*)iv_user, iv+IV_SIZE, IV_SIZE)) ||
                put_user(outl, (uint32_t __user*)params->reserved1)) {
                SEC_HAL_TRACE("failed in iv or outl copying to user");
                ret = SEC_HAL_RES_FAIL;
            }
        }
    }
    else {
        ret = SEC_HAL_RES_FAIL;
    }

    return ret;
}

/* copy semantics wrapper. */
static inline uint32_t
_drm_decrypt_audio(struct client_data *client,
                   sd_ioctl_params_t *params)
{
    uint32_t id, ret, inl, outl = 0, phys_in = 0, phys_out = 0;
    uint8_t iv[IV_SIZE*2] = {0};
    void *iv_user = 0, *input = 0, *output = 0;

    id = params->param0;
    if (has_drm_session(client, id)) {
        iv_user = (void*)params->param1;
        inl = params->param3;

        if (iv_user && copy_from_user(iv, (const void __user*)iv_user, IV_SIZE)) {
            return SEC_HAL_RES_FAIL;
        }

        input = kmalloc(pad_size(inl), GFP_KERNEL);
        output = kmalloc(pad_size(inl), GFP_KERNEL);
        if (!input || !output) {
            return -ENOMEM;
        }

        if (copy_from_user(input, (const void __user*)params->param2, inl)) {
            kfree(output);
            kfree(input);
            return SEC_HAL_RES_FAIL;
        }

        phys_in = dma_map_single(&client->device->pdev->dev,
                input, pad_size(inl), DMA_TO_DEVICE);
        if (phys_in == 0) {
            SEC_HAL_TRACE("dma_map_single error: %d", phys_in);
            kfree(output);
            kfree(input);
            return -ENOMEM;
        }

        phys_out = dma_map_single(&client->device->pdev->dev,
                output, pad_size(inl), DMA_FROM_DEVICE);
        if (phys_out == 0) {
            SEC_HAL_TRACE("dma_map_single error: %d", phys_out);
            kfree(output);
            kfree(input);
            return -ENOMEM;
        }

        ret = sec_hal_drm_decrypt_audio(id, iv, iv_user ? IV_SIZE : 0, inl,
                phys_in, phys_out,
                &outl, iv+IV_SIZE);
        if (SEC_HAL_RES_OK == ret &&
            copy_to_user((void __user*)params->param4, output, outl)) {
            SEC_HAL_TRACE("failed in output copying to user");
            ret = SEC_HAL_RES_FAIL;
        }
        dma_unmap_single(&client->device->pdev->dev,
                phys_in, pad_size(inl), DMA_TO_DEVICE);
        dma_unmap_single(&client->device->pdev->dev,
                phys_out, pad_size(inl), DMA_FROM_DEVICE);
        kfree(output);
        kfree(input);

        if (SEC_HAL_RES_OK == ret) {
            if ((iv_user && copy_to_user((void __user*)iv_user, iv+IV_SIZE, IV_SIZE)) ||
                put_user(outl, (uint32_t __user*)params->param5)) {
                SEC_HAL_TRACE("failed in iv or outl copying to user");
                ret = SEC_HAL_RES_FAIL;
            }
        }
    }
    else {
        ret = SEC_HAL_RES_FAIL;
    }

    return ret;
}

/* copy semantics wrapper. */
static inline uint32_t
_drm_wrap_keybox(struct client_data *client,
                 sd_ioctl_params_t *params)
{
    uint32_t ret, kbox_size = 0, wr_kbox_size = 0, trs_key_size = 0;
    void *kbox = 0, *wr_kbox = 0, *trs_key = 0;
    void *kbox_user = 0, *wr_kbox_user = 0, *trs_key_user = 0;

    kbox_user = (void*)params->param1;
    kbox_size = params->param2;
    wr_kbox_user = (void*)params->param3;
    trs_key_user = (void*)params->param5;
    trs_key_size = params->reserved1;
    if (!kbox_user || !kbox_size) {
        return -ENOMEM;
    }

    kbox = kmalloc(kbox_size, GFP_KERNEL);
    wr_kbox = wr_kbox_user ? kmalloc(2*kbox_size, GFP_KERNEL) : 0;
    trs_key = trs_key_size ? kmalloc(trs_key_size, GFP_KERNEL) : 0;
    if (!kbox) {
        return -ENOMEM;
    }

    if (copy_from_user(kbox, (const void __user*)kbox_user, kbox_size)) {
        kfree(trs_key);
        kfree(wr_kbox);
        kfree(kbox);
        return SEC_HAL_RES_FAIL;
    }
    if (copy_from_user(trs_key, (const void __user*)trs_key_user, trs_key_size)) {
        kfree(trs_key);
        kfree(wr_kbox);
        kfree(kbox);
        return SEC_HAL_RES_FAIL;
    }

    ret = sec_hal_drm_wrap_keybox((uint8_t*)kbox, kbox_size,
                (uint8_t*)wr_kbox, &wr_kbox_size,
                (uint8_t*)trs_key, trs_key_size);

    if (SEC_HAL_RES_OK == ret &&
        (wr_kbox_user && copy_to_user((void __user*)wr_kbox_user, wr_kbox, wr_kbox_size))) {
        ret = SEC_HAL_RES_FAIL;
    }
    if (put_user(wr_kbox_size, (uint32_t __user*)params->param4)) {
        ret = SEC_HAL_RES_FAIL;
    }

    kfree(trs_key);
    kfree(wr_kbox);
    kfree(kbox);
    return ret;
}

/* copy semantics wrapper. */
static inline uint32_t
_drm_install_keybox(struct client_data *client,
                    sd_ioctl_params_t *params)
{
    uint32_t ret, kbox_size = 0;
    void *kbox = 0;

    kbox_size = params->param2;
    kbox = kmalloc(kbox_size, GFP_KERNEL);
    if (!kbox) {
        return -ENOMEM;
    }

    if (copy_from_user(kbox, (const void __user*)params->param1, kbox_size)) {
        kfree(kbox);
        return SEC_HAL_RES_FAIL;
    }

    ret = sec_hal_drm_install_keybox((uint8_t*)kbox, kbox_size);

    kfree(kbox);
    return ret;
}

/* copy semantics wrapper. */
static inline uint32_t
_drm_valid_keybox(struct client_data *client,
                  sd_ioctl_params_t *params)
{
    return sec_hal_drm_is_keybox_valid();
}

static inline uint32_t
_get_random(struct client_data *client,
            sd_ioctl_params_t *params)
{
    uint32_t ret, size;
    void *rand;

    size = params->param2;
    rand = kmalloc(size, GFP_KERNEL);
    if (!rand) {
        return -ENOMEM;
    }

    ret = sec_hal_drm_get_random(size, (uint8_t*)rand);
    if (SEC_HAL_RES_OK == ret &&
        copy_to_user((void __user*)params->param1, rand, size)) {
        ret = SEC_HAL_RES_FAIL;
    }

    kfree(rand);
    return ret;
}

/* copy semantics wrapper. */
static inline uint32_t
_drm_get_device_id(struct client_data *client,
                   sd_ioctl_params_t *params)
{
    uint32_t ret, id_len, out_size = 0;
    void* id = 0;

    id_len = params->param1;
    if (!id_len){
        return -ENOMEM;
    }

    id = kmalloc(id_len, GFP_KERNEL);
    if (!id){
        return -ENOMEM;
    }

    ret = sec_hal_drm_get_device_id(id_len, (uint8_t*)id, &out_size);
    if (SEC_HAL_RES_OK == ret) {
        if (id_len < out_size) {
            ret = SEC_HAL_RES_FAIL;
        }
        else if(copy_to_user((void __user*)params->param2, id, out_size) ||
                put_user(out_size, (uint32_t __user*)params->param3)) {
            ret = SEC_HAL_RES_FAIL;
        }
    }

    kfree(id);
    return ret;
}

/* copy semantics wrapper. */
static inline uint32_t
_drm_get_keydata(struct client_data *client,
                 sd_ioctl_params_t *params)
{
    uint32_t ret, keyd_len, out_size = 0;
    void* keyd = 0;

    keyd_len = params->param1;
    if (!keyd_len){
        return -ENOMEM;
    }

    keyd = kmalloc(keyd_len, GFP_KERNEL);
    if (!keyd) {
        return -ENOMEM;
    }

    ret = sec_hal_drm_get_key_data(keyd_len, (uint8_t*)keyd, &out_size);
    if (SEC_HAL_RES_OK == ret) {
        if (keyd_len < out_size) {
            ret = SEC_HAL_RES_FAIL;
        }
        else if(copy_to_user((void __user*)params->param2, keyd, out_size) ||
                put_user(out_size, (uint32_t __user*)params->param3)) {
            ret = SEC_HAL_RES_FAIL;
        }
    }

    kfree(keyd);
    return ret;
}

#endif /* CONFIG_ARM_SEC_HAL_DRM_WVN */

static const int k_param_sz = sizeof(sd_ioctl_params_t);
/* ----------------------------------------------------------------------
 * sec_hal_cli_ioctl : IO control, make bi-directional sync operations.
 * --------------------------------------------------------------------*/
static long
sec_hal_cli_ioctl(struct file *filp,
                  unsigned int cmd,
                  unsigned long arg)
{
    long ret = (long) -EPERM;
    struct client_data *client = filp->private_data;
    struct device_data *device = client->device;
    sd_ioctl_params_t input;

    if (filp->f_flags & O_NONBLOCK) {
        SEC_HAL_TRACE_EXIT_INFO(
            "non-blocking IO not supported, aborting!");
        return -EPERM;
    }
    if(copy_from_user(&input, (void __user*)arg, k_param_sz)) {
        SEC_HAL_TRACE_EXIT_INFO(
            "failed to get args from user, aborting!");
        return -EFAULT;
    }

    if (down_interruptible(&device->sem)) {
        SEC_HAL_TRACE_EXIT_INFO("interrupted, restart syscall");
        return -ERESTARTSYS;
    }

    switch (cmd)
    {
#ifdef SEC_STORAGE_SELFTEST_ENABLE
        case SD_SEC_STORAGE_SELFTEST:
        {
            secure_storage_pid = current->tgid;
            ret = sec_hal_rt_sec_storage_selftest(&rpc_handler, input.param1);
        }break;
#endif
#ifdef SEC_STORAGE_DS_TEST_ENABLE
        /* TEMPORARY, for testing purposes, send SEC_SERV_SIMU_DS(0/1)_TEST message to Demo (DS) */
        case SD_SIMU_DS0_TEST:
        {
            secure_storage_pid = current->tgid;
            ret = sec_hal_rt_simu_ds_test(0, input.param1, 0, 1); /* DS0, with or without params(0, 1)*/
        }break;
        case SD_SIMU_DS1_TEST:
        {
            secure_storage_pid = current->tgid;
            ret = sec_hal_rt_simu_ds_test(1, input.param1, 0, 1); /* DS1, with or without params(0, 1)*/
        }break;
#endif
        case SD_INIT:{ret = 0;} break;
        case SD_EXIT:{SEC_HAL_TRACE("EXIT - NOP"); ret = 0;}break;
        case SD_KEY_INFO:
        {
            if (!access_ok(VERIFY_WRITE, (void __user *)input.param0, sizeof(sec_hal_key_info_t))) {
                SEC_HAL_TRACE("keyinfo not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_key_info_get((sec_hal_key_info_t*)input.param0);
            }
        }break;
        case SD_CERT_REGISTER:
        {
            if (!access_ok(VERIFY_READ, (void __user *)input.param0, input.param1)) {
                SEC_HAL_TRACE("cert not ok, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_WRITE, (void __user*)input.param2, PARAM_SZ)) {
                SEC_HAL_TRACE("objid not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_cert_register(
                        (void*)input.param0,/*IN*/
                        input.param1,/*IN*/
                        (uint32_t*)input.param2/*OUT*/);
            }
        }break;
        case SD_DATA_CERT_REGISTER:
        {
            if (!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1)) {
                SEC_HAL_TRACE("cert not ok, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_READ, (void __user*)input.param2, input.param3)) {
                SEC_HAL_TRACE("data not ok, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_WRITE, (void __user*)input.param4, sizeof(uint32_t))) {
                SEC_HAL_TRACE("obj_id ptr not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_data_cert_register(
                        (void*) input.param0, /*IN*/
                        input.param1, /*IN*/
                        (void*) input.param2, /*IN*/
                        input.param3, /*IN*/
                        (uint32_t*)input.param4);
            }
        }break;
#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
        case SD_RANDOM_NUMBER_GET:{ ret = _get_random(client, &input); }break;
#endif
        case SD_MAC_ADDRESS_GET:
        {
            if (input.param0 > SEC_HAL_MAX_MAC_INDEX) {
                SEC_HAL_TRACE("mac_addr_index out-of-bounds, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_WRITE, (void __user*)input.param1, SEC_HAL_MAC_SIZE)) {
                SEC_HAL_TRACE("mac_addr not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_mac_address_get(
                        input.param0, /*IN*/
                        (sec_hal_mac_address_t *)input.param1 /*OUT*/);
            }
        }break;
        case SD_IMEI_GET:
        {
            if (!access_ok(VERIFY_WRITE, (void __user*)input.param0, SEC_HAL_MAX_IMEI_SIZE)) {
                SEC_HAL_TRACE("imei not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_imei_get((sec_hal_imei_t *)input.param0 /*OUT*/);
            }
        }break;
        case SD_RAT_BAND_GET:{SEC_HAL_TRACE("!!RAT_BAND - NOT IMPL.!!");}break;
        case SD_PP_FLAGS_COUNT_GET:{SEC_HAL_TRACE("!!PP_FLAGS_COUNT - NOT IMPL.!!");}break;
        case SD_PP_FLAGS_GET:{SEC_HAL_TRACE("!!PP_FLAGS - NOT IMPL.!!");}break;
        case SD_SL_LEVELS_OPEN:
        {
            if (0/*!access_ok(VERIFY_READ, unlock_codes,)*/ /*TBDL: howto verify??*/) {
                SEC_HAL_TRACE("unlock_codes not ok, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_WRITE, (void __user*)input.param2, sizeof(uint32_t))) {
                SEC_HAL_TRACE("post_status ptr not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_simlock_levels_open(
                        (uint32_t)input.param0, /*IN*/
                        (void*) input.param1, /*IN*/
                        (uint32_t*)input.param2 /*OUT*/);
            }
        }break;
        case SD_SL_LEVEL_OPEN:
        {
            if (input.param1 > SEC_HAL_MAX_SIMLOCK_CODE_LENGTH) {
                SEC_HAL_TRACE("unlock code too long, aborting!");
                ret = -EFAULT;
            }
            if (!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1)) {
                SEC_HAL_TRACE("unlock code not ok, aborting!");
                ret = -EFAULT;
            }
            else if (input.param2 > SEC_HAL_MAX_SIMLOCK_LEVELS_COUNT) {
                SEC_HAL_TRACE("level too big, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_simlock_level_open(
                        (char*)input.param0, /*IN*/
                        (uint8_t)input.param2 /*IN*/);
            }
        }break;
        case SD_SL_LEVEL_STATUS_GET:
        {
            if (!access_ok(VERIFY_WRITE, (void __user*)input.param0, sizeof(uint32_t))) {
                SEC_HAL_TRACE("status not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_simlock_level_status_get((uint32_t*)input.param0 /*OUT*/);
            }
        }break;
        case SD_AUTH_DATA_SIZE_GET:
        {
            if (!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1)) {
                SEC_HAL_TRACE("input_data not ok, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_WRITE, (void __user*)input.param2, sizeof(uint32_t))) {
                SEC_HAL_TRACE("auth_data_size ptr not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_auth_data_size_get(
                        (void*)input.param0, /*IN*/
                        input.param1, /*IN*/
                        (uint32_t*)input.param2 /*OUT*/);
            }
        }break;
        case SD_AUTH_DATA_GET:
        {
            if (!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1)) {
                SEC_HAL_TRACE("input_data not ok, aborting!");
                ret = -EFAULT;
            }
            else if (!access_ok(VERIFY_WRITE, (void __user*)input.param2, input.param3)) {
                SEC_HAL_TRACE("auth_data not ok, aborting!");
                ret = -EFAULT;
            }
            else {
                ret = sec_hal_rt_auth_data_get(
                        (void*)input.param0, /*IN*/
                        input.param1, /*IN*/
                        (void*)input.param2, /*OUT*/
                        input.param3 /*IN*/);
            }
        }break;
        case SD_SELFTEST:{ ret = sec_hal_rt_selftest(); }break;
#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
        case SD_DRM_ENTER_PLAY:{ ret = _drm_enter(client, &input); }break;
        case SD_DRM_EXIT_PLAY:{ ret = _drm_exit(client, &input); }break;
        case SD_DRM_SET_ENTIT_KEY:{ ret = _drm_set_entit_key(client, &input); }break;
        case SD_DRM_DER_CTL_WORD:{ ret = _drm_derive_cw(client, &input); }break;
        case SD_DRM_DECRYPT_AUDIO:{ ret = _drm_decrypt_audio(client, &input); }break;
        case SD_DRM_DECRYPT_VIDEO:{ ret = _drm_decrypt_video(client, &input); }break;
        case SD_DRM_VALID_KEYBOX:{ ret = _drm_valid_keybox(client, &input); }break;
        case SD_DRM_DEVICE_ID_GET:{ ret = _drm_get_device_id(client, &input); }break;
        case SD_DRM_KEYDATA_GET:{ ret = _drm_get_keydata(client, &input); }break;
        case SD_DRM_WRAP_KEYBOX:{ ret = _drm_wrap_keybox(client, &input); }break;
        case SD_DRM_INSTALL_KEYBOX:{ ret = _drm_install_keybox(client, &input); }break;
#endif /* CONFIG_ARM_SEC_HAL_DRM_WVN */
		case SD_TEE_INIT_CONTEXT:
		{
			uint32_t* name = (uint32_t *)input.param0;
			uint32_t* context = (uint32_t *)input.param1;

            if(client->teec_context !=NULL)
                {
                SEC_HAL_TRACE("Context already open");
                ret = TEEC_ERROR_GENERIC;
                }
            else
                {
                client->teec_context = kmalloc(sizeof(TEEC_Context), GFP_KERNEL);

                copy_from_user(client->teec_context,context,sizeof(TEEC_Context));

                if(workaround_done==0)
                    {
                    ret = sec_hal_tee_allocate_TA_memory_workaround();
                    workaround_done = 1;
                    }

                ret = sec_hal_tee_initialize_context(name,client->teec_context);
                copy_to_user(context, client->teec_context, sizeof(TEEC_Context) );
                SEC_HAL_TRACE("client->teec_context->imp.tag 0x%x",client->teec_context->imp.tag);
                SEC_HAL_TRACE("client->teec_context->imp.hal_connection 0x%x",client->teec_context->imp.hal_connection);
                }
		}break;
		case SD_TEE_FINALIZE_CONTEXT:
		{
			uint32_t* context = (uint32_t *)input.param0;

			ret = sec_hal_tee_finalize_context(client->teec_context);
            client->teec_context=NULL;

		}break;

		case SD_TEE_OPEN_SESSION:
		{
			uint32_t* context = (uint32_t *)input.param0;
			uint32_t* session = (uint32_t *)input.param1;
			uint32_t* destination = (uint32_t *)input.param2;
			uint32_t connectionMethod = input.param3;
			uint32_t* connectionData = input.param4;
			uint32_t* operation = input.param5;
			uint32_t* returnOrigin = input.reserved1;
            TEEC_Session *kernel_session;


            kernel_session = kmalloc(sizeof(TEEC_Session),GFP_KERNEL);
            copy_from_user(kernel_session,session,sizeof(TEEC_Session));

		    ret = sec_hal_tee_open_session(client->teec_context,
                                           kernel_session,
                                           destination,
                                           connectionMethod,
                                           connectionData,
                                           operation,
                                           returnOrigin);

            SEC_HAL_TRACE("client->teec_context->imp.tag 0x%x",client->teec_context->imp.tag);
            SEC_HAL_TRACE("client->teec_context->imp.hal_connection 0x%x",client->teec_context->imp.hal_connection);
            SEC_HAL_TRACE("kernel_session->imp.tag 0x%x",kernel_session->imp.tag);
            copy_to_user(session, kernel_session, sizeof(TEEC_Session));
            kfree(kernel_session);
		}break;

		case SD_TEE_CLOSE_SESSION:
		{
			uint32_t* session = (uint32_t *)input.param0;
            TEEC_Session *kernel_session;

            kernel_session = kmalloc(sizeof(TEEC_Session),GFP_KERNEL);
            copy_from_user(kernel_session,session,sizeof(TEEC_Session));

			ret = sec_hal_tee_close_session(kernel_session);
            kfree(kernel_session);
		}break;

		case SD_TEE_INVOKE_COMMAND:
		{
			uint32_t* session = (uint32_t *)input.param0;
			uint32_t commandID = input.param1;
			uint32_t* operation = input.param2;
			uint32_t* returnOrigin = input.param3;
            TEEC_Session *kernel_session;

            kernel_session = kmalloc(sizeof(TEEC_Session),GFP_KERNEL);
            copy_from_user(kernel_session,session,sizeof(TEEC_Session));

			ret = sec_hal_tee_invoke_command(kernel_session, commandID, operation, returnOrigin);
            kfree(kernel_session);
		}break;

		case SD_TEE_PRE_MMAP:
		{
			uint32_t* shmem = (uint32_t *)input.param0;
            uint32_t size_rounded_to_pages;


            client->next_teec_shmem = kmalloc(sizeof(TEEC_SharedMemory), GFP_KERNEL);
            copy_from_user(client->next_teec_shmem,shmem,sizeof(TEEC_SharedMemory));

			SEC_HAL_TRACE("client->next_teec_shmem 0x%x", client->next_teec_shmem);
			SEC_HAL_TRACE("client->next_teec_shmem->size %d", client->next_teec_shmem->size);
			SEC_HAL_TRACE("client->next_teec_shmem->flags 0x%x", client->next_teec_shmem->flags);

            size_rounded_to_pages = ((client->next_teec_shmem->size / 4096) + 1) * 4096;
			SEC_HAL_TRACE("size_rounded_to_pages 0x%x", size_rounded_to_pages);

            client->next_teec_shmem_buffer = kmalloc(size_rounded_to_pages, GFP_KERNEL);
            client->next_teec_shmem->buffer = virt_to_phys(client->next_teec_shmem_buffer);

			SEC_HAL_TRACE("client->next_teec_shmem->buffer 0x%x", client->next_teec_shmem->buffer);

            if(TEEC_SUCCESS!=sec_hal_tee_register_shared_memory_area(client->teec_context,client->next_teec_shmem))
                {
                SEC_HAL_TRACE("registering shared memory failed\n");
                kfree(client->next_teec_shmem->buffer);
                }

            copy_to_user(shmem, client->next_teec_shmem, sizeof(TEEC_SharedMemory));

			ret = TEEC_SUCCESS;
		}break;
		case SD_SECURE_STORAGE_DAEMON_PID_REGISTER:
		{
            secure_storage_pid = current->tgid;
			ret = SEC_HAL_RES_OK;
		}break;
        default:{SEC_HAL_TRACE("DEFAULT!");}break;
    }

    up(&device->sem);

    return ret; /* directly return API definitions */
}

#define RPC_SUCCESS 0x00
#define RPC_FAILURE 0x01

void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz);
void  sec_hal_mem_msg_area_free(void *virt_addr);
/* **********************************************************************
 * RPC HANDLER
 * *********************************************************************/
static uint32_t
rpc_handler(uint32_t id,
            uint32_t p1,
            uint32_t p2,
            uint32_t p3,
            uint32_t p4)
{
    uint32_t ret = 0x00, size = 0x00;
    struct device_data* device = &g_device_data;
    void *kernel_data_ptr = NULL;
    sd_rpc_params_t params = {id, p1, p2, p3, p4, 0, (uint32_t)(current->tgid), (uint32_t)(current->tgid)};
    sec_msg_handle_t in_handle, out_handle;
    uint64_t offset, filesize;

    SEC_HAL_TRACE_ENTRY();

    switch (id) /* pre-ipc step for callbacks*/
    {
        case SEC_HAL_RPC_ALLOC:
        {
            ret = (uint32_t)SEC_HAL_MEM_VIR2PHY_FUNC(sec_hal_mem_msg_area_calloc(1, p1));
            SEC_HAL_TRACE("SECHAL_RPC_ALLOC: p1=0x%08x, ret=0x%08x",p1,ret);
            SEC_HAL_TRACE_EXIT();
            return ret;
        }break;
        case SEC_HAL_RPC_FREE:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FREE: p1=0x%08x",p1);
            sec_hal_mem_msg_area_free(SEC_HAL_MEM_PHY2VIR_FUNC(p1));
            SEC_HAL_TRACE_EXIT();
            return RPC_SUCCESS;
        }break;

        /* Secure Storage RPC's -- p1=*out_msg(to Secure), p2=*in_msg(from Secure), p3=0 */
        case SEC_HAL_RPC_FS_LOOKUP:
        {
            sec_msg_open(&in_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p2)));
            SEC_HAL_TRACE("SECHAL_RPC_FS_LOOKUP(1): p1=0x%08x, p2=0x%08x, in_handle=0x%08x",p1,p2,in_handle);
            /* Read in_msg from ICRAM (dir handle, namelen and filename) */
            /* Pass parameters (namelen and filename) to Sec Server */
            sec_msg_param_read32(&in_handle, &(params.param1));          /* dummy read: directory handle */
            sec_msg_param_read32(&in_handle, &(params.param1));                       /* filename length */

            /* Filename length must be valid */
            if (params.param1 > SEC_STORAGE_FILENAME_MAXLEN) {
                sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
                sec_msg_param_write32(&out_handle, SFS_STATUS_INVALID_NAME, SEC_MSG_PARAM_ID_NONE); /* status */
                sec_msg_param_write32(&out_handle, 0, SEC_MSG_PARAM_ID_NONE); /* null handle */
                sec_msg_close(&out_handle);
                sec_msg_close(&in_handle);
                SEC_HAL_TRACE("Too long filename! (%d)", params.param1);
                SEC_HAL_TRACE_EXIT();
                return RPC_SUCCESS; /* Too long filename, return without calling Sec Server */
            }
            /* Read filename only if it was not too long */
            sec_msg_param_read(&in_handle, (void *)(params.fname), (uint16_t)params.param1); /* filename */
            SEC_HAL_TRACE("SECHAL_RPC_FS_LOOKUP(1): fnamelen: %d, fname: %s",params.param1, params.fname);
            sec_msg_close(&in_handle);
	    params.reserved2 = secure_storage_pid; /* this is used to access sec storage at correct thread's context*/
        }break;
        case SEC_HAL_RPC_FS_READ:
        {
            sec_msg_open(&in_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p2)));
            SEC_HAL_TRACE("SECHAL_RPC_FS_READ(1): p1=0x%08x, p2=0x%08x, in_handle=0x%08x",p1,p2,in_handle);
            /* Read in_msg from ICRAM (file handle, offset(64b) and length) */
            /* Pass parameters (file handle and length) to Sec Server */
            sec_msg_param_read32(&in_handle, &(params.param1)); /* file handle */
            sec_msg_param_read64(&in_handle, &offset);          /* offset (unused) */
            sec_msg_param_read32(&in_handle, &(params.param2)); /* length of file */
            SEC_HAL_TRACE("SECHAL_RPC_FS_READ(1): f-handle=%d, file_len=%d",params.param1,params.param2);
            sec_msg_close(&in_handle);

            /* Filesize must be valid */
            if (params.param2 > SEC_STORAGE_FILE_MAXLEN) {
                sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
                sec_msg_param_write32(&out_handle, SFS_STATUS_INVALID_CALL, SEC_MSG_PARAM_ID_NONE); /* status */
                sec_msg_param_write32(&out_handle, 0, SEC_MSG_PARAM_ID_NONE); /* length read 0 */
                sec_msg_close(&out_handle);
                SEC_HAL_TRACE("Illegal filesize!");
                SEC_HAL_TRACE_EXIT();
                return RPC_SUCCESS; /* Illegal filesize, return without calling Sec Server */
            }
	    params.reserved2 = secure_storage_pid; /* this is used to access sec storage at correct thread's context*/
        }break;
        case SEC_HAL_RPC_FS_WRITE:
        {
            sec_msg_open(&in_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p2)));
            SEC_HAL_TRACE("SECHAL_RPC_FS_WRITE(1): p1=0x%08x, p2=0x%08x, in_handle=0x%08x",p1,p2,in_handle);
            /* Read in_msg from ICRAM (file handle, offset(64b), length and file contents) */
            /* Pass parameters (file handle, length and file contents) to Sec Server */
            sec_msg_param_read32(&in_handle, &(params.param1)); /* pass file handle */
            sec_msg_param_read64(&in_handle, &offset);          /* offset (unused) */
            sec_msg_param_read32(&in_handle, &(params.param2)); /* pass length of file */
            SEC_HAL_TRACE("SECHAL_RPC_FS_WRITE(1): f-handle=%d, file_len=%d", params.param1,params.param2);

            /* Filesize must be valid */
            if (params.param2 <= SEC_STORAGE_FILE_MAXLEN) {
                /* Read file from sec message to params.data */
                sec_msg_param_read(&in_handle, (void *)(params.data), (uint16_t)params.param2);
                sec_msg_close(&in_handle);
            }
            else {
                sec_msg_close(&in_handle);
                sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
                sec_msg_param_write32(&out_handle, SFS_STATUS_INVALID_CALL, SEC_MSG_PARAM_ID_NONE); /* status */
                sec_msg_close(&out_handle);
                SEC_HAL_TRACE("Illegal filesize!");
                SEC_HAL_TRACE_EXIT();
                return RPC_SUCCESS; /* Illegal filesize, return without calling Sec Server */
            }
	    params.reserved2 = secure_storage_pid; /* this is used to access sec storage at correct thread's context*/
        }break;
        case SEC_HAL_RPC_FS_CREATE:
        {
            sec_msg_open(&in_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p2)));
            SEC_HAL_TRACE("SECHAL_RPC_FS_CREATE(1): p1=0x%08x, p2=0x%08x, in_handle=0x%08x",p1,p2,in_handle);
            /* Read in_msg from ICRAM (dir handle, namelen, "filename") */
            /* Pass parameters (namelen and filename) to Sec Server     */
            sec_msg_param_read32(&in_handle, &(params.param1));          /* dummy read: directory handle */
            sec_msg_param_read32(&in_handle, &(params.param1));                       /* filename length */

            /* Filename length must be valid */
            if (params.param1 > SEC_STORAGE_FILENAME_MAXLEN) {
                sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
                sec_msg_param_write32(&out_handle, SFS_STATUS_INVALID_NAME, SEC_MSG_PARAM_ID_NONE); /* status */
                sec_msg_close(&out_handle);
                sec_msg_close(&in_handle);
                SEC_HAL_TRACE("Too long filename! (%d)", params.param1);
                SEC_HAL_TRACE_EXIT();
                return RPC_SUCCESS; /* Too long filename, return without calling Sec Server */
            }
            /* Read filename only if it was not too long */
            sec_msg_param_read(&in_handle, (void *)(params.fname), (uint16_t)params.param1); /* filename */
            SEC_HAL_TRACE("SECHAL_RPC_FS_CREATE(1): fnamelen: %d, fname: %s",params.param1, params.fname);
            sec_msg_close(&in_handle);
	    params.reserved2 = secure_storage_pid; /* this is used to access sec storage at correct thread's context*/
        }break;
        case SEC_HAL_RPC_FS_SIZE:
        {
            sec_msg_open(&in_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p2)));
            SEC_HAL_TRACE("SECHAL_RPC_FS_SIZE(1): p1=0x%08x, p2=0x%08x, in_handle=0x%08x",p1,p2,in_handle);
            /* Read in_msg from ICRAM (file handle) */
            /* Pass parameter (file handle) to Sec Server */
            sec_msg_param_read32(&in_handle, &(params.param1)); /* file handle */
            SEC_HAL_TRACE("SECHAL_RPC_FS_SIZE(1): f-handle=%d",params.param1);
            sec_msg_close(&in_handle);
	    params.reserved2 = secure_storage_pid; /* this is used to access sec storage at correct thread's context*/
        }break;
        case SEC_HAL_RPC_FS_ROOT:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FS_ROOT(x): p1=0x%08x",p1);
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            sec_msg_param_write32(&out_handle, SFS_STATUS_NOT_SUPPORTED, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_param_write32(&out_handle, 0, SEC_MSG_PARAM_ID_NONE); /* null handle */
            sec_msg_close(&out_handle);
            SEC_HAL_TRACE("SECHAL_RPC_FS_ROOT(x): STATUS_NOT_SUPPORTED=0x%08x, handle=0x%08x",SFS_STATUS_NOT_SUPPORTED,NULL);
            SEC_HAL_TRACE_EXIT();
            return RPC_SUCCESS;
        }break;
        case SEC_HAL_RPC_FS_MOVE:
        case SEC_HAL_RPC_FS_REMOVE:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FS_(RE)MOVE(x): p1=0x%08x, p2=0x%08x",p1,p2);
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            sec_msg_param_write32(&out_handle, SFS_STATUS_NOT_SUPPORTED, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_close(&out_handle);
            SEC_HAL_TRACE("SECHAL_RPC_FS_(RE)MOVE(x): STATUS_NOT_SUPPORTED=0x%08x",SFS_STATUS_NOT_SUPPORTED);
            SEC_HAL_TRACE_EXIT();
            return RPC_SUCCESS;
        }break;

        default: break;
    }

    SEC_HAL_TRACE("from tgid: %d, to tgid: %d",params.reserved1, params.reserved2);

    rpcq_add_wakeup(&device->rpc_read_waitq, &params);
    if (rpcq_get_wait(&device->rpc_write_waitq, &params)) {
        SEC_HAL_TRACE_EXIT_INFO("interrupted, aborting!");
        return RPC_FAILURE;
    }

    switch (id)/* post-ipc step for params conversion and etc. */
    {
        case SEC_HAL_RPC_PROT_DATA_ALLOC:
        {
            SEC_HAL_TRACE("case SEC_HAL_RPC_PROT_DATA_ALLOC:");
            SEC_HAL_TRACE("params.param4 (size): %d", params.param4);
            SEC_HAL_TRACE("params.param3 (user_data_prt): 0x%x", params.param3);
            sec_msg_handle_t ret_handle;
            sec_msg_t *ret_msg = sec_msg_alloc(&ret_handle,
                    3*sec_msg_param_size(sizeof(uint32_t)),
                    SEC_MSG_OBJECT_ID_NONE,
                    0,
                    SEC_HAL_MSG_BYTE_ORDER); /* dealloc by secenv */
            if (ret_msg && SEC_HAL_RES_OK == params.reserved1) {
                /* ensure that the prot_data is in SDRAM memory */
                if(NULL != params.param3 && NULL != params.param4) {
                    size = params.param4;
                    /* ensure that the prot_data is in SDRAM memory */
                    kernel_data_ptr = kmalloc(params.param4, GFP_KERNEL);
                    copy_from_user(kernel_data_ptr, params.param3, size);
                    SEC_HAL_MEM_CACHE_CLEAN_FUNC(kernel_data_ptr, size);
                    SEC_HAL_TRACE("kernel_data_ptr: 0x%x", kernel_data_ptr);
                }
                else {
                    SEC_HAL_TRACE("Allocated data is null!");
                    size = 0;
                }
            }
            SEC_HAL_TRACE("kernel_data_ptr: 0x%x", kernel_data_ptr);
            SEC_HAL_TRACE("params.reserved1: 0x%x", params.reserved1);

            sec_msg_param_write32(&ret_handle, params.reserved1,
                    SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write32(&ret_handle, size, SEC_MSG_PARAM_ID_NONE);
            sec_msg_param_write32(&ret_handle, virt_to_phys(kernel_data_ptr),
                    SEC_MSG_PARAM_ID_NONE);
            SEC_HAL_TRACE_EXIT_INFO("data_size == %u", size);
            return (uint32_t) SEC_HAL_MEM_VIR2PHY_FUNC(ret_msg);
        }break;
        case SEC_HAL_RPC_PROT_DATA_FREE: /*NOP*/ break;

        /* Secure Storage RPC's */
        case SEC_HAL_RPC_FS_LOOKUP:
        {
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            SEC_HAL_TRACE("SECHAL_RPC_FS_LOOKUP(2): p1=0x%08x",p1);
            /* Write out_msg to ICRAM: status, file handle */
            sec_msg_param_write32(&out_handle, params.reserved1, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_param_write32(&out_handle, params.param4, SEC_MSG_PARAM_ID_NONE);    /* file handle */
            sec_msg_close(&out_handle);
            SEC_HAL_TRACE("SECHAL_RPC_FS_LOOKUP(2): status=%d, handle=%d", params.reserved1,params.param4);
        }break;
        case SEC_HAL_RPC_FS_READ:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FS_READ(2): p1=0x%08x",p1);
            /* Write out_msg to ICRAM: status, length read, file */
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            sec_msg_param_write32(&out_handle, params.reserved1, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_param_write32(&out_handle, params.param4, SEC_MSG_PARAM_ID_NONE);    /* length read */

            /* Copy data only if length read > 0 */
            if (params.param4) {
                /* Write file to ICRAM from params.data */
                sec_msg_param_write(&out_handle, (void *)(params.data), params.param4, SEC_MSG_PARAM_ID_NONE);
                sec_msg_close(&out_handle);
            }
            SEC_HAL_TRACE("SECHAL_RPC_FS_READ(2): status=%d, l_read=%d",params.reserved1,params.param4);
        }break;
        case SEC_HAL_RPC_FS_WRITE:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FS_WRITE(2): p1=0x%08x",p1);
            /* Write out_msg to ICRAM: status */
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            sec_msg_param_write32(&out_handle, params.reserved1, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_close(&out_handle);
            SEC_HAL_TRACE("SECHAL_RPC_FS_WRITE(2): status=%d",params.reserved1);
        }break;
        case SEC_HAL_RPC_FS_CREATE:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FS_CREATE(2): p1=0x%08x",p1);
            /* Write out_msg to ICRAM: status */
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            sec_msg_param_write32(&out_handle, params.reserved1, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_close(&out_handle);
            SEC_HAL_TRACE("SECHAL_RPC_FS_CREATE(2): status=%d",params.reserved1);
        }break;
        case SEC_HAL_RPC_FS_SIZE:
        {
            SEC_HAL_TRACE("SECHAL_RPC_FS_SIZE(2): p1=0x%08x",p1);
            /* Write out_msg to ICRAM: status, size(64b) */
            filesize = params.param4;
            filesize <<= 32;
            filesize |= params.param5;
            sec_msg_open(&out_handle, (sec_msg_t *)(SEC_HAL_MEM_PHY2VIR_FUNC(p1)));
            sec_msg_param_write32(&out_handle, params.reserved1, SEC_MSG_PARAM_ID_NONE); /* status */
            sec_msg_param_write64(&out_handle, filesize, SEC_MSG_PARAM_ID_NONE);         /* size */
            sec_msg_close(&out_handle);
            SEC_HAL_TRACE("SECHAL_RPC_FS_SIZE(2): status=%d, size=%d",params.reserved1,filesize);
        }break;

        default: break;
    }

    SEC_HAL_TRACE_EXIT();
    return RPC_SUCCESS;
}

/* ----------------------------------------------------------------------
 * sec_hal_rpc_read : read content from rpc queque.
 * --------------------------------------------------------------------*/
static ssize_t
sec_hal_rpc_read(struct file *filp,
                 char __user* buf,
                 size_t count,
                 loff_t *ppos)
{
    int cnt = 0;
    struct client_data *client = filp->private_data;
    struct device_data *device = client->device;
    sd_rpc_params_t param;

    SEC_HAL_TRACE_ENTRY();

    if (filp->f_flags & O_NONBLOCK) {
        SEC_HAL_TRACE_EXIT_INFO(
            "non-blocking IO not supported, aborting!");
        return -EPERM;
    }

    if (rpcq_get_wait(&device->rpc_read_waitq, &param)) {
        SEC_HAL_TRACE_EXIT_INFO("interrupted, restart syscall");
        return -ERESTARTSYS;
    }
    cnt = copy_to_user(buf, &param, SD_RPC_PARAMS_SZ);

    SEC_HAL_TRACE_EXIT();
    return (SD_RPC_PARAMS_SZ-cnt);
}

/* ----------------------------------------------------------------------
 * sec_hal_rpc_write : write content to rpc queque.
 * --------------------------------------------------------------------*/
static ssize_t
sec_hal_rpc_write(struct file *filp,
                  const char *buf,
                  size_t count,
                  loff_t *ppos)
{
    int cnt = 0;
    struct client_data *client = filp->private_data;
    struct device_data *device = client->device;
    sd_rpc_params_t param;

    SEC_HAL_TRACE_ENTRY();

    if (filp->f_flags & O_NONBLOCK) {
        SEC_HAL_TRACE_EXIT_INFO("non-blocking IO not supported, aborting!");
        return -EPERM;
    }

    cnt = copy_from_user(&param, buf, SD_RPC_PARAMS_SZ);
    rpcq_add_wakeup(&device->rpc_write_waitq, &param);

    SEC_HAL_TRACE_EXIT();
    return (SD_RPC_PARAMS_SZ-cnt);
}


/*******************************************************************************
 * Function   : sec_hal_memory_tablewalk
 * Description: This function translates the virtual address into the physical address.
 * Parameters : virt_addr	   - virtual address
 * Returns	  : phys_addr	   - physical address
 *******************************************************************************/
unsigned long sec_hal_memory_tablewalk(
	unsigned long		virt_addr
)
{
	pgd_t			*pgd;
	pmd_t			*pmd;
	pte_t			*pte;

	unsigned long	page_num;
	unsigned long	phys_addr;
	unsigned long	*page_addr;

    SEC_HAL_TRACE_ENTRY();
    SEC_HAL_TRACE("virt_addr: 0x%08x",virt_addr);
    SEC_HAL_TRACE("PAGE_OFFSET: 0x%08x",PAGE_OFFSET);

    if (PAGE_OFFSET <= virt_addr) {
		phys_addr = virt_to_phys((void *)virt_addr);
	} else {
		pgd = pgd_offset(current->mm, virt_addr);
        SEC_HAL_TRACE("pgd: 0x%08x",pgd);
		pmd = pmd_offset(pgd, virt_addr);
        SEC_HAL_TRACE("pmd: 0x%08x",pmd);
		pte = pte_offset_map(pmd, virt_addr);
        SEC_HAL_TRACE("pte: 0x%08x",pte);
        SEC_HAL_TRACE("*pte: 0x%08x",*pte);

		phys_addr = (0xFFFFF000 & (*pte)) | (0x00000FFF & virt_addr);
	}

    SEC_HAL_TRACE("phys_addr: 0x%08x",phys_addr);
    SEC_HAL_TRACE_EXIT();
	return phys_addr;
}


/* load the module */
int init_mmap_memory_area(void)
{
    int i;
    unsigned long virt_addr;
    SEC_HAL_TRACE_ENTRY();

    SEC_HAL_TRACE("Not used at the moment");

    SEC_HAL_TRACE_EXIT();
    return(0);
}



void sec_hal_vma_open(struct vm_area_struct *vma)
{
    SEC_HAL_TRACE("Simple VMA open, virt %lx, phys %lx\n",
                   vma->vm_start, vma->vm_pgoff << PAGE_SHIFT);
}

void sec_hal_vma_close(struct vm_area_struct *vma)
{
    shared_memory_node *mem_node_to_remove;
    unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long phys_addr;
    unsigned long vsize = vma->vm_end - vma->vm_start;
    TEEC_SharedMemory *shmem_to_release;
    int *kmalloc_area = NULL;
    struct client_data *client = vma->vm_private_data;

/*    shmem_to_release = kmalloc(sizeof(TEEC_SharedMemory),GFP_KERNEL);*/
    shmem_to_release = kmalloc(sizeof(TEEC_SharedMemory),GFP_KERNEL);

    /* pointer to page aligned area */

    /*Tablewalk does not work because at this point virtual memory has been removed
    from page tables*/
/*    phys_addr = sec_hal_memory_tablewalk(vma->vm_start);*/

    phys_addr = off;

    kmalloc_area=phys_to_virt(phys_addr);

    int i;
    unsigned long virt_addr;


    SEC_HAL_TRACE("Simple VMA close.\n");
    SEC_HAL_TRACE("vma->vm_pgoff: 0x%08x",vma->vm_pgoff);
    SEC_HAL_TRACE("off: 0x%08x",off);
    SEC_HAL_TRACE("phys_addr: 0x%08x",phys_addr);
    SEC_HAL_TRACE("vma->vm_start: 0x%08x",vma->vm_start);
    SEC_HAL_TRACE("vma->vm_end: 0x%08x",vma->vm_end);
    SEC_HAL_TRACE("phys_to_virt(off): 0x%08x",phys_to_virt(off));

    /*TBD implement the counter so that memory is freed when all threads are
    closed*/


    /* Free the allocated memory */
    kfree(kmalloc_area);

    for (virt_addr=(unsigned long)kmalloc_area; virt_addr<(unsigned long)kmalloc_area+vsize; virt_addr+=PAGE_SIZE)
        {
        /* clear all pages to make them usable */
        ClearPageReserved(virt_to_page(virt_addr));
        }

    mem_node_to_remove = search_mem_node(&(client->shmem_list.head), vma->vm_start);

    SEC_HAL_TRACE("mem_node_to_remove 0x%x",mem_node_to_remove);
    shmem_to_release = mem_node_to_remove->shmem;
    SEC_HAL_TRACE("shmem_to_release 0x%x",shmem_to_release);

    sec_hal_tee_release_shared_memory_area(shmem_to_release);
    kfree(shmem_to_release);

    list_del(&(mem_node_to_remove->list));
    kfree(mem_node_to_remove);

}

static struct vm_operations_struct sec_hal_remap_vm_ops = {
.open = sec_hal_vma_open,
.close = sec_hal_vma_close,
};

#define MMAP_USES_KMALLOC_AREA
#ifdef MMAP_USES_KMALLOC_AREA
int sec_hal_mmap(struct file *file, struct vm_area_struct *vma)
{

    /* pointer to page aligned area */
    int *kmalloc_area = NULL;
    /* pointer to unaligned area */
    int *kmalloc_ptr = NULL;

    /* for internal memory bookkeeping */
    shared_memory_node *new_mem_node;

    struct client_data *client = file->private_data;
    struct device_data *device = client->device;

    int i;
    unsigned long virt_addr;
    unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long vsize = vma->vm_end - vma->vm_start;

    SEC_HAL_TRACE_ENTRY();

    new_mem_node = kmalloc(sizeof(shared_memory_node),GFP_KERNEL);

    /* assign the file private data to the vm private data */
	vma->vm_private_data = file->private_data;

	SEC_HAL_TRACE("client->next_teec_shmem 0x%x", client->next_teec_shmem);
    SEC_HAL_TRACE("vma->vm_pgoff: 0x%08x",vma->vm_pgoff);
    SEC_HAL_TRACE("off: 0x%08x",off);
    SEC_HAL_TRACE("vma->vm_start: 0x%08x",vma->vm_start);
    SEC_HAL_TRACE("vma->vm_end: 0x%08x",vma->vm_end);
    SEC_HAL_TRACE("PAGE_SHIFT: 0x%08x",PAGE_SHIFT);
    SEC_HAL_TRACE("vsize: %d",vsize);

#if 0
    SEC_HAL_TRACE("vma->vm_inode: 0x%08x",vma->vm_inode);
    SEC_HAL_TRACE("vma->vm_inode: 0x%08x",vma->vm_inode);
#endif

    SEC_HAL_TRACE("vma->vm_flags: 0x%08x",vma->vm_flags);



    /* get a memory area with kmalloc and aligned it to a page. This area
       will be physically contigous */
#if 0
    kmalloc_ptr=kmalloc(vsize, GFP_KERNEL);
    kmalloc_area=(int *)(((unsigned long)kmalloc_ptr + PAGE_SIZE -1) & PAGE_MASK);
    physical = virt_to_phys(kmalloc_area) + off;

    SEC_HAL_TRACE("kmalloc_area: 0x%08x",kmalloc_area);
    SEC_HAL_TRACE("phys kmalloc_area: 0x%08x",virt_to_phys((void*)((unsigned long)kmalloc_area)));
    SEC_HAL_TRACE("kmalloc_ptr: 0x%08x",kmalloc_ptr);
    SEC_HAL_TRACE("physical: 0x%08x",physical);
#endif

    kmalloc_area = phys_to_virt(client->next_teec_shmem->buffer);
    SEC_HAL_TRACE("kmalloc_area: 0x%08x",kmalloc_area);

    for (virt_addr=(unsigned long)kmalloc_area; virt_addr<(unsigned long)kmalloc_area+vsize; virt_addr+=PAGE_SIZE)
        {
        /* reserve all pages to make them remapable */
        SetPageReserved(virt_to_page(virt_addr));
        }

    SEC_HAL_TRACE("setpagereserved done");

#if 1
    for (i=0; i<(vsize/sizeof(int)); i+=2)
        {
        kmalloc_area[i]=(0xdead<<16) +i;
        kmalloc_area[i+1]=(0xbeef<<16) + i;
        }
#endif

    SEC_HAL_TRACE("deadbeef done");

#if 0
    for (i=0; i<vsize/sizeof(int); i+=1)
        {
        SEC_HAL_TRACE("0x%08x", kmalloc_area[i]);
        }
#endif
    

    /* Sets the size as size allocated in kernel not real size asked in user
    space */
    client->next_teec_shmem->size = vsize;


    if (remap_pfn_range(vma, vma->vm_start, (uint32_t)(client->next_teec_shmem->buffer) >> PAGE_SHIFT, vsize, vma->vm_page_prot))
        {
        SEC_HAL_TRACE("remap page range failed\n");
        return -ENXIO;
        }


    vma->vm_ops = &sec_hal_remap_vm_ops;

    new_mem_node->shmem = client->next_teec_shmem;
    new_mem_node->virt_addr = vma->vm_start;

    list_add(&new_mem_node->list, &(client->shmem_list.head));

    SEC_HAL_TRACE("new_mem_node 0x%x",new_mem_node);
    SEC_HAL_TRACE("new_mem_node->shmem 0x%x",new_mem_node->shmem);
    SEC_HAL_TRACE("new_mem_node->virt_addr 0x%x",new_mem_node->virt_addr);

/*    kfree(client->next_teec_shmem);*/
    client->next_teec_shmem=NULL;

	/* return the pointer */
	SEC_HAL_TRACE_EXIT();
	return(0);
}
#else
/* This is working in some ways */
int sec_hal_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long size = vma->vm_end - vma->vm_start;

    SEC_HAL_TRACE_ENTRY();
    SEC_HAL_TRACE("vma->vm_pgoff: 0x%08x",vma->vm_pgoff);
    SEC_HAL_TRACE("vma->vm_start: 0x%08x",vma->vm_start);
    SEC_HAL_TRACE("vma->vm_end: 0x%08x",vma->vm_end);
    SEC_HAL_TRACE("PAGE_SHIFT: 0x%08x",PAGE_SHIFT);
    SEC_HAL_TRACE("size: %d",size);

    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
        size,
        vma->vm_page_prot))
        {
        return -EAGAIN;
        }

    vma->vm_ops = &sec_hal_remap_vm_ops;
    sec_hal_vma_open(vma);



	/* return the pointer */
	SEC_HAL_TRACE_EXIT();
	return(0);
}
#endif


static struct file_operations k_sec_hal_fops = {
    .owner = THIS_MODULE,
    .open = &sec_hal_cli_open,
    .release = &sec_hal_cli_release,
    .unlocked_ioctl = &sec_hal_cli_ioctl,
    .read = &sec_hal_rpc_read,
    .write = &sec_hal_rpc_write,
    .llseek = &no_llseek,
    .mmap = &sec_hal_mmap,
};


/* ----------------------------------------------------------------------
 * add_attach_cdev :
 * --------------------------------------------------------------------*/
static int
add_attach_cdev(struct cdev* dev,
                struct file_operations* fops,
                struct class* cls,
                dev_t devno,
                const char* devname)
{
    SEC_HAL_TRACE_ENTRY();

    cdev_init(dev, fops);
    dev->owner = THIS_MODULE;
    if (cdev_add(dev, devno, 1)) {
        SEC_HAL_TRACE_EXIT_INFO("failed to add cdev, aborting!");
        return -ENODEV;
    }

    device_create(cls, NULL, devno, NULL, devname);

    SEC_HAL_TRACE_EXIT();
    return 0;
}

/* ----------------------------------------------------------------------
 * detach_del_cdev :
 * --------------------------------------------------------------------*/
static void
detach_del_cdev(struct cdev* dev,
                struct class* cls)
{
    SEC_HAL_TRACE_ENTRY();

    device_destroy(cls, dev->dev);
    cdev_del(dev);

    SEC_HAL_TRACE_EXIT();
}

/* ----------------------------------------------------------------------
 * sec_hal_setup_cdev_init : allocate & initialize cdev node
 * --------------------------------------------------------------------*/
static int
sec_hal_cdev_init(struct device_data *device_data)
{
    int ret = 0;
    dev_t devno = 0;
    struct class* cls;

    SEC_HAL_TRACE_ENTRY();

    if (0 > alloc_chrdev_region(&devno, 0, 1, DEVNAME)) {
        SEC_HAL_TRACE("cannot register device, aborting!");
        ret = -EIO;
        goto e1;
    }

    cls = class_create(THIS_MODULE, DEVNAME);
    if (IS_ERR(cls)) {
        SEC_HAL_TRACE("failed to class_create, aborting!");
        ret = PTR_ERR(cls);
        goto e2;
    }

    if (add_attach_cdev(&device_data->cdev, &k_sec_hal_fops, cls, devno, DEVNAME)) {
        SEC_HAL_TRACE("failed to add and attach cdev, aborting!");
        ret = -ENODEV;
        goto e2;
    }

    /* all ok, store heap data to argument struct. */
    sema_init(&device_data->sem, CONFIG_NR_CPUS);
    device_data->class = cls;
    rpcq_init(&device_data->rpc_read_waitq);
    rpcq_init(&device_data->rpc_write_waitq);

    SEC_HAL_TRACE_EXIT();
    return ret;

e2: class_destroy(cls);
e1: unregister_chrdev_region(MAJOR(devno), 1);
    SEC_HAL_TRACE_EXIT_INFO("failed to create /dev - nodes.");
    return ret;
}


/* ----------------------------------------------------------------------
 * sec_hal_cdev_exit : release cdevs related resources
 * --------------------------------------------------------------------*/
static void
sec_hal_cdev_exit(struct device_data *device_data)
{
    dev_t devno = device_data->cdev.dev;

    SEC_HAL_TRACE_ENTRY();

    detach_del_cdev(&device_data->cdev, device_data->class);
    class_destroy(device_data->class);
    unregister_chrdev_region(MAJOR(devno), 1);

    SEC_HAL_TRACE_EXIT();
}




/* **********************************************************************
 * ICRAM ALLOCATION RELATED FUNCTIONS
 * *********************************************************************/
#if (!defined(BLOCKCOUNT) && !BLOCKCOUNT)
#error !!local macro not defined, can cause div by zero exception!!
#endif
/* ----------------------------------------------------------------------
 * sec_hal_mem_msg_area_init :
 * --------------------------------------------------------------------*/
int
sec_hal_mem_msg_area_init(void)
{
    int ret = 0;
    unsigned int block_sz, index = 0;
    struct mem_msg_area *msg_area = &g_device_data.icram0;

    if (g_icram_mem_init_done == 1) {
        return ret;
    }

    if (!request_mem_region(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE), "msg area")) {
        ret = -ENODEV;
        goto e1;
    }

#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
    msg_area->virt_baseptr = kmalloc(UL(ICRAM1_SIZE), GFP_KERNEL);
#else
    msg_area->virt_baseptr = ioremap_nocache(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE));
#endif /* CONFIG_ARM_SEC_HAL_TEST_DISPATCHER */
    msg_area->phys_start = UL(ICRAM1_ADDRESS);
    msg_area->phys_size = UL(ICRAM1_SIZE);
    msg_area->offset = (unsigned long)(msg_area->virt_baseptr - msg_area->phys_start);

    if (NULL == msg_area->virt_baseptr) {
        ret = -EINVAL;
        goto e2;
    }

    block_sz = (msg_area->phys_size)/BLOCKCOUNT;

    /* initialize msg area alloc blocks */
    for (; index < BLOCKCOUNT; index++) {
        msg_area->msg_blocks[index].virt_addr = 
            ((__u8*)msg_area->virt_baseptr + block_sz*index);
        msg_area->msg_blocks[index].size = 0;
        msg_area->msg_blocks[index].allocated = FALSE;
    }

    g_icram_mem_init_done = 1;
    return ret;

e2: release_mem_region(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE));
e1: MEM_MSG_AREA_CLEAR(msg_area); /* leave mem struct as 'unallocated' */
    return ret;
}

 /* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_mem_msg_area_init);

/* ----------------------------------------------------------------------
 * sec_hal_mem_msg_area_exit :
 * --------------------------------------------------------------------*/
void
sec_hal_mem_msg_area_exit(struct mem_msg_area *msg_area)
{
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
    kfree(msg_area->virt_baseptr);
#else
    iounmap(msg_area->virt_baseptr);
#endif /* CONFIG_ARM_SEC_HAL_TEST_DISPATCHER */
    release_mem_region(msg_area->phys_start, msg_area->phys_size);
    MEM_MSG_AREA_CLEAR(msg_area); /* leave mem struct as 'unallocated' */
}


/* ----------------------------------------------------------------------
 * sec_hal_mem_msg_area_memcpy : simple memcpy
 * --------------------------------------------------------------------*/
unsigned long
sec_hal_mem_msg_area_memcpy(void *dst,
                            const void *src,
                            unsigned long sz)
{
    __u8* dst8 = (__u8*)dst;
    __u8* src8 = (__u8*)src;

    while (sz--) {
        *dst8++ = *src8++;
    }

    return (unsigned long)dst;
}

unsigned long
sec_hal_mem_msg_area_write(void *dst,
                           const void *src,
                           unsigned long sz)
{
    __u8* dst8 = (__u8*)dst;
    __u8* src8 = (__u8*)src;

    while (sz--) {
        *dst8++ = *src8++;
    }

    return (unsigned long)dst;
}

unsigned long
sec_hal_mem_msg_area_read(void *dst,
                          const void *src,
                          unsigned long sz)
{
    __u8* dst8 = (__u8*)dst;
    __u8* src8 = (__u8*)src;

    while (sz--) {
        *dst8++ =  *src8++;
    }

    return (unsigned long)dst;
}

/* ----------------------------------------------------------------------
 * sec_hal_mem_msg_area_memset : simple memset for ZI purposes.
 * --------------------------------------------------------------------*/
static inline void
sec_hal_mem_msg_area_memset(void *buff,
                            unsigned char data,
                            unsigned int cnt)
{
    __u8* ptr = (__u8*)buff;

    while (cnt > 0) {
        *ptr++ = data;
        cnt--;
    }
}

/* ----------------------------------------------------------------------
 * sec_hal_mem_msg_area_calloc : used by sec_hal_rt
 * --------------------------------------------------------------------*/
void*
sec_hal_mem_msg_area_calloc(unsigned int n,
                            unsigned int sz)
{
    unsigned int block_sz, block_cnt, block_ind, index = 0;
    int found = FALSE;
    void* virt_addr = NULL;
    struct mem_msg_area* ramb = &g_device_data.icram0;

    if (g_icram_mem_init_done == 0) {
        sec_hal_mem_msg_area_init();
    }

    if (0 == (n*sz)) {
        return NULL;
    }

    block_sz = (ramb->phys_size)/BLOCKCOUNT;
    block_cnt = ((n*sz)+block_sz-1)/block_sz;

    if (block_cnt > BLOCKCOUNT) {
        return NULL;
    }

    spin_lock(&g_dev_spinlock); /* protect 'ramb' access */
    /* critical section starting, do not 'call' anything that may sleep.*/

    /* seek big enough unallocated mem area */
    while (index < BLOCKCOUNT) {
        if (FALSE == ramb->msg_blocks[index].allocated) {
            found = TRUE;
            block_ind = block_cnt - 1;

            while (block_ind > 0 && (index+block_ind) < BLOCKCOUNT) {
                found = (found && FALSE == ramb->msg_blocks[index+block_ind].allocated);
                block_ind--;
            }
            if (TRUE == found){break;} /* check if the loop can be ended */
        }
        index++;
    }

    /* return ptr to first block, update allocation info & zero initialize */
    if (TRUE == found && index < BLOCKCOUNT) {
        /* allocated found message area */
        virt_addr = ramb->msg_blocks[index].virt_addr;
        ramb->msg_blocks[index].size = block_cnt;
        ramb->msg_blocks[index].allocated = TRUE;
        sec_hal_mem_msg_area_memset(virt_addr, 0, block_sz);
        block_cnt--;
    }

    /* also update allocation info for rest of the blocks & zero initialize */
    while (TRUE == found && block_cnt > 0 && (index+block_cnt) < BLOCKCOUNT) {
        ramb->msg_blocks[index+block_cnt].allocated = TRUE;
        sec_hal_mem_msg_area_memset(
                ramb->msg_blocks[index+block_cnt].virt_addr,
                0, block_sz);
        block_cnt--;
    }

    /* critical section ending. */
    spin_unlock(&g_dev_spinlock);

    return virt_addr; /* return allocated(or not) memory address */
}

/* ----------------------------------------------------------------------
 * sec_hal_mem_msg_area_free : used by sec_hal_rt
 * --------------------------------------------------------------------*/
void
sec_hal_mem_msg_area_free(void *virt_addr)
{
    unsigned int block_ind, index = 0;
    struct mem_msg_area* ramb = &g_device_data.icram0;

    if (NULL == virt_addr) {
        return;
    }

    spin_lock(&g_dev_spinlock); /* protect 'ramb' access */
    /* critical section starting, do not 'call' anything that may sleep.*/

    while (index < BLOCKCOUNT) {
        if (ramb->msg_blocks[index].virt_addr == virt_addr) {
            block_ind = ramb->msg_blocks[index].size - 1;

            /* free allocated message area */
            ramb->msg_blocks[index].size = 0;
            ramb->msg_blocks[index].allocated = FALSE;

            /* free rest of the blocks */
            while (0 < block_ind && (index+block_ind) < BLOCKCOUNT) {
                ramb->msg_blocks[index+block_ind].size = 0;
                ramb->msg_blocks[index+block_ind].allocated = FALSE;
                block_ind--;
            }
            break; /* terminate the seek-loop */
        }
        index++;/* seek next mem block */
    }

    /* critical section ending. */
    spin_unlock(&g_dev_spinlock);
}

/* ----------------------------------------------------------------------
 * 
 * --------------------------------------------------------------------*/
unsigned long sec_hal_virt_to_icram_phys(unsigned long virt_addr)
{
    unsigned long phys_addr;
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
    phys_addr = virt_to_phys(virt_addr);
#else
    phys_addr = virt_addr - g_device_data.icram0.offset;
#endif
    return phys_addr;
}

/* ----------------------------------------------------------------------
 * 
 * --------------------------------------------------------------------*/
unsigned long sec_hal_icram_phys_to_virt(unsigned long phys_addr)
{
    unsigned long virt_addr;
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
    virt_addr = phys_to_virt(phys_addr);
#else
    virt_addr = phys_addr + g_device_data.icram0.offset;
#endif
    return virt_addr;
}


/* **********************************************************************
** Function name      : sec_hal_dbg_irq_hdr
** Description        : IRQ handler for JTAG/hw debugger attach event.
**                      SJTAG not supported by this function.
** Return             : IRQ_HANDLED
** *********************************************************************/
static irqreturn_t
sec_hal_dbg_irq_hdr(
		int irq,
		void *dev_id)
{
	uint32_t ret;
	uint32_t reg1 = g_dbgreg1, reg2 = g_dbgreg2, reg3 = g_dbgreg3;

	ret = sec_hal_dbg_reg_set(&reg1, &reg2, &reg3);
	if (SEC_HAL_RES_OK == ret) {
		if (reg1 == g_dbgreg1)
			g_dbgreg1 = reg1;
		if (reg2 == g_dbgreg2)
			g_dbgreg2 = reg2;
		if (reg3 == g_dbgreg3)
			g_dbgreg3 = reg3;
	}

	/* Disable interrupt of TDBG because it requires only ONCE. */
	disable_irq_nosync(irq);
	return IRQ_HANDLED;
}

/* **********************************************************************
** Function name      : sec_hal_dbg_irq_init
** Description        : initializes IRQ handler for TDBG, IRQ should be
**                      triggered by JTAG/hw debugger attach event.
**                      SJTAG not supported by this function.
** *********************************************************************/
static void
sec_hal_dbg_irq_init(void)
{
	int rv;
	uint32_t irq, flags = IRQF_DISABLED;

    SEC_HAL_TRACE_ENTRY();

	irq = gic_spi(TDBG_SPI);
	set_irq_flags(irq, IRQF_VALID|IRQF_NOAUTOEN);
	rv = request_irq(irq, &sec_hal_dbg_irq_hdr, flags,
			"DBG_LINKUP_REQ", (void*)irq);
	if (rv < 0)
		free_irq(irq, (void*)irq);

	enable_irq(irq);

	SEC_HAL_TRACE_EXIT_INFO("rv: %d", rv);
}



/* **********************************************************************
 * PLATFORM DEVICE FRAMEWORK RELATED FUNCTIONS.
 * *********************************************************************/
/* ----------------------------------------------------------------------
 * sec_hal_platform_device_probe :
 * --------------------------------------------------------------------*/
static int
sec_hal_platform_device_probe(struct platform_device *plat_dev)
{
	int ret = 0;
	struct resource* mem = NULL;
	sec_hal_init_info_t rt_init;

	SEC_HAL_TRACE_ENTRY();

	if (NULL == (mem = platform_get_resource(
					plat_dev, IORESOURCE_MEM, 0))) {
		SEC_HAL_TRACE_EXIT_INFO("faulty arguments, aborting!");
		return -ENOMEM;
	}

	ret = sec_hal_mem_msg_area_init();
	if (ret)
		goto e0;

	ret = sec_hal_rt_init(&rt_init);
	if (ret)
		goto e1;

	printk(KERN_INFO "TRUSTZONE VERSION: 0x%08x%08x [0x%X, 0x%X, 0x%X]\n",
			(uint32_t)(rt_init.commit_id >> 32),
			(uint32_t)(rt_init.commit_id),
			rt_init.reset_info[0], rt_init.reset_info[1], rt_init.reset_info[2]);

	ret = sec_hal_rt_install_rpc_handler(&rpc_handler);
	if (ret) {
		goto e1;
	}

#if 0
	printk(KERN_INFO "sec_hal_rt_install_rpc_handler was ok\n");
#endif

	if (!is_recovery()) {
#if 0
		printk(KERN_INFO "!is_recovery()\n");
#endif
		ret = sec_hal_memfw_attr_reserve(0, 0x00UL, 0x00UL);
		if (ret){
			goto e1;
		}
#if 0
	printk(KERN_INFO "sec_hal_memfw_attr_reserve was ok\n");
#endif
	}


#if 0
	printk(KERN_INFO "trying sec_hal_cdev_init\n");
#endif
	ret = sec_hal_cdev_init(&g_device_data);
	if (ret){
		goto e2;
	}
#if 0
	printk(KERN_INFO "sec_hal_cdev_init was ok\n");
#endif

	ret = sec_hal_reset_info_addr_register();
	if (ret){
		goto e2;
	}
#if 0
	printk(KERN_INFO "sec_hal_reset_info_addr_register was ok\n");
#endif
    /*sdtoc_init*/
    sdtoc_root = (DATA_TOC_ENTRY *) ioremap_nocache(SDTOC_ADDRESS, SDTOC_SIZE);

    /* ask production mode from sdtoc we can decide if we need to use public to
    for imei and other customer data*/

#ifdef CONFIG_ARM_SEC_HAL_PUBLIC_TOC
    /*toc_init*/
    public_toc_root = kmalloc(PUBLIC_TOC_SIZE, GFP_KERNEL);

    SEC_HAL_TRACE("public_toc_root: 0x%08x",public_toc_root);

    toc_initialize(public_toc_root,PUBLIC_TOC_SIZE);

    toc_put_payload(public_toc_root,0x12341234,"abcdefghijklmnopqstuv",20,NULL,0,0);
    toc_put_payload(public_toc_root,0xffeeddcc,"1111111166666666",16,NULL,0,0);

    if(1)
    {
    uint32_t length;
    void * item_data;

    item_data = toc_get_payload(public_toc_root,0x12341234, &length);
    SEC_HAL_TRACE("item_data addr: 0x%08x",item_data);

    item_data = toc_get_payload(public_toc_root,0xffeeddcc, &length);
    SEC_HAL_TRACE("item_data addr: 0x%08x",item_data);
    }

    toc_put_payload(public_toc_root,SECURED_DATA_PUBLIC_ID,"eeeeeeeeaaaaaaaa1111111166666666",32,NULL,0,0);
#endif

    public_id = toc_get_payload(sdtoc_root,SECURED_DATA_PUBLIC_ID, &public_id_size);

    if(public_id!=NULL)
        {
        uint8_t* tmp;
        uint32_t i;

        tmp = public_id;
        printk("Device public ID (length %d): ",public_id_size);

        for(i=0;i<public_id_size;i++ )
            {
            printk("%02x",*tmp);
            tmp++;
            }
        printk("\n");
        }
    else
        {
        printk("Could not read Device public ID\n");
        }


	SEC_HAL_TRACE_EXIT();
	return ret;
e2:	sec_hal_cdev_exit(&g_device_data);
e1:
e0:
	SEC_HAL_TRACE_EXIT_INFO("failed in device init");
	return ret;
}

/* ----------------------------------------------------------------------
 * sec_hal_platform_device_remove :
 * --------------------------------------------------------------------*/
static int
sec_hal_platform_device_remove(struct platform_device *plat_dev)
{
    SEC_HAL_TRACE_ENTRY();

    sec_hal_cdev_exit(&g_device_data);
    sec_hal_mem_msg_area_exit(&g_device_data.icram0);

    SEC_HAL_TRACE_EXIT();
    return 0;
}

/* ----------------------------------------------------------------------
 * ---------------------------------------------------------------------*/
static struct platform_driver k_sec_hal_platform_device_driver =
{
    .probe    = sec_hal_platform_device_probe,
    .remove   = sec_hal_platform_device_remove,
    .driver   = {.name  = DEVNAME, .owner = THIS_MODULE},
};




/* **********************************************************************
 * EXPORTs, Modem Boot
 * *********************************************************************/
/* ----------------------------------------------------------------------
 * sec_hal_mdm_memcpy : copy data to a certain kind of protected memory.
 * --------------------------------------------------------------------*/
uint32_t
sec_hal_mdm_memcpy(uint32_t phys_dst,
                   uint32_t phys_src,
                   uint32_t size)
{
    return sec_hal_memcpy(phys_dst, phys_src, size);
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_mdm_memcpy);

/* ----------------------------------------------------------------------
 * sec_hal_mdm_authenticate : authenticate memory content with SW cert.
 * --------------------------------------------------------------------*/
uint32_t
sec_hal_mdm_authenticate(uint32_t cert_phys_addr,
                         uint32_t cert_size,
                         uint32_t *objid)
{
    return sec_hal_authenticate(cert_phys_addr, cert_size, objid);
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_mdm_authenticate);


/* **********************************************************************
 * MODULE INIT & EXIT
 * *********************************************************************/
static struct resource k_sec_hal_resources[] =
{
    [0] =
        { /* ICRAM0 */
        .start = UL(ICRAM1_ADDRESS),
        .end = UL(ICRAM1_ADDRESS) + UL(ICRAM1_SIZE) - 1,/* 4kb from start addr */
        .flags = IORESOURCE_MEM,
        }
};
static const u64 k_dma_mask = 0xffffffff;
static struct platform_device k_sec_hal_chardevice =
{
    .name =  DEVNAME,
    .id = 0,
    .dev = {
        .coherent_dma_mask = 0xffffffff,
        .dma_mask = &k_dma_mask
    },
    .resource = k_sec_hal_resources,
    .num_resources = ARRAY_SIZE(k_sec_hal_resources),
};
static struct platform_device *k_sec_hal_local_devs[] __initdata =
{
    &k_sec_hal_chardevice,
};

/* ----------------------------------------------------------------------
 * sec_hal_driver_init :
 * --------------------------------------------------------------------*/
static int __init
sec_hal_driver_init(void)
{
    int ret = 0;

    SEC_HAL_TRACE_INIT();
    SEC_HAL_TRACE_ENTRY();

    ret = platform_driver_register(&k_sec_hal_platform_device_driver);
    platform_add_devices(k_sec_hal_local_devs, ARRAY_SIZE(k_sec_hal_local_devs));

    SEC_HAL_TRACE_EXIT();
    return ret;
}

/* ----------------------------------------------------------------------
 * sec_hal_driver_exit :
 * --------------------------------------------------------------------*/
static void __exit
sec_hal_driver_exit(void)
{
    SEC_HAL_TRACE_ENTRY();

    platform_driver_unregister(&k_sec_hal_platform_device_driver);

    SEC_HAL_TRACE_EXIT();
}
/* PM start */
/* module_init(sec_hal_driver_init); */
module_exit(sec_hal_driver_exit);
pure_initcall(sec_hal_driver_init);
/* PM end */

MODULE_AUTHOR("Renesas Mobile Corporation");
MODULE_DESCRIPTION("Device driver for ARM TRUSTZONE access");
MODULE_LICENSE("Proprietary");

