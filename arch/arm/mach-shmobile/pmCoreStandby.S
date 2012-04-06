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

/************************************************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off)											*/
/* This function is running on SDRAM															*/
/*																								*/
/* Enter with: 																					*/
/* 	r0 = this pointer																			*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
ENTRY(start_corestandby)
	STMFD	sp!, {r4-r11, lr}

	MRC		p15, 0, r11, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r11, r11, #0, #2									/* Extract the CPUID Field						*/

	/*--------------------------------------------------------------*/
	/* Initialize the management information of backup area			*/
	/*--------------------------------------------------------------*/
	CMP		r11, #0												/* if(CPUID == 0)								*/
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
	/* Set SYS boot address Register values at this point are 		*/
	/*--------------------------------------------------------------*/
	LDR		r4, =SBAR
	LDR		r6, =ram0BasePhys									/* Load RAM0 physical address						*/
	BIC		r7, r6, #0x3										/* Set RAM0 physical address & clear BTMD[1:0]	*/
	STR		r7, [r4]

	/*------------------------------------------*/
	/* Change domain access settings			*/
	/*------------------------------------------*/
	MRC 	p15, 0, r4, c3, c0, 0
	STMFD	sp!, {r4}											/* Save original domain settings					*/
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0
	ISB

	/*--------------------------------------*/
	/* Jump to CoreStandby() on RAM0		*/
	/*--------------------------------------*/
	LDR		r4, =ram0CoreStandby							
	MOV		lr, pc												/* lr: Return address				*/
	MOV		pc, r4												/* Call CoreStandby()				*/
	NOP

	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4}											/* Load original domain access settings				*/
	MCR		p15, 0, r4, c3, c0, 0
	ISB

	LDMFD	sp!, {r4-r11, pc}

ENDPROC(start_corestandby)

/************************************************************************************************/
/*	Transition to System Suspend(A3SM Power Off)													*/
/* This function is running on SDRAM															*/
/*																								*/
/* Enter with: 																					*/
/* 	r0 = this pointer																			*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
ENTRY(jump_systemsuspend)
	STMFD	sp!, {r4-r11, lr}

	MRC		p15, 0, r11, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r11, r11, #0, #2									/* Extract the CPUID Field						*/

	/*--------------------------------------------------------------*/
	/* Initialize the management information of backup area 		*/
	/*--------------------------------------------------------------*/
	CMP		r11, #0												/* if(CPUID == 0)								*/
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
	STMFD	sp!, {r4}											/* Save original domain settings					*/
	LDR 	r5, =0x00FC
	ORR		r4, r4, r5
	MCR 	p15, 0, r4, c3, c0, 0

	/*-------------------------------------------*/
	/* Jump to systemsuspend() on RAM0			*/
	/*-------------------------------------------*/
	LDR		r4, =ram0SystemSuspend							/* r4: systemsuspend()function address on RAM0	*/
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call systemsuspend()							*/
	NOP
	
	/*------------------------------------------*/
	/* Restore domain access settings			*/
	/*------------------------------------------*/
	LDMFD	sp!, {r4}											/* Load original domain access settings				*/
	MCR		p15, 0, r4, c3, c0, 0

	LDMFD	sp!, {r4-r11, pc}
.pool

ENDPROC(jump_systemsuspend)

/************************************************************************************************/
/* Vector table																					*/
/* This function is running on RAM0	 (Inter connect RAM0)														*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/************************************************************************************************/
	.align	5
ENTRY(ArmVector)
	B		WAKEUP_ENTRY										/* RESET		*/
	NOP															/* UNDEF		*/
	NOP															/* SVC			*/
	NOP															/* PABT			*/
	NOP															/* DABT			*/
	NOP															/* RESERVED		*/
	NOP															/* IRQ			*/
	NOP															/* FIQ			*/
WAKEUP_EXCEPTLOOP:
	B		WAKEUP_EXCEPTLOOP
WAKEUP_ENTRY:													/* Entry point for wake up core	*/

	/* Enable the I-Cache and branch prediction */
	MCR		p15, 0, r0, c7, c5, 0								/* Invalidate I-Cache to PoC					*/
	MCR		p15, 0, r0, c7, c5, 6								/* Invalidate branch predictor					*/
	MOV		r0, #0x1800
	MCR		p15, 0, r0, c1, c0, 0								/* Enable the I-Cache and branch prediction		*/
	ISB
	MCR		p15, 0, r0, c8, c7, 0								/* Invalidate entire unified TLB				*/
	ISB

	/* Jump to wakeup code	*/
	MRC		p15, 0, r1, c0, c0, 5
	UBFX	r2, r1, #0x0, #0x2									/* Extract the CPU ID field						*/
	CMP		r2, #0
	LDREQ	r3, =ram0WakeupCodeAddr0Phys
	LDRNE	r3, =ram0WakeupCodeAddr1Phys
	LDR		r1, [r3]

	MOV		pc,	r1												/* Jump to WakeUp Code							*/
.pool
ENDPROC(ArmVector)

/************************************************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off)											*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/* 	r0 = this pointer																			*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
	.align	5
ENTRY(corestandby)
	STMFD	sp!, {r4-r11, lr}

	/*------------------------------------------*/
	/* Clear WUPSFAC							*/
	/*------------------------------------------*/
	LDR		r4, =WUPSFAC
	LDR		r4, [r4]											/* Clear WUPSFAC								*/

	/*------------------------------------------*/
	/* Save ARM registers						*/
	/*------------------------------------------*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0, #2										/* Extract the CPUID Field						*/
	CMP		r4, #0x0
	LDREQ	r0, =ram0Cpu0RegisterArea							/* r0: Backup area for CPU0						*/
	LDRNE	r0, =ram0Cpu1RegisterArea							/* r0: Backup area for CPU1						*/
	LDR		r0, [r0]
	LDR		r4, =ram0SaveArmRegister 						
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call save_arm_register()						*/
	NOP

#ifdef VMALLOC_EXPAND
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0, #2										/* Extract the CPUID Field						*/
	CMP		r4, #0x0
	LDREQ	r0, =ram0CoreStandbyRestoreCPU0
	LDRNE	r0, =ram0CoreStandbyRestoreCPU1						/* r0: Backup area of resume point after MMU is enable						*/
	ADR		r4, CORESTANDBY_RESTORE
	STR		r4, [r0]											
	DSB
	
	/*------------------------------------------*/
	/* Disable MMU							*/
	/*------------------------------------------*/
	LDR		r0, =ram0CoreStandbyPAPhys							/* Set jumpt point after disable MMU to corestandby_pa*/
	LDR		r4, =ram0DisableMMU
	MOV		pc, r4												/* Call disablemmu()							*/
	NOP
	
CORESTANDBY_RESTORE:
	NOP 
	MCR		p15, 0, r0, c8, c7, 0								/* Invalidate entire unified TLB				*/
	MCR		p15, 0, r0, c7, c5, 6								/* Invalidate branch predictor					*/
	DSB
	ISB
#endif /* VMALLOC_EXPAND */

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0, #2										/* Extract the CPUID Field						*/
	CMP		r4, #0x0
	LDREQ	r0, =ram0Cpu0RegisterArea
	LDRNE	r0, =ram0Cpu1RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA						
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_va					*/
	NOP
	
	LDMFD	sp!, {r4-r11,pc}
.pool
ENDPROC(corestandby)

/************************************************************************************************/
/*	Transition to System Suspend(A3SM Power Off)												*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/* 	r0 = this pointer																			*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
	.align	5
ENTRY(systemsuspend)
	STMFD	sp!, {r4-r11, lr}

	/* Check CPU ID					*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0, #2										/* Extract the CPUID Field						*/
	CMP		r4, #0												/* if(CPUID == 0)								*/
	BNE		CPU1_SYSTEMSUSPEND
	
	/*==============================================================*/
	/*																*/
	/* CPU 0 System Suspend											*/
	/*																*/
	/*==============================================================*/
CPU0_SYSTEMSUSPEND:

	/*------------------------------------------*/
	/* Clear WUPSFAC							*/
	/*------------------------------------------*/
	LDR		r6, =WUPSFAC
	LDR		r6, [r6]											/* Clear WUPSFAC								*/

	/*------------------------------------------*/
	/* Save ARM common registers				*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea							/* r0: backup area								*/
	LDR		r0, [r0]
	LDR		r4, =ram0SaveArmCommonRegister
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call save_arm_common_register()						*/
	NOP

	/*------------------------------------------*/
	/* Save ARM registers						*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea							/* r0: backup area								*/
	LDR		r0, [r0]
	LDR		r4, =ram0SaveArmRegister
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call save_arm_register()						*/
	NOP

	/*------------------------------------------*/
	/* Save Common registers					*/
	/*------------------------------------------*/
	LDR		r4, =ram0SaveCommonRegister
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call save_common_register()					*/
	NOP

#ifdef VMALLOC_EXPAND
	
	ADR		r4, SYSTEMSUSPEND_RESTORE_CPU0
	LDR		r0, =ram0SystemSuspendRestoreCPU0
	STR		r4, [r0]

	/*------------------------------------------*/
	/* Disable MMU							*/
	/*------------------------------------------*/
	
	LDR		r0, =ram0SystemSuspendCPU0PAPhys						/* Set jumpt point after disable MMU to systemsuspend_cpu0_pa*/
	LDR		r4, =ram0DisableMMU
	MOV		pc, r4												/* Call disablemmu()							*/
	NOP
	

SYSTEMSUSPEND_RESTORE_CPU0:
	
	MCR		p15, 0, r0, c8, c7, 0								/* Invalidate entire unified TLB				*/
	MCR		p15, 0, r0, c7, c5, 6								/* Invalidate branch predictor					*/
	DSB
	ISB
	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_va					*/
	NOP

	/*------------------------------------------*/
	/* Restore ARM common registers				*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmCommonRegister
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_va					*/
	NOP


#else /*!VMALLOC_EXPAND*/

	/*------------------------------------------*/
	/* Sys power down							*/
	/*------------------------------------------*/
	LDR		r4, =ram0SysPowerDown
	MOV		lr, pc
	MOV		pc, r4												/* Call sys_powerdown							*/
	NOP

	/*------------------------------------------*/
	/* Sys power up								*/
	/*------------------------------------------*/
	LDR		r4, =ram0SysPowerUpPhys
	MOV		lr, pc
	MOV		pc, r4												/* Call sys_powerup								*/
	NOP

	/*------------------------------------------*/
	/* Restore Common registers	(MMU off)		*/
	/*------------------------------------------*/
	LDR		r4, =ram0RestoreCommonRegisterPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_common_register()				*/
	NOP

	/*------------------------------------------*/
	/* Restore ARM registers (MMU off)			*/
	/*------------------------------------------*/
	LDR		r0, =ram0MmuSetting0Phys
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_pa()				*/
	NOP

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_va					*/
	NOP

	/*------------------------------------------*/
	/* Restore ARM common registers				*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu0RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmCommonRegister
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_common_register				*/
	NOP

#endif /*VMALLOC_EXPAND*/

	B		SYSTEMSUSPEND_EXIT

	/*==============================================================*/
	/*																*/
	/* CPU 1 System Suspend											*/
	/*																*/
	/*==============================================================*/
CPU1_SYSTEMSUSPEND:

	/*------------------------------------------*/
	/* Save ARM registers						*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu1RegisterArea							/* r0: backup area								*/
	LDR		r0, [r0]
	LDR		r4, =ram0SaveArmRegister
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call save_arm_register()						*/
	NOP
#ifdef VMALLOC_EXPAND
	
	ADR		r4, SYSTEMSUSPEND_RESTORE_CPU1
	LDR		r0, =ram0SystemSuspendRestoreCPU1
	STR		r4, [r0]

	/*------------------------------------------*/
	/* Disable MMU							*/
	/*------------------------------------------*/
	
	LDR		r0, =ram0SystemSuspendCPU1PAPhys						/* Set jumpt point after disable MMU to systemsuspend_cpu1_pa*/
	LDR		r4, =ram0DisableMMU
	MOV		pc, r4												/* Call disablemmu()							*/
	NOP
	

SYSTEMSUSPEND_RESTORE_CPU1:
	MCR		p15, 0, r0, c8, c7, 0								/* Invalidate entire unified TLB				*/
	MCR		p15, 0, r0, c7, c5, 6								/* Invalidate branch predictor					*/
	DSB
	ISB
	
#else /*!VMALLOC_EXPAND*/

	/*------------------------------------------*/
	/* CPU power down							*/
	/*------------------------------------------*/
	LDR		r7, =ram0WakeupCodeAddr1
	MOV		r4, #0x0300
	LDR		r5, =SCU_PWRST_ADDR
	LDR		r6, [r5]	
	ORR		r6, r6, r4
	STR		r6, [r5]	
	DSB
	STR		pc, [r7]	
	WFI
	NOP
	MOV		r4,	#0x0300
	LDR		r5, =SCU_PWRST_ADDR
	LDR		r6, [r5]	
	BIC		r6, r6, r4
	STR		r6, [r5]
	DSB

	/*------------------------------------------*/
	/* Invalidate L1 D-cache					*/
	/*------------------------------------------*/
	L1_DataCacheInvalidateSW									/* L1 D-Cache Invalidate						*/

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r0, =ram0MmuSetting1Phys
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_pa()				*/
	NOP

#endif /*VMALLOC_EXPAND*/

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r0, =ram0Cpu1RegisterArea
	LDR		r0, [r0]
	LDR		r4, =ram0RestoreArmRegisterVA
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_va					*/
	NOP

SYSTEMSUSPEND_EXIT:
	
	LDMFD	sp!, {r4-r11,pc}
.pool

ENDPROC(systemsuspend)


/************************************************************************************************/
/* Save ARM register																			*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/* r0:backup area																				*/
/* lr:return address																			*/
/************************************************************************************************/
	.align	5
ENTRY(save_arm_register)
	/*------------------------------*/
	/* Read backup start address	*/
	/*------------------------------*/
	MOV		r1, r0												/* r1 = backup area(manage area base address)	*/
	LDR		r2, =hoBackupAddr
	LDR		r0, [r0, r2]										/* Read backup start address					*/

	/*------------------------------*/
	/* Save SVC mode context		*/
	/*------------------------------*/
	LDR		r4, =hoSaveArmSvc
	STR		r0, [r1, r4]
	MRS		r4, CPSR											/* Get CPSR										*/
	MRS		r5, SPSR											/* Get SPSR										*/
	STMIA	r0!, {r4-r5, r12-r14}								/* Save CPSR,SPSR,r12,sp,lr						*/

	/*--------------------------------------*/
	/* Restore manage area base address		*/
	/*--------------------------------------*/
	MOV		r12, r1

	/*----------------------------------------------*/
	/* Save SYS,ABT,UND,IRQ,FIQ mode context		*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmExceptSvc
	STR		r0, [r12, r4]
	CPS		#POWER_MODESYS										/* Switch to SYS mode							*/
	STMIA	r0!, {r13-r14}										/* Save sp,lr									*/
	CPS		#POWER_MODEFIQ										/* Switch to FIQ mode							*/
	MRS		r4, SPSR											/* Get SPSR										*/
	STMIA	r0!, {r4, r8-r14}									/* Save SPSR,r8-r12,sp,lr						*/
	CPS		#POWER_MODEIRQ										/* Switch to IRQ mode							*/
	MRS		r4, SPSR											/* Get SPSR										*/
	STMIA	r0!, {r4, r13-r14}									/* Save SPSR,sp,lr								*/
	CPS		#POWER_MODEABT										/* Switch to ABT mode							*/
	MRS		r4, SPSR											/* Get SPSR										*/
	STMIA	r0!, {r4, r13-r14}									/* Save SPSR,sp,lr								*/
	CPS		#POWER_MODEUND										/* Switch to UND mode							*/
	MRS		r4, SPSR											/* Get SPSR										*/
	STMIA	r0!, {r4, r13-r14}									/* Save SPSR,sp,lr								*/
	CPS		#POWER_MODESVC										/* switch to SVC mode							*/

	/*----------------------------------------------*/
	/* Save VFP										*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmVfp
	STR		r0, [r12, r4]
	MRC		p15, 0, r3, c1, c0, 2								/* Read Coprocessor Access Control register		*/
	ORR		r4, r3, #0xF00000									/* Allows CP10 and CP11 access					*/
	MCR		p15, 0, r4, c1, c0, 2								/* Write Coprocessor Access Control register	*/
	ISB															/* Insrtuction Synchronization Barrier			*/
	MRS		r4, FPEXC											/* Read FPEXC register(VMRS -> MRS)				*/
	STR		r4, [r0], #0x4										/* Save FPEXC register							*/
	MOV		r5, #0x1
	MOV		r6, r5, LSL #0x1E									/* Set EN bit to 1.								*/
	MSR		FPEXC, r6											/* Write FPEXC register (Set global enable bit to 1)	(VMSR -> MSR)*/
	MRS		r7, FPSCR											/* Read FPSCR register(VMRS -> MRS)				*/
	STR		r7, [r0], #0x4										/* Save FPSCR register							*/
	VSTM	r0!, {D0-D15}										/* Save VFP-D16 register						*/
	MRS		r5, MVFR0											/* Read MVFR0 register(VMRS -> MRS)				*/
	AND		r6, r5, #0xF										/* Extract the RB field							*/
	TEQ		r6, #0x2											/* Check for Advanced SIMD/VFP-D32 support		*/
	POWER_VSTMIA(POWER_CC_EQ, 0, 16, 16)						/* Save VFP-D32									*/
	MSR		FPEXC, r4											/* Restore FPEXC register(VMSR -> MSR)			*/
	MCR		p15, 0, r3, c1, c0, 2								/* Restore Coprocessor Access Control register	*/


	/*----------------------------------------------*/
	/* Save System(MMU)								*/
	/*----------------------------------------------*/	
	LDR		r1, =ram0MmuSetting0
	MRC		p15, 0, r4, c0, c0, 5
	UBFX	r5, r4, #0x0, #0x2
	LDR		r6, =saveArmMmuSettingSize
	MUL		r7, r5, r6
	ADD		r1, r1, r7
	MRC		p15, 0, r3, c10, c2, 0								/* Primary Region Remap Register				*/
	MRC		p15, 0, r4, c10, c2, 1								/* Normal Memory Remap Register					*/
	MRC		p15, 0, r5, c2, c0, 0								/* Translation Table Base 0 Register			*/
	MRC		p15, 0, r6, c2, c0, 1								/* Translation Table Base 1 Register			*/
	MRC		p15, 0, r7, c2, c0, 2								/* Translation Table Base Control Register		*/
	MRC		p15, 0, r8, c3, c0, 0								/* Domain Access Control Register				*/
	MRC		p15, 0, r9, c13, c0, 1								/* Context ID Register							*/
	MRC		p15, 0, r10, c1, c0, 1								/* Auxiliary Control Register					*/
	MRC		p15, 0, r11, c1, c0, 0								/* System Control Register						*/
	STMIA	r1!, {r3-r11}	

	/*----------------------------------------------*/
	/* Save System									*/
	/*----------------------------------------------*/
	LDR		r4,  =hoSaveArmSystem
	STR		r0, [r12, r4]
	MRC		p15, 0, r4, c12, c0, 0								/* Vector Base Address Register					*/
	MRC		p15, 0, r5, c7, c4, 0								/* Physical Address Register					*/
	MRC		p15, 0, r6, c13, c0, 2								/* User Read/Write Thread ID Register			*/
	MRC		p15, 0, r7, c13, c0, 3								/* User Read-only Thread ID Register			*/
	MRC		p15, 0, r8, c13, c0, 4								/* Privileged Only ThreadID Register			*/
	MRC		p15, 2, r9, c0, c0, 0								/* Cache Size Selection Register				*/
	MRC		p15, 0, r10, c1, c0, 2								/* Coprocessor Access Control Register			*/
	MRC		p15, 0, r11, c15, c0, 0								/* Power Control Register						*/
	STMIA	r0!, {r4-r11}
	MRC		p15, 0, r4, c15, c0, 1								/* Diagnostic register							*/
	STMIA	r0!, {r4}

	/*----------------------------------------------*/
	/* Update backup start address					*/
	/*----------------------------------------------*/
	LDR		r4, =hoBackupAddr
	STR		r0, [r12, r4]

	/*----------------------------------------------*/
	/* Remove cluster								*/
	/*----------------------------------------------*/
	/* Disable D-Cache	*/
	MRC		p15, 0, r4, c1, c0, 0 
	BIC		r4, r4, #0x4										/* Clear C bit									*/
	MCR		p15, 0, r4, c1, c0, 0								/* Disable D-Cache								*/
	ISB
	
	/* Clean and invalidate D-Cache	*/
	L1_DataCacheCleanInvalidateSW

	/*----------------------------------------------*/
	/* Remove the CPU from the cluster				*/
	/*----------------------------------------------*/
	MRC		p15, 0, r4, c1, c0, 1
	BIC		r4, r4, #0x40										/* Clear SMP bit								*/
	MCR		p15, 0, r4, c1, c0, 1								/* Write the ACTLR								*/
	ISB
	
	MOV		pc, lr
.pool

ENDPROC(save_arm_register)

/************************************************************************************************/
/* Restore ARM register(running in MMU disable)													*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/*	r0:backup area																				*/
/*	r1:Jump point after enable MMU																				*/
/*	lr:return address																			*/
/************************************************************************************************/
	.align	5
ENTRY(restore_arm_register_pa)
	/*----------------------------------------------*/
	/* System(MMU)									*/
	/*----------------------------------------------*/
	LDMIA	r0!, {r3-r11}
	MCR		p15, 0, r9, c13, c0, 1								/* Context ID Register							*/
	ISB
	MCR		p15, 0, r8, c3, c0, 0								/* Domain Access Control Register				*/
	MCR		p15, 0, r7, c2, c0, 2								/* Translation Table Base Control Register		*/
	MCR		p15, 0, r6, c2, c0, 1								/* Translation Table Base 1 Register			*/
	MCR		p15, 0, r5, c2, c0, 0								/* Translation Table Base 0 Register			*/
	MCR		p15, 0, r4, c10, c2, 1								/* Normal Memory Remap Register					*/
	MCR		p15, 0, r3, c10, c2, 0								/* Primary Region Remap Register				*/
	/* Invalidate the duplicate tag store		*/
	MRC		p15, 0, r4, c0, c0, 5
	UBFX	r4, r4, #0x0, #0x2									/* Extract the CPU ID field						*/
	CMP		r4, #0x0
	LDREQ	r5, =0x0F											/* Set CPU0 ways								*/
	LDRNE	r5, =0xF0											/* Set CPU1 ways								*/
	LDR		r6, =BaseSCUPhys
	STR		r5, [r6, #SCU_INVALL]								/* Invalidate the duplicate tag store that associated with current cpu. */
	DSB

	MCR		p15, 0, r10, c1, c0, 1								/* Auxiliary Control Register(set SMP bit)		*/
	ISB
	MCR		p15, 0, r11, c1, c0, 0								/* System Control Register(enable MMU, D-Cache)	*/

#ifdef VMALLOC_EXPAND
	MOV		pc, r1
#endif /* VMALLOC_EXPAND */
	
.pool

ENDPROC(restore_arm_register_pa)

/************************************************************************************************/
/* Restore ARM register(running in MMU enable)													*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/*	r0:backup area																				*/
/*	lr:return address																			*/
/************************************************************************************************/
	.align	5
ENTRY(restore_arm_register_va)
	/*----------------------------------------------*/
	/* Copy backup area address						*/
	/*----------------------------------------------*/
	MOV		r12, r0												/* r12 = r0(backup area)						*/

	/*----------------------------------------------*/
	/* System										*/
	/*----------------------------------------------*/
	LDR		r4,  =hoSaveArmSystem
	LDR		r0, [r12, r4]
	LDMIA	r0!, {r4-r11}
	MCR		p15, 0, r4, c12, c0, 0								/* Vector Base Address Register					*/
	MCR		p15, 0, r5, c7, c4, 0								/* Physical Address Register					*/
	MCR		p15, 0, r6, c13, c0, 2								/* User Read/Write Thread ID Register			*/
	MCR		p15, 0, r7, c13, c0, 3								/* User Read-only Thread ID Register			*/
	MCR		p15, 0, r8, c13, c0, 4								/* Privileged Only ThreadID Register			*/
	MCR		p15, 2, r9, c0, c0, 0								/* Cache Size Selection Register				*/
	MCR		p15, 0, r10, c1, c0, 2								/* Coprocessor Access Control Register			*/
	MCR		p15, 0, r11, c15, c0, 0								/* Power Control Register						*/
	LDMIA	r0!, {r4}
	MCR		p15, 0, r4, c15, c0, 1								/* Diagnostic register							*/

	/*----------------------------------------------*/
	/* VFP											*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmVfp
	LDR		r0, [r12, r4]
	MRC		p15, 0, r3, c1, c0, 2								/* Read Coprocessor Access Control register		*/
	ORR		r4, r3, #0xF00000									/* Allows CP10 and CP11 access					*/
	MCR		p15, 0, r4, c1, c0, 2								/* Write Coprocessor Access Control register	*/
	ISB															/* Insrtuction Synchronization Barrier			*/
	MOV		r5, #0x1
	MOV		r6, r5, LSL #0x1E									/* Set EN bit to 1.										*/
	MSR		FPEXC, r6											/* Write FPEXC register (Set global enable bit to 1)	*/
	LDMIA	r0!, {r4-r5}										/* Load r4:FPEXC, r5:FPSCR								*/
	VLDM	r0!, {D0-D15}										/* Restore VFP-D16 register								*/
	MRS		r6, MVFR0											/* Read MVFR0 register									*/
	AND		r7, r6, #0xF										/* Extract the RB field									*/
	TEQ		r7, #0x2											/* Check for Advanced SIMD/VFP-D32 support				*/
	POWER_VLDMIA(POWER_CC_EQ, 0, 16, 16)						/* Restore VFP-D32										*/
	MSR		FPSCR, r5											/* Restore FPSCR										*/
	MSR		FPEXC, r4											/* Restore FPEXC										*/
	MCR		p15, 0, r3, c1, c0, 2								/* Restore Coprocessor Access Control register			*/

	/*----------------------------------------------*/
	/* SYS, ABT, UND, IRQ, FIQ mode context		*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmExceptSvc
	LDR		r0, [r12, r4]
	CPS		#POWER_MODESYS										/* Switch to SYS mode							*/
	LDMIA	r0!, {r13-r14}										/* Restore sp,lr								*/
	CPS		#POWER_MODEFIQ										/* Switch to FIQ mode							*/
	LDMIA	r0!, {r4, r8-r14}									/* Load SPSR, restore r8-r12,sp,lr				*/
	MSR		SPSR_fsxc, r4										/* Restore SPSR									*/
	CPS		#POWER_MODEIRQ										/* Switch to IRQ mode							*/
	LDMIA	r0!, {r4, r13-r14}									/* Load SPSR, restore sp,lr						*/
	MSR		SPSR_fsxc, r4										/* Restore SPSR									*/
	CPS		#POWER_MODEABT										/* Switch to ABT mode							*/
	LDMIA	r0!, {r4, r13-r14}									/* Load SPSR, restore sp,lr						*/
	MSR		SPSR_fsxc, r4										/* Restore SPSR									*/
	CPS		#POWER_MODEUND										/* Switch to UND mode							*/
	LDMIA	r0!, {r4, r13-r14}									/* Load SPSR, restore sp,lr						*/
	MSR		SPSR_fsxc, r4										/* Restore SPSR									*/
	CPS		#POWER_MODESVC										/* switch to SVC mode							*/

	/* SVC mode context */
	LDR		r4, =hoSaveArmSvc
	LDR		r0, [r12, r4]
	MOV		r1, lr												/* Backup link register							*/
	LDMIA	r0!, {r4-r5, r12-r14}								/* Load SPSR,CPSR & restore r12,sp,lr			*/
	MSR		SPSR_fsxc, r5										/* Restore SPSR									*/
	MSR		CPSR_fsxc, r4										/* Restore CPSR									*/

	MOV		lr, r1												/* Restore link register						*/
	MOV		pc, lr												/* return										*/
.pool

ENDPROC(restore_arm_register_va)

/************************************************************************************************/
/* Save arm common register																		*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/*	lr:return address																			*/
/************************************************************************************************/
	.align	5
ENTRY(save_arm_common_register)

	/*--------------------------------------*/
	/* Restore manage area base address		*/
	/*--------------------------------------*/
	MOV		r12, r0
	LDR		r4, =hoBackupAddr
	LDR		r0, [r12, r4]										/* Read backup start address					*/

	/*----------------------------------------------*/
	/* Save GIC										*/
	/*----------------------------------------------*/
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
	MRC		p15, 0, r3, c0, c0, 5								/* Read MP affinity register					*/
	UBFX	r3, r3, #0x0, #0x2									/* Extract the CPUID field						*/
	LDR		r1, =BaseDistrbutor
	CMP		r3, #0x0
	MOVNE	r4, #0x1											/* CPU != 0										*/
	BNE		SAVE_ARM_COMMON_LINENUMBER_EXIT
	LDR		r4, [r1, #ICD_ICTR]
	UBFX	r4, r4, #0x0, #0x5
	ADD		r4, r4, #0x1										/* CPU == 0										*/

SAVE_ARM_COMMON_LINENUMBER_EXIT:
	/* Interrupt Security register / Interrupt Set-Enable register	*/
	MOV		r5, r4
	MOV		r2, r1												/* r2 = Gic Distributor Base Address			*/

SAVE_ARM_COMMON_GIC_LOOP1:
	LDR		r6, [r2, #ICD_ISR]
	LDR		r7, [r2, #ICD_ISER]
	STMIA	r0!, {r6, r7}
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		SAVE_ARM_COMMON_GIC_LOOP1
	
	/* Interrupt Priority register / Interrupt Processor Targets register	*/
	MOV		r5, r4, LSL #0x3
	MOV		r2, r1												/* r2 = Gic Distributor Base Address			*/

SAVE_ARM_COMMON_GIC_LOOP2:
	LDR		r6, [r2, #ICD_IPR]
	LDR		r7, [r2, #ICD_IPTR]
	STMIA	r0!, {r6, r7}
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		SAVE_ARM_COMMON_GIC_LOOP2
	
	/* Interrupt Configuration register	*/
	MOV		r5, r4, LSL #0x1
	MOV		r2, r1												/* r2 = Gic Distributor Base Address			*/

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
	/* Save Timer									*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmTimer
	STR		r0, [r12, r4]

	/* CPG.PCLKCR */
	LDR		r1, =CPG_PCLKCR
	LDR		r4, [r1]
	STR		r4, [r0], #0x4

	/* Private Timer	*/
	LDR		r1, =BasePrivateAndWDT
	LDR		r4, [r1, #PRTIM_CONTROL]
	BIC		r3, r4, #0x1										/* Clear Timer Enable bit						*/
	STR		r3, [r1, #PRTIM_CONTROL]							/* Disable Private Timer						*/
	LDR		r5, [r1, #PRTIM_LOAD]
	LDR		r6, [r1, #PRTIM_COUNTER]
	LDR		r7, [r1, #PRTIM_INTSTATE]
	STMIA	r0!, {r4-r7}
	
	/* Watchdog	*/
	LDR		r4, [r1, #WDOG_CONTROL]
	BIC		r3, r4, #0x1										/* Clear Watchdog Enable bit					*/
	STR		r3, [r1, #WDOG_CONTROL]								/* Disable Watchdog								*/
	LDR		r5, [r1, #WDOG_LOAD]
	LDR		r6, [r1, #WDOG_COUNTER]
	LDR		r7, [r1, #WDOG_INTSTATE]
	STMIA	r0!, {r4-r7}

	/* Save Global Timer */
	LDR		r1, =BaseGlobalTimer
	LDR		r4, [r1, #GLTIM_CONTROL]
	BIC		r3, r4, #0xF										/* Clear Auto-incremenet/IRQ Enable/Comp Enable/Timer Enable bit	*/
	STR		r3, [r1, #GLTIM_CONTROL]							/* Stop Global timer												*/
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


	MOV		pc, lr												/* return										*/
	
.pool

ENDPROC(save_arm_common_register)

/************************************************************************************************/
/* Restore ARM common register																	*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/*	r0:backup area																				*/
/*	lr:return address																			*/
/************************************************************************************************/
	.align	5
ENTRY(restore_arm_common_register)

	/*--------------------------------------*/
	/* Restore manage area base address		*/
	/*--------------------------------------*/
	MOV		r12, r0

	/*----------------------------------------------*/
	/* Timer										*/
	/*----------------------------------------------*/
	LDR		r4, =hoSaveArmTimer
	LDR		r0, [r12, r4]

	/* CPG.PCLKCR */
	LDR		r4, [r0], #0x4
	LDR		r5, =CPG_PCLKCR
	STR		r4, [r5]

	/* Private Timer	*/
	LDR		r1, =BasePrivateAndWDT
	LDR		r4, [r1, #PRTIM_CONTROL]
	BIC		r4, r4, #0x1										/* Clear Timer Enable bit						*/
	STR		r4, [r1, #PRTIM_CONTROL]							/* Disable Private Timer						*/
	LDMIA	r0!, {r4-r7}
	STR		r5, [r1, #PRTIM_LOAD]
	ANDS	r8, r7, #0x1										/* r7 = Private Timer Interrupt Status			*/
	MOVNE	r9, #0x1
	MOVEQ	r9, r6												/* r6 = Private Timer Counter					*/
	STR		r9, [r1, #PRTIM_COUNTER]							/* Set counter									*/
	STR		r4, [r1, #PRTIM_CONTROL]

	/* Watchdog			*/
	LDR		r4, [r1, #WDOG_CONTROL]
	BIC		r4, r4, #0x1										/* Clear Watchdog Enable bit					*/
	STR		r4, [r1, #WDOG_CONTROL]								/* Disable Watchdog								*/
	LDMIA	r0!, {r4-r7}
	STR		r5, [r1, #WDOG_LOAD]
	ANDS	r8, r7, #0x1										/* r7 = Watchdog Interrupt Status				*/
	MOVNE	r9, #0x1
	MOVEQ	r9, r6												/* r6 = Watchdog Counter						*/
	STR		r9, [r1, #WDOG_COUNTER]								/* Set counter									*/
	ANDS	r8, r4, #0x8										/* r4 = Watchdog Control Register				*/
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
	BIC		r4, r4, #0xF										/* Clear Auto-incremenet/IRQ Enable/Comp Enable/Timer Enable bit	*/
	STR		r4, [r1, #GLTIM_CONTROL]							/* Stop Global timer												*/
	LDMIA	r0!, {r4-r6}
	ANDS	r9, r6, #0x1
	BEQ		RESTORE_ARM_COMMON_GLOBALTIMEREVENT_SKIP
	MOV		r7, #0x0
	STR		r7, [r1, #GLTIM_CNTLOW]
	STR		r7, [r1, #GLTIM_CNTHI]
	MOV		r8, #0x1
	STR		r8, [r1, #GLTIM_CMPLOW]
	STR		r7, [r1, #GLTIM_CMPHI]
	MOV		r7, #0x7											/* Set IRQ Enable/Comp Enable/Timer Enable bit */
	STR		r7, [r1, #GLTIM_CONTROL]
RESTORE_ARM_COMMON_GLOBALTIMEREVENT_LOOP:
	LDR		r8, [r1, #GLTIM_INTSTATE]
	ANDS	r8, r8, #0x1
	BEQ		RESTORE_ARM_COMMON_GLOBALTIMEREVENT_LOOP
	LDR		r7, [r1, #GLTIM_CONTROL]
	BIC		r7, r7, #0xF										/* Clear Auto-increment/IRQ Enable/Comp Enable/Timer Enable bit	*/
	STR		r7, [r1, #GLTIM_CONTROL]							/* Stop Global timer												*/
RESTORE_ARM_COMMON_GLOBALTIMEREVENT_SKIP:
	STR		r5, [r1, #GLTIM_AUTO]
	LDMIA	r0!, {r5-r8}
	STR		r5, [r1, #GLTIM_CMPLOW]
	STR		r6, [r1, #GLTIM_CMPHI]
	STR		r7, [r1, #GLTIM_CNTLOW]
	STR		r8, [r1, #GLTIM_CNTHI]
	STR		r4, [r1, #GLTIM_CONTROL]

	/*----------------------------------------------*/
	/* GIC											*/
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
	MRC		p15, 0, r3, c0, c0, 5								/* Read the MPIDR, MP affinity register			*/
	UBFX	r3, r3, #0x0, #0x2									/* Extract the CPUID field						*/
	LDR		r1, =BaseDistrbutor
	CMP		r3, #0x0
	MOVNE	r4, #0x1											/* CPUID != CPU0								*/
	BNE		RESTORE_ARM_COMMON_LINENUMBER_EXIT
	LDR		r4, [r1, #ICD_ICTR]
	UBFX	r4, r4, #0x0, #0x5
	ADD		r4, r4, #0x1										/* CPU == 0										*/

RESTORE_ARM_COMMON_LINENUMBER_EXIT:
	/* Interrupt Security register / Interrupt Set-Enable register	*/
	MOV		r5, r4
	MOV		r2, r1												/* r2 = Gic Distributor Base Address			*/

RESTORE_ARM_COMMON_GIC_LOOP1: 
	LDMIA	r0!, {r6, r7}
	STR		r6, [r2, #ICD_ISR]
	STR		r7, [r2, #ICD_ISER]
	ADD		r2, r2, #0x4
	SUBS	r5, r5, #0x1
	BNE		RESTORE_ARM_COMMON_GIC_LOOP1

	/* Interrupt Priority register / Interrupt Processor Targets register	*/
	MOV		r5, r4, LSL #0x3
	MOV		r2, r1												/* r2 = Gic Distributor Base Address			*/

RESTORE_ARM_COMMON_GIC_LOOP2:
	LDMIA	r0!, {r6, r7}
	STR		r6, [r2, #ICD_IPR]
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
	CMP		r3, #0x0											/* CPU == 0										*/
	BNE		RESTORE_ARM_COMMON_GIC_EXIT
	LDR		r6, [r0], #0x4
	STR		r6, [r1, #ICD_DCR]

RESTORE_ARM_COMMON_GIC_EXIT:

	MOV		pc, lr												/* Return										*/

.pool

ENDPROC(restore_arm_common_register)

/************************************************************************************************/
/* Save common register																			*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/*	lr:return address																			*/
/************************************************************************************************/
	.align	5
ENTRY(save_common_register)

	LDR		r0, =ram0CommonSetting

	/*----------------------------------------------*/
	/* Save SCU registers							*/
	/*----------------------------------------------*/
	LDR		r1, =BaseSCU
	LDR		r4, [r1, #SCU_FILTER_START_ADDR]
	LDR		r5, [r1, #SCU_FILTER_END_ADDR]
	LDR		r6, [r1, #SCU_SAC]
	LDR		r7, [r1, #SCU_SNSAC]
	LDR		r8, [r1, #SCU_CTL]
	STMIA	r0!, {r4-r8}

	/*----------------------------------------------*/
	/* Save PL310 registers							*/
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

	/* Cache Lockdown(0-7)							*/
	LDR		r2, =L2_LOCKDOWNBase
	MOV		r4, #0x8

SAVE_COMMON_LOCKDOWN_LOOP:
	CMP		r4, #0x0
	BEQ		SAVE_COMMON_LOCKDOWN_EXIT
	SUB		r4, r4, #0x1
	LDR		r5, [r2], #0x4										/* Data Lockdown 0-7							*/
	LDR		r6, [r2], #0x4										/* Instruction Lockdown 0-7						*/
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

/************************************************************************************************/
/* Restore common register																		*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/************************************************************************************************/
	.align	5
ENTRY(restore_common_register)

	LDR		r0, =ram0CommonSettingPhys

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
	STR		r4, [r1, #L2_CONTROL]								/* Write PL310 Control register (Disable L2 cache)	*/
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
	STR		r5, [r2], #0x4										/* Data Lockdown 0-7							*/
	STR		r6, [r2], #0x4										/* Instruction Lockdown 0-7						*/
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
	MOV		r3, r7												/* r3 = PL310 Control register					*/

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
	/* Restore PL310 Control register(Enable L2 cache)			*/
	/*----------------------------------------------------------*/
	STR		r3, [r1, #L2_CONTROL]
	DSB	

	MOV		pc, lr
	
.pool

ENDPROC(restore_common_register)

/************************************************************************************************/
/*	system(A3SM) power down																		*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
	.align	5
ENTRY(sys_powerdown)
#ifdef VMALLOC_EXPAND

	/*------------------------------*/
	/* Save wakeup code address		*/
	/*------------------------------*/
	LDR		r4, =ram0WakeupCodeAddr0Phys
	STR		lr, [r4]											/* Save wakeup code address						*/


#else /*!VMALLOC_EXPAND*/
	/*------------------------------*/
	/* Save wakeup code address		*/
	/*------------------------------*/
	LDR		r4, =ram0WakeupCodeAddr0
	STR		lr, [r4]											/* Save wakeup code address						*/

	/*------------------------------*/
	/* Disable MMU					*/
	/*------------------------------*/
	MRC		p15, 0, r4, c1, c0, 0								/* Read Control register						*/
	BIC		r4, r4, #0x1										/* Clear M bit									*/
	ISB	
	MCR		p15, 0, r4, c1, c0, 0								/* Write Control register (Disable MMU)			*/
	ISB	

#endif /*VMALLOC_EXPAND*/
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
	STR		r5, [r4, #L2_CONTROL]								/* Write PL310 Control register (Disable L2 cache)	*/
	DSB
	B		SYSPD_L2DISABLE_LOOP

SYSPD_L2DISABLE_SKIP:

	/*--------------------------------------*/
	/* Clean and Invalidate L2 Cache		*/
	/* r4: PL310 base address(phys)			*/
	/*--------------------------------------*/
	LDR		r5, =0xFFFF											/* Set Way 0-15									*/
	STR		r5, [r4, #L2_CLEAN_AND_INVALIDATE]					/* Clean adn Invalidate L2 Cache				*/
SYSPD_CLEANINVALIDATE_LOOP:
	LDR		r6, [r4, #L2_CLEAN_AND_INVALIDATE]
	ANDS	r6, r6, r5
	BNE		SYSPD_CLEANINVALIDATE_LOOP

/* PASR Setting */
#ifdef CONFIG_COMPACTION
	/* PASR setting for DRAM area 0*/
	LDR	r4, =ram0DramPasrSettingArea0Phys
	LDR	r5, [r4]												/* r5 = PASR setting area 0*/
	CMP	r5, #0x0												/* if there is no self block bit then*/
	BEQ	SKIP_PASR_DRAM_AREA0									/* skip*/	
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]												/* r7 = SDMRACR0A*/
	LDR	r8, =OPMA_MASK
	AND	r9, r7, r8												/* r9 = SDMRACR0A.OP[15:8], MA[7:0]*/
	LDR	r4, =ram0SaveSdmracr0aPhys
	STR	r9, [r4]												/* Save SDMRACR0A.OP[15:8], MA[7:0]*/
	BIC	r7, r7, r8												/* Clear SDMRACR0A.OP[15:8], MA[7:0]*/
	ORR	r7, r7, r5												/* Set SDMRACR0A.OP[15:8], MA[7:0]*/
	STR	r7, [r6]												/* Write SDMRACR0A*/
	LDR	r4, =SdramMrwCmd0Phys									/* Load address of MRW area 0*/
	MOV	r6, #0x0
	STR	r6, [r4]												/* MRW(PASR) for area 0*/
SKIP_PASR_DRAM_AREA0:

	
	/* PASR setting for DRAM area 1*/
	LDR	r4, =ram0DramPasrSettingArea1Phys
	LDR	r5, [r4]												/* r5 = PASR setting area 1*/
	CMP	r5, #0x0												/* if there is no self block bit then*/
	BEQ	SKIP_PASR_DRAM_AREA1									/* skip*/
	LDR	r6, =SDMRACR1A
	LDR	r7, [r6]												/* r7 = SDMRACR1A*/
	LDR	r8, =OPMA_MASK
	AND	r9, r7, r8												/* r9 = SDMRACR1A.OP[15:8], MA[7:0]*/
	LDR	r4, =ram0SaveSdmracr1aPhys
	STR	r9, [r4]												/* Save SDMRACR1A.OP[15:8], MA[7:0]*/
	BIC	r7, r7, r8												/* Clear SDMRACR1A.OP[15:8], MA[7:0]*/
	ORR	r7, r7, r5												/* Set SDMRACR1A.OP[15:8], MA[7:0]*/
	STR	r7, [r6]												/* Write SDMRACR1A*/
	LDR	r4, =SdramMrwCmd1Phys									/* Load address of MRW area 1*/
	MOV	r6, #0x0
	STR	r6, [r4]												/* MRW(PASR) for area 1*/
SKIP_PASR_DRAM_AREA1:	
#endif /* CONFIG_COMPACTION */

	/*--------------------------------------*/
	/* Change clock setting for powerdown	*/
	/*--------------------------------------*/
	LDR		r5, =ram0SetClockWorkPhys
	STR		lr, [r5]
	LDR		r4, =ram0SetClockSystemSuspendPhys
	MOV		r0, #0x0											/* Target(r0):Powerdown(0x0)						*/
	/*MOV		lr, pc	*/											/* lr: Return address								*/
	/*MOV		pc, r4 */												/* Call setclock_systemsuspend()						*/
	NOP
	LDR		r5, =ram0SetClockWorkPhys
	LDR		lr, [r5]
	
	/*--------------------------------------*/
	/* Turn off Watchdog timer				*/
	/*--------------------------------------*/

	LDR		r6, =RWTCNTPhys
	LDR		r7, =RWTCNT_CLEAR
	STRH	r7, [r6]

	LDR		r6, =RWDTCSRAPhys
SYSPD_LOOP_RWDT:
	LDRB	r7, [r6]
	TST		r7, #0x20
	BNE		SYSPD_LOOP_RWDT

	LDR		r6, =CPG_RMSTPCR4Phys
	LDR		r7, =CPG_SMSTPCR4Phys
	LDR		r4, [r6]
	LDR		r5, [r7]
	ORR		r4, r4, #MSTP402
	ORR		r5, r5, #MSTP402
	STR		r4, [r6]
	STR		r5, [r7]

	/*--------------------------------------*/
	/* Turn off PLL0						*/
	/*--------------------------------------*/
	LDR		r4, =CPG_PLLECRPhys
	LDR		r5, [r4]											/* Read PLLECR									*/
	BIC		r5, r5, #CPG_PLL0E								/* Clear PLL0E									*/
	STR		r5, [r4]											/* Turn off PLL0									*/
SYSPD_LOOP_TURNOFF_PLL0:
	LDR		r5, [r4]											/* Read PLLECR									*/
	TST		r5, #CPG_PLL0ST									/* Check PLLECR.PLL0ST							*/
	BNE		SYSPD_LOOP_TURNOFF_PLL0							/* Loop until operation is completed			*/

	/*--------------------------------------*/
	/* Power Down							*/
	/*--------------------------------------*/
	LDR		r4, =SPDCRPhys										/* Load SPDCR address							*/
	LDR		r5, =A3SM_PD								/* A3SM Power Down Value */
	LDR		r6, [r4]
	ORR		r5, r6, r5
	STR		r5, [r4]											/* Power Down									*/
SYSPD_POWERDOWN_LOOP:
	B		SYSPD_POWERDOWN_LOOP
.pool

ENDPROC(sys_powerdown)

/************************************************************************************************/
/*	system(A3SM) power up																		*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
	.align	5
ENTRY(sys_powerup)

	/*----------------------------------*/
	/* Turn on PLL0						*/
	/*----------------------------------*/
	LDR		r4, =CPG_PLLECRPhys
	LDR		r5, [r4]
	TST		r5, #CPG_PLL0ST									/* Check PLLECR.PLL0ST							*/
	BNE		SYSPU_TURNON_PLL0_SKIP								/* if (PLL0 == turned off)						*/
	ORR		r5, r5, #CPG_PLL0E
	STR		r5, [r4]											/* Turn on PLL0									*/
SYSPU_TURNON_PLL0_LOOP:
	LDR		r5, [r4]
	TST		r5, #CPG_PLL0ST									/* Check PLLECR.PLL0ST							*/
	BEQ		SYSPU_TURNON_PLL0_LOOP								/* Loop until operation is completed			*/
SYSPU_TURNON_PLL0_SKIP:
	NOP

	/*----------------------------------*/
	/* Invalidate L1 D-cache			*/
	/*----------------------------------*/
	L1_DataCacheInvalidateSW									/* L1 D-Cache Invalidate						*/

	/*--------------------------------------*/
	/* Turn on Watchdog timer				*/
	/*--------------------------------------*/	
	LDR		r6, =CPG_RMSTPCR4Phys
	LDR		r7, =CPG_SMSTPCR4Phys
	LDR		r4, [r6]
	LDR		r5, [r7]
	BIC		r4, r4, #MSTP402
	BIC		r5, r5, #MSTP402
	STR		r4, [r6]
	STR		r5, [r7]

	/*--------------------------------------*/
	/* Change clock setting for wakeup		*/
	/*--------------------------------------*/
	LDR		r5, =ram0SetClockWorkPhys
	STR		lr, [r5]
	LDR		r4, =ram0SetClockSystemSuspendPhys
	MOV		r0, #0x1											/* Target(r0):Wakeup(0x1)							*/
	/*MOV		lr, pc	*/											/* lr: Return address								*/
	/*MOV		pc, r4	*/											/* Call setclock_systemsuspend()						*/
	NOP
	LDR		r5, =ram0SetClockWorkPhys
	LDR		lr, [r5]
	
/* PASR Setting*/
#ifdef CONFIG_COMPACTION
	/*Release PASR setting for DRAM area 0*/
	LDR	r4, =ram0DramPasrSettingArea0Phys
	LDR	r5, [r4]												/* r5 = PASR setting*/
	CMP	r5, #0x0												/* if there is no self block bit then*/
	BEQ	SKIP_RELEASE_PASR_DRAM_AREA0							/* skip*/
	LDR	r6, =SDMRACR0A
	LDR	r7, [r6]												/* r7 = SDMRACR0A*/
	MOV	r8, #OP_MASK
	BIC	r7, r7, r8												/* Clear SDMRACR0A.OP[15:8]*/
	STR r7, [r6]												/* Write SDMRACR0A*/
	LDR	r8, =SdramMrwCmd0Phys									/* Load address of MRW area 0*/
	MOV	r9, #0x0
	STR	r9, [r8]												/* MRW (release PASR)*/
	LDR	r4, =ram0SaveSdmracr0aPhys
	LDR	r5, [r4]
	MOV	r8, #MA_MASK
	BIC	r7, r7, r8												/* Clear SDMRACR0A.MA[7:0]*/
	ORR	r7, r7, r5
	STR	r7, [r6]												/* Restore SDMRACR0A	*/
SKIP_RELEASE_PASR_DRAM_AREA0:
	
	/*Release PASR setting for DRAM area 1*/	
	LDR	r4, =ram0DramPasrSettingArea1Phys
	LDR	r5, [r4]												/* r5 = PASR setting*/
	CMP	r5, #0x0												/* if there is no self block bit then*/
	BEQ	SKIP_RELEASE_PASR_DRAM_AREA1							/* skip*/
	LDR	r6, =SDMRACR1A
	LDR	r7, [r6]												/* r7 = SDMRACR1A*/
	MOV	r8, #OP_MASK
	BIC	r7, r7, r8												/* Clear SDMRACR1A.OP[15:8]*/
	STR r7, [r6]												/* Write SDMRACR1A*/
	LDR	r8, =SdramMrwCmd1Phys									/* Load address of MRW area 1*/
	MOV	r9, #0x0
	STR	r9, [r8]												/* MRW (release PASR) for area 1*/
	LDR	r4, =ram0SaveSdmracr1aPhys
	LDR	r5, [r4]
	MOV	r8, #MA_MASK
	BIC	r7, r7, r8												/* Clear SDMRACR1A.MA[7:0]*/
	ORR	r7, r7, r5
	STR	r7, [r6]												/* Restore SDMRACR1A	*/	
SKIP_RELEASE_PASR_DRAM_AREA1:
#endif /* CONFIG_COMPACTION	*/
	
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
	STR		r5, [r4, #L2_CONTROL]								/* Write PL310 Control register (Disable L2 cache)	*/
	DSB															/* Data Synchronization Barrier						*/
	B		SYSPU_L2DISABLE_LOOP

SYSPU_L2DISABLE_SKIP:

	/*----------------------------------*/
	/* Restore L2 global setting		*/
	/*----------------------------------*/
	LDR		r0, =ram0Pl310GlobalSettingPhys
	LDR		r4, =BasePl310Phys
	LDMIA	r0!, {r5-r9}
	STR		r5, [r4, #L2_AUX_CONTROL]							/* Write PL310 Auxiliary Control Register 			*/
	STR		r6, [r4, #L2_LATENCY_CONTROL]						/* Write PL310 Tag RAM Latency Control Register		*/
	STR		r7, [r4, #L2_RAM_LATENCY_CONTROL]					/* Write PL310 Data RAM Latency Control Register	*/
	STR		r8, [r4, #L2_POWER_CONTROL]							/* Write PL310 Power Control Register				*/
	STR		r9, [r4, #L2_PREFETCH_CONTROL]						/* Write PL310 Prefetch control Register			*/

	/*----------------------------------*/
	/* Invalidate L2 Cache				*/
	/* r4: PL310 base address(phys)		*/
	/*----------------------------------*/
	LDR		r5, =0xFFFF											/* Set Way 0-15										*/
	STR		r5, [r4, #L2_INVALIDATW_BY_WAY] 

SYSPU_INVALIDATE_LOOP:
	LDR		r6, [r4, #L2_INVALIDATW_BY_WAY]
	ANDS	r6, r6, r5
	BNE		SYSPU_INVALIDATE_LOOP

	MOV		pc, lr												/* return											*/
.pool

ENDPROC(sys_powerup)

/************************************************************************************************/
/* Change the division ratio of System-CPU/SGX, Common and DDR clock divider					*/
/* This function is running on RAM0															*/
/* After modification of this function, please check the offset.								*/
/*																								*/
/* Enter with: 																					*/
/*  r0 = Setting Target (0:Powerdown / 1:Wakeup)												*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
	.align	5
ENTRY(setclock_systemsuspend)

	MOV		pc, lr												/* return										*/
.pool
ENDPROC(setclock_systemsuspend)

/************************************************************************************************/
/* Transition to WFI standby.																	*/
/* This function is running on SDRAM															*/
/*																								*/
/* Enter with: 																					*/
/* 	lr = Address of return point 																*/
/************************************************************************************************/
.align	5
ENTRY(start_wfi)
	STMFD	sp!, {r4-r11, lr}

	/* Get PM SpinLock						*/
	PM_Spin_Lock

	/* Update current cpu status			*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register					*/
	UBFX	r0, r4, #0x0, #0x2									/* Extract the CPUID Field							*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_WFI
	STR		r6, [r5]
	DSB
	/* Z clk change							*/
	/* r0:CPU number						*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu1Status
	LDRNE	r5, =ram0Cpu0Status
	LDR		r4, [r5]
	CMP		r4, #CPUSTATUS_SHUTDOWN
	BNE		STRWFI_ZCLKCNG_SKIP
	LDR		r5, =CPG_FRQCRB
	LDR		r6, [r5]
	LDR		r7, =FRRCRB_ZCLK_MASK
	LDR		r8, =ram0CpuClock
	AND		r4, r6, r7
	STR		r4, [r8]											/* Save Z clk setting								*/
	DSB
	LDR		r6, =FRQCRB_KICK
STRWFI_KICK_CLEAR_LOOP1:
	LDR		r4, [r5]
	TST		r4, r6
	BNE		STRWFI_KICK_CLEAR_LOOP1
	BIC		r4, r4, r7											/* Clear FRQCRB.ZSEL, ZFC[3:0] field				*/
	LDR		r8, =FRRCRB_ZCLK_1_48
	ORR		r4, r4, r8											/* Set Z clk = 1/48									*/
	ORR		r4, r4, r6											/* Set KICK bit										*/
	STR		r4, [r5]											/* Activate the Z clk setting						*/
STRWFI_KICK_CLEAR_LOOP2:
	LDR		r4, [r5]
	TST		r4, r6
	BNE		STRWFI_KICK_CLEAR_LOOP2
STRWFI_ZCLKCNG_SKIP:
	NOP

	/* Release PM SpinLock					*/
	PM_Spin_Unlock

	WFI															/* Transition to the WFI standby state				*/

	/* Get PM SpinLock						*/
	PM_Spin_Lock

	/* Resotre Z clk setting				*/
	/* r0: CPU number						*/
	LDR		r4, =CPG_FRQCRB
	LDR		r5, =FRRCRB_ZCLK_MASK
	LDR		r6, =FRRCRB_ZCLK_1_48
	LDR		r7, [r4]
	AND		r7, r7, r5
	CMP		r7, r6
	BNE		STRWFI_ZCLKRESTORE_SKIP
	LDR		r6, =FRQCRB_KICK
STRWFI_KICK_CLEAR_LOOP3:
	LDR		r8, [r4]
	TST		r8, r6
	BNE		STRWFI_KICK_CLEAR_LOOP3
	LDR		r9, =ram0CpuClock
	LDR		r7, [r9]
	BIC		r8, r8, r5											/* Clear the z clk setting							*/
	ORR		r8, r8, r7											/* Restore the Z clk setting						*/
	ORR		r8, r8, r6											/* Set KICK bit										*/
	STR		r8, [r4]											/* Activate the z clk setting						*/
STRWFI_KICK_CLEAR_LOOP4:
	LDR		r8, [r4]
	TST		r8, r6
	BNE		STRWFI_KICK_CLEAR_LOOP4
STRWFI_ZCLKRESTORE_SKIP:
	NOP
	/* Update current cpu status			*/
	/* r0: CPU number						*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0Status
	LDRNE	r5, =ram0Cpu1Status
	LDR		r6, =CPUSTATUS_RUN
	STR		r6, [r5]
	DSB

	/* Release PM SpinLock					*/
	PM_Spin_Unlock

	LDMFD	sp!, {r4-r11, pc}
.pool
ENDPROC(start_wfi)


#ifdef VMALLOC_EXPAND

/************************************************************************************************/
/* Disable MMU															*/
/* This function is running on RAM0															*/
/*																								*/
/* Enter with: 																					*/
/*	r0 = Physical address of jump point after disabling MMU*/
/************************************************************************************************/
.align	5
ENTRY(disablemmu)
	/*------------------------------*/
	/* Disable MMU					*/
	/*------------------------------*/
	MRC		p15, 0, r4, c1, c0, 0								/* Read Control register						*/
	BIC		r4, r4, #0x1										/* Clear M bit									*/
	ISB	
	MCR		p15, 0, r4, c1, c0, 0								/* Write Control register (Disable MMU)			*/
	MOV		pc, r0	
.pool
ENDPROC(disablemmu)


/************************************************************************************************/
/* Transition to System Sleep(A3SM Power Off) (CPU0 MMU OFF)										*/
/* This function is running on RAM0															*/
/*																								*/
/************************************************************************************************/
.align	5
ENTRY(systemsuspend_cpu0_pa)

	/*------------------------------------------*/
	/* Sys power down							*/
	/*------------------------------------------*/
	LDR		r4, =ram0SysPowerDownPhys
	MOV		lr, pc
	MOV		pc, r4										/* Call sys_powerdown	*/
	NOP
	NOP
	
	/*------------------------------------------*/
	/* Sys power up								*/
	/*------------------------------------------*/
	LDR		r4, =ram0SysPowerUpPhys
	MOV		lr, pc
	MOV		pc, r4									/* Call sys_powerup			*/
	NOP


	/*------------------------------------------*/
	/* Restore Common registers					*/
	/*------------------------------------------*/
	LDR		r4, =ram0RestoreCommonRegisterPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_common_register()				*/
	NOP

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r1, =ram0SystemSuspendRestoreCPU0Phys
	LDR		r1, [r1] 											/* Virtual address of next instruction after enabling MMU*/
	
	LDR		r0, =ram0MmuSetting0Phys
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_pa()				*/
	NOP

.pool
ENDPROC(systemsuspend_cpu0_pa)

/************************************************************************************************/
/* Transition to System Sleep(A3SM Power Off) (CPU1 MMU OFF)										*/
/* This function is running on RAM0															*/
/*																								*/
/************************************************************************************************/
.align	5
ENTRY(systemsuspend_cpu1_pa)

	/*------------------------------------------*/
	/* CPU power down							*/
	/*------------------------------------------*/
	LDR		r7, =ram0WakeupCodeAddr1Phys
	MOV		r4, #0x0300
	LDR		r5, =SCU_PWRST_ADDRPhys
	LDR		r6, [r5]	
	ORR		r6, r6, r4
	STR		r6, [r5]	
	DSB
	STR		pc, [r7]	
	WFI
	NOP
	MOV		r4,	#0x0300
	LDR		r5, =SCU_PWRST_ADDRPhys
	LDR		r6, [r5]	
	BIC		r6, r6, r4
	STR		r6, [r5]
	DSB

	/*------------------------------------------*/
	/* Invalidate L1 D-cache					*/
	/*------------------------------------------*/
	L1_DataCacheInvalidateSW									/* L1 D-Cache Invalidate						*/

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	LDR		r1, =ram0SystemSuspendRestoreCPU1Phys
	LDR		r1, [r1] 
	
	LDR		r0, =ram0MmuSetting1Phys
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc, r4												/* Call restore_arm_register_pa()				*/
	NOP

.pool
ENDPROC(systemsuspend_cpu1_pa)


/************************************************************************************************/
/*	Transition to coreStandby(A1SL0 or A1Sl1 Power Off) (MMU OFF)											*/
/* This function is running on RAM0															*/
/*																								*/
/* Enter with: 																					*/
/************************************************************************************************/
.align	5
ENTRY(corestandby_pa)

	/* Get PM SpinLock				*/
	PM_Spin_Lock

	/*------------------------------------------*/
	/* Set the SCU CPU power Status Register	*/
	/*------------------------------------------*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0, #2										/* Extract the CPUID Field						*/
	CMP		r4, #0x0
	LDREQ	r5, =0x0003											/* CPU0 Powered-off mode						*/
	LDRNE	r5, =0x0300											/* CPU1 Powered-off mode						*/
	LDR		r6, =SCU_PWRST_ADDRPhys
	LDR		r7, [r6]
	ORR		r7, r7, r5
	STR		r7, [r6]											/* Set the CPU Powered-off mode					*/
	DSB	

	/* Update current cpu status				*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register					*/
	UBFX	r0, r4, #0x0, #0x2									/* Extract the CPUID Field							*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu0StatusPhys
	LDRNE	r5, =ram0Cpu1StatusPhys
	LDR		r6, =CPUSTATUS_SHUTDOWN
	STR		r6, [r5]
	DSB

	/* PLL0 off or z clk 1/48					*/
	CMP		r0, #0x0
	LDREQ	r5, =ram0Cpu1StatusPhys
	LDRNE	r5, =ram0Cpu0StatusPhys
	LDR		r4, [r5]
	CMP		r4, #CPUSTATUS_SHUTDOWN								/* All cpu is shutdown state?						*/
	BNE		CORESTANDBY_CHECK_ZCLKCHG
	/* Turn off PLL0							*/
	LDR		r4, =CPG_PLLECRPhys
	LDR		r5, [r4]
	BIC		r5, r5, #CPG_PLL0E									/* Clear PLL0E										*/
	STR		r5, [r4]											/* Turn off PLL0									*/
CORESTANDBY_TURNOFF_PLL0_LOOP:
	LDR		r5, [r4]
	TST		r5, #CPG_PLL0ST										/* Check PLLECR.PLL0ST								*/
	BNE		CORESTANDBY_TURNOFF_PLL0_LOOP						/* Loop until operation is completed				*/
	B		CORESTANDBY_CLKCHG_EXIT
CORESTANDBY_CHECK_ZCLKCHG:
	CMP		r4, #CPUSTATUS_WFI									/* Another cpu is wfi state							*/
	BNE		CORESTANDBY_CLKCHG_EXIT
	/* Z clk change 1/48						*/
	LDR		r5, =CPG_FRQCRBPhys
	LDR		r6, [r5]
	LDR		r7, =FRRCRB_ZCLK_MASK
	LDR		r8, =ram0CpuClockPhys
	AND		r4, r6, r7
	STR		r4, [r8]											/* Save Z clk setting								*/
	DSB
	LDR		r6, =FRQCRB_KICK
CORESTANDBY_KICK_CLEAR_LOOP1:
	LDR		r4, [r5]
	TST		r4, r6
	BNE		CORESTANDBY_KICK_CLEAR_LOOP1
	BIC		r4, r4, r7											/* Clear FRQCRB.ZSEL, ZFC[3:0] field				*/
	LDR		r8, =FRRCRB_ZCLK_1_48
	ORR		r4, r4, r8											/* Set Z clk = 1/48									*/
	ORR		r4, r4, r6											/* Set KICK bit										*/
	STR		r4, [r5]											/* Activate the Z clk setting						*/
CORESTANDBY_KICK_CLEAR_LOOP2:
	LDR		r4, [r5]
	TST		r4, r6
	BNE		CORESTANDBY_KICK_CLEAR_LOOP2
CORESTANDBY_CLKCHG_EXIT:
	NOP

	/* Release PM SpinLock				*/
	PM_Spin_Unlock
	
	/*------------------------------------------*/
	/* CPU power down							*/
	/*------------------------------------------*/
	ADR		r5, CORESTANDBY_WAKEUP
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0x0, #0x2									/* Extract the CPUID Field						*/
	CMP		r4, #0x0
	LDREQ	r6, =ram0WakeupCodeAddr0Phys
	LDRNE	r6, =ram0WakeupCodeAddr1Phys
	STR		r5, [r6]											/* Set the CPU wakeup code address				*/
	DSB
	WFI															/* Transition to shutdown status				*/
CORESTANDBY_WAKEUP:
	NOP

	/* Get PM SpinLock				*/
	PM_Spin_Lock

	/* Turn on PLL0 */
	LDR		r4, =CPG_PLLECRPhys
	LDR		r5, [r4]
	TST		r5, #CPG_PLL0ST									/* Check PLLECR.PLL0ST							*/
	BNE		CORESTANDBY_TURNON_PLL0_SKIP						/* if (PLL0 == turned off)						*/
	ORR		r5, r5, #CPG_PLL0E
	STR		r5, [r4]											/* Turn on PLL0									*/
CORESTANDBY_TURNON_PLL0_LOOP:
	LDR		r5, [r4]
	TST		r5, #CPG_PLL0ST									/* Check PLLECR.PLL0ST							*/
	BEQ		CORESTANDBY_TURNON_PLL0_LOOP						/* Loop until operation is completed			*/
CORESTANDBY_TURNON_PLL0_SKIP:
	/* Resotre Z clk setting	*/
	LDR		r4, =CPG_FRQCRBPhys
	LDR		r5, =FRRCRB_ZCLK_MASK
	LDR		r6, =FRRCRB_ZCLK_1_48
	LDR		r7, [r4]
	AND		r7, r7, r5
	CMP		r7, r6
	BNE		CORESTANDBY_ZCLKRESTORE_SKIP
	LDR		r6, =FRQCRB_KICK
CORESTANDBY_KICK_CLEAR_LOOP3:
	LDR		r8, [r4]
	TST		r8, r6
	BNE		CORESTANDBY_KICK_CLEAR_LOOP3
	LDR		r9, =ram0CpuClockPhys
	LDR		r7, [r9]
	BIC		r8, r8, r5											/* Clear the z clk setting							*/
	ORR		r8, r8, r7											/* Restore the Z clk setting						*/
	ORR		r8, r8, r6											/* Set KICK bit										*/
	STR		r8, [r4]											/* Activate the z clk setting						*/
CORESTANDBY_KICK_CLEAR_LOOP4:
	LDR		r8, [r4]
	TST		r8, r6
	BNE		CORESTANDBY_KICK_CLEAR_LOOP4
CORESTANDBY_ZCLKRESTORE_SKIP:
	NOP
	/* Update current cpu status	*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register					*/
	UBFX	r4, r4, #0x0, #0x2									/* Extract the CPUID Field							*/
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
	LDREQ	r5, =0x0003										/* CPU0 status clear mask 						*/
	LDRNE	r5, =0x0300										/* CPU1 status clear mask 						*/
	LDR		r6, =SCU_PWRST_ADDRPhys
	LDR		r7, [r6]
	BIC		r7, r7, r5
	STR		r7, [r6]											/* Set the CPU Normal mode						*/
	DSB

	/* Release PM SpinLock				*/
	PM_Spin_Unlock

	/* Invalidate L1 D-cache		*/
	L1_DataCacheInvalidateSW									/* L1 D-Cache Invalidate						*/

	/*------------------------------------------*/
	/* Restore ARM registers					*/
	/*------------------------------------------*/
	MRC		p15, 0, r4, c0, c0, 5								/* Read MPIDR, MP affinty register				*/
	UBFX	r4, r4, #0, #2										/* Extract the CPUID Field						*/
	CMP		r4, #0x0
	LDREQ	r0, =ram0MmuSetting0Phys
	LDRNE	r0, =ram0MmuSetting1Phys
	
	LDREQ	r1, =ram0CoreStandbyRestoreCPU0Phys
	LDRNE	r1, =ram0CoreStandbyRestoreCPU1Phys	
	LDR		r1, [r1]
	
	LDR		r4, =ram0RestoreArmRegisterPAPhys
	MOV		lr, pc												/* lr: Return address							*/
	MOV		pc,r4												/* Call restore_arm_register_pa()				*/
	NOP

.pool
ENDPROC(corestandby_pa)

#endif /* VMALLOC_EXPAND */
