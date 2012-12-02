/*
 * D2153 ALSA SoC codec driver
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Written by Adam Thomson <Adam.Thomson.Opensource@diasemi.com>
 * Based on DA9055 ALSA SoC codec driver.
 * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
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
#include <sound/soundpath/d2153_extern.h>
#include <sound/soundpath/soundpath.h>
#include <linux/d2153/core.h>

extern struct sndp_codec_info g_sndp_codec_info;
extern void d2153_set_soc_controls(
	struct snd_kcontrol_new *controls,
	u_int array_size);

static struct snd_soc_dai_link fsi_dai_link[] = {
	{
		.name		= "fsia d2153-codec",
		.stream_name	= "hifi",
		.cpu_dai_name	= "fsia-dai",
		.codec_dai_name	= "d2153-codec-hifi",
		.platform_name	= "sh_fsi2.0",
		.codec_name	= "d2153-codec.0-0018",
	},
	{
		.name		= "fsib d2153-codec",
		.stream_name	= "fm",
		.cpu_dai_name	= "fsib-dai",
		.codec_dai_name	= "d2153-codec-fm",
		.platform_name	= "sh_fsi2.1",
		.codec_name	= "d2153-codec.0-0018",
	},
};

static struct snd_soc_card fsi_soc_card = {
	.name = "FSI",
	.dai_link = fsi_dai_link,
	.num_links = ARRAY_SIZE(fsi_dai_link),
};

static struct platform_device *fsi_snd_device;

static int __init fsi_d2153_init(void)
{
	int ret = -ENOMEM;

	g_sndp_codec_info.set_device =	d2153_set_device;
	g_sndp_codec_info.get_device =	d2153_get_device;
	g_sndp_codec_info.set_volum =  d2153_set_volume;
	g_sndp_codec_info.get_volume = d2153_get_volume;
	g_sndp_codec_info.set_mute = d2153_set_mute;
	g_sndp_codec_info.get_mute = d2153_get_mute;
	g_sndp_codec_info.set_speaker_amp = d2153_set_speaker_amp;
	g_sndp_codec_info.set_soc_controls = d2153_set_soc_controls;

	g_sndp_codec_info.out_dev_all = (D2153_DEV_PLAYBACK_SPEAKER	|
					 D2153_DEV_PLAYBACK_EARPIECE |
					 D2153_DEV_PLAYBACK_HEADPHONES);
	g_sndp_codec_info.in_dev_all = (D2153_DEV_CAPTURE_MIC |
					D2153_DEV_CAPTURE_HEADSET_MIC);
	g_sndp_codec_info.dev_none = D2153_DEV_NONE;
	g_sndp_codec_info.dev_playback_speaker = D2153_DEV_PLAYBACK_SPEAKER;
	g_sndp_codec_info.dev_playback_earpiece = D2153_DEV_PLAYBACK_EARPIECE;
	g_sndp_codec_info.dev_playback_headphones = D2153_DEV_PLAYBACK_HEADPHONES;
	g_sndp_codec_info.dev_capture_mic = D2153_DEV_CAPTURE_MIC;
	g_sndp_codec_info.dev_capture_headset_mic = D2153_DEV_CAPTURE_HEADSET_MIC;

	g_sndp_codec_info.codec_valume = D2153_VOLUMEL5;
	g_sndp_codec_info.mute_enable = D2153_MUTE_ENABLE;
	g_sndp_codec_info.mute_disable = D2153_MUTE_DISABLE;
	g_sndp_codec_info.speaker_enable = D2153_SPEAKER_AMP_ENABLE;
	g_sndp_codec_info.speaker_disable = D2153_SPEAKER_AMP_DISABLE;

	g_sndp_codec_info.power_on = D2153_POWER_ON;
	g_sndp_codec_info.power_off = D2153_POWER_OFF;

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

static void __exit fsi_d2153_exit(void)
{
	sndp_exit();
	platform_device_unregister(fsi_snd_device);
}

module_init(fsi_d2153_init);
module_exit(fsi_d2153_exit);


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("");
MODULE_AUTHOR("");
