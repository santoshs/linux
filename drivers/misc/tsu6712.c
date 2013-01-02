/*
 * driver/misc/tsu6712.c - TSU6712 micro USB switch device driver
 *
 * Copyright (C) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/tsu6712.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/input.h>
#include <linux/switch.h>
#include <linux/power_supply.h>
#include <linux/ctype.h>
#ifdef CONFIG_SEC_CHARGING_FEATURE
#include <linux/spa_power.h>
#endif
#include <linux/pmic/pmic-tps80032.h>

#include <mach/common.h>

static struct switch_dev switch_dock = {
	.name = "dock",
};

static struct switch_dev switch_usb_uart = {
        .name = "tsu6712",
};


static int usb_uart_switch_state;
static int state;
char at_isi_switch_buf[1000] = {0};
int KERNEL_LOG;

/* TSU6712 I2C registers */
#define TSU6712_REG_DEVID		0x01
#define TSU6712_REG_CTRL		0x02
#define TSU6712_REG_INT1		0x03
#define TSU6712_REG_INT2		0x04
#define TSU6712_REG_INT1_MASK		0x05
#define TSU6712_REG_INT2_MASK		0x06
#define TSU6712_REG_ADC			0x07
#define TSU6712_REG_TIMING1		0x08
#define TSU6712_REG_TIMING2		0x09
#define TSU6712_REG_DEV_T1		0x0a
#define TSU6712_REG_DEV_T2		0x0b
#define TSU6712_REG_BTN1		0x0c
#define TSU6712_REG_BTN2		0x0d
#define TSU6712_REG_MANSW1		0x13
#define TSU6712_REG_MANSW2		0x14
#define TSU6712_REG_MANUAL_OVERRIDES1	0x1B
#define TSU6712_REG_RESERVED_1D		0x1D
#define TSU6712_REG_RESERVED_20		0x20

/* Control */
#define CON_SWITCH_OPEN		(1 << 4)
#define CON_RAW_DATA		(1 << 3)
#define CON_MANUAL_SW		(1 << 2)
#define CON_WAIT		(1 << 1)
#define CON_INT_MASK		(1 << 0)
#define CON_MASK		(CON_SWITCH_OPEN | CON_RAW_DATA | \
				CON_MANUAL_SW | CON_WAIT)

/* Device Type 1 */
#define DEV_USB_OTG		(1 << 7)
#define DEV_DEDICATED_CHG	(1 << 6)
#define DEV_USB_CHG		(1 << 5)
#define DEV_CAR_KIT		(1 << 4)
#define DEV_UART		(1 << 3)
#define DEV_USB			(1 << 2)
#define DEV_AUDIO_2		(1 << 1)
#define DEV_AUDIO_1		(1 << 0)

#define DEV_T1_USB_MASK		(DEV_USB_OTG | DEV_USB_CHG | DEV_USB)
#define DEV_T1_UART_MASK	(DEV_UART)
#define DEV_T1_CHARGER_MASK	(DEV_DEDICATED_CHG | DEV_CAR_KIT)

/* Device Type 2 */
#define DEV_SMARTDOCK		(1 << 7)
#define DEV_AV			(1 << 6)
#define DEV_TTY			(1 << 5)
#define DEV_PPD			(1 << 4)
#define DEV_JIG_UART_OFF	(1 << 3)
#define DEV_JIG_UART_ON		(1 << 2)
#define DEV_JIG_USB_OFF		(1 << 1)
#define DEV_JIG_USB_ON		(1 << 0)

#define DEV_T2_USB_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON)
#define DEV_T2_UART_MASK	(DEV_JIG_UART_OFF)
#define DEV_T2_JIG_MASK		(DEV_JIG_USB_OFF | DEV_JIG_USB_ON | DEV_JIG_UART_OFF)
#define VBUS_VALID		(1 << 1)		

/*
 * Manual Switch
 * D- [7:5] / D+ [4:2]
 * 000: Open all / 001: USB / 010: AUDIO / 011: UART / 100: V_AUDIO
 */
#define SW_VAUDIO		((4 << 5) | (4 << 2) | (1 << 1) | (1 << 0))
#define SW_UART			((3 << 5) | (3 << 2))
#define SW_AUDIO		((2 << 5) | (2 << 2) | (1 << 1) | (1 << 0))
#define SW_DHOST		((1 << 5) | (1 << 2) | (1 << 1) | (1 << 0))
#define SW_AUTO			((0 << 5) | (0 << 2))
#define SW_USB_OPEN		(1 << 0)
#define SW_ALL_OPEN		(0)

/* Interrupt 1 */
#define INT_DETACH		(1 << 1)
#define INT_ATTACH		(1 << 0)

#define	ADC_GND			0x00
#define	ADC_MHL			0x01
#define	ADC_DOCK_PREV_KEY 0x04
#define	ADC_DOCK_NEXT_KEY 0x07
#define	ADC_DOCK_VOL_DN		0x0a
#define	ADC_DOCK_VOL_UP		0x0b
#define	ADC_DOCK_PLAY_PAUSE_KEY 0x0d
#define	ADC_CEA936ATYPE1_CHG	0x17
#define	ADC_JIG_USB_OFF		0x18
#define	ADC_JIG_USB_ON		0x19
#define	ADC_DESKDOCK		0x1a
#define	ADC_CEA936ATYPE2_CHG	0x1b
#define	ADC_JIG_UART_OFF	0x1c
#define	ADC_JIG_UART_ON		0x1d
#define	ADC_CARDOCK		0x1d
#define ADC_OPEN		0x1f

int uart_connecting;
EXPORT_SYMBOL(uart_connecting);

int detached_status;
EXPORT_SYMBOL(detached_status);

struct tsu6712_usbsw {
	struct i2c_client		*client;
	struct tsu6712_platform_data	*pdata;
	int				dev1;
	int				dev2;
	int				mansw;
	int				dock_attached;
    int     is_ovp;

	struct input_dev	*input;
	int			previous_key;

	struct delayed_work init_work;
	struct mutex		mutex;
	int				adc;
	int				deskdock;
	int				vbus;
	/* JIRA ID 1362/1396
	Wakelock introduced to avoid device getting
	into deep sleep when UART JIG connected.*/
	struct wake_lock uart_wakelock;
	struct irq_chip irq_chip;
	int             irq_base;
};

enum {
	DOCK_KEY_NONE			= 0,
	DOCK_KEY_VOL_UP_PRESSED,
	DOCK_KEY_VOL_UP_RELEASED,
	DOCK_KEY_VOL_DOWN_PRESSED,
	DOCK_KEY_VOL_DOWN_RELEASED,
	DOCK_KEY_PREV_PRESSED,
	DOCK_KEY_PREV_RELEASED,
	DOCK_KEY_PLAY_PAUSE_PRESSED,
	DOCK_KEY_PLAY_PAUSE_RELEASED,
	DOCK_KEY_NEXT_PRESSED,
	DOCK_KEY_NEXT_RELEASED,
};

static struct tsu6712_usbsw *local_usbsw;
static enum cable_type_t set_cable_status;

#ifdef CONFIG_SAMSUNG_MHL
#define CONFIG_VIDEO_MHL_V2
#endif
#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
#define MHL_DEVICE 2
static int isDeskdockconnected;
//extern u8 mhl_onoff_ex(bool onoff);
static inline u8 mhl_onoff_ex(bool onoff){ return 0;} //temp
#endif

static void tsu6712_reg_init(struct tsu6712_usbsw *usbsw);
static void tsu6712_init_usb_irq(struct tsu6712_usbsw *data);

int get_cable_type(void)
{    
    return set_cable_status;
}
EXPORT_SYMBOL(get_cable_type);

static int tsu6712_write_reg(struct i2c_client *client,        u8 reg, u8 data)
{
       int ret = 0;
       u8 buf[2];
       struct i2c_msg msg[1];

       buf[0] = reg;
       buf[1] = data;

       msg[0].addr = client->addr;
       msg[0].flags = 0;
       msg[0].len = 2;
       msg[0].buf = buf;

	   printk("[tsu6712] tsu8111_write_reg   reg[0x%2x] data[0x%2x]\n",buf[0],buf[1]);

       ret = i2c_transfer(client->adapter, msg, 1);
       if (ret != 1) {
               printk("\n [tsu6712] i2c Write Failed (ret=%d) \n", ret);
               return -1;
       }
       
       return ret;
}

static int tsu6712_read_reg(struct i2c_client *client, u8 reg, u8 *data)
{
       int ret = 0;
       u8 buf[1];
       struct i2c_msg msg[2];

       buf[0] = reg;

        msg[0].addr = client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = buf;

        msg[1].addr = client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 1;
        msg[1].buf = buf;
		
	printk("[tsu6712] tsu8111_read_reg reg[0x%2x] ", buf[0]);

       ret = i2c_transfer(client->adapter, msg, 2);
       if (ret != 2) {
               printk("\n [tsu6712] i2c Read Failed (ret=%d) \n", ret);
               return -1;
       }
       *data = buf[0];

      printk(" data [0x%2x]   i2c Read success\n",buf[0]);
       return 0;
}

static int tsu6712_read_word_reg(struct i2c_client *client, u8 reg, int *data)
{
       int ret = 0;
       u8 buf[1];
	u8 data1,data2;   
       struct i2c_msg msg[2];

       buf[0] = reg;

        msg[0].addr = client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = buf;

        msg[1].addr = client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 1;
        msg[1].buf = buf;
		
	printk("[tsu6712] tsu8111_read_reg reg[0x%2x] ", buf[0]);

       ret = i2c_transfer(client->adapter, msg, 2);
       if (ret != 2) {
               printk("\n [tsu6712] i2c Read Failed (ret=%d) \n", ret);
               return -1;
       }

	data1 = buf[0];

	  buf[0] = reg+1;

        msg[0].addr = client->addr;
        msg[0].flags = 0;
        msg[0].len = 1;
        msg[0].buf = buf;

        msg[1].addr = client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = 1;
        msg[1].buf = buf;
		
	printk("[tsu6712] tsu8111_read_reg reg[0x%2x] ", buf[0]);

       ret = i2c_transfer(client->adapter, msg, 2);
       if (ret != 2) {
               printk("\n [tsu6712] i2c Read Failed (ret=%d) \n", ret);
               return -1;
       }

	data2 = buf[0];
	*data = (int)((data2<<8) | data1);

      printk(" data [%d]   i2c Read success\n",*data);
       return 0;
}


static int tsu6712_dock_init(void)
{
	int ret;

	/* for CarDock, DeskDock */
	ret = switch_dev_register(&switch_dock);
	if (ret < 0) {
		pr_err("Failed to register dock switch. %d\n", ret);
		return ret;
	}
        /* for usb uart switch */
        ret = switch_dev_register(&switch_usb_uart);
        if (ret < 0) {
                pr_err("Failed to register dock switch. %d\n", ret);
                return ret;
        }

	return 0;
}
static int tsu6712_ex_init(void)
{
	tsu6712_dock_init();

       return 0;
}

static void tsu6712_otg_cb(bool attached)
{
	set_cable_status = attached ? CABLE_TYPE_USB : CABLE_TYPE_NONE;
	switch (set_cable_status) {
	case CABLE_TYPE_USB:
		break;
	case CABLE_TYPE_NONE:
		break;
        default:
            break;
	}
}

static void tsu6712_usb_cdp_cb(bool attached)
{
	pr_info("tsu6712_usb_cdp_cb attached %d\n", attached);
}

static void tsu6712_smartdock_cb(bool attached)
{
	pr_info("tsu6712_smartdock_cb attached %d\n", attached);
}

static void tsu6712_dock_cb(int attached)
{
	pr_info("%s attached %d\n", __func__, attached);
	switch_set_state(&switch_dock, attached);
}

static void tsu6712_ovp_cb(bool attached)
{
	pr_info("%s:%s\n",__func__,(attached?"TRUE":"FALSE")); 
	if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_SEC_CHARGING_FEATURE
		spa_event_handler(SPA_EVT_OVP, (int)attached);
#endif
	}
}

static void tsu6712_usb_cb(bool attached)
{
   pr_info("tsu6712_usb_cb attached %d\n", attached);

   set_cable_status = attached ? CABLE_TYPE_USB : CABLE_TYPE_NONE;
   if(attached)
   {	
      printk("USB attached : MUIC send switch state 100");
      usb_uart_switch_state = 100;
      switch_set_state(&switch_usb_uart,100); 
   }
   else
   {
      printk("USB detached : MUIC send switch state 101");
      usb_uart_switch_state = 101;
      switch_set_state(&switch_usb_uart,101);
   }

   printk("%s : %d", __func__,__LINE__);


   if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_SEC_CHARGING_FEATURE
     switch (set_cable_status) {
     case CABLE_TYPE_USB:
       spa_event_handler(SPA_EVT_CHARGER, POWER_SUPPLY_TYPE_USB);
       pr_info("%s USB attached\n",__func__);
       break;

     case CABLE_TYPE_NONE:
       spa_event_handler(SPA_EVT_CHARGER, POWER_SUPPLY_TYPE_BATTERY);
       pr_info("%s USB removed\n",__func__);
       break;
     default:
       break;
     }
#endif
   }
}

static void tsu6712_charger_cb(bool attached)
{
   pr_info("tsu6712_charger_cb attached %d\n", attached);

   set_cable_status = attached ? CABLE_TYPE_AC : CABLE_TYPE_NONE;
   if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_SEC_CHARGING_FEATURE
     switch (set_cable_status) {
     case CABLE_TYPE_AC:
       spa_event_handler(SPA_EVT_CHARGER, POWER_SUPPLY_TYPE_USB_DCP);
       pr_info("%s TA attached\n",__func__);
       break;
     case CABLE_TYPE_NONE:
       spa_event_handler(SPA_EVT_CHARGER, POWER_SUPPLY_TYPE_BATTERY);
       pr_info("%s TA removed\n",__func__);
       break;
     default:
       break;       
     }
#endif
   }
}

static void tsu6712_jig_cb(bool attached)
{    
   pr_info("tsu6712_jig_cb attached %d\n", attached);
   set_cable_status = CABLE_TYPE_NONE;
}

static void tsu6712_uart_cb(bool attached)
{
#ifdef CONFIG_SEC_CHARGING_FEATURE
   SPA_ACC_INFO_T acc_info;
#endif
   pr_info("tsu6712_uart_cb attached %d\n", attached);
   set_cable_status = CABLE_TYPE_NONE;
   printk("%s : %d", __func__,__LINE__);
   if(attached)
   {	
      printk("UART attached : send switch state 200");
      usb_uart_switch_state = 200;
      switch_set_state(&switch_usb_uart,200);
   }
   else
   {
      printk("UART detached : send switch state 201");
      usb_uart_switch_state = 201;
      switch_set_state(&switch_usb_uart,201);
   }
   printk("%s : %d", __func__,__LINE__);

  if (u2_get_board_rev() >= 5) {
#ifdef CONFIG_SEC_CHARGING_FEATURE
   if(attached==true)
   {
        acc_info=SPA_ACC_JIG_UART;    
   }
   else
   {
        acc_info=SPA_ACC_NONE;
   }
#endif
  }
#ifdef CONFIG_SEC_CHARGING_FEATURE
   spa_event_handler(SPA_EVT_ACC_INFO, (void *)acc_info);
#endif
}

static struct tsu6712_platform_data tsu6712_pdata = {
	.usb_cb = tsu6712_usb_cb,
	.charger_cb = tsu6712_charger_cb,
	.jig_cb = tsu6712_jig_cb,
	.uart_cb = tsu6712_uart_cb,
	.otg_cb = tsu6712_otg_cb,	
	.ovp_cb = tsu6712_ovp_cb,
	.dock_cb = tsu6712_dock_cb,
	.ex_init = tsu6712_ex_init,
	.usb_cdp_cb = tsu6712_usb_cdp_cb,
	.smartdock_cb = tsu6712_smartdock_cb,
};

static void DisableTSU6712Interrupts(void)
{
	struct i2c_client *client = local_usbsw->client;
	int value, ret;

	tsu6712_read_reg(client, TSU6712_REG_CTRL,&value);
	value |= 0x01;

	ret = tsu6712_write_reg(client, TSU6712_REG_CTRL, value);
	
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

}

static void EnableTSU6712Interrupts(void)
{
	struct i2c_client *client = local_usbsw->client;
	int value, ret;

	tsu6712_read_reg(client, TSU6712_REG_INT1,&value);
    tsu6712_read_reg(client, TSU6712_REG_CTRL, &value);
	value &= 0xFE;

	ret = tsu6712_write_reg(client, TSU6712_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

}

void TSU6712_CheckAndHookAudioDock(int value)
{
	struct i2c_client *client = local_usbsw->client;
	struct tsu6712_platform_data *pdata = local_usbsw->pdata;
	int ret = 0;

	if (value) {
		pr_info("TSU6712_CheckAndHookAudioDock ON\n");
			if (pdata->dock_cb)
				pdata->dock_cb(TSU6712_ATTACHED_DESK_DOCK);

			ret = tsu6712_write_reg(client,
					TSU6712_REG_MANSW1, SW_AUDIO);

			if (ret < 0)
				dev_err(&client->dev, "%s: err %d\n",__func__, ret);
			
			tsu6712_read_reg(client,TSU6712_REG_CTRL,&ret);
			if (ret < 0)
				dev_err(&client->dev, "%s: err %d\n",__func__, ret);

			ret = tsu6712_write_reg(client,	TSU6712_REG_CTRL,
					ret & ~CON_MANUAL_SW & ~CON_RAW_DATA);
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);
		} else {
			dev_info(&client->dev,"TSU6712_CheckAndHookAudioDock Off\n");

			if (pdata->dock_cb)
				pdata->dock_cb(TSU6712_DETACHED_DOCK);

			tsu6712_read_reg(client,TSU6712_REG_CTRL,&ret);
			
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);

			ret = tsu6712_write_reg(client,	TSU6712_REG_CTRL,
					ret | CON_MANUAL_SW | CON_RAW_DATA);
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);
	}
}

static ssize_t tsu6712_show_control(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	tsu6712_read_reg(client,TSU6712_REG_CTRL,&value);

	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	return snprintf(buf, 13, "CONTROL: %02x\n", value);
}

static ssize_t tsu6712_show_device_type(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int value;

	tsu6712_read_reg(client,TSU6712_REG_DEV_T1,&value);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	return snprintf(buf, 11, "DEVICE_TYPE: %02x\n", value);
}

static ssize_t tsu6712_show_manualsw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned int value;

	tsu6712_read_reg(client,TSU6712_REG_MANSW1,&value);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if (value == SW_VAUDIO)
		return snprintf(buf, 7, "VAUDIO\n");
	else if (value == SW_UART)
		return snprintf(buf, 5, "UART\n");
	else if (value == SW_AUDIO)
		return snprintf(buf, 6, "AUDIO\n");
	else if (value == SW_DHOST)
		return snprintf(buf, 6, "DHOST\n");
	else if (value == SW_AUTO)
		return snprintf(buf, 5, "AUTO\n");
	else
		return snprintf(buf, 4, "%x", value);
}

static ssize_t tsu6712_set_manualsw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned int value;
	unsigned int path = 0;
	int ret;

	tsu6712_read_reg(client,TSU6712_REG_CTRL,&value);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~CON_MANUAL_SW) !=	(CON_SWITCH_OPEN | CON_RAW_DATA | CON_WAIT))
		return 0;

	if (!strncmp(buf, "VAUDIO", 6)) {
		path = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "UART", 4)) {
		path = SW_UART;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "AUDIO", 5)) {
		path = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "DHOST", 5)) {
		path = SW_DHOST;
		value &= ~CON_MANUAL_SW;
	} else if (!strncmp(buf, "AUTO", 4)) {
		path = SW_AUTO;
		value |= CON_MANUAL_SW;
	} else {
		dev_err(dev, "Wrong command\n");
		return 0;
	}

	usbsw->mansw = path;

	ret = tsu6712_write_reg(client, TSU6712_REG_MANSW1, path);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	ret = tsu6712_write_reg(client, TSU6712_REG_CTRL, value);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	return count;
}

static ssize_t tsu6712_show_usb_state(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	unsigned char device_type1, device_type2;

	tsu6712_read_reg(client,TSU6712_REG_DEV_T1,&device_type1);
	tsu6712_read_reg(client,TSU6712_REG_DEV_T2,&device_type2);
	
	if (device_type1 & DEV_T1_USB_MASK || device_type2 & DEV_T2_USB_MASK)
		return snprintf(buf, 22, "USB_STATE_CONFIGURED\n");

	return snprintf(buf, 25, "USB_STATE_NOTCONFIGURED\n");
}

static ssize_t tsu6712_show_adc(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int adc;

	tsu6712_read_reg(client, TSU6712_REG_ADC,&adc);
	if (adc < 0) {
		dev_err(&client->dev,"%s: err at read adc %d\n", __func__, adc);
		return snprintf(buf, 9, "UNKNOWN\n");
	}

	return snprintf(buf, 4, "%x\n", adc);
}

static ssize_t tsu6712_reset(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	struct i2c_client *client = usbsw->client;
	int ret;

	if (!strncmp(buf, "1", 1)) {
		dev_info(&client->dev, "fsa9480 reset after delay 1000 msec.\n");
		mdelay(1000);
		ret = tsu6712_write_reg(client,TSU6712_REG_MANUAL_OVERRIDES1, 0x01);
		if (ret < 0)
			dev_err(&client->dev,"cannot soft reset, err %d\n", ret);

		dev_info(&client->dev, "fsa9480_reset_control done!\n");
	} 
	else {
		dev_info(&client->dev,"fsa9480_reset_control, but not reset_value!\n");
	}

	tsu6712_reg_init(usbsw);

	return count;
}

static ssize_t tsu6712_show_UUS_state(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	return snprintf(buf, 4, "%d\n", usb_uart_switch_state);
}

#define	SWITCH_AT	103
#define	SWITCH_ISI	104

static ssize_t set_manualsw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	if (0 == strncmp(buf, "switch at", 9))
		switch_set_state(&switch_dock, SWITCH_AT);

	if (0 == strncmp(buf, "switch isi", 9))
		switch_set_state(&switch_dock, SWITCH_ISI);

	return count;
}


/* AT-ISI Separation starts */
extern int stop_isi;
char at_isi_mode[100] = {0};

static ssize_t ld_show_mode(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	strcpy(buf, at_isi_mode);
	printk("LD MODE from TSU %s\n", at_isi_mode);
	return 3;
}

static ssize_t ld_show_manualsw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

ssize_t ld_set_manualsw(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	printk(" ld_set_manualsw invoked\n");
	if (0 == strncmp(buf, "switch at", 9)) {
		printk(" ld_set_manualsw switch at\n");
		memset((char *)at_isi_mode, 0, 100);
		strcpy((char *)at_isi_mode, "at");
		switch_set_state(&switch_usb_uart, SWITCH_AT);

		stop_isi = 1;
	}
	if (0 == strncmp(buf, "switch isi", 10)) {
		printk(" ld_set_manualsw switch isi\n");
		memset((char *)at_isi_mode, 0, 100);
		strcpy((char *)at_isi_mode, "isi");
		switch_set_state(&switch_usb_uart, SWITCH_ISI);
		stop_isi = 0;
	}
	return count;
}

static ssize_t ld_show_switch_buf(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	strcpy(buf, at_isi_switch_buf);
	printk("BUF from TSU %s\n", at_isi_switch_buf);
	return strlen(at_isi_switch_buf);
}


ssize_t ld_set_switch_buf(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int ret = 0;
	int error = 0;
	char *ptr = NULL;
	int i = 0;
	char temp[100];

	/* If UART is not connected ignore this sysfs access*/
	if (200 != usb_uart_switch_state)
		return 0;

	memset(temp, 0, 100);
	for (i = 0; i < count; i++)
		temp[i] = toupper(buf[i]);

	strncat((char *)at_isi_switch_buf, temp, count);

	if (at_isi_switch_buf) {
		if (strstr(at_isi_switch_buf, "\r\n"))
			printk("###WIPRO### r n\n");
		else if (strstr(at_isi_switch_buf, "\t\n"))
			printk("###WIPRO### t n\n");
		else if (strstr(at_isi_switch_buf, "\n"))
			printk("###WIPRO### n\n");
	}

	ptr = strstr("AT+ATSTART" , at_isi_switch_buf);
	if (NULL == ptr) {
		ptr = strstr("AT+ISISTART", at_isi_switch_buf);
		if (NULL == ptr)
			error = 1;
	}

	ptr = strstr(at_isi_switch_buf, "AT+ATSTART");
	if (NULL != ptr) {
		printk("ld_set_switch_buf : switch at");
		KERNEL_LOG = 0;
		ld_set_manualsw(NULL, NULL, "switch at", 9);
		memset(at_isi_switch_buf, 0, 1000);
		error = 0;
	} else {
		ptr = strstr(at_isi_switch_buf, "AT+ISISTART");
		if (NULL != ptr) {
			printk("ld_set_switch_buf : switch isi");
			KERNEL_LOG = 0;
			ld_set_manualsw(NULL, NULL, "switch isi", 10);
			memset(at_isi_switch_buf, 0, 1000);
			error = 0;
		}
	}

	if (error != 0) {
		count = -1;
		memset(at_isi_switch_buf, 0, 1000);
	}
	return count;
}

/* JIRA ID 1362/1396
Sysfs interface to release and acquire
uart-wakelock from user space */
ssize_t ld_uart_wakelock(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	int ret = 0;
	struct tsu6712_usbsw *usbsw = dev_get_drvdata(dev);
	int buf_val = 0;

	if (usbsw != NULL) {
		ret = sscanf(buf, "%d", &buf_val);
		if (1 != ret) {
			printk(KERN_ERR \
				"ld_uart_wakelock - Failed to read value\n");
			return -EINVAL;
		}
		if (buf_val == 1) {
			/* Release wakelock
			to allow device to get into deep sleep
			when UART JIG is disconnected */
			wake_unlock(&usbsw->uart_wakelock);
			ret = count ;
		} else if (buf_val == 0) {
			/* Acquire wakelock
			to avoid device getting into deep sleep
			when UART JIG is connected*/
			wake_lock(&usbsw->uart_wakelock);
			ret = count ;
		}
	}
	return ret ;
}
static DEVICE_ATTR(control, S_IRUGO, tsu6712_show_control, NULL);
static DEVICE_ATTR(device_type, S_IRUGO, tsu6712_show_device_type, NULL);
static DEVICE_ATTR(switch, S_IRUGO | S_IWUSR,
		tsu6712_show_manualsw, tsu6712_set_manualsw);
static DEVICE_ATTR(usb_state, S_IRUGO, tsu6712_show_usb_state, NULL);
static DEVICE_ATTR(adc, S_IRUGO, tsu6712_show_adc, NULL);
static DEVICE_ATTR(reset_switch, S_IWUSR | S_IWGRP, NULL, tsu6712_reset);
/* AT-ISI Separation starts */
static DEVICE_ATTR(at_isi_switch, S_IRUGO | S_IWUSR,
		ld_show_manualsw, ld_set_manualsw);
static DEVICE_ATTR(at_isi_mode, S_IRUGO | S_IWUSR,
		ld_show_mode, NULL);

static DEVICE_ATTR(at_isi_switch_buf, S_IRUGO | S_IWUSR,
		ld_show_switch_buf, ld_set_switch_buf);
/* AT-ISI Separation Ends */

static DEVICE_ATTR(UUS_state, S_IRUGO, tsu6712_show_UUS_state, NULL);
/* JIRA ID 1362/1396
Sysfs interface to release and acquire uart-wakelock from user space */
static DEVICE_ATTR(uart_wakelock, S_IRUGO | S_IWUSR,
		NULL, ld_uart_wakelock);

static struct attribute *tsu6712_attributes[] = {
	&dev_attr_control.attr,
	&dev_attr_device_type.attr,
	&dev_attr_switch.attr,
	&dev_attr_at_isi_switch.attr,	/* AT-ISI Separation */
	&dev_attr_at_isi_mode,		/* AT-ISI Separation */
	&dev_attr_at_isi_switch_buf,	/* AT-ISI Separation */
	&dev_attr_UUS_state.attr,
	/* JIRA ID 1362/1396
	uart-wakelock release */
	&dev_attr_uart_wakelock.attr,
	NULL
};

static const struct attribute_group tsu6712_group = {
	.attrs = tsu6712_attributes,
};

void tsu6712_otg_detach(void)
{
	unsigned int data = 0;
	int ret;
	struct i2c_client *client = local_usbsw->client;

	if (local_usbsw->dev1 & DEV_USB_OTG) {
		dev_info(&client->dev, "%s: real device\n", __func__);

	data = 0x00;
	ret = tsu6712_write_reg(client, TSU6712_REG_MANSW2, data);
	if (ret < 0)
		dev_info(&client->dev, "%s: err %d\n", __func__, ret);

	data = SW_ALL_OPEN;
	ret = tsu6712_write_reg(client, TSU6712_REG_MANSW1, data);
	if (ret < 0)
		dev_info(&client->dev, "%s: err %d\n", __func__, ret);

	data = 0x1A;
	ret = tsu6712_write_reg(client, TSU6712_REG_CTRL, data);
	if (ret < 0)
		dev_info(&client->dev, "%s: err %d\n", __func__, ret);
	} else
		dev_info(&client->dev, "%s: not real device\n", __func__);
}
EXPORT_SYMBOL(tsu6712_otg_detach);

//ENABLE_OTG
bool otg_status = 0;
//to check the ID status
u8 tsu6712_otg_status(void)
{
	return otg_status;
}
EXPORT_SYMBOL(tsu6712_otg_status);

//control the external OTG booster
void tsu6712_otg_vbus_en(u8 on)
{
	gpio_direction_output(8/*OTG_EN*/,on);//OTG_EN high
}
EXPORT_SYMBOL(tsu6712_otg_vbus_en);

void tsu6712_manual_switching(int path)
{
	struct i2c_client *client = local_usbsw->client;
	unsigned int value;
	unsigned int data = 0;
	int ret;

	tsu6712_read_reg(client,TSU6712_REG_CTRL,&value);
	if (value < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, value);

	if ((value & ~CON_MANUAL_SW) !=	(CON_SWITCH_OPEN | CON_RAW_DATA | CON_WAIT))
		return;

	if (path == SWITCH_PORT_VAUDIO) {
		data = SW_VAUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_UART) {
		data = SW_UART;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_AUDIO) {
		data = SW_AUDIO;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_USB) {
		data = SW_DHOST;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_AUTO) {
		data = SW_AUTO;
		value |= CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_USB_OPEN) {
		data = SW_USB_OPEN;
		value &= ~CON_MANUAL_SW;
	} else if (path ==  SWITCH_PORT_ALL_OPEN) {
		data = SW_ALL_OPEN;
		value &= ~CON_MANUAL_SW;
	} else {
		pr_info("%s: wrong path (%d)\n", __func__, path);
		return;
	}

	local_usbsw->mansw = data;

	/* path for FTM sleep */
	if (path ==  SWITCH_PORT_ALL_OPEN) {
		ret = tsu6712_write_reg(client,TSU6712_REG_MANUAL_OVERRIDES1, 0x0a);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6712_write_reg(client,	TSU6712_REG_MANSW1, data);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6712_write_reg(client,	TSU6712_REG_MANSW2, data);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6712_write_reg(client,	TSU6712_REG_CTRL, value);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	} else {
		ret = tsu6712_write_reg(client,	TSU6712_REG_MANSW1, data);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);

		ret = tsu6712_write_reg(client,	TSU6712_REG_CTRL, value);
		if (ret < 0)
			dev_err(&client->dev, "%s: err %d\n", __func__, ret);
	}
}
EXPORT_SYMBOL(tsu6712_manual_switching);

void tsu6712_vbus_check(bool vbus_status)
{
	struct tsu6712_usbsw *usbsw = local_usbsw;
	struct tsu6712_platform_data *pdata = local_usbsw->pdata;
		
	usbsw->vbus = (int)vbus_status;

	if(vbus_status)
	{
		if (pdata->charger_cb)
			pdata->charger_cb(TSU6712_ATTACHED);			
	}
	else
	{
		if (pdata->charger_cb)
			pdata->charger_cb(TSU6712_DETACHED);			
	}	
}
EXPORT_SYMBOL(tsu6712_vbus_check);

static int tsu6712_detect_dev(struct tsu6712_usbsw *usbsw)
{
	int device_type, ret;
	unsigned char val1, val2, adc;
	struct tsu6712_platform_data *pdata = usbsw->pdata;
	struct i2c_client *client = usbsw->client;
#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
	u8 mhl_ret = 0;
#endif
	tsu6712_read_word_reg(client, TSU6712_REG_DEV_T1,&device_type);
	tsu6712_read_reg(client, TSU6712_REG_ADC,&adc);

	if (device_type < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, device_type);
		return;
	}
	val1 = device_type & 0xff;
	val2 = device_type >> 8;

	dev_info(&client->dev, "dev1: 0x%x, dev2: 0x%x\n", val1, val2);

	if (usbsw->dock_attached)
		pdata->dock_cb(TSU6712_DETACHED_DOCK);

	if (adc == 0x10)
		val2 = DEV_SMARTDOCK;

	/* Attached */
	if (val1 || val2) {
		/* USB */
		if (val1 & DEV_USB || val2 & DEV_T2_USB_MASK) {
			dev_info(&client->dev, "usb Cconnect\n");

			if (pdata->usb_cb)
				pdata->usb_cb(TSU6712_ATTACHED);
			if (usbsw->mansw) {
				ret = tsu6712_write_reg(client,
				TSU6712_REG_MANSW1, usbsw->mansw);

				if (ret < 0)
					dev_err(&client->dev,"%s: err %d\n", __func__, ret);
			}
		}/* USB_CDP */ 
		else if (val1 & DEV_USB_CHG) {
			dev_info(&client->dev, "usb_cdp connect\n");

			if (pdata->usb_cdp_cb)
				pdata->usb_cdp_cb(TSU6712_ATTACHED);
			if (usbsw->mansw) {
				ret = tsu6712_write_reg(client,TSU6712_REG_MANSW1, usbsw->mansw);
				if (ret < 0)
					dev_err(&client->dev,"%s: err %d\n", __func__, ret);
			}
		}/* UART */ 
		else if (val1 & DEV_T1_UART_MASK || val2 & DEV_T2_UART_MASK) {
			uart_connecting = 1;
			dev_info(&client->dev, "uart connect\n");
			/* JIRA ID 1362/1396
			Acquire wakelock to avoid device getting
			into deep sleep when UART JIG is connected */
			wake_lock(&usbsw->uart_wakelock);
			tsu6712_write_reg(client,TSU6712_REG_CTRL, 0x1E);
			if (pdata->uart_cb)
				pdata->uart_cb(TSU6712_ATTACHED);
			if (usbsw->mansw) {
				ret = tsu6712_write_reg(client,
					TSU6712_REG_MANSW1, SW_UART);

				if (ret < 0)
					dev_err(&client->dev,"%s: err %d\n", __func__, ret);
			}
		/* CHARGER */
		} else if (val1 & DEV_T1_CHARGER_MASK) {
			dev_info(&client->dev, "charger connect\n");

			if (pdata->charger_cb)
				pdata->charger_cb(TSU6712_ATTACHED);

		/* for SAMSUNG OTG */
		} else if (val1 & DEV_USB_OTG) {
			dev_info(&client->dev, "otg connect\n");
//ENABLE_OTG
			tsu6712_write_reg(client,TSU6712_REG_MANSW1, 0x27);
			msleep(50);
			tsu6712_write_reg(client,TSU6712_REG_MANSW1, 0x27);
			tsu6712_write_reg(client,TSU6712_REG_CTRL, 0x1a);
#if defined(CONFIG_MACH_CAPRI_SS_S2VE) //Enable OTG only if the model is S2VE
			otg_status = 1;
			if (pdata->otg_cb)
				pdata->otg_cb(TSU6712_ATTACHED);
#endif	
		/* JIG */
		} else if (val2 & DEV_T2_JIG_MASK) {
			dev_info(&client->dev, "jig connect\n");

			if (pdata->jig_cb)
				pdata->jig_cb(TSU6712_ATTACHED);
		/* Desk Dock */
		} else if (val2 & DEV_AV) {
			if ((adc & 0x1F) == ADC_DESKDOCK) {
				pr_info("TSU Deskdock Attach\n");
				TSU6712_CheckAndHookAudioDock(1);
				usbsw->deskdock = 1;
#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
				isDeskdockconnected = 1;
#endif
				tsu6712_write_reg(client,TSU6712_REG_RESERVED_20, 0x08);
				if(usbsw->vbus)
					if (pdata->charger_cb)
						pdata->charger_cb(TSU6712_ATTACHED);					
			} 
			else {
				pr_info("TSU MHL Attach\n");
				tsu6712_write_reg(client,TSU6712_REG_RESERVED_20, 0x08);
		#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
			DisableTSU6712Interrupts();
			if (!isDeskdockconnected)
				mhl_ret = mhl_onoff_ex(1);

				if (mhl_ret != MHL_DEVICE &&(adc & 0x1F) == 0x1A) {
				TSU6712_CheckAndHookAudioDock(1);
				isDeskdockconnected = 1;
			}
			EnableTSU6712Interrupts();
		#else
				pr_info("FSA mhl attach, but not support MHL feature!\n");
		#endif
		}
		}/* Car Dock */ 
		else if (val2 & DEV_JIG_UART_ON) {
			if (pdata->dock_cb)
				pdata->dock_cb(TSU6712_ATTACHED_CAR_DOCK);
			pr_info("car dock connect\n");
			ret = tsu6712_write_reg(client,TSU6712_REG_MANSW1, SW_AUDIO);
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);

			tsu6712_read_reg(client,TSU6712_REG_CTRL,&ret);
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);

			tsu6712_write_reg(client,TSU6712_REG_CTRL, ret & ~CON_MANUAL_SW);
			usbsw->dock_attached = TSU6712_ATTACHED;
		/* SmartDock */
		} else if (val2 & DEV_SMARTDOCK) {
			usbsw->adc = (int)adc;
			dev_info(&client->dev, "smart dock connect\n");
			pr_info("smart dock connect\n");

			usbsw->mansw = SW_DHOST;
			ret = tsu6712_write_reg(client,TSU6712_REG_MANSW1, SW_DHOST);
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);

			tsu6712_read_reg(client,TSU6712_REG_CTRL,&ret);
			if (ret < 0)
				dev_err(&client->dev,"%s: err %d\n", __func__, ret);
			tsu6712_write_reg(client,TSU6712_REG_CTRL, ret & ~CON_MANUAL_SW);
	
			if (pdata->smartdock_cb)
				pdata->smartdock_cb(TSU6712_ATTACHED);
#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
			mhl_onoff_ex(1);
#endif
		}
		if(usbsw->is_ovp)
			pdata->ovp_cb(true);
	}/* Detached */
	else {
		/* USB */
		if (usbsw->dev1 & DEV_USB ||
				usbsw->dev2 & DEV_T2_USB_MASK) {
			if (pdata->usb_cb)
				pdata->usb_cb(TSU6712_DETACHED);
		} else if (usbsw->dev1 & DEV_USB_CHG) {
			if (pdata->usb_cdp_cb)
				pdata->usb_cdp_cb(TSU6712_DETACHED);

		/* UART */
		} else if (usbsw->dev1 & DEV_T1_UART_MASK ||usbsw->dev2 & DEV_T2_UART_MASK) {
			if (pdata->uart_cb)
				pdata->uart_cb(TSU6712_DETACHED);
			uart_connecting = 0;
			dev_info(&client->dev, "[TSU6712] uart disconnect\n");
			/* JIRA ID 1362/1396
			Release wakelock to allow device to get into deep sleep
			when UART JIG is disconnected */
			wake_unlock(&usbsw->uart_wakelock);

		/* CHARGER */
		} else if (usbsw->dev1 & DEV_T1_CHARGER_MASK) {
			if (pdata->charger_cb)
				pdata->charger_cb(TSU6712_DETACHED);
		/* for SAMSUNG OTG */
		} else if (usbsw->dev1 & DEV_USB_OTG) {
#if defined(CONFIG_MACH_CAPRI_SS_S2VE) //Enable OTG only if the model is S2VE
//ENABLE_OTG
			otg_status = 0;
			if (pdata->otg_cb)
				pdata->otg_cb(TSU6712_DETACHED);	
#endif						
			tsu6712_write_reg(client,TSU6712_REG_CTRL, 0x1E);
		/* JIG */
		} else if (usbsw->dev2 & DEV_T2_JIG_MASK) {
			if (pdata->jig_cb)
				pdata->jig_cb(TSU6712_DETACHED);
		/* Desk Dock */
		} else if (usbsw->dev2 & DEV_AV) {
			pr_info("FSA MHL Detach\n");
			tsu6712_write_reg(client,TSU6712_REG_RESERVED_20, 0x04);
#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
			if (isDeskdockconnected)
				TSU6712_CheckAndHookAudioDock(0);
#if 0//defined CONFIG_MHL_D3_SUPPORT
			mhl_onoff_ex(false);
			detached_status = 1;
#endif
			isDeskdockconnected = 0;
#else
			if (usbsw->deskdock) {
			TSU6712_CheckAndHookAudioDock(0);
				usbsw->deskdock = 0;
			} else {
				pr_info("FSA detach mhl cable, but not support MHL feature\n");
			}
#endif
		/* Car Dock */
		}else if (usbsw->dev2 & DEV_JIG_UART_ON) {
			if (pdata->dock_cb)
				pdata->dock_cb(TSU6712_DETACHED_DOCK);
			tsu6712_read_reg(client,TSU6712_REG_CTRL,&ret);
			tsu6712_write_reg(client,TSU6712_REG_CTRL,ret | CON_MANUAL_SW);
			usbsw->dock_attached = TSU6712_DETACHED;
			
		} else if (usbsw->adc == 0x10) {
			dev_info(&client->dev, "smart dock disconnect\n");

			tsu6712_read_reg(client,TSU6712_REG_CTRL,&ret);
			tsu6712_write_reg(client,TSU6712_REG_CTRL,ret | CON_MANUAL_SW);

			if (pdata->smartdock_cb)
				pdata->smartdock_cb(TSU6712_DETACHED);
			usbsw->adc = 0;
#if defined(CONFIG_VIDEO_MHL_V1) || defined(CONFIG_VIDEO_MHL_V2)
			mhl_onoff_ex(false);
#endif
		}
	}
	
	usbsw->dev1 = val1;
	usbsw->dev2 = val2;

	return adc;
}

static void tsu6712_reg_init(struct tsu6712_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	unsigned int ctrl = CON_MASK;
	int ret;
	pr_info("tsu6712_reg_init is called\n");

	tsu6712_write_reg(client,TSU6712_REG_INT1_MASK,0x5c);
	tsu6712_write_reg(client,TSU6712_REG_INT2_MASK,0x18);
	
	/* ADC Detect Time: 500ms */
	ret = tsu6712_write_reg(client, TSU6712_REG_TIMING1, 0x0);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	tsu6712_read_reg(client,TSU6712_REG_MANSW1,&ret);
	usbsw->mansw = ret;
	
	if (usbsw->mansw < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, usbsw->mansw);

	if (usbsw->mansw)
		ctrl &= ~CON_MANUAL_SW;	/* Manual Switching Mode */
	else
		ctrl &= ~(CON_INT_MASK);

	ret = tsu6712_write_reg(client, TSU6712_REG_CTRL, ctrl);
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	tsu6712_read_reg(client,TSU6712_REG_DEVID,&ret);
	
	if (ret < 0)
		dev_err(&client->dev, "%s: err %d\n", __func__, ret);

	dev_info(&client->dev, " tsu6712_reg_init dev ID: 0x%x\n", ret);
}

static int tsu6712_check_dev(struct tsu6712_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int device_type;
	tsu6712_read_word_reg(client, TSU6712_REG_DEV_T1,&device_type);
	if (device_type < 0) {
		dev_err(&client->dev, "%s: err %d\n", __func__, device_type);
		return 0;
	}
	return device_type;
}

static int tsu6712_handle_dock_vol_key(struct tsu6712_usbsw *info, int adc)
{
	struct input_dev *input = info->input;
	int pre_key = info->previous_key;
	unsigned int code;
	int state;

	if (adc == ADC_OPEN) {
		switch (pre_key) {
		case DOCK_KEY_VOL_UP_PRESSED:
			code = KEY_VOLUMEUP;
			state = 0;
			info->previous_key = DOCK_KEY_VOL_UP_RELEASED;
			break;
		case DOCK_KEY_VOL_DOWN_PRESSED:
			code = KEY_VOLUMEDOWN;
			state = 0;
			info->previous_key = DOCK_KEY_VOL_DOWN_RELEASED;
			break;
		case DOCK_KEY_PREV_PRESSED:
			code = KEY_PREVIOUSSONG;
			state = 0;
			info->previous_key = DOCK_KEY_PREV_RELEASED;
			break;
		case DOCK_KEY_PLAY_PAUSE_PRESSED:
			code = KEY_PLAYPAUSE;
			state = 0;
			info->previous_key = DOCK_KEY_PLAY_PAUSE_RELEASED;
			break;
		case DOCK_KEY_NEXT_PRESSED:
			code = KEY_NEXTSONG;
			state = 0;
			info->previous_key = DOCK_KEY_NEXT_RELEASED;
			break;
		default:
			return 0;
		}
		input_event(input, EV_KEY, code, state);
		input_sync(input);
		return 0;
	}

	if (pre_key == DOCK_KEY_NONE) {
		if (adc != ADC_DOCK_VOL_UP && adc != ADC_DOCK_VOL_DN
			&& adc != ADC_DOCK_PREV_KEY && adc != ADC_DOCK_NEXT_KEY
			&& adc != ADC_DOCK_PLAY_PAUSE_KEY)
			return 0;
	}

	switch (adc) {
	case ADC_DOCK_VOL_UP:
		code = KEY_VOLUMEUP;
		state = 1;
		info->previous_key = DOCK_KEY_VOL_UP_PRESSED;
		break;
	case ADC_DOCK_VOL_DN:
		code = KEY_VOLUMEDOWN;
		state = 1;
		info->previous_key = DOCK_KEY_VOL_DOWN_PRESSED;
		break;
	case ADC_DOCK_PREV_KEY-1 ... ADC_DOCK_PREV_KEY+1:
		code = KEY_PREVIOUSSONG;
		state = 1;
		info->previous_key = DOCK_KEY_PREV_PRESSED;
		break;
	case ADC_DOCK_PLAY_PAUSE_KEY-1 ... ADC_DOCK_PLAY_PAUSE_KEY+1:
		code = KEY_PLAYPAUSE;
		state = 1;
		info->previous_key = DOCK_KEY_PLAY_PAUSE_PRESSED;
		break;
	case ADC_DOCK_NEXT_KEY-1 ... ADC_DOCK_NEXT_KEY+1:
		code = KEY_NEXTSONG;
		state = 1;
		info->previous_key = DOCK_KEY_NEXT_PRESSED;
		break;
	case ADC_DESKDOCK:
		if (pre_key == DOCK_KEY_VOL_UP_PRESSED) {
			code = KEY_VOLUMEUP;
			state = 0;
			info->previous_key = DOCK_KEY_VOL_UP_RELEASED;
		} else if (pre_key == DOCK_KEY_VOL_DOWN_PRESSED) {
			code = KEY_VOLUMEDOWN;
			state = 0;
			info->previous_key = DOCK_KEY_VOL_DOWN_RELEASED;
		} else if (pre_key == DOCK_KEY_PREV_PRESSED) {
			code = KEY_PREVIOUSSONG;
			state = 0;
			info->previous_key = DOCK_KEY_PREV_RELEASED;
		} else if (pre_key == DOCK_KEY_PLAY_PAUSE_PRESSED) {
			code = KEY_PLAYPAUSE;
			state = 0;
			info->previous_key = DOCK_KEY_PLAY_PAUSE_RELEASED;
		} else if (pre_key == DOCK_KEY_NEXT_PRESSED) {
			code = KEY_NEXTSONG;
			state = 0;
			info->previous_key = DOCK_KEY_NEXT_RELEASED;
		} else {
			return 0;
		}
		break;
	default:		
		return 0;
	}

	input_event(input, EV_KEY, code, state);
	input_sync(input);

	return 1;
}

static irqreturn_t tsu6712_irq_thread(int irq, void *data)
{
	struct tsu6712_usbsw *usbsw = data;
	struct i2c_client *client = usbsw->client;
	int intr,  intr2, detect,temp;

	/* TSU6712 : Read interrupt -> Read Device
	 TSU6712 : Read Device -> Read interrupt */

	pr_info("tsu6712_irq_thread is called\n");
	mutex_lock(&usbsw->mutex);
	detect = tsu6712_detect_dev(usbsw);
	mutex_unlock(&usbsw->mutex);

	/* read and clear interrupt status bits */
	tsu6712_read_word_reg(client, TSU6712_REG_INT1,&intr);
	intr2 = intr >> 8;

	if (u2_get_board_rev() >= 5) {
		dev_info(&client->dev,"%s intr: 0x%x\n",__func__, intr);
		if (intr) {
			handle_nested_irq(IRQPIN_IRQ_BASE + 64 + TPS80032_INT_VBUSS_WKUP);
			handle_nested_irq(IRQPIN_IRQ_BASE + 64 + TPS80032_INT_VBUS);
		}
	}
	if (intr < 0) {
		msleep(100);
		dev_err(&client->dev, "%s: err %d\n", __func__, intr);
		tsu6712_read_word_reg(client, TSU6712_REG_INT1,&intr);
		if (intr < 0)
			dev_err(&client->dev,"%s: err at read %d\n", __func__, intr);
		tsu6712_reg_init(usbsw);
		return IRQ_HANDLED;
	} else if (intr == 0) {
		/* interrupt was fired, but no status bits were set,
		so device was reset. In this case, the registers were
		reset to defaults so they need to be reinitialised. */
		tsu6712_reg_init(usbsw);
	}
	else if(intr & 0x20) // ovp
	{
		usbsw->is_ovp = 1;
		usbsw->pdata->ovp_cb(true);
	}
	else if(intr & 0x80 && usbsw->is_ovp == 1)
	{
		usbsw->is_ovp = false;
		usbsw->pdata->ovp_cb(false);			
	}
	/* ADC_value(key pressed) changed at AV_Dock.*/
	if (intr2) {
		if (intr2 & 0x4) { /* for adc change */
			tsu6712_handle_dock_vol_key(usbsw, detect);
			dev_info(&client->dev,"intr2: 0x%x, adc_val: %x\n",intr2, detect);
		} else if (intr2 & 0x2) { /* for smart dock */
			tsu6712_read_word_reg(client, TSU6712_REG_INT1,&temp);

		} else if (intr2 & 0x1) { /* for av change (desk dock, hdmi) */
			dev_info(&client->dev,"%s enter Av charing\n", __func__);
	tsu6712_detect_dev(usbsw);
		} else {
			dev_info(&client->dev,"%s intr2 but, nothing happend, intr2: 0x%x\n",__func__, intr2);
		}
		return IRQ_HANDLED;
	}
	return IRQ_HANDLED;
}

static int tsu6712_irq_init(struct tsu6712_usbsw *usbsw)
{
	struct i2c_client *client = usbsw->client;
	int ret;

	if (client->irq) {
		ret = request_threaded_irq(client->irq, NULL,
			tsu6712_irq_thread, (IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND),"tsu6712 micro USB", usbsw);
		if (ret) {
			dev_err(&client->dev, "failed to reqeust IRQ\n");
			return ret;
		}

		ret = enable_irq_wake(client->irq);
		if (ret < 0)
			dev_err(&client->dev,"failed to enable wakeup src %d\n", ret);
	}

	return 0;
}

static void tsu6712_init_detect(struct work_struct *work)
{
	struct tsu6712_usbsw *usbsw = container_of(work,
			struct tsu6712_usbsw, init_work.work);

	dev_info(&usbsw->client->dev, "%s\n", __func__);

	mutex_lock(&usbsw->mutex);
	tsu6712_detect_dev(usbsw);
	mutex_unlock(&usbsw->mutex);

	msleep(1000);
	tsu6712_write_reg(usbsw->client, 0x1b, 0x01);  //reset
	msleep(100);
	tsu6712_reg_init(usbsw);
	msleep(100);

	mutex_lock(&usbsw->mutex);
	tsu6712_detect_dev(usbsw);
	mutex_unlock(&usbsw->mutex);
	msleep(100);
}
   
static int __devinit tsu6712_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct tsu6712_usbsw *usbsw;
	int ret = 0;
	struct input_dev *input;
	struct device *switch_dock;

	pr_info("tsu6712_probe\n");

	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;

	input = input_allocate_device();
	usbsw = kzalloc(sizeof(struct tsu6712_usbsw), GFP_KERNEL);
	if (!usbsw || !input) {
		dev_err(&client->dev, "failed to allocate driver data\n");
		input_free_device(input);
		kfree(usbsw);
		return -ENOMEM;
	}

	usbsw->input = input;
	input->name = client->name;
	input->phys = "deskdock-key/input0";
	input->dev.parent = &client->dev;
	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0001;

	/* Enable auto repeat feature of Linux input subsystem */
	__set_bit(EV_REP, input->evbit);
	input_set_capability(input, EV_KEY, KEY_VOLUMEUP);
	input_set_capability(input, EV_KEY, KEY_VOLUMEDOWN);
	input_set_capability(input, EV_KEY, KEY_PLAYPAUSE);
	input_set_capability(input, EV_KEY, KEY_PREVIOUSSONG);
	input_set_capability(input, EV_KEY, KEY_NEXTSONG);

	ret = input_register_device(input);
	if (ret)
		dev_err(&client->dev,"input_register_device %s: err %d\n", __func__, ret);

	usbsw->client = client;
      if(client->dev.platform_data)
 	usbsw->pdata = client->dev.platform_data;
      else
    	usbsw->pdata = &tsu6712_pdata;

   
	if (!usbsw->pdata)
		goto fail1;

	i2c_set_clientdata(client, usbsw);
	mutex_init(&usbsw->mutex);

	local_usbsw = usbsw;
	uart_connecting = 0;
	detached_status = 0;

	tsu6712_reg_init(usbsw);


	/* JIRA ID 1362/1396
	Init wakelock to prevent device getting into deep sleep
	when UART JIG is connected and allow device to get into
	deep sleep When UART JIG is disconnected */
	wake_lock_init(&usbsw->uart_wakelock,
			WAKE_LOCK_SUSPEND, "uart-wakelock");
	ret = sysfs_create_group(&client->dev.kobj, &tsu6712_group);
	if (ret) {
		dev_err(&client->dev,"failed to create tsu6712 attribute group\n");
 	}

#if 0
	/* make sysfs node /sys/class/sec/switch/usb_state */
	switch_dock = device_create(sec_class, NULL, 0, NULL, "tsu6712");
	if (IS_ERR(switch_dock)) {
		pr_err("[TSU6712] Failed to create device (switch_dock)!\n");
		ret = PTR_ERR(switch_dock);
		goto fail2;
	}

	ret = device_create_file(switch_dock, &dev_attr_usb_state);
	if (ret < 0) {
		pr_err("[TSU6712] Failed to create file (usb_state)!\n");
		goto err_create_file_state;
	}

	ret = device_create_file(switch_dock, &dev_attr_adc);
	if (ret < 0) {
		pr_err("[TSU6712] Failed to create file (adc)!\n");
		goto err_create_file_adc;
	}

	ret = device_create_file(switch_dock, &dev_attr_reset_switch);
	if (ret < 0) {
		pr_err("[TSU6712] Failed to create file (reset_switch)!\n");
		goto err_create_file_reset_switch;
	}
#endif

    if(usbsw->pdata->ex_init)
		usbsw->pdata->ex_init();
    
    usbsw->is_ovp = 0;
    usbsw->mansw = 0;
    set_cable_status = CABLE_TYPE_NONE;

	ret = tsu6712_irq_init(usbsw);
	if (ret)
		dev_info(&usbsw->client->dev,"failed to enable  irq init %s\n", __func__);

	/* initial cable detection */
	INIT_DELAYED_WORK(&usbsw->init_work, tsu6712_init_detect);
	schedule_delayed_work(&usbsw->init_work, msecs_to_jiffies(2700));
	if (u2_get_board_rev() >= 5) {
		tsu6712_init_usb_irq(usbsw);
	}
	return 0;

#if 0
err_create_file_reset_switch:
	device_remove_file(switch_dock, &dev_attr_reset_switch);
err_create_file_adc:
	device_remove_file(switch_dock, &dev_attr_adc);
err_create_file_state:
	device_remove_file(switch_dock, &dev_attr_usb_state);
#endif

fail1:
	mutex_destroy(&usbsw->mutex);
	i2c_set_clientdata(client, NULL);
	input_free_device(input);
	kfree(usbsw);
	return ret;
}

static int __devexit tsu6712_remove(struct i2c_client *client)
{
	struct tsu6712_usbsw *usbsw = i2c_get_clientdata(client);

	cancel_delayed_work(&usbsw->init_work);
	if (client->irq) {
		disable_irq_wake(client->irq);
		free_irq(client->irq, usbsw);
	}
	mutex_destroy(&usbsw->mutex);
	/* JIRA ID 1362/1396
	Destroy wakelock */
	wake_lock_destroy(&usbsw->uart_wakelock);
	i2c_set_clientdata(client, NULL);

	sysfs_remove_group(&client->dev.kobj, &tsu6712_group);
	kfree(usbsw);
	return 0;
}

static int tsu6712_suspend(struct i2c_client *client,
		pm_message_t state)
{
	return 0;
}

static int tsu6712_resume(struct i2c_client *client)
{
	int value;
	struct tsu6712_usbsw *usbsw = i2c_get_clientdata(client);

/* add for tsu6712_irq_thread i2c error during wakeup */
	tsu6712_read_reg(client,TSU6712_REG_INT1,&value);

	/* device detection */
	mutex_lock(&usbsw->mutex);
	tsu6712_detect_dev(usbsw);
	mutex_unlock(&usbsw->mutex);

	return 0;
}

static const struct i2c_device_id tsu6712_id[] = {
	{"tsu6712", 0},
	{}
};

static struct i2c_driver tsu6712_i2c_driver = {
	.driver = {
		.name = "tsu6712",
	},
	.probe = tsu6712_probe,
	.remove = __devexit_p(tsu6712_remove),
	.suspend = tsu6712_suspend,
	.resume = tsu6712_resume,
	.id_table = tsu6712_id,
};

static int __init tsu6712_init(void)
{
	return i2c_add_driver(&tsu6712_i2c_driver);
}
static void __exit tsu6712_exit(void)
{
	i2c_del_driver(&tsu6712_i2c_driver);
}

static void tsu6712_irq_enable(struct irq_data *data)
{
}
static void tsu6712_irq_disable(struct irq_data *data)
{
}
static void tsu6712_irq_lock(struct irq_data *data)
{
}
static void tsu6712_irq_sync_unlock(struct irq_data *data)
{
}

static void tsu6712_init_usb_irq(struct tsu6712_usbsw *data)
{
	int __irq[2] = { IRQPIN_IRQ_BASE + 64 + TPS80032_INT_VBUSS_WKUP,
					 IRQPIN_IRQ_BASE + 64 + TPS80032_INT_VBUS };
	int i;

	data->irq_base = IRQPIN_IRQ_BASE + 64;
	data->irq_chip.name = "tsu6712_irq_usb";
	data->irq_chip.irq_enable = tsu6712_irq_enable;
	data->irq_chip.irq_disable = tsu6712_irq_disable;
	data->irq_chip.irq_bus_lock = tsu6712_irq_lock;
	data->irq_chip.irq_bus_sync_unlock = tsu6712_irq_sync_unlock;

	for(i = 0; i < 2; i++ ) {
		irq_set_chip_data(__irq[i], data);
		irq_set_chip_and_handler(__irq[i], &data->irq_chip, handle_simple_irq);
		irq_set_nested_thread(__irq[i], 1);
#ifdef CONFIG_ARM
		set_irq_flags(__irq[i], IRQF_VALID);
#endif
	}
}
module_init(tsu6712_init);
module_exit(tsu6712_exit);

MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("TSU6712 USB Switch driver");
MODULE_LICENSE("GPL");
