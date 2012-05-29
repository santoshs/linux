/*
 *
 *
 * Copyright (C) 2012 Renesas Mobile Corporation.
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
#include<video/hdmi.h>
#include<video/av7100.h>
#include<linux/types.h>

struct dtd {
   unsigned long pixelFrequency;
   unsigned short hActive;
   unsigned short hBlank;
   unsigned short vActive;
   unsigned short vBlank;
   unsigned char videoMode;
   unsigned int ceaId;
};

int setHdmiState(enum av7100_hdmi_mode);

int hdmi_get_hpd_state();

enum av7100_hdmi_mode get_hdmi_state(void);

enum av7100_output_CEA_VESA hdmi_get_video_output_format();

int hdmi_set_video_output_format(enum av7100_output_CEA_VESA CURRENT_FORMAT);

void get_supported_formats(struct dtd *);

u8 hdmi_set_audio_input_format(u8);

u8 hdmi_get_audio_input_format();
   
