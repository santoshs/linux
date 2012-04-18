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
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc., 59
*  Temple Place - Suite 330, Boston, MA  02111-1307, USA
**********************************************************/


#ifndef __SEP_DRIVER_CONFIG_H__
#define __SEP_DRIVER_CONFIG_H__


/*--------------------------------------
  DRIVER CONFIGURATION FLAGS
  -------------------------------------*/

/* if flag is on , then the driver is running in polling and
	not interrupt mode */
#define SEP_DRIVER_POLLING_MODE                         0

/* flag which defines if the shared area address should be
	reconfiged (send to SEP anew) during init of the driver */
#define SEP_DRIVER_RECONFIG_MESSAGE_AREA                0

/* the mode for running on the ARM1172 Evaluation platform (flag is 1) */
#define SEP_DRIVER_ARM_DEBUG_MODE                       0

/* the mode for running on the PowerPC Evaluation platform (flag is 1) */
#define SEP_DRIVER_PPC_DEBUG_MODE                       0

/* RAR lock mode - cahce, resident and ext cache must be copied to
   external RAR and locked by driver in the same API */
#define SEP_DRIVER_LOCK_RAR_MODE                        0

/* flag which defines if the SEP driver should config the OTP (flag is 1),
   or not (flag is 0) */
#define SEP_DRIVER_CONFIG_OTP                           0

/*-------------------------------------------
	INTERNAL DATA CONFIGURATION
	-------------------------------------------*/

/* the dirver name */
#define DRIVER_NAME "sep_sec_driver"

/* the memory-resource name */
#define SEP_MEM_RESOURCE_NAME "sep_driver_mem"

/* the irq-resource name */
#define SEP_IRQ_RESOURCE_NAME "sep_driver_irq"

/* flag for the input array */
#define SEP_DRIVER_IN_FLAG                              0

/* flag for output array */
#define SEP_DRIVER_OUT_FLAG                             1

/* maximum number of entries in one LLI tables */
#define SEP_DRIVER_ENTRIES_PER_TABLE_IN_SEP             31

/* minimum data size of the MLLI table */
#define SEP_DRIVER_MIN_DATA_SIZE_PER_TABLE		16

/* flag that signifies tah the lock is
currently held by the proccess (struct file) */
#define SEP_DRIVER_OWN_LOCK_FLAG                        1

/* flag that signifies tah the lock is currently NOT
held by the proccess (struct file) */
#define SEP_DRIVER_DISOWN_LOCK_FLAG                     0

/* indicates whether driver has mapped/unmapped shared area */
#define SEP_REQUEST_DAEMON_MAPPED			1
#define SEP_REQUEST_DAEMON_UNMAPPED			0

/*--------------------------------------------------------
	SHARED AREA  memory total size is 36K
	it is divided is following:

	SHARED_MESSAGE_AREA                     8K         }
									}
	STATIC_POOL_AREA                        4K         } MAPPED AREA (28K)
									}
	DATA_POOL_AREA                          16K        }

	SYNCHRONIC_DMA_TABLES_AREA              5K

	SYSTEM_MEMORY_AREA                      3k

	SYSTEM_MEMORY total size is 3k
	it is divided as following:

	TIME_MEMORY_AREA                     8B

	RAR_MEMORY_AREA                      8B

    CURRENT_CALLER_ID_MEMORY             70B
-----------------------------------------------------------*/



/*
	the maximum length of the message - the rest of the message shared
	area will be dedicated to the dma lli tables
*/
#define SEP_DRIVER_MAX_MESSAGE_SIZE_IN_BYTES                  (8 * 1024)

/* the size of the message shared area in pages */
#define SEP_DRIVER_MESSAGE_SHARED_AREA_SIZE_IN_BYTES          (8 * 1024)

/* the size of the data pool static area in pages */
#define SEP_DRIVER_STATIC_AREA_SIZE_IN_BYTES                  (4 * 1024)

/* the size of the data pool shared area size in pages */
#define SEP_DRIVER_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES        (16 * 1024)

/* the size of the message shared area in pages */
#define SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES   (1024 * 5)

/* system data (time, caller id etc') pool */
#define SEP_DRIVER_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES           (1024 * 3)

/* the size in bytes of the time memory */
#define SEP_DRIVER_TIME_MEMORY_SIZE_IN_BYTES                  8

/* the size in bytes of the RAR parameters memory */
#define SEP_DRIVER_SYSTEM_RAR_MEMORY_SIZE_IN_BYTES            8

/* area size that is mapped  - we map the MESSAGE AREA, STATIC POOL and
	DATA POOL areas. area must be module 4k */
#define SEP_DRIVER_MMMAP_AREA_SIZE                            (1024 * 28)


/*-----------------------------------------------
	offsets of the areas starting from the shared area start address
*/

/* message area offset */
#define SEP_DRIVER_MESSAGE_AREA_OFFSET_IN_BYTES               0

/* static pool area offset */
#define SEP_DRIVER_STATIC_AREA_OFFSET_IN_BYTES \
		(SEP_DRIVER_MESSAGE_SHARED_AREA_SIZE_IN_BYTES)

/* data pool area offset */
#define SEP_DRIVER_DATA_POOL_AREA_OFFSET_IN_BYTES \
	(SEP_DRIVER_STATIC_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_STATIC_AREA_SIZE_IN_BYTES)

/* synhronic dma tables area offset */
#define SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_OFFSET_IN_BYTES \
	(SEP_DRIVER_DATA_POOL_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES)

/* system memory offset in bytes */
#define SEP_DRIVER_SYSTEM_DATA_MEMORY_OFFSET_IN_BYTES \
	(SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_OFFSET_IN_BYTES + \
	SEP_DRIVER_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES)

/* offset of the time area */
#define SEP_DRIVER_SYSTEM_TIME_MEMORY_OFFSET_IN_BYTES \
	(SEP_DRIVER_SYSTEM_DATA_MEMORY_OFFSET_IN_BYTES)

/* offset of the RAR area */
#define SEP_DRIVER_SYSTEM_RAR_MEMORY_OFFSET_IN_BYTES \
	(SEP_DRIVER_SYSTEM_TIME_MEMORY_OFFSET_IN_BYTES + \
	SEP_DRIVER_TIME_MEMORY_SIZE_IN_BYTES)

/* offset of the caller id area */
#define SEP_DRIVER_SYSTEM_CALLER_ID_MEMORY_OFFSET_IN_BYTES \
	(SEP_DRIVER_SYSTEM_RAR_MEMORY_OFFSET_IN_BYTES + \
	SEP_DRIVER_SYSTEM_RAR_MEMORY_SIZE_IN_BYTES)


/* start physical address of the SEP registers memory in HOST */
/*#if SEP_DRIVER_PPC_DEBUG_MODE
  #define SEP_IO_MEM_REGION_START_ADDRESS                       0x83F00000
#else
  #define SEP_IO_MEM_REGION_START_ADDRESS                       0xA2000000
#endif*/
#define SEP_IO_MEM_REGION_START_ADDRESS                       0xe6a20000

/*size of the SEP registers memory region  in HOST (for now 100 registers) */
/* #define SEP_IO_MEM_REGION_SIZE                                (0x10000) */
#define SEP_IO_MEM_REGION_SIZE                                (4*1024)


/* maximum number of add buffers */
#define SEP_MAX_NUM_ADD_BUFFERS                               100

/* number of flows */
#define SEP_DRIVER_NUM_FLOWS                                  4

/* maximum number of entries in flow table */
#define SEP_DRIVER_MAX_FLOW_NUM_ENTRIES_IN_TABLE              25

/* offset of the num entries in the block length entry of the LLI */
#define SEP_NUM_ENTRIES_OFFSET_IN_BITS                        24

/* offset of the interrupt flag in the block length entry of the LLI */
#define SEP_INT_FLAG_OFFSET_IN_BITS                           31

/* mask for extracting data size from LLI */
#define SEP_TABLE_DATA_SIZE_MASK                              0xFFFFFF

/* mask for entries after being shifted left */
#define SEP_NUM_ENTRIES_MASK                                  0x7F

/* maximum number of concurrent virtual buffers */
#define SEP_MAX_VIRT_BUFFERS_CONCURRENT                       100

/* the token that defines the start of time address */
#define SEP_TIME_VAL_TOKEN                                    0x12345678

/* size of the caller id hash (sha2) */
#define SEP_CALLER_ID_HASH_SIZE_IN_BYTES                      32

/* maximum number of entries in the caller id table */
#define SEP_CALLER_ID_TABLE_NUM_ENTRIES                       20

/* the token that defines the start of time address */
#define SEP_RAR_VAL_TOKEN                                     0xABABABAB

/* ioctl error that should be returned when trying
   to realloc the cache/resident second time */
#define SEP_ALREADY_INITIALIZED_ERR                           12

/* bit that locks access to the shared area */
#define SEP_MMAP_LOCK_BIT                                     0

/* bit that lock access to the poll  - after send_command */
#define SEP_SEND_MSG_LOCK_BIT                                 1


/* fatal error indicator values */
#define SEP_FATAL_REQ_DAEMON_TERMINATED	1
#define SEP_FATAL_RESOURCE_ERROR		2

/* FUNCTIONAL MACROS */





#endif /*__SEP_DRIVER_CONFIG_H__*/
