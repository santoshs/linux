/*	fastboot.lds										*/
/*														*/
/* Copyright (C) 2012 Renesas Mobile Corp.				*/
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
	.text 0xED0C1300: 
	{
		fb_boot.o
		*(.text)
		 *(.rodata)
	   	 _etext = . ; 
	} > rom

	.tors : {
		___ctors = . ;
		*(.ctors)
		___ctors_end = . ;
		___dtors = . ;
		*(.dtors)
		___dtors_end = . ;
	}  > rom

	.data : AT ( ADDR(.tors) + SIZEOF(.tors) ){
		___data = . ;
		*(.data)
		*(.tiny)
		 _edata = .;
	} > rom

	.bss : AT ( LOADADDR(.data) + SIZEOF(.data) ) {
		 __bss_start = . ;
		*(.bss)
		*(COMMON)
		 __bss_end = . ;  
	}  >ram

	.stack : {
		 __stack = . ; 
		*(.stack)
	}  > stack
}

