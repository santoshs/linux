/*
 * arch/arm/mach-shmobile/crashlog.c
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <mach/crashlog.h>


void set_log_info(unsigned long store_addr,
		unsigned long buffer_addr, unsigned long buffer_size_addr,
		unsigned long w_off_add, unsigned long head_addr)
{
	void __iomem *adr = ioremap(store_addr, 16);
	if (adr == NULL) {
		pr_err("crashlog reset log initialization failed\n");
		return;
	}

	__raw_writel(buffer_addr, adr);
	__raw_writel(buffer_size_addr, adr + 4);
	__raw_writel(w_off_add, adr + 8);
	__raw_writel(head_addr, adr + 12);
	iounmap(adr);
}
