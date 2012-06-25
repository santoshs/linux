/*
 * Copyright (C) 2011 Renesas Electronics Corporation
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

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/arch/hardware.h>

#define SD0CKCR			(CPG_BASE + 0x0074)
#define MPCKCR			(CPG_BASE + 0x0080)
#define SMSTPCR2		(CPG_BASE + 0x0138)
#define SMSTPCR3		(CPG_BASE + 0x013c)

#define PORTD127_096DSR		(GPIO_BASE + 0x5100)
#define MSEL3CR			(GPIO_BASE + 0x8020)

#define BSC_CS0WCR		(BSC_BASE + 0x0024)

#define GPIO_PORTCR(n)	({				\
	((n) <  96) ? (GPIO_BASE + 0x0000 + (n)) :	\
	((n) < 128) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 144) ? (GPIO_BASE + 0x1000 + (n)) :	\
	((n) < 192) ? 0 :				\
	((n) < 320) ? (GPIO_BASE + 0x2000 + (n)) :	\
	((n) < 328) ? (GPIO_BASE + 0x3000 + (n)) : 0; })

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_U2EVM;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* SCIFA0 */
	outl(inl(MPCKCR) & ~(1 << 10), MPCKCR); /* MPCKCSTP */
	outl(inl(MPCKCR) & ~(1 << 9), MPCKCR);  /* MPCKSTP */
	outl(inl(SMSTPCR2) & ~(1 << 4), SMSTPCR2); /* MSTP204 */
	outb(0x11, GPIO_PORTCR(128));	/* SCIFA0 TXD */
	outb(0xe1, GPIO_PORTCR(129));	/* SCIFA0 RXD */

	/* MMCIF0 */
	outl(inl(SMSTPCR3) & ~(1 << 15), SMSTPCR3); /* MSTP315 */
	outb(0xc1, GPIO_PORTCR(300));	/* MMCD0_0 */
	outb(0xc1, GPIO_PORTCR(301));	/* MMCD0_1 */
	outb(0xc1, GPIO_PORTCR(302));	/* MMCD0_2 */
	outb(0xc1, GPIO_PORTCR(303));	/* MMCD0_3 */
	outb(0xc1, GPIO_PORTCR(304));	/* MMCD0_4 */
	outb(0xc1, GPIO_PORTCR(305));	/* MMCD0_5 */
	outb(0xc1, GPIO_PORTCR(306));	/* MMCD0_6 */
	outb(0xc1, GPIO_PORTCR(307));	/* MMCD0_7 */
	outb(0xc1, GPIO_PORTCR(308));	/* MMCCMD0 */
	outb(0x11, GPIO_PORTCR(309));	/* MMCCLK0 */
	outb(0x11, GPIO_PORTCR(310));	/* MMCRST */

	/* SDHI0 */
	outl(inl(SD0CKCR) & ~(1 << 8), SD0CKCR); /* CKSTP */
	outl(inl(SMSTPCR3) & ~(1 << 14), SMSTPCR3); /* MSTP314 */
	outb(0xc1, GPIO_PORTCR(320));	/* SDHID0_D0 */
	outb(0xc1, GPIO_PORTCR(321));	/* SDHID0_D1 */
	outb(0xc1, GPIO_PORTCR(322));	/* SDHID0_D2 */
	outb(0xc1, GPIO_PORTCR(323));	/* SDHID0_D3 */
	outb(0xc1, GPIO_PORTCR(324));	/* SDHICMD0 */
	outb(0xc1, GPIO_PORTCR(325));	/* SDHIWP0 */
	outb(0x11, GPIO_PORTCR(326));	/* SDHCLK0 */
	outb(0xc1, GPIO_PORTCR(327));	/* SDHICD0 */
	outl(inl(MSEL3CR) | (1 << 28), MSEL3CR); /* IO power ON */

	/* Ethernet - SMSC LAN9220 */
	outl(0x00001d40, BSC_CS0WCR);
	outb(0x10, GPIO_PORTCR(105));	/* NRESET */
	outl(1 << (105 - 96), PORTD127_096DSR);

	/* CMT1 */
	outl(inl(SMSTPCR3) & ~(1 << 29), SMSTPCR3); /* MSTP329 */

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
#endif

#ifdef BOARD_LATE_INIT
int board_late_init(void)
{
	return 0;
}
#endif
