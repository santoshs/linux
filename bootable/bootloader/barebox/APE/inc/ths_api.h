/* ths_api.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __THS_API_H__
#define __THS_API_H__

#include "com_type.h"
#include "tmu_api.h"
#include "string.h"
#include "common.h"
#include "gpio.h"
#include "cpg.h"
#include "log_output.h"

#define STR   					(*((volatile unsigned long *)(0xE61F0000)))
#define ENR   					(*((volatile unsigned long *)(0xE61F0004)))
#define PORTRST_MASK			(*((volatile unsigned long *)(0xE61F0008)))
#define THSCR0					(*((volatile unsigned long *)(0xE61F012C)))
#define THSSR0					(*((volatile unsigned long *)(0xE61F0130)))
#define INTCTLR0				(*((volatile unsigned long *)(0xE61F0134)))
#define THSCR1					(*((volatile unsigned long *)(0xE61F022C)))
#define THSSR1					(*((volatile unsigned long *)(0xE61F0230)))
#define INTCTLR1				(*((volatile unsigned long *)(0xE61F0234)))

#define CPCTL					(0x1<<12)
#define THIDLE_MASK				(0x3<<8)
#define THIDLE_NORMAL_FULL		(0x0<<8)
#define THIDLE_NORMAL			(0x2<<8)
#define THIDLE_IDLE				(0x3<<8)

#define CTEMP_MASK				(0x3F)


/* Function prototype */
void ths_init(void);
int ths_get_cpu_temp(void);

#endif /* __THS_API_H__ */
