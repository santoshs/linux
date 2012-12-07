/*
 * d2153.h  --  D2153 Soundpath Driver
 *
 * Copyright (c) 2012 Dialog Semiconductor
 *
 * Author: Adam Thomson <Adam.Thomson@diasemi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <sound/soc.h>

/* Defines */
#define D2153_ENABLE		1    /* enable value */
#define D2153_DISABLE		0    /* disable value */

/* Methods */
int d2153_sndp_enable_vclk4(void);
int d2153_sndp_set_device(struct snd_soc_codec * codec, const u_long device,
			  const u_int pcm_value, u_int power);
int d2153_sndp_store_volume(struct snd_soc_codec *codec,
			    const u_short addr, const u_short value);
int d2153_sndp_restore_volume(struct snd_soc_codec *codec, const u_long device);		   
