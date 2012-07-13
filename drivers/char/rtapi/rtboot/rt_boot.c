/*
 * rt_boot.c
 *		booting rt_cpu.
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include "log_kernel.h"
#include "rt_boot_drv.h"
#include "rt_boot_local.h"

MODULE_AUTHOR("Renesas Electronics Corp.");
MODULE_DESCRIPTION("Device Driver (RT Boot)");


#define OS_KIND_LINUX	(0x50)
#define DRIVER_NAME		"rtboot"

/* prototype */
static int rtboot_init(void);
static void rtboot_exit(void);


static struct miscdevice g_device;

static const struct file_operations g_fops = {
	.owner          = THIS_MODULE,
};

/* boot_info */
struct rt_boot_info g_rtboot_info;

static int rtboot_init(void)
{
	int ret;
	unsigned int bootaddr = 0;

	MSG_HIGH("[RTBOOTK]IN |[%s]\n", __func__);

	memset(&g_rtboot_info, 0, sizeof(g_rtboot_info));

	g_device.name  = DRIVER_NAME;
	g_device.fops  = &g_fops;
	g_device.minor = MISC_DYNAMIC_MINOR;

	ret = misc_register(&g_device);
	if (0 != ret) {
		MSG_ERROR("[RTBOOTK]   |misc_register failed ret[%d]\n", ret);
		MSG_HIGH("[RTBOOTK]OUT|[%s] ret = 1\n", __func__);
		return 1;
	}

	do_ioremap_register();

	if (0 != read_rt_image(&bootaddr)) {
		MSG_ERROR("[RTBOOTK]   |read_rt_image() Error\n");
		do_iounmap_register();
		ret = misc_deregister(&g_device);
		if (0 != ret)
			MSG_ERROR("[RTBOOTK]   |misc_deregister failed ret[%d]\n", ret);
		MSG_HIGH("[RTBOOTK]OUT|[%s] ret = 1\n", __func__);
		return 1;
	}

	MSG_LOW("[RTBOOTK]   |write_rt_imageaddr bootaddr[%x]\n", bootaddr);

	MSG_LOW("[RTBOOTK]   |write_rt_imageaddr start\n");
	write_rt_imageaddr(bootaddr);
	MSG_LOW("[RTBOOTK]   |stop_rt_interrupt start\n");
	stop_rt_interrupt();
	MSG_LOW("[RTBOOTK]   |init_rt_register start\n");
	init_rt_register();
	MSG_LOW("[RTBOOTK]   |write_os_kind start\n");
	write_os_kind(OS_KIND_LINUX);
	MSG_LOW("[RTBOOTK]   |start_rt_cpu start\n");
	start_rt_cpu();
	MSG_LOW("[RTBOOTK]   |wait_rt_cpu start\n");
	ret = wait_rt_cpu(MAX_POLLING_COUNT);

	if (0 != ret) {
		MSG_ERROR("[RTBOOTK]   |RT boot error\n");
		do_iounmap_register();
		ret = misc_deregister(&g_device);
		if (0 != ret)
			MSG_ERROR("[RTBOOTK]   |misc_deregister failed ret[%d]\n", ret);
		MSG_HIGH("[RTBOOTK]OUT|[%s] ret = 1\n", __func__);
		return 1;
	}

	MSG_LOW("[RTBOOTK]   |write_req_comp start\n");
	write_req_comp();

	MSG_HIGH("[RTBOOTK]OUT|[%s] ret = 0\n", __func__);

	return 0;
}

static void rtboot_exit(void)
{
	int ret;

	MSG_HIGH("[RTBOOTK]IN |[%s]\n", __func__);

	do_iounmap_register();

	ret = misc_deregister(&g_device);
	if (0 != ret)
		MSG_ERROR("[RTBOOTK]   |misc_deregister failed ret[%d]\n", ret);

	MSG_HIGH("[RTBOOTK]OUT|[%s]\n", __func__);
}


int rtboot_get_section_header(struct rt_boot_info *info)
{
	int ret = 1;

	MSG_HIGH("[RTBOOTK]IN |[%s]\n", __func__);

	if (info) {
		*info = g_rtboot_info;
		ret = 0;
	}

	MSG_HIGH("[RTBOOTK]OUT|[%s]\n", __func__);

	return ret;
}
EXPORT_SYMBOL(rtboot_get_section_header);

module_init(rtboot_init);
module_exit(rtboot_exit);


