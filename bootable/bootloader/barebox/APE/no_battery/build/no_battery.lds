/*	no_battery.lds										*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.				*/
/* All rights reserved.									*/
/*														*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)


MEMORY
{
    rom(rx)		: o = 0xED0C1300, l = 0x00020000
    ram(rwx)	: o = 0xED0E1300, l = 0x00010000
    stack(rwx)	: o = 0xED0F1300, l = 0x00020000
}

SECTIONS
{
	.text 0xED0C1300 : {
		no_battery.o(.text)
		*(.text)
		 *(.rodata)
	   	 _etext = . ;
	} > rom

	.tors : {
		__ctors = . ;
		*(.ctors)
		__ctors_end = . ;
		__dtors = . ;
		*(.dtors)
		__dtors_end = . ;
	}  > rom

	.data : AT ( ADDR(.tors) + SIZEOF(.tors) ) {
		__data = . ;
		*(.data)
		*(.tiny)
		__edata = . ;
	} > rom

	.bss : AT ( LOADADDR(.data) + SIZEOF(.data) ) {
		__bss_start = . ;
		*(.bss)
		*(COMMON)
		__bss_end = . ;
	} > ram

	__stack = 0xED111300;
}
