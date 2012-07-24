/*
 * fb_flash.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 */	
 
/*
 * fb_flash.h
 */

#ifndef __FB_FLASH_H__
#define __FB_FLASH_H__

/*******************************
 * Prototypes
 *******************************/
int fb_flash_read(unsigned char *fb_buff, unsigned long size,
											unsigned long long addr_start);
int fb_flash_write(unsigned char *fb_buff, unsigned long size,
											unsigned long long addr_start);
int fb_flash_erase(unsigned long start_block, unsigned long end_block);

#endif	/* __FB_FLASH_H__ */
