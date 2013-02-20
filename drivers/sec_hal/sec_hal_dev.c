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
** MACRO(s)
** *******************************************************************/

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
uint32_t sec_hal_reset_info_addr_register(void);
#ifdef SEC_STORAGE_SELFTEST_ENABLE
static uint32_t rpc_handler(uint32_t id, uint32_t p1, uint32_t p2, uint32_t p3, uint32_t p4);
#endif
int workaround_done = 0;




/* **********************************************************************
** STRUCT(s)
** *********************************************************************/
struct mem_alloc_info {
	void* virt_addr; /* the start address of the message area */
	uint8_t size;/* the size of allocation */
	uint8_t allocated; /* status of the block */
};

#define BLOCKCOUNT 128
struct mem_msg_area {
	void* virt_baseptr; /* stores ioremap output */
	unsigned long phys_start; /* phys start addr */
	unsigned long phys_size; /* phys size */
	unsigned long offset; /* offset between physical and virtual addresses */
	/* blocks to-be-allocated for out & in msgs */
	struct mem_alloc_info msg_blocks[BLOCKCOUNT];
};

typedef struct {
	struct list_head list;
	void *virt_addr; /* virtual address */
	TEEC_SharedMemory *shmem;
} shared_memory_node;

typedef struct {
	struct list_head head;
} shared_memory_list;

struct device_data {
	struct class* class;
	struct cdev cdev;
	struct semaphore sem;
	unsigned int wdt_upd; /* initial wdt value */
	struct mem_msg_area icram0; /* memory (icram0) allocation information */
	struct platform_device *pdev;
};

struct client_data {
	struct device_data *device;
	void *drm_data; /* if non NULL then owned. */
	TEEC_Context *teec_context;
	TEEC_SharedMemory *next_teec_shmem;
	void * next_teec_shmem_buffer;
	shared_memory_list shmem_list;
};


/* RunTime functions decl to higher level.(legacy stuff) */
long sec_hal_rt_ioctl(unsigned int cmd, void **data, sd_ioctl_params_t *param);


/* RPC functions decl to higher level */
ssize_t sec_hal_rpc_read(struct file *filp, char __user* buf,
		size_t count, loff_t *ppos);
ssize_t sec_hal_rpc_write(struct file *filp, const char *buf,
		size_t count, loff_t *ppos);
long sec_hal_rpc_ioctl(unsigned int cmd, void **data, sd_ioctl_params_t *param);


/* DRM functions decl to higher level */
void sec_hal_drm_usr_exit(void *drm_data);
long sec_hal_drm_ioctl(unsigned int cmd, void **data, sd_ioctl_params_t *param,
		struct platform_device *pdev);


/* CNF functions decl to higher level */
long sec_hal_cnf_ioctl(unsigned int cmd, void **data, sd_ioctl_params_t *param,
		struct platform_device *pdev);

/* **********************************************************************
** W/A(s)
** *********************************************************************/
static inline
int is_recovery(void)
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
#define MEM_MSG_AREA_CLEAR(ptr) mem_msg_area_clear(ptr)

static
int search_mem_node(struct list_head *lst, void *virt_addr)
{
	struct list_head *pos;
	shared_memory_node *item;
	shared_memory_node *node_to_return = NULL;

	SEC_HAL_TRACE_ENTRY();
	SEC_HAL_TRACE("virt_addr: 0x%08x",virt_addr);

	list_for_each(pos, lst) {
		item = list_entry(pos, shared_memory_node, list);
		SEC_HAL_TRACE("item: 0x%08x",item);
		SEC_HAL_TRACE("item->virt_addr: 0x%08x",item->virt_addr);
		if (item->virt_addr == virt_addr) {
			SEC_HAL_TRACE("found node");
			node_to_return = item;
			break;
		}
	}

	SEC_HAL_TRACE_EXIT();
	return node_to_return;
}



/* **********************************************************************
** STATIC writable data
** *********************************************************************/
static struct device_data g_device_data =
{
	.wdt_upd = DEFAULT_WDT_VALUE,
	.icram0 = {.virt_baseptr = NULL},
};
static DEFINE_SPINLOCK(g_dev_spinlock); /* to protect above data struct */
/* initial DBGREG values, for Rnd.
 * Eventually should be read from general purpose register
 * which are accessible by hw debuggers. */
#define TDBG_SPI 81U
static uint32_t g_dbgreg1 = 0x20000000;
static uint32_t g_dbgreg2 = 0x00;
static uint32_t g_dbgreg3 = 0x00078077;
static int g_icram_mem_init_done = 0;
DATA_TOC_ENTRY *sdtoc_root;
DATA_TOC_ENTRY *public_toc_root;
uint8_t *public_id;
uint8_t *prcf;
uint32_t public_id_size;



/* **********************************************************************
** USR space access
** *********************************************************************/
static
int sec_hal_usr_open(struct inode *inode, struct file *filp)
{
	struct device_data *device;
	struct client_data *client;

	SEC_HAL_TRACE_ENTRY();

	device = container_of(inode->i_cdev, struct device_data, cdev);
	if (!device)
		return -ENODEV;

	if (nonseekable_open(inode, filp))
		return -ENODEV;

	client = kmalloc(sizeof(struct client_data), GFP_KERNEL);
	if (!client)
		return -ENODEV;

	client->device = device;
	client->drm_data = NULL;

	client->teec_context = NULL;
	client->next_teec_shmem = NULL;
	client->next_teec_shmem_buffer = NULL;
	INIT_LIST_HEAD(&(client->shmem_list.head));

	filp->private_data = client;

	SEC_HAL_TRACE_EXIT();
	return 0;
}

static
int sec_hal_usr_release(struct inode *inode, struct file *filp)
{
	struct client_data *client = filp->private_data;

	SEC_HAL_TRACE_ENTRY();

#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
	sec_hal_drm_usr_exit(client->drm_data); /* abnormal exit usecase */
	client->drm_data = NULL;
#endif
	/* In case of crash client mem_nodes need to be freed here */
	kfree(client);

	SEC_HAL_TRACE_EXIT();
	return 0;
}


static
long sec_hal_usr_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	long ret = (long) -EPERM;
	struct client_data *client = filp->private_data;
	struct device_data *device = client->device;
	sd_ioctl_params_t input;

	if (filp->f_flags & O_NONBLOCK)
		return -EPERM;

	if (copy_from_user(&input, (void __user*)arg, SD_IOCTL_PARAMS_SZ))
		return -EFAULT;

	if (down_interruptible(&device->sem))
		return -ERESTARTSYS;

	switch (cmd) {
	case SD_INIT:
	case SD_EXIT:
		ret = 0;
		break;
	case SD_KEY_INFO:
	case SD_CERT_REGISTER:
	case SD_DATA_CERT_REGISTER:
	case SD_MAC_ADDRESS_GET:
	case SD_IMEI_GET:
	case SD_RAT_BAND_GET:
	case SD_PP_FLAGS_COUNT_GET:
	case SD_PP_FLAGS_GET:
	case SD_SL_LEVELS_OPEN:
	case SD_SL_LEVEL_OPEN:
	case SD_SL_LEVEL_STATUS_GET:
	case SD_AUTH_DATA_SIZE_GET:
	case SD_AUTH_DATA_GET:
	case SD_SELFTEST:
		ret = sec_hal_rt_ioctl(cmd, NULL, &input);
		break;
#ifdef CONFIG_ARM_SEC_HAL_DRM_WVN
	case SD_DRM_ENTER_PLAY:
	case SD_DRM_EXIT_PLAY:
	case SD_DRM_SET_ENTIT_KEY:
	case SD_DRM_DER_CTL_WORD:
	case SD_DRM_DECRYPT_AUDIO:
	case SD_DRM_DECRYPT_VIDEO:
	case SD_DRM_VALID_KEYBOX:
	case SD_DRM_DEVICE_ID_GET:
	case SD_DRM_KEYDATA_GET:
	case SD_RANDOM_NUMBER_GET:
	case SD_DRM_WRAP_KEYBOX:
	case SD_DRM_INSTALL_KEYBOX:
		ret = sec_hal_drm_ioctl(cmd, &client->drm_data, &input, device->pdev);
		break;
#endif /* CONFIG_ARM_SEC_HAL_DRM_WVN */
	case SD_SEC_STORAGE_SELFTEST:
	case SD_SIMU_DS0_TEST:
	case SD_SIMU_DS1_TEST:
	case SD_SECURE_STORAGE_DAEMON_PID_REGISTER:
		ret = sec_hal_rpc_ioctl(cmd, NULL, &input);
		break;
	case SD_CNF_SIML:
	case SD_CNF_SIML_VALIDATE:
	case SD_CNF_SIML_VERIFY_DATA_GET:
	case SD_CNF_IMEI:
	case SD_CNF_MAC:
		ret = sec_hal_cnf_ioctl(cmd, NULL, &input, device->pdev);
		break;
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
        default:{SEC_HAL_TRACE("DEFAULT!");}break;
    }

	up(&device->sem);

	return ret; /* directly return API definitions */
}



/*******************************************************************************
 * Function   : sec_hal_memory_tablewalk
 * Description: This function translates the virtual address into the physical address.
 * Parameters : virt_addr	   - virtual address
 * Returns	  : phys_addr	   - physical address
 *******************************************************************************/
unsigned long sec_hal_memory_tablewalk(unsigned long virt_addr)
{
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long page_num;
	unsigned long phys_addr;
	unsigned long *page_addr;

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
    .open = &sec_hal_usr_open,
    .release = &sec_hal_usr_release,
    .unlocked_ioctl = &sec_hal_usr_ioctl,
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
** Function name      : sec_hal_dbg_irq_hdr
** Description        : IRQ handler for JTAG/hw debugger attach event.
**                      SJTAG not supported by this function.
** Return             : IRQ_HANDLED
** *********************************************************************/
static
irqreturn_t sec_hal_dbg_irq_hdr(int irq, void *dev_id)
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
static
void sec_hal_dbg_irq_init(void)
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



int sec_hal_icram0_init(void);
int sec_hal_rpc_init(void);
/* **********************************************************************
 * PLATFORM DEVICE FRAMEWORK RELATED FUNCTIONS.
 * *********************************************************************/
static
int sec_hal_pdev_probe(struct platform_device *pdev)
{
	int ret;
	struct resource* mem;
	sec_hal_init_info_t rt_init;

	SEC_HAL_TRACE_ENTRY();

	if (NULL == (mem = platform_get_resource(
					pdev, IORESOURCE_MEM, 0))) {
		SEC_HAL_TRACE_EXIT_INFO("faulty arguments, aborting!");
		return -ENOMEM;
	}

	ret = sec_hal_icram0_init();
	if (ret)
		goto e0;

	ret = sec_hal_rpc_init();
	if (ret)
		goto e0;

	ret = sec_hal_rt_init(&rt_init);
	if (ret)
		goto e1;

	printk(KERN_INFO "TRUSTZONE VERSION: 0x%08x%08x [0x%X, 0x%X, 0x%X]\n",
			(uint32_t)(rt_init.commit_id >> 32),
			(uint32_t)(rt_init.commit_id),
			rt_init.reset_info[0], rt_init.reset_info[1], rt_init.reset_info[2]);

#if 0
	ret = sec_hal_rt_install_rpc_handler(&sec_hal_rpc_cb);
	if (ret)
		goto e1;
#endif

	if (!is_recovery()) {
		ret = sec_hal_memfw_attr_reserve(0, 0x00UL, 0x00UL);
		if (ret)
			goto e1;
	}
	sec_hal_dbg_irq_init();

	ret = sec_hal_cdev_init(&g_device_data);
	if (ret)
		goto e2;

	ret = sec_hal_reset_info_addr_register();
	if (ret)
		goto e2;

#ifdef CONFIG_ARM_SEC_HAL_SDTOC
	/* sdtoc_init */
	sdtoc_root = (DATA_TOC_ENTRY *) ioremap_nocache(SDTOC_ADDRESS, SDTOC_SIZE);

	/* ask production mode from sdtoc we can decide if we need to use public to
	 * for imei and other customer data. */
#ifdef CONFIG_ARM_SEC_HAL_PUBLIC_TOC
	/*toc_init*/
	public_toc_root = kmalloc(PUBLIC_TOC_SIZE, GFP_KERNEL);
	SEC_HAL_TRACE("public_toc_root: 0x%08x",public_toc_root);
	toc_initialize(public_toc_root,PUBLIC_TOC_SIZE);

	toc_put_payload(public_toc_root,0x12341234,"abcdefghijklmnopqstuv",20,NULL,0,0);
	toc_put_payload(public_toc_root,0xffeeddcc,"1111111166666666",16,NULL,0,0);

	uint32_t length;
	void *item_data;
	item_data = toc_get_payload(public_toc_root,0x12341234, &length);
	SEC_HAL_TRACE("item_data addr: 0x%08x",item_data);
	item_data = toc_get_payload(public_toc_root,0xffeeddcc, &length);
	SEC_HAL_TRACE("item_data addr: 0x%08x",item_data);

	toc_put_payload(public_toc_root,SECURED_DATA_PUBLIC_ID,"eeeeeeeeaaaaaaaa1111111166666666",32,NULL,0,0);
#endif
	public_id = toc_get_payload(sdtoc_root,SECURED_DATA_PUBLIC_ID, &public_id_size);
	if (public_id != NULL) {
		printk("DEVICE PUBLIC ID (length %d): ",public_id_size);
		uint8_t* tmp = public_id;
		int i;
		for (i=0; i < public_id_size; i++) {
			printk("%02x",*tmp);
			tmp++;
		}
		printk("\n");
	} else {
		printk("DEVICE PUBLIC ID could not be read!\n");
	}
#else /*CONFIG_ARM_SEC_HAL_SDTOC*/
	sec_hal_public_id_t pub_id;
	ret = sec_hal_public_id_get(&pub_id);
	if (ret)
		goto e2;
	printk("DEVICE PUBLIC ID: ");
	int i;
	for (i=0; i < SEC_HAL_MAX_PUBLIC_ID_LENGTH; i++) {
		printk("%02x", pub_id.public_id[i]);
	}
	printk("\n");
#endif /*CONFIG_ARM_SEC_HAL_SDTOC*/

	SEC_HAL_TRACE_EXIT();
	return ret;
e2:	sec_hal_cdev_exit(&g_device_data);
e1:
e0:
	SEC_HAL_TRACE_EXIT_INFO("failed in device init");
	return ret;
}


static
int sec_hal_pdev_remove(struct platform_device *pdev)
{
	SEC_HAL_TRACE_ENTRY();

	sec_hal_cdev_exit(&g_device_data);
	//sec_hal_mem_msg_area_exit(&g_device_data.icram0);

	SEC_HAL_TRACE_EXIT();
	return 0;
}


static struct platform_driver k_sec_hal_platform_device_driver =
{
	.probe = sec_hal_pdev_probe,
	.remove = sec_hal_pdev_remove,
	.driver = {.name  = DEVNAME, .owner = THIS_MODULE},
};


/* **********************************************************************
 * EXPORTs, Modem Boot
 * *********************************************************************/
/* ----------------------------------------------------------------------
 * sec_hal_mdm_memcpy : copy data to a certain kind of protected memory.
 * --------------------------------------------------------------------*/
uint32_t sec_hal_mdm_memcpy(uint32_t pdst, uint32_t psrc, uint32_t sz)
{
	return sec_hal_memcpy(pdst, psrc, sz);
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_mdm_memcpy);

/* ----------------------------------------------------------------------
 * sec_hal_mdm_authenticate : authenticate memory content with SW cert.
 * --------------------------------------------------------------------*/
uint32_t sec_hal_mdm_authenticate(uint32_t c_paddr, uint32_t c_sz,
		uint32_t *objid)
{
	return sec_hal_authenticate(c_paddr, c_sz, objid);
}
/* made available for other kernel entities */
EXPORT_SYMBOL(sec_hal_mdm_authenticate);


/* **********************************************************************
 * MODULE init & exit
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
static
int __init sec_hal_driver_init(void)
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
static
void __exit sec_hal_driver_exit(void)
{
	SEC_HAL_TRACE_ENTRY();

	platform_driver_unregister(&k_sec_hal_platform_device_driver);

	SEC_HAL_TRACE_EXIT();
}
/* module_init(sec_hal_driver_init); */
module_exit(sec_hal_driver_exit);
pure_initcall(sec_hal_driver_init);

MODULE_AUTHOR("Renesas Mobile Corporation");
MODULE_DESCRIPTION("Device driver for ARM TRUSTZONE access");
MODULE_LICENSE("Proprietary");

