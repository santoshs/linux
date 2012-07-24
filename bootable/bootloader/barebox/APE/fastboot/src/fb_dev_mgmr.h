/*
 * fb_dev_mgmr.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */	
 
/*
 * fb_dev_mgmr.h
 */
#ifndef __FB_DEV__MGMR_H__
#define __FB_DEV__MGMR_H__

/*******************************
 * Definitions
 *******************************/ 

/*
 * Device ID type
 */
typedef enum t_fb_device_id {
	DEV_ID_USB = 0,
	DEV_ID_EMMC
} T_FB_DEVICE_ID;

/*
 * Module ID type
 */
typedef enum t_fb_module_id {
	FB_ID_MAIN = 0,
	FB_ID_COMM,
	FB_ID_FLASH,
	FB_ID_DEV,
	FB_ID_ANALY
} T_FB_MODULE_ID;

/*
 * Communication state type
 */
typedef enum t_fb_dev_comm_state {
	DEV_COMM_STA_OPEN = 0,
	DEV_COMM_STA_CLOSE,
	DEV_COMM_STA_RECV,
	DEV_COMM_STA_SEND,
	DEV_COMM_STA_DISC
} T_FB_DEV_COMM_STATE;

/*
 * Device structure
 */
struct t_fb_device {
	T_FB_DEVICE_ID	dev_id;			/*! ID of current device */
	int				used_count;		/*! Number of used time of device */
	struct t_fb_device	*next_dev;	/*! Pointer to next device object */
	void			*file_operation;/*! File operation */
	void			(*init)(void);	/*! Init function */
	void			(*remove)(void);	/*! Remove function */
};

/*
 * Device manage structure
 */
struct t_fb_device_manager {
	struct t_fb_device *top_lst_dev;		/*! Top of device list */
};

/*
 * File operation for communication devices
 */
struct t_fb_comm_operation {
	int (*fb_dev_comm_init)(unsigned char *param1);	/*! Init device */
	int (*fb_dev_comm_open)(void *param1);	/*! Open device */
	int (*fb_dev_comm_close)(void *param1);	/*! Close device */

	/*! Get current state of device */
	int (*fb_dev_comm_state)(void *param1);

	/*! Receive data */
	int (*fb_dev_comm_receive)(unsigned char *fb_buff, unsigned long size,
										unsigned long timeout, void *param1);

	/*! Send data */
	int (*fb_dev_comm_send)(unsigned char *fb_buff, unsigned long size,
										unsigned long timeout, void *param1);
};

/*
 * File operation for flash devices
 */
struct t_fb_flash_operation {
	/*! Mount flash device */
	int (*fb_dev_flash_mount)(void *param1);

	/*! Unmount flash device */
	int (*fb_dev_flash_unmount)(void *param1);
	/*! Read data from flash device */
	int (*fb_dev_flash_read)(unsigned char *fb_buff, unsigned long size,
								unsigned long long addr_start, void *param1);
	/*! Write data to flash device */
	int (*fb_dev_flash_write)(unsigned char *fb_buff, unsigned long size,
								unsigned long long addr_start, void *param1);
	/*! Erase data in flash device */
	int (*fb_dev_flash_erase)(unsigned long start_block,
								unsigned long end_block, void *param1);
	/*! Formate the flash device */
	int (*fb_dev_flash_format)(void *param1);
};

/*
 * Function for registering a device
 */
int fb_dev_init(void);
void* fb_dev_use_dev(T_FB_DEVICE_ID dev_id);
void fb_dev_release_dev(T_FB_DEVICE_ID dev_id);
int fb_dev_register_dev(struct t_fb_device *reg_dev);
void fb_dev_reboot_system(void);

#endif /* __FB_DEV__MGMR_H__ */
