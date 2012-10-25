#include <linux/serial_core.h>

extern void bcm_bt_lpm_exit_lpm_locked(struct uart_port *uport);

int __init bcm4334_bluetooth_init(void);

