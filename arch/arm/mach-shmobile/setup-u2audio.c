/* setup-u2audio.c
 *
 * Copyright (C) 2013 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/proc_fs.h>
#include <linux/gpio.h>
#if defined(CONFIG_ARCH_R8A7373)
#include <mach/r8a7373.h>
#endif
#include <mach/setup-u2audio.h>
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/d2153_codec.h>
#endif

/* Proc root entries */
struct proc_dir_entry *root_audio;
struct proc_dir_entry *root_device;
struct proc_dir_entry *root_input;

/* Proc sub entries */
/* device entries */
struct proc_dir_entry *a2220_entry;
struct proc_dir_entry *fm34_entry;
struct proc_dir_entry *tpa2026_entry;

/* Input/Output entries */
struct proc_dir_entry *sub_mic_entry;

/* Proc read handler */
static int proc_read_u2audio_device_none(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	count = snprintf(page, count, "%d", 0);
	return count;
}
static int proc_read_u2audio_device_exist(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	count = snprintf(page, count, "%d", 1);
	return count;
}

void u2audio_gpio_init()
{
#if defined(CONFIG_ARCH_R8A7373)
	gpio_request(GPIO_FN_FSIAISLD, "sound");
	gpio_request(GPIO_FN_FSIAOBT, "sound");
	gpio_request(GPIO_FN_FSIAOLR, "sound");
	gpio_request(GPIO_FN_FSIAOSLD, "sound");

	gpio_request(GPIO_FN_FSIBISLD, "sound");
	gpio_request(GPIO_FN_FSIBOBT, "sound");
	gpio_request(GPIO_FN_FSIBOLR, "sound");
	gpio_request(GPIO_FN_FSIBOSLD, "sound");
#endif /* CONFIG_ARCH_R8A7373 */
}

void u2audio_codec_micbias_level_init(unsigned int u2_board_rev)
{
	u8 micbias1 = D2153_MICBIAS_LEVEL_2_6V;

#if defined(CONFIG_MACH_LOGANLTE)
	if (0 == u2_board_rev)
#elif defined(CONFIG_MACH_LT02LTE)
	if (0 == u2_board_rev)
#endif
		micbias1 = D2153_MICBIAS_LEVEL_2_5V;

	/* set headset mic bias */
	d2153_pdata.audio.micbias1_level = micbias1;
	/* set sub mic bias */
	d2153_pdata.audio.micbias2_level = D2153_MICBIAS_LEVEL_2_1V;
	/* set main mic bias */
	d2153_pdata.audio.micbias3_level = D2153_MICBIAS_LEVEL_2_1V;
}

void u2audio_init(unsigned int u2_board_rev)
{
	u8 a2220_device;
	u8 fm34_device;
	u8 tpa2026_device;
	u8 sub_mic;

	u2audio_gpio_init();

#if defined(CONFIG_MACH_GARDALTE)
	if (u2_board_rev < 2) {
		a2220_device = DEVICE_EXIST;
		fm34_device = DEVICE_NONE;
	} else {
		a2220_device = DEVICE_NONE;
		fm34_device = DEVICE_EXIST;
	}
	tpa2026_device = DEVICE_NONE;
	sub_mic = DEVICE_EXIST;
#elif defined(CONFIG_MACH_LOGANLTE)
	if (u2_board_rev < 1) {
		fm34_device = DEVICE_EXIST;
		sub_mic = DEVICE_EXIST;
	} else {
		fm34_device = DEVICE_NONE;
		sub_mic = DEVICE_NONE;
	}
	a2220_device = DEVICE_NONE;
	tpa2026_device = DEVICE_NONE;
#elif defined(CONFIG_MACH_LT02LTE)
	a2220_device = DEVICE_NONE;
	fm34_device = DEVICE_NONE;
	tpa2026_device = DEVICE_EXIST;
	sub_mic = DEVICE_EXIST;
#else
	a2220_device = DEVICE_NONE;
	fm34_device = DEVICE_NONE;
	tpa2026_device = DEVICE_NONE;
	sub_mic = DEVICE_NONE;
#endif

	root_audio = proc_mkdir("audio", NULL);
	if (NULL != root_audio) {
		/* Create device entries */
		root_device = proc_mkdir("device", root_audio);
		if (NULL != root_device) {
#if defined(CONFIG_MACH_GARDALTE)
			a2220_entry = create_proc_entry("a2220",
				S_IRUGO, root_device);
			if (NULL != a2220_entry) {
				if (!a2220_device)
					a2220_entry->read_proc =
						proc_read_u2audio_device_none;
				else
					a2220_entry->read_proc =
						proc_read_u2audio_device_exist;
			} else {
				printk(KERN_ERR "%s Failed create_proc_entry a2220\n",
					__func__);
			}
#endif /* CONFIG_MACH_GARDALTE */
#if defined(CONFIG_MACH_GARDALTE) || defined(CONFIG_MACH_LOGANLTE)
			fm34_entry = create_proc_entry("fm34",
				S_IRUGO, root_device);
			if (NULL != fm34_entry) {
				if (!fm34_device)
					fm34_entry->read_proc =
						proc_read_u2audio_device_none;
				else
					fm34_entry->read_proc =
						proc_read_u2audio_device_exist;
			} else {
				printk(KERN_ERR "%s Failed create_proc_entry fm34\n",
					__func__);
			}
#endif /* CONFIG_MACH_GARDALTE || CONFIG_MACH_LOGANLTE*/
#if defined(CONFIG_MACH_LT02LTE)
			tpa2026_entry = create_proc_entry("tpa2026",
				S_IRUGO, root_device);
			if (NULL != tpa2026_entry) {
				if (!tpa2026_device)
					tpa2026_entry->read_proc =
						proc_read_u2audio_device_none;
				else
					tpa2026_entry->read_proc =
						proc_read_u2audio_device_exist;
			} else {
				printk(KERN_ERR "%s Failed create_proc_entry tpa2026\n",
					__func__);
			}
#endif /* CONFIG_MACH_LT02LTE */
		}
		/* Create input/output entries */
		root_input = proc_mkdir("input", root_audio);
		if (NULL != root_input) {
			sub_mic_entry = create_proc_entry("sub_mic",
				S_IRUGO, root_input);
			if (NULL != sub_mic_entry) {
				if (!sub_mic)
					sub_mic_entry->read_proc =
						proc_read_u2audio_device_none;
				else
					sub_mic_entry->read_proc =
						proc_read_u2audio_device_exist;
			} else {
				printk(KERN_ERR "%s Failed create_proc_entry sub_mic\n",
					__func__);
			}
		}
	} else {
		printk(KERN_ERR "%s Failed proc_mkdir\n", __func__);
	}

	u2audio_codec_micbias_level_init(u2_board_rev);

	return;
}
