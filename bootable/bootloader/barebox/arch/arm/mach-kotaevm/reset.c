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

#include <common.h>
#include <asm/io.h>

#define SBAR2		0xe6180060
#define RESCNT2		0xe6188020

/*
 * reset_cpu()
 * Input
 * 		@addr: address will be jumped to
 * Output		:None
 * Return		:None
 */
void __noreturn reset_cpu(unsigned long addr)
{
	writel(0, SBAR2);
	writel(readl(RESCNT2) | (1 << 31), RESCNT2);
	while(1);
}
EXPORT_SYMBOL(reset_cpu);
