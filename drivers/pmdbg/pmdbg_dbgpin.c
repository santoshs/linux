/*
 * pmdbg_dbgpin.c
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
#include <asm/errno.h>
#include <mach/pm.h>
#include "pmdbg_hw.h"
#ifdef CONFIG_ARM_TZ
#include <sec_hal_cmn.h>
static uint32_t g_dbgreg1 = 0x20000000;
static uint32_t g_dbgreg2 = 0x00;
static uint32_t g_dbgreg3 = 0x00078077;
#endif

#define DBGREG1				(void __iomem *)IO_ADDRESS(0xE6100020)
#define DBGREG3				(void __iomem *)IO_ADDRESS(0xE6100028)
#define DBGREG9				(void __iomem *)IO_ADDRESS(0xE6100040)
#define DBGREG11			(void __iomem *)IO_ADDRESS(0xE6100048)
#define SWUCR				(void __iomem *)IO_ADDRESS(0xE6180014)
#define PSTR				(void __iomem *)IO_ADDRESS(0xE6180080)
#define GPIO_MSEL03CR		(void __iomem *)IO_ADDRESS(0xE6058020)

#define DBGREG9_AID 		0xA5
#define DBGREG9_AID_SHIFT	8
#define DBGREG9_AID_MASK 	0xFF << DBGREG9_AID_SHIFT
#define DBGREG9_KEY_SHIFT	0x0
#define DBGREG9_KEY_MASK	0x1

#define DBGREG1_TRC_MASK 	0x0000E000
#define DBGREG1_TRC_SHIFT	13
#define TRC_MON				0x4

#define DBGREG1_MDA_MASK 	0x000000FF
#define DBGREG1_MDA_SHIFT	0
#define MDA_PA				0x06
#define MDA_CPU				0x05
#define MDA_CLK				0x03

#define MONITOR_NUM			0x23

#define GPIO_MSEL03CR_MSEL15_SHIFT	15
#define GPIO_MSEL03CR_MSEL15_MASK	(1 << GPIO_MSEL03CR_MSEL15_SHIFT)
#define GPIO_MSEL03CR_MSEL15_KEY	0
#define GPIO_MSEL03CR_MSEL15_BSC	1

static u32 dbgreg1_backup = 0;
static int mon_en = 0;
static char buf_reg[1024];

static char mon_name[MONITOR_NUM][32] = {
	[MDA_PA] = "Power area status",
	[MDA_CPU] = "CPU sleep mode",
	[MDA_CLK] = "Clock"
};

static int monitor_cmd(char*, int);
static int monpin_cmd(char*, int);
static int dbgpin_init(void);
static void dbgpin_exit(void);
static void dbgpin_show(char**);

LOCAL_DECLARE_MOD_SHOW(dbgpin, dbgpin_init, dbgpin_exit, dbgpin_show);

DECLARE_CMD(mon, monitor_cmd);
DECLARE_CMD(monpin, monpin_cmd);

/* mon <type>
 * type:
 * - no: restore to original
 * - get: get current monitor function
 * - pa: power area status monitoring
 * - cpu: CPU Sleep Mode monitoring
 * - clk: Clock monitoring
 */
extern int arch_hw_breakpoint_init_late(void);
extern int init_hw_perf_events_late(void);

static int monitor_cmd(char *para, int size)
{
	int ret = 0;
	char item[PAR_SIZE];
	int para_sz = 0;
	int pos = 0;
	char* s = buf_reg;
	u32 reg = 0;
	u32 key = 0;
	u32 trc = 0;
	u32 mda = 0;
	FUNC_MSG_IN;
	para = strim(para);

	ret = get_word(para, size, 0, item, &para_sz);
	if (ret <=0){
		ret = -EINVAL;
		goto fail;
	}
	pos = ret;

	/* no */
	ret = strncmp(item, "no", sizeof("no"));
	if (0 == ret){
		if (mon_en){
			reg = rreg32(DBGREG9);
			key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
			wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
							| DBGREG9_KEY_MASK, 0);
			wreg32(DBGREG1, dbgreg1_backup);
			mon_en = 0;
			s+= sprintf(s, "Monitor is disable\n");
			wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
							| (key << DBGREG9_KEY_SHIFT), 0);
			mreg32(DBGREG9, 0, 
							DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			goto end;
		}
		s+= sprintf(s, "Monitor has not been enable\n");
		goto end;
	}
	/*get*/
	ret = strncmp(item, "get", sizeof("get"));
	if (0 == ret){
		reg = rreg32(DBGREG9);
		key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| DBGREG9_KEY_MASK, 0);
		reg = rreg32(DBGREG1);
		trc = (reg & DBGREG1_TRC_MASK) >> DBGREG1_TRC_SHIFT;
		if (trc == TRC_MON){
			reg = rreg32(DBGREG1);
			mda = (reg & DBGREG1_MDA_MASK) >> DBGREG1_MDA_SHIFT;
			if (mda >= MONITOR_NUM)
				s+= sprintf(s, "Monitor type: Unknown");
			else
				s+= sprintf(s, "Monitor type: %s\n", mon_name[mda]);
		}
		else{
			s+= sprintf(s, "Monitor function is disable\n");
		}
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| (key << DBGREG9_KEY_SHIFT), 0);
		mreg32(DBGREG9, 0, 
				DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		goto end;
	}
	
	/*pa*/
	ret = strncmp(item, "pa", sizeof("pa"));
	if (0 == ret){
		reg = rreg32(DBGREG9);
		key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| DBGREG9_KEY_MASK, 0);
		if (!mon_en){
			reg = rreg32(DBGREG1);
			dbgreg1_backup = reg;
		}
		mreg32(DBGREG1, (DBGREG1_TRC_MASK & (TRC_MON << DBGREG1_TRC_SHIFT))|
						(DBGREG1_MDA_MASK & (MDA_PA << DBGREG1_MDA_SHIFT)), 
						DBGREG1_TRC_MASK | DBGREG1_MDA_MASK);
		s+= sprintf(s, "Power area status monitor is enable\n");
		mon_en = 1;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| (key << DBGREG9_KEY_SHIFT), 0);
		mreg32(DBGREG9, 0, 
				DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		goto end;
	}
	
	/*cpu*/
	ret = strncmp(item, "cpu", sizeof("cpu"));
	if (0 == ret){
		reg = rreg32(DBGREG9);
		key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| DBGREG9_KEY_MASK, 0);
		if (!mon_en){
			reg = rreg32(DBGREG1);
			dbgreg1_backup = reg;
		}
		mreg32(DBGREG1, (DBGREG1_TRC_MASK & (TRC_MON << DBGREG1_TRC_SHIFT))|
						(DBGREG1_MDA_MASK & (MDA_CPU << DBGREG1_MDA_SHIFT)), 
						DBGREG1_TRC_MASK | DBGREG1_MDA_MASK);
		s+= sprintf(s, "CPU Sleep mode monitor is enable\n");
		mon_en = 1;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| (key << DBGREG9_KEY_SHIFT), 0);
		mreg32(DBGREG9, 0, 
				DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		goto end;
	}
	
	/*clock*/
	ret = strncmp(item, "clk", sizeof("clk"));
	if (0 == ret){
		reg = rreg32(DBGREG9);
		key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| DBGREG9_KEY_MASK, 0);
		if (!mon_en){
			reg = rreg32(DBGREG1);
			dbgreg1_backup = reg;
		}
		mreg32(DBGREG1, (DBGREG1_TRC_MASK & (TRC_MON << DBGREG1_TRC_SHIFT))|
						(DBGREG1_MDA_MASK & (MDA_CLK << DBGREG1_MDA_SHIFT)), 
						DBGREG1_TRC_MASK | DBGREG1_MDA_MASK);
		s+= sprintf(s, "Clock monitor is enable\n");
		mon_en = 1;
		wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
						| (key << DBGREG9_KEY_SHIFT), 0);
		mreg32(DBGREG9, 0, 
				DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
		goto end;
	}

	/*debugmode*/
	ret = strncmp(item, "debugmode", sizeof("debugmode"));
	if (0 == ret){
		reg = rreg32(DBGREG1);
		if(reg&0x20000000){
			s+= sprintf(s, "Already in debugmode\n");
			goto end;
		} else {
#ifdef CONFIG_ARM_TZ
			uint32_t ret;
			uint32_t reg1 = g_dbgreg1, reg2 = g_dbgreg2, reg3 = g_dbgreg3;
			ret = sec_hal_dbg_reg_set(&reg1, &reg2, &reg3);
			if (ret == 0) {
				if (reg1 == g_dbgreg1)
					g_dbgreg1 = reg1;
				if (reg2 == g_dbgreg2)
					g_dbgreg2 = reg2;
				if (reg3 == g_dbgreg3)
					g_dbgreg3 = reg3;
			}
#else  /* !CONFIG_ARM_TZ */
			reg = rreg32(DBGREG9);
			key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
			wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
							| DBGREG9_KEY_MASK, 0);

			wreg32(DBGREG3, 0x000780ff);
			wreg32(DBGREG1, 0x20300000);
			wreg32(SWUCR,   0x00000002);
			while((rreg32(PSTR)&0x00000002)==0);
			s+= sprintf(s, "DBGREG3=%08x\n",rreg32(DBGREG3));
			s+= sprintf(s, "DBGREG1=%08x\n",rreg32(DBGREG1));
			s+= sprintf(s, "PSTR=%08x\n",rreg32(PSTR));
			wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
							| (key << DBGREG9_KEY_SHIFT), 0);
			mreg32(DBGREG9, 0,
					DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
#endif
			goto end;
		}
	}
	/*debugmode_init*/
	ret = strncmp(item, "debugmode_init", sizeof("debugmode_init"));
	if (0 == ret){
		reg = rreg32(DBGREG1);
		if(reg&0x20000000){
			s+= sprintf(s, "Already in debugmode\n");
		} else {
#ifdef CONFIG_ARM_TZ
			uint32_t ret;
			uint32_t reg1 = g_dbgreg1, reg2 = g_dbgreg2, reg3 = g_dbgreg3;
			ret = sec_hal_dbg_reg_set(&reg1, &reg2, &reg3);
			if (ret == 0) {
				if (reg1 == g_dbgreg1)
					g_dbgreg1 = reg1;
				if (reg2 == g_dbgreg2)
					g_dbgreg2 = reg2;
				if (reg3 == g_dbgreg3)
					g_dbgreg3 = reg3;
			}
#else  /* !CONFIG_ARM_TZ */
			reg = rreg32(DBGREG9);
			key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
			wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
							| DBGREG9_KEY_MASK, 0);

			wreg32(DBGREG3, 0x000780ff);
			wreg32(DBGREG1, rreg32(DBGREG1) | 0x20000000);
			wreg32(SWUCR,   0x00000002);
			while((rreg32(PSTR)&0x00000002)==0);
			s+= sprintf(s, "DBGREG3=%08x\n",rreg32(DBGREG3));
			s+= sprintf(s, "DBGREG1=%08x\n",rreg32(DBGREG1));
			s+= sprintf(s, "PSTR=%08x\n",rreg32(PSTR));
			wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
			mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
							| (key << DBGREG9_KEY_SHIFT), 0);
			mreg32(DBGREG9, 0,
					DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
#endif
		}
#ifdef CONFIG_HAVE_HW_BREAKPOINT
		arch_hw_breakpoint_init_late();
#endif
#ifdef CONFIG_HW_PERF_EVENTS
		init_hw_perf_events_late();
#endif
		goto end;
	}


fail:
	ret =  -EINVAL;
	s+= sprintf(s, "FAILED\n");
end:
	MSG_INFO("%s", buf_reg);
	FUNC_MSG_RET(ret);
}

/* Transcript to DBGMODE */
/* Following code is available in NON-TRUSTZONE Mode  */
#ifndef CONFIG_ARM_TZ
void pmdbg_dbgpin_to_dbgmode()
{
	u32 reg = 0;
	u32 key = 0;

	/* Clear the interrupt cause of TDBG */
	wreg32(DBGREG9, 0x0000A500);
	wreg32(DBGREG9, 0x0000A501);
	wreg32(DBGREG11, 0x00010000);

	/* Transition to DBGMODE */
	reg = rreg32(DBGREG9);
	key = (reg & DBGREG9_KEY_MASK) >> DBGREG9_KEY_SHIFT;
	wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
	mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
		   | DBGREG9_KEY_MASK, 0);

	wreg32(DBGREG3, 0x000780ff);
	wreg32(DBGREG1, rreg32(DBGREG1) | 0x20000000);
	wreg32(SWUCR,   0x00000002);
	while ((rreg32(PSTR)&0x00000002)==0);
	wreg32(DBGREG9, DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
	mreg32(DBGREG9, (DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT))
		   | (key << DBGREG9_KEY_SHIFT), 0);
	mreg32(DBGREG9, 0,
		   DBGREG9_AID_MASK & (DBGREG9_AID << DBGREG9_AID_SHIFT));
 #ifdef CONFIG_HAVE_HW_BREAKPOINT
	arch_hw_breakpoint_init_late();
 #endif
 #ifdef CONFIG_HW_PERF_EVENTS
	init_hw_perf_events_late();
 #endif
}
#endif

/* monpin <type>
 * type:
 * - get: Get current monitor pin
 * - bsc: Use BSC pin as monitor pin
 * - key: Use KEY pin as monitor pin
 */
static int monpin_cmd(char *para, int size)
{
	int ret = 0;
	char item[PAR_SIZE];
	int para_sz = 0;
	int pos = 0;
	char* s = buf_reg;
	u32 reg = 0;
	u32 msel15 = 0;
	FUNC_MSG_IN;
	para = strim(para);

	ret = get_word(para, size, 0, item, &para_sz);
	if (ret <=0){
		ret = -EINVAL;
		goto fail;
	}
	pos = ret;

	/*get*/
	ret = strncmp(item, "get", sizeof("get"));
	if (0 == ret){
		reg = rreg32(GPIO_MSEL03CR);
		msel15 = (reg & GPIO_MSEL03CR_MSEL15_MASK) >> GPIO_MSEL03CR_MSEL15_SHIFT;
		if (msel15 == GPIO_MSEL03CR_MSEL15_KEY){
			s+= sprintf(s, "Monitor pin: KEY\n");
		}
		else{
			s+= sprintf(s, "Monitor pin: BSC\n");
		}
		goto end;
	}
	
	/*pa*/
	ret = strncmp(item, "key", sizeof("key"));
	if (0 == ret){
		mreg32(GPIO_MSEL03CR, 0, 1 << GPIO_MSEL03CR_MSEL15_SHIFT);
		s+= sprintf(s, "KEY is used for monitor pin\n");
		goto end;
	}
	
	/*cpu*/
	ret = strncmp(item, "bsc", sizeof("bsc"));
	if (0 == ret){
		mreg32(GPIO_MSEL03CR, 1 << GPIO_MSEL03CR_MSEL15_SHIFT, 0);
		s+= sprintf(s, "BSC is used for monitor pin\n");
		goto end;
	}
fail:
	ret =  -EINVAL;
	s+= sprintf(s, "FAILED\n");
end:
	MSG_INFO("%s", buf_reg);
	FUNC_MSG_RET(ret);
}

static void dbgpin_show(char** buf)
{
	FUNC_MSG_IN;
	*buf = buf_reg;
	
	FUNC_MSG_OUT;
}

static int dbgpin_init(void)
{
	FUNC_MSG_IN;
	ADD_CMD(dbgpin, mon);
	ADD_CMD(dbgpin, monpin);

	FUNC_MSG_RET(0);
}

static void dbgpin_exit(void)
{
	FUNC_MSG_IN;
	DEL_CMD(dbgpin, mon);
	DEL_CMD(dbgpin, monpin);
	
	FUNC_MSG_OUT;
}

