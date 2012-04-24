/**********************************************************
*  Copyright(c) 2009 Discretix. All rights reserved.
*  Copyright(c) 2009 Intel Corporation. All rights reserved.
*  Copyright(c) 2011 Renesas Mobile Corp. All rights reserved.
*
*  This program is free software; you can redistribute it and/or modify it
*  under the terms of the GNU General Public License, version 2,
*  as published by the Free Software Foundation.
*
*  This program is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
*  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
*  more details.
*
*  You should have received a copy of the GNU General Public License along with
*  this program; if not, write to the Free Software Foundation, Inc., 59
*  Temple Place - Suite 330, Boston, MA  02111-1307, USA
**********************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <mach/r8a73734.h>
#if defined(DX_DEBUG_RAR_SUPPORT) || defined(DX_RAR_SUPPORT)
#include <linux/rar/memrar.h>
#include <linux/rar/rar_register.h>
#endif

#ifdef DX_CC52_SUPPORT
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#endif

#include "cc42_crypt_driver_hw_defs.h"
#include "cc42_crypt_driver_config.h"
#include "cc42_crypt_driver_api.h"

/*--------------------------------------------
	GLOBAL variables
--------------------------------------------*/
/* debug messages level */
int sep_debug;
module_param(sep_debug, int, 0);
MODULE_PARM_DESC(sep_debug, "Flag to enable SEP debug messages");

/* fatal error indicator */
int sep_fatal;
module_param(sep_fatal, int, 0444);
MODULE_PARM_DESC(sep_fatal, \
	"Flag which indicates that fatal error has occured");


/* platform device object */
static struct platform_device sep_device;

static struct platform_driver sep_driver;

static const struct file_operations sep_file_operarions;

/* context of the device */
struct device_context   sep_context;

struct ioctl_params_dma ioctl_params_dma;
/*---------------------------------------------
	FUNCTIONS
-----------------------------------------------*/
/*
this function registers the driver to the device subsystem(either PCI,USB,etc)
*/
int sep_register_driver_to_device(void)
{
	/* error */
	int error = 0;

	/*-------------------------
	CODE
	-----------------------------*/
	error = platform_device_register(&sep_device);
	sep_context.dev_ptr = &sep_device.dev;
printk(KERN_INFO "SEP Driver:platform_device_register error is %x\n", error);
	if (error)
		return error;

	error = platform_driver_register(&sep_driver);
printk(KERN_INFO "SEP Driver:platform_driver_register error is %x\n", error);

	if (error) {
		/* Note: Upper layer is responsible for unregistering device*/
		return error;
	}

	return error;
}


/*
This function registers the driver to the file system
*/
static int  sep_register_driver_to_fs(void)
{
	/* return value */
	int			ret_val;

	/* major number */
	int          major;

	/*---------------------
	CODE
	-----------------------*/

#ifdef DX_CC52_SUPPORT
	ret_val = alloc_chrdev_region(&sep_context.device_number,
					0,
					3,
					DRIVER_NAME);
#else
	ret_val = alloc_chrdev_region(&sep_context.device_number,
					0,
					2,
					DRIVER_NAME);
#endif
	if (ret_val) {
		printk(KERN_ERR "sep_driver:major number \
			allocation failed,retval is %d\n", ret_val);
		goto end_function;
	}

	/* get the major number */
	major = MAJOR(sep_context.device_number);

	/* init cdev */
	cdev_init(&sep_context.cdev, &sep_file_operarions);
	sep_context.cdev.owner = THIS_MODULE;

	/* register the driver with the kernel */
	ret_val = cdev_add(&sep_context.cdev, sep_context.device_number, 1);

	if (ret_val) {
		printk(KERN_ERR "sep_driver:cdev_add \
			failed,retval is %d\n", ret_val);
		goto end_function_unregister_devnum;
	}


	goto end_function;

end_function_unregister_devnum:

#ifdef DX_CC52_SUPPORT
	/* unregister dev numbers */
	unregister_chrdev_region(sep_context.device_number, 3);
#else
	unregister_chrdev_region(sep_context.device_number, 2);
#endif

end_function:

	return ret_val;
}


static void sep_platform_driver_release(struct device *dev)
{
	/* unmap */
	iounmap((void *)sep_context.reg_addr);
	printk(KERN_INFO "SEP Driver: iounmap\n");
}

/*
This function unregisters the driver
from the device subsystem(either PCI,USB,etc)
*/
int sep_unregister_driver_from_device(void)
{
	/* error */
	int error = 0;

	/*-------------------------
	CODE
	-----------------------------*/

printk(KERN_INFO "SEP Driver: sep_unregister_driver_from_device start\n");

	/* register driver */
	platform_driver_unregister(&sep_driver);
	printk(KERN_INFO "SEP Driver: platform_driver_unregister\n");

	platform_device_unregister(&sep_device);

	printk(KERN_INFO "SEP Driver: platform_device_unregister\n");


	return error;
}


/*
  this function unregisters driver from fs
*/
static void sep_unregister_driver_from_fs(void)
{
	/*-------------------
	CODE
	---------------------*/

	cdev_del(&sep_context.cdev);
	/*cdev_del(&sep_context.request_daemon_cdev); */

#ifdef DX_CC52_SUPPORT
	cdev_del(&sep_context.singleton_cdev);

/* unregister dev numbers */
	unregister_chrdev_region(sep_context.device_number, 3);
#else
	unregister_chrdev_region(sep_context.device_number, 2);
#endif
}


/*
This function locks all the physical pages of the application virtual buffer
and construct a basic lli  array, where each entry holds the physical page
address and the size that application data holds in this physical pages
*/
int sep_lock_user_pages(u32		app_virt_addr,
			u32		data_size,
			int in_out_flag)

{
	/* error */
	int				error;

	/* the the page of the end address of the user space buffer */
	u32		end_page;

	/* the page of the start address of the user space buffer */
	u32		start_page;

	/* the range in pages */
	u32		num_pages;

	/* array of pointers ot page */
	struct page		**page_array;

	struct sep_lli_entry_t		*lli_array = NULL;

	/* map array */
	struct sep_dma_map  *map_array;

	/* direction of the DMA mapping for locked pages */
	enum dma_data_direction	dir;

	/* count */
	u32		count;

	/* result */
	int				result;
	int i = 0;

	/*------------------------
	CODE
	--------------------------*/

	printk(KERN_INFO "SEP Driver:--------> sep_lock_user_pages start\n");

	error = 0;

	/* set start and end pages  and num pages */
	end_page = (app_virt_addr + data_size - 1) >> PAGE_SHIFT;
	start_page = app_virt_addr >> PAGE_SHIFT;
	num_pages = end_page - start_page + 1;

printk(KERN_INFO "SEP Driver: app_virt_addr is %08x\n", app_virt_addr);
	printk(KERN_INFO "SEP Driver: data_size is %x\n", data_size);
	printk(KERN_INFO "SEP Driver: start_page is %x\n", start_page);
	printk(KERN_INFO "SEP Driver: end_page is %x\n", end_page);
	printk(KERN_INFO "SEP Driver: num_pages is %x\n", num_pages);

	printk(KERN_INFO "SEP Driver: starting page_array malloc\n");

	/* allocate array of pages structure pointers */
	page_array = kmalloc(sizeof(struct page *) * num_pages, GFP_ATOMIC);
	if (!page_array) {
		printk(KERN_ERR "SEP Driver: kmalloc for page_array failed\n");

		error = -ENOMEM;
		goto end_function;
	}

	map_array = kmalloc(sizeof(struct sep_dma_map)*num_pages, GFP_ATOMIC);
	if (!map_array) {
		printk(KERN_ERR "SEP Driver: kmalloc for map_array failed\n");
		error = -ENOMEM;
		goto end_function_with_error1;
	}

	lli_array = kmalloc(sizeof(struct sep_lli_entry_t) * num_pages,
					GFP_ATOMIC);
	if (!lli_array) {
		edbg(KERN_ERR "SEP Driver: kmalloc for lli_array failed\n");

		error = -ENOMEM;
		goto end_function_with_error2;
	}

	printk(KERN_INFO "SEP Driver: starting get_user_pages\n");

	/* convert the application virtual address into a set of physical */
	down_read(&current->mm->mmap_sem);


	result = get_user_pages(current,
				current->mm,
				app_virt_addr,
				num_pages,
				1,
				0,
				page_array,
				0);

	up_read(&current->mm->mmap_sem);



	/*check the number of pages locked - if not all then exit with error*/
	if (result != num_pages) {
		printk(KERN_ERR "SEP Driver:not all pages \
		locked by get_user_pages\n");
		error = -ENOMEM;
		goto end_function_with_error3;
	}

printk(KERN_INFO "SEP Driver: get_user_pages succeeded\n");

	/* set direction */
	if (in_out_flag == 1002)
		dir = DMA_TO_DEVICE;
	else
		dir = DMA_FROM_DEVICE;



	/* fill the array using page array data and map the pages - this action
	will also flush the cache as needed */
	for (count = 0; count < num_pages; count++) {
		/* fill the map array */
		map_array[count].dma_addr =
			dma_map_page(sep_context.dev_ptr, page_array[count],
			0,
			PAGE_SIZE,
			/*dir*/DMA_BIDIRECTIONAL);

		map_array[count].size = PAGE_SIZE;
		/* fill the lli array entry */
		lli_array[count].bus_address = (u32)map_array[count].dma_addr;
		lli_array[count].block_size = PAGE_SIZE;
	}

	/* check the offset for the first page -
	data may start not at the beginning of the page */
	lli_array[0].bus_address =
	lli_array[0].bus_address + (app_virt_addr & (~PAGE_MASK));


	/* check that not all the data is in the first page only */
	if ((PAGE_SIZE - (app_virt_addr & (~PAGE_MASK))) >= data_size)
		lli_array[0].block_size = data_size;
	else
		lli_array[0].block_size =
			PAGE_SIZE - (app_virt_addr & (~PAGE_MASK));



	/* check the size of the last page */
	if (num_pages > 1) {
		lli_array[num_pages - 1].block_size =
			(app_virt_addr + data_size) & (~PAGE_MASK);

	/*check that the last page is not full*/
	if (lli_array[num_pages - 1].block_size == 0x0)
		lli_array[num_pages - 1].block_size = PAGE_SIZE;
}

	/* set output params acording to the in_out flag */
	if (in_out_flag == 1002) {
		printk(KERN_INFO "sep_lock_user_pages :SEP_DRIVER_IN_FLAG\n");
		sep_context.in_num_pages = num_pages;
		sep_context.in_page_array = page_array;
		sep_context.in_map_array = map_array;
	} else {
		printk(KERN_INFO "sep_lock_user_pages :DMA_FROM_DEVICE\n");
		sep_context.out_num_pages = num_pages;
		sep_context.out_page_array = page_array;
		sep_context.out_map_array = map_array;
	}

	/*for test Apps */

ioctl_params_dma.Multiple_LLIs.nValidLLIs = num_pages;
	printk(KERN_INFO "sep_lock_user_pages :  \
			Total pages = %d \n", num_pages);

	for (i = 0; i < num_pages; i++) {
	ioctl_params_dma.Multiple_LLIs.LLI_workPlans[i].dmaAddress_LLI_WORD0
						= lli_array[i].bus_address;
	ioctl_params_dma.Multiple_LLIs.LLI_workPlans[i].size_LLI_WORD1
						= lli_array[i].block_size;
	printk(KERN_INFO "sep_lock_user_pages :dma page address \
			 %d = %p \n", i, (void *)map_array[i].dma_addr);
	printk(KERN_INFO "sep_lock_user_pages :dma page size \
				%d = %d \n", i, map_array[i].size);

	}


	goto end_function;

end_function_with_error3:

	/* free lli array */
	kfree(lli_array);

end_function_with_error2:

  kfree(map_array);

end_function_with_error1:

	/* free page array */
	kfree(page_array);

end_function:

	printk(KERN_INFO "SEP Driver:<-------- sep_lock_user_pages end\n");
	return error;
}

/*
this function handles the request for creation of the DMA table
for the synchronic symmetric operations (AES,DES,HASH).
it returns the physical addresses of the created DMA table to the
user space which insert them as a parameters to the HOST-SEP message.
this pointers are NOT being treated by the user application in any case.
*/
static int sep_create_sync_dma_tables_handler(void *arg)
{
/* error */
int   error;

	/* command arguments */
	/*struct sep_driver_build_sync_table_t command_args; */


	/*------------------------
	CODE
	--------------------------*/

printk(KERN_INFO "SEP Driver: sep_create_sync_dma_tables_handler start\n");

	if (copy_from_user(&ioctl_params_dma,
			arg,
			sizeof(struct ioctl_params_dma))) {
			error = -EFAULT;
			goto end_function;
	}


	/* validate user parameters */
	if (!ioctl_params_dma.userSpaceVirtualAddress) {

		printk(KERN_ERR "SEP Driver: params validation error\n");

		error = -EINVAL;
		goto end_function;
	}

	error = sep_lock_user_pages(
		(u32)ioctl_params_dma.userSpaceVirtualAddress,
		ioctl_params_dma.memSize,
		ioctl_params_dma.DstRsrcLLI);

	if (error) {
		printk(KERN_ERR "SEP Driver: sep_lock_user_pages failed\n");
		goto end_function;
	}

	/* copy to user */
	if (copy_to_user(arg,
			(void *)&ioctl_params_dma,
			sizeof(struct ioctl_params_dma))) {
		printk(KERN_ERR "SEP Driver: copy_to_user failed\n");
		error = -EFAULT;
	}


end_function:

printk(KERN_INFO "SEP Driver: sep_create_sync_dma_tables_handler end\n");
	return error;
}

/* this function frees all preallocated dma resources */
static int sep_free_dma_table_data_handler(void *arg)
{
	int count;
	int error = 0 ;

	/*-------------------------
	CODE
	-----------------------------*/

printk(KERN_INFO "SEP Driver: sep_free_dma_table_data_handler start\n");

	/* unmap and free input map array */
	if (sep_context.in_map_array) {
		for (count = 0; count < sep_context.in_num_pages; count++) {
			dma_unmap_page(sep_context.dev_ptr ,
				sep_context.in_map_array[count].dma_addr,
				sep_context.in_map_array[count].size,
				/*DMA_TO_DEVICE*/DMA_BIDIRECTIONAL);
		}

		kfree(sep_context.in_map_array);
	}



	/* unmap output map array, DON'T free it yet */
	if (sep_context.out_map_array) {
		for (count = 0; count < sep_context.out_num_pages; count++) {
			dma_unmap_page(sep_context.dev_ptr ,
				sep_context.out_map_array[count].dma_addr,
				sep_context.out_map_array[count].size,
				/*DMA_FROM_DEVICE*/DMA_BIDIRECTIONAL);
		}

		kfree(sep_context.out_map_array);
	}

	/* free page cache for output */
	if (sep_context.in_page_array) {
		for (count = 0; count < sep_context.in_num_pages; count++) {
			flush_dcache_page(sep_context.in_page_array[count]);
			page_cache_release(sep_context.in_page_array[count]);
		}

		kfree(sep_context.in_page_array);

	}

	if (sep_context.out_page_array) {
		for (count = 0; count < sep_context.out_num_pages; count++) {
			if (!PageReserved(sep_context.out_page_array[count]))
			   SetPageDirty(sep_context.out_page_array[count]);
			flush_dcache_page(sep_context.out_page_array[count]);
			page_cache_release(sep_context.out_page_array[count]);
		}

		kfree(sep_context.out_page_array);
	}

	/* reset all the values */
	sep_context.in_page_array = 0;
	sep_context.out_page_array = 0;
	sep_context.in_num_pages = 0;
	sep_context.out_num_pages = 0;
	sep_context.in_map_array = 0;
	sep_context.out_map_array = 0;

printk(KERN_INFO "SEP Driver: sep_free_dma_table_data_handler end\n");
return error;
}

static int __devinit sep_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *res;

	/*--------------------------
	CODE
	------------------------------*/

	ret_val = 0;

	printk(KERN_INFO "SEP Driver: start sep_probe\n");


	/* get the resource of I/O memory */
	res = platform_get_resource_byname(&sep_device,
				IORESOURCE_MEM, SEP_MEM_RESOURCE_NAME);
	if (!res) {
		printk(KERN_INFO "sep_driver:get resource-data failed\n");
		ret_val = -ENOMEM;
	}

	/* remap the io memory region to the kernel addresses */
	printk(KERN_INFO "SEP Driver: res->start = 0x%08x.\n", res->start);
	printk(KERN_INFO "SEP Driver: resource_size(res) \
			 = 0x%08zx.\n", resource_size(res));
	sep_context.reg_addr = ioremap_nocache(
		res->start, resource_size(res));
	if (!sep_context.reg_addr) {
		printk(KERN_INFO "sep_driver:io memory remap failed\n");
		ret_val = -ENOMEM;
	}

	return ret_val;
}


/*----------------------------------------------------------------------
  open function of the character driver - initializes the private data
------------------------------------------------------------------------*/
static int sep_open(struct inode *inode_ptr, struct file *file_ptr)
{/* return value */
	int			error;

	/*-----------------
	CODE
	---------------------*/

	error = 0;

	printk(KERN_INFO "SEP Driver:--------> open start\n");

	/* fatal error check */
	if (sep_fatal) {
		error = -EBUSY;
		printk(KERN_INFO "detect fatal error in sep_open\n");
		goto end_function;
	}

	/* user count check */
	if (!atomic_dec_and_test(&sep_context.openable)) {
		atomic_inc(&sep_context.openable);
		error = -EBUSY;
		printk(KERN_INFO "user has already exists\n");
		goto end_function;
	}

	/* init the private data flag */
	file_ptr->private_data = (void *)SEP_DRIVER_DISOWN_LOCK_FLAG;

	printk(KERN_INFO "SEP Driver:<-------- open end\n");

end_function:

	return error;
}


/*------------------------------------------------------------
	release function
-------------------------------------------------------------*/
static int sep_release(struct inode *inode_ptr, struct file *file_ptr)
{
	/*-----------------
	CODE
	---------------------*/

	printk(KERN_INFO "SEP Driver:--------> sep_release start\n");


	/* unlock mutex if it is owned */
	if (mutex_is_locked(&sep_context.transaction_mutex))
		mutex_unlock(&sep_context.transaction_mutex);

	/* reset user count */
	atomic_inc(&sep_context.openable);

	printk(KERN_INFO "SEP Driver:<-------- sep_release end\n");

	return 0;
}

/*---------------------------------------------------------------
map function - this functions maps the message shared area
-----------------------------------------------------------------*/
static int sep_mmap(struct file  *filp, struct vm_area_struct  *vma)
{

	/* physical addr */
	dma_addr_t phys_addr;

	/* error */
	int error = 0;

	/*-----------------------
	CODE
	-------------------------*/

	printk(KERN_INFO "SEP Driver:--------> mmap start\n");

	/*check that the size of the mapped range is as the size of the
	message shared area */
	if ((vma->vm_end - vma->vm_start) > SEP_DRIVER_MMMAP_AREA_SIZE) {
		printk(KERN_ERR "SEP Driver mmap requested size \
				is more than allowed\n");
		error = -EAGAIN;
		goto end_function;
	}


	/* get physical address */
	phys_addr  = SEP_IO_MEM_REGION_START_ADDRESS ;

	printk(KERN_INFO "SEP Driver: phys_addr is %08x\n", (u32)phys_addr);

	if (remap_pfn_range(vma,
		vma->vm_start,
		phys_addr >> PAGE_SHIFT,
		vma->vm_end - vma->vm_start,
		pgprot_noncached(vma->vm_page_prot))) {
		printk(KERN_ERR "SEP Driver remap_page_range failed\n");
		error = -EAGAIN;

		goto end_function;
	}


	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot) ;


end_function:

	printk(KERN_INFO "SEP Driver:<-------- mmap end\n");
	return error;
}


void Chip_HwInit(void)
{

	/******************************************
	//
	// enable system module
	//
	*******************************************/
	/* system module-stop control/status register(s) 2 */
	volatile u32 i, *pSRCR2, *smstpcr2, *mstpsr2 = 0 ;

	smstpcr2 = (u32 *)IO_ADDRESS(0xe6150118) ;
	mstpsr2 = (u32 *)IO_ADDRESS(0xe6150040) ;

	/* check status before enabling -- debug*/
	printk(KERN_INFO "\nBEFORE -- module stop status \
			register value = 0x%x\n", __raw_readl(mstpsr2));

	/* enable CC4.2 module */
	i = __raw_readl(smstpcr2) ;
	i &= (~(3<<29)) ;  /* make bits 28 & 29 '0' */
	__raw_writel(i, smstpcr2) ;

	mdelay(20);  /* wait 20 ms */

	/* check status after module enabling -- debug */
	printk(KERN_INFO "\nAFTER -- module stop status \
			register value = 0x%x\n", __raw_readl(mstpsr2));

	/* reset CC4.2 from system domain */
	pSRCR2 = (u32 *)IO_ADDRESS(0xE61580B0);

	/*  generate a reset pulse like
	//     ______
	// ____|    |___________ */

	__raw_writel(0, pSRCR2);
	mdelay(10);
	__raw_writel(1, pSRCR2);
	mdelay(40);
	__raw_writel(0, pSRCR2);
	mdelay(10);

	/* reset CC4.2 chip/hw */
	iowrite32(0x1234, sep_context.reg_addr+0xbec);
	mdelay(20);  /* wait 20 ms */

	/* Initialize/Configure the CC4.2 chip for CRYPTOGRAPHY later*/
	iowrite32(0x1, sep_context.reg_addr+0xbf0);
	iowrite32(0x1, sep_context.reg_addr+0x82c);
	iowrite32(0x1, sep_context.reg_addr+0x850);
	iowrite32(0x0, sep_context.reg_addr+0xe08);
	iowrite32(0x0, sep_context.reg_addr+0xe0C);

	mdelay(20);  /* wait for things to settle down */

	/*read VERSION register to check if chip is responding for IO access*/
	printk(KERN_INFO "CC4.2 HW Version = 0x%08X\n",\
			ioread32(sep_context.reg_addr+0x928));

}


static long sep_ioctl(
					struct file			*filp,
					u32		cmd,
					unsigned long		arg)
{

	/* error */
	long			error = 0;

	printk(KERN_INFO "SEP Driver ioctl : cmd is %x\n", cmd);

	switch (cmd) {

	case SEP_IOCCREATESYMDMATABLE:
		printk(KERN_INFO "SEP Driver:SEP_IOCCREATESYMDMATABLE_CMD\n");
		/* create dma table for synhronic operation */
		error = sep_create_sync_dma_tables_handler((void *)arg);
		break;

	case SEP_IOCFREEDMATABLEDATA:
		 /* free the pages */
		printk(KERN_INFO "SEP Driver:SEP_IOCFREEDMATABLEDATA_CMD\n");
		error = sep_free_dma_table_data_handler((void *)arg);
		break;

	case SEP_HW_INIT:
		printk(KERN_INFO "SEP Driver:SEP_HW_INIT COMMAND\n");
		Chip_HwInit();
		break ;

	default:
		printk(KERN_INFO "SEP Driver: **** NO COMMAND ****\n");
		error = -ENOTTY;
		break;
	}


	printk(KERN_INFO "SEP Driver:<-------- ioctl end\n");
	return error;
}


static void sep_init_context(void)
{
	/*-------------
	CODE
	----------------*/

	/* zero fields */
	sep_context.in_page_array = 0;
	sep_context.out_page_array = 0;
	sep_context.in_num_pages = 0;
	sep_context.out_num_pages = 0;
	sep_context.in_map_array = 0;
	sep_context.out_map_array = 0;

	/* init transaction mutex */
	mutex_init(&sep_context.transaction_mutex);

	/* calculate the shared_area_size */
	sep_context.shared_area_size =
		SEP_DRIVER_MESSAGE_SHARED_AREA_SIZE_IN_BYTES +
		SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES +
		SEP_DRIVER_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES +
		SEP_DRIVER_STATIC_AREA_SIZE_IN_BYTES +
		SEP_DRIVER_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES;

	/* initialize the availablity flag */
	atomic_set(&sep_context.openable, 1);

}

/*--------------------------------------------------------------
init function
----------------------------------------------------------------*/
static int __init sep_init(void)
{

	/* return value */
	int			ret_val;
	printk(KERN_INFO "Inside sep_init function\n");

	/*------------------------
	CODE
	------------------------*/

	printk(KERN_INFO "SEP Driver:-------->Init start\n");

	ret_val = 0;

	/* initialize the context fields */
	sep_init_context();

	ret_val = sep_register_driver_to_device();
	if (ret_val) {
		printk(KERN_ERR "sep_driver:sep_driver_to_device \
			failed,ret_val is %d\n", ret_val);
		goto end_function_unregister_driver_from_device;
	}

	/* register driver to fs */
	ret_val = sep_register_driver_to_fs();
	if (ret_val) {
		printk(KERN_ERR "sep_driver:sep_register_driver_to_fs \
			failed,ret_val is %d\n", ret_val);
		goto end_function_deallocate_sep_shared_area;
	}

	goto end_function;

end_function_deallocate_sep_shared_area:



end_function_unregister_driver_from_device:
	/* un-register from the device model */
	sep_unregister_driver_from_device();

end_function:
	printk(KERN_INFO "SEP Driver:<-------- Init end\n");
	return ret_val;
}

/*-------------------------------------------------------------
  exit function
--------------------------------------------------------------*/
static void __exit sep_exit(void)
{

	/*-----------------------------
	CODE
	--------------------------------*/

	printk(KERN_INFO "SEP Driver:--------> Exit start\n");

	/* un-register from the device model */
	sep_unregister_driver_from_device();

	/* unregister from fs */
	sep_unregister_driver_from_fs();

	printk(KERN_INFO "SEP Driver: free pages SEP SHARED AREA\n");

	printk(KERN_INFO "SEP Driver:<-------- Exit end\n");

}

static struct resource sep_resources[] = {
	{
	.name   = SEP_MEM_RESOURCE_NAME,
	.start  = SEP_IO_MEM_REGION_START_ADDRESS,
	.end    = SEP_IO_MEM_REGION_START_ADDRESS +
			SEP_IO_MEM_REGION_SIZE - 1,
	.flags  = IORESOURCE_MEM,
	},
};

u64 sep_dma_mask = 0xffffffff;
static struct platform_device sep_device = {
	.name           = DRIVER_NAME,
	.dev            = {
	.release        = sep_platform_driver_release,
	.coherent_dma_mask = 0xffffffff,
	.dma_mask =	&sep_dma_mask,
	},
	.resource       = sep_resources,
	.num_resources  = ARRAY_SIZE(sep_resources),
};

static struct platform_driver sep_driver = {
	.probe          = sep_probe,
	.driver         = {
	.name   = DRIVER_NAME,
	.owner  = THIS_MODULE,
	/* .pm     = &sep_pm_ops, */
	},
};



/* file operation for normal sep operations */
static const struct file_operations sep_file_operarions = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = sep_ioctl,
	/*.poll = sep_poll, */
	.open = sep_open,
	.release = sep_release,
	.mmap = sep_mmap,
};




module_init(sep_init);
module_exit(sep_exit);

MODULE_AUTHOR("Renesas Mobile Corp.");
MODULE_DESCRIPTION("CryptoCell5 driver");
MODULE_LICENSE("GPL v2");
