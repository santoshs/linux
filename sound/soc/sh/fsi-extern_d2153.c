/*
 * fsi-extern_d2153.c - FSI ASoC driver for boards using d2153 codec.
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
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/fsi_d2153.h>
#include <linux/d2153/d2153_codec.h>

#include <sound/sh_fsi.h>
#include <sound/soundpath/soundpath.h>

void fsi_d2153_deactivate_input(struct snd_kcontrol *kcontrol)
{
	/* Nothing to do.*/
	return;
}
EXPORT_SYMBOL(fsi_d2153_deactivate_input);

void fsi_d2153_deactivate_output(struct snd_kcontrol *kcontrol)
{
	/* Nothing to do.*/
	return;
}

int fsi_d2153_set_ignore_suspend(struct snd_soc_card *card,
	unsigned int dev_id, unsigned int status)
{
	/* Nothing to do.*/
	return 0;
}

int fsi_d2153_enable_ignore_suspend(struct snd_soc_card *card,
	unsigned int dev_id)
{
	/* Nothing to do.*/
	return 0;
}
EXPORT_SYMBOL(fsi_d2153_enable_ignore_suspend);

int fsi_d2153_disable_ignore_suspend(struct snd_soc_card *card,
	unsigned int dev_id)
{
	/* Nothing to do.*/
	return 0;
}
EXPORT_SYMBOL(fsi_d2153_disable_ignore_suspend);

void fsi_d2153_set_dac_power(struct snd_kcontrol *kcontrol,
	int status)
{
	/* Nothing to do.*/
	return;
}
EXPORT_SYMBOL(fsi_d2153_set_dac_power);
