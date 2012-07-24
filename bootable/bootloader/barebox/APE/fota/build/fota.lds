/*	fota.lds										*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.				*/
/* All rights reserved.									*/
/*														*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)


MEMORY
{
    rom(rx)		: o = 0x41000000, l = 0x0000FFFF
    ram(rwx)	: o = 0x40020000, l = 0x0001FFFF
    stack(rwx)	: o = 0x40000000, l = 0x0001FFFF
}

SECTIONS
{
	.text 0x41000000 : {
		fota.o(.text)
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

	__stack = 0x40000000;
}
