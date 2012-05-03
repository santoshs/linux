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

/************************************************************/
/* Power Management Macros					*/
/* Macro List: 								*/
/*					PM_Spin_Lock			*/
/*					PM_Spin_Unlock			*/
/* Remove_CPU_Cluster__Disable_Clean_and_Invalidate_D_cache	*/
/************************************************************/


/*==============================*/
/*	Name: PM_Spin_Lock			*/
/*	Overview: Get PM SpinLock 	*/
/*==============================*/
	.macro PM_Spin_Lock
#ifdef CONFIG_SMP
	MOV		r4, #0x1
	MRC		p15, 0, r7, c1, c0, 0 /* System Control Register*/
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


/*==================================*/
/*	Name: PM_Spin_Unlock			*/
/*	Overview: Release PM SpinLock	*/
/*==================================*/
	.macro PM_Spin_Unlock
#ifdef CONFIG_SMP
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

/*====================================================================*/
/*	Name:PM Remove_CPU_Cluster__Disable_Clean_and_Invalidate_D_cache  */
/*	Over View: Release CPU Cluster, Disable, Clean, Invalidate D-cache*/
/*====================================================================*/
	.macro Remove_CPU_Cluster__Disable_Clean_and_Invalidate_D_cache
	/* Disable D-Cache	*/
	MRC		p15, 0, r4, c1, c0, 0
	BIC		r4, r4, #0x4			/* Clear C bit */
	MCR		p15, 0, r4, c1, c0, 0	/* Disable D-Cache */
	ISB

	/* Clean and invalidate D-Cache	*/
	L1_DataCacheCleanInvalidateSW

	/*----------------------------------------------*/
	/* Remove the CPU from the cluster				*/
	/*----------------------------------------------*/
	MRC		p15, 0, r4, c1, c0, 1
	BIC		r4, r4, #0x40			/* Clear SMP bit */
	MCR		p15, 0, r4, c1, c0, 1	/* Write the ACTLR */
	ISB
	.endm

