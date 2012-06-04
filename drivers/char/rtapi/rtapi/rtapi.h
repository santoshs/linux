/*
 * drivers/char/rtapi/rtapi/rtapi.h
 *     This file is RT-API wrapper function.
 *
 * Copyright (C) 2011-2012 Renesas Electronics Corporation
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

#ifndef __RTAPI_WRAPPER_H__
#define __RTAPI_WRAPPER_H__

/*-------------------------------------------------------*/
/* struct                                                */
/*-------------------------------------------------------*/

struct rtapi_regist_func{
    /* System - Memory */
	int	(*fn_system_memory_rt_alloc)(system_mem_rt_alloc*);
	int	(*fn_system_memory_rt_free)(system_mem_rt_free*);
	int	(*fn_system_memory_rt_change_rtaddr)(system_mem_rt_change_rtaddr*);
	int	(*fn_system_memory_rt_change_apaddr)(system_mem_rt_change_apaddr*);
	int	(*fn_system_memory_ap_open)(system_mem_ap_open*);
	int	(*fn_system_memory_ap_open_wb)(system_mem_ap_open*);
	int	(*fn_system_memory_ap_close)(system_mem_ap_close*);
	int	(*fn_system_memory_ap_alloc)(system_mem_ap_alloc*);
	int	(*fn_system_memory_ap_free)(system_mem_ap_free*);
	int	(*fn_system_memory_ap_change_rtaddr)(system_mem_ap_change_rtaddr*);
	int	(*fn_system_memory_ap_change_apaddr)(system_mem_ap_change_apaddr*);
	int	(*fn_system_memory_ap_cache_flush)(system_mem_ap_cache_flush*);
	int	(*fn_system_memory_ap_cache_clear)(system_mem_ap_cache_clear*);
	void(*fn_system_memory_ap_buffer_flush)(system_mem_ap_buffer_flush*);
	unsigned int (*fn_system_memory_ap_get_apmem_id)(system_mem_ap_get_apmem_id*);
	int (*fn_system_memory_ap_share_area)(system_mem_ap_share_area*);
	int (*fn_system_memory_ap_share_mem_offset)(system_mem_ap_share_mem_offset*);
	int (*fn_system_memory_ap_share_mem)(system_mem_ap_share_mem*);
	int (*fn_system_memory_rt_map)(system_mem_rt_map*);
	int (*fn_system_memory_rt_unmap)(system_mem_rt_unmap*);
	int (*fn_system_memory_reg_phymem)(system_mem_reg_phymem*);
	int (*fn_system_memory_unreg_phymem)(system_mem_unreg_phymem*);
	int (*fn_system_memory_phy_change_rtaddr)(system_mem_phy_change_rtaddr*);
	int (*fn_system_memory_rt_map_pnc)(system_mem_rt_map_pnc*);
	int (*fn_system_memory_rt_unmap_pnc)(system_mem_rt_unmap_pnc*);
	void* (*fn_system_memory_info_new)(void);
	int (*fn_system_memory_get_rtinfo)(system_mem_get_rtinfo*);
	int (*fn_system_memory_get_apinfo)(system_mem_get_apinfo*);
	void (*fn_system_memory_info_delete)(system_mem_info_delete*);
	int (*fn_system_memory_meram_alloc)(system_mem_meram_alloc*);
	int (*fn_system_memory_meram_free)(system_mem_meram_free*);

    /* Screen - Display */
	void* (*fn_screen_display_new)(void);
	int (*fn_screen_display_set_lcd_refresh)(screen_disp_set_lcd_refresh*);
	int (*fn_screen_display_start_lcd)(screen_disp_start_lcd*);
	int (*fn_screen_display_stop_lcd)(screen_disp_stop_lcd*);
	int (*fn_screen_display_start_hdmi)(screen_disp_start_hdmi*);
	int (*fn_screen_display_stop_hdmi)(screen_disp_stop_hdmi*);
	int (*fn_screen_display_get_address)(screen_disp_get_address*);
	int (*fn_screen_display_draw)(screen_disp_draw*);
/* #MU2DSP222 add -S- */
	int (*fn_screen_display_read_dsi_short_packet)(screen_disp_read_dsi_short*);
/* #MU2DSP222 add -E- */
	int (*fn_screen_display_write_dsi_short_packet)(screen_disp_write_dsi_short*);
	int (*fn_screen_display_write_dsi_long_packet)(screen_disp_write_dsi_long*);
	int (*fn_screen_display_set_lcd_if_parameters)(screen_disp_set_lcd_if_param*);
	void (*fn_screen_display_delete)(screen_disp_delete*);

    /* Screen - Graphics */
	void* (*fn_screen_graphics_new)(screen_grap_new*);
	int  (*fn_screen_graphics_initialize)(screen_grap_initialize*);
	int  (*fn_screen_graphics_image_conversion)(screen_grap_image_conv*);
	int  (*fn_screen_graphics_image_blend)(screen_grap_image_blend*);
	int  (*fn_screen_graphics_image_output)(screen_grap_image_output*);
	int  (*fn_screen_graphics_quit)(screen_grap_quit*);
	void (*fn_screen_graphics_delete)(screen_grap_delete*);
	
	/* Rt-domain power manegement */
	int (*fn_system_rt_standby)(void);
	int (*fn_system_rt_active)(void);
	void* (*fn_system_pwmng_new)(void);
	int (*fn_system_pwmng_powerarea_start_notify)(system_pmg_param*);
	int (*fn_system_pwmng_powerarea_end_notify)(system_pmg_param*);
	void (*fn_system_pwmng_delete)(system_pmg_delete*);
	
};

/*-------------------------------------------------------*/
/* RT-API Wrapper register function                      */
/*-------------------------------------------------------*/
int regist_rtapi_function(struct rtapi_regist_func *fnc);

struct rtapi_regist_func *get_rtapi_function_interface(void);

#endif	/* __RTAPI_WRAPPER_H__ */
