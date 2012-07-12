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
#include <mach/r8a73734.h>

#include <video/sh_mobile_lcdc.h>

#include <rtapi/screen_display.h>
#include <linux/backlight.h>
#include <linux/lcd.h>

/*#define S6E39A0X02_BRIGHTNESS_LINER */
/*#define S6E39A0X02_USE_PANEL_INIT */

#include "s6e39a0x02_gamma.h"

/* framebuffer address */
#define R_MOBILE_M_BUFF_ADDR		0x4BC00000

/* panel pixel */
#define R_MOBILE_M_PANEL_PIXEL_WIDTH	540
#define R_MOBILE_M_PANEL_PIXEL_HEIGHT	960

/* panel size (mm) */
#define R_MOBILE_M_PANEL_SIZE_WIDTH	54
#define R_MOBILE_M_PANEL_SIZE_HEIGHT	95

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

#define MIN_BRIGHTNESS	(0)
#define MAX_BRIGHTNESS	(255)
#define INIT_BRIGHTNESS	MAX_BRIGHTNESS
#define BRIGHTNESS_THR	(100)

struct fb_panel_info r_mobile_info = {
	.pixel_width	= R_MOBILE_M_PANEL_PIXEL_WIDTH,
	.pixel_height	= R_MOBILE_M_PANEL_PIXEL_HEIGHT,
	.size_width	= R_MOBILE_M_PANEL_SIZE_WIDTH,
	.size_height	= R_MOBILE_M_PANEL_SIZE_HEIGHT,
	.buff_address	= R_MOBILE_M_BUFF_ADDR
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
static int in_suspend;

#ifdef S6E39A0X02_USE_PANEL_INIT
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
#endif

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

static struct lcd_device *registed_ld;

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
	int ret;
	/* set freq 40Hz */
	printk(KERN_ALERT "set freq NOT SUPPORTED(40Hz)\n");
	return ret;
}

static int lcdfreq_lock_free(struct device *dev)
{
	int ret;
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

	ret = strict_strtoul(buf, 0, (unsigned long *)&value);

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

	ret = sysfs_create_group(&dev->kobj, &lcdfreq_attr_group);
	if (ret < 0) {
		printk(KERN_ALERT "fail to add sysfs entries, %d\n", __LINE__);
		return ret;
	}

	printk(KERN_DEBUG "%s is done\n", __func__);

	return 0;
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
			printk(KERN_INFO "panel_cmdset LONG Write\n");
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
			printk(KERN_INFO
			       "panel_cmdset SHORT Write with param\n");
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
			printk(KERN_INFO "panel_cmdset SHORT Write\n");
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
			u32 line_num;
			u32 line_size = 540*3 + 1;
			unsigned char *line_data
					= kmalloc(line_size, GFP_KERNEL);
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
				       "disp_write_dsi_long err 2!\n");
				kfree(line_data);
				return -1;
			}
			/* 2nd line */
			*line_data = 0x3C;
			for (line_num = 0; line_num < 959; line_num++) {
				ret = screen_display_write_dsi_long_packet(
					&write_dsi_l);
				if (ret != SMAP_LIB_DISPLAY_OK) {
					printk(KERN_ALERT
					       "disp_write_dsi_long err 3!\n");
					kfree(line_data);
					return -1;
				}
			}
			kfree(line_data);
			break;
		}
		case MIPI_DSI_DELAY:
			msleep(cmdset[loop].size);
			break;
		case MIPI_DSI_END:
		default:
			loop = -2;
			break;
		}
		loop++;
	}

	return 0;
}

static int s6e39a0x02_update_gamma_ctl(int gamma)
{
	screen_disp_delete disp_delete;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;
	void *screen_handle;
	int ret;

	printk(KERN_DEBUG "s6e39a0x02_update_gamma_ctrl gamma:%d\n", gamma);
	if (in_suspend) {
		printk(KERN_DEBUG "s6e39a0x02_update_gamma_ctrl rejected\n");
		return 0;
	}
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

	if (brightness < MIN_BRIGHTNESS ||
		brightness > bd->props.max_brightness) {
		printk(KERN_WARNING "lcd brightness should be %d to %d.\n",
		MIN_BRIGHTNESS, MAX_BRIGHTNESS);
		return -EINVAL;
	}

	ret = s6e39a0x02_update_gamma_ctl(bd->props.brightness);
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

static int s6e39a0x02_backlight_device_register(void)
{
	struct backlight_device *bd;
	bd = backlight_device_register("s6e39a0x02-bl",
		NULL, NULL, &s6e39a0x02_backlight_ops, NULL);
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

static int s6e39a0x02_panel_init(unsigned int mem_size)
{
	void *screen_handle;
	screen_disp_start_lcd start_lcd;
	screen_disp_set_lcd_if_param set_lcd_if_param;
	screen_disp_set_address set_address;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_delete disp_delete;
	int ret;

	printk(KERN_INFO "s6e39a0x02_panel_init\n");

	in_suspend = 0;
	screen_handle =  screen_display_new();


	/* GPIO control */
	gpio_request(GPIO_PORT31, NULL);
	gpio_request(GPIO_PORT89, NULL);
	gpio_request(GPIO_PORT33, NULL);
#ifdef S6E39A0X02_USE_PANEL_INIT
	gpio_direction_output(GPIO_PORT31, 1);
	msleep(10);
	gpio_direction_output(GPIO_PORT31, 0);
	msleep(1);

	gpio_direction_output(GPIO_PORT89, 1);
	msleep(25);
	gpio_direction_output(GPIO_PORT31, 1);
	msleep(10);
#endif
	gpio_direction_input(GPIO_PORT33);

	/* Setting peculiar to panel */
	set_lcd_if_param.handle			= screen_handle;
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
	set_address.address	= R_MOBILE_M_BUFF_ADDR;
	set_address.size	= mem_size;
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

#ifdef S6E39A0X02_USE_PANEL_INIT
	/* Transmit DSI command peculiar to a panel */
	ret = s6e39a0x02_panel_cmdset(screen_handle, initialize_cmdset);
	if (ret != 0) {
		printk(KERN_ALERT "s6e39a0x02_panel_cmdset err!\n");
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
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

	return 0;
}

static int s6e39a0x02_panel_suspend(void)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_stop_lcd disp_stop_lcd;
	screen_disp_delete disp_delete;

	printk(KERN_INFO "s6e39a0x02_panel_suspend\n");
	in_suspend = 1;


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

	/* Stop a display to LCD */
	disp_stop_lcd.handle		= screen_handle;
	disp_stop_lcd.output_mode	= RT_DISPLAY_LCD1;
	screen_display_stop_lcd(&disp_stop_lcd);

	/* GPIO control */
	gpio_direction_output(GPIO_PORT31, 0);
	msleep(10);
	gpio_direction_output(GPIO_PORT89, 0);
	msleep(25);

	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int s6e39a0x02_panel_resume(void)
{
	void *screen_handle;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_start_lcd start_lcd;
	screen_disp_delete disp_delete;
	int ret;

	printk(KERN_INFO "s6e39a0x02_panel_resume\n");

	screen_handle =  screen_display_new();

	/* GPIO control */
	gpio_direction_output(GPIO_PORT31, 1);
	msleep(10);
	gpio_direction_output(GPIO_PORT31, 0);
	msleep(1);

	gpio_direction_output(GPIO_PORT89, 1);
	msleep(25);
	gpio_direction_output(GPIO_PORT31, 1);
	msleep(10);
	gpio_direction_input(GPIO_PORT33);

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
	in_suspend = 0;
	if (registed_bd)
		registed_bd->ops->update_status(registed_bd);

	/* Memory clean */
	{
		screen_disp_draw disp_draw;

		disp_draw.handle = screen_handle;
		disp_draw.output_mode = RT_DISPLAY_LCD1;
		disp_draw.draw_rect.x = 0;
		disp_draw.draw_rect.y = 0;
		disp_draw.draw_rect.width =
			R_MOBILE_M_PANEL_PIXEL_WIDTH;
		disp_draw.draw_rect.height =
			R_MOBILE_M_PANEL_PIXEL_HEIGHT;
		disp_draw.format = RT_DISPLAY_FORMAT_ARGB8888;
		disp_draw.buffer_offset = 0;
		disp_draw.rotate = RT_DISPLAY_ROTATE_0;

		ret = screen_display_draw(&disp_draw);
		if (ret != SMAP_LIB_DISPLAY_OK) {
			printk(KERN_ALERT "screen_display_draw err!\n");
			disp_delete.handle = screen_handle;
			screen_display_delete(&disp_delete);
			return -1;
		}
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

static struct fb_panel_info s6e39a0x02_panel_info(void)
{
	static int initialized;
	if (!initialized) {
		/* register device for LCD */
		registed_ld = lcd_device_register("s6e39a0x02",
							NULL, NULL, NULL);
		if (!registed_ld) {
			printk(KERN_ALERT "registed_ld is null!\n");
			return r_mobile_info;
		}
		/* register sysfs for LCD frequency control */
		s6e39a0x02_lcd_frequency_register(&(registed_ld->dev));

		/* register device for backlight control */
		s6e39a0x02_backlight_device_register();

		initialized = 1;
	}
	return r_mobile_info;
}

struct fb_panel_func r_mobile_panel_func(int panel)
{

	struct fb_panel_func panel_func;

	memset(&panel_func, 0, sizeof(struct fb_panel_func));

/* e.g. support (panel=RT_DISPLAY_LCD1) */

	if (panel == RT_DISPLAY_LCD1) {
		panel_func.panel_init    = s6e39a0x02_panel_init;
		panel_func.panel_suspend = s6e39a0x02_panel_suspend;
		panel_func.panel_resume  = s6e39a0x02_panel_resume;
		panel_func.panel_info    = s6e39a0x02_panel_info;
	}

	return panel_func;
}
