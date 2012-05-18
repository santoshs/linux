/* scuw_ctrl.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __SCUW_CTRL_H__
#define __SCUW_CTRL_H__

#include <linux/kernel.h>

/*
 * SCUW Registers
 */

/* VD Register */
/* VDSET_VOC0     : Voice Data setting register */
#define SCUW_VD_VDSET			(0x00500)

/* SEL Register */
/* SELCR_SEL0     : Selector control register(SEL0) */
#define SCUW_SEL_SELCR0			(0x00504)
/* SELCR_SEL5     : Selector control register(SEL5) */
#define SCUW_SEL_SELCR5			(0x00518)
/* SELCR_SEL6     : Selector control register(SEL6) */
#define SCUW_SEL_SELCR6			(0x0051C)
/* SELCR_SEL7     : Selector control register(SEL7) */
#define SCUW_SEL_SELCR7			(0x00520)
/* SELCR_SEL12    : Selector control register(SEL12) */
#define SCUW_SEL_SELCR12		(0x00534)
/* SELCR_SEL15    : Selector control register(SEL15) */
#define	SCUW_SEL_SELCR15		(0x00540)
/* SELCR_SEL21    : Selector control register(SEL21) */
#define	SCUW_SEL_SELCR21		(0x00558)

/* FSIIF Registers */
/* SWRSR_FSIIF    : FSI IF Software reset register(FSI IF0) */
#define SCUW_FSIIF_SWRSR		(0x00600)
/* FSIIR_FSIIF    : FSI IF initialization register(FSI IF0) */
#define SCUW_FSIIF_FSIIR		(0x00604)
/* ADINR_FSIIF_W0 : Audio information register for write port 0(FSI IF0) */
#define SCUW_FSIIF_ADINRW0		(0x00608)
/* ADINR_FSIIF_W1 : Audio information register for write port 1(FSI IF0) */
#define SCUW_FSIIF_ADINRW1		(0x0060C)
/* ADINR_FSIIF_R0 : Audio information register for read port 0(FSI IF0) */
#define SCUW_FSIIF_ADINRR0		(0x00610)
/* ADINR_FSIIF_R1 : Audio information register for read port 1(FSI IF0) */
#define SCUW_FSIIF_ADINRR1		(0x00614)
/* WADCR_FSIIF_0  : FSI Write Address control register for port 0(FSI IF0) */
#define SCUW_FSIIF_WADCR0		(0x00618)
/* WADCR_FSIIF_1  : FSI Write Address control register for port 1(FSI IF0) */
#define SCUW_FSIIF_WADCR1		(0x0061C)
/* RADCR_FSIIF_0  : FSI Read Address control register for port 0(FSI IF0) */
#define SCUW_FSIIF_RADCR0		(0x00620)
/* RADCR_FSIIF_1  : FSI Read Address control register for port 1(FSI IF0) */
#define SCUW_FSIIF_RADCR1		(0x00624)

/* MSTP Register */
/* MSTP1_FSIIF    : FSIIF Module control register */
#define SCUW_MSTP1			(0x0065C)


#endif	/* __SCUW_CTRL_H__ */

