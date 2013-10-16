/*
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
#include <mach/board.h>
#include <mach/board-amethyst-config.h>
#include <mach/poweroff.h>
#include <mach/sbsc.h>
#ifdef CONFIG_MFD_D2153
#include <linux/d2153/core.h>
#include <linux/d2153/pmic.h>
#include <linux/d2153/d2153_battery.h>
#include <linux/d2153/d2153_aad.h>
#endif
#include <mach/dev-wifi.h>
#include <linux/ktd259b_bl.h>
#include <mach/setup-u2spa.h>
#include <mach/setup-u2vibrator.h>
#include <linux/proc_fs.h>
#if defined(CONFIG_RENESAS_GPS)|| defined(CONFIG_GPS_CSR_GSD5T)
#include <mach/dev-gps.h>
#endif
#if defined(CONFIG_RENESAS_NFC)
#ifdef CONFIG_PN544_NFC
#include <mach/dev-renesas-nfc.h>
#endif
#endif
#if defined(CONFIG_SAMSUNG_MHL)
#include <mach/dev-edid.h>
#endif
#ifdef CONFIG_USB_OTG
#include <linux/usb/tusb1211.h>
#endif
#if defined(CONFIG_GPS_BCM4752)
#include <mach/dev-gps.h>
#endif
#if defined(CONFIG_SEC_DEBUG)
#include <mach/sec_debug.h>
#include <mach/sec_debug_inform.h>
#endif
#if defined(CONFIG_SND_SOC_SH4_FSI)
#include <mach/setup-u2audio.h>
#endif /* CONFIG_SND_SOC_SH4_FSI */
#include <linux/i2c/fm34_we395.h>
#include <linux/leds-ktd253ehd.h>
#include <linux/leds-regulator.h>
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
#if defined(CONFIG_PN547_NFC) || defined(CONFIG_NFC_PN547)
#include <linux/nfc/pn547.h>
#endif

#include <mach/dev-sensor.h>

#if defined(CONFIG_BCMI2CNFC) || defined(CONFIG_PN547_NFC)  || defined(CONFIG_NFC_PN547)
#include <mach/dev-nfc.h>
#endif

#include <mach/dev-touchpanel.h>

#ifdef CONFIG_ARCH_R8A7373
#include <mach/setup-u2stm.h>
#endif

#if defined(CONFIG_RT8969) || defined(CONFIG_RT8973)
#include <linux/platform_data/rtmusc.h>
#endif

#if defined(CONFIG_SEC_CHARGING_FEATURE)
#include <linux/spa_power.h>
#endif

#include <mach/sbsc.h>
#include <media/soc_camera.h>
#include <media/sh_mobile_csi2.h>
#include <media/sh_mobile_rcu.h>
#include <media/sr030pc50.h>
#include <media/s5k4ecgx.h>

static int unused_gpios_amethyst[] = {
	GPIO_PORT3, GPIO_PORT6, GPIO_PORT8, GPIO_PORT10,
	GPIO_PORT14, GPIO_PORT17, GPIO_PORT18, GPIO_PORT23,
	GPIO_PORT26, GPIO_PORT29, GPIO_PORT33, GPIO_PORT34,
	GPIO_PORT35, GPIO_PORT36, GPIO_PORT80, GPIO_PORT81,
	GPIO_PORT82, GPIO_PORT83, GPIO_PORT86, GPIO_PORT87,
	GPIO_PORT88, GPIO_PORT89, GPIO_PORT219, GPIO_PORT90,
	GPIO_PORT275, GPIO_PORT276, GPIO_PORT277, GPIO_PORT311,
	GPIO_PORT312, GPIO_PORT140, GPIO_PORT198, GPIO_PORT199,
	GPIO_PORT200, GPIO_PORT201, GPIO_PORT271, GPIO_PORT294,
	GPIO_PORT295, GPIO_PORT296, GPIO_PORT297, GPIO_PORT298,
	GPIO_PORT299, GPIO_PORT44, GPIO_PORT46, GPIO_PORT47,
	GPIO_PORT102, GPIO_PORT103, GPIO_PORT104, GPIO_PORT105,
	GPIO_PORT325, GPIO_PORT72, GPIO_PORT73, GPIO_PORT74,
	GPIO_PORT75, GPIO_PORT141, GPIO_PORT142, GPIO_PORT224,
	GPIO_PORT225, GPIO_PORT226, GPIO_PORT227, GPIO_PORT228,
	GPIO_PORT229, GPIO_PORT230, GPIO_PORT231, GPIO_PORT232,
	GPIO_PORT233, GPIO_PORT234, GPIO_PORT235, GPIO_PORT236,
	GPIO_PORT237, GPIO_PORT238, GPIO_PORT239, GPIO_PORT240,
	GPIO_PORT241, GPIO_PORT242, GPIO_PORT243, GPIO_PORT244,
	GPIO_PORT245, GPIO_PORT246, GPIO_PORT247, GPIO_PORT248,
	GPIO_PORT249, GPIO_PORT250, GPIO_PORT251, GPIO_PORT252,
	GPIO_PORT253, GPIO_PORT254, GPIO_PORT255, GPIO_PORT256,
	GPIO_PORT257, GPIO_PORT258, GPIO_PORT259,
};

void (*shmobile_arch_reset)(char mode, const char *cmd);

static int proc_read_board_rev(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	count = snprintf(page, count, "%x", system_rev);
	return count;
}

static struct ktd253ehd_led_platform_data ktd253ehd_led_info = {
	.gpio_port = GPIO_PORT47,
};

static struct platform_device led_backlight_device = {
	.name = "ktd253ehd_led",
	.dev  = {
		.platform_data  = &ktd253ehd_led_info,
	},
};

#if (defined(CONFIG_BCM_RFKILL) || defined(CONFIG_BCM_RFKILL_MODULE))
#define BCMBT_VREG_GPIO       (GPIO_PORT268)
#define BCMBT_N_RESET_GPIO    (GPIO_PORT15) //(-1)
/* clk32 */
#define BCMBT_AUX0_GPIO       (-1)
/* UARTB_SEL */
#define BCMBT_AUX1_GPIO       (-1)

static struct bcmbt_rfkill_platform_data board_bcmbt_rfkill_cfg = {
	.vreg_gpio    = BCMBT_VREG_GPIO,
	.n_reset_gpio = BCMBT_N_RESET_GPIO,
	/* CLK32 */
	.aux0_gpio    = BCMBT_AUX0_GPIO,
	/* UARTB_SEL, probably not required */
	.aux1_gpio    = BCMBT_AUX1_GPIO,
};

static struct platform_device board_bcmbt_rfkill_device = {
	.name = "bcmbt-rfkill",
	.id   = -1,
	.dev  = {
		.platform_data=&board_bcmbt_rfkill_cfg,
	},
};
#endif

#ifdef CONFIG_BCM_BZHW

#define GPIO_BT_WAKE 	GPIO_PORT262
#define GPIO_HOST_WAKE 	GPIO_PORT272

static struct bcm_bzhw_platform_data bcm_bzhw_data = {
	.gpio_bt_wake   = GPIO_BT_WAKE,
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
	.name = "bcmbt-lpm",
	.id   = -1,
	.dev  = {
		.platform_data=&brcm_bt_lpm_data,
	},
};
#endif

struct fm34_platform_data fm34_data = {
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
	.name = "panel",
	.id   = -1,
	.dev  = {
		.platform_data = &bcm_ktd259b_backlight_data,
	},
};

static struct i2c_board_info __initdata i2c3_devices[] = {
#if defined(CONFIG_USE_MUIC)
	{
		I2C_BOARD_INFO(MUIC_NAME, MUIC_I2C_ADDRESS),
			.platform_data = NULL,
			.irq           = R8A7373_IRQC_IRQ(GPIO_MUS_INT),
	},
#endif
#if defined(CONFIG_RT8973)
	{
		I2C_BOARD_INFO("rt8973", 0x28>>1),
		.platform_data = NULL,
		.irq = irqpin2irq(GPIO_MUS_INT),
	},
#endif
#if defined(CONFIG_CHARGER_SMB328A)
	{
		I2C_BOARD_INFO("smb328a", SMB327B_ADDRESS),
		.irq            = irqpin2irq(GPIO_CHG_INT),
	},
#endif

};

/* Rhea Ray specific platform devices */
static struct platform_device *plat_devices[] __initdata = {
	&bcm_backlight_devices,
};

#if 0
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
#endif


static struct i2c_board_info i2cm_devices_d2153[] = {
	{
		I2C_BOARD_INFO(FM34_MODULE_NAME, 0x60),
		.platform_data = &fm34_data,
	},
};

#if defined(CONFIG_SOC_CAMERA_S5K4ECGX) && \
	defined(CONFIG_SOC_CAMERA_SR030PC50) /* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
	{
		I2C_BOARD_INFO("S5K4ECGX", 0x56),
	},
	{
		I2C_BOARD_INFO("SR030PC50", 0x30), /* TODO::HYCHO (0x61>>1) */
	},
};

struct soc_camera_link camera_links[] = {
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K4ECGX",
		.power			= S5K4ECGX_power,
	},
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "SR030PC50",
		.power			= SR030PC50_power,
	},
};
#else	/* Select by board Rev */
struct i2c_board_info i2c_cameras[] = {
	/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
	{
		I2C_BOARD_INFO("S5K4ECGX", 0x56),
	},
#endif
	/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
	{
		I2C_BOARD_INFO("SR030PC50", 0x30), /* TODO::HYCHO (0x61>>1) */
	},
#endif
};

struct soc_camera_link camera_links[] = {
	/* Rear Camera */
#if defined(CONFIG_SOC_CAMERA_S5K4ECGX)
	{
		.bus_id			= 0,
		.board_info		= &i2c_cameras[0],
		.i2c_adapter_id	= 1,
		.module_name	= "S5K4ECGX",
		.power			= S5K4ECGX_power,
	},
#endif
	/* Front Camera */
#if defined(CONFIG_SOC_CAMERA_SR030PC50)
	{
		.bus_id			= 1,
		.board_info		= &i2c_cameras[1],
		.i2c_adapter_id	= 1,
		.module_name	= "SR030PC50",
		.power			= SR030PC50_power,
	},
#endif
};
#endif	/* Select by board Rev */

struct platform_device camera_devices[] = {
		{
		.name   = "soc-camera-pdrv",
		.id             = 0,
		.dev    = {
		.platform_data = &camera_links[0],
		},
	},
	{
		.name   = "soc-camera-pdrv",
		.id     =       1,
		.dev    = {
		.platform_data = &camera_links[1],
		},
	},
};


static struct platform_device *gpio_i2c_devices[] __initdata = {
};

void board_restart(char mode, const char *cmd)
{
	printk(KERN_INFO "%s\n", __func__);
	shmobile_do_restart(mode, cmd, APE_RESETLOG_U2EVM_RESTART);
}
static void __init board_init(void)
{
	int stm_select = -1;    // Shall tell how to route STM traces. See setup-u2stm.c for details.
	void __iomem *sbsc_sdmra_28200 = NULL;
	void __iomem *sbsc_sdmra_38200 = NULL;
	int inx = 0;

	/* ES2.02 / LPDDR2 ZQ Calibration Issue WA */
	u8 reg8 = __raw_readb(STBCHRB3);

	if ((reg8 & 0x80) && !shmobile_is_older(U2_VERSION_2_2)) {
		printk(KERN_ALERT "< %s >Apply for ZQ calibration\n", __func__);
		printk(KERN_ALERT "< %s > Before CPG_PLL3CR 0x%8x\n",
				__func__, __raw_readl(PLL3CR));
		sbsc_sdmracr1a   = ioremap(SBSC_BASE + 0x000088, 0x4);
		sbsc_sdmra_28200 = ioremap(SBSC_BASE + 0x128200, 0x4);
		sbsc_sdmra_38200 = ioremap(SBSC_BASE + 0x138200, 0x4);
		if (sbsc_sdmracr1a && sbsc_sdmra_28200 && sbsc_sdmra_38200) {
			SBSC_Init_520Mhz();
			__raw_writel(SBSC_SDMRACR1A_ZQ, sbsc_sdmracr1a);
			__raw_writel(SBSC_SDMRA_DONE, sbsc_sdmra_28200);
			__raw_writel(SBSC_SDMRA_DONE, sbsc_sdmra_38200);
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
#ifdef CONFIG_ARCH_SHMOBILE
	r8a7373_avoid_a2slpowerdown_afterL2sync();
#endif
	r8a7373_pinmux_init();

	create_proc_read_entry("board_revision", 0444, NULL,
				proc_read_board_rev, NULL);

	r8a7373_add_standard_devices();

	r8a7373_hwlock_gpio = hwspin_lock_request_specific(SMGPIO);
	r8a7373_hwlock_cpg = hwspin_lock_request_specific(SMCPG);
	r8a7373_hwlock_sysc = hwspin_lock_request_specific(SMSYSC);
	pinmux_hwspinlock_init(r8a7373_hwlock_gpio);

	if (!shmobile_is_older(U2_VERSION_2_0)) {
		__raw_writew(0x0022, GPIO_DRVCR_SD0);
		__raw_writew(0x0022, GPIO_DRVCR_SIM1);
		__raw_writew(0x0022, GPIO_DRVCR_SIM2);
	}
	shmobile_arch_reset = board_restart;

	printk(KERN_INFO "%s hw rev : %d\n", __func__, system_rev);

	/* Init unused GPIOs */
	for (inx = 0; inx < ARRAY_SIZE(unused_gpios_amethyst); inx++)
		unused_gpio_port_init(unused_gpios_amethyst[inx]);

	/* SCIFA0 */
	gpio_request(GPIO_FN_SCIFA0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFA0_RXD, NULL);

	/* Bluetooth UART settings (ttySC4) */

	/* SCIFB0 */
	gpio_request(GPIO_FN_SCIFB0_TXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_RXD, NULL);
	gpio_request(GPIO_FN_SCIFB0_CTS_, NULL);
	gpio_request(GPIO_FN_SCIFB0_RTS_, NULL);

	/* GPS UART settings (ttySC5) */

	/* SCIFB1 */
	gpio_request(GPIO_FN_SCIFB1_TXD, NULL);
	gpio_request(GPIO_FN_SCIFB1_RXD, NULL);
	gpio_request(GPIO_FN_SCIFB1_CTS, NULL);
	gpio_request(GPIO_FN_SCIFB1_RTS, NULL);

#ifdef CONFIG_KEYBOARD_SH_KEYSC
	/* enable KEYSC */
	gpio_request(GPIO_FN_KEYIN0, NULL);
	gpio_request(GPIO_FN_KEYIN1, NULL);
	gpio_request(GPIO_FN_KEYIN2, NULL);
	gpio_request(GPIO_FN_KEYIN3, NULL);
	gpio_request(GPIO_FN_KEYIN5, NULL);
	gpio_request(GPIO_FN_KEYIN6, NULL);

	gpio_pull_up_port(GPIO_PORT44);
	gpio_pull_up_port(GPIO_PORT45);
	gpio_pull_up_port(GPIO_PORT46);
	gpio_pull_up_port(GPIO_PORT47);
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
		gpio_free(GPIO_PORT327);
		gpio_request(GPIO_FN_SDHICD0, NULL);
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
	}

	/* I2C */
	gpio_request(GPIO_FN_I2C_SCL0H, NULL);
	gpio_request(GPIO_FN_I2C_SDA0H, NULL);

	gpio_pull_off_port(GPIO_PORT84);
	gpio_pull_off_port(GPIO_PORT85);

	gpio_request(GPIO_PORT28, NULL);
	gpio_direction_input(GPIO_PORT28);
	gpio_pull_up_port(GPIO_PORT28);

#if defined(CONFIG_MFD_D2153)
	irq_set_irq_type(irqpin2irq(28), IRQ_TYPE_LEVEL_LOW);
#endif /* CONFIG_MFD_D2153 */

	/* Touch */
	gpio_request(GPIO_PORT32, NULL);
	gpio_direction_input(GPIO_PORT32);
	gpio_pull_up_port(GPIO_PORT32);

	USBGpio_init();

#if defined(CONFIG_SND_SOC_SH4_FSI)
	d2153_pdata.audio.fm34_device = DEVICE_NONE;
	d2153_pdata.audio.aad_codec_detect_enable = false;
	d2153_pdata.audio.debounce_ms = D2153_AAD_JACK_DEBOUNCE_MS;
	u2audio_init();
#endif /* CONFIG_SND_SOC_SH4_FSI */

#ifndef CONFIG_ARM_TZ
	r8a7373_l2cache_init();
#else
	/**
	 * In TZ-Mode of R-Mobile U2, it must notify the L2 cache
	 * related info to Secure World. However, SEC_HAL driver is not
	 * registered at the time of L2$ init because "r8a7373l2_cache_init()"
	 * function called more early.
	 */
	l2x0_init_later();
#endif

	camera_init();

	/* Camera ES version convert */
	camera_links[0].priv = &csi20_info;
	camera_links[1].priv = &csi21_info;
	printk(KERN_ALERT "Camera ISP ES version switch (ES2)\n");
	csi20_info.clients[0].lanes = 0x3;
	gpio_key_init(stm_select,
			devices_stm_sdhi0,
			ARRAY_SIZE(devices_stm_sdhi0),
			devices_stm_sdhi1,
			ARRAY_SIZE(devices_stm_sdhi1),
			devices_stm_none,
			ARRAY_SIZE(devices_stm_none));

	platform_device_register(&led_backlight_device);

#if defined(CONFIG_SAMSUNG_MHL)
	board_mhl_init();
	board_edid_init();
#endif

	i2c_register_board_info(0, i2c0_devices_d2153,
					ARRAY_SIZE(i2c0_devices_d2153));

#if defined (CONFIG_AMETHYST_SENSOR)
	board_sensor_init();
#endif

#if defined(CONFIG_CHARGER_SMB328A)
	/* rev0.0 uses SMB328A, rev0.1 uses SMB327B */
	if (system_rev == BOARD_REV_0_0 || system_rev > BOARD_REV_0_4) {
		int i;
		for (i = 0; i < sizeof(i2c3_devices)/sizeof(struct i2c_board_info); i++) {
			if (strcmp(i2c3_devices[i].type, "smb328a")==0) {
				i2c3_devices[i].addr = SMB328A_ADDRESS;
			}
		}
	}
#endif

	i2c_register_board_info(3, i2c3_devices, ARRAY_SIZE(i2c3_devices));


	/* Touch Panel auto detection */
	i2c_add_driver(&tsp_detector_driver);
	i2c_register_board_info(4, i2c4_devices_tsp_detector,
					ARRAY_SIZE(i2c4_devices_tsp_detector));

	i2c_register_board_info(8, i2cm_devices_d2153,
					ARRAY_SIZE(i2cm_devices_d2153));

#if defined(CONFIG_GPS_BCM4752)
	/* GPS Init */
	gps_gpio_init();
#endif

	platform_add_devices(gpio_i2c_devices, ARRAY_SIZE(gpio_i2c_devices));
	platform_add_devices(plat_devices,
					ARRAY_SIZE(plat_devices));

	/* PA devices init */
	spa_init();
	ss_vibrator_data.voltage = 2800000;
	u2_vibrator_init();

	printk(KERN_DEBUG "%s\n", __func__);
	crashlog_reset_log_write();

#if defined(CONFIG_PN547_NFC) || defined(CONFIG_NFC_PN547)
	pn547_device_i2c_register();
#endif
}

MACHINE_START(AMETHYST, "amethyst")
	.map_io         = r8a7373_map_io,
	.init_irq       = r8a7373_init_irq,
	.init_early     = r8a7373_init_early,
	.nr_irqs        = NR_IRQS_LEGACY,
	.handle_irq     = gic_handle_irq,
	.init_machine   = board_init,
	.timer          = &shmobile_timer,
	.restart        = board_restart,
	.reserve        = r8a7373_reserve,
MACHINE_END
