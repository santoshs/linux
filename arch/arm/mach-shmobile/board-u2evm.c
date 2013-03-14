#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/hwspinlock.h>
#include <linux/i2c.h>
#include <mach/common.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <linux/smsc911x.h>
#include <linux/platform_data/leds-renesas-tpu.h>
#include <mach/board-u2evm.h>
#include <mach/irqs.h>
#include <mach/board-u2evm-config.h>
#include <mach/poweroff.h>
#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic-tps80032.h>
#include <mach/setup-u2tps80032.h>
#endif
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif
#include <linux/ion.h>
#include <linux/memblock.h>
#include <linux/tpu_pwm.h>
#include <linux/pcm2pwm.h>
#include "board-renesas_wifi.h"
#include <linux/pmic/pmic-ncp6914.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_USB_SWITCH_TSU6712)
#include <linux/tsu6712.h>
#endif
#ifdef CONFIG_MPU_SENSORS_MPU6050B1
#include <linux/mpu.h>
#endif
#ifdef CONFIG_INPUT_YAS_SENSORS
#include <linux/yas.h>
#endif
#ifdef CONFIG_YAS_ACC_MULTI_SUPPORT
#include <linux/yas_accel.h>
#endif
#ifdef CONFIG_OPTICAL_GP2AP020A00F
#include <linux/i2c/gp2ap020.h>
#endif
#ifdef CONFIG_OPTICAL_TAOS_TRITON
#include <linux/i2c/taos.h>
#endif
#include <mach/setup-u2touchkey.h>
#include <mach/setup-u2mxt224.h>

#include <mach/crashlog.h>

#if defined(CONFIG_SEC_DEBUG)
#include <mach/sec_debug.h>
#endif

#if defined(CONFIG_RENESAS_BT)
#include <mach/board-u2evm-renesas-bt.h>
#endif
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
#include <linux/i2c-gpio.h>
#include <linux/nfc/pn544.h>
#endif
#endif
#if defined(CONFIG_SAMSUNG_MHL)
#include "board_mhl_sii8332.c"
#include "board_edid.c"
#include <linux/sii8332_platform.h>
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#if defined(CONFIG_VIBRATOR_ISA1000A)
#include <mach/setup-u2vibrator.h>
#endif
#if defined(CONFIG_SEC_CHARGING_FEATURE)
#include <mach/setup-u2smb328a.h>
#endif
#ifdef ARCH_HAS_READ_CURRENT_TIMER
#include <mach/setup-u2current_timer.h>
#endif
#include <mach/board-u2evm.h>

#ifdef CONFIG_ARCH_R8A7373
#include <mach/setup-u2stm.h>
#endif

#include <sound/a2220.h>
#include <mach/sbsc.h>

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
static void mpu_power_on(int onoff);
#endif

#ifdef CONFIG_INPUT_YAS_SENSORS
static void yas_power_on(int onoff);
#endif
#if defined(CONFIG_OPTICAL_GP2AP020A00F)
static void gp2a_power_on(int onoff);
#endif
#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || \
	defined(CONFIG_OPTICAL_TAOS_TRITON) || \
	defined(CONFIG_OPTICAL_GP2AP020A00F) || \
	defined(CONFIG_INPUT_YAS_SENSORS)
enum {
	SNS_PWR_OFF,
	SNS_PWR_ON,
	SNS_PWR_KEEP
};
#endif



#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || \
	defined(CONFIG_OPTICAL_TAOS_TRITON) || \
	defined(CONFIG_OPTICAL_GP2AP020A00F) || \
	defined(CONFIG_INPUT_YAS_SENSORS)
static void sensor_power_on_vdd(int);
#endif





#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)
#define ENT_TPS80032_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)

#if defined(CONFIG_RENESAS_GPS)
struct class *gps_class;
#endif

static int proc_read_board_rev(char *page, char **start, off_t off,
					int count, int *eof, void *data)
{
	count = snprintf(page, count, "%x", u2_board_rev);
	return count;
}

void (*shmobile_arch_reset)(char mode, const char *cmd);

#if defined(CONFIG_MFD_D2153)
static struct regulator *emmc_regulator;

void d2153_mmcif_pwr_control(int onoff)
{
	int ret;

	printk(KERN_EMERG "%s %s\n", __func__, (onoff) ? "on" : "off");
	if(emmc_regulator == NULL)
	{
		printk(" %s, %d \n", __func__, __LINE__ );			
		emmc_regulator = regulator_get(NULL, "vmmc"); 
		if(IS_ERR(emmc_regulator)){
			printk("can not get vmmc regulator\n");
			return;
		}
	}
	if(onoff==1)
	{
		printk(" %s, %d vmmc On\n", __func__, __LINE__ );	
		ret = regulator_enable(emmc_regulator);
		printk(KERN_INFO "regulator_enable ret = %d\n", ret);
	}
	else
	{
		printk("%s, %d vmmc Off\n", __func__, __LINE__ );	
		ret = regulator_disable(emmc_regulator);
		printk(KERN_INFO "regulator_disable ret = %d\n", ret);
	}

}
#endif


void mmcif_set_pwr(struct platform_device *pdev, int state)
{
	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
		d2153_mmcif_pwr_control(1);
#endif /* CONFIG_MFD_D2153 */
	}else{
#if defined(CONFIG_PMIC_INTERFACE)
		gpio_set_value(GPIO_PORT227, 1);
#endif /* CONFIG_PMIC_INTERFACE */
	}
}

void mmcif_down_pwr(struct platform_device *pdev)
{
	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
		d2153_mmcif_pwr_control(0);
#endif /* CONFIG_MFD_D2153 */
	}else{
#if defined(CONFIG_PMIC_INTERFACE)
		gpio_set_value(GPIO_PORT227, 0);
#endif /* CONFIG_PMIC_INTERFACE */
	}
}



#if defined(CONFIG_MPU_SENSORS_MPU6050B1) || defined(CONFIG_INPUT_YAS_SENSORS) || \
	defined(CONFIG_OPTICAL_TAOS_TRITON) || defined(CONFIG_OPTICAL_GP2AP020A00F)

#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)
static int gpiosensor = -1;
#else
static struct regulator *vsensor_3V;
#endif

static void sensor_power_on_vdd(int onoff)
{
       pr_err("%s: start ",__func__);

#if defined (CONFIG_MACH_U2EVM_SR_REV021) || defined (CONFIG_MACH_U2EVM_SR_REV022)

      if(gpiosensor < 0) {
          gpiosensor = gpio_request(GPIO_PORT9, "SENSOR_LDO");
	    if (gpiosensor < 0)
          {
		pr_err("SENSOR_LDO_EN gpio_request was failed\n");
             return;
           }
	gpio_pull_off_port(GPIO_PORT9);
      }

	if (onoff == SNS_PWR_ON) {
            gpio_direction_output(GPIO_PORT9,1);
            pr_err("%s: power on ",__func__);
	} else if ((onoff == SNS_PWR_OFF)) {
            //gpio_direction_output(GPIO_PORT9,0); // TEMP  fix for "inv_i2c_read error"
            pr_err("%s: power off ",__func__);
	}
#else
	int ret;

       pr_err("%s: start ",__func__);

	if (!vsensor_3V) {
		if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
			vsensor_3V = regulator_get(NULL, "sensor_3v");
#endif /* CONFIG_MFD_D2153 */
		} else {
#if defined(CONFIG_PMIC_INTERFACE)
			vsensor_3V = regulator_get(NULL, "vdd_touch");
#endif /* CONFIG_PMIC_INTERFACE */
		}
		pr_err("%s: regulator_get ",__func__);
		if (IS_ERR(vsensor_3V))
			return ;

		ret = regulator_set_voltage(vsensor_3V, 3000000, 3000000);
		if (ret)
			pr_err("%s: error vsensor_3V setting voltage ret=%d\n",__func__, ret);
	}

	if (onoff == SNS_PWR_ON) {
		ret = regulator_enable(vsensor_3V);
		pr_err("%s: regulator_enable ",__func__);
		if (ret)
			pr_err("%s: error enabling regulator\n", __func__);
	} else if ((onoff == SNS_PWR_OFF)) {
		if (regulator_is_enabled(vsensor_3V)) {
			ret = regulator_disable(vsensor_3V);
			if (ret)
				pr_err("%s: error vsensor_3V enabling regulator\n",__func__);
		}
	}
#endif
}
#endif


#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
static void mpu_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#ifdef CONFIG_INPUT_YAS_SENSORS
static void yas_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_GP2AP020A00F)
static void gp2a_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
static void taos_power_on(int onoff)
{
	sensor_power_on_vdd(onoff);
}
#endif

#if defined(CONFIG_OPTICAL_GP2A) || defined(CONFIG_OPTICAL_GP2AP020A00F)
static void gp2a_led_onoff(int onoff)
{

}
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
static void taos_led_onoff(int onoff)
{

}
#endif

#ifdef CONFIG_MPU_SENSORS_MPU6050B1
struct mpu_platform_data mpu6050_data = {
	.int_config = 0x10,
	.orientation = {-1, 0, 0,
			0, -1, 0,
			0, 0, 1},
	.poweron = mpu_power_on,
	};
	/* compass */
static struct ext_slave_platform_data inv_mpu_yas530_data = {
	.bus            = EXT_SLAVE_BUS_PRIMARY,
	.orientation = {1, 0, 0,
			0, 1, 0,
			0, 0, 1},
	};
#endif


#if defined(CONFIG_OPTICAL_GP2AP020A00F)
#define GPIO_ALS_INT	 108
static struct gp2a_platform_data opt_gp2a_data = {
	.gp2a_led_on	= gp2a_led_onoff,
	.power_on = gp2a_power_on,
	.p_out = GPIO_ALS_INT,
	.addr = 0x72>>1,
	.version = 0,
};

static struct platform_device opt_gp2a = {
	.name = "gp2a-opt",
	.id = -1,
	.dev        = {
		.platform_data  = &opt_gp2a_data,
	},
};
#endif

#if defined(CONFIG_OPTICAL_TAOS_TRITON)
#define GPIO_ALS_INT	 108
static void taos_power_on(int onoff);
static void taos_led_onoff(int onoff);

static struct taos_platform_data taos_pdata = {
	.power	= taos_power_on,
	.led_on	=	taos_led_onoff,
	.als_int = GPIO_ALS_INT,
	.prox_thresh_hi = 650,
	.prox_thresh_low = 510,
	.als_time = 0xED,
	.intr_filter = 0x33,
	.prox_pulsecnt = 0x08,
	.prox_gain = 0x28,
	.coef_atime = 50,
	.ga = 117,
	.coef_a = 1000,
	.coef_b = 1600,
	.coef_c = 660,
	.coef_d = 1250,
};
#endif

#if defined(CONFIG_INPUT_YAS_SENSORS)
static struct platform_device yas532_orient_device = {
	.name			= "orientation",
};
#endif


static struct i2c_board_info __initdata i2c2_devices[] = {
#ifdef CONFIG_MPU_SENSORS_MPU6050B1
	{
		I2C_BOARD_INFO("mpu6050", 0x68),
		.irq = R8A7373_IRQC_IRQ(46),
		.platform_data = &mpu6050_data,
	 },
#endif
#ifdef CONFIG_INPUT_YAS_SENSORS
	{
#ifdef CONFIG_YAS_ACC_MULTI_SUPPORT
		I2C_BOARD_INFO("accelerometer", 0x18),
		.platform_data = &accel_pdata,
#else
		I2C_BOARD_INFO("accelerometer", 0x19),
#endif
	},
	{
		I2C_BOARD_INFO("geomagnetic", 0x2e),
	},
#endif
#if defined(CONFIG_OPTICAL_GP2AP020A00F)
	{
		I2C_BOARD_INFO("gp2a", 0x72>>1),
	},
#elif defined(CONFIG_OPTICAL_TAOS_TRITON)
	{
		I2C_BOARD_INFO("taos", 0x39),
		.platform_data = &taos_pdata,
	},
#endif
};

#if defined(CONFIG_BATTERY_BQ27425)
#define BQ27425_ADDRESS (0xAA >> 1)
#define GPIO_FG_INT 44
#endif

#if defined(CONFIG_CHARGER_SMB328A)
#define SMB328A_ADDRESS (0x69 >> 1)
#define GPIO_CHG_INT 19
#endif

#if defined(CONFIG_RENESAS_GPS)

static ssize_t GNSS_NRST_value_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	int value = gpio_get_value(GPIO_PORT10);

	return sprintf(buf, "%d\n", value);
}

static ssize_t GNSS_NRST_value_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	long value;
	int  ret;

	ret = strict_strtol(buf, 0, &value);
	if (ret < 0)
		return ret;

	if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_MFD_D2153
		if (1 == value)
			d2153_clk32k_enable(1);		// on
		else
			d2153_clk32k_enable(0);		// off
#endif /* CONFIG_MFD_D2153 */
	} else {
#if defined(CONFIG_PMIC_INTERFACE)
		if (1 == value)
			pmic_clk32k_enable(CLK32KG, TPS80032_STATE_ON);
		else
			pmic_clk32k_enable(CLK32KG, TPS80032_STATE_OFF);
#endif /* CONFIG_PMIC_INTERFACE */
	}

	printk(KERN_ALERT "%s: %d\n", __func__, value);

	gpio_set_value(GPIO_PORT10, value);

	return count;
}

DEVICE_ATTR(value_nrst, 0644, GNSS_NRST_value_show, GNSS_NRST_value_store);

static ssize_t GNSS_EN_value_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int value = gpio_get_value(GPIO_PORT11);

	return sprintf(buf, "%d\n", value);
}

static ssize_t GNSS_EN_value_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	long value;
	int  ret;

	ret = strict_strtol(buf, 0, &value);
	if (ret < 0)
		return ret;

	printk(KERN_ALERT "%s: %d\n", __func__, value);

	gpio_set_value(GPIO_PORT11, value);

	return count;
}

DEVICE_ATTR(value_en, 0644, GNSS_EN_value_show, GNSS_EN_value_store);

static const struct attribute *GNSS_NRST_attrs[] = {
	&dev_attr_value_nrst.attr,
	NULL,
};

static const struct attribute_group GNSS_NRST_attr_group = {
	.attrs = (struct attribute **) GNSS_NRST_attrs,
};

static const struct attribute *GNSS_EN_attrs[] = {
	&dev_attr_value_en.attr,
	NULL,
};

static const struct attribute_group GNSS_EN_attr_group = {
	.attrs = (struct attribute **) GNSS_EN_attrs,
};

static void gps_gpio_init(void)
{
	struct device *gps_dev;
	struct device *gnss_nrst_dev;
	struct device *gnss_en_dev;
	int    status = -EINVAL;

	gps_class = class_create(THIS_MODULE, "gps");
	if (IS_ERR(gps_class)) {
		pr_err("Failed to create class(sec)!\n");
		return PTR_ERR(gps_class);
	}
	BUG_ON(!gps_class);

	gps_dev = device_create(gps_class, NULL, 0, NULL, "device_gps");
	BUG_ON(!gps_dev);

	gnss_nrst_dev = device_create(gps_class, gps_dev, 0, NULL, "GNSS_NRST");
	BUG_ON(!gnss_nrst_dev);

	gnss_en_dev = device_create(gps_class, gps_dev, 0, NULL, "GNSS_EN");
	BUG_ON(!gnss_en_dev);

	status = sysfs_create_group(&gnss_nrst_dev->kobj,
				    &GNSS_NRST_attr_group);

	if (status)
		pr_debug("%s: status for sysfs_create_group %d\n",
			 __func__, status);

	status = sysfs_create_group(&gnss_en_dev->kobj, &GNSS_EN_attr_group);

	if (status)
		pr_debug("%s: status for sysfs_create_group %d\n",
			 __func__, status);

	printk(KERN_ALERT "gps_gpio_init!!");

		gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
		gpio_pull_up_port(GPIO_PORT79);
		gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
		gpio_pull_off_port(GPIO_PORT78);
		gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
		gpio_pull_up_port(GPIO_PORT77);
		gpio_request(GPIO_FN_SCIFB1_RTS, NULL);
		gpio_pull_off_port(GPIO_PORT76);

		/* GPS Settings */
		gpio_request(GPIO_PORT10, "GNSS_NRST");
		gpio_pull_off_port(GPIO_PORT10);
		gpio_direction_output(GPIO_PORT10, 0);

		gpio_request(GPIO_PORT11, "GNSS_EN");
		gpio_pull_off_port(GPIO_PORT11);
		gpio_direction_output(GPIO_PORT11, 0);

	printk("gps_gpio_init done!!\n");
}
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
                .irq            = R8A7373_IRQC_IRQ(GPIO_FG_INT),
        },
#endif
#if defined(CONFIG_USB_SWITCH_TSU6712)
    {
		I2C_BOARD_INFO("tsu6712", TSU6712_ADDRESS),
			.platform_data = NULL,
			.irq            = R8A7373_IRQC_IRQ(GPIO_MUS_INT),
    },
#endif
#if defined(CONFIG_CHARGER_SMB328A)
    {
		I2C_BOARD_INFO("smb328a", SMB328A_ADDRESS),
/*			.platform_data = &tsu6712_pdata,*/
/*            .irq            = irqpin2irq(GPIO_CHG_INT),*/
    },
#endif
};

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
static void mpl_init(void)
{
	int rc = 0;
	rc = gpio_request(GPIO_PORT107, "MPUIRQ");
	if (rc < 0) {
		pr_err("GPIO_MPU3050_INT gpio_request was failed\n");
		return;
	}
	gpio_direction_input(GPIO_PORT107);
	gpio_pull_down_port(GPIO_PORT107);
}
#endif

#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC  

#define NFC_EN_GPIO         GPIO_PORT12
#define NFC_IRQ_GPIO        GPIO_PORT13
#define NFC_FIRM_GPIO       GPIO_PORT101
#define NFC_I2C_SDA_GPIO	GPIO_PORT274      
#define NFC_I2C_SCL_GPIO	GPIO_PORT273
#define NFC_I2C_BUS_ID		(8) 

static struct i2c_gpio_platform_data pn544_i2c_gpio_data = {
	.sda_pin = NFC_I2C_SDA_GPIO,
	.scl_pin =  NFC_I2C_SCL_GPIO,
 	.udelay = 1,  
};

static struct platform_device pn544_i2c_gpio_device = {
 	.name = "i2c-gpio",
 	.id = NFC_I2C_BUS_ID,
 	.dev = {
	.platform_data  = &pn544_i2c_gpio_data,
	},
};

static struct pn544_i2c_platform_data pn544_pdata = {
 	.irq_gpio 	= NFC_IRQ_GPIO,
 	.ven_gpio = NFC_EN_GPIO,
 	.firm_gpio = NFC_FIRM_GPIO,
};
 
static struct i2c_board_info pn544_info[] __initdata = {
{
	I2C_BOARD_INFO("pn544", 0x2b),
	.irq = R8A7373_IRQC_IRQ(NFC_IRQ_GPIO),
	.platform_data = &pn544_pdata,
 	},
};

#endif
#endif


struct a2220_platform_data  u2evm_a2220_data = {
	.a2220_hw_init = NULL,
	.gpio_reset = GPIO_PORT44,
	.gpio_wakeup = GPIO_PORT26,
};


/* I2C */

static struct i2c_board_info __initdata i2c0_devices_d2153[] = {
#if defined(CONFIG_MFD_D2153)
	{
		// for D2153 PMIC driver
		I2C_BOARD_INFO("d2153", D2153_PMIC_I2C_ADDR),
		.platform_data = &d2153_pdata,
		.irq = R8A7373_IRQC_IRQ(28),
	},
#endif /* CONFIG_MFD_D2153 */
};

static struct i2c_board_info __initdata i2c0_devices[] = {
#ifdef CONFIG_PMIC_INTERFACE
	{
  		I2C_BOARD_INFO("tps80032-power", 0x48),
		.platform_data = &tps_platform,
  	},
  	{
  		I2C_BOARD_INFO("tps80032-battery", 0x49),
		.irq = R8A7373_IRQC_IRQ(28),
  	},
  	{
  		I2C_BOARD_INFO("tps80032-dvs", 0x12),
  	},
  	{
  		I2C_BOARD_INFO("tps80032-jtag", 0x4A),
  	},
#else /* CONFIG_PMIC_INTERFACE */
	{
		I2C_BOARD_INFO("tps80032", 0x4A),
		.irq		= R8A7373_IRQC_IRQ(28),
		.platform_data	= &tps_platform,
	},
#endif /* CONFIG_PMIC_INTERFACE */
};

static struct i2c_board_info i2c4_devices[] = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data = &mxt224_platform_data,
		.irq	= R8A7373_IRQC_IRQ(32),
	},
	{
		I2C_BOARD_INFO("sec_touch", 0x48),
		.irq	= R8A7373_IRQC_IRQ(32),
	},
};

static struct NCP6914_platform_data ncp6914info= {
	.subpmu_pwron_gpio = GPIO_PORT3,
};

static struct i2c_board_info i2c9gpio_devices[] = {
	{
		I2C_BOARD_INFO("ncp6914", 0x10),//address 20/21
		.irq	= R8A7373_IRQC_IRQ(5),
		.platform_data = &ncp6914info,
	},
};

static struct i2c_board_info i2cm_devices_d2153[] = {
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &u2evm_a2220_data,
	},
};

static struct i2c_board_info i2cm_devices[] = {
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &u2evm_a2220_data,
	},
#if 0
	{
		I2C_BOARD_INFO("led", 0x74),
	},
	{
		I2C_BOARD_INFO("flash", 0x30),
	},
#endif /* 0 */
};
#if 0
i2c_board_info i2cm_devices_es2[] = {
#if 0
	{
	        I2C_BOARD_INFO("led", 0x74),
	},
	{
	        I2C_BOARD_INFO("flash", 0x30),
	},
#endif
};
#endif


static struct platform_device *gpio_i2c_devices[] __initdata = {
#if defined(CONFIG_SAMSUNG_MHL)
	&mhl_i2c_gpio_device,
#endif	
};

static struct map_desc u2evm_io_desc[] __initdata = {
	{
		.virtual	= 0xe6000000,
		.pfn		= __phys_to_pfn(0xe6000000),
		.length		= SZ_256M,
		.type		= MT_DEVICE
	},
	{
		/*
		 * Create 4 MiB of virtual address hole within a big 1:1 map
		 * requested above, which is dedicated for the RT-CPU driver.
		 *
		 * According to the hardware manuals, physical 0xefc00000
		 * space is reserved for Router and a data abort error will
		 * be generated if access is made there.  So this partial
		 * mapping change won't be a problem.
		 */
		.virtual        = 0xefc00000,
		.pfn            = __phys_to_pfn(0xffc00000),
		.length         = SZ_4M,
		.type           = MT_DEVICE
	},
};

#ifdef CONFIG_U2_STM_ETR_TO_SDRAM
static int wait_for_coresight_access_lock(u32 base)
{
        int retval = -1;
        int timeout = 512;
        int i;
        __raw_writel(0xc5acce55, base + 0xFB0); /* Lock Access */
        for (i = 0; i < timeout && retval; i++) {
                if ((__raw_readl(base + 0xFB4) & 2) == 0) retval = 0;
        }
        printk("wait_for_coresight_access_lock %d\n", retval);
        return retval;
}
#endif


static void irqc_set_chattering(int pin, int timing)
{
	u32 val;
	u32 *reg;

	reg = (pin >= 32) ? (u32 *)IRQC1_CONFIG_00 : (u32 *)IRQC0_CONFIG_00;
	reg += (pin & 0x1f);

	val = __raw_readl(reg) & ~0x80ff0000;
	__raw_writel(val | (timing << 16) | (1 << 31), reg);
}

#define SBAR2		__io(IO_ADDRESS(0xe6180060))
#define RESCNT2		__io(IO_ADDRESS(0xe6188020))

void u2evm_restart(char mode, const char *cmd)
{
	printk(KERN_INFO "%s\n", __func__);
	shmobile_do_restart(mode, cmd, APE_RESETLOG_U2EVM_RESTART);
}
int sec_rlte_hw_rev;

 
 
#define STBCHRB3		0xE6180043
/* SBSC register address */
#define CPG_PLL3CR_1040MHZ	(0x27000000)
#define CPG_PLLECR_PLL3ST	(0x00000800)
static void __iomem *sbsc_sdmracr1a;

static void __init u2evm_init(void)
{
	int stm_select = -1;	// Shall tell how to route STM traces. See setup-u2stm.c for details.
	void __iomem *sbsc_sdmra_28200 = 0;
	void __iomem *sbsc_sdmra_38200 = 0;

	/* ES2.02 / LPDDR2 ZQ Calibration Issue WA */
	u8 reg8 = __raw_readb(STBCHRB3);
	if ((reg8 & 0x80) && ((system_rev & 0xFFFF) >= 0x3E12)) {
		printk(KERN_ALERT "< %s >Apply for ZQ calibration\n", __func__);
		printk(KERN_ALERT "< %s > Before CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(PLL3CR));
		sbsc_sdmracr1a   = ioremap(SBSC_BASE + 0x400088, 0x4);
		sbsc_sdmra_28200 = ioremap(SBSC_BASE + 0x528200, 0x4);
		sbsc_sdmra_38200 = ioremap(SBSC_BASE + 0x538200, 0x4);
		if (sbsc_sdmracr1a && sbsc_sdmra_28200 && sbsc_sdmra_38200) {
			SBSC_Init_520Mhz();
			__raw_writel(SDMRACR1A_ZQ, sbsc_sdmracr1a);
			__raw_writel(SDMRA_DONE, sbsc_sdmra_28200);
			__raw_writel(SDMRA_DONE, sbsc_sdmra_38200);
		} else {
			printk(KERN_ERR "%s: ioremap failed.\n", __func__);
		}
		printk(KERN_ALERT "< %s > After CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(PLL3CR));
		if(sbsc_sdmracr1a)
			iounmap(sbsc_sdmracr1a);
		if(sbsc_sdmra_28200)
			iounmap(sbsc_sdmra_28200);
		if(sbsc_sdmra_38200)
			iounmap(sbsc_sdmra_38200);
	}

	r8a7373_pinmux_init();

	/* set board version */
	u2_board_rev = read_board_rev();

	create_proc_read_entry("board_revision", 0444, NULL,
					proc_read_board_rev, NULL);

	r8a7373_add_standard_devices();

	r8a7373_hwlock_gpio = hwspin_lock_request_specific(SMGPIO);
	r8a7373_hwlock_cpg = hwspin_lock_request_specific(SMCPG);
	r8a7373_hwlock_sysc = hwspin_lock_request_specific(SMSYSC);
	pinmux_hwspinlock_init(r8a7373_hwlock_gpio);

	if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{

		*GPIO_DRVCR_SD0 = 0x0022;
		*GPIO_DRVCR_SIM1 = 0x0022;
		*GPIO_DRVCR_SIM2 = 0x0022;
	}
	shmobile_arch_reset = u2evm_restart;
	printk(KERN_INFO "%s hw rev : %d\n", __func__, u2_board_rev);

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* SCIFB0 */
	gpio_request(GPIO_FN_SCIFB0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_RXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_CTS_, NULL);
	gpio_request(GPIO_FN_SCIFB0_RTS_, NULL);

#ifdef CONFIG_KEYBOARD_SH_KEYSC
	/* enable KEYSC */
	gpio_request(GPIO_FN_KEYIN0, NULL);
	gpio_request(GPIO_FN_KEYIN1, NULL);
	gpio_request(GPIO_FN_KEYIN2, NULL);
	gpio_request(GPIO_FN_KEYIN3, NULL);
	gpio_request(GPIO_FN_KEYIN4, NULL);
	gpio_request(GPIO_FN_KEYIN5, NULL);
	gpio_request(GPIO_FN_KEYIN6, NULL);

	gpio_pull_up_port(GPIO_PORT44);
	gpio_pull_up_port(GPIO_PORT45);
	gpio_pull_up_port(GPIO_PORT46);
	gpio_pull_up_port(GPIO_PORT47);
	gpio_pull_up_port(GPIO_PORT48);
	gpio_pull_up_port(GPIO_PORT96);
	gpio_pull_up_port(GPIO_PORT97);
#endif
	/* MMC0 */
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
	gpio_request(GPIO_FN_MMCRST, NULL);

	/* Disable GPIO Enable at initialization */
	
	// ===== CWS GPIO =====
	
	// GPS Reset
	gpio_request(GPIO_PORT10, NULL);
	gpio_direction_output(GPIO_PORT10, 0);
	
	// GPS Enable
	gpio_request(GPIO_PORT11, NULL);
	gpio_direction_output(GPIO_PORT11, 0);

#if defined(CONFIG_RENESAS_NFC)
	// NFC Firmware
	gpio_request(GPIO_PORT101, NULL);
	gpio_direction_output(GPIO_PORT101, 0);
#endif
	// ===== Misc GPIO =====

	// Sensor LDO Enable
	gpio_request(GPIO_PORT9, NULL);
	if (u2_board_rev >= 5) {
		/* do nothing */
	} else {
		gpio_direction_output(GPIO_PORT9, 0);
	}
	/* End */

	gpio_direction_none_port(GPIO_PORT309);

#ifdef CONFIG_ARCH_R8A7373
        stm_select = u2evm_init_stm_select();
#else
        stm_select = -1;
#endif

	if (0 != stm_select) {
		/* If STM Traces go to SDHI1 or NOWHERE, then SDHI0 can be used for SD-Card */
		/* SDHI0 */
		gpio_request(GPIO_FN_SDHID0_0, NULL);
		gpio_request(GPIO_FN_SDHID0_1, NULL);
		gpio_request(GPIO_FN_SDHID0_2, NULL);
		gpio_request(GPIO_FN_SDHID0_3, NULL);
		gpio_request(GPIO_FN_SDHICMD0, NULL);
		gpio_direction_none_port(GPIO_PORT326);
		gpio_request(GPIO_FN_SDHICLK0, NULL);
		gpio_request(GPIO_PORT327, NULL);
		gpio_direction_input(GPIO_PORT327);
		gpio_pull_off_port(GPIO_PORT327);
		irq_set_irq_type(R8A7373_IRQC_IRQ(50), IRQ_TYPE_EDGE_BOTH);
		gpio_set_debounce(GPIO_PORT327, 1000);	/* 1msec */
	}

	__raw_writel(__raw_readl(MSEL3CR) | (1<<27), MSEL3CR); /* ES2.0: SIM powers */

	if (1 != stm_select) {
		/* SDHI1 */
		gpio_request(GPIO_FN_SDHID1_0, NULL);
		gpio_request(GPIO_FN_SDHID1_1, NULL);
		gpio_request(GPIO_FN_SDHID1_2, NULL);
		gpio_request(GPIO_FN_SDHID1_3, NULL);
		gpio_request(GPIO_FN_SDHICMD1, NULL);
		gpio_request(GPIO_FN_SDHICLK1, NULL);

		gpio_pull_up_port(GPIO_PORT293);
		gpio_pull_up_port(GPIO_PORT292);
		gpio_pull_up_port(GPIO_PORT291);
		gpio_pull_up_port(GPIO_PORT290);
		gpio_pull_up_port(GPIO_PORT289);
		// move gpio request to board-renesas_wifi.c
		
		/* WLAN Init API call */
#ifdef CONFIG_RENESAS_WIFI		
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
		printk(KERN_ERR "Calling WLAN_INIT!\n");

	 	renesas_wlan_init();
		printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif
#endif	
	}

	/* touch key Interupt */
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);

	gpio_pull_up_port(GPIO_PORT104);

	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);

	if (u2_board_rev >= 4) {
		gpio_pull_off_port(GPIO_PORT84);
		gpio_pull_off_port(GPIO_PORT85);
	}

	if(u2_board_rev < 4) {
		gpio_request(GPIO_FN_I2C_SCL1H, NULL);
		gpio_request(GPIO_FN_I2C_SDA1H, NULL);
	}

	/* PMIC */
	gpio_request(GPIO_PORT0, NULL);	/* MSECURE */
	if (u2_get_board_rev() >= 5) {
		/* do nothing */
	} else {
		gpio_direction_output(GPIO_PORT0, 1);
	}
	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);

	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_MFD_D2153)
		irq_set_irq_type(R8A7373_IRQC_IRQ(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_MFD_D2153 */
	} else {
#if defined(CONFIG_PMIC_INTERFACE)
		irq_set_irq_type(R8A7373_IRQC_IRQ(28), IRQ_TYPE_EDGE_FALLING);
#else  /* CONFIG_PMIC_INTERFACE */
		irq_set_irq_type(R8A7373_IRQC_IRQ(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_PMIC_INTERFACE */
	}

#if defined(CONFIG_USB_SWITCH_TSU6712)
	gpio_request(GPIO_PORT97, NULL);
	gpio_direction_input(GPIO_PORT97);
	gpio_pull_up_port(GPIO_PORT97);
#endif

#if defined(CONFIG_CHARGER_SMB328A)
	if (RLTE_BOARD_REV_0_2_2 == u2_board_rev) {
		gpio_request(GPIO_PORT103, NULL);
		gpio_direction_input(GPIO_PORT103);
		gpio_pull_up_port(GPIO_PORT103);
	} else {
		gpio_request(GPIO_PORT19, NULL);
		gpio_direction_input(GPIO_PORT19);
		gpio_pull_up_port(GPIO_PORT19);
	}
#endif

#if defined(CONFIG_BATTERY_BQ27425)
	gpio_request(GPIO_PORT105, NULL);
	gpio_direction_input(GPIO_PORT105);
	if (u2_get_board_rev() >= 5) {
		/* do nothing */
	} else {
		gpio_pull_up_port(GPIO_PORT105);
	}
#endif

		/*TSP LDO Enable*/
	gpio_request(GPIO_PORT30, NULL);
	if (u2_board_rev >= 5) {
		/* do nothing */
	} else if (u2_board_rev >= 4)
		gpio_direction_output(GPIO_PORT30, 0);
	else
		gpio_direction_output(GPIO_PORT30, 1);
	/* Touch */
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	gpio_pull_up_port(GPIO_PORT32);

	USBGpio_init();

#ifdef CONFIG_SPI_SH_MSIOF
	if(u2_board_rev < 4) {
		/* enable MSIOF0 */
		gpio_request(GPIO_FN_MSIOF0_TXD, NULL);
		gpio_request(GPIO_FN_MSIOF0_SYNC, NULL);
		gpio_request(GPIO_FN_MSIOF0_SCK, NULL);
		gpio_request(GPIO_FN_MSIOF0_RXD, NULL);
	}
#endif

	/* enable sound */
	gpio_request(GPIO_FN_FSIAISLD, "sound");
	gpio_request(GPIO_FN_FSIAOBT, "sound");
	gpio_request(GPIO_FN_FSIAOLR, "sound");
	gpio_request(GPIO_FN_FSIAOSLD, "sound");

	gpio_request(GPIO_FN_FSIBISLD, "sound");
	gpio_request(GPIO_FN_FSIBOBT, "sound");
	gpio_request(GPIO_FN_FSIBOLR, "sound");
	gpio_request(GPIO_FN_FSIBOSLD, "sound");

	gpio_request(GPIO_PORT24, NULL);
	gpio_direction_input(GPIO_PORT24);
	gpio_pull_down_port(GPIO_PORT24);

#ifndef CONFIG_ARM_TZ
	r8a7373_l2cache_init();
#else
/*
	*In TZ-Mode of R-Mobile U2, it must notify the L2 cache
	*related info to Secure World. However, SEC_HAL driver is not
	*registered at the time of L2$ init because "r8a7373_l2cache_init()"
	*function called more early.
*/
	l2x0_init_later();
#endif
	camera_init(u2_board_rev);

	if(u2_get_board_rev() >= 5) {
		gpio_key_init(stm_select,
			u2_board_rev,
			u2_board_rev,
			u2evm_devices_stm_sdhi0,
			ARRAY_SIZE(u2evm_devices_stm_sdhi0),
			u2evm_devices_stm_sdhi1,
			ARRAY_SIZE(u2evm_devices_stm_sdhi1),
			u2evm_devices_stm_none,
			ARRAY_SIZE(u2evm_devices_stm_none));
	}

#if defined(CONFIG_MPU_SENSORS_MPU6050B1)
	mpl_init();
#endif

	if(u2_get_board_rev() < 4)
		touchkey_init_hw();

#if defined (CONFIG_SAMSUNG_MHL)
	board_mhl_init();
	board_edid_init();
#endif
	if(u2_get_board_rev() >= 5) {
		i2c_register_board_info(0, i2c0_devices_d2153, ARRAY_SIZE(i2c0_devices_d2153));
	} else {
		i2c_register_board_info(0, i2c0_devices, ARRAY_SIZE(i2c0_devices));
	}
	i2c_register_board_info(2, i2c2_devices, ARRAY_SIZE(i2c2_devices));
	i2c_register_board_info(3, i2c3_devices, ARRAY_SIZE(i2c3_devices));
	i2c_register_board_info(4, i2c4_devices, ARRAY_SIZE(i2c4_devices));
	if(u2_get_board_rev() >= 5) {
		i2c_register_board_info(8, i2cm_devices_d2153, ARRAY_SIZE(i2cm_devices_d2153));
	}else{
		i2c_register_board_info(8, i2cm_devices, ARRAY_SIZE(i2cm_devices));
	}

	/* GPS Init */
#if defined(CONFIG_RENESAS_GPS)
	gps_gpio_init();
#endif
	if(u2_get_board_rev() < 4)
		touchkey_i2c_register_board_info(10);
platform_add_devices(gpio_i2c_devices, ARRAY_SIZE(gpio_i2c_devices));	
	#if defined(CONFIG_VIBRATOR_ISA1000A)
    isa1000_vibrator_init();
#endif

	if (u2_get_board_rev() >= 5) {
#if defined(CONFIG_SEC_CHARGING_FEATURE)
		spa_power_init();
#endif
	}

	printk(KERN_DEBUG "%s\n", __func__);
	crashlog_r_local_ver_write(mmcoops_info.soft_version);
	crashlog_reset_log_write();
	crashlog_init_tmplog();

	if (u2_get_board_rev() <= 4) {
		i2c_register_board_info(9, i2c9gpio_devices, ARRAY_SIZE(i2c9gpio_devices));
	}
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
	i2c_register_board_info(8, pn544_info, ARRAY_SIZE(pn544_info)); 
#endif
#endif

}

static void __init u2evm_reserve(void)
{
	u2evm_ion_adjust();

#if defined(CONFIG_SEC_DEBUG)
        sec_debug_magic_init();
#endif

}


MACHINE_START(U2EVM, "u2evm")
	.reserve	= u2evm_reserve,
	.map_io		= r8a7373_map_io,
	.init_early	= r8a7373_init_early,
	.nr_irqs	= NR_IRQS_LEGACY,
	.init_irq	= r8a7373_init_irq,
	.handle_irq	= gic_handle_irq,
	.init_machine	= u2evm_init,
	.timer		= &shmobile_timer,
	.restart	= u2evm_restart,
MACHINE_END
