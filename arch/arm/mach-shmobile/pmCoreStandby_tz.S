/*
 * arch/arm/mach-shmobile/pmCoreStandby_tz.S
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

#include "pm_ram0_tz.h"
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
	PM_ES2_2_And_After
	BEQ		SPIN_LOCK_ES_2_2_AFTER_GET1_SKIP

	MRC	p15, 0, r8, c0, c0, 5	/* Read MPIDR */
	MOV		r4, #0x1
	ADD		r4, r4, r8 /* Different value for each CPU */

	MRC		p15, 0, r7, c1, c0, 0	/* System Control Register */
	TST		r7, #0x1
	LDRNE	r5, =ram0SpinLockVA
	LDREQ	r5, =ram0SpinLockPAPhys
	LDR		r5, [r5]

2:
	LDR	r6, [r5]
	CMP	r6, #0x0
	BNE	2b
	DMB

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
	CMP		r11, #0			/* if(CPUID == 0) */
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
	STMFD	sp!, {r4}		/* Save original domain settings */
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0
	ISB

	/*--------------------------------------*/
	/* Jump to CoreStandby() on RAM0		*/
	/*--------------------------------------*/
	LDR		r4, =ram0CoreStandby
	MOV		lr, pc			/* lr: Return address */
	MOV		pc, r4			/* Call CoreStandby() */
	NOP

	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4}	/* Load original domain access settings	*/
	MCR		p15, 0, r4, c3, c0, 0
	ISB

	LDMFD	sp!, {r4-r11, pc}

.pool
ENDPROC(start_corestandby)

	.align	5
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
	/* Initialize the return value of SEC_HAL	*/
	/*------------------------------------------*/
	MRC	p15, 0, r11, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r11, r11, #0, #2 /* Extract the CPUID Field */
	CMP		r11, #0 /* if(CPUID == 0) */
	LDREQ	r4, =ram0SecHalReturnCpu0
	LDRNE	r4, =ram0SecHalReturnCpu1
	LDR 	r5, =0x0
	STR		r5, [r4]

	/*------------------------------------------*/
	/* Change domain access settings			*/
	/*------------------------------------------*/
	MRC 	p15, 0, r4, c3, c0, 0
	STMFD	sp!, {r4} /* Save original domain settings */
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0

	/*------------------------------------------*/
	/* Jump to systemsuspend() on RAM0			*/
	/* r4: systemsuspend() address on RAM0		*/
	/*------------------------------------------*/
	LDR		r4, =ram0SystemSuspend
	MOV		lr, pc
	MOV		pc, r4	/* Call systemsuspend()	*/
	NOP

	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4}	/* Load original domain access settings */
	MCR		p15, 0, r4, c3, c0, 0

	LDMFD	sp!, {r4-r11, pc}
.pool

ENDPROC(jump_systemsuspend)

/********************************************************************/
/* Vector table */
/* This function is running on RAM0	 (Inter connect RAM0) */
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

	/* Jump to wakeup code MMU off */
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r3, =ram0WakeupCodeAddr0Phys
	LDRNE	r3, =ram0WakeupCodeAddr1Phys
	LDR		r1, [r3]
	MOV		pc,	r1 /* Jump to WakeUp Code */

.pool
ENDPROC(ArmVector)

/*****************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) */
/* This function is running on RAM0 */
/* After modification of this function, please check the offset. */
/* Enter with: */
/* 	r0 = this pointer */
/* 	lr = Address of return point */
/****************************************************************/
	.align	5
ENTRY(corestandby)
	STMFD	sp!, {r4-r11, lr}

	/*-----------------------------------*/
	/* Clear WUPSFAC */
	/*---------------------------------*/
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
	LDR		r4, =ram0SaveArmRegister
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call save_arm_register() */
	NOP

	/* Disable D-Cache */
	DSB
	MRC		p15, 0, r4, c1, c0, 0
	BIC		r4, r4, #0x4 /* Clear C bit	*/
	MCR		p15, 0, r4, c1, c0, 0 /* Disable D-Cache */
	ISB

	/* Clean and invalidate D-Cache	*/

	L1_DataCacheCleanInvalidateSW

	/* Get PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_Lock
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r0, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_SHUTDOWN
	STR		r6, [r5]
	DSB
#endif

	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu1Status
	LDRNE	r5, =ram0Cpu0Status
	LDR		r4, [r5]
	/* r7=0x2: Xtal though (PLL0 Oscillated) */
	/* r7=0x4: Xtal though (PLL0 Stop) */
	LDR		r5, =CPG_LPCKCR
	LDR		r7, =CPG_LPCKCR_26MHz
	CMP		r4, #CPUSTATUS_WFI
	BLE		CORESTANDBY_PA_XTAL_THOUGH_SET
	LDR		r7, =CPG_LPCKCR_PLLOFF
CORESTANDBY_PA_XTAL_THOUGH_SET:
	LDR		r4, =ram0xtal_though
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call xtal_though() */
	NOP

	/* Release PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_Unlock
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP

	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r0, =ram0CoreStandbyRestoreCPU0
	LDRNE r0, =ram0CoreStandbyRestoreCPU1
		/* r0: Backup area of resume point after MMU is enable */
	ADR		r4, CORESTANDBY_RESTORE
	STR		r4, [r0]
	DSB

	/*-----------------------------------*/
	/* CPU power down preparation for wakeup code MMU off */
	/*----------------------------------*/
	ADR		r5, CORESTANDBY_WAKEUP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r6, =ram0WakeupCodeAddr0
	LDRNE	r6, =ram0WakeupCodeAddr1
	STR		r5, [r6] /* Set the CPU wakeup code address	*/
	DSB

	LDR		r0, =COMA_MODE_CORE_STANDBY /* mode */
	LDR		r1, =WAKEUP_ADDRESS_CORESTANDBY
	LDR		r2, =0x0
	LDR		r3, =0x0

	LDR		r5, =ram0SecHalCommaEntry
	LDR		r5, [r5]
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r5 /* Call sec_hal_pm_coma_entry() */
	NOP

	/* Save return value of sec_hal_pm_coma_entry() for error case only */
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r5, =ram0SecHalReturnCpu0
	LDRNE	r5, =ram0SecHalReturnCpu1
	STR		r0, [r5] /* Save return value */
	DSB

	CMP		r0, #SEC_HAL_RES_FAIL
	BEQ		CORESTANDBY_RESUME_ERROR

CORESTANDBY_WAKEUP:
	NOP

	/* Get PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP

	/* r7=0x2: Xtal though (PLL0 Oscillated) */
	LDR		r5, =CPG_LPCKCRPhys
	LDR		r7, =CPG_LPCKCR_26MHz
	LDR		r4, =ram0xtal_thoughPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call xtal_though() */
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r5, =ram0Cpu0StatusPhys
	LDRNE	r5, =ram0Cpu1StatusPhys
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB
#endif

	/* Release PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_UnlockPhys
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

	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call restore_arm_register_pa() */
	NOP

CORESTANDBY_RESUME_ERROR:
	NOP

	WFI /* For power consumption reduction */
	NOP

	/*------------------------------*/
	/* Enable D-Cache					*/
	/*------------------------------*/
	MRC		p15, 0, r4, c1, c0, 0 /* Read Control register */
	ORR		r4, r4, #0x4 /* Set C bit	*/
	/* Write Control register (Enable D-Cache) */
	MCR	p15, 0, r4, c1, c0, 0
	ISB

	B	CORESTANDBY_FINISH


CORESTANDBY_RESTORE:
	NOP
#if 0
	MCR	p15, 0, r0, c8, c7, 0 /* Invalidate entire unified TLB */
	MCR	p15, 0, r0, c7, c5, 6 /* Invalidate branch predictor */
	DSB
	ISB
#endif
	/*-----------------------------------*/
	/* Restore ARM registers 			*/
	/*---------------------------------*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	LDREQ	r0, =ram0Cpu0RegisterArea
	LDRNE	r0, =ram0Cpu1RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call restore_arm_register_va */
	NOP

CORESTANDBY_FINISH:
	NOP

	LDMFD	sp!, {r4-r11,pc}
.pool
ENDPROC(corestandby)

/************************************************/
/* Transition to System Suspend(A3SM Power Off) */
/* This function is running on RAM0 */
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
	LDR		r4, =ram0SaveArmCommonRegister
	MOV		lr, pc
	MOV		pc, r4	/* Call save_arm_common_register()	*/
	NOP

	/*--------------------------------------*/
	/* Save ARM registers					*/
	/*--------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea /* r0: backup area */
	LDR		r0, [r0]
	LDR		r4, =ram0SaveArmRegister
	MOV		lr, pc
	MOV		pc, r4	/* Call save_arm_register()	*/
	NOP

	ADR		r4, SYSTEMSUSPEND_RESTORE_CPU0
	LDR		r0, =ram0SystemSuspendRestoreCPU0
	STR		r4, [r0]

	/*------------------*/
	/* Sys power down	*/
	/*------------------*/
	LDR		r4, =ram0SysPowerDown
	MOV		lr, pc
	MOV		pc, r4		/* Call sys_powerdown	*/
	NOP

	/*------------------*/
	/* Sys power up		*/
	/*------------------*/
	LDR		r4, =ram0SysPowerUpPhys
	MOV		lr, pc
	MOV		pc, r4		/* Call sys_powerup		*/
	NOP

	/*--------------------------------------------------------------*/
	/* Restore ARM registers */
	/* r1: Virtual address of next instruction after enabling MMU	*/
	/*--------------------------------------------------------------*/
	LDR		r1, =ram0SystemSuspendRestoreCPU0Phys
	LDR		r1, [r1]

	LDR		r0, =ram0MmuSetting0Phys
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc	/* lr: Return address */
	MOV		pc, r4	/* Call restore_arm_register_pa()	*/
	NOP

SYSTEMSUSPEND_RESTORE_CPU0:
	NOP
	/* Deal with error case
	 * SEC_HAL_RES_FAIL (0x10)
	 */
	LDR		r9, =ram0SecHalReturnCpu0
	LDR		r0, [r9]

	CMP 	r0, #SEC_HAL_RES_OK
	BNE		CPU0_SEC_HAL_NOT_OK

#if 0
	/* Invalidate entire unified TLB */
	MCR		p15, 0, r0, c8, c7, 0
	/* Invalidate branch predictor */
	MCR		p15, 0, r0, c7, c5, 6
	DSB
	ISB
#endif

CPU0_SEC_HAL_NOT_OK:
	/*----------------------------------*/
	/* Restore ARM registers			*/
	/*----------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc		/* lr: Return address */
	MOV		pc, r4		/* Call restore_arm_register_va */
	NOP

	/*------------------------------*/
	/* Restore ARM common registers	*/
	/*------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmCommonRegister
	MOV		lr, pc	/* lr: Return address	*/
	MOV		pc, r4	/* Call restore_arm_common_register	*/
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
	LDR		r4, =ram0SaveArmRegister
	MOV		lr, pc		/* lr: Return address */
	MOV		pc, r4		/* Call save_arm_register()	*/
	NOP

	ADR		r4, SYSTEMSUSPEND_RESTORE_CPU1
	LDR		r0, =ram0SystemSuspendRestoreCPU1
	STR		r4, [r0]

	/* Disable D-Cache */
	DSB
	MRC		p15, 0, r4, c1, c0, 0
	BIC		r4, r4, #0x4 /* Clear C bit	*/
	MCR		p15, 0, r4, c1, c0, 0 /* Disable D-Cache */
	ISB

	/* Clean and invalidate D-Cache	*/

	L1_DataCacheCleanInvalidateSW

	/* Get PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_Lock
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call PM_Spin_Lock() */
	NOP

	/* Update current cpu status			*/

	LDR		r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_HOTPLUG
	STR		r6, [r5]
	DSB

/* Release PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_Unlock
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP


	/*------------------------------------------*/
	/* Call sec_hal_pm_coma_entry					*/
	/*------------------------------------------*/
	LDR		r4, =is_suspend_request
	LDR		r5, [r4]
	CMP		r5, #0x1
	LDREQ	r0, =COMA_MODE_SUSPEND
	LDRNE	r0, =COMA_MODE_HOTPLUG
	LDR		r1, =0xE63A2000
	LDR 	r2, =0x0
	LDR 	r3, =0x0

	LDR		r5, =ram0SecHalCommaEntry
	LDR		r4, [r5]

	ADR		r5, SECURE_SUSPEND_WAKEUP_CPU1
	LDR		r7, =ram0WakeupCodeAddr1Phys
	STR 	r5, [r7]
	MOV		pc, r4		/* CPU1 down */
	NOP

	/* Deal with error case
	 * SEC_HAL_RES_FAIL (0x10)
	 */
	LDR		r9, =ram0SecHalReturnCpu1
	STR		r0, [r9]

	CMP 	r0, #SEC_HAL_RES_OK
	BNE		CPU1_SEC_HAL_NOT_OK

SECURE_SUSPEND_WAKEUP_CPU1:

	/* Get PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call PM_Spin_Lock() */
	NOP

	/* Update current cpu status			*/
	LDR		r5, =ram0Cpu1StatusPhys
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB

/* Release PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_UnlockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call PM_Spin_UnLock() */
	NOP

	/*------------------------------------------*/
	/* Invalidate L1 D-cache					*/
	/*------------------------------------------*/

	L1_DataCacheInvalidateSW	/* L1 D-Cache Invalidate	*/

	/*--------------------------------------------------------------*/
	/* Restore ARM registers (enable MMU) */
	/* r1: Virtual address of next instruction after enabling MMU	*/
	/*--------------------------------------------------------------*/
	LDR		r1, =ram0SystemSuspendRestoreCPU1Phys
	LDR		r1, [r1]

	LDR		r0, =ram0MmuSetting1Phys
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc
	MOV		pc, r4	/* Call restore_arm_register_pa()	*/
	NOP

SYSTEMSUSPEND_RESTORE_CPU1:
	NOP
#if 0
	/* Invalidate entire unified TLB */
	MCR		p15, 0, r0, c8, c7, 0
	/* Invalidate branch predictor	*/
	MCR		p15, 0, r0, c7, c5, 6
	DSB
	ISB
#endif

CPU1_SEC_HAL_NOT_OK:
	/*------------------------------------------*/
	/* Restore ARM registers */
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu1RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc		/* lr: Return address */
	MOV		pc, r4		/* Call restore_arm_register_va */
	NOP

SYSTEMSUSPEND_EXIT:

	LDMFD	sp!, {r4-r11,pc}
.pool

ENDPROC(systemsuspend)


/****************************************/
/* Save ARM register					*/
/* This function is running on RAM0		*/
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
	/* Save SYS,ABT,UND,IRQ mode context		*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmExceptSvc
	STR		r0, [r12, r4]
	CPS		#POWER_MODESYS /* Switch to SYS mode */
	STMIA	r0!, {r13-r14} /* Save sp,lr */

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

	/*-----------------------------------*/
	/* Save VFP	*/
	/*---------------------------------*/
	MRC	p15, 0, r4, c1, c1, 2
	/* Read Non-secure Access Control Register */
	UBFX	r5, r4, #0xA, #0x2 /* Extract the CP10 and CP11 Fields */
	CMP		r5, #0x3
	BNE		SAVE_VFP_SKIP

	LDR		r4, =hoSaveArmVfp
	STR		r0, [r12, r4]
	MRC	p15, 0, r3, c1, c0, 2
		/* Read Coprocessor Access Control register */
	ORR		r4, r3, #0xF00000 /* Allows CP10 and CP11 access */
	MCR	p15, 0, r4, c1, c0, 2
		/* Write Coprocessor Access Control register */
	ISB			/* Insrtuction Synchronization Barrier */
	MRS		r4, FPEXC /* Read FPEXC register(VMRS -> MRS) */
	STR		r4, [r0], #0x4 /* Save FPEXC register */
	MOV		r5, #0x1
	MOV		r6, r5, LSL #0x1E /* Set EN bit to 1. */
	MSR		FPEXC, r6
	/* Write FPEXC register (Set global enable bit to 1) (VMSR -> MSR)*/
	MRS		r7, FPSCR /* Read FPSCR register(VMRS -> MRS) */
	STR		r7, [r0], #0x4 /* Save FPSCR register */
	VSTM	r0!, {D0-D15} /* Save VFP-D16 register */
	MRS		r5, MVFR0 /* Read MVFR0 register(VMRS -> MRS) */
	AND		r6, r5, #0xF /* Extract the RB field */
	TEQ	r6, #0x2 /* Check for Advanced SIMD/VFP-D32 support	*/
	POWER_VSTMIA(POWER_CC_EQ, 0, 16, 16) /* Save VFP-D32 */
	MSR		FPEXC, r4 /* Restore FPEXC register(VMSR -> MSR) */
	MCR	p15, 0, r3, c1, c0, 2
	/* Restore Coprocessor Access Control register */

SAVE_VFP_SKIP:
	NOP

	/*-----------------------------------*/
	/* Save System(MMU)	*/
	/*---------------------------------*/
	LDR		r1, =ram0MmuSetting0
	MRC		p15, 0, r4, c0, c0, 5
	UBFX	r5, r4, #0x0, #0x2
	LDR		r6, =saveArmMmuSettingSize
	MUL		r7, r5, r6
	ADD		r1, r1, r7
	MRC	p15, 0, r3, c10, c2, 0 /* Primary Region Remap Register	*/
	MRC	p15, 0, r4, c10, c2, 1 /* Normal Memory Remap Register */
	MRC	p15, 0, r5, c2, c0, 0 /* Translation Table Base 0 Register */
	MRC	p15, 0, r6, c2, c0, 1 /* Translation Table Base 1 Register */
	/* Translation Table Base Control Register */
	MRC	p15, 0, r7, c2, c0, 2
	MRC	p15, 0, r8, c3, c0, 0 /* Domain Access Control Register	*/
	MRC	p15, 0, r9, c13, c0, 1 /* Context ID Register */
	MRC	p15, 0, r10, c1, c0, 0 /* System Control Register */
	STMIA	r1!, {r3-r10}


	/*-----------------------------------*/
	/* Save System */
	/*---------------------------------*/
	LDR		r4,  =hoSaveArmSystem
	STR		r0, [r12, r4]
	MRC	p15, 0, r4, c12, c0, 0 /* Vector Base Address Register */
	MRC	p15, 0, r5, c7, c4, 0 /* Physical Address Register */
	MRC	p15, 0, r6, c13, c0, 2 /* User Read/Write Thread ID Register */
	MRC	p15, 0, r7, c13, c0, 3
			/* User Read-only Thread ID Register	*/
	MRC	p15, 0, r8, c13, c0, 4
			/* Privileged Only ThreadID Register	*/
	MRC	p15, 2, r9, c0, c0, 0 /* Cache Size Selection Register */

	STMIA	r0!, {r4-r9}

	/*----------------------------------------------*/
	/* Update backup start address					*/
	/*----------------------------------------------*/
	LDR		r4, =hoBackupAddr
	STR		r0, [r12, r4]

	MOV		pc, lr
.pool

ENDPROC(save_arm_register)

/**************************************************/
/* Restore ARM register(running in MMU disable)	*/
/* This function is running on RAM0	*/
/* After modification of this function, please check the offset. */
/* Enter with: */
/*	r0:backup area */
/*	lr:return address */
/**********************************************/
	.align	5
ENTRY(restore_arm_register_pa)
	/*---------------------------------------*/
	/* System(MMU) */
	/*-------------------------------------*/

	LDMIA	r0!, {r3-r10}
	MCR		p15, 0, r9, c13, c0, 1 /* Context ID Register */
	ISB
	MCR	p15, 0, r8, c3, c0, 0 /* Domain Access Control Register	*/
	/* Translation Table Base Control Register */
	MCR	p15, 0, r7, c2, c0, 2
	MCR	p15, 0, r6, c2, c0, 1 /* Translation Table Base 1 Register */
	MCR	p15, 0, r5, c2, c0, 0 /* Translation Table Base 0 Register */
	MCR	p15, 0, r4, c10, c2, 1 /* Normal Memory Remap Register */
	MCR	p15, 0, r3, c10, c2, 0 /* Primary Region Remap Register	*/
	ISB
	/* System Control Register(enable MMU, D-Cache)*/
	MCR	p15, 0, r10, c1, c0, 0
	/* Invalidate entire unified TLB */
	MCR	p15, 0, r0, c8, c7, 0
	/* Invalidate branch predictor */
	MCR	p15, 0, r0, c7, c5, 6
	DSB
	ISB
	MOV		pc, r1

.pool

ENDPROC(restore_arm_register_pa)

/****************************************************************/
/* Restore ARM register(running in MMU enable) */
/* This function is running on RAM0 */
/* After modification of this function, please check the offset.*/
/* Enter with: */
/*	r0:backup area  */
/*	lr:return address  */
/****************************************************************/
	.align	5
ENTRY(restore_arm_register_va)
	/*-----------------------------------*/
	/* Copy backup area address */
	/*---------------------------------*/
	MOV		r12, r0		/* r12 = r0(backup area) */

	/*-----------------------------------*/
	/* System */
	/*---------------------------------*/
	LDR		r4,  =hoSaveArmSystem
	LDR		r0, [r12, r4]
	LDMIA	r0!, {r4-r9}

	MCR	p15, 0, r4, c12, c0, 0 /* Vector Base Address Register */
	MCR	p15, 0, r5, c7, c4, 0 /* Physical Address Register */
	MCR	p15, 0, r6, c13, c0, 2 /* User Read/Write Thread ID Register */
	MCR	p15, 0, r7, c13, c0, 3/* User Read-only Thread ID Register */
	MCR	p15, 0, r8, c13, c0, 4 /* Privileged Only ThreadID Register */
	MCR	p15, 2, r9, c0, c0, 0 /* Cache Size Selection Register */


	/*-------------------------------*/
	/* VFP */
	/*-----------------------------*/

	MRC	p15, 0, r4, c1, c1, 2
	/* Read Non-secure Access Control Register */
	UBFX r5, r4, #0xA, #0x2 /* Extract the CP10 and CP11 Fields */
	CMP		r5, #0x3
	BNE		RESTORE_VFP_SKIP

	LDR		r4, =hoSaveArmVfp
	LDR		r0, [r12, r4]
	MRC	p15, 0, r3, c1, c0, 2
			/* Read Coprocessor Access Control register */
	ORR		r4, r3, #0xF00000 /* Allows CP10 and CP11 access */
	MCR	p15, 0, r4, c1, c0, 2
			/* Write Coprocessor Access Control register */
	ISB		/* Insrtuction Synchronization Barrier */
	MOV		r5, #0x1
	MOV		r6, r5, LSL #0x1E /* Set EN bit to 1. */
	MSR		FPEXC, r6
			/* Write FPEXC register (Set global enable bit to 1) */
	LDMIA	r0!, {r4-r5} /* Load r4:FPEXC, r5:FPSCR	*/
	VLDM	r0!, {D0-D15} /* Restore VFP-D16 register */
	MRS		r6, MVFR0 /* Read MVFR0 register */
	AND		r7, r6, #0xF /* Extract the RB field */
	TEQ	r7, #0x2 /* Check for Advanced SIMD/VFP-D32 support	*/
	POWER_VLDMIA(POWER_CC_EQ, 0, 16, 16) /* Restore VFP-D32	*/
	MSR		FPSCR, r5 /* Restore FPSCR */
	MSR		FPEXC, r4 /* Restore FPEXC */
	MCR		p15, 0, r3, c1, c0, 2
			/* Restore Coprocessor Access Control register */

RESTORE_VFP_SKIP:
	NOP

	/*---------------------------------------*/
	/* SYS, ABT, UND, IRQ mode context		*/
	/*-------------------------------------*/
	LDR		r4, =hoSaveArmExceptSvc
	LDR		r0, [r12, r4]
	CPS		#POWER_MODESYS	/* Switch to SYS mode */
	LDMIA	r0!, {r13-r14}	/* Restore sp,lr */

	MSR		SPSR_fsxc, r4	/* Restore SPSR	*/
	CPS		#POWER_MODEIRQ	/* Switch to IRQ mode */
	LDMIA	r0!, {r4, r13-r14} /* Load SPSR, restore sp,lr	*/
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
	LDMIA 	r0!, {r4-r5, r12-r14}
			/* Load SPSR,CPSR & restore r12,sp,lr	*/
	MSR		SPSR_fsxc, r5 /* Restore SPSR */
	MSR		CPSR_fsxc, r4 /* Restore CPSR */

	MOV		lr, r1 /* Restore link register	*/
	MOV		pc, lr /* return */
.pool

ENDPROC(restore_arm_register_va)

/********************************************************************/
/* Save arm common register											*/
/* This function is running on RAM0 */
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

	LDR		r1, =BaseDistrbutor
	/* Distributor Control Register (ICDDCR) */
	LDR		r6, [r1, #ICD_DCR]
	STR		r6, [r0], #0x4

	/*----------------------------------------------*/
	/* Update backup start address					*/
	/*----------------------------------------------*/
	LDR		r4, =hoBackupAddr
	STR		r0, [r12, r4]

	MOV		pc, lr	/* return	*/

.pool

ENDPROC(save_arm_common_register)

/********************************************************************/
/* Restore ARM common register	*/
/* This function is running on RAM0 */
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
	/* GIC */
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmGic
	LDR		r0, [r12, r4]

	/* Interrupt interface register */
	LDR		r1, =BaseInterruptIF
	LDMIA	r0!, {r4-r6}
	STR		r4, [r1, #ICC_PMR]
	STR		r5, [r1, #ICC_BPR]
	STR		r6, [r1, #ICC_ICR]

	/* Distributor Control rergister	*/
	LDR	r1, =BaseDistrbutor
	LDR	r6, [r0], #0x4
	STR	r6, [r1, #ICD_DCR]

	MOV		pc, lr	/* Return	*/

.pool

ENDPROC(restore_arm_common_register)

/************************************************************************/
/* system(A3SM) power down	*/
/* This function is running on RAM0 */
/* After modification of this function, please check the offset. */
/* Enter with: 						*/
/* 	lr = Address of return point 	*/
/************************************************************************/
	.align	5
ENTRY(sys_powerdown)

	/*------------------------------*/
	/* Save wakeup code address		*/
	/*------------------------------*/
	LDR		r4, =ram0WakeupCodeAddr0
	STR		lr, [r4]	/* Save wakeup code address	*/

#if 0
/* PASR Setting */
	/* PASR setting for DRAM area 0*/
	LDR	r4, =ram0DramPasrSettingArea0Phys
	LDR	r5, [r4]	/* r5 = PASR setting area 0*/
	CMP	r5, #0x0	/* if there is no self block bit then*/
	BEQ	SKIP_PASR_DRAM_AREA0	/* skip*/
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]				/* r7 = SDMRACR0A*/
	LDR	r8, =OPMA_MASK
	AND	r9, r7, r8		/* r9 = SDMRACR0A.OP[15:8], MA[7:0]*/
	LDR	r4, =ram0SaveSdmracr0aPhys
	STR	r9, [r4]		/* Save SDMRACR0A.OP[15:8], MA[7:0]*/
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
	LDR	r6, =SDMRACR1A
	LDR	r7, [r6]		/* r7 = SDMRACR1A*/
	LDR	r8, =OPMA_MASK
	AND	r9, r7, r8		/* r9 = SDMRACR1A.OP[15:8], MA[7:0]*/
	LDR	r4, =ram0SaveSdmracr1aPhys
	STR	r9, [r4]		/* Save SDMRACR1A.OP[15:8], MA[7:0]*/
	BIC	r7, r7, r8		/* Clear SDMRACR1A.OP[15:8], MA[7:0]*/
	ORR	r7, r7, r5		/* Set SDMRACR1A.OP[15:8], MA[7:0]*/
	STR	r7, [r6]		/* Write SDMRACR1A*/
	LDR	r4, =SdramMrwCmd1Phys /* Load address of MRW area 1*/
	MOV	r6, #0x0
	STR	r6, [r4]		/* MRW(PASR) for area 1*/
SKIP_PASR_DRAM_AREA1:

	/*------------------------------------------*/
	/* Change clock setting for powerdown       */
	/* Switch by Sec_hal function return value  */
	/*   0x00: Perform clock change             */
	/*   0x20: Skip clock change                */
	/*------------------------------------------*/
	LDR		r0, =ram0SecHalReturnCpu0
	LDR		r0, [r0]
	CMP		r0, #SEC_HAL_FREQ_FAIL
	BEQ		POWERDOWN_NO_CHANGE_FREQUENCY
#endif
	LDR		r5, =ram0SetClockWork
	STR		lr, [r5]
	LDR		r4, =ram0SetClockSystemSuspend
	MOV		r0, #0x0	/* Target(r0):Powerdown(0x0)	*/
	MOV		lr, pc		/* lr: Return address	*/
	MOV		pc, r4		/* Call setclock_systemsuspend()*/
	NOP
	LDR		r5, =ram0SetClockWork
	LDR		lr, [r5]
POWERDOWN_NO_CHANGE_FREQUENCY:


	/*--------------------------------------*/
	/* Turn off Watchdog timer				*/
	/*--------------------------------------*/
	/* Check status before turn it off */
	LDR		r6, =CPG_MSTPSR4
	LDR		r7, [r6]
	AND		r4, r7, #MSTP402
	CMP		r4, #MSTP402		/* 0: ON, 1: OFF */
	LDR		r6, =ram0RwdtStatus
	LDREQ	r7, =0x1
	LDRNE	r7, =0x0
	STR		r7, [r6]
	BEQ		SKIP_TURN_OFF_RWDT

	LDR		r6, =RWTCNT
	LDR		r7, =RWTCNT_CLEAR
	STRH	r7, [r6]

	LDR		r6, =RWDTCSRA
SYSPD_LOOP_RWDT:
	LDRB	r7, [r6]
	TST		r7, #0x20
	BNE		SYSPD_LOOP_RWDT

	LDR		r6, =CPG_RMSTPCR4
	LDR		r7, =CPG_SMSTPCR4
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
	LDR 	r4, =EXMSKCNT1
	LDR		r5, [r4]
	LDR		r6, =ram0SaveEXMSKCNT1_suspend
	STR		r5, [r6]

	/* Read EXTAL1 Clock Stop Control Register (APSCSTP)*/
	LDR 	r4, =APSCSTP
	LDR		r5, [r4]
	LDR		r6, =ram0SaveAPSCSTP_suspend
	STR		r5, [r6]

	/* Read EXTAL1 Control Register (SYCKENMSK)*/
	LDR 	r4, =SYCKENMSK
	LDR		r5, [r4]
	LDR		r6, =ram0SaveSYCKENMSK_suspend
	STR		r5, [r6]

	/* Read C4 Area Power Control Register (C4POWCR)*/
	LDR 	r4, =C4POWCR
	LDR		r5, [r4]
	LDR		r6, =ram0SaveC4POWCR_suspend
	STR		r5, [r6]

	/* Read	C4 Area Power Control Register2 (PDNSEL)*/
	LDR 	r4, =PDNSEL
	LDR		r5, [r4]
	LDR		r6, =ram0SavePDNSEL_suspend
	STR		r5, [r6]

	/* Read	Power Status Register (PSTR)*/
	LDR 	r4, =PSTR
	LDR		r5, [r4]
	LDR		r6, =ram0SavePSTR_suspend
	STR		r5, [r6]
#endif /*__EXTAL1_INFO__*/

	/*--------------------------------------*/
	/* Power Down							*/
	/*--------------------------------------*/
	/* Disable D-Cache */
	DSB
	MRC		p15, 0, r4, c1, c0, 0
	BIC		r4, r4, #0x4 /* Clear C bit	*/
	MCR		p15, 0, r4, c1, c0, 0 /* Disable D-Cache */
	ISB

	/* Clean and invalidate D-Cache	*/

	L1_DataCacheCleanInvalidateSW

	/*------------------------------------------*/
	/* Call sec_hal_pm_coma_entry				*/
	/*------------------------------------------*/
	LDR		r0, =COMA_MODE_SUSPEND
	LDR		r1, =0xE63A2000
	/*LDR 	r2, =0x1*/
	LDR 	r2, =0x0
	/*LDR 	r3, =0x1B000000*/
	LDR 	r3, =0x0

	LDR		r5, =ram0SecHalCommaEntry
	LDR		r4, [r5]

	MOV		lr, pc
	MOV		pc, r4		/* A3SM down */

	/* Deal with error case
	 * SEC_HAL_RES_FAIL (0x10)
	 */
	LDR		r9, =ram0SecHalReturnCpu0
	STR		r0, [r9]

	/* Turn on Watchdog timer */
	LDR		r6, =ram0RwdtStatus
	LDR		r7, [r6]
	CMP		r7, #0x1		/* 0: enable RWDT, 1: skip */
	BEQ		SKIP_TURN_ON_RWDT3
	LDR		r6, =CPG_RMSTPCR4
	LDR		r7, =CPG_SMSTPCR4
	LDR		r4, [r6]
	LDR		r5, [r7]
	BIC		r4, r4, #MSTP402
	BIC		r5, r5, #MSTP402
	STR		r4, [r6]
	STR		r5, [r7]
SKIP_TURN_ON_RWDT3:

	LDR		r0, =ram0SystemSuspendRestoreCPU0
	LDR		r4, [r0]

	MOV		pc, r4
.pool

ENDPROC(sys_powerdown)

/********************************************/
/*	system(A3SM) power up					*/
/* This function is running on RAM0			*/
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
	LDR 	r4, =PDNSEL
	LDR		r5, [r4]
	LDR		r6, =ram0SavePDNSELPhys_resume
	STR		r5, [r6]

	/* Read	Power Status Register (PSTR)*/
	LDR 	r4, =PSTR
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

	LDR		r6, =CPG_RMSTPCR4Phys
	LDR		r7, =CPG_SMSTPCR4Phys
	LDR		r4, [r6]
	LDR		r5, [r7]
	BIC		r4, r4, #MSTP402
	BIC		r5, r5, #MSTP402
	STR		r4, [r6]
	STR		r5, [r7]
SKIP_TURN_ON_RWDT2:

#if 0
	/*------------------------------------------*/
	/* Change clock setting for wakeup          */
	/* Switch by Sec_hal function return value  */
	/*   0x00: Perform clock change             */
	/*   0x20: Skip clock change                */
	/*------------------------------------------*/
	LDR		r0, =ram0SecHalReturnCpu0
	LDR		r0, [r0]
	CMP		r0, #SEC_HAL_FREQ_FAIL
	BEQ		POWERUP_NO_CHANGE_FREQUENCY
#endif
	LDR		r5, =ram0SetClockWorkPhys
	STR		lr, [r5]
	LDR		r4, =ram0SetClockSystemSuspendPhys
	MOV		r0, #0x1	/* Target(r0):Wakeup(0x1) */
	MOV		lr, pc	/* lr: Return address */
	MOV		pc, r4	/* Call setclock_systemsuspend() */
	NOP
	LDR		r5, =ram0SetClockWorkPhys
	LDR		lr, [r5]
POWERUP_NO_CHANGE_FREQUENCY:
#if 0
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
	LDR	r4, =ram0SaveSdmracr0aPhys
	LDR	r5, [r4]
	MOV	r8, #MA_MASK
	BIC	r7, r7, r8			/* Clear SDMRACR0A.MA[7:0]*/
	ORR	r7, r7, r5
	STR	r7, [r6]			/* Restore SDMRACR0A	*/
SKIP_RELEASE_PASR_DRAM_AREA0:

	/*Release PASR setting for DRAM area 1*/
	LDR	r4, =ram0DramPasrSettingArea1Phys
	LDR	r5, [r4]			/* r5 = PASR setting*/
	CMP	r5, #0x0	/* if there is no self block bit then*/
	BEQ	SKIP_RELEASE_PASR_DRAM_AREA1 /* skip*/
	LDR	r6, =SDMRACR1A
	LDR	r7, [r6]			/* r7 = SDMRACR1A*/
	MOV	r8, #OP_MASK
	BIC	r7, r7, r8			/* Clear SDMRACR1A.OP[15:8]*/
	STR r7, [r6]			/* Write SDMRACR1A*/
	LDR	r8, =SdramMrwCmd1Phys	/* Load address of MRW area 1*/
	MOV	r9, #0x0
	STR	r9, [r8]	/* MRW (release PASR) for area 1 */
	LDR	r4, =ram0SaveSdmracr1aPhys
	LDR	r5, [r4]
	MOV	r8, #MA_MASK
	BIC	r7, r7, r8	/* Clear SDMRACR1A.MA[7:0]*/
	ORR	r7, r7, r5
	STR	r7, [r6]	/* Restore SDMRACR1A	*/
SKIP_RELEASE_PASR_DRAM_AREA1:
#endif

	MOV		pc, lr		/* return	*/
.pool

ENDPROC(sys_powerup)

/********************************************************************/
/* Change the division ratio of clocks 	*/
/* This function is running on RAM0		*/
/* After modification of this function, please check the offset. */
/* Enter with: 	*/
/*  r0 = Setting Target (0:Powerdown / 1:Wakeup) */
/* 	lr = Address of return point */
/********************************************************************/
	.align	5
ENTRY(setclock_systemsuspend)
	/*
	 * Before setting clock for the Powerdown,
	 * backup the current clock setting.
	 */
	CMP		r0, #0x0	/* 0: Suspend 1: Resume */
	BNE		SETCLOCK_BACKUP_CLOCK_SETTING_SKIP

	/* Backup CPG.FRQCRA */
	LDR		r4, =CPG_FRQCRA
	LDR		r5, =ram0SetClockFrqcra
	LDR		r6, [r4]
	LDR		r7, =ram0FRQCRAMask
	LDR		r7, [r7]
	AND		r6, r6, r7
	STR		r6, [r5]

	/*
	 * Backup CPG.FRQCRB
	 * Clear FRQCRB.ZFC[3:0] if FRQCRB.ZSEL == 0
	*/
	LDR		r4, =CPG_FRQCRB
	LDR		r5, =ram0SetClockFrqcrb
	LDR		r6, [r4]
	LDR		r7, =FRQCRB_MASK
	AND		r6, r6, r7
	STR		r6, [r5]

#if 0
	/*
	 * Backup CPG.FRQCRD
	 * Clear the FRQCRD.ZB30FC[3:0] if FRQCRD.ZB30SEL == 0
	 */
	LDR		r4, =CPG_FRQCRD
	LDR		r5, =ram0SetClockFrqcrd
	LDR		r6, [r4]
	LDR		r7, =FRQCRD_MASK
	AND		r6, r6, r7
	LDR		r8, =FRQCRD_ZB30SEL_BIT
	TST		r6, r8
	LDREQ	r9, =FRQCRD_ZB30FC_MASK
	BICEQ	r6, r6, r9
	STR		r6, [r5]
#endif
SETCLOCK_BACKUP_CLOCK_SETTING_SKIP:

	/* Change the division ratio of System-CPU/SGX and Common divider */
	/* In power down (r0 == 0) */
	/*   r4: Power-save setting to FRQCRA */
	/*   r5: Power-save setting to FRQCRB */
	/* In power up (r0 == 1) */
	/*   r4: Backup setting to FRQCRA */
	/*   r5: Backup setting to FRQCRB */
	/* r6: RAM0 address storing backup setting of FRQCRA */
	/* r7: RAM0 address storing backup setting of FRQCRB */
	CMP		r0, #0x0
	LDREQ	r4, =ram0FRQCRADown
	LDREQ	r4, [r4]
	LDREQ	r5, =ram0FRQCRBDown
	LDREQ	r5, [r5]
	LDRNE	r6, =ram0SetClockFrqcraPhys
	LDRNE	r4, [r6]
	LDRNE	r7, =ram0SetClockFrqcrbPhys
	LDRNE	r5, [r7]

	/* Compare the current setting with target setting */
	/* r4: Target setting to FRQCRA */
	/* r5: Target setting to FRQCRB */
	/* r6: Physical address of FRQCRA */
	/* r7: Physical address of FRQCRB */
	CMP		r0, #0x0
	LDREQ	r6, =CPG_FRQCRA
	LDREQ	r7, =CPG_FRQCRB
	LDRNE	r6, =CPG_FRQCRAPhys
	LDRNE	r7, =CPG_FRQCRBPhys
	LDR		r8, [r6]	/* Read CPG.FRQCRA		*/
	LDREQ	r9, =ram0FRQCRAMask
	LDRNE	r9, =ram0FRQCRAMaskPhys
	LDR		r9, [r9]
	AND		r8, r8, r9	/* Clear reserve bit.	*/
	CMP		r8, r4
	BNE		SETCLOCK_CHANGE_START
	LDR		r8, [r7]	/* Read CPG.FRQCRB	*/
	LDR		r9, =FRQCRB_MASK
	AND		r8, r8, r9	/* Clear control & reserve bit. */

	CMP		r8, r5
	BEQ		SETCLOCK_CHANGE_SKIP

	/* Perform division ratio setting	*/
	/* r4: Target setting to FRQCRA		*/
	/* r5: Target setting to FRQCRB		*/
	/* r6: Physical address of FRQCRA	*/
	/* r7: Physical address of FRQCRB	*/
SETCLOCK_CHANGE_START:

#if 0
/* ---Put a wait here in suspend semaphore---*/
/**Get semaphore 1*/
	LDR	r11, =BUS_SMGPxSRCPhys
/* Get semaphore */
TRY_GET_SEMAPHORE:
	LDR		r10, [r11]
	LDR		r9, =0x1
	ORR		r10, r10, r9
	STR		r10, [r11]

/* Check status of semaphore acquisition */
	LDR		r10, [r11]
	LDR		r9, =BUS_SMGPxSRC_MSK
	AND		r10, r10, r9
	CMP		r10, #0x40000000	/* Acquire by APE-CPU0 */
	BEQ		SETCLOCK_CHANGE_START_SEMAPHORE

/* Handle failed case */
	MOV		r9, #0x10	/* Loop 16 times before re-try */
SEMAPHORE_TRY_LOOP:
	SUB 	r9, r9, #0x1
	CMP		r9, #0x0
	BNE		SEMAPHORE_TRY_LOOP
	BEQ		TRY_GET_SEMAPHORE

SETCLOCK_CHANGE_START_SEMAPHORE:
#endif

	LDR		r8, =FRQCRB_KICK
SETCLOCK_KICK_CLEAR_LOOP1:
	LDR		r9, [r7]	/* Read CPG.FRQCRB	*/
	TST		r9, r8
	BNE		SETCLOCK_KICK_CLEAR_LOOP1
	/* Set target settings */

	LDR		r3, =ram0FRQCRAMask
	LDR 	r3, [r3]
	LDR		r9, [r6]	/* Read CPG.FRQCRA */
	BIC		r9, r9, r3	/* Clear B clock */
	ORR		r4, r4, r9	/* Set B clock target value */
	STR		r4, [r6]	/* Write CPG.FRQCRA */

	LDR		r3, =FRQCRB_MASK
	LDR		r9, [r7]	/* Read CPG.FRQCRB */
	BIC		r9, r9, r3	/* Clear ZX, ZS, HP clock */
	ORR		r5, r5, r9	/* Set ZX, ZS, HP clock target value */
	ORR		r5, r5, r8	/* Set KICK bit */
	STR		r5, [r7]	/* Write CPG.FRQCRB */

SETCLOCK_KICK_CLEAR_LOOP2:
	LDR		r9, [r7]	/* Read CPG.FRQCRB	*/
	TST		r9, r8
	BNE		SETCLOCK_KICK_CLEAR_LOOP2 /* Loop until KICK==b'0 */

#if 0
/**Release semaphore 1*/
	LDR		r11, =BUS_SMGPxSRCPhys
	LDR		r10, [r11]
	BIC		r10, r10, #0x1
	STR		r10, [r11]
/* ---Put a wait here in suspend semaphore---*/
/**Release semaphore 1*/
#endif

	SETCLOCK_CHANGE_SKIP:
#if 0
	/*
	/* Change the DDR clock divider
	/* In power down (r0 == 0)
	 *   r4: Power-save setting to FRQCRD
	 * In power up (r0 == 1)
	 *   r4: Backup setting to FRQCRD
	 * r6: RAM0 address storing backup setting of FRQCRD
	 */
	CMP		r0, #0x0
	LDREQ	r4, =POWERDOWN_FRQCRD
	LDRNE	r6, =ram0SetClockFrqcrdPhys
	LDRNE	r4, [r6]

	/*
	 * Compare the current setting with target setting
	 * r4: Target setting to FRQCRD
	 * r5: Physical address of FRQCRD
	 * r6: Current setting of FRQCRD
	 */
	LDR		r5, =CPG_FRQCRDPhys
	LDR		r6, [r5]
	LDR		r7, =FRQCRD_MASK
	AND		r6, r6, r7	/* Clear control and reserve bit. */
	LDR		r7, =FRQCRD_ZB30SEL_BIT
	TST		r6, r7		/* if (FRQCRD.ZB30SEL == 0) */
	LDREQ	r8, =FRQCRD_ZB30FC_MASK
	/* If FRQCRD.ZB30SEL is b'0, clear the FRQCRD.ZB30FC[3:0]. */
	BICEQ	r6, r6, r8

	/* Perform division ratio setting
	 * r4: Set setting for FRQCRD.
	 *		ZB3_HIGHSPEED	: 0x00000000	(1/2)
	 *		ZB3_MIDSPEED	: 0x00000010	(1/4)
	 *		ZB3_LOWSPEED	: 0x00000014	(1/16)
	 */
	CMP		r6, r4
	BEQ		SETCLOCK_ZB3CHANGE_SKIP
	/*
	 * Confirm direction of ZB3 change
	 * (based on status of APSR.Z,APSR.C bits of above CMP instruction)
	 * r6=1: MID -> HI, LOW -> HI, LOW -> MID	(Clk UP)
	 * r6=0: HI -> MID, HI -> LOW, MID -> LOW	(Clk DOWN)
	 */
	MOVHI	r6, #0x1
	MOVLS	r6, #0x0

	/*
	 * For cases MID -> HI, LOW -> HI, LOW -> MID
	 * Set wait setting
	 * r4: Target setting to FRQCRD
	 * r5: Physical address of FRQCRD
	 * r6: Target direction of ZB3 speed
	 */
	CMP		r6, #0x0
	BEQ		SETCLOCK_BEFORE_WAITSETTING_SKIP

	/* Clk up cases MID/HI */

	/* SBSC dummy read */
	CMP		r0, #0x0
	LDREQ	r7, =ram0SBSC_SDCR0AIOremap
	LDREQ	r7, [r7]
	LDRNE	r7, =SBSC_SDCR0APhys
	LDR		r8, =SDCR0A_DUMMY_READ
SETCLOCK_SBSC_DUMMYREAD_LOOP1:
	LDR		r9, [r7]	/* SBSC dummy read	*/
	TST		r9, r8
	BEQ		SETCLOCK_SBSC_DUMMYREAD_LOOP1
	/* Wait setting for HI or MID speed */
	MOV		r9, #0x7	/* Number of Wait setting registers */
	ADR		r10, SETCLOCK_WAIT_CONTROL_REGISTER
	LDR		r7, =ZB3_HIGHSPEED
	CMP		r4, r7		/* If target setting is HI speed */
	ADREQ	r11, SETCLOCK_WAIT_CONTROL_HI_VAL
	ADRNE	r11, SETCLOCK_WAIT_CONTROL_MID_VAL
SETCLOCK_BEFORE_WAITSETTING_LOOP:
	LDR		r7, [r10], #0x4
	LDR		r8, [r11], #0x4
	STR		r8, [r7]
	SUBS	r9, r9, #0x1
	BNE		SETCLOCK_BEFORE_WAITSETTING_LOOP
SETCLOCK_BEFORE_WAITSETTING_SKIP:

	/* Perform division ratio setting 		*/
	/* r4: Target setting to FRQCRD			*/
	/* r5: Physical address of FRQCRD		*/
	/* r6: Target direction of ZB3x speed	*/
	/* SBSC dummy read */
	CMP		r0, #0x0
	LDREQ	r7, =ram0SBSC_SDCR0AIOremap
	LDREQ	r7, [r7]
	LDRNE	r7, =SBSC_SDCR0APhys
	LDR		r8, =SDCR0A_DUMMY_READ
SETCLOCK_SBSC_DUMMYREAD_LOOP2:
	LDR		r9, [r7]
	TST		r9, r8
	BEQ		SETCLOCK_SBSC_DUMMYREAD_LOOP2
#if 0
/**Get semaphore 2*/
/* Check semaphore */
	LDR	r11, =BUS_SMGPxSRCPhys
/* Get semaphore */
TRY_GET_SEMAPHORE_ZB3_HI:
	LDR		r10, [r11]
	LDR		r9, =0x1
	ORR		r10, r10, r9
	STR		r10, [r11]

/* Check status of semaphore acquisition */
	LDR		r10, [r11]
	LDR		r9, =BUS_SMGPxSRC_MSK
	AND		r10, r10, r9
	CMP		r10, #0x40000000	/* Acquire by APE-CPU0 */
	BEQ		SETCLOCK_CHANGE_START_SEMAPHORE_ZB3_HI

/* Handle failed case */
	MOV		r9, #0x10	/* Loop 16 times before re-try */
SEMAPHORE_TRY_LOOP_ZB3_HI:
	SUB 	r9, r9, #0x1
	CMP		r9, #0x0
	BNE		SEMAPHORE_TRY_LOOP_ZB3_HI
	BEQ		TRY_GET_SEMAPHORE_ZB3_HI

SETCLOCK_CHANGE_START_SEMAPHORE_ZB3_HI:
/**Get semaphore 2*/
#endif
/**Apply 9 steps of RMU2-E054*/
/*(1) Check the FRQCRB bit31 (KICK bit) is cleared to 0.*/

	/* Check FRQCRB.KICK bit before setting for FRQCRD */
	LDR		r7, =FRQCRB_KICK
	LDR		r8, =CPG_FRQCRBPhys
SETCLOCK_KICK_CLEAR_LOOP3:
	LDR		r9, [r8]
	TST		r9, r7
	BNE		SETCLOCK_KICK_CLEAR_LOOP3

/*Check HI speed setting or not*/
	AND		r9, r4, #0x10	/* Read CPG.FRQCRD.ZB30SEL bit */
	CMP		r9, #0x0	/* Check division ratio is 1/2 or not */
	BNE		WA_SKIP		/* NE: Not 1/2 division */

/*(2) Set 0x00008000 in FRQCRD.*/
	LDR		r9, =CPG_FRQCRD_E054_1 /*if EQ: */
	STR		r9, [r5]

/*(3) Set FRQCRB bit31 (KICK bit) = 1*/
	LDR		r9, [r8]		/* Read CPG.FRQCRB */
	ORR		r9, r9, r7		/* Set KICK bit */
	STR		r9, [r8]		/* Write CPG.FRQCRB */

/*(4) Wait until FRQCRB bit31 (KICK bit) is cleared to 0 */
/* (the division setting is completed).*/
	/* Check FRQCRB.KICK bit after setting for FRQCRD */
SETCLOCK_KICK_CLEAR_LOOP4:
	LDR		r9, [r8]
	TST		r9, r7
	BNE		SETCLOCK_KICK_CLEAR_LOOP4

/*(5) Set 0xE61500C0.bit 14= 0 (CKSCR.ZB3MODE)*/
	LDR		r9, =CPG_CKSCRPhys
	LDR		r10, [r9]
	BIC		r10, r10, #0x4000
	STR		r10, [r9]

/*(6) Set 0x00008004 in FRQCRD*/
	LDR		r9, =CPG_FRQCRD_E054_2
	STR		r9, [r5]

/*(7) Set FRQCRB bit31 (KICK bit) = 1*/
	LDR		r9, [r8]	/* Read CPG.FRQCRB */
	ORR		r9, r9, r7	/* Set KICK bit */
	STR		r9, [r8]	/* Write CPG.FRQCRB */

/*(8) Wait until FRQCRB bit31 (KICK bit) is cleared to 0 */
/* (the division setting is completed).*/
	/* Check FRQCRB.KICK bit after setting for FRQCRD */
SETCLOCK_KICK_CLEAR_LOOP5:
	LDR		r9, [r8]
	TST		r9, r7
	BNE		SETCLOCK_KICK_CLEAR_LOOP5

/*(9) Set 0xE61500C0.bit 14= 1 (CKSCR.ZB3MODE)*/
	LDR		r9, =CPG_CKSCRPhys
	LDR		r10, [r9]
	ORR		r10, r10, #0x4000
	STR		r10, [r9]

	B		WA_END

WA_SKIP:

	STR	r4, [r5]	/* Write CPG.FRQCRD */

/* Set FRQCRB bit31 (KICK bit) = 1*/
	LDR		r9, [r8]	/* Read CPG.FRQCRB */
	ORR		r9, r9, r7	/* Set KICK bit */
	STR		r9, [r8]	/* Write CPG.FRQCRB */
/* Wait until FRQCRB bit31 (KICK bit) is cleared to 0 */
/* (the division setting is completed).*/
	/* Check FRQCRB.KICK bit after setting for FRQCRD */
SETCLOCK_KICK_CLEAR_LOOP6:
	LDR		r9, [r8]
	TST		r9, r7
	BNE		SETCLOCK_KICK_CLEAR_LOOP6

WA_END:
	/* SBSC dummy read */
	CMP		r0, #0x0
	LDREQ	r7, =ram0SBSC_SDCR0AIOremap
	LDREQ	r7, [r7]
	LDRNE	r7, =SBSC_SDCR0APhys
	LDR		r8, =SDCR0A_DUMMY_READ
SETCLOCK_SBSC_DUMMYREAD_LOOP3:
	LDR		r9, [r7]
	TST		r9, r8
	BEQ		SETCLOCK_SBSC_DUMMYREAD_LOOP3

	/* Wait setting (HI -> MID, HI -> LOW, MID -> LOW) */
	/* r4: Set value to FRQCRD				*/
	/* r6: Target direction of ZB3x speed	*/
	CMP		r6, #0x0
	BNE		SETCLOCK_AFTER_WAITSETTING_SKIP

	/* Clk down to MID/LOW */

	/* Wait setting for MID or LOW speed */
	MOV		r9, #0x7	/* Number of Wait setting registers */
	ADR		r10, SETCLOCK_WAIT_CONTROL_REGISTER
	LDR		r7,=ZB3_MIDSPEED
	CMP		r4, r7
	ADREQ	r11, SETCLOCK_WAIT_CONTROL_MID_VAL
	ADRNE	r11, SETCLOCK_WAIT_CONTROL_LOW_VAL
SETCLOCK_AFTER_WAITSETTING_LOOP:
	LDR		r7, [r10], #0x4
	LDR		r8, [r11], #0x4
	STR		r8, [r7]
	SUBS	r9, r9, #0x1
	BNE		SETCLOCK_AFTER_WAITSETTING_LOOP
	/* SBSC dummy read  */
	CMP		r0, #0x0
	LDREQ	r7, =ram0SBSC_SDCR0AIOremap
	LDREQ	r7, [r7]
	LDRNE	r7, =SBSC_SDCR0APhys
	LDR		r8, =SDCR0A_DUMMY_READ
SETCLOCK_SBSC_DUMMYREAD_LOOP4:
	LDR		r9, [r7]
	TST		r9, r8
	BEQ		SETCLOCK_SBSC_DUMMYREAD_LOOP4
SETCLOCK_AFTER_WAITSETTING_SKIP:

#if 0
/**Release semaphore 2*/
	LDR		r11, =BUS_SMGPxSRCPhys
	LDR		r10, [r11]
	BIC		r10, r10, #0x1
	STR		r10, [r11]
/**Release semaphore 2*/
#endif

SETCLOCK_ZB3CHANGE_SKIP:
#endif

	MOV		pc, lr		/* return	*/

#if 0
SETCLOCK_WAIT_CONTROL_REGISTER:

.word	ram0SBSC_SDWCRC0AIOremap, ram0SBSC_SDWCRC1AIOremap, \
		ram0SBSC_SDWCRC2AIOremap, ram0SBSC_SDWCR00AIOremap, \
		ram0SBSC_SDWCR01AIOremap, ram0SBSC_SDWCR10AIOremap, \
		ram0SBSC_SDWCR11AIOremap

/* ZB3:520.0MHz */
SETCLOCK_WAIT_CONTROL_HI_VAL:
.word	0x70A84115, 0x4320CD48, 0x00BB2E19, 0x51528909,\
		0x001F030E, 0x51528909, 0x001F030E

/* ZB3:195.0MHz */
SETCLOCK_WAIT_CONTROL_MID_VAL:
.word	0x70884111, 0x190C491B, 0x00461109, 0x10810303,\
		0x000B0109, 0x10810303, 0x000B0109

/* ZB3:65.0MHz*/
SETCLOCK_WAIT_CONTROL_LOW_VAL:
.word	0x70884111, 0x08043909, 0x00170503, 0x10208202,\
		0x00030109, 0x10208202, 0x00030109
#endif
.pool
ENDPROC(setclock_systemsuspend)

/**********************************************************/
/* Transition to WFI standby. */
/* This function is running on SDRAM */
/* Enter with: */
/* 	lr = Address of return point */
/********************************************************/
.align	5
ENTRY(start_wfi)
	STMFD	sp!, {r4-r11, lr}

	/* Get PM SpinLock */
	BL PM_Spin_Lock
	NOP

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r0, r4, #0x0, #0x2 /* Extract the CPUID Field */
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_WFI
	STR		r6, [r5]
	DSB
#endif
	/* r7=0x2: Xtal though (PLL0 Oscillated) */
	LDR		r5, =CPG_LPCKCR
	LDR		r7, =CPG_LPCKCR_26MHz
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
#if 0
	/* r7=0x2: Xtal though (PLL0 Oscillated) */
	LDR		r5, =CPG_LPCKCR
	LDR		r7, =CPG_LPCKCR_26MHz
	MOV		lr, pc /* lr: Return address */
	B	xtal_though
	NOP
#endif
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

/**********************************************************/
/* Transition to WFI2. */
/* This function is running on SDRAM */
/* Enter with: */
/* 	lr = Address of return point */
/********************************************************/
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

	/* r7=0x2: Xtal though (PLL0 Oscillated) */
	/* r7=0x4: Xtal though (PLL0 Stop) */
	LDR		r7, =CPG_LPCKCR_26MHz
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu1Status
	LDRNE	r5, =ram0Cpu0Status
	LDR		r4, [r5]

	CMP		r4, #CPUSTATUS_WFI
	BLE		START_WFI2_XTAL_THOUGH_SET

	LDR		r7, =CPG_LPCKCR_PLLOFF
#else
	LDR		r7, =CPG_LPCKCR_PLLOFF
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

	/* r7=0x2: Xtal though (PLL0 Oscillated) */
	LDR		r5, =CPG_LPCKCR
	LDR		r7, =CPG_LPCKCR_26MHz
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

/********************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) */
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
#if 0
	LDR		r4, =ram0Cpu1Status
	LDR		r4, [r4]
	CMP		r4, #CPUSTATUS_HOTPLUG
	BNE		CORESTANDBY_REDIRECT

	LDR		r4, =ram0CoreStandby_2
	MOV		lr, pc			/* lr: Return address */
	MOV		pc, r4			/* Call CoreStandby_2() */
	NOP
	B		CORESTANDY2_RESUME_CONTINUE
CORESTANDBY_REDIRECT:
	NOP
	/*--------------------------------------*/
	/* Jump to CoreStandby() on SDRAM		*/
	/*--------------------------------------*/
	LDR		r4, =ram0CoreStandby
	MOV		lr, pc			/* lr: Return address */
	MOV		pc, r4			/* Call CoreStandby() */
	NOP
CORESTANDY2_RESUME_CONTINUE:
	NOP
#else
	LDR		r4, =ram0CoreStandby_2
	MOV		lr, pc			/* lr: Return address */
	MOV		pc, r4			/* Call CoreStandby() */
	NOP
#endif
	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4} /* Load original domain access settings */
	MCR		p15, 0, r4, c3, c0, 0
	ISB

	LDMFD	sp!, {r4-r11, pc}
.pool
ENDPROC(start_corestandby_2)

/*****************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) */
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
	LDR		r4, =ram0SaveArmRegister
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call save_arm_register() */
	NOP

	/* Disable D-Cache */
	DSB
	MRC		p15, 0, r4, c1, c0, 0
	BIC		r4, r4, #0x4 /* Clear C bit	*/
	MCR		p15, 0, r4, c1, c0, 0 /* Disable D-Cache */
	ISB

	/* Clean and invalidate D-Cache	*/

	L1_DataCacheCleanInvalidateSW

#if 0 /* Only CPU0 is running...*/
	/* Get PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_Lock
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP
#endif

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	LDR	r5, =ram0Cpu0Status
	LDR		r6, =CPUSTATUS_SHUTDOWN2
	STR		r6, [r5]
	DSB
#endif
	/* Turn off PLL1 */
#if 0
	/* Condition to turn off PLL1 */
	LDR		r4, =CPG_MSTPSR1
	LDR		r4, [r4]
	BIC		r4, r4, #MSTPST1_PLL1_MASK
	CMP		r4, #MSTPST1_PLL1
	BNE		CORESTANDBY2_SET_PLL1_C4

	LDR		r4, =CPG_MSTPSR2
	LDR		r4, [r4]
	BIC		r4, r4, #MSTPST2_PLL1_MASK
	CMP		r4, #MSTPST2_PLL1
	BNE		CORESTANDBY2_SET_PLL1_C4

	LDR		r4, =CPG_MSTPSR3
	LDR		r4, [r4]
	BIC		r4, r4, #MSTPST3_PLL1_MASK
	CMP		r4, #MSTPST3_PLL1
	BNE		CORESTANDBY2_SET_PLL1_C4

	LDR		r4, =CPG_MSTPSR4
	LDR		r4, [r4]
	LDR		r5, =MSTPST4_PLL1_MASK
	BIC		r4, r4, r5
	LDR		r5, =MSTPST4_PLL1
	CMP		r4, r5
	BEQ		CORESTANDBY2_SET_PLL1_C4_SKIP

CORESTANDBY2_SET_PLL1_C4:
	LDR		r4, =CPG_PLL1STPCR
	LDR		r5, [r4]
	LDR		r6, =C4STP
	ORR		r5, r5, r6
	STR		r5, [r4]
	DSB

CORESTANDBY2_SET_PLL1_C4_SKIP:
	NOP
#endif

#if 0 /* Only CPU0 is running...*/

	/* Release PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_Unlock
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP
#endif
	LDR	r0, =ram0CoreStandby2RestoreCPU0
		/* r0: Backup area of resume point after MMU is enable */
	ADR		r4, CORESTANDBY2_RESTORE
	STR		r4, [r0]
	DSB

	/*-----------------------------------*/
	/* CPU power down preparation for wakeup code MMU off */
	/*----------------------------------*/
	ADR		r5, CORESTANDBY2_WAKEUP
	LDR	r6, =ram0WakeupCodeAddr0
	STR		r5, [r6] /* Set the CPU wakeup code address	*/
	DSB

	LDR		r0, =COMA_MODE_CORE_STANDBY_2 /* mode */
	/* LDR		r0, =COMA_MODE_CORE_STANDBY /* mode temp., fix after */
	LDR		r1, =WAKEUP_ADDRESS_CORESTANDBY
	LDR		r2, =0x0
	LDR		r3, =0x0

	LDR		r5, =ram0SecHalCommaEntry
	LDR		r5, [r5]
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r5 /* Call sec_hal_pm_coma_entry() */
	NOP

	/* Save return value of sec_hal_pm_coma_entry() for error case only */
	LDR	r5, =ram0SecHalReturnCpu0
	STR		r0, [r5] /* Save return value */
	DSB

	CMP		r0, #SEC_HAL_RES_FAIL
	BEQ		CORESTANDBY2_RESUME_ERROR

CORESTANDBY2_WAKEUP:
	NOP
#if 0 /* Only CPU0 is running...*/
	/* Get PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_LockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_Lock() */
	NOP
#endif

#if 0
	/* Turn on PLL1 */
	LDR		r4, =CPG_PLL1STPCRPhys
	LDR		r5, [r4]
	LDR		r6, =C4STP
	BIC		r5, r5, r6
	STR		r5, [r4]
	DSB
#endif

#ifdef CONFIG_PM_SMP
	/* Update current cpu status			*/
	LDR	r5, =ram0Cpu0StatusPhys
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB
#endif

#if 0 /* Only CPU0 is running...*/
	/* Release PM SpinLock				*/
	LDR		r4, =ram0PM_Spin_UnlockPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call PM_Spin_UnLock() */
	NOP
#endif

	/* Invalidate L1 D-cache */

	L1_DataCacheInvalidateSW /* L1 D-Cache Invalidate */

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR	r0, =ram0MmuSetting0Phys

	LDR	r1, =ram0CoreStandby2RestoreCPU0Phys
	LDR		r1, [r1]

	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc /* lr: Return address */
	MOV		pc,r4 /* Call restore_arm_register_pa() */
	NOP

CORESTANDBY2_RESUME_ERROR:
	NOP

	WFI /* For power consumption reduction */
	NOP

	/*------------------------------*/
	/* Enable D-Cache					*/
	/*------------------------------*/
	MRC		p15, 0, r4, c1, c0, 0 /* Read Control register */
	ORR		r4, r4, #0x4 /* Set C bit	*/
	/* Write Control register (Enable D-Cache) */
	MCR	p15, 0, r4, c1, c0, 0
	ISB

	B	CORESTANDBY2_FINISH


CORESTANDBY2_RESTORE:
	NOP
#if 0
	MCR	p15, 0, r0, c8, c7, 0 /* Invalidate entire unified TLB */
	MCR	p15, 0, r0, c7, c5, 6 /* Invalidate branch predictor */
	DSB
	ISB
#endif

	/*-----------------------------------*/
	/* Restore ARM registers 			*/
	/*---------------------------------*/
	LDR	r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc /* lr: Return address */
	MOV		pc, r4 /* Call restore_arm_register_va */
	NOP

CORESTANDBY2_FINISH:
	NOP

	LDMFD	sp!, {r4-r11,pc}
.pool
ENDPROC(corestandby_2)

