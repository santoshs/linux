/*
 *  Copyright (C) 2012 Dialog Semiconductor Ltd.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */

struct fsi_d2153_platform_data {
	int gpio_spkr_en;
	int gpio_hp_det;
	int gpio_hp_mute;
	int gpio_int_mic_en;
	int gpio_ext_mic_en;
	struct clk *main;
	struct clk *vclk4;
	void *private_data;
};
