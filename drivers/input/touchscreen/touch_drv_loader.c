#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <mach/r8a73734.h>
#ifdef CONFIG_PMIC_INTERFACE
#include <linux/pmic/pmic.h>
#endif

#define I2C_M_RD 1
#define MXT_FAMILY_ID 0x00

#include <linux/rwsem.h>

struct i2c_devinfo {
	struct list_head	list;
	int			busnum;
	struct i2c_board_info	board_info;
};

/* board_lock protects board_list and first_dynamic_bus_num.
 * only i2c core components are allowed to use these symbols.
 */
extern struct rw_semaphore	__i2c_board_lock;
extern struct list_head	__i2c_board_list;

static struct proc_dir_entry *proc_touch_entry;
static char touch_identify[20];
static char name[10][20];
static struct i2c_adapter *i2c_adapter;

static int read_reg(int clientadd, u16 reg, u16 len, void *val)
{
	struct i2c_msg xfer[2];
	u8 buf[2];


	buf[0] = reg & 0xff;
	buf[1] = (reg >> 8) & 0xff;

	/* Write register */
	xfer[0].addr = clientadd;
	xfer[0].flags = 0;
	xfer[0].len = 2;
	xfer[0].buf = buf;

	/* Read data */
	xfer[1].addr = clientadd;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = len;
	xfer[1].buf = val;

	if (i2c_transfer(i2c_adapter, xfer, 2) != 2) {
		printk(KERN_ERR "dyna_ts:%s: i2c transfer failed\n", __func__);
		return -EIO;
	}

	return 0;
}

#ifdef CONFIG_PMIC_INTERFACE
static int panel_power(int on)
{
	if (on) {
		gpio_direction_output(GPIO_PORT30, 1);
		return  pmic_set_power_on(E_POWER_VANA_MM);
	} else {
		gpio_direction_output(GPIO_PORT30, 0);
		return  pmic_set_power_off(E_POWER_VANA_MM);
	}
}
#endif

static int dyna_ts_read_reg(int clientaddr, u16 reg, u8 *val)
{
	return read_reg(clientaddr, reg, 1, val);
}

/* added for version Read procfs callback*/
static int read_version(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	count = sprintf(page, "%s", touch_identify);
	return count;
}
/* end */

static int __init dynats_init(void)
{
	unsigned char family_id, val;
	int error;
	struct i2c_devinfo	*devinfo;
	unsigned short addr[10];
	unsigned short int num = 0, i;
	u8 buf_w[1];

	i2c_adapter = i2c_get_adapter(4);

	if (!i2c_check_functionality(i2c_adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "dyna_ts:ERROR %s I2C check failed!!!\n");
		return -EIO;
	}

	down_read(&__i2c_board_lock);
	printk(KERN_INFO "dyna_ts: i2c board lock is taken\n");
	/* Iterate all devices registered in i2c channel 4 and
	 * find the touch devices from them */
	list_for_each_entry(devinfo, &__i2c_board_list, list) {
		if (devinfo->busnum == i2c_adapter->nr) {
			printk(KERN_INFO "dyna_ts: device = %s\n",
					devinfo->board_info.type);
			if (!strcmp(devinfo->board_info.type +
				(strlen(devinfo->board_info.type) - 3),
				"_ts")) {
				printk(KERN_INFO "dyna_ts: ts_devicename %s\n",
					devinfo->board_info.type);
				/* Save the touch device's slave address,name*/
				addr[num] = devinfo->board_info.addr;
				strcpy(name[num], devinfo->board_info.type);
				num++;
			}
		}
	}
	up_read(&__i2c_board_lock);

	/* Power on the touch panel device before i2c ping */
#ifdef CONFIG_PMIC_INTERFACE
	/* This is under the assumption that powerup sequence is
	* common for all touch devices */
	panel_power(0);
	msleep(100);
	panel_power(1);
	msleep(100);
	/* No need to power down the device because
	 * all touch drivers are not handling power at probe time */
#endif

	for (i = 0; i < num; i++) {
		/* Try to read reg value from touch device address
		 * to verify the device connection */
		if (!(dyna_ts_read_reg(addr[i], MXT_FAMILY_ID, &val))) {
			strcpy(touch_identify, name[i]);
			strcat(touch_identify, ".ko");
			break;
		}
	}

	if (i == num)
		strcat(touch_identify, "NONE");

	proc_touch_entry = create_proc_entry("dynamic_touch_drv", 0400, NULL);
	if (!proc_touch_entry) {
		printk(KERN_ERR "Error creating proc entry");
		return -1;
	}
	proc_touch_entry->read_proc = (read_proc_t *)read_version;
	return 0;

}

static void __exit dynats_exit(void)
{
	remove_proc_entry("dynamic_touch_drv", NULL);
}

module_init(dynats_init);
module_exit(dynats_exit);
/* Module information */
MODULE_AUTHOR("<Author>test@test.com");
MODULE_DESCRIPTION("Touch driver dynamic detect");
MODULE_LICENSE("GPL");