/* vcd_spuv_func.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/vmalloc.h>
#ifdef __VCD_POWERDOMAIN_ENABLE__
#include <mach/pm.h>
#endif /* __VCD_POWERDOMAIN_ENABLE__ */

#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#ifdef __VCD_MERAM_ENABLE__
#include "rtapi/system_memory.h"
#endif /* __VCD_MERAM_ENABLE__ */

#include "linux/vcd/vcd_common.h"
#include "vcd_spuv_func.h"


/*
 * global variable declaration
 */
void  *g_spuv_func_fw_static_buffer;
unsigned int g_spuv_func_sdram_static_area_top_phy;
unsigned int g_spuv_func_sdram_static_area_top;
unsigned int g_spuv_func_hpb_register_top;
unsigned int g_spuv_func_cpg_register_top;
unsigned int g_spuv_func_crmu_register_top;
unsigned int g_spuv_func_gtu_register_top;
unsigned int g_spuv_func_voiceif_register_top;
unsigned int g_spuv_func_intcvo_register_top;
unsigned int g_spuv_func_spuv_register_top;
unsigned int g_spuv_func_dsp0_register_top;
unsigned int g_spuv_func_xram_base_top;

unsigned int g_spuv_func_meram_physical_addr;
unsigned int g_spuv_func_meram_logical_addr;

#ifdef __VCD_MERAM_ENABLE__
system_mem_meram_alloc g_spuv_func_meram_alloc;
system_mem_meram_free g_spuv_func_meram_free;
system_mem_info_delete g_spuv_func_meram_delete;
#endif /* __VCD_MERAM_ENABLE__ */

struct vcd_spuv_func_fw_info g_spuv_func_pram_info;
struct vcd_spuv_func_fw_info g_spuv_func_xram_info;
struct vcd_spuv_func_fw_info g_spuv_func_yram_info;

#ifdef __VCD_POWERDOMAIN_ENABLE__
static struct device *g_spuv_func_power_domains
			[VCD_SPUV_FUNC_POWER_DOMAIN_MAX];
int g_spuv_func_pm_runtime_count;
struct clk *g_spuv_func_spuv_clk;
#endif /* __VCD_POWERDOMAIN_ENABLE__ */

bool g_spuv_func_is_spuv_clk;

bool g_spuv_func_is_completion;

static DECLARE_WAIT_QUEUE_HEAD(g_vcd_spuv_wait);


/*
 * static prototype declaration
 */
static int vcd_spuv_func_relocation_fw(
		struct vcd_spuv_func_read_fw_info *firmware_info);
static int vcd_spuv_func_get_meram(unsigned int fw_size);
static void vcd_spuv_func_free_meram(void);
#ifdef __VCD_MERAM_ENABLE__
static int vcd_spuv_func_meram_ioremap(unsigned int addr, unsigned int size);
static void vcd_spuv_func_meram_iounmap(void);
#endif /* __VCD_MERAM_ENABLE__ */
static void vcd_spuv_func_calc_ram(
		const unsigned int start_addr,
		const unsigned int global_size,
		const unsigned int *page_size,
		const unsigned int page_num,
		struct vcd_spuv_func_fw_info *ram_info,
		unsigned int *next_addr);
static void vcd_spuv_func_reg_firmware(void);
static int vcd_spuv_func_conv_global_size(const unsigned int global_size);
static void vcd_spuv_func_dsp_full_reset(void);

/* ========================================================================= */
/* Internal public functions                                                 */
/* ========================================================================= */

/**
 * @brief	spuv_func initialize function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_initialize(void)
{
	vcd_pr_start_spuv_function();

	memset((void *)g_spuv_func_sdram_static_area_top,
				0, SPUV_FUNC_SDRAM_AREA_SIZE);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	cache flush function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_cacheflush(void)
{
	int buf_addr = 0;

	vcd_pr_start_spuv_function();

	buf_addr = SPUV_FUNC_SDRAM_AREA_TOP_PHY +
		(SPUV_FUNC_SDRAM_PROC_MSG_BUFFER - SPUV_FUNC_SDRAM_AREA_TOP);

	dmac_flush_range((void *)SPUV_FUNC_SDRAM_PROC_MSG_BUFFER,
		(void *)(SPUV_FUNC_SDRAM_PROC_MSG_BUFFER + PAGE_SIZE));
	outer_flush_range((unsigned long)buf_addr,
		(unsigned long)(buf_addr + PAGE_SIZE));

	vcd_pr_end_spuv_function();
	return;
}


#ifdef __VCD_POWERDOMAIN_ENABLE__
/**
 * @brief	power supply control function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_SYSTEM	system error.
 * @retval	others		result of called function.
 */
int vcd_spuv_func_control_power_supply(int effective)
{
	int ret = VCD_ERR_NONE;
	int loop_count = 0;
	unsigned int hpbctrl2 = 0;
	size_t power_domain_count = 0;

	vcd_pr_start_spuv_function("effective[%d].\n", effective);

	if (VCD_SPUV_FUNC_ENABLE == effective) {

		/* mp-shwy dynamic module stop check */
		vcd_spuv_func_register(SPUV_FUNC_RW_32_HPB_HPBCTRL2, hpbctrl2);
		vcd_pr_spuv_info("HPB_HPBCTRL2[%08x].\n", hpbctrl2);

		/* get power domain devices */
		ret = power_domain_devices(VCD_DRIVER_NAME,
				g_spuv_func_power_domains,
				&power_domain_count);
		if (VCD_ERR_NONE != ret) {
			vcd_pr_err("get power domain error. ret[%d].\n", ret);
			goto rtn;
		}

		/* recovery power supply */
		for (g_spuv_func_pm_runtime_count = 0;
			g_spuv_func_pm_runtime_count < power_domain_count;
			g_spuv_func_pm_runtime_count++) {
			ret = pm_runtime_get_sync(
					g_spuv_func_power_domains
					[g_spuv_func_pm_runtime_count]);
			if (VCD_ERR_NONE > ret) {
				vcd_pr_err("pm_runtime_get_sync[%d].\n", ret);
				goto rtn;
			}
		}

		/* get clock */
		g_spuv_func_spuv_clk = clk_get(NULL, "spuv");
		if (IS_ERR(g_spuv_func_spuv_clk)) {
			vcd_pr_err("g_spuv_func_spuv_clk[%p].\n",
				g_spuv_func_spuv_clk);
			g_spuv_func_spuv_clk = NULL;
			ret = VCD_ERR_SYSTEM;
			goto rtn;
		}

		/* clock enable */
		ret = clk_enable(g_spuv_func_spuv_clk);
		if (VCD_ERR_NONE != ret) {
			vcd_pr_err("clock enable error. ret[%d].\n", ret);
			goto rtn;
		}
		g_spuv_func_is_spuv_clk = true;

		/* cpga spuv module reset */
#if 0
		sh73a0_get_cpg_hpb_sem_with_lock(flags);
#endif
		vcd_spuv_func_modify_register(
			0,
			VCD_SPUV_FUNC_SRCR2_SPU2V_RESET,
			SPUV_FUNC_RW_32_CPG_SRCR2);
		udelay(62);
		vcd_spuv_func_modify_register(
			VCD_SPUV_FUNC_SRCR2_SPU2V_RESET,
			0,
			SPUV_FUNC_RW_32_CPG_SRCR2);
		while (VCD_SPUV_FUNC_SRCR2_SPU2V_RESET &
				ioread32(SPUV_FUNC_RW_32_CPG_SRCR2))
			cpu_relax();
#if 0
		sh73a0_put_cpg_hpb_sem_with_lock(flags);
#endif

		vcd_spuv_func_set_register(
			VCD_SPUV_FUNC_CRMU_VOICEIF_ON,
			SPUV_FUNC_RW_32_CRMU_VOICEIF);

	} else {
		/* spuv clock */
		if (NULL != g_spuv_func_spuv_clk) {
			/* clock disable */
			if (g_spuv_func_is_spuv_clk)
				clk_disable(g_spuv_func_spuv_clk);
			/* put clock */
			clk_put(g_spuv_func_spuv_clk);
		}

		/* release power supply */
		for (loop_count = 0;
			loop_count < g_spuv_func_pm_runtime_count;
			loop_count++) {
			ret = pm_runtime_put_sync(
				g_spuv_func_power_domains[loop_count]);
			if (ret)
				vcd_pr_err("pm_runtime_put_sync[%d].\n", ret);
		}

		/* mp-shwy dynamic module stop check */
		vcd_spuv_func_register(SPUV_FUNC_RW_32_HPB_HPBCTRL2, hpbctrl2);
		vcd_pr_spuv_info("HPB_HPBCTRL2[%08x].\n", hpbctrl2);

		/* initialize */
		g_spuv_func_pm_runtime_count = 0;
		g_spuv_func_spuv_clk = NULL;
		g_spuv_func_is_spuv_clk = false;
	}

rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}
#else /* __VCD_POWERDOMAIN_ENABLE__ */
/**
 * @brief	power supply control function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_SYSTEM	system error.
 * @retval	others		result of called function.
 */
int vcd_spuv_func_control_power_supply(int effective)
{
	int ret = VCD_ERR_NONE;
	unsigned int swucr_adr = 0;
	unsigned int pstr_adr = 0;
	unsigned int reg = 0;
	unsigned int hpbctrl2 = 0;

	vcd_pr_start_spuv_function("effective[%d].\n", effective);

	/* get power domain status */
	pstr_adr = (unsigned int)ioremap(0xE6180080, 0x4);
	vcd_spuv_func_register(pstr_adr, reg);
	iounmap((void *)pstr_adr);

	if (VCD_SPUV_FUNC_ENABLE == effective) {
		/* mp-shwy dynamic module stop check */
		vcd_spuv_func_register(SPUV_FUNC_RW_32_HPB_HPBCTRL2, hpbctrl2);
		vcd_pr_spuv_info("HPB_HPBCTRL2[%08x].\n", hpbctrl2);

		/* a4mp */
		if (!(0x00000100 & reg)) {
			swucr_adr = (unsigned int)ioremap(0xE6180014, 0x4);

			vcd_spuv_func_modify_register(
					0, 0x00000100, swucr_adr);
			do {
				udelay(1);
				vcd_spuv_func_register(swucr_adr, reg);
			} while (0x00000100 & reg);

			iounmap((void *)swucr_adr);
		}

		/* spuv */
		if (!g_spuv_func_is_spuv_clk) {
			vcd_spuv_func_modify_register(
				0x00100000,
				0,
				SPUV_FUNC_RW_32_CPG_MMSTPCR2);
			vcd_spuv_func_modify_register(
				0x00100000,
				0,
				SPUV_FUNC_RW_32_CPG_SMSTPCR2);

			vcd_spuv_func_modify_register(
				0x0000013F,
				0x00000003,
				SPUV_FUNC_RW_32_CPG_SPUVCKCR);

			vcd_spuv_func_modify_register(
				0,
				VCD_SPUV_FUNC_SRCR2_SPU2V_RESET,
				SPUV_FUNC_RW_32_CPG_SRCR2);
			udelay(62);
			vcd_spuv_func_modify_register(
				VCD_SPUV_FUNC_SRCR2_SPU2V_RESET,
				0,
				SPUV_FUNC_RW_32_CPG_SRCR2);
			while (VCD_SPUV_FUNC_SRCR2_SPU2V_RESET &
				ioread32(SPUV_FUNC_RW_32_CPG_SRCR2))
				cpu_relax();

			vcd_spuv_func_set_register(
				VCD_SPUV_FUNC_CRMU_VOICEIF_ON,
				SPUV_FUNC_RW_32_CRMU_VOICEIF);

			g_spuv_func_is_spuv_clk = true;

		}
	} else {
		/* it dares not to drop. */
	}

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}
#endif /* __VCD_POWERDOMAIN_ENABLE__ */


/**
 * @brief	check power supply function.
 *
 * @param	none.
 *
 * @retval	VCD_SPUV_FUNC_DISABLE	spuv clock off.
 * @retval	VCD_SPUV_FUNC_ENABLE	spuv clock on.
 */
int vcd_spuv_func_check_power_supply(void)
{
	int ret = VCD_SPUV_FUNC_DISABLE;

	vcd_pr_start_spuv_function();

	if (g_spuv_func_is_spuv_clk)
		ret = VCD_SPUV_FUNC_ENABLE;

	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


#ifdef __VCD_SPUV_FW_FROM_SD_ENABLE__
/**
 * @brief	set spuv.bin function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE		successful.
 * @retval	VCD_ERR_FILE_NOT_FOUND	file not found.
 * @retval	VCD_ERR_SYSTEM		system error.
 * @retval	others			result of called function.
 */
int vcd_spuv_func_set_fw(void)
{
	int ret = VCD_ERR_NONE;
	struct file *filp = NULL;
	mm_segment_t fs = {0};
	size_t file_size = 0;
	size_t read_size = 0;
	char *read_data = NULL;

	struct vcd_spuv_func_read_fw_info read_fw_info = { {0} };
	void *page_data_addr = NULL;
	unsigned int current_offset = 0;
	unsigned int memory_type = 0;
	unsigned int page_number = 0;
	unsigned int global_size = 0;
	unsigned int page_size = 0;

	vcd_pr_start_spuv_function();

	/* open file */
	filp = filp_open(VCD_SPUV_FUNC_SPUV_FILENAME,
			O_RDONLY | O_LARGEFILE, 0);
	if (IS_ERR(filp)) {
		vcd_pr_err("filp open error.\n");
		ret = PTR_ERR(filp);
		goto rtn;
	}
	fs = get_fs();
	set_fs(get_ds());

	/* get file size */
	file_size = filp->f_dentry->d_inode->i_size;
	vcd_pr_spuv_info("file_size[%d].\n", file_size);

	/* set read file buffer */
	read_data = (char *)g_spuv_func_fw_static_buffer;

	/* initialize buffer */
	memset(read_data, 0, file_size);

	/* read file */
	read_size = filp->f_op->read(filp, read_data, file_size, &filp->f_pos);
	if (file_size != read_size) {
		vcd_pr_err("read error. (read_size[%d] / file_size[%d]).\n",
			read_size, file_size);
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}
	set_fs(fs);

	/* close file */
	filp_close(filp, NULL);

	/* set firmware to read_fw_info */
	while (1) {
		memory_type = *(read_data + current_offset +
			VCD_SPUV_FUNC_OFFSET_MEMORY_TYPE);
		page_number = *(read_data + current_offset +
			VCD_SPUV_FUNC_OFFSET_PAGE_NUM);
		page_data_addr = (read_data + current_offset +
			VCD_SPUV_FUNC_OFFSET_PAGE_DATA);
		page_size = *(unsigned int *)(read_data + current_offset +
			VCD_SPUV_FUNC_OFFSET_PAGE_SIZE) *
			VCD_SPUV_FUNC_UNIT_PAGE_SIZE;
		global_size = (*(read_data + current_offset +
			VCD_SPUV_FUNC_OFFSET_GLOBAL_SIZE) *
			VCD_SPUV_FUNC_UNIT_GLOBAL_SIZE);
		if (global_size == 0)
			global_size = VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_64KW;

		if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_PRAM) {
			read_fw_info.pram_addr[page_number] = page_data_addr;
			read_fw_info.pram_page_size[page_number] = page_size;
			read_fw_info.pram_global_size = global_size;
		} else if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_XRAM) {
			read_fw_info.xram_addr[page_number] = page_data_addr;
			read_fw_info.xram_page_size[page_number] = page_size;
			read_fw_info.xram_global_size = global_size;
		} else if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_YRAM) {
			read_fw_info.yram_addr[page_number] = page_data_addr;
			read_fw_info.yram_page_size[page_number] = page_size;
			read_fw_info.yram_global_size = global_size;
		} else {
			break;
		}

		current_offset += page_size + VCD_SPUV_FUNC_OFFSET_PAGE_DATA;
		if (file_size <= current_offset)
			break;
	}

	/* relocation fw buffer */
	ret = vcd_spuv_func_relocation_fw(&read_fw_info);
	if (0 != ret) {
		vcd_pr_err("copy firmware error. ret[%d].\n", ret);
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}
#else /* __VCD_SPUV_FW_FROM_SD_ENABLE__ */
/**
 * @brief	set spuv.bin function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE		successful.
 * @retval	VCD_ERR_FILE_NOT_FOUND	file not found.
 * @retval	VCD_ERR_SYSTEM		system error.
 * @retval	others			result of called function.
 */
int vcd_spuv_func_set_fw(void)
{
	int ret = VCD_ERR_NONE;
	char *read_data;
	struct vcd_spuv_func_read_fw_info read_fw_info = { {0} };
	void *page_data_addr = NULL;
	unsigned int current_offset = 0;
	unsigned int memory_type = 0;
	unsigned int page_number = 0;
	unsigned int global_size = 0;
	unsigned int page_size = 0;

	vcd_pr_start_spuv_function();

	/* set read file buffer */
	read_data = (char *)g_spuv_func_fw_static_buffer;

	/* set firmware to read_fw_info */
	while (1) {
		memory_type = *(read_data + current_offset +
				VCD_SPUV_FUNC_OFFSET_MEMORY_TYPE);
		page_number = *(read_data + current_offset +
				VCD_SPUV_FUNC_OFFSET_PAGE_NUM);
		page_data_addr = (read_data + current_offset +
				VCD_SPUV_FUNC_OFFSET_PAGE_DATA);
		page_size = *(unsigned int *)(read_data + current_offset +
				VCD_SPUV_FUNC_OFFSET_PAGE_SIZE) *
				VCD_SPUV_FUNC_UNIT_PAGE_SIZE;
		global_size = (*(read_data + current_offset +
				VCD_SPUV_FUNC_OFFSET_GLOBAL_SIZE) *
				VCD_SPUV_FUNC_UNIT_GLOBAL_SIZE);
		if (global_size == 0)
			global_size = VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_64KW;

		if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_PRAM) {
			read_fw_info.pram_addr[page_number] = page_data_addr;
			read_fw_info.pram_page_size[page_number] = page_size;
			read_fw_info.pram_global_size = global_size;
		} else if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_XRAM) {
			read_fw_info.xram_addr[page_number] = page_data_addr;
			read_fw_info.xram_page_size[page_number] = page_size;
			read_fw_info.xram_global_size = global_size;
		} else if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_YRAM) {
			read_fw_info.yram_addr[page_number] = page_data_addr;
			read_fw_info.yram_page_size[page_number] = page_size;
			read_fw_info.yram_global_size = global_size;
		}  else if (memory_type == VCD_SPUV_FUNC_MEMORY_TYPE_NONE) {
			/* read complete */
			break;
		} else {
			vcd_pr_err("unknown memory type. memory_type[%d].\n",
				memory_type);
			ret = VCD_ERR_SYSTEM;
			goto rtn;
		}

		current_offset += page_size + VCD_SPUV_FUNC_OFFSET_PAGE_DATA;
		if (VCD_SPUV_FUNC_FW_BUFFER_SIZE <= current_offset)
			break;
	}

	/* relocation fw buffer */
	ret = vcd_spuv_func_relocation_fw(&read_fw_info);
	if (0 != ret) {
		vcd_pr_err("copy firmware error. ret[%d].\n", ret);
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}
#endif /* __VCD_SPUV_FW_FROM_SD_ENABLE__ */


/**
 * @brief	dsp core reset function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_release_firmware(void)
{
	vcd_pr_start_spuv_function();

	/* free meram */
	if (0 != SPUV_FUNC_MERAM_FIRMWARE_BUFFER)
		vcd_spuv_func_free_meram();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dsp core reset function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dsp_core_reset(void)
{
	vcd_pr_start_spuv_function();

	/* wait on */
	g_spuv_func_is_completion = false;

	/* IEMASKC & IMASKC disable */
	vcd_spuv_func_set_register(VCD_SPUV_FUNC_IEMASKC_DISABLE,
						SPUV_FUNC_RW_32_IEMASKC);
	vcd_spuv_func_set_register(VCD_SPUV_FUNC_IMASKC_DSP_DISABLE,
						SPUV_FUNC_RW_32_IMASKC);

	/* dsp core reset */
	vcd_spuv_func_set_register(VCD_SPUV_FUNC_DSPCORERST_ACTIVE,
						SPUV_FUNC_RW_32_DSPCORERST);

	/* start wait */
	vcd_spuv_func_start_wait();

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	send message function.
 *
 * @param[in]	param	message address.
 * @param[in]	param	message length.
 *
 * @retval	none.
 */
void vcd_spuv_func_send_msg(int *param, int length)
{
	int i = 0;
	int *msg_buf_addr = 0;
	unsigned long msg_buf_physical_addr = 0;

	vcd_pr_start_spuv_function("param[%p], length[%d].\n", param, length);

	if (NULL == param) {
		vcd_pr_err("parameter error.\n");
		goto rtn;
	}

	/* set msg buffer */
	msg_buf_addr = (int *)SPUV_FUNC_SDRAM_SPUV_SEND_MSG_BUFFER;

	/* init msg buffer */
	memset((void *)msg_buf_addr, 0, (sizeof(int) * length));

	/* conversion format */
	vcd_pr_spuv_info("send param length[%d].\n", length);
	for (i = 0; length > i; i++) {
		vcd_pr_spuv_debug("param[%d][0x%02x].\n", i, param[i]);
		msg_buf_addr[i] =
			(param[i] << VCD_SPUV_FUNC_CPU_TO_DSP_BIT_SHIFT);
	}

	/* conversion address */
	msg_buf_physical_addr =
			vcd_spuv_func_sdram_logical_to_physical(
				SPUV_FUNC_SDRAM_SPUV_SEND_MSG_BUFFER);

	/* set com2 and com3 */
	vcd_spuv_func_set_register(((int)msg_buf_physical_addr &
			VCD_SPUV_FUNC_COM2_MASK),
			SPUV_FUNC_RW_32_COM2);
	vcd_spuv_func_set_register(((((int)msg_buf_physical_addr >> 8) &
			VCD_SPUV_FUNC_COM3_MASK) | length),
			SPUV_FUNC_RW_32_COM3);

	/* wait on */
	g_spuv_func_is_completion = false;

	/* set arm_msg_it */
	vcd_spuv_func_set_register(VCD_SPUV_FUNC_AMSGIT_MSG_REQ,
					SPUV_FUNC_RW_32_AMSGIT);
	udelay(1);
	vcd_spuv_func_set_register(VCD_SPUV_FUNC_AMSGIT_MSG_REQ,
					SPUV_FUNC_RW_32_AMSGIT);

	/* start wait */
	vcd_spuv_func_start_wait();

rtn:
	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	send ack message function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_send_ack(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_if_spuv("V --> F : ACK.\n");

	/* set arm_msg_it */
	vcd_spuv_func_modify_register(0,
		VCD_SPUV_FUNC_AMSGIT_MSG_ACK, SPUV_FUNC_RW_32_AMSGIT);
	udelay(1);
	vcd_spuv_func_modify_register(0,
		VCD_SPUV_FUNC_AMSGIT_MSG_ACK, SPUV_FUNC_RW_32_AMSGIT);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	get fw request function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_get_fw_request(void)
{
	int i = 0;
	unsigned long flags;
	unsigned int read_addr = 0;
	unsigned int *addr = 0;
	int length = 0;
	unsigned int *fw_req =
		(unsigned int *)SPUV_FUNC_SDRAM_FW_RESULT_BUFFER;

	vcd_pr_start_spuv_function();

	vcd_spuv_func_register(SPUV_FUNC_RW_32_COM0, read_addr);
	vcd_spuv_func_register(SPUV_FUNC_RW_32_COM1, length);
	vcd_pr_spuv_info("SPUV_FUNC_RW_32_COM0[%08x].\n", read_addr);
	vcd_pr_spuv_info("SPUV_FUNC_RW_32_COM1[%08x].\n", length);

	flags = pm_get_spinlock();

	if ((0 != read_addr) && (0 != length)) {
		addr = (unsigned int *)(g_spuv_func_xram_base_top +
			(read_addr * VCD_SPUV_FUNC_WORD_TO_BYTE));
		for (i = 0; i < length; i++) {
			fw_req[i] = *addr;
			fw_req[i] = ((fw_req[i] >> VCD_SPUV_FUNC_BIT_SHIFT) &
					VCD_SPUV_FUNC_COM2_MASK);
			addr++;
		}
	}

	pm_release_spinlock(flags);

	vcd_pr_end_spuv_function();
	return;
}


/* ========================================================================= */
/* Synchronous conversion functions                                          */
/* ========================================================================= */

/**
 * @brief	start wait function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_start_wait(void)
{
	vcd_pr_start_spuv_function();

	init_waitqueue_head(&g_vcd_spuv_wait);

	wait_event_interruptible_timeout(
		g_vcd_spuv_wait,
		g_spuv_func_is_completion,
		msecs_to_jiffies(VCD_SPUV_FUNC_MAX_WAIT_TIME));

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	end wait function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_end_wait(void)
{
	vcd_pr_start_spuv_function();

	g_spuv_func_is_completion = true;
	wake_up_interruptible(&g_vcd_spuv_wait);

	vcd_pr_end_spuv_function();
	return;
}


/* ========================================================================= */
/* Memory functions                                                          */
/* ========================================================================= */

/**
 * @brief	driver ioremap function.
 *
 * @param	none.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_SYSTEM	ioremap error.
 */
int vcd_spuv_func_ioremap(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	if ((system_rev & 0xff) == 0x00)
		g_spuv_func_sdram_static_area_top_phy =
			SPUV_FUNC_SDRAM_AREA_TOP_PHY_ES1;
	else
		g_spuv_func_sdram_static_area_top_phy =
			SPUV_FUNC_SDRAM_AREA_TOP_PHY_ES2;

	/* ioremap sdram */
	g_spuv_func_sdram_static_area_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_SDRAM_AREA_TOP_PHY,
			SPUV_FUNC_SDRAM_AREA_SIZE);
	if (g_spuv_func_sdram_static_area_top == 0) {
		vcd_pr_err("error ioremap sdram.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap hpb */
	g_spuv_func_hpb_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_HPB_REG_TOP_PHY,
			SPUV_FUNC_HPB_REG_SIZE);
	if (g_spuv_func_hpb_register_top == 0) {
		vcd_pr_err("error ioremap hpb.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap cpg */
	g_spuv_func_cpg_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_CPG_REG_TOP_PHY,
			SPUV_FUNC_CPG_REG_SIZE);
	if (g_spuv_func_cpg_register_top == 0) {
		vcd_pr_err("error ioremap cpg.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap crmu */
	g_spuv_func_crmu_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_CRMU_REG_TOP_PHY,
			SPUV_FUNC_CRMU_REG_SIZE);
	if (g_spuv_func_crmu_register_top == 0) {
		vcd_pr_err("error ioremap crmu.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap gtu */
	g_spuv_func_gtu_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_GTU_REG_TOP_PHY,
			SPUV_FUNC_GTU_REG_SIZE);
	if (g_spuv_func_gtu_register_top == 0) {
		vcd_pr_err("error ioremap gtu.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap voiceif */
	g_spuv_func_voiceif_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_VOICEIF_REG_TOP_PHY,
			SPUV_FUNC_VOICEIF_REG_SIZE);
	if (g_spuv_func_voiceif_register_top == 0) {
		vcd_pr_err("error ioremap voiceif.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap intcvo */
	g_spuv_func_intcvo_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_INTCVO_REG_TOP_PHY,
			SPUV_FUNC_INTCVO_REG_SIZE);
	if (g_spuv_func_intcvo_register_top == 0) {
		vcd_pr_err("error ioremap intcvo.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap spuv */
	g_spuv_func_spuv_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_SPUV_REG_TOP_PHY,
			SPUV_FUNC_SPUV_REG_SIZE);
	if (g_spuv_func_spuv_register_top == 0) {
		vcd_pr_err("error ioremap spuv.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap dsp0 */
	g_spuv_func_dsp0_register_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_DSP0_REG_TOP_PHY,
			SPUV_FUNC_DSP0_REG_SIZE);
	if (g_spuv_func_dsp0_register_top == 0) {
		vcd_pr_err("error ioremap dsp0.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

	/* ioremap xram0 */
	g_spuv_func_xram_base_top =
		(unsigned int)ioremap_nocache(
			SPUV_FUNC_XRAM0_PHY,
			SPUV_FUNC_DATA_RAM_SIZE);
	if (g_spuv_func_dsp0_register_top == 0) {
		vcd_pr_err("error ioremap xram0.\n");
		ret = VCD_ERR_SYSTEM;
		goto rtn;
	}

rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	driver iounmap function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_iounmap(void)
{
	vcd_pr_start_spuv_function();

	/* iounmap sdram */
	if (0 != g_spuv_func_sdram_static_area_top) {
		iounmap((void *)g_spuv_func_sdram_static_area_top);
		g_spuv_func_sdram_static_area_top = 0;
	}

	/* iounmap hpb */
	if (0 != g_spuv_func_hpb_register_top) {
		iounmap((void *)g_spuv_func_hpb_register_top);
		g_spuv_func_hpb_register_top = 0;
	}

	/* iounmap cpg */
	if (0 != g_spuv_func_cpg_register_top) {
		iounmap((void *)g_spuv_func_cpg_register_top);
		g_spuv_func_cpg_register_top = 0;
	}

	/* iounmap crmu */
	if (0 != g_spuv_func_crmu_register_top) {
		iounmap((void *)g_spuv_func_crmu_register_top);
		g_spuv_func_crmu_register_top = 0;
	}

	/* iounmap gtu */
	if (0 != g_spuv_func_gtu_register_top) {
		iounmap((void *)g_spuv_func_gtu_register_top);
		g_spuv_func_gtu_register_top = 0;
	}

	/* iounmap voiceif */
	if (0 != g_spuv_func_voiceif_register_top) {
		iounmap((void *)g_spuv_func_voiceif_register_top);
		g_spuv_func_voiceif_register_top = 0;
	}

	/* iounmap intcvo */
	if (0 != g_spuv_func_intcvo_register_top) {
		iounmap((void *)g_spuv_func_intcvo_register_top);
		g_spuv_func_intcvo_register_top = 0;
	}

	/* iounmap spuv */
	if (0 != g_spuv_func_spuv_register_top) {
		iounmap((void *)g_spuv_func_spuv_register_top);
		g_spuv_func_spuv_register_top = 0;
	}

	/* iounmap dsp0 */
	if (0 != g_spuv_func_dsp0_register_top) {
		iounmap((void *)g_spuv_func_dsp0_register_top);
		g_spuv_func_dsp0_register_top = 0;
	}

	/* ioremap xram0 */
	if (0 != g_spuv_func_xram_base_top) {
		iounmap((void *)g_spuv_func_xram_base_top);
		g_spuv_func_xram_base_top = 0;
	}

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	get fw buffer function.
 *
 * @param	none.
 *
 * @retval	none.
 */
int vcd_spuv_func_get_fw_buffer(void)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function();

	/* get buffer */
	g_spuv_func_fw_static_buffer = vmalloc(VCD_SPUV_FUNC_FW_BUFFER_SIZE);
	if (NULL == g_spuv_func_fw_static_buffer) {
		vcd_pr_err("vmalloc error.\n");
		g_spuv_func_fw_static_buffer = NULL;
		ret = -EBUSY;
		goto rtn;
	}

	memset(g_spuv_func_fw_static_buffer, 0, VCD_SPUV_FUNC_FW_BUFFER_SIZE);
	memcpy(g_spuv_func_fw_static_buffer,
		(const void *)SPUV_FUNC_SDRAM_FIRMWARE_READ_BUFFER,
		VCD_SPUV_FUNC_FW_BUFFER_SIZE);
	memset((void *)SPUV_FUNC_SDRAM_FIRMWARE_READ_BUFFER,
					0, VCD_SPUV_FUNC_FW_BUFFER_SIZE);

rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	free fw buffer function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_free_fw_buffer(void)
{
	vcd_pr_start_spuv_function();

	if (NULL != g_spuv_func_fw_static_buffer) {
		/* free buffer */
		vfree(g_spuv_func_fw_static_buffer);
	}

	vcd_pr_end_spuv_function();
	return;
}


/* ========================================================================= */
/* Register dump functions                                                   */
/* ========================================================================= */

/**
 * @brief	dump CPG registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_cpg_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("VCLKCR1      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_VCLKCR1),
		SPUV_FUNC_RW_32_CPG_VCLKCR1);
	vcd_pr_registers_dump("VCLKCR2      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_VCLKCR2),
		SPUV_FUNC_RW_32_CPG_VCLKCR2);
	vcd_pr_registers_dump("VCLKCR3      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_VCLKCR3),
		SPUV_FUNC_RW_32_CPG_VCLKCR3);
	vcd_pr_registers_dump("VCLKCR4      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_VCLKCR4),
		SPUV_FUNC_RW_32_CPG_VCLKCR4);
	vcd_pr_registers_dump("VCLKCR5      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_VCLKCR5),
		SPUV_FUNC_RW_32_CPG_VCLKCR5);
	vcd_pr_registers_dump("FSIACKCR     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_FSIACKCR),
		SPUV_FUNC_RW_32_CPG_FSIACKCR);
	vcd_pr_registers_dump("FSIBCKCR     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_FSIBCKCR),
		SPUV_FUNC_RW_32_CPG_FSIBCKCR);
	vcd_pr_registers_dump("SPUACKCR     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SPUACKCR),
		SPUV_FUNC_RW_32_CPG_SPUACKCR);
	vcd_pr_registers_dump("SPUVCKCR     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SPUVCKCR),
		SPUV_FUNC_RW_32_CPG_SPUVCKCR);
	vcd_pr_registers_dump("HSICKCR      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_HSICKCR),
		SPUV_FUNC_RW_32_CPG_HSICKCR);
	vcd_pr_registers_dump("MSTPSR0      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR0),
		SPUV_FUNC_RO_32_CPG_MSTPSR0);
	vcd_pr_registers_dump("MSTPSR1      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR1),
		SPUV_FUNC_RO_32_CPG_MSTPSR1);
	vcd_pr_registers_dump("MSTPSR2      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR2),
		SPUV_FUNC_RO_32_CPG_MSTPSR2);
	vcd_pr_registers_dump("MSTPSR3      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR3),
		SPUV_FUNC_RO_32_CPG_MSTPSR3);
	vcd_pr_registers_dump("MSTPSR4      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR4),
		SPUV_FUNC_RO_32_CPG_MSTPSR4);
	vcd_pr_registers_dump("MSTPSR5      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR5),
		SPUV_FUNC_RO_32_CPG_MSTPSR5);
	vcd_pr_registers_dump("MSTPSR6      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_CPG_MSTPSR6),
		SPUV_FUNC_RO_32_CPG_MSTPSR6);
	vcd_pr_registers_dump("RMSTPCR0     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR0),
		SPUV_FUNC_RW_32_CPG_RMSTPCR0);
	vcd_pr_registers_dump("RMSTPCR1     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR1),
		SPUV_FUNC_RW_32_CPG_RMSTPCR1);
	vcd_pr_registers_dump("RMSTPCR2     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR2),
		SPUV_FUNC_RW_32_CPG_RMSTPCR2);
	vcd_pr_registers_dump("RMSTPCR3     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR3),
		SPUV_FUNC_RW_32_CPG_RMSTPCR3);
	vcd_pr_registers_dump("RMSTPCR4     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR4),
		SPUV_FUNC_RW_32_CPG_RMSTPCR4);
	vcd_pr_registers_dump("RMSTPCR5     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR5),
		SPUV_FUNC_RW_32_CPG_RMSTPCR5);
	vcd_pr_registers_dump("RMSTPCR6     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_RMSTPCR6),
		SPUV_FUNC_RW_32_CPG_RMSTPCR6);
	vcd_pr_registers_dump("SMSTPCR0     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR0),
		SPUV_FUNC_RW_32_CPG_SMSTPCR0);
	vcd_pr_registers_dump("SMSTPCR1     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR1),
		SPUV_FUNC_RW_32_CPG_SMSTPCR1);
	vcd_pr_registers_dump("SMSTPCR2     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR2),
		SPUV_FUNC_RW_32_CPG_SMSTPCR2);
	vcd_pr_registers_dump("SMSTPCR3     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR3),
		SPUV_FUNC_RW_32_CPG_SMSTPCR3);
	vcd_pr_registers_dump("SMSTPCR4     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR4),
		SPUV_FUNC_RW_32_CPG_SMSTPCR4);
	vcd_pr_registers_dump("SMSTPCR5     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR5),
		SPUV_FUNC_RW_32_CPG_SMSTPCR5);
	vcd_pr_registers_dump("SMSTPCR6     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SMSTPCR6),
		SPUV_FUNC_RW_32_CPG_SMSTPCR6);
	vcd_pr_registers_dump("MMSTPCR0     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR0),
		SPUV_FUNC_RW_32_CPG_MMSTPCR0);
	vcd_pr_registers_dump("MMSTPCR1     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR1),
		SPUV_FUNC_RW_32_CPG_MMSTPCR1);
	vcd_pr_registers_dump("MMSTPCR2     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR2),
		SPUV_FUNC_RW_32_CPG_MMSTPCR2);
	vcd_pr_registers_dump("MMSTPCR3     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR3),
		SPUV_FUNC_RW_32_CPG_MMSTPCR3);
	vcd_pr_registers_dump("MMSTPCR4     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR4),
		SPUV_FUNC_RW_32_CPG_MMSTPCR4);
	vcd_pr_registers_dump("MMSTPCR5     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR5),
		SPUV_FUNC_RW_32_CPG_MMSTPCR5);
	vcd_pr_registers_dump("MMSTPCR6     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_MMSTPCR6),
		SPUV_FUNC_RW_32_CPG_MMSTPCR6);
	vcd_pr_registers_dump("SRCR0        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR0),
		SPUV_FUNC_RW_32_CPG_SRCR0);
	vcd_pr_registers_dump("SRCR1        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR1),
		SPUV_FUNC_RW_32_CPG_SRCR1);
	vcd_pr_registers_dump("SRCR2        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR2),
		SPUV_FUNC_RW_32_CPG_SRCR2);
	vcd_pr_registers_dump("SRCR3        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR3),
		SPUV_FUNC_RW_32_CPG_SRCR3);
	vcd_pr_registers_dump("SRCR4        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR4),
		SPUV_FUNC_RW_32_CPG_SRCR4);
	vcd_pr_registers_dump("SRCR5        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR5),
		SPUV_FUNC_RW_32_CPG_SRCR5);
	vcd_pr_registers_dump("SRCR6        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_SRCR6),
		SPUV_FUNC_RW_32_CPG_SRCR6);
	vcd_pr_registers_dump("CKSCR        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CPG_CKSCR),
		SPUV_FUNC_RW_32_CPG_CKSCR);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump CRMU registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_crmu_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("CRMU_GTU     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CRMU_GTU),
		SPUV_FUNC_RW_32_CRMU_GTU);
	vcd_pr_registers_dump("CRMU_VOICEIF [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CRMU_VOICEIF),
		SPUV_FUNC_RW_32_CRMU_VOICEIF);
	vcd_pr_registers_dump("CRMU_INTCVO  [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CRMU_INTCVO),
		SPUV_FUNC_RW_32_CRMU_INTCVO);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump GTU registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_gtu_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("SEL_20_PLS   [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SEL_20_PLS),
		SPUV_FUNC_RW_32_SEL_20_PLS);
	vcd_pr_registers_dump("PG_LOOP      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PG_LOOP),
		SPUV_FUNC_RW_32_PG_LOOP);
	vcd_pr_registers_dump("PG_RELOAD    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PG_RELOAD),
		SPUV_FUNC_RW_32_PG_RELOAD);
	vcd_pr_registers_dump("PG_MIN       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PG_MIN),
		SPUV_FUNC_RW_32_PG_MIN);
	vcd_pr_registers_dump("PG_MAX       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PG_MAX),
		SPUV_FUNC_RW_32_PG_MAX);
	vcd_pr_registers_dump("PG_20_EN     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PG_20_EN),
		SPUV_FUNC_RW_32_PG_20_EN);
	vcd_pr_registers_dump("PC_PH_STS    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_PC_PH_STS),
		SPUV_FUNC_RO_32_PC_PH_STS);
	vcd_pr_registers_dump("PC_DET_EN    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PC_DET_EN),
		SPUV_FUNC_RW_32_PC_DET_EN);
	vcd_pr_registers_dump("PC_MIN_TH    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PC_MIN_TH),
		SPUV_FUNC_RW_32_PC_MIN_TH);
	vcd_pr_registers_dump("PC_MAX_TH    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PC_MAX_TH),
		SPUV_FUNC_RW_32_PC_MAX_TH);
	vcd_pr_registers_dump("EN_TRG       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_EN_TRG),
		SPUV_FUNC_RW_32_EN_TRG);
	vcd_pr_registers_dump("DLY_P_UL     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DLY_P_UL),
		SPUV_FUNC_RW_32_DLY_P_UL);
	vcd_pr_registers_dump("DLY_P_DL     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DLY_P_DL),
		SPUV_FUNC_RW_32_DLY_P_DL);
	vcd_pr_registers_dump("DLY_B_UL     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DLY_B_UL),
		SPUV_FUNC_RW_32_DLY_B_UL);
	vcd_pr_registers_dump("DLY_SP_0     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DLY_SP_0),
		SPUV_FUNC_RW_32_DLY_SP_0);
	vcd_pr_registers_dump("DLY_SP_1     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DLY_SP_1),
		SPUV_FUNC_RW_32_DLY_SP_1);
	vcd_pr_registers_dump("PC_RL_VL     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_PC_RL_VL),
		SPUV_FUNC_RW_32_PC_RL_VL);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump VOICEIF registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_voiceif_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("UL1_BUF      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_UL1_BUF),
		SPUV_FUNC_RO_32_UL1_BUF);
	vcd_pr_registers_dump("UL2_BUF      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_UL2_BUF),
		SPUV_FUNC_RO_32_UL2_BUF);
	vcd_pr_registers_dump("UL3_BUF      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_UL3_BUF),
		SPUV_FUNC_RO_32_UL3_BUF);
	vcd_pr_registers_dump("UL4_BUF      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_UL4_BUF),
		SPUV_FUNC_RO_32_UL4_BUF);
	vcd_pr_registers_dump("UL5_BUF      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_UL5_BUF),
		SPUV_FUNC_RO_32_UL5_BUF);
	/* vcd_pr_registers_dump("DL_BUF       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_WO_32_DL_BUF),
		SPUV_FUNC_WO_32_DL_BUF); */
	vcd_pr_registers_dump("UL_EN1       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_UL_EN1),
		SPUV_FUNC_RW_32_UL_EN1);
	vcd_pr_registers_dump("UL_EN2       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_UL_EN2),
		SPUV_FUNC_RW_32_UL_EN2);
	vcd_pr_registers_dump("U1_P_LT      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_U1_P_LT),
		SPUV_FUNC_RO_32_U1_P_LT);
	vcd_pr_registers_dump("U2_P_LT      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_U2_P_LT),
		SPUV_FUNC_RO_32_U2_P_LT);
	vcd_pr_registers_dump("D_BUF_FL     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_D_BUF_FL),
		SPUV_FUNC_RW_32_D_BUF_FL);
	vcd_pr_registers_dump("D_P_LT       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_D_P_LT),
		SPUV_FUNC_RO_32_D_P_LT);
	vcd_pr_registers_dump("UL_EN3       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_UL_EN3),
		SPUV_FUNC_RW_32_UL_EN3);
	vcd_pr_registers_dump("UL_EN4       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_UL_EN4),
		SPUV_FUNC_RW_32_UL_EN4);
	vcd_pr_registers_dump("UL_EN5       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_UL_EN5),
		SPUV_FUNC_RW_32_UL_EN5);
	vcd_pr_registers_dump("U3_P_LT      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_U3_P_LT),
		SPUV_FUNC_RO_32_U3_P_LT);
	vcd_pr_registers_dump("U4_P_LT      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_U4_P_LT),
		SPUV_FUNC_RO_32_U4_P_LT);
	vcd_pr_registers_dump("U5_P_LT      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_U5_P_LT),
		SPUV_FUNC_RO_32_U5_P_LT);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump INTCVO registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_intcvo_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("DINTEN       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DINTEN),
		SPUV_FUNC_RW_32_DINTEN);
	vcd_pr_registers_dump("DINTMASK     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DINTMASK),
		SPUV_FUNC_RW_32_DINTMASK);
	vcd_pr_registers_dump("DINTCLR      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DINTCLR),
		SPUV_FUNC_RW_32_DINTCLR);
	vcd_pr_registers_dump("DINTSTS      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_DINTSTS),
		SPUV_FUNC_RO_32_DINTSTS);
	vcd_pr_registers_dump("AMSGIT       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_AMSGIT),
		SPUV_FUNC_RW_32_AMSGIT);
	vcd_pr_registers_dump("AINTEN       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_AINTEN),
		SPUV_FUNC_RW_32_AINTEN);
	vcd_pr_registers_dump("AINTMASK     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_AINTMASK),
		SPUV_FUNC_RW_32_AINTMASK);
	vcd_pr_registers_dump("AINTCLR      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_AINTCLR),
		SPUV_FUNC_RW_32_AINTCLR);
	vcd_pr_registers_dump("AINTSTS      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_AINTSTS),
		SPUV_FUNC_RO_32_AINTSTS);
	vcd_pr_registers_dump("BBINTSET     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_BBINTSET),
		SPUV_FUNC_RW_32_BBINTSET);
	vcd_pr_registers_dump("SHINTSET     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SHINTSET),
		SPUV_FUNC_RW_32_SHINTSET);
	vcd_pr_registers_dump("V20MSITEN    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_V20MSITEN),
		SPUV_FUNC_RW_32_V20MSITEN);
	vcd_pr_registers_dump("V20MSITCLR   [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_V20MSITCLR),
		SPUV_FUNC_RW_32_V20MSITCLR);
	vcd_pr_registers_dump("V20MSITSTAT  [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_V20MSITSTAT),
		SPUV_FUNC_RO_32_V20MSITSTAT);
	vcd_pr_registers_dump("BBITCLR      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_BBITCLR),
		SPUV_FUNC_RW_32_BBITCLR);
	vcd_pr_registers_dump("SHITCLR      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SHITCLR),
		SPUV_FUNC_RW_32_SHITCLR);
	vcd_pr_registers_dump("BBITSTAT     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_BBITSTAT),
		SPUV_FUNC_RO_32_BBITSTAT);
	vcd_pr_registers_dump("SHITSTAT     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_SHITSTAT),
		SPUV_FUNC_RO_32_SHITSTAT);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump spuv registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_spuv_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("CCTL         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CCTL),
		SPUV_FUNC_RW_32_CCTL);
	vcd_pr_registers_dump("P0RAM0       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE0),
		SPUV_FUNC_RW_32_P0BASE0);
	vcd_pr_registers_dump("X0RAM0       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE0),
		SPUV_FUNC_RW_32_X0BASE0);
	vcd_pr_registers_dump("Y0RAM0       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE0),
		SPUV_FUNC_RW_32_Y0BASE0);
	vcd_pr_registers_dump("SPUMSTS      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_SPUMSTS),
		SPUV_FUNC_RO_32_SPUMSTS);
	vcd_pr_registers_dump("P0RAM1       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE1),
		SPUV_FUNC_RW_32_P0BASE1);
	vcd_pr_registers_dump("X0RAM1       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE1),
		SPUV_FUNC_RW_32_X0BASE1);
	vcd_pr_registers_dump("Y0RAM1       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE1),
		SPUV_FUNC_RW_32_Y0BASE1);
	vcd_pr_registers_dump("P0RAM2       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE2),
		SPUV_FUNC_RW_32_P0BASE2);
	vcd_pr_registers_dump("X0RAM2       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE2),
		SPUV_FUNC_RW_32_X0BASE2);
	vcd_pr_registers_dump("Y0RAM2       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE2),
		SPUV_FUNC_RW_32_Y0BASE2);
	vcd_pr_registers_dump("P0RAM3       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE3),
		SPUV_FUNC_RW_32_P0BASE3);
	vcd_pr_registers_dump("X0RAM3       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE3),
		SPUV_FUNC_RW_32_X0BASE3);
	vcd_pr_registers_dump("Y0RAM3       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE3),
		SPUV_FUNC_RW_32_Y0BASE3);
	vcd_pr_registers_dump("P0RAM4       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE4),
		SPUV_FUNC_RW_32_P0BASE4);
	vcd_pr_registers_dump("X0RAM4       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE4),
		SPUV_FUNC_RW_32_X0BASE4);
	vcd_pr_registers_dump("Y0RAM4       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE4),
		SPUV_FUNC_RW_32_Y0BASE4);
	vcd_pr_registers_dump("P0RAM5       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE5),
		SPUV_FUNC_RW_32_P0BASE5);
	vcd_pr_registers_dump("X0RAM5       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE5),
		SPUV_FUNC_RW_32_X0BASE5);
	vcd_pr_registers_dump("Y0RAM5       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE5),
		SPUV_FUNC_RW_32_Y0BASE5);
	vcd_pr_registers_dump("P0RAM6       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE6),
		SPUV_FUNC_RW_32_P0BASE6);
	vcd_pr_registers_dump("X0RAM6       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE6),
		SPUV_FUNC_RW_32_X0BASE6);
	vcd_pr_registers_dump("Y0RAM6       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE6),
		SPUV_FUNC_RW_32_Y0BASE6);
	vcd_pr_registers_dump("P0RAM7       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE7),
		SPUV_FUNC_RW_32_P0BASE7);
	vcd_pr_registers_dump("X0RAM7       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE7),
		SPUV_FUNC_RW_32_X0BASE7);
	vcd_pr_registers_dump("Y0RAM7       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE7),
		SPUV_FUNC_RW_32_Y0BASE7);
	vcd_pr_registers_dump("P0RAM8       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE8),
		SPUV_FUNC_RW_32_P0BASE8);
	vcd_pr_registers_dump("X0RAM8       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE8),
		SPUV_FUNC_RW_32_X0BASE8);
	vcd_pr_registers_dump("Y0RAM8       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE8),
		SPUV_FUNC_RW_32_Y0BASE8);
	vcd_pr_registers_dump("P0RAM9       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE9),
		SPUV_FUNC_RW_32_P0BASE9);
	vcd_pr_registers_dump("X0RAM9       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE9),
		SPUV_FUNC_RW_32_X0BASE9);
	vcd_pr_registers_dump("Y0RAM9       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE9),
		SPUV_FUNC_RW_32_Y0BASE9);
	vcd_pr_registers_dump("P0RAM10      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE10),
		SPUV_FUNC_RW_32_P0BASE10);
	vcd_pr_registers_dump("X0RAM10      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE10),
		SPUV_FUNC_RW_32_X0BASE10);
	vcd_pr_registers_dump("Y0RAM10      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE10),
		SPUV_FUNC_RW_32_Y0BASE10);
	vcd_pr_registers_dump("P0RAM11      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE11),
		SPUV_FUNC_RW_32_P0BASE11);
	vcd_pr_registers_dump("X0RAM11      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE11),
		SPUV_FUNC_RW_32_X0BASE11);
	vcd_pr_registers_dump("Y0RAM11      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE11),
		SPUV_FUNC_RW_32_Y0BASE11);
	vcd_pr_registers_dump("P0RAM12      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE12),
		SPUV_FUNC_RW_32_P0BASE12);
	vcd_pr_registers_dump("X0RAM12      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE12),
		SPUV_FUNC_RW_32_X0BASE12);
	vcd_pr_registers_dump("Y0RAM12      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE12),
		SPUV_FUNC_RW_32_Y0BASE12);
	vcd_pr_registers_dump("P0RAM13      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE13),
		SPUV_FUNC_RW_32_P0BASE13);
	vcd_pr_registers_dump("X0RAM13      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE13),
		SPUV_FUNC_RW_32_X0BASE13);
	vcd_pr_registers_dump("Y0RAM13      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE13),
		SPUV_FUNC_RW_32_Y0BASE13);
	vcd_pr_registers_dump("P0RAM14      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE14),
		SPUV_FUNC_RW_32_P0BASE14);
	vcd_pr_registers_dump("X0RAM14      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE14),
		SPUV_FUNC_RW_32_X0BASE14);
	vcd_pr_registers_dump("Y0RAM14      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE14),
		SPUV_FUNC_RW_32_Y0BASE14);
	vcd_pr_registers_dump("P0RAM15      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_P0BASE15),
		SPUV_FUNC_RW_32_P0BASE15);
	vcd_pr_registers_dump("X0RAM15      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_X0BASE15),
		SPUV_FUNC_RW_32_X0BASE15);
	vcd_pr_registers_dump("Y0RAM15      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_Y0BASE15),
		SPUV_FUNC_RW_32_Y0BASE15);
	vcd_pr_registers_dump("CMOD         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CMOD),
		SPUV_FUNC_RW_32_CMOD);
	vcd_pr_registers_dump("SPUSRST      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SPUSRST),
		SPUV_FUNC_RW_32_SPUSRST);
	vcd_pr_registers_dump("SPUADR       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_SPUADR),
		SPUV_FUNC_RO_32_SPUADR);
	vcd_pr_registers_dump("ENDIAN       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_ENDIAN),
		SPUV_FUNC_RO_32_ENDIAN);
	vcd_pr_registers_dump("GCOM0        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM0),
		SPUV_FUNC_RW_32_GCOM0);
	vcd_pr_registers_dump("GCOM1        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM1),
		SPUV_FUNC_RW_32_GCOM1);
	vcd_pr_registers_dump("GCOM2        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM2),
		SPUV_FUNC_RW_32_GCOM2);
	vcd_pr_registers_dump("GCOM3        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM3),
		SPUV_FUNC_RW_32_GCOM3);
	vcd_pr_registers_dump("GCOM4        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM4),
		SPUV_FUNC_RW_32_GCOM4);
	vcd_pr_registers_dump("GCOM5        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM5),
		SPUV_FUNC_RW_32_GCOM5);
	vcd_pr_registers_dump("GCOM6        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM6),
		SPUV_FUNC_RW_32_GCOM6);
	vcd_pr_registers_dump("GCOM7        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCOM7),
		SPUV_FUNC_RW_32_GCOM7);
	vcd_pr_registers_dump("GCLK_CTRL    [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GCLK_CTRL),
		SPUV_FUNC_RW_32_GCLK_CTRL);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	dump dsp0 registers function.
 *
 * @param	none.
 *
 * @retval	none.
 */
void vcd_spuv_func_dump_dsp0_registers(void)
{
	vcd_pr_start_spuv_function();

	vcd_pr_registers_dump("DSPRST       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DSPRST),
		SPUV_FUNC_RW_32_DSPRST);
	vcd_pr_registers_dump("DSPCORERST   [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DSPCORERST),
		SPUV_FUNC_RW_32_DSPCORERST);
	vcd_pr_registers_dump("DSPHOLD      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_DSPHOLD),
		SPUV_FUNC_RO_32_DSPHOLD);
	vcd_pr_registers_dump("DSPRESTART   [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DSPRESTART),
		SPUV_FUNC_RW_32_DSPRESTART);
	vcd_pr_registers_dump("IEMASKC      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_IEMASKC),
		SPUV_FUNC_RW_32_IEMASKC);
	vcd_pr_registers_dump("IMASKC       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_IMASKC),
		SPUV_FUNC_RW_32_IMASKC);
	vcd_pr_registers_dump("IEVENTC      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_IEVENTC),
		SPUV_FUNC_RW_32_IEVENTC);
	/* vcd_pr_registers_dump("IEMASKD      [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_OO_32_IEMASKD),
		SPUV_FUNC_OO_32_IEMASKD); */
	/* vcd_pr_registers_dump("IMASKD       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_OO_32_IMASKD),
		SPUV_FUNC_OO_32_IMASKD); */
	vcd_pr_registers_dump("IESETD       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_IESETD),
		SPUV_FUNC_RW_32_IESETD);
	/* vcd_pr_registers_dump("IECLRD       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_OO_32_IECLRD),
		SPUV_FUNC_OO_32_IECLRD); */
	vcd_pr_registers_dump("OR           [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_OR),
		SPUV_FUNC_RW_32_OR);
	vcd_pr_registers_dump("COM0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM0),
		SPUV_FUNC_RW_32_COM0);
	vcd_pr_registers_dump("COM1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM1),
		SPUV_FUNC_RW_32_COM1);
	vcd_pr_registers_dump("COM2         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM2),
		SPUV_FUNC_RW_32_COM2);
	vcd_pr_registers_dump("COM3         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM3),
		SPUV_FUNC_RW_32_COM3);
	vcd_pr_registers_dump("COM4         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM4),
		SPUV_FUNC_RW_32_COM4);
	vcd_pr_registers_dump("COM5         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM5),
		SPUV_FUNC_RW_32_COM5);
	vcd_pr_registers_dump("COM6         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM6),
		SPUV_FUNC_RW_32_COM6);
	vcd_pr_registers_dump("COM7         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_COM7),
		SPUV_FUNC_RW_32_COM7);
	vcd_pr_registers_dump("BTADRU       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_BTADRU),
		SPUV_FUNC_RW_32_BTADRU);
	vcd_pr_registers_dump("BTADRL       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_BTADRL),
		SPUV_FUNC_RW_32_BTADRL);
	vcd_pr_registers_dump("WDATU        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_WDATU),
		SPUV_FUNC_RW_32_WDATU);
	vcd_pr_registers_dump("WDATL        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_WDATL),
		SPUV_FUNC_RW_32_WDATL);
	vcd_pr_registers_dump("RDATU        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_RDATU),
		SPUV_FUNC_RO_32_RDATU);
	vcd_pr_registers_dump("RDATL        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_RDATL),
		SPUV_FUNC_RO_32_RDATL);
	vcd_pr_registers_dump("BTCTRL       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_BTCTRL),
		SPUV_FUNC_RO_32_BTCTRL);
	vcd_pr_registers_dump("SPUSTS       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_SPUSTS),
		SPUV_FUNC_RO_32_SPUSTS);
	vcd_pr_registers_dump("GPI0_INTBIT  [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GPI0_INTBIT),
		SPUV_FUNC_RW_32_GPI0_INTBIT);
	vcd_pr_registers_dump("GPI1_INTBIT  [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GPI1_INTBIT),
		SPUV_FUNC_RW_32_GPI1_INTBIT);
	vcd_pr_registers_dump("GPI0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_GPI0),
		SPUV_FUNC_RO_32_GPI0);
	vcd_pr_registers_dump("GPI1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RO_32_GPI1),
		SPUV_FUNC_RO_32_GPI1);
	/* vcd_pr_registers_dump("_GO_TO_SLEEP [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_--_--_GO_TO_SLEEP),
		SPUV_FUNC_--_--_GO_TO_SLEEP); */
	vcd_pr_registers_dump("SBAR0        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SBAR0),
		SPUV_FUNC_RW_32_SBAR0);
	vcd_pr_registers_dump("SAR0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SAR0),
		SPUV_FUNC_RW_32_SAR0);
	vcd_pr_registers_dump("DBAR0        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DBAR0),
		SPUV_FUNC_RW_32_DBAR0);
	vcd_pr_registers_dump("DAR0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DAR0),
		SPUV_FUNC_RW_32_DAR0);
	vcd_pr_registers_dump("TCR0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_TCR0),
		SPUV_FUNC_RW_32_TCR0);
	vcd_pr_registers_dump("SHPRI0       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SHPRI0),
		SPUV_FUNC_RW_32_SHPRI0);
	vcd_pr_registers_dump("CHCR0        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CHCR0),
		SPUV_FUNC_RW_32_CHCR0);
	vcd_pr_registers_dump("SBAR1        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SBAR1),
		SPUV_FUNC_RW_32_SBAR1);
	vcd_pr_registers_dump("SAR1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SAR1),
		SPUV_FUNC_RW_32_SAR1);
	vcd_pr_registers_dump("DBAR1        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DBAR1),
		SPUV_FUNC_RW_32_DBAR1);
	vcd_pr_registers_dump("DAR1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DAR1),
		SPUV_FUNC_RW_32_DAR1);
	vcd_pr_registers_dump("TCR1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_TCR1),
		SPUV_FUNC_RW_32_TCR1);
	vcd_pr_registers_dump("SHPRI1       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SHPRI1),
		SPUV_FUNC_RW_32_SHPRI1);
	vcd_pr_registers_dump("CHCR1        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CHCR1),
		SPUV_FUNC_RW_32_CHCR1);
	vcd_pr_registers_dump("SBAR2        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SBAR2),
		SPUV_FUNC_RW_32_SBAR2);
	vcd_pr_registers_dump("SAR2         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SAR2),
		SPUV_FUNC_RW_32_SAR2);
	vcd_pr_registers_dump("DBAR2        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DBAR2),
		SPUV_FUNC_RW_32_DBAR2);
	vcd_pr_registers_dump("DAR2         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_DAR2),
		SPUV_FUNC_RW_32_DAR2);
	vcd_pr_registers_dump("TCR2         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_TCR2),
		SPUV_FUNC_RW_32_TCR2);
	vcd_pr_registers_dump("SHPRI2       [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_SHPRI2),
		SPUV_FUNC_RW_32_SHPRI2);
	vcd_pr_registers_dump("CHCR2        [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_CHCR2),
		SPUV_FUNC_RW_32_CHCR2);
	vcd_pr_registers_dump("LSA0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_LSA0),
		SPUV_FUNC_RW_32_LSA0);
	vcd_pr_registers_dump("LEA0         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_LEA0),
		SPUV_FUNC_RW_32_LEA0);
	vcd_pr_registers_dump("LSA1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_LSA1),
		SPUV_FUNC_RW_32_LSA1);
	vcd_pr_registers_dump("LEA1         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_LEA1),
		SPUV_FUNC_RW_32_LEA1);
	vcd_pr_registers_dump("LSA2         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_LSA2),
		SPUV_FUNC_RW_32_LSA2);
	vcd_pr_registers_dump("LEA2         [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_LEA2),
		SPUV_FUNC_RW_32_LEA2);
	vcd_pr_registers_dump("ASID_DSP     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_ASID_DSP),
		SPUV_FUNC_RW_32_ASID_DSP);
	vcd_pr_registers_dump("ASID_CPU     [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_ASID_CPU),
		SPUV_FUNC_RW_32_ASID_CPU);
	vcd_pr_registers_dump("ASID_DMA_CH0 [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_ASID_DMA_CH0),
		SPUV_FUNC_RW_32_ASID_DMA_CH0);
	vcd_pr_registers_dump("ASID_DMA_CH1 [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_ASID_DMA_CH1),
		SPUV_FUNC_RW_32_ASID_DMA_CH1);
	vcd_pr_registers_dump("ASID_DMA_CH2 [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_ASID_DMA_CH2),
		SPUV_FUNC_RW_32_ASID_DMA_CH2);
	vcd_pr_registers_dump("GADDR_CTRL_P [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GADDR_CTRL_P),
		SPUV_FUNC_RW_32_GADDR_CTRL_P);
	vcd_pr_registers_dump("GADDR_CTRL_X [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GADDR_CTRL_X),
		SPUV_FUNC_RW_32_GADDR_CTRL_X);
	vcd_pr_registers_dump("GADDR_CTRL_Y [%08x][0x%08x].\n",
		ioread32(SPUV_FUNC_RW_32_GADDR_CTRL_Y),
		SPUV_FUNC_RW_32_GADDR_CTRL_Y);

	vcd_pr_end_spuv_function();
	return;
}


/* ========================================================================= */
/* Internal functions                                                        */
/* ========================================================================= */

/**
 * @brief	relocation fw
 *
 * @param[in]	read_fw_info	firmware info
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_PARAM	parameter error.
 */
static int vcd_spuv_func_relocation_fw(
	struct vcd_spuv_func_read_fw_info *read_fw_info)
{
	int ret = VCD_ERR_NONE;
	int i = 0;
	unsigned int fw_size = 0;
	unsigned int start_addr = 0;
	unsigned int next_addr  = 0;

	vcd_pr_start_spuv_function("read_fw_info[%p].\n", read_fw_info);

	if (NULL == read_fw_info) {
		vcd_pr_err("parameter error.\n");
		ret = VCD_ERR_PARAM;
		goto rtn;
	}

	/* get meram area */
	for (i = 0; i < VCD_SPUV_FUNC_PAGE_SIZE; i++) {
		if (0 != read_fw_info->pram_page_size[i])
			fw_size += vcd_spuv_func_ceiling128(
					read_fw_info->pram_page_size[i]);
		if (0 != read_fw_info->xram_page_size[i])
			fw_size += vcd_spuv_func_ceiling128(
					read_fw_info->xram_page_size[i]);
		if (0 != read_fw_info->yram_page_size[i])
			fw_size += vcd_spuv_func_ceiling128(
					read_fw_info->yram_page_size[i]);
	}
	ret = vcd_spuv_func_get_meram(fw_size);
	if (0 != ret)
		vcd_pr_spuv_info("meram not use[%d].\n", ret);

	ret = VCD_ERR_NONE;

	if (0 != SPUV_FUNC_MERAM_FIRMWARE_BUFFER)
		/* firmware cop to meram */
		start_addr = SPUV_FUNC_MERAM_FIRMWARE_BUFFER;
	else
		/* firmware cop to sdram */
		start_addr = SPUV_FUNC_SDRAM_FIRMWARE_BUFFER;



	/* calculation ram base address */
	/* xram */
	vcd_spuv_func_calc_ram(
		start_addr,
		read_fw_info->xram_global_size,
		read_fw_info->xram_page_size,
		VCD_SPUV_FUNC_PAGE_SIZE,
		&(g_spuv_func_xram_info),
		&next_addr);
	start_addr = next_addr;
	/* pram */
	vcd_spuv_func_calc_ram(
		start_addr,
		read_fw_info->pram_global_size,
		read_fw_info->pram_page_size,
		VCD_SPUV_FUNC_PAGE_SIZE,
		&(g_spuv_func_pram_info),
		&next_addr);
	start_addr = next_addr;
	/* yram */
	vcd_spuv_func_calc_ram(
		start_addr,
		read_fw_info->yram_global_size,
		read_fw_info->yram_page_size,
		VCD_SPUV_FUNC_PAGE_SIZE,
		&(g_spuv_func_yram_info),
		&next_addr);

	/* save global size */
	g_spuv_func_pram_info.global_size = read_fw_info->pram_global_size;
	g_spuv_func_xram_info.global_size = read_fw_info->xram_global_size;
	g_spuv_func_yram_info.global_size = read_fw_info->yram_global_size;


	/* set cache to spuv register */
	vcd_spuv_func_reg_firmware();

	/* dsp full reset */
	vcd_spuv_func_dsp_full_reset();

	/* firmware copy to sdram */
	for (i = 0; i < VCD_SPUV_FUNC_PAGE_SIZE; i++) {
		/* pram */
		if (NULL != read_fw_info->pram_addr[i])
			memcpy((void *)(g_spuv_func_pram_info.base_addr[i]),
				read_fw_info->pram_addr[i],
				read_fw_info->pram_page_size[i]);
		/* xram */
		if (NULL != read_fw_info->xram_addr[i])
			memcpy((void *)(g_spuv_func_xram_info.base_addr[i]),
				read_fw_info->xram_addr[i],
				read_fw_info->xram_page_size[i]);
		/* yram */
		if (NULL != read_fw_info->yram_addr[i])
			memcpy((void *)(g_spuv_func_yram_info.base_addr[i]),
				read_fw_info->yram_addr[i],
				read_fw_info->yram_page_size[i]);
	}

rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	get meram function.
 *
 * @param[in]	fw_size			firmware size
 *
 * @retval	VCD_ERR_NONE		successful.
 * @retval	VCD_ERR_NOMEMORY	parameter error.
 */
static int vcd_spuv_func_get_meram(unsigned int fw_size)
{
	int ret = VCD_ERR_NONE;

	vcd_pr_start_spuv_function("fw_size[%d].\n", fw_size);

#ifdef __VCD_MERAM_ENABLE__

	/* initialize meram table */
	memset(&g_spuv_func_meram_alloc,  0, sizeof(g_spuv_func_meram_alloc));
	memset(&g_spuv_func_meram_free,   0, sizeof(g_spuv_func_meram_free));
	memset(&g_spuv_func_meram_delete, 0, sizeof(g_spuv_func_meram_delete));

	/* get handle */
	g_spuv_func_meram_alloc.handle = system_memory_info_new();
	if (NULL == g_spuv_func_meram_alloc.handle) {
		vcd_pr_err("system_memory_info_new error.\n");
		ret = VCD_ERR_NOMEMORY;
		goto rtn;
	}

	/* set handle */
	g_spuv_func_meram_free.handle = g_spuv_func_meram_alloc.handle;
	g_spuv_func_meram_delete.handle = g_spuv_func_meram_alloc.handle;

	/* allocate meram */
	g_spuv_func_meram_alloc.alloc_size = fw_size;
	ret = system_memory_meram_alloc(&g_spuv_func_meram_alloc);
	if (SMAP_LIB_MEMORY_OK != ret) {
		/* allocate meram error */
		vcd_pr_err("system_memory_meram_alloc error[%d].\n", ret);
		system_memory_info_delete(&g_spuv_func_meram_delete);
		goto rtn;
	}

	/* set ch number */
	g_spuv_func_meram_free.ch_num = g_spuv_func_meram_alloc.ch_num;

	/* set addr */
	g_spuv_func_meram_physical_addr =
		SPUV_FUNC_MERAM_TOP_PHY + g_spuv_func_meram_alloc.meram_offset;

	/* ioremap meram */
	ret = vcd_spuv_func_meram_ioremap(g_spuv_func_meram_physical_addr,
					 g_spuv_func_meram_alloc.alloc_size);
	if (VCD_ERR_NONE != ret) {
		vcd_spuv_func_free_meram();
		goto rtn;
	}

#else /* __VCD_MERAM_ENABLE__ */

	ret = VCD_ERR_NOMEMORY;
	goto rtn;
#endif /* __VCD_MERAM_ENABLE__ */
rtn:
	vcd_pr_end_spuv_function("ret[%d].\n", ret);
	return ret;
}


/**
 * @brief	free meram function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_func_free_meram(void)
{
	vcd_pr_start_spuv_function();

#ifdef __VCD_MERAM_ENABLE__

	/* iounmap meram */
	vcd_spuv_func_meram_iounmap();

	/* free meram */
	system_memory_meram_free(&g_spuv_func_meram_free);

	/* delete handle */
	system_memory_info_delete(&g_spuv_func_meram_delete);

	/* global valiable initialize */
	g_spuv_func_meram_logical_addr = 0;
	g_spuv_func_meram_physical_addr = 0;
	memset(&g_spuv_func_meram_alloc,  0, sizeof(g_spuv_func_meram_alloc));
	memset(&g_spuv_func_meram_free,   0, sizeof(g_spuv_func_meram_free));
	memset(&g_spuv_func_meram_delete, 0, sizeof(g_spuv_func_meram_delete));

#endif /* __VCD_MERAM_ENABLE__ */

	vcd_pr_end_spuv_function();
	return;

}


#ifdef __VCD_MERAM_ENABLE__
/**
 * @brief	meram ioremap function.
 *
 * @param	addr		physical address.
 * @param	size		meram size.
 *
 * @retval	VCD_ERR_NONE	successful.
 * @retval	VCD_ERR_SYSTEM	ioremap error.
 */
static int vcd_spuv_func_meram_ioremap(unsigned int addr, unsigned int size)
{
	int ret = VCD_ERR_NONE;
	unsigned int meram_addr = 0;

	vcd_pr_start_spuv_function();

	meram_addr = (unsigned int)ioremap_nocache(addr, size);
	if (0 == meram_addr) {
		vcd_pr_err("error ioremap MERAM addr. addr[0x%x].\n",
				meram_addr);
		ret = -VCD_ERR_SYSTEM;
	}

	g_spuv_func_meram_logical_addr = meram_addr;

	vcd_pr_end_spuv_function();
	return ret;
}


/**
 * @brief	meram iounma function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_func_meram_iounmap(void)
{
	vcd_pr_start_spuv_function();

	if (0 != g_spuv_func_meram_logical_addr) {
		iounmap((void *)g_spuv_func_meram_logical_addr);
		g_spuv_func_meram_logical_addr = 0;
	}

	vcd_pr_end_spuv_function();
	return;

}
#endif /* __VCD_MERAM_ENABLE__ */


/**
 * @brief	calculation address function.
 *
 * @param[in]	start_addr	base address.
 * @param[in]	global_size	global area size.
 * @param[in]	page_size	each size on page 0-15 for ram.
 * @param[in]	page_num	page num.
 * @param[out]	ram_info	dsp ram info.
 * @param[out]	next_addr	next base address.
 *
 * @retval	none.
 */
static void vcd_spuv_func_calc_ram(
		const unsigned int start_addr,
		const unsigned int global_size,
		const unsigned int *page_size,
		const unsigned int page_num,
		struct vcd_spuv_func_fw_info *ram_info,
		unsigned int *next_addr)
{
	int i = 0;

	vcd_pr_start_spuv_function();

	if ((NULL == page_size) || (NULL == ram_info) || (NULL == next_addr)) {
		vcd_pr_err("parameter error.\n");
		goto rtn;
	}

	/* calculation ram base address */
	/* page 0 */
	if (0 != page_size[0]) {
		ram_info->base_addr[0] = start_addr;
		ram_info->reg_addr[0]  = start_addr;
		ram_info->page_size[0] =
			vcd_spuv_func_ceiling128(page_size[0]);

		/* page 1 - page 15 */
		for (i = 1; i < page_num; i++) {
			if (0 != page_size[i]) {
				ram_info->base_addr[i] =
					ram_info->base_addr[i-1] +
					vcd_spuv_func_ceiling128(
						page_size[i-1]);
				ram_info->reg_addr[i] =
					ram_info->base_addr[i] -
					global_size;
				ram_info->page_size[i] =
					vcd_spuv_func_ceiling128(
						page_size[i]);
			} else {
				break;
			}
		}
		*next_addr = ram_info->base_addr[i - 1] +
				vcd_spuv_func_ceiling128(page_size[i-1]);
	}

	/* calculation logical to physcal */
	for (i = 0; i < page_num; i++) {
		if (0 != ram_info->reg_addr[i]) {
			if (0 != SPUV_FUNC_MERAM_FIRMWARE_BUFFER)
				ram_info->reg_addr_physcal[i] =
				vcd_spuv_func_meram_logical_to_physical(
				ram_info->reg_addr[i]);
			else
				ram_info->reg_addr_physcal[i] =
				vcd_spuv_func_sdram_logical_to_physical(
				ram_info->reg_addr[i]);
		}
	}

rtn:
	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	set spuv cache memory function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_func_reg_firmware(void)
{
	int global_size = 0;
	unsigned int spumsts = 0;
	int i = 0;

	vcd_pr_start_spuv_function();

	/* set cache memory */
	/* pram */
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[0],
		SPUV_FUNC_RW_32_P0BASE0);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[1],
		SPUV_FUNC_RW_32_P0BASE1);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[2],
		SPUV_FUNC_RW_32_P0BASE2);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[3],
		SPUV_FUNC_RW_32_P0BASE3);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[4],
		SPUV_FUNC_RW_32_P0BASE4);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[5],
		SPUV_FUNC_RW_32_P0BASE5);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[6],
		SPUV_FUNC_RW_32_P0BASE6);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[7],
		SPUV_FUNC_RW_32_P0BASE7);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[8],
		SPUV_FUNC_RW_32_P0BASE8);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[9],
		SPUV_FUNC_RW_32_P0BASE9);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[10],
		SPUV_FUNC_RW_32_P0BASE10);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[11],
		SPUV_FUNC_RW_32_P0BASE11);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[12],
		SPUV_FUNC_RW_32_P0BASE12);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[13],
		SPUV_FUNC_RW_32_P0BASE13);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[14],
		SPUV_FUNC_RW_32_P0BASE14);
	vcd_spuv_func_set_register(
		g_spuv_func_pram_info.reg_addr_physcal[15],
		SPUV_FUNC_RW_32_P0BASE15);
	/* xram */
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[0],
		SPUV_FUNC_RW_32_X0BASE0);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[1],
		SPUV_FUNC_RW_32_X0BASE1);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[2],
		SPUV_FUNC_RW_32_X0BASE2);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[3],
		SPUV_FUNC_RW_32_X0BASE3);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[4],
		SPUV_FUNC_RW_32_X0BASE4);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[5],
		SPUV_FUNC_RW_32_X0BASE5);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[6],
		SPUV_FUNC_RW_32_X0BASE6);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[7],
		SPUV_FUNC_RW_32_X0BASE7);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[8],
		SPUV_FUNC_RW_32_X0BASE8);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[9],
		SPUV_FUNC_RW_32_X0BASE9);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[10],
		SPUV_FUNC_RW_32_X0BASE10);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[11],
		SPUV_FUNC_RW_32_X0BASE11);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[12],
		SPUV_FUNC_RW_32_X0BASE12);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[13],
		SPUV_FUNC_RW_32_X0BASE13);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[14],
		SPUV_FUNC_RW_32_X0BASE14);
	vcd_spuv_func_set_register(
		g_spuv_func_xram_info.reg_addr_physcal[15],
		SPUV_FUNC_RW_32_X0BASE15);
	/* yram */
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[0],
		SPUV_FUNC_RW_32_Y0BASE0);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[1],
		SPUV_FUNC_RW_32_Y0BASE1);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[2],
		SPUV_FUNC_RW_32_Y0BASE2);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[3],
		SPUV_FUNC_RW_32_Y0BASE3);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[4],
		SPUV_FUNC_RW_32_Y0BASE4);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[5],
		SPUV_FUNC_RW_32_Y0BASE5);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[6],
		SPUV_FUNC_RW_32_Y0BASE6);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[7],
		SPUV_FUNC_RW_32_Y0BASE7);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[8],
		SPUV_FUNC_RW_32_Y0BASE8);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[9],
		SPUV_FUNC_RW_32_Y0BASE9);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[10],
		SPUV_FUNC_RW_32_Y0BASE10);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[11],
		SPUV_FUNC_RW_32_Y0BASE11);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[12],
		SPUV_FUNC_RW_32_Y0BASE12);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[13],
		SPUV_FUNC_RW_32_Y0BASE13);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[14],
		SPUV_FUNC_RW_32_Y0BASE14);
	vcd_spuv_func_set_register(
		g_spuv_func_yram_info.reg_addr_physcal[15],
		SPUV_FUNC_RW_32_Y0BASE15);

	/* set cache memory control setting */
	vcd_spuv_func_set_register(
		(VCD_SPUV_FUNC_CCTL_LWSWAP_LW |
		VCD_SPUV_FUNC_CCTL_FORM_UPPER |
		VCD_SPUV_FUNC_CCTL_CACHE_ON),
		SPUV_FUNC_RW_32_CCTL);
	udelay(1);
	vcd_spuv_func_set_register(
		(VCD_SPUV_FUNC_CCTL_LWSWAP_LW |
		VCD_SPUV_FUNC_CCTL_FORM_UPPER |
		VCD_SPUV_FUNC_CCTL_CACHE_OFF),
		SPUV_FUNC_RW_32_CCTL);

	i = VCD_SPUV_FUNC_SPUMSTS_WAIT_MAX;
	while (i--) {
		vcd_spuv_func_register(SPUV_FUNC_RO_32_SPUMSTS, spumsts);
		if (VCD_SPUV_FUNC_SPUMSTS_IDL_NOP == spumsts)
			break;
		udelay(1);
	}

	/* set global size */
	/* pram */
	global_size = vcd_spuv_func_conv_global_size(
				g_spuv_func_pram_info.global_size);
	vcd_spuv_func_set_register(
				global_size, SPUV_FUNC_RW_32_GADDR_CTRL_P);
	/* xram */
	global_size = vcd_spuv_func_conv_global_size(
				g_spuv_func_xram_info.global_size);
	vcd_spuv_func_set_register(
				global_size, SPUV_FUNC_RW_32_GADDR_CTRL_X);
	/* yram */
	global_size = vcd_spuv_func_conv_global_size(
				g_spuv_func_yram_info.global_size);
	vcd_spuv_func_set_register(global_size, SPUV_FUNC_RW_32_GADDR_CTRL_Y);

	vcd_pr_end_spuv_function();
	return;
}


/**
 * @brief	convert gaddr ctrl register global size function.
 *
 * @param[in]	global_size	global size for software.
 *
 * @retval	size	global size for hardware.
 */
static int vcd_spuv_func_conv_global_size(const unsigned int global_size)
{
	int size = 0;

	vcd_pr_start_spuv_function("global_size[%d].\n", global_size);

	switch (global_size) {
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_1KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_1KW;
		break;
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_2KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_2KW;
		break;
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_4KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_4KW;
		break;
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_8KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_8KW;
		break;
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_16KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_16KW;
		break;
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_32KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_32KW;
		break;
	case VCD_SPUV_FUNC_GLOBAL_AREA_SIZE_64KW:
		size = VCD_SPUV_FUNC_GADDR_CTRL_64KW;
		break;
	default:
		vcd_pr_err("Unknown global size. global_size[%d].\n",
			global_size);
		size = VCD_SPUV_FUNC_GADDR_CTRL_64KW;
		break;
	}

	vcd_pr_end_spuv_function("size[%d].\n", size);
	return size;
}


/**
 * @brief	dsp core full reset function.
 *
 * @param	none.
 *
 * @retval	none.
 */
static void vcd_spuv_func_dsp_full_reset(void)
{
	vcd_pr_start_spuv_function();

	/* dsp reset register reset cancellation setting */
	vcd_spuv_func_set_register(VCD_SPUV_FUNC_DSPRST_ACTIVE,
		SPUV_FUNC_RW_32_DSPRST);

	vcd_pr_end_spuv_function();
	return;
}
