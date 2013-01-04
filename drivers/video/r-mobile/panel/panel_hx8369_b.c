/*
 * drivers/video/r-mobile/panel/panel_hx8369_b.c
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

#include <video/sh_mobile_lcdc.h>

#include <rtapi/screen_display.h>


#include <linux/platform_device.h>
#include <linux/fb.h>

#include <linux/regulator/consumer.h>

#include <mach/common.h>

#include "panel_hx8369_b.h"

#define HX8369_B_POWAREA_MNG_ENABLE

#ifdef HX8369_B_POWAREA_MNG_ENABLE
#include <rtapi/system_pwmng.h>
#endif

/* framebuffer address and size */
#define R_MOBILE_M_BUFF_ADDR		0x48C00000
#define R_MOBILE_M_BUFF_SIZE		(4 * 1024 * 1024)

/* panel pixel */
#define R_MOBILE_M_PANEL_PIXEL_WIDTH	 480
#define R_MOBILE_M_PANEL_PIXEL_HEIGHT	 800

/* panel size (mm) */
#define R_MOBILE_M_PANEL_SIZE_WIDTH	56
#define R_MOBILE_M_PANEL_SIZE_HEIGHT	94


/* pixclock = 1000000 / DCF */
#define R_MOBILE_M_PANEL_PIXCLOCK	 24359

/* timing */
#define R_MOBILE_M_PANEL_LEFT_MARGIN	 49
#define R_MOBILE_M_PANEL_RIGHT_MARGIN	 305
#define R_MOBILE_M_PANEL_HSYNC_LEN	 17
#define R_MOBILE_M_PANEL_UPPER_MARGIN	 12
#define R_MOBILE_M_PANEL_LOWER_MARGIN	 8
#define R_MOBILE_M_PANEL_VSYNC_LEN	 4
#define R_MOBILE_M_PANEL_H_TOTAL	 834
#define R_MOBILE_M_PANEL_V_TOTAL	 820

#define LCD_DSITCKCR		0x00000007
#define LCD_DSI0PCKCR		0x00000025
#define LCD_DSI0PHYCR		0x2A800014
#define LCD_SYSCONF		0x00000703
#define LCD_TIMSET0		0x4C2B3332
#define LCD_TIMSET1		0x00050091
#define LCD_DSICTRL		0x00000001
#define LCD_VMCTR1		0x0001003E
#define LCD_VMCTR2		0x00020710
#define LCD_VMLEN1		0x05A00000
#define LCD_VMLEN2		0x005E0000
#define LCD_VMLEN3		0x00000000
#define LCD_VMLEN4		0x00000000
#define LCD_DTCTR		0x00000007
#define LCD_MLDHCNR		0x003C006E
#define LCD_MLDHSYNR		0x00020062
#define LCD_MLDHAJR		0x00030101
#define LCD_MLDVLNR		0x0320033E
#define LCD_MLDVSYNR		0x00040329
#define LCD_MLDMT1R		0x0400000B
#define LCD_LDDCKR		0x00010040
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
#define LCD_MASK_MLDHCNR	0x07FF07FF
#define LCD_MASK_MLDHSYNR	0x001F07FF
#define LCD_MASK_MLDHAJR	0x07070707
#define LCD_MASK_MLDVLNR	0x1FFF1FFF
#define LCD_MASK_MLDVSYNR	0x001F1FFF
#define LCD_MASK_MLDMT1R	0x1F03FCCF
#define LCD_MASK_LDDCKR		0x0007007F
#define LCD_MASK_MLDDCKPAT1R	0x0FFFFFFF
#define LCD_MASK_MLDDCKPAT2R	0xFFFFFFFF

#define LCD_DSI0PCKCR_50HZ	0x00000031
#define LCD_DSI0PHYCR_50HZ	0x2A800016

static struct fb_panel_info r_mobile_info = {
	.pixel_width	= R_MOBILE_M_PANEL_PIXEL_WIDTH,
	.pixel_height	= R_MOBILE_M_PANEL_PIXEL_HEIGHT,
	.size_width	= R_MOBILE_M_PANEL_SIZE_WIDTH,
	.size_height	= R_MOBILE_M_PANEL_SIZE_HEIGHT,
	.buff_address	= R_MOBILE_M_BUFF_ADDR,
	.pixclock      = R_MOBILE_M_PANEL_PIXCLOCK,
	.left_margin   = R_MOBILE_M_PANEL_LEFT_MARGIN,
	.right_margin  = R_MOBILE_M_PANEL_RIGHT_MARGIN,
	.upper_margin  = R_MOBILE_M_PANEL_UPPER_MARGIN,
	.lower_margin  = R_MOBILE_M_PANEL_LOWER_MARGIN,
	.hsync_len     = R_MOBILE_M_PANEL_HSYNC_LEN,
	.vsync_len     = R_MOBILE_M_PANEL_VSYNC_LEN,
};

static screen_disp_lcd_if r_mobile_lcd_if_param = {
	.DSITCKCR    = LCD_DSITCKCR,
	.DSI0PCKCR   = LCD_DSI0PCKCR,
	.DSI0PHYCR   = LCD_DSI0PHYCR,
	.SYSCONF     = LCD_SYSCONF,
	.TIMSET0     = LCD_TIMSET0,
	.TIMSET1     = LCD_TIMSET1,
	.DSICTRL     = LCD_DSICTRL,
	.VMCTR1      = LCD_VMCTR1,
	.VMCTR2      = LCD_VMCTR2,
	.VMLEN1      = LCD_VMLEN1,
	.VMLEN2      = LCD_VMLEN2,
	.VMLEN3      = LCD_VMLEN3,
	.VMLEN4      = LCD_VMLEN4,
	.DTCTR       = LCD_DTCTR,
	.MLDHCNR     = LCD_MLDHCNR,
	.MLDHSYNR    = LCD_MLDHSYNR,
	.MLDHAJR     = LCD_MLDHAJR,
	.MLDVLNR     = LCD_MLDVLNR,
	.MLDVSYNR    = LCD_MLDVSYNR,
	.MLDMT1R     = LCD_MLDMT1R,
	.LDDCKR      = LCD_LDDCKR,
	.MLDDCKPAT1R = LCD_MLDDCKPAT1R,
	.MLDDCKPAT2R = LCD_MLDDCKPAT2R
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

static unsigned int reset_gpio;
static unsigned int irq_portno;
struct regulator *power_ldo_3v;
struct regulator *power_ldo_1v8;
static unsigned int power_gpio;

struct specific_cmdset {
	unsigned char cmd;
	unsigned char *data;
	int size;
};
#define MIPI_DSI_DCS_LONG_WRITE		(0x39)
#define MIPI_DSI_GEN_LONG_WRITE		(0x29)
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM	(0x15)
#define MIPI_DSI_GEN_SHORT_WRITE_PARAM	(0x23)
#define MIPI_DSI_DCS_SHORT_WRITE	(0x05)
#define MIPI_DSI_GEN_SHORT_WRITE	(0x03)
#define MIPI_DSI_SET_MAX_RETURN_PACKET	(0x37)
#define MIPI_DSI_DCS_READ		(0x06)
#define MIPI_DSI_DELAY			(0x00)
#define MIPI_DSI_BLACK			(0x01)
#define MIPI_DSI_END			(0xFF)

/* Enable extention command */
static unsigned char setextc[] = { 0xB9, 0xFF, 0x83, 0x69 };

/* Interface pixel format */
static unsigned char colmod[] = { 0x3A, 0x77 };

/* GOA Timing Control */
static unsigned char setgip[] = { 0xD5,
		0x00, 0x00, 0x13, 0x03, 0x35, 0x00, 0x01, 0x10, 0x01, 0x00,
		0x00, 0x00, 0x01, 0x7A, 0x16, 0x04, 0x20, 0x13, 0x11, 0x34,
		0x13, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x40, 0x00,
		0x88, 0x88, 0x54, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x88,
		0x88, 0x67, 0x13, 0x50, 0x00, 0x00, 0x50, 0x00, 0x88, 0x88,
		0x76, 0x31, 0x10, 0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x45,
		0x02, 0x40, 0x00, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x0A,
		0x00, 0xEF, 0x00, 0xEF, 0x0A, 0x00, 0xEF, 0x00, 0xEF, 0x00,
		0x01, 0x5A };

/* MIPI command */
static unsigned char mipicmd[] = { 0xBA,
		0x31, 0x00, 0x16, 0xCA, 0xB0, 0x0A, 0x00, 0x10, 0x28, 0x02,
		0x21, 0x21, 0x9A, 0x1A, 0x8F };

/* Power Control */
static unsigned char setpower[] = { 0xB1,
		0x0A, 0x83, 0x77, 0x00, 0x92, 0x12, 0x16, 0x16, 0x0C, 0x02 };

/* Gamma Setting */
static unsigned char setgamma[] = { 0xE0,
		0x00, 0x05, 0x0B, 0x2F, 0x2F, 0x30, 0x1B, 0x3E, 0x07, 0x0D,
		0x0E, 0x12, 0x13, 0x12, 0x14, 0x13, 0x1A, 0x00, 0x05, 0x0B,
		0x2F, 0x2F, 0x30, 0x1B, 0x3E, 0x07, 0x0D, 0x0E, 0x12, 0x13,
		0x12, 0x14, 0x13, 0x1A, 0x01 };

/* Digital Gamma */
static unsigned char setdgc[] = { 0xC1,
		0x03, 0x00, 0x09, 0x11, 0x17, 0x1C, 0x25, 0x2D, 0x34, 0x3D,
		0x45, 0x4C, 0x56, 0x5F, 0x68, 0x6F, 0x77, 0x80, 0x88, 0x90,
		0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8, 0xD0, 0xD8, 0xE0,
		0xE8, 0xF0, 0xF8, 0xFF, 0x00, 0x00, 0xAA, 0x0A, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x09, 0x11, 0x17, 0x1C, 0x25, 0x2D,
		0x34, 0x3D, 0x45, 0x4C, 0x56, 0x5F, 0x68, 0x6F, 0x77, 0x80,
		0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8, 0xD0,
		0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0xFF, 0x00, 0x00, 0xAA, 0x0A,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x11, 0x17, 0x1C,
		0x25, 0x2D, 0x34, 0x3D, 0x45, 0x4C, 0x56, 0x5F, 0x68, 0x6F,
		0x77, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0,
		0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0xFF, 0x00, 0x00,
		0xAA, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* RGB Setting */
static unsigned char setrgbif[] = { 0xB3, 0x83, 0x00, 0x31, 0x03 };

/* Display Inversion Setting */
static unsigned char setcyc[] = { 0xB4, 0x02 };

/* BGP voltage */
static unsigned char setbgp[] = { 0xB5, 0x0B, 0x0B, 0x24 };

/* Display direction */
static unsigned char setpanel[] = { 0xCC, 0x0E };

/* Internal used */
static unsigned char intused[] = { 0xC6, 0x40 };

/* Set Source Option */
static unsigned char setstba[] = { 0xC0, 0x73, 0x50, 0x00, 0x34, 0xC4, 0x02 };

/* Source EQ */
static unsigned char srceq[] = { 0xE3, 0x00, 0x00, 0x13, 0x1B };

/* SET CLOCK */
static unsigned char setclk[] = { 0xCB, 0x6D };

/* CABC Control */
static unsigned char setcabc[] = { 0xEA, 0x62 };

/* Sleep Out */
static unsigned char slpout[] = { 0x11 };

/* Display On */
static unsigned char dispon[] = { 0x29 };

/* Display off */
static unsigned char dispoff[] = { 0x28 };

/* Sleep in */
static unsigned char slpin[] = { 0x10 };


static const struct specific_cmdset initialize_cmdset[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,  setextc,   sizeof(setextc)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  colmod,    sizeof(colmod)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setgip,    sizeof(setgip)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  mipicmd,   sizeof(mipicmd)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  setpower,  sizeof(setpower) },
	{ MIPI_DSI_DCS_LONG_WRITE,  setgamma,  sizeof(setgamma) },
	{ MIPI_DSI_DCS_LONG_WRITE,  setdgc,    sizeof(setdgc)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setrgbif,  sizeof(setrgbif) },
	{ MIPI_DSI_DCS_LONG_WRITE,  setcyc,    sizeof(setcyc)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setbgp,    sizeof(setbgp)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setpanel,  sizeof(setpanel) },
	{ MIPI_DSI_DCS_LONG_WRITE,  intused,   sizeof(intused)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  setstba,   sizeof(setstba)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  srceq,     sizeof(srceq)    },
	{ MIPI_DSI_DCS_LONG_WRITE,  setclk,    sizeof(setclk)   },
	{ MIPI_DSI_DCS_LONG_WRITE,  setcabc,   sizeof(setcabc)  },
	{ MIPI_DSI_DCS_LONG_WRITE,  slpout,    sizeof(slpout)   },
	{ MIPI_DSI_DELAY,           NULL,      120              },
	{ MIPI_DSI_BLACK,           NULL,      0                },
	{ MIPI_DSI_DELAY,           NULL,      20               },
	{ MIPI_DSI_DCS_LONG_WRITE,  dispon,    sizeof(dispon)   },
	{ MIPI_DSI_DELAY,           NULL,      100              },
	{ MIPI_DSI_END,             NULL,      0                }
};

static const struct specific_cmdset demise_cmdset[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,  dispoff,   sizeof(dispoff)  },
	{ MIPI_DSI_DELAY,           NULL,      100              },
	{ MIPI_DSI_DCS_LONG_WRITE,  slpin,     sizeof(slpin)    },
	{ MIPI_DSI_DELAY,           NULL,      100              },
	{ MIPI_DSI_END,             NULL,      0                }
};

static int is_dsi_read_enabled;

static struct fb_info *common_fb_info;

enum lcdfreq_level_idx {
	LEVEL_NORMAL,		/* 60Hz */
	LEVEL_LOW,		/* 50Hz */
	LCDFREQ_LEVEL_END
};

struct lcdfreq_info {
	enum lcdfreq_level_idx	level;	/* Current level */
	struct mutex		lock;	/* Lock for change frequency */
	struct device		*dev;	/* Hold device of LCD */
};

static struct lcdfreq_info lcdfreq_info_data;

static int lcdfreq_lock_free(struct device *dev)
{
	void *screen_handle;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_delete disp_delete;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

	screen_handle =  screen_display_new();

	/* Setting peculiar to panel */
	set_lcd_if_param.handle			= screen_handle;
	set_lcd_if_param.port_no		= irq_portno;
	set_lcd_if_param.lcd_if_param		= &r_mobile_lcd_if_param;
	set_lcd_if_param.lcd_if_param_mask	= &r_mobile_lcd_if_param_mask;
	ret = screen_display_set_lcd_if_parameters(&set_lcd_if_param);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_lcd_if_parameters err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}


static ssize_t level_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	printk(KERN_DEBUG "%s\n", __func__);
	return sprintf(buf, "%d\n", lcdfreq_info_data.level);
}

static ssize_t level_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

	mutex_lock(&lcdfreq_info_data.lock);

	ret = strict_strtoul(buf, 0, (unsigned long *)&value);

	printk(KERN_DEBUG "\t%s :: value=%d\n", __func__, value);

	if (value >= LCDFREQ_LEVEL_END) {
		count = -EINVAL;
		goto out;
	}

	if (value) {
		/* set freq 50Hz */
		printk(KERN_ALERT "set low freq(50Hz)\n");

		r_mobile_lcd_if_param.DSI0PCKCR = LCD_DSI0PCKCR_50HZ;
		r_mobile_lcd_if_param.DSI0PHYCR = LCD_DSI0PHYCR_50HZ;
	} else {
		/* set freq 60Hz */
		printk(KERN_ALERT "set normal freq(60Hz)\n");

		r_mobile_lcd_if_param.DSI0PCKCR = LCD_DSI0PCKCR;
		r_mobile_lcd_if_param.DSI0PHYCR = LCD_DSI0PHYCR;
	}

	ret = lcdfreq_lock_free(dev);
	if (ret) {
		printk(KERN_ALERT "%s fail\n", __func__);
		count = -EINVAL;
		goto out;
	}

	lcdfreq_info_data.level = value;
out:
	mutex_unlock(&lcdfreq_info_data.lock);

	return count;
}

static DEVICE_ATTR(level, S_IRUGO|S_IWUSR, level_show, level_store);

static struct attribute *lcdfreq_attributes[] = {
	&dev_attr_level.attr,
	NULL,
};

static struct attribute_group lcdfreq_attr_group = {
	.name = "lcdfreq",
	.attrs = lcdfreq_attributes,
};

static int hx8369_b_lcd_frequency_register(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = NULL;
	int ret;

	printk(KERN_DEBUG "%s\n", __func__);

	memset(&lcdfreq_info_data, 0, sizeof(lcdfreq_info_data));

	lcdfreq = &lcdfreq_info_data;
	lcdfreq->dev = dev;
	lcdfreq->level = LEVEL_NORMAL;

	mutex_init(&lcdfreq->lock);

	ret = sysfs_create_group(&lcdfreq->dev->kobj, &lcdfreq_attr_group);
	if (ret < 0) {
		printk(KERN_ALERT "fail to add sysfs entries, %d\n", __LINE__);

		return ret;
	}

	printk(KERN_DEBUG "%s is done\n", __func__);

	return 0;
}

static void hx8369_b_lcd_frequency_unregister(void)
{
	printk(KERN_DEBUG "%s\n", __func__);

	sysfs_remove_group(&lcdfreq_info_data.dev->kobj,
					&lcdfreq_attr_group);
	mutex_destroy(&lcdfreq_info_data.lock);

	printk(KERN_DEBUG "%s is done\n", __func__);

}

int hx8369_b_dsi_read(int id, int reg, int len, char *buf)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_read_dsi_short read_dsi_s;
	screen_disp_delete disp_delete;
	int ret = 0;

	printk(KERN_INFO "%s\n", __func__);

	if (!is_dsi_read_enabled) {
		printk(KERN_ALERT "sequence error!!\n");
		return -EINVAL;
	}

	if ((len <= 0) || (len > 60) || (buf == NULL)) {
		printk(KERN_ALERT "argument error!!\n");
		return -EINVAL;
	}

	screen_handle =  screen_display_new();

	/* Set maximum return packet size  */
	write_dsi_s.handle		= screen_handle;
	write_dsi_s.output_mode	= RT_DISPLAY_LCD1;
	write_dsi_s.data_id		= MIPI_DSI_SET_MAX_RETURN_PACKET;
	write_dsi_s.reg_address	= len;
	write_dsi_s.write_data		= 0x00;
	write_dsi_s.reception_mode	= RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		goto out;
	}

	/* DSI read */
	read_dsi_s.handle		= screen_handle;
	read_dsi_s.output_mode		= RT_DISPLAY_LCD1;
	read_dsi_s.data_id		= id;
	read_dsi_s.reg_address		= reg;
	read_dsi_s.write_data		= 0;
	read_dsi_s.data_count		= len;
	read_dsi_s.read_data		= &buf[0];
	ret = screen_display_read_dsi_short_packet(&read_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK)
		printk(KERN_ALERT "disp_dsi_read err! ret = %d\n", ret);

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static int panel_specific_cmdset(void *lcd_handle,
				   const struct specific_cmdset *cmdset)
{
	int ret;
	int loop = 0;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;

	printk(KERN_INFO "%s\n", __func__);

	while (0 <= loop) {
		switch (cmdset[loop].cmd) {
		case MIPI_DSI_DCS_LONG_WRITE:
		case MIPI_DSI_GEN_LONG_WRITE:
			printk(KERN_INFO "panel_cmdset LONG Write\n");
			write_dsi_l.handle         = lcd_handle;
			write_dsi_l.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_l.data_id        = cmdset[loop].cmd;
			write_dsi_l.data_count     = cmdset[loop].size;
			write_dsi_l.write_data     = cmdset[loop].data;
			write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_ON;
			write_dsi_l.send_mode      = RT_DISPLAY_SEND_MODE_LP;
			ret = screen_display_write_dsi_long_packet(
					&write_dsi_l);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "display_write_dsi_long err %d!\n", ret);
				return -1;
			}
			break;
		case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
		case MIPI_DSI_GEN_SHORT_WRITE_PARAM:
			printk(KERN_INFO
			       "panel_cmdset SHORT Write with param\n");
			write_dsi_s.handle         = lcd_handle;
			write_dsi_s.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_s.data_id        = cmdset[loop].cmd;
			write_dsi_s.reg_address    = cmdset[loop].data[0];
			write_dsi_s.write_data     = cmdset[loop].data[1];
			write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
			ret = screen_display_write_dsi_short_packet(
					&write_dsi_s);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "disp_write_dsi_short err %d!\n", ret);
				return -1;
			}
			break;
		case MIPI_DSI_DCS_SHORT_WRITE:
		case MIPI_DSI_GEN_SHORT_WRITE:
			printk(KERN_INFO "panel_cmdset SHORT Write\n");
			write_dsi_s.handle         = lcd_handle;
			write_dsi_s.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_s.data_id        = cmdset[loop].cmd;
			write_dsi_s.reg_address    = cmdset[loop].data[0];
			write_dsi_s.write_data     = 0x00;
			write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
			ret = screen_display_write_dsi_short_packet(
					&write_dsi_s);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "disp_write_dsi_short err %d!\n", ret);
				return -1;
			}
			break;
		case MIPI_DSI_BLACK:
		{
			u32 panel_width  = R_MOBILE_M_PANEL_PIXEL_WIDTH;
			u32 panel_height = R_MOBILE_M_PANEL_PIXEL_HEIGHT;
#if 0 /* using dsi command */
			u32 line_size    = panel_width*3 + 1;
			u32 lines        = panel_height - 1;
			u32 line_num;
			unsigned char *line_data
					= kmalloc(line_size, GFP_KERNEL);
			if (line_data == NULL) {
				printk(KERN_ALERT
				       "kmalloc err!\n");
				return -1;
			}
			memset(line_data, 0, line_size);
			printk(KERN_INFO "panel_cmdset Black Paint\n");
			/* 1st line */
			*line_data = 0x2C;
			write_dsi_l.handle         = lcd_handle;
			write_dsi_l.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_l.data_id        = MIPI_DSI_DCS_LONG_WRITE;
			write_dsi_l.data_count     = line_size;
			write_dsi_l.write_data     = line_data;
			write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_OFF;
			write_dsi_l.send_mode      = RT_DISPLAY_SEND_MODE_HS;
			ret = screen_display_write_dsi_long_packet(
					&write_dsi_l);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "1st line write err!%d\n", ret);
				kfree(line_data);
				return -1;
			}
			/* 2nd line */
			*line_data = 0x3C;
			for (line_num = 0; line_num < lines; line_num++) {
				ret = screen_display_write_dsi_long_packet(
					&write_dsi_l);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					printk(KERN_ALERT
					       "2nd line or later write  err!"
					       "ret=%d line=%d\n",
					       ret, line_num);
					kfree(line_data);
					return -1;
				}
			}
			kfree(line_data);
#else  /* using rtapi */
			screen_disp_draw disp_draw;

			printk(KERN_ALERT
				"num_registered_fb = %d\n", num_registered_fb);

			if (!num_registered_fb) {
				printk(KERN_ALERT
					"num_registered_fb err!\n");
				return -1;
			}
			if (!registered_fb[0]->fix.smem_start) {
				printk(KERN_ALERT
					"registered_fb[0]->fix.smem_start"
					" is NULL err!\n");
				return -1;
			}
			printk(KERN_INFO
			       "registerd_fb[0]-> fix.smem_start: %08x\n"
			       "screen_base :%08x\n"
			       "fix.smem_len :%08x\n",
			       (unsigned)(registered_fb[0]->fix.smem_start),
			       (unsigned)(registered_fb[0]->screen_base),
			       (unsigned)(registered_fb[0]->fix.smem_len));
			memset(registered_fb[0]->screen_base, 0x0,
					registered_fb[0]->fix.smem_len);
#if 0 /* fill color for debug */
			{
				int xx;
				int max = (registered_fb[0]->fix.smem_len) / 4;
				unsigned int color = cmdset[loop].size;
				unsigned int *bbb =
				 (unsigned int *)registered_fb[0]->screen_base;
				printk(KERN_ALERT "color =  %08x\n", color);
				for (xx = 0; xx < max; xx++)
					bbb[xx] = color;
			}
#endif
			/* Memory clean */
			disp_draw.handle = lcd_handle;
			disp_draw.output_mode = RT_DISPLAY_LCD1;
/*			disp_draw.output_mode = RT_DISPLAY_LCD1_ASYNC;	*/
			disp_draw.buffer_id   = RT_DISPLAY_BUFFER_A;
			disp_draw.draw_rect.x = 0;
			disp_draw.draw_rect.y = 0;
			disp_draw.draw_rect.width  = panel_width;
			disp_draw.draw_rect.height = panel_height;
#ifdef CONFIG_FB_SH_MOBILE_RGB888
			disp_draw.format = RT_DISPLAY_FORMAT_RGB888;
#else
			disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
#endif
			disp_draw.buffer_offset = 0;
			disp_draw.rotate = RT_DISPLAY_ROTATE_270;
			ret = screen_display_draw(&disp_draw);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT "screen_display_draw err!\n");
				return -1;
			}
#endif
			break;
		}
		case MIPI_DSI_DELAY:
			msleep(cmdset[loop].size);
			break;

		case MIPI_DSI_END:
			loop = -2;
			break;
		default:
			printk(KERN_ALERT "Undefine command err!\n");
			return -1;
		}
		loop++;
	}

	return 0;
}

static void mipi_display_reset(void)
{

#if 0
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
	int ret;
#endif

	printk(KERN_INFO "%s\n", __func__);

#if 0
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK)
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

	/* GPIO control */
	gpio_request(reset_gpio, NULL);
	if (u2_get_board_rev() <= 4) {
		gpio_request(power_gpio, NULL);
	}
	gpio_direction_output(reset_gpio, 0);
	if (u2_get_board_rev() >= 5) {
		if(power_ldo_3v==NULL)
			printk(KERN_ERR "power_ldo_3v failed\n");
		else
			regulator_enable(power_ldo_3v);
		
		if(power_ldo_1v8==NULL)
			printk(KERN_ERR "power_ldo_1v8 failed\n");
		else
			regulator_enable(power_ldo_1v8);
	} else {
		gpio_direction_output(power_gpio, 1);
	}

	msleep(20);

	gpio_direction_output(reset_gpio, 1);

	msleep(50);
}


static int hx8369_b_panel_init(unsigned int mem_size)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_set_address set_address;
	screen_disp_delete disp_delete;
	unsigned char read_data[60];
	int ret = 0;

#ifdef HX8369_B_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
#endif

	printk(KERN_INFO "%s\n", __func__);

#ifdef HX8369_B_POWAREA_MNG_ENABLE
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK)
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

retry:
	screen_handle =  screen_display_new();

	/* LCD panel reset */
	mipi_display_reset();

	/* Setting peculiar to panel */
	set_lcd_if_param.handle			= screen_handle;
	set_lcd_if_param.port_no		= irq_portno;
	set_lcd_if_param.lcd_if_param		= &r_mobile_lcd_if_param;
	set_lcd_if_param.lcd_if_param_mask	= &r_mobile_lcd_if_param_mask;
	ret = screen_display_set_lcd_if_parameters(&set_lcd_if_param);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_lcd_if_parameters err!\n");
		goto out;
	}

	/* Setting FB address */
	set_address.handle	= screen_handle;
	set_address.output_mode	= RT_DISPLAY_LCD1;
	set_address.buffer_id	= RT_DISPLAY_BUFFER_A;
	set_address.address	= R_MOBILE_M_BUFF_ADDR;
	set_address.size	= R_MOBILE_M_BUFF_SIZE;
	ret = screen_display_set_address(&set_address);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_address err!\n");
		goto out;
	}

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		goto out;
	}
	is_dsi_read_enabled = 1;

	/* Read display identification information */
	ret = hx8369_b_dsi_read(MIPI_DSI_DCS_READ, 0x04, 1, &read_data[0]);
	if (ret == 0) {
		printk(KERN_DEBUG "read_data(RDID1) = %02X\n", read_data[1]);
		printk(KERN_DEBUG "read_data(RDID2) = %02X\n", read_data[2]);
		printk(KERN_DEBUG "read_data(RDID3) = %02X\n", read_data[3]);
	}

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		is_dsi_read_enabled = 0;
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		goto retry;
	}

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static int hx8369_b_panel_suspend(void)
{
	void *screen_handle;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_delete disp_delete;
	int ret;

#ifdef HX8369_B_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_end_notify;
	system_pmg_delete pmg_delete;
#endif
	printk(KERN_INFO "%s\n", __func__);

	screen_handle =  screen_display_new();

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, demise_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
#if 0
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
#endif
	}

	is_dsi_read_enabled = 0;

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	screen_display_stop_lcd(&disp_stop_lcd);

	/* GPIO control */
	gpio_direction_output(reset_gpio, 0);
	msleep(20);
	if (u2_get_board_rev() >= 5) {
	
		if(power_ldo_3v==NULL)
			printk(KERN_ERR "power_ldo_3v failed\n");
		else
			regulator_disable(power_ldo_3v);
		
		if(power_ldo_1v8==NULL)
			printk(KERN_ERR "power_ldo_1v8 failed\n");
		else
			regulator_disable(power_ldo_1v8);

	} else {
		gpio_direction_output(power_gpio, 0);
	}
	msleep(25);

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

#ifdef HX8369_B_POWAREA_MNG_ENABLE
	printk(KERN_INFO "End A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_end_notify.handle		= system_handle;
	powarea_end_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_end_notify(&powarea_end_notify);
	if (ret != SMAP_LIB_PWMNG_OK) {
		printk(KERN_ALERT "system_pwmng_powerarea_end_notify err!\n");
		pmg_delete.handle = system_handle;
		system_pwmng_delete(&pmg_delete);
		return -1;
	}

	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

	return 0;
}

static int hx8369_b_panel_resume(void)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_delete disp_delete;
	int ret = 0;

#ifdef HX8369_B_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
#endif

	printk(KERN_INFO "%s\n", __func__);

#ifdef HX8369_B_POWAREA_MNG_ENABLE
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK)
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

retry:
	screen_handle =  screen_display_new();

	/* LCD panel reset */
	mipi_display_reset();

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		goto out;
	}
	is_dsi_read_enabled = 1;

	/* Transmit DSI command peculiar to a panel */
	ret = panel_specific_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "panel_specific_cmdset err!\n");
		is_dsi_read_enabled = 0;
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		goto retry;
	}

out:
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return ret;
}

static int hx8369_b_panel_probe(struct fb_info *info,
			    struct fb_panel_hw_info hw_info)
{
	struct platform_device *pdev;
	struct resource *res_power_gpio;
	int ret;

	printk(KERN_INFO "%s\n", __func__);

	reset_gpio = hw_info.gpio_reg;
	irq_portno = hw_info.dsi_irq;

	/* fb parent device info to platform_device */
	pdev = to_platform_device(info->device);

	/* get resource info from platform_device */
	res_power_gpio	= platform_get_resource_byname(pdev,
							IORESOURCE_MEM,
							"panel_power_port");
	if (!res_power_gpio) {
		printk(KERN_ALERT "panel_power_port is NULL!!\n");
		return -ENODEV;
	}
	if (u2_get_board_rev() >= 5) {
		power_ldo_3v = regulator_get(NULL, "vlcd_3v");
		power_ldo_1v8 = regulator_get(NULL, "vlcd_1v8");
	} else {
		power_gpio	= res_power_gpio->start;
	}
	if (u2_get_board_rev() <= 4) {
		printk(KERN_INFO "GPIO_PORT%d : for panel power\n", power_gpio);
	}
	printk(KERN_INFO "GPIO_PORT%d : for panel reset\n", reset_gpio);
	printk(KERN_INFO "IRQ%d       : for panel te\n", irq_portno);

	common_fb_info = info;
	is_dsi_read_enabled = 0;

	/* register sysfs for LCD frequency control */
	ret = hx8369_b_lcd_frequency_register(info->device);
	if (ret < 0)
		return ret;

	return 0;
}

static int hx8369_b_panel_remove(struct fb_info *info)
{
	printk(KERN_INFO "%s\n", __func__);

	/* unregister sysfs for LCD frequency control */
	hx8369_b_lcd_frequency_unregister();

	return 0;
}

static struct fb_panel_info hx8369_b_panel_info(void)
{
	printk(KERN_INFO "%s\n", __func__);

	return r_mobile_info;
}

#ifndef CONFIG_FB_R_MOBILE_PANEL_SWITCH
struct fb_panel_func r_mobile_panel_func(int panel)
{

	struct fb_panel_func panel_func;

	printk(KERN_INFO "%s\n", __func__);

	memset(&panel_func, 0, sizeof(struct fb_panel_func));

/* e.g. support (panel=RT_DISPLAY_LCD1) */

	if (panel == RT_DISPLAY_LCD1) {
		panel_func.panel_init    = hx8369_b_panel_init;
		panel_func.panel_suspend = hx8369_b_panel_suspend;
		panel_func.panel_resume  = hx8369_b_panel_resume;
		panel_func.panel_probe   = hx8369_b_panel_probe;
		panel_func.panel_remove  = hx8369_b_panel_remove;
		panel_func.panel_info    = hx8369_b_panel_info;
	}

	return panel_func;
}
#else
struct fb_panel_func hx8369_b_func_list()
{
	struct fb_panel_func panel_func;

	printk(KERN_INFO "%s\n", __func__);

	memset(&panel_func, 0, sizeof(struct fb_panel_func));

	panel_func.panel_init    = hx8369_b_panel_init;
	panel_func.panel_suspend = hx8369_b_panel_suspend;
	panel_func.panel_resume  = hx8369_b_panel_resume;
	panel_func.panel_probe   = hx8369_b_panel_probe;
	panel_func.panel_remove  = hx8369_b_panel_remove;
	panel_func.panel_info    = hx8369_b_panel_info;

	return panel_func;
}
#endif
