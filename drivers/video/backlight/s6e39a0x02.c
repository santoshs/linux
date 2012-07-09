/*
 * s6e39a0x02 AMOLED LCD panel driver.
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
#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <video/mipi_display.h>

#include <rtapi/screen_display.h>

#include "s6e39a0x02_gamma.h"

#define MIN_BRIGHTNESS		(0)
#define MAX_BRIGHTNESS		(255)
#define INIT_BRIGHTNESS		MAX_BRIGHTNESS
#define BRIGHTNESS_THR		(100)

struct s6e39a0x02 {
	u8			gamma_table[GAMMA_TABLE_COUNT];
	struct backlight_device	*bd;
};

static int s6e39a0x02_update_gamma_ctl(struct s6e39a0x02 *data, int gamma)
{
	screen_disp_delete disp_delete;
	screen_disp_write_dsi_short write_dsi_s;
	screen_disp_write_dsi_long  write_dsi_l;
	void *screen_handle;
	int ret;

	dev_dbg(&data->bd->dev, "update_gamma_ctrl gamma:%d\n", gamma);
	screen_handle = screen_display_new();

	/* set gamma table */
	write_dsi_l.handle	= screen_handle;
	write_dsi_l.output_mode = RT_DISPLAY_LCD1;
	write_dsi_l.data_id = MIPI_DSI_DCS_LONG_WRITE;

	if (gamma >= BRIGHTNESS_THR) {
		int index = (gamma - BRIGHTNESS_THR) /
			((MAX_BRIGHTNESS - BRIGHTNESS_THR) / MAX_GAMMA_LEVEL);
		if (index >= MAX_GAMMA_LEVEL) index = MAX_GAMMA_LEVEL - 1;
		dev_dbg(&data->bd->dev, "from gamma table index:%d\n", index);
		write_dsi_l.data_count = GAMMA_TABLE_COUNT;
		write_dsi_l.write_data = (u8 *)s6e39a0x02_22_gamma_table[index];
	} else {
		data->gamma_table[20] = (gamma + 0) >> 8;
		data->gamma_table[21] = (gamma + 0) & 0xff;
		data->gamma_table[22] = (gamma + 4) >> 8;
		data->gamma_table[23] = (gamma + 4) & 0xff;
		data->gamma_table[24] = (gamma + 52) >> 8;
		data->gamma_table[25] = (gamma + 52) & 0xff;
		write_dsi_l.data_count = sizeof(data->gamma_table);
		write_dsi_l.write_data = data->gamma_table;
	}
	ret = screen_display_write_dsi_long_packet(&write_dsi_l);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		dev_err(&data->bd->dev, "write_dsi_long_packet err:%d\n", ret);
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
	ret = screen_display_write_dsi_short_packet(&write_dsi_s);
	if (ret != SMAP_LIB_DISPLAY_OK) {
		dev_err(&data->bd->dev, "disp_write_dsi_short err: %d\n", ret);
		disp_delete.handle = screen_handle;
		screen_display_delete(&disp_delete);
		return -1;
	}
	disp_delete.handle = screen_handle;
	screen_display_delete(&disp_delete);

	return 0;
}

static int s6e39a0x02_gamma_ctl(struct s6e39a0x02 *data, int gamma)
{
	s6e39a0x02_update_gamma_ctl(data, gamma);
	return 0;
}

static int s6e39a0x02_get_brightness(struct backlight_device *bd)
{
	dev_dbg(&bd->dev, "brightness:%d\n", bd->props.brightness);
	return bd->props.brightness;
}

static int s6e39a0x02_set_brightness(struct backlight_device *bd)
{
	int ret = 0, brightness = bd->props.brightness;
	struct s6e39a0x02 *data = bl_get_data(bd);

	if (brightness < MIN_BRIGHTNESS ||
		brightness > bd->props.max_brightness) {
		dev_err(&bd->dev, "lcd brightness should be %d to %d.\n",
			MIN_BRIGHTNESS, MAX_BRIGHTNESS);
		return -EINVAL;
	}

	ret = s6e39a0x02_gamma_ctl(data, bd->props.brightness);
	if (ret) {
		dev_err(&bd->dev, "lcd brightness setting failed.\n");
		return -EIO;
	}

	return ret;
}

static const struct backlight_ops s6e39a0x02_backlight_ops  = {
	.get_brightness = s6e39a0x02_get_brightness,
	.update_status = s6e39a0x02_set_brightness,
};

static int __devinit s6e39a0x02_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct s6e39a0x02 *data = NULL;

	dev_dbg(&pdev->dev, "s6e39a0x02_dev prob start\n");

	data = kzalloc(sizeof(struct s6e39a0x02), GFP_KERNEL);

	data->bd = backlight_device_register("s6e39a0x02-bl",
		&pdev->dev, data, &s6e39a0x02_backlight_ops, NULL);
	if (IS_ERR(data->bd)) {
		dev_err(&pdev->dev, "backlight_device_register err\n");
		ret = PTR_ERR(data->bd);
		goto out_free_lcd;
	}
	platform_set_drvdata(pdev, data->bd);

	data->bd->props.max_brightness = MAX_BRIGHTNESS;
	data->bd->props.brightness = INIT_BRIGHTNESS;
	data->bd->props.type = BACKLIGHT_RAW;
	/* set default gamma table */
	memcpy(data->gamma_table, s6e39a0x02_22_gamma_table[0],
		GAMMA_TABLE_COUNT);

	dev_dbg(&pdev->dev, "s6e39a0x02 Backlight Driver Initialized\n");

	return 0;

out_free_lcd:
	kfree(data);
	return ret;
}

static int __devexit s6e39a0x02_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct s6e39a0x02 *data = bl_get_data(bl);
	backlight_device_unregister(bl);
	kfree(data);
	return 0;
}

#ifdef CONFIG_PM
/* static unsigned int before_power; */

static int s6e39a0x02_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	int ret = 0;

	dev_dbg(&pdev->dev, "s6e39a0x02_suspend\n");
/* TODO
	struct s6e39a0x02 *lcd = dev_get_drvdata(pdev);
	before_power = lcd->power;
	ret = s6e39a0x02_power(lcd, FB_BLANK_POWERDOWN);
*/
	return ret;
}

static int s6e39a0x02_resume(struct platform_device *pdev)
{
	int ret = 0;
	dev_dbg(&pdev->dev, "s6e39a0x02_resume\n");
/* TODO
	struct s6e39a0x02 *lcd = dev_get_drvdata(pdev);
	if (before_power == FB_BLANK_UNBLANK)
		lcd->power = FB_BLANK_POWERDOWN;
	ret = s6e39a0x02_power(lcd, before_power);
*/
	return ret;
}
#else
#define s6e39a0x02_suspend	NULL
#define s6e39a0x02_resume	NULL
#endif

static struct platform_driver s6e39a0x02_driver = {
	.driver = {
		.name	= "s6e39a0x02",
		.owner	= THIS_MODULE,
	},
	.probe		= s6e39a0x02_probe,
	.remove		= __devexit_p(s6e39a0x02_remove),
	.suspend	= s6e39a0x02_suspend,
	.resume		= s6e39a0x02_resume,
};

static int __init s6e39a0x02_init(void)
{
	return platform_driver_register(&s6e39a0x02_driver);
}

static void __exit s6e39a0x02_exit(void)
{
	platform_driver_unregister(&s6e39a0x02_driver);
}

module_init(s6e39a0x02_init);
module_exit(s6e39a0x02_exit);

MODULE_DESCRIPTION("s6e39a0x02 LCD Backlight Driver");
MODULE_AUTHOR("Renesas Electronics");
MODULE_LICENSE("GPL");
