/*
 * D2153 Soundpath Codec Wrapper
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Written by Adam Thomson <Adam.Thomson.Opensource@diasemi.com>
  * 
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/wait.h>
#include <linux/clk.h>
#include <sound/asound.h>
#include <sound/control.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <sound/soundpath/soundpath.h>
#include <sound/soundpath/d2153_extern.h>
#include <linux/delay.h>

//#define CONFIG_SND_SOC_USE_DA9055_HW

#include "d2153_sndp_wrapper.h"
#ifdef CONFIG_SND_SOC_USE_DA9055_HW
#include "d2153_da9055.h"
#else
#include <linux/d2153/d2153_codec.h>
#endif /* CONFIG_SND_SOC_USE_DA9055_HW */
#ifdef CONFIG_SND_SOC_D2153_AAD
#include <linux/d2153/d2153_aad.h>
#endif /* CONFIG_SND_SOC_D2153_AAD */

#define D2153_SNDP_MCLK_RATE	26000000

/*
 * Codec controls
 */
static struct snd_kcontrol *d2153_sndp_list_kcontrol(struct snd_card *card,
							 char *name)
{
	int i=0;
	struct snd_kcontrol *kctl;
	
	list_for_each_entry(kctl, &card->controls, list) {
		dlg_info("%s Kcontrol name %d ->  %s \n",__func__,i++,kctl->id.name);
	}

	return NULL;
}

static struct snd_kcontrol *d2153_sndp_find_kcontrol(struct snd_card *card,
						     char *name)
{
	struct snd_kcontrol *kctl;
	
	list_for_each_entry(kctl, &card->controls, list) {
		if (strcmp(kctl->id.name, name) == 0)
			return kctl;
	}

	return NULL;
}

/* Set single value type Kcontrol */
static int d2153_sndp_set_kcontrol_single(struct snd_soc_codec *codec,
					  char *name, int val)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value ucontrol;
	int ret;

	/* Find a matching control for given name */
	kctl = d2153_sndp_find_kcontrol(codec->card->snd_card, name);
	if (!kctl) {
		dev_err(codec->dev, "Could not find control %s\n", name);
		return -EINVAL;
	}

	/* Get info on Kcontrol */
	ret = kctl->info(kctl, &uinfo);
	if (ret < 0) {
		dev_err(codec->dev, "Could not retrieve control info for %s\n",
			name);
		return -EINVAL;
	}

	/* Check we're setting a SINGLE type control */
	if ((uinfo.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED) || (uinfo.count != 1)) {
		dev_err(codec->dev, "Control %s is incorrect type\n", name);
		return -EINVAL;
	}

	/* Check value in valid range */
	if ((val < uinfo.value.integer.min) || (val > uinfo.value.integer.max)) {
		dev_err(codec->dev, "Value out of range for control %s\n",
			name);
		return -EINVAL;
	}
	
	/* Set value */
	ucontrol.value.integer.value[0] = val;
	return kctl->put(kctl, &ucontrol);
}

/* Set double value type Kcontrol */
static int d2153_sndp_set_kcontrol_double(struct snd_soc_codec *codec,
					  char *name, int lval, int rval)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value ucontrol;
	int ret;

	/* Find a matching control for given name */
	kctl = d2153_sndp_find_kcontrol(codec->card->snd_card, name);
	if (!kctl) {
		dev_err(codec->dev, "Could not find control %s\n", name);
		return -EINVAL;
	}

	/* Get info on Kcontrol */
	ret = kctl->info(kctl, &uinfo);
	if (ret < 0) {
		dev_err(codec->dev, "Could not retrieve control info for %s\n",
			name);
		return -EINVAL;
	}

	/* Check we're setting a DOUBLE type control */
	if ((uinfo.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED) || (uinfo.count != 2)) {
		dev_err(codec->dev, "Control %s is incorrect type\n", name);
		return -EINVAL;
	}

	/* Check values in valid range */
	if ((lval < uinfo.value.integer.min) || (lval > uinfo.value.integer.max)) {
		dev_err(codec->dev, "Left value out of range for control %s\n",
			name);
		return -EINVAL;
	}
	if ((rval < uinfo.value.integer.min) || (rval > uinfo.value.integer.max)) {
		dev_err(codec->dev, "Right value out of range for control %s\n",
			name);
		return -EINVAL;
	}

	/* Set value */
	ucontrol.value.integer.value[0] = lval;
	ucontrol.value.integer.value[1] = rval;
	return kctl->put(kctl, &ucontrol);
}


/*
 * DAPM Widgets
 */

/* Find named widget */
static struct snd_soc_dapm_widget
	*d2153_sndp_find_dapm_widget(const struct snd_soc_card *card,
				     char *name)
{
	const struct snd_soc_dapm_widget *widget;
	
	/* Find the widget */
	list_for_each_entry(widget, &card->widgets, list) {
		if (strcmp(widget->name, name) == 0)
			return (struct snd_soc_dapm_widget *)widget;
	}
	
	return NULL;
}

/* DLG - Don't think we really need this, but have left it in just in case */
/* Find name control of widget */
static struct snd_kcontrol
	*d2153_sndp_find_dapm_kcontrol(struct snd_soc_dapm_widget *widget,
				       char *name)
{	
	int i;

	for (i = 0; i < widget->num_kcontrols; ++i) {
		if (strcmp(widget->kcontrols[i]->id.name, name) == 0)
			return widget->kcontrols[i];
	}

	return NULL;
}

/* Set register assigned to widget without Kcontrols */
static int d2153_sndp_set_dapm_widget(struct snd_soc_codec *codec,
				      char *name, int enable)
{
	struct snd_soc_dapm_widget *widget;
	unsigned int mask = 0;
	unsigned int value = 0;
	
	widget = d2153_sndp_find_dapm_widget(codec->card, name);
	if (!widget) {
		dev_err(codec->dev, "Could not find widget %s\n", name);
		return -EINVAL;
	}

	value = (enable ? 1 : 0);
	mask = 1 << widget->shift;
	if (widget->invert)
		value = 1 - value;
	widget->value = value;

	return snd_soc_update_bits_locked(codec, widget->reg, mask,
					  (value << widget->shift));
}

/* Set widget single value Kcontrol */
static int d2153_sndp_set_dapm_kcontrol_single(struct snd_soc_codec *codec,
					       char *name, int val)
{
	struct snd_kcontrol *kctl;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value ucontrol;
	int ret;
	
	/* Find control to call */
	kctl = d2153_sndp_find_kcontrol(codec->card->snd_card, name);
	if (!kctl) {
		dev_err(codec->dev, "Could not find control %s\n", name);
		return -EINVAL;
	}

	/* Get info on Kcontrol */
	ret = kctl->info(kctl, &uinfo);
	if (ret < 0) {
		dev_err(codec->dev, "Could not retrieve control info for %s\n",
			name);
		return -EINVAL;
	}

	/* Check we're setting a SINGLE type control */
	if ((uinfo.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED) || (uinfo.count != 1)) {
		dev_err(codec->dev, "Control %s is incorrect type\n", name);
		return -EINVAL;
	}

	/* Check value in valid range */
	if ((val < uinfo.value.integer.min) || (val > uinfo.value.integer.max)) {
		dev_err(codec->dev, "Value out of range for control %s\n",
			name);
		return -EINVAL;
	}

	/* Set value */
	ucontrol.value.integer.value[0] = val;
	/* 
	 * Have to call this method because kcontrol 'put' method
	 * requires DAPM paths to have any impact on registers.
	 */
	return kctl->put(kctl, &ucontrol);
}

/* Set widget single enum Kcontrol */
static int d2153_sndp_set_dapm_kcontrol_enum(struct snd_soc_codec *codec,
					     char *name, int val)
{
	struct snd_kcontrol *kctl;
	struct soc_enum *e;
	struct snd_ctl_elem_info uinfo;
	struct snd_ctl_elem_value ucontrol;
	int ret;
	
	/* Find control to call */
	kctl = d2153_sndp_find_kcontrol(codec->card->snd_card, name);
	if (!kctl) {
		dev_err(codec->dev, "Could not find control %s\n", name);
		return -EINVAL;
	}

	/* Get info on Kcontrol */
	ret = kctl->info(kctl, &uinfo);
	if (ret < 0) {
		dev_err(codec->dev, "Could not retrieve control info for %s\n",
			name);
		return -EINVAL;
	}

	/* Check we're setting a SINGLE type control */
	if ((uinfo.type != SNDRV_CTL_ELEM_TYPE_ENUMERATED) || (uinfo.count != 1)) {
		dev_err(codec->dev, "Control %s is incorrect type\n", name);
		return -EINVAL;
	}

	/* Check value in valid range */
	e = (struct soc_enum *)kctl->private_value;
	if ((val < 0) || (val > (e->max - 1))) {
		dev_err(codec->dev, "Value out of range for control %s\n",
			name);
		return -EINVAL;
	}

	/* Set value */
	ucontrol.value.enumerated.item[0] = val;
	/* 
	 * Have to call this method because kcontrol 'put' method
	 * requires DAPM paths to have any impact on registers.
	 */
	return kctl->put(kctl, &ucontrol);
}

/*
 * Soundpath specific function calls
 */

int d2153_sndp_enable_vclk4(void)
{
	int ret = 0;
	struct clk *main_clk = NULL;
	struct clk *vclk4_clk = NULL;

	vclk4_clk = clk_get(NULL, "vclk4_clk");

	if (IS_ERR(vclk4_clk)) {
		ret = IS_ERR(vclk4_clk);
		goto err_gpio_request;
	}

	main_clk = clk_get(NULL, "main_clk");

	if (IS_ERR(main_clk)) {
		ret = IS_ERR(main_clk);
		goto err_gpio_request;
	}

	/* vclk4_clk */
	ret = clk_set_parent(vclk4_clk, main_clk);

	if (0 != ret) {
		goto err_gpio_request;
	}

	ret = clk_set_rate(vclk4_clk, D2153_SNDP_MCLK_RATE);

	if (0 != ret) {
		goto err_gpio_request;
	}

	ret = clk_enable(vclk4_clk);

	if (0 != ret) {
		goto err_gpio_request;
	}

	clk_put(vclk4_clk);

	return ret;

err_gpio_request:
	return ret;
}

void d2153_sndp_set_fmt(struct snd_soc_codec *codec, u_int pcm_mode)
{
	struct snd_soc_dai *codec_dai = codec->card->rtd->codec_dai;
	const struct snd_soc_dai_ops *dai_ops = codec_dai->driver->ops;
	//struct snd_pcm_substream *substream = codec_dai->runtime->trigger_master;
	struct snd_pcm_hw_params params;
	struct snd_mask *aif_fmt_mask;
	struct snd_interval *sr;
	int fmt = 0;
	int ret;

	if (SNDP_MODE_INCALL == pcm_mode) {
//		snd_soc_update_bits(codec, D2153_AIF_CTRL, D2153_AIF_FORMAT_MONO_MASK,
//			    D2153_AIF_FORMAT_MONO);
		fmt = SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_NB_IF | SND_SOC_DAIFMT_I2S;
	}
	else {
//		snd_soc_update_bits(codec, D2153_AIF_CTRL, D2153_AIF_FORMAT_MONO_MASK,
//	    D2153_AIF_FORMAT_STEREO);
		fmt = SND_SOC_DAIFMT_CBS_CFS | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_I2S;
	}	
	
	ret = dai_ops->set_fmt(codec_dai, fmt);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set dai fmt\n");
		return;
	}

	memset(&params, 0, sizeof(params));
	
	aif_fmt_mask = hw_param_mask(&params, SNDRV_PCM_HW_PARAM_FORMAT);
	sr = hw_param_interval(&params, SNDRV_PCM_HW_PARAM_RATE);

	if (SNDP_MODE_INCALL == pcm_mode) {
		/* SNDP_MODE_INCALL */
		/* AIF1 Sample Rate = 16.0 kHz, */
		/* AIF1CLK/Fs ratio = 256 (Default Register Value) */
		snd_mask_set(aif_fmt_mask, SNDRV_PCM_FORMAT_S16_LE);
		sr->min = 16000;
	} else {
		/* SNDP_MODE_NORMAL/SNDP_MODE_RING/SNDP_MODE_INCOMM */
		/* AIF1 Sample Rate = 44.1 kHz, */
		/* AIF1CLK/Fs ratio = 256 (Default Register Value) */
		snd_mask_set(aif_fmt_mask, SNDRV_PCM_FORMAT_S16_LE);
		sr->min = 48000; //44100;
	}

	ret = d2153_hw_params(NULL, &params, codec_dai);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set hw params\n");
		
	return;
}

void d2153_sndp_set_pll(struct snd_soc_codec *codec)
{
	struct snd_soc_dai *codec_dai = codec->card->rtd->codec_dai;
	const struct snd_soc_dai_ops *ops = codec_dai->driver->ops;
	int ret;

	ret = ops->set_sysclk(codec_dai, D2153_CLKSRC_MCLK,
			      D2153_SNDP_MCLK_RATE, 0);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set dai sysclk\n");
		return;
	}
		
	ret = ops->set_pll(codec_dai, 0, D2153_SYSCLK_PLL, 0,
			   D2153_PLL_FREQ_OUT_90316800);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set dai pll\n");

	d2153_sndp_set_dapm_widget(codec, "AIF", 1);
	
	return;
}

int d2153_sndp_store_volume(struct snd_soc_codec *codec,
			    const u_short addr, const u_short value)
{
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
	int ret = 0;
	u_short reg_id = 0;

	switch (addr) {
	case D2153_SP_GAIN:
		reg_id = E_VOL_SPEAKER_L;
		break;
	case D2153_EP_GAIN:
		reg_id = E_VOL_EARPIECE_L;
		break;
	case D2153_HP_L_GAIN:
		reg_id = E_VOL_HEADPHONE_L;
		break;
#ifndef CONFIG_SND_SOC_USE_DA9055_HW		
	case D2153_HP_R_GAIN:
		reg_id = E_VOL_HEADPHONE_R;
		break;
#endif		
	case D2153_MIC_L_GAIN:
		reg_id = E_VOL_MAIN_MIC;
		break;
	case D2153_AUX_L_GAIN:
		reg_id = E_VOL_SUB_MIC;
		break;
	case D2153_MIC_R_GAIN:
		reg_id = E_VOL_HEADSET_MIC;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (0 == ret) {
		d2153_codec->volume[reg_id] = value;
		d2153_codec->volume_saved[reg_id] = D2153_ENABLE;
	}

	return ret;
}

int d2153_sndp_restore_volume(struct snd_soc_codec *codec, const u_long device)
{
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
	int ret = 0;

	if ((D2153_DEV_PLAYBACK_SPEAKER & device) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_SPEAKER_L])) {
		ret = snd_soc_write(codec, D2153_SP_GAIN,
				    d2153_codec->volume[E_VOL_SPEAKER_L]);
	}

	if ((D2153_DEV_PLAYBACK_EARPIECE & device) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_EARPIECE_L])) {
		ret = snd_soc_write(codec, D2153_EP_GAIN,
				    d2153_codec->volume[E_VOL_EARPIECE_L]);		
	}

	if ((D2153_DEV_PLAYBACK_HEADPHONES & device) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_HEADPHONE_L]) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_HEADPHONE_R])) {
		ret = snd_soc_write(codec, D2153_HP_L_GAIN,
				    d2153_codec->volume[E_VOL_HEADPHONE_L]);
		ret = snd_soc_write(codec,D2153_HP_R_GAIN,
				    d2153_codec->volume[E_VOL_HEADPHONE_R]);
	}

	if ((D2153_DEV_CAPTURE_MIC & device) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_MAIN_MIC]) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_SUB_MIC])) {
		ret = snd_soc_write(codec, D2153_MIC_L_GAIN,
				    d2153_codec->volume[E_VOL_MAIN_MIC]);
		//ret = snd_soc_write(codec, D2153_AUX_L_GAIN,
		//		    d2153_codec->volume[E_VOL_SUB_MIC]);
	}

	if ((D2153_DEV_CAPTURE_HEADSET_MIC & device) &&
	    (D2153_ENABLE == d2153_codec->volume_saved[E_VOL_HEADSET_MIC])) {
		ret = snd_soc_write(codec, D2153_MIC_R_GAIN,
				    d2153_codec->volume[E_VOL_HEADSET_MIC]);
	}

	return ret;
}

static int d2153_sndp_set_speaker_device(struct snd_soc_codec * codec,
					 const u_int cur_dev,
					 const u_int new_dev)
{
	int ret = 0;	
	struct snd_soc_dai *codec_dai = codec->card->rtd->codec_dai;
	const struct snd_soc_dai_ops *ops = codec_dai->driver->ops;

	dlg_info("%s() Start cur_dev=%d new_dev=%d \n",__FUNCTION__,cur_dev,new_dev);
	if (cur_dev != new_dev) {
		if(D2153_ENABLE == new_dev) {
			d2153_sndp_set_dapm_widget(codec, "Out Mixer Speaker PGA", 1);
			d2153_sndp_set_dapm_widget(codec, "Speaker PGA", 1);

			d2153_sndp_set_dapm_kcontrol_enum(codec,"DAC Left Source MUX",2);
			d2153_sndp_set_dapm_kcontrol_enum(codec,"DAC Right Source MUX",3);
			
			d2153_sndp_set_dapm_widget(codec, "DAC Right", 1);
			
			d2153_sndp_set_dapm_kcontrol_single(codec,
							    "Out Mixer Speaker DAC Right Switch",
							    1);
			ops->digital_mute(codec_dai,0);

			d2153_sndp_set_kcontrol_double(codec, "DAC Volume", 0x6F, 0x6F);
			
			d2153_sndp_set_kcontrol_single(codec, "SP Switch", 1);
		}
		else
		{			
			d2153_sndp_set_kcontrol_single(codec, "SP Switch", 0);
			d2153_sndp_set_dapm_widget(codec, "Speaker PGA", 0);
		}
	} else {
		/* nothing to do. */
	}

	return ret;
}

static int d2153_sndp_set_earpiece_device(struct snd_soc_codec *codec,
					  const u_int cur_dev,
					  const u_int new_dev)
{
	int ret = 0;
	struct snd_soc_dai *codec_dai = codec->card->rtd->codec_dai;
	const struct snd_soc_dai_ops *ops = codec_dai->driver->ops;

	dlg_info("%s() Start cur_dev=%d new_dev=%d \n",__FUNCTION__,cur_dev,new_dev);
	if (cur_dev != new_dev) {
		if(D2153_ENABLE == new_dev) {
			d2153_sndp_set_dapm_widget(codec, "Out Mixer Right PGA", 1);
			d2153_sndp_set_dapm_widget(codec, "Earpiece PGA", 1);
			d2153_sndp_set_dapm_widget(codec, "Charge Pump", 1);

			d2153_sndp_set_dapm_kcontrol_enum(codec,"DAC Left Source MUX",2);
			d2153_sndp_set_dapm_kcontrol_enum(codec,"DAC Right Source MUX",3);
			
			d2153_sndp_set_dapm_widget(codec, "DAC Right", 1);
			
			d2153_sndp_set_dapm_kcontrol_single(codec,
							    "Out Mixer Earpiece DAC Right Switch",
							    1);
			ops->digital_mute(codec_dai,0);

			d2153_sndp_set_kcontrol_double(codec, "DAC Volume", 0x6F, 0x6F);
			
			d2153_sndp_set_kcontrol_single(codec, "EP Switch", 1);
		}
		else {
			d2153_sndp_set_kcontrol_single(codec, "EP Switch", 0);
			d2153_sndp_set_dapm_widget(codec, "Earpiece PGA", 0);
			d2153_sndp_set_dapm_widget(codec, "Charge Pump", 0);
		}
	} else {
		/* nothing to do. */
	}
	
	return ret;
}

static int d2153_sndp_set_headphone_device(struct snd_soc_codec *codec,
					   const u_int cur_dev,
					   const u_int new_dev)
{
	int ret = 0;
	struct snd_soc_dai *codec_dai = codec->card->rtd->codec_dai;
	const struct snd_soc_dai_ops *ops = codec_dai->driver->ops;

	dlg_info("%s() Start cur_dev=%d new_dev=%d \n",__FUNCTION__,cur_dev,new_dev);
	if (cur_dev != new_dev) {
		if(D2153_ENABLE == new_dev) { // headphone on
			d2153_sndp_set_dapm_widget(codec, "Charge Pump", 1);
			d2153_sndp_set_dapm_widget(codec, "Out Mixer Left PGA", 1);
			d2153_sndp_set_dapm_widget(codec, "Out Mixer Right PGA", 1);

			d2153_sndp_set_dapm_kcontrol_enum(codec,"DAC Left Source MUX",2);
			d2153_sndp_set_dapm_kcontrol_enum(codec,"DAC Right Source MUX",3);
				
			d2153_sndp_set_dapm_widget(codec, "DAC Left", 1);
			d2153_sndp_set_dapm_widget(codec, "DAC Right", 1);

			d2153_sndp_set_dapm_kcontrol_single(codec,
								"Out Mixer Left DAC Left Switch",1);
			d2153_sndp_set_dapm_kcontrol_single(codec,
								"Out Mixer Right DAC Right Switch",1);
			ops->digital_mute(codec_dai,0);

			d2153_sndp_set_kcontrol_double(codec, "DAC Volume", 0x6F, 0x6F);

			d2153_sndp_set_dapm_widget(codec, "Headphone Left PGA", 1);
			d2153_sndp_set_dapm_widget(codec, "Headphone Right PGA", 1);
			d2153_sndp_set_kcontrol_double(codec, "HP Switch", 1,1);
			msleep(50);
		}
		else { //headphone off
		
			d2153_sndp_set_kcontrol_double(codec, "HP Switch", 0,0);
			d2153_sndp_set_dapm_widget(codec, "Headphone Left PGA", 0);
			d2153_sndp_set_dapm_widget(codec, "Headphone Right PGA", 0);
			msleep(50);
			d2153_sndp_set_dapm_widget(codec, "Charge Pump", 0);
		}
			

	} else {
		/* nothing to do. */
	}

	return ret;
}

static int d2153_sndp_set_mic_device(struct snd_soc_codec *codec,
				     const u_int cur_dev, const u_int new_dev)
{
	int ret = 0;

	if (cur_dev != new_dev) { //MIC_R(MIC3)
		
		dlg_info("%s() Start cur_dev=%d new_dev=%d \n",__FUNCTION__);
		if(D2153_ENABLE == new_dev) {
	    	d2153_sndp_set_dapm_widget(codec, "In Mixer Right PGA", 1);
	    	d2153_sndp_set_dapm_widget(codec, "Mic Right PGA", 1);
	    	d2153_sndp_set_dapm_widget(codec, "Mic Bias 3", 1);

			d2153_sndp_set_dapm_kcontrol_enum(codec,"AIF Left Source MUX",1); //right
			d2153_sndp_set_dapm_kcontrol_enum(codec,"AIF Right Source MUX",1);
			
	    	d2153_sndp_set_dapm_widget(codec, "ADC Right", 1);
	    		
	    	d2153_sndp_set_dapm_kcontrol_single(codec,
	    						    "In Mixer Right Mic Right Switch",
	    						    1);

			d2153_sndp_set_kcontrol_double(codec, "In Mixer PGA Volume", 9, 9);
	    	d2153_sndp_set_kcontrol_double(codec, "In Mixer PGA Switch", 0, 1);
	    	d2153_sndp_set_kcontrol_double(codec, "ADC Switch", 0,1);
			d2153_sndp_set_kcontrol_double(codec, "Mic Switch", 0,1);	

		}
		else
		{
	    	d2153_sndp_set_dapm_widget(codec, "Mic Right PGA", 0);
	    	d2153_sndp_set_dapm_widget(codec, "Mic Bias 3", 0);
			d2153_sndp_set_kcontrol_double(codec, "Mic Switch", 0,0);	    	
		}
	} else {
		/* nothing to do. */
	}

	return ret;
}

static int d2153_sndp_set_headset_mic_device(struct snd_soc_codec *codec,
					     const u_int cur_dev,
					     const u_int new_dev)
{
	int ret = 0;
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
	
	if (cur_dev != new_dev) { //EXT(MIC1)
		dlg_info("%s() Start cur_dev=%d new_dev=%d \n",__FUNCTION__);
		if(D2153_ENABLE == new_dev) {
//			d2153_aad_update_bits(d2153_codec->aad_i2c_client, D2153_ACCDET_CONFIG,
//					  D2153_ACCDET_BTN_EN,
//					  0);
			d2153_sndp_set_dapm_widget(codec, "In Mixer Left PGA", 1);
			d2153_sndp_set_dapm_widget(codec, "Mic Ext PGA", 1);
//			d2153_sndp_set_dapm_widget(codec, "Mic Bias 1", 1); 

			d2153_sndp_set_dapm_kcontrol_enum(codec,"AIF Left Source MUX",0); //left
			d2153_sndp_set_dapm_kcontrol_enum(codec,"AIF Right Source MUX",0);
			
			d2153_sndp_set_dapm_widget(codec, "ADC Left", 1);
			
			d2153_sndp_set_dapm_kcontrol_single(codec,
							    "In Mixer Left Mic Ext Switch",
							    1);
			
			d2153_sndp_set_kcontrol_double(codec, "In Mixer PGA Volume", 9, 9);
			d2153_sndp_set_kcontrol_double(codec, "In Mixer PGA Switch", 1, 0);
			d2153_sndp_set_kcontrol_double(codec, "ADC Switch", 1, 0);
			d2153_sndp_set_kcontrol_single(codec, "Mic Ext Switch", 1);
		}
		else {
//			d2153_aad_update_bits(d2153_codec->aad_i2c_client, D2153_ACCDET_CONFIG,
//					  D2153_ACCDET_BTN_EN, D2153_ACCDET_BTN_EN);
			d2153_sndp_set_dapm_widget(codec, "Mic Ext PGA", 0);
//			d2153_sndp_set_dapm_widget(codec, "Mic Bias 1", 0); 
			d2153_sndp_set_kcontrol_single(codec, "Mic Ext Switch", 0);

		}
	} else {
		/* nothing to do. */
	}

	return ret;
}

static int d2153_sndp_resume(struct snd_soc_codec *codec, const u_long device)
{
	int ret = 0;
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
	
	d2153_codec->sndp_power_mode=1;
	//d2153_codec_power(codec,1);
	ret = d2153_sndp_restore_volume(codec, device);
	
	return ret;
}

static int d2153_sndp_suspend(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
	
	d2153_codec->sndp_power_mode=0;
	//d2153_codec_power(codec,0);
	
	return ret;
}

static void d2153_sndp_setup(struct snd_soc_codec *codec)
{
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
		
#ifdef CONFIG_SND_SOC_USE_DA9055_HW
	/*
	 * Default to using ALC manual offset calibration mode.
	 * Auto not supported on DA9055.
	 */
	snd_soc_update_bits(codec, D2153_ALC_CTRL1, D2153_ALC_CALIB_MODE_MAN,
			    D2153_ALC_CALIB_MODE_MAN);
	d2153_codec->alc_calib_auto = false;
#else
	/* Default to using ALC auto offset calibration mode. */
	snd_soc_update_bits(codec, D2153_ALC_CTRL1, D2153_ALC_CALIB_MODE_MAN, 0);
	d2153_codec->alc_calib_auto = true;
#endif /* CONFIG_SND_SOC_USE_DA9055_HW */

	/* 
	 * DLG - The following defaults should probably be platform
	 * data, but for now they're hardcoded here as a reminder.
	 */

	/* Default to using SRM for slave mode */
	d2153_codec->srm_en = true;

	/* Default dmic settings */
	snd_soc_update_bits(codec, D2153_MIC_CONFIG, D2153_DMIC_DATA_SEL_MASK,
			    D2153_DMIC_DATA_SEL_RL_FR);
	snd_soc_update_bits(codec, D2153_MIC_CONFIG,
			    D2153_DMIC_SAMPLEPHASE_MASK,
			    D2153_DMIC_SAMPLEPHASE_ON_CLK_EDGE);
	snd_soc_update_bits(codec, D2153_MIC_CONFIG, D2153_DMIC_CLK_RATE_MASK,
			    D2153_DMIC_CLK_RATE_3MHZ);

	/* Default mic bias levels */
#ifdef CONFIG_SND_SOC_USE_DA9055_HW
	snd_soc_update_bits(codec, D2153_MIC_CONFIG, D2153_MICBIAS_LEVEL_MASK,
			    D2153_MICBIAS_LEVEL_1_5V);
#else
	snd_soc_update_bits(codec, D2153_MICBIAS1_CTRL,
			    D2153_MICBIAS_LEVEL_MASK,
			    D2153_MICBIAS_LEVEL_2_5V);
	snd_soc_update_bits(codec, D2153_MICBIAS2_CTRL,
			    D2153_MICBIAS_LEVEL_MASK,
			    D2153_MICBIAS_LEVEL_2_5V);
	snd_soc_update_bits(codec, D2153_MICBIAS3_CTRL,
			    D2153_MICBIAS_LEVEL_MASK,
			    D2153_MICBIAS_LEVEL_2_5V);
#endif /* CONFIG_SND_SOC_USE_DA9055_HW */
	
	/*
	 * DLG - From the technical datasheet diagram for D2153 this should
	 * not be needed as there is a bias for each MIC input.
	 */
#ifdef CONFIG_SND_SOC_USE_DA9055_HW
	/* Default to MIC Bias 1 for right in mixer */
	snd_soc_update_bits(codec, D2153_MIXIN_R_SELECT,
			    D2153_MIC_BIAS_OUTPUT_SELECT_BIAS2, 0);
#else
	/* Default to MIC Bias 2 for right in mixer */
	snd_soc_update_bits(codec, D2153_MIXIN_R_SELECT,
			    D2153_MIC_BIAS_OUTPUT_SELECT_BIAS2,
			    D2153_MIC_BIAS_OUTPUT_SELECT_BIAS2);
#endif /* CONFIG_SND_SOC_USE_DA9055_HW */

	/* Speaker config defaults */
	snd_soc_write(codec, D2153_SP_CFG1, 0);
	snd_soc_write(codec, D2153_SP_CFG2, 0);

	/* Default IO settings */
	snd_soc_update_bits(codec, D2153_IO_CTRL,
			    D2153_IO_VOLTAGE_LEVEL_1_2V_2_8V,
			    D2153_IO_VOLTAGE_LEVEL_MASK);
	
	/* Default LDO settings */
	snd_soc_update_bits(codec, D2153_LDO_CTRL, D2153_LDO_LEVEL_SELECT_MASK,
			    D2153_LDO_LEVEL_SELECT_1_05V);
	snd_soc_update_bits(codec, D2153_LDO_CTRL, D2153_LDO_EN_MASK,
			    D2153_LDO_EN_MASK);

	/* Enable all Gain Ramping Controls */
	snd_soc_update_bits(codec, D2153_AUX_L_CTRL,
			    D2153_AUX_AMP_RAMP_EN, D2153_AUX_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_AUX_R_CTRL,
			    D2153_AUX_AMP_RAMP_EN, D2153_AUX_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_MIXIN_L_CTRL,
			    D2153_MIXIN_AMP_RAMP_EN, D2153_MIXIN_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_MIXIN_R_CTRL,
			    D2153_MIXIN_AMP_RAMP_EN, D2153_MIXIN_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_ADC_L_CTRL,
			    D2153_ADC_RAMP_EN, D2153_ADC_RAMP_EN);
	snd_soc_update_bits(codec, D2153_ADC_R_CTRL,
			    D2153_ADC_RAMP_EN, D2153_ADC_RAMP_EN);
	snd_soc_update_bits(codec, D2153_DAC_L_CTRL,
			    D2153_DAC_RAMP_EN, D2153_DAC_RAMP_EN);
	snd_soc_update_bits(codec, D2153_DAC_R_CTRL,
			    D2153_DAC_RAMP_EN, D2153_DAC_RAMP_EN);
	snd_soc_update_bits(codec, D2153_HP_L_CTRL,
			    D2153_HP_AMP_RAMP_EN, D2153_HP_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_HP_R_CTRL,
			    D2153_HP_AMP_RAMP_EN, D2153_HP_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_EP_CTRL,
			    D2153_EP_AMP_RAMP_EN, D2153_EP_AMP_RAMP_EN);
	snd_soc_update_bits(codec, D2153_SP_CTRL,
			    D2153_SP_AMP_RAMP_EN, D2153_SP_AMP_RAMP_EN);
	
	/*
	 * There are two separate control bits for input and output mixers as
	 * well as headphone and speaker outs.
	 * One to enable corresponding amplifier and other to enable its
	 * output. As amplifier bits are related to power control, they are
	 * being managed by DAPM while other (non power related) bits are
	 * enabled here
	 */

	/* Enable Left and Right input mixers */
	snd_soc_update_bits(codec, D2153_MIXIN_L_CTRL,
			    D2153_MIXIN_MIX_EN, D2153_MIXIN_MIX_EN);
	snd_soc_update_bits(codec, D2153_MIXIN_R_CTRL,
			    D2153_MIXIN_MIX_EN, D2153_MIXIN_MIX_EN);

	/* Enable Left and Right output mixers */
	snd_soc_update_bits(codec, D2153_MIXOUT_L_CTRL,
			    D2153_MIXOUT_MIX_EN, D2153_MIXOUT_MIX_EN);
	snd_soc_update_bits(codec, D2153_MIXOUT_R_CTRL,
			    D2153_MIXOUT_MIX_EN, D2153_MIXOUT_MIX_EN);

	/* Enable Speaker output mixer */
	snd_soc_update_bits(codec, D2153_MIXOUT_SP_CTRL,
			    D2153_MIXOUT_MIX_EN, D2153_MIXOUT_MIX_EN);

	/* Set charge pump mode */
	//snd_soc_update_bits(codec, D2153_CP_CTRL, D2153_CP_MCHANGE_SM_SIZE,
			    //D2153_CP_MCHANGE_SM_SIZE);
	snd_soc_update_bits(codec, D2153_CP_CTRL, D2153_CP_MCHANGE_LARGEST_VOL,
			    D2153_CP_MCHANGE_LARGEST_VOL);

	/* Enable AIF output */
	snd_soc_update_bits(codec, D2153_AIF_CTRL, D2153_AIF_OE, D2153_AIF_OE);

	/* Enable Left and Right Headphone Output */
	snd_soc_update_bits(codec, D2153_HP_L_CTRL,
			    D2153_HP_AMP_OE, D2153_HP_AMP_OE);
	snd_soc_update_bits(codec, D2153_HP_R_CTRL,
			    D2153_HP_AMP_OE, D2153_HP_AMP_OE);

	/* Enable Earpiece Output Enable */
	snd_soc_update_bits(codec, D2153_EP_CTRL,
			    D2153_EP_AMP_OE, D2153_EP_AMP_OE);
}

static int d2153_sndp_conv_device_info(const u_long device,
				       struct d2153_info *device_info)
{
	int ret = 0;

	/* check param */
	if (NULL == device_info) {
		ret = -EINVAL;
		return ret;
	}

	device_info->raw_device = device;
	device_info->speaker = (D2153_DEV_PLAYBACK_SPEAKER & device) ?
				D2153_ENABLE : D2153_DISABLE;
	device_info->earpiece = (D2153_DEV_PLAYBACK_EARPIECE & device) ?
				 D2153_ENABLE : D2153_DISABLE;
	device_info->headphone = (D2153_DEV_PLAYBACK_HEADPHONES & device) ?
				  D2153_ENABLE : D2153_DISABLE;
	device_info->mic = (D2153_DEV_CAPTURE_MIC & device) ?
			    D2153_ENABLE : D2153_DISABLE;
	device_info->headset_mic = (D2153_DEV_CAPTURE_HEADSET_MIC & device) ?
				    D2153_ENABLE : D2153_DISABLE;
	return 0;
}

static int d2153_sndp_check_device(const u_long device)
{
	int ret = 0;
	u_long dev = (D2153_DEV_PLAYBACK_SPEAKER |
		      D2153_DEV_PLAYBACK_EARPIECE |
		      D2153_DEV_PLAYBACK_HEADPHONES |
		      D2153_DEV_CAPTURE_MIC |
		      D2153_DEV_CAPTURE_HEADSET_MIC);

	/* check param */
	if (~dev & device) {
		ret = -EINVAL;
	}
	return ret;
}

int d2153_sndp_set_device(struct snd_soc_codec * codec, const u_long device,
			  const u_int pcm_value, u_int power)
{
	int ret = 0;
	struct d2153_codec_priv *d2153_codec = snd_soc_codec_get_drvdata(codec);
	struct d2153_info new_device = d2153_codec->info;
	static int audio_on = D2153_DISABLE;
	u_int pcm_mode = 0;
	struct clk *vclk4_clk = NULL;
	int i;

	dlg_info("%s() device=%d pcm_value=%d power=%d \n",__FUNCTION__,device,pcm_value,power);
	
	if (D2153_DEV_NONE != device) {
		//clk enable
		/* vclk4 enable */
		#if 1
		vclk4_clk = clk_get(NULL, "vclk4_clk");
		clk_enable(vclk4_clk);
		clk_put(vclk4_clk);
		#endif
		if ((D2153_POWER_ON == power))
		{
			d2153_codec->sndp_power_mode=1;
			//d2153_codec_power(codec,1);
		}
	}

	ret = d2153_sndp_check_device(device);

	if (0 != ret) {		
		dlg_err("%s() Error d2153_sndp_check_device \n",__FUNCTION__);
		return ret;
	}

	/* check update */
	if (d2153_codec->info.raw_device == device) {		
		dlg_info("%s() no changed \n",__FUNCTION__);
		return ret;
	}

	
	ret = d2153_sndp_conv_device_info(device, &new_device);
	
	if (0 != ret) {		
		dlg_err("%s() device convert error. ret[%d]",__FUNCTION__, ret);
		return ret;
	}

	if (D2153_DISABLE == audio_on) {

			d2153_codec->sndp_power_mode=1;
			//d2153_codec_power(codec,1);
			
			d2153_sndp_setup(codec);

			snd_soc_update_bits(codec, D2153_REFERENCES,
					    D2153_VMID_EN | D2153_BIAS_EN | D2153_VMID_FAST_CHARGE,
					    D2153_VMID_EN | D2153_BIAS_EN | D2153_VMID_FAST_CHARGE);
			msleep(100);
			snd_soc_update_bits(codec, D2153_REFERENCES,
					    D2153_VMID_FAST_CHARGE,
					    0);
			msleep(10);
			snd_soc_update_bits(codec, D2153_REFERENCES,
					    D2153_VMID_FAST_CHARGE | D2153_VMID_FAST_DISCHARGE,
					    D2153_VMID_FAST_CHARGE | D2153_VMID_FAST_DISCHARGE);
			
	}

	/* get pcm mode type */
	pcm_mode = SNDP_GET_MODE_VAL(pcm_value);

	if(pcm_mode == SNDP_MODE_INCALL) {
		for(i=0; i<codec->card->num_rtd ; i++)
			codec->card->rtd[i].dai_link->ignore_suspend =1;
	}
	else {
		for(i=0; i<codec->card->num_rtd ; i++)
			codec->card->rtd[i].dai_link->ignore_suspend =0;
	}	
		
	if (d2153_codec->pcm_mode != pcm_mode) {
		d2153_codec->pcm_mode = pcm_mode;

		d2153_sndp_set_pll(codec);

		d2153_sndp_set_fmt(codec, pcm_mode);
		
	}

	if (D2153_DISABLE == audio_on) {
		//on configuration
		ret = d2153_sndp_resume(codec, device);
	}
	/* set value */
	ret = d2153_sndp_set_speaker_device(codec, d2153_codec->info.speaker,
					    new_device.speaker);

	if (0 != ret)
		goto err_set_device;

	ret = d2153_sndp_set_earpiece_device(codec, d2153_codec->info.earpiece,
					     new_device.earpiece);

	if (0 != ret)
		goto err_set_device;

	ret = d2153_sndp_set_headphone_device(codec, d2153_codec->info.headphone,
					      new_device.headphone);

	if (0 != ret)
		goto err_set_device;


	ret = d2153_sndp_set_mic_device(codec, d2153_codec->info.mic,
					new_device.mic);

	if (0 != ret)
		goto err_set_device;

	ret = d2153_sndp_set_headset_mic_device(codec,
						d2153_codec->info.headset_mic,
						new_device.headset_mic);

	if (0 != ret)
		goto err_set_device;
	

	if (D2153_DISABLE == audio_on) {
		/***********************************/
		/* Unmutes                         */
		/***********************************/

		audio_on = D2153_ENABLE;
	}

	if (D2153_DEV_NONE == device) {
		if(power==D2153_POWER_ON) {
			ret = d2153_sndp_suspend(codec);
		}
		audio_on = D2153_DISABLE;
	}

	d2153_codec->info = new_device;
	return ret;

err_set_device:
	d2153_codec->info = new_device;
	return ret;
}
