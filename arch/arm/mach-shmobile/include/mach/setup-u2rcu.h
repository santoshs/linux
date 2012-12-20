#ifndef __ASM_ARCH_RCU_H
#define __ASM_ARCH_RCU_H

extern struct platform_device rcu0_device;
extern struct platform_device rcu1_device;

extern struct resource rcu0_resources[];
extern struct resource rcu1_resources[];

extern struct resource rcu0_resources_es1[];
extern struct resource rcu1_resources_es1[];

extern struct sh_mobile_rcu_info sh_mobile_rcu0_info;
extern struct sh_mobile_rcu_info sh_mobile_rcu1_info;

int rcu1_resources_es1_size(void);
#endif /* __ASM_ARCH_RCU_H */
