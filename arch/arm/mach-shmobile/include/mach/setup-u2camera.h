#ifndef __ASM_ARCH_CAMERA_H
#define __ASM_ARCH_CAMERA_H

/**
 * Camera
 */

extern struct platform_device camera_devices[];

int camera_init(unsigned int);
#if defined(CONFIG_SOC_CAMERA_IMX175)
int IMX175_power(struct device *dev, int power_on);
#endif
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
int S5K4ECGX_power(struct device *dev, int power_on);
#endif
int main_cam_led(int light, int mode);
#if defined(CONFIG_SOC_CAMERA_S5K6AAFX13)
int S5K6AAFX13_power(struct device *dev, int power_on);
#endif
#if defined(CONFIG_SOC_CAMERA_ISX012)
int ISX012_power(struct device *dev, int power_on);
#endif
#if defined(CONFIG_SOC_CAMERA_DB8131)
int DB8131_power(struct device *dev, int power_on);
#endif
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
int SR030PC50_power(struct device *dev, int power_on);
#endif

#endif /* __ASM_ARCH_CAMERA_H */
