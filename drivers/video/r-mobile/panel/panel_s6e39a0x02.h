/*
 * drivers/video/r-mobile/panel/panel_s6e39a0x02.h
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
#ifndef __PANEL_S6E39A0X02_H__
#define __PANEL_S6E39A0X02_H__

extern int s6e39a0x02_dsi_read(int id, int reg, int len, char *buf);

#ifdef CONFIG_FB_R_MOBILE_PANEL_SWITCH
#include <video/sh_mobile_lcdc.h>
extern struct fb_panel_func s6e39a0x02_func_list(void);
#endif

#endif /* __PANEL_S6E39A0X02_H__ */
