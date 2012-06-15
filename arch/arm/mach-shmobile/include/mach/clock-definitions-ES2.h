#define VCLKCR5         IO_ADDRESS(0xE6150034)
#define SLIMBCKCR	IO_ADDRESS(0xE6150088)
#define M4CKCR          IO_ADDRESS(0xE6150098)
#define DSI1PCKCR	IO_ADDRESS(0xE6150068)
#define DSI1PHYCR	IO_ADDRESS(0xE6150070)
#define PLL22CR		IO_ADDRESS(0xE61501F4)

static struct clk es2_fsiack_clk = { };

static struct clk es2_fsibck_clk = { };

/* PLL22 */
static unsigned long pll22_recalc(struct clk *clk)
{
	unsigned long rate = clk->parent->rate;
	u32 pll22cr = __raw_readl(clk->enable_reg);
	if (__raw_readl(PLLECR) & (1 << (8 + clk->enable_bit))) {
		if (clk->enable_bit == 4)
			rate *= ((pll22cr >> 24) & 0x7f) + 1;
		if (pll22cr & (1 << 20))
			rate *= 2;
	}
	return rate;
}

static struct clk_ops pll22_clk_ops = {
	.recalc = pll22_recalc,
};

/* PLL22 */
/* This shares its enable bit with PLL2 */
/* This has same set of parent as PLL2 but can be controlled independently */

static struct clk pll22_cksel_clk = SH_CLK_CKSEL(NULL, PLL22CR, 0, 0,
						 pll2_parent,
						 ARRAY_SIZE(pll2_parent), 5, 3);

static struct clk pll22_clk = {
	.parent = &pll22_cksel_clk,
	.ops = &pll22_clk_ops,
	.enable_reg = (void *__iomem)PLL22CR,
	.enable_bit = 4,	/* bit in PLLECR */
};

enum {
	ES2_DIV4_I, ES2_DIV4_ZG, ES2_DIV4_M3, ES2_DIV4_B, ES2_DIV4_M1,
	    ES2_DIV4_M5,
	ES2_DIV4_Z, ES2_DIV4_ZTR, ES2_DIV4_ZT, ES2_DIV4_ZX, ES2_DIV4_ZS,
	    ES2_DIV4_HP,
	ES2_DIV4_DDR, ES2_DIV4_NR
};

static struct clk es2_div4_clks[ES2_DIV4_NR] = {
	[ES2_DIV4_I] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 20, 0xdff, 0),
	[ES2_DIV4_ZG] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 16, 0x97f, 0),
	[ES2_DIV4_M3] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 12, 0x1dff, 0),
	[ES2_DIV4_M5] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 0, 0x1dff, 0),
	[ES2_DIV4_B] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 8, 0xdff, 0),
	[ES2_DIV4_M1] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 4, 0x1dff, 0),
	[ES2_DIV4_Z] = SH_CLK_DIV4(NULL, FRQCRB, 24, 0x097f, 0),
	[ES2_DIV4_ZTR] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 20, 0x0dff, 0),
	[ES2_DIV4_ZT] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 16, 0xdff, 0),
	[ES2_DIV4_ZX] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 12, 0xdff, 0),
	[ES2_DIV4_ZS] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 8, 0xdff, 0),
	[ES2_DIV4_HP] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 4, 0xdff, 0),
	[ES2_DIV4_DDR] = SH_CLK_DIV4(&pll3_clk, FRQCRD, 0, 0x97f, 0),
};

static struct clk es2_ztr_clk = {
	.parent = &es2_div4_clks[ES2_DIV4_ZTR],
	.ops = &const_div_clk_ops,
	.priv = (void *)1,
};

static struct clk *es2_ddr_parent[] = {
	[0] = &pll3_clk,
	[1] = &es2_div4_clks[ES2_DIV4_DDR],
};

static struct clk es2_ddr_clk = SH_CLK_CKSEL(NULL, FRQCRD, 0, 0,
					     es2_ddr_parent,
					     ARRAY_SIZE(es2_ddr_parent), 4, 1);

static struct clk es2_zb30_clk = {	/* ZB3 Implementation not needed, ZB30=ZB3 */
	.parent = &es2_ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk es2_ddr_div2_clk = {
	.parent = &es2_ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk es2_ddr_div4_clk = {
	.parent = &es2_ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)4,
};

static struct clk *es2_zb30sl_parent[] = {
	[0] = &es2_ddr_div2_clk,
	[1] = &es2_ddr_div4_clk,
};

static struct clk es2_zb30sl_clk = SH_CLK_CKSEL(NULL, FRQCRD, 0, 0,
						es2_zb30sl_parent,
						ARRAY_SIZE(es2_zb30sl_parent),
						15, 1);

/* SLIMB Clock */

static struct clk *es2_slimb_parent[] = {
	[0] = &pll1_div2_clk,
	[1] = &pll22_clk,
	[2] = &pll2_clk,
};

static struct clk es2_slimb_clk = SH_CLK_CKSEL(NULL, SLIMBCKCR, 8, 0,
					       es2_slimb_parent,
					       ARRAY_SIZE(es2_slimb_parent), 7,
					       1);

enum {
	ES2_DIV6_MP,
	ES2_DIV6_ZB,
	ES2_DIV6_SD0,
	ES2_DIV6_SD1,
	ES2_DIV6_VCK1,
	ES2_DIV6_VCK2,
	ES2_DIV6_VCK3,
	ES2_DIV6_VCK4,
	ES2_DIV6_FSIA,
	ES2_DIV6_FSIB,
	ES2_DIV6_SPUA,
	ES2_DIV6_HSI,
	ES2_DIV6_DSIT,
	ES2_DIV6_DSI0P,
	ES2_DIV6_SD2,
	ES2_DIV6_VCK5,
/* VCD add start */
	ES2_DIV6_SPUV,
/* VCD add end */
	ES2_DIV6_M4,
	ES2_DIV6_DSI1P,
	ES2_DIV6_SLIMB,
	ES2_DIV6_NR
};

static struct clk es2_div6_clks[ES2_DIV6_NR] = {
	[ES2_DIV6_MP] = SH_CLK_DIV6_EXT(NULL, MPCKCR, 9, CLK_DIV_SHARED,
					div6_two_parent,
					ARRAY_SIZE(div6_two_parent), 6, 1),
	[ES2_DIV6_ZB] = SH_CLK_DIV6_EXT(NULL, ZBCKCR, 8, 0,
					div6_two_parent,
					ARRAY_SIZE(div6_two_parent), 7, 1),
	[ES2_DIV6_SD0] = SH_CLK_DIV6_EXT(NULL, SD0CKCR, 8, 0,
					 sd_parent, ARRAY_SIZE(sd_parent), 6,
					 2),
	[ES2_DIV6_SD1] = SH_CLK_DIV6_EXT(NULL, SD1CKCR, 8, 0,
					 sd_parent, ARRAY_SIZE(sd_parent), 6,
					 2),
	[ES2_DIV6_SD2] = SH_CLK_DIV6_EXT(NULL, SD2CKCR, 8, 0,
					 sd_parent, ARRAY_SIZE(sd_parent), 6,
					 2),
	[ES2_DIV6_VCK1] = SH_CLK_DIV6_EXT(NULL, VCLKCR1, 8, 0,
					  vclk_parent, ARRAY_SIZE(vclk_parent),
					  12, 3),
	[ES2_DIV6_VCK2] = SH_CLK_DIV6_EXT(NULL, VCLKCR2, 8, 0,
					  vclk_parent, ARRAY_SIZE(vclk_parent),
					  12, 3),
	[ES2_DIV6_VCK3] = SH_CLK_DIV6_EXT(&vclk3_pdiv_clk, VCLKCR3, 8, 0,
					  NULL, 0, 0, 0),
	[ES2_DIV6_VCK4] = SH_CLK_DIV6_EXT(NULL, VCLKCR4, 8, 0,
					  vclk_parent, ARRAY_SIZE(vclk_parent),
					  12, 3),
	[ES2_DIV6_VCK5] = SH_CLK_DIV6_EXT(NULL, VCLKCR5, 8, 0,
					  vclk_parent, ARRAY_SIZE(vclk_parent),
					  12, 3),
	[ES2_DIV6_FSIA] = SH_CLK_DIV6_EXT(NULL, FSIACKCR, 8, 0,
					  div6_two_parent,
					  ARRAY_SIZE(div6_two_parent), 6, 1),
	[ES2_DIV6_FSIB] = SH_CLK_DIV6_EXT(NULL, FSIBCKCR, 8, 0,
					  div6_two_parent,
					  ARRAY_SIZE(div6_two_parent), 6, 1),
	[ES2_DIV6_SPUA] = SH_CLK_DIV6_EXT(NULL, SPUACKCR, 8, 0,
					  div6_two_parent,
					  ARRAY_SIZE(div6_two_parent), 6, 1),
/* VCD add start */
	[ES2_DIV6_SPUV] = SH_CLK_DIV6_EXT(NULL, SPUVCKCR, 8, 0,
					  div6_two_parent,
					  ARRAY_SIZE(div6_two_parent), 6, 1),
/* VCD add end */
	[ES2_DIV6_M4] = SH_CLK_DIV6_EXT(&pll22_clk, M4CKCR, 8, 0,
					NULL, 0, 0, 0),
	[ES2_DIV6_HSI] = SH_CLK_DIV6_EXT(NULL, HSICKCR, 8, 0,
					 hsi_parent, ARRAY_SIZE(hsi_parent), 6,
					 2),
	[ES2_DIV6_DSIT] = SH_CLK_DIV6_EXT(NULL, DSITCKCR, 8, 0,
					  div6_two_parent,
					  ARRAY_SIZE(hsi_parent), 7, 1),
	[ES2_DIV6_DSI0P] = SH_CLK_DIV6(&pll1_div2_clk, DSI0PCKCR, 0),
	[ES2_DIV6_DSI1P] = SH_CLK_DIV6(&pll2_clk, DSI1PCKCR, 0),
	[ES2_DIV6_SLIMB] = SH_CLK_DIV6_EXT(NULL, SLIMBCKCR, 8, 0,
					   es2_slimb_parent,
					   ARRAY_SIZE(es2_slimb_parent), 7, 1),
};

static unsigned long m4d2_recalc(struct clk *clk)
{
	unsigned long rate = clk->parent->rate;
	u32 m4d2cr = __raw_readl(clk->enable_reg);
	if (m4d2cr & (1 << 7))
		rate /= 2;
	return rate;
}

static struct clk_ops m4d2_clk_ops = {
	.recalc = m4d2_recalc,
};

static struct clk es2_m4d2_clk = {
	.parent = &es2_div6_clks[ES2_DIV6_M4],
	.ops = &m4d2_clk_ops,
	.enable_reg = (void *__iomem)M4CKCR,
};

static struct clk *es2_mp_parent[] = {
	[0] = &es2_div6_clks[ES2_DIV6_MP],
	[1] = &extal2_clk,
};

static struct clk es2_mp_clk = SH_CLK_CKSEL(NULL, MPCKCR, 9, CLK_CKSEL_CKSTP,
					    es2_mp_parent,
					    ARRAY_SIZE(es2_mp_parent), 7, 1);

static struct clk es2_mpc_clk = SH_CLK_CKSEL(NULL, MPCKCR, 10, CLK_CKSEL_CKSTP,
					     es2_mp_parent,
					     ARRAY_SIZE(es2_mp_parent), 7, 1);
/* sound add */
static struct clk es2_mpmp_clk = SH_CLK_CKSEL(NULL, MPCKCR, 11, CLK_CKSEL_CKSTP,
					      es2_mp_parent,
					      ARRAY_SIZE(es2_mp_parent), 7, 1);
/* sound add */

static struct clk *es2_fsia_parent[] = {
	[0] = &es2_div6_clks[ES2_DIV6_FSIA],
	[1] = &es2_fsiack_clk,
};

static struct clk es2_fsia_clk = SH_CLK_CKSEL(NULL, FSIACKCR, 0, 0,
					      es2_fsia_parent,
					      ARRAY_SIZE(es2_fsia_parent), 7,
					      1);

static struct clk *es2_fsib_parent[] = {
	[0] = &es2_div6_clks[ES2_DIV6_FSIB],
	[1] = &es2_fsibck_clk,
};

static struct clk es2_fsib_clk = SH_CLK_CKSEL(NULL, FSIBCKCR, 0, 0,
					      es2_fsib_parent,
					      ARRAY_SIZE(es2_fsib_parent), 7,
					      1);

static struct clk *es2_spua_parent[] = {
	[0] = &es2_div6_clks[ES2_DIV6_SPUA],
	[1] = &extal2_clk,
};

static struct clk es2_spua_clk = SH_CLK_CKSEL(NULL, SPUACKCR, 0, 0,
					      es2_spua_parent,
					      ARRAY_SIZE(es2_spua_parent), 7,
					      1);

/* VCD add start */
static struct clk *es2_spuv_parent[] = {
	[0] = &es2_div6_clks[ES2_DIV6_SPUV],
	[1] = &extal2_clk,
};

static struct clk es2_spuv_clk = SH_CLK_CKSEL(NULL, SPUVCKCR, 0, 0,
					      es2_spuv_parent,
					      ARRAY_SIZE(es2_spuv_parent), 7,
					      1);

/* VCD add end */

/* DSI1P
 * HW clock topology is like
 *  {parents}[SELMON0? EXSRC : EXSRCB] / (SELMON0? DIV : DIVB)
 * But here implemented as
 *  SELMON0? ({parents}[EXSRC] / DIV) : ({parents}[EXSRCB] / DIVB)
 */

static struct clk *dsi1p_parent[] = {
	[0] = &pll2_clk,
	[1] = &pll1_div2_clk,
	[2] = &main_clk,
	[3] = &extal2_clk,
	[4] = &extcki_clk,
};

static struct clk dsi1p0_cksel_clk = SH_CLK_CKSEL(NULL, DSI1PCKCR,
						  8, CLK_CKSEL_CKSTP,
						  dsi1p_parent,
						  ARRAY_SIZE(dsi1p_parent), 24,
						  3);

static struct clk dsi1p0_clk = {
	.parent = &dsi1p0_cksel_clk,
	.ops = &div_clk_ops,
	.enable_reg = (void *__iomem)DSI1PCKCR,
	.src_shift = 16,
	.src_width = 6,
};

static struct clk dsi1p1_cksel_clk = SH_CLK_CKSEL(NULL, DSI1PCKCR,
						  8, CLK_CKSEL_CKSTP,
						  dsi1p_parent,
						  ARRAY_SIZE(dsi1p_parent), 12,
						  3);

static struct clk dsi1p1_clk = {
	.parent = &dsi1p1_cksel_clk,
	.ops = &div_clk_ops,
	.enable_reg = (void *__iomem)DSI1PCKCR,
	.src_shift = 0,
	.src_width = 6,
};

static struct clk *selmon1_parent[2] = {
	[0] = &dsi1p0_clk,
	[1] = &dsi1p1_clk,
};

static struct clk dsi1p_clk = {
	.ops = &selmon_clk_ops,
	.enable_reg = (void *__iomem)DSI1PCKCR,
	.enable_bit = 8,
	.parent_table = selmon1_parent,
	.src_shift = 27,
	.src_width = 1,
};

enum {
	ES2_MSTP031,
	ES2_MSTP030,
	ES2_MSTP029,
	ES2_MSTP026,
	ES2_MSTP022,
	ES2_MSTP021,
	ES2_MSTP019,
	ES2_MSTP018,
	ES2_MSTP017,
	ES2_MSTP016,
	ES2_MSTP015,
	ES2_MSTP007,
	/*ES2_MSTP002,*/
	ES2_MSTP001,
	ES2_MSTP000,
	ES2_MSTP130,
/* EOS-CSI ADD-S */
	ES2_MSTP128,
/* EOS-CSI ADD-E */
	ES2_MSTP126,
	ES2_MSTP125,
	ES2_MSTP124,
	ES2_MSTP122,
	ES2_MSTP119,
	ES2_MSTP118,
	ES2_MSTP116,
	ES2_MSTP115,
	ES2_MSTP113,
	ES2_MSTP112,
	ES2_MSTP111,
	ES2_MSTP108,
	ES2_MSTP107,
	ES2_MSTP106,
/* EOS-RCU ADD-S */
	ES2_MSTP105,
/* EOS-RCU ADD-E */
	ES2_MSTP101,
	ES2_MSTP100,
	ES2_MSTP229,
	ES2_MSTP228,
	ES2_MSTP224,
	ES2_MSTP223,
	ES2_MSTP220,
	ES2_MSTP218,
	ES2_MSTP217,
	ES2_MSTP216,
	ES2_MSTP215,
	ES2_MSTP214,
	ES2_MSTP213,
	ES2_MSTP208,
	ES2_MSTP207,
	ES2_MSTP206,
	ES2_MSTP205,
	ES2_MSTP204,
	ES2_MSTP203,
	ES2_MSTP202,
	ES2_MSTP201,
	ES2_MSTP330,
	ES2_MSTP329,
	ES2_MSTP328,
	ES2_MSTP326,
	ES2_MSTP325,
	ES2_MSTP324,
	ES2_MSTP323,
	ES2_MSTP322,
	ES2_MSTP321,
	ES2_MSTP319,
	ES2_MSTP316,
	ES2_MSTP315,
	ES2_MSTP314,
	ES2_MSTP313,
	ES2_MSTP312,
	ES2_MSTP309,
	/*ES2_MSTP308,*/
	ES2_MSTP307,
	ES2_MSTP306,
	ES2_MSTP305,
	ES2_MSTP304,
	ES2_MSTP427,
	ES2_MSTP426,
	ES2_MSTP425,
	ES2_MSTP424,
	ES2_MSTP411,
	/*ES2_MSTP415,*/
	ES2_MSTP412,
	/*ES2_MSTP410,*/
	/*ES2_MSTP409,*/
	ES2_MSTP407,
	ES2_MSTP406,
	ES2_MSTP403,
	ES2_MSTP402,
	ES2_MSTP530,
	ES2_MSTP529,
	ES2_MSTP528,
	ES2_MSTP527,
	ES2_MSTP526,
	ES2_MSTP525,
	ES2_MSTP523,
	ES2_MSTP522,
	ES2_MSTP519,
	ES2_MSTP518,
	ES2_MSTP517,
	ES2_MSTP516,
	ES2_MSTP508,
	ES2_MSTP507,
	ES2_MSTP501,
	ES2_MSTP500,
	ES2_MSTP_NR
};

static struct clk es2_mstp_clks[ES2_MSTP_NR] = {
	[ES2_MSTP031] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 31, 0),	/* RT-CPU TLB */
	[ES2_MSTP030] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 30, 0),	/* RT-CPU Instruction Cache (IC) */
	[ES2_MSTP029] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 29, 0),	/* RT-CPU Operand Cache (OC) */
	[ES2_MSTP026] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 26, 0),	/* RT-CPU X/Y memory */
	[ES2_MSTP022] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR0, MSTPSR0, 22, 0),	/* INTCS */
	[ES2_MSTP021] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_ZS], SMSTPCR0, MSTPSR0, 21, 0),	/* RT-DMAC */
	[ES2_MSTP019] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR0, MSTPSR0, 19, 0),	/* H-UDI */
	[ES2_MSTP018] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR0, MSTPSR0, 18, 0),	/* RT-CPU debug module 1 */
	[ES2_MSTP017] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 17, 0),	/* UBC */
	[ES2_MSTP016] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR0, MSTPSR0, 16, 0),	/* RT-CPU debug module 2 */
	[ES2_MSTP015] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 15, 0),	/* ILRAM */
	[ES2_MSTP007] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 7, 0),	/* ICB */
	/*[ES2_MSTP002] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 2, 0),*/ /* test module. */
	[ES2_MSTP001] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR0, MSTPSR0, 1, 0),	/* IIC2 */
	[ES2_MSTP000] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR0, MSTPSR0, 0, 0),	/* MSIOF0 */
	[ES2_MSTP130] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 30, 0),	/* VIO60 */
/* EOS-CSI ADD-S */
	[ES2_MSTP128] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_ZS], SMSTPCR1, MSTPSR1, 28, 0),	/* CSI2-RX1 */
/* EOS-CSI ADD-E */
	[ES2_MSTP126] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_ZS], SMSTPCR1, MSTPSR1, 26, 0),	/* CSI2-RX0 */
	[ES2_MSTP125] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR1, MSTPSR1, 25, 0),	/* TMU0 */
	[ES2_MSTP124] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_B], SMSTPCR1, MSTPSR1, 24, 0),	/* CMT0 */
	[ES2_MSTP122] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 22, 0),	/* TSG */
	[ES2_MSTP119] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 19, 0),	/* ISP */
	[ES2_MSTP118] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_M3], SMSTPCR1, MSTPSR1, 18, 0),	/* DSI-TX0 */
	[ES2_MSTP116] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR1, MSTPSR1, 16, 0),	/* IIC0 */
	[ES2_MSTP115] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_B], SMSTPCR1, MSTPSR1, 15, 0),	/* 2D-DMAC */
	[ES2_MSTP113] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_ZS], SMSTPCR1, MSTPSR1, 13, 0),	/* MERAM */
	[ES2_MSTP112] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_ZG], SMSTPCR1, MSTPSR1, 12, 0),	/* SGX543 */
	[ES2_MSTP111] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR1, MSTPSR1, 11, 0),	/* TMU1 */
	[ES2_MSTP108] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 8, 0),	/* TSIF0 */
	[ES2_MSTP107] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 7, 0),	/* RCUA */
	[ES2_MSTP106] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_B], SMSTPCR1, MSTPSR1, 6, 0),	/* JPU */
/* EOS-RCU ADD-S */
	[ES2_MSTP105] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 5, 0),	/* RCUA1 */
/* EOS-RCU ADD-E */
	[ES2_MSTP101] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_M1], SMSTPCR1, MSTPSR1, 1, 0),	/* VPU */
	[ES2_MSTP100] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_M3], SMSTPCR1, MSTPSR1, 0, 0),	/* LCDC0 */
	[ES2_MSTP229] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR2, MSTPSR2, 29, 0),	/* Crypt1 */
	[ES2_MSTP228] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR2, MSTPSR2, 28, 0),	/* Crypt */
/* sound edit */
/*      [ES2_MSTP224] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 24, 0),*//* CLKGEN */
	[ES2_MSTP224] = SH_CLK_MSTP32_EXT(&es2_mpmp_clk, SMSTPCR2, MSTPSR2, 24, 0),	/* CLKGEN */
/* sound edit */
	[ES2_MSTP223] = SH_CLK_MSTP32_EXT(&es2_spua_clk, SMSTPCR2, MSTPSR2, 23, 0),	/* SPU2A */
/* VCD mod start */
/*	[MSTP220] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 20, 0),*/ /* SPU2V */
	[ES2_MSTP220] = SH_CLK_MSTP32_EXT(&es2_spuv_clk, SMSTPCR2, MSTPSR2, 20, 0),	/* SPU2V */
/* VCD mod end */
	[ES2_MSTP218] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR2, MSTPSR2, 18, 0),	/* SY-DMAC */
	[ES2_MSTP217] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 17, 0),	/* SCIFB3 */
	[ES2_MSTP216] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 16, 0),	/* SCIFB2 */
	[ES2_MSTP215] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 15, 0),	/* MSIOF3 */
	[ES2_MSTP214] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 14, 0),	/* USB-DMAC */
	[ES2_MSTP213] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR2, MSTPSR2, 13, 0),	/* MFIS */
	[ES2_MSTP208] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 8, 0),	/* MSIOF1 */
	[ES2_MSTP207] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 7, 0),	/* SCIFB1 */
	[ES2_MSTP206] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 6, 0),	/* SCIFB0 */
	[ES2_MSTP205] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 5, 0),	/* MSIOF2 */
	[ES2_MSTP204] = SH_CLK_MSTP32_EXT(&es2_mpc_clk, SMSTPCR2, MSTPSR2, 4, 0),	/* SCIFA0 */
	[ES2_MSTP203] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 3, 0),	/* SCIFA1 */
	[ES2_MSTP202] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 2, 0),	/* SCIFA2 */
	[ES2_MSTP201] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR2, MSTPSR2, 1, 0),	/* SCIFA3 */
	[ES2_MSTP330] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 30, 0),	/* MSU */
	[ES2_MSTP329] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR3, MSTPSR3, 29, 0),	/* CMT1 */
/* sound edit */
	/*	[ES2_MSTP328] = SH_CLK_MSTP32_EXT(&es2_fsia_clk, SMSTPCR3, MSTPSR3, 28, 0),*//* FSI */
	[ES2_MSTP328] = SH_CLK_MSTP32_EXT(&es2_mpmp_clk, SMSTPCR3, MSTPSR3, 28, 0),	/* FSI */
	/*	[ES2_MSTP326] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR3, MSTPSR3, 26, 0),*//* SCUW */
	[ES2_MSTP326] = SH_CLK_MSTP32_EXT(&es2_mpmp_clk, SMSTPCR3, MSTPSR3, 26, 0),	/* SCUW */
/* sound edit */
	[ES2_MSTP325] = SH_CLK_MSTP32_EXT(&es2_div6_clks[ES2_DIV6_HSI], SMSTPCR3, MSTPSR3, 25, 0),	/* HSI1 */
	[ES2_MSTP324] = SH_CLK_MSTP32_EXT(&es2_div6_clks[ES2_DIV6_HSI], SMSTPCR3, MSTPSR3, 24, 0),	/* HSI0 */
	[ES2_MSTP323] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR3, MSTPSR3, 23, 0),	/* IIC1 */
	[ES2_MSTP322] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR3, MSTPSR3, 22, 0),	/* USB */
	[ES2_MSTP321] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 21, 0),	/* SBSC performance monitor */
	[ES2_MSTP319] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 19, 0),	/* RT-DMAC scheduler */
	[ES2_MSTP316] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_ZS], SMSTPCR3, MSTPSR3, 16, 0),	/* SHWYSTAT */
	[ES2_MSTP315] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR3, MSTPSR3, 15, 0),	/* MMCIF0 */
	[ES2_MSTP314] = SH_CLK_MSTP32_EXT(&es2_div6_clks[ES2_DIV6_SD0], SMSTPCR3, MSTPSR3, 14, 0),	/* SDHI0 */
	[ES2_MSTP313] = SH_CLK_MSTP32_EXT(&es2_div6_clks[ES2_DIV6_SD1], SMSTPCR3, MSTPSR3, 13, 0),	/* SDHI1 */
	[ES2_MSTP312] = SH_CLK_MSTP32_EXT(&es2_div6_clks[ES2_DIV6_SD2], SMSTPCR3, MSTPSR3, 12, 0),	/* SDHI2 */
	[ES2_MSTP309] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR3, MSTPSR3, 9, 0),	/* ICU-USB */
	/*[ES2_MSTP308] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 8, 0), *//* MFIR */
	[ES2_MSTP307] = SH_CLK_MSTP32_EXT(&es2_mpc_clk, SMSTPCR3, MSTPSR3, 7, 0),	/* ICU-DMAC1 */
	[ES2_MSTP306] = SH_CLK_MSTP32_EXT(&es2_mpc_clk, SMSTPCR3, MSTPSR3, 6, 0),	/* ICU-DMAC0 */
	[ES2_MSTP305] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR3, MSTPSR3, 5, 0),	/* MMC1 */
	[ES2_MSTP304] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_M1], SMSTPCR3, MSTPSR3, 4, 0),	/* TPU0 */
	[ES2_MSTP427] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 27, 0),	/* IIC3H */
	[ES2_MSTP426] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 26, 0),	/* IIC2H */
	[ES2_MSTP425] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 25, 0),	/* IIC1H */
	[ES2_MSTP424] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 24, 0),	/* IIC0H */
	[ES2_MSTP411] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 11, 0),	/* IIC3 */
	/*[ES2_MSTP415] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR4, MSTPSR4, 15, 0),*/ /* GPRMP */
	[ES2_MSTP412] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR4, MSTPSR4, 12, 0),	/* IICMS */
	/*[ES2_MSTP410] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 10, 0),*/ /* IIC4 */
	/*[ES2_MSTP409] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR4, MSTPSR4, 9, 0),*/ /* IIC5 */
	[ES2_MSTP407] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR4, MSTPSR4, 7, 0),	/* USB-DMAC1 */
	[ES2_MSTP406] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR4, MSTPSR4, 6, 0),	/* DDM */
	[ES2_MSTP403] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR4, MSTPSR4, 3, 0),	/* KEYSC */
	[ES2_MSTP402] = SH_CLK_MSTP32_EXT(&extalr_clk, SMSTPCR4, MSTPSR4, 2, 0),	/* RWDT0 */
	[ES2_MSTP530] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 30, 0),	/* Secure ROM */
	[ES2_MSTP529] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 29, 0),	/* Secure RAM */
	[ES2_MSTP528] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 28, 0),	/* Internal RAM1 */
	[ES2_MSTP527] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 27, 0),	/* Internal RAM0 */
	[ES2_MSTP526] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 26, 0),	/* Public ROM */
	[ES2_MSTP525] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 25, 0),	/* IICB0 */
	[ES2_MSTP523] = SH_CLK_MSTP32_EXT(&es2_mp_clk, SMSTPCR5, MSTPSR5, 23, 0), /* PCM2PWM */
	[ES2_MSTP522] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR5, MSTPSR5, 22, 0),	/* Thermal Sensor */
	[ES2_MSTP519] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 19, 0),	/* O2S */
	[ES2_MSTP518] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 18, 0),	/* O2M */
	[ES2_MSTP517] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 17, 0),	/* S2O1 */
	[ES2_MSTP516] = SH_CLK_MSTP32_EXT(&es2_div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 16, 0),	/* S2O0 */
	[ES2_MSTP508] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 8, 0),	/* INTCM */
	[ES2_MSTP507] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 7, 0),	/* IRQC */
	[ES2_MSTP501] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 1, 0),	/* SPU2A Core1 */
	[ES2_MSTP500] = SH_CLK_MSTP32_EXT(&es2_div4_clks[ES2_DIV4_HP], SMSTPCR5, MSTPSR5, 0, 0),	/* SPU2A Core0 */
};

static struct clk_lookup es2_lookups[] = {
	/* reparent candidates for cksel */
	CLKDEV_CON_ID("extal1_clk", &extal1_clk),
	CLKDEV_CON_ID("extal1_div2_clk", &extal1_div2_clk),
	CLKDEV_CON_ID("extal2_clk", &extal2_clk),
	CLKDEV_CON_ID("extal2_div2_clk", &extal2_div2_clk),
	CLKDEV_CON_ID("extal2_div4_clk", &extal2_div4_clk),
	CLKDEV_CON_ID("extalr_clk", &extalr_clk),
	CLKDEV_CON_ID("extcki_clk", &extcki_clk),
	CLKDEV_CON_ID("main_clk", &main_clk),
	CLKDEV_CON_ID("main_div2_clk", &main_div2_clk),
	CLKDEV_CON_ID("pll1_div2_clk", &pll1_div2_clk),
	CLKDEV_CON_ID("pll1_div7_clk", &pll1_div7_clk),
	CLKDEV_CON_ID("pll1_div13_clk", &pll1_div13_clk),
	CLKDEV_CON_ID("pll2_clk", &pll2_clk),
	CLKDEV_CON_ID("pll22_clk", &pll22_clk),
	CLKDEV_CON_ID("pll3_clk", &pll3_clk),
	CLKDEV_CON_ID("ddr_div2_clk", &es2_ddr_div2_clk),
	CLKDEV_CON_ID("ddr_div4_clk", &es2_ddr_div4_clk),
	CLKDEV_CON_ID("div4_ddr_clk", &es2_div4_clks[ES2_DIV4_DDR]),
	CLKDEV_CON_ID("div6_fsia_clk", &es2_div6_clks[ES2_DIV6_FSIA]),
	CLKDEV_CON_ID("div6_fsib_clk", &es2_div6_clks[ES2_DIV6_FSIB]),
	CLKDEV_CON_ID("m4_clk", &es2_div6_clks[ES2_DIV6_M4]),
	CLKDEV_CON_ID("m4d2_clk", &es2_m4d2_clk),
	CLKDEV_CON_ID("div6_mp_clk", &es2_div6_clks[ES2_DIV6_MP]),
	CLKDEV_CON_ID("div6_spua_clk", &es2_div6_clks[ES2_DIV6_SPUA]),
	CLKDEV_CON_ID("dsit_clk", &es2_div6_clks[ES2_DIV6_DSIT]),
	CLKDEV_CON_ID("dsi0p0_clk", &dsi0p0_clk),
	CLKDEV_CON_ID("dsi0p1_clk", &dsi0p1_clk),
	CLKDEV_CON_ID("fsiack_clk", &es2_fsiack_clk),
	CLKDEV_CON_ID("fsibck_clk", &es2_fsibck_clk),
	CLKDEV_CON_ID("dsi0p_clk", &es2_div6_clks[ES2_DIV6_DSI0P]),
	CLKDEV_ICK_ID("dsit_clk", "sh-mipi-dsi.0",
		      &es2_div6_clks[ES2_DIV6_DSIT]),
	CLKDEV_ICK_ID("dsi0p_clk", "sh-mipi-dsi.0",
		      &es2_div6_clks[ES2_DIV6_DSI0P]),
	CLKDEV_CON_ID("sd0_clk", &es2_div6_clks[ES2_DIV6_SD0]),
	CLKDEV_CON_ID("sd1_clk", &es2_div6_clks[ES2_DIV6_SD1]),
	CLKDEV_CON_ID("div6_zb_clk", &es2_div6_clks[ES2_DIV6_ZB]),
	CLKDEV_CON_ID("div6_hsi_clk", &es2_div6_clks[ES2_DIV6_HSI]),
	CLKDEV_CON_ID("vclk1_clk", &es2_div6_clks[ES2_DIV6_VCK1]),
	CLKDEV_CON_ID("vclk2_clk", &es2_div6_clks[ES2_DIV6_VCK2]),
	CLKDEV_CON_ID("vclk3_clk", &es2_div6_clks[ES2_DIV6_VCK3]),
	CLKDEV_CON_ID("vclk4_clk", &es2_div6_clks[ES2_DIV6_VCK4]),
	CLKDEV_CON_ID("vclk5_clk", &es2_div6_clks[ES2_DIV6_VCK5]),
	/* reparent candidates for peripherals */
	CLKDEV_CON_ID("r_clk", &extalr_clk),
	CLKDEV_CON_ID("i_clk", &es2_div4_clks[ES2_DIV4_I]),
	CLKDEV_CON_ID("b_clk", &es2_div4_clks[ES2_DIV4_B]),
	CLKDEV_CON_ID("zx_clk", &es2_div4_clks[ES2_DIV4_ZX]),
	CLKDEV_CON_ID("zt_clk", &es2_div4_clks[ES2_DIV4_ZT]),
/*	CLKDEV_CON_ID("z_clk", &div4_clks[DIV4_Z]), */
	CLKDEV_CON_ID("ztr_clk", &es2_ztr_clk),
	CLKDEV_CON_ID("zg_clk", &es2_div4_clks[ES2_DIV4_ZG]),
	CLKDEV_CON_ID("zs_clk", &es2_div4_clks[ES2_DIV4_ZS]),
	CLKDEV_CON_ID("hp_clk", &es2_div4_clks[ES2_DIV4_HP]),
	CLKDEV_CON_ID("cp_clk", &cp_clk),
	CLKDEV_CON_ID("m1_clk", &es2_div4_clks[ES2_DIV4_M1]),
	CLKDEV_CON_ID("mpc_clk", &es2_mpc_clk),
/* sound add */
	/*CLKDEV_CON_ID("mpmp_clk", &mpmp_clk),*/
/* sound add */
	CLKDEV_CON_ID("m3_clk", &es2_div4_clks[ES2_DIV4_M3]),
	CLKDEV_CON_ID("m5_clk", &es2_div4_clks[ES2_DIV4_M5]),
	CLKDEV_CON_ID("mp_clk", &es2_mp_clk),
	CLKDEV_CON_ID("z_clk", &z_clk),

	/* stray clocks */
	CLKDEV_CON_ID("zb30_clk", &es2_zb30_clk),

	CLKDEV_CON_ID("usb0_dmac", &es2_mstp_clks[ES2_MSTP214]),	/* USBHS-DMAC */
	CLKDEV_CON_ID("mfis", &es2_mstp_clks[ES2_MSTP213]),	/* MFIS */
	CLKDEV_CON_ID("currtimer", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_CON_ID("rwdt0", &es2_mstp_clks[ES2_MSTP402]),	/* RWDT0 */
	CLKDEV_CON_ID("internal_ram0", &es2_mstp_clks[ES2_MSTP527]),
	CLKDEV_CON_ID("clkgen", &es2_mstp_clks[ES2_MSTP224]),
/* VCD add start */
	CLKDEV_CON_ID("spuv", &es2_mstp_clks[ES2_MSTP220]),	/* SPUV */
/* VCD add end */
/* sound add */
	CLKDEV_CON_ID("scuw", &es2_mstp_clks[ES2_MSTP326]),	/* SCUW */
	CLKDEV_CON_ID("fsi", &es2_mstp_clks[ES2_MSTP328]),	/* FSI */
	CLKDEV_DEV_ID("sh_fsi2.0", &es2_mstp_clks[ES2_MSTP328]),	/* FSIA */
	CLKDEV_DEV_ID("sh_fsi2.1", &es2_mstp_clks[ES2_MSTP328]),	/* FSIB */
/* sound add */
	CLKDEV_DEV_ID("i2c-sh_mobile.2", &es2_mstp_clks[ES2_MSTP001]),	/* I2C2 */
	CLKDEV_DEV_ID("spi_sh_msiof.0", &es2_mstp_clks[ES2_MSTP000]),	/* MSIOF0 */
	CLKDEV_DEV_ID("sh-mipi-dsi.0", &es2_mstp_clks[ES2_MSTP118]),	/* DSI-TX0 */
	CLKDEV_DEV_ID("i2c-sh_mobile.0", &es2_mstp_clks[ES2_MSTP116]),	/* I2C0 */
	CLKDEV_DEV_ID("pvrsrvkm", &es2_mstp_clks[ES2_MSTP112]),	/* SGX544 */
	CLKDEV_DEV_ID("sh_mobile_lcdc_fb.0", &es2_mstp_clks[ES2_MSTP100]),	/* LCDC0 */
	CLKDEV_CON_ID("Crypt1", &es2_mstp_clks[ES2_MSTP229]),	/* Crypt1 */
	CLKDEV_DEV_ID("sh-dma-engine.0", &es2_mstp_clks[ES2_MSTP218]),	/* DMAC */
	CLKDEV_DEV_ID("sh-sci.6", &es2_mstp_clks[ES2_MSTP216]),	/* SCIFB2 */
	CLKDEV_DEV_ID("sh-sci.5", &es2_mstp_clks[ES2_MSTP207]),	/* SCIFB1 */
	CLKDEV_DEV_ID("sh-sci.4", &es2_mstp_clks[ES2_MSTP206]),	/* SCIFB0 */
	CLKDEV_DEV_ID("sh-sci.0", &es2_mstp_clks[ES2_MSTP204]),	/* SCIFA0 */
	CLKDEV_DEV_ID("sh-sci.1", &es2_mstp_clks[ES2_MSTP203]),	/* SCIFA1 */
	CLKDEV_DEV_ID("sh-sci.2", &es2_mstp_clks[ES2_MSTP202]),	/* SCIFA2 */
	CLKDEV_DEV_ID("sh-sci.3", &es2_mstp_clks[ES2_MSTP201]),	/* SCIFA3 */
	CLKDEV_DEV_ID("sh_cmt.10", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.11", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.12", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.13", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.14", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.15", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.16", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.17", &es2_mstp_clks[ES2_MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("i2c-sh_mobile.1", &es2_mstp_clks[ES2_MSTP323]),	/* I2C1 */
	CLKDEV_DEV_ID("i2c-sh_mobile.8", &es2_mstp_clks[ES2_MSTP427]),	/* I2C3H */
	CLKDEV_DEV_ID("i2c-sh_mobile.7", &es2_mstp_clks[ES2_MSTP426]),	/* I2C2H */
	CLKDEV_DEV_ID("i2c-sh_mobile.5", &es2_mstp_clks[ES2_MSTP425]),	/* I2C1H */
	CLKDEV_DEV_ID("i2c-sh_mobile.4", &es2_mstp_clks[ES2_MSTP424]),	/* I2C0H */
	CLKDEV_DEV_ID("i2c-sh_mobile.3", &es2_mstp_clks[ES2_MSTP411]),	/* I2C3 */
	CLKDEV_DEV_ID("r8a66597_udc.0", &es2_mstp_clks[ES2_MSTP322]),	/* USBHS */
	CLKDEV_DEV_ID("sh_mmcif.0", &es2_mstp_clks[ES2_MSTP315]),	/* MMCIF0 */
	CLKDEV_DEV_ID("renesas_sdhi.0", &es2_mstp_clks[ES2_MSTP314]),	/* SDHI0 */
	CLKDEV_DEV_ID("renesas_sdhi.1", &es2_mstp_clks[ES2_MSTP313]),	/* SDHI1 */
	CLKDEV_DEV_ID("renesas_sdhi.2", &es2_mstp_clks[ES2_MSTP312]),	/* SDHI2 */
	CLKDEV_DEV_ID("tpu-renesas-sh_mobile.0", &es2_mstp_clks[ES2_MSTP304]),	/* TPU0 */
	/*CLKDEV_DEV_ID("i2c-sh_mobile.4", &es2_mstp_clks[ES2_MSTP410]),*/ /* I2C4 */
	/*CLKDEV_DEV_ID("i2c-sh_mobile.5", &es2_mstp_clks[ES2_MSTP409]),*/ /* I2C5 */
	CLKDEV_DEV_ID("i2c-sh7730.6", &es2_mstp_clks[ES2_MSTP412]),	/* I2CM0 */
	CLKDEV_DEV_ID("sh_keysc.0", &es2_mstp_clks[ES2_MSTP403]),	/* KEYSC */
/* EOS-RCU ADD-S */
	CLKDEV_CON_ID("icb", &es2_mstp_clks[ES2_MSTP007]),	/* ICB */
	CLKDEV_CON_ID("meram", &es2_mstp_clks[ES2_MSTP113]),	/* MERAM */
	CLKDEV_CON_ID("rcu0", &es2_mstp_clks[ES2_MSTP107]),	/* RCUA0 */
	CLKDEV_CON_ID("rcu1", &es2_mstp_clks[ES2_MSTP105]),	/* RCUA1 */
	CLKDEV_CON_ID("RCU0", &es2_mstp_clks[ES2_MSTP107]),	/* RCUA0 */
	CLKDEV_CON_ID("RCU1", &es2_mstp_clks[ES2_MSTP105]),	/* RCUA1 */
	CLKDEV_DEV_ID("sh_mobile_rcu.0", &es2_mstp_clks[ES2_MSTP107]),	/* RCUA0 */
	CLKDEV_DEV_ID("sh_mobile_rcu.1", &es2_mstp_clks[ES2_MSTP105]),	/* RCUA1 */
	CLKDEV_DEV_ID("thermal_sensor.0", &es2_mstp_clks[ES2_MSTP522]),	/* Thermal Sensor */
/* EOS-RCU ADD-S */

/* EOS-CSI ADD-S */
	CLKDEV_CON_ID("csi21", &es2_mstp_clks[ES2_MSTP128]),	/* CSI2-RX1 */
	CLKDEV_CON_ID("csi20", &es2_mstp_clks[ES2_MSTP126]),	/* CSI2-RX0 */
	CLKDEV_DEV_ID("sh-mobile-csi2.1", &es2_mstp_clks[ES2_MSTP128]),	/* CSI2-RX1 */
	CLKDEV_DEV_ID("sh-mobile-csi2.0", &es2_mstp_clks[ES2_MSTP126]),	/* CSI2-RX0 */
/* EOS-CSI ADD-S */

	CLKDEV_CON_ID("slimb_clk", &es2_div6_clks[ES2_DIV6_SLIMB]),
	CLKDEV_CON_ID("sd2_clk", &es2_div6_clks[ES2_DIV6_SD2]),
/* VCD add start */
	CLKDEV_CON_ID("div6_spuv_clk", &es2_div6_clks[ES2_DIV6_SPUV]),
/* VCD add end */
	CLKDEV_CON_ID("dsi1p_clk", &es2_div6_clks[ES2_DIV6_DSI1P]),
	CLKDEV_CON_ID("dsi1p0_clk", &dsi1p0_clk),
	CLKDEV_CON_ID("dsi1p1_clk", &dsi1p1_clk),
	CLKDEV_DEV_ID("pcm2pwm-renesas-sh_mobile.1", &es2_mstp_clks[ES2_MSTP523]), /* PCM2PWM */
};
