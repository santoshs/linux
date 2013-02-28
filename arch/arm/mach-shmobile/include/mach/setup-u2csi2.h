#ifndef __ASM_ARCH_CSI2_H
#define __ASM_ARCH_CSI2_H

/**
 * Camera Serial Interface.
 */

extern struct platform_device csi20_device;
extern struct platform_device csi21_device;

extern struct sh_csi2_pdata csi20_info;
extern struct sh_csi2_pdata csi21_info;

extern struct resource csi20_resources[2];
extern struct resource csi21_resources[2];

#endif /* __ASM_ARCH_CSI2_H */
