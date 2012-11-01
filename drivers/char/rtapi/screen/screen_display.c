/*
 * screen_display.c
 *  screen display function file.
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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/completion.h>
#include <linux/mfis.h>
#include <system_memory.h>
#include <log_kernel.h>
#include <screen_display.h>
#include <iccom_drv_id.h>
#include <iccom_drv.h>
#include <screen_id.h>
#include <iccom_drv_standby.h>
#include <rtds_memory_drv.h>
#include "screen_display_private.h"
#include "system_rtload_internal.h"

#define	REG_SYSC_PSTR	((unsigned long)0xE6180080)
#define	POWER_A3R		((unsigned long)0x00002000)

static int output_state;


static int  iccom_wq_system_mem_rt_map_thread(void *param)
{
	struct	iccom_wq_system_mem_rt_map*	rtmap	= (struct iccom_wq_system_mem_rt_map*)param;
	struct	completion					comp;

	MSG_HIGH("[RTAPIK] IN |[%s], pid=%d\n", __func__, (int)current->pid );

	rtmap->result = system_memory_rt_map(rtmap->sys_rt_map);

	/* Check result */
	if (SMAP_LIB_MEMORY_OK != rtmap->result) {
		MSG_ERROR("[RTAPIK] ERR|[%s][%d] system_memory_rt_map() ret = [%d], 0x%08x -> 0x%08x\n",
			 __func__, __LINE__, rtmap->result, rtmap->sys_rt_map->phys_addr, rtmap->sys_rt_map->rtaddr );

		/* release semaphore */
		up(&rtmap->sem);
		return	0;
	}

	/* release semaphore */
	up(&rtmap->sem);

	/* Wait infinite */
	init_completion(&comp);
	wait_for_completion(&comp);

	MSG_ERROR("[RTAPIK] ERR|[%s][%d] illigal LEAVED!!!!\n", __func__, __LINE__ );
	return	0;
}


static int iccom_wq_system_mem_rt_map(system_mem_rt_map* sys_rt_map)
{
	static struct task_struct *			rtmap_thread;
	struct	iccom_wq_system_mem_rt_map	rtmap;

	MSG_HIGH("[RTAPIK] IN |[%s], pid=%d\n", __func__, (int)current->pid );

	/* Initialize struct member */
	rtmap.sys_rt_map	= sys_rt_map;
	rtmap.result		= SMAP_LIB_MEMORY_NG;
	sema_init(&rtmap.sem, 0);

	rtmap_thread	= kthread_run(iccom_wq_system_mem_rt_map_thread, &rtmap, "iccom_rtmap");
	if (NULL == rtmap_thread) {
		MSG_ERROR( "[RTAPIK] ERR|[%s][%d] kthread_run() ret=%d\n",
			__func__, __LINE__, (int)rtmap_thread );
		return	SMAP_LIB_MEMORY_NG;
	}

	/* wait completion */
	down(&rtmap.sem);

	MSG_HIGH("[RTAPIK] OUT|[%s][%d] result=%d, 0x%08x -> 0x%08x\n",
		__func__, __LINE__,
		rtmap.result, sys_rt_map->phys_addr, sys_rt_map->rtaddr);
	return	rtmap.result;
}


void screen_display_request(void *user_data, int result, int func_id,
							 unsigned char *addr, int length)
{

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	MSG_MED("[RTAPIK]    |info   [0x%08X]\n", (unsigned int)user_data);
	MSG_MED("[RTAPIK]    |result [%d]\n", result);
	MSG_MED("[RTAPIK]    |func_id[%d]\n", func_id);
	MSG_MED("[RTAPIK]    |addr   [0x%08X]\n", (unsigned int)addr);
	MSG_MED("[RTAPIK]    |length [%d]\n", length);

	if (result < 0) {
		MSG_ERROR(
		"[SRN]ERR|[%s][%d] screen_display_request result=%d,func_id=%d\n",
		__func__, __LINE__, result, func_id);
	}

	MSG_HIGH("[RTAPIK] OUT|[%s]\n", __func__);
}
EXPORT_SYMBOL(screen_display_request);
void *screen_display_new(void)
{
	screen_disp_handle *disp_handle;
	iccom_drv_init_param         iccom_init;
	iccom_drv_cleanup_param  iccom_cleanup;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);

	output_state = ICCOM_DRV_STATE_LCD_ON;

	MSG_MED("[RTAPIK]    |callback[0x%08X]\n", (unsigned int)NULL);

	disp_handle = (screen_disp_handle *)kmalloc(
		sizeof(screen_disp_handle),
		GFP_KERNEL);
	if (NULL == disp_handle) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%s][%d] kmalloc() aInfo NULL error\n",
		__func__,
		__LINE__);
		return NULL;
	}

	iccom_init.user_data      = (void *)disp_handle;
	iccom_init.comp_notice = &screen_display_request;

	disp_handle->handle = iccom_drv_init(&iccom_init);
	if (NULL == disp_handle->handle) {
		kfree(disp_handle);
		MSG_ERROR(
		"[RTAPIK] ERR|[%s][%d] iccom_drv_init() aInfo NULL error\n",
		__func__,
		__LINE__);
		return NULL;
	}

	disp_handle->rtds_mem_handle = rtds_memory_drv_init();
	if (NULL == disp_handle->rtds_mem_handle) {
		iccom_cleanup.handle = disp_handle->handle;
		iccom_drv_cleanup(&iccom_cleanup);
		kfree(disp_handle);
		MSG_ERROR(
		"[RTAPIK] ERR|[%s][%d] rtds_memory_drv_init() aInfo NULL error\n",
		__func__,
		__LINE__);
		return NULL;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] disp_handle[0x%08X]\n",
	__func__,
	__LINE__,
	(unsigned int)disp_handle);
	return (void *)disp_handle;
}
EXPORT_SYMBOL(screen_display_new);

int screen_display_get_address(screen_disp_get_address *address)
{
	get_section_header_param	get_section;
	system_rt_section_header    section;
	int result;
	unsigned long disp_addr;
	void *kernel_disp_addr;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);

	if (NULL == address) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	MSG_MED("[RTAPIK]    |handle      [0x%08X]\n", (unsigned int)address->handle);
	MSG_MED("[RTAPIK]    |output_mode [%d]\n", address->output_mode);

	if  ((NULL == address->handle) ||
		 (RT_DISPLAY_LCD1 != address->output_mode)
		) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] output_mode = [%d]\n",
		__LINE__,
		address->output_mode);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	switch (address->output_mode) {
	case RT_DISPLAY_LCD1:
		get_section.section_header = &section;
		result = sys_get_section_header(&get_section);
		if (SMAP_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] result = [%d]\n",
			__LINE__,
			result);
			return SMAP_LIB_DISPLAY_NG;
		}

		disp_addr = section.displaybuff_address;
#if 1
		kernel_disp_addr = (void *)ioremap_nocache(disp_addr	, RT_DISPLAY_SCRNDATAINFO_SIZE);
		if (NULL == kernel_disp_addr) {

			MSG_ERROR(
			"[RTAPIK] ERR|[%d] \n",
			__LINE__);

			return SMAP_LIB_DISPLAY_NG;
		}

		address->address = (unsigned int)disp_addr + *((unsigned long *)((unsigned long)kernel_disp_addr + RT_DISPLAY_LCD1_OFFSET));

		iounmap(kernel_disp_addr);
#endif
		break;
	default:
		break;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d] address[0x%08X]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK,
	(int)address->address);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_get_address);

int screen_display_draw(screen_disp_draw *disp_draw)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	int					 disptask_id;	/* #MU2DSP582 add */
	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == disp_draw) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	MSG_MED("[RTAPIK]    |aInfo   [0x%08X]\n", (unsigned int)disp_draw->handle);
	MSG_MED("[RTAPIK]    |aLcdMode[%d]\n", disp_draw->output_mode);
	MSG_MED("[RTAPIK]    |aX      [%d]\n", disp_draw->draw_rect.x);
	MSG_MED("[RTAPIK]    |aY      [%d]\n", disp_draw->draw_rect.y);
	MSG_MED("[RTAPIK]    |aW      [%d]\n", disp_draw->draw_rect.width);
	MSG_MED("[RTAPIK]    |aH      [%d]\n", disp_draw->draw_rect.height);
	MSG_MED("[RTAPIK]    |aFormat [%d]\n", disp_draw->format);
	MSG_MED("[RTAPIK]    |buffer_id     [%d]\n", disp_draw->buffer_id);
	MSG_MED("[RTAPIK]    |buffer_offset [%d]\n", disp_draw->buffer_offset);
	MSG_MED("[RTAPIK]    |rotate        [%d]\n", disp_draw->rotate);

	if ((NULL == disp_draw->handle) ||
		((RT_DISPLAY_LCD1 != disp_draw->output_mode) &&
		(RT_DISPLAY_LCD2  != disp_draw->output_mode) &&
/* #MU2DSP582 mod -S- */
/*		(RT_DISPLAY_HDMI  != disp_draw->output_mode)) || */
		(RT_DISPLAY_HDMI  != disp_draw->output_mode) &&
		(RT_DISPLAY_LCDHDMI  != disp_draw->output_mode)) ||
/* #MU2DSP582 mod -E- */
		((RT_DISPLAY_LCD1 == disp_draw->output_mode) &&
		 (RT_DISPLAY_FORMAT_RGB565   != disp_draw->format) &&
		 (RT_DISPLAY_FORMAT_ARGB8888 != disp_draw->format))
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] \n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

/* #MU2DSP582 add -S- */
	if ((RT_DISPLAY_LCD1    == disp_draw->output_mode) ||
	    (RT_DISPLAY_LCDHDMI == disp_draw->output_mode)) {
		disptask_id = TASK_DISPLAY;
	} else {
		disptask_id = TASK_DISPLAY2;
	}
/* #MU2DSP582 add -E- */

	iccom_send_cmd.handle      = ((screen_disp_handle *)disp_draw->handle)->handle;
/* #MU2DSP582 mod -S- */
	iccom_send_cmd.task_id     = disptask_id;
/* #MU2DSP582 mod -E- */
	iccom_send_cmd.function_id = EVENT_DISPLAY_DRAW;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_draw);
	iccom_send_cmd.send_data   = (unsigned char *)disp_draw;
	iccom_send_cmd.recv_size   = 0;
	iccom_send_cmd.recv_data   = NULL;

	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_draw);

int screen_display_start_lcd(screen_disp_start_lcd *start_lcd)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	iccom_drv_lcd_state_param iccom_lcd_state;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);

	if (NULL == start_lcd) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo       [0x%08X]\n", (unsigned int)start_lcd->handle);
	MSG_MED("[RTAPIK]    |aRefreshMode[%d]\n", start_lcd->output_mode);


	if ((NULL == start_lcd->handle) ||
		((RT_DISPLAY_LCD1 != start_lcd->output_mode) &&
		 (RT_DISPLAY_LCD2 != start_lcd->output_mode))
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] \n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	iccom_lcd_state.handle    = ((screen_disp_handle *)start_lcd->handle)->handle;
	iccom_lcd_state.lcd_state = ICCOM_DRV_STATE_LCD_ON;
	iccom_drv_set_lcd_state(&iccom_lcd_state);

	iccom_send_cmd.handle      = ((screen_disp_handle *)start_lcd->handle)->handle;
	iccom_send_cmd.task_id     = TASK_DISPLAY;
	iccom_send_cmd.function_id = EVENT_DISPLAY_STARTLCD;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_start_lcd);
	iccom_send_cmd.send_data   = (unsigned char *)start_lcd;
	iccom_send_cmd.recv_size   = 0;
	iccom_send_cmd.recv_data   = NULL;

	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		iccom_lcd_state.handle    = ((screen_disp_handle *)start_lcd->handle)->handle;
		iccom_lcd_state.lcd_state = output_state;
		iccom_drv_set_lcd_state(&iccom_lcd_state);

		return result;
	}

	output_state = ICCOM_DRV_STATE_LCD_ON;

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_start_lcd);

int screen_display_stop_lcd(screen_disp_stop_lcd *stop_lcd)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	iccom_drv_lcd_state_param iccom_lcd_state;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);

	if (NULL == stop_lcd) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo       [0x%08X]\n", (unsigned int)stop_lcd->handle);
	MSG_MED("[RTAPIK]    |aRefreshMode[%d]\n", stop_lcd->output_mode);


	if ((NULL == stop_lcd->handle) ||
		((RT_DISPLAY_LCD1 != stop_lcd->output_mode) &&
		 (RT_DISPLAY_LCD2 != stop_lcd->output_mode))
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] \n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	iccom_send_cmd.handle      = ((screen_disp_handle *)stop_lcd->handle)->handle;
	iccom_send_cmd.task_id     = TASK_DISPLAY;
	iccom_send_cmd.function_id = EVENT_DISPLAY_STOPLCD;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_stop_lcd);
	iccom_send_cmd.send_data   = (unsigned char *)stop_lcd;
	iccom_send_cmd.recv_size   = 0;
	iccom_send_cmd.recv_data   = NULL;

	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		iccom_lcd_state.handle    = ((screen_disp_handle *)stop_lcd->handle)->handle;
		iccom_lcd_state.lcd_state = output_state;
		iccom_drv_set_lcd_state(&iccom_lcd_state);
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		return result;
	}

	output_state = ICCOM_DRV_STATE_LCD_OFF;

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_stop_lcd);

int screen_display_set_lcd_refresh(screen_disp_set_lcd_refresh *set_lcd_refresh)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	iccom_drv_lcd_state_param iccom_lcd_state;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);

	if (NULL == set_lcd_refresh) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo       [0x%08X]\n", (unsigned int)set_lcd_refresh->handle);
	MSG_MED("[RTAPIK]    |aRefreshMode[%d]\n", set_lcd_refresh->refresh_mode);


	if ((NULL == set_lcd_refresh->handle) ||
		((RT_DISPLAY_LCD1 != set_lcd_refresh->output_mode) &&
		 (RT_DISPLAY_LCD2 != set_lcd_refresh->output_mode)) ||
		((RT_DISPLAY_REFRESH_ON  != set_lcd_refresh->refresh_mode) &&
		 (RT_DISPLAY_REFRESH_OFF != set_lcd_refresh->refresh_mode))
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] \n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	if (!(POWER_A3R & inl(REG_SYSC_PSTR))) {
		result = mfis_drv_resume();
		if (SMAP_OK != result) {
			return result;
		}
	}

	iccom_send_cmd.handle      = ((screen_disp_handle *)set_lcd_refresh->handle)->handle;
	iccom_send_cmd.task_id     = TASK_DISPLAY;
	iccom_send_cmd.function_id = EVENT_DISPLAY_SETLCDREFRESH;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_set_lcd_refresh);
	iccom_send_cmd.send_data   = (unsigned char *)set_lcd_refresh;
	iccom_send_cmd.recv_size   = 0;
	iccom_send_cmd.recv_data   = NULL;

	if (RT_DISPLAY_REFRESH_OFF == set_lcd_refresh->refresh_mode) {
		iccom_lcd_state.handle    = ((screen_disp_handle *)set_lcd_refresh->handle)->handle;
		iccom_lcd_state.lcd_state = ICCOM_DRV_STATE_LCD_ON;
		iccom_drv_set_lcd_state(&iccom_lcd_state);
	}

	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		iccom_lcd_state.handle    = ((screen_disp_handle *)set_lcd_refresh->handle)->handle;
		iccom_lcd_state.lcd_state = output_state;
		iccom_drv_set_lcd_state(&iccom_lcd_state);

		return result;
	}

	if (RT_DISPLAY_REFRESH_ON == set_lcd_refresh->refresh_mode) {
		iccom_lcd_state.handle    = ((screen_disp_handle *)set_lcd_refresh->handle)->handle;
		iccom_lcd_state.lcd_state = ICCOM_DRV_STATE_LCD_REFRESH;
		iccom_drv_set_lcd_state(&iccom_lcd_state);

		output_state = ICCOM_DRV_STATE_LCD_REFRESH;
	} else {
		output_state = ICCOM_DRV_STATE_LCD_ON;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_set_lcd_refresh);

int screen_display_start_hdmi(screen_disp_start_hdmi *start_hdmi)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	iccom_drv_disable_standby_param iccom_disable_standby;
	iccom_drv_enable_standby_param  iccom_standby_eneble;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == start_hdmi) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |handle       [0x%08X]\n", (unsigned int)start_hdmi->handle);
	MSG_MED("[RTAPIK]    |format[%d]\n", start_hdmi->format);
	MSG_MED("[RTAPIK]    |background_color  [%d]\n", start_hdmi->background_color);


	if ((NULL == start_hdmi->handle) ||
		((RT_DISPLAY_720_480P60	!= start_hdmi->format) &&
		 (RT_DISPLAY_1280_720P60	!= start_hdmi->format) &&
/* #MU2DSP582 mod -S- */
/*		 (RT_DISPLAY_1920_1080I60	!= start_hdmi->format) &&	*/
/*		 (RT_DISPLAY_1920_1080P24	!= start_hdmi->format))		*/
		 (RT_DISPLAY_720_576P50		!= start_hdmi->format) &&
		 (RT_DISPLAY_1280_720P50	!= start_hdmi->format) &&
		 (RT_DISPLAY_1920_1080P60	!= start_hdmi->format) &&
		 (RT_DISPLAY_1920_1080P50	!= start_hdmi->format) &&
		 (RT_DISPLAY_720_480P60A43	!= start_hdmi->format) &&
		 (RT_DISPLAY_720_576P50A43	!= start_hdmi->format) &&
/* #MU2DSP582 mod -E- */
/* #MU2DSP939 add -S- */
		 (RT_DISPLAY_USE_IF_PARAM	!= start_hdmi->format))
/* #MU2DSP939 add -E- */
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] \n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	iccom_send_cmd.handle      = ((screen_disp_handle *)start_hdmi->handle)->handle;
/* #MU2DSP582 mod -S- */
/*	iccom_send_cmd.task_id     = TASK_DISPLAY; */
	iccom_send_cmd.task_id     = TASK_DISPLAY2;
/* #MU2DSP582 mod -E- */
	iccom_send_cmd.function_id = EVENT_DISPLAY_STARTHDMI;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_start_hdmi);
	iccom_send_cmd.send_data   = (unsigned char *)start_hdmi;
	iccom_send_cmd.recv_size   = 0;
	iccom_send_cmd.recv_data   = NULL;

	iccom_disable_standby.handle = ((screen_disp_handle *)start_hdmi->handle)->handle;
	result = iccom_drv_disable_standby(&iccom_disable_standby);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_disable_standby() ret = [%d]\n",
		__LINE__,
		result);

		return result;
	}

	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		iccom_standby_eneble.handle = ((screen_disp_handle *)start_hdmi->handle)->handle;
		iccom_drv_enable_standby(&iccom_standby_eneble);
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);

		return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_start_hdmi);

int screen_display_stop_hdmi(screen_disp_stop_hdmi *stop_hdmi)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	iccom_drv_enable_standby_param  iccom_standby_eneble;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == stop_hdmi) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo[0x%08X]\n", (unsigned int)stop_hdmi->handle);

	if (NULL == stop_hdmi->handle) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] aInfo NULL error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	iccom_send_cmd.handle      = ((screen_disp_handle *)stop_hdmi->handle)->handle;
	iccom_send_cmd.task_id     = TASK_DISPLAY;
	iccom_send_cmd.function_id = EVENT_DISPLAY_STOPHDMI;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = 0;
	iccom_send_cmd.send_data   = NULL;
	iccom_send_cmd.recv_size   = 0;
	iccom_send_cmd.recv_data   = NULL;

	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		return result;
	}

	iccom_standby_eneble.handle = ((screen_disp_handle *)stop_hdmi->handle)->handle;
	result = iccom_drv_enable_standby(&iccom_standby_eneble);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%s] iccom_drv_enable_standby() ret = [%d]\n",
		__func__,
		result);
	}
	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_stop_hdmi);

/* #MU2DSP222 add -S- */
int screen_display_read_dsi_short_packet(screen_disp_read_dsi_short *read_dsi_s)
{
	int result;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	int					 disptask_id;	/* #MU2DSP582 add */

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == read_dsi_s) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo     [0x%08X]\n", (unsigned int)read_dsi_s->handle);
	MSG_MED("[RTAPIK]    |aLcdMode  [%d]\n", read_dsi_s->output_mode);
	MSG_MED("[RTAPIK]    |aDataID   [%d]\n", read_dsi_s->data_id);
	MSG_MED("[RTAPIK]    |aWriteAdr [%d]\n", read_dsi_s->reg_address);
	MSG_MED("[RTAPIK]    |aWriteData[%d]\n", read_dsi_s->write_data);
	MSG_MED("[RTAPIK]    |data_count[%d]\n", read_dsi_s->data_count);
	MSG_MED("[RTAPIK]    |read_data [0x%08X]\n", (unsigned int)read_dsi_s->read_data);

/* #MU2DSP582 mod -S- */
/*	if (NULL == read_dsi_s->handle) */
	if ((NULL == read_dsi_s->handle) ||
		((RT_DISPLAY_LCD1 != read_dsi_s->output_mode) &&
		 (RT_DISPLAY_LCD2 != read_dsi_s->output_mode) &&	/* #MU2DSP852 add */
		 (RT_DISPLAY_HDMI != read_dsi_s->output_mode))
	   ) {
/* #MU2DSP582 mod -E- */
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] aInfo NULL error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	if (0 == read_dsi_s->data_count) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] data_count Zero error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	if (NULL == read_dsi_s->read_data) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] read_data NULL error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

/* #MU2DSP582 add -S- */
	if (RT_DISPLAY_LCD1 == read_dsi_s->output_mode) {
		disptask_id = TASK_DISPLAY;
	} else {
		disptask_id = TASK_DISPLAY2;
	}
/* #MU2DSP582 add -E- */

	iccom_send_cmd.handle      = ((screen_disp_handle *)read_dsi_s->handle)->handle;
/* #MU2DSP582 mod -S- */
	iccom_send_cmd.task_id     = disptask_id;
/* #MU2DSP582 add -E- */
	iccom_send_cmd.function_id = EVENT_DISPLAY_READDSISHORTPACKET;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_read_dsi_short);
	iccom_send_cmd.send_data   = (unsigned char *)read_dsi_s;
	iccom_send_cmd.recv_size   = read_dsi_s->data_count;
	iccom_send_cmd.recv_data   = read_dsi_s->read_data;
	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d] \n",
	__func__,
	__LINE__,
	result);
	return SMAP_LIB_DISPLAY_OK;
}
/* #MU2DSP222 add -E- */
EXPORT_SYMBOL(screen_display_read_dsi_short_packet);

int screen_display_write_dsi_short_packet(screen_disp_write_dsi_short *write_dsi_s)
{
	int result;
	int dsi_info;
	iccom_drv_send_cmd_param  iccom_send_cmd;
	int					 disptask_id;	/* #MU2DSP582 add */

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == write_dsi_s) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo     [0x%08X]\n", (unsigned int)write_dsi_s->handle);
	MSG_MED("[RTAPIK]    |aLcdMode  [%d]\n", write_dsi_s->output_mode);
	MSG_MED("[RTAPIK]    |aDataID   [%d]\n", write_dsi_s->data_id);
	MSG_MED("[RTAPIK]    |aWriteAdr [%d]\n", write_dsi_s->reg_address);
	MSG_MED("[RTAPIK]    |aWriteData[%d]\n", write_dsi_s->write_data);
	MSG_MED("[RTAPIK]    |reception_mode[%d]\n", write_dsi_s->reception_mode);		/* #MU2DSP860 */

/* #MU2DSP582 mod -S- */
/*	if (NULL == write_dsi_s->handle) */
	if ((NULL == write_dsi_s->handle) ||
		((RT_DISPLAY_LCD1 != write_dsi_s->output_mode) &&
		 (RT_DISPLAY_LCD2 != write_dsi_s->output_mode) &&	/* #MU2DSP852 add */
		 (RT_DISPLAY_HDMI != write_dsi_s->output_mode))
	   ) {
/* #MU2DSP582 mod -E- */
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] aInfo NULL error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

/* #MU2DSP582 add -S- */
	if (RT_DISPLAY_LCD1 == write_dsi_s->output_mode) {
		disptask_id = TASK_DISPLAY;
	} else {
		disptask_id = TASK_DISPLAY2;
	}
/* #MU2DSP582 add -E- */

	iccom_send_cmd.handle      = ((screen_disp_handle *)write_dsi_s->handle)->handle;
/* #MU2DSP582 mod -S- */
	iccom_send_cmd.task_id     = disptask_id;
/* #MU2DSP582 mod -E- */
	iccom_send_cmd.function_id = EVENT_DISPLAY_WRITEDSISHORTPACKET;
	iccom_send_cmd.send_mode   = ICCOM_DRV_SYNC;
	iccom_send_cmd.send_size   = sizeof(screen_disp_write_dsi_short);
	iccom_send_cmd.send_data   = (unsigned char *)write_dsi_s;
	iccom_send_cmd.recv_size   = sizeof(dsi_info);
	iccom_send_cmd.recv_data   = (unsigned char *)&dsi_info;
	result = iccom_drv_send_command(&iccom_send_cmd);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command() ret = [%d]\n",
		__LINE__,
		result);
		return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d] \n",
	__func__,
	__LINE__,
	result);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_write_dsi_short_packet);

int screen_display_write_dsi_long_packet(screen_disp_write_dsi_long *write_dsi_l)
{
	int result;
	int dsi_info;
	unsigned int write_num  = 0;
	unsigned int ap_address = 0;
	unsigned int rt_address = 0;
	iccom_drv_cmd_data      cmd_array[2];
	iccom_drv_send_cmd_array_param iccom_send_cmd_array;
	system_mem_ap_open      ap_open;
	system_mem_ap_alloc     ap_alloc;
	system_mem_ap_change_rtaddr ap_change_rtaddr;
	system_mem_ap_free      ap_free;
	system_mem_ap_close     ap_close;
	system_mem_rt_map_pnc	rt_map_pnc;
	system_mem_rt_unmap_pnc	rt_unmap_pnc;
	int					 disptask_id;	/* #MU2DSP582 add */

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == write_dsi_l) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |aInfo     [0x%08X]\n", (unsigned int)write_dsi_l->handle);
	MSG_MED("[RTAPIK]    |aLcdMode  [%d]\n", write_dsi_l->output_mode);
	MSG_MED("[RTAPIK]    |aDataID   [%d]\n", write_dsi_l->data_id);
	MSG_MED("[RTAPIK]    |aWordCount[%d]\n", write_dsi_l->data_count);
	MSG_MED("[RTAPIK]    |aWriteData[0x%08X]\n", (unsigned int)write_dsi_l->write_data);
	MSG_MED("[RTAPIK]    |reception_mode[%d]\n", write_dsi_l->reception_mode);
	MSG_MED("[RTAPIK]    |send_mode[%d]\n", write_dsi_l->send_mode);

	if ((NULL == write_dsi_l->handle) ||
/* #MU2DSP582 add -S- */
		((RT_DISPLAY_LCD1 != write_dsi_l->output_mode) &&
		(RT_DISPLAY_LCD2  != write_dsi_l->output_mode) &&	/* #MU2DSP852 add */
		(RT_DISPLAY_HDMI  != write_dsi_l->output_mode)) ||
/* #MU2DSP582 add -E- */
		(0 == write_dsi_l->data_count) ||
		(NULL == write_dsi_l->write_data)
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] \n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	cmd_array[0].size	= sizeof(screen_disp_write_dsi_long);
	cmd_array[0].data	= (unsigned int *)write_dsi_l;

	if (RT_DISPLAY_LONGPACKETSIZE >= write_dsi_l->data_count) {
		cmd_array[1].size	= (write_dsi_l->data_count * sizeof(unsigned char));
		cmd_array[1].data	= (unsigned int *)write_dsi_l->write_data;
	} else {
		ap_open.handle       = write_dsi_l->handle;
		ap_open.aparea_size  = RT_MEMORY_APAREA_SIZE(RT_DISPLAY_LONGPACKETMEMSIZE);
		ap_open.cache_kind   = RT_MEMORY_NONCACHE;

		result = system_memory_ap_open(&ap_open);
		if (SMAP_LIB_MEMORY_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_ap_open() ret = [%d]\n",
			__LINE__,
			result);
			return result;
		}

		rt_map_pnc.handle       = write_dsi_l->handle;
		rt_map_pnc.apaddr       = ap_open.apaddr;
		rt_map_pnc.map_size     = RT_MEMORY_APAREA_SIZE(RT_DISPLAY_LONGPACKETMEMSIZE);
		rt_map_pnc.pages        = ap_open.pages;
		rt_map_pnc.rtcache_kind = RT_MEMORY_RTMAP_WBNC;

		result = system_memory_rt_map_pnc(&rt_map_pnc);
		if (SMAP_LIB_MEMORY_OK != result) {

			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_rt_map_pnc() ret = [%d]\n",
			__LINE__,
			result);

			ap_close.handle = write_dsi_l->handle;
			ap_close.apaddr = ap_open.apaddr;
			ap_close.pages  = ap_open.pages;
			system_memory_ap_close(&ap_close);
			return result;
		}

		ap_alloc.handle       = write_dsi_l->handle;
		ap_alloc.alloc_size   = RT_MEMORY_APMEM_SIZE(RT_DISPLAY_LONGPACKETMEMSIZE);
		ap_alloc.apmem_handle = rt_map_pnc.apmem_handle;

		result = system_memory_ap_alloc(&ap_alloc);
		if (SMAP_LIB_MEMORY_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_ap_alloc() ret = [%d]\n",
			__LINE__,
			result);

			rt_unmap_pnc.handle = write_dsi_l->handle;
			rt_unmap_pnc.apmem_handle = rt_map_pnc.apmem_handle;
			ap_close.handle = write_dsi_l->handle;
			ap_close.apaddr = ap_open.apaddr;
			ap_close.pages  = ap_open.pages;

			system_memory_rt_unmap_pnc(&rt_unmap_pnc);
			system_memory_ap_close(&ap_close);
			return result;
		}

		ap_address = ap_alloc.apmem_apaddr;

		memcpy((void *)ap_address, write_dsi_l->write_data, (size_t)write_dsi_l->data_count);

		ap_change_rtaddr.handle       = write_dsi_l->handle;
		ap_change_rtaddr.cache_kind   = RT_MEMORY_NONCACHE;
		ap_change_rtaddr.apmem_handle = rt_map_pnc.apmem_handle;
		ap_change_rtaddr.apmem_apaddr = ap_address;

		result = system_memory_ap_change_rtaddr(&ap_change_rtaddr);
		if (SMAP_LIB_MEMORY_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_ap_change_rtaddr() ret = [%d]\n",
			__LINE__,
			result);

			ap_free.handle       = write_dsi_l->handle;
			ap_free.apmem_handle = ap_alloc.apmem_handle;
			ap_free.apmem_apaddr = ap_address;
			rt_unmap_pnc.handle = write_dsi_l->handle;
			rt_unmap_pnc.apmem_handle = rt_map_pnc.apmem_handle;
			ap_close.handle = write_dsi_l->handle;
			ap_close.apaddr = ap_open.apaddr;
			ap_close.pages  = ap_open.pages;

			system_memory_ap_free(&ap_free);
			system_memory_rt_unmap_pnc(&rt_unmap_pnc);
			system_memory_ap_close(&ap_close);
			return result;
		}
		rt_address = ap_change_rtaddr.apmem_rtaddr;
		cmd_array[1].size = sizeof(&rt_address);
		cmd_array[1].data  = (unsigned int *)&rt_address;
	}

	MSG_MED("[RTAPIK]    |cmd size   [%d]\n",     cmd_array[1].size);
	MSG_MED("[RTAPIK]    |packet ptr [0x%08X]\n", (unsigned int)cmd_array[1].data);
	write_num  = sizeof(cmd_array)/sizeof(cmd_array[0]);

/* #MU2DSP582 add -S- */
	if (RT_DISPLAY_LCD1 == write_dsi_l->output_mode) {
		disptask_id = TASK_DISPLAY;
	} else {
		disptask_id = TASK_DISPLAY2;
	}
/* #MU2DSP582 add -E- */

	iccom_send_cmd_array.handle          = ((screen_disp_handle *)write_dsi_l->handle)->handle;
/* #MU2DSP582 mod -S- */
	iccom_send_cmd_array.task_id         = disptask_id;
/* #MU2DSP582 mod -E- */
	iccom_send_cmd_array.function_id     = EVENT_DISPLAY_WRITEDSILONGPACKET;
	iccom_send_cmd_array.send_mode       = ICCOM_DRV_SYNC;
	iccom_send_cmd_array.send_num        = write_num;
	iccom_send_cmd_array.send_data       = cmd_array;
	iccom_send_cmd_array.recv_size       = sizeof(dsi_info);
	iccom_send_cmd_array.recv_data       = (unsigned char *)&dsi_info;

	result = iccom_drv_send_command_array(&iccom_send_cmd_array);
	if (SMAP_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] iccom_drv_send_command_array() ret = [%d]\n",
			__LINE__,
			result);

		if (RT_DISPLAY_LONGPACKETSIZE < write_dsi_l->data_count) {
			ap_free.handle       = write_dsi_l->handle;
			ap_free.apmem_handle = ap_alloc.apmem_handle;
			ap_free.apmem_apaddr = ap_address;
			rt_unmap_pnc.handle = write_dsi_l->handle;
			rt_unmap_pnc.apmem_handle = rt_map_pnc.apmem_handle;
			ap_close.handle = write_dsi_l->handle;
			ap_close.apaddr = ap_open.apaddr;
			ap_close.pages  = ap_open.pages;

			system_memory_ap_free(&ap_free);
			system_memory_rt_unmap_pnc(&rt_unmap_pnc);
			system_memory_ap_close(&ap_close);
		}
		return result;
	}

	if (RT_DISPLAY_LONGPACKETSIZE < write_dsi_l->data_count) {

		ap_free.handle       = write_dsi_l->handle;
		ap_free.apmem_handle = ap_alloc.apmem_handle;
		ap_free.apmem_apaddr = ap_address;

		result = system_memory_ap_free(&ap_free);
		if (SMAP_LIB_MEMORY_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_ap_free() ret = [%d]\n",
			__LINE__,
			result);
			return result;
		}

		rt_unmap_pnc.handle = write_dsi_l->handle;
		rt_unmap_pnc.apmem_handle = rt_map_pnc.apmem_handle;

		result = system_memory_rt_unmap_pnc(&rt_unmap_pnc);
		if (SMAP_LIB_MEMORY_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_rt_unmap_pnc() ret = [%d]\n",
			__LINE__,
			result);
			return result;
		}

		ap_close.handle = write_dsi_l->handle;
		ap_close.apaddr = ap_open.apaddr;
		ap_close.pages  = ap_open.pages;

		result = system_memory_ap_close(&ap_close);
		if (SMAP_LIB_MEMORY_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] system_memory_ap_close() ret = [%d]\n",
			__LINE__,
			result);
			return result;
		}
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d] \n",
	__func__,
	__LINE__,
	result);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_write_dsi_long_packet);

int screen_display_set_lcd_if_parameters(screen_disp_set_lcd_if_param *set_lcd_if_param)
{
	int result;
	iccom_drv_cmd_data      cmd_array[3];
	iccom_drv_send_cmd_array_param iccom_send_cmd_array;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == set_lcd_if_param) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |handle            [0x%08X]\n", (unsigned int)set_lcd_if_param->handle);
	MSG_MED("[RTAPIK]    |lcd_if_param      [0x%08X]\n", (unsigned int)set_lcd_if_param->lcd_if_param);
	MSG_MED("[RTAPIK]    |lcd_if_param_mask [0x%08X]\n", (unsigned int)set_lcd_if_param->lcd_if_param_mask);

	if ((NULL == set_lcd_if_param->handle) ||
		(NULL == set_lcd_if_param->lcd_if_param) ||
		(NULL == set_lcd_if_param->lcd_if_param_mask)
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] address NULL error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	cmd_array[0].size	= sizeof(unsigned int);
	cmd_array[0].data	= (unsigned int *)&(set_lcd_if_param->port_no);
	cmd_array[1].size	= sizeof(screen_disp_lcd_if);
	cmd_array[1].data	= (unsigned int *)set_lcd_if_param->lcd_if_param;
	cmd_array[2].size	= sizeof(screen_disp_lcd_if);
	cmd_array[2].data	= (unsigned int *)set_lcd_if_param->lcd_if_param_mask;

	iccom_send_cmd_array.handle          = ((screen_disp_handle *)set_lcd_if_param->handle)->handle;
	iccom_send_cmd_array.task_id         = TASK_DISPLAY;
	iccom_send_cmd_array.function_id     = EVENT_DISPLAY_SETLCDIFPARAMETERS;
	iccom_send_cmd_array.send_mode       = ICCOM_DRV_SYNC;
	iccom_send_cmd_array.send_num        = sizeof(cmd_array)/sizeof(cmd_array[0]);
	iccom_send_cmd_array.send_data       = cmd_array;
	iccom_send_cmd_array.recv_size       = 0;
	iccom_send_cmd_array.recv_data       = NULL;

	result = iccom_drv_send_command_array(&iccom_send_cmd_array);
	if (SMAP_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] iccom_drv_send_command_array() ret = [%d]\n",
			__LINE__,
			result);
			return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_set_lcd_if_parameters);

int screen_display_set_address(screen_disp_set_address *address)
{
	int result;
	iccom_drv_cmd_data      cmd_array[2];
	iccom_drv_send_cmd_array_param iccom_send_cmd_array;
	system_mem_rt_map       sys_rt_map;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == address) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |handle       [0x%08X]\n", (unsigned int)address->handle);
	MSG_MED("[RTAPIK]    |output_mode  [%d]\n", address->output_mode);
	MSG_MED("[RTAPIK]    |buffer_id    [%d]\n", address->buffer_id);
	MSG_MED("[RTAPIK]    |address      [0x%08X]\n", (unsigned int)address->address);
	MSG_MED("[RTAPIK]    |size         [%d]\n", address->size);

	if  ((NULL == address->handle) ||
		 (RT_DISPLAY_LCD1 != address->output_mode) ||
		 (0 == address->address) ||
		 (0 == address->size)
		) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] param error\n", __LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	sys_rt_map.handle	 = address->handle;
	sys_rt_map.phys_addr = address->address;
	sys_rt_map.map_size  = address->size + 0x2000;

	result = iccom_wq_system_mem_rt_map( &sys_rt_map );
	if (SMAP_LIB_MEMORY_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] system_memory_rt_map() ret = [%d]\n",
		__LINE__,
		result);
		return result;
	}

	if (0 == sys_rt_map.rtaddr) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] rtaddr NULL\n", __LINE__);
		return SMAP_LIB_DISPLAY_NG;
	}

	cmd_array[0].size	= sizeof(screen_disp_set_address);
	cmd_array[0].data	= (unsigned int *)address;
	cmd_array[1].size	= sizeof(sys_rt_map.rtaddr);
	cmd_array[1].data	= (unsigned int *)&sys_rt_map.rtaddr;

	iccom_send_cmd_array.handle          = ((screen_disp_handle *)address->handle)->handle;
	iccom_send_cmd_array.task_id         = TASK_DISPLAY;
	iccom_send_cmd_array.function_id     = EVENT_DISPLAY_SETADDRESS;
	iccom_send_cmd_array.send_mode       = ICCOM_DRV_SYNC;
	iccom_send_cmd_array.send_num        = sizeof(cmd_array)/sizeof(cmd_array[0]);
	iccom_send_cmd_array.send_data       = cmd_array;
	iccom_send_cmd_array.recv_size       = 0;
	iccom_send_cmd_array.recv_data       = NULL;

	result = iccom_drv_send_command_array(&iccom_send_cmd_array);
	if (SMAP_OK != result) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] iccom_drv_send_command_array() ret = [%d]\n",
		__LINE__,
		result);

		return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_set_address);

/* #MU2DSP939 add -S- */
int screen_display_set_hdmi_if_parameters(screen_disp_set_hdmi_if_param *set_hdmi_if_param)
{
	int result;
	iccom_drv_cmd_data      cmd_array[3];
	iccom_drv_send_cmd_array_param iccom_send_cmd_array;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == set_hdmi_if_param) {
		return SMAP_LIB_DISPLAY_PARAERR;
	}
	MSG_MED("[RTAPIK]    |handle          [0x%08X]\n", (unsigned int)set_hdmi_if_param->handle);
	MSG_MED("[RTAPIK]    |ipmode          [%d]\n", set_hdmi_if_param->ipmode);
	MSG_MED("[RTAPIK]    |aspect          [0x%08X]\n", (unsigned int)set_hdmi_if_param->aspect);
	MSG_MED("[RTAPIK]    |hdmi_if_param   [0x%08X]\n", (unsigned int)set_hdmi_if_param->hdmi_if_param);

	if ((NULL == set_hdmi_if_param->handle) ||
		(NULL == set_hdmi_if_param->aspect) ||
		(NULL == set_hdmi_if_param->hdmi_if_param)
	   ) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] address NULL error\n",
		__LINE__);
		return SMAP_LIB_DISPLAY_PARAERR;
	}

	cmd_array[0].size	= sizeof(set_hdmi_if_param->ipmode);
	cmd_array[0].data	= (unsigned int *)&(set_hdmi_if_param->ipmode);
	cmd_array[1].size	= sizeof(screen_disp_aspect);
	cmd_array[1].data	= (unsigned int *)set_hdmi_if_param->aspect;
	cmd_array[2].size	= sizeof(screen_disp_hdmi_if);
	cmd_array[2].data	= (unsigned int *)set_hdmi_if_param->hdmi_if_param;

	iccom_send_cmd_array.handle          = ((screen_disp_handle *)set_hdmi_if_param->handle)->handle;
	iccom_send_cmd_array.task_id         = TASK_DISPLAY2;
	iccom_send_cmd_array.function_id     = EVENT_DISPLAY_SETHDMIIFPARAMETERS;
	iccom_send_cmd_array.send_mode       = ICCOM_DRV_SYNC;
	iccom_send_cmd_array.send_num        = sizeof(cmd_array)/sizeof(cmd_array[0]);
	iccom_send_cmd_array.send_data       = cmd_array;
	iccom_send_cmd_array.recv_size       = 0;
	iccom_send_cmd_array.recv_data       = NULL;

	result = iccom_drv_send_command_array(&iccom_send_cmd_array);
	if (SMAP_OK != result) {
			MSG_ERROR(
			"[RTAPIK] ERR|[%d] iccom_drv_send_command_array() ret = [%d]\n",
			__LINE__,
			result);
			return result;
	}

	MSG_HIGH(
	"[RTAPIK] OUT|[%s][%d] ret = [%d]\n",
	__func__,
	__LINE__,
	SMAP_LIB_DISPLAY_OK);
	return SMAP_LIB_DISPLAY_OK;
}
EXPORT_SYMBOL(screen_display_set_hdmi_if_parameters);
/* #MU2DSP939 add -E- */

void screen_display_delete(screen_disp_delete *disp_delete)
{
	iccom_drv_cleanup_param  iccom_cleanup;
	rtds_memory_drv_cleanup_param mem_cleanup;

	MSG_HIGH("[RTAPIK] IN |[%s]\n", __func__);
	if (NULL == disp_delete) {
		return;
	}
	MSG_MED("[RTAPIK]    |aInfo[0x%08X]\n", (unsigned int)disp_delete->handle);

	if (NULL == disp_delete->handle) {
		MSG_ERROR(
		"[RTAPIK] ERR|[%d] aInfo NULL error\n",
		__LINE__);
	} else {
		mem_cleanup.handle = ((screen_disp_handle *)disp_delete->handle)->rtds_mem_handle;
		rtds_memory_drv_cleanup(&mem_cleanup);
		iccom_cleanup.handle = ((screen_disp_handle *)disp_delete->handle)->handle;
		iccom_drv_cleanup(&iccom_cleanup);
		kfree(disp_delete->handle);
	}

	MSG_HIGH("[RTAPIK] OUT|[%s]\n", __func__);
}
EXPORT_SYMBOL(screen_display_delete);
