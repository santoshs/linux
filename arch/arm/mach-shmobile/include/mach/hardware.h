#ifndef __ASM_MACH_HARDWARE_H
#define __ASM_MACH_HARDWARE_H

/*
 * Fixed physical->virtual IO address translations:
 *
 * E6000000..E6FFFFFF -> FC000000..FCFFFFFF (16MiB) - HPB
 * F0000000..F01FFFFF -> FE000000..FE1FFFFF  (2MiB) - SCU, GIC, L2C etc
 *
 * (NB upcoming ARM Linux reserves FEE00000 for PCI I/O space; avoid this)
 *
 * Any other registers do not have fixed mappings so should be accessed through
 * ioremap. Note also that since Linux 3.3, ioremap will return pointers to the
 * fixed mappings where possible.
 */
#define __IO_ADDRESS(x)		(((x) | 0x1C000000) ^ 0x02000000)
#define IO_ADDRESS(x)		IOMEM(__IO_ADDRESS(x))

/* for STBCHR2 */
#define APE_RESETLOG_PANIC_START         (0x01)
#define APE_RESETLOG_PANIC_END           (0x02)
#define APE_RESETLOG_PM_RESTART          (0x04)
#define APE_RESETLOG_PM_POWEROFF         (0x08)
#define APE_RESETLOG_U2EVM_RESTART       (0x10)
#define APE_RESETLOG_RWDT_SOFTWARE_RESET (0x20)
#define APE_RESETLOG_RWDT_CMT_FIQ        (0x40)
#define APE_RESETLOG_INIT_COMPLETE       (0x80)

/* for STBCHR3 */
#define APE_RESETLOG_BOOT1  	(0x01)
#define APE_RESETLOG_BOOT2  	(0x02)
#define APE_RESETLOG_DEBUG  	(0x04)
#define APE_RESETLOG_TMPLOG_END (0x08)
#define APE_RESETLOG_TRACELOG 	(0x10)

/* For enabling/disabling trace logs */
/* Similar macro needs to be enabled/disabled in kernel/arch/arm/kernel/entry-header.S */
/*#define CONFIG_IRQ_TRACE*/
#ifdef CONFIG_IRQ_TRACE
/* TMPLOG_SIZE_PERCPU = (TMPLOG_TOTAL_SIZE / 2) = 128kB per CPU */
#define TMPLOG_SIZE_PERCPU 	0x00020000
/* TMPLOG_ENTRIES_PERCPU = (TMPLOG_TOTAL_SIZE / 32 bytes) = 4096 entries per CPU */
#define TMPLOG_ENTRIES_PERCPU	0x00001000
#define TMPLOG_TIME_OFFSET 	0x00000010

#ifndef __ASSEMBLER__
extern char *tmplog_nocache_address;
#endif
#endif  /* CONFIG_IRQ_TRACE */

#endif /* __ASM_MACH_HARDWARE_H */
