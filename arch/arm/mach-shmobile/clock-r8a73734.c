#include <mach/clock-definitions-ES2.h>

void __init r8a73734_clock_init(void)
{
	/* detect System-CPU clock parent */
	if (__raw_readl(PLLECR) & (1 << 8)) {	/* PLL0ST */
		es2_div4_clks[ES2_DIV4_Z].parent = &pll0_clk;

		if (__raw_readl(FRQCRB) & (1 << 28))	/* ZSEL */
			z_clk.parent = &es2_div4_clks[ES2_DIV4_Z];
		else
			z_clk.parent = &pll0_clk;
	} else {
		es2_div4_clks[ES2_DIV4_Z].parent = &main_clk;
		z_clk.parent = &main_clk;
	}

	/* check MPCLK for errata */
	if (__raw_readl(MPCKCR) & (1 << 7))
		es2_mpc_clk.parent = &es2_mp_clk;
	else
		es2_mp_clk.flags = 0;

	clk_register(&extal1_clk);
	clk_register(&extal2_clk);
	clk_register(&extalr_clk);

	clk_register(&extal1_div2_clk);
	clk_register(&extal2_div2_clk);
	clk_register(&extal2_div4_clk);

	if(((system_rev & 0xFFFF)>>4) >= 0x3E1)
	{
		clk_register(&es2_fsiack_clk);
		clk_register(&es2_fsibck_clk);
	}

	sh_clk_cksel_register(&main_clk, 1);
	clk_register(&main_div2_clk);

	sh_clk_cksel_register(&pll2_cksel_clk, 1);

	sh_clk_cksel_register(&pll22_cksel_clk, 1);

	clk_register(&pll0_clk);
	clk_register(&pll1_clk);
	clk_register(&pll1_div2_clk);
	clk_register(&pll1_div7_clk);
	clk_register(&pll1_div13_clk);
	clk_register(&pll2_clk);

	clk_register(&pll22_clk);

	clk_register(&pll3_clk);

	clk_register(&z_clk);

	/* common divider following pll1 and pll3 */
	sh_clk_div4_register(es2_div4_clks, ES2_DIV4_NR,
				     &common_div4_table);
	/* siblings of common divider following pll1 */

	clk_register(&es2_ztr_clk);

	/* siblings of common divider following pll3 */
	sh_clk_cksel_register(&es2_ddr_clk, 1);

	clk_register(&es2_zb30_clk);	/* ZB3 Implementation not needed, ZB30=ZB3 */

	clk_register(&es2_ddr_div2_clk);
	clk_register(&es2_ddr_div4_clk);

	sh_clk_cksel_register(&es2_zb30sl_clk, 1);

	/* dedicated dividers */
	sh_clk_cksel_register(&vclk3_cksel_clk, 1);
	clk_register(&vclk3_pdiv_clk);
	sh_clk_div6_reparent_register(es2_div6_clks, ES2_DIV6_NR);
	clk_register(&es2_m4d2_clk);

	clk_register(&extcki_clk);
	sh_clk_cksel_register(&dsi0p0_cksel_clk, 1);
	sh_clk_cksel_register(&dsi0p1_cksel_clk, 1);
	clk_register(&dsi0p0_clk);
	clk_register(&dsi0p1_clk);
	clk_register(&dsi0p_clk);

	sh_clk_cksel_register(&dsi1p0_cksel_clk, 1);
	sh_clk_cksel_register(&dsi1p1_cksel_clk, 1);
	clk_register(&dsi1p0_clk);
	clk_register(&dsi1p1_clk);
	clk_register(&dsi1p_clk);
	clk_register(&es2_slimb_clk);

	sh_clk_cksel_register(&es2_mp_clk, 1);

	sh_clk_cksel_register(&es2_mpc_clk, 1);
	/* Sound add */

	sh_clk_cksel_register(&es2_mpmp_clk, 1);
	/* Sound add */
	sh_clk_cksel_register(&es2_fsia_clk, 1);
	sh_clk_cksel_register(&es2_fsib_clk, 1);
	sh_clk_cksel_register(&es2_spua_clk, 1);
	sh_clk_cksel_register(&es2_spuv_clk, 1);

	sh_clk_cksel_register(&cp_clk, 1);

	clk_set_parent(&dsi0p_clk, NULL);

	sh_clk_mstp32_register(es2_mstp_clks, ES2_MSTP_NR);

	clkdev_add_table(es2_lookups, ARRAY_SIZE(es2_lookups));

	clk_init();
};
