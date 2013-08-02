/*
 * pmdbg_suspend.c
 *
 * Copyright (C) 2011-2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/pm.h>
#include <linux/regulator/driver.h>

#define PMDBG_ENABLE_PMIC_D2153

#ifdef PMDBG_ENABLE_PMIC_D2153
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_reg.h>
#include <linux/d2153/hwmon.h>
#include <linux/d2153/core.h>
#endif /* PMDBG_ENABLE_PMIC_D2153 */

#include "pmdbg_suspend.h"
#include "pmdbg_hw.h"

static char pmdbg_enable_dump_suspend;
static char suspend_buf[1024];

LOCAL_DECLARE_MOD(suspend, suspend_init, suspend_exit);

DECLARE_CMD(suspend, suspend_cmd);
DECLARE_CMD(wakelock, wakelock_cmd);
DECLARE_CMD(enable_dump_suspend, enable_dump_suspend_cmd);
DECLARE_CMD(disable_dump_suspend, disable_dump_suspend_cmd);

#ifdef PMDBG_ENABLE_PMIC_D2153
struct pmic_table {
	const char	*name;
	unsigned char	reg;
	unsigned char	mctl_reg;
};

static struct pmic_table pmic_reg_tbl[D2153_NUMBER_OF_REGULATORS] = {
	{
		.name		= "D2153_BUCK_1",
		.reg		= D2153_BUCK2PH_BUCK1_REG,
		.mctl_reg	= D2153_BUCK1_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_2",
		.reg		= D2153_BUCKCORE_BUCK2_REG,
		.mctl_reg	= D2153_BUCK2_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_3",
		.reg		= D2153_BUCKPRO_BUCK3_REG,
		.mctl_reg	= D2153_BUCK3_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_4",
		.reg		= D2153_BUCKMEM_BUCK4_REG,
		.mctl_reg	= D2153_BUCK4_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_5",
		.reg		= D2153_BUCKPERI_BUCK5_REG,
		.mctl_reg	= D2153_BUCK5_MCTL_REG,
	},
	{
		.name		= "D2153_BUCK_6",
		.reg		= D2153_BUCKRF_CONF_REG,
		.mctl_reg	= D2153_BUCK_RF_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_1",
		.reg		= D2153_LDO1_REG,
		.mctl_reg	= D2153_LDO1_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_2",
		.reg		= D2153_LDO2_REG,
		.mctl_reg	= D2153_LDO2_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_3",
		.reg		= D2153_LDO3_REG,
		.mctl_reg	= D2153_LDO3_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_4",
		.reg		= D2153_LDO4_REG,
		.mctl_reg	= D2153_LDO4_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_5",
		.reg		= D2153_LDO5_REG,
		.mctl_reg	= D2153_LDO5_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_6",
		.reg		= D2153_LDO6_REG,
		.mctl_reg	= D2153_LDO6_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_7",
		.reg		= D2153_LDO7_REG,
		.mctl_reg	= D2153_LDO7_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_8",
		.reg		= D2153_LDO8_REG,
		.mctl_reg	= D2153_LDO8_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_9",
		.reg		= D2153_LDO9_REG,
		.mctl_reg	= D2153_LDO9_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_10",
		.reg		= D2153_LDO10_REG,
		.mctl_reg	= D2153_LDO10_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_11",
		.reg		= D2153_LDO11_REG,
		.mctl_reg	= D2153_LDO11_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_12",
		.reg		= D2153_LDO12_REG,
		.mctl_reg	= D2153_LDO12_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_13",
		.reg		= D2153_LDO13_REG,
		.mctl_reg	= D2153_LDO13_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_14",
		.reg		= D2153_LDO14_REG,
		.mctl_reg	= D2153_LDO14_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_15",
		.reg		= D2153_LDO15_REG,
		.mctl_reg	= D2153_LDO15_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_16",
		.reg		= D2153_LDO16_REG,
		.mctl_reg	= D2153_LDO16_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_17",
		.reg		= D2153_LDO17_REG,
		.mctl_reg	= D2153_LDO17_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_18",
		.reg		= D2153_LDO18_LDO_VRFANA_REG,
		.mctl_reg	= D2153_LDO18_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_19",
		.reg		= D2153_LDO19_LDO_19_REG,
		.mctl_reg	= D2153_LDO19_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_20",
		.reg		= D2153_LDO20_LDO_20_REG,
		.mctl_reg	= D2153_LDO20_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_AUD1(LDO_21)",
		.reg		= D2153_LDO21_LDO_AUD1_REG,
		.mctl_reg	= D2153_LDO21_MCTL_REG,
	},
	{
		.name		= "D2153_LDO_AUD2(LDO_22)",
		.reg		= D2153_LDO22_LDO_AUD2_REG,
		.mctl_reg	= D2153_LDO22_MCTL_REG,
	},
};
#endif /* PMDBG_ENABLE_PMIC_D2153 */

/* suspend [type]
 * type:
 * - force: force suspend (skip early suspend)
 * */
static int suspend_cmd(char *para, int size)
{
	int ret = 0;
	char item[PAR_SIZE];
	int para_sz = 0;
	int pos = 0;
	struct timeval beforeTime;
	struct timeval afterTime;
	int suspend_time;
	FUNC_MSG_IN;
	para = strim(para);
	ret = get_word(para, size, 0, item, &para_sz);
	pos = ret;
	if (para_sz > 0) {
		ret = strncmp(item, "force", sizeof("force"));
		if (0 == ret) {
			do_gettimeofday(&beforeTime);
			ret = suspend_devices_and_enter(PM_SUSPEND_MEM);
		} else {
			ret = -ENOTSUPP;
			MSG_INFO("No supported");
			goto fail;
		}
	} else {
		do_gettimeofday(&beforeTime);
		ret = pm_suspend(PM_SUSPEND_MEM);
	}
	do_gettimeofday(&afterTime);
	suspend_time = (afterTime.tv_sec -
			beforeTime.tv_sec) * 1000000
			+ (afterTime.tv_usec - beforeTime.tv_usec);

	MSG_INFO("Suspended in %12uus", suspend_time);
fail:
	FUNC_MSG_RET(ret);
}


/* wakelock
 * - no parameter: Display list of active wakelock to console
 * */
static int wakelock_cmd(char *para, int size)
{
	int ret = 0;
	FUNC_MSG_IN;

#ifdef CONFIG_ARCH_R8A7373
	ret = has_wake_lock_no_expire(WAKE_LOCK_SUSPEND);
	if (ret == 0)
		MSG_INFO("No active suspend wakelock");
#else /*!CONFIG_ARCH_R8A7373*/
	ret = has_wake_lock(WAKE_LOCK_SUSPEND);
	if (ret == 0)
		MSG_INFO("No active suspend wakelock");
#endif /*CONFIG_ARCH_R8A7373*/

	FUNC_MSG_RET(0);
}

/* enable_dump_suspend
 * - no parameter:
 * */
static int enable_dump_suspend_cmd(char *para, int size)
{
	char *s = suspend_buf;

	FUNC_MSG_IN;

	pmdbg_enable_dump_suspend = 1;
	s += sprintf(s, "Suspended: dump suspend log output Enabled");
	MSG_INFO("%s", suspend_buf);

	FUNC_MSG_RET(0);
}

/* disable_dump_suspend
 * - no parameter:
 * */
static int disable_dump_suspend_cmd(char *para, int size)
{
	char *s = suspend_buf;

	FUNC_MSG_IN;

	pmdbg_enable_dump_suspend = 0;
	s += sprintf(s, "Suspended: dump suspend log output Disabled");
	MSG_INFO("%s", suspend_buf);

	FUNC_MSG_RET(0);
}

static int suspend_init(void)
{
	FUNC_MSG_IN;
	ADD_CMD(suspend, suspend);
	ADD_CMD(suspend, wakelock);
	ADD_CMD(suspend, enable_dump_suspend);
	ADD_CMD(suspend, disable_dump_suspend);
	pmdbg_enable_dump_suspend = 0;

	FUNC_MSG_RET(0);
}

static void suspend_exit(void)
{
	FUNC_MSG_IN;
	DEL_CMD(suspend, suspend);
	DEL_CMD(suspend, wakelock);
	DEL_CMD(suspend, enable_dump_suspend);
	DEL_CMD(suspend, disable_dump_suspend);

	FUNC_MSG_OUT;
}

void pmdbg_dump_suspend(void)
{
	CPG_dump_suspend();
	SYSC_dump_suspend();
	GPIO_dump_suspend();
	Other_dump_suspend();

	return;
}

static void CPG_dump_suspend(void)
{
	void __iomem *vir_addr = NULL;
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	vir_addr = ioremap(SBSC_Freq_APE, 0x8);
	if (!vir_addr) {
		sprintf(suspend_buf, "FAILED");
		goto fail;
	}

	s += sprintf(s, "0x%x: 0x%x\n", FRQCRA_PHYS, rreg32(FRQCRA));
	s += sprintf(s, "0x%x: 0x%x\n", FRQCRB_PHYS, rreg32(FRQCRB));
	s += sprintf(s, "0x%x: 0x%x\n", FRQCRD_PHYS, rreg32(FRQCRD));
	s += sprintf(s, "0x%x: 0x%x\n", SBSC_Freq_APE, rreg32(vir_addr));
	s += sprintf(s, "0x%x: 0x%x\n", SBSC_Freq_BB, rreg32(vir_addr + 0x4));
	s += sprintf(s, "0x%x: 0x%x\n", BBFRQCRD_PHYS, rreg32(BBFRQCRD));
	s += sprintf(s, "0x%x: 0x%x\n", VCLKCR1_PHYS, rreg32(VCLKCR1));
	s += sprintf(s, "0x%x: 0x%x\n", VCLKCR2_PHYS, rreg32(VCLKCR2));
	s += sprintf(s, "0x%x: 0x%x\n", VCLKCR3_PHYS, rreg32(VCLKCR3));
	s += sprintf(s, "0x%x: 0x%x\n", VCLKCR4Phys, rreg32(VCLKCR4));
	s += sprintf(s, "0x%x: 0x%x\n", VCLKCR5_PHYS, rreg32(VCLKCR5));
	s += sprintf(s, "0x%x: 0x%x\n", ZBCKCR_PHYS, rreg32(ZBCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", SD0CKCR_PHYS , rreg32(SD0CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", SD1CKCR_PHYS , rreg32(SD1CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", SD2CKCR_PHYS, rreg32(SD2CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", FSIACKCR_PHYS, rreg32(FSIACKCR));
	s += sprintf(s, "0x%x: 0x%x\n", FSIBCKCR_PHYS, rreg32(FSIBCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", MPCKCR_PHYS, rreg32(MPCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", SPUACKCR_PHYS, rreg32(SPUACKCR));
	MSG_INFO("%s", suspend_buf);

	/* Done until here */
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", SPUVCKCR_PHYS, rreg32(SPUVCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", SLIMBCKCR_PHYS, rreg32(SLIMBCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", HSICKCR_PHYS, rreg32(HSICKCR));
	s += sprintf(s, "0x%x: 0x%x\n", M4CKCR_PHYS, rreg32(M4CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSITCKCR_PHYS, rreg32(CPG_DSITCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI0PCKCR_PHYS, rreg32(CPG_DSI0PCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI1PCKCR_PHYS, rreg32(CPG_DSI1PCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI0PHYCR_PHYS, rreg32(CPG_DSI0PHYCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI1PHYCR_PHYS, rreg32(CPG_DSI1PHYCR));
	s += sprintf(s, "0x%x: 0x%x\n", MPMODE_PHYS, rreg32(MPMODE));
	s += sprintf(s, "0x%x: 0x%x\n", PLLECRPhys, rreg32(PLLECR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL0CR_PHYS, rreg32(PLL0CR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL1CR_PHYS, rreg32(PLL1CR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL2CR_PHYS, rreg32(CPG_PLL2CR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL22CR_PHYS, rreg32(PLL22CR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL3CR_PHYS, rreg32(PLL3CR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL0STPCR_PHYS, rreg32(PLL0STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL1STPCR_PHYS, rreg32(PLL1STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL2STPCR_PHYS, rreg32(PLL2STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", PLL22STPCR_PHYS, rreg32(PLL22STPCR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", PLL3STPCR_PHYS, rreg32(PLL3STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR0Phys, rreg32(MSTPSR0));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR1_PHYS, rreg32(MSTPSR1));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR2Phys, rreg32(MSTPSR2));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR3_PHYS, rreg32(MSTPSR3));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR4_PHYS, rreg32(MSTPSR4));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR5_PHYS, rreg32(MSTPSR5));
	s += sprintf(s, "0x%x: 0x%x\n", MSTPSR6_PHYS, rreg32(MSTPSR6));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR0Phys, rreg32(RMSTPCR0));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR1_PHYS, rreg32(RMSTPCR1));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR2Phys, rreg32(RMSTPCR2));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR3_PHYS, rreg32(RMSTPCR3));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR4_PHYS, rreg32(RMSTPCR4));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR5_PHYS, rreg32(RMSTPCR5));
	s += sprintf(s, "0x%x: 0x%x\n", RMSTPCR6_PHYS, rreg32(RMSTPCR6));

	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR0_PHYS, rreg32(SMSTPCR0));
	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR1_PHYS, rreg32(SMSTPCR1));
	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR2Phys, rreg32(SMSTPCR2));
	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR3_PHYS, rreg32(SMSTPCR3));
	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR4_PHYS, rreg32(SMSTPCR4));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR5_PHYS, rreg32(SMSTPCR5));
	s += sprintf(s, "0x%x: 0x%x\n", SMSTPCR6_PHYS, rreg32(SMSTPCR6));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR0_PHYS, rreg32(MMSTPCR0));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR1_PHYS, rreg32(MMSTPCR1));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR2_PHYS, rreg32(MMSTPCR2));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR3_PHYS, rreg32(MMSTPCR3));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR4_PHYS, rreg32(MMSTPCR4));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR5_PHYS, rreg32(MMSTPCR5));
	s += sprintf(s, "0x%x: 0x%x\n", MMSTPCR6_PHYS, rreg32(MMSTPCR6));
	s += sprintf(s, "0x%x: 0x%x\n", CKSCR_PHYS, rreg32(CKSCR));
	s += sprintf(s, "0x%x: 0x%x\n", PCLKCR_PHYS, rreg32(PCLKCR));
	s += sprintf(s, "0x%x: 0x%x\n", ZDIVCR5_PHYS, rreg32(ZDIVCR5)); /* You may have to change this */
	MSG_INFO("%s", suspend_buf);

	iounmap(vir_addr);

	return;

fail:
	MSG_INFO("%s", suspend_buf);
	return;
}

static void SYSC_dump_suspend(void)
{
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	s += sprintf(s, "0x%x: 0x%x\n", WUPRMSK_PHYS, rreg32(WUPRMSK));
	s += sprintf(s, "0x%x: 0x%x\n", WUPSMSK_PHYS, rreg32(WUPSMSK));
	s += sprintf(s, "0x%x: 0x%x\n", WUPMMSK_PHYS, rreg32(WUPMMSK));
	s += sprintf(s, "0x%x: 0x%x\n", C4POWCRPhys, rreg32(C4POWCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_PSTR_PHYS, rreg32(SYSC_PSTR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_LPMWUMON_PHYS, rreg32(SYSC_LPMWUMON));
	s += sprintf(s, "0x%x: 0x%x\n", EXSTMON2_PHYS, rreg32(EXSTMON2));
	s += sprintf(s, "0x%x: 0x%x\n", EXSTMON1_PHYS, rreg32(EXSTMON1));
	s += sprintf(s, "0x%x: 0x%x\n",
		SYSC_LPMWUMSKMON_PHYS, rreg32(SYSC_LPMWUMSKMON));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPRFAC_PHYS, rreg32(SYSC_WUPRFAC));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPSFAC_PHYS, rreg32(SYSC_WUPSFAC));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPMFAC_PHYS, rreg32(SYSC_WUPMFAC));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_RWBCR_PHYS, rreg32(SYSC_RWBCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_SWBCR_PHYS, rreg32(SYSC_SWBCR));
	s += sprintf(s, "0x%x: 0x%x\n", LPMWUCNT_PHYS, rreg32(LPMWUCNT));
	s += sprintf(s, "0x%x: 0x%x\n",
		LPMWUMSKCNT_PHYS, rreg32(LPMWUMSKCNT));
	s += sprintf(s, "0x%x: 0x%x\n", APSCSTPPhys, rreg32(APSCSTP));
	s += sprintf(s, "0x%x: 0x%x\n", SYCKENMSKPhys, rreg32(SYCKENMSK));
	s += sprintf(s, "0x%x: 0x%x\n", PDNSELPhys, rreg32(PDNSEL));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPRCR_PHYS, rreg32(SYSC_WUPRCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPSCR_PHYS, rreg32(SYSC_WUPSCR));
	MSG_INFO("%s", suspend_buf);

	return;
}

static void GPIO_dump_suspend(void)
{
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	s += sprintf(s, "0x%x: 0x%x\n", PORT0_CR_PHYS, rreg8(PORT0_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT1_CR_PHYS, rreg8(PORT1_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT2_CR_PHYS, rreg8(PORT2_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT3_CR_PHYS, rreg8(PORT3_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT4_CR_PHYS, rreg8(PORT4_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT5_CR_PHYS, rreg8(PORT5_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT6_CR_PHYS, rreg8(PORT6_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT7_CR_PHYS, rreg8(PORT7_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT8_CR_PHYS, rreg8(PORT8_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT9_CR_PHYS, rreg8(PORT9_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT10_CR_PHYS, rreg8(PORT10_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT11_CR_PHYS, rreg8(PORT11_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT12_CR_PHYS, rreg8(PORT12_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT13_CR_PHYS, rreg8(PORT13_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT14_CR_PHYS, rreg8(PORT14_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT15_CR_PHYS, rreg8(PORT15_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT16_CR_PHYS, rreg8(PORT16_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT17_CR_PHYS, rreg8(PORT17_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT18_CR_PHYS, rreg8(PORT18_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT19_CR_PHYS, rreg8(PORT19_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", PORT20_CR_PHYS, rreg8(PORT20_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT21_CR_PHYS, rreg8(PORT21_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT22_CR_PHYS, rreg8(PORT22_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT23_CR_PHYS, rreg8(PORT23_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT24_CR_PHYS, rreg8(PORT24_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT25_CR_PHYS, rreg8(PORT25_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT26_CR_PHYS, rreg8(PORT26_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT27_CR_PHYS, rreg8(PORT27_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT28_CR_PHYS, rreg8(PORT28_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT29_CR_PHYS, rreg8(PORT29_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT30_CR_PHYS, rreg8(PORT30_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT31_CR_PHYS, rreg8(PORT31_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT32_CR_PHYS, rreg8(PORT32_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT33_CR_PHYS, rreg8(PORT33_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT34_CR_PHYS, rreg8(PORT34_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT35_CR_PHYS, rreg8(PORT35_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT36_CR_PHYS, rreg8(PORT36_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT37_CR_PHYS, rreg8(PORT37_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT38_CR_PHYS, rreg8(PORT38_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT39_CR_PHYS, rreg8(PORT39_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT80_CR_PHYS, rreg8(PORT80_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT81_CR_PHYS, rreg8(PORT81_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT82_CR_PHYS, rreg8(PORT82_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT83_CR_PHYS, rreg8(PORT83_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT84_CR_PHYS, rreg8(PORT84_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT85_CR_PHYS, rreg8(PORT85_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT86_CR_PHYS, rreg8(PORT86_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT87_CR_PHYS, rreg8(PORT87_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT88_CR_PHYS, rreg8(PORT88_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT89_CR_PHYS, rreg8(PORT89_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT219_CR_PHYS, rreg8(PORT219_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT90_CR_PHYS, rreg8(PORT90_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT91_CR_PHYS, rreg8(PORT91_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT215_CR_PHYS, rreg8(PORT215_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT216_CR_PHYS, rreg8(PORT216_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT273_CR_PHYS, rreg8(PORT273_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT274_CR_PHYS, rreg8(PORT274_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT275_CR_PHYS, rreg8(PORT275_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT276_CR_PHYS, rreg8(PORT276_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT277_CR_PHYS, rreg8(PORT277_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT261_CR_PHYS, rreg8(PORT261_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT263_CR_PHYS, rreg8(PORT263_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT264_CR_PHYS, rreg8(PORT264_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT265_CR_PHYS, rreg8(PORT265_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT311_CR_PHYS, rreg8(PORT311_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT312_CR_PHYS, rreg8(PORT312_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT218_CR_PHYS, rreg8(PORT218_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT137_CR_PHYS, rreg8(PORT137_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT138_CR_PHYS, rreg8(PORT138_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT140_CR_PHYS, rreg8(PORT140_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT198_CR_PHYS, rreg8(PORT198_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT199_CR_PHYS, rreg8(PORT199_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT200_CR_PHYS, rreg8(PORT200_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT201_CR_PHYS, rreg8(PORT201_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT260_CR_PHYS, rreg8(PORT260_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT262_CR_PHYS, rreg8(PORT262_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT266_CR_PHYS, rreg8(PORT266_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT267_CR_PHYS, rreg8(PORT267_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT268_CR_PHYS, rreg8(PORT268_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT269_CR_PHYS, rreg8(PORT269_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT270_CR_PHYS, rreg8(PORT270_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT271_CR_PHYS, rreg8(PORT271_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT272_CR_PHYS, rreg8(PORT272_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", PORT288_CR_PHYS, rreg8(PORT288_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT289_CR_PHYS, rreg8(PORT289_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT290_CR_PHYS, rreg8(PORT290_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT291_CR_PHYS, rreg8(PORT291_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT292_CR_PHYS, rreg8(PORT292_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT293_CR_PHYS, rreg8(PORT293_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT294_CR_PHYS, rreg8(PORT294_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT295_CR_PHYS, rreg8(PORT295_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT296_CR_PHYS, rreg8(PORT296_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT297_CR_PHYS, rreg8(PORT297_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT298_CR_PHYS, rreg8(PORT298_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT299_CR_PHYS, rreg8(PORT299_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT300_CR_PHYS, rreg8(PORT300_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT301_CR_PHYS, rreg8(PORT301_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT302_CR_PHYS, rreg8(PORT302_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT303_CR_PHYS, rreg8(PORT303_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT304_CR_PHYS, rreg8(PORT304_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT305_CR_PHYS, rreg8(PORT305_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT306_CR_PHYS, rreg8(PORT306_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT307_CR_PHYS, rreg8(PORT307_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT308_CR_PHYS, rreg8(PORT308_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT309_CR_PHYS, rreg8(PORT309_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT310_CR_PHYS, rreg8(PORT310_CR));

	s += sprintf(s, "0x%x: 0x%x\n", PORT44_CR_PHYS, rreg8(PORT44_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT45_CR_PHYS, rreg8(PORT45_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT46_CR_PHYS, rreg8(PORT46_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT47_CR_PHYS, rreg8(PORT47_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT48_CR_PHYS, rreg8(PORT48_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT96_CR_PHYS, rreg8(PORT96_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT97_CR_PHYS, rreg8(PORT97_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT98_CR_PHYS, rreg8(PORT98_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", PORT99_CR_PHYS, rreg8(PORT99_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT100_CR_PHYS, rreg8(PORT100_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT101_CR_PHYS, rreg8(PORT101_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT102_CR_PHYS, rreg8(PORT102_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT103_CR_PHYS, rreg8(PORT103_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT104_CR_PHYS, rreg8(PORT104_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT105_CR_PHYS, rreg8(PORT105_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT106_CR_PHYS, rreg8(PORT106_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT107_CR_PHYS, rreg8(PORT107_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT108_CR_PHYS, rreg8(PORT108_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT109_CR_PHYS, rreg8(PORT109_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT110_CR_PHYS, rreg8(PORT110_CR));

	s += sprintf(s, "0x%x: 0x%x\n", PORT139_CR_PHYS, rreg8(PORT139_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT320_CR_PHYS, rreg8(PORT320_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT321_CR_PHYS, rreg8(PORT321_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT322_CR_PHYS, rreg8(PORT322_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT323_CR_PHYS, rreg8(PORT323_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT324_CR_PHYS, rreg8(PORT324_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT325_CR_PHYS, rreg8(PORT325_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT326_CR_PHYS, rreg8(PORT326_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT327_CR_PHYS, rreg8(PORT327_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT64_CR_PHYS, rreg8(PORT64_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT65_CR_PHYS, rreg8(PORT65_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT66_CR_PHYS, rreg8(PORT66_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT67_CR_PHYS, rreg8(PORT67_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT68_CR_PHYS, rreg8(PORT68_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT69_CR_PHYS, rreg8(PORT69_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT70_CR_PHYS, rreg8(PORT70_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT71_CR_PHYS, rreg8(PORT71_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT130_CR_PHYS, rreg8(PORT130_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT131_CR_PHYS, rreg8(PORT131_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT203_CR_PHYS, rreg8(PORT203_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT204_CR_PHYS, rreg8(PORT204_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT205_CR_PHYS, rreg8(PORT205_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT206_CR_PHYS, rreg8(PORT206_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT207_CR_PHYS, rreg8(PORT207_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT208_CR_PHYS, rreg8(PORT208_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT209_CR_PHYS, rreg8(PORT209_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT210_CR_PHYS, rreg8(PORT210_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT211_CR_PHYS, rreg8(PORT211_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT212_CR_PHYS, rreg8(PORT212_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT213_CR_PHYS, rreg8(PORT213_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT214_CR_PHYS, rreg8(PORT214_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT217_CR_PHYS, rreg8(PORT217_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT72_CR_PHYS, rreg8(PORT72_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT73_CR_PHYS, rreg8(PORT73_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT74_CR_PHYS, rreg8(PORT74_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT75_CR_PHYS, rreg8(PORT75_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT76_CR_PHYS, rreg8(PORT76_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT77_CR_PHYS, rreg8(PORT77_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT78_CR_PHYS, rreg8(PORT78_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT79_CR_PHYS, rreg8(PORT79_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT40_CR_PHYS, rreg8(PORT40_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT41_CR_PHYS, rreg8(PORT41_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT42_CR_PHYS, rreg8(PORT42_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT43_CR_PHYS, rreg8(PORT43_CR));

	s += sprintf(s, "0x%x: 0x%x\n", PORT202_CR_PHYS, rreg8(PORT202_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT141_CR_PHYS, rreg8(PORT141_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT142_CR_PHYS, rreg8(PORT142_CR));

	s += sprintf(s, "0x%x: 0x%x\n", PORT133_CR_PHYS, rreg8(PORT133_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT134_CR_PHYS, rreg8(PORT134_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT135_CR_PHYS, rreg8(PORT135_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT136_CR_PHYS, rreg8(PORT136_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORT128_CR_PHYS, rreg8(PORT128_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT129_CR_PHYS, rreg8(PORT129_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT224_CR_PHYS, rreg8(PORT224_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT225_CR_PHYS, rreg8(PORT225_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT226_CR_PHYS, rreg8(PORT226_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT227_CR_PHYS, rreg8(PORT227_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT228_CR_PHYS, rreg8(PORT228_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT229_CR_PHYS, rreg8(PORT229_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT230_CR_PHYS, rreg8(PORT230_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT231_CR_PHYS, rreg8(PORT231_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT232_CR_PHYS, rreg8(PORT232_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT233_CR_PHYS, rreg8(PORT233_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT234_CR_PHYS, rreg8(PORT234_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT235_CR_PHYS, rreg8(PORT235_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT236_CR_PHYS, rreg8(PORT236_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT237_CR_PHYS, rreg8(PORT237_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT238_CR_PHYS, rreg8(PORT238_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT239_CR_PHYS, rreg8(PORT239_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT240_CR_PHYS, rreg8(PORT240_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT241_CR_PHYS, rreg8(PORT241_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", PORT242_CR_PHYS, rreg8(PORT242_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT243_CR_PHYS, rreg8(PORT243_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT244_CR_PHYS, rreg8(PORT244_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT245_CR_PHYS, rreg8(PORT245_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT246_CR_PHYS, rreg8(PORT246_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT247_CR_PHYS, rreg8(PORT247_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT248_CR_PHYS, rreg8(PORT248_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT249_CR_PHYS, rreg8(PORT249_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT250_CR_PHYS, rreg8(PORT250_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT251_CR_PHYS, rreg8(PORT251_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT252_CR_PHYS, rreg8(PORT252_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT253_CR_PHYS, rreg8(PORT253_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT254_CR_PHYS, rreg8(PORT254_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT255_CR_PHYS, rreg8(PORT255_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT256_CR_PHYS, rreg8(PORT256_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT257_CR_PHYS, rreg8(PORT257_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT258_CR_PHYS, rreg8(PORT258_CR));
	s += sprintf(s, "0x%x: 0x%x\n", PORT259_CR_PHYS, rreg8(PORT259_CR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL031_000DR_PHYS, rreg32(GPIO_PORTL031_000DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL063_032DR_PHYS, rreg32(GPIO_PORTL063_032DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL095_064DR_PHYS, rreg32(GPIO_PORTL095_064DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL127_096DR_PHYS, rreg32(GPIO_PORTL127_096DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL159_128DR_PHYS, rreg32(GPIO_PORTL159_128DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL223_192DR_PHYS, rreg32(GPIO_PORTL223_192DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL255_224DR_PHYS, rreg32(GPIO_PORTL255_224DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL287_256DR_PHYS, rreg32(GPIO_PORTL287_256DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL319_288DR_PHYS, rreg32(GPIO_PORTL319_288DR));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORTL351_320DR_PHYS, rreg32(GPIO_PORTL351_320DR));
	MSG_INFO("%s", suspend_buf);

	return;
}

static void Other_dump_suspend(void)
{
	char *s = suspend_buf;
	void __iomem *vir_addr = NULL;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_PLL5_CR_PHYS, rreg32(MODEM_PLL5_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_RFCLK_CR_PHYS, rreg32(MODEM_RFCLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_HPSSCLK_CR_PHYS, rreg32(MODEM_HPSSCLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_WPSSCLK_CR_PHYS, rreg32(MODEM_WPSSCLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_FDICLK_CR_PHYS, rreg32(MODEM_FDICLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_RETMEMPDRS_CR_PHYS, rreg32(MODEM_RETMEMPDRS_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_PSTR_PHYS, rreg32(MODEM_PSTR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_MONREG_PHYS, rreg32(MODEM_MONREG));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_MONREG1_PHYS, rreg32(MODEM_MONREG1));
	MSG_INFO("%s", suspend_buf);

	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", THERMAL_THSCR0_PHYS, rreg32(THERMAL_THSCR0));
	s += sprintf(s, "0x%x: 0x%x\n", THERMAL_THSCR1_PHYS, rreg32(THERMAL_THSCR1));

	vir_addr = ioremap_nocache(SBSC_SDPDCR0APhys, 0x4);
	if (!vir_addr) {
		sprintf(suspend_buf, "Read 0x%x FAILED", SBSC_SDPDCR0APhys);
		goto fail;
	}
	s += sprintf(s, "0x%x: 0x%x\n", SBSC_SDPDCR0APhys, rreg32(vir_addr));
	iounmap(vir_addr);

	MSG_INFO("%s", suspend_buf);

	return;
fail:
	MSG_INFO("%s", suspend_buf);
	return;
}

void pmdbg_pmic_dump_suspend(void *pmic_data)
{
#ifdef PMDBG_ENABLE_PMIC_D2153
	int i, ret;
	unsigned char val;
	char *s = suspend_buf;
	struct d2153 *d2153 = (struct d2153 *)pmic_data;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	if (NULL == d2153) {
		sprintf(suspend_buf, "pmic_data is NULL");
		goto fail;
	}

	/* Read BUCK, LDO  */
	for (i = D2153_BUCK_1; i < D2153_NUMBER_OF_REGULATORS; i++) {
		ret = d2153_reg_read(d2153, pmic_reg_tbl[i].reg, &val);
		if (0 > ret) {
			sprintf(suspend_buf, "Read 0x%04x FAILED",
						pmic_reg_tbl[i].reg);
			goto fail;
		}

		s += sprintf(s, "0x%04x: 0x%02x\n",
						pmic_reg_tbl[i].reg, val);
	}
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	/* Read MCTL settings */
	for (i = D2153_BUCK_1; i < D2153_NUMBER_OF_REGULATORS; i++) {
		ret = d2153_reg_read(d2153, pmic_reg_tbl[i].mctl_reg, &val);
		if (0 > ret) {
			sprintf(suspend_buf, "Read 0x%04x FAILED",
						pmic_reg_tbl[i].mctl_reg);
			goto fail;
		}

		s += sprintf(s, "0x%04x: 0x%02x\n",
						pmic_reg_tbl[i].mctl_reg, val);
	}
	MSG_INFO("%s", suspend_buf);

	return;

fail:
	MSG_INFO("%s", suspend_buf);
	return;

#else

	/* D2153 Excepting Not Support  */

	return;

#endif /* PMDBG_ENABLE_PMIC_D2153 */
}

char pmdbg_get_enable_dump_suspend(void)
{
	/* enable:1, disable:0 */
	return pmdbg_enable_dump_suspend;
}
