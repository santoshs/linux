/*	high_temp.s
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

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
	
	LDR	r3,_bss_start
	LDR	r4,_bss_end
	MOV	r2,#0
Loop_RAM_Clear:
	CMP	r3,r4
	STRCC	r2,[r3],#4
	BCC	Loop_RAM_Clear

    bl  high_temp_main


.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word __bss_end

.globl _stack
_stack:
	.word __stack

HIGHTEMP_VER:
	.ascii "\n HIGHTEMP version : 0.13\n"
