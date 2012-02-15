#ifndef __ASM_MACH_VMALLOC_H
#define __ASM_MACH_VMALLOC_H

#ifdef CONFIG_ARCH_R8A73734
/* Vmalloc at ... - 0xf5ffffff */
#define VMALLOC_END 0xf6000000UL
#else
/* Vmalloc at ... - 0xe5ffffff */
#define VMALLOC_END 0xe6000000UL
#endif

#endif /* __ASM_MACH_VMALLOC_H */
