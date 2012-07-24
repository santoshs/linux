/* fb_common.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include <string.h>
#include <fb_common.h>
#include "fb_dev_mgmr.h"
#include "fb_comm.h"

 
static fb_ptentry ptable[MAX_PTN];
static unsigned int pcount = 0;

/*
 * fb_main_detect_addr(): detecting addr_start and size from value
 * Input
 * 		@value		: command value
 * Output
 * 		@addr_start	: address starting
 * 		@size		: size of partition
 * Return
 * 		FB_OK
 *		FB_ERR_UNSUPPORT
 */
int fb_main_detect_addr(unsigned int value, unsigned long long *addr_start,
												unsigned long *size)
{
	fb_ptentry *pte;

	pte = fb_flash_find_ptn(value);
	if (!pte) {
		return FB_ERR_UNSUPPORT;
	}
	*addr_start = pte->addr_start;
	*size       = pte->size;

	return FB_OK;
}

/*
 * Partition entry table
 */
static struct fb_ptentry fb_ptentrys[] = {
		  /* id,		 add32, 	 add32, 	 add64, size(Bytes), flags */
	{ BOOTLOADER,	0x00000000, 0x00040600, 0x00000000,  	 256*1024,		0},	/* barebox.bin	*/
	{ BOOT,			0x00000000, 0x01100000, 0x00000000,  15*1024*1024,		0},	/* boot.img 	*/
	{ SYSTEM,		0x00000000, 0x02000000, 0x00000000, 500*1024*1024,		0},	/* system.img	*/
	{ USERDATA,		0x00000000, 0x21600000, 0x00000000, 640*1024*1024,		0},	/* userdata.img	*/
	{ RECOVERY,		0x00000000, 0x4FA00000, 0x00000000,  12*1024*1024,		0},	/* recovery.img	*/
	{ R_LOADER,		0x00000000, 0x00000200, 0x00000000,      128*1024,		0},	/* r_loader.bin	*/
	{ FASTBOOT,		0x00000000, 0x00080600, 0x00000000,       64*1024,		0},	/* fastboot.bin	*/
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
	{ 0x00,			0x00000000, 0x00000000, 0x00000000, 0*1024,			0},
};


/*
 * fb_flash_add_ptn()
 * Input	:None
 * Output
 *		@ptn: partition entry
 * Return	:None
 */
void fb_flash_add_ptn(fb_ptentry *ptn)
{
	if (pcount < MAX_PTN) {
		memcpy(ptable + pcount, ptn, sizeof(*ptn));
		pcount++;
	}
}

/*
 * fb_flash_find_ptn()
 * Input
 *			@id	: partition id
 * Output		:None
 * Return
 *			@partiton table
 *			@NULL
 */ 
fb_ptentry *fb_flash_find_ptn(unsigned int id)
{
	unsigned int n;

	for (n = 0; n < pcount; n++) {
		if (id == ptable[n].id) {
			return ptable + n;
		}
	}
	return NULL;
}

/*
 * fb_flash_init()
 * Input	:None
 * Output	:None
 * Return
 *		0	: Success
 */
int fb_flash_init() 
{
	struct fb_ptentry pte;
	int i;
 	union {
		unsigned long addr32[2];
		unsigned long long addr64;
	} addr;
	int ret = 0;
	struct t_fb_flash_operation *fl_ops = NULL;
	
	/* Use device */
	fl_ops = fb_dev_use_dev(DEV_ID_EMMC); 
	if (!fl_ops) {
		fb_comm_respond("FAILParameter error ...\n");
	}
	/* Mount to eMMC */
	ret = fl_ops->fb_dev_flash_mount(UNUSED);
	if (ret != FB_OK) {
		fb_comm_respond("FAILMount Fail ...\n");
	}

	for (i = 0; i < MAX_PTN; i++) {
		pte.id = fb_ptentrys[i].id;

		addr.addr32[1] = fb_ptentrys[i].addr_start1;
		addr.addr32[0] = fb_ptentrys[i].addr_start2;
		pte.addr_start = addr.addr64;

		pte.size = fb_ptentrys[i].size;
		pte.flags = 0;

		fb_flash_add_ptn(&pte);
	}

	return 0;
}
