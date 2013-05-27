/*
 * pmdbg_cpg.c
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

#include "pmdbg_api.h"
#include "pmdbg_hw.h"
#include <asm/page.h>
#include <linux/io.h>
#include <mach/r8a7373.h>

#define CPG_SIZE			0xFFFF
/*Below macros are offsets from the CPG_BASE_ADDR. These are offsets used only
  for PMDBG utility */
#define FRQCRA_OFFSET		0x0000
#define FRQCRB_OFFSET		0x0004
#define FRQCRD_OFFSET		0x00E4
#define PLLECR_OFFSET		0x00D0
#define PLL0CR_OFFSET		0x00D8
#define PLL1CR_OFFSET		0x0028
#define PLL2CR_OFFSET		0x002C
#define PLL3CR_OFFSET		0x00DC
#define PLL0STPCR_OFFSET	0x00F0
#define PLL1STPCR_OFFSET	0x00C8
#define PLL2STPCR_OFFSET	0x00F8
#define PLL3STPCR_OFFSET	0x00FC
#define MSTPSR0_OFFSET		0x0030
#define MSTPSR1_OFFSET		0x0038
#define MSTPSR2_OFFSET		0x0040
#define MSTPSR3_OFFSET		0x0048
#define MSTPSR4_OFFSET		0x004C
#define MSTPSR5_OFFSET		0x003C

static int cpg_init(void);
static void cpg_exit(void);
static void cpg_show(char **);

static char buf_reg[1024];

LOCAL_DECLARE_MOD_SHOW(cpg, cpg_init, cpg_exit, cpg_show);

void cpg_show(char **buf)
{
	void __iomem *vir_addr = NULL;
	char *s = buf_reg;
	FUNC_MSG_IN;
	vir_addr = ioremap_nocache(CPG_BASEPhys, PAGE_SIZE);

	if (!vir_addr) {
		s += sprintf(s, "Failed: No memory\n");
		goto end;
	}

	memset(buf_reg, 0, sizeof(buf_reg));

	s += sprintf(s, "FRQCRA (0x%x): 0x%x\n",
				CPG_BASEPhys + FRQCRA_OFFSET,
				rreg(vir_addr + FRQCRA_OFFSET));
	s += sprintf(s, "FRQCRB (0x%x): 0x%x\n",
				CPG_BASEPhys + FRQCRB_OFFSET,
				rreg(vir_addr + FRQCRB_OFFSET));
	s += sprintf(s, "FRQCRD (0x%x): 0x%x\n",
				CPG_BASEPhys + FRQCRD_OFFSET,
				rreg(vir_addr + FRQCRD_OFFSET));
	s += sprintf(s, "PLLECR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLLECR_OFFSET,
				rreg(vir_addr + PLLECR_OFFSET));
	s += sprintf(s, "PLL0CR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL0CR_OFFSET,
				rreg(vir_addr + PLL0CR_OFFSET));
	s += sprintf(s, "PLL1CR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL1CR_OFFSET,
				rreg(vir_addr + PLL1CR_OFFSET));
	s += sprintf(s, "PLL2CR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL2CR_OFFSET,
				rreg(vir_addr + PLL2CR_OFFSET));
	s += sprintf(s, "PLL3CR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL3CR_OFFSET,
				rreg(vir_addr + PLL3CR_OFFSET));
	s += sprintf(s, "PLL0STPCR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL0STPCR_OFFSET,
				rreg(vir_addr + PLL0STPCR_OFFSET));
	s += sprintf(s, "PLL1STPCR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL1STPCR_OFFSET,
				rreg(vir_addr + PLL1STPCR_OFFSET));
	s += sprintf(s, "PLL2STPCR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL2STPCR_OFFSET,
				rreg(vir_addr + PLL2STPCR_OFFSET));
	s += sprintf(s, "PLL3STPCR (0x%x): 0x%x\n",
				CPG_BASEPhys + PLL3STPCR_OFFSET,
				rreg(vir_addr + PLL3STPCR_OFFSET));
	s += sprintf(s, "MSTPSR0 (0x%x): 0x%x\n",
				CPG_BASEPhys + MSTPSR0_OFFSET,
				rreg(vir_addr + MSTPSR0_OFFSET));
	s += sprintf(s, "MSTPSR1 (0x%x): 0x%x\n",
				CPG_BASEPhys + MSTPSR1_OFFSET,
				rreg(vir_addr + MSTPSR1_OFFSET));
	s += sprintf(s, "MSTPSR2 (0x%x): 0x%x\n",
				CPG_BASEPhys + MSTPSR2_OFFSET,
				rreg(vir_addr + MSTPSR2_OFFSET));
	s += sprintf(s, "MSTPSR3 (0x%x): 0x%x\n",
				CPG_BASEPhys + MSTPSR3_OFFSET,
				rreg(vir_addr + MSTPSR3_OFFSET));
	s += sprintf(s, "MSTPSR4 (0x%x): 0x%x\n",
				CPG_BASEPhys + MSTPSR4_OFFSET,
				rreg(vir_addr + MSTPSR4_OFFSET));
	s += sprintf(s, "MSTPSR5 (0x%x): 0x%x\n",
				CPG_BASEPhys + MSTPSR5_OFFSET,
				rreg(vir_addr + MSTPSR5_OFFSET));

	iounmap(vir_addr);
	vir_addr = NULL;
end:
	*buf = buf_reg;
	FUNC_MSG_OUT;
}


int cpg_init(void)
{
	FUNC_MSG_IN;
	FUNC_MSG_RET(0);
}

void cpg_exit(void)
{
	FUNC_MSG_IN;
	FUNC_MSG_OUT;
}



