/*
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
*/

#ifndef __HARDWARE_BASE_H
#define __HARDWARE_BASE_H

/* EOS I/O address base */
#define SRAM_BASE	0xE5580000
#define HPB_BASE	0xE6000000
#define RWDT_BASE	0xE6020000
#define GPIO_BASE	0xE6050000
#define CMT1_BASE	0xE6130000
#define CPG_BASE	0xE6150000
#define MMCIF0_BASE	0xE6BD0000
#define MMCIF1_BASE	0xE6BE0000
#define SCIF0_BASE	0xE6450000
#define SCIF2_BASE	0xE6C60000
#define SDHI0_BASE	0xEE100000
#define GIC_CPU_BASE	0xF0000100
#define SBSC1_BASE	0xFE400000

/* RWDT Symbols */
#define RWTCSRA0_A	(RWDT_BASE + 0x0004)

/* etc Symbols */
#define CCCR		(HPB_BASE + 0x101C)
#define SMGPIOTIME	(HPB_BASE + 0x1828)
#define SMCPGTIME	(HPB_BASE + 0x1858)
#define SMSYSCTIME	(HPB_BASE + 0x1878)

/* MMC0 - EOS */
#define GPIO_PORT300CR		(GPIO_BASE + 0x212C)	/* MMCD0_0 */
#define GPIO_PORT301CR		(GPIO_BASE + 0x212D)	/* MMCD0_1 */
#define GPIO_PORT302CR		(GPIO_BASE + 0x212E)	/* MMCD0_2 */
#define GPIO_PORT303CR		(GPIO_BASE + 0x212F)	/* MMCD0_3 */
#define GPIO_PORT304CR		(GPIO_BASE + 0x2130)	/* MMCD0_4 */
#define GPIO_PORT305CR		(GPIO_BASE + 0x2131)	/* MMCD0_5 */
#define GPIO_PORT306CR		(GPIO_BASE + 0x2132)	/* MMCD0_6 */
#define GPIO_PORT307CR		(GPIO_BASE + 0x2133)	/* MMCD0_7 */
#define GPIO_PORT308CR		(GPIO_BASE + 0x2134)	/* MMCCMD0 */
#define GPIO_PORT309CR		(GPIO_BASE + 0x2135)	/* MMCCLK0 */
#define GPIO_PORT310CR		(GPIO_BASE + 0x2136)	/* MMCRST */

/* SDHI0 - EOS */
#define GPIO_PORT320CR		(GPIO_BASE + 0x3140)	/* SDHID0_0 */
#define GPIO_PORT321CR		(GPIO_BASE + 0x3141)	/* SDHID0_1 */
#define GPIO_PORT322CR		(GPIO_BASE + 0x3142)	/* SDHID0_2 */
#define GPIO_PORT323CR		(GPIO_BASE + 0x3143)	/* SDHID0_3 */
#define GPIO_PORT324CR		(GPIO_BASE + 0x3144)	/* SDHICMD0 */
#define GPIO_PORT325CR		(GPIO_BASE + 0x3145)	/* SDHIWP0 */
#define GPIO_PORT326CR		(GPIO_BASE + 0x3146)	/* SDHICLK0 */
#define GPIO_PORT327CR		(GPIO_BASE + 0x3147)	/* SDHICD0 */
#define GPIO_DRVR_SYS		(GPIO_BASE + 0x8112)
#define GPIO_DRVCR_SDCLK0	(GPIO_BASE + 0x811E)
#define GPIO_DRVCR_SD0		(GPIO_BASE + 0x818E)

#define GPIO_DRVCR_VCCQ		(GPIO_BASE + 0x8120)

/* SCIFA2 */
#define GPIO_PORT88CR		(GPIO_BASE + 0x0058)	/* SCIFA2_TXD */
#define GPIO_PORT89CR		(GPIO_BASE + 0x0059)	/* SCIFA2_RXD */
#define GPIO_PORT31CR		(GPIO_BASE + 0x001F)	/* SCIFA2_RTS_ */
#define GPIO_PORT32CR		(GPIO_BASE + 0x0020)	/* SCIFA2_CTS_ */
/* SCIFA0 */
#define GPIO_PORT128CR		(GPIO_BASE + 0x0080)	/* SCIFA0_RTS */
#define GPIO_PORT129CR		(GPIO_BASE + 0x0081)	/* SCIFA0_CTS */
/* I2C */
#define SMSTPCR1	(CPG_BASE + 0x0134)
#define SRCR1		(CPG_BASE + 0x80A8)
#define I2C0_BASE				0xE6820000		/* I2C bus interface(IIC0) */
#define GPIO_PORT84CR		(GPIO_BASE + 0x0054)
#define GPIO_PORT85CR		(GPIO_BASE + 0x0055)

/* I2C setting value */
#define I2C_SLAVE_ADDR_ID1	(0x48)
/* Register address */
#define LDO1_CFG_STATE		(0x9E)
#define LDO1_CFG_TRANS		(0x9D)
#define LDO1_CFG_VOLTAGE	(0x9F)
/* Setting value */
#define STATE_ON_VIO_SD		(0x01)	/* LDO1 = ON */
#define TRANS_ACTIVE_MODE	(0x03)

/*--- Registers Value ---*/
#define SMSTPCR3_CMT1	(1 << 29)
#define MSTPST314	(1 << 14)
#define MSTPST315	(1 << 15)

#define GPIO_MSEL3CR			(GPIO_BASE + 0x8020)

/* MEMORY */
#define CONFIG_NR_DRAM_BANKS		1
#define EOS_SDRAM_BASE			(0x40000000)
#define PHYS_SDRAM_1			EOS_SDRAM_BASE
#define PHYS_SDRAM_1_SIZE		(1024 * 1024 * 1024)
// #define CONFIG_SDRAM_OFFSET_FOR_RT	(16 * 1024 * 1024)	/* 16MB reserved for ISR */
#define MODEM_MISC_SIZE			0x10000000

#define CONFIG_SYS_SDRAM_BASE	(EOS_SDRAM_BASE + MODEM_MISC_SIZE)
#define CONFIG_SYS_SDRAM_SIZE	(PHYS_SDRAM_1_SIZE - MODEM_MISC_SIZE)
#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 0x7fc0)
// #define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 * 1024)

/* Board Clock */
#define CONFIG_SYS_CLK_FREQ	48000000
#define CONFIG_SYS_HZ		1000
#define CONFIG_BAUDRATE		115200

/* Ethernet */
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#define CONFIG_NET_MULTI

#define GPIO_PORT229CR	(GPIO_BASE + 0x20E5)
#define CONFIG_SMC911X		1
#define CONFIG_SMC911X_16_BIT	1
#define CONFIG_SMC911X_BASE	0x00080000	/* CS0_N */

/* CMT1 channel 0 */
#define SMSTPCR3	(CPG_BASE + 0x013C)
#define CMT_BASE	CMT1_BASE

/* Clock enable register */
#define CMCLKE		(CMT_BASE + 0x1000)
/* Enable clock for channel 0*/
#define CH0CLKE		(0 << 0)
#define CMSTR		(CMT_BASE + 0x0000)	/* 32bit */
#define CMCSR		(CMT_BASE + 0x0010)	/* 32bit */
#define CMCNT		(CMT_BASE + 0x0014)	/* 32bit */
#define CMCOR		(CMT_BASE + 0x0018)	/* 32bit */


// #define TIMER_CLOCK		4096 /* (32768 / 8) */
#define TIMER_CLOCK		32768 /* (32768 / 1) */

#endif /* __HARDWARE_BASE_H */