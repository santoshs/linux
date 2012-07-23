/*	ramset_loader.lds										*/
/*														*/
/* Copyright (C) 2012, Renesas Mobile Corp.				*/
/* All rights reserved.									*/
/*														*/

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)


MEMORY
{
    rom(rx)		: o = 0xE63A0000, l = 0x00001800
    ram(rwx)	: o = 0xE63A1800, l = 0x00000C00
    stack(rwx)	: o = 0xE63A2400, l = 0x00000400
}

SECTIONS
{
	.text 0xE63A0000 : {
		ramset_entry.o(.text)
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

	__stack = 0xE63A2800;
}
