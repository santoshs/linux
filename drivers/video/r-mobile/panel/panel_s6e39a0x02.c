/*
 * drivers/video/r-mobile/panel/panel_s6e39a0x02.c
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
#include <rtapi/system_pwmng.h>
#include <linux/backlight.h>

#include <linux/platform_device.h>

/* #define S6E39A0X02_DRAW_BLACK_KERNEL */

#define S6E39A0X02_BRIGHTNESS_LINER
#define S6E39A0X02_POWAREA_MNG_ENABLE

#ifndef S6E39A0X02_BRIGHTNESS_LINER
#include "s6e39a0x02_gamma.h"
#endif
#include "panel_s6e39a0x02.h"

/* framebuffer address and size */
#define R_MOBILE_M_BUFF_ADDR		0x48C00000
#define R_MOBILE_M_BUFF_SIZE		(4 * 1024 * 1024)

/* panel pixel */
#define R_MOBILE_M_PANEL_PIXEL_WIDTH	540
#define R_MOBILE_M_PANEL_PIXEL_HEIGHT	960

/* panel size (mm) */
#define R_MOBILE_M_PANEL_SIZE_WIDTH	54
#define R_MOBILE_M_PANEL_SIZE_HEIGHT	95


/* pixclock = 1000000 / DCF */
#define R_MOBILE_M_PANEL_PIXCLOCK	19230

/* timing */
#define R_MOBILE_M_PANEL_LEFT_MARGIN	8
#define R_MOBILE_M_PANEL_RIGHT_MARGIN	496
#define R_MOBILE_M_PANEL_HSYNC_LEN	8
#define R_MOBILE_M_PANEL_UPPER_MARGIN	4
#define R_MOBILE_M_PANEL_LOWER_MARGIN	4
#define R_MOBILE_M_PANEL_VSYNC_LEN	2

#define LCD_DSITCKCR		0x00000007
#define LCD_DSI0PCKCR		0x00000313
#define LCD_DSI0PHYCR		0x2A80000E
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
/* #define LCD_MLDHCNR		0x003C007C */
/* #define LCD_MLDHSYNR		0x0001007A */
#define LCD_MLDHAJR		0x00000000
/* #define LCD_MLDVLNR		0x0360036A */
/* #define LCD_MLDVSYNR		0x00020364 */
#define LCD_MLDMT1R		0x0400000B
#define LCD_LDDCKR		0x0001007C
#define LCD_MLDDCKPAT1R		0x00000000
#define LCD_MLDDCKPAT2R		0x00000000
#define LCD_PHYTEST		0x0000038C

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
#define LCD_MASK_PHYTEST	0x000003CC

#define MIN_BRIGHTNESS	(0)
#define MAX_BRIGHTNESS	(255)
#define INIT_BRIGHTNESS	MAX_BRIGHTNESS
#define BRIGHTNESS_THR	(100)

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
	.MLDHAJR     = LCD_MLDHAJR,
	.MLDMT1R     = LCD_MLDMT1R,
	.LDDCKR      = LCD_LDDCKR,
	.MLDDCKPAT1R = LCD_MLDDCKPAT1R,
	.MLDDCKPAT2R = LCD_MLDDCKPAT2R,
	.PHYTEST     = LCD_PHYTEST
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
	LCD_MASK_MLDDCKPAT2R,
	LCD_MASK_PHYTEST
};

static unsigned int reset_gpio;
static unsigned int irq_portno;
static unsigned int power_gpio;

struct _s6e39a0x02_cmdset {
	unsigned char cmd;
	unsigned char *data;
	int size;
};
#define MIPI_DSI_DCS_LONG_WRITE		(0x39)
#define MIPI_DSI_DCS_SHORT_WRITE_PARAM	(0x15)
#define MIPI_DSI_DCS_SHORT_WRITE	(0x05)
#define MIPI_DSI_SET_MAX_RETURN_PACKET	(0x37)
#define MIPI_DSI_DCS_READ		(0x06)
#define MIPI_DSI_DELAY			(0x00)
#define MIPI_DSI_BLACK			(0x01)
#define MIPI_DSI_END			(0xFF)

static unsigned char data_01[] = { 0xf0, 0x5a, 0x5a };
static unsigned char data_02[] = { 0xf1, 0x5a, 0x5a };
static unsigned char data_03[] = { 0xfc, 0x5a, 0x5a };

static unsigned char data_04[] = { 0xfa, 0x02, 0x58, 0x42, 0x56, 0xaa, 0xc8,
				   0xae, 0xb5, 0xc1, 0xbe, 0xb4, 0xc0, 0xb2,
				   0x93, 0x9f, 0x93, 0xa6, 0xad, 0xa2, 0x00,
				   0xe9, 0x00, 0xdb, 0x01, 0x0f };
static unsigned char data_05[] = { 0xfa, 0x03 };

static unsigned char data_06[] = { 0xf8, 0x27, 0x27, 0x08, 0x08, 0x4e, 0xaa,
				   0x5e, 0x8a, 0x10, 0x3f, 0x10, 0x10, 0x00 };
static unsigned char data_07[] = { 0xf7, 0x03 };
static unsigned char data_08[] = { 0xb3, 0x63, 0x02, 0xc3, 0x32, 0xff };

static unsigned char data_09[] = { 0xf6, 0x00, 0x84, 0x09 };
static unsigned char data_10[] = { 0xb0, 0x09 };
static unsigned char data_11[] = { 0xd5, 0x64 };
static unsigned char data_12[] = { 0xb0, 0x0b };
static unsigned char data_13[] = { 0xd5, 0xa4, 0x7e, 0x20 };
static unsigned char data_14[] = { 0xb0, 0x08 };
static unsigned char data_15[] = { 0xfd, 0xf8 };
static unsigned char data_16[] = { 0xb0, 0x01 };
static unsigned char data_17[] = { 0xf2, 0x07 };
static unsigned char data_18[] = { 0xb0, 0x04 };
static unsigned char data_19[] = { 0xf2, 0x4d };
static unsigned char data_20[] = { 0xb1, 0x01, 0x00, 0x16 };
static unsigned char data_21[] = { 0xb2, 0x15, 0x15, 0x15, 0x15 };
/* Sleep Out */
static unsigned char data_22[] = { 0x11, 0x00 };

static unsigned char data_23[] = { 0x2a, 0x00, 0x00, 0x02, 0x57 };
static unsigned char data_24[] = { 0x2b, 0x00, 0x00, 0x03, 0xff };
static unsigned char data_25[] = { 0x2c, 0x00 };

static unsigned char data_26[] = { 0x35, 0x00 };
/* panel size is qHD 540x960 */
static unsigned char data_27[] = { 0x2a, 0x00, 0x1e, 0x02, 0x39 };
/* panel size is qHD 540x960 */
static unsigned char data_28[] = { 0x2b, 0x00, 0x00, 0x03, 0xbf };
static unsigned char data_29[] = { 0xd1, 0x8a };
static unsigned char last_gamma_table[] = {
	0xfa, 0x02, 0x58, 0x42, 0x56, 0xaa, 0xc8, 0xae, 0xb5,
	0xc1, 0xbe, 0xb4, 0xc0, 0xb2, 0x93, 0x9f, 0x93, 0xa6,
	0xad, 0xa2, 0x00, 0xe9, 0x00, 0xdb, 0x01, 0x0f };
static struct backlight_device *registed_bd;
static int is_backlight_enabled;
static int is_backlight_called;
static int is_dsi_read_enabled;

static const struct _s6e39a0x02_cmdset initialize_cmdset[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,        data_01, sizeof(data_01) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_02, sizeof(data_02) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_03, sizeof(data_03) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_04, sizeof(data_04) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_05, sizeof(data_05) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_06, sizeof(data_06) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_07, sizeof(data_07) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_08, sizeof(data_08) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_09, sizeof(data_09) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_10, sizeof(data_10) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_11, sizeof(data_11) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_12, sizeof(data_12) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_13, sizeof(data_13) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_14, sizeof(data_14) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_15, sizeof(data_15) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_16, sizeof(data_16) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_17, sizeof(data_17) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_18, sizeof(data_18) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_19, sizeof(data_19) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_20, sizeof(data_20) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_21, sizeof(data_21) },
	{ MIPI_DSI_DCS_SHORT_WRITE,       data_22, sizeof(data_22) },
	{ MIPI_DSI_DELAY,                 NULL,    120             },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_23, sizeof(data_23) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_24, sizeof(data_24) },
	{ MIPI_DSI_DCS_SHORT_WRITE,       data_25, sizeof(data_25) },
	{ MIPI_DSI_DELAY,                 NULL,    20              },
	{ MIPI_DSI_DCS_SHORT_WRITE,       data_26, sizeof(data_26) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_27, sizeof(data_27) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_28, sizeof(data_28) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_29, sizeof(data_29) },
	{ MIPI_DSI_BLACK,                 NULL,    0               },
	{ MIPI_DSI_END,                   NULL,    0               }
};

static const struct _s6e39a0x02_cmdset resume_cmdset[] = {
	{ MIPI_DSI_DCS_LONG_WRITE,        data_01, sizeof(data_01) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_02, sizeof(data_02) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_03, sizeof(data_03) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_04, sizeof(data_04) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_05, sizeof(data_05) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_06, sizeof(data_06) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_07, sizeof(data_07) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_08, sizeof(data_08) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_09, sizeof(data_09) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_10, sizeof(data_10) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_11, sizeof(data_11) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_12, sizeof(data_12) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_13, sizeof(data_13) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_14, sizeof(data_14) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_15, sizeof(data_15) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_16, sizeof(data_16) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_17, sizeof(data_17) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_18, sizeof(data_18) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_19, sizeof(data_19) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_20, sizeof(data_20) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_21, sizeof(data_21) },
	{ MIPI_DSI_DCS_SHORT_WRITE,       data_22, sizeof(data_22) },
	{ MIPI_DSI_DELAY,                 NULL,    120             },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_23, sizeof(data_23) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_24, sizeof(data_24) },
	{ MIPI_DSI_DCS_SHORT_WRITE,       data_25, sizeof(data_25) },
	{ MIPI_DSI_DELAY,                 NULL,    20              },
	{ MIPI_DSI_DCS_SHORT_WRITE,       data_26, sizeof(data_26) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_27, sizeof(data_27) },
	{ MIPI_DSI_DCS_LONG_WRITE,        data_28, sizeof(data_28) },
	{ MIPI_DSI_DCS_SHORT_WRITE_PARAM, data_29, sizeof(data_29) },
	{ MIPI_DSI_END,                   NULL,    0               }
};

static struct fb_info *common_fb_info;

enum lcdfreq_level_idx {
	LEVEL_NORMAL,		/* 60Hz */
	LEVEL_LOW,		/* 40Hz */
	LCDFREQ_LEVEL_END
};

struct lcdfreq_info {
	enum lcdfreq_level_idx	level;	/* Current level */
	struct mutex		lock;	/* Lock for change frequency */
	struct device		*dev;	/* Hold device of LCD */
};

static struct lcdfreq_info lcdfreq_info_data;

static int lcdfreq_lock(struct device *dev)
{
	int ret = 0;
	/* set freq 40Hz */
	printk(KERN_ALERT "set freq NOT SUPPORTED(40Hz)\n");
	return ret;
}

static int lcdfreq_lock_free(struct device *dev)
{
	int ret = 0;
	/* set freq 60Hz */
	printk(KERN_ALERT "set freq NOT SUPPORTED(60Hz)\n");
	return ret;
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

	ret = kstrtoul(buf, 0, (unsigned long *)&value);

	printk(KERN_DEBUG "\t%s :: value=%d\n", __func__, value);

	if (value >= LCDFREQ_LEVEL_END) {
		count = -EINVAL;
		goto out;
	}

	if (value)
		ret = lcdfreq_lock(dev);
	else
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

static int s6e39a0x02_lcd_frequency_register(struct device *dev)
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

static void s6e39a0x02_lcd_frequency_unregister(void)
{
	printk(KERN_DEBUG "%s\n", __func__);

	sysfs_remove_group(&lcdfreq_info_data.dev->kobj,
					&lcdfreq_attr_group);
	mutex_destroy(&lcdfreq_info_data.lock);

	printk(KERN_DEBUG "%s is done\n", __func__);

}

static int s6e39a0x02_panel_cmdset(void *lcd_handle,
				   const struct _s6e39a0x02_cmdset *cmdset)
{
	int ret;
	int loop = 0;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;

	while (0 <= loop) {
		switch (cmdset[loop].cmd) {
		case MIPI_DSI_DCS_LONG_WRITE:
			write_dsi_l.handle         = lcd_handle;
			write_dsi_l.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_l.data_id        = MIPI_DSI_DCS_LONG_WRITE;
			write_dsi_l.data_count     = cmdset[loop].size;
			write_dsi_l.write_data     = cmdset[loop].data;
			write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_OFF;
			write_dsi_l.send_mode      = RT_DISPLAY_SEND_MODE_HS;
			ret = screen_display_write_dsi_long_packet(
					&write_dsi_l);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
					"disp_write_dsi_long err= %d "
					"loop = %d, data = 0x%02x, 0x%02x\n",
					ret, loop, cmdset[loop].data[0],
					cmdset[loop].data[1]);
				return -1;
			}
			break;
		case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
			write_dsi_s.handle         = lcd_handle;
			write_dsi_s.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_s.data_id
				= MIPI_DSI_DCS_SHORT_WRITE_PARAM;
			write_dsi_s.reg_address    = cmdset[loop].data[0];
			write_dsi_s.write_data     = cmdset[loop].data[1];
			write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
			ret = screen_display_write_dsi_short_packet(
					&write_dsi_s);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "disp_write_dsi_short err 1!\n");
				return -1;
			}
			break;
		case MIPI_DSI_DCS_SHORT_WRITE:
			write_dsi_s.handle         = lcd_handle;
			write_dsi_s.output_mode    = RT_DISPLAY_LCD1;
			write_dsi_s.data_id        = MIPI_DSI_DCS_SHORT_WRITE;
			write_dsi_s.reg_address    = cmdset[loop].data[0];
			write_dsi_s.write_data     = 0x00;
			write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_ON;
			ret = screen_display_write_dsi_short_packet(
					&write_dsi_s);
			if (ret != SMAP_LIB_DISPLAY_OK) {
				printk(KERN_ALERT
				       "disp_write_dsi_short err 2!\n");
				return -1;
			}
			break;
		case MIPI_DSI_BLACK:
		{
			u32 panel_width  = R_MOBILE_M_PANEL_PIXEL_WIDTH;
			u32 panel_height = R_MOBILE_M_PANEL_PIXEL_HEIGHT;
			screen_disp_draw disp_draw;

#ifdef S6E39A0X02_DRAW_BLACK_KERNEL
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
#endif
			/* Memory clean */
			disp_draw.handle = lcd_handle;
#ifdef S6E39A0X02_DRAW_BLACK_KERNEL
			disp_draw.output_mode = RT_DISPLAY_LCD1;
			disp_draw.buffer_id   = RT_DISPLAY_BUFFER_A;
#else
			disp_draw.output_mode = RT_DISPLAY_LCD1_ASYNC;
			disp_draw.buffer_id   = RT_DISPLAY_DRAW_BLACK;
#endif
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

#ifdef SMART_DIMMING
static int s6e39a0x02_read_panel_id(void)
{
	int ret = 0, i = 0;
	unsigned char read_data[10];

	/* Read ID 1 : OLED module's manufacturer */
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, 0xDA, 1, &read_data[0]);
	if (ret) {
		printk(KERN_DEBUG "Error in dsi read with %d\n", ret);
		return ret;
	}

	/* Read ID 2 : OLED module/driver version */
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, 0xDB, 1, &read_data[1]);
	if (ret) {
		printk(KERN_DEBUG "Error in dsi read with %d\n", ret);
		return ret;
	}

	/* Read ID 3 : OLED module/driver */
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, 0xDC, 1, &read_data[2]);
	if (ret) {
		printk(KERN_DEBUG "Error in dsi read with %d\n", ret);
		return ret;
	}

	for (i = 0; i < 3; i++)
		smart.panelid[i] = read_data[i];

	printk(KERN_DEBUG
		"read id = %02X, %02X, %02X\n",
		read_data[0], read_data[1], read_data[2]);

	if (read_data[0] == PANEL_A1_SM2) {
		support_elvss = 1;
		aid = read_data[2] & 0xe0 >> 5;
		elvss.reference = read_data[2];

		printk(KERN_DEBUG
			"Dynamic ELVSS Information, "
			"refrence : %02x , aid= %02x\n", elvss.reference, aid);
	} else if ((read_data[0] == PANEL_A1_M3) ||
		   (read_data[0] == PANEL_A2_M3)) {
		support_elvss = 0;
		printk(KERN_DEBUG
			"Dynamic ELVSS is not supported in this panel.\n");
	} else {
		support_elvss = 0;
		printk(KERN_DEBUG
			"Panel ID is not proper for Smart Dimming.\n");
	}

	return 0;
}

static int s6e39a0x02_read_panel_mtp(u8 *mtp_data)
{
	int ret;
	u8 retry_cnt = 3;

read_retry:
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, LDI_MTP_ADDR,
					LDI_MTP_LENGTH, mtp_data);
	if (ret) {
		if (retry_cnt) {
			printk(KERN_WARNING
				"[WARN:LCD] : %s : retry cnt : %d\n",
				__func__, retry_cnt);
			retry_cnt--;
			goto read_retry;
		} else
			printk(KERN_ERR "ERROR:MTP read failed\n");
	}

	return ret;
}

static u32 transform_gamma(u32 brightness)
{
	u32 gamma;

	switch (brightness) {
	case 0:
		gamma = 30;
		break;
	case 1 ... 29:
		gamma = 30;
		break;
	case 30 ... 34:
		gamma = 40;
		break;
	case 35 ... 44:
		gamma = 70;
		break;
	case 45 ... 54:
		gamma = 80;
		break;
	case 55 ... 64:
		gamma = 90;
		break;
	case 65 ... 74:
		gamma = 100;
		break;
	case 75 ... 83:
		gamma = 110;
		break;
	case 84 ... 93:
		gamma = 120;
		break;
	case 94 ... 103:
		gamma = 130;
		break;
	case 104 ... 113:
		gamma = 140;
		break;
	case 114 ... 122:
		gamma = 150;
		break;
	case 123 ... 132:
		gamma = 160;
		break;
	case 133 ... 142:
		gamma = 170;
		break;
	case 143 ... 152:
		gamma = 180;
		break;
	case 153 ... 162:
		gamma = 190;
		break;
	case 163 ... 171:
		gamma = 200;
		break;
	case 172 ... 181:
		gamma = 210;
		break;
	case 182 ... 191:
		gamma = 220;
		break;
	case 192 ... 201:
		gamma = 230;
		break;
	case 202 ... 210:
		gamma = 240;
		break;
	case 211 ... 220:
		gamma = 250;
		break;
	case 221 ... 230:
		gamma = 260;
		break;
	case 231 ... 240:
		gamma = 270;
		break;
	case 241 ... 250:
		gamma = 280;
		break;
	case 251 ... 255:
		gamma = 290;
		break;
	default:
		gamma = 150;
		break;
	}
	return gamma - 1;
}

static u8 get_offset_brightness(u32 candela)
{
	u8 offset = 0;

	switch (candela) {
	case 0 ... 100:
		offset = ELVSS_OFFSET_MIN;
		break;
	case 101 ... 160:
		offset = ELVSS_OFFSET_1;
		break;
	case 161 ... 200:
		offset = ELVSS_OFFSET_2;
		break;
	case 201 ...  300:
		offset = ELVSS_OFFSET_MAX;
		break;
	default:
		offset = ELVSS_OFFSET_MAX;
		break;
	}
	return offset;
}

static u8 get_elvss_value(u32 candela)
{
	u8 ref = 0;
	u8 offset;

	ref = (elvss.reference | 0x80);
	offset = get_offset_brightness(candela);
	ref += offset;

	printk(KERN_DEBUG "%s ref =0x%x , offset = 0x%x\n",
					__func__, ref, offset);

	if (ref < DYNAMIC_ELVSS_MIN_VALUE)
		ref = DYNAMIC_ELVSS_MIN_VALUE;
	else if (ref > DYNAMIC_ELVSS_MAX_VALUE)
		ref = DYNAMIC_ELVSS_MAX_VALUE;

	return ref;
}

static int s6e39a0x02_update_elvss(u32 candela)
{
	u8 elvss_cmd[3];
	u8 elvss;
#if 0
	elvss = get_elvss_value(candela);
	if (!elvss) {
		printk(KERN_ERR
			"[ERROR:LCD]: %s:get_elvss_value() failed\n", __func__);
		return -EPERM;
	}

	elvss_cmd[0] = 0xb1;
	elvss_cmd[1] = 0x04;
	elvss_cmd[2] = elvss;

	printk(KERN_DEBUG "elvss reg : %02x\n", elvss_cmd[2]);
	s6e8ax0_write(lcd, elvss_cmd, sizeof(elvss_cmd));
#endif

	return 0;
}

static int s6e39a0x02_update_gamma(u32 brightness)
{
	screen_disp_delete disp_delete;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;
	void *screen_handle;
	int ret = 0;

	u8 gamma_regs[GAMMA_TABLE_COUNT] = {0,};
	u32 gamma;

	gamma_regs[0] = 0xFA;
	gamma_regs[1] = 0x02;

	gamma = brightness;

	calc_gamma_table(&smart, gamma, gamma_regs+2);

	screen_handle = screen_display_new();

	/* set gamma table */
	write_dsi_l.handle	= screen_handle;
	write_dsi_l.output_mode = RT_DISPLAY_LCD1;
	write_dsi_l.data_id = MIPI_DSI_DCS_LONG_WRITE;
	write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_OFF;
	write_dsi_l.send_mode = RT_DISPLAY_SEND_MODE_HS;
	write_dsi_l.data_count = GAMMA_TABLE_COUNT;
	write_dsi_l.write_data = gamma_regs;
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ERR "write_dsi_long_packet err:%d\n", ret);
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	/* update gamma */
	write_dsi_s.handle	= screen_handle;
	write_dsi_s.output_mode = RT_DISPLAY_LCD1;
	write_dsi_s.data_id	= MIPI_DSI_DCS_SHORT_WRITE_PARAM;
	write_dsi_s.reg_address = 0xFA;
	write_dsi_s.write_data	= 0x03;
	write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_OFF;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ERR "disp_write_dsi_short err: %d\n", ret);
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int update_brightness(u32 br)
{
	u32 gamma;
	int ret = 0;

	if (!is_backlight_enabled) {
		printk(KERN_DEBUG "update_brightness rejected\n");
		return 0;
	}

	gamma = transform_gamma(br);

	printk(KERN_DEBUG
		"update_brightness: brightness=%d, gamma=%d\n", br, gamma);

	ret = s6e39a0x02_update_gamma(gamma);

	/* ret = s6e8ax0_set_acl(lcd); */

	if (support_elvss)
		ret = s6e39a0x02_update_elvss(gamma);

	return ret;
}

#endif	/* SMART_DIMMING */


static int s6e39a0x02_update_gamma_ctl(int gamma)
{
	screen_disp_delete disp_delete;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;
	void *screen_handle;
	int ret;

	if (!is_backlight_enabled) {
		printk(KERN_DEBUG
			"s6e39a0x02_update_gamma_ctrl rejected. "
			"just a warning, not a real error.\n");
		return 0;
	}

	printk(KERN_DEBUG "s6e39a0x02_update_gamma_ctrl gamma:%d\n", gamma);

	screen_handle = screen_display_new();

	/* set gamma table */
	write_dsi_l.handle	= screen_handle;
	write_dsi_l.output_mode = RT_DISPLAY_LCD1;
	write_dsi_l.data_id = MIPI_DSI_DCS_LONG_WRITE;
	write_dsi_l.reception_mode = RT_DISPLAY_RECEPTION_OFF;
	write_dsi_l.send_mode = RT_DISPLAY_SEND_MODE_HS;

#ifndef S6E39A0X02_BRIGHTNESS_LINER
	if (gamma >= BRIGHTNESS_THR) {
		int idx = (gamma - BRIGHTNESS_THR) /
			((MAX_BRIGHTNESS - BRIGHTNESS_THR) / MAX_GAMMA_LEVEL);
		if (idx >= MAX_GAMMA_LEVEL)
			idx = MAX_GAMMA_LEVEL - 1;
		printk(KERN_DEBUG "set from gamma table index:%d\n", idx);
		write_dsi_l.data_count = GAMMA_TABLE_COUNT;
		write_dsi_l.write_data = (u8 *)s6e39a0x02_22_gamma_table[idx];
	} else {
#endif /* S6E39A0X02_BRIGHTNESS_LINER */
		last_gamma_table[20] = (gamma + 0) >> 8;
		last_gamma_table[21] = (gamma + 0) & 0xff;
		last_gamma_table[22] = (gamma + 4) >> 8;
		last_gamma_table[23] = (gamma + 4) & 0xff;
		last_gamma_table[24] = (gamma + 52) >> 8;
		last_gamma_table[25] = (gamma + 52) & 0xff;
		write_dsi_l.data_count = sizeof(last_gamma_table);
		write_dsi_l.write_data = last_gamma_table;
#ifndef S6E39A0X02_BRIGHTNESS_LINER
	}
#endif /* S6E39A0X02_BRIGHTNESS_LINER */
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ERR "write_dsi_long_packet err:%d\n", ret);
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	/* update gamma */
	write_dsi_s.handle	= screen_handle;
	write_dsi_s.output_mode = RT_DISPLAY_LCD1;
	write_dsi_s.data_id	= MIPI_DSI_DCS_SHORT_WRITE_PARAM;
	write_dsi_s.reg_address = 0xFA;
	write_dsi_s.write_data	= 0x03;
	write_dsi_s.reception_mode = RT_DISPLAY_RECEPTION_OFF;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ERR "disp_write_dsi_short err: %d\n", ret);
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int get_brightness(struct backlight_device *bd)
{
	printk(KERN_DEBUG "brightness:%d\n", bd->props.brightness);
	return bd->props.brightness;
}

static int set_brightness(struct backlight_device *bd)
{
	int ret = 0, brightness = bd->props.brightness;

	is_backlight_called = 1;

	if (brightness < MIN_BRIGHTNESS ||
		brightness > bd->props.max_brightness) {
		printk(KERN_WARNING "lcd brightness should be %d to %d.\n",
		MIN_BRIGHTNESS, MAX_BRIGHTNESS);
		return -EINVAL;
	}
#ifdef SMART_DIMMING
	ret = update_brightness(bd->props.brightness);
#else
	ret = s6e39a0x02_update_gamma_ctl(bd->props.brightness);
#endif
	if (ret) {
		printk(KERN_ERR "lcd brightness setting failed.\n");
		return -EIO;
	}

	return ret;
}

static const struct backlight_ops s6e39a0x02_backlight_ops  = {
	.get_brightness = get_brightness,
	.update_status = set_brightness,
};

static int s6e39a0x02_backlight_device_register(struct device *dev)
{
	struct backlight_device *bd;

	is_backlight_enabled = 0;
	is_backlight_called = 0;

	bd = backlight_device_register("panel",
		dev, NULL, &s6e39a0x02_backlight_ops, NULL);
	if (IS_ERR(bd)) {
		printk(KERN_ERR "backlight_device_register err\n");
		return PTR_ERR(bd);
	}

	bd->props.max_brightness = MAX_BRIGHTNESS;
	bd->props.brightness = INIT_BRIGHTNESS;
	registed_bd = bd;
	printk(KERN_INFO "s6e39a0x02 Backlight Driver Initialized\n");

	return 0;
}

static void s6e39a0x02_backlight_device_unregister(void)
{
	if (registed_bd)
		backlight_device_unregister(registed_bd);

	printk(KERN_INFO "s6e39a0x02 Backlight Driver Terminated\n");
}

int s6e39a0x02_dsi_read(int id, int reg, int len, char *buf)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_read_dsi_short read_dsi_s;
	screen_disp_delete disp_delete;
	int ret;

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
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
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
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_dsi_read err! ret = %d\n", ret);
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int s6e39a0x02_panel_init(unsigned int mem_size)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_set_address set_address;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_delete disp_delete;

#ifdef S6E39A0X02_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
#endif

	int ret;
	unsigned int tmp;
	unsigned char read_data[60];
#ifdef SMART_DIMMING
	u8 mtp_data[LDI_MTP_LENGTH] = {0,};
#endif
	printk(KERN_INFO "%s\n", __func__);

	screen_handle = screen_display_new();

#ifdef S6E39A0X02_POWAREA_MNG_ENABLE
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK) {
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
		pmg_delete.handle = system_handle;
		system_pwmng_delete(&pmg_delete);
		return -1;
	}

	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

	/* GPIO control */
	gpio_request(reset_gpio, NULL);
	gpio_request(power_gpio, NULL);

	gpio_direction_output(reset_gpio, 1);
	msleep(10);
	gpio_direction_output(reset_gpio, 0);
	msleep(1);

	gpio_direction_output(power_gpio, 1);
	msleep(25);
	gpio_direction_output(reset_gpio, 1);

	msleep(100);


	tmp = R_MOBILE_M_PANEL_PIXEL_WIDTH + R_MOBILE_M_PANEL_HSYNC_LEN;
	tmp += R_MOBILE_M_PANEL_LEFT_MARGIN;
	tmp += R_MOBILE_M_PANEL_RIGHT_MARGIN;
	tmp /= 8; /* HTCN */
	tmp |= (R_MOBILE_M_PANEL_PIXEL_WIDTH / 8) << 16; /* HDCN */
	r_mobile_lcd_if_param.MLDHCNR = tmp;

	tmp = R_MOBILE_M_PANEL_PIXEL_WIDTH + R_MOBILE_M_PANEL_RIGHT_MARGIN;
	tmp /= 8; /* HSYNP */
	tmp |= (R_MOBILE_M_PANEL_HSYNC_LEN / 8) << 16; /* HSYNW */
	r_mobile_lcd_if_param.MLDHSYNR = tmp;

	tmp = R_MOBILE_M_PANEL_PIXEL_HEIGHT + R_MOBILE_M_PANEL_VSYNC_LEN;
	tmp += R_MOBILE_M_PANEL_UPPER_MARGIN;
	tmp += R_MOBILE_M_PANEL_LOWER_MARGIN; /* VTLN */
	tmp |= R_MOBILE_M_PANEL_PIXEL_HEIGHT << 16; /* VDLN */
	r_mobile_lcd_if_param.MLDVLNR = tmp;

	/* VSYNP */
	tmp = R_MOBILE_M_PANEL_PIXEL_HEIGHT + R_MOBILE_M_PANEL_LOWER_MARGIN;
	tmp |= R_MOBILE_M_PANEL_VSYNC_LEN << 16; /* VSYNW */
	r_mobile_lcd_if_param.MLDVSYNR = tmp;


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

	/* Setting FB address */
	set_address.handle	= screen_handle;
	set_address.output_mode	= RT_DISPLAY_LCD1;
	set_address.buffer_id	= RT_DISPLAY_BUFFER_A;
	set_address.address	= R_MOBILE_M_BUFF_ADDR;
	set_address.size	= R_MOBILE_M_BUFF_SIZE;
	ret = screen_display_set_address(&set_address);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_set_address err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	is_dsi_read_enabled = 1;

	/* Read ID1 */
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, 0xDA, 1, &read_data[0]);
	if (ret == 0)
		printk(KERN_DEBUG "read_data(0xDA) = %02X\n", read_data[0]);

	/* Read ID2 */
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, 0xDB, 1, &read_data[0]);
	if (ret == 0)
		printk(KERN_DEBUG "read_data(0xDB) = %02X\n", read_data[0]);

	/* Read ID3 */
	ret = s6e39a0x02_dsi_read(MIPI_DSI_DCS_READ, 0xDC, 1, &read_data[0]);
	if (ret == 0)
		printk(KERN_DEBUG "read_data(0xDC) = %02X\n", read_data[0]);


	/* Transmit DSI command peculiar to a panel */
	ret = s6e39a0x02_panel_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "s6e39a0x02_panel_cmdset err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
#ifdef SMART_DIMMING
	s6e39a0x02_read_panel_id();

	init_table_info(&smart);

	ret = s6e39a0x02_read_panel_mtp(mtp_data);
	if (ret) {
		printk(KERN_ERR "[LCD:ERROR] : %s read mtp failed\n", __func__);
		/*return -EPERM;*/
	}

	calc_voltage_table(&smart, mtp_data);
#endif

	/* Display on */
	write_dsi_s.handle		= screen_handle;
	write_dsi_s.output_mode		= RT_DISPLAY_LCD1;
	write_dsi_s.data_id		= MIPI_DSI_DCS_SHORT_WRITE;
	write_dsi_s.reg_address		= 0x29;
	write_dsi_s.write_data		= 0x00;
	write_dsi_s.reception_mode	= RT_DISPLAY_RECEPTION_ON;
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_write_dsi_short err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	/* update gamma table at last changed */
	is_backlight_enabled = 1;
	if (registed_bd && is_backlight_called)
		registed_bd->ops->update_status(registed_bd);

	return 0;
}

static int s6e39a0x02_panel_suspend(void)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_delete disp_delete;

#ifdef S6E39A0X02_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_end_notify;
	system_pmg_delete pmg_delete;
#endif

	int ret = 0;
	printk(KERN_INFO "%s\n", __func__);
	is_backlight_enabled = 0;


	screen_handle =  screen_display_new();

	/* Sleep in */
	write_dsi_s.handle		= screen_handle;
	write_dsi_s.output_mode		= RT_DISPLAY_LCD1;
	write_dsi_s.data_id		= MIPI_DSI_DCS_SHORT_WRITE;
	write_dsi_s.reg_address		= 0x10;
	write_dsi_s.write_data		= 0x00;
	write_dsi_s.reception_mode	= RT_DISPLAY_RECEPTION_ON;
	screen_display_write_dsi_short_packet(&write_dsi_s);

	/* 120ms wait */
	msleep(120);

	is_dsi_read_enabled = 0;

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	screen_display_stop_lcd(&disp_stop_lcd);

	/* GPIO control */
	gpio_direction_output(reset_gpio, 0);
	msleep(10);
	gpio_direction_output(power_gpio, 0);
	msleep(25);

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

#ifdef S6E39A0X02_POWAREA_MNG_ENABLE
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

static int s6e39a0x02_panel_resume(void)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_start_lcd start_lcd;
	screen_disp_draw disp_draw;
	screen_disp_delete disp_delete;
	int ret;

#ifdef S6E39A0X02_POWAREA_MNG_ENABLE
	void *system_handle;
	system_pmg_param powarea_start_notify;
	system_pmg_delete pmg_delete;
#endif
	printk(KERN_INFO "%s\n", __func__);

	screen_handle =  screen_display_new();

#ifdef S6E39A0X02_POWAREA_MNG_ENABLE
	printk(KERN_INFO "Start A4LC power area\n");
	system_handle = system_pwmng_new();

	/* Notifying the Beginning of Using Power Area */
	powarea_start_notify.handle		= system_handle;
	powarea_start_notify.powerarea_name	= RT_PWMNG_POWERAREA_A4LC;
	ret = system_pwmng_powerarea_start_notify(&powarea_start_notify);
	if (ret != SMAP_LIB_PWMNG_OK) {
		printk(KERN_ALERT "system_pwmng_powerarea_start_notify err!\n");
		pmg_delete.handle = system_handle;
		system_pwmng_delete(&pmg_delete);
		return -1;
	}

	pmg_delete.handle = system_handle;
	system_pwmng_delete(&pmg_delete);
#endif

	/* GPIO control */
	gpio_direction_output(reset_gpio, 1);
	msleep(10);
	gpio_direction_output(reset_gpio, 0);
	msleep(1);

	gpio_direction_output(power_gpio, 1);
	msleep(25);
	gpio_direction_output(reset_gpio, 1);

	/* 120ms wait */
	msleep(120);

	/* Start a display to LCD */
	start_lcd.handle	= screen_handle;
	start_lcd.output_mode	= RT_DISPLAY_LCD1;
	ret = screen_display_start_lcd(&start_lcd);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "disp_start_lcd err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	is_dsi_read_enabled = 1;

	/* Draw display */
	ret = s6e39a0x02_panel_cmdset(screen_handle, resume_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "s6e39a0x02_panel_cmdset err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	/* 17ms wait */
	msleep(17);

	/* update gamma table at last changed */
	is_backlight_enabled = 1;
	if (registed_bd)
		registed_bd->ops->update_status(registed_bd);

	/* Memory clean */
	disp_draw.handle = screen_handle;
	disp_draw.output_mode = RT_DISPLAY_LCD1;
	disp_draw.draw_rect.x = 0;
	disp_draw.draw_rect.y = 0;
	disp_draw.draw_rect.width = R_MOBILE_M_PANEL_PIXEL_WIDTH;
	disp_draw.draw_rect.height = R_MOBILE_M_PANEL_PIXEL_HEIGHT;
#ifdef CONFIG_FB_SH_MOBILE_RGB888
	disp_draw.format = RT_DISPLAY_FORMAT_RGB888;
#else
	disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
#endif
	disp_draw.buffer_id = RT_DISPLAY_BUFFER_A;
	disp_draw.buffer_offset = 0;
	disp_draw.rotate = RT_DISPLAY_ROTATE_270;
	ret = screen_display_draw(&disp_draw);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		printk(KERN_ALERT "screen_display_draw err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}

	/* Display on */
	write_dsi_s.handle		= screen_handle;
	write_dsi_s.output_mode		= RT_DISPLAY_LCD1;
	write_dsi_s.data_id		= MIPI_DSI_DCS_SHORT_WRITE;
	write_dsi_s.reg_address		= 0x29;
	write_dsi_s.write_data		= 0x00;
	write_dsi_s.reception_mode	= RT_DISPLAY_RECEPTION_ON;
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

static int s6e39a0x02_panel_probe(struct fb_info *info,
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
	power_gpio	= res_power_gpio->start;
	printk(KERN_INFO "GPIO_PORT%d : for panel power\n", power_gpio);
	printk(KERN_INFO "GPIO_PORT%d : for panel reset\n", reset_gpio);
	printk(KERN_INFO "IRQ%d       : for panel te\n", irq_portno);

	common_fb_info = info;
	is_dsi_read_enabled = 0;

	/* register sysfs for LCD frequency control */
	ret = s6e39a0x02_lcd_frequency_register(info->device);
	if (ret < 0)
		return ret;

	/* register device for backlight control */
	ret = s6e39a0x02_backlight_device_register(info->dev);

	return ret;
}

static int s6e39a0x02_panel_remove(struct fb_info *info)
{
	printk(KERN_INFO "%s\n", __func__);

	/* unregister device for backlight control */
	s6e39a0x02_backlight_device_unregister();

	/* unregister sysfs for LCD frequency control */
	s6e39a0x02_lcd_frequency_unregister();

	return 0;
}

static struct fb_panel_info s6e39a0x02_panel_info(void)
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
		panel_func.panel_init    = s6e39a0x02_panel_init;
		panel_func.panel_suspend = s6e39a0x02_panel_suspend;
		panel_func.panel_resume  = s6e39a0x02_panel_resume;
		panel_func.panel_probe   = s6e39a0x02_panel_probe;
		panel_func.panel_remove  = s6e39a0x02_panel_remove;
		panel_func.panel_info    = s6e39a0x02_panel_info;
	}

	return panel_func;
}
#else
struct fb_panel_func s6e39a0x02_func_list()
{
	struct fb_panel_func panel_func;

	printk(KERN_INFO "%s\n", __func__);

	memset(&panel_func, 0, sizeof(struct fb_panel_func));

	panel_func.panel_init    = s6e39a0x02_panel_init;
	panel_func.panel_suspend = s6e39a0x02_panel_suspend;
	panel_func.panel_resume  = s6e39a0x02_panel_resume;
	panel_func.panel_probe   = s6e39a0x02_panel_probe;
	panel_func.panel_remove  = s6e39a0x02_panel_remove;
	panel_func.panel_info    = s6e39a0x02_panel_info;

	return panel_func;
}
#endif
