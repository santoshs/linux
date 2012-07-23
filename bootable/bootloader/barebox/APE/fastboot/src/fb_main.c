/* fb_main.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include <common.h>
#include <string.h>

#include <usb_api.h>
#include <fb_common.h>
#include "fb_comm.h"
#include "fb_analy.h"
#include "fb_flash.h"
#include "fb_dev_mgmr.h"
#include "simg2img.h"

extern unsigned long gTransferCnt;
unsigned long	g_size;
void Wait(ulong loop);

static const char MODE_STRING_FASTBOOT[]	= {"fastboot"}; 

/*
 * fb_main_exec_cmd(): execute commands send by Host
 * Input
 * 		@fb_buff: pointer to received buffer
 * 		@code	: command [0x01,..,0x07]
 * 		@value	: command parameter [0x01,..,0x0B]
 * Output		:None
 * Return		:None
 */
void fb_main_exec_cmd(unsigned char *fb_buff, unsigned int code,
												unsigned int value)
{
	int					ret = 0;
	char				respond[64] = "";	/* buffer for sending message */
	unsigned long long	addr_start = 0;		/* address starting */
	unsigned long 		p_size = 0;			/* size of partition */
	unsigned long		erase_size = 0;		/* erasing size (temp) */
	unsigned long		size_e = 0;			/* erasing size */
	unsigned long		offset = 0;
	struct t_fb_flash_operation *fl_ops = NULL;
	
	switch (code) {
	case GETVAR:				/* get information */
	{
		switch (value) {
		case VERSION:
		{
			snprintf(respond,9,"OKAY%s",FB_VERSION);
			fb_comm_respond(respond);
			break;
		}
		case PRODUCT:
		{
			snprintf(respond,12,"OKAY%s",FB_PRODUCT);
			fb_comm_respond(respond);
			break;
		}
		case CPUREV:
		{
			snprintf(respond,8,"OKAY%s",FB_CPUREV);
			fb_comm_respond(respond);
			break;
		}
		case DOWNLOADSIZE:
		{
			snprintf(respond,12,"OKAY%s",FB_DOWNLOADSIZE);
			fb_comm_respond(respond);
			break;
		}
		case VERSION_BOOTLOADER:
		{
			snprintf(respond,9,"OKAY%s", BOOTLOADER_VERSION);
			fb_comm_respond(respond);
			break;
		}
		case VERSION_BASEBAND:
		{
			snprintf(respond,9,"OKAY%s", BASEBAND_VERSION);
			fb_comm_respond(respond);
			break;
		}
		case SERIALNO:
		{
			snprintf(respond,14,"OKAY%s", SERIAL);
			fb_comm_respond(respond);
			break;
		}
		default:
			break;
		}
		break;
	}
	case BOOT:					/* boot kernel, not flash */
	{
		fb_comm_respond("FAILFunction is unsupported");
		break;
	}
	case REBOOT:				/* reboot */
	case REBOOT_BOOTLOADER:		/* reboot */
	{

		fl_ops = fb_dev_use_dev(DEV_ID_EMMC); 
		/* Unmount */
		fl_ops->fb_dev_flash_unmount(UNUSED);
		/* Release device */
		fb_dev_release_dev(DEV_ID_EMMC);
		fb_comm_respond("OKAY");
		Wait(5); /* Wait a moment for finishing sending to host */
		fb_dev_reboot_system();
		break;
	}
	case DOWNLOAD:				/* download and store data to SDRam*/
	{
		g_size = value;
		snprintf(respond, 13, "DATA%x", g_size);
		fb_comm_respond(respond);
		/* get data send by Host and store to fb_buff */
		usb_skip_startpos();
		gTransferCnt = 1; /* change to receive data packet */		
		ret = fb_comm_get_data(fb_buff, g_size, 0);
		gTransferCnt = 0; /* change to receive command packet */
		if (ret < 0) {
			fb_comm_respond("FAILDownload failed");
		} else {
			fb_comm_respond("OKAY");
		}
		break;
	}
	case FLASH:					/* Write to eMMC */
	{

		ret = fb_main_detect_addr(value, &addr_start, &p_size);
		if (ret != FB_OK) {
			fb_comm_respond("FAILTarget is unsupported");
			break;
		}
		if (g_size > p_size) {
			fb_comm_respond("FAILFile size lager than target");
			break;
		}
		if(value == SYSTEM || value == USERDATA) {
			if(check_sparse_form(fb_buff + 64) == 1) {
				fb_comm_respond("INFOSparsing image");
				ret = sparse(fb_buff + 64, addr_start);
				if(ret == SPARSE_OK) {
					fb_comm_respond("OKAY");
				} else {
					fb_comm_respond("FAILSparsing error");
				}
			} else { /* Not sparseform file */
				fb_comm_respond("FAILFile provided not in sparse form");
			}
		} else {
			/* write to eMMC without sparsing */
			ret = fb_flash_write(fb_buff + MAX_LEN_CMD, g_size, addr_start);
			if (ret != FB_OK) {
				fb_comm_respond("FAILFlash fail!");
			} else {
				fb_comm_respond("OKAY");
			}
		}
		break;
	
	}
	case ERASE:					/* Erase from eMMC*/
	{
		/* detect addr_start and p_size from value */
		ret = fb_main_detect_addr(value, &addr_start, &p_size);
		if (ret != FB_OK) {
			fb_comm_respond("FAILTarget is unsupported");
			break;
		}
		size_e = p_size;
		/* Write 0xFF to the 1MB buffer which will be erased */
		memset(fb_buff + MAX_LEN_CMD, 0xFF, WRITE_MAX_SIZE);

		do {
			erase_size = (size_e > WRITE_MAX_SIZE) ? WRITE_MAX_SIZE : size_e;
			ret = fb_flash_write(fb_buff + MAX_LEN_CMD, erase_size, addr_start + offset);
			if (ret != FB_OK) {
					fb_comm_respond("FAILCan not erase Flash");
					break;
			}
			size_e -= erase_size;
			offset += erase_size;

		} while (size_e > 0); /* ERASE completed */

		fb_comm_respond("OKAY");
		break;
	}

	default:
		break;
	}
}

/*
 * fb_init()	: Initialization for all modules
 * Input		:None
 * Output		:None
 * Return		:None
 */
void fb_init(void)
{
	fb_dev_init();		/* Init for Device manager module */ 
	fb_comm_init();		/* Init for Communication module */
	fb_flash_init();	/* Init for Flash manipulate module */
}

/*
 * fb_check_nvm()	: Check and clear NVM flag
 * Input		:None
 * Output		:None
 * Return		:None
 */
void fb_check_nvm(void)
{
	int ret;
	uchar nvm_flag[BOOTFLAG_SIZE];
	struct t_fb_flash_operation *fl_ops = NULL;
	
	fl_ops = fb_dev_use_dev(DEV_ID_EMMC); 
	
	ret = fl_ops->fb_dev_flash_read((uchar*)(&nvm_flag), BOOTFLAG_SIZE, (uint64)BOOTFLAG_ADDR, NULL);
	if (FB_OK != ret) {
		return;
	}
	
	/* check the boot flag */
	if (strncmp(((const char *)(nvm_flag)), MODE_STRING_FASTBOOT, BOOTFLAG_SIZE) == 0) {
		/* erase NVM boot flag */
		memset((void *)&nvm_flag, 0x00, BOOTFLAG_SIZE);
		ret = fl_ops->fb_dev_flash_write((uchar *)(&nvm_flag), (ulong)(BOOTFLAG_SIZE), (uint64)(BOOTFLAG_ADDR), UNUSED);
		if (FB_OK != ret) {
			return;
		}
	}
	
}

/*
 * fb_main()	: Main module
 * Input		:None
 * Output		:None
 * Return		:None
 */
void fb_main(void)
{
	int				ret = 0;
	unsigned char	*fb_buff = (unsigned char *)COMMAND_BUFF_ADDR;
	unsigned int	code = 0;
	unsigned int	value = 0;

	/* Initialization for all modules */
	fb_init();

	/* Check and erase VNM flag */
	fb_check_nvm();
	
	while(1) {
		/* detect connection */
		ret = fb_comm_detect_conn();
		if (ret == FB_ERR_DISC) {		/* disconnect */
			/* re-connect */
			fb_comm_connect();
			continue;
		}

		/* get command send by Host and store to fb_buff*/
		fb_comm_get_cmd(fb_buff);
		/* get code and value from fb_buff */
		ret = fb_analy_analy_cmd(fb_buff, &code, &value);
		if (ret == FB_OK) {
			fb_main_exec_cmd(fb_buff, code, value);
		} else if (ret == FB_ERR_UNSUPPORT_PART) {
			fb_comm_respond("FAILUnsupported partition");
		} else {
			fb_comm_respond("FAILUnsupported command");
		}
		usb_skip_startpos();
	}
}

void Wait(ulong loop)
{
	/* It corrects not to be optimized by the GCC compiler. */
	asm volatile("        ldr r1, __Wait_waitCount");
	asm volatile("        mul r0, r0, r1");
	asm volatile("        b   __Wait_loopCmp");
	asm volatile("__Wait_loopSub:");
	asm volatile("        sub r0, r0, #1");
	asm volatile("__Wait_loopCmp:");
	asm volatile("        cmp r0, #0");
	asm volatile("        bne __Wait_loopSub");
	asm volatile("__Wait_loopEnd:");
	asm volatile("__Wait_waitCount:");
	asm volatile("        .word  %a0" : : "i" (202000));
}
