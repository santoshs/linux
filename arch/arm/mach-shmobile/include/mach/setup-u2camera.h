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

#if defined(CONFIG_SOC_CAMERA)
int camera_init(int gpio_cam_pwr_en, int gpio_cam_rst_n, int gpio_cam_stby);
void add_primary_cam_flash_rt8547(int gpio_cam_flash_enset,
		int gpio_cam_flash_flen);
void add_primary_cam_flash_mic2871(int gpio_cam_flash_enset,
		int gpio_cam_flash_flen);
void add_s5k4ecgx_primary_camera(void);
void add_ov5645_primary_camera(void);
void add_sr030pc50_secondary_camera(void);
void add_hm2056_secondary_camera(void);
#else
static inline int camera_init(int gpio_cam_pwr_en, int gpio_cam_rst_n,
		int gpio_cam_stby) { return 0; }
static inline void add_primary_cam_flash_rt8547(int gpio_cam_flash_enset,
		int gpio_cam_flash_flen) {}
static inline void add_primary_cam_flash_mic2871(int gpio_cam_flash_enset,
		int gpio_cam_flash_flen) {}
static inline void add_s5k4ecgx_primary_camera(void) {}
static inline void add_ov5645_primary_camera(void) {}
static inline void add_sr030pc50_secondary_camera(void) {}
static inline void add_hm2056_secondary_camera(void) {}
#endif

#if defined(CONFIG_SOC_CAMERA_OV5645)
int OV5645_power(struct device *dev, int power_on);
#else
static inline int OV5645_power(struct device *dev, int power_on) { return 0; }
#endif
#if defined(CONFIG_SOC_CAMERA_HM2056)
int HM2056_power(struct device *dev, int power_on);
#else
static inline int HM2056_power(struct device *dev, int power_on) { return 0; }
#endif
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
int S5K4ECGX_power(struct device *dev, int power_on);
#else
static inline int S5K4ECGX_power(struct device *dev, int power_on) { return 0; }
#endif
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
int SR030PC50_power(struct device *dev, int power_on);
#else
static inline int SR030PC50_power(struct device *dev, int power_on) { return 0; }
#endif


#endif /* __ASM_ARCH_CAMERA_H */
