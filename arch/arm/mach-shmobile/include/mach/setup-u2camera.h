/*
 * arch/arm/mach-shmobile/include/mach/setup-u2camera.h
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef __ASM_ARCH_CAMERA_H
#define __ASM_ARCH_CAMERA_H

/**
 * Camera
 */

extern struct platform_device camera_devices[];
int OV5645_power(struct device *dev, int power_on);
int HM2056_power(struct device *dev, int power_on);
int S5K4ECGX_power(struct device *dev, int power_on);
int SR030PC50_power(struct device *dev, int power_on);

int camera_init(void);
int main_cam_led(int light, int mode);

#endif /* __ASM_ARCH_CAMERA_H */
