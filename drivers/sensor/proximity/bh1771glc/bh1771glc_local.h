/******************************************************************************
 * MODULE     : bh1771glc_local.h
 * FUNCTION   : BH1771 header of Proximity Sensor and Ambient Light Sensor IC
 *****************************************************************************/
#ifndef _BH1771GLC_LOCAL_H__
#define _BH1771GLC_LOCAL_H__

#define BH1771_DRIVER_VER  ("1.0.0")
#define SM_TIME_UNIT  (1000)
#define MN_TIME_UNIT  (1000000)
#define CLR_LOW2BIT   (0xFC)
#define BH1771_SETTING_MAX	15

/************************************************
*   	Debug definition section	 	*
************************************************/ 
#define BH1771GLC_LOG
#ifdef BH1771GLC_LOG
	#define bh1771glc_log(fmt, ... ) printk(KERN_DEBUG "%s(%d): " fmt, __func__, __LINE__, ## __VA_ARGS__)
#else 
	#define bh1771glc_log(x...) do { } while (0) 
#endif

/******************************************************************************
 * FUNCTION   : BH1771 interface header of 
			    Proximity Sensor and Ambient Light Sensor IC
 *****************************************************************************/

/************ definition to dependent on sensor IC ************/
#define BH1771_I2C_NAME "bh1771glc"
#define BH1771_I2C_ADDRESS (0x38) //7 bits slave address 011 1000
#define I2C_RETRIES 5
#define I2C_M_WR	0
#define PROX_IRQ 47
#define MAX_DELAY_TIME 2000
#define MIN_DELAY_TIME 10
#define BH1771_I2C_CHANNEL 2

/************ define register for IC ************/
/* BH1771 REGSTER */
#define REG_ALSCONTROL     (0x40)
#define REG_PSCONTROL      (0x41)
#define REG_ILED           (0x42)
#define REG_ILED3           (0x43)
#define REG_ALSPSMEAS      (0x44)
#define REG_PSMEASRATE     (0x45)
#define REG_ALSMEASRATE    (0x46)
#define REG_ALSDATA        (0x4C)
    #define REG_ALSDATA_L      (0x4C)
    #define REG_ALSDATA_H      (0x4D)
#define REG_ALSPSSTATUS    (0x4E)
#define REG_PSDATA         (0x4F)
#define REG_INTERRUPT      (0x52)
#define REG_PSTH_H_L1         (0x53)
#define REG_PSTH_H_L2         (0x54)
#define REG_PSTH_H_L3         (0x55)
#define REG_ALSTHUP        (0x56)
    #define REG_ALSTHUP_L      (0x56)
    #define REG_ALSTHUP_H      (0x57)
#define REG_ALSTHLOW       (0x58)
    #define REG_ALSTHLOW_L     (0x58)
    #define REG_ALSTHLOW_H     (0x59)
#define REG_ALSSENSITIVITY (0x5A)
#define REG_PERSISTENCE    (0x5B)
#define REG_PSTH_L_L1         (0x5C)
#define REG_PSTH_L_L2         (0x5D)
#define REG_PSTH_L_L3         (0x5E)

/* structure of peculiarity to use by system */
struct bh1771_data {
    struct i2c_client *client;
    struct mutex lock;
    struct wake_lock wakelock;
	struct input_dev *input_dev;
	
    atomic_t available;
     	
    struct work_struct als_work;
    struct hrtimer als_timer;
    atomic_t als_power_flg;
    atomic_t als_enable;
    u16  als_delay;
    u16  als_poll_interval;
	u16 	 als_ill_per_count; 	// illuminant per 1 count

	struct work_struct ps_work_poll;
	struct work_struct ps_work_irq;
	struct hrtimer ps_timer;
    int irq;
	atomic_t ps_power_flg;
    atomic_t ps_enable;
	u16  ps_delay;
    u16  ps_poll_interval;
};




struct bh1771glc_output_rate {
	//Output data rate in millisecond
	int poll_rate_ms;
	
	//The corresponding output data rate in bh1771glc register
	u8 mask;
};


/************ define parameter for register ************/
/* REG_ALSCONTROL(0x40) */
#define ALSRES_HMODE   0
#define ALSRES_MMODE   1
#define SWRST_NON      0
#define SWRST_EXCUTE   1
#define CTL_STANDBY    0
#define CTL_FORCE      2
#define CTL_STANDALONE 3
#define REG_ALSCTL_MAX 16

/* REG_PSCONTROL(0x41) */
#define REG_PSCTL_MAX (0x4)

/* REG_ILED(0x42) */
#define LEDCURRENT_5		(0)
#define LEDCURRENT_10		(1)
#define LEDCURRENT_20		(2)
#define LEDCURRENT_50		(3)
#define LEDCURRENT_100		(4)
#define LEDCURRENT_150		(5)
#define LEDCURRENT_200		(6)
#define REG_ILED_MAX		(7)
#define REG_ILED_DEF		(0x18)

/* REG_PSMEASRATE(0x45) */
#define PSRATE_10		(0)
#define PSRATE_20		(1)
#define PSRATE_30		(2)
#define PSRATE_50		(3)
#define PSRATE_70		(4)
#define PSRATE_100		(5)
#define PSRATE_200		(6)
#define PSRATE_500		(7)
#define PSRATE_1000		(8)
#define PSRATE_2000		(9)
#define REG_PSMEASRATE_MAX (16)

/* REG_ALSMEASRATE(0x46) */
#define ALSRATE_DISABLE    1
#define ALSRATE_ENABLE     0
#define ALSRATE_100		(0)
#define ALSRATE_200     (1)
#define ALSRATE_500     (2)
#define ALSRATE_1000	(3)
#define ALSRATE_2000	(4)
#define ALSRATE_MASK	(0x87)

/* REG_INTERRUPT(0x52) */
#define PSH_THLETH_ONLY    0
#define PSHL_THLETH_BOTH   1
#define OUTPUT_ANYTIME     0
#define OUTPUT_LUTCH       1
#define POLA_ACTIVEL       0
#define POLA_INACTIVEL     1
#define MODE_NONUSE        (0)
#define MODE_PROXIMITY     (1)
#define MODE_ILLUMINANCE   (2)
#define MODE_BOTH          (3)
#define REG_INTERRUPT_MAX  (0x20)
#define INTR_VAL_MAKS      (0x1B)

/* REG_PERSISTENCE (0x5B) */
#define PERSISTENCE_MAX (0x10)
#define ALS_PERSIS_SFT  (4)

#define PS_LED1 0
#define PS_LED2 1
#define PS_LED3 2
#define PS_LED_ALL 3

#define PS_LED_MODE1 0
#define PS_LED_MODE2 1
#define PS_LED_MODE3 2
#define PS_LED_MODE4 3

#define EVENT_TYPE_PROXIMITY ABS_DISTANCE
#define EVENT_TYPE_LIGHT ABS_MISC
#define ABS_MAX_VAL_ALS 65535
#define ABS_MAX_VAL_PS 255

#endif /* _BH1771GLC_LOCAL_H__ */
