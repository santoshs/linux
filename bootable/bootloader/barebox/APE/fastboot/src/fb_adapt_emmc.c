/* fb_adapt_emmc.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include <flash_api.h>
#include <fb_common.h>
#include "fb_adapt_emmc.h"
#include "fb_dev_mgmr.h"

/*
 * fb_adapt_flash_mount(): Mount flash drive
 * Input
 * 		@param1: UNUSED.
 * Output		None.
 * Return
 * 		FB_OK
 * 		FB_ERR_MOUNT
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_ALREADY_MOUNT
 */
int fb_adapt_flash_mount(void *param1)
{
	int ret = 0;

	/* Flash unmount */
	ret = Flash_Access_Mount(UNUSED);
	if (ret == FLASH_ACCESS_SUCCESS) {
		ret = FB_OK;
	} else if (ret == FLASH_ACCESS_ERR_MOUNT) {
		ret = FB_ERR_MOUNT;
	} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
		ret = FB_ERR_NOT_INIT;
	} else {
		ret = FB_ERR_ALREADY_MOUNT;
	}

	return ret;
}

/*
 * fb_adapt_flash_unmount(): Unmount flash drive
 * Input
 * 		@param1: UNUSED.
 * Output		None.
 * Return
 * 		FB_OK
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_NOT_MOUNT
 */
int fb_adapt_flash_unmount(void *param1)
{
	int ret = 0;

	/* Flash unmount */
	ret = Flash_Access_Unmount(UNUSED);
	if (ret == FLASH_ACCESS_SUCCESS) {
		ret = FB_OK;
	} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
		ret = FB_ERR_NOT_INIT;
	} else {
		ret = FB_ERR_NOT_MOUNT;
	}

	return ret;
}

/*
 * fb_adapt_flash_read(): Read from flash drive
 * Input
 * 		@fb_buff	: pointer to received buffer
 * 		@size		: size of received buffer
 * 		@addr_start	: start address of received buffer
 * 		@param1		: user data.
 * Output		None.
 * Return
 */
int fb_adapt_flash_read(unsigned char *fb_buff, unsigned long size,
							unsigned long long addr_start, void *param1)
{
	int ret = 0;

	/* Flash read */
	ret = Flash_Access_Read(fb_buff, size, addr_start, (unsigned long)param1);
	if (ret == FLASH_ACCESS_SUCCESS) {
		ret = FB_OK;
	} else if (ret == FLASH_ACCESS_ERR_NOT_MOUNT) {
		ret = FB_ERR_NOT_MOUNT;
	} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
		ret = FB_ERR_NOT_INIT;
	} else if (ret == FLASH_ACCESS_ERR_READ) {
		ret = FB_ERR_READ;
	} else {
		ret = FB_ERR_PARAM;
	}

	return ret;
}

/*
 * fb_adapt_flash_write(): Write from flash drive
 * Input
 * 		@fb_buff	: pointer to received buffer
 * 		@size		: size of received buffer
 * 		@addr_start	: start address of received buffer
 * 		@param1		: user data.
 * Output			None.
 * Return
 * 		FB_OK
 * 		FB_ERR_WRITE
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_NOT_MOUNT
 * 		FB_ERR_PARAM
 */
int fb_adapt_flash_write(unsigned char *fb_buff, unsigned long size,
							unsigned long long addr_start, void *param1)
{
	int ret = 0;

	/* Flash write */
	ret = Flash_Access_Write(fb_buff, size, addr_start, (unsigned long)param1);
	if (ret == FLASH_ACCESS_SUCCESS) {
		ret = FB_OK;
	} else if (ret == FLASH_ACCESS_ERR_NOT_MOUNT) {

		ret = FB_ERR_NOT_MOUNT;
	} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {

		ret = FB_ERR_NOT_INIT;
	} else if (ret == FLASH_ACCESS_ERR_WRITE) {

		ret = FB_ERR_WRITE;
	} else {

		ret = FB_ERR_PARAM;
	}

	return ret;
}

/*
 * fb_adapt_flash_erase(): Read data of flash drive
 * Input
 * 		@start_block: start block of data
 * 		@end_block	: end block of data
 * 		@param1		: user data.
 * Output		None.
 * Return
 * 		FB_OK
 * 		FB_ERR_ERASE
 * 		FB_ERR_NOT_INIT
 * 		FB_ERR_NOT_MOUNT
 * 		FB_ERR_PARAM
 */
int fb_adapt_flash_erase(unsigned long start_block,
							unsigned long end_block, void *param1)
{
	int ret = 0;

	/* Flash erase */
	ret = Flash_Access_Erase(start_block, end_block, UNUSED);
	if (ret == FLASH_ACCESS_SUCCESS) {
		ret = FB_OK;
	} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
		ret = FB_ERR_NOT_INIT;
	} else if (ret == FLASH_ACCESS_ERR_NOT_MOUNT) {
		ret = FB_ERR_NOT_MOUNT;
	} else if (ret == FLASH_ACCESS_ERR_PARAM) {
		ret = FB_ERR_PARAM;
	} else {
		ret = FB_ERR_ERASE;
	}

	return ret;
}

/*
 * fb_adapt_flash_format(): Format flash drive
 * Input
 * 		@param1: UNUSED.
 * Output		None.
 * Return
 *		FB_OK
 *		FB_ERR_NOT_INIT
 *		FB_ERR_FORMAT
 *		FB_ERR_ALREADY_MOUNT
 */
int fb_adapt_flash_format(void *param1)
{
	int ret = 0;
	/* Flash unmount */
	ret = Flash_Access_Format(UNUSED);
	if (ret == FLASH_ACCESS_SUCCESS) {
		ret = FB_OK;
	} else if (ret == FLASH_ACCESS_ERR_NOT_INIT) {
		ret = FB_ERR_NOT_INIT;
	} else if (ret == FLASH_ACCESS_ERR_FORMAT) {
		ret = FB_ERR_FORMAT;
	} else {
		ret = FB_ERR_ALREADY_MOUNT;
	}

	return ret;
}

/*
 * flash device operation struct
 */
static const struct t_fb_flash_operation flash_device_operation = {
	.fb_dev_flash_mount		= fb_adapt_flash_mount,
	.fb_dev_flash_unmount	= fb_adapt_flash_unmount,
	.fb_dev_flash_read		= fb_adapt_flash_read,
	.fb_dev_flash_write		= fb_adapt_flash_write,
	.fb_dev_flash_erase		= fb_adapt_flash_erase,
	.fb_dev_flash_format	= fb_adapt_flash_format,
};

/*
 * flash device struct
 */
static struct t_fb_device flash_device = {
	.dev_id			= DEV_ID_EMMC,
	.used_count		= 0,
	.next_dev		= NULL,
	.file_operation	= (void*)&flash_device_operation,
	.init			= flash_init,
	.remove			= flash_remove,
};

/*
 * fb_adap_flash_register()
 * Input		None
 * Output		None
 * Return		None
 */
void fb_adap_flash_register()
{
	fb_dev_register_dev(&flash_device);
}

/*
 * flash_init(): Init flash drive
 * Input		None
 * Output		None
 * Return		None
 */
void flash_init()
{
	/* flash initialize */
	Flash_Access_Init(UNUSED);
}

/*
 * flash_remove()
 * Input		None
 * Output		None
 * Return		None
 */
void flash_remove()
{
	/* T.B.D */
}
