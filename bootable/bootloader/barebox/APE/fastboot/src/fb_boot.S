# fb_boot.s
#
# Copyright (C) 2012 Renesas Mobile Corp.
# All rights reserved.
#
#-------------------------------------------
# Entry point
#-------------------------------------------
.global _start
_start:

	MOV	r0, #0
	MOV	r1, #0
	MOV	r2, #0
	MOV	r3, #0
	MOV	r4, #0
	MOV	r5, #0
	MOV	r6, #0
	MOV	r7, #0
	MOV	r8, #0
	MOV	r9, #0
	MOV	r10, #0
	MOV	r11, #0
	MOV	r12, #0
	MOV	r14, #0

	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 12 (Z---) BTB
	mcr	p15, 0, r0, c1, c0, 0
	
	LDR	r3,_bss_start
	LDR	r4,_bss_end
	MOV	r2,#0
Loop_RAM_Clear:
	CMP	r3,r4
	STRCC	r2,[r3],#4
	BCC	Loop_RAM_Clear

    bl  fb_main


.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word __bss_end

.globl _stack
_stack:
	.word __stack

FASTBOOT_VER:
	.ascii "\n FASTBOOT version : 0.15\n"
