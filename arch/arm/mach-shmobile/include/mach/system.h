#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

static inline void arch_idle(void)
{
	cpu_do_idle();
}

#ifdef CONFIG_ARCH_R8A73734
extern void (*shmobile_arch_reset)(char mode, const char *cmd);
#endif

static inline void arch_reset(char mode, const char *cmd)
{
#ifdef CONFIG_ARCH_R8A73734
	/* call the CPU-specific reset function */
	if (shmobile_arch_reset)
		shmobile_arch_reset(mode, cmd);
	else
		soft_restart(0);
#else
	soft_restart(0);
#endif
}

#endif
