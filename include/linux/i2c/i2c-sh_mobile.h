#ifndef __I2C_SH_MOBILE_H__
#define __I2C_SH_MOBILE_H__

#include <linux/platform_device.h>

struct i2c_sh_mobile_port_info {
	unsigned int port_num;
	unsigned int port_func;
};

struct i2c_sh_mobile_platform_data {
	unsigned long	bus_speed;
	bool		pin_multi;
	unsigned char bus_data_delay;
	struct i2c_sh_mobile_port_info	scl_info;
	struct i2c_sh_mobile_port_info	sda_info;
	unsigned int clks_per_count;
};

extern unsigned int u2_get_board_rev();
#endif /* __I2C_SH_MOBILE_H__ */
