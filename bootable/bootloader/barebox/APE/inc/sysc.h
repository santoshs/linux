/* sysc.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef __SYSC_H__
#define __SYSC_H__

#include "com_type.h"
#include "compile_option.h"


/* SYSC register address */
#define SYSC_BASE		(0xE6180000ul)
#define SYS_STBCHR0		((volatile ulong *)(SYSC_BASE + 0x0000))
#define SYS_STBCHR1		((volatile ulong *)(SYSC_BASE + 0x0001))
#define SYS_STBCHR2		((volatile ulong *)(SYSC_BASE + 0x0002))
#define SYS_STBCHR3		((volatile ulong *)(SYSC_BASE + 0x0003))
#define SYS_RPDCR		((volatile ulong *)(SYSC_BASE + 0x0004))
#define SYS_SPDCR		((volatile ulong *)(SYSC_BASE + 0x0008))
#define SYS_RWUCR		((volatile ulong *)(SYSC_BASE + 0x0010))
#define SYS_SWUCR		((volatile ulong *)(SYSC_BASE + 0x0014))
#define SYS_RBAR		((volatile ulong *)(SYSC_BASE + 0x001C))
#define SYS_SBAR		((volatile ulong *)(SYSC_BASE + 0x0020))
#define SYS_WUPRMSK		((volatile ulong *)(SYSC_BASE + 0x0028))
#define SYS_WUPSMSK		((volatile ulong *)(SYSC_BASE + 0x002C))
#define SYS_WUPMMSK		((volatile ulong *)(SYSC_BASE + 0x0030))
#define SYS_STBCHRB0	((volatile uchar *)(SYSC_BASE + 0x0040))
#define SYS_STBCHRB1	((volatile uchar *)(SYSC_BASE + 0x0041))
#define SYS_STBCHRB2	((volatile uchar *)(SYSC_BASE + 0x0042))
#define SYS_STBCHRB3	((volatile uchar *)(SYSC_BASE + 0x0043))
#define SYS_C4POWCR		((volatile uchar *)(SYSC_BASE + 0x004C))
#define SYS_PSTR		((volatile ulong *)(SYSC_BASE + 0x0080))

#define SYS_EXMSKCNT1	((volatile ulong *)(SYSC_BASE + 0x0214))
#define SYS_APSCSTP		((volatile ulong *)(SYSC_BASE + 0x0234))
#define SYS_SYCKENMSK	((volatile ulong *)(SYSC_BASE + 0x024C))
#define SYS_PDNSEL 		((volatile ulong *)(SYSC_BASE + 0x0254))
#define SYS_WUPSCR		((volatile ulong *)(SYSC_BASE + 0x0264))

#define SYS_RESCNT		((volatile ulong *)(SYSC_BASE + 0x801C))
#define SYS_RESCNT2		((volatile ulong *)(SYSC_BASE + 0x8020))
#define SYS_RESCNT3		((volatile ulong *)(SYSC_BASE + 0x8024))



#define SYS_SWRESET_FLAG			(0x01)

/* SYSC register setting value */
#define SYS_SWUCR_ALL				0x01FFF142
#define SYS_RESCNT2_SRMD			(1<<8)
#define SYS_RESCNT2_PRES			(1<<31)

#define SYS_SPDCR_A2SL				(1<<20)
#define SYS_SPDCR_A3SM				(1<<19)
#define SYS_SPDCR_A3SG				(1<<18)
#define SYS_SPDCR_A3SP				(1<<17)
#define SYS_SPDCR_C4				(1<<16)
#define SYS_SPDCR_A2RI				(1<<15)
#define SYS_SPDCR_A2RV				(1<<14)
#define SYS_SPDCR_A3R				(1<<13)
#define SYS_SPDCR_A4RM				(1<<12)
#define SYS_SPDCR_A4MP				(1<<8)
#define SYS_SPDCR_A4LC				(1<<6)
#define SYS_SPDCR_A4SF				(1<<4)
#define SYS_SPDCR_D4				(1<<1)

#define SYS_SPDCR_OFFCHARGE			(SYS_SPDCR_A3SG | SYS_SPDCR_A2RI | SYS_SPDCR_A2RV)


/* Function Prototypes */
void SYSC_Power_Supply(void);
uchar SYSC_SWReset_Flag(void);
void SYSC_Set_SWReset_Flag(void);

void SYSC_Soft_Power_On_Reset(void);
void SYSC_Reduce_Power(ulong power_mask);

void SYSC_PM_Fix(void);

#endif /* __SYSC_H__ */
