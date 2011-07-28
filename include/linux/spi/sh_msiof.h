#ifndef __SPI_SH_MSIOF_H__
#define __SPI_SH_MSIOF_H__

struct sh_msiof_spi_cs_info {
	s32 cs_port;
};

struct sh_msiof_spi_info {
	int tx_fifo_override;
	int rx_fifo_override;
	u16 num_chipselect;
};

#endif /* __SPI_SH_MSIOF_H__ */
