/*
 * arch/arm/mach-shmobile/pm_ram0.h
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
#ifndef __PM_RAM0_H__
#define __PM_RAM0_H__

#include <mach/vmalloc.h>
#include <mach/r8a73734.h>

#define 	__EXTAL1_INFO__

#define		VMALLOC_EXPAND /* vmalloc area is expanded or not */
#undef CONFIG_PM_SMP
#if (defined(CONFIG_SMP) && (CONFIG_NR_CPUS > 1))
#define CONFIG_PM_SMP
#endif

#ifndef		CONFIG_PM_SMP
#define		CORESTANDBY_A2SL /* CORESTANDBY_A2SL */
#endif

#ifdef DISPLAY_LOG_DEBUG
#define DEBUG_IDLE
#endif

/*
 * Used Inter Connect RAM0 (hereafter, Internal RAM0 or RAM0) for
 *  - Function area
 *  - Backup area
 *  - SBSC clock change area
 */

/****************************************************************************/
/* RAM0 MAPPING															*/
/****************************************************************************/
#define ram0BasePhys	0xE63A2000	/* RAM0 Base physical address */
#define ram0Base		IO_ADDRESS(ram0BasePhys)


/* Size of backup area	*/
#define	saveCommonSettingSize				0x90
#define	savePl30GlobalSettingSize			0x14
#define	saveArmMmuSettingSize				0x28

/* Size of CPU Register backup area	*/
#define	saveCpuRegisterAreaSize				0x660

/* Size of code	*/
#define	fsArmVector							0x80				/* ARM Vector size							*/
#define	fsCoreStandby						0x300				/* Core Standby function size				*/
#define	fsSystemSuspend						0x300				/* System Suspend function size				*/
#define	fsSaveArmRegister					0x1C0				/* Save ARM register function size			*/
#define	fsRestoreArmRegisterPA				0x80				/* Restore ARM register function(PA) size	*/
#define	fsRestoreArmRegisterVA				0x100				/* Restore ARM register function(VA) size	*/
#define	fsSaveArmCommonRegister				0x160				/* Save ARM common register function size	*/
#define	fsRestoreArmCommonRegister			0x1C0				/* Restore ARM common register function size*/
#define	fsSaveCommonRegister				0xE0				/* Save common register function size		*/
#define	fsRestoreCommonRegister				0x100				/* Restore common register function size	*/
#define	fsSysPowerDown						0x240				/* power down function size					*/
#define	fsSysPowerUp						0x300				/* power up function size					*/
#define	fsSetClockSystemSuspend				0x320				/* Set clock function size					*/
#ifdef VMALLOC_EXPAND
#define	fsSystemSuspendCPU0PA				0x100				/* System Suspend for CPU 0 function with MMU off size	*/
#define	fsCoreStandbyPA						0x418				/* Core Standby function with MMU off size				*/
#define	fsDisableMMU						0x20				/* Disable MMU function size							*/
#define	fsSystemSuspendCPU1PA				0x120				/* System Suspend for CPU 1 function with MMU off size	*/
#endif /* VMALLOC_EXPAND */

/*-----------------------------------------------*/
/* Offset of RAM0 area							*/
/*	function area			(RAM0:0x2000-0x54FF) */
/*	backup area				(RAM0:0x5500-0x56FF) */
/*	SBSC clock change area	(RAM0:0x5700-0x5AFF) */
/*-----------------------------------------------*/
/* function area */

/* Offset to the ARM Vector	*/
#define		hoArmVector						\
0x0
/* Offset to the Core Standby function */
#define		hoCoreStandby					\
(hoArmVector				+ fsArmVector)
/* Offset to the System Suspend function */
#define		hoSystemSuspend					\
(hoCoreStandby				+ fsCoreStandby)
/* Offset to the Save ARM register function */
#define		hoSaveArmRegister				\
(hoSystemSuspend			+ fsSystemSuspend)
/* Offset to the Restore ARM register function(PA) */
#define		hoRestoreArmRegisterPA			\
(hoSaveArmRegister			+ fsSaveArmRegister)
/* Offset to the Restore ARM register function(VA) */
#define		hoRestoreArmRegisterVA			\
(hoRestoreArmRegisterPA		+ fsRestoreArmRegisterPA)
/* Offset to the Save ARM common register function */
#define		hoSaveArmCommonRegister			\
(hoRestoreArmRegisterVA		+ fsRestoreArmRegisterVA)
/* Offset to the Restore ARM common register function */
#define		hoRestoreArmCommonRegister		\
(hoSaveArmCommonRegister	+ fsSaveArmCommonRegister)
/* Offset to the Save common register function */
#define		hoSaveCommonRegister			\
(hoRestoreArmCommonRegister	+ fsRestoreArmCommonRegister)
/* Offset to the Restore common register function */
#define		hoRestoreCommonRegister			\
(hoSaveCommonRegister		+ fsSaveCommonRegister)
/* Offset to the power down function */
#define		hoSysPowerDown					\
(hoRestoreCommonRegister	+ fsRestoreCommonRegister)
/* Offset to the power up function */
#define		hoSysPowerUp					\
(hoSysPowerDown				+ fsSysPowerDown)
/* Offset to the Set clock function */
#define		hoSetClockSystemSuspend			\
(hoSysPowerUp				+ fsSysPowerUp)
#ifdef VMALLOC_EXPAND
/* Offset to the System Suspend for CPU 0 function with MMU off	*/
#define		hoSystemSuspendCPU0PA			\
(hoSetClockSystemSuspend	+ fsSetClockSystemSuspend)
/* Offset to the Core Standby function with MMU off	*/
#define		hoCoreStandbyPA					\
(hoSystemSuspendCPU0PA		+ fsSystemSuspendCPU0PA)
/* Offset to the Disable MMU function */
#define		hoDisableMMU					\
(hoCoreStandbyPA		+ fsCoreStandbyPA)
/* Offset to the System Suspend for CPU 1 function with MMU off	*/
#define		hoSystemSuspendCPU1PA			\
(hoDisableMMU			 + fsDisableMMU)
#endif /*VMALLOC_EXPAND*/

/* backup area 					*/
#define	hoBackup		0x1E00		/* Offset to the Area for backup */

/*-------------------------------------------------------*/
/* Address of RAM0 area */
/*	function area			(RAM0:0xE63A2000-0xE63A3DFF) */
/*  backup area				(RAM0:0xE63A3E00-0xE63A3FFF) */
/*	SBSC clock change		(T.B.D)                      */
/*-------------------------------------------------------*/
/* Address of function			*/

/* Address of ARM Vector function */
#define		ram0ArmVector						\
(ram0Base + hoArmVector)
/* Address of Core Standby function */
#define		ram0CoreStandby						\
(ram0Base + hoCoreStandby)
/* Address of System Suspend function */
#define		ram0SystemSuspend					\
(ram0Base + hoSystemSuspend)
/* Address of Save ARM register function */
#define		ram0SaveArmRegister					\
(ram0Base + hoSaveArmRegister)
/* Address of Restore ARM register function(PA) */
#define		ram0RestoreArmRegisterPA			\
(ram0Base + hoRestoreArmRegisterPA)
/* Address of Restore ARM register function(VA) */
#define		ram0RestoreArmRegisterVA			\
(ram0Base + hoRestoreArmRegisterVA)
/* Address of Save ARM common register function */
#define		ram0SaveArmCommonRegister			\
(ram0Base + hoSaveArmCommonRegister)
/* Address of Restore ARM common register function */
#define		ram0RestoreArmCommonRegister		\
(ram0Base + hoRestoreArmCommonRegister)
/* Address of Save common register function */
#define		ram0SaveCommonRegister				\
(ram0Base + hoSaveCommonRegister)
/* Address of Restore common register function */
#define		ram0RestoreCommonRegister			\
(ram0Base + hoRestoreCommonRegister)
/* Address of System power down function */
#define		ram0SysPowerDown					\
(ram0Base + hoSysPowerDown)
/* Address of System power up function */
#define		ram0SysPowerUp						\
(ram0Base + hoSysPowerUp)
/* Address of Set clock function */
#define		ram0SetClockSystemSuspend			\
(ram0Base + hoSetClockSystemSuspend)
#ifdef VMALLOC_EXPAND
/* Address of System Suspend for CPU 0 function with MMU off */
#define		ram0SystemSuspendCPU0PA				\
(ram0Base + hoSystemSuspendCPU0PA)
/* Address of Core Standby function with MMU off */
#define		ram0CoreStandbyPA					\
(ram0Base + hoCoreStandbyPA)
/* Address of Disable MMU function */
#define		ram0DisableMMU						\
(ram0Base + hoDisableMMU)
/* Address of System Suspend for CPU 1 function with MMU off */
#define		ram0SystemSuspendCPU1PA				\
(ram0Base + hoSystemSuspendCPU1PA)
#endif /* VMALLOC_EXPAND */

/* Address of function (Phys) */

/* Address of ARM Vector function */
#define	ram0ArmVectorPhys					\
(ram0BasePhys + hoArmVector)
/* Address of Core Standby function */
#define	ram0CoreStandbyPhys					\
(ram0BasePhys + hoCoreStandby)
/* Address of System Suspend function */
#define	ram0SystemSuspendPhys				\
(ram0BasePhys + hoSystemSuspend)
/* Address of Save ARM register function */
#define	ram0SaveArmRegisterPhys				\
(ram0BasePhys + hoSaveArmRegister)
/* Address of Restore ARM register function(PA) */
#define	ram0RestoreArmRegisterPAPhys		\
(ram0BasePhys + hoRestoreArmRegisterPA)
/* Address of Restore ARM register function(VA) */
#define	ram0RestoreArmRegisterVAPhys		\
(ram0BasePhys + hoRestoreArmRegisterVA)
/* Address of Save ARM common register function */
#define	ram0SaveArmCommonRegisterPhys		\
(ram0BasePhys + hoSaveArmCommonRegister)
/* Address of Restore ARM common register function */
#define	ram0RestoreArmCommonRegisterPhys	\
(ram0BasePhys + hoRestoreArmCommonRegister)
/* Address of Save common register function */
#define	ram0SaveCommonRegisterPhys			\
(ram0BasePhys + hoSaveCommonRegister)
/* Address of Restore common register function */
#define	ram0RestoreCommonRegisterPhys		\
(ram0BasePhys + hoRestoreCommonRegister)
/* Address of System power down function */
#define	ram0SysPowerDownPhys				\
(ram0BasePhys + hoSysPowerDown)
/* Address of System power up function */
#define	ram0SysPowerUpPhys					\
(ram0BasePhys + hoSysPowerUp)
/* Address of Set clock function */
#define	ram0SetClockSystemSuspendPhys		\
(ram0BasePhys + hoSetClockSystemSuspend)
#ifdef VMALLOC_EXPAND
/* Address of System Suspend for CPU 0 function with MMU off */
#define	ram0SystemSuspendCPU0PAPhys			\
(ram0BasePhys + hoSystemSuspendCPU0PA)
/* Address of Core Standby function with MMU off */
#define	ram0CoreStandbyPAPhys				\
(ram0BasePhys + hoCoreStandbyPA)
/* Address of Disable MMU function */
#define	ram0DisableMMUPhys					\
(ram0BasePhys + hoDisableMMU)
/* Address of System Suspend for CPU 1 function with MMU off */
#define	ram0SystemSuspendCPU1PAPhys			\
(ram0BasePhys + hoSystemSuspendCPU1PA)
#endif /* VMALLOC_EXPAND */

/* Address of backup area top */
/* Address of backup(L) */
#define	ram0Backup							\
(ram0Base		+ hoBackup)
/* Address of backup(P) */
#define	ram0BackupPhys						\
(ram0BasePhys	+ hoBackup)

/* Backup area */
#define	ram0CommonSetting	ram0Backup
#define	ram0Pl310GlobalSetting				\
(ram0CommonSetting				+ saveCommonSettingSize)
#define	ram0MmuSetting0						\
(ram0Pl310GlobalSetting			+ savePl30GlobalSettingSize)
#define	ram0MmuSetting1						\
(ram0MmuSetting0				+ saveArmMmuSettingSize)
#define	ram0WakeupCodeAddr0					\
(ram0MmuSetting1				+ saveArmMmuSettingSize)
#define	ram0WakeupCodeAddr1					\
(ram0WakeupCodeAddr0			+ 0x4)
#define	ram0Cpu0RegisterArea				\
(ram0WakeupCodeAddr1			+ 0x4)
#define	ram0Cpu1RegisterArea				\
(ram0Cpu0RegisterArea			+ 0x4)
#define	ram0SpinLockVA						\
(ram0Cpu1RegisterArea			+ 0x4)
#define	ram0SpinLockPA						\
(ram0SpinLockVA					+ 0x4)
#define	ram0Cpu0Status						\
(ram0SpinLockPA					+ 0x4)
#define	ram0Cpu1Status						\
(ram0Cpu0Status					+ 0x4)
#define	ram0CpuClock						\
(ram0Cpu1Status					+ 0x4)
#define	ram0SetClockWork					\
(ram0CpuClock					+ 0x4)
#define	ram0SetClockFrqcra					\
(ram0SetClockWork				+ 0x4)
#define	ram0SetClockFrqcrb					\
(ram0SetClockFrqcra				+ 0x4)
#define	ram0SetClockFrqcrd					\
(ram0SetClockFrqcrb				+ 0x4)

#define	ram0SecHalCommaEntry				\
(ram0SetClockFrqcrd				+ 0x4)

#define ram0SecHalReturnCpu0						\
(ram0SecHalCommaEntry						+ 0x4)

#define ram0SecHalReturnCpu1						\
(ram0SecHalReturnCpu0						+ 0x4)

#define ram0ZClockFlag						\
(ram0SecHalReturnCpu1						+ 0x4)


#define	ram0DramPasrSettingArea0			\
(ram0ZClockFlag				+ 0x4)

#define	ram0DramPasrSettingArea1			\
(ram0DramPasrSettingArea0		+ 0x4)
#define	ram0SaveSdmracr0a					\
(ram0DramPasrSettingArea1		+ 0x4)
#define	ram0SaveSdmracr1a					\
(ram0SaveSdmracr0a				+ 0x4)
#ifdef __EXTAL1_INFO__
#define	ram0SaveEXMSKCNT1_suspend 	\
(ram0SaveSdmracr1a + 0x4)
#define	ram0SaveAPSCSTP_suspend 	\
(ram0SaveEXMSKCNT1_suspend + 0x4)
#define	ram0SaveSYCKENMSK_suspend 	\
(ram0SaveAPSCSTP_suspend + 0x4)
#define	ram0SaveC4POWCR_suspend 	\
(ram0SaveSYCKENMSK_suspend + 0x4)
#define	ram0SavePDNSEL_suspend 		\
(ram0SaveC4POWCR_suspend + 0x4)
#define	ram0SavePSTR_suspend 		\
(ram0SavePDNSEL_suspend + 0x4)

#define	ram0SaveEXMSKCNT1_resume 	\
(ram0SavePSTR_suspend + 0x4)
#define	ram0SaveAPSCSTP_resume 		\
(ram0SaveEXMSKCNT1_resume + 0x4)
#define	ram0SaveSYCKENMSK_resume 	\
(ram0SaveAPSCSTP_resume + 0x4)
#define	ram0SaveC4POWCR_resume 		\
(ram0SaveSYCKENMSK_resume + 0x4)
#define	ram0SavePDNSEL_resume 		\
(ram0SaveC4POWCR_resume + 0x4)
#define	ram0SavePSTR_resume 		\
(ram0SavePDNSEL_resume + 0x4)

#endif

#ifdef VMALLOC_EXPAND

/*Restore point after enable MMU for System Suspend with CPU0*/
#ifdef __EXTAL1_INFO__
#define	ram0SystemSuspendRestoreCPU0	\
(ram0SavePSTR_resume + 0x4)
#else
#define	ram0SystemSuspendRestoreCPU0	\
(ram0SaveSdmracr1a + 0x4)
#endif

/*Restore point after enable MMU for System Suspend with CPU1*/
#define	ram0SystemSuspendRestoreCPU1		\
(ram0SystemSuspendRestoreCPU0	+ 0x4)
/*Restore point after enable MMU for CoreStandby with CPU0*/
#define	ram0CoreStandbyRestoreCPU0			\
(ram0SystemSuspendRestoreCPU1	+ 0x4)
/*Restore point after enable MMU for CoreStandby with CPU1*/
#define	ram0CoreStandbyRestoreCPU1			\
(ram0CoreStandbyRestoreCPU0 + 0x4)
/*FRQCRA mask*/
#define	ram0FRQCRAMask			\
(ram0CoreStandbyRestoreCPU1 + 0x4)
/*FRQCRA mask*/
#define	ram0FRQCRADown			\
(ram0FRQCRAMask + 0x4)

#endif /* VMALLOC_EXPAND */

/* SPI Status Registers */
#define	ram0_ICSPISR0		\
(ram0FRQCRADown + 0x4)			
#define	 ram0_ICSPISR1						\
(ram0_ICSPISR0 + 0x4)

/* Backup area Phys			*/
#define	ram0CommonSettingPhys				ram0BackupPhys
#define	ram0Pl310GlobalSettingPhys			\
(ram0CommonSettingPhys			+ saveCommonSettingSize)
#define	ram0MmuSetting0Phys					\
(ram0Pl310GlobalSettingPhys		+ savePl30GlobalSettingSize)
#define	ram0MmuSetting1Phys					\
(ram0MmuSetting0Phys			+ saveArmMmuSettingSize)
#define	ram0WakeupCodeAddr0Phys				\
(ram0MmuSetting1Phys			+ saveArmMmuSettingSize)
#define	ram0WakeupCodeAddr1Phys				\
(ram0WakeupCodeAddr0Phys		+ 0x4)
#define	ram0Cpu0RegisterAreaPhys			\
(ram0WakeupCodeAddr1Phys		+ 0x4)
#define	ram0Cpu1RegisterAreaPhys			\
(ram0Cpu0RegisterAreaPhys		+ 0x4)
#define	ram0SpinLockVAPhys					\
(ram0Cpu1RegisterAreaPhys		+ 0x4)
#define	ram0SpinLockPAPhys					\
(ram0SpinLockVAPhys				+ 0x4)
#define	ram0Cpu0StatusPhys					\
(ram0SpinLockPAPhys				+ 0x4)
#define	ram0Cpu1StatusPhys					\
(ram0Cpu0StatusPhys				+ 0x4)
#define	ram0CpuClockPhys					\
(ram0Cpu1StatusPhys				+ 0x4)
#define	ram0SetClockWorkPhys				\
(ram0CpuClockPhys				+ 0x4)
#define	ram0SetClockFrqcraPhys				\
(ram0SetClockWorkPhys			+ 0x4)
#define	ram0SetClockFrqcrbPhys				\
(ram0SetClockFrqcraPhys			+ 0x4)
#define	ram0SetClockFrqcrdPhys				\
(ram0SetClockFrqcrbPhys			+ 0x4)

#define	ram0SecHalCommaEntryPhys			\
(ram0SetClockFrqcrdPhys			+ 0x4)

#define ram0SecHalReturnCpu0Phys			\
(ram0SecHalCommaEntryPhys						+ 0x4)

#define ram0SecHalReturnCpu1Phys			\
(ram0SecHalReturnCpu0Phys					+ 0x4)

#define ram0ZClockFlagPhys						\
(ram0SecHalReturnCpu1Phys						+ 0x4)

#define	ram0DramPasrSettingArea0Phys		\
(ram0ZClockFlagPhys		+ 0x4)

#define	ram0DramPasrSettingArea1Phys		\
(ram0DramPasrSettingArea0Phys	+ 0x4)
#define	ram0SaveSdmracr0aPhys				\
(ram0DramPasrSettingArea1Phys	+ 0x4)
#define	ram0SaveSdmracr1aPhys				\
(ram0SaveSdmracr0aPhys			+ 0x4)

#ifdef __EXTAL1_INFO__
#define	ram0SaveEXMSKCNT1Phys_suspend 	\
(ram0SaveSdmracr1aPhys + 0x4)
#define	ram0SaveAPSCSTPPhys_suspend 	\
(ram0SaveEXMSKCNT1Phys_suspend + 0x4)
#define	ram0SaveSYCKENMSKPhys_suspend 	\
(ram0SaveAPSCSTPPhys_suspend + 0x4)
#define	ram0SaveC4POWCRPhys_suspend 	\
(ram0SaveSYCKENMSKPhys_suspend + 0x4)
#define	ram0SavePDNSELPhys_suspend 		\
(ram0SaveC4POWCRPhys_suspend + 0x4)
#define	ram0SavePSTRPhys_suspend 		\
(ram0SavePDNSELPhys_suspend + 0x4)

#define	ram0SaveEXMSKCNT1Phys_resume 	\
(ram0SavePSTRPhys_suspend + 0x4)
#define	ram0SaveAPSCSTPPhys_resume 		\
(ram0SaveEXMSKCNT1Phys_resume + 0x4)
#define	ram0SaveSYCKENMSKPhys_resume 	\
(ram0SaveAPSCSTPPhys_resume + 0x4)
#define	ram0SaveC4POWCRPhys_resume 		\
(ram0SaveSYCKENMSKPhys_resume + 0x4)
#define	ram0SavePDNSELPhys_resume 		\
(ram0SaveC4POWCRPhys_resume + 0x4)
#define	ram0SavePSTRPhys_resume 		\
(ram0SavePDNSELPhys_resume + 0x4)

#endif

#ifdef VMALLOC_EXPAND
/* Restore point after enable MMU for System Sleep with CPU0 */

#ifdef __EXTAL1_INFO__
#define	ram0SystemSuspendRestoreCPU0Phys	\
(ram0SavePSTRPhys_resume + 0x4)
#else
#define	ram0SystemSuspendRestoreCPU0Phys	\
(ram0SaveSdmracr1aPhys + 0x4)
#endif

/* Restore point after enable MMU for System Sleep with CPU1 */
#define	ram0SystemSuspendRestoreCPU1Phys	\
(ram0SystemSuspendRestoreCPU0Phys	+ 0x4)
/* Restore point after enable MMU for CoreStandby with CPU0 */
#define	ram0CoreStandbyRestoreCPU0Phys		\
(ram0SystemSuspendRestoreCPU1Phys	+ 0x4)
/* Restore point after enable MMU for CoreStandby with CPU1 */
#define	ram0CoreStandbyRestoreCPU1Phys		\
(ram0CoreStandbyRestoreCPU0Phys + 0x4)
/*FRQCRA mask*/
#define	ram0FRQCRAMaskPhys			\
(ram0CoreStandbyRestoreCPU1Phys + 0x4)
/*FRQCRA mask*/
#define	ram0FRQCRADownPhys			\
(ram0FRQCRAMaskPhys + 0x4)

#endif /*VMALLOC_EXPAND*/

/* SPI Status Registers */
#define	ram0_ICSPISR0Phys		\
(ram0FRQCRADownPhys + 0x4)			
#define	 ram0_ICSPISR1Phys					\
(ram0_ICSPISR0Phys + 0x4)




/*------------------------------------------------*/
/* Offset of CPU register buckup area */
/*	Defining the offset of allocated memory area. */
/*	Subject to the offset address is stored */
/*	in ram0Cpu0RegisterArea/ram0Cpu1RegisterArea. */
/*------------------------------------------------*/
/* Backup setting(CPU register)	*/
#define	hoSaveArmSvc			0
#define	hoSaveArmExceptSvc		(hoSaveArmSvc + 0x4)
#define	hoSaveArmVfp			(hoSaveArmExceptSvc + 0x4)
#define	hoSaveArmGic			(hoSaveArmVfp + 0x4)
#define	hoSaveArmTimer			(hoSaveArmGic + 0x4)
#define	hoSaveArmSystem			(hoSaveArmTimer + 0x4)
#define	hoSaveArmPerformanceMonitor	(hoSaveArmSystem + 0x4)
#define	hoSaveArmCti			(hoSaveArmPerformanceMonitor + 0x4)
#define	hoSaveArmPtm			(hoSaveArmCti + 0x4)
#define	hoSaveArmDebug			(hoSaveArmPtm + 0x4)
#define	hoBackupAddr			(hoSaveArmDebug	+ 0x4)
#define	hoDataArea				(hoBackupAddr + 0x4)

/*-----------------------------------------*/
/* Definition of CPU status	*/
/*----------------------------------------*/
#define CPUSTATUS_RUN						0x0
#define CPUSTATUS_WFI						0x1
#define CPUSTATUS_SHUTDOWN					0x3

/*----------------------------------------------*/
/* Definition parameters of sec_hal_coma_entry()*/
/*----------------------------------------------*/
/* Mode (R0) */
#define SEC_HAL_CORE_STANDBY_REQ	0x0
#define SEC_HAL_SUSPEND_REQ			0x1
#define SEC_HAL_SLEEP				0x2

/* Frequency (R1) */
#define CLK_NOCHANGED				0x00
#define PLL0_OFF					0x01
#define EXTAL2_OFF					0x02
#define Z_CLK_CHANGED				0x04
#define ZG_CLK_CHANGED				0x08
#define ZTR_CLK_CHANGED				0x10
#define ZT_CLK_CHANGED				0x20
#define ZX_CLK_CHANGED				0x40
#define ZS_CLK_CHANGED				0x80
#define HP_CLK_CHANGED				0x100
#define I_CLK_CHANGED				0x200
#define B_CLK_CHANGED				0x400
#define M1_CLK_CHANGED				0x800
#define M3_CLK_CHANGED				0x1000
#define M5_CLK_CHANGED				0x2000
#define ZB3_CLK_CHANGED				0x4000

#define CORESTANDBY_CLK_CHANGED	(PLL0_OFF | Z_CLK_CHANGED | CLK_NOCHANGED)

#define SUSPEND_CLK_CHANGED	(PLL0_OFF | EXTAL2_OFF | Z_CLK_CHANGED | \
ZG_CLK_CHANGED | ZTR_CLK_CHANGED | ZT_CLK_CHANGED | ZX_CLK_CHANGED | \
ZS_CLK_CHANGED | HP_CLK_CHANGED | I_CLK_CHANGED | B_CLK_CHANGED | \
M1_CLK_CHANGED | M3_CLK_CHANGED | M5_CLK_CHANGED | ZB3_CLK_CHANGED)
/*#define SUSPEND_CLK_CHANGED					0x03FFB */


/* wake-up address (R2)	*/
#define WAKEUP_ADDRESS				ram0ArmVectorPhys
#define WAKEUP_ADDRESS_DUMMY		0x00000000

/* Context save address (R3) */
#define CONTEXT_SAVE_ADDRESS		0x00000000
#define CONTEXT_SAVE_ADDRESS_DUMMY	0x00000000

/* Return value	 (R0) */
#define SEC_HAL_RES_OK				0x00000000
#define SEC_HAL_RES_FAIL			0x00000010
#define SEC_HAL_FREQ_FAIL			0x00000020

#if 0
/*---------------------------------------------------------------*/
/* Definition needed for changing the SBSC clock. */
/*-------------------------------------------------------------*/
/* Base address					*/
#define	ram0ChangeClkBase		(ram0Base		+ 0x3700)
#define	ram0ChangeClkBasePhys	(ram0BasePhys	+ 0x3700)
/* Size of code					*/
#define	fsChangeSbscClkOnInternalMemory	0x2A0
#define	fsWaitOnInternalMemory			0xE0
/* Offset of code				*/
#define	hoChangeSbscClkOnInternalMemory	0x0000
#define	hoWaitOnInternalMemory			\
(hoChangeSbscClkOnInternalMemory + fsChangeSbscClkOnInternalMemory)

/* Address of function			*/
#define	ram0ChangeSbscClkOnInternalMemory	\
(ram0ChangeClkBase			+ hoChangeSbscClkOnInternalMemory)
#define	ram0WaitOnInternalMemory			\
(ram0ChangeClkBase			+ hoWaitOnInternalMemory)
/* Address of function(Phys)	*/
#define	ram0ChangeSbscClkOnInternalMemoryPhys	\
(ram0ChangeClkBasePhys		+ hoChangeSbscClkOnInternalMemory)
#define	ram0WaitOnInternalMemoryPhys			\
(ram0ChangeClkBasePhys		+ hoWaitOnInternalMemory)


/*------------------------------------------*/
/* Wait state		"BEGIN WAIT"	:0x0	*/
/*					"PREPARE WAIT"	:0x1	*/
/*					"ENTER WAIT"	:0x2	*/
/*					"FINISH WAIT"	:0x3	*/
/*					"END WAIT"		:0x4	*/
/* Wait request		"BEGIN"			:0x0	*/
/*					"PREPARE"		:0x1	*/
/*					"ENTER"			:0x2	*/
/*					"FINISH"		:0x3	*/
/*					"END"			:0x4	*/
/*------------------------------------------*/
/* Address of flags				*/
#define	ram0WaitState					\
(ram0ChangeClkBase + hoWaitOnInternalMemory + fsWaitOnInternalMemory)
#define	ram0WaitRequest					\
(ram0WaitState				+ 0x4)
/* Address of flags(Phys)		*/
#define	ram0WaitStatePhys				\
(ram0ChangeClkBasePhys + hoWaitOnInternalMemory + fsWaitOnInternalMemory)
#define	ram0WaitRequestPhys				\
(ram0WaitStatePhys			+ 0x4)
#endif

#endif /* __PM_RAM0_H__ */

