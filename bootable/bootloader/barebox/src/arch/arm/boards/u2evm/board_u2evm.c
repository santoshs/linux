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
#include <console.h>
#include <init.h>
#include <driver.h>
#include <asm/io.h>
#include <clock.h>
#include <asm/armlinux.h>
#include <mach/hardware-base.h>
#include <linux/err.h>
#include <generated/mach-types.h>
#include <sizes.h>
#include <partition.h>
#include <i2c/r_mobile_i2c.h>

#define SCIF_BASE	SCIF0_BASE
#define SD0CKCR			(CPG_BASE + 0x0074)

/* Environment partition */
#define ENV_PATTITON_ADDRESS (unsigned long long)(0x00000000002A4800)
#define ENV_PATTITON_SIZE (0x40000)	/* 256 Kbytes */

/*
* Init scifa, mmc, sdcard
*/

void board_init(void)
{
	/* Initial H/W: moved to R-loader  */
}

#ifdef CONFIG_DRIVER_R_MOBILE_SERIAL

static struct device_d kota_serial_device = {
	.id = -1,
	.name = "r_mobile_serial",
	.map_base = SCIF_BASE,
	.size = SZ_1K,
};

/**
 * @brief serial port initialization
 *
 * @return result of device registration
 */
static int kota_console_init(void)
{
	/* Register the serial port */
	return register_device(&kota_serial_device);
}
console_initcall(kota_console_init);
#endif /* CONFIG_DRIVER_R_MOBILE_SERIAL */

/*
* SDRAM Init
*/
static struct memory_platform_data kota_sram_pdata = {
	.name	= "ram0",
	.flags	= DEVFS_RDWR,
};

static struct device_d kota_sdram_dev = {
	.id		= -1,
	.name		= "mem",
	.map_base	= CONFIG_SYS_SDRAM_BASE,
	.size		= CONFIG_SYS_SDRAM_SIZE,
	.platform_data	= &kota_sram_pdata,
};

/*
* Ethernet Init
*/
static struct device_d smsc911x_eth_dev = {
	.id		= -1,
	.name		= "smc911x",
	.map_base	= CONFIG_SMC911X_BASE,
	.size		= SZ_64K,
};

#if 0 /* Do not need I2C to enable SD card*/
/*
* I2C Init
*/
static struct device_d r_mobile_i2c_dev = {
	.id = -1,
	.name = "r_mobile_i2c",
	.map_base = I2C0_BASE,
	.size = SZ_1K,
};
#endif

/*
* eMMC Init
*/
static struct device_d r_mobile_mmcif_dev = {
	.id = -1,
	.name = "r_mobile_mmcif",
	.map_base = MMCIF0_BASE,
	.size = SZ_4K,
};

/*
* SDHI0 Init
*/
static struct device_d r_mobile_sdhi_dev = {
	.id = -1,
	.name = "r_mobile_sdhi",
	.map_base = SDHI0_BASE,
	.size = SZ_1K,
};

static int kotaevm_init_devices(void)
{
	int ret;
	board_init();
	ret = register_device(&kota_sdram_dev);
	if(ret)
	{
		goto failed;
	}

#ifdef CONFIG_MCI_R_MOBILE_MMCIF
	register_device(&r_mobile_mmcif_dev);
#endif /* CONFIG_MCI_R_MOBILE_MMCIF */

#ifdef CONFIG_R_MOBILE_I2C
	register_device(&r_mobile_i2c_dev);
#endif /* CONFIG_R_MOBILE_I2C */

#ifdef CONFIG_MCI_R_MOBILE_SDHI
	register_device(&r_mobile_sdhi_dev);
#endif

#ifdef	CONFIG_DRIVER_NET_SMC911X
	register_device(&smsc911x_eth_dev);
#endif	/* CONFIG_DRIVER_NET_SMC911X */

	armlinux_add_dram(&kota_sdram_dev);
	/* adress of boot parameters */
 	armlinux_set_bootparams((void *)(CONFIG_SYS_SDRAM_BASE + 0x100));
	/* board id for linux */
 	armlinux_set_architecture(MACH_TYPE_KOTAEVM);
failed:
	return ret;
}
coredevice_initcall(kotaevm_init_devices);

static int register_env_partition(void)
{
	/* Add partition for storing environment variable */
	devfs_add_partition("disk0", ENV_PATTITON_ADDRESS, ENV_PATTITON_SIZE, PARTITION_FIXED, "env0");	/* 255kB */
	printf("Registering environment partition %s to drive %s: Start address: 0x%llx, Size: 0x%x\n", 
			"evn0", "disk0", ENV_PATTITON_ADDRESS, ENV_PATTITON_SIZE);
	return 0;
}
late_initcall(register_env_partition);