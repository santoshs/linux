#ifndef __ASM_ARCH_CAMERA_H
#define __ASM_ARCH_CAMERA_H

/**
 * Camera
 */

extern struct platform_device camera_devices[];

int camera_init(unsigned int);
int IMX175_power(struct device *dev, int power_on);
int main_cam_led(int light, int mode);
int S5K6AAFX13_power(struct device *dev, int power_on);
int ISX012_power(struct device *dev, int power_on);
int DB8131_power(struct device *dev, int power_on);

#endif /* __ASM_ARCH_CAMERA_H */
