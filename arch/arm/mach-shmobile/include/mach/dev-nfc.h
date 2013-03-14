#ifndef __ASM_MACH_DEV_NFC_H
#define __ASM_MACH_DEV_NFC_H

#if defined(CONFIG_NFC_PN547) 
extern struct i2c_board_info PN547_info[];

extern struct platform_device PN547_i2c_gpio_device;

int pn547_info_size(void);
#endif

#endif /* __ASM_MACH_DEV_NFC_H */
