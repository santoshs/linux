/*
 * FSI - WM18110 sound support
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <sound/sh_fsi.h>
#include <sound/soundpath/wm1811_extern.h>
#include <sound/soundpath/soundpath.h>

extern struct sndp_codec_info g_sndp_codec_info;
extern void wm1811_set_soc_controls(
	struct snd_kcontrol_new *controls,
	u_int array_size);

static struct snd_soc_dai_link fsi_dai_link[] = {
	{
		.name		= "fsia wm1811",
		.stream_name	= "hifi",
		.cpu_dai_name	= "fsia-dai",
		.codec_dai_name	= "wm1811-hifi",
		.platform_name	= "sh_fsi2.0",
		.codec_name	= "wm1811.6-001a",
	},
	{
		.name		= "fsib wm1811",
		.stream_name	= "fm",
		.cpu_dai_name	= "fsib-dai",
		.codec_dai_name	= "wm1811-fm",
		.platform_name	= "sh_fsi2.1",
		.codec_name	= "wm1811.6-001a",
	},
};

static struct snd_soc_card fsi_soc_card = {
	.name = "FSI",
	.dai_link = fsi_dai_link,
	.num_links = ARRAY_SIZE(fsi_dai_link),
};

static struct platform_device *fsi_snd_device;

static int __init fsi_wm1811_init(void)
{
	int ret = -ENOMEM;

	g_sndp_codec_info.set_device =	wm1811_set_device;
	g_sndp_codec_info.get_device =	wm1811_get_device;
	g_sndp_codec_info.set_volum =  wm1811_set_volume;
	g_sndp_codec_info.get_volume = wm1811_get_volume;
	g_sndp_codec_info.set_mute = wm1811_set_mute;
	g_sndp_codec_info.get_mute = wm1811_get_mute;
	g_sndp_codec_info.set_speaker_amp = wm1811_set_speaker_amp;
	g_sndp_codec_info.set_soc_controls = wm1811_set_soc_controls;

	g_sndp_codec_info.out_dev_all = (WM1811_DEV_PLAYBACK_SPEAKER	|
					 WM1811_DEV_PLAYBACK_EARPIECE |
					 WM1811_DEV_PLAYBACK_HEADPHONES);
	g_sndp_codec_info.in_dev_all = (WM1811_DEV_CAPTURE_MIC |
					WM1811_DEV_CAPTURE_HEADSET_MIC);
	g_sndp_codec_info.dev_none = WM1811_DEV_NONE;
	g_sndp_codec_info.dev_playback_speaker = WM1811_DEV_PLAYBACK_SPEAKER;
	g_sndp_codec_info.dev_playback_earpiece = WM1811_DEV_PLAYBACK_EARPIECE;
	g_sndp_codec_info.dev_playback_headphones = WM1811_DEV_PLAYBACK_HEADPHONES;
	g_sndp_codec_info.dev_capture_mic = WM1811_DEV_CAPTURE_MIC;
	g_sndp_codec_info.dev_capture_headset_mic = WM1811_DEV_CAPTURE_HEADSET_MIC;

	g_sndp_codec_info.codec_valume = WM1811_VOLUMEL5;
	g_sndp_codec_info.mute_enable = WM1811_MUTE_ENABLE;
	g_sndp_codec_info.mute_disable = WM1811_MUTE_DISABLE;
	g_sndp_codec_info.speaker_enable = WM1811_SPEAKER_AMP_ENABLE;
	g_sndp_codec_info.speaker_disable = WM1811_SPEAKER_AMP_DISABLE;

	ret = sndp_init(fsi_soc_dai, &fsi_soc_platform);
	if (ret)
		goto out;

	fsi_snd_device = platform_device_alloc("soc-audio", -1);
	if (!fsi_snd_device)
		goto out;

	platform_set_drvdata(fsi_snd_device, &fsi_soc_card);
	ret = platform_device_add(fsi_snd_device);

	if (ret)
		platform_device_put(fsi_snd_device);

out:
	return ret;

}

static void __exit fsi_wm1811_exit(void)
{
/*	sndp_exit(); */
	platform_device_unregister(fsi_snd_device);
}

module_init(fsi_wm1811_init);
module_exit(fsi_wm1811_exit);


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("");
MODULE_AUTHOR("");
