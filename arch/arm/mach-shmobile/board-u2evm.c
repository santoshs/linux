#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <mach/r8a73734.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <linux/smsc911x.h>

static struct resource smsc9220_resources[] = {
	{
		.start	= 0x14000000,
		.end	= 0x14000000 + SZ_64K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= gic_spi(9),
		.flags	= IORESOURCE_IRQ | IRQ_TYPE_LEVEL_LOW,
	},
};

static struct smsc911x_platform_config smsc9220_platdata = {
	.flags		= SMSC911X_USE_32BIT,
	.phy_interface	= PHY_INTERFACE_MODE_MII,
	.irq_polarity	= SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
	.irq_type	= SMSC911X_IRQ_TYPE_PUSH_PULL,
};

static struct platform_device eth_device = {
	.name		= "smsc911x",
	.id		= 0,
	.dev	= {
		.platform_data = &smsc9220_platdata,
	},
	.resource	= smsc9220_resources,
	.num_resources	= ARRAY_SIZE(smsc9220_resources),
};

static struct platform_device *u2evm_devices[] __initdata = {
	&eth_device,
};

static struct map_desc u2evm_io_desc[] __initdata = {
	{
		.virtual	= 0xe6000000,
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= 256 << 20,
		.type		= MT_DEVICE_NONSHARED
	},
};

static void __init u2evm_map_io(void)
{
	iotable_init(u2evm_io_desc, ARRAY_SIZE(u2evm_io_desc));
	r8a73734_add_early_devices();
	shmobile_setup_console();
}

#define IRQC_INTEN_SET0 0xe61c0008
#define IRQC_CONFIG_09 0xe61c01a4

void __init u2evm_init_irq(void)
{
	r8a73734_init_irq();
	__raw_writel(1<<9, IRQC_INTEN_SET0); /* route IRQ9 to CPU0 */
	__raw_writel(1, IRQC_CONFIG_09); /* low level */

}

static void __init u2evm_init(void)
{
	r8a73734_pinmux_init();

	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RTS_, NULL);
	gpio_request(GPIO_FN_SCIFA0_CTS_, NULL);

	gpio_request(GPIO_FN_MMCCLK0, NULL);
	gpio_request(GPIO_FN_MMCD0_0, NULL);
	gpio_request(GPIO_FN_MMCD0_1, NULL);
	gpio_request(GPIO_FN_MMCD0_2, NULL);
	gpio_request(GPIO_FN_MMCD0_3, NULL);
	gpio_request(GPIO_FN_MMCD0_4, NULL);
	gpio_request(GPIO_FN_MMCD0_5, NULL);
	gpio_request(GPIO_FN_MMCD0_6, NULL);
	gpio_request(GPIO_FN_MMCD0_7, NULL);
	gpio_request(GPIO_FN_MMCCMD0, NULL);

	gpio_request(GPIO_PORT9, NULL);
	gpio_direction_input(GPIO_PORT9); /* for IRQ */
	gpio_request(GPIO_PORT10, NULL);
	gpio_direction_output(GPIO_PORT10, 1); /* release NRESET */

#ifdef CONFIG_CACHE_L2X0
	l2x0_init(__io(0xf0100000), 0x40460000, 0x82000fff);
#endif
	r8a73734_add_standard_devices();
	platform_add_devices(u2evm_devices, ARRAY_SIZE(u2evm_devices));
}

static void __init u2evm_timer_init(void)
{
	r8a73734_clock_init();
	shmobile_timer.init();
}

struct sys_timer u2evm_timer = {
	.init	= u2evm_timer_init,
};

MACHINE_START(U2EVM, "u2evm")
	.map_io		= u2evm_map_io,
	.init_irq	= u2evm_init_irq,
	.handle_irq	= shmobile_handle_irq_gic,
	.init_machine	= u2evm_init,
	.timer		= &u2evm_timer,
MACHINE_END
