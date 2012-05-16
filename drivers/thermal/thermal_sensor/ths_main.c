/*
 * Thermal Sensor Driver
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <mach/r8a73734.h>
#include <mach/pm.h>


#include "ths_user.h"
#include "ths_main.h"
#include "ths_hardware.h"

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#define RESCNT			 IO_ADDRESS(0xE618801C)	/* Reset control register */
#define TRESV			 0x00000008				/* Temperature sensor reset enable bit */

struct thermal_sensor *ths;
int suspend_state = FALSE;

/* Define the functions of Temperature control part */
static void ths_enable_reset_signal(void);
static void ths_initialize_hardware(void);
static int ths_suspend(struct device *dev);
static int ths_resume(struct device *dev);
static int ths_initialize_platform_data(struct platform_device *pdev);
static int ths_initialize_resource(struct platform_device *pdev);
static irqreturn_t ths_isr(int irq, void *dev_id);
static void ths_work_tj0(struct work_struct *work);
static void ths_work_tj1(struct work_struct *work);
static void ths_work_tj2(struct work_struct *work);
static int ths_start_module(struct platform_device *pdev);
static void ths_stop_module(struct platform_device *pdev);
static int __devinit ths_probe(struct platform_device *pdev);
static int __devexit ths_remove(struct platform_device *pdev);
static int __init ths_init(void);
static void __exit ths_exit(void);

/* Implement the functions of Temperature control part */

/*
 * __ths_get_cur_temp: get the current temperature of LSI
 *  @ths_id  : index of Thermal Sensor device (THS0 or THS1)
 * 	@cur_temp: current temperature of LSI 
 * return: 
 * 		-EINVAL (-22): invalid argument
 *		-ENXIO   (-6): Thermal Sensor device is IDLE state
 *		-EACCES (-13): Permission denied due to driver in suspend state
 *		0			 : Get current temperature successfully
 */
 
int __ths_get_cur_temp(unsigned int ths_id, int *cur_temp)
{
	int ret = 0;
	u32 ctemp = 0; /* Store value of CTEMP */
	
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	if (TRUE == suspend_state) {
		dev_err(&ths->pdev->dev, "Error! Thermal Sensor driver is suspended.\n");
		ret = -EACCES;
		goto ths_error;
	}
	
	if ((ths_id != 0) && (ths_id != 1)) {
		dev_err(&ths->pdev->dev, "Error! Index of Thermal Sensor is invalid.\n");
		ret = -EINVAL;
		goto ths_error;
	}
	
	if (E_IDLE == ths->pdata[ths_id].current_mode) {
		dev_err(&ths->pdev->dev, "Error! Thermal Sensor device %d is Idle.\n", ths_id);
		ret = -ENXIO;
		goto ths_error;
	}
	
	ctemp     = get_register_32(THSSR(ths_id)) & CTEMP_MASK;
	*cur_temp = ctemp * 5 - 65; 
	
	THS_DEBUG_MSG("%s end (Normal case) <<<\n", __func__);
	
	return 0;
ths_error:
	
	THS_DEBUG_MSG("%s end (Error case) <<<\n", __func__);
	
	return ret;
}

/*
 * __ths_set_op_mode: set operation mode of Thermal Sensor device
 *  @ths_mode: operation mode of Thermal Sensor device
 *  @ths_id  : index of Thermal Sensor device (THS0 or THS1)
 * return: 
 *		0			 : set new operation mode successfully
 *		-EINVAL	(-22): invalid argument
  *		-EACCES (-13): Permission denied
 */
 
int __ths_set_op_mode(enum mode ths_mode, unsigned int ths_id)
{
	int ret = 0;
	
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	if (TRUE == suspend_state) {
		dev_err(&ths->pdev->dev, "Error! Thermal Sensor driver is suspended.\n");
		ret = -EACCES;
		goto ths_error;
	}
	
	if ((ths_id != 0) && (ths_id != 1)) {
		dev_err(&ths->pdev->dev, "Error! Index of Thermal Sensor is invalid.\n");
		ret = -EINVAL;
		goto ths_error;
	}
	
	if(ths_mode == ths->pdata[ths_id].current_mode) {
		/* Same as current current mode, do nothing */
		return 0;
	}
	
	switch (ths_mode) {
	case E_NORMAL_1:	/* Set THS#ths_id to Normal 1 operation */
		modify_register_32(THSCR(ths_id), 0, THIDLE1 | THIDLE0);
		break;	
	case E_NORMAL_2:	/* Set THS#ths_id to Normal 2 operation */
		modify_register_32(THSCR(ths_id), THIDLE1, THIDLE0);
		break;
	case E_IDLE:		/* Set THS#ths_id to Idle operation */
		modify_register_32(THSCR(ths_id), THIDLE1 | THIDLE0, 0);
		break;
	default:
		dev_err(&ths->pdev->dev, "Thermal Sensor operation mode is invalid.\n");
		ret = -EINVAL;
		goto ths_error;
	}
	
	mutex_lock(&ths->sensor_mutex);
	
	ths->pdata[ths_id].current_mode = (int)ths_mode;
	
	mutex_unlock(&ths->sensor_mutex);
	
ths_error:

	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
	return ret;
}

/*
 * __ths_get_op_mode: get the current operation mode of Thermal Sensor device.
 *  	@ths_id: index of Thermal Sensor device (THS0 or THS1)
 * return: 
 *		E_NORMAL_1 (0): is equivalent to NORMAL 1 mode.
 *		E_NORMAL_2 (1): is equivalent to NORMAL 2 mode.
 *		E_IDLE     (2): is equivalent to IDLE mode
 *		-EINVAL  (-22): invalid argument (ths_id is different from 0 and 1)
  *		-EACCES  (-13): Permission denied
 */

int __ths_get_op_mode(unsigned int ths_id)
{	
	int ret = 0;

	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	if (TRUE == suspend_state) {
		dev_err(&ths->pdev->dev, "Error! Thermal Sensor driver is suspended.\n");
		ret = -EACCES;
		goto ths_error;
	}
	
	if ((ths_id != 0) && (ths_id != 1)) {
		dev_err(&ths->pdev->dev, "Error! Index of Thermal Sensor is invalid.\n");
		ret = -EINVAL;
		goto ths_error;
	}
	
	mutex_lock(&ths->sensor_mutex);
	
	ret = ths->pdata[ths_id].current_mode;
	
	mutex_unlock(&ths->sensor_mutex);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
ths_error:

	return ret;
}

/*
 * ths_enable_reset_signal: enable reset request
 * return: 
 *		None
 */
 
static void ths_enable_reset_signal(void)
{
	unsigned int value;
	
	/* Enable reset signal */
	value  = __raw_readl(RESCNT);
	value &= ~TRESV;
	value |= TRESV;
	__raw_writel(value, RESCNT);

}

/*
 * ths_initialize_hardware: initialize the operation for Thermal Sensor devices
 * return: 
 *		None
 */
 
static void ths_initialize_hardware(void)
{	
	int cur_temp = 0;

	THS_DEBUG_MSG(">>> %s start\n", __func__);

	/* Disable chattering restraint function */
	set_register_32(FILONOFF0_RW_32B, FILONOFF_CHATTERING_DI);
	set_register_32(FILONOFF1_RW_32B, FILONOFF_CHATTERING_DI);
	
	/* 
	 * Set detection mode for both THSs: 
	 *	+ Tj1, Tj2, Tj3 are rising
	 *	+ Tj0 is falling 
	 */
	set_register_32(POSNEG0_RW_32B, POSNEG_DETECTION);
	set_register_32(POSNEG1_RW_32B, POSNEG_DETECTION);
	
	/* Clear Interrupt Status register */
	set_register_32(STR_RW_32B, TJST_ALL_CLEAR);
	
	/* 
	 * Set operation mode for THS0/THS1: Normal 1 mode
	 * and TSC decides a value of CPTAP automatically
	 */
	modify_register_32(THSCR0_RW_32B, CPCTL, THIDLE1 | THIDLE0);
	modify_register_32(THSCR1_RW_32B, CPCTL, THIDLE1 | THIDLE0);
	
	/* Enable all interrupts in THS0 and only INTDT3 in THS1 */
	set_register_32(ENR_RW_32B, TJ13_EN | TJ03_EN | TJ02_EN | TJ01_EN | TJ00_EN);
	
	ths_enable_reset_signal();
	
	/* Set threshold interrupts INTDT3/2/1/0 interrupts for both THS0/1 */
	set_register_32(INTCTLR0_RW_32B, CTEMP3_HEX | CTEMP2_HEX | CTEMP1_HEX | CTEMP0_HEX);	
	
	/* Un-mask to output reset signal when Tj > Tj3 for both THS0/1 */
	modify_register_32(PORTRST_MASK_RW_32B, TJ13PORT_MSK | TJ03PORT_MSK, TJ13RST_MSK | TJ03RST_MSK);
	
	/* Wait for THS operating */
	udelay(300);	/* 300us */
	
	/* Get current temperature here to judge which Tj will be monitored (Tj0/1/2) */
	__ths_get_cur_temp(0, &cur_temp);
	
	THS_DEBUG_MSG("cur_temp:%d \n", cur_temp);
	
	if (cur_temp < CTEMP0_DEC) {
		/* Mask Tj3, Tj2, Tj0; Un-mask Tj1 to monitor Tj1 */
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK | TJ02INT_MSK | TJ00INT_MSK, TJ01INT_MSK);
	} else if ((CTEMP0_DEC <= cur_temp) && (cur_temp < CTEMP1_DEC)) {
		/* Mask Tj3, Tj2; Un-mask Tj1, Tj0 to monitor Tj1, Tj0 */
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK | TJ02INT_MSK, TJ00INT_MSK | TJ01INT_MSK);
	} else if (cur_temp >= CTEMP1_DEC) {
		/* Mask Tj3; Un-mask Tj2, Tj1, Tj0 to monitor Tj2, Tj1, Tj0 */
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK, TJ00INT_MSK | TJ01INT_MSK | TJ02INT_MSK);
	} else if (cur_temp >= CTEMP2_DEC) {
		/* Mask Tj3, Tj1; Un-mask Tj0, Tj2 to monitor Tj0, Tj2 */
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK | TJ01INT_MSK, TJ00INT_MSK | TJ02INT_MSK);
	}
	
	/* Mask Tj0/1/2/3 in THS1 to not output them to INTC */
	modify_register_32(INT_MASK_RW_32B, TJ13INT_MSK | TJ12INT_MSK | TJ11INT_MSK | TJ10INT_MSK, 0);
	
	set_register_32(STR_RW_32B, TJST_ALL_CLEAR);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
}

/*
 * ths_suspend: change the current state of Thermal Sensor device to Mono-Active state
 *  @dev: a struct device
 * return: 0: successful suspend
 */
 
static int ths_suspend(struct device *dev)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	/* Update the last mode */
	ths->pdata[0].last_mode = ths->pdata[0].current_mode;
	ths->pdata[1].last_mode = ths->pdata[1].current_mode;

	if (ths->pdata[0].current_mode != E_IDLE) {
		__ths_set_op_mode(E_IDLE, 0);
	}	
	
	if (ths->pdata[1].current_mode != E_IDLE) {
		__ths_set_op_mode(E_IDLE, 1);
	}
	
	suspend_state = TRUE;

	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
	return 0;
}

/*
 * ths_resume: change the state of Thermal Sensor device to its state as before system suspends
 *  @dev: a struct device
 * return: successful resume
 */
 
static int ths_resume(struct device *dev)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	suspend_state = FALSE;
	
	__ths_set_op_mode((enum mode)ths->pdata[0].last_mode, 0);
	__ths_set_op_mode((enum mode)ths->pdata[1].last_mode, 1);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
	return 0;
}

/*
 * ths_initialize_platform_data: initialize Thermal Sensor platform data
 *  @pdev: a struct platform_device
 * return: 
 *		0            : Initialize successfully
 *		-EINVAL(-22) : No such device or address
 *		-ENOMEM (-12): Can't get clock
 */
 
static int ths_initialize_platform_data(struct platform_device *pdev)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);

	if (!pdev->dev.platform_data) {
		dev_err(&pdev->dev, "Error! No platform data defined\n");
		return -EINVAL;
	}
	
	ths = kzalloc(sizeof(struct thermal_sensor), GFP_KERNEL);
	
	if (NULL == ths) {
		dev_err(&pdev->dev, "Error! Failed to allocate driver data\n");
		return -ENOMEM;
	}
	
	platform_set_drvdata(pdev, ths);
	ths->pdev = pdev;
	memcpy(ths->pdata, pdev->dev.platform_data, 2 * sizeof(struct thermal_sensor_data));
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
	return 0;
}

/*
 * ths_initialize_resource: Get base address of Thermal Sensor module then remap I/O memory. 
 *  @pdev: a struct platform_device
 * return: 
 *		0          : Initialize successfully
 *		-ENXIO (-6): No such device or address
 */
 
static int ths_initialize_resource(struct platform_device *pdev)
{	
	struct resource *rs = NULL;
	
	THS_DEBUG_MSG(">>> %s start\n", __func__);

	rs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (NULL == rs) {
		dev_err(&pdev->dev, "Error! Failed to get I/O memory for Thermal Sensor module\n");
		return -ENXIO;
	}
	
	ths->iomem_base = ioremap_nocache(rs->start, resource_size(rs));
	if (NULL == ths->iomem_base) {
		dev_err(&pdev->dev, "Error! Failed to remap I/O memory for Thermal Sensor module\n");
		return -ENXIO;
	}
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
	return 0;
}


/*
 * ths_isr: Interrupt handler in case of INTDT2/1/0 interrupts 
 *  @irq  : Irq number for Thermal Sensor module
 *  @dev_id: ID of device registered
 * return: 
 *		0     		    : Register successfully
 *		Other error code: Register unsuccessfully
 */
static irqreturn_t ths_isr(int irq, void *dev_id)
{
	int intr_status = -1;

	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	disable_irq_nosync(irq);
	
	intr_status = get_register_32(STR_RW_32B);
		
	if (TJ02ST == (intr_status & TJ02ST)) { /* INTDT2 interrupt occurs */
		/* Un-mask INTDT0 interrupt to output it to INTC(only THS0)] */
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK | TJ02INT_MSK | TJ01INT_MSK, TJ00INT_MSK);
		queue_work(ths->queue, &ths->tj2_work); 
	} else if (TJ01ST == (intr_status & TJ01ST)) { /* INTDT1 interrupt occurs */
		/* Un-mask INTDT0 and INTDT2 interrupts to output them to INTC (only THS0)] */		
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK | TJ01INT_MSK, TJ00INT_MSK | TJ02INT_MSK);
		queue_work(ths->queue, &ths->tj1_work); 
	} else if ( TJ00ST == (intr_status & TJ00ST)) { /* INTDT0 interrupt occurs */
		/* Un-mask INTDT1 interrupt to output it to INTC(only THS0)] */
		modify_register_32(INT_MASK_RW_32B, TJ03INT_MSK | TJ02INT_MSK | TJ00INT_MSK, TJ01INT_MSK);
		queue_work(ths->queue, &ths->tj0_work); 
	} 
	
	/* Clear Interrupt Status register */
	set_register_32(STR_RW_32B, TJST_ALL_CLEAR);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
	return IRQ_HANDLED;
}

/*
 * ths_work_tj0: configure DFS driver to limit the maximum frequency
 *  in OVERDRIVE(option) - MAX - MID - LOW - Very LOW frequency 
 *  (1456.0[MHz] - 1196.0[MHz] - 598.0[MHz] - 299.0[MHz] - 199.3[MHz])
 *  in case of CTEMP <= CTEMP0 
 *  @work: a struct work_struct
 * return: 
 *		None
 */
 
static void ths_work_tj0(struct work_struct *work)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
#ifdef CONFIG_SHMOBILE_CPUFREQ
	limit_max_cpufreq(LIMIT_NONE);
#else
	THS_DEBUG_MSG("%s DFS is disabled <<<\n", __func__);
#endif /* CONFIG_SHMOBILE_CPUFREQ */
	
	enable_irq(ths->ths_irq);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
}

/*
 * ths_work_tj1: configure DFS driver to limit the maximum frequency
 *  in MID - LOW - Very LOW frequency 
 *  in case of CTEMP > CTEMP1 
 *  @work: a struct work_struct
 * return: 
 *		None
 */

static void ths_work_tj1(struct work_struct *work)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);

#ifdef CONFIG_SHMOBILE_CPUFREQ
	limit_max_cpufreq(LIMIT_MID);
#else
	THS_DEBUG_MSG("%s DFS is disabled <<<\n", __func__);
#endif /* CONFIG_SHMOBILE_CPUFREQ */

	enable_irq(ths->ths_irq);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
}

/*
 * ths_work_tj2: configure DFS driver to limit the maximum frequency
 *  in LOW - Very LOW frequency
 *  in case of CTEMP > CTEMP2 
 *  @work: a struct work_struct
 * return: 
 *		None
 */

static void ths_work_tj2(struct work_struct *work)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);

#ifdef CONFIG_SHMOBILE_CPUFREQ
	limit_max_cpufreq(LIMIT_LOW);
#else
	THS_DEBUG_MSG("%s DFS is disabled <<<\n", __func__);
#endif /* CONFIG_SHMOBILE_CPUFREQ */

	enable_irq(ths->ths_irq);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	
}

/*
 * ths_start_module: Activate Thermal sensor module
 * return: 
 *		-EINVAL (-22): Invalid clock
 *		Other: Other errors
 */
static int ths_start_module(struct platform_device *pdev)
{
	int ret = 0;
	char clk_name[50];

	THS_DEBUG_MSG(">>> %s start\n", __func__);
	snprintf(clk_name, sizeof(clk_name), "thermal_sensor.%d", pdev->id);
	ths->clk = clk_get(&pdev->dev, clk_name);
	if (IS_ERR(ths->clk)) {
		ret = PTR_ERR(ths->clk);
		dev_err(&pdev->dev, "Error! Failed to get Thermal Sensor clock\n");
		goto error;
	}
	
	ret = clk_enable(ths->clk);
	if(ret < 0) {
		ret = -EINVAL;
		dev_err(&pdev->dev, "Error! Failed to enable Thermal Sensor clock\n");
	}

error:
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	return ret;
}

/*
 * ths_stop_module: Stop Thermal sensor module
 * return: 
 *		None
 */
static void ths_stop_module(struct platform_device *pdev)
{
	THS_DEBUG_MSG(">>> %s start\n", __func__);

	clk_disable(ths->clk);
	clk_put(ths->clk);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
}

/*
 * ths_probe: Register Thermal sensor platform driver
 *  @pdev: a struct platform_device
 * return: 
 *		0     		    : Register successfully
 *		Other error code: Register unsuccessfully
 */
 
static int __devinit ths_probe(struct platform_device *pdev)
{
	int ret = -1;
	
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	/* Register miscdevice (initialize user interface) */
	ret = misc_register(&ths_user);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error! Can't register miscdevice (user interface)\n");
		goto error_0;
	}
	
	/* Get default Thermal Sensor driver data: operation mode */
	ret = ths_initialize_platform_data(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error! Can't initialize platform data\n");
		goto error_1;
	}
	
	/* Get default driver resource: mapping memory */
	ret = ths_initialize_resource(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error! Can't initialize resource\n");
		goto error_2;
	}
	
	/* Initialize mutex */
	mutex_init(&ths->sensor_mutex);
	
	/* Get irq and bind interrupt to isr */
	ths->ths_irq = platform_get_irq(pdev, 0);
	if (ths->ths_irq < 0) {
		ret = ths->ths_irq;
		dev_err(&pdev->dev, "Error! Failed to get irq\n");
		goto error_3;
	}	
	ret = request_irq(ths->ths_irq, ths_isr, 0, pdev->name, pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error! Failed to request IRQ\n");
		goto error_3;
	}
	
	/* Register callback function for a work to processing when Tj > Tj2/Tj1 or Tj < Tj0 */
	INIT_WORK(&ths->tj0_work, ths_work_tj0);
	INIT_WORK(&ths->tj1_work, ths_work_tj1);
	INIT_WORK(&ths->tj2_work, ths_work_tj2);
	
	/* Create a new queue for bottom-half processing */
	ths->queue = create_singlethread_workqueue("ths_queue");
	
	/* Activate Thermal Sensor module */
	ret = ths_start_module(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error! Can not start Thermal Sensor module\n");
		goto error_3;
	}

	/* Initialize the operation of Thermal Sensor device */
	ths_initialize_hardware();
	
	THS_DEBUG_MSG("%s end (Normal case) <<<\n", __func__);
	return 0;
error_3:
	mutex_destroy(&ths->sensor_mutex);
	iounmap(ths->iomem_base);
error_2:
	platform_set_drvdata(pdev, NULL);
	kfree(ths);
error_1:
	misc_deregister(&ths_user);
error_0:
	THS_DEBUG_MSG("%s end (Error case) <<<\n", __func__);
	
	return ret;
}

/*
 * ths_remove: Deregister Thermal sensor platform driver
 *  @pdev: a struct platform_device
 * return: 
 *		0     		    : Deregister successfully
 *		Other error code: Deregister unsuccessfully
 */
 
static int __devexit ths_remove(struct platform_device *pdev)
{	
	THS_DEBUG_MSG(">>> %s start\n", __func__);
	
	misc_deregister(&ths_user);
	iounmap(ths->iomem_base);
	free_irq(platform_get_irq(pdev, 0), pdev);
	platform_set_drvdata(pdev, NULL);
	mutex_destroy(&ths->sensor_mutex);
	kfree(ths);
	ths_stop_module(pdev);
	
	THS_DEBUG_MSG("%s end <<<\n", __func__);
	return 0;
}

static const struct dev_pm_ops ths_dev_pm_ops = {
	.suspend = ths_suspend,
	.resume  = ths_resume,
};

static struct platform_driver ths_driver = {
	.probe  = ths_probe,
	.remove = __devexit_p(ths_remove),
	.driver = {
		.name = "thermal_sensor",
		.pm   = &ths_dev_pm_ops,
	}
};

/*
 * ths_init: Register Thermal sensor module
 *  @ths_driver: Thermal sensor platform driver
 * return: 
 *		0     		    : Register successfully
 *		Other error code: Register unsuccessfully
 */
 
static int __init ths_init(void)
{
	return platform_driver_register(&ths_driver);
}

/*
 * ths_exit: Deregister Thermal sensor module
 *  @ths_driver: Thermal sensor platform driver
 * return: 
 *		None
 */
 
static void __exit ths_exit(void)
{
	platform_driver_unregister(&ths_driver);
}


module_init(ths_init);
module_exit(ths_exit);

MODULE_AUTHOR("Renesas Mobile Corporation");
MODULE_DESCRIPTION("THERMAL SENSOR DRIVER");
MODULE_LICENSE("GPL");
