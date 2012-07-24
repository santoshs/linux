/*
 * fb_dev_mgmr.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */	
 
/*
 * fb_dev_mgmr.c
 */

#include <common.h>
#include <sbsc.h>
#include <flash_api.h>
#include <fb_common.h>
#include "fb_dev_mgmr.h"
#include "fb_adapt_emmc.h"
#include "fb_adapt_usb.h"

static struct t_fb_device_manager _tdm;

/*
 * fb_dev_use_dev(): init (first time) and get file operation of used device
 * Input
 * 		@dev_id	: device id
 * Output 		None
 * Return		:file operation
 */
void* fb_dev_use_dev(T_FB_DEVICE_ID dev_id)
{
	struct t_fb_device *dev;
	struct t_fb_device_manager *tdm = &_tdm;
	void *f_opt;
	dev = tdm->top_lst_dev;
	
	while(1) {
		if (!dev) {
			f_opt = NULL;
			break;
		} else if (dev->dev_id != dev_id) {
			dev = dev->next_dev;
			continue;
		}
		/* get file operatiom */
		f_opt = dev->file_operation;
		dev->used_count++;
		if (dev->used_count > 1) {
			break;
		} else if (dev->used_count == 1) {
			/* Init device at first time */
			dev->init();
			break;
		}
	}
	return f_opt;
}

/*
 * fb_dev_release_dev(): release device
 * Input
 * 		@dev_id	: device id
 * Output		:None
 * Return		:None
 */
void fb_dev_release_dev(T_FB_DEVICE_ID dev_id)
{
	struct t_fb_device *dev;
	struct t_fb_device_manager *tdm = &_tdm;

	dev = tdm->top_lst_dev;

	while(1) {
		if (!dev) {
			break;
		}
		/* Find device by dev_id */
		if (dev->dev_id != dev_id) {
			dev = dev->next_dev;
			continue;
		} else if (dev->used_count <= 0) {
			break;
		}

		dev->used_count--;
		/* Remove device if no one used */
		if (dev->used_count == 0) {
			dev->remove();
		}
	}
}

/*
 * fb_dev_register_dev(): register device
 * Input		:None
 * Output
 * 		@reg_dev:
 * Return
 *		FB_OK
 *		FB_ERR_EXISTED
 */
int fb_dev_register_dev(struct t_fb_device *reg_dev)
{
	int ret;
	struct t_fb_device *dev;
	struct t_fb_device_manager *tdm = &_tdm;

	dev = tdm->top_lst_dev;
	if (!dev) {
		if (reg_dev->next_dev != NULL) {
			reg_dev->next_dev = NULL;
		}
		tdm->top_lst_dev = reg_dev;
		ret = FB_OK;
	} else {
		while(1) {
			/* If end of device list*/
			if (dev->next_dev == NULL) {
				if (reg_dev->next_dev != NULL) {
					reg_dev->next_dev = NULL;
				}
				/* Add device */
				dev->next_dev = reg_dev;
				ret = FB_OK;
				break;
			} else if (dev->dev_id != reg_dev->dev_id) {
				dev = dev->next_dev;
				continue;
			}
			ret = FB_ERR_EXISTED;
			break;
		}
	}

	return ret;
}

/*
 * fb_dev_init(): Init dev
 * Input	:None
 * Output	:None
 * Return
 * 		FB_OK
 */
int fb_dev_init()
{
	fb_adap_comm_register();
	fb_adap_flash_register();

	/* Disable WDT */
	WDT_DISABLE();

	/* Set Clock SYS-CPU(Z)=403MHz, SHwy(ZS)=208MHz, Peripheral(HP)=104MHz */
	SetUsbBootClock();

	/* Init SDRAM */
	//SBSC_Init();

	return FB_OK;
}

/*
 * fb_dev_reboot_system(): reboot board
 * Input	:None
 * Output	:None
 * Return	:None
 */
void fb_dev_reboot_system()
{
	/* Set Reset Control Register2 */
	*RESCNT2 |= RESCNT2_PRES;
	while(1){
		/* reset wait */
	}
}
