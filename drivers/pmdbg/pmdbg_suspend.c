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

	s += sprintf(s, "0x%x: 0x%x\n", CPG_FRQCRA, rreg32(CPG_FRQCRA));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_FRQCRB, rreg32(CPG_FRQCRB));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_FRQCRD, rreg32(CPG_FRQCRD));
	s += sprintf(s, "0x%x: 0x%x\n", SBSC_Freq_APE, rreg32(vir_addr));
	s += sprintf(s, "0x%x: 0x%x\n", SBSC_Freq_BB, rreg32(vir_addr + 0x4));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_BBFRQCRD, rreg32(CPG_BBFRQCRD));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_VCLKCR1, rreg32(CPG_VCLKCR1));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_VCLKCR2, rreg32(CPG_VCLKCR2));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_VCLKCR3, rreg32(CPG_VCLKCR3));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_VCLKCR4, rreg32(CPG_VCLKCR4));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_VCLKCR5, rreg32(CPG_VCLKCR5));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_ZBCKCR, rreg32(CPG_ZBCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SD0CKCR , rreg32(CPG_SD0CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SD1CKCR , rreg32(CPG_SD1CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SD2CKCR, rreg32(CPG_SD2CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_FSIACKCR, rreg32(CPG_FSIACKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_FSIBCKCR, rreg32(CPG_FSIBCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MPCKCR, rreg32(CPG_MPCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SPU2ACKCR, rreg32(CPG_SPU2ACKCR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SPU2VCKCR, rreg32(CPG_SPU2VCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SLIMBCKCR, rreg32(CPG_SLIMBCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_HSICKCR, rreg32(CPG_HSICKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_M4CKCR, rreg32(CPG_M4CKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSITCKCR, rreg32(CPG_DSITCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI0PCKCR, rreg32(CPG_DSI0PCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI1PCKCR, rreg32(CPG_DSI1PCKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI0PHYCR, rreg32(CPG_DSI0PHYCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_DSI1PHYCR, rreg32(CPG_DSI1PHYCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MPMODE, rreg32(CPG_MPMODE));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLLECR, rreg32(CPG_PLLECR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL0CR, rreg32(CPG_PLL0CR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL1CR, rreg32(CPG_PLL1CR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL2CR, rreg32(CPG_PLL2CR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL22CR, rreg32(CPG_PLL22CR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL3CR, rreg32(CPG_PLL3CR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL0STPCR, rreg32(CPG_PLL0STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL1STPCR, rreg32(CPG_PLL1STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL2STPCR, rreg32(CPG_PLL2STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL22STPCR, rreg32(CPG_PLL22STPCR));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PLL3STPCR, rreg32(CPG_PLL3STPCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR0, rreg32(CPG_MSTPSR0));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR1, rreg32(CPG_MSTPSR1));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR2, rreg32(CPG_MSTPSR2));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR3, rreg32(CPG_MSTPSR3));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR4, rreg32(CPG_MSTPSR4));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR5, rreg32(CPG_MSTPSR5));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MSTPSR6, rreg32(CPG_MSTPSR6));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR0, rreg32(CPG_RMSTPCR0));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR1, rreg32(CPG_RMSTPCR1));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR2, rreg32(CPG_RMSTPCR2));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR3, rreg32(CPG_RMSTPCR3));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR4, rreg32(CPG_RMSTPCR4));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR5, rreg32(CPG_RMSTPCR5));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_RMSTPCR6, rreg32(CPG_RMSTPCR6));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR0, rreg32(CPG_SMSTPCR0));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR1, rreg32(CPG_SMSTPCR1));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR2, rreg32(CPG_SMSTPCR2));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR3, rreg32(CPG_SMSTPCR3));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR4, rreg32(CPG_SMSTPCR4));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR5, rreg32(CPG_SMSTPCR5));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SMSTPCR6, rreg32(CPG_SMSTPCR6));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR0, rreg32(CPG_MMSTPCR0));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR1, rreg32(CPG_MMSTPCR1));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR2, rreg32(CPG_MMSTPCR2));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR3, rreg32(CPG_MMSTPCR3));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR4, rreg32(CPG_MMSTPCR4));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR5, rreg32(CPG_MMSTPCR5));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_MMSTPCR6, rreg32(CPG_MMSTPCR6));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_CKSCR, rreg32(CPG_CKSCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_PCLKCR, rreg32(CPG_PCLKCR));
	s += sprintf(s, "0x%x: 0x%x\n", CPG_SCGCR, rreg32(CPG_SCGCR));
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

	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPRMSK, rreg32(SYSC_WUPRMSK));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPSMSK, rreg32(SYSC_WUPSMSK));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPMMSK, rreg32(SYSC_WUPMMSK));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_C4POWCR, rreg32(SYSC_C4POWCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_PSTR, rreg32(SYSC_PSTR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_LPMWUMON, rreg32(SYSC_LPMWUMON));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_EXSTMON2, rreg32(SYSC_EXSTMON2));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_EXSTMON1, rreg32(SYSC_EXSTMON1));
	s += sprintf(s, "0x%x: 0x%x\n",
		SYSC_LPMWUMSKMON, rreg32(SYSC_LPMWUMSKMON));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPRFAC, rreg32(SYSC_WUPRFAC));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPSFAC, rreg32(SYSC_WUPSFAC));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPMFAC, rreg32(SYSC_WUPMFAC));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_RWBCR, rreg32(SYSC_RWBCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_SWBCR, rreg32(SYSC_SWBCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_LPMWUCNT, rreg32(SYSC_LPMWUCNT));
	s += sprintf(s, "0x%x: 0x%x\n",
		SYSC_LPMWUMSKCNT, rreg32(SYSC_LPMWUMSKCNT));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_APSCSTP, rreg32(SYSC_APSCSTP));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_SYCKENMSK, rreg32(SYSC_SYCKENMSK));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_PDNSEL, rreg32(SYSC_PDNSEL));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPRCR, rreg32(SYSC_WUPRCR));
	s += sprintf(s, "0x%x: 0x%x\n", SYSC_WUPSCR, rreg32(SYSC_WUPSCR));
	MSG_INFO("%s", suspend_buf);

	return;
}

static void GPIO_dump_suspend(void)
{
	char *s = suspend_buf;

	sprintf(suspend_buf, ">> %s", __func__);
	MSG_INFO("%s", suspend_buf);

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT0, rreg8(GPIO_PORT0));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT1, rreg8(GPIO_PORT1));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT2, rreg8(GPIO_PORT2));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT3, rreg8(GPIO_PORT3));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT4, rreg8(GPIO_PORT4));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT5, rreg8(GPIO_PORT5));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT6, rreg8(GPIO_PORT6));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT7, rreg8(GPIO_PORT7));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT8, rreg8(GPIO_PORT8));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT9, rreg8(GPIO_PORT9));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT10, rreg8(GPIO_PORT10));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT11, rreg8(GPIO_PORT11));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT12, rreg8(GPIO_PORT12));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT13, rreg8(GPIO_PORT13));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT14, rreg8(GPIO_PORT14));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT15, rreg8(GPIO_PORT15));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT16, rreg8(GPIO_PORT16));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT17, rreg8(GPIO_PORT17));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT18, rreg8(GPIO_PORT18));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT19, rreg8(GPIO_PORT19));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT20, rreg8(GPIO_PORT20));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT21, rreg8(GPIO_PORT21));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT22, rreg8(GPIO_PORT22));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT23, rreg8(GPIO_PORT23));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT24, rreg8(GPIO_PORT24));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT25, rreg8(GPIO_PORT25));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT26, rreg8(GPIO_PORT26));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT27, rreg8(GPIO_PORT27));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT28, rreg8(GPIO_PORT28));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT29, rreg8(GPIO_PORT29));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT30, rreg8(GPIO_PORT30));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT31, rreg8(GPIO_PORT31));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT32, rreg8(GPIO_PORT32));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT33, rreg8(GPIO_PORT33));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT34, rreg8(GPIO_PORT34));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT35, rreg8(GPIO_PORT35));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT36, rreg8(GPIO_PORT36));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT37, rreg8(GPIO_PORT37));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT38, rreg8(GPIO_PORT38));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT39, rreg8(GPIO_PORT39));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT80, rreg8(GPIO_PORT80));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT81, rreg8(GPIO_PORT81));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT82, rreg8(GPIO_PORT82));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT83, rreg8(GPIO_PORT83));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT84, rreg8(GPIO_PORT84));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT85, rreg8(GPIO_PORT85));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT86, rreg8(GPIO_PORT86));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT87, rreg8(GPIO_PORT87));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT88, rreg8(GPIO_PORT88));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT89, rreg8(GPIO_PORT89));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT219, rreg8(GPIO_PORT219));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT90, rreg8(GPIO_PORT90));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT91, rreg8(GPIO_PORT91));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT215, rreg8(GPIO_PORT215));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT216, rreg8(GPIO_PORT216));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT273, rreg8(GPIO_PORT273));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT274, rreg8(GPIO_PORT274));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT275, rreg8(GPIO_PORT275));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT276, rreg8(GPIO_PORT276));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT277, rreg8(GPIO_PORT277));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT261, rreg8(GPIO_PORT261));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT263, rreg8(GPIO_PORT263));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT264, rreg8(GPIO_PORT264));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT265, rreg8(GPIO_PORT265));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT311, rreg8(GPIO_PORT311));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT312, rreg8(GPIO_PORT312));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT218, rreg8(GPIO_PORT218));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT137, rreg8(GPIO_PORT137));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT138, rreg8(GPIO_PORT138));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT140, rreg8(GPIO_PORT140));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT198, rreg8(GPIO_PORT198));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT199, rreg8(GPIO_PORT199));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT200, rreg8(GPIO_PORT200));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT201, rreg8(GPIO_PORT201));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT260, rreg8(GPIO_PORT260));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT262, rreg8(GPIO_PORT262));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT266, rreg8(GPIO_PORT266));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT267, rreg8(GPIO_PORT267));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT268, rreg8(GPIO_PORT268));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT269, rreg8(GPIO_PORT269));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT270, rreg8(GPIO_PORT270));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT271, rreg8(GPIO_PORT271));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT272, rreg8(GPIO_PORT272));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT288, rreg8(GPIO_PORT288));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT289, rreg8(GPIO_PORT289));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT290, rreg8(GPIO_PORT290));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT291, rreg8(GPIO_PORT291));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT292, rreg8(GPIO_PORT292));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT293, rreg8(GPIO_PORT293));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT294, rreg8(GPIO_PORT294));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT295, rreg8(GPIO_PORT295));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT296, rreg8(GPIO_PORT296));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT297, rreg8(GPIO_PORT297));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT298, rreg8(GPIO_PORT298));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT299, rreg8(GPIO_PORT299));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT300, rreg8(GPIO_PORT300));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT301, rreg8(GPIO_PORT301));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT302, rreg8(GPIO_PORT302));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT303, rreg8(GPIO_PORT303));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT304, rreg8(GPIO_PORT304));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT305, rreg8(GPIO_PORT305));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT306, rreg8(GPIO_PORT306));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT307, rreg8(GPIO_PORT307));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT308, rreg8(GPIO_PORT308));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT309, rreg8(GPIO_PORT309));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT310, rreg8(GPIO_PORT310));

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT44, rreg8(GPIO_PORT44));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT45, rreg8(GPIO_PORT45));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT46, rreg8(GPIO_PORT46));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT47, rreg8(GPIO_PORT47));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT48, rreg8(GPIO_PORT48));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT96, rreg8(GPIO_PORT96));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT97, rreg8(GPIO_PORT97));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT98, rreg8(GPIO_PORT98));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT99, rreg8(GPIO_PORT99));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT100, rreg8(GPIO_PORT100));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT101, rreg8(GPIO_PORT101));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT102, rreg8(GPIO_PORT102));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT103, rreg8(GPIO_PORT103));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT104, rreg8(GPIO_PORT104));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT105, rreg8(GPIO_PORT105));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT106, rreg8(GPIO_PORT106));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT107, rreg8(GPIO_PORT107));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT108, rreg8(GPIO_PORT108));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT109, rreg8(GPIO_PORT109));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT110, rreg8(GPIO_PORT110));

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT139, rreg8(GPIO_PORT139));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT320, rreg8(GPIO_PORT320));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT321, rreg8(GPIO_PORT321));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT322, rreg8(GPIO_PORT322));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT323, rreg8(GPIO_PORT323));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT324, rreg8(GPIO_PORT324));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT325, rreg8(GPIO_PORT325));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT326, rreg8(GPIO_PORT326));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT327, rreg8(GPIO_PORT327));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT64, rreg8(GPIO_PORT64));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT65, rreg8(GPIO_PORT65));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT66, rreg8(GPIO_PORT66));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT67, rreg8(GPIO_PORT67));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT68, rreg8(GPIO_PORT68));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT69, rreg8(GPIO_PORT69));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT70, rreg8(GPIO_PORT70));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT71, rreg8(GPIO_PORT71));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT130, rreg8(GPIO_PORT130));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT131, rreg8(GPIO_PORT131));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT203, rreg8(GPIO_PORT203));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT204, rreg8(GPIO_PORT204));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT205, rreg8(GPIO_PORT205));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT206, rreg8(GPIO_PORT206));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT207, rreg8(GPIO_PORT207));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT208, rreg8(GPIO_PORT208));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT209, rreg8(GPIO_PORT209));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT210, rreg8(GPIO_PORT210));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT211, rreg8(GPIO_PORT211));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT212, rreg8(GPIO_PORT212));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT213, rreg8(GPIO_PORT213));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT214, rreg8(GPIO_PORT214));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT217, rreg8(GPIO_PORT217));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT72, rreg8(GPIO_PORT72));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT73, rreg8(GPIO_PORT73));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT74, rreg8(GPIO_PORT74));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT75, rreg8(GPIO_PORT75));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT76, rreg8(GPIO_PORT76));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT77, rreg8(GPIO_PORT77));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT78, rreg8(GPIO_PORT78));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT79, rreg8(GPIO_PORT79));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT40, rreg8(GPIO_PORT40));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT41, rreg8(GPIO_PORT41));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT42, rreg8(GPIO_PORT42));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT43, rreg8(GPIO_PORT43));

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT202, rreg8(GPIO_PORT202));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT141, rreg8(GPIO_PORT141));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT142, rreg8(GPIO_PORT142));

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT133, rreg8(GPIO_PORT133));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT134, rreg8(GPIO_PORT134));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT135, rreg8(GPIO_PORT135));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT136, rreg8(GPIO_PORT136));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT128, rreg8(GPIO_PORT128));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT129, rreg8(GPIO_PORT129));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT224, rreg8(GPIO_PORT224));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT225, rreg8(GPIO_PORT225));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT226, rreg8(GPIO_PORT226));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT227, rreg8(GPIO_PORT227));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT228, rreg8(GPIO_PORT228));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT229, rreg8(GPIO_PORT229));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT230, rreg8(GPIO_PORT230));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT231, rreg8(GPIO_PORT231));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT232, rreg8(GPIO_PORT232));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT233, rreg8(GPIO_PORT233));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT234, rreg8(GPIO_PORT234));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT235, rreg8(GPIO_PORT235));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT236, rreg8(GPIO_PORT236));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT237, rreg8(GPIO_PORT237));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT238, rreg8(GPIO_PORT238));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT239, rreg8(GPIO_PORT239));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT240, rreg8(GPIO_PORT240));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT241, rreg8(GPIO_PORT241));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT242, rreg8(GPIO_PORT242));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT243, rreg8(GPIO_PORT243));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT244, rreg8(GPIO_PORT244));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT245, rreg8(GPIO_PORT245));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT246, rreg8(GPIO_PORT246));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT247, rreg8(GPIO_PORT247));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT248, rreg8(GPIO_PORT248));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT249, rreg8(GPIO_PORT249));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT250, rreg8(GPIO_PORT250));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT251, rreg8(GPIO_PORT251));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT252, rreg8(GPIO_PORT252));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT253, rreg8(GPIO_PORT253));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT254, rreg8(GPIO_PORT254));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT255, rreg8(GPIO_PORT255));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT256, rreg8(GPIO_PORT256));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT257, rreg8(GPIO_PORT257));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT258, rreg8(GPIO_PORT258));
	s += sprintf(s, "0x%x: 0x%x\n", GPIO_PORT259, rreg8(GPIO_PORT259));
	MSG_INFO("%s", suspend_buf);
	s = suspend_buf;

	s += sprintf(s, "0x%x: 0x%x\n", PORTL031_000DR, rreg32(PORTL031_000DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTL063_032DR, rreg32(PORTL063_032DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTL095_064DR, rreg32(PORTL095_064DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTL127_096DR, rreg32(PORTL127_096DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTD159_128DR, rreg32(PORTD159_128DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTR223_192DR, rreg32(PORTR223_192DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTR255_224DR, rreg32(PORTR255_224DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTR287_256DR, rreg32(PORTR287_256DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTR319_288DR, rreg32(PORTR319_288DR));
	s += sprintf(s, "0x%x: 0x%x\n", PORTR351_320DR, rreg32(PORTR351_320DR));
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
		MODEM_PLL5_CR, rreg32(MODEM_PLL5_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_RFCLK_CR, rreg32(MODEM_RFCLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_HPSSCLK_CR, rreg32(MODEM_HPSSCLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_WPSSCLK_CR, rreg32(MODEM_WPSSCLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_FDICLK_CR, rreg32(MODEM_FDICLK_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_RetMemPDRS_CR, rreg32(MODEM_RetMemPDRS_CR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_PSTR, rreg32(MODEM_PSTR));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_MONREG, rreg32(MODEM_MONREG));
	s += sprintf(s, "0x%x: 0x%x\n",
		MODEM_MONREG1, rreg32(MODEM_MONREG1));
	MSG_INFO("%s", suspend_buf);

	s = suspend_buf;
	s += sprintf(s, "0x%x: 0x%x\n", THERMAL_THSCR0, rreg32(THERMAL_THSCR0));
	s += sprintf(s, "0x%x: 0x%x\n", THERMAL_THSCR1, rreg32(THERMAL_THSCR1));
	vir_addr = ioremap_nocache(SBSC_SDPDCR0A, 0x4);
	if (!vir_addr) {
		sprintf(suspend_buf, "Read 0x%x FAILED", SBSC_SDPDCR0A);
		goto fail;
	}
	s += sprintf(s, "0x%x: 0x%x\n", SBSC_SDPDCR0A, rreg32(vir_addr));
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
