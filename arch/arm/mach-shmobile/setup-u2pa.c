#include <linux/spa_power.h>
#if defined(CONFIG_USB_SWITCH_TSU6712)
#include <linux/tsu6712.h>
#endif

#if defined(CONFIG_BATTERY_BQ27425)
#define BQ27425_ADDRESS (0xAA >> 1)
#define GPIO_FG_INT 44
#endif

#if defined(CONFIG_CHARGER_SMB328A)
#define SMB328A_ADDRESS (0x69 >> 1)
#define GPIO_CHG_INT 19
#endif

#if defined(CONFIG_USB_SWITCH_TSU6712)
#define TSU6712_ADDRESS (0x4A >> 1)
#define GPIO_MUS_INT 41

static struct tsu6712_platform_data tsu6712_pdata = {
};
#endif

static struct i2c_board_info __initdata i2c3_devices[] = {
#if defined(CONFIG_BATTERY_BQ27425)
    {
                I2C_BOARD_INFO("bq27425", BQ27425_ADDRESS),
                .irq            = irqpin2irq(GPIO_FG_INT),
        },
#endif
#if defined(CONFIG_USB_SWITCH_TSU6712)
    {
            I2C_BOARD_INFO("tsu6712", TSU6712_ADDRESS),
            .platform_data = NULL,
            .irq            = irqpin2irq(GPIO_MUS_INT),
    },
#endif
#if defined(CONFIG_CHARGER_SMB328A)
    {
            I2C_BOARD_INFO("smb328a", SMB328A_ADDRESS),
            .irq            = irqpin2irq(GPIO_CHG_INT),
    },
#endif
};

#ifdef CONFIG_VIBRATOR_ISA1000A
#include <linux/isa1000a_haptic.h>
/******************************************************/
/*
 * ISA1000 HAPTIC MOTOR Driver IC.
 * MOTOR Resonance frequency: 205HZ.
 * Input PWM Frequency: 205 * 128 = 26240 HZ.
 * PWM_period_ns = 1000000000/26240 = 38109.
 * PWM Enable GPIO number = 189.
*/
/******************************************************/

#define GPIO_MOTOR_EN	GPIO_PORT226

static int isa1000_enable(bool en)
{
	return gpio_direction_output(GPIO_MOTOR_EN, en);
}

static struct platform_isa1000_vibrator_data isa1000_vibrator_data = {
	.gpio_en	= isa1000_enable,
	.pwm_name	= "TPU0TO0",
	.pwm_duty	= 542,
	.pwm_period_ns	= 580, /*0x244, VIB_CYC*/
	.polarity	= 0,
	.regulator_id	= "vibldo_uc",
};

static struct platform_device isa1000_device = {
	.name     = "isa1000-vibrator",
	.id       = -1,
	.dev      =	{
		.platform_data = &isa1000_vibrator_data,
	},
};

static void isa1000_gpio_init(void)
{
	gpio_request(GPIO_MOTOR_EN, "MOTOR_EN");
	gpio_direction_output(GPIO_MOTOR_EN, 0);
	gpio_export(GPIO_MOTOR_EN, 0);
}

static void __init isa1000_vibrator_init(void)
{
	isa1000_gpio_init();
	platform_device_register(&isa1000_device);
}
#endif

#if defined(CONFIG_SEC_CHARGING_FEATURE)
#include <linux/spa_power.h>
#include <linux/spa_agent.h>

/* Samsung charging feature
 +++ for board files, it may contain changeable values */
static struct spa_temp_tb batt_temp_tb[] = {
	{869, -300},			/* -30 */
	{769, -200},			/* -20 */
	{643, -100},            /* -10 */
	{568, -50},				/* -5  */
	{509,   0},             /* 0   */
	{382,  100},            /* 10  */
	{275,  200},            /* 20  */
	{231,  250},            /* 25  */
	{196,  300},            /* 30  */
	{138,  400},            /* 40  */
	{95 ,  500},            /* 50  */
	{68 ,  600},            /* 60  */
	{54 ,  650},            /* 65  */
	{46 ,  700},			/* 70  */
	{34 ,  800},			/* 80  */
};

/*struct spa_power_data spa_power_pdata = {
	.charger_name = "spa_agent_chrg",
	.eoc_current = 100,
	.recharge_voltage = 4150,
	.charging_cur_usb = 500,
	.charging_cur_wall = 800,
	.suspend_temp_hot = 600,
	.recovery_temp_hot = 400,
	.suspend_temp_cold = -50,
	.recovery_temp_cold = 0,
	.charge_timer_limit = CHARGE_TIMER_6HOUR,
	.batt_temp_tb = &batt_temp_tb[0],
	.batt_temp_tb_len = ARRAY_SIZE(batt_temp_tb),
};
EXPORT_SYMBOL(spa_power_pdata);*/
extern struct spa_power_data spa_power_pdata;

static struct platform_device spa_power_device = {
	.name = "spa_power",
	.id = -1,
	.dev.platform_data = &spa_power_pdata,
};

static struct platform_device spa_agent_device={
	.name = "spa_agent",
	.id=-1,
};

static int spa_power_init(void)
{
	platform_device_register(&spa_agent_device);
	platform_device_register(&spa_power_device);
	return 0;
}
#endif

static void PA_devices_init(void)
{

#if defined(CONFIG_USB_SWITCH_TSU6712)
   gpio_request(GPIO_PORT97, NULL);
   gpio_direction_input(GPIO_PORT97);
   gpio_pull_up_port(GPIO_PORT97);
#endif

#if defined(CONFIG_CHARGER_SMB328A)
	if(SEC_RLTE_REV0_2_2 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT103, NULL);
	   gpio_direction_input(GPIO_PORT103);
	   gpio_pull_up_port(GPIO_PORT103);
	}
	else if(SEC_RLTE_REV0_3_1 == sec_rlte_hw_rev)
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull_up_port(GPIO_PORT19);
	}
	else
	{
	   gpio_request(GPIO_PORT19, NULL);
	   gpio_direction_input(GPIO_PORT19);
	   gpio_pull_up_port(GPIO_PORT19);
	}
#endif

#if defined(CONFIG_BATTERY_BQ27425)
   gpio_request(GPIO_PORT105, NULL);
   gpio_direction_input(GPIO_PORT105);
   gpio_pull_up_port(GPIO_PORT105);
#endif

#if defined(CONFIG_VIBRATOR_ISA1000A)
	isa1000_vibrator_init();
#endif

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	spa_power_init();
#endif
}
