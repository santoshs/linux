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
#include <mach/r8a7373.h>
#include <linux/io.h>
#include <linux/module.h>
#include <asm/system.h>

#define gpio_error_log(fmt, ...) printk(fmt, ##__VA_ARGS__)

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


/* Function to set GPIO CR in suspend/resume from driver code */
void gpio_set_portncr_value(unsigned int port_count,
	struct portn_gpio_setting_info *gpio_setting_info, int suspend_mode)
{
	int i = 0, ret = 0;
	int port = 0;
	struct portn_gpio_setting *gpio_prev = NULL, *gpio_current = NULL;

	if (gpio_setting_info == NULL || port_count == 0)
		return ;

	for (i = 0; i < port_count; i++) {
		port = gpio_setting_info[i].port;
		if (suspend_mode == 1) {
			gpio_current =
				&gpio_setting_info[i].inactive;
			gpio_prev  = &gpio_setting_info[i].active;
		} else {
			gpio_current = &gpio_setting_info[i].active;
			gpio_prev = &gpio_setting_info[i].inactive;
		}

		if (gpio_setting_info[i].flag == 1) {
			/* Change required */
			gpio_free(gpio_prev->port_fn);
			/* Select Function 0 */
			ret = gpio_request(port, NULL);
			if (ret < 0)
				gpio_error_log("gpio_request failed"
			" at Line %d in File %s\n", __LINE__, __FILE__);
			/* Set Input/Output direction & Output level */
			/*
			 * gpio_direction_input() and gpio_direction_output
			 * are only used by Function 0. So, in the case of
			 * selecting function other than Function 0,
			 * it is necessary to do following sequence.
			 * 1.gpio_request()   <- Function0
			 * 2.gpio_direction_output()/gpio_direction_input()
			 * 3.gpio_free()
			 * 4.gpio_request() <- except Function0.
			*/
			switch (gpio_current->direction) {
			case PORTn_CR_DIRECTION_NOT_SET:
				break ;
			case PORTn_CR_DIRECTION_NONE:
				/* Either gpio_direction_input or
				gpio_direction_output should be invoked
				before gpio_direction_none_port */
				ret = gpio_direction_input(port);
				if (ret < 0)
					gpio_error_log("gpio_direction_input"
			" failed at Line %d in File %s\n", __LINE__, __FILE__);
				gpio_direction_none_port(port);
				break;
			case PORTn_CR_DIRECTION_OUTPUT:
				ret = gpio_direction_output(port,
						gpio_current->output_level);
				if (ret < 0)
					gpio_error_log("gpio_direction_output"
			" failed at Line %d in File %s\n", __LINE__, __FILE__);
				break;

			case PORTn_CR_DIRECTION_INPUT:
				ret = gpio_direction_input(port);
				if (ret < 0)
					gpio_error_log("gpio_direction_input"
			" failed at Line %d in File %s\n", __LINE__, __FILE__);
				break;
			case PORTn_CR_BI_DIRECTIONAL:
				gpio_bidirection_port(port);
				break;
			default:
				break;
			}

			/* Set Pull up/down/off */
			switch (gpio_current->pull) {
			case PORTn_CR_PULL_NOT_SET:
				break;
			case PORTn_CR_PULL_OFF:
				gpio_pull_off_port(port);
				break;
			case PORTn_CR_PULL_DOWN:
				gpio_pull_down_port(port);
				break;

			case PORTn_CR_PULL_UP:
				gpio_pull_up_port(port);
				break;
			default:
				break;
			}
			if (gpio_current->port_fn != port) {
				gpio_free(port);
				ret = gpio_request(gpio_current->port_fn, NULL);
				if (ret < 0)
					gpio_error_log("gpio_request"
			" failed at Line %d in File %s\n", __LINE__, __FILE__);
			}
		}
	}
	return;
}
EXPORT_SYMBOL_GPL(gpio_set_portncr_value);
