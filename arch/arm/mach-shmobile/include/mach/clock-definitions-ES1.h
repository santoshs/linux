#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/sh_clk.h>
#include <asm/clkdev.h>
#include <mach/common.h>
#include <mach/r8a73734.h>

#define FRQCRA		IO_ADDRESS(0xE6150000)
#define FRQCRB		IO_ADDRESS(0xE6150004)
#define FRQCRD		IO_ADDRESS(0xE61500E4)
#define VCLKCR1		IO_ADDRESS(0xE6150008)
#define VCLKCR2		IO_ADDRESS(0xE615000C)
#define VCLKCR3		IO_ADDRESS(0xE6150014)
#define VCLKCR4		IO_ADDRESS(0xE615001C)
#define ZBCKCR		IO_ADDRESS(0xE6150010)
#define SD0CKCR		IO_ADDRESS(0xE6150074)
#define SD1CKCR		IO_ADDRESS(0xE6150078)
#define SD2CKCR		IO_ADDRESS(0xE615007C)
#define FSIACKCR	IO_ADDRESS(0xE6150018)
#define FSIBCKCR	IO_ADDRESS(0xE6150090)
#define MPCKCR		IO_ADDRESS(0xE6150080)
#define SPUACKCR	IO_ADDRESS(0xE6150084)
/* VCD add start */
#define SPUVCKCR	IO_ADDRESS(0xE6150094)
/* VCD add end */
#define HSICKCR		IO_ADDRESS(0xE615008C)
#define DSITCKCR	IO_ADDRESS(0xE6150060)
#define DSI0PCKCR	IO_ADDRESS(0xE6150064)
#define DSI0PHYCR	IO_ADDRESS(0xE615006C)
#define MPMODE		IO_ADDRESS(0xE61500CC)
#define RTSTBCR		IO_ADDRESS(0xE6150020)
#define PLLECR		IO_ADDRESS(0xE61500D0)
#define PLL0CR		IO_ADDRESS(0xE61500D8)
#define PLL1CR		IO_ADDRESS(0xE6150028)
#define PLL2CR		IO_ADDRESS(0xE615002C)
#define PLL3CR		IO_ADDRESS(0xE61500DC)
#define PLL0STPCR	IO_ADDRESS(0xE61500F0)
#define PLL1STPCR	IO_ADDRESS(0xE61500C8)
#define PLL2STPCR	IO_ADDRESS(0xE61500F8)
#define PLL3STPCR	IO_ADDRESS(0xE61500FC)
#define MSTPSR0		IO_ADDRESS(0xE6150030)
#define MSTPSR1		IO_ADDRESS(0xE6150038)
#define MSTPSR2		IO_ADDRESS(0xE6150040)
#define MSTPSR3		IO_ADDRESS(0xE6150048)
#define MSTPSR4		IO_ADDRESS(0xE615004C)
#define MSTPSR5		IO_ADDRESS(0xE615003C)
#define RMSTPCR0	IO_ADDRESS(0xE6150110)
#define RMSTPCR1	IO_ADDRESS(0xE6150114)
#define RMSTPCR2	IO_ADDRESS(0xE6150118)
#define RMSTPCR3	IO_ADDRESS(0xE615011C)
#define RMSTPCR4	IO_ADDRESS(0xE6150120)
#define RMSTPCR5	IO_ADDRESS(0xE6150124)
#define SMSTPCR0	IO_ADDRESS(0xE6150130)
#define SMSTPCR1	IO_ADDRESS(0xE6150134)
#define SMSTPCR2	IO_ADDRESS(0xE6150138)
#define SMSTPCR3	IO_ADDRESS(0xE615013C)
#define SMSTPCR4	IO_ADDRESS(0xE6150140)
#define SMSTPCR5	IO_ADDRESS(0xE6150144)
#define MMSTPCR0	IO_ADDRESS(0xE6150150)
#define MMSTPCR1	IO_ADDRESS(0xE6150154)
#define MMSTPCR2	IO_ADDRESS(0xE6150158)
#define MMSTPCR3	IO_ADDRESS(0xE615015C)
#define MMSTPCR4	IO_ADDRESS(0xE6150160)
#define MMSTPCR5	IO_ADDRESS(0xE6150164)
#define SRCR0		IO_ADDRESS(0xE61580A0)
#define SRCR1		IO_ADDRESS(0xE61580A8)
#define SRCR2		IO_ADDRESS(0xE61580B0)
#define SRCR3		IO_ADDRESS(0xE61580B8)
#define SRCR4		IO_ADDRESS(0xE61580BC)
#define SRCR5		IO_ADDRESS(0xE61580C4)
#define ASTAT		IO_ADDRESS(0xE6150054)
#define CKSCR		IO_ADDRESS(0xE61500C0)
#define SEQMON		IO_ADDRESS(0xE6150108)
#define VREFCR		IO_ADDRESS(0xE61500EC)
#define WUPCR		IO_ADDRESS(0xE6151010)
#define SRESCR		IO_ADDRESS(0xE6151018)
#define PCLKCR		IO_ADDRESS(0xE6151020)
#define PSTR		IO_ADDRESS(0xE6151040)
#define CPU0RFR		IO_ADDRESS(0xE6151104)
#define CPU1RFR		IO_ADDRESS(0xE6151114)
#define SPCTR		IO_ADDRESS(0xE61501A4)
#define SPCMMR		IO_ADDRESS(0xE61501AC)
#define SPCDMR		IO_ADDRESS(0xE61501B0)

#define EXSTMON2	IO_ADDRESS(0xe6180088)

static int extal2_clk_enable(struct clk *clk)
{
	__raw_writel(__raw_readl(PLL2CR) & ~1, PLL2CR);
	while (__raw_readl(EXSTMON2) & (1 << 16))	/* EX2MSK */
		cpu_relax();
	return 0;
}

static void extal2_clk_disable(struct clk *clk)
{
	__raw_writel(__raw_readl(PLL2CR) | 1, PLL2CR);	/* XOE */
	/*
	 * Make sure that EX2MSK bit gets set here in response to XOE bit
	 * set to '1' (Stop XTAL2 oscillation) above.   This is needed for
	 * safety in case that EXTAL2 clock gets enabled right after it's
	 * disabled.
	 */
	while (!(__raw_readl(EXSTMON2) & (1 << 16)))	/* EX2MSK */
		cpu_relax();
}

static struct clk_ops extal2_clk_ops = {
	.enable = extal2_clk_enable,
	.disable = extal2_clk_disable,
};

/* Externals */
static struct clk extal1_clk = {
	.rate = 26 * 1000 * 1000,
};

static struct clk extal2_clk = {
	.rate = 48 * 1000 * 1000,
	.ops = &extal2_clk_ops,
};

static struct clk extalr_clk = {
	.rate = 32768,
};

static struct clk fsiack_clk = { };

static struct clk fsibck_clk = { };

/* Constant divider */
static unsigned long const_div_recalc(struct clk *clk)
{
	return clk->parent->rate / (int)(clk->priv);
}

static struct clk_ops const_div_clk_ops = {
	.recalc = const_div_recalc,
};

static struct clk extal1_div2_clk = {
	.parent = &extal1_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk extal2_div2_clk = {
	.parent = &extal2_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk extal2_div4_clk = {
	.parent = &extal2_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)4,
};

static struct clk_ops followparent_clk_ops = {
	.recalc = followparent_recalc,
};

static struct clk *main_clk_parent[] = {
	[0] = &extal1_clk,
	[1] = &extal1_div2_clk,
	[2] = &extal2_clk,
	[3] = &extal2_div2_clk,
};

static struct clk main_clk = SH_CLK_CKSEL(NULL, CKSCR, 0, 0,
					  main_clk_parent,
					  ARRAY_SIZE(main_clk_parent), 28, 2);

static struct clk main_div2_clk = {
	.parent = &main_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

/* PLL0 */
static unsigned long pll0_recalc(struct clk *clk)
{
	unsigned long mult = 1;

	if (__raw_readl(PLLECR) & (1 << (8 + 0)))
		mult = ((__raw_readl(PLL0CR) >> 24) & 0x3f) + 1;

	return clk->parent->rate * mult;
}

static struct clk_ops pll0_clk_ops = {
	.recalc = pll0_recalc,
};

static struct clk pll0_clk = {
	.parent = &main_clk,
	.ops = &pll0_clk_ops,
};

/* PLL1 */
static unsigned long pll1_recalc(struct clk *clk)
{
	/* Assuming this type is big enough for calculation */
	unsigned long rate = clk->parent->rate;
	u32 pll1cr = __raw_readl(PLL1CR);
	if (__raw_readl(PLLECR) & (1 << (8 + 1))) {
		/* Do mul. then div., to avoid lower bit loss */
		rate *= ((pll1cr >> 24) & 0x3f) + 1;
		if (pll1cr & (1 << 20))	/* CFG */
			rate *= 2;
		if (pll1cr & (1 << 7))	/* CKSEL */
			rate /= 2;
	}
	return rate;
}

static struct clk_ops pll1_clk_ops = {
	.recalc = pll1_recalc,
};

static struct clk pll1_clk = {
	.parent = &main_clk,
	.ops = &pll1_clk_ops,
};

static struct clk pll1_div2_clk = {
	.parent = &pll1_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk pll1_div7_clk = {
	.parent = &pll1_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)7,
};

static struct clk pll1_div13_clk = {
	.parent = &pll1_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)13,
};

/* PLL2 */
/* Ops for pll2 and pll3 */
static unsigned long pll2223_recalc(struct clk *clk)
{
	unsigned long rate = clk->parent->rate;
	u32 pll2223cr = __raw_readl(clk->enable_reg);
	if (__raw_readl(PLLECR) & (1 << (8 + clk->enable_bit))) {
		rate *= ((pll2223cr >> 24) & 0x3f) + 1;	/* STC */
		if (pll2223cr & (1 << 20))	/* CFG for PLL2,22 but 3 */
			rate *= 2;
	}
	return rate;
}

static struct clk_ops pll2223_clk_ops = {
	.recalc = pll2223_recalc,
};

/* parent table for both pll2 and pll22 */
static struct clk *pll2_parent[] = {
	[0] = &main_div2_clk,
	[1] = &extal2_div2_clk,
	[3] = &extal2_div4_clk,
	[4] = &main_clk,
	[5] = &extal2_clk,
};

static struct clk pll2_cksel_clk = SH_CLK_CKSEL(NULL, PLL2CR, 0, 0,
						pll2_parent,
						ARRAY_SIZE(pll2_parent), 5, 3);

static struct clk pll2_clk = {
	.parent = &pll2_cksel_clk,
	.ops = &pll2223_clk_ops,
	.enable_reg = (void *__iomem)PLL2CR,
	.enable_bit = 2,	/* bit in PLLECR */
};

/* PLL3 */
static struct clk pll3_clk = {
	.parent = &main_clk,
	.ops = &pll2223_clk_ops,
	.enable_reg = (void *__iomem)PLL3CR,
	.enable_bit = 3,	/* bit in PLLECR */
};

/* Common divider */
static void common_kick(struct clk *clk)
{
	__raw_writel(__raw_readl(FRQCRB) | (1 << 31), FRQCRB);
}

static unsigned int common_divisors[] = {
	[0] = 2,
	[1] = 3,
	[2] = 4,
	[3] = 6,
	[4] = 8,
	[5] = 12,
	[6] = 16,
	[7] = 18,
	[8] = 24,
	[10] = 36,
	[11] = 48,
	[12] = 7,
};

static struct clk_div_mult_table common_div_mult_table = {
	.divisors = common_divisors,
	.nr_divisors = ARRAY_SIZE(common_divisors),
};

static struct clk_div4_table common_div4_table = {
	.div_mult_table = &common_div_mult_table,
	.kick = common_kick,
};

enum {
	DIV4_I, DIV4_ZG, DIV4_M3, DIV4_B, DIV4_M1,
	DIV4_Z, DIV4_ZTR, DIV4_ZT, DIV4_ZX, DIV4_ZS, DIV4_HP,
	DIV4_DDR,
	DIV4_NR
};

static struct clk div4_clks[DIV4_NR] = {
	[DIV4_I] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 20, 0xdff, 0),
	[DIV4_ZG] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 16, 0x97f, 0),
	[DIV4_M3] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 12, 0x1dff, 0),
	[DIV4_B] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 8, 0xdff, 0),
	[DIV4_M1] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 4, 0x1dff, 0),
	[DIV4_Z] = SH_CLK_DIV4(NULL, FRQCRB, 24, 0x097f, 0),
	[DIV4_ZTR] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 20, 0x0dff, 0),
	[DIV4_ZT] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 16, 0xdff, 0),
	[DIV4_ZX] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 12, 0xdff, 0),
	[DIV4_ZS] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 8, 0xdff, 0),
	[DIV4_HP] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 4, 0xdff, 0),
	[DIV4_DDR] = SH_CLK_DIV4(&pll3_clk, FRQCRD, 0, 0x97f, 0),
};

static struct clk ztr_clk = {
	.parent = &div4_clks[DIV4_ZTR],
	.ops = &const_div_clk_ops,
	.priv = (void *)1,
};

static struct clk ztrd2_clk = {
	.parent = &div4_clks[DIV4_ZTR],
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk *ddr_parent[] = {
	[0] = &pll3_clk,
	[1] = &div4_clks[DIV4_DDR],
};

static struct clk ddr_clk = SH_CLK_CKSEL(NULL, FRQCRD, 0, 0,
					 ddr_parent, ARRAY_SIZE(ddr_parent), 4,
					 1);

static struct clk zb30_clk = {
	.parent = &ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk zb30d2_clk = {
	.parent = &ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)4,
};

static struct clk ddr_div2_clk = {
	.parent = &ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)2,
};

static struct clk ddr_div4_clk = {
	.parent = &ddr_clk,
	.ops = &const_div_clk_ops,
	.priv = (void *)4,
};

static struct clk *zb30sl_parent[] = {
	[0] = &ddr_div2_clk,
	[1] = &ddr_div4_clk,
};

static struct clk zb30sl_clk = SH_CLK_CKSEL(NULL, FRQCRD, 0, 0,
					    zb30sl_parent,
					    ARRAY_SIZE(zb30sl_parent), 15, 1);

/* DIV6s */
static struct clk *div6_two_parent[] = {
	[0] = &pll1_div2_clk,
	[1] = &pll2_clk,
};

static struct clk *sd_parent[] = {
	[0] = &pll1_div2_clk,
	[1] = &pll2_clk,
	[2] = &pll1_div13_clk,
};

static struct clk *hsi_parent[] = {
	[0] = &pll1_div2_clk,
	[1] = &pll2_clk,
	[2] = &pll1_div7_clk,
};

static struct clk *vclk_parent[] = {
	[0] = &pll1_div2_clk,
	[1] = &pll2_clk,
	[3] = &extal2_clk,
	[4] = &main_div2_clk,
	[5] = &extalr_clk,
	[6] = &main_clk,
};

static struct clk vclk3_cksel_clk = SH_CLK_CKSEL(NULL, VCLKCR3, 0, 0,
						 vclk_parent,
						 ARRAY_SIZE(vclk_parent), 12,
						 3);

/* Dynamic divider. Only for recalc. No setting capable. */
static unsigned long div_recalc(struct clk *clk)
{
	return clk->parent->rate /
	    ((__raw_readl(clk->enable_reg) >> clk->src_shift
	      & ((1 << clk->src_width) - 1))
	     + 1);
}

static struct clk_ops div_clk_ops = {
	.recalc = div_recalc,
};

/* pre divider followed by vclk3 */
static struct clk vclk3_pdiv_clk = {
	.parent = &vclk3_cksel_clk,
	.ops = &div_clk_ops,
	.enable_reg = (void *__iomem)VCLKCR3,
	.src_shift = 6,
	.src_width = 2,
};

enum {
	DIV6_MP,
	DIV6_ZB,
	DIV6_SD0,
	DIV6_SD1,
	DIV6_VCK1,
	DIV6_VCK2,
	DIV6_VCK3,
	DIV6_VCK4,
	DIV6_FSIA,
	DIV6_FSIB,
	DIV6_SPUA,
/* VCD add start */
	DIV6_SPUV,
/* VCD add end */
	DIV6_HSI,
	DIV6_DSIT,
	DIV6_DSI0P,
	DIV6_NR
};

static struct clk div6_clks[DIV6_NR] = {
	[DIV6_MP] = SH_CLK_DIV6_EXT(NULL, MPCKCR, 9, CLK_DIV_SHARED,
				    div6_two_parent,
				    ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_ZB] = SH_CLK_DIV6_EXT(NULL, ZBCKCR, 8, 0,
				    div6_two_parent,
				    ARRAY_SIZE(div6_two_parent), 7, 1),
	[DIV6_SD0] = SH_CLK_DIV6_EXT(NULL, SD0CKCR, 8, 0,
				     sd_parent, ARRAY_SIZE(sd_parent), 6, 2),
	[DIV6_SD1] = SH_CLK_DIV6_EXT(NULL, SD1CKCR, 8, 0,
				     sd_parent, ARRAY_SIZE(sd_parent), 6, 2),
	[DIV6_VCK1] = SH_CLK_DIV6_EXT(NULL, VCLKCR1, 8, 0,
				      vclk_parent, ARRAY_SIZE(vclk_parent), 12,
				      3),
	[DIV6_VCK2] = SH_CLK_DIV6_EXT(NULL, VCLKCR2, 8, 0,
				      vclk_parent, ARRAY_SIZE(vclk_parent), 12,
				      3),
	[DIV6_VCK3] = SH_CLK_DIV6_EXT(&vclk3_pdiv_clk, VCLKCR3, 8, 0,
				      NULL, 0, 0, 0),
	[DIV6_VCK4] = SH_CLK_DIV6_EXT(NULL, VCLKCR4, 8, 0,
				      vclk_parent, ARRAY_SIZE(vclk_parent), 12,
				      3),
	[DIV6_FSIA] = SH_CLK_DIV6_EXT(NULL, FSIACKCR, 8, 0,
				      div6_two_parent,
				      ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_FSIB] = SH_CLK_DIV6_EXT(NULL, FSIBCKCR, 8, 0,
				      div6_two_parent,
				      ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_SPUA] = SH_CLK_DIV6_EXT(NULL, SPUACKCR, 8, 0,
				      div6_two_parent,
				      ARRAY_SIZE(div6_two_parent), 6, 1),
/* VCD add start */
	[DIV6_SPUV] = SH_CLK_DIV6_EXT(NULL, SPUVCKCR, 8, 0,
				      div6_two_parent,
				      ARRAY_SIZE(div6_two_parent), 6, 1),
/* VCD add end */
	[DIV6_HSI] = SH_CLK_DIV6_EXT(NULL, HSICKCR, 8, 0,
				     hsi_parent, ARRAY_SIZE(hsi_parent), 6, 2),
	[DIV6_DSIT] = SH_CLK_DIV6_EXT(NULL, DSITCKCR, 8, 0,
				      div6_two_parent, ARRAY_SIZE(hsi_parent),
				      7, 1),
	[DIV6_DSI0P] = SH_CLK_DIV6(&pll1_div2_clk, DSI0PCKCR, 0),
};

static struct clk *mp_parent[] = {
	[0] = &div6_clks[DIV6_MP],
	[1] = &extal2_clk,
};

static struct clk mp_clk = SH_CLK_CKSEL(NULL, MPCKCR, 9, CLK_CKSEL_CKSTP,
					mp_parent, ARRAY_SIZE(mp_parent), 7, 1);

static struct clk mpc_clk = SH_CLK_CKSEL(NULL, MPCKCR, 10, CLK_CKSEL_CKSTP,
					 mp_parent, ARRAY_SIZE(mp_parent), 7,
					 1);
/* sound add */
static struct clk mpmp_clk = SH_CLK_CKSEL(NULL, MPCKCR, 11, CLK_CKSEL_CKSTP,
					  mp_parent, ARRAY_SIZE(mp_parent), 7,
					  1);
/* sound add */

static struct clk *fsia_parent[] = {
	[0] = &div6_clks[DIV6_FSIA],
	[1] = &fsiack_clk,
};

static struct clk fsia_clk = SH_CLK_CKSEL(NULL, FSIACKCR, 0, 0,
					  fsia_parent, ARRAY_SIZE(fsia_parent),
					  7, 1);

static struct clk *fsib_parent[] = {
	[0] = &div6_clks[DIV6_FSIB],
	[1] = &fsibck_clk,
};

static struct clk fsib_clk = SH_CLK_CKSEL(NULL, FSIBCKCR, 0, 0,
					  fsib_parent, ARRAY_SIZE(fsib_parent),
					  7, 1);

static struct clk *spua_parent[] = {
	[0] = &div6_clks[DIV6_SPUA],
	[1] = &extal2_clk,
};

static struct clk spua_clk = SH_CLK_CKSEL(NULL, SPUACKCR, 0, 0,
					  spua_parent, ARRAY_SIZE(spua_parent),
					  7, 1);

/* VCD add start */
static struct clk *spuv_parent[] = {
	[0] = &div6_clks[DIV6_SPUV],
	[1] = &extal2_clk,
};

static struct clk spuv_clk = SH_CLK_CKSEL(NULL, SPUVCKCR, 0, 0,
					  spuv_parent, ARRAY_SIZE(spuv_parent),
					  7, 1);

/* VCD add end */

/* DSI0P
 * HW clock topology is like
 *  {parents}[SELMON0? EXSRC : EXSRCB] / (SELMON0? DIV : DIVB)
 * But here implemented as
 *  SELMON0? ({parents}[EXSRC] / DIV) : ({parents}[EXSRCB] / DIVB)
 */
static struct clk extcki_clk = { };

static struct clk *dsi0p_parent[] = {
	[0] = &pll1_div2_clk,
	[1] = &pll2_clk,
	[2] = &main_clk,
	[3] = &extal2_clk,
	[4] = &extcki_clk,
};

static struct clk dsi0p0_cksel_clk = SH_CLK_CKSEL(NULL, DSI0PCKCR,
						  8, CLK_CKSEL_CKSTP,
						  dsi0p_parent,
						  ARRAY_SIZE(dsi0p_parent), 24,
						  3);

static struct clk dsi0p0_clk = {
	.parent = &dsi0p0_cksel_clk,
	.ops = &div_clk_ops,
	.enable_reg = (void *__iomem)DSI0PCKCR,
	.src_shift = 16,
	.src_width = 6,
};

static struct clk dsi0p1_cksel_clk = SH_CLK_CKSEL(NULL, DSI0PCKCR,
						  8, CLK_CKSEL_CKSTP,
						  dsi0p_parent,
						  ARRAY_SIZE(dsi0p_parent), 12,
						  3);

static struct clk dsi0p1_clk = {
	.parent = &dsi0p1_cksel_clk,
	.ops = &div_clk_ops,
	.enable_reg = (void *__iomem)DSI0PCKCR,
	.src_shift = 0,
	.src_width = 6,
};

/* You can not set dsi0pckcr.selmon0, but it tggles.
 * Call as sel_reparent(dsi0pclk, NULL) to set parent according to
 * the register value. */
/* @parent not used */
static int selmon_reparent(struct clk *clk, struct clk *parent)
{
	int sel = (__raw_readl(clk->enable_reg) >> clk->src_shift)
	    & ((1 << clk->src_width) - 1);
	return clk_reparent(clk, clk->parent_table[sel]);
}

static int selmon_enable(struct clk *clk)
{
	__raw_writel(__raw_readl(clk->enable_reg) & ~(1 << clk->enable_bit),
		     clk->enable_reg);
	return 0;
}

static void selmon_disable(struct clk *clk)
{
	__raw_writel(__raw_readl(clk->enable_reg) | (1 << clk->enable_bit),
		     clk->enable_reg);
}

static struct clk_ops selmon_clk_ops = {
	.recalc = followparent_recalc,
	.set_parent = selmon_reparent,
	.enable = selmon_enable,
	.disable = selmon_disable,
};

static struct clk *selmon0_parent[2] = {
	[0] = &dsi0p0_clk,
	[1] = &dsi0p1_clk,
};

static struct clk dsi0p_clk = {
	.ops = &selmon_clk_ops,
	.enable_reg = (void *__iomem)DSI0PCKCR,
	.enable_bit = 8,
	.parent_table = selmon0_parent,
	.src_shift = 27,
	.src_width = 1,
};

static struct clk hdmi0_clk = {
	.ops = &selmon_clk_ops,
	.enable_reg = (void *__iomem)DSI0PCKCR,
	.enable_bit = 9,
	.parent_table = selmon0_parent,
	.src_shift = 27,
	.src_width = 1,
};

static struct clk *cp_parent[] = {
	[0] = &main_div2_clk,
	[1] = &extalr_clk,
};

static struct clk cp_clk = SH_CLK_CKSEL(&main_div2_clk, 0, 0, 0,
					cp_parent, ARRAY_SIZE(cp_parent), 0, 0);

static struct clk z_clk = {
	.ops = &followparent_clk_ops,
};

enum {
	MSTP031,
	MSTP030,
	MSTP029,
	MSTP026,
	MSTP022,
	MSTP021,
	MSTP019,
	MSTP018,
	MSTP017,
	MSTP016,
	MSTP015,
	MSTP007,
	MSTP002,
	MSTP001,
	MSTP000,
	MSTP130,
	MSTP128,
	MSTP126,
	MSTP125,
	MSTP124,
	MSTP122,
	MSTP119,
	MSTP118,
	MSTP116,
	MSTP115,
	MSTP113,
	MSTP112,
	MSTP111,
	MSTP108,
	MSTP107,
	MSTP106,
	MSTP105,
	MSTP101,
	MSTP100,
	MSTP229,
	MSTP228,
	MSTP224,
	MSTP223,
	MSTP220,
	MSTP218,
	MSTP216,
	MSTP215,
	MSTP214,
	MSTP213,
	MSTP208,
	MSTP207,
	MSTP206,
	MSTP205,
	MSTP204,
	MSTP203,
	MSTP202,
	MSTP201,
	MSTP330,
	MSTP329,
	MSTP328,
	MSTP326,
	MSTP325,
	MSTP324,
	MSTP323,
	MSTP322,
	MSTP321,
	MSTP319,
	MSTP316,
	MSTP315,
	MSTP314,
	MSTP313,
	MSTP309,
	MSTP308,
	MSTP307,
	MSTP306,
	MSTP305,
	MSTP304,
	MSTP415,
	MSTP412,
	MSTP410,
	MSTP409,
	MSTP407,
	MSTP406,
	MSTP403,
	MSTP402,
	MSTP530,
	MSTP529,
	MSTP528,
	MSTP527,
	MSTP526,
	MSTP525,
	MSTP522,
	MSTP519,
	MSTP518,
	MSTP517,
	MSTP516,
	MSTP508,
	MSTP507,
	MSTP501,
	MSTP500,
	MSTP_NR
};

#define DIV4_ 0			/* XXX fill all of these */

static struct clk mstp_clks[MSTP_NR] = {
	[MSTP031] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 31, 0),	/* RT-CPU TLB */
	[MSTP030] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 30, 0),	/* RT-CPU Instruction Cache (IC) */
	[MSTP029] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 29, 0),	/* RT-CPU Operand Cache (OC) */
	[MSTP026] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 26, 0),	/* RT-CPU X/Y memory */
	[MSTP022] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 22, 0),	/* INTCS */
	[MSTP021] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR0, MSTPSR0, 21, 0),	/* RT-DMAC */
	[MSTP019] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 19, 0),	/* H-UDI */
	[MSTP018] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 18, 0),	/* RT-CPU debug module 1 */
	[MSTP017] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 17, 0),	/* UBC */
	[MSTP016] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 16, 0),	/* RT-CPU debug module 2 */
	[MSTP015] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 15, 0),	/* ILRAM */
	[MSTP007] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 7, 0),	/* ICB */
	[MSTP002] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 2, 0),	/* test module. */
	[MSTP001] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 1, 0),	/* IIC2 */
	[MSTP000] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR0, MSTPSR0, 0, 0),	/* MSIOF0 */
	[MSTP130] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 30, 0),	/* VIO60 */
	[MSTP128] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS],
				      SMSTPCR1, MSTPSR1, 28, 0),	/* CSI2-RX1 */
	[MSTP126] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR1, MSTPSR1, 26, 0),	/* CSI2-RX0 */
	[MSTP125] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR1, MSTPSR1, 25, 0),	/* TMU0 */
	[MSTP124] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_B], SMSTPCR1, MSTPSR1, 24, 0),	/* CMT0 */
	[MSTP122] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 22, 0),	/* TSG */
	[MSTP119] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 19, 0),	/* ISP */
	[MSTP118] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M3], SMSTPCR1, MSTPSR1, 18, 0),	/* DSI-TX0 */
	[MSTP116] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR1, MSTPSR1, 16, 0),	/* IIC0 */
	[MSTP115] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_B], SMSTPCR1, MSTPSR1, 15, 0),	/* 2D-DMAC */
	[MSTP113] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR1, MSTPSR1, 13, 0),	/* MERAM */
	[MSTP112] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZG], SMSTPCR1, MSTPSR1, 12, 0),	/* SGX543 */
	[MSTP111] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR1, MSTPSR1, 11, 0),	/* TMU1 */
	[MSTP108] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 8, 0),	/* TSIF0 */
	[MSTP107] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 7, 0),	/* RCUA */
	[MSTP106] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_B], SMSTPCR1, MSTPSR1, 6, 0),	/* JPU */
	[MSTP105] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_],
				      SMSTPCR1, MSTPSR1, 5, 0),	/* RCUA1 */
	[MSTP101] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M1], SMSTPCR1, MSTPSR1, 1, 0),	/* VPU */
	[MSTP100] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M3], SMSTPCR1, MSTPSR1, 0, 0),	/* LCDC0 */
	[MSTP229] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 29, 0),	/* Crypt1 */
	[MSTP228] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 28, 0),	/* Crypt */
/* sound edit */
	/*	[MSTP224] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 24, 0),*//* CLKGEN */
	[MSTP224] = SH_CLK_MSTP32_EXT(&mpmp_clk, SMSTPCR2, MSTPSR2, 24, 0),	/* CLKGEN */
/* sound edit */
	[MSTP223] = SH_CLK_MSTP32_EXT(&spua_clk, SMSTPCR2, MSTPSR2, 23, 0),	/* SPU2A */
/* VCD mod start */
/*	[MSTP220] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 20, 0),*/ /* SPU2V */
	[MSTP220] = SH_CLK_MSTP32_EXT(&spuv_clk, SMSTPCR2, MSTPSR2, 20, 0),	/* SPU2V */
/* VCD mod end */
	[MSTP218] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 18, 0),	/* SY-DMAC */
	[MSTP216] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 16, 0),	/* SCIFB2 */
	[MSTP215] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 15, 0),	/* MSIOF3 */
	[MSTP214] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 14, 0),	/* USB-DMAC */
	[MSTP213] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 13, 0),	/* MFIS */
	[MSTP208] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 8, 0),	/* MSIOF1 */
	[MSTP207] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 7, 0),	/* SCIFB1 */
	[MSTP206] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 6, 0),	/* SCIFB0 */
	[MSTP205] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 5, 0),	/* MSIOF2 */
	[MSTP204] = SH_CLK_MSTP32_EXT(&mpc_clk, SMSTPCR2, MSTPSR2, 4, 0),	/* SCIFA0 */
	[MSTP203] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 3, 0),	/* SCIFA1 */
	[MSTP202] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 2, 0),	/* SCIFA2 */
	[MSTP201] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 1, 0),	/* SCIFA3 */
	[MSTP330] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 30, 0),	/* MSU */
	[MSTP329] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR3, MSTPSR3, 29, 0),	/* CMT1 */
/* sound edit */
	/*	[MSTP328] = SH_CLK_MSTP32_EXT(&fsia_clk, SMSTPCR3, MSTPSR3, 28, 0),*//* FSI */
	[MSTP328] = SH_CLK_MSTP32_EXT(&mpmp_clk, SMSTPCR3, MSTPSR3, 28, 0),	/* FSI */
	/*	[MSTP326] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 26, 0),*//* SCUW */
	[MSTP326] = SH_CLK_MSTP32_EXT(&mpmp_clk, SMSTPCR3, MSTPSR3, 26, 0),	/* SCUW */
/* sound edit */
	[MSTP325] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_HSI], SMSTPCR3, MSTPSR3, 25, 0),	/* HSI1 */
	[MSTP324] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_HSI], SMSTPCR3, MSTPSR3, 24, 0),	/* HSI0 */
	[MSTP323] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 23, 0),	/* IIC1 */
	[MSTP322] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR3, MSTPSR3, 22, 0),	/* USB */
	[MSTP321] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 21, 0),	/* SBSC performance monitor */
	[MSTP319] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 19, 0),	/* RT-DMAC scheduler */
	[MSTP316] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR3, MSTPSR3, 16, 0),	/* SHWYSTAT */
	[MSTP315] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 15, 0),	/* MMCIF0 */
	[MSTP314] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_SD0], SMSTPCR3, MSTPSR3, 14, 0),	/* SDHI0 */
	[MSTP313] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_SD1], SMSTPCR3, MSTPSR3, 13, 0),	/* SDHI1 */
	[MSTP309] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR3, MSTPSR3, 9, 0),	/* ICU-USB */
	[MSTP308] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 8, 0),	/* MFIR */
	[MSTP307] = SH_CLK_MSTP32_EXT(&mpc_clk, SMSTPCR3, MSTPSR3, 7, 0),	/* ICU-DMAC1 */
	[MSTP306] = SH_CLK_MSTP32_EXT(&mpc_clk, SMSTPCR3, MSTPSR3, 6, 0),	/* ICU-DMAC0 */
	[MSTP305] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 5, 0),	/* MMC1 */
	[MSTP304] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M1], SMSTPCR3, MSTPSR3, 4, 0),	/* TPU0 */
	[MSTP415] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR4, MSTPSR4, 15, 0),	/* GPRMP */
	[MSTP412] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR4, MSTPSR4, 12, 0),	/* IICMS */
	[MSTP410] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR4, MSTPSR4, 10, 0),	/* IIC4 */
	[MSTP409] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR4, MSTPSR4, 9, 0),	/* IIC5 */
	[MSTP407] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR4, MSTPSR4, 7, 0),	/* USB-DMAC1 */
	[MSTP406] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR4, MSTPSR4, 6, 0),	/* DDM */
	[MSTP403] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR4, MSTPSR4, 3, 0),	/* KEYSC */
	[MSTP402] = SH_CLK_MSTP32_EXT(&extalr_clk, SMSTPCR4, MSTPSR4, 2, 0),	/* RWDT0 */
	[MSTP530] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 30, 0),	/* Secure ROM */
	[MSTP529] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 29, 0),	/* Secure RAM */
	[MSTP528] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 28, 0),	/* Internal RAM1 */
	[MSTP527] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 27, 0),	/* Internal RAM0 */
	[MSTP526] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 26, 0),	/* Public ROM */
	[MSTP525] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 25, 0),	/* IICB0 */
	[MSTP522] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR5, MSTPSR5, 22, 0),	/* Thermal Sensor */
	[MSTP519] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 19, 0),	/* O2S */
	[MSTP518] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 18, 0),	/* O2M */
	[MSTP517] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 17, 0),	/* S2O1 */
	[MSTP516] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 16, 0),	/* S2O0 */
	[MSTP508] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 8, 0),	/* INTCM */
	[MSTP507] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 7, 0),	/* IRQC */
	[MSTP501] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 1, 0),	/* SPU2A Core1 */
	[MSTP500] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 0, 0),	/* SPU2A Core0 */
};

static struct clk_lookup lookups[] = {
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
	CLKDEV_CON_ID("pll3_clk", &pll3_clk),
	CLKDEV_CON_ID("ddr_div2_clk", &ddr_div2_clk),
	CLKDEV_CON_ID("ddr_div4_clk", &ddr_div4_clk),
	CLKDEV_CON_ID("div4_ddr_clk", &div4_clks[DIV4_DDR]),
	CLKDEV_CON_ID("div6_fsia_clk", &div6_clks[DIV6_FSIA]),
	CLKDEV_CON_ID("div6_fsib_clk", &div6_clks[DIV6_FSIB]),
	CLKDEV_CON_ID("div6_mp_clk", &div6_clks[DIV6_MP]),
	CLKDEV_CON_ID("div6_spua_clk", &div6_clks[DIV6_SPUA]),
/* VCD add start */
	CLKDEV_CON_ID("div6_spuv_clk", &div6_clks[DIV6_SPUV]),
/* VCD add end */
	CLKDEV_CON_ID("dsit_clk", &div6_clks[DIV6_DSIT]),
	CLKDEV_CON_ID("div6_hsi_clk", &div6_clks[DIV6_HSI]),
	CLKDEV_CON_ID("dsi0p_clk", &div6_clks[DIV6_DSI0P]),
	CLKDEV_CON_ID("dsi0p0_clk", &dsi0p0_clk),
	CLKDEV_CON_ID("dsi0p1_clk", &dsi0p1_clk),
	CLKDEV_CON_ID("fsiack_clk", &fsiack_clk),
	CLKDEV_CON_ID("fsibck_clk", &fsibck_clk),
	CLKDEV_ICK_ID("dsit_clk", "sh-mipi-dsi.0", &div6_clks[DIV6_DSIT]),
	CLKDEV_ICK_ID("dsi0p_clk", "sh-mipi-dsi.0", &div6_clks[DIV6_DSI0P]),
	CLKDEV_CON_ID("sd0_clk", &div6_clks[DIV6_SD0]),
	CLKDEV_CON_ID("sd1_clk", &div6_clks[DIV6_SD1]),
	CLKDEV_CON_ID("vclk1_clk", &div6_clks[DIV6_VCK1]),
	CLKDEV_CON_ID("vclk2_clk", &div6_clks[DIV6_VCK2]),
	CLKDEV_CON_ID("vclk3_clk", &div6_clks[DIV6_VCK3]),
	CLKDEV_CON_ID("vclk4_clk", &div6_clks[DIV6_VCK4]),

	/* reparent candidates for peripherals */
	CLKDEV_CON_ID("r_clk", &extalr_clk),
	CLKDEV_CON_ID("i_clk", &div4_clks[DIV4_I]),
	CLKDEV_CON_ID("b_clk", &div4_clks[DIV4_B]),
	CLKDEV_CON_ID("zx_clk", &div4_clks[DIV4_ZX]),
	CLKDEV_CON_ID("zt_clk", &div4_clks[DIV4_ZT]),
/*	CLKDEV_CON_ID("z_clk", &div4_clks[DIV4_Z]), */
	CLKDEV_CON_ID("ztr_clk", &ztr_clk),
	CLKDEV_CON_ID("zs_clk", &div4_clks[DIV4_ZS]),
	CLKDEV_CON_ID("hp_clk", &div4_clks[DIV4_HP]),
	CLKDEV_CON_ID("cp_clk", &cp_clk),
	CLKDEV_CON_ID("m1_clk", &div4_clks[DIV4_M1]),
	CLKDEV_CON_ID("mpc_clk", &mpc_clk),
	CLKDEV_CON_ID("zg_clk", &div4_clks[DIV4_ZG]),
	CLKDEV_CON_ID("div6_zb_clk", &div6_clks[DIV6_ZB]),
/* sound add */
	CLKDEV_CON_ID("mpmp_clk", &mpmp_clk),
/* sound add */
	CLKDEV_CON_ID("m3_clk", &div4_clks[DIV4_M3]),
	CLKDEV_CON_ID("mp_clk", &mp_clk),
	CLKDEV_CON_ID("z_clk", &z_clk),

	/* stray clocks */
	CLKDEV_CON_ID("ztrd2_clk", &ztrd2_clk),
	CLKDEV_CON_ID("zb30_clk", &zb30_clk),
	CLKDEV_CON_ID("zb30d2_clk", &zb30d2_clk),

	CLKDEV_CON_ID("usb0_dmac", &mstp_clks[MSTP214]),	/* USBHS-DMAC */
	CLKDEV_CON_ID("mfis", &mstp_clks[MSTP213]),	/* MFIS */
	CLKDEV_CON_ID("currtimer", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_CON_ID("rwdt0", &mstp_clks[MSTP402]),	/* RWDT0 */
	CLKDEV_CON_ID("internal_ram0", &mstp_clks[MSTP527]),

	CLKDEV_CON_ID("clkgen", &mstp_clks[MSTP224]),
/* VCD add start */
	CLKDEV_CON_ID("spuv", &mstp_clks[MSTP220]),	/* SPUV */
/* VCD add end */
/* sound add */
	CLKDEV_CON_ID("scuw", &mstp_clks[MSTP326]),	/* SCUW */
	CLKDEV_CON_ID("fsi", &mstp_clks[MSTP328]),	/* FSI */

	CLKDEV_DEV_ID("sh_fsi2.0", &mstp_clks[MSTP328]),	/* FSIA */
	CLKDEV_DEV_ID("sh_fsi2.1", &mstp_clks[MSTP328]),	/* FSIB */
/* sound add */
	CLKDEV_DEV_ID("i2c-sh_mobile.2", &mstp_clks[MSTP001]),	/* I2C2 */
	CLKDEV_DEV_ID("spi_sh_msiof.0", &mstp_clks[MSTP000]),	/* MSIOF0 */
	CLKDEV_DEV_ID("sh-mipi-dsi.0", &mstp_clks[MSTP118]),	/* DSI-TX0 */
	CLKDEV_DEV_ID("i2c-sh_mobile.0", &mstp_clks[MSTP116]),	/* I2C0 */
	CLKDEV_DEV_ID("pvrsrvkm", &mstp_clks[MSTP112]),	/* SGX544 */
	CLKDEV_DEV_ID("sh_mobile_lcdc_fb.0", &mstp_clks[MSTP100]),	/* LCDC0 */
	CLKDEV_CON_ID("Crypt1", &mstp_clks[MSTP229]),	/* Crypt1 */
	CLKDEV_DEV_ID("sh-dma-engine.0", &mstp_clks[MSTP218]),	/* DMAC */
	CLKDEV_DEV_ID("sh-sci.6", &mstp_clks[MSTP216]),	/* SCIFB2 */
	CLKDEV_DEV_ID("sh-sci.5", &mstp_clks[MSTP207]),	/* SCIFB1 */
	CLKDEV_DEV_ID("sh-sci.4", &mstp_clks[MSTP206]),	/* SCIFB0 */
	CLKDEV_DEV_ID("sh-sci.0", &mstp_clks[MSTP204]),	/* SCIFA0 */
	CLKDEV_DEV_ID("sh-sci.1", &mstp_clks[MSTP203]),	/* SCIFA1 */
	CLKDEV_DEV_ID("sh-sci.2", &mstp_clks[MSTP202]),	/* SCIFA2 */
	CLKDEV_DEV_ID("sh-sci.3", &mstp_clks[MSTP201]),	/* SCIFA3 */
	CLKDEV_DEV_ID("sh_cmt.10", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.11", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.12", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.13", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.14", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.15", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.16", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.17", &mstp_clks[MSTP329]),	/* CMT1 */
	CLKDEV_DEV_ID("i2c-sh_mobile.1", &mstp_clks[MSTP323]),	/* I2C1 */
	CLKDEV_DEV_ID("r8a66597_udc.0", &mstp_clks[MSTP322]),	/* USBHS */
	CLKDEV_DEV_ID("sh_mmcif.0", &mstp_clks[MSTP315]),	/* MMCIF0 */
	CLKDEV_DEV_ID("renesas_sdhi.0", &mstp_clks[MSTP314]),	/* SDHI0 */
	CLKDEV_DEV_ID("renesas_sdhi.1", &mstp_clks[MSTP313]),	/* SDHI1 */
	CLKDEV_DEV_ID("tpu-renesas-sh_mobile.0", &mstp_clks[MSTP304]),	/* TPU0 */
	CLKDEV_DEV_ID("i2c-sh_mobile.4", &mstp_clks[MSTP410]),	/* I2C4 */
	CLKDEV_DEV_ID("i2c-sh_mobile.5", &mstp_clks[MSTP409]),	/* I2C5 */
	CLKDEV_DEV_ID("i2c-sh7730.6", &mstp_clks[MSTP412]),	/* I2CM0 */
	CLKDEV_DEV_ID("sh_keysc.0", &mstp_clks[MSTP403]),	/* KEYSC */
	CLKDEV_DEV_ID("thermal_sensor.0", &mstp_clks[MSTP522]),	/* Thermal Sensor */
	CLKDEV_CON_ID("icb", &mstp_clks[MSTP007]),	/* ICB */
	CLKDEV_CON_ID("meram", &mstp_clks[MSTP113]),	/* MERAM */
	CLKDEV_CON_ID("rcu0", &mstp_clks[MSTP107]),	/* RCUA0 */
	CLKDEV_CON_ID("rcu1", &mstp_clks[MSTP105]),	/* RCUA1 */
	CLKDEV_CON_ID("RCU0", &mstp_clks[MSTP107]),	/* RCUA0 */
	CLKDEV_CON_ID("RCU1", &mstp_clks[MSTP105]),	/* RCUA1 */
	CLKDEV_DEV_ID("sh_mobile_rcu.0", &mstp_clks[MSTP107]),	/* RCUA0 */
	CLKDEV_DEV_ID("sh_mobile_rcu.1", &mstp_clks[MSTP105]),	/* RCUA1 */
	CLKDEV_CON_ID("csi21", &mstp_clks[MSTP128]),	/* CSI2-RX1 */
	CLKDEV_CON_ID("csi20", &mstp_clks[MSTP126]),	/* CSI2-RX0 */
	CLKDEV_DEV_ID("sh-mobile-csi2.1", &mstp_clks[MSTP128]),	/* CSI2-RX1 */
	CLKDEV_DEV_ID("sh-mobile-csi2.0", &mstp_clks[MSTP126]),	/* CSI2-RX0 */
};
