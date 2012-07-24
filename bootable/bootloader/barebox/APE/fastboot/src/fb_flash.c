/* fb_flash.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 */
 
#include <string.h>
#include <fb_common.h>
#include "fb_flash.h"
#include "fb_dev_mgmr.h"

/*
 * fb_flash_read(): Read data from eMMC
 * Input
 * 		@fb_buff	: pointer to received buffer
 * 		@size		: received size
 * 		@addr_start	: address to store data.
 * Output			:None
 * Return
 * 		FB_OK
 * 		FB_ERR_MOUNT
 * 		FB_ERR_READ
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_NOT_MOUNT
 * 		FB_ERR_PARAM
 */
int fb_flash_read(unsigned char *fb_buff, unsigned long size,
											unsigned long long addr_start)
{
	int ret;
	struct t_fb_flash_operation *fl_ops = NULL;

	/* Use device */
	fl_ops = fb_dev_use_dev(DEV_ID_EMMC);
	if (!fl_ops) {
		return FB_ERR_PARAM;
	}


	ret = fl_ops->fb_dev_flash_read(fb_buff, size, addr_start, UNUSED);

	return ret;
}

/*
 * fb_flash_write():  Write data to eMMC
 * Input
 * 		@fb_buff	:
 * 		@size		:
 * 		@addr_start	:
 * 		@param1		:
 * Output
 * Return
 * 		FB_OK
 * 		FB_ERR_MOUNT
 * 		FB_ERR_WRITE
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_NOT_MOUNT
 * 		FB_ERR_PARAM
 */
int fb_flash_write(unsigned char *fb_buff, unsigned long size,
											unsigned long long addr_start)
{
	int ret;
	unsigned char *pData;
	unsigned long remain;
	struct t_fb_flash_operation *fl_ops = NULL;

	/* Use device */
	fl_ops = fb_dev_use_dev(DEV_ID_EMMC); 
	if (!fl_ops) {
		return FB_ERR_PARAM;
	}
	/* Write data to eMMC */
		pData = fb_buff;
		remain = size;
		do {
			if (remain > WRITE_MAX_SIZE) { /* Max write size is > 32MB - 512 bytes	*/	
				ret = fl_ops->fb_dev_flash_write(pData, WRITE_MAX_SIZE, addr_start, UNUSED);
				if (ret != FB_OK) {
					return ret;
				}
				/* Update for next write turn */
				pData += WRITE_MAX_SIZE;
				remain -= WRITE_MAX_SIZE;
				addr_start += WRITE_MAX_SIZE;
			} else { /* Write data size <= 32MB -512 */
				ret = fl_ops->fb_dev_flash_write(pData, remain, addr_start, UNUSED);
				if (ret != FB_OK) {
					return ret;
				}
				remain = 0;
			}
		} while (remain > 0);
		
	return ret;
}

/*
 * fb_flash_erase():  Erase data of eMMC
 * Input
 * 		@start_block:
 * 		@end_block	:
 * Output
 * Return
 * 		FB_OK
 * 		FB_ERR_MOUNT
 * 		FB_ERR_ERASE
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_NOT_MOUNT
 * 		FB_ERR_PARAM
 */
int fb_flash_erase(unsigned long start_block, unsigned long end_block)
{
	int ret;
	struct t_fb_flash_operation *fl_ops = NULL;
	
	/* Erase data of eMMC */
	ret = fl_ops->fb_dev_flash_erase(start_block, end_block, UNUSED);

	return ret;
}
