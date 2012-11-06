/*
 * screen_display_private.h
 *  screen display private header file.
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
#ifndef __SCREEN_DISPLAY_PRIVATE_H__
#define __SCREEN_DISPLAY_PRIVATE_H__

#define RT_DISPLAY_LONGPACKETMEMSIZE	(65535)

#define RT_DISPLAY_LONGPACKETSIZE		(1996)	/* #MU2DSP874 */

#define RT_DISPLAY_SCRNDATAINFO_SIZE	(16)
#define RT_DISPLAY_LCD1_OFFSET			(4)

typedef struct {
	void *handle;
	void *rtds_mem_handle;
} screen_disp_handle;

/* #MU2DISP1088 add -S- */
typedef struct {
	unsigned short    height;
	unsigned short    width;
	unsigned short    stride;
	unsigned short    mode;
} screen_display_screen_data_info;
/* #MU2DISP1088 add -E- */

struct iccom_wq_system_mem_rt_map
{
	struct semaphore	sem;
	system_mem_rt_map*	sys_rt_map;
	int					result;
};

#endif /* __SCREEN_OVERLAY_PRIVATE_H__ */

