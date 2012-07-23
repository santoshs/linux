@ 
@  Copyright © Renesas Mobile Corporation 2010 . All rights reserved     
@ 
@  This material, including documentation and any related source code and information, is protected by 
@  copyright controlled by Renesas. All rights are reserved. Copying, including reproducing, storing, adapting,
@  translating and modifying, including decompiling or reverse engineering, any or all of this material
@  requires the prior written consent of Renesas. This material also contains confidential information, which
@  may not be disclosed to others without the prior written consent of Renesas.                                                              
@

@/*************************************************************************
@ **     Copyright (C) 2010 Nokia Corporation. All rights reserved.      **
@ **                                                                     **
@ ** Permission is hereby granted, free of charge, to any person         **
@ ** obtaining a copy of this software and associated documentation      **
@ ** files (the "Software"), to deal in the Software without             **
@ ** restriction, including without limitation the rights to use, copy,  **
@ ** modify, merge, publish, distribute, sublicense, and/or sell copies  **
@ ** of the Software, and to permit persons to whom the Software is      **
@ ** furnished to do so, subject to the following conditions:            **
@ **                                                                     **
@ ** The above copyright notice and this permission notice shall be      **
@ ** included in all copies or substantial portions of the Software.     **
@ ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,     **
@ ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF  **
@ ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND               **
@ ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS **
@ ** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN  **
@ ** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN   **
@ ** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    **
@ ** SOFTWARE.                                                           **
@ **                                                                     **
@ *************************************************************************
@ **                                                                     **
@ ** File:  hw_arm.s                                                     **
@ **                                                                     **
@ ** Desc:  ARM core specific code                                       **
@ **        NB: ARM 1136 not supported in this implementation            **
@ **                                                                     **
@ *************************************************************************/

.include "arm.inc"

@ Following definition is used for removing any entries from lockdown region 
@ of the TLB, that is all entries in lockdown region will be written to with
@ given value. This definition is used only with ARM926 and ARM1136 based
@ processors

.ifndef TRUSTZONE

.equ tlb_lockdown_address, 0x00010000    @ sec rom

        .extern sec_rom_mmu_l1_table

.endif @ !TRUSTZONE

@=============================================================================
@ void hw_arm_fiq_enable(void)
@      This function enables ARM FIQ interrupt.
@
@=============================================================================
.arm
.globl hw_arm_fiq_enable

hw_arm_fiq_enable:
        MRS     R0, cpsr
        BIC     R0, R0, #FIQ_MASK
        MSR     cpsr_cxsf, R0

        BX      LR

@=============================================================================
@ dword hw_arm_fiq_disable(void)
@      This function disables ARM FIQ interrupt.
@      Returns the old FIQ bit of the CPSR.
@=============================================================================
.arm
.globl hw_arm_fiq_disable

hw_arm_fiq_disable:
        MRS     R1, cpsr
        AND     R0, R1, #FIQ_MASK

        ORR     R1, R1, #FIQ_MASK
        MSR     cpsr_cxsf, R1

        BX      LR

@=============================================================================
@ void hw_arm_irq_enable(void)
@      This function enables ARM IRQ interrupt.
@
@=============================================================================
.arm
.globl hw_arm_irq_enable

hw_arm_irq_enable:
        MRS     R0, cpsr
        BIC     R0, R0, #IRQ_MASK
        MSR     cpsr_cxsf, R0

        BX      LR

@=============================================================================
@ dword hw_arm_irq_disable(void)
@      This function disable ARM IRQ interrupt.
@      Returns the old IRQ bit of the CPSR.
@=============================================================================
.arm
.globl hw_arm_irq_disable

hw_arm_irq_disable:

        MRS     R1, cpsr
        AND     R0, R1, #IRQ_MASK

        ORR     R1, R1, #IRQ_MASK
        MSR     cpsr_cxsf, R1

        BX      LR

@=============================================================================
@ void hw_arm_irq_fiq_enable(void)
@      This function enables ARM IRQ interrupt.
@
@=============================================================================
.arm
.globl hw_arm_fiq_irq_enable

hw_arm_fiq_irq_enable:
        MRS     R0, cpsr
        BIC     R0, R0, #FIQ_IRQ_MASK
        MSR     cpsr_cxsf, R0

        BX      LR

@=============================================================================
@ dword hw_arm_irq_fiq_disable(void)
@      This function disable ARM IRQ interrupt.
@      Returns the old IRQ and FIQ bits of the CPSR.
@=============================================================================
.arm
.globl hw_arm_fiq_irq_disable

hw_arm_fiq_irq_disable:

        MRS     R1, cpsr
        AND     R0, R1, #FIQ_IRQ_MASK

        ORR     R1, R1, #FIQ_IRQ_MASK
        MSR     cpsr_cxsf, R1

        BX      LR

.ifndef TRUSTZONE

@=============================================================================
@ void hw_arm_icache_enable(void)
@      This function enables icache.
@
@=============================================================================
.arm
.globl hw_arm_icache_enable

hw_arm_icache_enable:
        MOV     R1, #0
        MCR     p15,  0, R1, c7, c5,  0 @ write to Cache operations register

        @ Flush entire branch target cache
        MOV     R1, #0
        MCR     p15, 0, R1, c7, c5, 6 @ write to Cache operations register

        @ Unlock all ICache ways
        MRC     p15,  0, R1, c9, c0 , 1 @ read ICache lockdown reg
        BIC     R1, R1, #0x0F           @ reset L bits of all ways
        MCR     p15,  0, R1, c9, c0 , 1 @ write ICache lockdown reg

        @ Enable ICache
        MRC     p15,  0, R1, c1, c0 , 0 @ read control reg
        ORR     R1, R1, #HW_ARM_CP15_CR_I_MASK  @ set ICache enable bit
        MCR     p15,  0, R1, c1, c0 , 0 @ write control reg

        BX      LR
        
@=============================================================================
@ void hw_arm_icache_disable(void)
@      This function disable icache.
@
@=============================================================================
.arm
.globl hw_arm_icache_disable

hw_arm_icache_disable:

        @ Disable ICache
        MRC     p15,  0, R1, c1, c0 , 0 @ read control register
        BIC     R1, R1, #HW_ARM_CP15_CR_I_MASK  @ reset ICache enable bit
        MCR     p15,  0, R1, c1, c0 , 0 @ write control register

        @ Invalidate entire L1 ICache
        MOV     R1, #0
        MCR     p15,  0, R1, c7, c5,  0 @ write to Cache operations register

        @ Flush entire branch target cache
        MOV     R1, #0
        MCR     p15, 0, R1, c7, c5, 6 @ write to Cache operations register

        @ All ICache ways are already unlocked so do not need to unlock them
        @ here

        BX      LR

.endif  @!TRUSTZONE
 

@=============================================================================
@ dword hw_arm_icache_state_get(void)
@
@       This function returns icache state.
@       Return value: 0 = disabled
@                     1 = enabled
@
@=============================================================================

.arm
.globl hw_arm_icache_state_get

hw_arm_icache_state_get:
        @ Check DCache state
        MRC     p15,  0, R0, c1, c0 , 0    @ read control reg
        TST     R0, #HW_ARM_CP15_CR_I_MASK @ test ICache enable bit
        MOVEQ   R0, #0
        MOVNE   R0, #1

        BX      LR

@=============================================================================
@ void hw_arm_icache_invalidate(void)
@      This function invalidates icache.
@
@=============================================================================
.arm
.globl hw_arm_icache_invalidate

hw_arm_icache_invalidate:
        MOV     R0, #0
        MCR     p15, 0, R0, c7, c5, 0   @ Invalidate Entire Instruction Cache

        MCR     p15, 0, R0, c7, c5, 6   @ Flush Entire Branch Target Cache      

        BX LR

@=============================================================================
@ void hw_arm_icache_area_invalidate(void* start, void* end)
@      This function invalidates an area from icache.
@
@=============================================================================
.arm
.globl hw_arm_icache_area_invalidate

hw_arm_icache_area_invalidate:

        @ Ignore call if i-cache is disbled
        MRC     p15, 0, R2, c1, c0, 0   @ Read CP15 control register
        TST     R2, #HW_ARM_CP15_CR_I_MASK  @ ICache enabled?
        BEQ     s295                    @ If not, return

        @ Check that end >= start. Otherwise return.
        CMP     R0, R1
        BHI     s295

        BIC     R0, R0, #0x1F           @ Mask 5 LSBits
s293:
        MCR     p15, 0, R0, c7, c5, 1   @ Invalidate ICache single entry (MVA)
        ADD     R0, R0, #(1<<5)         @ Next cache line
        CMP     R1, R0
        BPL     s293

        MOV     R0, #0
        MCR     p15, 0, R0, c7, c5, 6   @ Flush Entire Branch Target Cache

s295:
        BX LR

@=============================================================================
@ dword hw_arm_dcache_state_get(void)
@
@       This function returns dcache state.
@       Return value: 0 = disabled
@                     1 = enabled
@
@=============================================================================

.arm
.globl hw_arm_dcache_state_get

hw_arm_dcache_state_get:

        @ Check DCache state
        MRC     p15,  0, R0, c1, c0 , 0    @ read control reg
        TST     R0, #HW_ARM_CP15_CR_C_MASK @ test DCache enable bit
        MOVEQ   R0, #0
        MOVNE   R0, #1

        BX      LR

@=============================================================================
@ void hw_arm_dcache_clean(void)
@      This function cleans the entire d-cache.
@
@=============================================================================
.arm
.globl hw_arm_dcache_clean

hw_arm_dcache_clean:
        MOV     R1, #0
        MCR     p15, 0, R1, c7, c10, 0

        BX      LR

@=============================================================================
@ void hw_arm_dcache_area_clean(void* start, void* end)
@      This function cleans an area from d-cache.
@
@=============================================================================
.arm
.globl hw_arm_dcache_area_clean

hw_arm_dcache_area_clean:

        @ Check that end >= start. Otherwise return.
        CMP     R0, R1
        BHI     s395

        BIC     R0, R0, #0x1F           @ Mask 5 LSBits
s393:
        MCR     p15, 0, R0, c7, c10, 1  @ Clean DCache single entry (MVA)
        ADD     R0, R0, #(1<<5)         @ Next cache line
        CMP     R1, R0
        BPL     s393

s395:
        BX LR

@=============================================================================
@ void hw_arm_dcache_invalidate(void)
@      This function invalidates dcache.
@
@=============================================================================
.arm
.globl hw_arm_dcache_invalidate

hw_arm_dcache_invalidate:

        MOV     R1, #0
        MCR     p15,  0, R1, c7, c6,  0

        BX LR



@=============================================================================
@ void hw_arm_dcache_area_invalidate(void* start, void* end)
@      This function invalidates an area from dcache.
@
@=============================================================================
.arm
.globl hw_arm_dcache_area_invalidate

hw_arm_dcache_area_invalidate:

        @ Check that end >= start. Otherwise return.
        CMP     R0, R1
        BHI     s495

        BIC     R0, R0, #0x1F           @ Mask 5 LSBits
s493:
        MCR     p15, 0, R0, c7, c6, 1   @ Invalidate DCache single entry (MVA)
        ADD     R0, R0, #(1<<5)         @ Next cache line
        CMP     R1, R0
        BPL     s393

s495:
        BX LR

@=============================================================================
@ void hw_arm_write_buffer_drain(void)
@
@      This function drains write buffer.
@
@=============================================================================
.arm
.globl hw_arm_write_buffer_drain

hw_arm_write_buffer_drain:

        DSB

        BX LR

.arm
.globl hw_arm_isb

hw_arm_isb:

        ISB

        BX LR

@=============================================================================
@ void hw_arm_dcache_maintenance(uint32_t operation, uint32_t level)
@
@      This function is a wrapper around the function
@      hw_arm_dcache_maintenance_no_stack performing register
@      saving and restoring so that it can be called with
@      normal call convention.
@      For closer description, see hw_arm_dcache_maintenance_no_stack_usage
@
@=============================================================================

.arm
.globl hw_arm_dcache_maintenance

hw_arm_dcache_maintenance:

        STMFD   SP!, {R4-R12,LR} @ R12 just to align SP correctly
        
        BL      hw_arm_dcache_maintenance_no_stack_usage

        LDMIA   SP!, {R4-R12,PC}

@=============================================================================
@ void hw_arm_dcache_maintenance_no_stack_usage(uint32_t operation, uint32_t level)
@
@       This function can be used to perform DCache maintenance
@       operations for entire DCache or entire Dcache and L2 cache.
@
@       Operation can be one of
@       - clean (0)
@       - invalidate (1)
@       - clean&invalidate (2)
@       and level can be one of:
@       - point of unification, meaning L1 Dcache (0)
@       - point of coherency and up, meaning L1 Dcache and L2 cache (1)
@       - point of coherency only, meaning only L2 cache (2)
@
@       This function shall not use any stack so that
@       it can be called when stack is not yet available at boot
@       time. Side effect is that it corrupts contents of quite 
@       many registers so if called from somewhere else, wrapper
@       function hw_arm_dcache_maintenance shall be called instead
@       of direct call to this function.
@
@       Code is modified version of an example code presented in ARMv7 
@       Architecture Introduction document. This code is generic for
@       all ARMv7 processors.
@
@       Note that 'point of coherency and up' and 'point of coherency only'
@       work correctly only in processors implementing L2 maintenance operations
@       via CP15 registers. Otherwise L2 maintenance operations have to be
@       done via L2 control registers.
@
@       Function corrupts registers R0-R11.
@
@=============================================================================

.arm
.globl hw_arm_dcache_maintenance_no_stack_usage


hw_arm_dcache_maintenance_no_stack_usage:

        MRC     p15, 0, R8, c1, c0, 0  @ Disable datacaches with CP15 r1 C bit
        BIC     R3, R8, #HW_ARM_CP15_CR_C_MASK
        MCR     p15, 0, R3, c1, c0, 0
        
        MOV     R6, R0                 @ Move operation type info to R6
        CMP     R6, #2
        BGT     s590
        MRC     p15, 1, R0, c0, c0, 1  @ Read CLIDR
        CMP     R1, #1                 @ Point of coherency and up?
        BEQ     s505
        CMP     R1, #2                 @ Point of coherency only?
        BEQ     s510
		ANDS    R3, R0, #0x38000000     @ Requested level = PoU
		MOV     R3, R3, LSR #26
        BEQ     s590
        MOV     R10, #0
        B       s515
s505:
        ANDS    R3, R0, #0x7000000      @ Requested level = PoC and up
        MOV     R3, R3, LSR #23
        BEQ     s590
        MOV     R10, #0
        B       s515
s510:
        ANDS    R3, R0, #0x7000000      @ Requested level = PoC only
        MOV     R3, R3, LSR #23
        BEQ     s590
        MOV     R10, R3
        B       s515
s515: 
        ADD     R2, R10, R10, LSR #1   @ Work out 3xcachelevel
        MOV     R1, R0, LSR R2         @ Bottom 3 bits are the Ctype for this
        AND     R1, R1, #7             @ level, get those bits alone
        CMP     R1, #2
        BLT     s580                   @ No cache or only ICache at this level
        MCR     p15, 2,R10, c0, c0, 0  @ Write the Cache Size selection reg
        MOV     R1, #0
        MCR     p15, 0, R1, c7, c5, 4  @ PrefetchFlush to sync the change
                                       @ to the CacheSizeID reg
        MRC     p15, 1, R1, c0, c0, 0  @ Reads current Cache Size ID reg
        AND     R2, R1, #7            @ Extract the line length field
        ADD     R2, R2, #4             @ Add 4 for the line length offset
        LDR     R4, =0x3FF
        ANDS    R4, R4, R1, LSR #3     @ R4 is the max number on the way size
        CLZ     R5, R4                 @ R5 is the bit position of the way size
                                       @ increment
        LDR     R7, =0x00007FFF
        ANDS    R7, R7, R1, LSR #13    @ R7 is the max number of the index size

        CMP     R6, #0                 @ Let's check requested operation
        BEQ     s540
        CMP     R6, #1
        BEQ     s560
s520:                                   @ *** 'Clean&Invalidate' ************
        MOV     R9, R4                 @ R9 working copy of the max way size
s530: 
        ORR     R11, R10, R9, LSL R5   @ Factor in the way nbr and cache nbr
        ORR     R11, R11, R7, LSL R2   @ Factor in the index number
        MCR     p15, 0,R11, c7,c14, 2  @ Clean&Invalidate by set/way
        SUBS    R9, R9, #1             @ Decrement the way number
        BGE     s530
        SUBS    R7, R7, #1             @ Decrement the index
        BGE     s520
        B       s580
s540:                                   @ *** 'Clean' ***********************
        MOV     R9, R4                 @ R9 working copy of the max way size
s550: 
        ORR     R11, R10, R9, LSL R5   @ Factor in the way nbr and cache nbr
        ORR     R11, R11, R7, LSL R2   @ Factor in the index number
        MCR     p15, 0,R11, c7,c10, 2  @ Clean by set/way
        SUBS    R9, R9, #1             @ Decrement the way number
        BGE     s550
        SUBS    R7, R7, #1             @ Decrement the index
        BGE     s540
        B       s580
s560:                                   @ *** 'Invalidate' ******************
        MOV     R9, R4                 @ R9 working copy of the max way size
s570: 
        ORR     R11, R10, R9, LSL R5   @ Factor in the way nbr and cache nbr
        ORR     R11, R11, R7, LSL R2   @ Factor in the index number
        MCR     p15, 0,R11, c7, c6, 2  @ Invalidate by set/way
        SUBS    R9, R9, #1             @ Decrement the way number
        BGE     s570
        SUBS    R7, R7, #1             @ Decrement the index
        BGE     s560
s580: 
        ADD     R10, R10, #2           @ Increment the cache number
        CMP     R3, R10
        BGT     s515
s590:
        MCR     p15, 0, R8, c1, c0, 0  @ Restore CP15 r1 C bit        
        BX      LR
        
@=============================================================================
@ uint32_t hw_arm_processor_core_get(void)
@      returns processor core number (Affinity level 0 in MPIDR register)
@
@ This function shall not use stack, so that it can be called from early
@ boot-up phase where stack has not yet been taken into use.
@
@=============================================================================

hw_arm_processor_core_get:
.globl hw_arm_processor_core_get

        MRC      p15, 0, R0, c0, c0, 5  @ Read MPIDR into R0
        AND      R0,#0xFF               @ Get Aff0
        BX       LR

@=============================================================================
@ uint32_t hw_arm_invasive_debug_status_get(void)
@      returns state of invasive debug control indicated by logical result
@      of (SE OR NSE) of CP14 debug register DBGAUTHSTATUS
@
@=============================================================================

hw_arm_invasive_debug_status_get:
.globl hw_arm_invasive_debug_status_get

        MRC      p14, 0, R0, c7, c14, 6  @ Read DBGAUTHSTATUS into R0
        TST      R0,#(1<<4)|(1<<0)       @  Check SE and NSE
        MOVEQ    R0, #0
        MOVNE    R0, #1
        BX       LR

@=============================================================================
@ uint32_t hw_arm_non_invasive_debug_status_get(void)
@      returns state of non-invasive debug control indicated by logical result
@      of (SNE OR NSNE) of CP14 debug register DBGAUTHSTATUS
@
@=============================================================================

hw_arm_non_invasive_debug_status_get:
.globl hw_arm_non_invasive_debug_status_get

        MRC      p14, 0, R0, c7, c14, 6  @ Read DBGAUTHSTATUS into R0
        TST      R0,#(1<<6)|(1<<2)       @  Check SNE and NSNE
        MOVEQ    R0, #0
        MOVNE    R0, #1
        BX       LR

@=============================================================================
@ uint32_t simulator_in_use(void)
@      returns non-zero if processor ID register shows R0P2
@=============================================================================

simulator_in_use:
.globl simulator_in_use

        MRC      p15, 0, R0, c0, c0, 0
        UBFX     R1,R0,#20,#4
        LSL      R0,R0,#4
        AND      R0,#(0xF<<4)
        ORR      R0,R1,R0
        SUBS     R0,#2  @ R0P2
        MOV      R0,#0
        MOVEQ    R0,#1
        BX       LR

.end

@ End of hw_arm.s

@/* vim: set autoindent shiftwidth=8 smarttab expandtab : */
@/* -*- mode: C@ c-basic-indent: 8@ indent-tabs-mode: nil -*- */
