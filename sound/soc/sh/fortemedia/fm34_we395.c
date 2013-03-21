/* drivers/misc/fm34_we395.c - fm34_we395 voice processor driver
 * sound/soc/sh/fortmedia/fm34_we395.c - fm34_we395 voice processor driver
 *
 * Copyright (C) 2012 Samsung Corporation.
 * Copyright (C) 2013 Renesas Mobile Corp.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/module.h>

#include "fm34_we395.h"
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <mach/common.h>
#include "sound/soundpath/soundpath.h"
#include <linux/platform_device.h>

/* Global parameters */
static struct i2c_client *g_fm34_client;
static struct fm34_platform_data *g_fm34_pdata;
struct mutex g_fm34_lock;
static struct clk *g_fm34_vclk4_clk;
int g_fm34_event;
int g_fm34_curt_status;
static struct proc_dir_entry *g_fm34_parent;
unsigned int g_fm34_log_level = FM34_LOG_ALL;
static bool g_fm34_is_vclk4_enable;
static unsigned int g_fm34_vclk_adr;
static struct sndp_extdev_callback_func g_fm34_callback_func;

/* Function prototype */
static int fm34_suspend(struct device *dev);
static int fm34_resume(struct device *dev);
static int fm34_runtime_suspend(struct device *dev);
static int fm34_runtime_resume(struct device *dev);

static int fm34_nop(void);
static int fm34_error(void);
static int fm34_set_HS_mode(void);
static int fm34_set_idle_mode(void);
static int fm34_set_bypass_mode(void);
static int fm34_set_hw_bypass_mode(void);

static int fm34_enable_vclk4(void);
static void fm34_disable_vclk4(void);

static int fm34_create_event
	(unsigned int mode, unsigned int device, unsigned int ch_dev);
static int fm34_set_mode
	(unsigned int mode, unsigned int device, unsigned int ch_dev);

/*
 * callback object
 */
static const struct dev_pm_ops fm34_dev_pm_ops = {
	.suspend		= fm34_suspend,
	.resume			= fm34_resume,
	.runtime_suspend	= fm34_runtime_suspend,
	.runtime_resume		= fm34_runtime_resume,
};

/*
 * device object
 */
static struct platform_device fm34_platform_device = {
	.name = FM34_MODULE_NAME,
};

/*
 * driver object
 */
static struct platform_driver fm34_platform_driver = {
	.driver		= {
		.name	= FM34_MODULE_NAME,
		.pm	= &fm34_dev_pm_ops,
		.probe	= NULL,
		.remove	= NULL,
	},
};

/*
 * event type
 */
enum fm34_event_type {
	FM34_EVENT_IDLE = 0,
	FM34_EVENT_BYPASS,
	FM34_EVENT_VOICE,
	FM34_EVENT_SUSPEND,
	FM34_EVENT_RESUME,
	FM34_EVENT_MAX
};

/*
 * status type
 */
enum fm34_status_type {
	FM34_STATUS_IDLE = 0,
	FM34_STATUS_BYPASS,
	FM34_STATUS_VOICE,
	FM34_STATUS_SUSPEND,
	FM34_STATUS_MAX,
	FM34_STATUS_NOP
};



typedef int (*fm34_state_func)(void);

struct fm34_table {
	fm34_state_func exec_func;
	int next_state;
};

/* State table */
struct fm34_table fm34_nop_table = {
	fm34_nop, FM34_STATUS_NOP
};
struct fm34_table fm34_idle_table = {
	fm34_set_idle_mode, FM34_STATUS_IDLE
};
struct fm34_table fm34_idle_to_bypass_table = {
	fm34_nop, FM34_STATUS_BYPASS
};
struct fm34_table fm34_bypass_table = {
	fm34_set_hw_bypass_mode, FM34_STATUS_BYPASS
};
struct fm34_table fm34_bypass_to_idle_table = {
	fm34_nop, FM34_STATUS_IDLE
};
struct fm34_table fm34_voice_table = {
	fm34_set_HS_mode, FM34_STATUS_VOICE
};
struct fm34_table fm34_error_table = {
	fm34_error, FM34_STATUS_NOP
};
struct fm34_table fm34_suspend_table = {
	fm34_nop, FM34_STATUS_SUSPEND
};

struct fm34_table *fm34_state_table[FM34_STATUS_MAX][FM34_EVENT_MAX] = {
	/* FM34_STATUS_IDLE */
	{
		&fm34_nop_table,		/* FM34_EVENT_IDLE */
		&fm34_idle_to_bypass_table,	/* FM34_EVENT_BYPASS */
		&fm34_nop_table,		/* FM34_EVENT_VOICE */
		&fm34_suspend_table,		/* FM34_EVENT_SUSPEND */
		&fm34_error_table,		/* FM34_EVENT_RESUME */
	},
	/* FM34_STATUS_BYPASS */
	{
		&fm34_bypass_to_idle_table,	/* FM34_EVENT_IDLE */
		&fm34_nop_table,		/* FM34_EVENT_BYPASS */
		&fm34_nop_table,		/* FM34_EVENT_VOICE */
		&fm34_error_table,		/* FM34_EVENT_SUSPEND */
		&fm34_error_table,		/* FM34_EVENT_RESUME */
	},
	/* FM34_STATUS_VOICE */
	{
		&fm34_idle_table,	/* FM34_EVENT_IDLE */
		&fm34_bypass_table,	/* FM34_EVENT_BYPASS */
		&fm34_nop_table,	/* FM34_EVENT_VOICE */
		&fm34_error_table,	/* FM34_EVENT_SUSPEND */
		&fm34_error_table,	/* FM34_EVENT_RESUME */
	},
	/* FM34_STATUS_SUSPEND */
	{
		&fm34_error_table,	/* FM34_EVENT_IDLE */
		&fm34_error_table,	/* FM34_EVENT_BYPASS */
		&fm34_error_table,	/* FM34_EVENT_VOICE */
		&fm34_nop_table,	/* FM34_EVENT_SUSPEND */
		&fm34_idle_table,	/* FM34_EVENT_RESUME */
	},
};

#if 1/*defined(CONFIG_MACH_C1_KOR_LGT) || defined(CONFIG_MACH_BAFFIN_KOR_LGT)*/
unsigned char bypass_cmd[] = {
/*0xC0,*/
	0xFC, 0xF3, 0x3B, 0x22, 0xC0, 0x00, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xC1, 0x00, 0x01,
	0xFC, 0xF3, 0x3B, 0x22, 0xC2, 0x00, 0x02,
	0xFC, 0xF3, 0x3B, 0x22, 0xC3, 0x00, 0x02,
	0xFC, 0xF3, 0x3B, 0x22, 0xC6, 0x00, 0x7D,
	0xFC, 0xF3, 0x3B, 0x22, 0xC7, 0x00, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xC8, 0x00, 0x18,
	0xFC, 0xF3, 0x3B, 0x22, 0xD2, 0x82, 0x94,
	0xFC, 0xF3, 0x3B, 0x22, 0xEE, 0x00, 0x01,
	0xFC, 0xF3, 0x3B, 0x22, 0xF5, 0x00, 0x03,
	0xFC, 0xF3, 0x3B, 0x22, 0xF6, 0x00, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xF8, 0x80, 0x01,
	0xFC, 0xF3, 0x3B, 0x22, 0xF9, 0x08, 0x7F,
	0xFC, 0xF3, 0x3B, 0x22, 0xFA, 0x24, 0x8B,
	0xFC, 0xF3, 0x3B, 0x23, 0x07, 0x00, 0x00,
	0xFC, 0xF3, 0x3B, 0x23, 0x0A, 0x1A, 0x00,
	0xFC, 0xF3, 0x3B, 0x23, 0x0C, 0x00, 0xB8,
	0xFC, 0xF3, 0x3B, 0x23, 0x0D, 0x02, 0x00,
	0xFC, 0xF3, 0x3B, 0x23, 0x65, 0x08, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xFB, 0x00, 0x00
};
#else
unsigned char bypass_cmd[] = {
/*0xC0,*/
	0xFC, 0xF3, 0x3B, 0x22, 0xF5, 0x00, 0x03,
	0xFC, 0xF3, 0x3B, 0x22, 0xF8, 0x80, 0x03,
	0xFC, 0xF3, 0x3B, 0x22, 0xC6, 0x00, 0x7D,
	0xFC, 0xF3, 0x3B, 0x22, 0xC7, 0x00, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xC8, 0x00, 0x18,
	0xFC, 0xF3, 0x3B, 0x23, 0x0A, 0x1A, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xFA, 0x24, 0x8B,
	0xFC, 0xF3, 0x3B, 0x22, 0xF9, 0x00, 0x7F,
	0xFC, 0xF3, 0x3B, 0x22, 0xF6, 0x00, 0x00,
	0xFC, 0xF3, 0x3B, 0x22, 0xD2, 0x82, 0x94,
	0xFC, 0xF3, 0x3B, 0x22, 0xEE, 0x00, 0x01,
	0xFC, 0xF3, 0x3B, 0x22, 0xFB, 0x00, 0x00,
};
#endif

static void fm34_i2c_debug(unsigned char *i2c_cmds, int size)
{
	int i = 0;

	/* Output i2c parameters. */
	for (i = 0; i < size; i += 1) {
		fm34_pr_info("i2c_cmds[%d/%d] = 0x%x\n",
				i, size, i2c_cmds[i]);
	}
}

#if 0
static int fm34_i2c_read(char *rxData, int length)
{
	int rc;

	struct i2c_msg msgs[] = {
		{
			.addr = g_fm34_client->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxData,
		},
	};

	fm34_pr_func_start();

	if (FM34_LOG_DEBUG & g_fm34_log_level)
		fm34_i2c_debug(rxData, length);

	rc = i2c_transfer(g_fm34_client->adapter, msgs, 1);
	if (rc < 0) {
		fm34_pr_err("transfer error[%d]\n", rc);
		return rc;
	}

	fm34_pr_func_end();
	return 0;
}
#endif

static int fm34_i2c_write(char *txData, int length)
{
	int rc;

	struct i2c_msg msg[] = {
		{
			.addr = g_fm34_client->addr,
			.flags = 0,
			.len = length,
			.buf = txData,
		},
	};

	fm34_pr_func_start();

	if (FM34_LOG_DEBUG & g_fm34_log_level)
		fm34_i2c_debug(txData, length);

	rc = i2c_transfer(g_fm34_client->adapter, msg, 1);
	if (rc < 0) {
		fm34_pr_err("transfer error[%d]\n", rc);
		return rc;
	}

	fm34_pr_func_end();
	return 0;
}

static void fm34_parameter_reset(void)
{
	fm34_pr_func_start();

	if (g_fm34_pdata->gpio_rst) {
		gpio_set_value(g_fm34_pdata->gpio_rst, FM34_RST_ENABLE);
		fm34_pr_debug("gpio_rst[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_rst));
		usleep_range(FM34_TRST_US, FM34_TRST_US);
		gpio_set_value(g_fm34_pdata->gpio_rst, FM34_RST_DISABLE);
		fm34_pr_debug("gpio_rst[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_rst));
		usleep_range(FM34_TSU_RST2PP, FM34_TSU_RST2PP);
	}
	fm34_pr_func_end();
}

static int fm34_wakeup(void)
{
	int ret = 0;
	int val = gpio_get_value(g_fm34_pdata->gpio_pwdn);

	fm34_pr_func_start();

	/* vclk4 enable */
	ret = fm34_enable_vclk4();
	if (ret != 0)
		fm34_pr_err("failed return [%d]\n", ret);

	/* Powerdown-mode release */
	if (val == FM34_PWDN_ENABLE) {
		usleep_range(FM34_TSU_PP2PD, FM34_TSU_PP2PD);
		gpio_set_value(g_fm34_pdata->gpio_pwdn, FM34_PWDN_DISABLE);
		fm34_pr_debug("gpio_pwdn[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_pwdn));
	}

	fm34_pr_func_end();
	return ret;
}

#if 1/*defined(CONFIG_MACH_C1_KOR_LGT) || defined(CONFIG_MACH_BAFFIN_KOR_LGT)*/
static int fm34_set_bypass_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	fm34_parameter_reset();

	i2c_cmds = bypass_cmd;
	size = sizeof(bypass_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}
#else
static int fm34_set_bypass_mode(void)
{
	int i = 0, rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	i2c_cmds = bypass_cmd;
	size = sizeof(bypass_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
	} else if (pdata->gpio_pwdn) {
		msleep(20);
		gpio_set_value(pdata->gpio_pwdn, FM34_PWDN_ENABLE);
		fm34_pr_debug("gpio_pwdn[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_pwdn));
	}

	fm34_pr_func_end();
	return rc;
}
#endif

#if 0
static int fm34_set_loopback_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = loopback_cmd;
	size = sizeof(loopback_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_SPK_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HF_cmd;
	size = sizeof(HF_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_HS_NS_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HS_NS_cmd;
	size = sizeof(HS_NS_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);


	fm34_pr_func_end();
	return rc;
}

static int fm34_set_HS_ExtraVol_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HS_ExtraVol_cmd;
	size = sizeof(HS_ExtraVol_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_SPK_ExtraVol_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HF_ExtraVol_cmd;
	size = sizeof(HF_ExtraVol_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_ExtraVol_NS_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HS_ExtraVol_NS_cmd;
	size = sizeof(HS_ExtraVol_NS_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_EP_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = EP_cmd;
	size = sizeof(EP_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_BTSCO_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = BT_SCO_cmd;
	size = sizeof(BT_SCO_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_factory_rcv_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HS_FACTORY_RCV_cmd;
	size = sizeof(HS_FACTORY_RCV_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_factory_spk_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HS_FACTORY_SPK_cmd;
	size = sizeof(HS_FACTORY_SPK_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}
#endif

static int fm34_set_hw_bypass_mode(void)
{
	int val = gpio_get_value(g_fm34_pdata->gpio_pwdn);

	fm34_pr_func_start();

	/* vclk4 disable */
	fm34_disable_vclk4();

	if (val == FM34_PWDN_DISABLE) {
		usleep_range(FM34_TSU_PP2PD, FM34_TSU_PP2PD);
		gpio_set_value(g_fm34_pdata->gpio_pwdn, FM34_PWDN_ENABLE);
		fm34_pr_debug("gpio_pwdn[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_pwdn));
	}

	fm34_pr_func_end();
	return 0;
}

static int fm34_set_HS_mode(void)
{
	int rc = 0, size = 0;
	unsigned char *i2c_cmds;

	fm34_pr_func_start();

	rc = fm34_wakeup();
	if (rc < 0) {
		fm34_pr_err("failed return [%d]\n", rc);
		return rc;
	}

	fm34_parameter_reset();

	i2c_cmds = HS_cmd;
	size = sizeof(HS_cmd);

	rc = fm34_i2c_write(i2c_cmds, size);

	if (rc < 0)
		fm34_pr_err("failed return [%d]\n", rc);

	fm34_pr_func_end();
	return rc;
}

static int fm34_set_idle_mode(void)
{
	int rc = 0;
	fm34_pr_func_start();

	/* FM34 change state to powerdown-mode. */
	rc = fm34_set_hw_bypass_mode();

	fm34_pr_func_end();
	return rc;
}

static int fm34_nop(void)
{
	fm34_pr_func_start();

	/* Kept last status. */

	fm34_pr_func_end();
	return 0;
}

static int fm34_error(void)
{
	fm34_pr_func_start();

	/* Status change error. */
	/* It should be called in case of state change error. */

	fm34_pr_debug("g_fm34_curt_status[%d] g_fm34_event[%d]\n",
			g_fm34_curt_status, g_fm34_event);
	fm34_pr_func_end();
	return -EINVAL;
}

static int fm34_suspend(struct device *dev)
{
	int ret = 0;
#if FM34_BYPASS_ONLY
#else
	struct fm34_table *table;
#endif

	fm34_pr_func_start();

	mutex_lock(&g_fm34_lock);

#if FM34_BYPASS_ONLY
#else
	/* Get state table. */
	table = fm34_state_table[g_fm34_curt_status][FM34_EVENT_SUSPEND];

	/* Call function of state table. */
	ret = table->exec_func();
	if ((0 == ret) && (FM34_STATUS_NOP != table->next_state))
		g_fm34_curt_status = table->next_state;

#endif

	mutex_unlock(&g_fm34_lock);

	fm34_pr_func_end();
	return ret;
}

static int fm34_resume(struct device *dev)
{
	int ret = 0;
#if FM34_BYPASS_ONLY
#else
	struct fm34_table *table;
#endif

	fm34_pr_func_start();

	mutex_lock(&g_fm34_lock);

#if FM34_BYPASS_ONLY
#else
	/* Get state table. */
	table = fm34_state_table[g_fm34_curt_status][FM34_EVENT_RESUME];

	/* Call function of state table. */
	ret = table->exec_func();
	if ((0 == ret) && (FM34_STATUS_NOP != table->next_state))
		g_fm34_curt_status = table->next_state;
#endif

	mutex_unlock(&g_fm34_lock);

	fm34_pr_func_end();
	return ret;
}

static int fm34_runtime_suspend(struct device *dev)
{
	int ret = 0;

	fm34_pr_func_start();

	mutex_lock(&g_fm34_lock);

#if FM34_BYPASS_ONLY
#else
	/* Status except IDLE doesn't change to Suspend. */
	if (FM34_STATUS_IDLE != g_fm34_curt_status)
		ret = -1;
#endif

	mutex_unlock(&g_fm34_lock);
	fm34_pr_func_end();
	return ret;
}

static int fm34_runtime_resume(struct device *dev)
{
	fm34_pr_func_start();
	/* nop */
	fm34_pr_func_end();
	return 0;
}

static bool fm34_is_bt_mhl(unsigned int device)
{
	bool bt_mhl = false;

	fm34_pr_func_start();

	/* Check device from sound driver. */
	if ((device & SNDP_OUT_BLUETOOTH_SCO) ||
		(device & SNDP_OUT_BLUETOOTH_SCO_HEADSET) ||
		(device & SNDP_OUT_BLUETOOTH_SCO_CARKIT) ||
		(device & SNDP_OUT_BLUETOOTH_A2DP) ||
		(device & SNDP_OUT_BLUETOOTH_A2DP_HEADPHONES) ||
		(device & SNDP_OUT_BLUETOOTH_A2DP_SPEAKER) ||
		(device & SNDP_OUT_AUX_DIGITAL) ||
		(device & SNDP_IN_BLUETOOTH_SCO_HEADSET) ||
		(device & SNDP_IN_AUX_DIGITAL)) {
		bt_mhl = true;
	}

	fm34_pr_func_end();
	return bt_mhl;
}

static int fm34_create_event
	(unsigned int mode, unsigned int device, unsigned int ch_dev)
{
	int event;

	if (fm34_is_bt_mhl(device)) {
		/* event update */
		event = FM34_EVENT_IDLE;
	} else {
		if ((SNDP_EXTDEV_STOP == ch_dev)) {
			/* event update */
			event = FM34_EVENT_IDLE;
		} else {
			/* earpiece incall */
			if ((SNDP_MODE_INCALL == mode) &&
			    (SNDP_OUT_EARPIECE == device)) {
				/* event update */
				event = FM34_EVENT_VOICE;
			} else {
				/* event update */
				event = FM34_EVENT_BYPASS;
			}
		}
	}
	fm34_pr_debug("event[%d]\n", event);
	return event;
}

static int fm34_set_mode
	(unsigned int mode, unsigned int device, unsigned int ch_dev)
{
	int ret = 0;
#if FM34_BYPASS_ONLY
#else
	struct fm34_table *table;
#endif
	fm34_pr_if_start("mode[0x%x] device[0x%x] ch_dev[0x%x]\n",
			mode, device, ch_dev);

	mutex_lock(&g_fm34_lock);

#if FM34_BYPASS_ONLY
	/* Change path (bypass-mode). */
	ret = fm34_set_hw_bypass_mode();
	g_fm34_curt_status = FM34_STATUS_BYPASS;
#else

	/* Create next event. */
	g_fm34_event = fm34_create_event(mode, device, ch_dev);

	fm34_pr_debug("status[%d] event[%d]\n",
		g_fm34_curt_status, g_fm34_event);

	/* Get state table. */
	table = fm34_state_table[g_fm34_curt_status][g_fm34_event];

	/* Call function of state table. */
	ret = table->exec_func();
	if ((0 == ret) && (FM34_STATUS_NOP != table->next_state)) {
		fm34_pr_debug("status change -- [%d] -> [%d]\n",
				g_fm34_curt_status, table->next_state);
		/* Update state. */
		g_fm34_curt_status = table->next_state;
	} else {
		/* mode change error */
		fm34_pr_err("fm34 mode change error[%d]\n", ret);
	}
#endif
	mutex_unlock(&g_fm34_lock);

	fm34_pr_if_end("ret[%d]\n", ret);
	return ret;
}

static int fm34_gpio_init(void)
{
	int ret = 0;

	fm34_pr_func_start();

	if ((g_fm34_pdata->gpio_rst) &&
		(g_fm34_pdata->gpio_pwdn) &&
		(g_fm34_pdata->gpio_bp)) {

		gpio_request(g_fm34_pdata->gpio_rst, "FM34_RESET");
		gpio_direction_output(g_fm34_pdata->gpio_rst,
			FM34_RST_DISABLE);
		gpio_set_value(g_fm34_pdata->gpio_rst,
			FM34_RST_ENABLE);
		fm34_pr_debug("gpio_rst[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_rst));
		usleep_range(10000, 10000);

		gpio_request(g_fm34_pdata->gpio_pwdn, "FM34_PWDN");
		gpio_direction_output(g_fm34_pdata->gpio_pwdn,
			FM34_PWDN_DISABLE);
		gpio_set_value(g_fm34_pdata->gpio_pwdn,
			FM34_PWDN_DISABLE);
		fm34_pr_debug("gpio_pwdn[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_pwdn));

		gpio_request(g_fm34_pdata->gpio_bp, "FM34_BYPASS");
		gpio_direction_output(g_fm34_pdata->gpio_bp,
			FM34_BP_DISABLE);
		gpio_set_value(g_fm34_pdata->gpio_bp,
			FM34_BP_DISABLE);
		fm34_pr_debug("gpio_bp[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_bp));

		gpio_request(g_fm34_pdata->gpio_avdd, "FM34_AVDD");
		gpio_direction_output(g_fm34_pdata->gpio_avdd,
			FM34_AVDD_DISABLE);
		gpio_set_value(g_fm34_pdata->gpio_avdd,
			FM34_AVDD_DISABLE);
		fm34_pr_debug("gpio_avdd[0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_avdd));
	} else {
		ret = -EINVAL;
		fm34_pr_err("gpio not active.[%d]\n", ret);
	}

	fm34_pr_func_end();
	return ret;
}

static void fm34_gpio_free(void)
{
	fm34_pr_func_start();

	if ((g_fm34_pdata->gpio_rst) &&
		(g_fm34_pdata->gpio_pwdn) &&
		(g_fm34_pdata->gpio_bp)) {

		gpio_free(g_fm34_pdata->gpio_rst);
		gpio_free(g_fm34_pdata->gpio_pwdn);
		gpio_free(g_fm34_pdata->gpio_bp);
	}

	fm34_pr_func_end();
}

static int fm34_read_proc(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	int len = 0;

	len = snprintf(page, count, "0x%x\n", g_fm34_log_level);

	return len;
}

static int fm34_write_proc(struct file *filp, const char *buffer,
					unsigned long len, void *data)
{
	int rc = 0;
	unsigned char buf[11] = {0};
	unsigned int code = 0;

	if (11 <= len) {
		/* size over */
		return len;
	}

	if (copy_from_user(buf, (void __user *)buffer, len)) {
		/* failed copy_from_user */
		return len;
	}

	rc = kstrtouint(buf, 0, &code);
	if (0 != rc) {
		/* kstrtouint failed */
		return len;
	}

	/* Set log level. */
	if (!(FM34_LOG_LEVEL_CHECK & code))
		g_fm34_log_level = code;

	return len;
}

static int fm34_read_exec_proc(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	return count;
}

static int fm34_write_exec_proc(struct file *filp, const char *buffer,
					unsigned long len, void *data)
{
	int rc = 0;
	unsigned char buf[11] = {0};
	unsigned int code = 0;

	if (11 <= len) {
		/* size over */
		return len;
	}

	if (copy_from_user(buf, (void __user *)buffer, len)) {
		/* failed copy_from_user */
		return len;
	}

	rc = kstrtouint(buf, 0, &code);
	if (0 != rc) {
		/* kstrtouint failed */
		return len;
	}

	mutex_lock(&g_fm34_lock);

	switch (code) {
	case 0x0:
		/* FM34 parameter area initialized. */
		rc = fm34_set_bypass_mode();
		if (rc < 0)
			fm34_pr_err("bypass setting failed [%d]\n", rc);

		/* Change path (bypass-mode). */
		rc = fm34_set_idle_mode();
		if (rc < 0)
			fm34_pr_err("idle failed [%d]\n", rc);

		g_fm34_event = 0;
		g_fm34_curt_status = FM34_STATUS_IDLE;
		break;
	case 0x1:
		/* FM34 parameter area initialized. */
		rc = fm34_set_bypass_mode();
		if (rc < 0)
			fm34_pr_err("bypass setting failed [%d]\n", rc);
		break;
	case 0x2:
		/* Change path (bypass-mode). */
		rc = fm34_set_idle_mode();
		if (rc < 0)
			fm34_pr_err("idle failed [%d]\n", rc);

		g_fm34_curt_status = FM34_STATUS_IDLE;
		break;
	case 0x3:
		/* Change path (voice process). */
		rc = fm34_set_HS_mode();
		if (rc < 0)
			fm34_pr_err("EP mode failed [%d]\n", rc);

		g_fm34_curt_status = FM34_STATUS_VOICE;
		break;
	case 0x10:
		fm34_pr_debug("g_fm34_event      [0x%08x]\n",
			g_fm34_event);
		fm34_pr_debug("g_fm34_curt_status[0x%08x]\n",
			g_fm34_curt_status);
		fm34_pr_debug("g_fm34_vclk_adr   [0x%08x]\n",
			ioread32(g_fm34_vclk_adr));
		fm34_pr_debug("g_fm34_log_level  [0x%08x]\n",
			g_fm34_log_level);
		fm34_pr_debug("gpio_rst          [0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_rst));
		fm34_pr_debug("gpio_pwdn         [0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_pwdn));
		fm34_pr_debug("gpio_bp           [0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_bp));
		fm34_pr_debug("gpio_avdd         [0x%08x]\n",
			gpio_get_value(g_fm34_pdata->gpio_avdd));
		break;
	default:
		break;
	}

	mutex_unlock(&g_fm34_lock);

	return len;
}

static int fm34_create_proc_entry(void)
{
	int rc = 0;
	struct proc_dir_entry *log_level = NULL;
	struct proc_dir_entry *exec = NULL;

	/* Create FM34 directory */
	 g_fm34_parent = proc_mkdir(FM34_MODULE_NAME, NULL);
	if (NULL != g_fm34_parent) {
		/* Log level procfile create.  */
		log_level = create_proc_entry("log_level",
				(S_IFREG | S_IRUGO | S_IWUGO), g_fm34_parent);
		if (NULL != log_level) {
			log_level->read_proc  = fm34_read_proc;
			log_level->write_proc = fm34_write_proc;
		} else {
			rc = -EACCES;
			goto rm_dir;
		}

		/* Exec procfile create.  */
		exec = create_proc_entry("exec",
				(S_IFREG | S_IRUGO | S_IWUGO), g_fm34_parent);
		if (NULL != exec) {
			exec->read_proc  = fm34_read_exec_proc;
			exec->write_proc = fm34_write_exec_proc;
		} else {
			rc = -EACCES;
			goto rm_dir;
		}

	} else {
		rc = -EACCES;
		return rc;
	}
	return rc;

rm_dir:
	remove_proc_entry(FM34_MODULE_NAME, NULL);
	g_fm34_parent = NULL;
	return rc;
}

static void fm34_remove_proc_entry(void)
{
	/* Delete FM34 directory */
	if (NULL != g_fm34_parent) {
		remove_proc_entry("exec", g_fm34_parent);
		remove_proc_entry("log_level", g_fm34_parent);
		remove_proc_entry(FM34_MODULE_NAME, NULL);
		g_fm34_parent = NULL;
	}
}

static int fm34_enable_vclk4(void)
{
	int ret = 0;

	fm34_pr_func_start();

	if (g_fm34_is_vclk4_enable == false) {
		ret = clk_enable(g_fm34_vclk4_clk);
		if (0 != ret) {
			fm34_pr_err("clk_enable error[%d]\n", ret);
			return ret;
		}
		g_fm34_is_vclk4_enable = true;
		usleep_range(1000, 1000);
	} else {
		fm34_pr_err("already enable vclk4.\n");
	}

	fm34_pr_debug("g_fm34_vclk_adr[0x%08x]\n", ioread32(g_fm34_vclk_adr));

	fm34_pr_func_end();
	return 0;
}

static void fm34_disable_vclk4(void)
{
	fm34_pr_func_start();

	if (g_fm34_is_vclk4_enable == true) {
		clk_disable(g_fm34_vclk4_clk);
		g_fm34_is_vclk4_enable = false;
	} else {
		fm34_pr_err("already disabled vclk4.\n");
	}

	fm34_pr_debug("g_fm34_vclk_adr[0x%08x]\n", ioread32(g_fm34_vclk_adr));

	fm34_pr_func_end();
}

static int fm34_bootup_init(void)
{
	int ret = 0;

	fm34_pr_func_start();

	/* enable vclk4 */
	ret = fm34_enable_vclk4();
	if (0 == ret) {
		/* fm34 reset */
		if (g_fm34_pdata->gpio_rst) {
			msleep(20);
			gpio_set_value(g_fm34_pdata->gpio_rst,
				FM34_RST_ENABLE);
			fm34_pr_debug("gpio_rst[0x%08x]\n",
				gpio_get_value(g_fm34_pdata->gpio_rst));
			msleep(20);
			gpio_set_value(g_fm34_pdata->gpio_rst,
				FM34_RST_DISABLE);
			fm34_pr_debug("gpio_rst[0x%08x]\n",
				gpio_get_value(g_fm34_pdata->gpio_rst));
			msleep(50);
		}
	}

	fm34_pr_func_end();
	return ret;
}

static int fm34_probe(
		struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret = 0;
	struct clk *local_main_clk = NULL;
	static struct fm34_platform_data *pdata;

	fm34_pr_func_start();

	/* Check platform data. */
	pdata = client->dev.platform_data;
	if (pdata == NULL) {
		ret = -EINVAL;
		fm34_pr_err("platform data is NULL[%d]\n", ret);
		goto err_alloc_data_failed;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		ret = -EINVAL;
		fm34_pr_err("i2c check functionality error[%d]\n", ret);
		goto err_alloc_data_failed;
	}

	/* Keep platform data of global area. */
	g_fm34_pdata = client->dev.platform_data;

	i2c_set_clientdata(client, NULL);
	g_fm34_client = client;

	/* Mutex init. */
	mutex_init(&g_fm34_lock);

	/* GPIO initialized. */
	ret = fm34_gpio_init();
	if (0 != ret) {
		fm34_pr_err("gpio init error[%d]\n", ret);
		goto err_init_gpio_failed;
	}

	g_fm34_vclk_adr = (unsigned int)ioremap(FM34_VCLKCR4_ADDR,
						FM34_VCLKCR4_REGSIZE);

	g_fm34_vclk4_clk = clk_get(NULL, "vclk4_clk");
	if (IS_ERR(g_fm34_vclk4_clk)) {
		ret = IS_ERR(g_fm34_vclk4_clk);
		fm34_pr_err("clk_get(vclk) error[%d]\n", ret);
		goto err_init_gpio_failed;
	}

	local_main_clk = clk_get(NULL, "main_clk");
	if (IS_ERR(local_main_clk)) {
		ret = IS_ERR(local_main_clk);
		fm34_pr_err("clk_get(main) error[%d]\n", ret);
		goto err_clk_get_main;
	}

	ret = clk_set_parent(g_fm34_vclk4_clk, local_main_clk);
	if (0 != ret) {
		fm34_pr_err("clk_set_parent error[%d]\n", ret);
		goto err_clk_set_parent;
	}

	ret = clk_set_rate(g_fm34_vclk4_clk, 13000000);
	if (0 != ret) {
		fm34_pr_err("clk_set_rate error[%d]\n", ret);
		goto err_clk_set_parent;
	}

	clk_put(local_main_clk);

	/* FM34 reset. */
	ret = fm34_bootup_init();
	if (0 != ret) {
		fm34_pr_err("bootup init error[%d]\n", ret);
		goto err_init_bootup_failed;
	}

	/* set callback function */
	g_fm34_callback_func.set_state = fm34_set_mode;

	/* Register sound driver callback */
	sndp_extdev_regist_callback(&g_fm34_callback_func);

	/* Create platform driver. */
	ret = platform_device_register(&fm34_platform_device);
	if (0 != ret) {
		fm34_pr_err("platform_device_register failed.[%d]\n",
			 ret);
		goto err_pf_device_failed;
	}

	ret = platform_driver_register(&fm34_platform_driver);
	if (0 != ret) {
		fm34_pr_err("platform_driver_register failed.[%d]\n",
			ret);
		goto err_pf_driver_failed;
	}

	/* Create proc. */
	ret = fm34_create_proc_entry();
	if (0 != ret) {
		fm34_pr_err("create_proc_entry failed.[%d]\n", ret);
		goto err_create_proc_failed;
	}

	/* log_level initialize. */
	g_fm34_log_level = FM34_LOG_ERR;

	/* FM34 parameter area initialized. */
	ret = fm34_set_bypass_mode();
	if (ret < 0) {
		fm34_pr_err("bypass setting failed [%d]\n", ret);
		goto err_bypass_mode_failed;
	}

	/* Change path (bypass-mode). */
	ret = fm34_set_idle_mode();
	if (ret < 0) {
		fm34_pr_err("bypass setting failed [%d]\n", ret);
		goto err_bypass_mode_failed;
	}

	/* Global parameter initialized. */
	g_fm34_event = 0;
	g_fm34_curt_status = FM34_STATUS_IDLE;

	fm34_pr_func_end();

	return 0;

err_bypass_mode_failed:
	fm34_remove_proc_entry();
err_create_proc_failed:
	platform_driver_unregister(&fm34_platform_driver);
err_pf_driver_failed:
	platform_device_unregister(&fm34_platform_device);
err_pf_device_failed:
	fm34_disable_vclk4();
err_init_bootup_failed:
	iounmap(&g_fm34_vclk_adr);
	fm34_gpio_free();
err_clk_set_parent:
	clk_put(local_main_clk);
err_clk_get_main:
	clk_put(g_fm34_vclk4_clk);
err_init_gpio_failed:
	mutex_destroy(&g_fm34_lock);
err_alloc_data_failed:
	fm34_pr_func_end();
	return ret;
}

static int fm34_remove(struct i2c_client *client)
{
	struct fm34_platform_data *pfm34data = i2c_get_clientdata(client);

	fm34_pr_func_start();

	/* register function remove */
	fm34_remove_proc_entry();
	platform_driver_unregister(&fm34_platform_driver);
	platform_device_unregister(&fm34_platform_device);
	fm34_disable_vclk4();
	clk_put(g_fm34_vclk4_clk);
	iounmap(&g_fm34_vclk_adr);
	fm34_gpio_free();
	mutex_destroy(&g_fm34_lock);
	kfree(pfm34data);

	fm34_pr_func_end();
	return 0;
}

static const struct i2c_device_id fm34_id[] = {
	{ FM34_MODULE_NAME, 0 },
	{ }
};

static struct i2c_driver fm34_driver = {
	.probe = fm34_probe,
	.remove = fm34_remove,
	.id_table = fm34_id,
	.driver = {
		.name = FM34_MODULE_NAME,
	},
};

static int __init fm34_init(void)
{
#if defined(CONFIG_MACH_GARDALTE)
	unsigned int board_rev = 0;
	/* get board rev */
	board_rev = u2_get_board_rev();
	if (board_rev == 6)
		board_rev = 1;
	if (board_rev <= 1)
		return -ENODEV;
#endif
#if defined(CONFIG_MACH_LOGANLTE)
	unsigned int board_rev = 0;
	/* get board rev */
	board_rev = u2_get_board_rev();
	if (board_rev > 0)
		return -ENODEV;
#endif
	return i2c_add_driver(&fm34_driver);
}

static void __exit fm34_exit(void)
{
#if defined(CONFIG_MACH_GARDALTE)
	unsigned int board_rev = 0;
	/* get board rev */
	board_rev = u2_get_board_rev();
	if (board_rev == 6)
		board_rev = 1;
	if (board_rev <= 1)
		return;
#endif
#if defined(CONFIG_MACH_LOGANLTE)
	unsigned int board_rev = 0;
	/* get board rev */
	board_rev = u2_get_board_rev();
	if (board_rev > 0)
		return;
#endif
	i2c_del_driver(&fm34_driver);
}

module_init(fm34_init);
module_exit(fm34_exit);

MODULE_DESCRIPTION("fm34 voice processor driver");
MODULE_LICENSE("GPL");
