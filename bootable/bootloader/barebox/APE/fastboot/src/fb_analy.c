/*
 * fb_analy.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */	
 
/*
 * fb_analy.c
 */
 
#include <string.h>
#include <fb_common.h>
#include "fb_analy.h"

/*
 * fb_analy_analy_cmd(): use to check command support or not
 * Input
 * 		@fb_buff	: pointer to received budder
 * Output
 * 		@code		:command
 * 		@value		:command parameter
 * @Return: 
 *     	FB_OK
 *     	FB_ERR_UNSUPPORT
 */
int fb_analy_analy_cmd(unsigned char *fb_buff, unsigned int *code,
											unsigned int *value)
{
	int ret = FB_OK;
	const char *cmdbuf = (char *)fb_buff;
 	
 	if(memcmp(cmdbuf, "getvar:", 7) == 0){
		*code  = GETVAR;
		if(memcmp(cmdbuf + 7, "version\0", 8) == 0){
			*value = VERSION;
		}else if(memcmp(cmdbuf + 7, "product\0", 8) == 0){
			*value = PRODUCT;
		}else if(memcmp(cmdbuf + 7, "cpurev\0", 7) == 0){
			*value = CPUREV;
		}else if(memcmp(cmdbuf + 7, "downloadsize\0", 13) == 0){
			*value = DOWNLOADSIZE;
		}else if(memcmp(cmdbuf + 7, "version-bootloader\0", 19) == 0){
			*value = VERSION_BOOTLOADER;
		}else if(memcmp(cmdbuf + 7, "version-baseband\0", 17) == 0){
			*value = VERSION_BASEBAND;
		}else if(memcmp(cmdbuf + 7, "serialno\0", 9) == 0){
			*value = SERIALNO;
		}else{
			ret = FB_ERR_UNSUPPORT;
		}
	}else if((memcmp(cmdbuf, "reboot\0", 7) == 0) ||
			 (memcmp(cmdbuf, "reboot-bootloader\0", 18) == 0)){
		*code = REBOOT;
	}else if(memcmp(cmdbuf, "download:", 9) == 0){
		*code  = DOWNLOAD;
		*value = simple_strtoul(cmdbuf + 9, NULL, 16);
	}else if(memcmp(cmdbuf, "boot\0", 5) == 0){
		*code  = BOOT;
	}else if(memcmp(cmdbuf, "flash:", 6) == 0){
		*code = FLASH;
		if(memcmp(cmdbuf + 6, "r_loader\0", 8) == 0){
			*value = R_LOADER;
		}else if(memcmp(cmdbuf + 6, "fastboot\0", 9) == 0){
			*value = FASTBOOT;
		}else if(memcmp(cmdbuf + 6, "bootloader\0", 11) == 0){
			*value = BOOTLOADER;
		}else if(memcmp(cmdbuf + 6, "boot\0",5) == 0){
			*value = BOOT;
		}else if(memcmp(cmdbuf + 6, "system\0", 7) == 0){
			*value = SYSTEM;
		}else if(memcmp(cmdbuf + 6, "userdata\0", 9) == 0){
			*value = USERDATA;
		}else if(memcmp(cmdbuf + 6, "recovery\0", 9) == 0){
			*value = RECOVERY;
		}else{
			ret = FB_ERR_UNSUPPORT_PART;
		}
	}else if(memcmp(cmdbuf, "erase:", 6) == 0){
		*code = ERASE;
		if(memcmp(cmdbuf + 6, "r_loader\0", 9) == 0){
			*value = R_LOADER;
		}else if(memcmp(cmdbuf + 6, "fastboot\0", 9) == 0){
			*value = FASTBOOT;
		}else if(memcmp(cmdbuf + 6, "bootloader\0",11) == 0){
			*value = BOOTLOADER;
		}else if(memcmp(cmdbuf + 6, "boot\0",5) == 0){
			*value = BOOT;
		}else if(memcmp(cmdbuf + 6, "system\0", 7) == 0){
			*value = SYSTEM;
		}else if(memcmp(cmdbuf + 6, "userdata\0", 9) == 0){
			*value = USERDATA;
		}else if(memcmp(cmdbuf + 6, "recovery\0", 9) == 0){
			*value = RECOVERY;
		}else{
			ret = FB_ERR_UNSUPPORT_PART;
		}
	}else{
		ret = FB_ERR_UNSUPPORT;
		}
	
	return ret;
}
