#ifndef __TPU_PWM_BOARD_H
#define __TPU_PWM_BOARD_H

#ifdef __KERNEL__

/* Enumerating value of TPU modules. */
enum tpu_modules {
	TPU_MODULE_0,  /* This value indicate TPU0 */
	TPU_MODULE_MAX /*< This value is supremum numbers */
};

/*  Enumerating value of TPU channels */
enum tpu_channels {
	TPU_CHANNEL_0,  /* This value indicate Channel0 */
	TPU_CHANNEL_1,  /* This value indicate Channel1 */
	TPU_CHANNEL_2,  /* This value indicate Channel2 */
	TPU_CHANNEL_3,  /* This value indicate Channel3 */
	TPU_CHANNEL_MAX /* This value is supremum numbers */
};

/* GPIO Settings - PULMD (Pull OFF/Pull DOWN/Pull UP) */
#define PORTn_CR_PULL_NOT_SET -1
#define PORTn_CR_PULL_OFF 0
#define PORTn_CR_PULL_DOWN 1
#define PORTn_CR_PULL_UP 2

/* GPIO Settings - IE/OE */
#define PORTn_CR_DIRECTION_NOT_SET (-1)
#define PORTn_CR_DIRECTION_NONE 0
#define PORTn_CR_DIRECTION_OUTPUT 1
#define PORTn_CR_DIRECTION_INPUT 2

/* GPIO Settings - Output data level High/Low */
#define PORTn_OUTPUT_LEVEL_NOT_SET (-1)
#define PORTn_OUTPUT_LEVEL_LOW 0
#define PORTn_OUTPUT_LEVEL_HIGH 1

struct portn_gpio_setting_tpu {
	u32	port_fn;	/* Pin function select*/
	s32	pull;		/* Pull Off/Down/Up */
	s32 direction;  /* Input/Output direction */
	s32 output_level; /* It become enable only when direction is output. */
};

struct portn_gpio_setting_info_tpu {
	u32	flag;	/* 0 or 1 no change or change required on suspend*/
	u32	port;		/* port number */
	struct portn_gpio_setting_tpu active; /* to be retained on resume */
	struct portn_gpio_setting_tpu inactive; /*settings on suspend state*/
};
/* Port information */
struct port_info {
	const int port_func;
	const char *func_name;
	/* GPIO setting */
	u32	port_count ;
	struct portn_gpio_setting_info_tpu *tpu_gpio_setting_info;
};

#endif /* __KERNEL__ */

#endif /* __TPU_PWM_BOARD_H */
