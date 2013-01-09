/*
 * arch/arm/mach-shmobile/pmCoreStandby.S
 *
 * Copyright (C) 2012 Renesas Mobile Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include "pm_ram0.h"

#include "pmRegisterDef.h"
#include "pmMacro.S"
#include "pmCachemaintenance.S"

	.align	5

/*==========================================*/
/*	Name: PM_Spin_Lock						*/
/*	Overview: Get PM SpinLock 				*/
/*==========================================*/
ENTRY(PM_Spin_Lock)
#ifdef CONFIG_PM_SMP
	MRC		p15, 0, r7, c1, c0, 0	/* System Control Register */
	TST		r7, #0x1
	LDRNE	r9, =ram0ES_2_2_AndAfter
	LDREQ	r9, =ram0ES_2_2_AndAfterPhys
	LDR 	r9, [r9]
	CMP		r9, #0x0	/* es < 2.0.2 */
	BEQ		SPIN_LOCK_ES_2_2_AFTER_GET1_SKIP

	MOV		r4, #0x1

	TST		r7, #0x1
	LDRNE	r5, =ram0SpinLockVA
	LDREQ	r5, =ram0SpinLockPAPhys
	LDR		r5, [r5]

	CMP		r9, #0x1
	BEQ		2f	/* es = 2.0.2 */

3: /* es > 2.0.2 */
	LDREX	r6, [r5]
	CMP		r6, #0x0
	/* WFENE */
	STREXEQ	r6, r4, [r5]
	CMPEQ	r6, #0x0
	BNE		3b
	DMB
	B	SPIN_LOCK_GET1_END

2:
	LDR	r6, [r5]
	CMP	r6, #0x0
	BNE	2b
	DMB

	MRC	p15, 0, r8, c0, c0, 5	/* Read MPIDR */
	ADD		r4, r4, r8 /* Different value for each CPU */

1:
	LDREX	r6, [r5]
	CMP		r6, #0x0
	/* WFENE */
	STREXEQ	r6, r4, [r5]
	LDR		r6, [r5]
	CMPEQ	r6, r4
	BNE		1b
	DMB
	B	SPIN_LOCK_GET1_END

SPIN_LOCK_ES_2_2_AFTER_GET1_SKIP:
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	MRC		p15, 0, r7, c1, c0, 0	/* System Control Register */
40:
	CMP		r4, #0x0
	BNE		41f
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU1SpinLock
	LDREQ		r8, =ram0CPU1SpinLockPhys
	MOV		r10, #2 /* check count */
	B		50f
41:
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU0SpinLock
	LDREQ		r8, =ram0CPU0SpinLockPhys
	MOV		r10, #3 /* check count */
50:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU1 Flag check */
	BNE		40b
	SUBS	r10, r10, #1
	BNE		50b

	CMP		r4, #0x0
	BNE		511f
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU0SpinLock
	LDREQ		r8, =ram0CPU0SpinLockPhys
	B		512f
511:
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU1SpinLock
	LDREQ		r8, =ram0CPU1SpinLockPhys
512:
	MOV		r10, #1
	STR		r10, [r8] /* CPU0 Flag set */
	DSB
	MOV		r10, #2 /* check count */

	CMP		r4, #0x0
	BNE		51f
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU1SpinLock
	LDREQ		r8, =ram0CPU1SpinLockPhys
	B		60f
51:
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU0SpinLock
	LDREQ		r8, =ram0CPU0SpinLockPhys
60:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU1 Flag check */
	BEQ		70f
	CMP		r4, #0x0
	BNE		61f
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU0SpinLock
	LDREQ		r8, =ram0CPU0SpinLockPhys
	B		62f
61:
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU1SpinLock
	LDREQ		r8, =ram0CPU1SpinLockPhys
62:
	MOV	r10, #0
	STR	r10, [r8] /* CPU0 Flag clear */
	B		40b
70:
	SUBS	r10, r10, #1
	BNE		60b

SPIN_LOCK_GET1_END:
	mov pc, lr

#endif
.pool
ENDPROC(PM_Spin_Lock)

	.align	5
/*==========================================*/
/*	Name: PM_Spin_Unlock					*/
/*	Overview: Release PM SpinLock 			*/
/*==========================================*/
ENTRY(PM_Spin_Unlock)
#ifdef CONFIG_PM_SMP
	PM_ES2_2_And_After
	BEQ		SPIN_UNLOCK_ES_2_2_AFTER_GET1_SKIP

	DMB
	MOV		r4, #0x0
	MRC		p15, 0, r7, c1, c0, 0	/* System Control Register */
	TST		r7, #0x1
	LDRNE	r5, =ram0SpinLockVA
	LDREQ	r5, =ram0SpinLockPAPhys
	LDR		r5, [r5]
	STR		r4, [r5]
	DSB
	/* SEV */
	B	SPIN_UNLOCK_GET1_END
SPIN_UNLOCK_ES_2_2_AFTER_GET1_SKIP:
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	MRC		p15, 0, r7, c1, c0, 0	/* System Control Register */
	CMP		r4, #0x0
	BNE		1f
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU0SpinLock
	LDREQ		r8, =ram0CPU0SpinLockPhys
	B		2f
1:
	TST		r7, #0x1
	LDRNE		r8, =ram0CPU1SpinLock
	LDREQ		r8, =ram0CPU1SpinLockPhys
2:
	MOV		r9, #0
	STR		r9, [r8]


SPIN_UNLOCK_GET1_END:
	MOV		pc, lr
#endif
.pool
ENDPROC(PM_Spin_Unlock)

/********************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) */
/* This function is running on SDRAM */
/* Enter with: */
/* 	r0 = this pointer */
/* 	lr = Address of return point */
/********************************************************/
.align	5
ENTRY(start_corestandby)
	STMFD	sp!, {r4-r11, lr}

	MRC	p15, 0, r11, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r11, r11, #0, #2 /* Extract the CPUID Field */

	/*--------------------------------------------------------------*/
	/* Initialize the management information of backup area */
	/*--------------------------------------------------------------*/
	CMP		r11, #0 /* if(CPUID == 0) */
	LDREQ	r4, =ram0Cpu0RegisterArea
	LDRNE	r4, =ram0Cpu1RegisterArea

	LDR		r4, [r4]
	LDR		r5, =hoDataArea
	MOV		r0, r4
	LDR 	r1, =0x0
	MOV		r2, r5
	BL		memset
	ADD		r6, r4, r5
	LDR		r7, =hoBackupAddr
	STR		r6, [r4, r7]

	/*--------------------------------------------------------------*/
	/* Set SYS boot address Register values at this point are */
	/*--------------------------------------------------------------*/
	LDR		r4, =SBAR
	LDR		r6, =ram1BasePhys /* Load RAM1 physical address */
	BIC	r7, r6, #0x3 /* Set RAM1 physical address & clear BTMD[1:0] */
	STR		r7, [r4]

	/*------------------------------------------*/
	/* Change domain access settings			*/
	/*------------------------------------------*/
	MRC 	p15, 0, r4, c3, c0, 0
	STMFD	sp!, {r4} /* Save original domain settings */
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0
	ISB

	/*--------------------------------------*/
	/* Jump to CoreStandby() on SDRAM		*/
	/*--------------------------------------*/
	MOV		lr, pc /* lr: Return address */
	B		corestandby /* Call CoreStandby() */
	NOP

	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4} /* Load original domain access settings */
	MCR		p15, 0, r4, c3, c0, 0
	ISB

	LDMFD	sp!, {r4-r11, pc}

ENDPROC(start_corestandby)

/************************************************************/
/*	Transition to System Suspend(A3SM Power Off)			*/
/* This function is running on SDRAM */
/* Enter with: */
/* r0 = this pointer */
/* lr = Address of return point */
/************************************************************/
ENTRY(jump_systemsuspend)
	STMFD	sp!, {r4-r11, lr}

	MRC	p15, 0, r11, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX r11, r11, #0, #2 /* Extract the CPUID Field */

	/*------------------------------*/
	/* Initialize the backup area	*/
	/*------------------------------*/
	CMP		r11, #0 /* if(CPUID == 0) */
	LDREQ	r4, =ram0Cpu0RegisterArea
	LDRNE	r4, =ram0Cpu1RegisterArea
	LDR		r4, [r4]
	LDR		r5, =hoDataArea
	MOV		r0, r4
	LDR 	r1, =0x0
	MOV		r2, r5
	BL		memset
	ADD		r6, r4, r5
	LDR		r7, =hoBackupAddr
	STR		r6, [r4, r7]

	/*------------------------------------------*/
	/* Change domain access settings			*/
	/*------------------------------------------*/
	MRC 	p15, 0, r4, c3, c0, 0
	STMFD	sp!, {r4} /* Save original domain settings */
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0

	/*------------------------------------------*/
	/* Jump to systemsuspend() on SDRAM			*/
	/*------------------------------------------*/
	MOV		lr, pc
	B		systemsuspend /* Call systemsuspend() */
	NOP

	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4} /* Load original domain access settings */
	MCR		p15, 0, r4, c3, c0, 0

	LDMFD	sp!, {r4-r11, pc}
.pool

ENDPROC(jump_systemsuspend)

/********************************************************************/
/* Vector table */
/* This function is running on RAM1	 (Inter connect RAM1) */
/* After modification of this function, please check the offset. */
/* Enter with: */
/********************************************************************/
	.align	5
ENTRY(ArmVector)
	B		WAKEUP_ENTRY	/* RESET */
	NOP						/* UNDEF */
	NOP						/* SVC */
	NOP						/* PABT	*/
	NOP						/* DABT	*/
	NOP						/* RESERVED */
	NOP						/* IRQ */
	NOP						/* FIQ */
WAKEUP_EXCEPTLOOP:
	B		WAKEUP_EXCEPTLOOP
WAKEUP_ENTRY:		/* Entry point for wake up core	*/

	/* Enable the I-Cache and branch prediction */
	MCR		p15, 0, r0, c7, c5, 0 /* Invalidate I-Cache to PoC */
	MCR		p15, 0, r0, c7, c5, 6 /* Invalidate branch predictor */
	MOV		r0, #0x1800
	MCR	p15, 0, r0, c1, c0, 0 /* Enable I-Cache and branch prediction */
	ISB
	MCR	p15, 0, r0, c8, c7, 0 /* Invalidate entire unified TLB */
	ISB

	/*------------------------------------------*/
	/* LPDDR2 ZQ Calibration Issue WA			*/
	/*------------------------------------------*/
	LDR r0, =ram0ZQCalibPhys
	LDR r1, [r0]
	CMP r1, #0
	BEQ		SKIP_ZQ_CALIBRATION_WA

	LDR 	r4, =STBCHRB3
	LDR		r5, =STBCHRB3_bit7
	LDRB	r6, [r4]
	AND		r6, r6, r5
	CMP		r6, #0	/* 0: one-SDRAM, skip WA */
	BEQ		SKIP_ZQ_CALIBRATION_WA
	/* write 0x00000000 to the address 0xFE528200 */
	LDR 	r4, =SdramZQCalib1Phys
	LDR		r5, =0
	STR		r5, [r4]
	/* write 0x00000000 to the address 0xFE538200 */
	LDR 	r4, =SdramZQCalib2Phys
	LDR		r5, =0
	STR		r5, [r4]
SKIP_ZQ_CALIBRATION_WA:

	/* Jump to wakeup code	*/
	MRC		p15, 0, r1, c0, c0, 5
	UBFX	r2, r1, #0x0, #0x2 /* Extract the CPU ID field */
	CMP		r2, #0
	LDREQ	r3, =ram0WakeupCodeAddr0Phys
	LDRNE	r3, =ram0WakeupCodeAddr1Phys
	LDR		r1, [r3]

	MOV		pc,	r1 /* Jump to WakeUp Code */
.pool
ENDPROC(ArmVector)

/*****************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) */
/* This function is running on SDRAM */
/* After modification of this function, please check the offset. */
/* Enter with: */
/* 	r0 = this pointer */
/* 	lr = Address of return point */
/******************************************************************/
	.align	5
ENTRY(corestandby)
	STMFD	sp!, {r4-r11, lr}

	/*------------------------------------------*/
	/* Clear WUPSFAC */
	/*------------------------------------------*/
	LDR		r4, =WUPSFAC
	LDR		r4, [r4] /* Clear WUPSFAC */

	/*------------------------------------------*/
	/* Save ARM registers						*/
	/*------------------------------------------*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r0, =ram0Cpu0RegisterArea /* r0: Backup area for CPU0 */
	LDRNE	r0, =ram0Cpu1RegisterArea /* r0: Backup area for CPU1 */
	LDR		r0, [r0]
	MOV		lr, pc /* lr: Return address */
	B		save_arm_register
	NOP

	/*------------------------------------------------------*/
	/* r0: Backup area of resume point after MMU is enable */
	/*------------------------------------------------------*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r0, =ram0CoreStandbyRestoreCPU0
	LDRNE	r0, =ram0CoreStandbyRestoreCPU1
	ADR		r4, CORESTANDBY_RESTORE
	STR		r4, [r0]
	DSB

	/*------------------------------------------*/
	/* Disable MMU							*/
	/*------------------------------------------*/
	/* Jump point to corestandby_pa non-MMU */
	LDR		r0, =ram1CoreStandbyPAPhys
	LDR		r4, =ram1DisableMMU
	MOV		pc, r4 /* Call disablemmu() */
	NOP

CORESTANDBY_RESTORE:
	NOP
	MCR	p15, 0, r0, c8, c7, 0 /* Invalidate entire unified TLB */
	MCR	p15, 0, r0, c7, c5, 6 /* Invalidate branch predictor */
	DSB
	ISB

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r0, =ram0Cpu0RegisterArea
	LDRNE	r0, =ram0Cpu1RegisterArea
	LDR		r0, [r0]
	MOV		lr, pc /* lr: Return address */
	/* Call restore_arm_register_va */
	B		restore_arm_register_va
	NOP

	LDMFD	sp!, {r4-r11,pc}
.pool
ENDPROC(corestandby)

/************************************************/
/* Transition to System Suspend(A3SM Power Off) */
/* This function is running on SDRAM */
/* Enter with: */
/* 	r0 = this pointer */
/* 	lr = Address of return point */
/***********************************************/
	.align	5
ENTRY(systemsuspend)
	STMFD	sp!, {r4-r11, lr}

	/* Check CPU ID	*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX r4, r4, #0, #2		/* Extract the CPUID Field */
	CMP		r4, #0			/* if(CPUID == 0) */
	BNE		CPU1_SYSTEMSUSPEND

	/*==================================*/
	/* CPU 0 System Suspend				*/
	/*==================================*/
CPU0_SYSTEMSUSPEND:

	/*--------------------------------------*/
	/* Clear WUPSFAC						*/
	/*--------------------------------------*/
	LDR		r6, =WUPSFAC
	LDR		r6, [r6]		/* Clear WUPSFAC */
	/*--------------------------------------*/
	/* Save ARM common registers			*/
	/*--------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea /* r0: backup area */
	LDR		r0, [r0]
	MOV		lr, pc
	B save_arm_common_register /* Call save_arm_common_register()	*/
	NOP

	/*--------------------------------------*/
	/* Save ARM registers					*/
	/*--------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea /* r0: backup area */
	LDR		r0, [r0]
	MOV		lr, pc
	B		save_arm_register	/* Call save_arm_register() */
	NOP

	/*--------------------------------------*/
	/* Save Common registers				*/
	/*--------------------------------------*/
	MOV		lr, pc		/* lr: Return address */
	B		save_common_register /* Call save_common_register() */
	NOP

	ADR		r4, SYSTEMSUSPEND_RESTORE_CPU0
	LDR		r0, =ram0SystemSuspendRestoreCPU0
	STR		r4, [r0]

	/*--------------------------------------*/
	/* Disable MMU							*/
	/*--------------------------------------*/
	/* Set jump point after disable MMU to systemsuspend_cpu0_pa */
	LDR		r0, =ram1SystemSuspendCPU0PAPhys
	LDR		r4, =ram1DisableMMU
	MOV		pc, r4		/* Call disablemmu() */
	NOP

SYSTEMSUSPEND_RESTORE_CPU0:
	NOP

	/* Invalidate entire unified TLB */
	MCR		p15, 0, r0, c8, c7, 0
	/* Invalidate branch predictor */
	MCR		p15, 0, r0, c7, c5, 6
	DSB
	ISB

	/* Invalidate the duplicate tag store */
	LDR		r4, =0xFF		/* Set CPU0,1 ways*/
	LDR		r6, =BaseSCU
	STR		r4, [r6, #SCU_INVALL]
	DSB

	/*------------------------------*/
	/* Restore ARM registers		*/
	/*------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	MOV		lr, pc		/* lr: Return address */
	B restore_arm_register_va /* Call restore_arm_register_va */
	NOP

	/*------------------------------*/
	/* Restore ARM common registers	*/
	/*------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	MOV		lr, pc		/* lr: Return address	*/
	B restore_arm_common_register /* Call restore_arm_common_register */
	NOP

	B		SYSTEMSUSPEND_EXIT

	/*======================*/
	/* CPU 1 System Suspend	*/
	/*======================*/
CPU1_SYSTEMSUSPEND:

	/*----------------------*/
	/* Save ARM registers	*/
	/*----------------------*/
	LDR		r0, =ram0Cpu1RegisterArea	/* r0: backup area */
	LDR		r0, [r0]
	MOV		lr, pc		/* lr: Return address */
	B		save_arm_register /* Call save_arm_register()	*/
	NOP

	ADR		r4, SYSTEMSUSPEND_RESTORE_CPU1
	LDR		r0, =ram0SystemSuspendRestoreCPU1
	STR		r4, [r0]

	/*----------------------*/
	/* Disable MMU			*/
	/*----------------------*/
	/* Set jumpt point after disable MMU to systemsuspend_cpu1_pa */
	LDR		r0, =ram1SystemSuspendCPU1PAPhys
	LDR		r4, =ram1DisableMMU
	MOV		pc, r4		/* Call disablemmu()	*/
	NOP

SYSTEMSUSPEND_RESTORE_CPU1:
	NOP
	/* Invalidate entire unified TLB */
	MCR		p15, 0, r0, c8, c7, 0
	/* Invalidate branch predictor	*/
	MCR		p15, 0, r0, c7, c5, 6
	DSB
	ISB

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu1RegisterArea
	LDR		r0, [r0]
	MOV		lr, pc		/* lr: Return address */
	B restore_arm_register_va /* Call restore_arm_register_va */
	NOP

SYSTEMSUSPEND_EXIT:

	LDMFD	sp!, {r4-r11,pc}
.pool

ENDPROC(systemsuspend)


/****************************************/
/* Save ARM register					*/
/* This function is running on SDRAM	*/
/* Enter with: 							*/
/* 	r0:backup area						*/
/* 	lr:return address					*/
/****************************************/

	.align	5
ENTRY(save_arm_register)
	/*------------------------------*/
	/* Read backup start address	*/
	/*------------------------------*/
	MOV		r1, r0 /* r1 = backup area(manage area base address) */
	LDR		r2, =hoBackupAddr
	LDR		r0, [r0, r2] /* Read backup start address */

	/*------------------------------*/
	/* Save SVC mode context		*/
	/*------------------------------*/
	LDR		r4, =hoSaveArmSvc
	STR		r0, [r1, r4]
	MRS		r4, CPSR /* Get CPSR */
	MRS		r5, SPSR /* Get SPSR */
	STMIA	r0!, {r4-r5, r12-r14} /* Save CPSR,SPSR,r12,sp,lr */

	/*--------------------------------------*/
	/* Restore manage area base address		*/
	/*--------------------------------------*/
	MOV		r12, r1

	/*----------------------------------------------*/
	/* Save SYS,ABT,UND,IRQ,FIQ mode context		*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmExceptSvc
	STR		r0, [r12, r4]
	CPS		#POWER_MODESYS /* Switch to SYS mode */
	STMIA	r0!, {r13-r14} /* Save sp,lr */
	CPS		#POWER_MODEFIQ /* Switch to FIQ mode */
	MRS		r4, SPSR /* Get SPSR */
	STMIA	r0!, {r4, r8-r14} /* Save SPSR,r8-r12,sp,lr */
	CPS		#POWER_MODEIRQ /* Switch to IRQ mode */
	MRS		r4, SPSR /* Get SPSR */
	STMIA	r0!, {r4, r13-r14} /* Save SPSR,sp,lr */
	CPS		#POWER_MODEABT /* Switch to ABT mode */
	MRS		r4, SPSR /* Get SPSR */
	STMIA	r0!, {r4, r13-r14} /* Save SPSR,sp,lr */
	CPS		#POWER_MODEUND /* Switch to UND mode */
	MRS		r4, SPSR /* Get SPSR */
	STMIA	r0!, {r4, r13-r14} /* Save SPSR,sp,lr */
	CPS		#POWER_MODESVC /* switch to SVC mode */

	/*----------------------------------------------*/
	/* Save VFP */
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmVfp
	STR		r0, [r12, r4]
	/* Read Coprocessor Access Control register */
	MRC	p15, 0, r3, c1, c0, 2
	ORR		r4, r3, #0xF00000 /* Allows CP10 and CP11 access */
	/* Write Coprocessor Access Control register*/
	MCR	p15, 0, r4, c1, c0, 2
	ISB		 /* Insrtuction Synchronization Barrier */
	MRS		r4, FPEXC /* Read FPEXC register(VMRS -> MRS) */
	STR		r4, [r0], #0x4 /* Save FPEXC register */
	MOV		r5, #0x1
	MOV		r6, r5, LSL #0x1E /* Set EN bit to 1. */
	/* Write FPEXC (Set global enable bit to 1)(VMSR -> MSR)*/
	MSR	FPEXC, r6
	MRS		r7, FPSCR /* Read FPSCR register(VMRS -> MRS) */
	STR		r7, [r0], #0x4 /* Save FPSCR register */
	VSTM	r0!, {D0-D15} /* Save VFP-D16 register */
	MRS		r5, MVFR0 /* Read MVFR0 register(VMRS -> MRS) */
	AND		r6, r5, #0xF /* Extract the RB field */
	TEQ		r6, #0x2 /* Check for Advanced SIMD/VFP-D32 support */
	POWER_VSTMIA(POWER_CC_EQ, 0, 16, 16) /* Save VFP-D32 */
	MSR		FPEXC, r4 /* Restore FPEXC register(VMSR -> MSR) */
	MCR	p15, 0, r3, c1, c0, 2 /* Restore Coprocessor Access Control */


	/*----------------------------------------------*/
	/* Save System(MMU) */
	/*----------------------------------------------*/
	LDR		r1, =ram0MmuSetting0
	MRC		p15, 0, r4, c0, c0, 5
	UBFX	r5, r4, #0x0, #0x2
	LDR		r6, =saveArmMmuSettingSize
	MUL		r7, r5, r6
	ADD		r1, r1, r7
	MRC	p15, 0, r3, c10, c2, 0 /* Primary Region Remap Register */
	MRC	p15, 0, r4, c10, c2, 1 /* Normal Memory Remap Register */
	MRC	p15, 0, r5, c2, c0, 0 /* Translation Table Base 0 Register */
	MRC	p15, 0, r6, c2, c0, 1 /* Translation Table Base 1 Register */
	/* Translation Table Base Control Register */
	MRC	p15, 0, r7, c2, c0, 2
	MRC	p15, 0, r8, c3, c0, 0 /* Domain Access Control Register */
	MRC	p15, 0, r9, c13, c0, 1 /* Context ID Register */
	MRC	p15, 0, r10, c1, c0, 1 /* Auxiliary Control Register */
	MRC	p15, 0, r11, c1, c0, 0 /* System Control Register */
	STMIA	r1!, {r3-r11}

	/*----------------------------------------------*/
	/* Save System */
	/*----------------------------------------------*/
	LDR		r4,  =hoSaveArmSystem
	STR		r0, [r12, r4]
	MRC	p15, 0, r4, c12, c0, 0 /* Vector Base Address Register */
	MRC	p15, 0, r5, c7, c4, 0 /* Physical Address Register */
	MRC	p15, 0, r6, c13, c0, 2 /* User Read/Write Thread ID Register */
	MRC	p15, 0, r7, c13, c0, 3 /* User Read-only Thread ID Register */
	MRC	p15, 0, r8, c13, c0, 4 /* Privileged Only ThreadID Register */
	MRC	p15, 2, r9, c0, c0, 0 /* Cache Size Selection Register */
	MRC	p15, 0, r10, c1, c0, 2 /* Coprocessor Access Control Register */
	MRC	p15, 0, r11, c15, c0, 0 /* Power Control Register */
	STMIA	r0!, {r4-r11}
	MRC		p15, 0, r4, c15, c0, 1 /* Diagnostic register */
	STMIA	r0!, {r4}

	/*----------------------------------------------*/
	/* Update backup start address					*/
	/*----------------------------------------------*/
	LDR		r4, =hoBackupAddr
	STR		r0, [r12, r4]

	/*----------------------------------------------*/
	/* Remove cluster */
	/*----------------------------------------------*/
	/* Disable D-Cache	*/
	DSB
	MRC		p15, 0, r4, c1, c0, 0
	BIC		r4, r4, #0x4 /* Clear C bit */
	MCR		p15, 0, r4, c1, c0, 0 /* Disable D-Cache */
	ISB

	/* Clean and invalidate D-Cache	*/
	L1_DataCacheCleanInvalidateSW

#ifdef CONFIG_PM_SMP
	/*----------------------------------------------*/
	/* Remove the CPU from the cluster				*/
	/*----------------------------------------------*/
	MRC		p15, 0, r4, c1, c0, 1
	BIC		r4, r4, #0x40 /* Clear SMP bit */
	MCR		p15, 0, r4, c1, c0, 1 /* Write the ACTLR */
	ISB
#endif

	MOV		pc, lr
.pool

ENDPROC(save_arm_register)

/****************************************************************/
/* Restore ARM register(running in MMU disable) */
/* This function is running on SDRAM */
/* After modification of this function, please check the offset.*/
/* Enter with: */
/*	r0:backup area */
/*	r1:Jump point after enable MMU */
/*	lr:return address */
/****************************************************************/
	.align	5
ENTRY(restore_arm_register_pa)
	/*----------------------------------------------*/
	/* System(MMU) */
	/*----------------------------------------------*/
	LDMIA	r0!, {r3-r11}
	MCR		p15, 0, r9, c13, c0, 1 /* Context ID Register */
	ISB
	MCR	p15, 0, r8, c3, c0, 0 /* Domain Access Control Register */
	/* Translation Table Base Control Register */
	MCR	p15, 0, r7, c2, c0, 2
	MCR	p15, 0, r6, c2, c0, 1 /* Translation Table Base 1 Register */
	MCR	p15, 0, r5, c2, c0, 0 /* Translation Table Base 0 Register */
	MCR	p15, 0, r4, c10, c2, 1 /* Normal Memory Remap Register */
	MCR	p15, 0, r3, c10, c2, 0 /* Primary Region Remap Register */
	DSB

	/* Enable MMU */
	ISB
    LDR		r0, =0x1801             /* set I,Z,M bit */
    MCR		p15, 0, r0, c1, c0, 0   /* MMU Enable */
    MCR		p15, 0, r0, c7, c5, 6   /* invalidate branch predictor */
	DSB
    ISB
    MCR     p15,0,r0,c8,c7,0        /* invalidate all TLB entries on this cpu */
    ISB

	/* Auxiliary Control Register(set SMP bit) */
	MRC	    p15, 0, r2, c1, c0, 1   /* Dummy read */
	ORR		r10, r10, #0x40         /* Set SMP bit */
	MCR		p15, 0, r10, c1, c0, 1  /* Write the ACTLR */
	ISB

	/* Enable D-Cache */
	MCR	p15, 0, r11, c1, c0, 0
	DSB
	ISB

	MOV		pc, r1

.pool

ENDPROC(restore_arm_register_pa)

/****************************************************************/
/* Restore ARM register(running in MMU enable) */
/* This function is running on SDRAM */
/* After modification of this function, please check the offset.*/
/* Enter with: */
/*	r0:backup area  */
/*	lr:return address  */
/****************************************************************/
	.align	5
ENTRY(restore_arm_register_va)
	/*----------------------------------------------*/
	/* Copy backup area address */
	/*----------------------------------------------*/
	MOV		r12, r0 /* r12 = r0(backup area) */

	/*----------------------------------------------*/
	/* System */
	/*----------------------------------------------*/
	LDR		r4,  =hoSaveArmSystem
	LDR		r0, [r12, r4]
	LDMIA	r0!, {r4-r11}
	MCR	p15, 0, r4, c12, c0, 0 /* Vector Base Address Register */
	MCR	p15, 0, r5, c7, c4, 0 /* Physical Address Register */
	MCR	p15, 0, r6, c13, c0, 2 /* User Read/Write Thread ID Register */
	MCR	p15, 0, r7, c13, c0, 3 /* User Read-only Thread ID Register */
	MCR	p15, 0, r8, c13, c0, 4 /* Privileged Only ThreadID Register */
	MCR	p15, 2, r9, c0, c0, 0 /* Cache Size Selection Register */
	MCR	p15, 0, r10, c1, c0, 2 /* Coprocessor Access Control Register */
	MCR	p15, 0, r11, c15, c0, 0 /* Power Control Register */
	LDMIA	r0!, {r4}
	MCR	p15, 0, r4, c15, c0, 1 /* Diagnostic register */

	/*----------------------------------------------*/
	/* VFP */
	/*----------------------------------------------*/
	LDR	r4, =hoSaveArmVfp
	LDR	r0, [r12, r4]
	MRC	p15, 0, r3, c1, c0, 2 /* Read Coprocessor Access Control */
	ORR	r4, r3, #0xF00000 /* Allows CP10 and CP11 access */
	MCR	p15, 0, r4, c1, c0, 2 /* Write Coprocessor Access Control*/
	ISB	 /* Insrtuction Synchronization Barrier */
	MOV	r5, #0x1
	MOV	r6, r5, LSL #0x1E /* Set EN bit to 1. */
	MSR	FPEXC, r6 /* Write FPEXC (Set global enable bit to 1) */
	LDMIA	r0!, {r4-r5} /* Load r4:FPEXC, r5:FPSCR */
	VLDM	r0!, {D0-D15} /* Restore VFP-D16 register */
	MRS	r6, MVFR0 /* Read MVFR0 register */
	AND	r7, r6, #0xF /* Extract the RB field */
	TEQ	r7, #0x2 /* Check for Advanced SIMD/VFP-D32 support */
	POWER_VLDMIA(POWER_CC_EQ, 0, 16, 16) /* Restore VFP-D32 */
	MSR		FPSCR, r5 /* Restore FPSCR */
	MSR		FPEXC, r4 /* Restore FPEXC */
	MCR	p15, 0, r3, c1, c0, 2 /* Restore Coprocessor Access Control */

	/*----------------------------------------------*/
	/* SYS, ABT, UND, IRQ, FIQ mode context		*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmExceptSvc
	LDR		r0, [r12, r4]
	CPS		#POWER_MODESYS /* Switch to SYS mode */
	LDMIA	r0!, {r13-r14} /* Restore sp,lr */
	CPS		#POWER_MODEFIQ /* Switch to FIQ mode */
	LDMIA	r0!, {r4, r8-r14} /* Load SPSR, restore r8-r12,sp,lr */
	MSR		SPSR_fsxc, r4 /* Restore SPSR */
	CPS		#POWER_MODEIRQ /* Switch to IRQ mode */
	LDMIA	r0!, {r4, r13-r14} /* Load SPSR, restore sp,lr */
	MSR		SPSR_fsxc, r4 /* Restore SPSR */
	CPS		#POWER_MODEABT /* Switch to ABT mode */
	LDMIA	r0!, {r4, r13-r14} /* Load SPSR, restore sp,lr */
	MSR		SPSR_fsxc, r4 /* Restore SPSR */
	CPS		#POWER_MODEUND /* Switch to UND mode */
	LDMIA	r0!, {r4, r13-r14} /* Load SPSR, restore sp,lr */
	MSR		SPSR_fsxc, r4 /* Restore SPSR */
	CPS		#POWER_MODESVC /* switch to SVC mode */

	/* SVC mode context */
	LDR		r4, =hoSaveArmSvc
	LDR		r0, [r12, r4]
	MOV		r1, lr /* Backup link register */
	LDMIA	r0!, {r4-r5, r12-r14} /* Load SPSR,CPSR & restore r12,sp,lr */
	MSR		SPSR_fsxc, r5 /* Restore SPSR */
	MSR		CPSR_fsxc, r4 /* Restore CPSR */

	MOV		lr, r1 /* Restore link register */
	MOV		pc, lr /* return */
.pool

ENDPROC(restore_arm_register_va)

/********************************************************************/
/* Save arm common register											*/
/* This function is running on SDRAM */
/* After modification of this function, please check the offset.	*/
/* Enter with: */
/*	lr:return address */
/********************************************************************/
	.align	5
ENTRY(save_arm_common_register)

	/*--------------------------------------*/
	/* Save manage area base address		*/
	/*--------------------------------------*/
	MOV		r12, r0
	LDR		r4, =hoBackupAddr
	LDR		r0, [r12, r4] /* Read backup start address */

	/*---------------*/
	/* Save GIC		*/
	/*-------------*/
	LDR		r4, =hoSaveArmGic
	STR		r0, [r12, r4]

	/* Interrupt interface register	*/
	LDR		r1, =BaseInterruptIF
	LDR		r4, [r1, #ICC_PMR]
	LDR		r5, [r1, #ICC_BPR]
	LDR		r6, [r1, #ICC_ICR]
	STMIA	r0!, {r4-r6}

	/* Block interrupts					*/
	MOV		r4, #0x0
	STR		r4, [r1, #ICC_PMR]
	/* Distributor register				*/
	MRC		p15, 0, r3, c0, c0, 5 /* Read MP affinity register */
	UBFX	r3, r3, #0x0, #0x2 /* Extract the CPUID field */
	LDR		r1, =BaseDistrbutor
	CMP		r3, #0x0
	MOVNE	r4, #0x1	/* CPU != 0 */
	BNE		SAVE_ARM_COMMON_LINENUMBER_EXIT
	LDR		r4, [r1, #ICD_ICTR]
	UBFX	r4, r4, #0x0, #0x5
	ADD		r4, r4, #0x1 /* CPU == 0 */

SAVE_ARM_COMMON_LINENUMBER_EXIT:
	MOV		r5, r4
	MOV		r2, r1 /* Gic Distributor Base Address	*/

SAVE_ARM_COMMON_GIC_LOOP1:
	/* Interrupt Security register */
	LDR		r6, [r2, #ICD_ISR]
	/* Interrupt Set-Enable register */
	LDR		r7, [r2, #ICD_ISER]
	STMIA	r0!, {r6, r7}
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		SAVE_ARM_COMMON_GIC_LOOP1

	MOV		r5, r4, LSL #0x3
	MOV		r2, r1 /* Gic Distributor Base Address */

SAVE_ARM_COMMON_GIC_LOOP2:
	/* Interrupt Priority register */
	LDR		r6, [r2, #ICD_IPR]
	/* Interrupt Processor Targets register	*/
	LDR		r7, [r2, #ICD_IPTR]
	STMIA	r0!, {r6, r7}
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		SAVE_ARM_COMMON_GIC_LOOP2

	/* Interrupt Configuration register	*/
	MOV		r5, r4, LSL #0x1
	MOV		r2, r1 /* Gic Distributor Base Address */

SAVE_ARM_COMMON_GIC_LOOP3:
	LDR		r6, [r2, #ICD_ICFR]
	STR		r6, [r0], #0x4
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		SAVE_ARM_COMMON_GIC_LOOP3

	/* Distributor Control rergister	*/
	CMP		r3, #0x0
	BNE		SAVE_ARM_COMMON_GIC_EXIT
	LDR		r6, [r1, #ICD_DCR]
	STR		r6, [r0], #0x4

SAVE_ARM_COMMON_GIC_EXIT:

	/*----------------------------------------------*/
	/* Save Timer					*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmTimer
	STR		r0, [r12, r4]

	/* CPG.PCLKCR */
	LDR		r1, =PCLKCR
	LDR		r4, [r1]
	STR		r4, [r0], #0x4

	/* Private Timer	*/
	LDR		r1, =BasePrivateAndWDT
	LDR		r4, [r1, #PRTIM_CONTROL]
	BIC		r3, r4, #0x1	/* Clear Timer Enable bit */
	STR		r3, [r1, #PRTIM_CONTROL] /* Disable Private Timer */
	LDR		r5, [r1, #PRTIM_LOAD]
	LDR		r6, [r1, #PRTIM_COUNTER]
	LDR		r7, [r1, #PRTIM_INTSTATE]
	STMIA	r0!, {r4-r7}

	/* Watchdog	*/
	LDR		r4, [r1, #WDOG_CONTROL]
	BIC		r3, r4, #0x1	/* Clear Watchdog Enable bit */
	STR		r3, [r1, #WDOG_CONTROL] /* Disable Watchdog */
	LDR		r5, [r1, #WDOG_LOAD]
	LDR		r6, [r1, #WDOG_COUNTER]
	LDR		r7, [r1, #WDOG_INTSTATE]
	STMIA	r0!, {r4-r7}

	/* Save Global Timer */
	LDR		r1, =BaseGlobalTimer
	LDR		r4, [r1, #GLTIM_CONTROL]
	/* Clear Global Timers settings	  */
	/*  Auto-incremenet, IRQ Enable   */
	/*  Comp Enable, Timer Enable bit */
	BIC		r3, r4, #0xF
	STR		r3, [r1, #GLTIM_CONTROL] /* Stop Global timer */
	LDR		r5, [r1, #GLTIM_AUTO]
	LDR		r6, [r1, #GLTIM_INTSTATE]
	STMIA	r0!, {r4-r6}
	LDR		r4, [r1, #GLTIM_CMPLOW]
	LDR		r5, [r1, #GLTIM_CMPHI]
	LDR		r6, [r1, #GLTIM_CNTLOW]
	LDR		r7, [r1, #GLTIM_CNTHI]
	STMIA	r0!, {r4-r7}

	/*----------------------------------------------*/
	/* Update backup start address					*/
	/*----------------------------------------------*/
	LDR		r4, =hoBackupAddr
	STR		r0, [r12, r4]


	MOV		pc, lr		/* return */

.pool

ENDPROC(save_arm_common_register)

/********************************************************************/
/* Restore ARM common register										*/
/* This function is running on SDRAM */
/* After modification of this function, please check the offset.	*/
/* Enter with: 						*/
/*	r0:backup area					*/
/*	lr:return address				*/
/********************************************************************/
	.align	5
ENTRY(restore_arm_common_register)

	/*--------------------------------------*/
	/* Restore manage area base address		*/
	/*--------------------------------------*/
	MOV		r12, r0

	/*----------------------------------------------*/
	/* Timer		*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmTimer
	LDR		r0, [r12, r4]

	/* CPG.PCLKCR */
	LDR		r4, [r0], #0x4
	LDR		r5, =PCLKCR
	STR		r4, [r5]

	/* Private Timer	*/
	LDR		r1, =BasePrivateAndWDT
	LDR		r4, [r1, #PRTIM_CONTROL]
	BIC		r4, r4, #0x1	/* Clear Timer Enable bit */
	STR		r4, [r1, #PRTIM_CONTROL] /* Disable Private Timer */
	LDMIA	r0!, {r4-r7}
	STR		r5, [r1, #PRTIM_LOAD]
	ANDS	r8, r7, #0x1	/* r7: Private Timer Interrupt Status */
	MOVNE	r9, #0x1
	MOVEQ	r9, r6			/* r6: Private Timer Counter */
	STR		r9, [r1, #PRTIM_COUNTER] /* Set counter */
	STR		r4, [r1, #PRTIM_CONTROL]

	/* Watchdog			*/
	LDR		r4, [r1, #WDOG_CONTROL]
	BIC		r4, r4, #0x1	/* Clear Watchdog Enable bit */
	STR		r4, [r1, #WDOG_CONTROL] /* Disable Watchdog */
	LDMIA	r0!, {r4-r7}
	STR		r5, [r1, #WDOG_LOAD]
	ANDS	r8, r7, #0x1	/* r7: Watchdog Interrupt Status */
	MOVNE	r9, #0x1
	MOVEQ	r9, r6			/* r6 = Watchdog Counter */
	STR		r9, [r1, #WDOG_COUNTER] /* Set counter */
	ANDS	r8, r4, #0x8	/* r4 = Watchdog Control Register */
	BNE		RESTORE_ARM_COMMON_WATCHDOGDISABLE_SKIP
	LDR		r9, =0x12345678
	STR		r9, [r1, #WDOG_DISABLE]
	LDR		r9, =0x87654321
	STR		r9, [r1, #WDOG_DISABLE]

RESTORE_ARM_COMMON_WATCHDOGDISABLE_SKIP:
	STR		r4, [r1, #WDOG_CONTROL]

	/* Global Timer	*/
	LDR		r1, =BaseGlobalTimer
	LDR		r4, [r1, #GLTIM_CONTROL]
	/* Clear Global Timers settings	  */
	/*  Auto-incremenet, IRQ Enable   */
	/*  Comp Enable, Timer Enable bit */
	BIC		r4, r4, #0xF
	STR		r4, [r1, #GLTIM_CONTROL] /* Stop Global timer */
	LDMIA	r0!, {r4-r6}
	ANDS	r9, r6, #0x1
	BEQ		RESTORE_ARM_COMMON_GLOBALTIMEREVENT_SKIP
	MOV		r7, #0x0
	STR		r7, [r1, #GLTIM_CNTLOW]
	STR		r7, [r1, #GLTIM_CNTHI]
	MOV		r8, #0x1
	STR		r8, [r1, #GLTIM_CMPLOW]
	STR		r7, [r1, #GLTIM_CMPHI]
	/* Set IRQ Enable/Comp Enable/Timer Enable bit */
	MOV		r7, #0x7
	STR		r7, [r1, #GLTIM_CONTROL]
RESTORE_ARM_COMMON_GLOBALTIMEREVENT_LOOP:
	LDR		r8, [r1, #GLTIM_INTSTATE]
	ANDS	r8, r8, #0x1
	BEQ		RESTORE_ARM_COMMON_GLOBALTIMEREVENT_LOOP
	LDR		r7, [r1, #GLTIM_CONTROL]
	/* Clear Auto-increment/IRQ Enable/Comp Enable/Timer Enable bit	*/
	BIC		r7, r7, #0xF
	STR		r7, [r1, #GLTIM_CONTROL] /* Stop Global timer */
RESTORE_ARM_COMMON_GLOBALTIMEREVENT_SKIP:
	STR		r5, [r1, #GLTIM_AUTO]
	LDMIA	r0!, {r5-r8}
	STR		r5, [r1, #GLTIM_CMPLOW]
	STR		r6, [r1, #GLTIM_CMPHI]
	STR		r7, [r1, #GLTIM_CNTLOW]
	STR		r8, [r1, #GLTIM_CNTHI]
	STR		r4, [r1, #GLTIM_CONTROL]

	/*----------------------------------------------*/
	/* GIC				*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmGic
	LDR		r0, [r12, r4]

	/* Interrupt interface register */
	LDR		r1, =BaseInterruptIF
	LDMIA	r0!, {r4-r6}
	STR		r4, [r1, #ICC_PMR]
	STR		r5, [r1, #ICC_BPR]
	STR		r6, [r1, #ICC_ICR]

	/* Distributor register				*/
	MRC p15, 0, r3, c0, c0, 5 /* Read the MPIDR, MP affinity register */
	UBFX	r3, r3, #0x0, #0x2 /* Extract the CPUID field */
	LDR		r1, =BaseDistrbutor
	CMP		r3, #0x0
	MOVNE	r4, #0x1		/* CPUID != CPU0 */
	BNE		RESTORE_ARM_COMMON_LINENUMBER_EXIT
	LDR		r4, [r1, #ICD_ICTR]
	UBFX	r4, r4, #0x0, #0x5
	ADD		r4, r4, #0x1	/* CPU == 0 */

RESTORE_ARM_COMMON_LINENUMBER_EXIT:
	/* Interrupt Security register / Interrupt Set-Enable register	*/
	MOV		r5, r4
	MOV		r2, r1 /* r2 = Gic Distributor Base Address */

RESTORE_ARM_COMMON_GIC_LOOP1:
	LDMIA	r0!, {r6, r7}
	STR		r6, [r2, #ICD_ISR]
	STR		r7, [r2, #ICD_ISER]
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		RESTORE_ARM_COMMON_GIC_LOOP1

	MOV		r5, r4, LSL #0x3
	MOV		r2, r1 /* r2 = Gic Distributor Base Address */

RESTORE_ARM_COMMON_GIC_LOOP2:
	LDMIA	r0!, {r6, r7}
	/* Interrupt Priority register */
	STR		r6, [r2, #ICD_IPR]
	/* Interrupt Processor Targets register */
	STR		r7, [r2, #ICD_IPTR]
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		RESTORE_ARM_COMMON_GIC_LOOP2

	/* Interrupt Configuration register	*/
	MOV		r5, r4, LSL #0x1
	MOV		r2, r1

RESTORE_ARM_COMMON_GIC_LOOP3:
	LDR		r6, [r0], #0x4
	STR		r6, [r2, #ICD_ICFR]
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		RESTORE_ARM_COMMON_GIC_LOOP3

	/* Distributor Control rergister	*/
	CMP		r3, #0x0		/* CPU == 0 */
	BNE		RESTORE_ARM_COMMON_GIC_EXIT
	LDR		r6, [r0], #0x4
	STR		r6, [r1, #ICD_DCR]

RESTORE_ARM_COMMON_GIC_EXIT:

	MOV		pc, lr			/* Return */

.pool

ENDPROC(restore_arm_common_register)

/********************************************************************/
/* Save common register												*/
/* This function is running on SDRAM */
/* After modification of this function, please check the offset. */
/* Enter with: 						*/
/*	lr:return address				*/
/********************************************************************/
	.align	5
ENTRY(save_common_register)

	LDR		r0, =ram0CommonSetting

#ifdef CONFIG_PM_SMP
	/*----------------------------------------------*/
	/* Save SCU registers	*/
	/*----------------------------------------------*/
	LDR		r1, =BaseSCU
	LDR		r4, [r1, #SCU_FILTER_START_ADDR]
	LDR		r5, [r1, #SCU_FILTER_END_ADDR]
	LDR		r6, [r1, #SCU_SAC]
	LDR		r7, [r1, #SCU_SNSAC]
	LDR		r8, [r1, #SCU_CTL]
	STMIA	r0!, {r4-r8}

#endif /* CONFIG_PM_SMP */
	/*----------------------------------------------*/
	/* Save PL310 registers	*/
	/*----------------------------------------------*/
	LDR		r1, =BasePl310

	/* Interrupt/Counter Control Registers	*/
	LDR		r4, [r1, #L2_EV_CNT_CONTROL]
	LDR		r5, [r1, #L2_EV_CNT_CONF1]
	LDR		r6, [r1, #L2_EV_CNT_CONF0]
	LDR		r7, [r1, #L2_EV_CNT_VAL1]
	LDR		r8, [r1, #L2_EV_CNT_VAL0]
	LDR		r9, [r1, #L2_INIT_MASK]
	STMIA	r0!, {r4-r9}

	/* Cache Lockdown(0-7) */
	LDR		r2, =L2_LOCKDOWNBase
	MOV		r4, #0x8

SAVE_COMMON_LOCKDOWN_LOOP:
	CMP		r4, #0x0
	BEQ		SAVE_COMMON_LOCKDOWN_EXIT
	SUB		r4, r4, #0x1
	LDR		r5, [r2], #0x4		/* Data Lockdown 0-7 */
	LDR		r6, [r2], #0x4		/* Instruction Lockdown 0-7 */
	STMIA	r0!, {r5, r6}
	B		SAVE_COMMON_LOCKDOWN_LOOP

SAVE_COMMON_LOCKDOWN_EXIT:
	LDR		r4, [r1, #L2_LOCKDOWN_BY_LINE_EN]
	LDR		r5, [r1, #L2_UNLOCK_ALL_LINES]
	STMIA	r0!, {r4, r5}

	/* Address Filtering / Control / Debug	*/
	LDR		r4, [r1, #L2_ADDR_FILTER_START]
	LDR		r5, [r1, #L2_ADDR_FILTER_END]
	LDR		r6, [r1, #L2_DEBUG_CONTROL]
	LDR		r7, [r1, #L2_CONTROL]
	STMIA	r0!, {r4-r7}

	/*----------------------------------------------------------*/
	/* Global Setting(Control / Debug, Prefetch and Power)		*/
	/*----------------------------------------------------------*/
	/* Save Global Setting address	*/
	LDR		r0, =ram0Pl310GlobalSetting
	LDR		r4, [r1, #L2_AUX_CONTROL]
	LDR		r5, [r1, #L2_LATENCY_CONTROL]
	LDR		r6, [r1, #L2_RAM_LATENCY_CONTROL]
	LDR		r7, [r1, #L2_POWER_CONTROL]
	LDR		r8, [r1, #L2_PREFETCH_CONTROL]
	STMIA	r0!, {r4-r8}

	/*----------------------------------*/
	/* Check Way operation				*/
	/*----------------------------------*/
SAVE_COMMON_CHECK_WAY_OPERATION:
	LDR		r4, [r1, #L2_CLEAN_AND_INVALIDATE]
	CMP		r4, #0x0
	BNE		SAVE_COMMON_CHECK_WAY_OPERATION

	/*----------------------------------*/
	/* Check Unlock Way operation		*/
	/*----------------------------------*/
SAVE_COMMON_CHECK_UNLOCK_OPERATION:
	LDR		r4, [r1, #L2_UNLOCK_ALL_LINES]
	CMP		r4, #0x0
	BNE		SAVE_COMMON_CHECK_UNLOCK_OPERATION

	MOV		pc, lr

.pool

ENDPROC(save_common_register)

/********************************************************************/
/* Restore common register											*/
/* This function is running on SDRAM */
/* After modification of this function, please check the offset. */
/* Enter with: 						*/
/********************************************************************/
	.align	5
ENTRY(restore_common_register)

	LDR		r0, =ram0CommonSettingPhys

#ifdef CONFIG_PM_SMP
	/*----------------------------------*/
	/* Restore SCU registers			*/
	/*----------------------------------*/
	LDR		r1, =BaseSCUPhys
	LDMIA	r0!, {r4-r8}
	STR		r4, [r1, #SCU_FILTER_START_ADDR]
	STR		r5, [r1, #SCU_FILTER_END_ADDR]
	STR		r6, [r1, #SCU_SAC]
	STR		r7, [r1, #SCU_SNSAC]

	/*------------------------------------------*/
	/* Invalidate the duplicate TAG store		*/
	/*------------------------------------------*/
	LDR		r4, =0xFFFF
	STR		r4, [r1, #SCU_INVALL]

	/*----------------------------------*/
	/* Restore Control register			*/
	/*----------------------------------*/
	STR		r8, [r1, #SCU_CTL]

#endif /* CONFIG_PM_SMP */
	/*----------------------------------*/
	/* Restore PL310 registers			*/
	/*----------------------------------*/
	/* Disable L2 cache	*/
	MOV		r4, #0x0
	LDR		r1, =BasePl310Phys

RESTORE_COMMON_L2DISABLE_LOOP:
	LDR		r5, [r1, #L2_CONTROL]
	AND		r5, r5, #0x1
	CMP		r5, r4
	BEQ		RESTORE_COMMON_L2DISABLE_SKIP
	/* Write PL310 Control register (Disable L2 cache)	*/
	STR		r4, [r1, #L2_CONTROL]
	DSB
	B		RESTORE_COMMON_L2DISABLE_LOOP

RESTORE_COMMON_L2DISABLE_SKIP:

	/*------------------------------------------*/
	/* Interrupt/Counter Control Registers		*/
	/*------------------------------------------*/
	LDMIA	r0!, {r4-r9}
	STR		r4, [r1, #L2_EV_CNT_CONTROL]
	STR		r5, [r1, #L2_EV_CNT_CONF1]
	STR		r6, [r1, #L2_EV_CNT_CONF0]
	STR		r7, [r1, #L2_EV_CNT_VAL1]
	STR		r8, [r1, #L2_EV_CNT_VAL0]
	STR		r9, [r1, #L2_INIT_MASK]

	/*------------------------------------------*/
	/* Cache Lockdown(0-7)						*/
	/*------------------------------------------*/
	LDR		r2, =L2_LOCKDOWNBasePhys
	MOV		r4, #0x8

RESTORE_COMMON_LOCKDOWN_LOOP:
	CMP		r4, #0x0
	BEQ		RESTORE_COMMON_LOCKDOWN_EXIT
	SUB		r4, r4, #0x1
	LDMIA	r0!, {r5, r6}
	STR		r5, [r2], #0x4		 /* Data Lockdown 0-7 */
	STR		r6, [r2], #0x4		 /* Instruction Lockdown 0-7 */
	B		RESTORE_COMMON_LOCKDOWN_LOOP

RESTORE_COMMON_LOCKDOWN_EXIT:
	LDMIA	r0!, {r4, r5}
	STR		r4, [r1, #L2_LOCKDOWN_BY_LINE_EN]
	STR		r5, [r1, #L2_UNLOCK_ALL_LINES]

	/*------------------------------------------*/
	/* Address Filtering / Control / Debug		*/
	/*------------------------------------------*/
	LDMIA	r0!, {r4-r7}
	STR		r4, [r1, #L2_ADDR_FILTER_START]
	STR		r5, [r1, #L2_ADDR_FILTER_END]
	STR		r6, [r1, #L2_DEBUG_CONTROL]
	MOV		r3, r7		/* r3 = PL310 Control register */

	/*----------------------------------------------------------*/
	/* Global Setting(Control / Debug, Prefetch and Power)		*/
	/*----------------------------------------------------------*/
	LDR		r0, =ram0Pl310GlobalSettingPhys
	LDMIA	r0!, {r4-r8}
	STR		r4, [r1, #L2_AUX_CONTROL]
	STR		r5, [r1, #L2_LATENCY_CONTROL]
	STR		r6, [r1, #L2_RAM_LATENCY_CONTROL]
	STR		r7, [r1, #L2_POWER_CONTROL]
	STR		r8, [r1, #L2_PREFETCH_CONTROL]

	/*----------------------------------------------------------*/
	/* Restore PL310 Control register(Enable L2 cache) */
	/*----------------------------------------------------------*/
	STR		r3, [r1, #L2_CONTROL]
	DSB

	MOV		pc, lr

.pool

ENDPROC(restore_common_register)

/************************************************************************/
/* system(A3SM) power down												*/
/* This function is running on SDRAM */
/* After modification of this function, please check the offset. */
/* Enter with: 						*/
/* 	lr = Address of return point 	*/
/************************************************************************/
	.align	5
ENTRY(sys_powerdown)

	/*------------------------------*/
	/* Save wakeup code address		*/
	/*------------------------------*/
	LDR		r4, =ram0WakeupCodeAddr0Phys
	STR		lr, [r4]	/* Save wakeup code address	*/

	/*------------------------------*/
	/* Disable L2 cache				*/
	/*------------------------------*/
	LDR		r4, =BasePl310Phys
	MOV		r5, #0x0

SYSPD_L2DISABLE_LOOP:
	LDR		r6, [r4, #L2_CONTROL]
	AND		r6, r6, #0x1
	CMP		r6, r5
	BEQ		SYSPD_L2DISABLE_SKIP
	/* Write PL310 Control register (Disable L2 cache)	*/
	STR		r5, [r4, #L2_CONTROL]
	DSB
	B		SYSPD_L2DISABLE_LOOP

SYSPD_L2DISABLE_SKIP:

	/*--------------------------------------*/
	/* Clean and Invalidate L2 Cache		*/
	/* r4: PL310 base address(phys)			*/
	/*--------------------------------------*/

	LDR		r5, =0xFFFF		/* Set Way 0-15	*/
	/* Clean adn Invalidate L2 Cache */
	STR		r5, [r4, #L2_CLEAN_AND_INVALIDATE]

SYSPD_CLEANINVALIDATE_LOOP:
	LDR		r6, [r4, #L2_CLEAN_AND_INVALIDATE]
	ANDS	r6, r6, r5
	BNE		SYSPD_CLEANINVALIDATE_LOOP

/* PASR Setting */
	/* PASR setting for DRAM area 0*/
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]				/* r7 = SDMRACR0A*/
	LDR	r8, =OPMA_MASK
	AND	r9, r7, r8		/* r9 = SDMRACR0A.OP[15:8], MA[7:0]*/
	LDR	r4, =ram0SaveSdmracr0aPhys
	STR	r9, [r4]		/* Save SDMRACR0A.OP[15:8], MA[7:0]*/
	LDR	r4, =ram0DramPasrSettingArea0Phys
	LDR	r5, [r4]	/* r5 = PASR setting area 0*/
	CMP	r5, #0x0	/* if there is no self block bit then*/
	BEQ	SKIP_PASR_DRAM_AREA0	/* skip*/
	BIC	r7, r7, r8		/* Clear SDMRACR0A.OP[15:8], MA[7:0]*/
	ORR	r7, r7, r5		/* Set SDMRACR0A.OP[15:8], MA[7:0]*/
	STR	r7, [r6]		/* Write SDMRACR0A*/
	LDR	r4, =SdramMrwCmd0Phys /* Load address of MRW area 0*/
	MOV	r6, #0x0
	STR	r6, [r4]		/* MRW(PASR) for area 0*/
SKIP_PASR_DRAM_AREA0:

	/* PASR setting for DRAM area 1*/
	LDR	r4, =ram0DramPasrSettingArea1Phys
	LDR	r5, [r4]		/* r5 = PASR setting area 1*/
	CMP	r5, #0x0		/* if there is no self block bit then*/
	BEQ	SKIP_PASR_DRAM_AREA1	/* skip*/
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]		/* r7 = SDMRACR0A*/
	LDR	r8, =OPMA_MASK
	BIC	r7, r7, r8		/* Clear SDMRACR0A.OP[15:8], MA[7:0]*/
	ORR	r7, r7, r5		/* Set SDMRACR0A.OP[15:8], MA[7:0]*/
	STR	r7, [r6]		/* Write SDMRACR0A*/
	LDR	r4, =SdramMrwCmd1SDMRACR0Phys /* Load address of MRW area 1*/
	MOV	r6, #0x0
	STR	r6, [r4]		/* MRW(PASR) for area 1*/
SKIP_PASR_DRAM_AREA1:

	/*--------------------------------------*/
	/* Turn off Watchdog timer				*/
	/*--------------------------------------*/
	/* Check status before turn it off */
	LDR		r6, =MSTPSR4Phys
	LDR		r7, [r6]
	AND		r7, r7, #MSTP402
	CMP		r7, #MSTP402		/* 0: ON, 1: OFF */
	LDR		r6, =ram0RwdtStatusPhys
	LDREQ	r7, =0x1
	LDRNE	r7, =0x0
	STR		r7, [r6]
	BEQ		SKIP_TURN_OFF_RWDT

	LDR		r6, =RWTCNTPhys
	LDR		r7, =RWTCNT_CLEAR
	STRH	r7, [r6]

	LDR		r6, =RWDTCSRAPhys
SYSPD_LOOP_RWDT:
	LDRB	r7, [r6]
	TST		r7, #0x20
	BNE		SYSPD_LOOP_RWDT

	LDR		r6, =RMSTPCR4Phys
	LDR		r7, =SMSTPCR4Phys
	LDR		r4, [r6]
	LDR		r5, [r7]
	ORR		r4, r4, #MSTP402
	ORR		r5, r5, #MSTP402
	STR		r4, [r6]
	STR		r5, [r7]
SKIP_TURN_OFF_RWDT:

#ifdef __EXTAL1_INFO__
	/*------------------------------------------*/
	/* Log information for disabling EXTAL1	*/
	/*------------------------------------------*/

	/* Read EXTAL1 Mask Count Register (EXMSKCNT1)*/
	LDR 	r4, =EXMSKCNT1Phys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveEXMSKCNT1Phys_suspend
	STR		r5, [r6]

	/* Read EXTAL1 Clock Stop Control Register (APSCSTP)*/
	LDR 	r4, =APSCSTPPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveAPSCSTPPhys_suspend
	STR		r5, [r6]

	/* Read EXTAL1 Control Register (SYCKENMSK)*/
	LDR 	r4, =SYCKENMSKPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveSYCKENMSKPhys_suspend
	STR		r5, [r6]

	/* Read C4 Area Power Control Register (C4POWCR)*/
	LDR 	r4, =C4POWCRPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveC4POWCRPhys_suspend
	STR		r5, [r6]

	/* Read	C4 Area Power Control Register2 (PDNSEL)*/
	LDR 	r4, =PDNSELPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SavePDNSELPhys_suspend
	STR		r5, [r6]

	/* Read	Power Status Register (PSTR)*/
	LDR 	r4, =SYSC_PSTR
	LDR		r5, [r4]
	LDR		r6, =ram0SavePSTRPhys_suspend
	STR		r5, [r6]
#endif /*__EXTAL1_INFO__*/

    /*--------------------------------------*/
    /* Power Down                            */
    /*--------------------------------------*/
	LDR		r4, =SPDCRPhys	/* Load SPDCR address    */
	LDR		r5, =A3SM_PD	/* A3SM Power Down Value */
TRY_TURNOFF_A3SM:
SPDCR_CHECK_LOOP_A3SM:
	LDR		r6, [r4]
	DSB
	CMP		r6, #0x0
	BNE		SPDCR_CHECK_LOOP_A3SM
 	LDR		r6, =A3SM_PD
	STR		r6, [r4]	/* Power Down    */
	DSB
	LDR		r6, =SYSC_PSTR
	LDR		r6, [r6]
	DSB
	AND		r6, r6, r5
	CMP		r6, #0
	BNE		TRY_TURNOFF_A3SM

.pool

ENDPROC(sys_powerdown)

/********************************************/
/*	system(A3SM) power up					*/
/* This function is running on SDRAM		*/
/* Enter with: 								*/
/* 	lr = Address of return point 			*/
/********************************************/
.align	5
ENTRY(sys_powerup)

	/*------------------------------------------*/
	/* Log information of SPI Status Registers	*/
	/*------------------------------------------*/

	/*Read SPI Status Registers 0 */
	LDR		r10, =ICSPISR0Phys
	LDR		r11, [r10]

	LDR		r12, =ram0_ICSPISR0Phys
	STR		r11, [r12]

	/*Read SPI Status Registers 1 */
	LDR		r10, =ICSPISR1Phys
	LDR		r11, [r10]

	LDR		r12, =ram0_ICSPISR1Phys
	STR		r11, [r12]

#ifdef __EXTAL1_INFO__
	/*------------------------------------------*/
	/* Log information for disabling EXTAL1	*/
	/*------------------------------------------*/

	/* Read EXTAL1 Mask Count Register (EXMSKCNT1)*/
	LDR 	r4, =EXMSKCNT1Phys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveEXMSKCNT1Phys_resume
	STR		r5, [r6]

	/* Read EXTAL1 Clock Stop Control Register (APSCSTP)*/
	LDR 	r4, =APSCSTPPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveAPSCSTPPhys_resume
	STR		r5, [r6]

	/* Read EXTAL1 Control Register (SYCKENMSK)*/
	LDR 	r4, =SYCKENMSKPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveSYCKENMSKPhys_resume
	STR		r5, [r6]

	/* Read C4 Area Power Control Register (C4POWCR)*/
	LDR 	r4, =C4POWCRPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SaveC4POWCRPhys_resume
	STR		r5, [r6]

	/* Read	C4 Area Power Control Register2 (PDNSEL)*/
	LDR 	r4, =PDNSELPhys
	LDR		r5, [r4]
	LDR		r6, =ram0SavePDNSELPhys_resume
	STR		r5, [r6]

	/* Read	Power Status Register (PSTR)*/
	LDR 	r4, =SYSC_PSTR
	LDR		r5, [r4]
	LDR		r6, =ram0SavePSTRPhys_resume
	STR		r5, [r6]
#endif /*__EXTAL1_INFO__*/

	/*----------------------------------*/
	/* Invalidate L1 D-cache			*/
	/*----------------------------------*/
	L1_DataCacheInvalidateSW /* L1 D-Cache Invalidate */

	/*----------------------------------*/
	/* Turn on Watchdog timer			*/
	/*----------------------------------*/
	LDR		r6, =ram0RwdtStatusPhys
	LDR		r7, [r6]
	CMP		r7, #0x1		/* 0: enable RWDT, 1: skip */
	BEQ		SKIP_TURN_ON_RWDT2

	LDR		r6, =RMSTPCR4Phys
	LDR		r7, =SMSTPCR4Phys
	LDR		r4, [r6]
	LDR		r5, [r7]
	BIC		r4, r4, #MSTP402
	BIC		r5, r5, #MSTP402
	STR		r4, [r6]
	STR		r5, [r7]
SKIP_TURN_ON_RWDT2:

	/*--------------------------------------*/
	/* Change clock setting for wakeup		*/
	/*--------------------------------------*/

/* PASR Setting*/
	/*Release PASR setting for DRAM area 0*/
	LDR	r4, =ram0DramPasrSettingArea0Phys
	LDR	r5, [r4]	/* r5 = PASR setting*/
	CMP	r5, #0x0	/* if there is no self block bit then*/
	BEQ	SKIP_RELEASE_PASR_DRAM_AREA0 /* skip*/
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]			/* r7 = SDMRACR0A*/
	MOV	r8, #OP_MASK
	BIC	r7, r7, r8			/* Clear SDMRACR0A.OP[15:8]*/
	STR r7, [r6]			/* Write SDMRACR0A*/
	LDR	r8, =SdramMrwCmd0Phys	/* Load address of MRW area 0*/
	MOV	r9, #0x0
	STR	r9, [r8]			/* MRW (release PASR)*/
SKIP_RELEASE_PASR_DRAM_AREA0:

	/*Release PASR setting for DRAM area 1*/
	LDR	r4, =ram0DramPasrSettingArea1Phys
	LDR	r5, [r4]	/* r5 = PASR setting*/
	CMP	r5, #0x0	/* if there is no self block bit then*/
	BEQ	SKIP_RELEASE_PASR_DRAM_AREA1 /* skip*/
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]			/* r7 = SDMRACR0A*/
	MOV	r8, #OP_MASK
	BIC	r7, r7, r8			/* Clear SDMRACR0A.OP[15:8]*/
	STR r7, [r6]			/* Write SDMRACR0A*/
	LDR	r8, =SdramMrwCmd1SDMRACR0Phys	/* Load address of MRW area 1*/
	MOV	r9, #0x0
	STR	r9, [r8]	/* MRW (release PASR) for area 1*/
SKIP_RELEASE_PASR_DRAM_AREA1:
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]			/* r7 = SDMRACR0A*/
	LDR	r4, =ram0SaveSdmracr0aPhys
	LDR	r5, [r4]
	LDR	r8, =OPMA_MASK
	BIC	r7, r7, r8			/* Clear SDMRACR0A.MA/OP*/
	ORR	r7, r7, r5
	STR	r7, [r6]			/* Restore SDMRACR0A	*/

	/*----------------------------------*/
	/* Invalidate L2 cache				*/
	/* Disable L2 cache					*/
	/*----------------------------------*/
	LDR		r4, =BasePl310Phys
	MOV		r5, #0x0
SYSPU_L2DISABLE_LOOP:
	LDR		r6, [r4, #L2_CONTROL]
	AND		r6, r6, #0x1
	CMP		r6, r5
	BEQ		SYSPU_L2DISABLE_SKIP
	/* Write PL310 Control register (Disable L2 cache)	*/
	STR		r5, [r4, #L2_CONTROL]
	DSB		/* Data Synchronization Barrier	*/
	B		SYSPU_L2DISABLE_LOOP

SYSPU_L2DISABLE_SKIP:

	/*----------------------------------*/
	/* Restore L2 global setting		*/
	/*----------------------------------*/
	LDR		r0, =ram0Pl310GlobalSettingPhys
	LDR		r4, =BasePl310Phys
	LDMIA	r0!, {r5-r9}
	/* Write PL310 Auxiliary Control Register 	*/
	STR		r5, [r4, #L2_AUX_CONTROL]
	/* Write PL310 Tag RAM Latency Control Register	*/
	STR		r6, [r4, #L2_LATENCY_CONTROL]
	/* Write PL310 Data RAM Latency Control Register */
	STR		r7, [r4, #L2_RAM_LATENCY_CONTROL]
	/* Write PL310 Power Control Register	*/
	STR		r8, [r4, #L2_POWER_CONTROL]
	/* Write PL310 Prefetch control Register */
	STR		r9, [r4, #L2_PREFETCH_CONTROL]

	/*----------------------------------*/
	/* Invalidate L2 Cache				*/
	/* r4: PL310 base address(phys)		*/
	/*----------------------------------*/
	LDR		r5, =0xFFFF		/* Set Way 0-15	*/
	STR		r5, [r4, #L2_INVALIDATW_BY_WAY]

SYSPU_INVALIDATE_LOOP:
	LDR		r6, [r4, #L2_INVALIDATW_BY_WAY]
	ANDS	r6, r6, r5
	BNE		SYSPU_INVALIDATE_LOOP

	MOV		pc, lr				/* return */
.pool

ENDPROC(sys_powerup)

/****************************************/
/* Transition to WFI standby.			*/
/* This function is running on SDRAM	*/
/* Enter with: 							*/
/* 	lr = Address of return point 		*/
/****************************************/
.align	5
ENTRY(start_wfi)
	STMFD	sp!, {r4-r11, lr}

	/* Get PM SpinLock	*/
	BL PM_Spin_Lock
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status	*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r0, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_WFI
	STR		r6, [r5]
	DSB
#endif
	/* r7=0x0: legacy mode */
	LDR		r5, =CPG_LPCKCR
	LDR		r7, =CPG_LPCKCR_LEGACY
	MOV		lr, pc /* lr: Return address */
	B	xtal_though
	NOP

	/* Release PM SpinLock					*/
	BL PM_Spin_Unlock
	NOP

	WFI		/* Transition to the WFI standby state */

	/* Get PM SpinLock						*/
	BL PM_Spin_Lock
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	/* r0: CPU number						*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB
#endif

	/* Release PM SpinLock					*/
	BL PM_Spin_Unlock
	NOP

	LDMFD	sp!, {r4-r11, pc}
.pool
ENDPROC(start_wfi)




/****************************************/
/* Transition to WFI standby.			*/
/* This function is running on SDRAM	*/
/* Enter with: 							*/
/* 	lr = Address of return point 		*/
/****************************************/
.align	5
ENTRY(start_wfi2)
	STMFD	sp!, {r4-r11, lr}

	/* Get PM SpinLock						*/
	BL PM_Spin_Lock
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r0, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_WFI2
	STR		r6, [r5]
	DSB

	/* r7=0x0: legacy mode */
	/* r7=0x4: Xtal though (PLL0 Stop) */
	LDR		r7, =CPG_LPCKCR_LEGACY
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu1Status
	LDRNE	r5, =ram0Cpu0Status
	LDR		r4, [r5]

	CMP		r4, #CPUSTATUS_WFI
	BLE		START_WFI2_XTAL_THOUGH_SET

/*	LDR		r7, =CPG_LPCKCR_PLLOFF */
#else
/*	LDR		r7, =CPG_LPCKCR_PLLOFF */
	LDR		r7, =CPG_LPCKCR_LEGACY
#endif
START_WFI2_XTAL_THOUGH_SET:
	LDR		r5, =CPG_LPCKCR
	MOV		lr, pc /* lr: Return address */
	B	xtal_though
	NOP

	/* Release PM SpinLock					*/
	BL PM_Spin_Unlock
	NOP

	WFI		/* Transition to the WFI standby state */

	/* Get PM SpinLock						*/
	BL PM_Spin_Lock
	NOP

	/* r7=0x0: legacy mode */
	LDR		r5, =CPG_LPCKCR
	LDR		r7, =CPG_LPCKCR_LEGACY
	MOV		lr, pc /* lr: Return address */
	B	xtal_though
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	/* r0: CPU number						*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB
#endif

	/* Release PM SpinLock					*/
	BL PM_Spin_Unlock
	NOP

	LDMFD	sp!, {r4-r11, pc}
.pool
ENDPROC(start_wfi2)



/************************************************************/
/* Disable MMU */
/* This function is running on RAM1 */
/* Enter with: */
/*	r0 = Physical address of jump point after disabling MMU */
/************************************************************/
.align	5
ENTRY(disablemmu)
	/*------------------------------*/
	/* Disable MMU					*/
	/*------------------------------*/
	MRC		p15, 0, r4, c1, c0, 0 /* Read Control register */
	BIC		r4, r4, #0x1 /* Clear M bit */
    MCR     p15, 0, r4, c1, c0, 0 /* Write Control register (Disable MMU)*/
	MCR		p15, 0, r0, c7, c5, 6 /* Invalidate branch predictor */
	DSB
	ISB
    MCR     p15, 0, r0, c8, c7, 0 /* Invalidate entire unified TLB */
	DSB
	ISB


	MOV		pc, r0
.pool
ENDPROC(disablemmu)


/****************************************************************/
/* Transition to System Sleep(A3SM Power Off) (CPU0 MMU OFF) */
/* This function is running on SDRAM */
/****************************************************************/
.align	5
ENTRY(systemsuspend_cpu0_pa)

	/*------------------------------------------*/
	/* Sys power down	*/
	/*------------------------------------------*/
	LDR		r4, =ram1SysPowerDownPhys
	MOV		lr, pc
	MOV		pc, r4 /* Call sys_powerdown */
	NOP

	/*------------------------------------------*/
	/* Sys power up			*/
	/*------------------------------------------*/
	LDR		r4, =ram1SysPowerUpPhys
	MOV		lr, pc
	MOV		pc, r4 /* Call sys_powerup */
	NOP

	/*------------------------------------------*/
	/* Restore Common registers					*/
	/*------------------------------------------*/
	LDR		r4, =ram1RestoreCommonRegisterPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call restore_common_register() */
	NOP

	/*--------------------------------------------------------------*/
	/* Restore ARM registers		*/
	/* r1: Virtual address of next instruction after enabling MMU	*/
	/*--------------------------------------------------------------*/
	LDR		r1, =ram0SystemSuspendRestoreCPU0Phys
	LDR		r1, [r1]

	LDR		r0, =ram0MmuSetting0Phys
	LDR		r4, =ram1RestoreArmRegisterPAPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call restore_arm_register_pa() */
	NOP

.pool
ENDPROC(systemsuspend_cpu0_pa)

/********************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) (MMU OFF) */
/* This function is running on RAM1 */
/* Enter with: */
/********************************************************************/
.align	5
ENTRY(corestandby_pa)

	/* Get PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP

	LDR		r8, =CPUSTATUS_SHUTDOWN
	LDR		r4, =ram1corestandby_down_statusPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call corestandby_down_status() */
	NOP

	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r5, =ram0Cpu1StatusPhys
	LDRNE	r5, =ram0Cpu0StatusPhys
	LDR		r4, [r5]

	/* r7=0x0: legacy mode */
	/* r7=0x4: Xtal though (PLL0 Stop) */
	LDR		r5, =CPG_LPCKCRPhys
	LDR		r7, =CPG_LPCKCR_LEGACY
	CMP		r4, #CPUSTATUS_WFI
	BLE		CORESTANDBY_PA_XTAL_THOUGH_SET
/*	LDR		r7, =CPG_LPCKCR_PLLOFF */

CORESTANDBY_PA_XTAL_THOUGH_SET:
	LDR		r4, =ram1xtal_thoughPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call xtal_though() */
	NOP

	/* Release PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_UnlockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP

	/*------------------------------------------*/
	/* CPU power down */
	/*------------------------------------------*/
	ADR		r5, CORESTANDBY_WAKEUP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r6, =ram0WakeupCodeAddr0Phys
	LDRNE	r6, =ram0WakeupCodeAddr1Phys
	STR		r5, [r6] /* Set the CPU wakeup code address */
	DSB

	LDR		r6, =1023
	LDR		r5, =BaseInterruptIFPhys
	LDR		r3, [r5, #ICC_HPIR]
	AND		r3, r3, r6
	CMP		r3, r6
	BNE		CORESTANDBY_WAKEUP
	LDR		r6, =0x50000
	DSB
	WFI
1:	SUBS	r6, r6, #1
	BNE		1b

CORESTANDBY_WAKEUP:
	NOP

	/* Get PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP

	/* r7=0x0: legacy mode */
	LDR		r5, =CPG_LPCKCRPhys
	LDR		r7, =CPG_LPCKCR_LEGACY
	LDR		r4, =ram1xtal_thoughPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call xtal_though() */
	NOP

	LDR		r4, =ram1corestandby_up_statusPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call corestandby_up_status() */
	NOP

	/* Release PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_UnlockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP

    /* Invalidate L1 D-cache */
    L1_DataCacheInvalidateSW /* L1 D-Cache Invalidate */

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r0, =ram0MmuSetting0Phys
	LDRNE	r0, =ram0MmuSetting1Phys

	LDREQ	r1, =ram0CoreStandbyRestoreCPU0Phys
	LDRNE	r1, =ram0CoreStandbyRestoreCPU1Phys
	LDR		r1, [r1]

	LDR		r4, =ram1RestoreArmRegisterPAPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call restore_arm_register_pa() */
	NOP

.pool
ENDPROC(corestandby_pa)
/****************************************************************/
/* Transition to System Sleep(A3SM Power Off) (CPU1 MMU OFF) */
/* This function is running on SDRAM */
/****************************************************************/
.align	5
ENTRY(systemsuspend_cpu1_pa)

	/* Get PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP

	LDR		r8, =CPUSTATUS_HOTPLUG
	LDR		r4, =ram1corestandby_down_statusPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call corestandby_down_status() */
	NOP

	/* Release PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_UnlockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP

	/*------------------------------------------*/
	/* CPU power down */
	/*------------------------------------------*/
	LDR		r7, =ram0WakeupCodeAddr1Phys
	ADR		r4, CPU1_SUSPEND_WAKEUP_VMALLOC_EXPAND
	STR		r4, [r7]
	WFI
	NOP
	MOV	r6,#1000
	LDR	r4, =PLLECR
1:	LDR	r5, [r4]
	SUBS	r6, r6, #1
	BNE	1b
	NOP
CPU1_SUSPEND_WAKEUP_VMALLOC_EXPAND:
	NOP

	/* Get PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP

	LDR		r4, =ram1corestandby_up_statusPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call corestandby_up_status() */
	NOP

	/* Release PM SpinLock				*/
	LDR		r4, =ram1PM_Spin_UnlockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP

    /* Invalidate L1 D-cache */
    L1_DataCacheInvalidateSW /* L1 D-Cache Invalidate */

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r1, =ram0SystemSuspendRestoreCPU1Phys
	LDR		r1, [r1]

	LDR		r0, =ram0MmuSetting1Phys
	LDR		r4, =ram1RestoreArmRegisterPAPhys
	MOV		lr, pc
	MOV		pc, r4 /* Call restore_arm_register_pa() */
	NOP

.pool
ENDPROC(systemsuspend_cpu1_pa)

/*********************************************/
/* r8: */
/*		CPUSTATUS_SHUTDOWN: Corestandby */
/*		CPUSTATUS_SHUTDOWN2: Corestandby_2 */
/**********************************************/
.align	5
ENTRY(corestandby_down_status)
	/*------------------------------------------*/
	/* Set the SCU CPU power Status Register	*/
	/*------------------------------------------*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r5, =0x0003 /* CPU0 Powered-off mode */
	LDRNE	r5, =0x0300 /* CPU1 Powered-off mode */
	LDR		r6, =SCU_PWRST_ADDRPhys
	LDR		r7, [r6]
	ORR		r7, r7, r5
	STR		r7, [r6] /* Set the CPU Powered-off mode */
	DSB

	/* Update current cpu status */
	/* r4:CPU number */
	CMP		r4, #0x0
	LDREQ	r5, =ram0Cpu0StatusPhys
	LDRNE	r5, =ram0Cpu1StatusPhys
	MOV		r6, r8
	STR		r6, [r5]
	DSB

	MOV		pc, lr /* return */
.pool
ENDPROC(corestandby_down_status)

/*********************************************/
/* Update status for corestandby/corestandby_2 up */
/**********************************************/
.align	5
ENTRY(corestandby_up_status)
	/* Update current cpu status	*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r5, =ram0Cpu0StatusPhys
	LDRNE	r5, =ram0Cpu1StatusPhys
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB

	/*------------------------------------------*/
	/* Set the SCU CPU power Status Register	*/
	/*------------------------------------------*/
	/* r4:CPU number */
	CMP		r4, #0x0
	LDREQ	r5, =0x0003 /* CPU0 status clear mask */
	LDRNE	r5, =0x0300 /* CPU1 status clear mask */
	LDR		r6, =SCU_PWRST_ADDRPhys
	LDR		r7, [r6]
	BIC		r7, r7, r5
	STR		r7, [r6] /* Set the CPU Normal mode */
	DSB

	MOV		pc, lr /* return */
.pool
ENDPROC(corestandby_up_status)

/*********************************************/
/* r5: Address of XTAL though mode register */
/* r7: */
/*		0x2: Xtal though (PLL0 Oscillated)  */
/*		0x4: Xtal though (PLL0 Stop)  */
/**********************************************/
.align	5
ENTRY(xtal_though)
	LDR		r6, [r5]
#ifdef CONFIG_PM_SMP
	CMP		r6, r7
	BEQ		1f /* Race condition */
#endif
	MOV		r6, r7
	STR		r6, [r5]
	DSB
1:
	MOV		pc, lr /* return */

.pool
ENDPROC(xtal_though)

/*********************************************/
/* r5: Address of XTAL though mode register */
/*********************************************/
.align	5
ENTRY(xtal_though_restore)
	LDR		r6, [r5]
	CMP		r6, #0x0
	BEQ		1f
	MOV		r6, #0x0
	STR		r6, [r5]
	DSB
1:
	MOV		pc, lr /* return */

.pool
ENDPROC(xtal_though_restore)


/********************************************************/
/*	Transition to corestandby(A1SL0 or A1Sl1 Power Off) */
/*	Transition to corestandby_2(A2SL Power Off) */
/* This function is running on SDRAM */
/* Enter with: */
/* 	r0 = this pointer */
/* 	lr = Address of return point */
/********************************************************/
	.align	5
ENTRY(start_corestandby_2)
	STMFD	sp!, {r4-r11, lr}

	/*--------------------------------------------------------------*/
	/* Initialize the management information of backup area */
	/*--------------------------------------------------------------*/
	LDR	r4, =ram0Cpu0RegisterArea

	LDR		r4, [r4]
	LDR		r5, =hoDataArea
	MOV		r0, r4
	LDR 	r1, =0x0
	MOV		r2, r5
	BL		memset
	ADD		r6, r4, r5
	LDR		r7, =hoBackupAddr
	STR		r6, [r4, r7]

	/*--------------------------------------------------------------*/
	/* Set SYS boot address Register values at this point are */
	/*--------------------------------------------------------------*/
	LDR		r4, =SBAR
	LDR		r6, =ram1BasePhys /* Load RAM1 physical address */
	BIC	r7, r6, #0x3 /* Set RAM1 physical address & clear BTMD[1:0] */
	STR		r7, [r4]

	/*------------------------------------------*/
	/* Change domain access settings			*/
	/*------------------------------------------*/
	MRC 	p15, 0, r4, c3, c0, 0
	STMFD	sp!, {r4} /* Save original domain settings */
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0
	ISB

	/*--------------------------------------*/
	/* Jump to CoreStandby_2() on SDRAM		*/
	/*--------------------------------------*/
	MOV		lr, pc /* lr: Return address */
	B		corestandby_2 /* Call CoreStandby_2() */
	NOP

	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4} /* Load original domain access settings */
	MCR		p15, 0, r4, c3, c0, 0
	ISB

	LDMFD	sp!, {r4-r11, pc}

ENDPROC(start_corestandby_2)

/*****************************************************************/
/*	Transition to corestandby_2(A2SL Power Off) */
/* This function is running on SDRAM */
/* After modification of this function, please check the offset. */
/* Enter with: */
/* 	r0 = this pointer */
/* 	lr = Address of return point */
/******************************************************************/
	.align	5
ENTRY(corestandby_2)
	STMFD	sp!, {r4-r11, lr}

	/*------------------------------------------*/
	/* Clear WUPSFAC */
	/*------------------------------------------*/
	LDR		r4, =WUPSFAC
	LDR		r4, [r4] /* Clear WUPSFAC */

	/*------------------------------------------*/
	/* Save ARM registers						*/
	/*------------------------------------------*/
	LDR	r0, =ram0Cpu0RegisterArea /* r0: Backup area for CPU0 */
	LDR		r0, [r0]
	MOV		lr, pc /* lr: Return address */
	B		save_arm_register
	NOP

	/*------------------------------------------*/
	/* Save Common registers					*/
	/*------------------------------------------*/
	MOV		lr, pc /* lr: Return address */
	B		save_common_register
	NOP

	/*------------------------------------------------------*/
	/* r0: Backup area of resume point after MMU is enable */
	/*------------------------------------------------------*/
	LDR	r0, =ram0CoreStandby2RestoreCPU0
	ADR		r4, CORESTANDBY2_RESTORE
	STR		r4, [r0]
	DSB

	/*------------------------------------------*/
	/* Disable MMU							*/
	/*------------------------------------------*/
	/* Jump point to corestandby_pa_2 non-MMU */
	LDR		r0, =ram1CoreStandbyPA2Phys
	LDR		r4, =ram1DisableMMU
	MOV		pc, r4 /* Call disablemmu() */
	NOP

CORESTANDBY2_RESTORE:
	NOP
	MCR	p15, 0, r0, c8, c7, 0 /* Invalidate entire unified TLB */
	MCR	p15, 0, r0, c7, c5, 6 /* Invalidate branch predictor */
	DSB
	ISB

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR	r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	MOV		lr, pc /* lr: Return address */
	/* Call restore_arm_register_va */
	B		restore_arm_register_va
	NOP

	LDMFD	sp!, {r4-r11,pc}
.pool
ENDPROC(corestandby_2)

/********************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) (MMU OFF) */
/* This function is running on RAM1 */
/* Enter with: */
/********************************************************************/
.align	5
ENTRY(corestandby_pa_2)

	LDR		r8, =CPUSTATUS_SHUTDOWN2
	LDR		r4, =ram1corestandby_down_statusPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call corestandby_down_status() */
	NOP

	/* Set L2 standby mode enable */
	LDR		r4, =BasePl310Phys
	LDR		r5, [r4, #L2_POWER_CONTROL]
	ORR		r5, r5, #0x1
	STR 	r5, [r4, #L2_POWER_CONTROL]
	/* L2 Cache sync */
	MOV 	r5, #0x0
	DSB
	STR 	r5, [r4, #L2_CHACH_SYNC]
	DSB
CPUPD_CACHESYNC_LOOP:
	LDR		r5, [r4, #L2_CHACH_SYNC]
	TST		r5, #0x1
	BNE		CPUPD_CACHESYNC_LOOP

	/*------------------------------------------*/
	/* CPU power down */
	/*------------------------------------------*/
	ADR		r5, CORESTANDBY2_WAKEUP
	LDR	r6, =ram0WakeupCodeAddr0Phys
	STR		r5, [r6] /* Set the CPU wakeup code address */
	DSB

	/*--------------------------------------*/
	/* Power Down							*/
	/* CoreStandby A2SL power down	*/
	/*------------------------------------------*/
	LDR		r4, =SPDCRPhys /* Load SPDCR address */
CORE_POWERDOWN_LOOP:
SPDCR_CHECK_LOOP_A2SL:
	LDR		r5, [r4]
	DSB
	CMP		r5, #0x0
	BNE		SPDCR_CHECK_LOOP_A2SL
 	LDR		r5, =A2SLPD
	STR		r5, [r4]
	DSB
	LDR		r6, =SYSC_PSTR /* Load PSTR address */
	LDR		r6, [r6]
	DSB
	AND		r6, r6, #A2SLST
	CMP		r6, #0x0
	BNE		CORE_POWERDOWN_LOOP

CORESTANDBY2_WAKEUP:
	NOP

	LDR		r4, =ram1corestandby_up_statusPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call corestandby_up_status() */
	NOP

	/* Invalidate L1 D-cache */
	L1_DataCacheInvalidateSW /* L1 D-Cache Invalidate */

	/*------------------------------------------*/
	/* Restore Common registers					*/
	/*------------------------------------------*/
	LDR		r4, =ram1RestoreCommonRegisterPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call restore_common_register() */
	NOP

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR	r0, =ram0MmuSetting0Phys

	LDR	r1, =ram0CoreStandby2RestoreCPU0Phys
	LDR		r1, [r1]

	LDR		r4, =ram1RestoreArmRegisterPAPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call restore_arm_register_pa() */
	NOP

.pool
ENDPROC(corestandby_pa_2)
