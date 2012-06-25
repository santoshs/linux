/*
 * drivers/video/sh_mobile_lcdcfb.c
 *
 * Copyright (C) 2011 Renesas Electronics Corporation
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
/*
 * SuperH Mobile LCDC Framebuffer
 *
 * Copyright (c) 2008 Magnus Damm
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <video/sh_mobile_lcdc.h>
/*#include <media/sh_mobile_overlay.h>*/
#include <linux/atomic.h>
#include <linux/uaccess.h>

#include <linux/gpio.h>
#include <mach/r8a73734.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#include <rtapi/screen_display.h>
#include <rtapi/screen_common.h>

#define LCD_BG_BLACK 0x0

#define FB_SH_MOBILE_REFRESH 0

#define REFRESH_TIME_MSEC 100

#if FB_SH_MOBILE_REFRESH
#include <linux/mfis.h>
#define COUNT_MFIS_SUSPEND 10
#endif

#define FB_SH_MOBILE_HDMI 1

#define CHAN_NUM 2

#define PALETTE_NR 16
#define SIDE_B_OFFSET 0x1000
#define MIRROR_OFFSET 0x2000

#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#include <linux/sh_mobile_composer.h>
#endif

struct sh_mobile_lcdc_priv;
struct sh_mobile_lcdc_chan {
	struct sh_mobile_lcdc_priv *lcdc;
	struct sh_mobile_lcdc_chan_cfg cfg;
	u32 pseudo_palette[PALETTE_NR];
	struct fb_info *info;
	dma_addr_t dma_handle;
	unsigned long pan_offset;
};

struct sh_mobile_lcdc_priv {
	struct device *dev;
	struct sh_mobile_lcdc_chan ch[CHAN_NUM];
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend    early_suspend;
#endif /* CONFIG_HAS_EARLYSUSPEND */

};

struct sh_mobile_lcdc_ext_param {
	struct semaphore sem_lcd;
	int lcd_type;
	void *aInfo;
	unsigned short o_mode;
	unsigned short draw_mode;
	unsigned int phy_addr;
	unsigned int vir_addr;
	unsigned short rect_x;
	unsigned short rect_y;
	unsigned short rect_width;
	unsigned short rect_height;
	unsigned short alpha;
	unsigned short key_clr;
	unsigned short v4l2_state;
	unsigned short draw_bpp;
	unsigned int mem_size;
	unsigned short delay_flag;
	unsigned short refresh_on;
	struct delayed_work ref_work;
	unsigned short mfis_err_flag;
	unsigned short rotate;
};

struct sh_mobile_lcdc_ext_param lcd_ext_param[CHAN_NUM];

struct semaphore   sh_mobile_sem_hdmi;

#if FB_SH_MOBILE_REFRESH
struct workqueue_struct *sh_mobile_wq;
#endif

static u32 fb_debug;
#define DBG_PRINT(FMT, ARGS...)	      \
	if (fb_debug)		   \
		printk(KERN_INFO "%s(): " FMT, __func__, ##ARGS)

#if 1 // SSG qHD Display
struct _s6e39a0x02_cmdset {
	unsigned char cmd;
	unsigned char *data;
	int size;
};
#define MIPI_DSI_DCS_LONG_WRITE		(0x39)
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM	(0x15)
#define MIPI_DSI_DCS_SHORT_WRITE	(0x05)
#define MIPI_DSI_DELAY			(0x00)
#define MIPI_DSI_BLACK			(0x01)
#define MIPI_DSI_END			(0xFF)

unsigned char data_to_send_01[] = { 0xf0, 0x5a, 0x5a };
unsigned char data_to_send_02[] = { 0xf1, 0x5a, 0x5a };
unsigned char data_to_send_03[] = { 0xfc, 0x5a, 0x5a };

unsigned char data_to_send_04[] = { 0xfa, 0x02, 0x58, 0x42, 0x56, 0xaa, 0xc8, 0xae, 0xb5, 0xc1, 0xbe, 0xb4, 0xc0, 0xb2, 0x93, 0x9f, 0x93, 0xa6, 0xad, 0xa2, 0x00, 0xe9, 0x00, 0xdb, 0x01, 0x0f };
unsigned char data_to_send_05[] = { 0xfa, 0x03 };

unsigned char data_to_send_06[] = { 0xf8, 0x27, 0x27, 0x08, 0x08, 0x4e, 0xaa, 0x5e, 0x8a, 0x10, 0x3f, 0x10, 0x10, 0x00 };
unsigned char data_to_send_07[] = { 0xf7, 0x03 };
unsigned char data_to_send_08[] = { 0xb3, 0x63, 0x02, 0xc3, 0x32, 0xff };

unsigned char data_to_send_09[] = { 0xf6, 0x00, 0x84, 0x09 };
unsigned char data_to_send_10[] = { 0xb0, 0x09 };
unsigned char data_to_send_11[] = { 0xd5, 0x64 };
unsigned char data_to_send_12[] = { 0xb0, 0x0b };
unsigned char data_to_send_13[] = { 0xd5, 0xa4, 0x7e, 0x20 };
unsigned char data_to_send_14[] = { 0xb0, 0x08 };
unsigned char data_to_send_15[] = { 0xfd, 0xf8 };
unsigned char data_to_send_16[] = { 0xb0, 0x01 };
unsigned char data_to_send_17[] = { 0xf2, 0x07 };
unsigned char data_to_send_18[] = { 0xb0, 0x04 };
unsigned char data_to_send_19[] = { 0xf2, 0x4d };
unsigned char data_to_send_20[] = { 0xb1, 0x01, 0x00, 0x16 };
unsigned char data_to_send_21[] = { 0xb2, 0x15, 0x15, 0x15, 0x15 };
unsigned char data_to_send_22[] = { 0x11, 0x00 };	/* Sleep Out */

unsigned char data_to_send_23[] = { 0x2a, 0x00, 0x00, 0x02, 0x57 };
unsigned char data_to_send_24[] = { 0x2b, 0x00, 0x00, 0x03, 0xff };
unsigned char data_to_send_25[] = { 0x2c, 0x00 };

unsigned char data_to_send_26[] = { 0x35, 0x00 };
unsigned char data_to_send_27[] = { 0x2a, 0x00, 0x1e, 0x02, 0x39 };	/* panel size is qHD 540x960 */
unsigned char data_to_send_28[] = { 0x2b, 0x00, 0x00, 0x03, 0xbf };	/* panel size is qHD 540x960 */
unsigned char data_to_send_29[] = { 0xd1, 0x8a };

unsigned char data_to_send_30[] = { 0x29, 0x00 };	/* Display On */

struct _s6e39a0x02_cmdset s6e39a0x02_cmdset[]= {
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_01,	sizeof(data_to_send_01)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_02,	sizeof(data_to_send_02)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_03,	sizeof(data_to_send_03)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_04,	sizeof(data_to_send_04)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_05,	sizeof(data_to_send_05)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_06,	sizeof(data_to_send_06)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_07,	sizeof(data_to_send_07)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_08,	sizeof(data_to_send_08)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_09,	sizeof(data_to_send_09)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_10,	sizeof(data_to_send_10)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_11,	sizeof(data_to_send_11)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_12,	sizeof(data_to_send_12)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_13,	sizeof(data_to_send_13)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_14,	sizeof(data_to_send_14)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_15,	sizeof(data_to_send_15)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_16,	sizeof(data_to_send_16)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_17,	sizeof(data_to_send_17)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_18,	sizeof(data_to_send_18)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_19,	sizeof(data_to_send_19)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_20,	sizeof(data_to_send_20)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_21,	sizeof(data_to_send_21)	},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_22,	sizeof(data_to_send_22)	},
	{MIPI_DSI_DELAY,		NULL,			120			},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_23,	sizeof(data_to_send_23)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_24,	sizeof(data_to_send_24)	},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_25,	sizeof(data_to_send_25)	},
	{MIPI_DSI_DELAY,		NULL,			20			},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_26,	sizeof(data_to_send_26)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_27,	sizeof(data_to_send_27)	},
	{MIPI_DSI_DCS_LONG_WRITE,	data_to_send_28,	sizeof(data_to_send_28)	},
	{MIPI_DSI_DCS_SHORT_WRITE_PARAM,data_to_send_29,	sizeof(data_to_send_29)	},
	{MIPI_DSI_BLACK,		NULL,			0			},
	{MIPI_DSI_DCS_SHORT_WRITE,	data_to_send_30,	sizeof(data_to_send_30)	},
	{MIPI_DSI_END,			NULL,			0			},
};

#endif


module_param(fb_debug, int, 0644);
MODULE_PARM_DESC(fb_debug, "SH LCD debug level");

static int sh_mobile_lcdc_setcolreg(u_int regno,
				    u_int red, u_int green, u_int blue,
				    u_int transp, struct fb_info *info)
{
	/* No. of hw registers */
	if (regno >= 256)
		return 1;

	/* grayscale works only partially under directcolor */
	if (info->var.grayscale) {
		/* grayscale = 0.30*R + 0.59*G + 0.11*B */
		red = green = blue = (red * 77 + green * 151 + blue * 28) >> 8;
	}

#define CNVT_TOHW(val, width) ((((val)<<(width))+0x7FFF-(val))>>16)
	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:	/* FALL THROUGH */
	case FB_VISUAL_PSEUDOCOLOR:
	{
		red = CNVT_TOHW(red, info->var.red.length);
		green = CNVT_TOHW(green, info->var.green.length);
		blue = CNVT_TOHW(blue, info->var.blue.length);
		transp = CNVT_TOHW(transp, info->var.transp.length);
		break;
	}
	case FB_VISUAL_DIRECTCOLOR:
	{
		red = CNVT_TOHW(red, 8);	/* expect 8 bit DAC */
		green = CNVT_TOHW(green, 8);
		blue = CNVT_TOHW(blue, 8);
		/* hey, there is bug in transp handling... */
		transp = CNVT_TOHW(transp, 8);
		break;
	}
	}
#undef CNVT_TOHW
	/* Truecolor has hardware independent palette */
	if (info->fix.visual == FB_VISUAL_TRUECOLOR) {
		u32 v;

		if (regno >= 16)
			return 1;

		v = (red << info->var.red.offset) |
		    (green << info->var.green.offset) |
		    (blue << info->var.blue.offset) |
		    (transp << info->var.transp.offset);
		switch (info->var.bits_per_pixel) {
		case 16:	/* FALL THROUGH */
		case 24:	/* FALL THROUGH */
		case 32:
			((u32 *) (info->pseudo_palette))[regno] = v;
			break;
		case 8:		/* FALL THROUGH */
		default:
			break;
		}
	}

	return 0;
}

static struct fb_fix_screeninfo sh_mobile_lcdc_fix  = {
	.id		= "SH Mobile LCDC",
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_TRUECOLOR,
	.accel		= FB_ACCEL_NONE,
	.xpanstep	= 0,
	.ypanstep	= 1,
	.ywrapstep	= 0,
};

static int display_initialize(int lcd_num)
{
	/*screen_disp_param disp_param;*/
	screen_disp_get_address disp_addr;
	screen_disp_delete disp_delete;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;

	int ret = 0;

	lcd_ext_param[lcd_num].aInfo = screen_display_new();
	if (lcd_ext_param[lcd_num].aInfo == NULL)
		return -2;

#if 0
	disp_param.handle = lcd_ext_param[lcd_num].aInfo;
	disp_param.output_mode = lcd_ext_param[lcd_num].o_mode;
	disp_param.key_color = lcd_ext_param[lcd_num].key_clr;
	disp_param.alpha = lcd_ext_param[lcd_num].alpha;

	ret = screen_display_set_parameters(&disp_param);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_parameters err!\n");
		disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
		screen_display_delete(&disp_delete);
		return -1;
	}
#endif

	disp_addr.handle = lcd_ext_param[lcd_num].aInfo;
	disp_addr.output_mode = lcd_ext_param[lcd_num].o_mode;
	ret = screen_display_get_address(&disp_addr);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_get_address err!\n");
		disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
		screen_display_delete(&disp_delete);
		return -1;
	}

#if 1

	gpio_direction_output(GPIO_PORT89, 1);
	mdelay(25);
	gpio_direction_output(GPIO_PORT31, 1);
	mdelay(10);

	// SSG qHD Display
	printk(KERN_ALERT "S6E39A0X02 qHD LCD Initialize Start\n");
	if (lcd_ext_param[lcd_num].o_mode == RT_DISPLAY_LCD1) {
		int loop = 0;
		while(0 <= loop) {
			switch(s6e39a0x02_cmdset[loop].cmd){
			case MIPI_DSI_DCS_LONG_WRITE:
				printk(KERN_ALERT "S6E39A0X02 LONG Write\n");
				write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
				write_dsi_l.output_mode	= lcd_ext_param[lcd_num].o_mode;
				write_dsi_l.data_id	= MIPI_DSI_DCS_LONG_WRITE;
				write_dsi_l.data_count	= s6e39a0x02_cmdset[loop].size;
				write_dsi_l.write_data	= s6e39a0x02_cmdset[loop].data;
				ret = screen_display_write_dsi_long_packet(&write_dsi_l);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					printk(KERN_ALERT "screen_display_write_dsi_long_packet err 1!\n");
					disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
					screen_display_delete(&disp_delete);
					return -1;
				}
				break;
			case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
				printk(KERN_ALERT "S6E39A0X02 SHORT Write with\n");
				write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
				write_dsi_s.data_id	= MIPI_DSI_DCS_SHORT_WRITE_PARAM;
				write_dsi_s.reg_address = s6e39a0x02_cmdset[loop].data[0];
				write_dsi_s.write_data	= s6e39a0x02_cmdset[loop].data[1];
				write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
				ret = screen_display_write_dsi_short_packet(&write_dsi_s);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					printk(KERN_ALERT "disp_write_dsi_short err 1!\n");
					disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
					screen_display_delete(&disp_delete);
					return -1;
				}
				break;
			case MIPI_DSI_DCS_SHORT_WRITE:
				printk(KERN_ALERT "S6E39A0X02 SHORT Write\n");
				write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
				write_dsi_s.data_id	= MIPI_DSI_DCS_SHORT_WRITE;
				write_dsi_s.reg_address = s6e39a0x02_cmdset[loop].data[0];
				write_dsi_s.write_data	= 0x00;
				write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
				ret = screen_display_write_dsi_short_packet(&write_dsi_s);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					printk(KERN_ALERT "disp_write_dsi_short err 1!\n");
					disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
					screen_display_delete(&disp_delete);
					return -1;
				}
				break;
			case MIPI_DSI_BLACK:
			{
				u32 line_num;
				u32 line_size = 540*3 + 1;
				unsigned char *line_data = kmalloc(line_size, GFP_KERNEL);
				memset(line_data,0,line_size);
				
				printk(KERN_ALERT "S6E39A0X02 Black Paint\n");
				
				/* 1st line */
				*line_data = 0x2C;
				write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
				write_dsi_l.output_mode	= lcd_ext_param[lcd_num].o_mode;
				write_dsi_l.data_id	= MIPI_DSI_DCS_LONG_WRITE;
				write_dsi_l.data_count	= line_size;
				write_dsi_l.write_data	= line_data;
				ret = screen_display_write_dsi_long_packet(&write_dsi_l);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					printk(KERN_ALERT "screen_display_write_dsi_long_packet err 1!\n");
					disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
					screen_display_delete(&disp_delete);
					kfree(line_data);
					return -1;
				}
				/* 2nd line */
				*line_data = 0x3C;
				for (line_num=0; line_num < 959; line_num++) {
					ret = screen_display_write_dsi_long_packet(&write_dsi_l);
					if (ret != SMAP_LIB_DISPLAY_OK) {
						printk(KERN_ALERT "screen_display_write_dsi_long_packet err 1!\n");
						disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
						screen_display_delete(&disp_delete);
						kfree(line_data);
						return -1;
					}
				}
				
				kfree(line_data);
				break;
			}
			case MIPI_DSI_DELAY:
				msleep(s6e39a0x02_cmdset[loop].size);
				break;
			case MIPI_DSI_END:
			default:
				loop = -2;
				break;
			}
			loop++;
		}
	}
	printk(KERN_ALERT "S6E39A0X02 qHD LCD Initialize End\n");
#endif
#if 0
	//SSG Display
	if (lcd_ext_param[lcd_num].o_mode == RT_DISPLAY_LCD1) {
	  
		unsigned char mtp[] 			= {0xf1,0xa5,0xa5};
		unsigned char level2[] 			= {0xf0,0xa5,0xa5};
		unsigned char panel_condition[] 	= {0xf8,0x01,0x27,0x27,0x07,0x07,0x54,0x9f,0x63,0x86,0x1a,0x33,0x0d,0x00,0x00};
		unsigned char display_condition_1[] 	= {0xf2,0x02,0x03,0x1c,0x10,0x10};
//		unsigned char display_condition_2[] 	= {0xf7,0x00,0x00,0x00};
		unsigned char display_condition_2[] 	= {0xf7,0x03,0x00,0x00};
		unsigned char gamma_condition_1[] 	= {0xfa,0x02,0x18,0x08,0x24,0x70,0x6e,0x4e,0xbc,0xc0,0xaf,0xb3,0xb8,0xa5,0xc5,0xc7,0xbb,0x00,0xb9,0x00,0xbb,0x00,0xfc};
// info from SSG - no different in outcome		unsigned char gamma_condition_1[] 	= {0xfa,0x02,0x31,0x00,0x4f,0x13,0x60,0x01,0xaf,0xcd,0xa6,0xac,0xc7,0x98,0xc4,0xcf,0xb6,0x00,0x74,0x00,0x6b,0x00,0x8a};

		unsigned char etc_condition_1[]		= {0xf6,0x00,0x8e,0x0f};
		unsigned char etc_condition_3[]		= {0xb5,0x2c,0x12,0x0c,0x0a,0x10,0x0e,0x17,0x13,0x1f,0x1a,0x2a,0x24,0x1f,0x1b,0x1a,0x17,0x2b,0x26,0x22,0x20,0x3a,0x34,0x30,0x2c,0x29,0x26,0x25,0x23,0x21,0x20,0x1e,0x1e};
		unsigned char etc_condition_4[]		= {0xb6,0x00,0x00,0x11,0x22,0x33,0x44,0x44,0x44,0x55,0x55,0x66,0x66,0x66,0x66,0x66,0x66};
		unsigned char etc_condition_5[]		= {0xb7,0x2c,0x12,0x0c,0x0a,0x10,0x0e,0x17,0x13,0x1f,0x1a,0x2a,0x24,0x1f,0x1b,0x1a,0x17,0x2b,0x26,0x22,0x20,0x3a,0x34,0x30,0x2c,0x29,0x26,0x25,0x23,0x21,0x20,0x1e,0x1e};
		unsigned char etc_condition_6[]		= {0xb8,0x00,0x00,0x11,0x22,0x33,0x44,0x44,0x44,0x55,0x55,0x66,0x66,0x66,0x66,0x66,0x66};
		unsigned char etc_condition_7[]		= {0xb9,0x2c,0x12,0x0c,0x0a,0x10,0x0e,0x17,0x13,0x1f,0x1a,0x2a,0x24,0x1f,0x1b,0x1a,0x17,0x2b,0x26,0x22,0x20,0x3a,0x34,0x30,0x2c,0x29,0x26,0x25,0x23,0x21,0x20,0x1e,0x1e};
		unsigned char etc_condition_8[]		= {0xba,0x00,0x00,0x11,0x22,0x33,0x44,0x44,0x44,0x55,0x55,0x66,0x66,0x66,0x66,0x66,0x66};
		unsigned char dynamic_elvss_1[]		= {0xb2,0x10,0x10,0x10,0x10};
			
		/* wait 5ms */
		msleep(5);
		
		//MTP key enable
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x3;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = mtp;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 1!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		//Level2 key enable
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x3;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = level2;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 2!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		/* wait 1ms */
		msleep(1);
	  
		//Sleep out command
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x05;
		write_dsi_s.reg_address = 0x11;
		write_dsi_s.write_data	= 0x00;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err 1!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}

		
		/* wait 10ms */
		msleep(10);
		
		//Panel Condition Set
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x0F;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = panel_condition;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 3!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		//Display Condition Set
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x06;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = display_condition_1;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 4!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x04;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = display_condition_2;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 4!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}


		//Gamma Condition Set
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x23;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = gamma_condition_1;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 5!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x15;
		write_dsi_s.reg_address = 0xFA;
		write_dsi_s.write_data	= 0x03;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err 2!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		//Etc Condition Set
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x04;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_1;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 6!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x15;
		write_dsi_s.reg_address = 0xB3;
		write_dsi_s.write_data	= 0x6C;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err 3!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}

		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x21;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_3;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 7!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x11;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_4;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 8!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x21;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_5;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 9!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x11;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_6;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 10!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x21;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_7;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 11!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x11;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = etc_condition_8;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 12!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		//Smart DYNAMIC ELVSS set
		write_dsi_l.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_l.output_mode = lcd_ext_param[lcd_num].o_mode;
		write_dsi_l.data_id = 0x39;
		//write_dsi_l.dummy = NULL;
		write_dsi_l.data_count = 0x05;
		//write_dsi_l.dummy2 = NULL;		//#MU2DSP188
		write_dsi_l.write_data = dynamic_elvss_1;
		ret = screen_display_write_dsi_long_packet(&write_dsi_l);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_write_dsi_long_packet err 13!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x15;
		write_dsi_s.reg_address = 0xB1;
		write_dsi_s.write_data	= 0x0B;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err 4!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}

		/* wait 120ms */
		msleep(120);
		
		//Display On command
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x05;
		write_dsi_s.reg_address = 0x29;
		write_dsi_s.write_data	= 0x00;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err 5!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
	
	}
		
#endif
#if 0 
	//KOTA display
	if (lcd_ext_param[lcd_num].o_mode == RT_DISPLAY_LCD1) {
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x05;
		write_dsi_s.reg_address = 0x11;
		write_dsi_s.write_data	= 0x00;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}

		/* wait 120ms */
		msleep(120);

		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x15;
		write_dsi_s.reg_address = 0x53;
		write_dsi_s.write_data	= 0x24;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}

		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x15;
		write_dsi_s.reg_address = 0x51;
		write_dsi_s.write_data	= 0xFF;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
		write_dsi_s.handle	= lcd_ext_param[lcd_num].aInfo;
		write_dsi_s.data_id	= 0x05;
		write_dsi_s.reg_address = 0x29;
		write_dsi_s.write_data	= 0x00;
		write_dsi_s.output_mode = lcd_ext_param[lcd_num].o_mode;
		ret = screen_display_write_dsi_short_packet(&write_dsi_s);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "disp_write_dsi_short err!\n");
			disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
			screen_display_delete(&disp_delete);
			return -1;
		}
	}
#endif
	disp_delete.handle = lcd_ext_param[lcd_num].aInfo;
	screen_display_delete(&disp_delete);

	return 0;

}

int sh_mobile_lcdc_keyclr_set(unsigned short s_key_clr,
			      unsigned short output_mode)
{

#if 0
	int i, ret;
	screen_disp_param disp_param;
	screen_disp_delete disp_delete;
	void *screen_handle;

	for (i = 0 ; i < CHAN_NUM ; i++) {
		if (output_mode == lcd_ext_param[i].o_mode)
			break;
	}
	if (i >= CHAN_NUM) {
		printk(KERN_ALERT "lcdc_key_clr_set param ERR\n");
		return -1;
	}
	if (down_interruptible(&lcd_ext_param[i].sem_lcd)) {
		printk(KERN_ALERT "down_interruptible failed\n");
		return -1;
	}

	lcd_ext_param[i].key_clr = s_key_clr;

	if (lcd_ext_param[i].aInfo != NULL) {
		screen_handle = screen_display_new();
		disp_param.handle = screen_handle;
		disp_param.output_mode = lcd_ext_param[i].o_mode;
		disp_param.key_color = lcd_ext_param[i].key_clr;
		disp_param.alpha = lcd_ext_param[i].alpha;
		ret = screen_display_set_parameters(&disp_param);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			up(&lcd_ext_param[i].sem_lcd);
			return -1;
		}
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
	}

	up(&lcd_ext_param[i].sem_lcd);

#endif
	return 0;

}
EXPORT_SYMBOL(sh_mobile_lcdc_keyclr_set);

int sh_mobile_lcdc_alpha_set(unsigned short s_alpha,
			      unsigned short output_mode)
{

#if 0
	int i, ret;
	screen_disp_param disp_param;
	screen_disp_delete disp_delete;
	void *screen_handle;

	for (i = 0 ; i < CHAN_NUM ; i++) {
		if (output_mode == lcd_ext_param[i].o_mode)
			break;
	}
	if (i >= CHAN_NUM) {
		printk(KERN_ALERT "lcdc_key_clr_set param ERR\n");
		return -1;
	}
	if (down_interruptible(&lcd_ext_param[i].sem_lcd)) {
		printk(KERN_ALERT "down_interruptible failed\n");
		return -1;
	}

	lcd_ext_param[i].alpha = s_alpha;

	if (lcd_ext_param[i].aInfo != NULL) {
		screen_handle = screen_display_new();
		disp_param.handle = screen_handle;
		disp_param.output_mode = lcd_ext_param[i].o_mode;
		disp_param.key_color = lcd_ext_param[i].key_clr;
		disp_param.alpha = lcd_ext_param[i].alpha;
		ret = screen_display_set_parameters(&disp_param);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			up(&lcd_ext_param[i].sem_lcd);
			return -1;
		}
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
	}

	up(&lcd_ext_param[i].sem_lcd);

#endif
	return 0;

}
EXPORT_SYMBOL(sh_mobile_lcdc_alpha_set);

#if FB_SH_MOBILE_REFRESH
static void refresh_work(struct work_struct *work)
{

	int ret;
	screen_disp_set_lcd_refresh disp_refresh;
	screen_disp_delete disp_delete;
	void *screen_handle;
	int loop_count = COUNT_MFIS_SUSPEND;

	struct sh_mobile_lcdc_ext_param *extp =
		container_of(work, struct sh_mobile_lcdc_ext_param,
			     ref_work.work);

	if (extp->aInfo != NULL) {
		screen_handle = screen_display_new();
		disp_refresh.handle = screen_handle;
		disp_refresh.output_mode = extp->o_mode;
		disp_refresh.refresh_mode = RT_DISPLAY_REFRESH_ON;
		ret = screen_display_set_lcd_refresh(&disp_refresh);
		if (ret != SMAP_LIB_DISPLAY_OK)
			printk(KERN_ALERT "display_set_lcd_refresh ON ERR%d\n"
			       , ret);

		DBG_PRINT("screen_display_set_lcd_refresh ON\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		ret = mfis_drv_suspend();
		extp->refresh_on = 1;

		while ((ret != 0 && extp->mfis_err_flag == 0) && loop_count) {
			DBG_PRINT("mfis_drv_suspend err :sleep 100mS\n");
			msleep(100);
			ret = mfis_drv_suspend();
			loop_count--;
		}
		if (!loop_count)
			printk(KERN_ALERT "##mfis_drv_suspend ERR%d\n", ret);
	}

	extp->delay_flag = 0;
	return;
}
#endif


int sh_mobile_lcdc_refresh(unsigned short set_state,
			   unsigned short output_mode)
{

#if FB_SH_MOBILE_REFRESH
	int i, ret;
	screen_disp_set_lcd_refresh disp_refresh;
	screen_disp_delete disp_delete;
	void *screen_handle;
	for (i = 0 ; i < CHAN_NUM ; i++) {
		if (output_mode == lcd_ext_param[i].o_mode)
			break;
	}
	if (i >= CHAN_NUM) {
		printk(KERN_ALERT "lcdc_key_clr_set param ERR\n");
		return -1;
	}
	if (down_interruptible(&lcd_ext_param[i].sem_lcd)) {
		printk(KERN_ALERT "down_interruptible failed\n");
		return -1;
	}

	lcd_ext_param[i].v4l2_state = set_state;
	if (lcd_ext_param[i].v4l2_state == RT_DISPLAY_REFRESH_OFF) {
		if (lcd_ext_param[i].delay_flag == 1) {
			lcd_ext_param[i].mfis_err_flag = 1;
			cancel_delayed_work_sync(&lcd_ext_param[i].ref_work);
			lcd_ext_param[i].delay_flag = 0;
		}
		if (lcd_ext_param[i].refresh_on == 1) {
			if (lcd_ext_param[i].aInfo != NULL) {
				ret = mfis_drv_resume();
				if (ret != 0) {
					printk(KERN_ALERT "##mfis_drv_resume"
					       " ERR%d\n", ret);
				}
				screen_handle = screen_display_new();
				disp_refresh.handle = screen_handle;
				disp_refresh.output_mode
					= lcd_ext_param[i].o_mode;
				disp_refresh.refresh_mode
					= RT_DISPLAY_REFRESH_OFF;
				ret = screen_display_set_lcd_refresh(
					&disp_refresh);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					disp_delete.handle = screen_handle;
					screen_display_delete(&disp_delete);
					up(&lcd_ext_param[i].sem_lcd);
					return -1;
				}
				lcd_ext_param[i].refresh_on = 0;
				disp_delete.handle = screen_handle;
				screen_display_delete(&disp_delete);
			}
		}
	} else {
		queue_delayed_work(
			sh_mobile_wq, &lcd_ext_param[i].ref_work, 0);
		lcd_ext_param[i].delay_flag = 1;
		lcd_ext_param[i].mfis_err_flag = 0;
	}
	up(&lcd_ext_param[i].sem_lcd);
#endif
	return 0;

}
EXPORT_SYMBOL(sh_mobile_lcdc_refresh);

#if FB_SH_MOBILE_HDMI
int sh_mobile_fb_hdmi_set(struct fb_hdmi_set_mode *set_mode)
{

	void *hdmi_handle;
	screen_disp_start_hdmi disp_start_hdmi;
	screen_disp_stop_hdmi disp_stop_hdmi;
	screen_disp_delete disp_delete;
	int hdmi_mode = RT_DISPLAY_1280_720P60;
	int ret;

	if (set_mode->start != SH_FB_HDMI_START &&
	    set_mode->start != SH_FB_HDMI_STOP) {
		DBG_PRINT("set_mode->start param\n");
		return -1;
	}
	if (set_mode->format < SH_FB_HDMI_480P60 ||
	    set_mode->format > SH_FB_HDMI_576P50A43) {
		DBG_PRINT("set_mode->format param\n");
		return -1;
	}

	hdmi_handle = screen_display_new();

	if (set_mode->start == SH_FB_HDMI_STOP) {
		if (down_interruptible(&lcd_ext_param[0].sem_lcd)) {
			printk(KERN_ALERT "down_interruptible failed\n");
			disp_delete.handle = hdmi_handle;
			screen_display_delete(&disp_delete);
			return -1;
		}
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
#if SH_MOBILE_COMPOSER_SUPPORT_HDMI

		sh_mobile_composer_hdmiset(0);
#endif
#endif

		disp_stop_hdmi.handle = hdmi_handle;
		ret = screen_display_stop_hdmi(&disp_stop_hdmi);
		DBG_PRINT("screen_display_stop_hdmi ret = %d\n", ret);
		up(&lcd_ext_param[0].sem_lcd);
		sh_mobile_lcdc_refresh(
			RT_DISPLAY_REFRESH_ON, RT_DISPLAY_LCD1);
	} else {
		sh_mobile_lcdc_refresh(
			RT_DISPLAY_REFRESH_OFF, RT_DISPLAY_LCD1);

		switch (set_mode->format) {
		case SH_FB_HDMI_480P60:
		{
			hdmi_mode = RT_DISPLAY_720_480P60;
			break;
		}
		case SH_FB_HDMI_720P60:
		{
			hdmi_mode = RT_DISPLAY_1280_720P60;
			break;
		}
		case SH_FB_HDMI_1080I60:
		{
			hdmi_mode = RT_DISPLAY_1920_1080I60;
			break;
		}
		case SH_FB_HDMI_1080P24:
		{
			hdmi_mode = RT_DISPLAY_1920_1080P24;
			break;
		}
		case SH_FB_HDMI_576P50:
		{
			hdmi_mode = RT_DISPLAY_720_576P50;
			break;
		}
		case SH_FB_HDMI_720P50:
		{
			hdmi_mode = RT_DISPLAY_1280_720P50;
			break;
		}
		case SH_FB_HDMI_1080P60:
		{
			hdmi_mode = RT_DISPLAY_1920_1080P60;
			break;
		}
		case SH_FB_HDMI_1080P50:
		{
			hdmi_mode = RT_DISPLAY_1920_1080P50;
			break;
		}
		case SH_FB_HDMI_480P60A43:
		{
			hdmi_mode = RT_DISPLAY_720_480P60A43;
			break;
		}
		case SH_FB_HDMI_576P50A43:
		{
			hdmi_mode = RT_DISPLAY_720_576P50A43;
			break;
		}
		}

		disp_start_hdmi.handle = hdmi_handle;
		disp_start_hdmi.format = hdmi_mode;
		disp_start_hdmi.background_color = LCD_BG_BLACK;
		DBG_PRINT("screen_display_start_hdmi handle = %x\n",
			  (unsigned int)disp_start_hdmi.handle);
		DBG_PRINT("screen_display_start_hdmi format = %d\n",
			  disp_start_hdmi.format);
		DBG_PRINT("screen_display_start_hdmi background_color = %x\n",
			  disp_start_hdmi.background_color);

		ret = screen_display_start_hdmi(&disp_start_hdmi);
		DBG_PRINT("screen_display_start_hdmi return = %d\n", ret);

		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_start_hdmi ERR%d\n"
			       , ret);
		}
	}

	disp_delete.handle = hdmi_handle;
	screen_display_delete(&disp_delete);

	return 0;
}
#endif

static int sh_mobile_fb_pan_display(struct fb_var_screeninfo *var,
				     struct fb_info *info)
{
	struct sh_mobile_lcdc_chan *ch = info->par;
	unsigned long new_pan_offset;

	int ret = 0;

/* onscreen buffer 2 */
	unsigned int i;
	unsigned short set_format;
	unsigned char  lcd_num;
	screen_disp_draw disp_draw;
	screen_disp_delete disp_delete;
	void *screen_handle;
#if FB_SH_MOBILE_REFRESH
	screen_disp_set_lcd_refresh disp_refresh;
#endif
	new_pan_offset = (var->yoffset * info->fix.line_length) +
		(var->xoffset * (info->var.bits_per_pixel / 8));

	for (i = 0 ; i < CHAN_NUM ; i++) {
		if (ch->cfg.chan == lcd_ext_param[i].lcd_type)
			break;
	}
	lcd_num = i;
	if (lcd_num >= CHAN_NUM)
		return -EINVAL;

	if (down_interruptible(&lcd_ext_param[lcd_num].sem_lcd)) {
		printk(KERN_ALERT "down_interruptible failed\n");
		return -ERESTARTSYS;
	}

	/* Set the source address for the next refresh */

	if (lcd_ext_param[lcd_num].aInfo == NULL) {
		ret = display_initialize(lcd_num);
		if (ret == -1) {
			up(&lcd_ext_param[lcd_num].sem_lcd);
			return -EIO;
		} else if (ret == -2)
			printk(KERN_ALERT "nothing MFI driver\n");
	}
	if (lcd_ext_param[lcd_num].aInfo != NULL) {
		screen_handle = screen_display_new();
#if FB_SH_MOBILE_REFRESH
		if (lcd_ext_param[lcd_num].v4l2_state
		    == RT_DISPLAY_REFRESH_ON) {
			if (lcd_ext_param[lcd_num].delay_flag == 1) {
				lcd_ext_param[lcd_num].mfis_err_flag = 1;
				cancel_delayed_work_sync(
					&lcd_ext_param[lcd_num].ref_work);
				lcd_ext_param[lcd_num].delay_flag = 0;
			}
			if (lcd_ext_param[lcd_num].refresh_on == 1) {
				ret = mfis_drv_resume();
				if (ret != 0) {
					printk(KERN_ALERT
					       "##mfis_drv_resume "
					       "ERR%d\n", ret);
				}
				disp_refresh.handle = screen_handle;
				disp_refresh.output_mode
					= lcd_ext_param[lcd_num].o_mode;
				disp_refresh.refresh_mode
					= RT_DISPLAY_REFRESH_OFF;
				ret = screen_display_set_lcd_refresh(
					&disp_refresh);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					up(&lcd_ext_param[lcd_num].sem_lcd);
					disp_delete.handle = screen_handle;
					screen_display_delete(&disp_delete);
					return -EIO;
				}
				lcd_ext_param[lcd_num].refresh_on = 0;
			}
		}
#endif
		if (var->bits_per_pixel == 16)
			set_format = RT_DISPLAY_FORMAT_RGB565;
		else
			set_format = RT_DISPLAY_FORMAT_ARGB8888;

#ifdef CONFIG_FB_SH_MOBILE_DOUBLE_BUF
		disp_draw.handle = screen_handle;
		disp_draw.output_mode = lcd_ext_param[lcd_num].draw_mode;
		disp_draw.draw_rect.x = lcd_ext_param[lcd_num].rect_x;
		disp_draw.draw_rect.y = lcd_ext_param[lcd_num].rect_y;
		disp_draw.draw_rect.width =
			lcd_ext_param[lcd_num].rect_width;
		disp_draw.draw_rect.height =
			lcd_ext_param[lcd_num].rect_height;
		disp_draw.format = set_format;
		disp_draw.buffer_offset = new_pan_offset;
		disp_draw.rotate = lcd_ext_param[lcd_num].rotate;
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		sh_mobile_composer_blendoverlay(disp_draw.buffer_offset
						+ info->fix.smem_start);
#endif
		DBG_PRINT("screen_display_draw handle %x\n",
			  (unsigned int)disp_draw.handle);
		DBG_PRINT("screen_display_draw output_mode %d\n",
			  disp_draw.output_mode);
		DBG_PRINT("screen_display_draw draw_rect.x %d\n",
			  disp_draw.draw_rect.x);
		DBG_PRINT("screen_display_draw draw_rect.y %d\n",
			  disp_draw.draw_rect.y);
		DBG_PRINT("screen_display_draw draw_rect.width %d\n",
			  disp_draw.draw_rect.width);
		DBG_PRINT("screen_display_draw draw_rect.height %d\n",
			  disp_draw.draw_rect.height);
		DBG_PRINT("screen_display_draw format %d\n", disp_draw.format);
		DBG_PRINT("screen_display_draw buffer_offset %d\n",
			  disp_draw.buffer_offset);
		DBG_PRINT("screen_display_draw rotate %d\n", disp_draw.rotate);
		ret = screen_display_draw(&disp_draw);
		DBG_PRINT("screen_display_draw return %d\n", ret);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			up(&lcd_ext_param[lcd_num].sem_lcd);
			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			return -EIO;
		}
#else

		memcpy((void *)lcd_ext_param[lcd_num].vir_addr,
		       (void *)(info->screen_base + new_pan_offset),
		       (lcd_ext_param[lcd_num].rect_width *
			lcd_ext_param[lcd_num].rect_height *
			var->bits_per_pixel / 8));

		disp_draw.handle = screen_handle;
		disp_draw.output_mode = lcd_ext_param[lcd_num].draw_mode;
		disp_draw.draw_rect.x = lcd_ext_param[lcd_num].rect_x;
		disp_draw.draw_rect.y = lcd_ext_param[lcd_num].rect_y;
		disp_draw.draw_rect.width =
			lcd_ext_param[lcd_num].rect_width;
		disp_draw.draw_rect.height =
			lcd_ext_param[lcd_num].rect_height;
		disp_draw.format = set_format;
		disp_draw.buffer_offset = 0;
		disp_draw.rotate = lcd_ext_param[lcd_num].rotate;
#ifdef CONFIG_MISC_R_MOBILE_COMPOSER_REQUEST_QUEUE
		sh_mobile_composer_blendoverlay(info->fix.smem_start);
#endif
		ret = screen_display_draw(&disp_draw);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			up(&lcd_ext_param[lcd_num].sem_lcd);
			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			return -EIO;
		}
#endif

#if FB_SH_MOBILE_REFRESH
		if (lcd_ext_param[lcd_num].v4l2_state
		    == RT_DISPLAY_REFRESH_ON) {
			queue_delayed_work(
				sh_mobile_wq, &lcd_ext_param[lcd_num].ref_work,
				msecs_to_jiffies(REFRESH_TIME_MSEC));
			lcd_ext_param[lcd_num].delay_flag = 1;
			lcd_ext_param[lcd_num].mfis_err_flag = 0;
		}
#endif
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
	}

	ch->pan_offset = new_pan_offset;

	up(&lcd_ext_param[lcd_num].sem_lcd);

	return 0;
}

static int sh_mobile_ioctl(struct fb_info *info, unsigned int cmd,
		       unsigned long arg)
{
	int retval;
#if FB_SH_MOBILE_HDMI
	struct fb_hdmi_set_mode hdmi_set;
#endif
	switch (cmd) {
	case FBIO_WAITFORVSYNC:
		retval = 0;
		break;

#if FB_SH_MOBILE_HDMI
	case SH_MOBILE_FB_HDMI_SET:
		if (arg == 0) {
			retval = -EINVAL;
			break;
		}
		if (copy_from_user(&hdmi_set, (void *)arg,
				   sizeof(struct fb_hdmi_set_mode))) {
			printk(KERN_ALERT "copy_from_user failed\n");
			retval = -EFAULT;
			break;
		}
		if (sh_mobile_fb_hdmi_set(&hdmi_set)) {
			retval = -EINVAL;
			break;
		} else {
			retval = 0;
			break;
		}
#endif
	default:
		retval = -ENOIOCTLCMD;
		break;
	}
	return retval;
}

static int sh_mobile_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	unsigned long start;
	unsigned long off;
	u32 len;

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT))
		return -EINVAL;

	off = vma->vm_pgoff << PAGE_SHIFT;
	start = info->fix.smem_start;
	len = PAGE_ALIGN((start & ~PAGE_MASK) + info->fix.smem_len);

	if ((vma->vm_end - vma->vm_start + off) > len)
		return -EINVAL;

	off += start;
	vma->vm_pgoff = off >> PAGE_SHIFT;

	/* Accessing memory will be done non-cached. */
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	/* To stop the swapper from even considering these pages */
	vma->vm_flags |= (VM_IO | VM_RESERVED);

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}
static int sh_mobile_fb_check_var(struct fb_var_screeninfo *var,
				  struct fb_info *info)
{
	switch (var->bits_per_pixel) {
	case 16: /* RGB 565 */
		var->red.offset    = 11;
		var->red.length    = 5;
		var->green.offset  = 5;
		var->green.length  = 6;
		var->blue.offset   = 0;
		var->blue.length   = 5;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32: /* ARGB 8888*/
		var->red.offset    = 16;
		var->red.length    = 8;
		var->green.offset  = 8;
		var->green.length  = 8;
		var->blue.offset   = 0;
		var->blue.length   = 8;
		var->transp.offset = 24;
		var->transp.length = 8;
		break;
	default:
		return -EINVAL;

	}
	return 0;

}

static struct fb_ops sh_mobile_lcdc_ops = {
	.owner          = THIS_MODULE,
	.fb_setcolreg	= sh_mobile_lcdc_setcolreg,
	.fb_check_var	= sh_mobile_fb_check_var,
	.fb_read        = fb_sys_read,
	.fb_write       = fb_sys_write,
	.fb_fillrect	= sys_fillrect,
	.fb_copyarea	= sys_copyarea,
	.fb_imageblit	= sys_imageblit,
	.fb_pan_display = sh_mobile_fb_pan_display,
	.fb_ioctl       = sh_mobile_ioctl,
	.fb_mmap	= sh_mobile_mmap,
};

static int sh_mobile_lcdc_set_bpp(struct fb_var_screeninfo *var, int bpp)
{
	switch (bpp) {
	case 16: /* RGB 565 */
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.offset = 0;
		var->blue.length = 5;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;

	case 32: /* ARGB 8888 */
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 24;
		var->transp.length = 8;
		break;
	default:
		return -EINVAL;
	}
	var->bits_per_pixel = bpp;
	var->red.msb_right = 0;
	var->green.msb_right = 0;
	var->blue.msb_right = 0;
	var->transp.msb_right = 0;
	return 0;
}


static int sh_mobile_lcdc_suspend(struct device *dev)
{
	int lcd_num;
	/*int format, bg_color;*/
	/*screen_disp_stop_hdmi disp_stop_hdmi;*/
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_draw disp_draw;
	screen_disp_delete disp_delete;
	void *suspend_handle;
#if FB_SH_MOBILE_REFRESH
	screen_disp_set_lcd_refresh disp_refresh;
#endif

	suspend_handle = screen_display_new();

	down(&sh_mobile_sem_hdmi);

	for (lcd_num = 0; lcd_num < CHAN_NUM; lcd_num++) {
		if (lcd_ext_param[lcd_num].aInfo != NULL) {
			down(&lcd_ext_param[lcd_num].sem_lcd);
#if FB_SH_MOBILE_REFRESH
			if (lcd_ext_param[lcd_num].v4l2_state
			    == RT_DISPLAY_REFRESH_ON) {
				if (lcd_ext_param[lcd_num].delay_flag == 1) {
					lcd_ext_param[lcd_num].mfis_err_flag
						= 1;
					cancel_delayed_work_sync(
						&lcd_ext_param[lcd_num].
						ref_work);
					lcd_ext_param[lcd_num].delay_flag = 0;
				}
				if (lcd_ext_param[lcd_num].refresh_on == 1) {
					mfis_drv_resume();
					disp_refresh.handle = suspend_handle;
					disp_refresh.output_mode
						= lcd_ext_param[lcd_num].
						o_mode;
					disp_refresh.refresh_mode
						= RT_DISPLAY_REFRESH_OFF;
					screen_display_set_lcd_refresh(
						&disp_refresh);
					lcd_ext_param[lcd_num].refresh_on = 0;
				}
			}
#endif
			disp_draw.handle = suspend_handle;
			disp_draw.output_mode = lcd_ext_param[lcd_num].o_mode;
			disp_draw.draw_rect.x = lcd_ext_param[lcd_num].rect_x;
			disp_draw.draw_rect.y = lcd_ext_param[lcd_num].rect_y;
			disp_draw.draw_rect.width =
				lcd_ext_param[lcd_num].rect_width;
			disp_draw.draw_rect.height =
				lcd_ext_param[lcd_num].rect_height;
			if (lcd_ext_param[lcd_num].draw_bpp == 16) {
				disp_draw.format = RT_DISPLAY_FORMAT_RGB565;
				memset((void *)lcd_ext_param[lcd_num].vir_addr
				       , 0, lcd_ext_param[lcd_num].mem_size);
			} else {
				int i;
				unsigned int *cpy_address;
				disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
				cpy_address =
					(unsigned int *)
					lcd_ext_param[lcd_num].vir_addr;
				for (i = 0;
				    i < lcd_ext_param[lcd_num].mem_size / 4;
				    i++) {
					*cpy_address++ = 0xFF000000;
				}
			}
			disp_draw.buffer_offset = 0;
			disp_draw.rotate = lcd_ext_param[lcd_num].rotate;
			screen_display_draw(&disp_draw);
			disp_stop_lcd.handle = suspend_handle;
			disp_stop_lcd.output_mode =
				lcd_ext_param[lcd_num].o_mode;
			screen_display_stop_lcd(&disp_stop_lcd);

			up(&lcd_ext_param[lcd_num].sem_lcd);
		}
	}
#if 0
	if (sh_mobile_v4l2_display_getstateHDMI(&format, &bg_color) == 0) {
		disp_stop_hdmi.handle = suspend_handle;
		screen_display_stop_hdmi(&disp_stop_hdmi);
	}
#endif
	up(&sh_mobile_sem_hdmi);

	disp_delete.handle = suspend_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int sh_mobile_lcdc_resume(struct device *dev)
{
	unsigned int lcd_num;
	/*unsigned int format, bg_color;*/
	/*screen_disp_start_hdmi disp_start_hdmi;*/
	screen_disp_start_lcd disp_start_lcd;
	screen_disp_delete disp_delete;
	void *suspend_handle;

	suspend_handle = screen_display_new();

	down(&sh_mobile_sem_hdmi);

	for (lcd_num = 0; lcd_num < CHAN_NUM; lcd_num++) {
		if (lcd_ext_param[lcd_num].aInfo != NULL) {
			disp_start_lcd.handle = suspend_handle;
			disp_start_lcd.output_mode =
				lcd_ext_param[lcd_num].o_mode;
			screen_display_start_lcd(&disp_start_lcd);
		}
	}
#if 0
	if (sh_mobile_v4l2_display_getstateHDMI(&format, &bg_color) == 0) {
		disp_start_hdmi.handle = suspend_handle;
		disp_start_hdmi.format = format;
		disp_start_hdmi.background_color = bg_color;
		screen_display_start_hdmi(&disp_start_hdmi);
	}
#endif
	up(&sh_mobile_sem_hdmi);

	disp_delete.handle = suspend_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

#if 0 /* this function is implemented in future. */
static int sh_mobile_lcdc_runtime_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_mobile_lcdc_priv *p = platform_get_drvdata(pdev);
	struct sh_mobile_lcdc_chan *ch;
	int k, n;

	/* save per-channel registers */
	for (k = 0; k < ARRAY_SIZE(p->ch); k++) {
		ch = &p->ch[k];
		if (!ch->enabled)
			continue;
		for (n = 0; n < NR_CH_REGS; n++)
			ch->saved_ch_regs[n] = lcdc_read_chan(ch, n);
	}

	/* save shared registers */
	for (n = 0; n < NR_SHARED_REGS; n++)
		p->saved_shared_regs[n] = lcdc_read(p, lcdc_shared_regs[n]);

	/* turn off LCDC hardware */
	lcdc_write(p, _LDCNT1R, 0);
	return 0;
}

static int sh_mobile_lcdc_runtime_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_mobile_lcdc_priv *p = platform_get_drvdata(pdev);
	struct sh_mobile_lcdc_chan *ch;
	int k, n;

	/* restore per-channel registers */
	for (k = 0; k < ARRAY_SIZE(p->ch); k++) {
		ch = &p->ch[k];
		if (!ch->enabled)
			continue;
		for (n = 0; n < NR_CH_REGS; n++)
			lcdc_write_chan(ch, n, ch->saved_ch_regs[n]);
	}

	/* restore shared registers */
	for (n = 0; n < NR_SHARED_REGS; n++)
		lcdc_write(p, lcdc_shared_regs[n], p->saved_shared_regs[n]);

	return 0;
}

static const struct dev_pm_ops sh_mobile_lcdc_dev_pm_ops = {
	.suspend = sh_mobile_lcdc_suspend,
	.resume = sh_mobile_lcdc_resume,
	.runtime_suspend = sh_mobile_lcdc_runtime_suspend,
	.runtime_resume = sh_mobile_lcdc_runtime_resume,
};
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
static void sh_mobile_fb_early_suspend(struct early_suspend *h)
{
	struct sh_mobile_lcdc_priv *priv;

	priv = container_of(h, struct sh_mobile_lcdc_priv, early_suspend);

	sh_mobile_lcdc_suspend(priv->dev);

}

static void sh_mobile_fb_late_resume(struct early_suspend *h)
{
	struct sh_mobile_lcdc_priv *priv;

	priv = container_of(h, struct sh_mobile_lcdc_priv, early_suspend);

	sh_mobile_lcdc_resume(priv->dev);

}
#endif /* CONFIG_HAS_EARLYSUSPEND */

static int sh_mobile_lcdc_remove(struct platform_device *pdev);

static unsigned long RoundUpToMultiple(unsigned long x, unsigned long y)
{
	unsigned long div = x / y;
	unsigned long rem = x % y;

	return (div + ((rem == 0) ? 0 : 1)) * y;
}

static unsigned long GCD(unsigned long x, unsigned long y)
{
	while (y != 0) {
		unsigned long r = x % y;
		x = y;
		y = r;
	}
	return x;
}

static unsigned long LCM(unsigned long x, unsigned long y)
{
	unsigned long gcd = GCD(x, y);

	return (gcd == 0) ? 0 : ((x / gcd) * y);
}

static int __devinit sh_mobile_lcdc_probe(struct platform_device *pdev)
{
	struct fb_info *info;
	struct sh_mobile_lcdc_priv *priv;
	struct sh_mobile_lcdc_info *pdata;
	struct sh_mobile_lcdc_chan_cfg *cfg;
	struct resource *res;
	int error = 0;
	int i, j;
	unsigned long ulLCM;
	void *temp = NULL;

#ifndef CONFIG_FB_SH_MOBILE_DOUBLE_BUF
	void *buf = NULL;
#endif

	printk(KERN_ALERT "sh_mobile_lcdc_probe\n");

	if (!pdev->dev.platform_data) {
		dev_err(&pdev->dev, "no platform data defined\n");
		error = -EINVAL;
		goto err0;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "cannot allocate device data\n");
		error = -ENOMEM;
		goto err0;
	}

	priv->dev = &pdev->dev;
	platform_set_drvdata(pdev, priv);
	pdata = pdev->dev.platform_data;

#if FB_SH_MOBILE_REFRESH
	sh_mobile_wq = create_singlethread_workqueue("sh_mobile_lcd");
#endif

	j = 0;
	for (i = 0; i < ARRAY_SIZE(pdata->ch); i++) {
		priv->ch[j].lcdc = priv;
		memcpy(&priv->ch[j].cfg, &pdata->ch[i], sizeof(pdata->ch[i]));

		priv->ch[j].pan_offset = 0;
		switch (pdata->ch[i].chan) {
		case LCDC_CHAN_MAINLCD:
			lcd_ext_param[i].o_mode = RT_DISPLAY_LCD1;
#if FB_SH_MOBILE_HDMI
			lcd_ext_param[i].draw_mode = RT_DISPLAY_LCDHDMI;
#else
			lcd_ext_param[i].draw_mode = RT_DISPLAY_LCD1;
#endif
			lcd_ext_param[i].rect_x = SH_MLCD_RECTX;
			lcd_ext_param[i].rect_y = SH_MLCD_RECTY;
			lcd_ext_param[i].rect_width = SH_MLCD_WIDTH;
			lcd_ext_param[i].rect_height = SH_MLCD_HEIGHT;
			lcd_ext_param[i].alpha = 0xFF;
			lcd_ext_param[i].key_clr = SH_MLCD_TRCOLOR;
			lcd_ext_param[i].v4l2_state = RT_DISPLAY_REFRESH_ON;
			lcd_ext_param[i].delay_flag = 0;
			lcd_ext_param[i].refresh_on = 0;
			lcd_ext_param[i].phy_addr = SCREEN_DISPLAY_BUFF_ADDR;
			if (SH_MLCD_HEIGHT > SH_MLCD_WIDTH) {
				lcd_ext_param[i].rotate
					= RT_DISPLAY_ROTATE_270;
			} else {
				lcd_ext_param[i].rotate
					= RT_DISPLAY_ROTATE_0;
			}
#if FB_SH_MOBILE_REFRESH
			INIT_DELAYED_WORK(&lcd_ext_param[i].ref_work,
					  refresh_work);
#endif
			j++;
			break;
		case LCDC_CHAN_SUBLCD:
			lcd_ext_param[i].o_mode = RT_DISPLAY_LCD2;
			lcd_ext_param[i].draw_mode = RT_DISPLAY_LCD2;
			lcd_ext_param[i].rect_x = SH_SLCD_RECTX;
			lcd_ext_param[i].rect_y = SH_SLCD_RECTY;
			lcd_ext_param[i].rect_width = SH_SLCD_WIDTH;
			lcd_ext_param[i].rect_height = SH_SLCD_HEIGHT;
			lcd_ext_param[i].alpha = 0xFF;
			lcd_ext_param[i].key_clr = SH_SLCD_TRCOLOR;
			lcd_ext_param[i].v4l2_state = RT_DISPLAY_REFRESH_ON;
			lcd_ext_param[i].delay_flag = 0;
			lcd_ext_param[i].refresh_on = 0;
			/* SUBLCD undefined */
			lcd_ext_param[i].phy_addr = 0;
			lcd_ext_param[i].rotate = RT_DISPLAY_ROTATE_0;
#if FB_SH_MOBILE_REFRESH
			INIT_DELAYED_WORK(&lcd_ext_param[i].ref_work,
					  refresh_work);
#endif
			j++;
			break;
		}
	}

	if (!j) {
		dev_err(&pdev->dev, "no channels defined\n");
		error = -EINVAL;
		goto err1;
	}

	for (i = 0; i < j; i++) {
		cfg = &priv->ch[i].cfg;

		priv->ch[i].info = framebuffer_alloc(0, &pdev->dev);
		if (!priv->ch[i].info) {
			dev_err(&pdev->dev, "unable to allocate fb_info\n");
			error = -ENOMEM;
			break;
		}

		info = priv->ch[i].info;
		info->fbops = &sh_mobile_lcdc_ops;
		info->var.xres = info->var.xres_virtual = cfg->lcd_cfg->xres;
		info->var.yres = cfg->lcd_cfg->yres;
		/* Default Y virtual resolution is 2x panel size */
		info->var.width = cfg->lcd_size_cfg.width;
		info->var.height = cfg->lcd_size_cfg.height;
		info->var.activate = FB_ACTIVATE_NOW;
		error = sh_mobile_lcdc_set_bpp(&info->var, cfg->bpp);
		if (error)
			break;
		info->fix = sh_mobile_lcdc_fix;
		info->fix.line_length = cfg->lcd_cfg->xres * (cfg->bpp / 8);

		/* 4kbyte align */
		ulLCM = LCM(info->fix.line_length, 0x1000);
		info->fix.smem_len = RoundUpToMultiple(
			info->fix.line_length*info->var.yres, ulLCM);
		info->fix.smem_len *= 2;

		info->var.yres_virtual = info->fix.smem_len
			/ info->fix.line_length;

		lcd_ext_param[i].mem_size = info->fix.smem_len;

#ifdef CONFIG_FB_SH_MOBILE_DOUBLE_BUF
/* onscreen buffer 2 */
		temp = ioremap(lcd_ext_param[i].phy_addr,
			       info->fix.smem_len);
		if (NULL == temp) {
			error = -ENOMEM;
			break;
		} else {
			lcd_ext_param[i].vir_addr = (unsigned int)temp;
		}
#else
		buf = dma_alloc_coherent(&pdev->dev, info->fix.smem_len,
					 &priv->ch[i].dma_handle, GFP_KERNEL);
		if (!buf) {
			dev_err(&pdev->dev, "unable to allocate buffer\n");
			error = -ENOMEM;
			break;
		}
		temp = ioremap(lcd_ext_param[i].phy_addr,
			       info->fix.smem_len);
		if (NULL == temp) {
			error = -ENOMEM;
			break;
		} else {
			lcd_ext_param[i].vir_addr = (unsigned int)temp;
		}
#endif
		info->pseudo_palette = &priv->ch[i].pseudo_palette;
		info->flags = FBINFO_FLAG_DEFAULT;

		error = fb_alloc_cmap(&info->cmap, PALETTE_NR, 0);
		if (error < 0) {
			dev_err(&pdev->dev, "unable to allocate cmap\n");
			break;
		}
		fb_set_cmap(&info->cmap, info);

#ifdef CONFIG_FB_SH_MOBILE_DOUBLE_BUF
/* onscreen buffer 2 */
		info->fix.smem_start = lcd_ext_param[i].phy_addr;
		info->screen_base = (char __iomem *)lcd_ext_param[i].vir_addr;
#else
		memset(buf, 0, info->fix.smem_len);
		info->fix.smem_start = priv->ch[i].dma_handle;
		info->screen_base = buf;
#endif
		info->device = &pdev->dev;
		info->par = &priv->ch[i];
	}

	if (error)
		goto err1;

	for (i = 0; i < j; i++) {
		struct sh_mobile_lcdc_chan *ch = priv->ch + i;

		info = ch->info;

		error = register_framebuffer(info);
		if (error < 0)
			goto err1;

		dev_info(info->dev,
			 "registered %s/%s as %dx%d %dbpp.\n",
			 pdev->name,
			 (ch->cfg.chan == LCDC_CHAN_MAINLCD) ?
			 "mainlcd" : "sublcd",
			 (int) ch->cfg.lcd_cfg->xres,
			 (int) ch->cfg.lcd_cfg->yres,
			 ch->cfg.bpp);

		lcd_ext_param[i].aInfo = NULL;
		lcd_ext_param[i].lcd_type = ch->cfg.chan;
		lcd_ext_param[i].draw_bpp = ch->cfg.bpp;
		sema_init(&lcd_ext_param[i].sem_lcd, 1);
	}

	sema_init(&sh_mobile_sem_hdmi, 1);

	fb_debug = 0;

#ifdef CONFIG_HAS_EARLYSUSPEND
	priv->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	priv->early_suspend.suspend = sh_mobile_fb_early_suspend;
	priv->early_suspend.resume = sh_mobile_fb_late_resume;
	register_early_suspend(&priv->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	return 0;
err1:
	sh_mobile_lcdc_remove(pdev);
err0:
	return error;
}

static int sh_mobile_lcdc_remove(struct platform_device *pdev)
{
	struct sh_mobile_lcdc_priv *priv = platform_get_drvdata(pdev);
	struct fb_info *info;
	int i;


#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&priv->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

#if FB_SH_MOBILE_REFRESH
	destroy_workqueue(sh_mobile_wq);
#endif

	for (i = 0; i < ARRAY_SIZE(priv->ch); i++)
		if (priv->ch[i].info->dev)
			unregister_framebuffer(priv->ch[i].info);

	for (i = 0; i < ARRAY_SIZE(priv->ch); i++) {
		info = priv->ch[i].info;

		if (!info || !info->device)
			continue;

#ifdef CONFIG_FB_SH_MOBILE_DOUBLE_BUF
		if (lcd_ext_param[i].vir_addr != 0)
			iounmap((void __iomem *)lcd_ext_param[i].vir_addr);
#else
		if (info->screen_base != NULL)
			dma_free_coherent(&pdev->dev, info->fix.smem_len,
					  info->screen_base,
					  priv->ch[i].dma_handle);

		if (lcd_ext_param[i].vir_addr != 0)
			iounmap((void __iomem *)lcd_ext_param[i].vir_addr);

#endif
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);
	}

	kfree(priv);
	return 0;
}

static struct platform_driver sh_mobile_lcdc_driver = {
	.driver		= {
		.name		= "sh_mobile_lcdc_fb",
		.owner		= THIS_MODULE,
#if 0
		.pm		= &sh_mobile_lcdc_dev_pm_ops,
#endif
	},
	.probe		= sh_mobile_lcdc_probe,
	.remove		= sh_mobile_lcdc_remove,
};

static int __init sh_mobile_lcdc_init(void)
{
	return platform_driver_register(&sh_mobile_lcdc_driver);
}

static void __exit sh_mobile_lcdc_exit(void)
{
	platform_driver_unregister(&sh_mobile_lcdc_driver);
}

module_init(sh_mobile_lcdc_init);
module_exit(sh_mobile_lcdc_exit);

MODULE_DESCRIPTION("SuperH Mobile LCDC Framebuffer driver");
MODULE_AUTHOR("Renesas Electronics");
MODULE_LICENSE("GPL v2");
