#ifndef __ASM_ARCH_RCU_H
#define __ASM_ARCH_RCU_H

extern struct platform_device rcu0_device;
extern struct platform_device rcu1_device;

extern struct sh_mobile_rcu_companion csi20;
extern struct sh_mobile_rcu_companion csi21;

extern struct resource rcu0_resources[];
extern struct resource rcu1_resources[];


extern struct sh_mobile_rcu_info sh_mobile_rcu0_info;
extern struct sh_mobile_rcu_info sh_mobile_rcu1_info;

#endif /* __ASM_ARCH_RCU_H */
