/*
 * fsi-d2153.c - FSI ASoC driver for boards using d2153 codec.
 *
 * Copyright (C) 2012-2013 Renesas Mobile Corp.
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
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <mach/common.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/fsi_d2153.h>
#include <linux/d2153/d2153_codec.h>

#include <sound/sh_fsi.h>
#include <sound/soundpath/soundpath.h>

#if 1 /*** Analog audio dock support ***/

#include <mach/pm.h>
#include <linux/gpio.h>
#include <linux/proc_fs.h>

/* DEFINE Definitions */
#define GPIO_DOCK_EN	GPIO_PORT33

#endif

/*
 * Marco Definition
 */
#define D2153_SNDP_MCLK_RATE	13000000

#define D2153_PLAYBACK_STREAM_NAME	"Playback"
#define D2153_CAPTURE_STREAM_NAME	"Capture"

struct clk *vclk4_clk;
struct clk *main_clk;
static int g_boot_flag;

static DEFINE_SPINLOCK(fsi_d2153_lock); /* Guards the ignore suspend */

static int vclk4_supply_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event);
static int post_playback_event(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *kcontrol, int event);

static void fsi_d2153_set_active(struct snd_soc_codec *codec,
	const char *stream, int active);

struct fsi_d2153_priv {
	struct sndp_workqueue *fsi_d2153_workqueue;
	struct sndp_work_info sync_work;
};

static struct snd_soc_codec *fsi_d2153_codec;
static struct snd_soc_pcm_runtime *fsi_d2153_rtd;
static struct snd_soc_dai_ops fsi_d2153_ops_save;

static void fsi_d2153_set_active(struct snd_soc_codec *codec,
	const char *stream, int active)
{
	struct snd_soc_dapm_widget *w;

	list_for_each_entry(w, &codec->card->widgets, list) {
		if (!w->sname)
			continue;
		if (strstr(w->sname, stream)) {
			w->active = active;
			dapm_mark_dirty(w, "fsi_d2153_set_active");
			sndp_log_info("w->name[%s] w->active[%d]\n",
				w->name, w->active);
			snd_soc_dapm_sync(&codec->dapm);
			continue;
		}
	}
}

void fsi_d2153_deactivate_input(struct snd_kcontrol *kcontrol)
{
	struct snd_soc_codec *codec;
	struct snd_card *card;

	if (!kcontrol) {
		sndp_log_err("kcontrol is NULL\n");
		return;
	}
	codec = (struct snd_soc_codec *)kcontrol->private_data;
	card = codec->card->snd_card;

	sndp_log_info("start\n");

	snd_soc_dapm_disable_pin(&codec->dapm, "RECCHL");
	snd_soc_dapm_disable_pin(&codec->dapm, "RECCHR");

	fsi_d2153_set_active(codec, D2153_CAPTURE_STREAM_NAME, 0);

	sndp_log_info("end\n");
	return;
}
EXPORT_SYMBOL(fsi_d2153_deactivate_input);

void fsi_d2153_deactivate_output(struct snd_kcontrol *kcontrol)
{
	/* Nothing to do.*/
	return;
}

void fsi_d2153_set_dac_power(struct snd_kcontrol *kcontrol,
	int status)
{
	struct snd_soc_codec *codec;
	struct snd_card *card;

	if (!kcontrol) {
		sndp_log_err("kcontrol is NULL\n");
		return;
	}
	codec = (struct snd_soc_codec *)kcontrol->private_data;
	card = codec->card->snd_card;

	sndp_log_info("start\n");

	fsi_d2153_set_active(codec, D2153_PLAYBACK_STREAM_NAME, status);

	sndp_log_info("end\n");
	return;
}
EXPORT_SYMBOL(fsi_d2153_set_dac_power);

int fsi_d2153_set_ignore_suspend(struct snd_soc_card *card,
	unsigned int dev_id, unsigned int status)
{
	struct snd_soc_dapm_widget *w;

	if (!card || dev_id > card->num_rtd)
		return -EINVAL;
	else {
		list_for_each_entry(w, &card->widgets, list) {
			switch (w->id) {
			case snd_soc_dapm_adc:
			case snd_soc_dapm_aif_out:
			case snd_soc_dapm_output:
			case snd_soc_dapm_hp:
			case snd_soc_dapm_spk:
			case snd_soc_dapm_line:
			case snd_soc_dapm_dac:
			case snd_soc_dapm_aif_in:
			case snd_soc_dapm_vmid:
			case snd_soc_dapm_mic:
			case snd_soc_dapm_input:
				w->ignore_suspend = status;
				continue;
			default:
				continue;
			}
		}
		spin_lock(&fsi_d2153_lock);
		card->rtd[dev_id].dai_link->ignore_suspend = status;
		spin_unlock(&fsi_d2153_lock);
		sndp_log_info("dev_id[%d], ignore_suspend[%d]\n",
			dev_id, status);
	}
	return 0;
}

int fsi_d2153_enable_ignore_suspend(struct snd_soc_card *card,
	unsigned int dev_id)
{
	return fsi_d2153_set_ignore_suspend(card, dev_id, 1);
}
EXPORT_SYMBOL(fsi_d2153_enable_ignore_suspend);

int fsi_d2153_disable_ignore_suspend(struct snd_soc_card *card,
	unsigned int dev_id)
{
	return fsi_d2153_set_ignore_suspend(card, dev_id, 0);
}
EXPORT_SYMBOL(fsi_d2153_disable_ignore_suspend);

/*
 * While PT loopback execution, we do not execute
 * cpu_dai->driver->ops->shutdown() and
 * codec_dai->driver->ops->hw_params() callbacks.
 * The reason is shown below.
 * 1. shutdown() calls deactivate_output() on device-change, so loopback
 *    sound is muted unexpectedly.
 * 2. hw_params() sets codec sample rate for playing music (such as 48kHz).
 *    Some codec requires sample rate setting though its setting is slave mode.
 *    So, we have to keep the voice rate (16kHz) while PT loopback.
 */
int fsi_d2153_set_sampling_rate(struct snd_pcm_hw_params *params)
{
	u8 fs;

	/* Set sampling rate */
	switch (params_rate(params)) {
	case 8000:
		fs = D2153_SR_8000;
		break;
	case 11025:
		fs = D2153_SR_11025;
		break;
	case 12000:
		fs = D2153_SR_12000;
		break;
	case 16000:
		fs = D2153_SR_16000;
		break;
	case 22050:
		fs = D2153_SR_22050;
		break;
	case 32000:
		fs = D2153_SR_32000;
		break;
	case 44100:
		fs = D2153_SR_44100;
		break;
	case 48000:
		fs = D2153_SR_48000;
		break;
	case 88200:
		fs = D2153_SR_88200;
		break;
	case 96000:
		fs = D2153_SR_96000;
		break;
	default:
		return -EINVAL;
	}

	snd_soc_write(fsi_d2153_codec, D2153_SR, fs);
	return 0;
}
EXPORT_SYMBOL(fsi_d2153_set_sampling_rate);

int fsi_d2153_loopback_notify(int status)
{
	int ret = 0;

	if (FSI_D2153_LOOPBACK_START == status) {
		fsi_d2153_ops_save.startup =
			fsi_d2153_rtd->cpu_dai->driver->ops->startup;
		fsi_d2153_rtd->cpu_dai->driver->ops->startup = NULL;

		fsi_d2153_ops_save.shutdown =
			fsi_d2153_rtd->cpu_dai->driver->ops->shutdown;
		fsi_d2153_rtd->cpu_dai->driver->ops->shutdown = NULL;

		fsi_d2153_ops_save.hw_params =
			fsi_d2153_rtd->codec_dai->driver->ops->hw_params;
		fsi_d2153_rtd->codec_dai->driver->ops->hw_params = NULL;

		fsi_d2153_ops_save.hw_free =
			fsi_d2153_rtd->codec_dai->driver->ops->hw_free;
		fsi_d2153_rtd->codec_dai->driver->ops->hw_free = NULL;
	} else if (FSI_D2153_LOOPBACK_STOP == status) {
		fsi_d2153_rtd->cpu_dai->driver->ops->startup =
					fsi_d2153_ops_save.startup;
		fsi_d2153_ops_save.startup = NULL;

		fsi_d2153_rtd->cpu_dai->driver->ops->shutdown =
					fsi_d2153_ops_save.shutdown;
		fsi_d2153_ops_save.shutdown = NULL;

		fsi_d2153_rtd->codec_dai->driver->ops->hw_params =
					fsi_d2153_ops_save.hw_params;
		fsi_d2153_ops_save.hw_params = NULL;

		fsi_d2153_rtd->codec_dai->driver->ops->hw_free =
					fsi_d2153_ops_save.hw_free;
		fsi_d2153_ops_save.hw_free = NULL;
	} else
		ret = -EINVAL;

	return ret;
}
EXPORT_SYMBOL(fsi_d2153_loopback_notify);

void fsi_d2153_soc_write(int dev)
{
	if (0 == dev) {
		snd_soc_write(fsi_d2153_codec, 0xc6, 0x98);
	} else {
/*		snd_soc_write(fsi_d2153_codec, 0xcd, 0x5F); */
/*		snd_soc_write(fsi_d2153_codec, 0xd0, 0x5F); */
	}
}
EXPORT_SYMBOL(fsi_d2153_soc_write);

#if 1 /*** Analog audio dock support ***/
/*!
   @brief PUT callback function for hooks control(Playback gpio setting)
   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data
   @retval	0		Successful
 */
int fsi_d2153_get_playback_gpio(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return ERROR_NONE;
}

/*!
   @brief PUT callback function for hooks control(Playback gpio setting)
   @param[-]	kcontrol	Not use
   @param[in]	ucontrol	Element data
   @retval	0		Successful
 */
int fsi_d2153_put_playback_gpio(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int state_gpio = 0;

	gpio_set_value(GPIO_DOCK_EN, ucontrol->value.enumerated.item[0]);
	/*** test code start ***/
	state_gpio = gpio_get_value(GPIO_DOCK_EN);
	printk(KERN_INFO "%s gpio_get_value(GPIO_DOCK_EN):%d\n",
		__func__, state_gpio);
	/*** test code end ***/
	return ERROR_NONE;
}
#endif

int fsi_d2153_sndp_soc_info(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_info *uinfo)
{
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 1;

	return 0;
}

int fsi_d2153_sndp_soc_get(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

int fsi_d2153_sndp_soc_put(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec;
	codec = (struct snd_soc_codec *)kcontrol->private_data;

	return sndp_soc_put(kcontrol, ucontrol);
}

int fsi_d2153_snd_soc_get_adc(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

int fsi_d2153_snd_soc_put_dac(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec =
		(struct snd_soc_codec *)kcontrol->private_data;
	unsigned int val;

	val = ucontrol->value.integer.value[0];
	fsi_d2153_set_active(codec, D2153_PLAYBACK_STREAM_NAME, val);

	return 0;
}

int fsi_d2153_snd_soc_get_dac(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

int fsi_d2153_snd_soc_put_adc(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec =
		(struct snd_soc_codec *)kcontrol->private_data;
	unsigned int val;

	val = ucontrol->value.integer.value[0];
	fsi_d2153_set_active(codec, D2153_CAPTURE_STREAM_NAME, val);

	if (!val) {
		sndp_log_info("Disable RECCH\n");
		snd_soc_dapm_disable_pin(&codec->dapm, "RECCHL");
		snd_soc_dapm_disable_pin(&codec->dapm, "RECCHR");
	} else {
		sndp_log_info("Enable RECCH\n");
		snd_soc_dapm_enable_pin(&codec->dapm, "RECCHL");
		snd_soc_dapm_enable_pin(&codec->dapm, "RECCHR");
	}
	snd_soc_dapm_sync(&codec->dapm);
	return 0;
}

static int fsi_d2153_sndp_soc_get_voice_out_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return sndp_soc_get_voice_out_volume(kcontrol, ucontrol);
}

static int fsi_d2153_sndp_soc_put_voice_out_volume(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return sndp_soc_put_voice_out_volume(kcontrol, ucontrol);
}

static int fsi_d2153_sndp_soc_get_playback_mute(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return sndp_soc_get_playback_mute(kcontrol, ucontrol);
}

static int fsi_d2153_sndp_soc_put_playback_mute(
	struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	return sndp_soc_put_playback_mute(kcontrol, ucontrol);
}


static struct snd_kcontrol_new fsi_d2153_controls[] = {
	SOC_SINGLE_BOOL_EXT("ADC Activate", 0,
		fsi_d2153_snd_soc_get_adc, fsi_d2153_snd_soc_put_adc),
	FSI_SOC_SINGLE_EXT("Path", 0, fsi_d2153_sndp_soc_info,
		fsi_d2153_sndp_soc_get, fsi_d2153_sndp_soc_put),
	FSI_SOC_SINGLE("Earpiece Volume" , 0, 0, 25, 0,
		fsi_d2153_sndp_soc_get_voice_out_volume,
		fsi_d2153_sndp_soc_put_voice_out_volume),
	FSI_SOC_SINGLE("Earpiece Switch" , 0, 0, 1,  0,
		fsi_d2153_sndp_soc_get_playback_mute,
		fsi_d2153_sndp_soc_put_playback_mute),
	SOC_SINGLE_BOOL_EXT("DAC Activate", 0,
		fsi_d2153_snd_soc_get_dac, fsi_d2153_snd_soc_put_dac),
#if 1 /*** Analog audio dock support ***/
	SOC_SINGLE_BOOL_EXT("Dock Switch" , 0,
		fsi_d2153_get_playback_gpio, fsi_d2153_put_playback_gpio),
#endif
};

static const struct snd_soc_dapm_widget fsi_d2153_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("RECCHL"), /* Dummy widget */
	SND_SOC_DAPM_OUTPUT("RECCHR"), /* Dummy widget */
	SND_SOC_DAPM_POST("Post Playback", post_playback_event),
	SND_SOC_DAPM_SUPPLY("VCLK4", SND_SOC_NOPM, 0, 0, vclk4_supply_event,
		SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD),

};

static const struct snd_soc_dapm_route fsi_d2153_audio_map[] = {
	{"RECCHL", NULL, "AIFOUTL"},
	{"RECCHR", NULL, "AIFOUTR"},
	{"AIFINL", NULL, "VCLK4"},
	{"AIFINR", NULL, "VCLK4"},
	{"AIFOUTL", NULL, "VCLK4"},
	{"AIFOUTR", NULL, "VCLK4"},
};

static int vclk4_supply_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = clk_enable(vclk4_clk);
		sndp_log_info("VCLKCR4[0x%x] ret[%d]\n",
			__raw_readl(0xE615001C), ret);
		break;
	case SND_SOC_DAPM_POST_PMD:
		clk_disable(vclk4_clk);
		sndp_log_info("VCLKCR4[0x%x]\n",
			__raw_readl(0xE615001C));
		break;
	}
	return 0;
}

static void fsi_d2153_sync_work_func(struct sndp_work_info *work)
{
	snd_soc_dapm_sync(&fsi_d2153_codec->dapm);
}

static int post_playback_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_dapm_widget *widget;
	struct snd_soc_codec *codec = w->codec;
	struct fsi_d2153_priv *priv =
			snd_soc_card_get_drvdata(codec->card);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		if (!g_boot_flag) {
			g_boot_flag = 1;
			/* Force disable DACL/R */
			list_for_each_entry(widget, &codec->card->widgets, list) {
				if (!widget->sname)
					continue;
				if (strstr(widget->sname, D2153_PLAYBACK_STREAM_NAME)) {
					widget->active = 0;
					dapm_mark_dirty(widget,
						"Force disable DAC on post_playback_event");
					sndp_log_info("w->name[%s] w->active[%d] w->power[%d]\n",
						widget->name, widget->active, widget->power);
					continue;
				}
			}
			sndp_workqueue_enqueue(priv->fsi_d2153_workqueue,
				&priv->sync_work);
		}
		break;
	default:
		break;
	}
	return 0;
}

static int fsi_hifi_d2153_pcm_prepare(struct snd_pcm_substream *substream)
{
	int ret = 0;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_widget *w;

	if (!g_boot_flag) {
		/* Force disable DACL/R */
		list_for_each_entry(w, &codec->card->widgets, list) {
			if (w) {
				if (!strcmp("Post Playback", w->name)) {
					dapm_mark_dirty(w,
						"Force disable DAC on fsi_hifi_d2153_pcm_prepare");
					return ret;
				}
			}
		}
	}
	return ret;
}

static int fsi_hifi_d2153_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	unsigned int pll_out = 16000;
	int ret = 0;

	/* set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S |
		SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0)
		return ret;

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S |
		SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	snd_soc_dai_set_sysclk(codec_dai, 0,
		D2153_SNDP_MCLK_RATE, SND_SOC_CLOCK_IN);

	switch (params_rate(params)) {
	case 8000:
	case 12000:
	case 16000:
	case 24000:
	case 32000:
	case 48000:
	case 96000:
		pll_out = D2153_PLL_FREQ_OUT_98304000;
		break;
	case 11025:
	case 22050:
	case 44100:
	case 88100:
		pll_out = D2153_PLL_FREQ_OUT_90316800;
		break;
	default:
		pr_err("Invalid sampling rate for D2153 with PLL\n");
		return -EINVAL;
	}
	snd_soc_dai_set_pll(codec_dai, 0, D2153_SYSCLK_PLL, 48000, pll_out);

	return 0;
}

static struct snd_soc_ops fsi_hifi_d2153_ops = {
	.hw_params = fsi_hifi_d2153_hw_params,
	.prepare = fsi_hifi_d2153_pcm_prepare,
};

static int fsi_hifi_d2153_init(struct snd_soc_pcm_runtime *rtd)
{
	int ret;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;

	fsi_d2153_codec = codec;
	fsi_d2153_rtd = codec->card->rtd;

	ret = snd_soc_add_codec_controls(codec, fsi_d2153_controls,
				   ARRAY_SIZE(fsi_d2153_controls));
	if (ret < 0) {
		pr_err("Failed to snd_soc_add_codec_controls(%d)\n", ret);
		return ret;
	}

	snd_soc_dapm_new_controls(dapm, fsi_d2153_dapm_widgets,
		ARRAY_SIZE(fsi_d2153_dapm_widgets));
	snd_soc_dapm_add_routes(dapm, fsi_d2153_audio_map,
		ARRAY_SIZE(fsi_d2153_audio_map));

	snd_soc_dapm_disable_pin(dapm, "RECCHL");
	snd_soc_dapm_disable_pin(dapm, "RECCHR");

	return 0;
}

static struct snd_soc_dai_link fsi_dai_link[] = {
	{
		.name = "fsia d2153",
		.stream_name = "Hifi",
		.cpu_dai_name	= "fsia-dai",
		.codec_name = "d2153-codec.0-0018",
		.platform_name = "sh_fsi2.0",
		.codec_dai_name = "d2153-aif1",
		.init = fsi_hifi_d2153_init,
		.ops = &fsi_hifi_d2153_ops,
	},
	{
		.name = "fsib Wireless Transciever",
		.stream_name = "Wireless Transciever",
		.cpu_dai_name	= "fsib-dai",
		.codec_name = "sh_fsi_wireless_transciever.0",
		.platform_name = "sh_fsi2.1",
		.codec_dai_name = "sh_fsi_wireless_transciever",
	},
};

static struct snd_soc_card fsi_soc_card = {
	.name = "FSI",
	.dai_link = fsi_dai_link,
	.num_links = ARRAY_SIZE(fsi_dai_link),
};

static __devinit int fsi_d2153_driver_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &fsi_soc_card;
	struct fsi_d2153_priv *priv = NULL;
	int ret = -ENOMEM;
	unsigned int mclk;

	priv = kzalloc(sizeof(struct fsi_d2153_priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		sndp_log_err("cannot allocate fsi_d2153 private data\n");
		return ret;
	}

	priv->fsi_d2153_workqueue =
		sndp_workqueue_create("fsi_d2153 workqueue");
	if (!priv->fsi_d2153_workqueue) {
		ret = -ENOMEM;
		sndp_log_err("Failed to create workqueue\n");
		goto err_create_singlethread_workqueue;
	}
	sndp_work_initialize(&priv->sync_work, fsi_d2153_sync_work_func);

	vclk4_clk = clk_get(NULL, "vclk4_clk");
	if (IS_ERR(vclk4_clk)) {
		ret = IS_ERR(vclk4_clk);
		sndp_log_err("cannot get vclk4 clock\n");
		goto err_vclk4_clk;
	}
	main_clk = clk_get(NULL, "main_clk");
	if (IS_ERR(main_clk)) {
		ret = IS_ERR(main_clk);
		sndp_log_err("cannot get main clock\n");
		goto err_main_clk;
	}
	ret = clk_set_parent(vclk4_clk, main_clk);
	if (0 != ret) {
		sndp_log_err("clk_set_parent failed (%d)\n", ret);
		goto err_clk_set_parent;
	}
	mclk = 13000000;

	ret = clk_set_rate(vclk4_clk, mclk);
	if (ret < 0) {
		sndp_log_err("cannot set vclk4 rate\n");
		goto err_clk_set_rate;
	}

	ret = sndp_init(fsi_soc_dai, &fsi_soc_platform, card);
	if (ret) {
		sndp_log_err("sndp_init failed (%d)\n", ret);
		goto err_sndp_init;
	}

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, priv);

	ret = snd_soc_register_card(card);
	if (ret) {
		sndp_log_err("snd_soc_register_card failed (%d)\n", ret);
		goto err_snd_soc_register_card;
	}
	return 0;

err_snd_soc_register_card:
err_sndp_init:
err_clk_set_rate:
err_clk_set_parent:
	clk_put(main_clk);
err_main_clk:
	clk_put(vclk4_clk);
err_vclk4_clk:
	sndp_workqueue_destroy(priv->fsi_d2153_workqueue);
err_create_singlethread_workqueue:
	kfree(priv);
	return ret;
}

static int __devexit fsi_d2153_driver_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct fsi_d2153_priv *priv =
			snd_soc_card_get_drvdata(card);

	fsi_d2153_codec = NULL;
	fsi_d2153_rtd = NULL;
	fsi_d2153_ops_save.startup = NULL;
	fsi_d2153_ops_save.shutdown = NULL;
	fsi_d2153_ops_save.hw_params = NULL;
	fsi_d2153_ops_save.hw_free = NULL;
	sndp_workqueue_destroy(priv->fsi_d2153_workqueue);
	kfree(priv);
	clk_put(main_clk);
	clk_put(vclk4_clk);
	return 0;
}

static struct platform_driver fsi_d2153_driver = {
	.driver = {
		.name = "fsi-snd-d2153",
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
	},
	.probe = fsi_d2153_driver_probe,
	.remove = __devexit_p(fsi_d2153_driver_remove),
};

static int __init fsi_d2153_modinit(void)
{
	g_boot_flag = 0;
	return platform_driver_register(&fsi_d2153_driver);
}
module_init(fsi_d2153_modinit);

static void __exit fsi_d2153_modexit(void)
{
	sndp_exit();
	platform_driver_unregister(&fsi_d2153_driver);
}
module_exit(fsi_d2153_modexit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Fsi+d2153 machine ASoC driver");
MODULE_AUTHOR("Renesas Mobile Corp");
