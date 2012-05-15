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
#include <asm/io.h>


#define CPG_BASE_PHYS		0xE6150000
#define CPG_SIZE			0xFFFF
#define FRQCRA		0x0000
#define FRQCRB		0x0004
#define FRQCRD		0x00E4
#define PLLECR		0x00D0
#define PLL0CR		0x00D8
#define PLL1CR		0x0028
#define PLL2CR		0x002C
#define PLL3CR		0x00DC
#define PLL0STPCR	0x00F0
#define PLL1STPCR	0x00C8
#define PLL2STPCR	0x00F8
#define PLL3STPCR	0x00FC
#define MSTPSR0		0x0030
#define MSTPSR1		0x0038
#define MSTPSR2		0x0040
#define MSTPSR3		0x0048
#define MSTPSR4		0x004C
#define MSTPSR5		0x003C

int cpg_init(void);
void cpg_exit(void);
void cpg_show(char**);

static char buf_reg[1024];

LOCAL_DECLARE_MOD_SHOW(cpg, cpg_init, cpg_exit, cpg_show);

void cpg_show(char** buf)
{
	void __iomem *vir_addr = NULL;
	char* s = buf_reg;
	FUNC_MSG_IN;
	vir_addr = ioremap_nocache(CPG_BASE_PHYS, PAGE_SIZE);

	if (!vir_addr){
		s+= sprintf(s, "Failed: No memory\n");
		goto end;
	}
	
	memset(buf_reg, 0, sizeof(buf_reg));
	
	s+= sprintf(s, "FRQCRA (0x%x): 0x%x\n", CPG_BASE_PHYS + FRQCRA,
											rreg(vir_addr + FRQCRA));
	s+= sprintf(s, "FRQCRB (0x%x): 0x%x\n", CPG_BASE_PHYS + FRQCRB,
											rreg(vir_addr + FRQCRB));
	s+= sprintf(s, "FRQCRD (0x%x): 0x%x\n", CPG_BASE_PHYS + FRQCRD,
											rreg(vir_addr + FRQCRD));
	s+= sprintf(s, "PLLECR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLLECR,
											rreg(vir_addr + PLLECR));
	s+= sprintf(s, "PLL0CR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL0CR,
											rreg(vir_addr + PLL0CR));
	s+= sprintf(s, "PLL1CR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL1CR,
											rreg(vir_addr + PLL1CR));
	s+= sprintf(s, "PLL2CR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL2CR,
											rreg(vir_addr + PLL2CR));
	s+= sprintf(s, "PLL3CR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL3CR,
											rreg(vir_addr + PLL3CR));
	s+= sprintf(s, "PLL0STPCR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL0STPCR,
												rreg(vir_addr + PLL0STPCR));
	s+= sprintf(s, "PLL1STPCR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL1STPCR,
												rreg(vir_addr + PLL1STPCR));
	s+= sprintf(s, "PLL2STPCR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL2STPCR,
												rreg(vir_addr + PLL2STPCR));
	s+= sprintf(s, "PLL3STPCR (0x%x): 0x%x\n", CPG_BASE_PHYS + PLL3STPCR,
												rreg(vir_addr + PLL3STPCR));
	s+= sprintf(s, "MSTPSR0 (0x%x): 0x%x\n", CPG_BASE_PHYS + MSTPSR0,
											rreg(vir_addr + MSTPSR0));
	s+= sprintf(s, "MSTPSR1 (0x%x): 0x%x\n", CPG_BASE_PHYS + MSTPSR1,
											rreg(vir_addr + MSTPSR1));
	s+= sprintf(s, "MSTPSR2 (0x%x): 0x%x\n", CPG_BASE_PHYS + MSTPSR2,
											rreg(vir_addr + MSTPSR2));
	s+= sprintf(s, "MSTPSR3 (0x%x): 0x%x\n", CPG_BASE_PHYS + MSTPSR3,
											rreg(vir_addr + MSTPSR3));
	s+= sprintf(s, "MSTPSR4 (0x%x): 0x%x\n", CPG_BASE_PHYS + MSTPSR4,
											rreg(vir_addr + MSTPSR4));
	s+= sprintf(s, "MSTPSR5 (0x%x): 0x%x\n", CPG_BASE_PHYS + MSTPSR5,
											rreg(vir_addr + MSTPSR5));

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



