/*
 * android/kernel/drivers/memlog/memlog.c
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/time.h>
#include <memlog/memlog.h>

static struct kobject *memlog_kobj;

#ifdef CONFIG_ARM_TZ
/* TrustZone ON  : Use SDRAM */
#define MEMLOG_ADDRESS			0x448F1C00
#else
/* TrustZone OFF : Use SecRAM */
#define MEMLOG_ADDRESS			0xE6305000
#endif
#define MEMLOG_SIZE				0x00002380
#define CPU0_PROC_SIZE			0x00000400
#define CPU1_PROC_SIZE			0x00000400
#define CPU0_IRQ_SIZE			0x00000400
#define CPU1_IRQ_SIZE			0x00000400
#define CPU0_FUNC_SIZE			0x00000400
#define CPU1_FUNC_SIZE			0x00000400

#define CMCNT0					0xE6130014

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
#define MEMLOG_END						\
(CPU1_FUNC_START_INDEX + CPU1_FUNC_SIZE)


static char *logdata;
static unsigned long capture;
static unsigned long cpu0_proc_index;
static unsigned long cpu1_proc_index;
static unsigned long cpu0_irq_index;
static unsigned long cpu1_irq_index;
static unsigned long cpu0_func_index;
static unsigned long cpu1_func_index;


void memory_log_proc(const char *name, unsigned long pid)
{
	int len = 0;
	int cpu = 0;
	unsigned long index = 0;
	unsigned long data[6] = {0, 0, 0, 0, 0, 0};
	unsigned long flags = 0;
	if (!logdata || !name || !capture)
		return;

	cpu = raw_smp_processor_id();
	data[0] = __raw_readl(CMCNT0);
	data[1] = pid;
	snprintf((char *)&data[2], 16, "%s", name);
	len = sizeof(data);

	local_irq_save(flags);

	if (cpu == 0) {
		if (cpu0_proc_index + len > CPU1_PROC_START_INDEX)
			cpu0_proc_index = CPU0_PROC_START_INDEX;

		index = cpu0_proc_index;
		cpu0_proc_index += len;
	} else if (cpu == 1) {
		if (cpu1_proc_index + len > CPU0_IRQ_START_INDEX)
			cpu1_proc_index = CPU1_PROC_START_INDEX;

		index = cpu1_proc_index;
		cpu1_proc_index += len;
	}

	local_irq_restore(flags);
	memcpy(logdata + index, data, len);
}
EXPORT_SYMBOL_GPL(memory_log_proc);

void memory_log_worker(unsigned long func_addr, unsigned long pid)
{
	char str[16];
	sprintf(str, "0x%lx", func_addr);
	memory_log_proc(str, pid);
}
EXPORT_SYMBOL_GPL(memory_log_worker);

void memory_log_irq(unsigned int irq, int in)
{
	int len = 0;
	int cpu = 0;
	unsigned long index = 0;
	unsigned long data[2] = {0, 0};
	unsigned long flags = 0;
	if (!logdata || !capture)
		return;

	cpu = raw_smp_processor_id();
	data[0] = __raw_readl(CMCNT0);
	data[1] = irq + (in << 24);
	len = sizeof(data);

	local_irq_save(flags);

	if (cpu == 0) {
		if (cpu0_irq_index + len > CPU1_IRQ_START_INDEX)
			cpu0_irq_index = CPU0_IRQ_START_INDEX;

		index = cpu0_irq_index;
		cpu0_irq_index += len;
	} else if (cpu == 1) {
		if (cpu1_irq_index + len > CPU0_FUNC_START_INDEX)
			cpu1_irq_index = CPU1_IRQ_START_INDEX;

		index = cpu1_irq_index;
		cpu1_irq_index += len;
	}

	local_irq_restore(flags);
	memcpy(logdata + index, data, len);
}
EXPORT_SYMBOL_GPL(memory_log_irq);

void memory_log_func(unsigned long func_id, int in)
{
	int len = 0;
	int cpu = 0;
	unsigned long index = 0;
	unsigned long data[2] = {0, 0};
	unsigned long flags = 0;
	if (!logdata || !capture)
		return;

	cpu = raw_smp_processor_id();
	data[0] = __raw_readl(CMCNT0);
	data[1] = func_id + (in << 24);
	len = sizeof(data);

	local_irq_save(flags);

	if (cpu == 0) {
		if (cpu0_func_index + len > CPU1_FUNC_START_INDEX)
			cpu0_func_index = CPU0_FUNC_START_INDEX;

		index = cpu0_func_index;
		cpu0_func_index += len;
	} else if (cpu == 1) {
		if (cpu1_func_index + len > MEMLOG_END)
			cpu1_func_index = CPU1_FUNC_START_INDEX;

		index = cpu1_func_index;
		cpu1_func_index += len;
	}

	local_irq_restore(flags);
	memcpy(logdata + index, data, len);
}
EXPORT_SYMBOL_GPL(memory_log_func);

void memory_log_init(void)
{
	cpu0_proc_index = CPU0_PROC_START_INDEX;
	cpu1_proc_index = CPU1_PROC_START_INDEX;
	cpu0_irq_index = CPU0_IRQ_START_INDEX;
	cpu1_irq_index = CPU1_IRQ_START_INDEX;
	cpu0_func_index = CPU0_FUNC_START_INDEX;
	cpu1_func_index = CPU1_FUNC_START_INDEX;
	memset(logdata, 0, MEMLOG_SIZE);
	capture = 1;
}

static ssize_t capture_show(
	struct kobject *kobj,
	struct kobj_attribute *attr,
	char *buf)
{
	return sprintf(buf, "%ld\n", capture);
}

static ssize_t capture_store(
	struct kobject *kobj,
	struct kobj_attribute *attr,
	const char *buf,
	size_t count)
{
	int tmp = 0;
	sscanf(buf, "%du", &tmp);
	if (capture != tmp) {
		if (tmp == 0)
			capture = 0;
		else if (tmp == 1)
			memory_log_init();
	}
	return count;
}

static struct kobj_attribute capture_attribute =
	__ATTR(capture, 0644, capture_show, capture_store);

static ssize_t read_log(
	struct file *filp,
	struct kobject *kobj,
	struct bin_attribute *bin_attr,
	char *buf,
	loff_t pos,
	size_t count)
{
	unsigned long max_count = MEMLOG_SIZE - pos;
	if (count > max_count)
		count = max_count;

	memcpy(buf, logdata + pos, count);
	return count;
}

static struct bin_attribute log_attr = {
	.attr = {
		.name = "log",
		.mode = S_IRUGO,
	},
	.size = MEMLOG_SIZE,
	.read = read_log,
};

static int __init init_memlog(void)
{
	int ret = 0;

	memlog_kobj = kobject_create_and_add("memlog", NULL);
	if (!memlog_kobj) {
		printk(KERN_ERR "kobject_create_and_add: failed\n");
		ret = -ENOMEM;
		goto exit;
	}

	ret = sysfs_create_file(memlog_kobj, &capture_attribute.attr);
	if (ret) {
		printk(KERN_ERR "sysfs_create_file: failed (%d)", ret);
		goto kset_exit;
	}

	ret = sysfs_create_bin_file(memlog_kobj, &log_attr);
	if (ret) {
		printk(KERN_ERR "sysfs_create_bin_file: failed (%d)", ret);
		goto kset_exit;
	}

	logdata = (char *)ioremap_nocache(MEMLOG_ADDRESS, MEMLOG_SIZE);
	if (!logdata) {
		printk(KERN_ERR "ioremap_nocache: failed");
		ret = -ENOMEM;
		goto exit;
	}

	memory_log_init();
	return 0;

kset_exit:
	kobject_put(memlog_kobj);
exit:
	return ret;
}


static void __exit exit_memlog(void)
{
	kobject_put(memlog_kobj);
}

module_init(init_memlog);
module_exit(exit_memlog);

MODULE_DESCRIPTION("Memory Log");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Renesas Mobile Corp.");

