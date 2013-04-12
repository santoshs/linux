/*
 * Driver for OmniVision OV5640 CMOS Image Sensor
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __OV5640_H__
#define __OV5640_H__

#include <linux/device.h>

int OV5640_power(struct device *dev, int power_on);
int OV5640_l_reset(void *handle, int reset);

#endif /* __OV5640_H__ */
