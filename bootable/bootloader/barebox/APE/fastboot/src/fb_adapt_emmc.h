/*
 * fb_adapt_emmc.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

 /*
 * fb_adapt_emmc.h
 */
 
#ifndef __FB_ADAPT_EMMC_H__
#define __FB_ADAPT_EMMC_H__

/*******************************
 * Definitions
 *******************************/
 
/*******************************
 * Global data
 *******************************/
  
/*******************************
 * Prototypes
 *******************************/
void flash_init(void);
void flash_remove(void);
int fb_adapt_flash_mount(void *param1);
int fb_adapt_flash_unmount(void *param1);
int fb_adapt_flash_read(unsigned char *pBuff, unsigned long size,
							unsigned long long addr_start, void *param1);
int fb_adapt_flash_write(unsigned char *pBuff, unsigned long size,
							unsigned long long addr_start, void *param1);
int fb_adapt_flash_erase(unsigned long start_block,
							unsigned long end_block, void *param1);
int fb_adapt_flash_format(void *param1);
void fb_adap_flash_register(void);

#endif /* __FB_ADAPT_EMMC_H__ */
