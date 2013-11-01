#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/d2153/pmic.h>
#include <linux/regulator/consumer.h>
#include <mach/irqs.h>

#include <linux/i2c/bcmtch15xxx.h>
#include <mach/bcmtch15xxx_settings.h>

#define BCMTCH15XXX_TSC_NAME	"bcmtch15xxx"

#if 0
static int BCMTCH_TSP_PowerOnOff(bool on) {
/* PLACE TOUCH CONTROLLER REGULATOR CODE HERE – SEE STEP 6 */
}
#endif

static struct bcmtch_platform_data bcmtch15xxx_i2c_platform_data = {
	.i2c_bus_id       = BCMTCH_HW_I2C_BUS_ID,
	.i2c_addr_spm = BCMTCH_HW_I2C_ADDR_SPM,
	.i2c_addr_sys   = BCMTCH_HW_I2C_ADDR_SYS,

	.gpio_interrupt_pin       = BCMTCH_HW_GPIO_INTERRUPT_PIN,
	.gpio_interrupt_trigger = BCMTCH_HW_GPIO_INTERRUPT_TRIGGER,

	.gpio_reset_pin           = BCMTCH_HW_GPIO_RESET_PIN,
	.gpio_reset_polarity   = BCMTCH_HW_GPIO_RESET_POLARITY,
	.gpio_reset_time_ms = BCMTCH_HW_GPIO_RESET_TIME_MS,

	//.ext_button_count = BCMTCH_BUTTON_COUNT,
	//.ext_button_map   = bcmtch_button_map,
#if 0
	.axis_orientation_flag =
		((BCMTCH_HW_AXIS_ REVERSE _X << BCMTCH_AXIS_FLAG_X_BIT_POS)
		|(BCMTCH_HW_AXIS_ REVERSE _Y << BCMTCH_AXIS_FLAG_Y_BIT_POS)
		|(BCMTCH_HW_AXIS_SWAP_X_Y << BCMTCH_AXIS_FLAG_X_Y_BIT_POS)),

	//.bcmtch_on = BCMTCH_TSP_PowerOnOff,
#endif
};

static struct i2c_board_info __initdata bcmtch15xxx_i2c_boardinfo[] = {
        {
                I2C_BOARD_INFO(BCMTCH15XXX_TSC_NAME, BCMTCH_HW_I2C_ADDR_SPM),
                .irq = irqpin2irq(BCMTCH_HW_GPIO_INTERRUPT_PIN),
		  .platform_data =	&bcmtch15xxx_i2c_platform_data,
        },
};

void __init bcmtch15xxx_tsp_init(void)
{
	printk(" *** [TSP] bcmtch15xxx_tsp_init + \n");

	i2c_register_board_info(bcmtch15xxx_i2c_platform_data.i2c_bus_id, bcmtch15xxx_i2c_boardinfo, ARRAY_SIZE(bcmtch15xxx_i2c_boardinfo));
}

