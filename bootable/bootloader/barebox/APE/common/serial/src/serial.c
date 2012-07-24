/* serial.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
 
#include "com_type.h"
#include "serial.h"

/* Base register */
#define SCIF_BASE 			(0xE6450000ul)	/* SCIFA0 */

#define GPIO_BASE			(0xE6050000ul)

/* SCIFA0 for DEBUG */
#define GPIO_PORT128CR		((volatile uchar *)(GPIO_BASE + 0x0080))	/* SCIFA0_RTS */
#define GPIO_PORT129CR		((volatile uchar *)(GPIO_BASE + 0x0081))	/* SCIFA0_CTS */

#define SCSMR				((volatile ushort *)(SCIF_BASE + 0x0))
#define SCBRR				((volatile uchar  *)(SCIF_BASE + 0x4))
#define SCSCR				((volatile ushort *)(SCIF_BASE + 0x8))
#define SCFCR				((volatile ushort *)(SCIF_BASE + 0x18))

# define SCFSR				((volatile ushort *)(SCIF_BASE + 0x14))	/* SCSSR */
# define SCFTDR				((volatile uchar  *)(SCIF_BASE + 0x20))

#define SCR_RE		(1 << 4)
#define SCR_TE		(1 << 5)
#define FCR_RFRST	(1 << 1)
#define FCR_TFRST	(1 << 2)
#define FSR_DR		(1 << 0)
#define FSR_RDF		(1 << 1)
#define FSR_FER		(1 << 3)
#define FSR_BRK		(1 << 4)
#define FSR_FER		(1 << 3)
#define FSR_TEND	(1 << 6)
#define FSR_ER		(1 << 7)

void uart_init_port(void);

/*
 *
 */
void uart_init_port()
{
	unsigned short dr;
	/* Reset */
	*SCSCR = 0;

	*SCSCR = (SCR_RE | SCR_TE); /* 0x0030 */
	
	*SCSMR = 0;
	*SCSMR = 0;
	
	*SCFCR = (FCR_RFRST | FCR_TFRST); /* 0x0006 */
	
	dr = *SCFCR;
	
	*SCFCR = 0;
}

/*
 * uart_putc -  Put a character to the serial port
 */
void uart_putc(const char c)
{
	/* Tx fifo is empty */
	while (((*SCFSR) & FSR_TEND)==0)
	{
	}
	*SCFTDR = c;
	*SCFSR &= ~FSR_TEND;

}

/*
 * serial_init - Initialize uart
 */
void serial_init()
{
	/* SCIFA0 */
	*GPIO_PORT128CR = 0x11;	/* output */
	*GPIO_PORT129CR = 0xE1;	/* input */
	
	uart_init_port();
	/* Set baudrate */
	*SCBRR = 0x19;	/* 115200 */

}
