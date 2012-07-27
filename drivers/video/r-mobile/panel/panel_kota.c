/*
 * drivers/video/r-mobile/panel/panel_kota.c
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
#include <linux/delay.h>

#include <linux/gpio.h>
#include <mach/r8a73734.h>

#include <video/sh_mobile_lcdc.h>

#include <rtapi/screen_display.h>

/* framebuffer address */
#define R_MOBILE_M_BUFF_ADDR		0x4BC00000

/* panel pixel */
#define R_MOBILE_M_PANEL_PIXEL_WIDTH	480
#define R_MOBILE_M_PANEL_PIXEL_HEIGHT	864

/* panel size (mm) */
#define R_MOBILE_M_PANEL_SIZE_WIDTH	48
#define R_MOBILE_M_PANEL_SIZE_HEIGHT	87

#define R_MOBILE_IRQNO		33

#define LCD_DSITCKCR		0x00000007
#define LCD_DSI0PCKCR		0x00000014
#define LCD_DSI0PHYCR		0x2A83800D
#define LCD_SYSCONF		0x00000703
#define LCD_TIMSET0		0x50006454
#define LCD_TIMSET1		0x000B0113
#define LCD_DSICTRL		0x00000001
#define LCD_VMCTR1		0x0021003E
#define LCD_VMCTR2		0x00E20530
#define LCD_VMLEN1		0x05A00008
#define LCD_VMLEN2		0x00080000
#define LCD_VMLEN3		0x00000000
#define LCD_VMLEN4		0x00000000
#define LCD_DTCTR		0x00000007
#define LCD_MLDHCNR		0x003C007C
#define LCD_MLDHSYNR		0x0001007A
#define LCD_MLDHAJR		0x00000000
#define LCD_MLDVLNR		0x0360036A
#define LCD_MLDVSYNR		0x00020364
#define LCD_MLDMT1R		0x0400000B
#define LCD_LDDCKR		0x0001007C
#define LCD_MLDDCKPAT1R		0x00000000
#define LCD_MLDDCKPAT2R		0x00000000

#define LCD_MASK_DSITCKCR	0x000000BF
#define LCD_MASK_DSI0PCKCR	0x0000703F
#define LCD_MASK_DSI0PHYCR	0x000000FF
#define LCD_MASK_SYSCONF	0x00000F0F
#define LCD_MASK_TIMSET0	0x7FFFF7F7
#define LCD_MASK_TIMSET1	0x003F03FF
#define LCD_MASK_DSICTRL	0x00000601
#define LCD_MASK_VMCTR1		0x00F3F03F
#define LCD_MASK_VMCTR2		0x07E2073B
#define LCD_MASK_VMLEN1		0xFFFFFFFF
#define LCD_MASK_VMLEN2		0xFFFFFFFF
#define LCD_MASK_VMLEN3		0xFFFFFFFF
#define LCD_MASK_VMLEN4		0xFFFF0000
#define LCD_MASK_DTCTR		0x00000002
#define LCD_MASK_MLDHCNR	0x000001FF
#define LCD_MASK_MLDHSYNR	0x000F01FF
#define LCD_MASK_MLDHAJR	0x07070707
#define LCD_MASK_MLDVLNR	0x07FF07FF
#define LCD_MASK_MLDVSYNR	0x000F07FF
#define LCD_MASK_MLDMT1R	0x1F03FCCF
#define LCD_MASK_LDDCKR		0x0007007F
#define LCD_MASK_MLDDCKPAT1R	0x0FFFFFFF
#define LCD_MASK_MLDDCKPAT2R	0xFFFFFFFF

static struct fb_panel_info r_mobile_info = {
	.pixel_width   = R_MOBILE_M_PANEL_PIXEL_WIDTH,
	.pixel_height  = R_MOBILE_M_PANEL_PIXEL_HEIGHT,
	.size_width    = R_MOBILE_M_PANEL_SIZE_WIDTH,
	.size_height   = R_MOBILE_M_PANEL_SIZE_HEIGHT,
	.buff_address  = R_MOBILE_M_BUFF_ADDR,
};

static screen_disp_lcd_if r_mobile_lcd_if_param = {
	LCD_DSITCKCR,
	LCD_DSI0PCKCR,
	LCD_DSI0PHYCR,
	LCD_SYSCONF,
	LCD_TIMSET0,
	LCD_TIMSET1,
	LCD_DSICTRL,
	LCD_VMCTR1,
	LCD_VMCTR2,
	LCD_VMLEN1,
	LCD_VMLEN2,
	LCD_VMLEN3,
	LCD_VMLEN4,
	LCD_DTCTR,
	LCD_MLDHCNR,
	LCD_MLDHSYNR,
	LCD_MLDHAJR,
	LCD_MLDVLNR,
	LCD_MLDVSYNR,
	LCD_MLDMT1R,
	LCD_LDDCKR,
	LCD_MLDDCKPAT1R,
	LCD_MLDDCKPAT2R
};

static screen_disp_lcd_if r_mobile_lcd_if_param_mask = {
	LCD_MASK_DSITCKCR,
	LCD_MASK_DSI0PCKCR,
	LCD_MASK_DSI0PHYCR,
	LCD_MASK_SYSCONF,
	LCD_MASK_TIMSET0,
	LCD_MASK_TIMSET1,
	LCD_MASK_DSICTRL,
	LCD_MASK_VMCTR1,
	LCD_MASK_VMCTR2,
	LCD_MASK_VMLEN1,
	LCD_MASK_VMLEN2,
	LCD_MASK_VMLEN3,
	LCD_MASK_VMLEN4,
	LCD_MASK_DTCTR,
	LCD_MASK_MLDHCNR,
	LCD_MASK_MLDHSYNR,
	LCD_MASK_MLDHAJR,
	LCD_MASK_MLDVLNR,
	LCD_MASK_MLDVSYNR,
	LCD_MASK_MLDMT1R,
	LCD_MASK_LDDCKR,
	LCD_MASK_MLDDCKPAT1R,
	LCD_MASK_MLDDCKPAT2R
};

static int kota_dsi_startsetting(int draw_flag)
{

	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long write_dsi_l;
	screen_disp_draw disp_draw;
	screen_disp_delete disp_delete;
	unsigned char ubSendData044[] = {0x44, 0x03, 0x2C};
	unsigned char ubSendData035[] = {0x35, 0x00 };
	int ret;

	screen_handle =  screen_display_new();

	/* wait */
	msleep(60);

	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = 0x05;
	write_dsi_s.reg_address = 0x11;
	write_dsi_s.write_data  = 0x00;
	write_dsi_s.output_mode = RT_DISPLAY_LCD1;
	write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	/* wait 120ms */
	msleep(120);
	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = 0x15;
	write_dsi_s.reg_address = 0x53;
	write_dsi_s.write_data  = 0x24;
	write_dsi_s.output_mode = RT_DISPLAY_LCD1;
	write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = 0x15;
	write_dsi_s.reg_address = 0x51;
	write_dsi_s.write_data  = 0xFF;
	write_dsi_s.output_mode = RT_DISPLAY_LCD1;
	write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	write_dsi_l.handle = screen_handle;
	write_dsi_l.data_id = 0x39;
	write_dsi_l.data_count = sizeof(ubSendData044);
	write_dsi_l.write_data = ubSendData044;
	write_dsi_l.output_mode = RT_DISPLAY_LCD1;
	write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_OFF;
	write_dsi_l.send_mode = RT_DISPLAY_SEND_MODE_HS;
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_long err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	write_dsi_l.handle = screen_handle;
	write_dsi_l.data_id = 0x39;
	write_dsi_l.data_count = sizeof(ubSendData035);
	write_dsi_l.write_data = ubSendData035;
	write_dsi_l.output_mode = RT_DISPLAY_LCD1;
	write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_OFF;
	write_dsi_l.send_mode = RT_DISPLAY_SEND_MODE_HS;
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_long err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	if (draw_flag) {
		disp_draw.handle = screen_handle;
		disp_draw.output_mode = RT_DISPLAY_LCD1;
		disp_draw.draw_rect.x = 0;
		disp_draw.draw_rect.y = 0;
		disp_draw.draw_rect.width = R_MOBILE_M_PANEL_PIXEL_WIDTH;
		disp_draw.draw_rect.height = R_MOBILE_M_PANEL_PIXEL_HEIGHT;
		disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
		disp_draw.buffer_offset = 0;
		disp_draw.rotate = RT_DISPLAY_ROTATE_270;
		ret = screen_display_draw(&disp_draw);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_draw err!\n");
			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			return -1;
		}
	}

	write_dsi_s.handle      = screen_handle;
	write_dsi_s.data_id     = 0x05;
	write_dsi_s.reg_address = 0x29;
	write_dsi_s.write_data  = 0x00;
	write_dsi_s.output_mode = RT_DISPLAY_LCD1;
	write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int kota_panel_init(unsigned int mem_size)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_set_address set_address;
	screen_disp_delete disp_delete;
	int ret;

	screen_handle =  screen_display_new();

	/* LCD panel reset */
	gpio_direction_output(GPIO_PORT31, 0);

	/* wait 1ms */
	msleep(1);

	gpio_direction_output(GPIO_PORT31, 1);

	set_lcd_if_param.handle = screen_handle;
	set_lcd_if_param.port_no = R_MOBILE_IRQNO;
	set_lcd_if_param.lcd_if_param = &r_mobile_lcd_if_param;
	set_lcd_if_param.lcd_if_param_mask = &r_mobile_lcd_if_param_mask;
	ret = screen_display_set_lcd_if_parameters(&set_lcd_if_param);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_lcd_if_parameters err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	set_address.handle = screen_handle;
	set_address.output_mode = RT_DISPLAY_LCD1;
	set_address.address = R_MOBILE_M_BUFF_ADDR;
	set_address.size = mem_size;
	ret = screen_display_set_address(&set_address);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "screen_display_set_address err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	start_lcd.handle = screen_handle;
	start_lcd.output_mode = RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	ret = kota_dsi_startsetting(0);

	return ret;
}

static int kota_panel_suspend(void)
{
	void *screen_handle;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_delete disp_delete;

	screen_handle =  screen_display_new();

	disp_stop_lcd.handle = screen_handle;
	disp_stop_lcd.output_mode = RT_DISPLAY_LCD1;
	screen_display_stop_lcd(&disp_stop_lcd);

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	/* LCD panel reset */
	gpio_direction_output(GPIO_PORT31, 0);

	return 0;
}

static int kota_panel_resume(void)
{

	void *screen_handle;
	screen_disp_start_lcd disp_start_lcd;
	screen_disp_delete disp_delete;
	int ret;

	screen_handle =  screen_display_new();

	/* LCD panel reset */
	gpio_direction_output(GPIO_PORT31, 0);

	/* wait 1ms */
	msleep(1);

	gpio_direction_output(GPIO_PORT31, 1);

	disp_start_lcd.handle = screen_handle;
	disp_start_lcd.output_mode = RT_DISPLAY_LCD1;
	screen_display_start_lcd(&disp_start_lcd);

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	ret = kota_dsi_startsetting(1);

	return ret;
}

static struct fb_panel_info kota_panel_info(void)
{

	return r_mobile_info;

}

struct fb_panel_func r_mobile_panel_func(int panel)
{

	struct fb_panel_func panel_func;

	memset(&panel_func, 0, sizeof(struct fb_panel_func));

/* support (panel=RT_DISPLAY_LCD1) */

	if (panel == RT_DISPLAY_LCD1) {
		panel_func.panel_init    = kota_panel_init;
		panel_func.panel_suspend = kota_panel_suspend;
		panel_func.panel_resume  = kota_panel_resume;
		panel_func.panel_info    = kota_panel_info;
		panel_func.panel_probe   = NULL;
		panel_func.panel_remove  = NULL;
	}

	return panel_func;
}
