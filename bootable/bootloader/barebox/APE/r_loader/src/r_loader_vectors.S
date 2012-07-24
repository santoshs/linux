/*
 * r_loader_vectors.s
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */


    .globl _r_loader_Start

    #=================================================================
    #=== Exception Vectors                                         ===
    #=================================================================

    # Note: LDR PC instructions are used here, though branch (B) instructions
    # could also be used, unless the ROM is at an address >32MB.

    LDR     pc, =Reset_Handler
    LDR     pc, =Undefined_Handler
    LDR     pc, =SWI_Handler
    LDR     pc, =Prefetch_Handler
    LDR     pc, =Abort_Handler
    NOP                                 @ Reserved vector
    LDR     pc, =IRQ_Handler
    LDR     pc, =FIQ_Handler


    #=================================================================
    #=== Stop here                                                 ===
    #=================================================================

    .macro StopHere
1:  nop
    B       1b
    .endm


    #=================================================================
    #=== Exception Handlers                                        ===
    #=================================================================

Reset_Handler:
    LDR     pc, =_r_loader_Start

    # The following dummy handlers do not do anything useful in this example.
    # They are set up here for completeness.

Undefined_Handler:
    LDR     pc, =Default_Interrupt_Handler

SWI_Handler:
    LDR     pc, =Default_Interrupt_Handler

Prefetch_Handler:
    LDR     pc, =Default_Interrupt_Handler

Abort_Handler:
    LDR     pc, =Default_Interrupt_Handler

IRQ_Handler:
    LDR     pc, =Default_Interrupt_Handler

FIQ_Handler:
    LDR     pc, =Default_Interrupt_Handler

Default_Interrupt_Handler:
    # Interrupt_Handler Down.
    StopHere


