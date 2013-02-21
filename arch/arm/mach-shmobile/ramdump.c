/*
 * arch/arm/mach-shmobile/ramdump.c
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kmsg_dump.h>
#include <linux/kdebug.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <asm/system_misc.h>
#include <asm/cacheflush.h>
#include <asm/atomic.h>
#include <mach/ramdump.h>

/* TODO: disable debug prints when k345 is in better shape
 */
static int debug_enabled = 1;
module_param_named(debug_enabled, debug_enabled, int,
		S_IRUGO | S_IWUSR | S_IWGRP);

#define dprintk(fmt, arg...) \
	do { \
		if (debug_enabled) \
			printk(KERN_INFO fmt, ##arg); \
	} while (0)

struct ramdump {
	/* base address & size for register dump area */
	void *reg_dump_base;
	size_t reg_dump_size;

	/* base address for core register dump */
	void *core_reg_dump_base;
	/* size reserved for each core */
	size_t core_reg_dump_size;

	/* offset for exception registers */
	unsigned int excp_reg_offset;

	/* base address & size for hw register dump */
	void *lsi_reg_dump_base;
	size_t lsi_reg_dump_size;

	/* ioremap variables for capturing Hardware-Register value */
	void __iomem *sbsc_setting_00;
	void __iomem *sbsc_setting_01;
	void __iomem *sbsc_mon_setting;
	void __iomem *sbsc_phy_setting_00;
	void __iomem *sbsc_phy_setting_01;
	void __iomem *sbsc_phy_setting_02;
	void __iomem *ipmmu_setting;

	/* pointer to reg dump area for easy finding in ramdump analyzator */
	struct hw_register_dump *hw_register_dump;
	atomic_t hw_registers_saved;
};

static struct ramdump *ramdump;



/**
 *  my_emergency_restart - reboot the system
 *
 *  This is a hacked version of emergency_restart to avoid stopping
 *  peripherals etc which should not happen in emergency restart (machine
 *  shutdown calls shmobile_pm_stop_peripheral_devices in ARCH_R8A73734).
 */

static void my_emergency_restart(void)
{
	dprintk("%s\n", __func__);
	kmsg_dump(KMSG_DUMP_EMERG);

	arm_pm_restart('h', NULL);

	/* reboot failed, let's wait watchdog */
	while(1);
}

/*
 * For analysis tool compatibility, the layout should only be extended, not
 * changed. Layout is:
 * +000: R0
 * +004: R1
 * +008: R2
 * +00C: R3
 * +010: R4
 * +014: R5
 * +018: R6
 * +01C: R7
 * +020: R8
 * +024: R9
 * +028: R10
 * +02C: R11
 * +030: R12
 * +034: R13_usr
 * +038: R14_usr
 * +03C: R15
 * +040: CPSR
 * +044: SPSR_svc
 * +04C: R14_svc
 * +050: SPSR_abt
 * +054: R13_abt
 * +058: R14_abt
 * +05C: SPSR_und
 * +060: R13_und
 * +064: R14_und
 * +068: SPSR_irq
 * +06C: R13_irq
 * +070: R14_irq
 * +074: SPSR_fiq
 * +078: R8_fiq
 * +07C: R9_fiq
 * +080: R10_fiq
 * +084: R11_fiq
 * +088: R12_fiq
 * +08C: R13_fiq
 * +090: R14_fiq
 * +094: MPIDR
 * +098: SCTLR
 * +09C: ACTLR
 * +0A0: TTBR0
 * +0A4: TTBR1
 * +0A8: TTBCR
 * +0AC: DACR
 * +0B0: DFSR
 * +0B4: IFSR
 * +0B8: DFAR
 * +0BC: IFAR
 * +0C0: PRRR
 * +0C4: NMRR
 */
static void *save_core_registers(void *ptr)
{
	register void *p asm("r0") = ptr;

	/* Routine assumes it is entered in SVC mode. */
	asm volatile(
		"STMIA	r0, {r0-r15}^\n\t"	/* User registers */
		"ADD	r0, r0, #64\n\t"
		"MRS	r1, cpsr\n\t"
		"MRS	r2, spsr\n\t"
		"STMIA	r0!, {r1,r2,r13,r14}\n\t"/* SVC SPSR,R13,R14 */
		"CPSID	a, #0x17\n\t"
		"MRS	r2, spsr\n\t"
		"STMIA	r0!, {r2,r13,r14}\n\t"	/* ABT SPSR,R13,R14 */
		"CPS	#0x1B\n\t"
		"MRS	r2, spsr\n\t"
		"STMIA	r0!, {r2,r13,r14}\n\t"	/* UND SPSR,R13,R14 */
		"CPSID	i, #0x12\n\t"
		"MRS	r2, spsr\n\t"
		"STMIA	r0!, {r2,r13,r14}\n\t"	/* IRQ SPSR,R13,R14 */
		"CPSID	f, #0x11\n\t"
		"MRS	r2, spsr\n\t"
		"STMIA	r0!, {r2,r8-r14}\n\t"	/* FIQ SPSR,R8-R14 */
		"MSR	cpsr_cx, r1\n\t"	/* Restore SVC mode+interrupt status */

		"MRC	p15, 0, r1, c0, c0, 5\n\t" /* MPIDR (Multiprocessor Affinity) */
		"MRC	p15, 0, r2, c1, c0, 0\n\t" /* SCTLR (System Control) */
		"MRC	p15, 0, r3, c1, c0, 1\n\t" /* ACTLR (Auxiliary Control) */
		"STMIA	r0!, {r1-r3}\n\t"
		"MRC	p15, 0, r1, c2, c0, 0\n\t" /* TTBR0 (Translation Table Base 0) */
		"MRC	p15, 0, r2, c2, c0, 1\n\t" /* TTBR1 */
		"MRC	p15, 0, r3, c2, c0, 2\n\t" /* TTBCR (Translation Table Base Control) */
		"MRC	p15, 0, r12, c3, c0, 0\n\t" /* DACR (Domain Access Control) */
		"STMIA	r0!, {r1-r3,r12}\n\t"
		"MRC	p15, 0, r1, c5, c0, 0\n\t" /* DFSR (Data Fault Status) */
		"MRC	p15, 0, r2, c5, c0, 1\n\t" /* IFSR (Instruction Fault Status) */
		"MRC	p15, 0, r3, c6, c0, 0\n\t" /* DFAR (Data Fault Address) */
		"MRC	p15, 0, r12, c6, c0, 2\n\t" /* IFAR (Instruction Fault Address) */
		"STMIA r0!, {r1-r3,r12}\n\t"
		"MRC p15, 0, r1, c10, c2, 0\n\t" /* PRRR (Primary Region Remap) */
		"MRC p15, 0, r2, c10, c2, 1\n\t" /* NMRR (Normal Memory Remap) */
		"STMIA r0!, {r1-r2}"

		: "+r" (p) /* output */
		:
		: "r1", "r2", "r3", "r12", "memory" /* clobber */
	);
	return p;
}


/**
 * Read_32bit_HwRegisters - Read 32bit HW Register
 * @return : none
 */
static void Read_32bit_HwRegisters(unsigned long *write_ptr,
				   unsigned long *start_addr,
				   unsigned long *end_addr)
{
	unsigned long *ulong_ptr;
	unsigned long *read_addr;

	ulong_ptr = write_ptr;
	read_addr = start_addr;

	if ((NULL != ulong_ptr) && (NULL != read_addr) && (NULL != end_addr)) {
		while (read_addr <= end_addr) {
			*ulong_ptr = __raw_readl(read_addr);
			dprintk("%p, %p : %08lx\n", read_addr, ulong_ptr,
					*ulong_ptr);
			ulong_ptr++;
			read_addr++;
		}
	}
}

static inline void Read_32bit_HwRegister(unsigned long *write_ptr,
					 unsigned long *read_addr)
{
	*write_ptr = __raw_readl(read_addr);
	dprintk("%p, %p : %08lx\n", read_addr, write_ptr, *write_ptr);
}

static inline void Read_16bit_HwRegister(unsigned long *write_ptr,
					 unsigned long *read_addr)
{
	*write_ptr = (unsigned long)__raw_readw(read_addr);
	dprintk("%p, %p : %08lx\n", read_addr, write_ptr, *write_ptr);
}

static inline void Read_8bit_HwRegister(unsigned long *write_ptr,
					unsigned long *read_addr)
{
	*write_ptr = (unsigned long)__raw_readb(read_addr);
	dprintk("%p, %p : %08lx\n", read_addr, write_ptr, *write_ptr);
}
/**
 * Save_HwRegister_To_RAM - Write logs to eMMC
 * @return : none
 */
static void Save_HwRegister_To_RAM(void)
{
	ulong len;
	struct hw_register_dump	*reg_dump = ramdump->hw_register_dump;

	dprintk("--- Start of %s() ---\n", __func__);

	/* Check if we already have dumped the LSI registers */
	if (!atomic_inc_and_test(&ramdump->hw_registers_saved)) {
		dprintk("%s HW registers already dumped\n", __func__);
		return;
	}


	/*  Transfer value of each registers to the struct variable  */

	/* -----	SBSC1 ----- */
	dprintk("   Exec %s() SBSC1\n", __func__);
	if (ramdump->sbsc_setting_00) {
		len = (SBSC_SETTING_00_E - SBSC_SETTING_00_S);
		Read_32bit_HwRegisters(	reg_dump->SBSC_Setting_00,
					ramdump->sbsc_setting_00,
					ramdump->sbsc_setting_00 + len);

		dprintk("     SDCR0A  = %08lx\n", reg_dump->SBSC_Setting_00[2]);
		dprintk("     SDCR1A  = %08lx\n", reg_dump->SBSC_Setting_00[3]);
		dprintk("     SDPCRA  = %08lx\n", reg_dump->SBSC_Setting_00[4]);
		dprintk("     EBMCRA  = %08lx\n", reg_dump->SBSC_Setting_00[5]);
		dprintk("   ioremap OK in %s() : SBSC1 #01\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : SBSC1 #01\n", __func__);
	}

	if (ramdump->sbsc_setting_01) {
		len = (SBSC_SETTING_01_E - SBSC_SETTING_01_S);
		Read_32bit_HwRegisters(	reg_dump->SBSC_Setting_01,
					ramdump->sbsc_setting_01,
					ramdump->sbsc_setting_01 + len);

		dprintk("   ioremap OK in %s() : SBSC1 #02\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : SBSC1 #02\n", __func__);
	}

	if (ramdump->sbsc_mon_setting) {
		Read_32bit_HwRegister(	&reg_dump->SBSC_Mon_Setting,
					ramdump->sbsc_mon_setting);
		dprintk("   ioremap OK in %s() : SBSC1 #03\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : SBSC1 #03\n", __func__);
	}

	if (ramdump->sbsc_phy_setting_00) {
		len = (SBSC_PHY_SETTING_00_E - SBSC_PHY_SETTING_00_S);
		Read_32bit_HwRegisters(	reg_dump->SBSC_PHY_Setting_00,
					ramdump->sbsc_phy_setting_00,
					ramdump->sbsc_phy_setting_00 + len);
		dprintk("   ioremap OK in %s() : SBSC1 #04\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : SBSC1 #04\n", __func__);
	}

	if (ramdump->sbsc_phy_setting_01) {
		Read_32bit_HwRegister(	&reg_dump->SBSC_PHY_Setting_01,
					ramdump->sbsc_phy_setting_01);

		dprintk("   ioremap OK in %s() : SBSC1 #05\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : SBSC1 #05\n", __func__);
	}

	if (ramdump->sbsc_phy_setting_02) {
		len = (SBSC_PHY_SETTING_02_E - SBSC_PHY_SETTING_02_S);
		Read_32bit_HwRegisters(	reg_dump->SBSC_PHY_Setting_02,
					ramdump->sbsc_phy_setting_02,
					ramdump->sbsc_phy_setting_02 + len);

		dprintk("   ioremap OK in %s() : SBSC1 #06\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : SBSC1 #06\n", __func__);
	}

	/* -----	CPG ----- */
	dprintk("   Exec %s() CPG\n", __func__);
	Read_32bit_HwRegisters(	reg_dump->CPG_Setting_00,
				(unsigned long *)CPG_SETTING_00_S,
				(unsigned long *)CPG_SETTING_00_E);
	Read_32bit_HwRegisters(	reg_dump->CPG_Setting_01,
				(unsigned long *)CPG_SETTING_01_S,
				(unsigned long *)CPG_SETTING_01_E);

	/* -----	RWDT ----- */
	dprintk("   Exec %s() RWDT\n", __func__);
	Read_16bit_HwRegister(	&reg_dump->RWDT_Condition[0],
				(unsigned long *)RWDT_CONDITION_00);
	Read_8bit_HwRegister(	&reg_dump->RWDT_Condition[1],
				(unsigned long *)RWDT_CONDITION_01);
	Read_8bit_HwRegister(	&reg_dump->RWDT_Condition[2],
				(unsigned long *)RWDT_CONDITION_02);

	/* -----	SWDT ----- */
	dprintk("   Exec %s() SWDT\n", __func__);
	Read_16bit_HwRegister(	&reg_dump->SWDT_Condition[0],
				(unsigned long *)SWDT_CONDITION_00);
	Read_8bit_HwRegister(	&reg_dump->SWDT_Condition[1],
				(unsigned long *)SWDT_CONDITION_02);
	Read_8bit_HwRegister(	&reg_dump->SWDT_Condition[2],
				(unsigned long *)SWDT_CONDITION_02);

	/* -----	Secure Up-time Clock ----- */
	dprintk("   Exec %s() Secure Up-time Clock\n", __func__);
	Read_16bit_HwRegister(	&reg_dump->SUTC_Condition[0],
				(unsigned long *)SUTC_CONDITION_00);
	Read_16bit_HwRegister(	&reg_dump->SUTC_Condition[1],
				(unsigned long *)SUTC_CONDITION_01);

	Read_32bit_HwRegisters(	&(reg_dump->SUTC_Condition[2]),
				(unsigned long *)SUTC_CONDITION_02,
				(unsigned long *)SUTC_CONDITION_03);

	/* -----	CMT15 ----- */
	dprintk("   Exec %s() CMT15\n", __func__);
	Read_32bit_HwRegister(	&reg_dump->CMT15_Condition[0],
				(unsigned long *)CMT15_CONDITION_00);
	Read_32bit_HwRegister(	&reg_dump->CMT15_Condition[1],
				(unsigned long *)CMT15_CONDITION_01);
	Read_32bit_HwRegister(	&reg_dump->CMT15_Condition[2],
				(unsigned long *)CMT15_CONDITION_02);
	Read_32bit_HwRegister(	&reg_dump->CMT15_Condition[3],
				(unsigned long *)CMT15_CONDITION_03);

	/* -----	SYSC ----- */

	dprintk("   Exec %s() SYSC\n", __func__);
	Read_32bit_HwRegisters(	reg_dump->SYSC_Setting_00,
				(unsigned long *)SYSC_SETTING_00_S,
				(unsigned long *)SYSC_SETTING_00_E);
	Read_32bit_HwRegisters(	reg_dump->SYSC_Setting_01,
				(unsigned long *)SYSC_SETTING_01_S,
				(unsigned long *)SYSC_SETTING_01_E);
	Read_32bit_HwRegisters(	reg_dump->SYSC_Rescnt,
				(unsigned long *)SYSC_RESCNT_00,
				(unsigned long *)SYSC_RESCNT_02);

	/* -----	DBG ----- */
	dprintk("   Exec %s() DBG\n", __func__);
	Read_32bit_HwRegister(	&reg_dump->DBG_Setting[0],
				(unsigned long *)DBG_SETTING_00);

	Read_32bit_HwRegisters(	&(reg_dump->DBG_Setting[1]),
				(unsigned long *)DBG_SETTING_01,
				(unsigned long *)DBG_SETTING_02);

	/* -----	GIC ----- */
	dprintk("   Exec %s() GIC\n", __func__);
	Read_32bit_HwRegister(	&reg_dump->GIC_Setting[0],
				(unsigned long *)GIC_SETTING_00);
	Read_32bit_HwRegister(	&reg_dump->GIC_Setting[1],
				(unsigned long *)GIC_SETTING_01);

	/* -----	L2C-310 ----- */
	dprintk("   Exec %s() L2C-310\n", __func__);
	Read_32bit_HwRegister(	&reg_dump->PL310_Setting[0],
				(unsigned long *)PL310_SETTING_00);
	Read_32bit_HwRegister(	&reg_dump->PL310_Setting[1],
				(unsigned long *)PL310_SETTING_01);

	/* -----	INTC-SYS ----- */
	dprintk("   Exec %s() INTC-SYS\n", __func__);
	Read_32bit_HwRegisters(	&(reg_dump->INTC_SYS_Info[0]),
				(unsigned long *)INTC_SYS_INFO_00,
				(unsigned long *)INTC_SYS_INFO_01);
	Read_32bit_HwRegisters(	&(reg_dump->INTC_SYS_Info[2]),
				(unsigned long *)INTC_SYS_INFO_02,
				(unsigned long *)INTC_SYS_INFO_03);

	/* ----- INTC-BB ----- */
	dprintk("   Exec %s() INTC-BB\n", __func__);
	Read_32bit_HwRegister(	&reg_dump->INTC_BB_Info[0],
				(unsigned long *)INTC_BB_INFO_00);
	Read_32bit_HwRegister(	&reg_dump->INTC_BB_Info[1],
				(unsigned long *)INTC_BB_INFO_01);

	/* -----	ICC0 ----- */
	dprintk("   Exec %s() ICC0\n", __func__);
	Read_8bit_HwRegister(	&reg_dump->IIC0_Setting[0],
				(unsigned long *)IIC0_SETTING_00);
	Read_8bit_HwRegister(	&reg_dump->IIC0_Setting[1],
				(unsigned long *)IIC0_SETTING_01);
	Read_8bit_HwRegister(	&reg_dump->IIC0_Setting[2],
				(unsigned long *)IIC0_SETTING_02);

	/* -----	ICCB ----- */
	dprintk("   Exec %s() ICCB\n", __func__);
	Read_8bit_HwRegister(	&reg_dump->IICB_Setting[0],
				(unsigned long *)IICB_SETTING_00);
	Read_8bit_HwRegister(	&reg_dump->IICB_Setting[1],
				(unsigned long *)IICB_SETTING_01);
	Read_8bit_HwRegister(	&reg_dump->IICB_Setting[1],
				(unsigned long *)IICB_SETTING_02);

	/* -----	IPMMU ----- */
	dprintk("   Exec %s() IPMMU\n", __func__);
	if (ramdump->ipmmu_setting) {
		len = (IPMMU_SETTING_E - IPMMU_SETTING_S);
		Read_32bit_HwRegisters(	reg_dump->IPMMU_Setting,
					ramdump->ipmmu_setting,
					ramdump->ipmmu_setting + len);

		dprintk("   ioremap OK in %s() : IPMMU #01\n", __func__);
	} else {
		dprintk("   ioremap error in %s() : IPMMU #01\n", __func__);
	}

	dprintk("--- Finished %s() ---\n", __func__);
}



static void ramdump_kmsg_dump_handler(struct kmsg_dumper *dumper,
				      enum kmsg_dump_reason reason,
				      const char *s1, unsigned long l1,
				      const char *s2, unsigned long l2)
{
	dprintk("%s reason = %d\n", __func__, reason);
	switch (reason) {
	case KMSG_DUMP_HALT:
	case KMSG_DUMP_RESTART:
	case KMSG_DUMP_POWEROFF:
		/* No action on ordinary shutdown */
		return;
	case KMSG_DUMP_PANIC:
		/* This is not ready for K34 yet. all the addresses needs to be ioremapped.
		 * Save_HwRegister_To_RAM();*/
	case KMSG_DUMP_EMERG:
	case KMSG_DUMP_OOPS:
	default: /* And anything else we don't recognise */
		break;
	}

	/*
	* Only action we take at the moment is to flush as much cache as
	* we can to try to ensure RAM dumps are useful. Flush everything,
	* because we want to see not only the kmsg dump, but all other data.
	* (We only need a clean, not a flush, but no API for that).
	*/
	flush_cache_all();
	outer_flush_all();
}

static struct kmsg_dumper kmsg_dump_block = {
	.dump = ramdump_kmsg_dump_handler,
};




static int ramdump_die_notifier(struct notifier_block *unused,
		     unsigned long val, void *data)
{
	void *p = NULL;
	struct pt_regs *regs = NULL;
	struct die_args *args = data;
	dprintk("%s val %lu\n", __func__, val);


	/* find the exception register dump location */
	p = ramdump->core_reg_dump_base
			+ ramdump->core_reg_dump_size*smp_processor_id()
			+ ramdump->excp_reg_offset;

	/* Check if exceptions registers are already saved and exit if they are.
	 * Only the first set of exception registers are saved to catch the
	 * problem which occurred first */
	regs = (struct pt_regs *)p;
	if (regs->ARM_cpsr != 0) {
		dprintk("%s exception registers already saved CPSR=%lx\n",
				__func__, regs->ARM_cpsr);
		return NOTIFY_DONE;
		}

	/* save exception registers to the end of dump area */
	memcpy(p, args->regs, sizeof(struct pt_regs));

	return NOTIFY_DONE;
}


static struct notifier_block ramdump_die_notifier_nb = {
	.notifier_call = ramdump_die_notifier,
	.priority = INT_MAX /* we want to be first to save excp regs */
};

static int ramdump_panic_handler(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	void *p = NULL;
	dprintk("%s event %lu\n", __func__, event);

	/* save core registers to the beginning of the dump area */
	p = (void *)ramdump->core_reg_dump_base +
			ramdump->core_reg_dump_size*smp_processor_id();
	dprintk("core reg dump start=%p\n", p);
	save_core_registers(p);

	/* cache cleaning will happen in kmsg dump handler */
	my_emergency_restart();
	
	return NOTIFY_DONE;
}

static struct notifier_block rmc_panic_block = {
	.notifier_call = ramdump_panic_handler,
	.priority = -INT_MAX /* we do restart so be last in list */
};

static void __iomem *remap(struct platform_device *pd, int id)
{
	struct resource *io;
	void __iomem *ret = NULL;

	dprintk("%s pd %p, id %d\n", __func__, pd, id);

	io = platform_get_resource(pd, IORESOURCE_MEM, id);
	if (!io) {
		dev_err(&pd->dev, "IO memory resource error\n");
		return ret;
	}

	io = devm_request_mem_region(&pd->dev, io->start,
					resource_size(io), dev_name(&pd->dev));
	if (!io) {
		dev_err(&pd->dev, "IO memory region request failed\n");
		return ret;
	}

	ret = devm_ioremap(&pd->dev, io->start, resource_size(io));
	if (!ret)
		dev_err(&pd->dev, "%s IO memory remap failed\n", io->name);
	return ret;
}

static int __devinit ramdump_probe(struct platform_device *pd)
{
	int ret = 0;
	struct ramdump_plat_data *pdata = pd->dev.platform_data;

	ramdump = kzalloc(sizeof(*ramdump), GFP_KERNEL);
	if (!ramdump) {
		dev_err(&pd->dev, "%s kzalloc: failed", __func__);
		return -ENOMEM;
	}
	atomic_set(&ramdump->hw_registers_saved, -1);


	dprintk("%s reg_dump_base phys 0x%lx, size %d\n", __func__,
				pdata->reg_dump_base, pdata->reg_dump_size);
	ramdump->reg_dump_base = ioremap_nocache(
			pdata->reg_dump_base, pdata->reg_dump_size);
	if (!ramdump->reg_dump_base) {
		dev_err(&pd->dev, "%s ioremap_nocache: failed", __func__);
		goto ioremap_failed;
	}
	ramdump->reg_dump_size = pdata->reg_dump_size;
	memset(ramdump->reg_dump_base, 0, ramdump->reg_dump_size);
	dprintk("%s reg_dump_base virt 0x%p\n", __func__,
			ramdump->reg_dump_base);

	/* setup register dump area */
	ramdump->core_reg_dump_base = ramdump->reg_dump_base;
	ramdump->core_reg_dump_size = pdata->core_reg_dump_size;
	ramdump->excp_reg_offset =
			ramdump->core_reg_dump_size - sizeof(struct pt_regs);

	/* setup lsi register dump area. it starts after core reg dump areas */
	ramdump->lsi_reg_dump_base = ramdump->reg_dump_base +
			ramdump->core_reg_dump_size*CONFIG_NR_CPUS;
	ramdump->lsi_reg_dump_size = pdata->reg_dump_size -
			ramdump->core_reg_dump_size*CONFIG_NR_CPUS;
	dprintk("%s lsi_reg_dump_base virt 0x%p, size %d\n", __func__,
			(unsigned long *)ramdump->lsi_reg_dump_base,
			ramdump->lsi_reg_dump_size);
	ramdump->hw_register_dump =
			(struct hw_register_dump *)ramdump->lsi_reg_dump_base;

	/* Map hw registers to be dumped
	 * If some of these fails to map, then just continue and
	 * dump only the ranges that we could map */
	ramdump->sbsc_setting_00 = remap(pd, 0);
	ramdump->sbsc_setting_01 = remap(pd, 1);
	ramdump->sbsc_mon_setting = remap(pd, 2);
	ramdump->sbsc_phy_setting_00 = remap(pd, 3);
	ramdump->sbsc_phy_setting_01 = remap(pd, 4);
	ramdump->sbsc_phy_setting_02 = remap(pd, 5);
	ramdump->ipmmu_setting = remap(pd, 6);

	/* register callbacks */
	ret = register_die_notifier(&ramdump_die_notifier_nb);
	if (ret) {
		dev_err(&pd->dev, "%s register_die_notifier: failed %d",
				__func__, ret);
		goto die_notifier_register_failed;
	}

	ret = kmsg_dump_register(&kmsg_dump_block);
	if (ret) {
		dev_err(&pd->dev, "%s kmsg_dump_register: failed %d", __func__,
				ret);
		goto kmsg_notifier_register_failed;
	}

	ret = atomic_notifier_chain_register(&panic_notifier_list,
			&rmc_panic_block);
	if (ret) {
		dev_err(&pd->dev, "%s atomic_notifier_chain_register: failed %d",
				__func__, ret);
		goto panic_notifier_register_failed;
	}

	return 0;

panic_notifier_register_failed:
	if (kmsg_dump_unregister(&kmsg_dump_block))
		dev_err(&pd->dev, "%s: kmsg_dump_unregister failed\n",
				__func__);
kmsg_notifier_register_failed:
	if (unregister_die_notifier(&ramdump_die_notifier_nb))
		dev_err(&pd->dev, "%s: unregister_die_notifier failed\n",
				__func__);
die_notifier_register_failed:
	iounmap(ramdump->core_reg_dump_base);
ioremap_failed:
	kfree(ramdump);
	return ret;
}

static int __exit ramdump_remove(struct platform_device *pd)
{
	int ret = 0;
	dev_err(&pd->dev, "%s ***RAMDUMP feature being removed***\n", __func__);

	ret = atomic_notifier_chain_unregister(&panic_notifier_list,
			&rmc_panic_block);
	if (ret)
		dev_err(&pd->dev, "%s: atomic_notifier_chain_unregister " \
				" failed %d\n", __func__, ret);

	if (kmsg_dump_unregister(&kmsg_dump_block))
		dev_err(&pd->dev, "%s: kmsg_dump_unregister failed\n",
				__func__);

	if (unregister_die_notifier(&ramdump_die_notifier_nb))
		dev_err(&pd->dev, "%s: unregister_die_notifier failed\n",
				__func__);

	iounmap(ramdump->reg_dump_base);
	kfree(ramdump);
	return 0;
}

static struct platform_driver ramdump_pdriver = {
	.probe	=  ramdump_probe,
	.remove	= __exit_p(ramdump_remove),
	.driver	= {
			.name	= "ramdump",
			.owner	= THIS_MODULE,
	},
};

static int __init init_ramdump(void)
{
	int ret = 0;
	dprintk("%s\n", __func__);

	ret = platform_driver_register(&ramdump_pdriver);
	if (ret) {
		printk(KERN_ERR "%s: platform_driver_unregister failed  %d\n",
				__func__, ret);
	}

	return ret;
}


static void __exit exit_ramdump(void)
{
	printk(KERN_ERR "%s ***RAMDUMP feature being removed***\n", __func__);
	platform_driver_unregister(&ramdump_pdriver);
}

module_init(init_ramdump);
module_exit(exit_ramdump);

MODULE_DESCRIPTION("RMC Ramdump");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Renesas Mobile Corp.");
