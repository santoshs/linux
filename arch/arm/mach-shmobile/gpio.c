/*
 * GPIO API supplement
 *
 * Generic GPIO library lacks of some functions; it doesn't provide any
 * APIs to specify pull-up or pull-down of the port, nor an API to disable
 * both the input and the output of the port.  These supplementary APIs
 * are to fill in the missing piece of generic GPIO APIs.
 *
 * Note that these APIs are supposed to be used _after_ primary port
 * configuration of each port has been done using generic GPIO APIs.
 */

#include <mach/gpio.h>
#include <mach/board-u2evm.h>
#include <mach/r8a73734.h>
#include <linux/io.h>
#include <linux/module.h>
#include <asm/system.h>

void gpio_direction_none_port(int gpio)
{

	__raw_writeb(__raw_readb(GPIO_PORTCR(gpio)) & GPIO_DIRECTION_NONE,
	GPIO_PORTCR(gpio));
}
EXPORT_SYMBOL_GPL(gpio_direction_none_port);

void gpio_pull_off_port(int gpio)
{
	__raw_writeb(__raw_readb(GPIO_PORTCR(gpio)) & GPIO_PULL_OFF,
	GPIO_PORTCR(gpio));
}
EXPORT_SYMBOL_GPL(gpio_pull_off_port);

void gpio_pull_up_port(int gpio)
{
	__raw_writeb((__raw_readb(GPIO_PORTCR(gpio)) & GPIO_PULL_OFF)
					| GPIO_PULL_UP, GPIO_PORTCR(gpio));
}
EXPORT_SYMBOL_GPL(gpio_pull_up_port);

void gpio_pull_down_port(int gpio)
{
	__raw_writeb((__raw_readb(GPIO_PORTCR(gpio)) & GPIO_PULL_OFF)
					| GPIO_PULL_DOWN, GPIO_PORTCR(gpio));
}
EXPORT_SYMBOL_GPL(gpio_pull_down_port);

void gpio_bidirection_port(int gpio)
{
	__raw_writeb(__raw_readb(GPIO_PORTCR(gpio)) | GPIO_BIDIRECTION,
	GPIO_PORTCR(gpio));
}
EXPORT_SYMBOL_GPL(gpio_bidirection_port);
