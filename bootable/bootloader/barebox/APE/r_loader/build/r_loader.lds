/*
 * r_loader.lds
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

/*----              Memory map              ----*/
MEMORY
{

    rom(rx)        : o = 0xED050000, l = 0x00030000
    ram(w)         : o = 0xED080000, l = 0x00020000
    stack(w)       : o = 0xED0A0000, l = 0x00020000
    stack_boot(w)  : o = 0xED0C0000, l = 0x00000100
    vectors(rx)    : o = 0xED0C0100, l = 0x00001000
}

SECTIONS
{
	.text 0xED050000 : {
		r_loader_boot.o
		__vector_tbl_start = . ;
		r_loader_vectors.o(.text)
		r_loader_vectors.o(.rodata)
		__vector_tbl_end = . ;
		*(.text)
		__rom_data_start = . ;
		*(.rodata)
		__etext = . ;
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

	__stack      = 0xED0C0000 ;
	__stack_boot = 0xED0C0100 ;

	.vector : {
		__vector_start = . ;
		r_loader_vectors.o
		*(.isr_vector)
		__vector_end = . ;
	} > vectors
}
