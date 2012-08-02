#ifndef __ARCH_ARM_MACH_SHMOBILE_PM_R8A7373_H
#define __ARCH_ARM_MACH_SHMOBILE_PM_R8A7373_H

/* SCU */
#define SCU_BASE	0xf0000000
#define SCU_PWRST	(SCU_BASE + 0x08)
#define SCU_INVALL	(SCU_BASE + 0x0c)

/* CPG */
#define CPG_BASE	0xe6150000
#define WUPCR		(CPG_BASE + 0x1010)
#define SRESCR		(CPG_BASE + 0x1018)
#define PCLKCR		(CPG_BASE + 0x1020)
#define SCPUSTR		(CPG_BASE + 0x1040)
#define CPU0RFR		(CPG_BASE + 0x1104)
#define CPU1RFR		(CPG_BASE + 0x1114)
#define SPCTR		(CPG_BASE + 0x01a4)
#define SPCMMR		(CPG_BASE + 0x01ac)
#define SPCDMR		(CPG_BASE + 0x01b0)
#define LPMR		(CPG_BASE + 0x0200)

/* SYSC */
#define SYSC_BASE	0xe6180000
#define SPDCR		(SYSC_BASE + 0x0008)
#define SWUCR		(SYSC_BASE + 0x0014)
#define SBAR		(SYSC_BASE + 0x0020)
#define SBAR2		(SYSC_BASE + 0x0060)
#define PSTR		(SYSC_BASE + 0x0080)

#define APARMBAREA	0xe6f10020

/*
 * Inter connect RAM0
 */
#define RAM0_BASE	0xe63a2000

/* size */
#define RAM0_VECTOR_SIZE	0x80

#define RAM0_SAVE_OFFSET	0x1e00

/* code area */
#define RAM0_VECTOR_ADDR	(RAM0_BASE + 0x00)

/* save area */
#define RAM0_SAVE_BASE		(RAM0_BASE + RAM0_SAVE_OFFSET)
#define RAM0_WAKEUP_ADDR0	(RAM0_SAVE_BASE + 0x00)
#define RAM0_WAKEUP_ADDR1	(RAM0_WAKEUP_ADDR0 + 0x04)

#ifndef __ASSEMBLY__
extern void r8a7373_common_vector(void);
extern unsigned int r8a7373_common_vector_size;

extern void r8a7373_resume_core_standby(void);
extern int r8a7373_do_idle_core_standby(unsigned long unused);
#endif

#endif
