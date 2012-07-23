/*	flashwriter.lds										*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.				*/
/* All rights reserved.									*/
/*														*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)


MEMORY
{
    rom(rx)        : o = 0xED050000, l = 0x00040000
    ram(w)         : o = 0xED090000, l = 0x00040000
    stack(w)       : o = 0xED0D0000, l = 0x000B0000
}

SECTIONS
{
	.text 0xED050000 : {
		flw_boot.o(.text)
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

	__stack = 0xED0D0000;
}
