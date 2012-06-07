/*
 * drivers/char/rtapi/rtapi/rtapi.c
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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <rtapi/system_memory.h>
#include <rtapi/screen_display.h>
#include <rtapi/screen_graphics.h>
#include <rtapi/system_standby.h>
#include <rtapi/system_pwmng.h>
#include "rtapi.h"

static struct rtapi_regist_func func;

int regist_rtapi_function(struct rtapi_regist_func *fnc)
{
    if( fnc ){
        /* System - Memory */
        func.fn_system_memory_rt_alloc          = fnc->fn_system_memory_rt_alloc;
        func.fn_system_memory_rt_free           = fnc->fn_system_memory_rt_free;
        func.fn_system_memory_rt_change_rtaddr  = fnc->fn_system_memory_rt_change_rtaddr;
        func.fn_system_memory_rt_change_apaddr  = fnc->fn_system_memory_rt_change_apaddr;
        func.fn_system_memory_ap_open           = fnc->fn_system_memory_ap_open;
        func.fn_system_memory_ap_open_wb        = fnc->fn_system_memory_ap_open_wb;
        func.fn_system_memory_ap_close          = fnc->fn_system_memory_ap_close;
        func.fn_system_memory_ap_alloc          = fnc->fn_system_memory_ap_alloc;
        func.fn_system_memory_ap_free           = fnc->fn_system_memory_ap_free;
        func.fn_system_memory_ap_change_rtaddr  = fnc->fn_system_memory_ap_change_rtaddr;
        func.fn_system_memory_ap_change_apaddr  = fnc->fn_system_memory_ap_change_apaddr;
        func.fn_system_memory_ap_cache_flush    = fnc->fn_system_memory_ap_cache_flush;
        func.fn_system_memory_ap_cache_clear    = fnc->fn_system_memory_ap_cache_clear;
        func.fn_system_memory_ap_buffer_flush   = fnc->fn_system_memory_ap_buffer_flush;
        func.fn_system_memory_ap_get_apmem_id   = fnc->fn_system_memory_ap_get_apmem_id;
        func.fn_system_memory_ap_share_area     = fnc->fn_system_memory_ap_share_area;
        func.fn_system_memory_ap_share_mem_offset = fnc->fn_system_memory_ap_share_mem_offset;
        func.fn_system_memory_ap_share_mem      = fnc->fn_system_memory_ap_share_mem;
        func.fn_system_memory_rt_map            = fnc->fn_system_memory_rt_map;
        func.fn_system_memory_rt_unmap          = fnc->fn_system_memory_rt_unmap;
        func.fn_system_memory_reg_phymem        = fnc->fn_system_memory_reg_phymem;
        func.fn_system_memory_unreg_phymem      = fnc->fn_system_memory_unreg_phymem;
        func.fn_system_memory_phy_change_rtaddr = fnc->fn_system_memory_phy_change_rtaddr;
        func.fn_system_memory_rt_map_pnc        = fnc->fn_system_memory_rt_map_pnc;
        func.fn_system_memory_rt_unmap_pnc      = fnc->fn_system_memory_rt_unmap_pnc;
        func.fn_system_memory_info_new          = fnc->fn_system_memory_info_new;
        func.fn_system_memory_get_rtinfo        = fnc->fn_system_memory_get_rtinfo;
        func.fn_system_memory_get_apinfo        = fnc->fn_system_memory_get_apinfo;
        func.fn_system_memory_info_delete       = fnc->fn_system_memory_info_delete;
        func.fn_system_memory_meram_alloc       = fnc->fn_system_memory_meram_alloc;
        func.fn_system_memory_meram_free        = fnc->fn_system_memory_meram_free;

        /* Screen - Display */
        func.fn_screen_display_new              = fnc->fn_screen_display_new;
        func.fn_screen_display_set_parameters   = fnc->fn_screen_display_set_parameters;
        func.fn_screen_display_set_lcd_refresh  = fnc->fn_screen_display_set_lcd_refresh;
        func.fn_screen_display_start_lcd        = fnc->fn_screen_display_start_lcd;
        func.fn_screen_display_stop_lcd         = fnc->fn_screen_display_stop_lcd;
        func.fn_screen_display_start_hdmi       = fnc->fn_screen_display_start_hdmi;
        func.fn_screen_display_stop_hdmi        = fnc->fn_screen_display_stop_hdmi;
        func.fn_screen_display_get_address      = fnc->fn_screen_display_get_address;
        func.fn_screen_display_draw             = fnc->fn_screen_display_draw;
//#MU2DSP222 add -S-
        func.fn_screen_display_read_dsi_short_packet   = fnc->fn_screen_display_read_dsi_short_packet;
//#MU2DSP222 add -E-
        func.fn_screen_display_write_dsi_short_packet   = fnc->fn_screen_display_write_dsi_short_packet;
        func.fn_screen_display_write_dsi_long_packet    = fnc->fn_screen_display_write_dsi_long_packet;
        func.fn_screen_display_set_lcd_if_parameters    = fnc->fn_screen_display_set_lcd_if_parameters;
        func.fn_screen_display_set_address      = fnc->fn_screen_display_set_address;
        func.fn_screen_display_delete           = fnc->fn_screen_display_delete;

        /* Screen - Graphics */
        func.fn_screen_graphics_new             = fnc->fn_screen_graphics_new;
        func.fn_screen_graphics_initialize      = fnc->fn_screen_graphics_initialize;
        func.fn_screen_graphics_image_conversion= fnc->fn_screen_graphics_image_conversion;
        func.fn_screen_graphics_image_blend     = fnc->fn_screen_graphics_image_blend;
        func.fn_screen_graphics_quit            = fnc->fn_screen_graphics_quit;
        func.fn_screen_graphics_delete          = fnc->fn_screen_graphics_delete;

        /* Rt-domain power manegement */
        func.fn_system_rt_standby               = fnc->fn_system_rt_standby;
        func.fn_system_rt_active                = fnc->fn_system_rt_active;
        func.fn_system_pwmng_new                        = fnc->fn_system_pwmng_new;
        func.fn_system_pwmng_powerarea_start_notify     = fnc->fn_system_pwmng_powerarea_start_notify;
        func.fn_system_pwmng_powerarea_end_notify       = fnc->fn_system_pwmng_powerarea_end_notify;
        func.fn_system_pwmng_delete                     = fnc->fn_system_pwmng_delete;

    }
    else{
        memset( &func,0,sizeof(func) );
    }
    return 0;
}
EXPORT_SYMBOL(regist_rtapi_function);

struct rtapi_regist_func *get_rtapi_function_interface(void)
{
    return &func;
}

/* System - Memory */
int system_memory_rt_alloc
(
    system_mem_rt_alloc* rt_alloc
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_alloc ){
        ret = func.fn_system_memory_rt_alloc( rt_alloc );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_alloc);

int system_memory_rt_free
(
    system_mem_rt_free* rt_free
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_free ){
        ret = func.fn_system_memory_rt_free( rt_free );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_free);

int system_memory_rt_change_rtaddr
(
    system_mem_rt_change_rtaddr* rt_change_rtaddr
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_change_rtaddr ){
        ret = func.fn_system_memory_rt_change_rtaddr( rt_change_rtaddr );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_change_rtaddr);

int system_memory_rt_change_apaddr
(
    system_mem_rt_change_apaddr* rt_change_apaddr
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_change_apaddr ){
        ret = func.fn_system_memory_rt_change_apaddr( rt_change_apaddr );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_change_apaddr);

int system_memory_ap_open
(
    system_mem_ap_open*   ap_open
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_open ){
        ret = func.fn_system_memory_ap_open( ap_open );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_open);

int system_memory_ap_open_wb
(
    system_mem_ap_open*   ap_open
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_open_wb ){
        ret = func.fn_system_memory_ap_open_wb( ap_open );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_open_wb);

int system_memory_ap_close
(
    system_mem_ap_close*  ap_close
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_close ){
        ret = func.fn_system_memory_ap_close( ap_close );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_close);

int system_memory_ap_alloc
(
    system_mem_ap_alloc*  ap_alloc
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_alloc ){
        ret = func.fn_system_memory_ap_alloc( ap_alloc );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_alloc);

int system_memory_ap_free
(
    system_mem_ap_free*   ap_free
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_free ){
        ret = func.fn_system_memory_ap_free( ap_free );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_free);

int system_memory_ap_change_rtaddr
(
    system_mem_ap_change_rtaddr* ap_change_rtaddr
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_change_rtaddr ){
        ret = func.fn_system_memory_ap_change_rtaddr( ap_change_rtaddr );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_change_rtaddr);

int system_memory_ap_change_apaddr
(
    system_mem_ap_change_apaddr* ap_change_apaddr
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_change_apaddr ){
        ret = func.fn_system_memory_ap_change_apaddr( ap_change_apaddr );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_change_apaddr);

int system_memory_ap_cache_flush
(
    system_mem_ap_cache_flush* ap_cache_flush
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_cache_flush ){
        ret = func.fn_system_memory_ap_cache_flush( ap_cache_flush );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_cache_flush);

int system_memory_ap_cache_clear
(
    system_mem_ap_cache_clear* ap_cache_clear
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_cache_clear ){
        ret = func.fn_system_memory_ap_cache_clear( ap_cache_clear );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_cache_clear);

void system_memory_ap_buffer_flush
(
    system_mem_ap_buffer_flush* apm_buffer_flush
){

    if( func.fn_system_memory_ap_buffer_flush ){
        func.fn_system_memory_ap_buffer_flush( apm_buffer_flush );
    }
    return;
}
EXPORT_SYMBOL(system_memory_ap_buffer_flush);

unsigned int system_memory_ap_get_apmem_id
(
    system_mem_ap_get_apmem_id* ap_get_apmem_id
){
    unsigned int ret = 0;

    if( func.fn_system_memory_ap_get_apmem_id ){
        ret = func.fn_system_memory_ap_get_apmem_id( ap_get_apmem_id );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_get_apmem_id);

int system_memory_ap_share_area
(
    system_mem_ap_share_area* ap_share_area
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_share_area ){
        ret = func.fn_system_memory_ap_share_area( ap_share_area );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_share_area);

int system_memory_ap_share_mem_offset
(
    system_mem_ap_share_mem_offset* ap_share_mem_offset
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_share_mem_offset ){
        ret = func.fn_system_memory_ap_share_mem_offset( ap_share_mem_offset );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_share_mem_offset);

int system_memory_ap_share_mem
(
    system_mem_ap_share_mem* ap_share_mem
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_ap_share_mem ){
        ret = func.fn_system_memory_ap_share_mem( ap_share_mem );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_ap_share_mem);

int system_memory_rt_map
(
    system_mem_rt_map*   rt_map
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_map ){
        ret = func.fn_system_memory_rt_map( rt_map );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_map);

int system_memory_rt_unmap
(
    system_mem_rt_unmap*  rt_unmap
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_unmap ){
        ret = func.fn_system_memory_rt_unmap( rt_unmap );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_unmap);

int system_memory_reg_phymem
(
    system_mem_reg_phymem* reg_phymem
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_reg_phymem ){
        ret = func.fn_system_memory_reg_phymem( reg_phymem );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_reg_phymem);

int system_memory_unreg_phymem
(
    system_mem_unreg_phymem* unreg_phymem
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_unreg_phymem ){
        ret = func.fn_system_memory_unreg_phymem( unreg_phymem );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_unreg_phymem);

int system_memory_phy_change_rtaddr
(
    system_mem_phy_change_rtaddr*   phy_change_rtaddr
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_phy_change_rtaddr ){
        ret = func.fn_system_memory_phy_change_rtaddr( phy_change_rtaddr );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_phy_change_rtaddr);

int system_memory_rt_map_pnc
(
    system_mem_rt_map_pnc*   rt_map_pnc
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_map_pnc ){
        ret = func.fn_system_memory_rt_map_pnc( rt_map_pnc );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_map_pnc);

int system_memory_rt_unmap_pnc
(
    system_mem_rt_unmap_pnc*   rt_unmap_pnc
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_rt_unmap_pnc ){
        ret = func.fn_system_memory_rt_unmap_pnc( rt_unmap_pnc );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_rt_unmap_pnc);

void* system_memory_info_new
(
    void
){
    void* ret = NULL;

    if( func.fn_system_memory_info_new ){
        ret = func.fn_system_memory_info_new();
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_info_new);

int system_memory_get_rtinfo
(
    system_mem_get_rtinfo*  get_rtinfo
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_get_rtinfo ){
        ret = func.fn_system_memory_get_rtinfo( get_rtinfo );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_get_rtinfo);

int system_memory_get_apinfo
(
    system_mem_get_apinfo*  get_apinfo
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_get_apinfo ){
        ret = func.fn_system_memory_get_apinfo( get_apinfo );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_get_apinfo);

void system_memory_info_delete
(
    system_mem_info_delete*  info_delete
){
    if( func.fn_system_memory_info_delete ){
        func.fn_system_memory_info_delete( info_delete );
    }
    return;
}
EXPORT_SYMBOL(system_memory_info_delete);

int system_memory_meram_alloc
(
    system_mem_meram_alloc*  meram_alloc
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_meram_alloc ){
        ret = func.fn_system_memory_meram_alloc( meram_alloc );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_meram_alloc);

int system_memory_meram_free
(
    system_mem_meram_free*  meram_free
){
    int ret = SMAP_LIB_MEMORY_NG;

    if( func.fn_system_memory_meram_free ){
        ret = func.fn_system_memory_meram_free( meram_free );
    }
    return ret;
}
EXPORT_SYMBOL(system_memory_meram_free);

/* Screen - Display */
void* screen_display_new
(
    void
){
    void* ret = NULL;

    if( func.fn_screen_display_new ){
        ret = func.fn_screen_display_new();
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_new);

int screen_display_set_parameters
(
    screen_disp_param* disp_param
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_set_parameters ){
        ret = func.fn_screen_display_set_parameters( disp_param );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_set_parameters);

int screen_display_set_lcd_refresh
(
    screen_disp_set_lcd_refresh* set_lcd_refresh
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_set_lcd_refresh ){
        ret = func.fn_screen_display_set_lcd_refresh( set_lcd_refresh );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_set_lcd_refresh);

int screen_display_start_lcd
(
    screen_disp_start_lcd* start_lcd
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_start_lcd ){
        ret = func.fn_screen_display_start_lcd( start_lcd );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_start_lcd);

int screen_display_stop_lcd
(
    screen_disp_stop_lcd* stop_lcd
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_stop_lcd ){
        ret = func.fn_screen_display_stop_lcd( stop_lcd );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_stop_lcd);

int screen_display_start_hdmi
(
    screen_disp_start_hdmi* start_hdmi
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_start_hdmi ){
        ret = func.fn_screen_display_start_hdmi( start_hdmi );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_start_hdmi);

int screen_display_stop_hdmi
(
    screen_disp_stop_hdmi* stop_hdmi
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_stop_hdmi ){
        ret = func.fn_screen_display_stop_hdmi( stop_hdmi );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_stop_hdmi);

int screen_display_get_address
(
    screen_disp_get_address* address
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_get_address ){
        ret = func.fn_screen_display_get_address( address );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_get_address);

int screen_display_draw
(
    screen_disp_draw* disp_draw
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_draw ){
        ret = func.fn_screen_display_draw( disp_draw );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_draw);

//#MU2DSP222 add -S-
int screen_display_read_dsi_short_packet
(
	screen_disp_read_dsi_short* read_dsi_s
){
	int ret = SMAP_LIB_DISPLAY_NG;
	
	if( func.fn_screen_display_read_dsi_short_packet ){
		ret = func.fn_screen_display_read_dsi_short_packet( read_dsi_s );
	}
	return ret;
}
//#MU2DSP222 add -E-
EXPORT_SYMBOL(screen_display_read_dsi_short_packet);

int screen_display_write_dsi_short_packet
(
    screen_disp_write_dsi_short* write_dsi_s
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_write_dsi_short_packet ){
        ret = func.fn_screen_display_write_dsi_short_packet( write_dsi_s );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_write_dsi_short_packet);

int screen_display_write_dsi_long_packet
(
    screen_disp_write_dsi_long* write_dsi_l
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_write_dsi_long_packet ){
        ret = func.fn_screen_display_write_dsi_long_packet( write_dsi_l );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_write_dsi_long_packet);

int screen_display_set_lcd_if_parameters
(
    screen_disp_set_lcd_if_param* set_lcd_if_param
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_set_lcd_if_parameters ){
        ret = func.fn_screen_display_set_lcd_if_parameters( set_lcd_if_param );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_set_lcd_if_parameters);

int screen_display_set_address
(
    screen_disp_set_address* address
){
    int ret = SMAP_LIB_DISPLAY_NG;

    if( func.fn_screen_display_set_address ){
        ret = func.fn_screen_display_set_address( address );
    }
    return ret;
}
EXPORT_SYMBOL(screen_display_set_address);

void screen_display_delete
(
    screen_disp_delete* disp_delete
){
    if( func.fn_screen_display_delete ){
        func.fn_screen_display_delete( disp_delete );
    }
    return;
}
EXPORT_SYMBOL(screen_display_delete);

/* Screen - Graphics */
void* screen_graphics_new
(
    screen_grap_new* grap_new
){
    void* ret = NULL;

    if( func.fn_screen_graphics_new ){
        ret = func.fn_screen_graphics_new( grap_new );
    }
    return ret;
}
EXPORT_SYMBOL(screen_graphics_new);

int screen_graphics_initialize
(
    screen_grap_initialize* grap_initialize
){
    int ret = SMAP_LIB_GRAPHICS_NG;

    if( func.fn_screen_graphics_initialize ){
        ret = func.fn_screen_graphics_initialize( grap_initialize );
    }
    return ret;
}
EXPORT_SYMBOL(screen_graphics_initialize);

int screen_graphics_image_conversion
(
    screen_grap_image_conv* grap_image_conv
){
    int ret = SMAP_LIB_GRAPHICS_NG;

    if( func.fn_screen_graphics_image_conversion ){
        ret = func.fn_screen_graphics_image_conversion( grap_image_conv );
    }
    return ret;
}
EXPORT_SYMBOL(screen_graphics_image_conversion);

int screen_graphics_image_blend
(
    screen_grap_image_blend* grap_image_blend
){
    int ret = SMAP_LIB_GRAPHICS_NG;

    if( func.fn_screen_graphics_image_blend ){
        ret = func.fn_screen_graphics_image_blend( grap_image_blend );
    }
    return ret;
}
EXPORT_SYMBOL(screen_graphics_image_blend);

int screen_graphics_quit
(
    screen_grap_quit* grap_quit
){
    int ret = SMAP_LIB_GRAPHICS_NG;

    if( func.fn_screen_graphics_quit ){
        ret = func.fn_screen_graphics_quit( grap_quit );
    }
    return ret;
}
EXPORT_SYMBOL(screen_graphics_quit);

void screen_graphics_delete
(
    screen_grap_delete* grap_delete
){
    if( func.fn_screen_graphics_delete ){
        func.fn_screen_graphics_delete( grap_delete );
    }
    return;
}
EXPORT_SYMBOL(screen_graphics_delete);

int system_rt_standby(void)
{
    int ret = SMAP_LIB_STANDBY_NG;
    
    if( func.fn_system_rt_standby ){
        ret = func.fn_system_rt_standby();
    }
    return ret;
}
EXPORT_SYMBOL(system_rt_standby);

int system_rt_active(void)
{
    int ret = SMAP_LIB_STANDBY_NG;
    
    if( func.fn_system_rt_active ){
        ret = func.fn_system_rt_active();
    }
    return ret;
}
EXPORT_SYMBOL(system_rt_active);

void* system_pwmng_new
(
	void
)
{
    void *ret = NULL;

    if ( func.fn_system_pwmng_new ){
         ret = func.fn_system_pwmng_new();
    }
    return ret;
}
EXPORT_SYMBOL(system_pwmng_new);

int system_pwmng_powerarea_start_notify
(
    system_pmg_param* pwmng_param
)
{
    int ret = SMAP_LIB_PWMNG_NG;

    if( func.fn_system_pwmng_powerarea_start_notify ){
        ret = func.fn_system_pwmng_powerarea_start_notify( pwmng_param );
    }
    return ret;
}
EXPORT_SYMBOL(system_pwmng_powerarea_start_notify);

int system_pwmng_powerarea_end_notify
(
    system_pmg_param* pwmng_param
)
{
    int ret = SMAP_LIB_PWMNG_NG;

    if( func.fn_system_pwmng_powerarea_end_notify ){
        ret = func.fn_system_pwmng_powerarea_end_notify( pwmng_param );
    }
    return ret;
}
EXPORT_SYMBOL(system_pwmng_powerarea_end_notify);

void system_pwmng_delete
(
     system_pmg_delete* pwmng_delete
)
{
    if( func.fn_system_pwmng_delete ){
        func.fn_system_pwmng_delete( pwmng_delete );
    }
    return;
}
EXPORT_SYMBOL(system_pwmng_delete);

