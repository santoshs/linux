/*
 * /drivers/misc/pcm2pwm/pcm2pwm.c
 *
 * Copyright (C) 2011-2012 Renesas Mobile Corporation
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<linux/slab.h>
#include	<linux/module.h>
#include	<linux/delay.h>
#include	<linux/wakelock.h>
#include	<linux/pm_runtime.h>
#include	<linux/platform_device.h>
#include	<linux/err.h>
#include	<linux/clk.h>
#include	<linux/io.h>
#include	<linux/ioport.h>
#include	<linux/gpio.h>
#include	<linux/fs.h>
#include	<mach/sh73a0.h>
#include	<linux/sh_dma.h>
#include	<linux/dmaengine.h>
#include	<linux/pcm2pwm.h>

/* function prototype */
static int __init pcm2pwm_pf_init(void);
static void __exit pcm2pwm_pf_exit(void);
static int __devexit pcm2pwm_pf_remove(struct platform_device *pdev);
static int pcm2pwm_pf_probe(struct platform_device *pdev);
static int pcm2pwm_pf_resume(struct platform_device *pdev);
static int pcm2pwm_pf_suspend(struct platform_device *pdev, pm_message_t state);

static void dma_complete_cb(void *param);
static void dma_request_release(void);
static int dma_request_start(const void *buf, unsigned int buf_size);
static void work_dma_callback(struct work_struct *work);
/* Enumurate */
enum pcm2pwm_state {
	PCM2PWM_CLOSE,
	PCM2PWM_OPEN,
	PCM2PWM_STOP,
	PCM2PWM_START
};

enum pcm2pwm_dma_state {
	DMA_STOP,
	DMA_TRANSMIT
};

/* Structure */
static struct platform_driver pcm2pwm_platform_driver = {
	.probe		= pcm2pwm_pf_probe,
	.remove		= __devexit_p(pcm2pwm_pf_remove),
	.suspend	= pcm2pwm_pf_suspend,
	.resume		= pcm2pwm_pf_resume,
	.driver		= {
		.name	= "pcm2pwm-renesas-sh_mobile",
		.owner	= THIS_MODULE,
	},
};

struct pcm2pwm_device{
	struct platform_device	*pdev;
	struct clk				*extal2_clk;
	struct clk				*clk_gen;
	u16						*base_address;
	enum pcm2pwm_state		state;
};

struct pcm2pwm_dma_device {
	struct dma_chan *tx_chan;
	struct scatterlist *tx_sg;
	unsigned int tx_sg_len;
	int dma_state;
};

/* Macro */
#define UN_SUSPEND 0
#define SUSPEND    1

#define PCM2PWM_CTL(base)	(*(base) + 0x00)	/* address of PWM_CONTROL register */
#define PCM2PWM_DMA(base)	(*(base) + 0x02)	/* address of PWM_DMA  register */
#define PCM2PWM_CNT(base)	(*(base) + 0x04)	/* address of PWM_COUNT register */
#define PCM2PWM_FIFO(base)	(*(base) + 0x80)	/* address of PWM_PCM_FIFO buffer */

/* Variable */
static struct pcm2pwm_device pcm2pwm_platdevice = {
	.pdev = NULL,
	.extal2_clk = NULL,
	.clk_gen = NULL,
	.base_address = NULL,
	.state = PCM2PWM_CLOSE
};

struct pcm2pwm_dma_device pcm2pwm_dma_dev = {
	.tx_chan = NULL,
	.tx_sg = NULL,
	.tx_sg_len = 0,
	.dma_state = DMA_STOP
};

static struct wake_lock pcm2pwm_wakelock;
static int pcm2pwm_suspend_state = UN_SUSPEND;
static struct mutex pcm2pwm_mutex;
static struct work_struct dma_work;

/*
 * pcm2pwm_pf_probe:
 * @pdev: pointer to structure platform_device
 * return:
 *        0 if success.
 */
static int pcm2pwm_pf_probe(struct platform_device *pdev)
{
	pcm2pwm_platdevice.pdev = pdev;
	pm_runtime_enable(&pdev->dev);
	return 0;
}

/*
 * pcm2pwm_pf_exit: Unregister TPU driver
 * return: void
 */
static void __exit pcm2pwm_pf_exit(void)
{
	platform_driver_unregister(&pcm2pwm_platform_driver);
}

/*
 * pcm2pwm_pf_remove: This function is called when the device
 *						is removed from the system
 * @pdev	: a pointer of struct platform_device
 * return: 0
 */
static int __devexit pcm2pwm_pf_remove(struct platform_device *pdev)
{
	cancel_work_sync(&dma_work);
	pm_runtime_disable(&pdev->dev);
	return 0;
}

/*
 * pcm2pwm_pf_resume: Call-back function of platform driver
 *						in case of moving to resume state
 * @pdev:
 * return: 0
 */
static int pcm2pwm_pf_resume(struct platform_device *pdev)
{
	mutex_lock(&pcm2pwm_mutex); /* lock to prevent multiple thread */
	pcm2pwm_suspend_state = UN_SUSPEND;
	mutex_unlock(&pcm2pwm_mutex);
	return 0;
}

/*
 * pcm2pwm_pf_suspend: Call-back function of platform driver
 *						in case of moving to low-power state
 * @pdev
 * @state
 * return: 0
 */
static int pcm2pwm_pf_suspend(struct platform_device *pdev, pm_message_t state)
{
	mutex_lock(&pcm2pwm_mutex); /* lock to prevent multiple thread*/
	pcm2pwm_suspend_state = SUSPEND;
	cancel_work_sync(&dma_work);
	mutex_unlock(&pcm2pwm_mutex);
	return 0;
}

/*
 * pcm2pwm_open: Make PCM2PWM IP ready for using
 * return:
 *        0 if success, otherwise return negative value
 */
extern int pcm2pwm_open(void)
{
	struct platform_device *pdev;
	int ret;

	if (pcm2pwm_suspend_state == SUSPEND) { /* in suspend state */
		return  -EBUSY;
	}

	if (PCM2PWM_CLOSE != pcm2pwm_platdevice.state) {
		/* device already open */
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_open] device is openning\n");
		return -EBUSY;
	}

	pdev = pcm2pwm_platdevice.pdev;

	wake_lock(&pcm2pwm_wakelock); /*prevent suspend state */

	ret = pm_runtime_get_sync(&pdev->dev);
	if (ret) {
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_open] get PM sync unsuccessfully\n");
		return ret;
	}

	/* request clock extal2 */
	pcm2pwm_platdevice.extal2_clk = clk_get(NULL, "extal2");
	if (IS_ERR(pcm2pwm_platdevice.extal2_clk)) {
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_open] clk_get for extal2 unsuccessfully\n");
		return -EINVAL;
	}

	/* enable extal2 clock */
	ret = clk_enable(pcm2pwm_platdevice.extal2_clk);
	if (ret) {
		printk(KERN_ERR "[PCM2PWM ERR pcm2pwm_open] enable clock extal2 unsuccessfully\n");
		return ret;
	}

	/* request clkgen */
	pcm2pwm_platdevice.clk_gen = clk_get(NULL, "clkgen");
	if (IS_ERR(pcm2pwm_platdevice.clk_gen)) {
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_open] clk_get for clkgen unsuccessfully\n");
		return -EINVAL;
	}

	/* enable clkgen clock */
	ret = clk_enable(pcm2pwm_platdevice.clk_gen);
	if (ret) {
		printk(KERN_ERR "[PCM2PWM ERR pcm2pwm_open] enable clock clkgen unsuccessfully\n");
		return ret;
	}

	/* map I/O memory */
	if (!request_mem_region(pdev->resource->start,
							resource_size(pdev->resource),
							pdev->resource->name)) {
		printk(KERN_ERR "[PWM ERR pcm2pwm_open] the mapped IO memory is in use\n");
		return -EBUSY;
	}

	pcm2pwm_platdevice.base_address = (u16 *)ioremap_nocache(pdev->resource->start, resource_size(pdev->resource));

	pcm2pwm_platdevice.state = PCM2PWM_OPEN;

	return ret;
}
EXPORT_SYMBOL(pcm2pwm_open);

/*
 * pcm2pwm_close: Stop supplying power, clock.
 * return:
 *        0 if success, otherwise return negative value
 */
extern int pcm2pwm_close(void)
{
	struct platform_device *pdev;
	int ret;

	if (pcm2pwm_suspend_state == SUSPEND) { /* in suspend state */
		return -EBUSY;
	}

	if (PCM2PWM_CLOSE == pcm2pwm_platdevice.state) {
		/* device already close */
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_close] device is closing\n");
		return -EINVAL;
	}

	pdev = pcm2pwm_platdevice.pdev;

	/* unmap I/O memory */
	iounmap(pcm2pwm_platdevice.base_address);
	release_mem_region(pdev->resource->start, resource_size(pdev->resource));

	pcm2pwm_platdevice.base_address = NULL;

	/* disable extal2 clock */
	if (pcm2pwm_platdevice.extal2_clk) {
		clk_disable(pcm2pwm_platdevice.extal2_clk);
		clk_put(pcm2pwm_platdevice.extal2_clk);
		pcm2pwm_platdevice.extal2_clk = NULL;
	}

	/* disable clkgen clock */
	if (pcm2pwm_platdevice.clk_gen) {
		clk_disable(pcm2pwm_platdevice.clk_gen);
		clk_put(pcm2pwm_platdevice.clk_gen);
		pcm2pwm_platdevice.clk_gen = NULL;
	}

	/* Disable A4MP */
	ret = pm_runtime_put_sync(&pdev->dev);
	if (ret) {
		printk(KERN_ERR "[PWM ERR - handle_tpu_close] put PM sync unsuccessfully\n");
		return ret;
	}

	pcm2pwm_platdevice.state = PCM2PWM_CLOSE;

	wake_unlock(&pcm2pwm_wakelock);
	return ret;
}
EXPORT_SYMBOL(pcm2pwm_close);

/*
 * pcm2pwm_enable: Start/Stop outputting PWM signal from PCM2PWM
 * @state		: Parameter to control ON/OFF PWM signal
 * @src			: Address of PCM data
 * @data_sz		: Size of PCM data
 * @cnt			: Counter set for PCM2PWM IP
 * return:
 *        0 if success, otherwise return negative value
 */
extern int pcm2pwm_enable(enum pcm2pwm_request_state state,
							const void *src, unsigned int data_sz, u16 cnt)
{
	int ret;
	u16 *base_address = NULL;
	u16 value;

	if (pcm2pwm_suspend_state == SUSPEND) { /* in suspend state */
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_enable] in suspend state\n");
		return -EBUSY;
	}

	if ((src == NULL) || (data_sz < 0) || (cnt > 2048)) {
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_enable] invalid argument\n");
		return -EINVAL;
	}

	if (PCM2PWM_CLOSE == pcm2pwm_platdevice.state) {
		/* device has not open yet*/
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_enable] device is not open\n");
		return -EINVAL;
	}

	base_address = pcm2pwm_platdevice.base_address;
	if (STOP_PCM2PWM == state) {
		if (PCM2PWM_STOP == pcm2pwm_platdevice.state) {
			return 0; /* stopping */
		}
		/* stop DMA in case PCM2PWM_START */
		dma_request_release();

		/* clear bit 0 of PWM_CTL */
		value = __raw_readw(PCM2PWM_CTL(base_address));
		value = value & 0xFFFE;
		__raw_writew(value, PCM2PWM_CTL(base_address));

		pcm2pwm_platdevice.state = PCM2PWM_STOP;
		return 0;
	}

	/* update cnt */
	if ((PCM2PWM_START == pcm2pwm_platdevice.state)) {
		/* need to stop DMA */
		/* Update PWM_COUNT */
		__raw_writew(cnt, PCM2PWM_CNT(base_address));
		return 0;
	}

	/* In state PCM2PWM_STOP */

	/* Step 1: DMA enable and DMA request size setting
			Note: Issue a DMA transfer request when FIFO has empty area of 16 word or more
	*/
	value  = __raw_readw(PCM2PWM_DMA(base_address));
	value = value & 0xFFCE; /* clear bit 0, 4, 5 */
	value = value | 0x0011; /* set bit 0 and bit 4 */
	__raw_writew(value, PCM2PWM_DMA(base_address));

	/* Step 2: PWM_COUNT setting */
	__raw_writew(cnt, PCM2PWM_CNT(base_address));

	/* use drm driver to start transfer */
	ret = dma_request_start(src, data_sz);
	if (ret) {
		printk(KERN_ERR "[PCM2PWM ERR - pcm2pwm_enable] Can not start DMA transfer\n");
		/* clear bit 0 of PWM_CTL */
		value = __raw_readw(PCM2PWM_CTL(base_address));
		value = value & 0xFFFE;
		__raw_writew(value, PCM2PWM_CTL(base_address));
		return ret;
	}
	pcm2pwm_platdevice.state = PCM2PWM_START;
	return ret;
}
EXPORT_SYMBOL(pcm2pwm_enable);

/*
 * dma_request_start: start DMA transfer
 * @buf			: Address of transfer buffer
 * @buf_size	: Size of transfer buffer
 * return:
 *        0 if success, otherwise return negative value
 */
static int dma_request_start(const void *buf, unsigned int buf_size)
{
	dma_cap_mask_t mask;
	struct dma_chan *chan = pcm2pwm_dma_dev.tx_chan;
	struct dma_async_tx_descriptor *tx_desc;
	dma_cookie_t tx_cookie;
	int nent;

	if (DMA_STOP != pcm2pwm_dma_dev.dma_state) {
		return -EBUSY;
	}

	if ((NULL == buf) || (0 == buf_size)) {
		return -EINVAL;
	}

	/* Initialize bit-mask & set bit-mask */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	/* Request a exclusive channel */
	chan = dma_request_channel(mask, NULL, NULL);
	if (chan) {
		sg_init_table(pcm2pwm_dma_dev.tx_sg, 1);
		sg_set_page(pcm2pwm_dma_dev.tx_sg, virt_to_page(buf),
					buf_size, offset_in_page((int)buf));
		nent = dma_map_sg(&pcm2pwm_platdevice.pdev->dev,
							pcm2pwm_dma_dev.tx_sg, 1, DMA_FROM_DEVICE);
		if (!nent) {
			dma_request_release();
			printk(KERN_ERR "[PCM2PWM ERR - dma_request] cannot map a scatter/gather DMA operation\n");
			return -1;
		}
		pcm2pwm_dma_dev.tx_sg_len = nent;
	} else {
		printk(KERN_ERR "[PCM2PWM ERR - dma_request] dma_request_channel is NULL\n");
		return -1;
	}

	/* Prepare DMA transfer information */
	tx_desc = chan->device->device_prep_slave_sg(chan, pcm2pwm_dma_dev.tx_sg,
												pcm2pwm_dma_dev.tx_sg_len,
												DMA_FROM_DEVICE,
												DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (NULL == tx_desc) {
		dma_request_release();
		printk(KERN_ERR "[PCM2PWM ERR - dma_request] Error in prepare DMA transfer");
		return -1;
	}

	/* Set DMA Call back */
	tx_desc->callback = dma_complete_cb;
	tx_desc->callback_param = NULL;

	/* Submit descriptor for transfering */
	tx_cookie = tx_desc->tx_submit(tx_desc);
	if (tx_cookie < 0) {
		dma_request_release();
		return -1;
	}

	/* Start DMA transfering */
	dma_async_issue_pending(chan);

	/* Set DMA_TRANSMIT to DMA state */
	pcm2pwm_dma_dev.dma_state = DMA_TRANSMIT;

	return 0;
}

/*
 * dma_request_release: release DMA channel
 * return: None
 */
static void dma_request_release(void)
{
	if (DMA_STOP == pcm2pwm_dma_dev.dma_state) {
		return;
	}
	mutex_lock(&pcm2pwm_mutex);
	if (pcm2pwm_dma_dev.tx_chan) {
		dma_release_channel(pcm2pwm_dma_dev.tx_chan);
		pcm2pwm_dma_dev.tx_sg_len = 0;
	}

	pcm2pwm_dma_dev.dma_state = DMA_STOP;
	mutex_unlock(&pcm2pwm_mutex);
}


/*
 * dma_complete_cb: handle callback of DMA when transfer is finished
					(Re-send the buffer set by dma_request_start)
 * return: none
 */
static void dma_complete_cb(void *param)
{
	schedule_work(&dma_work);
}

/*
 * work_dma_callback:
 * return: None
 */
static void work_dma_callback(struct work_struct *work)
{
	struct dma_chan *chan = pcm2pwm_dma_dev.tx_chan;
	struct dma_async_tx_descriptor	*tx_desc;
	dma_cookie_t tx_cookie;

	mutex_lock(&pcm2pwm_mutex);
	if (DMA_STOP == pcm2pwm_dma_dev.dma_state) {
		printk(KERN_ERR "[PCM2PWM ERR - dma_complete_cb] DMA state is DMA_STOP\n");
		return;
	}

	if (!pcm2pwm_dma_dev.tx_chan || !pcm2pwm_dma_dev.tx_sg
		|| 0 == pcm2pwm_dma_dev.tx_sg_len) {
		printk(KERN_ERR "[PCM2PWM ERR - dma_complete_cb] tx_chan, tx_sg is NULL and tx_sg_len is 0\n");
		return;
	}

	/* Prepare DMA transfer information */
	tx_desc = chan->device->device_prep_slave_sg(chan,
												pcm2pwm_dma_dev.tx_sg,
												pcm2pwm_dma_dev.tx_sg_len,
												DMA_FROM_DEVICE,
												DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
		if (NULL == tx_desc) {
			dma_request_release();
			printk(KERN_ERR "[PCM2PWM ERR - dma_complete_cb] Error in preparing DMA continuous transfer\n");
			return;
		}

	/* Set DMA Call back */
	tx_desc->callback = dma_complete_cb;
	tx_desc->callback_param = NULL;

	/* Submit descriptor for transfering */
	tx_cookie = tx_desc->tx_submit(tx_desc);
	if (tx_cookie < 0) {
		dma_request_release();
		printk(KERN_ERR "[PCM2PWM ERR - dma_complete_cb] Error in summitting DMA continuous transfer\n");
		return;
	}

	/* Start DMA transfering */
	dma_async_issue_pending(chan);
	mutex_unlock(&pcm2pwm_mutex);
}

/*
 * pcm2pwm_pf_init: init pcm2pwm driver
 * return:
 *        0 if success, otherwise return negative value
 */
static int __init pcm2pwm_pf_init(void)
{
	int ret;
	ret = platform_driver_register(&pcm2pwm_platform_driver);
	if (ret) {
		return ret;
	}
	/* wake-lock init */
	wake_lock_init(&pcm2pwm_wakelock, WAKE_LOCK_SUSPEND, "pcm2pwm-wakelock");
	/* mutex init */
	mutex_init(&pcm2pwm_mutex);
	/* init dma work*/
	INIT_WORK(&dma_work, work_dma_callback);

	return 0;
}

module_init(pcm2pwm_pf_init);
module_exit(pcm2pwm_pf_exit);

MODULE_AUTHOR("Renesas Mobile");
MODULE_DESCRIPTION("Driver that controls PCM2PWM IP");
MODULE_LICENSE("GPL v2");
