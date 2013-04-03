/*
 * arch/arm/mach-shmobile/board-gardalte.c
 *
 * Copyright (C) 2013 Renesas Mobile Corporation
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
/*******************************************************************************
	Board file to support GARDA Products
*******************************************************************************/
#include <asm/hardware/gic.h>
#include <linux/dma-mapping.h>
#include <mach/irqs.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <mach/common.h>
#include <mach/r8a7373.h>
#include <mach/setup-u2usb.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <linux/mmc/host.h>
#include <video/sh_mobile_lcdc.h>
#include <mach/board-gardalte.h>
#include <mach/board-gardalte-config.h>
#include <mach/poweroff.h>
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#endif
#include <linux/ktd259b_bl.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_RENESAS_GPS)|| defined(CONFIG_GPS_CSR_GSD5T)
#include <mach/dev-gps.h>
#endif
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
#include <mach/dev-renesas-nfc.h>
#endif
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#ifdef ARCH_HAS_READ_CURRENT_TIMER
#include <mach/setup-u2current_timer.h>
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#if defined(CONFIG_SEC_DEBUG)
#include <mach/sec_debug.h>
#endif
#include <sound/a2220.h>
#include <linux/i2c/fm34_we395.h>
#include <linux/leds-ktd253ehd.h>
#ifdef CONFIG_BOARD_VERSION_GARDA
#include <linux/leds-regulator.h>
#endif /* CONFIG_BOARD_VERSION_GARDA */
#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#include <linux/broadcom/bcmbt_rfkill.h>
#endif
#ifdef CONFIG_BCM_BT_LPM
#include <linux/broadcom/bcmbt_lpm.h>
#endif
#ifdef CONFIG_BCM_BZHW
#include <linux/broadcom/bcm_bzhw.h>
#endif
#if defined(CONFIG_BCMI2CNFC)
#include <linux/bcmi2cnfc.h>
#endif
#ifdef CONFIG_NFC_PN547
#include <linux/nfc/pn547.h>
#endif /* CONFIG_NFC_PN547	*/
#if defined(CONFIG_SAMSUNG_SENSOR)
#include <mach/dev-sensor.h>
#endif
#if defined(CONFIG_BCMI2CNFC) || defined(CONFIG_NFC_PN547)
#include <mach/dev-nfc.h>
#endif
#define ENT_TPS80031_IRQ_BASE	(IRQPIN_IRQ_BASE + 64)

#ifdef CONFIG_ARCH_R8A7373
#include <mach/setup-u2stm.h>
#endif

#if defined(CONFIG_SEC_CHARGING_FEATURE)
#include <linux/spa_power.h>
#endif

#define STBCHRB3			0xE6180043
#define PHYFUNCTR			IO_ADDRESS(0xe6890104) /* 16-bit */

/* SBSC register address */
#define CPG_PLL3CR_1040MHZ		(0x27000000)
#define CPG_PLLECR_PLL3ST		(0x00000800)

#if defined(CONFIG_CHARGER_SMB328A)
#define SMB328A_ADDRESS (0x69 >> 1)
#define SMB327A_ADDRESS (0xA9 >> 1)
#define GPIO_CHG_INT 19
#endif
#include <mach/sbsc.h>

void (*shmobile_arch_reset)(char mode, const char *cmd);
int renesas_wlan_init(void);

static int proc_read_board_rev(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	count = snprintf(page, count, "%x", u2_board_rev);
	return count;
}

static struct ktd253ehd_led_platform_data ktd253ehd_led_info = {
	.gpio_port = GPIO_PORT47,
};

static struct platform_device led_backlight_device = {
	.name	= "ktd253ehd_led",
	.dev	= {
		.platform_data  = &ktd253ehd_led_info,
	},
};

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#define BCMBT_VREG_GPIO       (GPIO_PORT268)
#define BCMBT_N_RESET_GPIO    (-1)
/* clk32 */
#define BCMBT_AUX0_GPIO       (-1)  
/* UARTB_SEL */
#define BCMBT_AUX1_GPIO       (-1)    

static struct bcmbt_rfkill_platform_data board_bcmbt_rfkill_cfg = {
	.vreg_gpio 		= BCMBT_VREG_GPIO,
	.n_reset_gpio 	= BCMBT_N_RESET_GPIO,
	/* CLK32 */
	.aux0_gpio 		= BCMBT_AUX0_GPIO,  
	/* UARTB_SEL, probably not required */
	.aux1_gpio 		= BCMBT_AUX1_GPIO,  
};

static struct platform_device board_bcmbt_rfkill_device = {
	.name 	= "bcmbt-rfkill",
	.id 	= -1,
	.dev 	= {
		.platform_data=&board_bcmbt_rfkill_cfg,
	},
};
#endif

#ifdef CONFIG_BCM_BZHW

#define GPIO_BT_WAKE 	GPIO_PORT262
#define GPIO_HOST_WAKE 	GPIO_PORT272

static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake 	= GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcm_bzhw_device = {
	.name = "bcm_bzhw",
	.id = -1,
	.dev = {
		.platform_data = &bcm_bzhw_data,
	},
};
#endif

#ifdef CONFIG_BCM_BT_LPM
#define GPIO_BT_WAKE   6 
#define GPIO_HOST_WAKE 14

static struct bcm_bt_lpm_platform_data brcm_bt_lpm_data = {
	.gpio_bt_wake = GPIO_BT_WAKE,
	.gpio_host_wake = GPIO_HOST_WAKE,
};

static struct platform_device board_bcmbt_lpm_device = {
	.name 	= "bcmbt-lpm",
	.id 	= -1,
	.dev 	= {
		.platform_data=&brcm_bt_lpm_data,
	},
};
#endif


struct a2220_platform_data  gardalte_a2220_data = {
	.a2220_hw_init = NULL,
	.gpio_reset = GPIO_PORT44,
	.gpio_wakeup = GPIO_PORT26,
};

struct fm34_platform_data  gardalte_fm34_data = {
	.set_mclk = NULL,
	.gpio_pwdn = GPIO_PORT26,
	.gpio_rst = GPIO_PORT44,
	.gpio_bp = GPIO_PORT46,
};

/* I2C */
static struct i2c_board_info __initdata i2c0_devices_d2153[] = {
#if defined(CONFIG_MFD_D2153)
	{
		/* for D2153 PMIC driver */
		I2C_BOARD_INFO("d2153", D2153_PMIC_I2C_ADDR),
		.platform_data = &d2153_pdata,
		.irq = irqpin2irq(28),
	},
#endif /* CONFIG_MFD_D2153 */
};

static struct platform_ktd259b_backlight_data bcm_ktd259b_backlight_data = {
	.max_brightness = 255,
	.dft_brightness = 143,
	.ctrl_pin = 47,
};

static struct platform_device bcm_backlight_devices = {
	.name	= "panel",
	.id 	= -1,
	.dev 	= {
		.platform_data = &bcm_ktd259b_backlight_data,
	},
};

/* Rhea Ray specific platform devices */
static struct platform_device *guardian__plat_devices[] __initdata = {
	&bcm_backlight_devices,
};

#if defined(CONFIG_USB_SWITCH_TSU6712)
#define TSU6712_ADDRESS (0x4A >> 1)
#define GPIO_MUS_INT 41
#endif

static struct i2c_board_info __initdata i2c3_devices[] = {
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
		.irq            = irqpin2irq(GPIO_CHG_INT),
	},
#endif
};

static struct i2c_board_info i2c4_devices_melfas[] = {
	{
		I2C_BOARD_INFO("sec_touch", 0x48),
		.irq = irqpin2irq(32),
	},
};

#ifdef CONFIG_BOARD_VERSION_GARDA
static struct i2c_board_info i2c4_devices_imagis[] = {
	{
		I2C_BOARD_INFO("IST30XX", 0xA0>>1),
		.irq = irqpin2irq(32),
	},
};

/* Touch Panel auto detection for Garda_Kyle (Melfas and Imagis) */
static struct i2c_client *tsp_detector_i2c_client;

static int __devinit tsp_detector_probe(struct i2c_client *client,
		const struct i2c_device_id * id)
{
	int ret=0;
	struct i2c_adapter *adap = client->adapter;
	struct regulator *touch_regulator;
	unsigned short addr_list_melfas[] = { 0x48, I2C_CLIENT_END };

	touch_regulator = regulator_get(NULL, "vtsp_3v");
	if(IS_ERR(touch_regulator)){
		printk(KERN_ERR "failed to get regulator for Touch Panel");
		return -ENODEV;
	}
	regulator_set_voltage(touch_regulator, 3000000, 3000000); /* 3.0V */
	regulator_enable(touch_regulator);
	msleep(20);

	if ((tsp_detector_i2c_client = i2c_new_probed_device(adap,
						&i2c4_devices_melfas[0],
						addr_list_melfas, NULL))){
		printk(KERN_INFO "Touch Panel: Melfas MMS-13X\n");
	} else {
		tsp_detector_i2c_client = i2c_new_device(adap,
						&i2c4_devices_imagis[0]);
		printk(KERN_INFO "Touch Panel: Imagis IST30XX\n");
	}

	regulator_disable(touch_regulator);
	regulator_put(touch_regulator);
	return ret;
}

static int tsp_detector_remove(struct i2c_client *client)
{
	i2c_unregister_device(tsp_detector_i2c_client);
	tsp_detector_i2c_client = NULL;
	return 0;
}

static struct i2c_device_id tsp_detector_idtable[] = {
	{ "tsp_detector", 0 },
	{},
};
static struct i2c_driver tsp_detector_driver = {
	.driver = {
		.name = "tsp_detector",
	},
	.probe 		= tsp_detector_probe,
	.remove 	= tsp_detector_remove,
	.id_table 	= tsp_detector_idtable,
};

static struct i2c_board_info i2c4_devices_tsp_detector[] = {
	{
		I2C_BOARD_INFO("tsp_detector", 0x7f),
	},
};

static struct led_regulator_platform_data key_backlight_data = {
	.name   = "button-backlight",
};

static struct platform_device key_backlight_device = {
	.name = "leds-regulator",
	.id   = 0,
	.dev  = {
		.platform_data = &key_backlight_data,
	},
};
#endif /* CONFIG_BOARD_VERSION_GARDA */

static struct i2c_board_info i2cm_devices_d2153[] = {
	{
		I2C_BOARD_INFO("audience_a2220", 0x3E),
		.platform_data = &gardalte_a2220_data,
	},
	{
		I2C_BOARD_INFO(FM34_MODULE_NAME, 0x60),
		.platform_data = &gardalte_fm34_data,
	},
};


static struct platform_device *gpio_i2c_devices[] __initdata = {
};

#ifdef CONFIG_U2_STM_ETR_TO_SDRAM
static int wait_for_coresight_access_lock(u32 base)
{
	int i = 0x00;
	int retval = -1;
	int timeout = 512;
	/* Lock Access */
	__raw_writel(0xc5acce55, base + 0xFB0); 
	for (i = 0; i < timeout && retval; i++) {
		if ((__raw_readl(base + 0xFB4) & 2) == 0)
			retval = 0;
	}
	printk("wait_for_coresight_access_lock %d\n", retval);
	return retval;
}
#endif

void gardalte_restart(char mode, const char *cmd)
{
	printk(KERN_INFO "%s\n", __func__);
	shmobile_do_restart(mode, cmd, APE_RESETLOG_U2EVM_RESTART);
}

static void __init gardalte_init(void)
{
	int stm_select = -1;    // Shall tell how to route STM traces. See setup-u2stm.c for details.
	void __iomem *sbsc_sdmra_28200 = 0;
	void __iomem *sbsc_sdmra_38200 = 0;

	/* ES2.02 / LPDDR2 ZQ Calibration Issue WA */

	u8 reg8 = __raw_readb(STBCHRB3);
	u8 i = 0;

	if ((reg8 & 0x80) && ((system_rev & 0xFFFF) >= 0x3E12)) {
		printk(KERN_ALERT "< %s >Apply for ZQ calibration\n", __func__);
		printk(KERN_ALERT "< %s > Before CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(PLL3CR));
		sbsc_sdmracr1a   = ioremap(SBSC_BASE + 0x000088, 0x4);
		sbsc_sdmra_28200 = ioremap(SBSC_BASE + 0x128200, 0x4);
		sbsc_sdmra_38200 = ioremap(SBSC_BASE + 0x438200, 0x4);
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
	shmobile_arch_reset = gardalte_restart;

	printk(KERN_INFO "%s hw rev : %d\n", __func__, u2_board_rev);

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* Bluetooth UART settings (ttySC4) */

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

	/* ===== CWS GPIO ===== */

	/* GPS Reset */
	gpio_request(GPIO_PORT10, NULL);
	gpio_direction_output(GPIO_PORT10, 0);

	/* GPS Enable */
	gpio_request(GPIO_PORT11, NULL);
	gpio_direction_output(GPIO_PORT11, 0);

#if defined(CONFIG_RENESAS_NFC)
	nfc_gpio_init();
#endif

	gpio_direction_none_port(GPIO_PORT309);

#ifdef CONFIG_ARCH_R8A7373
        stm_select = u2evm_init_stm_select();
#else
        stm_select = -1;
#endif

	if (0 != stm_select) {
		/* If STM Traces go to SDHI1 or NOWHERE, then SDHI0 can be used
		   for SD-Card */

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
		irq_set_irq_type(irqpin2irq(50), IRQ_TYPE_EDGE_BOTH);
		gpio_set_debounce(GPIO_PORT327, 1000);	/* 1msec */
	}

        /* ES2.0: SIM powers */
	__raw_writel(__raw_readl(MSEL3CR) | (1<<27), MSEL3CR);

	/* WLAN Init and SDIO device call */
	if (1 != stm_select) {
		/* SDHI1 for WLAN */
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
		/* move gpio request to board-renesas_wifi.c */

		/* WLAN Init API call */
#ifdef CONFIG_RENESAS_WIFI		
#ifdef CONFIG_BRCM_UNIFIED_DHD_SUPPORT
		printk(KERN_ERR "Calling WLAN_INIT!\n");
		renesas_wlan_init();
		printk(KERN_ERR "DONE WLAN_INIT!\n");
#endif	
#endif
		/* add the SDIO device */
	}

	/* touch key Interupt */
	gpio_request(GPIO_PORT104, NULL);
	gpio_direction_input(GPIO_PORT104);

	gpio_pull_up_port(GPIO_PORT104);
	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);

	gpio_pull_off_port(GPIO_PORT84);
	gpio_pull_off_port(GPIO_PORT85);

	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);

#if defined(CONFIG_MFD_D2153)
	irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_MFD_D2153 */


	/* Touch */
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	gpio_pull_up_port(GPIO_PORT32);

	USBGpio_init();

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
	*registered at the time of L2$ init because "r8a7373l2_cache_init()"
	*function called more early.
*/
	l2x0_init_later();
#endif

	camera_init(u2_board_rev);
	gpio_key_init(stm_select, u2_board_rev, u2_board_rev,
			devices_stm_sdhi0,
			ARRAY_SIZE(devices_stm_sdhi0),
			devices_stm_sdhi1,
			ARRAY_SIZE(devices_stm_sdhi1),
			devices_stm_none,
			ARRAY_SIZE(devices_stm_none));

	platform_device_register(&led_backlight_device);

	i2c_register_board_info(0, i2c0_devices_d2153,
					ARRAY_SIZE(i2c0_devices_d2153));

#if defined(CONFIG_SAMSUNG_SENSOR)
	sensor_init();
#endif

#if defined(CONFIG_MACH_GARDALTE)
	if (u2_get_board_rev() == 2) {
		for (i = 0;
			i < (sizeof(i2c3_devices)/sizeof(struct i2c_board_info));
			i++) {
			if (!strcmp(i2c3_devices[i].type, "smb328a")) {
				i2c3_devices[i].addr = SMB327A_ADDRESS;
				break;
			}
		}
	}
#endif
	i2c_register_board_info(3, i2c3_devices, ARRAY_SIZE(i2c3_devices));

#ifdef CONFIG_NFC_PN547
	i2c_register_board_info(8, PN547_info, pn547_info_size()); 
#endif

#ifdef CONFIG_BOARD_VERSION_GARDA
	/* Touch Panel auto detection */
	i2c_add_driver(&tsp_detector_driver);
	i2c_register_board_info(4, i2c4_devices_tsp_detector,
					ARRAY_SIZE(i2c4_devices_tsp_detector));
	platform_device_register(&key_backlight_device);
#else /* CONFIG_BOARD_VERSION_GARDA */

	i2c_register_board_info(4, i2c4_devices_melfas,
					ARRAY_SIZE(i2c4_devices_melfas));
#endif /* CONFIG_BOARD_VERSION_GARDA */

	i2c_register_board_info(8, i2cm_devices_d2153,
					ARRAY_SIZE(i2cm_devices_d2153));

#if defined(CONFIG_GPS_CSR_GSD5T)
	/* GPS Init */
	gps_gpio_init();
#endif

	platform_add_devices(gpio_i2c_devices, ARRAY_SIZE(gpio_i2c_devices));
	platform_add_devices(guardian__plat_devices,
					ARRAY_SIZE(guardian__plat_devices));

#if defined(CONFIG_SEC_CHARGING_FEATURE)
	init_spa_power();
#endif

#if defined(CONFIG_USB_SWITCH_TSU6712)
	gpio_request(GPIO_PORT97, NULL);
	gpio_direction_input(GPIO_PORT97);
	gpio_pull_up_port(GPIO_PORT97);
#endif

#if defined(CONFIG_CHARGER_SMB328A)
	gpio_request(GPIO_PORT19, NULL);
	gpio_direction_input(GPIO_PORT19);
	gpio_pull_up_port(GPIO_PORT19);
#endif

	printk(KERN_DEBUG "%s\n", __func__);
	crashlog_r_local_ver_write(mmcoops_info.soft_version);
	crashlog_reset_log_write();
	crashlog_init_tmplog();
}


static void __init gardalte_reserve(void)
{
	u2evm_ion_adjust();

#if defined(CONFIG_SEC_DEBUG)
	sec_debug_magic_init();
#endif
}

MACHINE_START(U2EVM, "u2evm")
	.map_io			= r8a7373_map_io,//gardalte_map_io,
	.init_irq		= r8a7373_init_irq,//gardalte_init_irq,
        .init_early             = r8a7373_init_early,
        .nr_irqs                = NR_IRQS_LEGACY,
	.handle_irq		= gic_handle_irq,
	.init_machine		= gardalte_init,
	.timer			= &shmobile_timer,//&gardalte_timer,
	.restart		= gardalte_restart,
	.reserve		= gardalte_reserve,
MACHINE_END
