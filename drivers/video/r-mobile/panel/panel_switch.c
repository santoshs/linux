/*
 * drivers/video/r-mobile/panel/panel_switch.c
 *
 * Copyright (C) 2012 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/kernel.h>
#include <mach/common.h>
#include <video/sh_mobile_lcdc.h>
#include <rtapi/screen_display.h>

#define PANEL_SWITCH_HW_ID_REV_0_2_1 1
#define PANEL_SWITCH_HW_ID_REV_0_2_2 2
#define PANEL_SWITCH_HW_ID_REV_0_3_X 3
#define PANEL_SWITCH_HW_ID_REV_0_4_1 4
#define PANEL_SWITCH_HW_ID_REV_0_5_X 10

#ifdef CONFIG_FB_R_MOBILE_S6E39A0X02
#include "panel_s6e39a0x02.h"
#endif
#ifdef CONFIG_FB_R_MOBILE_HX8369_B
#include "panel_hx8369_b.h"
#endif

struct fb_panel_func r_mobile_panel_func(int panel)
{
	struct fb_panel_func panel_func;
	unsigned int hw_id;

	printk(KERN_INFO "%s\n", __func__);

	/* initialize */
	memset(&panel_func, 0, sizeof(struct fb_panel_func));

	if (panel != RT_DISPLAY_LCD1)
		goto out;

	hw_id = u2_get_board_rev();

	switch (hw_id) {
#ifdef CONFIG_FB_R_MOBILE_S6E39A0X02
	case PANEL_SWITCH_HW_ID_REV_0_2_1:
	case PANEL_SWITCH_HW_ID_REV_0_2_2:
	case PANEL_SWITCH_HW_ID_REV_0_3_X:
		printk(KERN_INFO "Switch s6e39a0x02.(ID=%d)\n", hw_id);
		panel_func = s6e39a0x02_func_list();
		break;
#endif
#ifdef CONFIG_FB_R_MOBILE_HX8369_B
	case PANEL_SWITCH_HW_ID_REV_0_4_1:
	case PANEL_SWITCH_HW_ID_REV_0_5_X:
		printk(KERN_INFO "Switch hx8369_b.(ID=%d)\n", hw_id);
		panel_func = hx8369_b_func_list();
		break;
#endif
	default:
		printk(KERN_INFO "Unknown HWID.(ID=%d)\n", hw_id);
	}

out:
	return panel_func;
}
