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
#include "sec_hal_rt.h"
#include "sec_hal_rt_cmn.h" /* TBDL: consider removing, should not be visible here. */
#include "sec_msg.h" /* TBDL: consider removing, should not be visible here. */
#include "sec_hal_rt_trace.h"
#include "sec_hal_dev_ioctl.h"
#include "sec_hal_dev_info.h"
#include "sec_hal_mdm.h"
#include "sec_hal_pm.h"
#include "sec_hal_sdtoc.h"
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
#include <linux/interrupt.h> /* This can be removed later, test for memory protection interrupt */
#include <asm/io.h>




/* ****************************************************************************
 * LOCAL MACROS, static constants.
 * ***************************************************************************/
#define FALSE 0
#define TRUE  1
#define DEFAULT_WDT_VALUE 60000
#define CONFIG_ARM_SEC_HAL_BUILTIN_PLATDEV 1
#define ICRAM1_SIZE 4096
#define ICRAM1_ADDRESS 0xE63A0000

void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz);

const char* k_module_name;

/* ****************************************************************************
 * MEM ALLOC information, defined here so that one spinlock can protect all.
 * ***************************************************************************/
struct mem_alloc_info
{
	void* virt_addr;	/* the start address of the message area */
	uint8_t size;		/* the size of allocation */
	uint8_t allocated;	/* status of the block */
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

static inline void mem_msg_area_clear(struct mem_msg_area *ptr)
{
	int i = 0;
	if(ptr)
	{
		ptr->virt_baseptr = NULL;
		ptr->phys_start = 0;
		ptr->phys_size = 0;
		for(; i < BLOCKCOUNT; i++)
		{
			ptr->msg_blocks[i].virt_addr = NULL;
			ptr->msg_blocks[i].size = 0;
			ptr->msg_blocks[i].allocated = FALSE;
		}
	}
}
#define MEM_MSG_AREA_CLEAR(ptr) mem_msg_area_clear(ptr)




#ifdef CONFIG_ARM_SEC_HAL_RPC_HANDLER
/* ****************************************************************************
 * RPC related data structures
 * ***************************************************************************/
typedef struct
{
	struct list_head list;
	sd_ioctl_params_t param;
} rpc_queue_item_t;
typedef struct
{
	struct list_head list_head;
	wait_queue_head_t waitq;
} rpc_queue_t;
static int rpcq_init(
		rpc_queue_t* q)
{
	INIT_LIST_HEAD(&q->list_head);
	init_waitqueue_head(&q->waitq);
	return 0;
}

/* ****************************************************************************
 * ***************************************************************************/
static int _rpcq_add(
		struct list_head* lst,
		sd_ioctl_params_t* param_in)
{
	rpc_queue_item_t* new = kmalloc(sizeof(rpc_queue_item_t), GFP_KERNEL);
	if(!new){ return -ENOMEM; }

	new->param = *param_in;
	new->param.reserved2 = (unsigned int) current->tgid;

	list_add(&new->list, lst);
	return 0;
}
static int rpcq_add_wakeup(
		rpc_queue_t* q,
		sd_ioctl_params_t* param_in)
{
	_rpcq_add(&q->list_head, param_in);
	wake_up_interruptible(&q->waitq);
	return 0;
}

/* ****************************************************************************
 * ***************************************************************************/
static int _rpcq_get(
		struct list_head* lst,
		sd_ioctl_params_t* param_out)
{
	unsigned int tgid = (unsigned int)current->tgid;
	struct list_head *pos;
	rpc_queue_item_t *item;

	list_for_each(pos, lst)
	{
		item = list_entry(pos, rpc_queue_item_t, list);
		if (item->param.reserved2 == tgid)
		{
			*param_out = item->param;
			list_del(&item->list);
			kfree(item);
			return 1;
		}
	}

	return 0;
}
static int rpcq_get_wait(
		rpc_queue_t* q,
		sd_ioctl_params_t* item)
{
	return wait_event_interruptible(q->waitq, _rpcq_get(&q->list_head, item));
}
#endif /* CONFIG_ARM_SEC_HAL_RPC_HANDLER */


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
} g_cli_dev_data =
{
	.wdt_upd = DEFAULT_WDT_VALUE,
	.icram0 = {.virt_baseptr = NULL},
};
static DEFINE_SPINLOCK(g_dev_spinlock); /* to protect above data struct */
SEC_HAL_TRACE_DEF_GLOBALS; /* for tracing, used only with debug builds. */

g_icram_mem_init_done = 0;


/* ****************************************************************************
 * CLIENT INTERFACE RELATED FUNCTIONS.
 * ***************************************************************************/
/* ----------------------------------------------------------------------------
 * sec_hal_cli_open : allocate handle specific resources
 * --------------------------------------------------------------------------*/
static int sec_hal_cli_open(
		struct inode *inode,
		struct file *filp)
{
	struct cli_dev_data* dev;

	SEC_HAL_TRACE_ENTRY
	
	dev = container_of(inode->i_cdev, struct cli_dev_data, cdev);
	if(NULL == dev)
	{
		SEC_HAL_TRACE_EXIT_INFO("failed to find cdev from container, aborting!")
		return -ENODEV;
	}
	if(nonseekable_open(inode, filp))
	{
		SEC_HAL_TRACE_EXIT_INFO("failed set node as 'nonseekable', aborting!")
		return -ENODEV;
	}

	filp->private_data = dev;

	SEC_HAL_TRACE_EXIT
	return 0;
}
/* ----------------------------------------------------------------------------
 * sec_hal_cli_release :
 * --------------------------------------------------------------------------*/
static int sec_hal_cli_release(
		struct inode *inode,
		struct file *filp)
{
	SEC_HAL_TRACE_ENTRY
	SEC_HAL_TRACE_EXIT
	return 0;
}

static const int k_param_sz = sizeof(sd_ioctl_params_t);
/* ----------------------------------------------------------------------------
 * sec_hal_cli_ioctl : IO control, make bi-directional sync operations.
 * --------------------------------------------------------------------------*/
static long sec_hal_cli_ioctl(
		struct file *filp,
		unsigned int cmd,
		unsigned long arg)
{
	long ret = (long) -EPERM;
	struct cli_dev_data* dev = filp->private_data;
	sd_ioctl_params_t input;

	SEC_HAL_TRACE_ENTRY

	if(filp->f_flags & O_NONBLOCK)
	{	
		SEC_HAL_TRACE_EXIT_INFO("non-blocking IO not supported, aborting!")
		return -EPERM;
	}
	if(copy_from_user(&input, (void __user*)arg, k_param_sz))
	{
		SEC_HAL_TRACE_EXIT_INFO("failed to get args from user, aborting!")
		return -EFAULT;
	}

	if(down_interruptible(&dev->sem))
	{
		SEC_HAL_TRACE_EXIT_INFO("interrupted, restart syscall")
		return -ERESTARTSYS;
	}

	switch(cmd)
	{
		case SD_INIT:
		{
			if(!access_ok(VERIFY_WRITE, (unsigned int __user*)input.param0, sizeof(unsigned int)) ||
				put_user(dev->wdt_upd, (unsigned int __user*)input.param0))
			{
				SEC_HAL_TRACE("wdt_value copy not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = 0;
			}
		}break;
		case SD_EXIT:{SEC_HAL_TRACE("EXIT - NOP") ret = 0;}break;
		case SD_KEY_INFO:
		{
			if(!access_ok(VERIFY_WRITE, (void __user *)input.param0, sizeof(sec_hal_key_info_t)))
			{
				SEC_HAL_TRACE("keyinfo not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_key_info_get((sec_hal_key_info_t*)input.param0);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_CERT_REGISTER:
		{
			if(!access_ok(VERIFY_READ, (void __user *)input.param0, input.param1))
			{
				SEC_HAL_TRACE("cert not ok, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_WRITE, (void __user*)input.param2, PARAM_SZ))
			{
				SEC_HAL_TRACE("objid not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_cert_register(
						(void*)input.param0,/*IN*/
						input.param1,/*IN*/
						(uint32_t*)input.param2/*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_DATA_CERT_REGISTER:
		{
			if(!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1))
			{
				SEC_HAL_TRACE("cert not ok, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_READ, (void __user*)input.param2, input.param3))
			{
				SEC_HAL_TRACE("data not ok, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_WRITE, (void __user*)input.param4, sizeof(uint32_t)))
			{
				SEC_HAL_TRACE("obj_id ptr not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_data_cert_register(
						(void*) input.param0, /*IN*/
						input.param1, /*IN*/
						(void*) input.param2, /*IN*/
						input.param3, /*IN*/
						(uint32_t*)input.param4);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_RANDOM_NUMBER_GET:{SEC_HAL_TRACE("!!RANDOM - NOT IMPL.!!")}break;
		case SD_MAC_ADDRESS_GET:
		{
			if(input.param0 > SEC_HAL_MAX_MAC_INDEX)
			{
				SEC_HAL_TRACE("mac_addr_index out-of-bounds, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_WRITE, (void __user*)input.param1, SEC_HAL_MAC_SIZE))
			{
				SEC_HAL_TRACE("mac_addr not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_mac_address_get(
						input.param0, /*IN*/
						(sec_hal_mac_address_t *)input.param1 /*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_IMEI_GET:
		{
			if(!access_ok(VERIFY_WRITE, (void __user*)input.param0, SEC_HAL_MAX_IMEI_SIZE))
			{
				SEC_HAL_TRACE("imei not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_imei_get(
						(sec_hal_imei_t *)input.param0 /*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_RAT_BAND_GET:{SEC_HAL_TRACE("!!RAT_BAND - NOT IMPL.!!")}break;
		case SD_PP_FLAGS_COUNT_GET:{SEC_HAL_TRACE("!!PP_FLAGS_COUNT - NOT IMPL.!!")}break;
		case SD_PP_FLAGS_GET:{SEC_HAL_TRACE("!!PP_FLAGS - NOT IMPL.!!")}break;
		case SD_SL_LEVELS_OPEN:
		{
			if(0/*!access_ok(VERIFY_READ, unlock_codes,)*/ /*TBDL: howto verify??*/)
			{
				SEC_HAL_TRACE("unlock_codes not ok, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_WRITE, (void __user*)input.param2, sizeof(uint32_t)))
			{
				SEC_HAL_TRACE("post_status ptr not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_simlock_levels_open(
						(uint32_t)input.param0, /*IN*/
						(void*) input.param1, /*IN*/
						(uint32_t*)input.param2 /*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_SL_LEVEL_OPEN:
		{
			if(input.param1 > SEC_HAL_MAX_SIMLOCK_CODE_LENGTH)
			{
				SEC_HAL_TRACE("unlock code too long, aborting!")
				ret = -EFAULT;
			}
			if(!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1))
			{
				SEC_HAL_TRACE("unlock code not ok, aborting!")
				ret = -EFAULT;
			}
			else if(input.param2 > SEC_HAL_MAX_SIMLOCK_LEVELS_COUNT)
			{
				SEC_HAL_TRACE("level too big, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_simlock_level_open(
						(char*)input.param0, /*IN*/
						(uint8_t)input.param2 /*IN*/);
			}
		}break;
		case SD_SL_LEVEL_STATUS_GET:
		{
			if(!access_ok(VERIFY_WRITE, (void __user*)input.param0, sizeof(uint32_t)))
			{
				SEC_HAL_TRACE("status not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_simlock_level_status_get(
						(uint32_t*)input.param0 /*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_AUTH_DATA_SIZE_GET:
		{
			if(!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1))
			{
				SEC_HAL_TRACE("input_data not ok, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_WRITE, (void __user*)input.param2, sizeof(uint32_t)))
			{
				SEC_HAL_TRACE("auth_data_size ptr not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_auth_data_size_get(
						(void*)input.param0, /*IN*/
						input.param1, /*IN*/
						(uint32_t*)input.param2 /*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_AUTH_DATA_GET:
		{
			if(!access_ok(VERIFY_READ, (void __user*)input.param0, input.param1))
			{
				SEC_HAL_TRACE("input_data not ok, aborting!")
				ret = -EFAULT;
			}
			else if(!access_ok(VERIFY_WRITE, (void __user*)input.param2, input.param3))
			{
				SEC_HAL_TRACE("auth_data not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_auth_data_get(
						(void*)input.param0, /*IN*/
						input.param1, /*IN*/
						(void*)input.param2, /*OUT*/
						input.param3 /*IN*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_PERIODIC_ICHECK:
		{
			if(!access_ok(VERIFY_WRITE, (void __user*)input.param0, sizeof(uint32_t)))
			{
				SEC_HAL_TRACE("wdt_upd not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_rt_periodic_integrity_check(
						(uint32_t*)input.param0 /*OUT*/);
				/* if success, copy_to_user applied internally */
			}
		}break;
		case SD_SELFTEST:
		{
			ret = sec_hal_rt_selftest();
		}break;
		default:{SEC_HAL_TRACE("DEFAULT!")}break;
#ifdef CONFIG_ARM_SEC_HAL_SDTOC
		case SD_TOC_READ:
		{
			uint32_t* input_data = (uint32_t *)input.param0;
			uint32_t* output_data = (uint32_t *)input.param1;
			if(access_ok(VERIFY_READ, input_data,sizeof(uint32_t)))
			{
				SEC_HAL_TRACE("input data pointer not ok, aborting!")
				ret = -EFAULT;
			}
			if(0/*!access_ok(VERIFY_WRITE, output_data, sizeof(what?))*/)
			{
				SEC_HAL_TRACE("output data pointer not ok, aborting!")
				ret = -EFAULT;
			}
			else
			{
				ret = sec_hal_sdtoc_read(input_data,(void *)output_data);
				/* if success, copy_to_user applied internally */
			}
		}break;
#endif
	}

	up(&dev->sem);

	SEC_HAL_TRACE_EXIT
	return ret; /* directly return API definitions */
}

#ifdef CONFIG_ARM_SEC_HAL_RPC_HANDLER
#define RPC_SUCCESS 0x00
#define RPC_FAILURE 0x01
void* sec_hal_mem_msg_area_calloc(unsigned int n, unsigned int sz);
void  sec_hal_mem_msg_area_free(void *virt_addr);
/* ****************************************************************************
 * RPC HANDLER
 * ***************************************************************************/
static uint32_t rpc_handler(
		uint32_t id,
		uint32_t p1,
		uint32_t p2,
		uint32_t p3,
		uint32_t p4)
{
	uint32_t ret = 0x00;
	struct cli_dev_data* dev = &g_cli_dev_data;
	sd_ioctl_params_t params = {id, p1, p2, p3, p4};

	SEC_HAL_TRACE_ENTRY

	switch(id) /* pre-ipc step for callbacks that are handled internally by hal.*/
	{
		case SEC_HAL_RPC_ALLOC:
		{
			ret = (uint32_t)virt_to_phys(sec_hal_mem_msg_area_calloc(1, p1));
			SEC_HAL_TRACE_EXIT
			return ret;
		}break;
		case SEC_HAL_RPC_FREE:
		{
			sec_hal_mem_msg_area_free(phys_to_virt(p1));
			SEC_HAL_TRACE_EXIT
			return RPC_SUCCESS;
		}break;
		default: break;
	}

	rpcq_add_wakeup(&dev->rpc_read_waitq, &params);
	if(rpcq_get_wait(&dev->rpc_write_waitq, &params))
	{
		SEC_HAL_TRACE_EXIT_INFO("interrupted, aborting!")
		return RPC_FAILURE;
	}

	switch(id)/* post-ipc step for params conversion and etc. */
	{
		case SEC_HAL_RPC_PROT_DATA_ALLOC:
		{
			sec_msg_handle_t ret_handle;
			sec_msg_t *ret_msg = sec_msg_alloc(&ret_handle,
					3*sec_msg_param_size(sizeof(uint32_t)),
					SEC_MSG_OBJECT_ID_NONE,
					0,
					SEC_HAL_MSG_BYTE_ORDER); /* dealloc by secenv */
			if (ret_msg && SEC_HAL_RES_OK == params.reserved1)
			{
				/* ensure that the prot_data is in SDRAM memory */
				SEC_HAL_MEM_CACHE_CLEAN_FUNC((void __user*)params.param3, params.param4);
			}
			sec_msg_param_write32(&ret_handle, params.reserved1, SEC_MSG_PARAM_ID_NONE);
			sec_msg_param_write32(&ret_handle, params.param4, SEC_MSG_PARAM_ID_NONE);
			sec_msg_param_write32(&ret_handle, params.param3, SEC_MSG_PARAM_ID_NONE);
			SEC_HAL_TRACE_EXIT_INFO("data_size == %u", params.param4);
			return (uint32_t) ret_msg;
		}break;
		case SEC_HAL_RPC_PROT_DATA_FREE: /*NOP*/ break;
		default: break;
	}

	SEC_HAL_TRACE_EXIT
	return RPC_SUCCESS;
}

/* ----------------------------------------------------------------------------
 * sec_hal_rpc_read : read content from rpc queque.
 * --------------------------------------------------------------------------*/
static ssize_t sec_hal_rpc_read(
		struct file *filp,
		char __user* buf,
		size_t count,
		loff_t *ppos)
{
	int cnt = 0;
	struct cli_dev_data* dev = filp->private_data;
	sd_ioctl_params_t param;

	SEC_HAL_TRACE_ENTRY

	if(filp->f_flags & O_NONBLOCK)
	{
		SEC_HAL_TRACE_EXIT_INFO("non-blocking IO not supported, aborting!");
		return -EPERM;
	}
	
	if(rpcq_get_wait(&dev->rpc_read_waitq, &param))
	{
		SEC_HAL_TRACE_EXIT_INFO("interrupted, restart syscall")
		return -ERESTARTSYS;
	}
	cnt = copy_to_user(buf, &param, k_param_sz);

	SEC_HAL_TRACE_EXIT
	return (k_param_sz-cnt);
}

/* ----------------------------------------------------------------------------
 * sec_hal_rpc_write : write content to rpc queque.
 * --------------------------------------------------------------------------*/
static ssize_t sec_hal_rpc_write(
		struct file *filp,
		const char *buf,
		size_t count,
		loff_t *ppos)
{
	int cnt = 0;
	struct cli_dev_data* dev = filp->private_data;
	sd_ioctl_params_t param;

	SEC_HAL_TRACE_ENTRY

	if(filp->f_flags & O_NONBLOCK)
	{
		SEC_HAL_TRACE_EXIT_INFO("non-blocking IO not supported, aborting!");
		return -EPERM;
	}
	
	cnt = copy_from_user(&param, buf, k_param_sz);
	rpcq_add_wakeup(&dev->rpc_write_waitq, &param);

	SEC_HAL_TRACE_EXIT
	return (k_param_sz-cnt);
}

#endif /* CONFIG_ARM_SEC_HAL_RPC_HANDLER */
/* ----------------------------------------------------------------------------
 * --------------------------------------------------------------------------*/
static struct file_operations k_sec_hal_fops = {
	.owner = THIS_MODULE,
	.open = &sec_hal_cli_open,
	.release = &sec_hal_cli_release,
	.unlocked_ioctl = &sec_hal_cli_ioctl,
#ifdef CONFIG_ARM_SEC_HAL_RPC_HANDLER
	.read = &sec_hal_rpc_read,
	.write = &sec_hal_rpc_write,
#endif  /*CONFIG_ARM_SEC_HAL_RPC_HANDLER*/
	.llseek = &no_llseek,
};


/* ----------------------------------------------------------------------------
 * add_attach_cdev :
 * --------------------------------------------------------------------------*/
static int add_attach_cdev(
		struct cdev* dev,
		struct file_operations* fops,
		struct class* cls,
		dev_t devno,
		const char* devname)
{
	SEC_HAL_TRACE_ENTRY

	cdev_init(dev, fops);
	dev->owner = THIS_MODULE;
	if(cdev_add(dev, devno, 1))
	{
		SEC_HAL_TRACE_EXIT_INFO("failed to add cdev, aborting!")
		return -ENODEV;
	}
	device_create(cls, NULL, devno, NULL, devname);

	SEC_HAL_TRACE_EXIT
	return 0;
}

/* ----------------------------------------------------------------------------
 * detach_del_cdev :
 * --------------------------------------------------------------------------*/
static void detach_del_cdev(
		struct cdev* dev,
		struct class* cls)
{
	SEC_HAL_TRACE_ENTRY

	device_destroy(cls, dev->dev);
	cdev_del(dev);

	SEC_HAL_TRACE_EXIT
}

/* ----------------------------------------------------------------------------
 * sec_hal_setup_cdev_init : allocate & initialize cdev node related structures.
 * --------------------------------------------------------------------------*/
static int sec_hal_cdev_init(
		struct cli_dev_data *dev_data)
{
	int ret = 0;
	dev_t devno = 0;
	struct class* cls;

	SEC_HAL_TRACE_ENTRY

	if(0 > alloc_chrdev_region(&devno, 0, 1, DEVNAME))
	{
		SEC_HAL_TRACE("cannot register device, aborting!")
		ret = -EIO;
		goto e1;
	}

	cls = class_create(THIS_MODULE, DEVNAME);
	if(IS_ERR(cls))
	{
		SEC_HAL_TRACE("failed to class_create, aborting!")
		ret = PTR_ERR(cls);
		goto e2;
	}

	if(add_attach_cdev(&dev_data->cdev, &k_sec_hal_fops, cls, devno, DEVNAME))
	{
		SEC_HAL_TRACE("failed to add and attach cdev, aborting!")
		ret = -ENODEV;
		goto e2;
	}

	/* all ok, store heap data to argument struct. */
	sema_init(&dev_data->sem, CONFIG_NR_CPUS);
	dev_data->class = cls;
#ifdef CONFIG_ARM_SEC_HAL_RPC_HANDLER
	rpcq_init(&dev_data->rpc_read_waitq);
	rpcq_init(&dev_data->rpc_write_waitq);
#endif /* CONFIG_ARM_SEC_HAL_RPC_HANDLER */

	SEC_HAL_TRACE_EXIT
	return ret;

e2:	class_destroy(cls);
e1:	unregister_chrdev_region(MAJOR(devno), 1);
	SEC_HAL_TRACE_EXIT_INFO("failed to create /dev - nodes.")
	return ret;
}


/* ----------------------------------------------------------------------------
 * sec_hal_cdev_exit : release cdevs related resources
 * --------------------------------------------------------------------------*/
static void sec_hal_cdev_exit(
		struct cli_dev_data *dev_data)
{
	dev_t devno = dev_data->cdev.dev;

	SEC_HAL_TRACE_ENTRY

	detach_del_cdev(&dev_data->cdev, dev_data->class);
	class_destroy(dev_data->class);
	unregister_chrdev_region(MAJOR(devno), 1);

	SEC_HAL_TRACE_EXIT
}




/* ****************************************************************************
 * ICRAM ALLOCATION RELATED FUNCTIONS
 * ***************************************************************************/
#if (!defined(BLOCKCOUNT) && !BLOCKCOUNT)
#error !!local macro not defined, can cause div by zero exception!!
#endif
/* ----------------------------------------------------------------------------
 * sec_hal_mem_msg_area_init :
 * --------------------------------------------------------------------------*/
int sec_hal_mem_msg_area_init()
{
	int ret = 0;
	unsigned int block_sz, index = 0;
	struct mem_msg_area *msg_area = &g_cli_dev_data.icram0;

	SEC_HAL_TRACE_ENTRY

    if(g_icram_mem_init_done == 1)
    {
    SEC_HAL_TRACE("sec_hal_mem_msg_area_init already done")
    return ret;
    }

	if(!request_mem_region(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE), "msg area"))
	{
             printk("Sec_hal_mem_msg_area_init error \n");
		ret = -ENODEV;
		goto e1;
	}
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
	msg_area->virt_baseptr = kmalloc(size, GFP_KERNEL);
#else
	msg_area->virt_baseptr = ioremap_nocache(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE));
#endif /* CONFIG_ARM_SEC_HAL_TEST_DISPATCHER */
    SEC_HAL_TRACE("msg_area->virt_baseptr 0x%x",msg_area->virt_baseptr)
	msg_area->phys_start = UL(ICRAM1_ADDRESS);
    SEC_HAL_TRACE("msg_area->phys_start 0x%x",msg_area->phys_start)
	msg_area->phys_size = UL(ICRAM1_SIZE);
    SEC_HAL_TRACE("msg_area->phys_size 0x%x",msg_area->phys_size)
	msg_area->offset = msg_area->virt_baseptr - msg_area->phys_start;
    SEC_HAL_TRACE("msg_area->offset 0x%x",msg_area->offset)
	msg_area->valist_size = 20;

	if(NULL == msg_area->virt_baseptr)
	{
		ret = -EINVAL;
		goto e2;
	}

	block_sz = (msg_area->phys_size)/BLOCKCOUNT;
	SEC_HAL_TRACE("block_sz == %d", block_sz)

	/* initialize msg area alloc blocks */
	for(; index < BLOCKCOUNT; index++)
	{
		msg_area->msg_blocks[index].virt_addr =
				((__u8*)msg_area->virt_baseptr + block_sz*index);
		msg_area->msg_blocks[index].size = 0;
		msg_area->msg_blocks[index].allocated = FALSE;
	}

    msg_area->valist_ptr_ptr = sec_hal_mem_msg_area_calloc(1,4);
	if(NULL == msg_area->valist_ptr_ptr)
	{
	       printk("msg_area->valist_ptr_ptr is bad \n");
		ret = -EINVAL;
		goto e2;
	}

    msg_area->valist_ptr = sec_hal_mem_msg_area_calloc(1,20);
	if(NULL == msg_area->valist_ptr)
	{
	       printk("msg_area->valist_ptr is bad \n");
		ret = -EINVAL;
		goto e2;
	}
    g_icram_mem_init_done=1;
	SEC_HAL_TRACE_EXIT
	return ret;

e2: release_mem_region(UL(ICRAM1_ADDRESS), UL(ICRAM1_SIZE));
e1: MEM_MSG_AREA_CLEAR(msg_area); /* leave mem struct as 'unallocated' */
	SEC_HAL_TRACE_EXIT_INFO("failure in mem area allocation")
	return ret;
}

EXPORT_SYMBOL(sec_hal_mem_msg_area_init); /* made available for other kernel entities */

/* ----------------------------------------------------------------------------
 * sec_hal_mem_msg_area_exit :
 * --------------------------------------------------------------------------*/
void sec_hal_mem_msg_area_exit(
		struct mem_msg_area *msg_area)
{
#ifdef CONFIG_ARM_SEC_HAL_TEST_DISPATCHER
	kfree(msg_area->virt_baseptr);
#else
	iounmap(msg_area->virt_baseptr);
#endif /* CONFIG_ARM_SEC_HAL_TEST_DISPATCHER */
	release_mem_region(msg_area->phys_start, msg_area->phys_size);
	MEM_MSG_AREA_CLEAR(msg_area); /* leave mem struct as 'unallocated' */
}


/* ----------------------------------------------------------------------------
 * sec_hal_mem_msg_area_memcpy : simple memcpy
 * --------------------------------------------------------------------------*/
unsigned long sec_hal_mem_msg_area_memcpy(
		void *dst,
		const void *src,
		unsigned long sz)
{
	__u8* dst8 = (__u8*)dst;
	__u8* src8 = (__u8*)src;
	while (sz--){ *dst8++ = *src8++; }
	return (unsigned long)dst;
}

unsigned long sec_hal_mem_msg_area_write(
		void *dst,
		const void *src,
		unsigned long sz)
{
	__u8* dst8 = (__u8*)dst;
	__u8* src8 = (__u8*)src;
    int tmp = 0;
	SEC_HAL_TRACE_ENTRY;

    printk("Dest addr 0x%x", dst);
    printk("Source addr 0x%x", src);
    printk("Data size %d", sz);
    printk("Print data from src8 0x%x", src8);
	while (sz--)
    {
        printk("0x%x ",*src8);
        iowrite8(*src8++,dst++);
        tmp++;
    }
    printk("Data size was %d", tmp);

	SEC_HAL_TRACE_EXIT;
	return (unsigned long)dst;
}

unsigned long sec_hal_mem_msg_area_read(
		void *dst,
		const void *src,
		unsigned long sz)
{
	__u8* dst8 = (__u8*)dst;
	__u8* src8 = (__u8*)src;
	SEC_HAL_TRACE_ENTRY;
         printk(" sec_hal_mem_msg_area_read \n");
	 
	while (sz--){*dst8++ =  ioread8(src8++);}

	SEC_HAL_TRACE_EXIT;
	return (unsigned long)dst;
}

/* ----------------------------------------------------------------------------
 * sec_hal_mem_msg_area_memset : simple memset for ZI purposes.
 * --------------------------------------------------------------------------*/
static inline void sec_hal_mem_msg_area_memset(
		void *buff,
		unsigned char data,
		unsigned int cnt)
{
	__u8* ptr = (__u8*)buff;
	while(cnt > 0){ *ptr++ = data; cnt--; }
}

/* ----------------------------------------------------------------------------
 * sec_hal_mem_msg_area_calloc : used by sec_hal_rt
 * --------------------------------------------------------------------------*/
void* sec_hal_mem_msg_area_calloc(
		unsigned int n,
		unsigned int sz)
{
	unsigned int block_sz, block_cnt, block_ind, index = 0;
	int found = FALSE;
	void* virt_addr = NULL;
	struct mem_msg_area* ramb = &g_cli_dev_data.icram0;

	SEC_HAL_TRACE_ENTRY

	if (0 == (n*sz))
	{
		SEC_HAL_TRACE_EXIT_INFO("!!n*sz is zero, aborting!!")
		return NULL;
	}

	block_sz  = (ramb->phys_size)/BLOCKCOUNT;
	block_cnt = ((n*sz)+block_sz-1)/block_sz;
	SEC_HAL_TRACE("block_sz = %d, block_cnt = %d", block_sz, block_cnt)

	if (block_cnt > BLOCKCOUNT)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!too big block count, aborting!!")
		return NULL;
	}

	spin_lock(&g_dev_spinlock); /* protect 'ramb' access */
	/* critical section starting, do not 'call' anything that may sleep.*/

	/* seek big enough unallocated mem area */
	while (index < BLOCKCOUNT)
	{
		if (FALSE == ramb->msg_blocks[index].allocated)
		{
			found = TRUE;
			block_ind = block_cnt - 1;
			while (block_ind > 0 && (index+block_ind) < BLOCKCOUNT)
				{
				found = (found && FALSE == ramb->msg_blocks[index+block_ind].allocated);
				block_ind--;
				}
			if (TRUE == found){break;} /* check if the loop can be ended */
		}
		index++;
	}

	/* return ptr to first block, update allocation info & zero initialize */
	if (TRUE == found && index < BLOCKCOUNT)
	{
		/* allocated found message area */
		virt_addr = ramb->msg_blocks[index].virt_addr;
		ramb->msg_blocks[index].size = block_cnt;
		ramb->msg_blocks[index].allocated = TRUE;
		sec_hal_mem_msg_area_memset(virt_addr, 0, block_sz);
		block_cnt--;
	}

	/* also update allocation info for rest of the blocks & zero initialize */
	while (TRUE == found && block_cnt > 0 && (index+block_cnt) < BLOCKCOUNT)
	{
		ramb->msg_blocks[index+block_cnt].allocated = TRUE;
		sec_hal_mem_msg_area_memset(
				ramb->msg_blocks[index+block_cnt].virt_addr,
				0, block_sz);
		block_cnt--;
	}

	/* critical section ending. */
	spin_unlock(&g_dev_spinlock);

	SEC_HAL_TRACE_EXIT_INFO("found: %d", found)
	return virt_addr; /* return allocated(or not) memory address */
}

/* ----------------------------------------------------------------------------
 * sec_hal_mem_msg_area_free : used by sec_hal_rt
 * --------------------------------------------------------------------------*/
void sec_hal_mem_msg_area_free(
		void *virt_addr)
{
	unsigned int block_ind, index = 0;
	struct mem_msg_area* ramb = &g_cli_dev_data.icram0;

	SEC_HAL_TRACE_ENTRY

	if(NULL == virt_addr)
	{
		SEC_HAL_TRACE_EXIT_INFO("!!NULL, aborting!!")
		return;
	}

	spin_lock(&g_dev_spinlock); /* protect 'ramb' access */
	/* critical section starting, do not 'call' anything that may sleep.*/

	while (index < BLOCKCOUNT)
	{
		if(ramb->msg_blocks[index].virt_addr == virt_addr)
		{
			block_ind = ramb->msg_blocks[index].size - 1;

			/* free allocated message area */
			ramb->msg_blocks[index].size = 0;
			ramb->msg_blocks[index].allocated = FALSE;

			/* free rest of the blocks */
			while(0 < block_ind && (index+block_ind) < BLOCKCOUNT)
			{
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

	SEC_HAL_TRACE_EXIT
}

#if 0
static void print_access(void * memory_ptr)
{
   printk("Tried to access 0x%x \n",memory_ptr");
}

void memory_protection_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
   /* This variables are static because they need to be 
    * accessible (through pointers) to the bottom half routine.
    */
/*   static unsigned char scancode;
   static struct tq_struct task = {NULL, 0, print_access, &scancode};
   unsigned char status; */

    void * memptr;
    static struct tq_struct task = {NULL, 0, print_access, NULL};
   /* Read keyboard status */
/*   status = inb(0x64);
   scancode = inb(0x60);*/
  
   /* Schedule bottom half to run */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,2,0)
   queue_task(&task, &tq_immediate);
#else
   queue_task_irq(&task, &tq_immediate);
#endif
   mark_bh(IMMEDIATE_BH);
}
#endif

/* ****************************************************************************
 * PLATFORM DEVICE FRAMEWORK RELATED FUNCTIONS.
 * ***************************************************************************/
/* ----------------------------------------------------------------------------
 * sec_hal_platform_device_probe :
 * --------------------------------------------------------------------------*/
static int sec_hal_platform_device_probe(
		struct platform_device *plat_dev)
{
	int ret = 0;
	struct resource* mem = NULL;

	SEC_HAL_TRACE_ENTRY

#if 0
	if(g_cli_dev_data.icram0.virt_baseptr)
	{
		/* only one platdev supported at the moment. */
		SEC_HAL_TRACE_EXIT_INFO("alloc already done")
		return -ENODEV;
	}
#endif
	if(plat_dev && NULL == (mem = platform_get_resource(
			plat_dev, IORESOURCE_MEM, 0)))
	{
		SEC_HAL_TRACE_EXIT_INFO("faulty arguments, aborting!")
		return -ENOMEM;
	}

	ret = sec_hal_mem_msg_area_init();
	if(ret){goto e0;}

#ifdef CONFIG_ARM_SEC_HAL_RT_INIT
	ret = sec_hal_rt_init(&g_cli_dev_data.wdt_upd);
	if(ret){goto e1;}
#endif /* CONFIG_ARM_SEC_HAL_RT_INIT */

#ifdef CONFIG_ARM_SEC_HAL_RPC_HANDLER
	ret = sec_hal_rt_install_rpc_handler(&rpc_handler);
	if(ret){goto e1;}
#endif /* CONFIG_ARM_SEC_HAL_RPC_HANDLER */

	ret = sec_hal_cdev_init(&g_cli_dev_data);
	if(ret){goto e2;}

    /* Init sdtoc memory area */
#ifdef CONFIG_ARM_SEC_HAL_SDTOC
	if(plat_dev && NULL == (mem = platform_get_resource(plat_dev, IORESOURCE_MEM, 1)))
	{
		SEC_HAL_TRACE_EXIT_INFO("faulty arguments, aborting!")
		return -ENOMEM;
	}
	ret = sec_hal_sdtoc_area_init(mem->start, resource_size(mem));
	if(ret){goto e2;}
#endif /* CONFIG_ARM_SEC_HAL_SDTOC */

#if 0
   request_irq(1,   /* The number of the keyboard IRQ on PCs */ 
          memory_protection_irq_handler, /* our handler */
          68, 
          "test_memory_protection_irq_handler", NULL);
#endif

	SEC_HAL_TRACE_EXIT
	return ret;

e2:	sec_hal_cdev_exit(&g_cli_dev_data);
e1:	sec_hal_mem_msg_area_exit(&g_cli_dev_data.icram0);
e0:
	SEC_HAL_TRACE_EXIT_INFO("failed in device init")
	return ret;
}

/* ----------------------------------------------------------------------------
 * sec_hal_platform_device_remove :
 * --------------------------------------------------------------------------*/
static int sec_hal_platform_device_remove(
		struct platform_device *plat_dev)
{
	SEC_HAL_TRACE_ENTRY

	sec_hal_cdev_exit(&g_cli_dev_data);
	sec_hal_mem_msg_area_exit(&g_cli_dev_data.icram0);

	SEC_HAL_TRACE_EXIT
	return 0;
}

/* ----------------------------------------------------------------------------
 * sec_hal_platform_device_shutdown :
 * --------------------------------------------------------------------------*/
static void sec_hal_platform_device_shutdown(
		struct platform_device *plat_dev)
{
	SEC_HAL_TRACE_ENTRY
	// TBDL
	SEC_HAL_TRACE_EXIT
}

/* ----------------------------------------------------------------------------
 * sec_hal_platform_device_suspend :
 * --------------------------------------------------------------------------*/
static int sec_hal_platform_device_suspend(
		struct platform_device *plat_dev,
		pm_message_t state)
{
	SEC_HAL_TRACE_ENTRY
	// TBDL
	SEC_HAL_TRACE_EXIT
	return 0;
}

/* ----------------------------------------------------------------------------
 * sec_hal_platform_device_resume :
 * --------------------------------------------------------------------------*/
static int sec_hal_platform_device_resume(struct platform_device *plat_dev)
{
	SEC_HAL_TRACE_ENTRY
	// TBDL
	SEC_HAL_TRACE_EXIT
	return 0;
}

/* ----------------------------------------------------------------------------
 * --------------------------------------------------------------------------*/
static struct platform_driver k_sec_hal_platform_device_driver =
{
	.probe    = sec_hal_platform_device_probe,
	.remove   = sec_hal_platform_device_remove,
	.shutdown = sec_hal_platform_device_shutdown,
	.suspend  = sec_hal_platform_device_suspend,
	.resume   = sec_hal_platform_device_resume,
	.driver   = {.name  = DEVNAME, .owner = THIS_MODULE},
};




/* ****************************************************************************
 * EXPORTs, Modem Boot
 * ***************************************************************************/
/* ----------------------------------------------------------------------------
 * sec_hal_mdm_memcpy : copy data to a certain kind of protected memory.
 * --------------------------------------------------------------------------*/
uint32_t sec_hal_mdm_memcpy(
		void* phys_dst,
		void* phys_src,
		uint32_t size)
{
	uint32_t ret;
	struct cli_dev_data* dev = &g_cli_dev_data;

	SEC_HAL_TRACE_ENTRY

	if(down_interruptible(&dev->sem))
	{
		SEC_HAL_TRACE("interrupted, restart syscall")
		return SEC_HAL_MDM_RES_FAIL;
	}

	ret = sec_hal_rt_memcpy(phys_dst, phys_src, size);

	up(&dev->sem);

	SEC_HAL_TRACE_EXIT
	return ret;
}
EXPORT_SYMBOL(sec_hal_mdm_memcpy); /* made available for other kernel entities */

/* ----------------------------------------------------------------------------
 * sec_hal_mdm_authenticate : authenticate memory content with SW cert.
 * --------------------------------------------------------------------------*/
uint32_t sec_hal_mdm_authenticate(
		void* phys_addr,
		uint32_t size)
{
	uint32_t ret;
	struct cli_dev_data* dev = &g_cli_dev_data;

	SEC_HAL_TRACE_ENTRY

	if(down_interruptible(&dev->sem))
	{
		SEC_HAL_TRACE("interrupted, restart syscall")
		return SEC_HAL_MDM_RES_FAIL;
	}

	ret = sec_hal_rt_cert_register(phys_to_virt((phys_addr_t)phys_addr), size, NULL);

	up(&dev->sem);

	SEC_HAL_TRACE_EXIT
	return ret;
}
EXPORT_SYMBOL(sec_hal_mdm_authenticate); /* made available for other kernel entities */


/* ****************************************************************************
 * MODULE INIT & EXIT
 * ***************************************************************************/
#ifdef CONFIG_ARM_SEC_HAL_BUILTIN_PLATDEV
	static struct resource k_sec_hal_resources[] =
	{
		[0] =
		{	/* ICRAM0 */
			.start = UL(ICRAM1_ADDRESS),
			.end = UL(ICRAM1_ADDRESS) + UL(ICRAM1_SIZE) - 1,/* 4kb from start addr */
			.flags = IORESOURCE_MEM,
		},
#ifdef CONFIG_ARM_SEC_HAL_SDTOC
		[1] =
		{	/* SDRAM TOC */
			.start = 0x47FE0000UL,
			.end = 0x47FE0000UL + 0x4000UL - 1,/* 16kb from start addr */
			.flags = IORESOURCE_MEM,
		},
#endif
	};
	static struct platform_device k_sec_hal_chardevice =
	{
		.name =  DEVNAME,
		.id = 0,
		.resource = k_sec_hal_resources,
		.num_resources = ARRAY_SIZE(k_sec_hal_resources),
	};
	static struct platform_device *k_sec_hal_local_devs[] __initdata =
	{
		&k_sec_hal_chardevice,
	};
#endif
/* ----------------------------------------------------------------------------
 * sec_hal_driver_init :
 * --------------------------------------------------------------------------*/
static int __init sec_hal_driver_init(void)
{
	int ret = 0;

	SEC_HAL_TRACE_INIT
	SEC_HAL_TRACE_ENTRY

	ret = platform_driver_register(&k_sec_hal_platform_device_driver);

#ifdef CONFIG_ARM_SEC_HAL_BUILTIN_PLATDEV
	platform_add_devices(k_sec_hal_local_devs, ARRAY_SIZE(k_sec_hal_local_devs));
#endif

	SEC_HAL_TRACE_EXIT
	return ret;
}

/* ----------------------------------------------------------------------------
 * sec_hal_driver_exit :
 * --------------------------------------------------------------------------*/
static void __exit sec_hal_driver_exit(void)
{
	SEC_HAL_TRACE_ENTRY

	platform_driver_unregister(&k_sec_hal_platform_device_driver);

	SEC_HAL_TRACE_EXIT
}
module_init(sec_hal_driver_init);
module_exit(sec_hal_driver_exit);

MODULE_AUTHOR("Renesas Mobile Corporation");
MODULE_DESCRIPTION("Device driver for accessing ARM TRUSTZONE during runtime");
MODULE_LICENSE("Proprietary");

