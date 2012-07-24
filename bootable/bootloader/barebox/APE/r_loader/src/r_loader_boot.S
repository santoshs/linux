/* r_loader_boot.s
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

.globl _start
_start:

#-------------------------------------------------
# Entry point
#-------------------------------------------------
.globl _r_loader_Boot
_r_loader_Boot:

# Remap by SYS Boot Address Register
    LDR     r1, SBAR
    LDR     r0, Remap_MERAM
    STR     r0, [r1]

# Disable MMU
#	MRC		p15, 0, r4, c1, c0, 0								/* Read Control register						*/
#	BIC		r4, r4, #0x1										/* Clear M bit									*/
#	ISB	
#	MCR		p15, 0, r4, c1, c0, 0								/* Write Control register (Disable MMU)			*/
#	ISB	

# vector copy
Vector_Copy_Start:
    LDR     r0, Vector_Start_ROM_Ptr
    LDR     r1, Vector_End_ROM_Ptr
    LDR     r2, Vector_Start_Ptr

Vector_Copy:
    LDR     r3, [r0], #4
    STR     r3, [r2], #4
    CMP     r1, r0
    BNE     Vector_Copy


.globl _r_loader_Start
_r_loader_Start:
    LDR     sp, Stack_Ptr_For_Boot      @ The base stack pointer is set

#-------------------------------------------------
# register Initialization
#-------------------------------------------------
    MOV     r0,   #0
    MOV     r1,   #0
    MOV     r2,   #0
    MOV     r3,   #0
    MOV     r4,   #0
    MOV     r5,   #0
    MOV     r6,   #0
    MOV     r7,   #0
    MOV     r8,   #0
    MOV     r9,   #0
    MOV     r10,  #0
    MOV     r11,  #0
    MOV     r12,  #0
    MOV     r14,  #0

#-------------------------------------------------
# Stack Initialization
#-------------------------------------------------
# stack_base could be defined above, or located in a scatter file
    LDR     sp, Stack_Ptr               @ The base stack pointer is set

#-------------------------------------------------
# Clear .bss Section of RAM
#-------------------------------------------------
    LDR     r3, RAM_BSS_Start_Ptr       @ Get Start Address of .bss Section of RAM
    LDR     r4, RAM_BSS_End_Ptr         @ Get End Address of .bss Section of RAM
    MOV     r2, #0                      @ 0 is stored

Loop_RAM_Clear:
    CMP     r3, r4                      @ Compare Address
    STRCC   r2, [r3], #4                @ 0 is stored and The address is advanced by 4Bytes , if it's r3 < r1
    BCC     Loop_RAM_Clear              @ It jumps to Loop_RAM_Clear , if it's r3 < r1

Boot_r_loader_main:
    B       r_loader_main


#-------------------------------------------------
# Symbol definition
#-------------------------------------------------
#--- System memory locations ---------------------
SBAR:
    .word   0xE6180020                  @ SYS Boot Address Register

Remap_MERAM:
    .word   0xE55E0000                  @ MERAM Vector Address

#--- RAM Clear -----------------------------------
Vector_Start_Ptr:                       @ .vector section start address
    .word   __vector_start

Vector_Start_ROM_Ptr:                   @ vector table start address
    .word   __vector_tbl_start

Vector_End_ROM_Ptr:                     @ vector table end address
    .word   __vector_tbl_end

RAM_BSS_Start_Ptr:                      @ .bss section start address
    .word   __bss_start

RAM_BSS_End_Ptr:                        @ .bss section end address
    .word   __bss_end

#--- It is and data copy of initial value --------
ROM_Data_Start_Ptr:                     @ .data section start address when loading it
    .word   __rom_data_start

RAM_Data_Start_Ptr:                     @ .data section start address when executing it
    .word   __data

RAM_Data_End_Ptr:                       @ .data section end address when executing it
    .word   __edata

Stack_Ptr:                              @ .stack section start address of stack
    .word   __stack

Stack_Ptr_For_Boot:                     @ .stack section start address of stack boot
    .word   __stack_boot

#------------
R_LOADER_VER:
    .ascii  "\nRLOADER version : 0.17\n"
