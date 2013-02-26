/*
 * android/kernel/include/memlog/memlog.h
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 */
#ifndef __MEMLOG_H__
#define __MEMLOG_H__

#ifdef CONFIG_MEMLOG
extern void memory_log_proc(const char *name, unsigned long pid);
extern void memory_log_worker(unsigned long func_addr, unsigned long pid);
extern void memory_log_irq(unsigned int irq, int in);
extern void memory_log_func(unsigned long func_id, int in);
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
#endif

#define PM_FUNC_ID_START_WFI			0x000001
#define PM_FUNC_ID_START_WFI2			0x000002
#define PM_FUNC_ID_START_CORESTANDBY	0x000003
#define PM_FUNC_ID_START_CORESTANDBY2	0x000004
#define PM_FUNC_ID_JUMP_SYSTEMSUSPEND	0x000005

#define PM_FUNC_ID_EARLY_SUSPEND					0x000106
#define PM_FUNC_ID_LATE_RESUME						0x000107

#define PM_FUNC_ID_DPM_PREPARE						0x000201
#define PM_FUNC_ID_DPM_SUSPEND						0x000202
#define PM_FUNC_ID_DPM_SUSPEND_NOIRQ				0x000203
#define PM_FUNC_ID_DPM_RESUME_NOIRQ					0x000204
#define PM_FUNC_ID_DPM_RESUME						0x000205
#define PM_FUNC_ID_DPM_COMPLETE						0x000206

#define PM_FUNC_ID_SHMOBILE_SUSPEND_BEGIN			0x000300
#define PM_FUNC_ID_SHMOBILE_SUSPEND_END				0x000301
#define PM_FUNC_ID_SHMOBILE_SUSPEND_ENTER			0x000302
#define PM_FUNC_ID_SHMOBILE_SUSPEND_PREPARE			0x000303
#define PM_FUNC_ID_SHMOBILE_SUSPEND_PREPARE_LATE	0x000304
#define PM_FUNC_ID_SHMOBILE_SUSPEND_WAKE			0x000305

#endif
