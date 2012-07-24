/*
 * r_loader.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#include "string.h"
#include "r_loader.h"
#include "common.h"
#include "cpg.h"
#include "gpio.h"
#include "i2c.h"
#include "sbsc.h"
#include "sysc.h"
#include "flash_api.h"

#ifdef __RL_LCD_ENABLE__
#include "lcd_api.h"
#endif	/* __RL_LCD_ENABLE__ */

#include "boot_init.h"
#include "log_output.h"
#include "tmu_api.h"
#include "r_loader_boot_log.h"
#include "usb_api.h"
#include "pmic.h"
#include "r_loader_crash_log.h"

#ifdef __INTEGRITY_CHECK_ENABLE__
#include "disk_drive.h"
#endif /* __INTEGRITY_CHECK_ENABLE__ */

/*
 * Pre-declaration 
 */

extern BOOT_LOG log_info;
extern BOOT_MATRIX matrix_info;
 
/* String for factory2 */
static const char MODE_STRING_FACTORY2[]	= {"factory2"}; 
/* String for off charge */
static const char MODE_STRING_OFFCHARGE[]	= {"offcharge"}; 
/* String for high temperature */
static const char MODE_STRING_HIGHTEMP[]	= {"hightemp"}; 
/* String for no battery */
static const char MODE_STRING_NOBATT[]	= {"nobattery"}; 
/* Module load table */
static Module_load_table module_load_table;
 
/**
 * r_loader_main - R-loader main process
 * @return None
 */
void r_loader_main(void)
{
	RC ret;
	RC branch_mode = MODULE_BRANCH_MODE_BOOTLOADER;
#ifndef __INTEGRITY_CHECK_ENABLE__	
	/* For SMP issue as temporary */
	/* clear L2 cache */
	*L2CLEANWAY |= 0x0000FFFF;
	while((*L2CLEANWAY & 0x0000FFFF) != 0x0)
	{
		;
	}
	/* disable L2 cache */
	*L2CTRLREG &= 0xFFFFFFFE;
	
	/* ARM secure = GRP0 */
	*ARM_SECURE1 = 0xFFFFFFBF;
	*ARM_SECURE2 = 0x00001000;
	/* Set SBSC priority */
	*SBSC_SAPRICR0 = 0x00000000;
	
#endif /* __INTEGRITY_CHECK_ENABLE__ */

	/* R-loader initalize */
	r_loader_init();

	/* Clear RWDT */
	WDT_CLEAR();
	
	/* BEGIN: CR1040: Clean up source code which accesses the eMMC directly */
	/* Save bootflag */
	if(SYSC_SWReset_Flag() == TRUE) {
		PRINTF("r_loader_save_NVM_bootflag \n");
		r_loader_save_NVM_bootflag();
	}
	/* END: CR1040: Clean up source code which accesses the eMMC directly */
	
	/* Branching decision or Power OFF */
	branch_mode = r_loader_select_module();
	if(0 > branch_mode)	{
		PRINTF("FAIL to detect branch mode - ret=%d\n", branch_mode);
		/* Save Boot log */
		ret = Save_Boot_Log_To_eMMC(__LINE__, branch_mode, branch_mode);
		/* Save Crash log */
		Save_Crash_Log_To_eMMC(&log_info, ret);
		/* Error - change to flashing mode*/
		ChangeFlashingMode();
	}
	
#ifdef __RL_LCD_ENABLE__
	/* Show Logo on LCD */
	r_loader_load_logo();
#endif	/* __RL_LCD_ENABLE__ */
	
	/* Loading module load table */
	ret = r_loader_load_module(branch_mode);
	if(R_LOADER_SUCCESS != ret)	{
		PRINTF("FAIL to load module - ret=%d\n", ret);
		/* No software */
		LCD_DisplayWarning("No software");
		return;
	}
	
#ifdef __INTEGRITY_CHECK_ENABLE__
	/* Secure Check */
	ret = r_loader_check_module(branch_mode);
	if(ret != R_LOADER_SUCCESS)	{
		PRINTF("FAIL to check module - ret=%d\n", ret);
		/* Software error */
		LCD_DisplayWarning("Software error");
		return;
	}
#endif /* __INTEGRITY_CHECK_ENABLE__ */

	if(branch_mode != MODULE_BRANCH_MODE_UPDATER)	{
		/* Copy table from flash memory to SDRAM */
		r_loader_copy_table_to_RAM();
		/* Copy SPU bin from flash memory to SDRAM */
		r_loader_copy_SPU_to_RAM();
	}
	
	/* 2nd HW setting */
	if((branch_mode != MODULE_BRANCH_MODE_OFFCHARGE) && (branch_mode != MODULE_BRANCH_MODE_UPDATER)&& (branch_mode != MODULE_BRANCH_MODE_FASTBOOT)){
		GPIO_2nd_Setting();
		CPG_2nd_Setting();
		/* Temporally fix for PM */
		SYSC_PM_Fix();
	}
	//BEGIN: STM is enabled in Deep Sleep
	ulong pstr_bak;
	ulong volatile val;
	
	pstr_bak = *((volatile ulong *) 0xE6180080);	/* backup PSTR */
	*((volatile ulong *) 0xE6180014) = 0x01FFF153;	/* all power on */
	while (1) {val = *((volatile ulong *) 0xE6180014); if (val == 0) break;}

	*((volatile ulong *) 0xE6151180) = 0x01fc3330;
	*((volatile ulong *) 0xE61900C0) = 0x0000800B;
	*((volatile ulong *) 0xE618801C) = 0x90Cff300;
	*((volatile ulong *) 0xE61800C0) = 0x01FFF951;

	*((volatile ulong *) 0xE6180008) = (0x01FFF153 & ~pstr_bak); /* power down except pstr_bak */
	while (1) {
		val = *((volatile ulong *) 0xE6180008);
		if (val == 0) break;
	}
	
	//For ASIC bug
	*((volatile ulong *) 0xE61900C0) = 0x0000800B;
	//END: STM is enabled in Deep Sleep
	
	if(matrix_info.charger == CHRG_NONE)
	{
		CPG_Stop_VCK3clock();
	}
	
	/* Set current limit and enable charger */
#ifndef _R_LOADER_IGNORE_BATT_CHECK_
	switch(matrix_info.charger){
	case CHRG_USB_CDP_DCP:
		pmic_set_current_limit(PMIC_CUR_LIMIT_1500mA);
		pmic_enable_charger(PMIC_CHRG_VBUS);
		break;
	case CHRG_USB_SDP:
		pmic_set_current_limit(PMIC_CUR_LIMIT_500mA);
		pmic_enable_charger(PMIC_CHRG_VBUS);
		break;
	case CHRG_VAC:
		pmic_enable_charger(PMIC_CHRG_VAC);
		break;
	case CHRG_NONE:
		pmic_disable_charger();
		break;
	default:
		break;	
	}	
#endif
	
	/* Save boot log to eMMC */
	ret = Save_Boot_Log_To_eMMC(__LINE__, 0, branch_mode);
	/* Save Crash log to eMMC */
	Save_Crash_Log_To_eMMC(&log_info, ret);
	
	/* flash memory unmount */
	ret = Flash_Access_Unmount(UNUSED);
	if ((ret != FLASH_ACCESS_SUCCESS) && (ret != FLASH_ACCESS_ERR_NOT_MOUNT)){
		PRINTF("FAIL eMMC to unmount flash memory - ret=%d\n", ret);
		/* Save Boot log */
		ret = Save_Boot_Log_To_eMMC(__LINE__, ret, branch_mode);
		/* Save Crash log */
		Save_Crash_Log_To_eMMC(&log_info, ret);
		/* Reset and Change to USB downloader */
		ChangeFlashingMode();
	}
	
	/* Save R-Loader process complete info */
	*STBCHR3 |= 0x01;

	/* Start-up next module */
	r_loader_startup_module(branch_mode);

	return ;
}

/**
 * r_loader_init - R-loader initialize process
 * @return None
 */
void r_loader_init(void)
{
	RC ret = 0;
	CPU_CLOCK clock;

	/* Decide CPU clock */
	if (CHIP_VERSION() >= CHIP_RMU2_ES20) {
		clock = CPU_CLOCK_1456MHZ;
	} else {
		clock = CPU_CLOCK_988MHZ;
	}

	/* Clear boot log */
	boot_log_clear();
	
	/* Init serial */
	serial_init();
	PRINTF("\n---------[R-loader]--------- \n");
	PRINTF("\nSerial Init");
	
	/* Flash memory initialize */
	ret = Flash_Access_Init(UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		PRINTF("FAIL eMMC to initialize flash memory - ret=%d\n", ret);
		/* Reset and Change to USB downloader */
		ChangeFlashingMode();		
		/* The CPU is reset, below code can not reach */
		while(1);
	}

	/* Flash memory mount */
	ret = Flash_Access_Mount(UNUSED);
	if ((ret != FLASH_ACCESS_SUCCESS) && (ret != FLASH_ACCESS_ERR_ALREADY_MOUNT))
	{
		PRINTF("FAIL eMMC to mount flash memory - ret=%d\n", ret);
		/* Reset and Change to USB downloader */
		ChangeFlashingMode();		
		/* The CPU is reset, below code can not reach */
		while(1);
	}
		
	/* Power supply */
	SYSC_Power_Supply();
	/* Sensor Framework powers down (A4SF) */
	SYSC_Reduce_Power(SYS_SPDCR_A4SF);
	PRINTF("\nPower supply\n");
	
	/* Boot init */
	Boot_Init();
	PRINTF("\nBoot Init\n");
	
	/* Set Clock */
	CPG_Set_Clock(clock);
	PRINTF("\nCPG Init\n");

	/* Set GPIO */
	GPIO_Init();
	PRINTF("GPIO Init\n");
	
	/* Initialize I2C */
	I2C_Init();
	PRINTF("I2C Init\n");
	
	/* Start RTC */
	pmic_rtc_unlock();
	pmic_rtc_start();
	
	/* Initialize key */
	GPIO_Key_Init();
	PRINTF("Key Init \n");
	
	if (CHIP_VERSION() == CHIP_RMU2_ES10)
	{
		/* temporary for controls arbitration*/
		*SHBARCR11 = SHBARCR_ALL;
	}
	
	/* Initialize InterconnectRAM */
	CPG_InterconnectRAM_Enable();
	
#ifndef __INTEGRITY_CHECK_ENABLE__	
	if (CHIP_VERSION() >= CHIP_RMU2_ES20) {
		/* Initialize SDRAM */
		SBSC_Init();
		PRINTF("SBSC Init \n");
	}
#endif /* __INTEGRITY_CHECK_ENABLE__ */

	/* Initialize SW semaphore(SD-RAM)*/
	SBSC_SW_Semaphore();
	
	/* Set RESCNT2.SRMD, keep SDRAM data when SW/HW reset occurrs */
	SYSC_Soft_Power_On_Reset();
	
	/* TUSB 1211 Init */
	usb_phy_init();
}

/**
 * r_loader_load_module - Module load processing
 * @return R_LOADER_SUCCESS              : The module loading success
 *         R_LOADER_ERR_LOAD_MLT         : The module load table reading error
 *         R_LOADER_ERR_LOAD_NUM         : The number of modules is an error
 *         R_LOADER_ERR_LOAD_VRL         : The VRL loading error
 *         R_LOADER_ERR_LOAD_MODULE      : The module loading error
 *         R_LOADER_ERR_MODULE_NOT_EXIST : Module does not exist error
 *         R_LOADER_ERR_SECURE           : Secure check error
 *         R_LOADER_ERR_BRANCH_MODE      : Branch mode error
 */
RC r_loader_load_module(RC branch_mode)
{
	RC ret;
	uint64 src_addr;
	ulong  dst_addr;
	ulong  src_size;
	
	
	if (branch_mode > 0) {
		/* Display branching module to serial */
		char MOD_INFO_MESS[]	= {"Selected module:             "};
		char *inf = ((char *)(module_load_table.info[branch_mode].boot_flags));
		uppercase(inf);
		RC index = 0;
		while (0x00 != inf[index])
		{
			MOD_INFO_MESS[index + 17] = inf[index];
			index++;
		}
		PRINTF(MOD_INFO_MESS);
		PRINTF("\n");
		
		/* Display branching module to LCD */
#ifdef __RL_LCD_ENABLE__
		LCD_Print(30, 650, inf);
				
#endif	/* __RL_LCD_ENABLE__ */
	}

	/* Get parameter */
	src_addr = *((uint64 *)(&module_load_table.info[branch_mode].flash_addr));
	dst_addr = *((ulong *)(&module_load_table.info[branch_mode].load_addr));
	src_size = *((ulong *)(&module_load_table.info[branch_mode].payload));
	/* Load module */
	ret = Flash_Access_Read((uchar *)dst_addr, src_size, src_addr, UNUSED);
	if(ret != FLASH_ACCESS_SUCCESS)	{
		PRINTF("FAIL eMMC to load module - ret=%d\n", ret);
		/* No change to Force USB boot mode, display "No software" */
		return R_LOADER_ERR_LOAD_MODULE;
	}
	return R_LOADER_SUCCESS;
}

/**
 * r_loader_startup_module - Start up Bootloader / Recovery / Fastboot
 * @return None
 */
void r_loader_startup_module(RC branch_mode)
{
	ulong jump_addr;
	void (*module_jump)(void);
	/* Get jump address */
	jump_addr = *((ulong *)(&module_load_table.info[branch_mode].load_addr));

	/* Get module address */
	module_jump = (void*)jump_addr;
	
	WDT_CLEAR();
	
	PRINTF("\n\nStarting next module... \n\n");
	
	/* Set 1st domain completion flag */
	*STBCHRB1 |= STBCHRB1_FIRST_DOMAIN_COMPLETE;
	
	/* start module */
	module_jump();
	/* This code below can't be reached*/
	while(1){};	
}

/**
 * r_loader_check_module - Module check processing
 * @return R_LOADER_SUCCESS              : Result of check module is OK
 *         R_LOADER_ERR_SECURE           : Secure check error
 *		   R_LOADER_ERR_LOAD_MODULE		 : Load module/certificate error
 */
#ifdef __INTEGRITY_CHECK_ENABLE__
RC r_loader_check_module(ulong branch_mode)
{
	RC ret;
	uint64 src_addr;	/* Certificate address on flash */
	ulong dst_addr;		/* Certificate address on RAM */	
	ulong src_size;		/* Certificate size */
	ulong module_addr;	/* Module address on RAM */
	uchar order;
	partition_info part;
	
	PRINTF("1st module secure check \n");
	/* Get parameter for 1st module certificate */
	src_addr = *((uint64 *)(&module_load_table.info[branch_mode].signature_addr1));
	if(src_addr == CERTIFICATE_NONE)
	{
		PRINTF("Address of 1st certificate is not available in MLT\n");
		return R_LOADER_ERR_SECURE;
		
	}
	dst_addr = (ulong)(CERTIFICATE_ADDRESS);
	src_size = (ulong)(CERTIFICATE_SIZE);
	module_addr = *((ulong *)(&module_load_table.info[branch_mode].load_addr));
	
	/* Load 1st module certificate */
	ret = Flash_Access_Read((uchar *)dst_addr, src_size, src_addr, UNUSED);
	if(ret != FLASH_ACCESS_SUCCESS)	{
		PRINTF("FAIL to load 1st certificate \n");
		/* No change to Force USB boot mode, display "No software" */
		return R_LOADER_ERR_LOAD_MODULE;
	}
	
	/* Secure check for 1st module */
	ret = Secure_Check((void *)dst_addr, (ulong *)module_addr);
	if(ret != 0) {
		/* Don't start-up next module. */
		PRINTF("Secure check FAIL but continue booting - ret = %d \n", ret);
		/* Temporally remove when checking certificates fail*/
		// return R_LOADER_ERR_SECURE;
	}

	/* Get parameter for 2nd module certificate */
	src_addr = *((uint64 *)(&module_load_table.info[branch_mode].signature_addr2));
	if(src_addr == CERTIFICATE_NONE) {
		PRINTF("Address of 2nd certificate is not available in MLT\n");
	}
	else {
		PRINTF("2nd module secure check \n");
		dst_addr = (ulong)(CERTIFICATE_ADDRESS2);
		src_size = (ulong)(CERTIFICATE_SIZE);
				
		/* Load 2nd certificate to RAM */
		ret = Flash_Access_Read((uchar *)dst_addr, src_size, src_addr, UNUSED);
		if(ret != FLASH_ACCESS_SUCCESS)	{
			PRINTF("FAIL to load 2nd certificate\n");
			return R_LOADER_ERR_LOAD_MODULE;
		}
		
		/* Get partition info of 2nd module on eMMC */
		order = ((uchar)(module_load_table.info[branch_mode].stbchrb2)) & (~0x80) ;
		PRINTF("Parttition of 2nd module: %d \n", order);
		
		/* Get the information of 2nd module on eMMC */
		disk_get_partition(order, &part);

		/* Copy 2nd module to RAM */
		ret = Flash_Access_Read((uchar *)(SECOND_MODULE_ADDRESS), part.size, part.address, UNUSED);
		if(ret != FLASH_ACCESS_SUCCESS)	{
			PRINTF("FAIL to load 2nd module \n");
			return R_LOADER_ERR_LOAD_MODULE;
		}
		
		/* Secure check for 2nd module */
		ret = Secure_Check((void *)dst_addr, (ulong *)SECOND_MODULE_ADDRESS);
		if(ret != 0) {
			/* Don't start-up next module. */
			PRINTF("Secure check FAIL but continue booting - ret = %d \n", ret);
			/* Temporally remove when checking certificates fail*/
			// return R_LOADER_ERR_SECURE;
		}
	}
	
	return R_LOADER_SUCCESS;
}
#endif /* __INTEGRITY_CHECK_ENABLE__ */

/**
 * r_loader_select_module - Select start-up module
 * @return R_LOADER_ERR_KEY     	      : Error relating to KEY
 *         R_LOADER_ERR_FLASH             : Error relating to eMMC
 *         R_LOADER_ERR_BOOT_MATRIX       : Error relating to Boot log module
 *         R_LOADER_ERR_LOAD_MLT          : Error in checking MLT
 *         Other modules defined in module load table
 */
RC r_loader_select_module(void)
{
	RC ret;
	RC boot_mat_res;
	RC branch_mode = MODULE_BRANCH_MODE_BOOTLOADER;
	ulong key;
	uchar boot_flags[BOOTFLAG_SIZE];
	
	/* Update Boot matrix */
	ret = boot_matrix_update();
	if (BOOT_LOG_OK != ret)	{
		PRINTF("FAIL Boot log update Boot matrix - ret=%d\n", ret);
		return R_LOADER_ERR_BOOT_MATRIX;
	}
	
	/* Print boot matrix */
	boot_matrix_print();
	
	/* Get boot matrix result */
	boot_mat_res = boot_matrix_check();
	PRINTF("Boot matrix return %d\n", boot_mat_res);
	
	/* flash read (read module load table) */
	memset((void *)&module_load_table, 0x00, sizeof(module_load_table));
	ret = Flash_Access_Read((uchar *)&module_load_table, sizeof(module_load_table), MLT_SRC_ADDR, UNUSED);
	if(ret != FLASH_ACCESS_SUCCESS)	{
		PRINTF("FAIL eMMC to read module load table - ret=%d\n", ret);
		return R_LOADER_ERR_FLASH;
	}
	
#ifndef _R_LOADER_IGNORE_CPU_TEMP_CHECK_
	/* Check CPU temperature */
	if ((MAT_HIGH_TEMP == boot_mat_res) || (matrix_info.cpu_temp >= BOOT_CPU_TEMP)) {
		/* Search and load High temperature module */
		branch_mode = r_loader_bootflags_lookup(MODE_STRING_HIGHTEMP);
		if(branch_mode != R_LOADER_ERR_LOAD_MLT){
			return branch_mode;
		}
		PRINTF("FAIL Can not find High temperture module in Module loade table\n");
		return R_LOADER_ERR_LOAD_MLT;
	}
#endif	/* _R_LOADER_IGNORE_CPU_TEMP_CHECK_	*/
	
	/* Check device status first */
	switch(boot_mat_res) {
		case MAT_PWR_OFF:
			PRINTF("POWER OFF\n");
			pmic_force_off_hw();
			return 0;
		case MAT_OFF_CHARGE:
			branch_mode = r_loader_bootflags_lookup(MODE_STRING_OFFCHARGE);
			if(branch_mode != R_LOADER_ERR_LOAD_MLT){
				PRINTF("Boot Off-charge - MLT entry %d\n", branch_mode);
				return branch_mode;
			}
			PRINTF("FAIL Can not find Off charge module in Module loade table\n");
			return R_LOADER_ERR_LOAD_MLT;
		case MAT_NO_BATT:
			/* Check MD3 pin status */
			if((*RESCNT & MD3_HIGH) == MD3_HIGH) {
				branch_mode = r_loader_bootflags_lookup(MODE_STRING_FACTORY2);
				if(branch_mode != R_LOADER_ERR_LOAD_MLT){
					PRINTF("Boot for MD3 pin in No battery mode - MLT entry %d\n", branch_mode);
					*STBCHRB2 = ((uchar)(module_load_table.info[branch_mode].stbchrb2));
					return branch_mode;
				}
			}
			else {
				branch_mode = r_loader_bootflags_lookup(MODE_STRING_NOBATT);
				if(branch_mode != R_LOADER_ERR_LOAD_MLT){
					PRINTF("No battery mode - MLT entry %d\n", branch_mode);
					return branch_mode;
				}
			}
			PRINTF("FAIL Can not find No battery module in Module loade table\n");
			return R_LOADER_ERR_LOAD_MLT;
		default:
			break;
	}
	
	/* Detect key input */
	ret = Detect_Key_Input(&key);
	if(ret != R_LOADER_SUCCESS)	{
		PRINTF("FAIL to detect key input - ret=%d\n", ret);
		return ret;
	}

	switch(key)
	{
	case KEY_MENU_HOME:
		PRINTF("Key pressed: KEY_MENU_HOME\n");break;
	case KEY_MENU_BACK:
		PRINTF("Key pressed: KEY_MENU_BACK\n");break;
	case KEY_MENU_CAM_H:
		PRINTF("Key pressed: KEY_MENU_CAM_H\n");break;
	case KEY_MENU_CAM_F:
		PRINTF("Key pressed: KEY_MENU_CAM_F\n");break;
	case KEY_CUSTOM_MENU:
		PRINTF("Key pressed: KEY_CUSTOM_MENU\n");break;
	default:
		PRINTF("No key input\n");break;
	}

	/* Get boot flags (non volatile memory area) */
	ret = Flash_Access_Read(boot_flags, BOOTFLAG_SIZE, BOOTFLAG_ADDR, UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS)
	{
		PRINTF("Fail eMMC to read non-volatile boot flag - ret=%d\n", ret);
		return R_LOADER_ERR_FLASH;
	}
	
	/* Check MD3 pin status (battery > 3.4V) */
	if((*RESCNT & MD3_HIGH) == MD3_HIGH) {
		branch_mode = r_loader_bootflags_lookup(MODE_STRING_FACTORY2);
			if(branch_mode != R_LOADER_ERR_LOAD_MLT){
				PRINTF("Boot for MD3 pin - MLT entry %d\n", branch_mode);
				*STBCHRB2 = ((uchar)(module_load_table.info[branch_mode].stbchrb2));
				return branch_mode;
			}
	}
	
	/* Check key input */
	branch_mode = r_loader_keymap_lookup(key);
	if(branch_mode != R_LOADER_ERR_LOAD_MLT){
		PRINTF("Input key matched - MLT entry %d\n", branch_mode);
		*STBCHRB2 = ((uchar)(module_load_table.info[branch_mode].stbchrb2));
		return branch_mode;
	}
	
	/* Check boot flags */
	branch_mode = r_loader_bootflags_lookup((const char *)boot_flags);
	if(branch_mode != R_LOADER_ERR_LOAD_MLT){
		PRINTF("Boot flag matched - MLT entry %d\n", branch_mode);
		*STBCHRB2 = ((uchar)(module_load_table.info[branch_mode].stbchrb2));
		return branch_mode;
	}
	PRINTF("FAIL There is no module matched in MLT\n");
	return R_LOADER_ERR_LOAD_MLT;
}

/**
 * Detect_Key_Input - Detect Key Input
 * @return R_LOADER_SUCCESS    : Successful
 *         R_LOADER_ERR_KEY    : Key input error.
 */
RC Detect_Key_Input(ulong *key)
{
	uchar wdt_st, wdt_now;
	ushort wdt_offset, wdt_chk;
	ulong continuation = 0;
	ulong result = 0;
	ulong prev = 0;	
	
	/* Get RWDT initial value */
	wdt_st = WDT_CNT();
	wdt_now = wdt_st;
	wdt_offset = 0;
	wdt_chk = 0;
	
	/* Clear result buffer */
	*key = 0;
	
	/* Polling of key input */
	do {
		result = GPIO_Check_Key();

		if (result == prev)	{
			/* detected continuously */
			continuation++;
			if (continuation >= KEY_INPUT_DETECT_COUNT)
			{
				/* successful */
				*key = result;
				return R_LOADER_SUCCESS;
			}
		}else{
			/* reset detection */
			continuation = 0;
			prev = result;
		}
		
		/* Get WDT count */
		wdt_now = WDT_CNT();
		
		/* Check WDT count 5sec */
		if ((wdt_now - wdt_st) >= (uchar)WDT_5SEC){
			/* Add time until 5sec */
			wdt_offset += (ushort)(wdt_now - wdt_st);
			
			/* WDT clear */
			WDT_CLEAR();
			wdt_st = 0;
			wdt_now = 0;
		}
		
		/* Add elapsed time */
		wdt_chk = wdt_offset + (ushort)(wdt_now - wdt_st);
		
		/* Wait about 4ms */
		TMU_Wait_MS(KEY_INPUT_POLLING_TIME);
		
	} while (wdt_chk < (ushort)KEY_INPUT_TIMEOUT);
	
	/* Timeout error */
	return R_LOADER_ERR_KEY;
}

/**
 * r_loader_copy_table_to_RAM - Copy register table
 * @return R_LOADER_SUCCESS   : Successful
 *         R_LOADER_ERR_FLASH : Flash access error
 */
RC r_loader_copy_table_to_RAM(void)
{
	PRINTF("Copy Tuneup value from eMMC to RAM\n");

	RC ret = FLASH_ACCESS_SUCCESS;
	uint64 src_addr;
	ulong  dst_addr;
	ulong size;

	/* Copy register table1(tuneup_value.bin) to SDRAM */
	if (CHIP_VERSION() == CHIP_RMU2_ES10) {
		dst_addr = TABLE1_SDRAM_ADDR_ES1;
	} else {
		dst_addr = TABLE1_SDRAM_ADDR_ES2;
	}
	
	size = (TABLE1_SEC_SIZE * SECTOR_LENGTH);
	src_addr = (TABLE1_SEC_ADDR * SECTOR_LENGTH);

	ret = Flash_Access_Read((uchar *)(dst_addr), (ulong)(size), (uint64 )(src_addr), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS){
		PRINTF("FAIL eMMC to read Tuneup value - ret=%d\n", ret);
		return R_LOADER_ERR_FLASH;
	}
	return R_LOADER_SUCCESS;
}

/**
 * r_loader_copy_SPU_to_RAM - Copy SPU binary
 * @return R_LOADER_SUCCESS   : Successful
 *         R_LOADER_ERR_FLASH : Flash access error
 */
RC r_loader_copy_SPU_to_RAM(void)
{
	PRINTF("Copy SPU from eMMC to RAM\n");
	
	RC ret = FLASH_ACCESS_SUCCESS;
	uint64 src_addr;
	ulong  dst_addr;
	ulong size;

	/* Copy SPU binary to SDRAM */
	if (CHIP_VERSION() == CHIP_RMU2_ES10) {
		dst_addr = SPU_SDRAM_ADDR_ES1;
	} else {
		dst_addr = SPU_SDRAM_ADDR_ES2;
	}
	size = (SPU_SEC_SIZE * SECTOR_LENGTH);
	src_addr = (SPU_SEC_ADDR * SECTOR_LENGTH);

	ret = Flash_Access_Read((uchar *)(dst_addr), (ulong)(size), (uint64 )(src_addr), UNUSED);
	if (ret != FLASH_ACCESS_SUCCESS){
		PRINTF("FAIL eMMC to read SPU binary - ret=%d\n", ret);
		return R_LOADER_ERR_FLASH;
	}
	return R_LOADER_SUCCESS;
}

#ifdef __RL_LCD_ENABLE__

/**
 * r_loader_load_logo - Load logo to LCD
 * @return None
 */
void r_loader_load_logo(void)
{
	int ret;
	ulong  logo_addr = LOGO_RAM_ADDR;
	
	/* LCD Display Show */
	LCD_Init();
	ret = LCD_Check_Board();
	if(LCD_SUCCESS != ret) {
		PRINTF("FAIL to initialize LCD - ret=%d\n", ret);
		LCD_Backlight_On();
	}
	
	/* LCD setting */
	LCD_Clear( LCD_COLOR_WHITE );
	LCD_Set_Text_color(LCD_COLOR_RED);
	LCD_Display_On();
	
	/* flash read (load module) */
	ret = Flash_Access_Read((uchar *)logo_addr, (ulong)(LOGO_BUFF_SIZE), (uint64)(LOGO_eMMC_ADDR), UNUSED);
	if(FLASH_ACCESS_SUCCESS != ret)	{
		PRINTF("FAIL eMMC to read logo - ret=%d\n", ret);
		LCD_Backlight_On();
		return;
	}
	
	/* Draw image to LCD */
	ret = LCD_BMP_Draw( LOGO_X, LOGO_Y, LOGO_WIDTH, LOGO_HIGH, logo_addr);
	if (LCD_SUCCESS != ret) {
		PRINTF("FAIL LCD to draw Bitmap file - ret=%d\n", ret);
		LCD_Print(30, 250, "Renesas Mobile");
	}
	LCD_Draw_Cmode();
	LCD_Backlight_On();
}
#endif	/* __RL_LCD_ENABLE__ */

/**
 * LCD_DisplayWarning - Display "Info" string on LCD and stop
 * @return None
 */
void LCD_DisplayWarning(const char* info)
{
#ifdef __RL_LCD_ENABLE__
	LCD_Print(30, 650, info);
#endif	/* __RL_LCD_ENABLE__ */
	/* This code below can't be reached*/
	while(1){};
}

/**
 * r_loader_bootflags_lookup - Lookup for branch based on bootflags
 * @return >0: branch mode
 *			R_LOADER_ERR_LOAD_MLT: not found	
 */
RC r_loader_bootflags_lookup(const char * flag)
{
	int i;
	RC branch_mode;
	for (i=0; i<MLT_MODULE_MAX; i++)
		{
			if (strncmp(((const char *)(module_load_table.info[i].boot_flags)), flag, BOOTFLAG_SIZE) == 0) {
				branch_mode = i;
				return branch_mode;
			}
		}
	return R_LOADER_ERR_LOAD_MLT;
	
}

/**
 * r_loader_keymap_lookup - Lookup for branch based on keymap
 * @return >0: branch mode
 *			R_LOADER_ERR_LOAD_MLT: not found	
 */
RC r_loader_keymap_lookup(ulong key)
{
	int i;
	RC branch_mode;
	for(i=0; i < MLT_MODULE_MAX; i++)
	{
		if(strncmp(((const char *)&key), (const char *)(module_load_table.info[i].key_mask), 3) == 0) {
			branch_mode = i;
			return branch_mode;
		}
	}
	return R_LOADER_ERR_LOAD_MLT;
	
}

/* BEGIN: CR1040: Clean up source code which accesses the eMMC directly */
/**
 * r_loader_save_NVM_bootflag; save boot flag from SDRAM to eMMC 
 * @return R_LOADER_SUCCESS:   Successful
 *		   R_LOADER_ERR_FLASH: Flash access error	
 */

RC r_loader_save_NVM_bootflag(void)
{
	RC ret;
	uchar flag;

	memcpy(&flag, (void*)BOOTFLAG_SDRAM_ADDR, BOOTFLAG_SDRAM_OFFSET);

	if(flag == 0xA5) {
		/* Save boot flags */
		ret = Flash_Access_Write((uchar *)(BOOTFLAG_SDRAM_ADDR + BOOTFLAG_SDRAM_OFFSET), BOOTFLAG_SIZE, BOOTFLAG_ADDR, UNUSED);
		if (ret != FLASH_ACCESS_SUCCESS)
		{
			return R_LOADER_ERR_FLASH;
		}

		/* Clear boot flags */
		memset((void *)BOOTFLAG_SDRAM_ADDR, 0x00, BOOTFLAG_SDRAM_SIZE);
	}
	return R_LOADER_SUCCESS;
}
/* END: CR1040: Clean up source code which accesses the eMMC directly */
