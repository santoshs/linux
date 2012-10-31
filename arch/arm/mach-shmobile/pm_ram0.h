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

#define __EXTAL1_INFO__

#undef CONFIG_PM_SMP
#if (defined(CONFIG_SMP) && (CONFIG_NR_CPUS > 1))
#define CONFIG_PM_SMP
#endif

#ifndef	CONFIG_PM_SMP
#define		CORESTANDBY_A2SL /* CORESTANDBY_A2SL */
#endif


/*
 * Used Inter Connect RAM0/1 (hereafter, Internal RAM0/1 or RAM0/1) for
 *  - Function area
 *  - Backup area
 */

/****************************************************************************/
/* RAM0/RAM1 MAPPING													*/
/****************************************************************************/
#define ram0BasePhys	0xE63A2000	/* RAM0 Base physical address */
#define ram0Base		IO_ADDRESS(ram0BasePhys)

#define ram1BasePhys	0xE63C0000	/* RAM1 Base physical address */
#define ram1Base		IO_ADDRESS(ram1BasePhys)

/* Size of backup area	*/
#define	saveCommonSettingSize				0x90
#define	savePl30GlobalSettingSize			0x14
#define	saveArmMmuSettingSize				0x28

/* Size of CPU Register backup area	*/
#define	saveCpuRegisterAreaSize				0x660

/* Size of code	*/
#define	fsArmVector					0x100 /* ARM Vector */
#define	fsCoreStandby				0x0 /* Core Standby */
#define	fsSystemSuspend				0x0 /* System Suspend */
#define	fsSaveArmRegister			0x0 /* Save ARM register */
#define	fsRestoreArmRegisterPA		0x60 /* Restore ARM register (PA) */
#define	fsRestoreArmRegisterVA		0x0/* Restore ARM register VA) */
#define	fsSaveArmCommonRegister		0x0 /* Save ARM common register */
#define	fsRestoreArmCommonRegister	0x0 /* Restore ARM common register */
#define	fsSaveCommonRegister		0x0 /* Save common register */
#define	fsRestoreCommonRegister		0x100 /* Restore common register */
#define	fsSysPowerDown				0x240 /* power down */
#define	fsSysPowerUp				0x2C0 /* power up */
#define	fsSetClockSystemSuspend		0x160 /* Set clock */

#define	fsSystemSuspendCPU0PA		0x80 /* System Suspend for CPU0 with MMU off */
#define	fsCoreStandbyPA				0x5C0 /* CoreStandby function with MMU off */
#define	fsDisableMMU				0x20 /* Disable MMU function */
#define	fsSystemSuspendCPU1PA		0xE0 /* System Suspend for CPU1 with MMU off */

/*--------------------------------------------------*/
/* Offset of RAM1 area */
/* Function area				*/
/*--------------------------------------------------*/
/* Offset to the ARM Vector */
#define		hoArmVector						\
0x0
/* Offset to the Core Standby function */
#define		hoCoreStandby					\
(hoArmVector + fsArmVector)
/* Offset to the System Suspend function */
#define		hoSystemSuspend					\
(hoCoreStandby + fsCoreStandby)
/* Offset to the Save ARM register function */
#define		hoSaveArmRegister				\
(hoSystemSuspend + fsSystemSuspend)
/* Offset to the Restore ARM register function(PA) */
#define		hoRestoreArmRegisterPA			\
(hoSaveArmRegister + fsSaveArmRegister)
/* Offset to the Restore ARM register function(VA) */
#define		hoRestoreArmRegisterVA			\
(hoRestoreArmRegisterPA + fsRestoreArmRegisterPA)
/* Offset to the Save ARM common register function */
#define		hoSaveArmCommonRegister			\
(hoRestoreArmRegisterVA + fsRestoreArmRegisterVA)
/* Offset to the Restore ARM common register function */
#define		hoRestoreArmCommonRegister		\
(hoSaveArmCommonRegister + fsSaveArmCommonRegister)
/* Offset to the Save common register function */
#define		hoSaveCommonRegister			\
(hoRestoreArmCommonRegister + fsRestoreArmCommonRegister)
/* Offset to the Restore common register function */
#define		hoRestoreCommonRegister			\
(hoSaveCommonRegister + fsSaveCommonRegister)
/* Offset to the power down function */
#define		hoSysPowerDown					\
(hoRestoreCommonRegister + fsRestoreCommonRegister)
/* Offset to the power up function */
#define		hoSysPowerUp					\
(hoSysPowerDown + fsSysPowerDown)
/* Offset to the Set clock function */
#define		hoSetClockSystemSuspend			\
(hoSysPowerUp + fsSysPowerUp)


/* Offset to the System Suspend for CPU 0 function with MMU off */
#define		hoSystemSuspendCPU0PA			\
(hoSetClockSystemSuspend + fsSetClockSystemSuspend)
/* Offset to the Core Standby function with MMU off */
#define		hoCoreStandbyPA					\
(hoSystemSuspendCPU0PA + fsSystemSuspendCPU0PA)
/* Offset to the Disable MMU function */
#define		hoDisableMMU					\
(hoCoreStandbyPA + fsCoreStandbyPA)
/* Offset to the System Suspend for CPU 1 function with MMU off */
#define		hoSystemSuspendCPU1PA			\
(hoDisableMMU + fsDisableMMU)




/*--------------------------------------------------------------------------*/
/* Address of RAM0 area */
/* function area				(RAM0:0xE63A2000-0xE63A54FF) */
/* backup area					(RAM0:0xE63A5500-0xE63A56FF) */
/* SBSC clock change			(RAM0:0xE63A5700-0xE63A5AFF) */
/*--------------------------------------------------------------------------*/
/* Address of function */

/* Address of function (Virt)	*/
/* Address of ARM Vector function */
#define		ram1ArmVector					\
(ram1Base + hoArmVector)
/* Address of Core Standby function */
#define		ram1CoreStandby					\
(ram1Base + hoCoreStandby)
/* Address of System Suspend function */
#define		ram1SystemSuspend				\
(ram1Base + hoSystemSuspend)
/* Address of Save ARM register function */
#define		ram1SaveArmRegister				\
(ram1Base + hoSaveArmRegister)
/* Address of Restore ARM register function(PA) */
#define		ram1RestoreArmRegisterPA		\
(ram1Base + hoRestoreArmRegisterPA)
/* Address of Restore ARM register function(VA) */
#define		ram1RestoreArmRegisterVA		\
(ram1Base + hoRestoreArmRegisterVA)
/* Address of Save ARM common register function */
#define		ram1SaveArmCommonRegister		\
(ram1Base + hoSaveArmCommonRegister)
/* Address of Restore ARM common register function */
#define		ram1RestoreArmCommonRegister	\
(ram1Base + hoRestoreArmCommonRegister)
/* Address of Save common register function */
#define		ram1SaveCommonRegister			\
(ram1Base + hoSaveCommonRegister)
/* Address of Restore common register function */
#define		ram1RestoreCommonRegister		\
(ram1Base + hoRestoreCommonRegister)
/* Address of System power down function */
#define		ram1SysPowerDown				\
(ram1Base + hoSysPowerDown)
/* Address of System power up function */
#define		ram1SysPowerUp					\
(ram1Base + hoSysPowerUp)
/* Address of Set clock function */
#define		ram1SetClockSystemSuspend		\
(ram1Base + hoSetClockSystemSuspend)


/* Address of System Suspend for CPU 0 function with MMU off */
#define	ram1SystemSuspendCPU0PA				\
(ram1Base + hoSystemSuspendCPU0PA)
/* Address of Core Standby function with MMU off */
#define	ram1CoreStandbyPA					\
(ram1Base + hoCoreStandbyPA)
/* Address of Disable MMU function */
#define		ram1DisableMMU					\
(ram1Base + hoDisableMMU)
/* Address of System Suspend for CPU 1 function with MMU off */
#define	ram1SystemSuspendCPU1PA				\
(ram1Base + hoSystemSuspendCPU1PA)

/* Address of function (Phys)	*/
/* Address of ARM Vector function */
#define		ram1ArmVectorPhys					\
(ram1BasePhys + hoArmVector)
/* Address of Core Standby function */
#define	ram1CoreStandbyPhys					\
(ram1BasePhys + hoCoreStandby)
/* Address of System Suspend function */
#define	ram1SystemSuspendPhys				\
(ram1BasePhys + hoSystemSuspend)
/* Address of Save ARM register function */
#define	ram1SaveArmRegisterPhys				\
(ram1BasePhys + hoSaveArmRegister)
/* Address of Restore ARM register function(PA) */
#define	ram1RestoreArmRegisterPAPhys		\
(ram1BasePhys + hoRestoreArmRegisterPA)
/* Address of Restore ARM register function(VA) */
#define	ram1RestoreArmRegisterVAPhys		\
(ram1BasePhys + hoRestoreArmRegisterVA)
/* Address of Save ARM common register function */
#define	ram1SaveArmCommonRegisterPhys		\
(ram1BasePhys + hoSaveArmCommonRegister)
/* Address of Restore ARM common register function */
#define	ram1RestoreArmCommonRegisterPhys	\
(ram1BasePhys + hoRestoreArmCommonRegister)
/* Address of Save common register function */
#define	ram1SaveCommonRegisterPhys			\
(ram1BasePhys + hoSaveCommonRegister)
/* Address of Restore common register function */
#define	ram1RestoreCommonRegisterPhys		\
(ram1BasePhys + hoRestoreCommonRegister)
/* Address of System power down function */
#define	ram1SysPowerDownPhys				\
(ram1BasePhys + hoSysPowerDown)
/* Address of System power up function */
#define	ram1SysPowerUpPhys					\
(ram1BasePhys + hoSysPowerUp)
/* Address of Set clock function */
#define	ram1SetClockSystemSuspendPhys		\
(ram1BasePhys + hoSetClockSystemSuspend)


/* Address of System Suspend for CPU 0 function with MMU off */
#define	ram1SystemSuspendCPU0PAPhys			\
(ram1BasePhys + hoSystemSuspendCPU0PA)
/* Address of Core Standby function with MMU off */
#define	ram1CoreStandbyPAPhys				\
(ram1BasePhys + hoCoreStandbyPA)
/* Address of Disable MMU function			*/
#define	ram1DisableMMUPhys					\
(ram1BasePhys + hoDisableMMU)
/* Address of System Suspend for CPU 1 function with MMU off */
#define	ram1SystemSuspendCPU1PAPhys			\
(ram1BasePhys + hoSystemSuspendCPU1PA)



/*--------------------------------------------------*/
/* Offset of RAM0 area */
/* Backup area					*/
/*--------------------------------------------------*/

/* backup area */
#define	hoBackup	0x0000 /* Offset to the Area for backup */

/* Address of backup area top	*/
/* Address of backup(L) */
#define	ram0Backup							\
(ram0Base		+ hoBackup)
/* Address of backup(P) */
#define	ram0BackupPhys						\
(ram0BasePhys	+ hoBackup)

/* Backup area */
#define	ram0CommonSetting ram0Backup
#define	ram0Pl310GlobalSetting				\
(ram0CommonSetting + saveCommonSettingSize)
#define	ram0MmuSetting0						\
(ram0Pl310GlobalSetting + savePl30GlobalSettingSize)
#define	ram0MmuSetting1						\
(ram0MmuSetting0 + saveArmMmuSettingSize)
#define	ram0WakeupCodeAddr0					\
(ram0MmuSetting1 + saveArmMmuSettingSize)
#define	ram0WakeupCodeAddr1					\
(ram0WakeupCodeAddr0 + 0x4)
#define	ram0Cpu0RegisterArea				\
(ram0WakeupCodeAddr1 + 0x4)
#define	ram0Cpu1RegisterArea				\
(ram0Cpu0RegisterArea + 0x4)
#define	ram0SpinLockVA						\
(ram0Cpu1RegisterArea + 0x4)
#define	ram0SpinLockPA						\
(ram0SpinLockVA + 0x4)
#define	ram0Cpu0Status						\
(ram0SpinLockPA + 0x4)
#define	ram0Cpu1Status						\
(ram0Cpu0Status + 0x4)
#define	ram0CpuClock						\
(ram0Cpu1Status + 0x4)
#define	ram0SetClockWork					\
(ram0CpuClock + 0x4)
#define	ram0SetClockFrqcra					\
(ram0SetClockWork + 0x4)
#define	ram0SetClockFrqcrb					\
(ram0SetClockFrqcra + 0x4)
#define	ram0SetClockFrqcrd					\
(ram0SetClockFrqcrb + 0x4)

/* Errata(ECR0285) */
#define	ram0ES_2_2_AndAfter				\
(ram0SetClockFrqcrd + 0x4)

#define	ram0CPU0SpinLock					\
(ram0ES_2_2_AndAfter + 0x4)
#define	ram0CPU1SpinLock					\
(ram0CPU0SpinLock + 0x4)

#define	ram0DramPasrSettingArea0			\
(ram0CPU1SpinLock + 0x4)
#define	ram0DramPasrSettingArea1			\
(ram0DramPasrSettingArea0 + 0x4)
#define	ram0SaveSdmracr0a					\
(ram0DramPasrSettingArea1 + 0x4)
#define	ram0SaveSdmracr1a					\
(ram0SaveSdmracr0a + 0x4)

/* SPI Status Registers */
#define	ram0_ICSPISR0				\
(ram0SaveSdmracr1a + 0x4)			
#define	 ram0_ICSPISR1				\
(ram0_ICSPISR0 + 0x4)
/*FRQCRA mask*/
#define	ram0FRQCRAMask				\
(ram0_ICSPISR1 + 0x4)
/*FRQCRA Down*/
#define	ram0FRQCRADown				\
(ram0FRQCRAMask + 0x4)

#ifdef __EXTAL1_INFO__
#define	ram0SaveEXMSKCNT1_suspend 	\
(ram0FRQCRADown + 0x4)
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

/*Restore point after enable MMU for System Suspend with CPU0*/
#ifdef __EXTAL1_INFO__
#define	ram0SystemSuspendRestoreCPU0	\
(ram0SavePSTR_resume + 0x4)
#else
#define	ram0SystemSuspendRestoreCPU0	\
(ram0FRQCRADown + 0x4)
#endif

/*Restore point after enable MMU for System Suspend with CPU1*/
#define	ram0SystemSuspendRestoreCPU1		\
(ram0SystemSuspendRestoreCPU0 + 0x4)
/*Restore point after enable MMU for CoreStandby with CPU0*/
#define	ram0CoreStandbyRestoreCPU0			\
(ram0SystemSuspendRestoreCPU1 + 0x4)
/*Restore point after enable MMU for CoreStandby with CPU1*/
#define	ram0CoreStandbyRestoreCPU1			\
(ram0CoreStandbyRestoreCPU0 + 0x4)
/* Do ZQ Calibration or not */
#define	ram0ZQCalib		\
(ram0CoreStandbyRestoreCPU1 + 0x4)

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
(ram0WakeupCodeAddr0Phys	+ 0x4)
#define	ram0Cpu0RegisterAreaPhys			\
(ram0WakeupCodeAddr1Phys	+ 0x4)
#define	ram0Cpu1RegisterAreaPhys			\
(ram0Cpu0RegisterAreaPhys	+ 0x4)
#define	ram0SpinLockVAPhys					\
(ram0Cpu1RegisterAreaPhys	+ 0x4)
#define	ram0SpinLockPAPhys					\
(ram0SpinLockVAPhys			+ 0x4)
#define	ram0Cpu0StatusPhys					\
(ram0SpinLockPAPhys			+ 0x4)
#define	ram0Cpu1StatusPhys					\
(ram0Cpu0StatusPhys			+ 0x4)
#define	ram0CpuClockPhys					\
(ram0Cpu1StatusPhys			+ 0x4)
#define	ram0SetClockWorkPhys				\
(ram0CpuClockPhys			+ 0x4)
#define	ram0SetClockFrqcraPhys				\
(ram0SetClockWorkPhys		+ 0x4)
#define	ram0SetClockFrqcrbPhys				\
(ram0SetClockFrqcraPhys		+ 0x4)
#define	ram0SetClockFrqcrdPhys				\
(ram0SetClockFrqcrbPhys		+ 0x4)

/* Errata(ECR0285) */
#define	ram0ES_2_2_AndAfterPhys				\
(ram0SetClockFrqcrdPhys + 0x4)

#define	ram0CPU0SpinLockPhys					\
(ram0ES_2_2_AndAfterPhys + 0x4)

#define	ram0CPU1SpinLockPhys					\
(ram0CPU0SpinLockPhys + 0x4)

#define	ram0DramPasrSettingArea0Phys		\
(ram0CPU1SpinLockPhys			+ 0x4)
#define	ram0DramPasrSettingArea1Phys		\
(ram0DramPasrSettingArea0Phys	+ 0x4)
#define	ram0SaveSdmracr0aPhys				\
(ram0DramPasrSettingArea1Phys	+ 0x4)
#define	ram0SaveSdmracr1aPhys				\
(ram0SaveSdmracr0aPhys			+ 0x4)

/* SPI Status Registers */
#define	ram0_ICSPISR0Phys				\
(ram0SaveSdmracr1aPhys + 0x4)
#define	 ram0_ICSPISR1Phys				\
(ram0_ICSPISR0Phys + 0x4)

/*FRQCRA mask*/
#define	ram0FRQCRAMaskPhys				\
(ram0_ICSPISR1Phys + 0x4)
/*FRQCRA mask*/
#define	ram0FRQCRADownPhys				\
(ram0FRQCRAMaskPhys + 0x4)

#ifdef __EXTAL1_INFO__
#define	ram0SaveEXMSKCNT1Phys_suspend	\
(ram0FRQCRADownPhys + 0x4)
#define	ram0SaveAPSCSTPPhys_suspend		\
(ram0SaveEXMSKCNT1Phys_suspend + 0x4)
#define	ram0SaveSYCKENMSKPhys_suspend	\
(ram0SaveAPSCSTPPhys_suspend + 0x4)
#define	ram0SaveC4POWCRPhys_suspend		\
(ram0SaveSYCKENMSKPhys_suspend + 0x4)
#define	ram0SavePDNSELPhys_suspend		\
(ram0SaveC4POWCRPhys_suspend + 0x4)
#define	ram0SavePSTRPhys_suspend		\
(ram0SavePDNSELPhys_suspend + 0x4)

#define	ram0SaveEXMSKCNT1Phys_resume	\
(ram0SavePSTRPhys_suspend + 0x4)
#define	ram0SaveAPSCSTPPhys_resume		\
(ram0SaveEXMSKCNT1Phys_resume + 0x4)
#define	ram0SaveSYCKENMSKPhys_resume	\
(ram0SaveAPSCSTPPhys_resume + 0x4)
#define	ram0SaveC4POWCRPhys_resume		\
(ram0SaveSYCKENMSKPhys_resume + 0x4)
#define	ram0SavePDNSELPhys_resume		\
(ram0SaveC4POWCRPhys_resume + 0x4)
#define	ram0SavePSTRPhys_resume			\
(ram0SavePDNSELPhys_resume + 0x4)
#endif


/* Restore point after enable MMU for System Sleep with CPU0 */

#ifdef __EXTAL1_INFO__
#define	ram0SystemSuspendRestoreCPU0Phys	\
(ram0SavePSTRPhys_resume + 0x4)
#else
#define	ram0SystemSuspendRestoreCPU0Phys	\
(ram0FRQCRADownPhys + 0x4)
#endif

/* Restore point after enable MMU for System Sleep with CPU1 */
#define ram0SystemSuspendRestoreCPU1Phys	\
(ram0SystemSuspendRestoreCPU0Phys + 0x4)
/* Restore point after enable MMU for CoreStandby with CPU0 */
#define	ram0CoreStandbyRestoreCPU0Phys		\
(ram0SystemSuspendRestoreCPU1Phys + 0x4)
/* Restore point after enable MMU for CoreStandby with CPU1 */
#define	ram0CoreStandbyRestoreCPU1Phys		\
(ram0CoreStandbyRestoreCPU0Phys + 0x4)
/* Do ZQ Calibration or not */
#define	ram0ZQCalibPhys		\
(ram0CoreStandbyRestoreCPU1Phys + 0x4)

/*---------------------------------------------------------------------------*/
/* Offset of CPU register buckup area */
/* Defining the offset of allocated memory area. */
/* Subject to the offset address is stored */
/* in ram0Cpu0RegisterArea/ram0Cpu1RegisterArea. */
/*---------------------------------------------------------------------------*/
/* Backup setting(CPU register)	*/
#define	hoSaveArmSvc				0
#define	hoSaveArmExceptSvc			(hoSaveArmSvc + 0x4)
#define	hoSaveArmVfp				(hoSaveArmExceptSvc + 0x4)
#define	hoSaveArmGic				(hoSaveArmVfp + 0x4)
#define	hoSaveArmTimer				(hoSaveArmGic + 0x4)
#define	hoSaveArmSystem				(hoSaveArmTimer + 0x4)
#define	hoSaveArmPerformanceMonitor	(hoSaveArmSystem + 0x4)
#define	hoSaveArmCti		(hoSaveArmPerformanceMonitor + 0x4)
#define	hoSaveArmPtm				(hoSaveArmCti + 0x4)
#define	hoSaveArmDebug				(hoSaveArmPtm + 0x4)
#define	hoBackupAddr				(hoSaveArmDebug + 0x4)
#define	hoDataArea					(hoBackupAddr + 0x4)

/*---------------------------------------------------------------------------*/
/* Definition of CPU status */
/*---------------------------------------------------------------------------*/
#define CPUSTATUS_RUN				0x0
#define CPUSTATUS_WFI				0x1
#define CPUSTATUS_SHUTDOWN			0x3
#define CPUSTATUS_WFI2				0x4
#endif /* __PM_RAM0_H__ */

/* ### CTM_DEBUG ### */
#define CTM_DEBUG_STBCHR0		0xE6180000
#define CTM_DEBUG_STBCHR1		0xE6180001
#define CTM_DEBUG_STBCHR2		0xE6180002
#define CTM_DEBUG_STBCHR3		0xE6180003


