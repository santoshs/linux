#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/sh_clk.h>
#include <asm/clkdev.h>
#include <mach/common.h>     

#define	FRQCRA	0xE6150000UL
#define	FRQCRB	0xE6150004UL
#define	FRQCRD	0xE61500E4UL
#define	VCLKCR1	0xE6150008UL
#define	VCLKCR2	0xE615000CUL
#define	VCLKCR3	0xE6150014UL
#define	VCLKCR4	0xE615001CUL
#define	ZBCKCR	0xE6150010UL
#define	SD0CKCR	0xE6150074UL
#define	SD1CKCR	0xE6150078UL
#define	SD2CKCR	0xE615007CUL
#define	FSIACKCR	0xE6150018UL
#define	FSIBCKCR	0xE6150090UL
#define	MPCKCR	0xE6150080UL
#define	SPUACKCR	0xE6150084UL
#define	HSICKCR	0xE615008CUL
#define	DSITCKCR	0xE6150060UL
#define	DSI0PCKCR	0xE6150064UL
#define	DSI0PHYCR	0xE615006CUL
#define	MPMODE	0xE61500CCUL
#define	RTSTBCR	0xE6150020UL
#define	PLLECR	0xE61500D0UL
#define	PLL0CR	0xE61500D8UL
#define	PLL1CR	0xE6150028UL
#define	PLL2CR	0xE615002CUL
#define	PLL3CR	0xE61500DCUL
#define	PLL0STPCR	0xE61500F0UL
#define	PLL1STPCR	0xE61500C8UL
#define	PLL2STPCR	0xE61500F8UL
#define	PLL3STPCR	0xE61500FCUL
#define	MSTPSR0	0xE6150030UL
#define	MSTPSR1	0xE6150038UL
#define	MSTPSR2	0xE6150040UL
#define	MSTPSR3	0xE6150048UL
#define	MSTPSR4	0xE615004CUL
#define	MSTPSR5	0xE615003CUL
#define	RMSTPCR0	0xE6150110UL
#define	RMSTPCR1	0xE6150114UL
#define	RMSTPCR2	0xE6150118UL
#define	RMSTPCR3	0xE615011CUL
#define	RMSTPCR4	0xE6150120UL
#define	RMSTPCR5	0xE6150124UL
#define	SMSTPCR0	0xE6150130UL
#define	SMSTPCR1	0xE6150134UL
#define	SMSTPCR2	0xE6150138UL
#define	SMSTPCR3	0xE615013CUL
#define	SMSTPCR4	0xE6150140UL
#define	SMSTPCR5	0xE6150144UL
#define	MMSTPCR0	0xE6150150UL
#define	MMSTPCR1	0xE6150154UL
#define	MMSTPCR2	0xE6150158UL
#define	MMSTPCR3	0xE615015CUL
#define	MMSTPCR4	0xE6150160UL
#define	MMSTPCR5	0xE6150164UL
#define	SRCR0	0xE61580A0UL
#define	SRCR1	0xE61580A8UL
#define	SRCR2	0xE61580B0UL
#define	SRCR3	0xE61580B8UL
#define	SRCR4	0xE61580BCUL
#define	SRCR5	0xE61580C4UL
#define	ASTAT	0xE6150054UL
#define	CKSCR	0xE61500C0UL
#define	SEQMON	0xE6150108UL
#define	VREFCR	0xE61500ECUL
#define	WUPCR	0xE6151010UL
#define	SRESCR	0xE6151018UL
#define	PCLKCR	0xE6151020UL
#define	PSTR	0xE6151040UL
#define	CPU0RFR	0xE6151104UL
#define	CPU1RFR	0xE6151114UL
#define	SPCTR	0xE61501A4UL
#define	SPCMMR	0xE61501ACUL
#define	SPCDMR	0xE61501B0UL

#define EXSTMON2	0xe6180088UL

static int extal2_clk_enable(struct clk *clk)
{
	__raw_writel(__raw_readl(PLL2CR) & ~1, PLL2CR);
	while (__raw_readl(EXSTMON2) & (1 << 16)) /* EX2MSK */
		cpu_relax();
	return 0;
}

static void extal2_clk_disable(struct clk *clk)
{
	__raw_writel(__raw_readl(PLL2CR) | 1, PLL2CR); /* XOE */
	/*
	 * Make sure that EX2MSK bit gets set here in response to XOE bit
	 * set to '1' (Stop XTAL2 oscillation) above.   This is needed for
	 * safety in case that EXTAL2 clock gets enabled right after it's
	 * disabled.
	 */
	while (!(__raw_readl(EXSTMON2) & (1 << 16))) /* EX2MSK */
		cpu_relax();
}

static struct clk_ops extal2_clk_ops = {
	.enable		= extal2_clk_enable,
	.disable	= extal2_clk_disable,
};

/* Externals */
static struct clk extal1_clk = {
	.rate	= 26*1000*1000,
};

static struct clk extal2_clk = {
	.rate	= 48*1000*1000,
	.ops	= &extal2_clk_ops,
};

static struct clk extalr_clk = {
	.rate	= 32768,
};

static struct clk fsiack_clk = {};

static struct clk fsibck_clk = {};

/* Constant divider */
static unsigned long const_div_recalc(struct clk *clk)
{
	return clk->parent->rate / (int)(clk->priv);
}

static struct clk_ops const_div_clk_ops = {
	.recalc	= const_div_recalc,
};

static struct clk extal1_div2_clk = {
	.parent	= &extal1_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

static struct clk extal2_div2_clk = {
	.parent	= &extal2_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

static struct clk extal2_div4_clk = {
	.parent	= &extal2_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)4,
};

static struct clk *main_clk_parent[] = {
	[0]	= &extal1_clk,
	[1]	= &extal1_div2_clk,
	[2]	= &extal2_clk,
	[3]	= &extal2_div2_clk,
};

static struct clk main_clk = SH_CLK_CKSEL(NULL, CKSCR, 0, 0,
	main_clk_parent, ARRAY_SIZE(main_clk_parent), 28, 2);

static struct clk main_div2_clk = {
	.parent	= &main_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

/* PLL0 */
static unsigned int zfc_divisors[16] = {
	[0]	= 2,
	[1]	= 3,
	[2]	= 4,
	[3]	= 6,
	[4]	= 8,
	[5]	= 12,
	[6]	= 16,
	[8]	= 24,
	[11]	= 48,
};

/* pll0 + sys-cpu divider */
/* Just follows HW setting. No modify, no kick.  */
static unsigned long pll0_recalc(struct clk *clk)
{
	unsigned long rate = clk->parent->rate;
	u32 pll0cr = __raw_readl(PLL0CR);
	int pll0control = (pll0cr >> (8 + 0)) & 1;
	/* pll0 */
	if (pll0control)
		rate *= ((pll0cr >> 24) & 0x3f) + 1;
	/* sys-cpu divider */
	if (pll0control) {
		u32 frqcrb = __raw_readl(FRQCRB);
		if (frqcrb & (1 << 28)) { /* ZSEL */
			int div = zfc_divisors[(frqcrb >> 24) & 0xf];
			if (div)
				rate /= div;
			else
				rate = 0; /* warn */
		}
	} else
		rate = clk->parent->rate;
	return rate;
}

static struct clk_ops pll0_clk_ops = {
	.recalc	= pll0_recalc,
};

static struct clk pll0_clk = {
	.parent	= &main_clk,
	.ops	= &pll0_clk_ops,
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
		if (pll1cr & (1 << 20)) /* CFG */
			rate *= 2;
		if (pll1cr & (1 << 7)) /* CKSEL */
			rate /= 2;
	}
	return rate;
}

static struct clk_ops pll1_clk_ops = {
	.recalc	= pll1_recalc,
};

static struct clk pll1_clk = {
	.parent	= &main_clk,
	.ops	= &pll1_clk_ops,
};

static struct clk pll1_div2_clk = {
	.parent	= &pll1_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

static struct clk pll1_div7_clk = {
	.parent	= &pll1_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)7,
};

static struct clk pll1_div13_clk = {
	.parent	= &pll1_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)13,
};

/* PLL2 */
/* Ops for pll2, pll22, and pll3 */
static unsigned long pll2223_recalc(struct clk *clk)
{
	unsigned long rate = clk->parent->rate;
	u32 pll2223cr = __raw_readl(clk->enable_reg);
	if (__raw_readl(PLLECR) & (1 << (8 + clk->enable_bit))) {
		rate *= ((pll2223cr >> 24) & 0x3f) + 1; /* STC */
		if (pll2223cr & (1 << 20)) /* CFG for PLL2,22 but 3 */
			rate *= 2;
	}
	return rate;
}

static struct clk_ops pll2223_clk_ops = {
	.recalc	= pll2223_recalc,
};

/* parent table for both pll2 and pll22 */
static struct clk *pll2_parent[] = {
	[0]	= &main_div2_clk,
	[1]	= &extal2_div2_clk,
	[3]	= &extal2_div4_clk,
	[4]	= &main_clk,
	[5]	= &extal2_clk,
};

static struct clk pll2_cksel_clk = SH_CLK_CKSEL(NULL, PLL2CR, 0, 0,
	pll2_parent, ARRAY_SIZE(pll2_parent), 5, 3);

static struct clk pll2_clk = {
	.parent		= &pll2_cksel_clk,
	.ops		= &pll2223_clk_ops,
	.enable_reg	= (void *__iomem)PLL2CR,
	.enable_bit	= 2, /* bit in PLLECR */
};

#if 0
/* PLL22 */
/* This shares its enable bit with PLL2 */
/* This has same set of parent as PLL2 but can be controlled independently */

static struct clk pll22_cksel_clk = SH_CLK_CKSEL(NULL, PLL22CR,
	pll2_parent, ARRAY_SIZE(pll2_parent), 5, 3);

static struct clk pll22_clk = {
	.parent		= &pll22_cksel_clk,
	.ops		= &pll2223_clk_ops,
	.enable_reg	= PCC22CR,
	.enable_bit	= 2, /* bit in PLLECR */
};
#endif

/* PLL3 */
static struct clk pll3_clk = {
	.parent		= &main_clk,
	.ops		= &pll2223_clk_ops,
	.enable_reg	= (void *__iomem)PLL3CR,
	.enable_bit	= 3, /* bit in PLLECR */
};

/* Common divider */
static void common_kick(struct clk *clk)
{
	__raw_writel(__raw_readl(FRQCRB) | (1 << 31), FRQCRB);
}

static unsigned int common_divisors[] = {
	[0]	= 2,
	[1]	= 3,
	[2]	= 4,
	[3]	= 6,
	[4]	= 8,
	[5]	= 12,
	[6]	= 16,
	[7]	= 18,
	[8]	= 24,
	[10]	= 36,
	[11]	= 48,
	[12]	= 7,
};

static struct clk_div_mult_table common_div_mult_table = {
	.divisors	= common_divisors,
	.nr_divisors	= ARRAY_SIZE(common_divisors),
};

static struct clk_div4_table common_div4_table = {
	.div_mult_table	= &common_div_mult_table,
	.kick		= common_kick,
};

enum {
	DIV4_I, DIV4_ZG, DIV4_M3, DIV4_B, DIV4_M1,
	DIV4_ZTR, DIV4_ZT, DIV4_ZX, DIV4_ZS, DIV4_HP,
	DIV4_DDR,
	DIV4_NR
};

static struct clk div4_clks[DIV4_NR] = {
	[DIV4_I] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 20, 0xdff, 0),
	[DIV4_ZG] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 16, 0x97f, 0),
	[DIV4_M3] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 12, 0x1dff, 0),
	[DIV4_B] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 8, 0xdff, 0),
	[DIV4_M1] = SH_CLK_DIV4(&pll1_clk, FRQCRA, 4, 0x1dff, 0),
	[DIV4_ZTR] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 20, 0x0dff, 0),
	[DIV4_ZT] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 16, 0xdff, 0),
	[DIV4_ZX] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 12, 0xdff, 0),
	[DIV4_ZS] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 8, 0xdff, 0),
	[DIV4_HP] = SH_CLK_DIV4(&pll1_clk, FRQCRB, 4, 0xdff, 0),
	[DIV4_DDR] = SH_CLK_DIV4(&pll3_clk, FRQCRD, 0, 0x97f, 0),
};

static struct clk ztr_clk = {
	.parent	= &div4_clks[DIV4_ZTR],
	.ops	= &const_div_clk_ops,
	.priv	= (void *)1,
};

static struct clk ztrd2_clk = {
	.parent	= &div4_clks[DIV4_ZTR],
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

static struct clk *ddr_parent[] = {
	[0]	= &pll3_clk,
	[1]	= &div4_clks[DIV4_DDR],
};

static struct clk ddr_clk = SH_CLK_CKSEL(NULL, FRQCRD, 0, 0,
	ddr_parent, ARRAY_SIZE(ddr_parent), 4, 1);

static struct clk zb30_clk = {
	.parent	= &ddr_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

static struct clk zb30d2_clk = {
	.parent	= &ddr_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)4,
};

static struct clk ddr_div2_clk = {
	.parent	= &ddr_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)2,
};

static struct clk ddr_div4_clk = {
	.parent	= &ddr_clk,
	.ops	= &const_div_clk_ops,
	.priv	= (void *)4,
};

static struct clk *zb30sl_parent[] = {
	[0]	= &ddr_div2_clk,
	[1]	= &ddr_div4_clk,
};

static struct clk zb30sl_clk = SH_CLK_CKSEL(NULL, FRQCRD, 0, 0,
	zb30sl_parent, ARRAY_SIZE(zb30sl_parent), 15, 1);

/* DIV6s */
static struct clk *div6_two_parent[] = {
	[0]	= &pll1_div2_clk,
	[1]	= &pll2_clk,
};

static struct clk *sd_parent[] = {
	[0]	= &pll1_div2_clk,
	[1]	= &pll2_clk,
	[2]	= &pll1_div13_clk,
};

static struct clk *hsi_parent[] = {
	[0]	= &pll1_div2_clk,
	[1]	= &pll2_clk,
	[2]	= &pll1_div7_clk,
};

static struct clk *vclk_parent[] = {
	[0]	= &pll1_div2_clk,
	[1]	= &pll2_clk,
	[3]	= &extal2_clk,
	[4]	= &main_div2_clk,
	[5]	= &extalr_clk,
	[6]	= &main_clk,
};

static struct clk vclk3_cksel_clk = SH_CLK_CKSEL(NULL, VCLKCR3, 0, 0,
	vclk_parent, ARRAY_SIZE(vclk_parent), 5, 3);

/* Dynamic divider. Only for recalc. No setting capable. */
static unsigned long div_recalc(struct clk *clk)
{
	return clk->parent->rate /
		((__raw_readl(clk->enable_reg) >> clk->src_shift
			& ((1 << clk->src_width) - 1))
		 + 1);
}

static struct clk_ops div_clk_ops = {
	.recalc	= div_recalc,
};

/* pre divider followed by vclk3 */
static struct clk vclk3_pdiv_clk = {
	.parent		= &vclk3_cksel_clk,
	.ops		= &div_clk_ops,
	.enable_reg	= (void * __iomem)VCLKCR3,
	.src_shift	= 6,
	.src_width	= 2,
};

enum {
	DIV6_MP,
	DIV6_ZB,
	DIV6_SD0,
	DIV6_SD1,
	DIV6_VCLKCR1,
	DIV6_VCLKCR2,
	DIV6_VCLKCR3,
	DIV6_VCLKCR4,
	DIV6_FSIA,
	DIV6_FSIB,
	DIV6_SPUA,
	DIV6_HSI,
	DIV6_NR
};

static struct clk div6_clks[DIV6_NR] = {
	[DIV6_MP] = SH_CLK_DIV6_EXT(NULL, MPCKCR, 9, CLK_DIV_SHARED,
			div6_two_parent, ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_ZB] = SH_CLK_DIV6_EXT(NULL, ZBCKCR, 8, 0,
			div6_two_parent, ARRAY_SIZE(div6_two_parent), 7, 1),
	[DIV6_SD0] = SH_CLK_DIV6_EXT(NULL, SD0CKCR, 8, 0,
			sd_parent, ARRAY_SIZE(sd_parent), 6, 2),
	[DIV6_SD1] = SH_CLK_DIV6_EXT(NULL, SD1CKCR, 8, 0,
			sd_parent, ARRAY_SIZE(sd_parent), 6, 2),
	[DIV6_VCLKCR1] = SH_CLK_DIV6_EXT(NULL, VCLKCR1, 8, 0,
			vclk_parent, ARRAY_SIZE(vclk_parent), 12, 3),
	[DIV6_VCLKCR2] = SH_CLK_DIV6_EXT(NULL, VCLKCR2, 8, 0,
			vclk_parent, ARRAY_SIZE(vclk_parent), 12, 3),
	[DIV6_VCLKCR3] = SH_CLK_DIV6_EXT(&vclk3_pdiv_clk, VCLKCR3, 8, 0,
			NULL, 0, 0, 0),
	[DIV6_VCLKCR4] = SH_CLK_DIV6_EXT(NULL, VCLKCR4, 8, 0,
			vclk_parent, ARRAY_SIZE(vclk_parent), 12, 3),
	[DIV6_FSIA] = SH_CLK_DIV6_EXT(NULL, FSIACKCR, 8, 0,
			div6_two_parent, ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_FSIB] = SH_CLK_DIV6_EXT(NULL, FSIBCKCR, 8, 0,
			div6_two_parent, ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_SPUA] = SH_CLK_DIV6_EXT(NULL, SPUACKCR, 8, 0,
			div6_two_parent, ARRAY_SIZE(div6_two_parent), 6, 1),
	[DIV6_HSI] = SH_CLK_DIV6_EXT(NULL, HSICKCR, 8, 0,
			hsi_parent, ARRAY_SIZE(hsi_parent), 6, 2),
};

static struct clk *mp_parent[] = {
	[0]	= &div6_clks[DIV6_MP],
	[1]	= &extal2_clk,
};

static struct clk mp_clk = SH_CLK_CKSEL(NULL, MPCKCR, 9, CLK_CKSEL_CKSTP,
	mp_parent, ARRAY_SIZE(mp_parent), 7, 1);

static struct clk mpc_clk = SH_CLK_CKSEL(NULL, MPCKCR, 10, CLK_CKSEL_CKSTP,
	mp_parent, ARRAY_SIZE(mp_parent), 7, 1);

static struct clk *fsia_parent[] = {
	[0]	= &div6_clks[DIV6_FSIA],
	[1]	= &fsiack_clk,
};

static struct clk fsia_clk = SH_CLK_CKSEL(NULL, FSIACKCR, 0, 0,
	fsia_parent, ARRAY_SIZE(fsia_parent), 7, 1);

static struct clk *fsib_parent[] = {
	[0]	= &div6_clks[DIV6_FSIB],
	[1]	= &fsibck_clk,
};

static struct clk fsib_clk = SH_CLK_CKSEL(NULL, FSIBCKCR, 0, 0,
	fsib_parent, ARRAY_SIZE(fsib_parent), 7, 1);

static struct clk *spua_parent[] = {
	[0]	= &div6_clks[DIV6_SPUA],
	[1]	= &extal2_clk,
};

static struct clk spua_clk = SH_CLK_CKSEL(NULL, SPUACKCR, 0, 0,
	spua_parent, ARRAY_SIZE(spua_parent), 7, 1);

/* DSI0P
 * HW clock topology is like
 *  {parents}[SELMON0? EXSRC : EXSRCB] / (SELMON0? DIV : DIVB)
 * But here implemented as
 *  SELMON0? ({parents}[EXSRC] / DIV) : ({parents}[EXSRCB] / DIVB)
 */
static struct clk extcki_clk = {};

static struct clk *dsi0p_parent[] = {
	[0]	= &pll1_div2_clk,
	[1]	= &pll2_clk,
	[2]	= &main_clk,
	[3]	= &extal2_clk,
	[4]	= &extcki_clk,
};

static struct clk dsi0p0_cksel_clk = SH_CLK_CKSEL(NULL, DSI0PCKCR,
	8, CLK_CKSEL_CKSTP, dsi0p_parent, ARRAY_SIZE(dsi0p_parent), 24, 3);

static struct clk dsi0p0_clk = {
	.parent		= &dsi0p0_cksel_clk,
	.ops		= &div_clk_ops,
	.enable_reg	= (void * __iomem)DSI0PCKCR,
	.src_shift	= 16,
	.src_width	= 6,
};

static struct clk dsi0p1_cksel_clk = SH_CLK_CKSEL(NULL, DSI0PCKCR,
	8, CLK_CKSEL_CKSTP, dsi0p_parent, ARRAY_SIZE(dsi0p_parent), 12, 3);

static struct clk dsi0p1_clk = {
	.parent		= &dsi0p1_cksel_clk,
	.ops		= &div_clk_ops,
	.enable_reg	= (void * __iomem)DSI0PCKCR,
	.src_shift	= 0,
	.src_width	= 6,
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
	__raw_writel(__raw_readl(clk->enable_reg) | (1 << clk->enable_bit),
		clk->enable_reg);
	return 0;
}

static void selmon_disable(struct clk *clk)
{
	__raw_writel(__raw_readl(clk->enable_reg) & ~(1 << clk->enable_bit),
		clk->enable_reg);
}

static struct clk_ops selmon_clk_ops = {
	.recalc		= followparent_recalc,
	.set_parent	= selmon_reparent,
	.enable		= selmon_enable,
	.disable	= selmon_disable,
};

static struct clk *selmon0_parent[2] = {
	[0]	= &dsi0p0_clk,
	[1]	= &dsi0p1_clk,
};

static struct clk dsi0p_clk = {
	.ops		= &selmon_clk_ops,
	.enable_reg	= (void *__iomem)DSI0PCKCR,
	.enable_bit	= 8,
	.parent_table	= selmon0_parent,
	.src_shift	= 27,
	.src_width	= 1,
};

static struct clk hdmi0_clk = {
	.ops		= &selmon_clk_ops,
	.enable_reg	= (void *__iomem)DSI0PCKCR,
	.enable_bit	= 9,
	.parent_table	= selmon0_parent,
	.src_shift	= 27,
	.src_width	= 1,
};

static struct clk *cp_parent[] = {
	[0]	= &main_div2_clk,
	[1]	= &extalr_clk,
};

static struct clk cp_clk = SH_CLK_CKSEL(&main_div2_clk, 0, 0, 0,
		cp_parent, ARRAY_SIZE(cp_parent), 0, 0);

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

#define DIV4_ 0	/* XXX fill all of these */

static struct clk mstp_clks[MSTP_NR] = {
	[MSTP031] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 31, 0), /* RT-CPU TLB */
	[MSTP030] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 30, 0), /* RT-CPU Instruction Cache (IC) */
	[MSTP029] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 29, 0), /* RT-CPU Operand Cache (OC) */
	[MSTP026] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 26, 0), /* RT-CPU X/Y memory */
	[MSTP022] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 22, 0), /* INTCS */
	[MSTP021] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR0, MSTPSR0, 21, 0), /* RT-DMAC */
	[MSTP019] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 19, 0), /* H-UDI */
	[MSTP018] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 18, 0), /* RT-CPU debug module 1 */
	[MSTP017] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 17, 0), /* UBC */
	[MSTP016] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 16, 0), /* RT-CPU debug module 2 */
	[MSTP015] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 15, 0), /* ILRAM */
	[MSTP007] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 7, 0), /* ICB */
	[MSTP002] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR0, MSTPSR0, 2, 0), /* test module. */
	[MSTP001] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR0, MSTPSR0, 1, 0), /* IIC2 */
	[MSTP000] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR0, MSTPSR0, 0, 0), /* MSIOF0 */
	[MSTP130] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 30, 0), /* VIO60 */
	[MSTP126] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR1, MSTPSR1, 26, 0), /* CSI2-RX0 */
	[MSTP125] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR1, MSTPSR1, 25, 0), /* TMU0 */
	[MSTP124] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_B], SMSTPCR1, MSTPSR1, 24, 0), /* CMT0 */
	[MSTP122] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 22, 0), /* TSG */
	[MSTP119] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 19, 0), /* ISP */
	[MSTP118] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M3], SMSTPCR1, MSTPSR1, 18, 0), /* DSI-TX0 */
	[MSTP116] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR1, MSTPSR1, 16, 0), /* IIC0 */
	[MSTP115] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_B], SMSTPCR1, MSTPSR1, 15, 0), /* 2D-DMAC */
	[MSTP113] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR1, MSTPSR1, 13, 0), /* MERAM */
	[MSTP112] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZG], SMSTPCR1, MSTPSR1, 12, 0), /* SGX543 */
	[MSTP111] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR1, MSTPSR1, 11, 0), /* TMU1 */
	[MSTP108] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 8, 0), /* TSIF0 */
	[MSTP107] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR1, MSTPSR1, 7, 0), /* RCUA */
	[MSTP106] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_B], SMSTPCR1, MSTPSR1, 6, 0), /* JPU */
	[MSTP101] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M1], SMSTPCR1, MSTPSR1, 1, 0), /* VPU */
	[MSTP100] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M3], SMSTPCR1, MSTPSR1, 0, 0), /* LCDC0 */
	[MSTP229] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 29, 0), /* Crypt1 */
	[MSTP228] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 28, 0), /* Crypt */
	[MSTP224] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 24, 0), /* CLKGEN */
	[MSTP223] = SH_CLK_MSTP32_EXT(&spua_clk, SMSTPCR2, MSTPSR2, 23, 0), /* SPU2A */
	[MSTP220] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 20, 0), /* SPU2V */
	[MSTP218] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 18, 0), /* SY-DMAC */
	[MSTP216] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR2, MSTPSR2, 16, 0), /* EFRAM */
	[MSTP215] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 15, 0), /* MSIOF3 */
	[MSTP214] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 14, 0), /* USB-DMAC */
	[MSTP213] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR2, MSTPSR2, 13, 0), /* MFIS */
	[MSTP208] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 8, 0), /* MSIOF1 */
	[MSTP207] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 7, 0), /* SCIFA5 */
	[MSTP206] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 6, 0), /* SCIFB */
	[MSTP205] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 5, 0), /* MSIOF2 */
	[MSTP204] = SH_CLK_MSTP32_EXT(&mpc_clk, SMSTPCR2, MSTPSR2, 4, 0), /* SCIFA0 */
	[MSTP203] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 3, 0), /* SCIFA1 */
	[MSTP202] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 2, 0), /* SCIFA2 */
	[MSTP201] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR2, MSTPSR2, 1, 0), /* SCIFA3 */
	[MSTP330] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 30, 0), /* MSU */
	[MSTP329] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR3, MSTPSR3, 29, 0), /* CMT1 */
	[MSTP328] = SH_CLK_MSTP32_EXT(&fsia_clk, SMSTPCR3, MSTPSR3, 28, 0), /* FSI */
	[MSTP326] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 26, 0), /* SCUW */
	[MSTP325] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_HSI], SMSTPCR3, MSTPSR3, 25, 0), /* HSI1 */
	[MSTP324] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_HSI], SMSTPCR3, MSTPSR3, 24, 0), /* HSI0 */
	[MSTP323] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 23, 0), /* IIC1 */
	[MSTP322] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 22, 0), /* USB */
	[MSTP321] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 21, 0), /* SBSC performance monitor */
	[MSTP319] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 19, 0), /* RT-DMAC scheduler */
	[MSTP316] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_ZS], SMSTPCR3, MSTPSR3, 16, 0), /* SHWYSTAT */
	[MSTP315] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 15, 0), /* MMC0 */
	[MSTP314] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_SD0], SMSTPCR3, MSTPSR3, 14, 0), /* SDHI0 */
	[MSTP313] = SH_CLK_MSTP32_EXT(&div6_clks[DIV6_SD1], SMSTPCR3, MSTPSR3, 13, 0), /* SDHI1 */
	[MSTP309] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR3, MSTPSR3, 9, 0), /* ICU-USB */
	[MSTP308] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR3, MSTPSR3, 8, 0), /* MFIR */
	[MSTP307] = SH_CLK_MSTP32_EXT(&mpc_clk, SMSTPCR3, MSTPSR3, 7, 0), /* ICU-DMAC1 */
	[MSTP306] = SH_CLK_MSTP32_EXT(&mpc_clk, SMSTPCR3, MSTPSR3, 6, 0), /* ICU-DMAC0 */
	[MSTP305] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR3, MSTPSR3, 5, 0), /* MMC1 */
	[MSTP304] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_M1], SMSTPCR3, MSTPSR3, 4, 0), /* TPU0 */
	[MSTP415] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR4, MSTPSR4, 15, 0), /* GPRMP */
	[MSTP412] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR4, MSTPSR4, 12, 0), /* IICMS */
	[MSTP410] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR4, MSTPSR4, 10, 0), /* IIC4 */
	[MSTP409] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR4, MSTPSR4, 9, 0), /* IIC5 */
	[MSTP407] = SH_CLK_MSTP32_EXT(&mp_clk, SMSTPCR4, MSTPSR4, 7, 0), /* USB-DMAC1 */
	[MSTP406] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR4, MSTPSR4, 6, 0), /* DDM */
	[MSTP403] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR4, MSTPSR4, 3, 0), /* KEYSC */
	[MSTP402] = SH_CLK_MSTP32_EXT(&cp_clk, SMSTPCR4, MSTPSR4, 2, 0), /* RWDT0 */
	[MSTP530] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 30, 0), /* Secure ROM */
	[MSTP529] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 29, 0), /* Secure RAM */
	[MSTP528] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 28, 0), /* Internal RAM1 */
	[MSTP527] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 27, 0), /* Internal RAM0 */
	[MSTP526] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 26, 0), /* Public ROM */
	[MSTP525] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 25, 0), /* IICB0 */
	[MSTP519] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 19, 0), /* O2S */
	[MSTP518] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 18, 0), /* O2M */
	[MSTP517] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 17, 0), /* S2O1 */
	[MSTP516] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_], SMSTPCR5, MSTPSR5, 16, 0), /* S2O0 */
	[MSTP508] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 8, 0), /* INTCM */
	[MSTP507] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 7, 0), /* IRQC */
	[MSTP501] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 1, 0), /* SPU2A Core1 */
	[MSTP500] = SH_CLK_MSTP32_EXT(&div4_clks[DIV4_HP], SMSTPCR5, MSTPSR5, 0, 0), /* SPU2A Core0 */
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
	CLKDEV_CON_ID("dsi0p0_clk", &dsi0p0_clk),
	CLKDEV_CON_ID("dsi0p1_clk", &dsi0p1_clk),
	CLKDEV_CON_ID("fsiack_clk", &fsiack_clk),
	CLKDEV_CON_ID("fsibck_clk", &fsibck_clk),

	/* reparent candidates for peripherals */
	CLKDEV_CON_ID("rclk_clk", &extalr_clk),
	CLKDEV_CON_ID("i_clk", &div4_clks[DIV4_I]),
	CLKDEV_CON_ID("b_clk", &div4_clks[DIV4_B]),
	CLKDEV_CON_ID("z_clk", &pll0_clk),
	CLKDEV_CON_ID("zx_clk", &div4_clks[DIV4_ZX]),
	CLKDEV_CON_ID("zt_clk", &div4_clks[DIV4_ZT]),
	CLKDEV_CON_ID("ztr_clk", &ztr_clk),
	CLKDEV_CON_ID("zs_clk", &div4_clks[DIV4_ZS]),
	CLKDEV_CON_ID("hp_clk", &div4_clks[DIV4_HP]),
	CLKDEV_CON_ID("cp_clk", &cp_clk),
	CLKDEV_CON_ID("m1_clk", &div4_clks[DIV4_M1]),
	CLKDEV_CON_ID("mpc_clk", &mpc_clk),
	CLKDEV_CON_ID("m3_clk", &div4_clks[DIV4_M3]),
	CLKDEV_CON_ID("mp_clk", &mp_clk),

	/* stray clocks */
	CLKDEV_CON_ID("ztrd2_clk", &ztrd2_clk),
	CLKDEV_CON_ID("zb30_clk", &zb30_clk),
	CLKDEV_CON_ID("zb30d2_clk", &zb30d2_clk),

	CLKDEV_DEV_ID("sh_cmt.0", &mstp_clks[MSTP329]), /* CMT1 */
	CLKDEV_DEV_ID("sh_cmt.1", &mstp_clks[MSTP329]), /* CMT1 */
	CLKDEV_DEV_ID("sh-sci.0", &mstp_clks[MSTP204]), /* SCIFA0 */
	CLKDEV_DEV_ID("sh-sci.1", &mstp_clks[MSTP203]), /* SCIFA1 */
	CLKDEV_DEV_ID("sh-sci.2", &mstp_clks[MSTP202]), /* SCIFA2 */
	CLKDEV_DEV_ID("sh-sci.3", &mstp_clks[MSTP201]), /* SCIFA3 */
};

void __init r8a73734_clock_init(void)
{
	clk_register(&extal1_clk);
	clk_register(&extal2_clk);
	clk_register(&extalr_clk);

	clk_register(&extal1_div2_clk);
	clk_register(&extal2_div2_clk);
	clk_register(&extal2_div4_clk);

	clk_register(&fsiack_clk);
	clk_register(&fsibck_clk);

	sh_clk_cksel_register(&main_clk, 1);
	clk_register(&main_div2_clk);

	sh_clk_cksel_register(&pll2_cksel_clk, 1);
	/*sh_clk_cksel_register(&pll22_cksel_clk, 1);*/

	clk_register(&pll0_clk);
	clk_register(&pll1_clk);
	clk_register(&pll1_div2_clk);
	clk_register(&pll1_div7_clk);
	clk_register(&pll1_div13_clk);
	clk_register(&pll2_clk);
/*	clk_register(&pll22_clk);*/
	clk_register(&pll3_clk);

	/* common divider following pll1 and pll3 */
	sh_clk_div4_register(div4_clks, DIV4_NR, &common_div4_table);
	/* siblings of common divider following pll1 */
	clk_register(&ztr_clk);
	clk_register(&ztrd2_clk);

	/* siblings of common divider following pll3 */
	sh_clk_cksel_register(&ddr_clk, 1);
	clk_register(&zb30_clk);
	clk_register(&zb30d2_clk);
	clk_register(&ddr_div2_clk);
	clk_register(&ddr_div4_clk);
	sh_clk_cksel_register(&zb30sl_clk, 1);

	/* dedicated dividers */
	sh_clk_cksel_register(&vclk3_cksel_clk, 1);
	clk_register(&vclk3_pdiv_clk);
	sh_clk_div6_reparent_register(div6_clks, DIV6_NR);

	clk_register(&extcki_clk);
	sh_clk_cksel_register(&dsi0p0_cksel_clk, 1);
	sh_clk_cksel_register(&dsi0p1_cksel_clk, 1);
	clk_register(&dsi0p0_clk);
	clk_register(&dsi0p1_clk);
	clk_register(&dsi0p_clk);
	clk_register(&hdmi0_clk);

	sh_clk_cksel_register(&mp_clk, 1);
	sh_clk_cksel_register(&mpc_clk, 1);
	sh_clk_cksel_register(&fsia_clk, 1);
	sh_clk_cksel_register(&fsib_clk, 1);
	sh_clk_cksel_register(&spua_clk, 1);
	sh_clk_cksel_register(&cp_clk, 1);

	clk_set_parent(&dsi0p_clk, NULL);
	clk_set_parent(&hdmi0_clk, NULL);

	sh_clk_mstp32_register(mstp_clks, MSTP_NR);

	clkdev_add_table(lookups, ARRAY_SIZE(lookups));

	clk_init();
};
