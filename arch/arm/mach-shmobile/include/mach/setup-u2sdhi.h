#ifndef __ASM_ARCH_SDHI_H
#define __ASM_ARCH_SDHI_H

void __init u2_add_sdhi0_device(void);
void __init u2_add_sdhi1_device(void);

/* For Future usage
extern struct platform_device sdhi2_device;
extern struct renesas_sdhi_platdata sdhi1_info;
extern struct renesas_sdhi_platdata sdhi2_info;
*/
#endif /* __ASM_ARCH_SDHI_H */
