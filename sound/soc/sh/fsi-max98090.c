/*
 * FSI - MAX98090 sound support
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
#include <sound/soundpath/max98090_extern.h>
#include <sound/soundpath/soundpath.h>

extern struct sndp_codec_info g_sndp_codec_info;
extern void max98090_set_soc_controls(
	struct snd_kcontrol_new *controls,
	u_int array_size);

static struct snd_soc_dai_link fsi_dai_link[] = {
	{
		.name		= "fsia max98090",
		.stream_name	= "max98090 hifi",
		.cpu_dai_name	= "fsia-dai",
		.codec_dai_name	= "max98090-hifi",
		.platform_name	= "sh_fsi2.0",
		.codec_name	= "max98090.6-0010",
	},
	{
		.name		= "fsib max98090",
		.stream_name	= "max98090 fm",
		.cpu_dai_name	= "fsib-dai",
		.codec_dai_name	= "max98090-fm",
		.platform_name	= "sh_fsi2.1",
		.codec_name	= "max98090.6-0010",
	},
};

static struct snd_soc_card fsi_soc_card = {
	.name = "FSI",
	.dai_link = fsi_dai_link,
	.num_links = ARRAY_SIZE(fsi_dai_link),
};

static struct platform_device *fsi_snd_device;

static int __init fsi_max98090_init(void)
{
	int ret = -ENOMEM;

	/* Set AudioLsi information */
	g_sndp_codec_info.set_device =	max98090_set_device;
	g_sndp_codec_info.get_device =	max98090_get_device;
	g_sndp_codec_info.set_volum =  max98090_set_volume;
	g_sndp_codec_info.get_volume = max98090_get_volume;
	g_sndp_codec_info.set_mute = max98090_set_mute;
	g_sndp_codec_info.get_mute = max98090_get_mute;
	g_sndp_codec_info.set_speaker_amp = max98090_set_speaker_amp;
	g_sndp_codec_info.set_soc_controls = max98090_set_soc_controls;

	g_sndp_codec_info.out_dev_all = (MAX98090_DEV_PLAYBACK_SPEAKER	|
					 MAX98090_DEV_PLAYBACK_EARPIECE |
					 MAX98090_DEV_PLAYBACK_HEADPHONES);
	g_sndp_codec_info.in_dev_all = (MAX98090_DEV_CAPTURE_MIC |
					MAX98090_DEV_CAPTURE_HEADSET_MIC);
	g_sndp_codec_info.dev_none = MAX98090_DEV_NONE;
	g_sndp_codec_info.dev_playback_speaker = MAX98090_DEV_PLAYBACK_SPEAKER;
	g_sndp_codec_info.dev_playback_earpiece =
					MAX98090_DEV_PLAYBACK_EARPIECE;
	g_sndp_codec_info.dev_playback_headphones =
					MAX98090_DEV_PLAYBACK_HEADPHONES;
	g_sndp_codec_info.dev_capture_mic = MAX98090_DEV_CAPTURE_MIC;
	g_sndp_codec_info.dev_capture_headset_mic =
					MAX98090_DEV_CAPTURE_HEADSET_MIC;

	g_sndp_codec_info.codec_valume = MAX98090_VOLUMEL5;
	g_sndp_codec_info.mute_enable = MAX98090_MUTE_ENABLE;
	g_sndp_codec_info.mute_disable = MAX98090_MUTE_DISABLE;
	g_sndp_codec_info.speaker_enable = MAX98090_SPEAKER_AMP_ENABLE;
	g_sndp_codec_info.speaker_disable = MAX98090_SPEAKER_AMP_DISABLE;

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

static void __exit fsi_max98090_exit(void)
{
/*	sndp_exit(); */
	platform_device_unregister(fsi_snd_device);
}

module_init(fsi_max98090_init);
module_exit(fsi_max98090_exit);


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("");
MODULE_AUTHOR("");
