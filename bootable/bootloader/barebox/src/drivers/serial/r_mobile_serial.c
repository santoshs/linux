/*
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * R-Mobile SCIF device driver.
 * Copyright (c) 2007,2008 Nobuhiro Iwamatsu
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

#include <common.h>
#include <driver.h>
#include <init.h>
#include <errno.h>
#include <malloc.h>
#include <asm/io.h>
#include <mach/hardware-base.h>	/* addresses base */

/* Base register */
// #define SCIF_BASE	SCIF2_BASE
#define SCIF_BASE	SCIF0_BASE

#define	CONFIG_BAUDRATE	115200
#define	CONFIG_SYS_CLK_FREQ 48000000

#define SCSMR	(volatile unsigned short *)(SCIF_BASE + 0x0)
#define SCBRR	(volatile unsigned char  *)(SCIF_BASE + 0x4)
#define SCSCR	(volatile unsigned short *)(SCIF_BASE + 0x8)
#define SCFCR	(volatile unsigned short *)(SCIF_BASE + 0x18)
#define SCFDR	(volatile unsigned short *)(SCIF_BASE + 0x1C)

#define SCFSR	(volatile unsigned short *)(SCIF_BASE + 0x14)	/* SCSSR */
#define SCFTDR	(volatile unsigned char *)(SCIF_BASE + 0x20)
#define SCFRDR	(volatile unsigned char *)(SCIF_BASE + 0x24)

#define SCLSR		SCFSR
#define LSR_ORER	0x0200
#define FIFOLEVEL_MASK	0x1F

#define FSR_ERR_CLEAR	0x0063
#define RDRF_CLEAR		0x00fc

#define SCR_RE		(1 << 4)
#define SCR_TE		(1 << 5)
#define FCR_RFRST	(1 << 1)	/* RFCL */
#define FCR_TFRST	(1 << 2)	/* TFCL */
#define FSR_DR		(1 << 0)
#define FSR_RDF		(1 << 1)
#define FSR_FER		(1 << 3)
#define FSR_BRK		(1 << 4)
#define FSR_FER		(1 << 3)
#define FSR_TEND	(1 << 6)
#define FSR_ER		(1 << 7)

/* CPG */
#define MPCKCR			((volatile ulong *)(0xE6150080))
#define SMSTPCR2		((volatile ulong *)(0xE6150138))

/* Sets the serial clock frequency for transmission and reception */
#define SCBRR_VALUE(bps, clk) (((clk * 2) + 16 * bps) / (32 * bps) - 1)

/*
 * @brief Set the baudrate for the uart port
 *
 * @param[in] cdev  console device
 * @param[in] baud_rate baud rate to set
 *
 * @return  0-implied to support the baudrate
 */
static int r_mobile_serial_setbaudrate(struct console_device *cdev, int baud_rate)
{
	writeb(SCBRR_VALUE(CONFIG_BAUDRATE, CONFIG_SYS_CLK_FREQ), SCBRR);
	return 0;
}

/*
 * handle_error()
 * Input		:None
 * Output		:None
 * Return		:None
 */
void handle_error(void)
{
	readw(SCFSR);
	writew(FSR_ERR_CLEAR, SCFSR);
	readw(SCLSR);
	writew(0x00, SCLSR);
}

/*
 * r_mobile_serial_getc_check()
 * Input		:None
 * Output		:None
 * Return		:None
 */
int r_mobile_serial_getc_check(void)
{
	unsigned short status;

	status = readw(SCFSR);

	if (status & (FSR_FER | FSR_ER | FSR_BRK)){
		handle_error();
	}
	if (readw(SCLSR) & LSR_ORER){
		handle_error();
	}
	return status & (FSR_DR | FSR_RDF);
}

/**
 * @brief Initialize the device
 *
 * @param[in] cdev pointer to console device
 */
static void r_mobile_serial_init_port(struct console_device *cdev)
{
	writew((SCR_RE | SCR_TE), SCSCR);
	writew(0, SCSMR);
	writew(0, SCSMR);
	writew((FCR_RFRST | FCR_TFRST), SCFCR);
	readw(SCFCR);
	writew(0, SCFCR);
}

/*
 * serial_raw_putc()
 * Input		:None
 * Output		:None
 * Return		:None
 */
void serial_raw_putc(char c)
{
	unsigned int fsr_bits_to_clear;
	while (1) {
		if (readw(SCFSR) & FSR_TEND) { /* Tx fifo is empty */
			fsr_bits_to_clear = FSR_TEND;
			break;
		}
	}

	writeb(c, SCFTDR);
	if (fsr_bits_to_clear != 0){
		writew(readw(SCFSR) & ~fsr_bits_to_clear, SCFSR);
	}
}
/*
 * @brief Put a character to the serial port
 *
 * @param[in] cdev pointer to console device
 * @param[in] c character to put
 */
static void r_mobile_serial_putc(struct console_device *cdev, char c)
{
	if (c == '\n'){
		serial_raw_putc('\r');
	}
	serial_raw_putc(c);
}
/*
 * @brief Retrieve a character from serial port
 *
 * @param[in] cdev pointer to console device
 *
 * @return return the character read
 */
static int r_mobile_serial_getc(struct console_device *cdev)
{
	unsigned short status;
	char ch;

	while (1)
	{
		if(r_mobile_serial_getc_check()){
			break;
		}
	}
	

	ch = readb(SCFRDR);
	status = readw(SCFSR);
	
	writew(RDRF_CLEAR, SCFSR);

	if (status & (FSR_FER | FSR_FER | FSR_ER | FSR_BRK)){
		handle_error();
	}

	if (readw(SCLSR) & LSR_ORER){
		handle_error();
	}
	
	return ch;
}

/*
 * serial_rx_fifo_level()
 * Input		:None
 * Output		:None
 * Return		:None
 */
static int serial_rx_fifo_level(void)
{
	return ((readw(SCFDR) >> 0) & FIFOLEVEL_MASK);
}
/**
 * @brief Test if character is available
 * @param[in] cdev pointer to console device
 * @return  - status based on data availability
 */
static int r_mobile_serial_tstc(struct console_device *cdev)
{
	return serial_rx_fifo_level() ? 1 : 0;
}
/*
 * @brief Probe entry point -called on the first match for device
 *
 * @param[in] dev matched device
 *
 * @return EINVAL if platform_data is not populated,
 *	   ENOMEM if calloc failed
 *	   else return result of console_register
 */
static int r_mobile_serial_probe(struct device_d *dev)
{
	struct console_device *cdev;
	cdev = xzalloc(sizeof(*cdev));
	dev->type_data = cdev;
	cdev->dev = dev;
	cdev->f_caps = CONSOLE_STDIN | CONSOLE_STDOUT | CONSOLE_STDERR;
	cdev->tstc = r_mobile_serial_tstc;
	cdev->putc = r_mobile_serial_putc;
	cdev->getc = r_mobile_serial_getc;
	cdev->setbrg = r_mobile_serial_setbaudrate;

	r_mobile_serial_init_port(cdev);
		
	return console_register(cdev);
}
/*
 * @brief Driver registration structure
 */
static struct driver_d r_mobile_serial_driver = {
	.name = "r_mobile_serial",
	.probe = r_mobile_serial_probe,
};

/*
 * @brief driver initialization function
 *
 * @return result of register_driver
 */
static int r_mobile_serial_init(void)
{
	return register_driver(&r_mobile_serial_driver);
}
console_initcall(r_mobile_serial_init);
