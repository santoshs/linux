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


#ifndef __SEP_DRIVER_API_H__
#define __SEP_DRIVER_API_H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/wait.h>

/*-----------------
    DEFINE
-------------------*/
#define		_IN_
#define		_OUT_
#define		_IN_OUT_


#define SEP_DRIVER_SRC_REPLY    1
#define SEP_DRIVER_SRC_REQ      2
#define SEP_DRIVER_SRC_PRINTF   3
#define		MAX_PAGES_FOR_LLI_WORK_PLAN		256

/*for test application */

struct LLI_workPlan	{
	/* bus/dma/physical address */
	u32 dmaAddress_LLI_WORD0 ;

	/* Bit# 30 denotes last entry. Bit# 31 First entry */
	u32 size_LLI_WORD1 ;

} __attribute__((aligned(32), packed));

struct Multiple_LLIs {
	u32 nValidLLIs;
	struct LLI_workPlan LLI_workPlans[MAX_PAGES_FOR_LLI_WORK_PLAN];
} __attribute__((aligned(32), packed));

struct ioctl_params_dma {
	_IN_		void	*userSpaceVirtualAddress ;
	_IN_		int		memSize ;

	/*As IN param -- caller sets MAX number of pages that can be created */
	/*AS OUT, driver sets the actual number of pages that were created */
	_IN_OUT_	struct	Multiple_LLIs Multiple_LLIs;

	/* DMA from device or to device ? */
	_IN_		int		DstRsrcLLI ;

} __attribute__((aligned(32), packed)) ;

/**
 * @struct sep_dma_map
 *
 *Structure that contains all information needed for mapping the user pages
 *           or kernel buffers for dma operations
 *
 *
 */
struct sep_dma_map {

	/* mapped dma address */
	dma_addr_t    dma_addr;

	/* size of the mapped data */
	size_t        size;
};


/*
    context of the device
*/
struct device_context {

	/* address of the shared memory allocated during init for SEP driver */
	void  *shared_area_virt_addr;

	/* shared area size */
	u32   shared_area_size;

	/* major and minor device numbers */
	dev_t   device_number;

	/* cdev struct of the driver */
	struct cdev			cdev;


	/* mutex for locking access to SEP from different threads */
	struct mutex		transaction_mutex;


	/* start address of the access to the SEP registers from driver */
	void __iomem *reg_addr;

	/* device pointer, used for DMA APIs - initialized either as
	platform device (default), or according to the type of the SeP
	on the platform (PCI etc' ) */
	struct device *dev_ptr;

	/* array of pointers to the pages that represent
	input data for the synchronic DMA action */
	struct page		**in_page_array;

	/* array of pointers to the pages that represent out
	data for the synchronic DMA action */
	struct page		**out_page_array;

	/* number of pages in the sep_in_page_array */
	u32			in_num_pages;

	/* number of pages in the sep_out_page_array */
	u32					out_num_pages;

	/* map array of the input data */
	struct sep_dma_map  *in_map_array;

	/* map array of the output data */
	struct sep_dma_map  *out_map_array;


	/* whether or not the device-special-file can be opened */
	atomic_t openable;

	/* this parameter indicates resource status */
	/* 0: power-off now, and it's going to power-up. */
	/* 1: power-on now, and it's going to power-down. */
	atomic_t resource;


};

	/*
	  structure that represent one entry in the DMA LLI table
	*/
	struct sep_lli_entry_t {
	/* physical address */
	u32  bus_address;

	/* block size */
	u32  block_size;
};


/* DEBUG LEVEL MASKS */
#define SEP_DEBUG_LEVEL_BASIC       0x1

#define SEP_DEBUG_LEVEL_REGISTERS   0x2

#define SEP_DEBUG_LEVEL_EXTENDED    0x4

/* fatal error indicator values */
#define SEP_FATAL_REQ_DAEMON_TERMINATED	1
#define SEP_FATAL_RESOURCE_ERROR		2

/* FUNCTIONAL MACROS */

#define dbg(fmt, args...) \
do {\
	if (sep_debug & SEP_DEBUG_LEVEL_BASIC) \
		printk(KERN_DEBUG fmt, ##args); \
} while (0);

#define edbg(fmt, args...) \
do { \
	if (sep_debug & SEP_DEBUG_LEVEL_EXTENDED) \
		printk(KERN_DEBUG fmt, ##args); \
} while (0);


/*----------------------------------------------------------------
  IOCTL command defines
  -----------------------------------------------------------------*/

/* magic number 1 of the sep IOCTL command */
#define SEP_IOC_MAGIC_NUMBER                           's'

/* sends interrupt to sep that message is ready */
#define SEP_IOCSENDSEPCOMMAND     \
	_IO(SEP_IOC_MAGIC_NUMBER, 0)

/* create sym dma lli tables */
#define SEP_IOCCREATESYMDMATABLE      \
	_IOW(SEP_IOC_MAGIC_NUMBER, 5, struct ioctl_params_dma)


/* free dynamic data aalocated during table creation */
#define SEP_IOCFREEDMATABLEDATA     \
	_IOW(SEP_IOC_MAGIC_NUMBER , 7, struct ioctl_params_dma)

#define SEP_IOCSTARTTRANSACTION      \
	_IO(SEP_IOC_MAGIC_NUMBER , 35)

#define SEP_HW_INIT      \
	_IO(SEP_IOC_MAGIC_NUMBER , 70)

#endif
