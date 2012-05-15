/*
 * pmdbg_sysc.c
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


#define SYSC_BASE_PHYS		0xE6180000
#define SYSC_SIZE			0xFFFF
#define SBAR_OFFSET			0x0020
#define PSTR_OFFSET			0x0080
#define WUPSFAC_OFFSET		0x0098
#define WUPRFAC_OFFSET		0x0094
#define SRSTFR_OFFSET		0x00B4
#define WUPSMSK_OFFSET		0x002C
#define WUPRMSK_OFFSET		0x0028
#define SWUCR_OFFSET		0x0014
#define RWUCR_OFFSET		0x0010
#define SWBCR_OFFSET		0x0204
#define RWBCR_OFFSET		0x0200
#define SYCKENMSK_OFFSET	0x024C
#define APSCSTP_OFFSET		0x0234
#define EXMSKCNT1_OFFSET	0x0214
#define EXMSKCNT2_OFFSET	0x0210
#define WUPRCR_OFFSET		0x0260
#define WUPSCR_OFFSET		0x0264

int sysc_init(void);
void sysc_exit(void);
void sysc_show(char**);

static char buf_reg[1024];

LOCAL_DECLARE_MOD_SHOW(sysc, sysc_init, sysc_exit, sysc_show);


void sysc_show(char** buf)
{
	void __iomem *vir_addr = NULL;
	char* s = buf_reg;
	FUNC_MSG_IN;
	vir_addr = ioremap_nocache(SYSC_BASE_PHYS, PAGE_SIZE);
	if (!vir_addr){
		s+= sprintf(s, "Failed: No memory\n");
		goto end;
	}
	memset(buf_reg, 0, sizeof(buf_reg));
	
	s+= sprintf(s, "SBAR (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + SBAR_OFFSET,
											rreg(vir_addr + SBAR_OFFSET));
	s+= sprintf(s, "PSTR (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + PSTR_OFFSET,
											rreg(vir_addr + PSTR_OFFSET));
	s+= sprintf(s, "WUPSFAC (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + WUPSFAC_OFFSET,
												rreg(vir_addr + WUPSFAC_OFFSET));
	s+= sprintf(s, "WUPRFAC (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + WUPRFAC_OFFSET,
												rreg(vir_addr + WUPRFAC_OFFSET));
	s+= sprintf(s, "SRSTFR (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + SRSTFR_OFFSET,
												rreg(vir_addr + SRSTFR_OFFSET));
	s+= sprintf(s, "WUPSMSK (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + WUPSMSK_OFFSET,
												rreg(vir_addr + WUPSMSK_OFFSET));
	s+= sprintf(s, "WUPRMSK (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + WUPRMSK_OFFSET,
												rreg(vir_addr + WUPRMSK_OFFSET));
	s+= sprintf(s, "SWUCR (0x%x): 0x%x\n", 		SYSC_BASE_PHYS + SWUCR_OFFSET,
												rreg(vir_addr + SWUCR_OFFSET));
	s+= sprintf(s, "RWUCR (0x%x): 0x%x\n", 		SYSC_BASE_PHYS + RWUCR_OFFSET,
												rreg(vir_addr + RWUCR_OFFSET));
	s+= sprintf(s, "SWBCR (0x%x): 0x%x\n", 		SYSC_BASE_PHYS + SWBCR_OFFSET,
												rreg(vir_addr + SWBCR_OFFSET));
	s+= sprintf(s, "RWBCR (0x%x): 0x%x\n", 		SYSC_BASE_PHYS + RWBCR_OFFSET,
												rreg(vir_addr + RWBCR_OFFSET));
	s+= sprintf(s, "SYCKENMSK (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + SYCKENMSK_OFFSET,
												rreg(vir_addr + SYCKENMSK_OFFSET));
	s+= sprintf(s, "APSCSTP (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + APSCSTP_OFFSET,
												rreg(vir_addr + APSCSTP_OFFSET));
	s+= sprintf(s, "EXMSKCNT1 (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + EXMSKCNT1_OFFSET,
												rreg(vir_addr + EXMSKCNT1_OFFSET));
	s+= sprintf(s, "EXMSKCNT2 (0x%x): 0x%x\n", 	SYSC_BASE_PHYS + EXMSKCNT2_OFFSET,
												rreg(vir_addr + EXMSKCNT2_OFFSET));
	s+= sprintf(s, "WUPRCR (0x%x): 0x%x\n", SYSC_BASE_PHYS + WUPRCR_OFFSET,
											rreg(vir_addr + WUPRCR_OFFSET));
	s+= sprintf(s, "WUPSCR (0x%x): 0x%x\n", SYSC_BASE_PHYS + WUPSCR_OFFSET,
											rreg(vir_addr + WUPSCR_OFFSET));

	iounmap(vir_addr);
	vir_addr = NULL;
end:
	*buf = buf_reg;
	FUNC_MSG_OUT;
}


int sysc_init(void)
{
	FUNC_MSG_IN;

	FUNC_MSG_RET(0);
}

void sysc_exit(void)
{
	FUNC_MSG_IN;
	FUNC_MSG_OUT;
}



