/*
 * arch/arm/mach-shmobile/pmMacro.S
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

/********************************************/
/* Power Management Macros					*/
/* Macro List: 								*/
/*					PM_Spin_Lock			*/
/*					PM_Spin_Unlock			*/
/*					CPU0_SpinLock_Get		*/
/*					CPU0_SpinLock_Release	*/
/*					CPU1_SpinLock_Get		*/
/*					CPU1_SpinLock_Release	*/
/*					CPU0_SpinLock_Get_Phys	*/
/*					CPU0_SpinLock_Release_Phys*/
/*					CPU1_SpinLock_Get_Phys	*/
/*					CPU1_SpinLock_Release_Phys*/
/********************************************/


/*==========================================*/
/*	Name: PM_Spin_Lock						*/
/*	Overview: Get PM SpinLock 				*/
/*==========================================*/
	.macro PM_Spin_Lock
#ifdef CONFIG_PM_SMP
	MOV		r4, #0x1
	MRC		p15, 0, r7, c1, c0, 0	/* System Control Register */
	TST		r7, #0x1
	LDRNE	r5, =ram0SpinLockVA
	LDREQ	r5, =ram0SpinLockPAPhys
	LDR		r5, [r5]
1:
	LDREX	r6, [r5]
	CMP		r6, #0x0
	/* WFENE */
	STREXEQ	r6, r4, [r5]
	CMPEQ	r6, #0x0
	BNE		1b
	DMB
#endif
	.endm


/*==========================================*/
/*	Name: PM_Spin_Unlock					*/
/*	Overview: Release PM SpinLock 			*/
/*==========================================*/
	.macro PM_Spin_Unlock
#ifdef CONFIG_PM_SMP
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
#endif
	.endm


/*===========================*/
/*	Name:CPU0_SpinLock_Get */
/*	Over View: Get CPU0 SpinLock */
/*===========================*/
	.macro CPU0_SpinLock_Get
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	BNE		4f
1:
	LDR		r8, =ram0CPU1SpinLock
	MOV		r10, #2 /* check count */
2:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU1 Flag check */
	BNE		1b
	SUBS	r10, r10, #1
	BNE		2b

	LDR		r8, =ram0CPU0SpinLock
	MOV		r10, #1
	STR		r10, [r8] /* CPU0 Flag set */
	DSB
	MOV		r10, #2 /* check count */

	LDR		r8, =ram0CPU1SpinLock
3:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU1 Flag check */
	LDRNE	r8, =ram0CPU0SpinLock
	MOVNE	r10, #0
	STRNE	r10, [r8] /* CPU0 Flag clear */
	BNE		1b

	SUBS	r10, r10, #1
	BNE		3b

4:
#endif
	.endm


/*===========================*/
/*	Name:CPU0_SpinLock_Release */
/*	Over View: Release CPU0 SpinLock */
/*===========================*/
	.macro CPU0_SpinLock_Release
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	BNE		4f
	
	LDR		r8, =ram0CPU0SpinLock
	MOV		r9, #0
	STR		r9, [r8]

4:
#endif
	.endm


/*===========================*/
/*	Name:CPU1_SpinLock_Get */
/*	Over View: Get CPU1 SpinLock */
/*===========================*/
	.macro CPU1_SpinLock_Get
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x1
	BNE		4f

1:
	LDR		r8, =ram0CPU0SpinLock
	MOV		r10, #3 /* check count */
2:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU0 Flug check */
	BNE		1b
	SUBS	r10, r10, #1
	BNE		2b

	LDR		r8, =ram0CPU1SpinLock
	MOV		r10, #1
	STR		r10, [r8] /* CPU1 Flag set */
	DSB
	MOV		r10, #2 /* check count */

	LDR		r8, =ram0CPU0SpinLock
3:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU0 Flag check */
	LDRNE	r8, =ram0CPU1SpinLock
	MOVNE	r10, #0
	STRNE	r10, [r8] /* CPU1 Flag clear */
	BNE		1b

	SUBS	r10, r10, #1
	BNE		3b

4:
#endif
	.endm


/*======================================================================*/
/*	Name:CPU1_SpinLock_Release											*/
/*	Over View: Release CPU1 SpinLock 									*/
/*======================================================================*/
	.macro CPU1_SpinLock_Release
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x1
	BNE		4f

	LDR		r8, =ram0CPU1SpinLock
	MOV		r9, #0
	STR		r9, [r8]

4:
#endif
	.endm


/*===========================*/
/*	Name:CPU0_SpinLock_Get_Phys */
/*	Over View: Get CPU0 SpinLock */
/*===========================*/
	.macro CPU0_SpinLock_Get_Phys
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	BNE		4f
1:
	LDR		r8, =ram0CPU1SpinLockPhys
	MOV		r10, #2 /* check count */
2:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU1 Flag check */
	BNE		1b
	SUBS	r10, r10, #1
	BNE		2b

	LDR		r8, =ram0CPU0SpinLockPhys
	MOV		r10, #1
	STR		r10, [r8] /* CPU0 Flag set */
	DSB
	MOV		r10, #2 /* check count */

	LDR		r8, =ram0CPU1SpinLockPhys
3:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU1 Flag check */
	LDRNE	r8, =ram0CPU0SpinLockPhys
	MOVNE	r10, #0
	STRNE	r10, [r8] /* CPU0 Flag clear */
	BNE		1b

	SUBS	r10, r10, #1
	BNE		3b

4:
#endif
	.endm


/*===========================*/
/*	Name:CPU0_SpinLock_Release_Phys */
/*	Over View: Release CPU0 SpinLock */
/*===========================*/
	.macro CPU0_SpinLock_Release_Phys
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x0
	BNE		4f
	
	LDR		r8, =ram0CPU0SpinLockPhys
	MOV		r9, #0
	STR		r9, [r8]

4:
#endif
	.endm


/*===========================*/
/*	Name:CPU1_SpinLock_Get_Phys */
/*	Over View: Get CPU1 SpinLock */
/*===========================*/
	.macro CPU1_SpinLock_Get_Phys
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x1
	BNE		4f

1:
	LDR		r8, =ram0CPU0SpinLockPhys
	MOV		r10, #3 /* check count */
2:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU0 Flug check */
	BNE		1b
	SUBS	r10, r10, #1
	BNE		2b

	LDR		r8, =ram0CPU1SpinLockPhys
	MOV		r10, #1
	STR		r10, [r8] /* CPU1 Flag set */
	DSB
	MOV		r10, #2 /* check count */

	LDR		r8, =ram0CPU0SpinLockPhys
3:
	LDR		r9, [r8]
	CMP		r9, #0 /* CPU0 Flag check */
	LDRNE	r8, =ram0CPU1SpinLockPhys
	MOVNE	r10, #0
	STRNE	r10, [r8] /* CPU1 Flag clear */
	BNE		1b

	SUBS	r10, r10, #1
	BNE		3b

4:
#endif
	.endm


/*======================================================================*/
/*	Name:CPU1_SpinLock_Release_Phys											*/
/*	Over View: Release CPU1 SpinLock 									*/
/*======================================================================*/
	.macro CPU1_SpinLock_Release_Phys
#ifdef CONFIG_PM_SMP
	MRC	p15, 0, r4, c0, c0, 5 /* Read MPIDR, MP affinty register */
	UBFX	r4, r4, #0, #2 /* Extract the CPUID Field */
	CMP		r4, #0x1
	BNE		4f

	LDR		r8, =ram0CPU1SpinLockPhys
	MOV		r9, #0
	STR		r9, [r8]

4:
#endif
	.endm


