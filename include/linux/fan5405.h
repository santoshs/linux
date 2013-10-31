/*
 * fan5405.h
 *
 *Copyright (C) 2013 Broadcom Mobile
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *version 2 as published by the Free Software Foundation.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *MA  02110-1301, USA.
 */

#ifndef _LINUX_FAN5405_H
#define _LINUX_FAN5405_H

#define fan5405_CONTROL0    0x00
#define fan5405_CONTROL1    0x01
#define fan5405_OREG        0x02
#define fan5405_IC_INFO     0x03
#define fan5405_IBAT        0x04
#define fan5405_SP_CHARGER  0x05
#define fan5405_SAFETY      0x06
#define fan5405_MONITOR     0x10

/**********************************************************
 *
 *[MASK/SHIFT]
 *
 **********************************************************/
/* CONTROL0 */
#define CON0_TMR_RST_MASK   0x01
#define CON0_TMR_RST_SHIFT  7

#define CON0_EN_STAT_MASK   0x01
#define CON0_EN_STAT_SHIFT  6

#define CON0_STAT_MASK      0x03
#define CON0_STAT_SHIFT     4

#define CON0_FAULT_MASK     0x07
#define CON0_FAULT_SHIFT    0

/* CONTROL1 */
#define CON1_IN_LIMIT_MASK  0x03
#define CON1_IN_LIMIT_SHIFT 6

#define CON1_LOWV_MASK      0x03
#define CON1_LOWV_SHIFT     4

#define CON1_TE_MASK        0x01
#define CON1_TE_SHIFT       3

#define CON1_CE_MASK        0x01
#define CON1_CE_SHIFT       2

#define CON1_HZ_MODE_MASK   0x01
#define CON1_HZ_MODE_SHIFT  1

#define CON1_OPA_MODE_MASK  0x01
#define CON1_OPA_MODE_SHIFT 0

/* OREG */
#define OREG_VFLOAT_MASK      0x3F
#define OREG_VFLOAT_SHIFT     2

#define OREG_OTG_PL_MASK    0x01
#define OREG_OTG_PL_SHIFT   1

#define OREG_OTG_EN_MASK    0x01
#define OREG_OTG_EN_SHIFT   0

/* IC_INFO */
#define ICINFO_VENDER_CODE_MASK   0x07
#define ICINFO_VENDER_CODE_SHIFT  5

#define ICINFO_PN_MASK        0x03
#define ICINFO_PN_SHIFT       3

#define ICINFO_REVISION_MASK  0x07
#define ICINFO_REVISION_SHIFT 0

/* I_BAT */
#define IBAT_RESET_MASK     0x01
#define IBAT_RESET_SHIFT    7

#define IBAT_IOCHARGE_MASK      0x07
#define IBAT_IOCHARGE_SHIFT     4

#define IBAT_ITERM_MASK     0x07
#define IBAT_ITERM_SHIFT    0

/* SP_CHARGER */
#define SPC_DIS_VREG_MASK  0x01
#define SPC_DIS_VREG_SHIFT 6

#define SPC_IOLEVEL_MASK   0x1
#define SPC_IOLEVEL_SHIFT  5

#define SPC_SP_MASK        0x1
#define SPC_SP_SHIFT       4

#define SPC_VSP_MASK       0x07
#define SPC_VSP_SHIFT      0

/* SAFETY */
#define SAFETY_ISAFE_MASK     0x07
#define SAFETY_ISAFE_SHIFT    4

#define SAFETY_VSAFE_MASK     0x0F
#define SAFETY_VSAFE_SHIFT    0

/* MONITOR */
#define MONITOR_VBUS_VALID_MASK   0x1
#define MONITOR_VBUS_VALID_SHIFT  0x1

#endif
