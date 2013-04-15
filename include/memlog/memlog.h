/*
 * android/kernel/include/memlog/memlog.h
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 */
#ifndef __MEMLOG_H__
#define __MEMLOG_H__
#include <mach/memory-r8a7373.h>

#ifdef CONFIG_MEMLOG
#ifndef __ASSEMBLY__
extern void memory_log_proc(const char *name, unsigned long pid);
extern void memory_log_worker(unsigned long func_addr, unsigned long pid);
extern void memory_log_irq(unsigned int irq, int in);
extern void memory_log_func(unsigned long func_id, int in);
extern void memory_log_dump_int(unsigned char dump_id, int dump_data);
extern void memory_log_pm(unsigned long pm_id);
#endif

/* Use SDRAM */
#define MEMLOG_ADDRESS			SDRAM_MEMLOG_START_ADDRESS
#define MEMLOG_SIZE				0x00002010
#define CPU0_PROC_SIZE			0x00000400
#define CPU1_PROC_SIZE			0x00000400
#define CPU0_IRQ_SIZE			0x00000400
#define CPU1_IRQ_SIZE			0x00000400
#define CPU0_FUNC_SIZE			0x00000400
#define CPU1_FUNC_SIZE			0x00000400
#define CPU0_DUMP_SIZE			0x00000400
#define CPU1_DUMP_SIZE			0x00000400
#define CPU0_PM_SIZE			0x00000008
#define CPU1_PM_SIZE			0x00000008

#define CPU0_PROC_START_INDEX			\
0x00000000
#define CPU1_PROC_START_INDEX			\
(CPU0_PROC_START_INDEX + CPU0_PROC_SIZE)
#define CPU0_IRQ_START_INDEX			\
(CPU1_PROC_START_INDEX + CPU1_PROC_SIZE)
#define CPU1_IRQ_START_INDEX			\
(CPU0_IRQ_START_INDEX + CPU0_IRQ_SIZE)
#define CPU0_FUNC_START_INDEX			\
(CPU1_IRQ_START_INDEX + CPU1_IRQ_SIZE)
#define CPU1_FUNC_START_INDEX			\
(CPU0_FUNC_START_INDEX + CPU0_FUNC_SIZE)
#define CPU0_DUMP_START_INDEX			\
(CPU1_FUNC_START_INDEX + CPU1_FUNC_SIZE)
#define CPU1_DUMP_START_INDEX			\
(CPU0_DUMP_START_INDEX + CPU0_DUMP_SIZE)
#define CPU0_PM_START_INDEX				\
(CPU1_DUMP_START_INDEX + CPU1_DUMP_SIZE)
#define CPU1_PM_START_INDEX				\
(CPU0_PM_START_INDEX + CPU0_PM_SIZE)
#define MEMLOG_END						\
(CPU1_PM_START_INDEX + CPU1_PM_SIZE)

#else
static inline void memory_log_proc(const char *name, unsigned long pid)
{
}
static inline void memory_log_worker(unsigned long func_addr, unsigned long pid)
{
}
static inline void memory_log_irq(unsigned int irq, int in)
{
}
static inline void memory_log_func(unsigned long func_id, int in)
{
}
static inline void memory_log_dump_int(unsigned char dump_id, int dump_data)
{
}
#endif

#define PM_FUNC_ID_START_WFI			0x000001
#define PM_FUNC_ID_START_WFI2			0x000002
#define PM_FUNC_ID_START_CORESTANDBY	0x000003
#define PM_FUNC_ID_START_CORESTANDBY2	0x000004
#define PM_FUNC_ID_JUMP_SYSTEMSUSPEND	0x000005

#define PM_FUNC_ID_EARLY_SUSPEND				0x000106
#define PM_FUNC_ID_LATE_RESUME					0x000107

#define PM_FUNC_ID_DPM_PREPARE					0x000201
#define PM_FUNC_ID_DPM_SUSPEND					0x000202
#define PM_FUNC_ID_DPM_SUSPEND_NOIRQ				0x000203
#define PM_FUNC_ID_DPM_RESUME_NOIRQ				0x000204
#define PM_FUNC_ID_DPM_RESUME					0x000205
#define PM_FUNC_ID_DPM_COMPLETE					0x000206

#define PM_FUNC_ID_SHMOBILE_SUSPEND_BEGIN			0x000300
#define PM_FUNC_ID_SHMOBILE_SUSPEND_END				0x000301
#define PM_FUNC_ID_SHMOBILE_SUSPEND_ENTER			0x000302
#define PM_FUNC_ID_SHMOBILE_SUSPEND_PREPARE			0x000303
#define PM_FUNC_ID_SHMOBILE_SUSPEND_PREPARE_LATE		0x000304
#define PM_FUNC_ID_SHMOBILE_SUSPEND_WAKE			0x000305

#define PM_FUNC_ID_SEC_HAL_PM_COMA_ENTRY			0x000400
#define PM_FUNC_ID_PUB2SEC_DISPATCHER				0x000401
#define PM_FUNC_ID_HW_SEC_ROM_PUB_BRIDGE			0x000402

#define PM_DUMP_ID_DFS_FREQ					0x000001
#define PM_DUMP_ID_DFS_MINMAX_FREQ				0x000002
#define PM_DUMP_ID_ZB3DFS_FREQ_REQ				0x000003
#define PM_DUMP_ID_ZB3DFS_FREQ					0x000004

#define PM_PM_ID_ARMVECTOR					0x000001
#define PM_PM_ID_SUSPEND_IN					0x000002
#define PM_PM_ID_SUSPEND_OUT					0x000003
#define PM_PM_ID_CORESTANDBY_IN					0x000004
#define PM_PM_ID_CORESTANDBY_OUT				0x000005
#define PM_PM_ID_CORESTANDBY2_IN				0x000006
#define PM_PM_ID_CORESTANDBY2_OUT				0x000007
#define PM_PM_ID_HOTPLUG_IN					0x000008
#define PM_PM_ID_HOTPLUG_OUT					0x000009

#endif
