/* emmc_private.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */

#ifndef _EMMC_PRIVATE_H_
#define _EMMC_PRIVATE_H_

#include "com_type.h"


/*****************************************************************************
; Register Definition
******************************************************************************/
/*------------------------------------------------------*/
/*	RWTCNT Registers									*/
/*------------------------------------------------------*/
/* write RWTCNT, value is 0x5A5Axxxx, align is 32bit */
#define RWTCNT		(0xE6020000)
#define RWTCNT_W	(volatile ulong*)RWTCNT

/* RWTCNT clear */
#define RWDT_CLEAR	(ulong)(0x5A5A0000)

/*------------------------------------------------------*/
/*	MMCIF Registers										*/
/*------------------------------------------------------*/
#define MMCIF_TOP_ADDR	(0xE6BD0000)
#define CE_CMD_SET		(MMCIF_TOP_ADDR + 0x00)	/* MMC Command */
#define CE_ARG			(MMCIF_TOP_ADDR + 0x08)	/* MMC Command Argument */
#define CE_ARG_CMD12	(MMCIF_TOP_ADDR + 0x0C)	/* CMD12 Argument */
#define CE_CMD_CTRL		(MMCIF_TOP_ADDR + 0x10)	/* Command Control */
#define CE_BLOCK_SET	(MMCIF_TOP_ADDR + 0x14)	/* Block Control */
#define CE_CLK_CTRL		(MMCIF_TOP_ADDR + 0x18)	/* Clock Control */
#define CE_BUF_ACC		(MMCIF_TOP_ADDR + 0x1C)	/* Buffer Access Control */
#define CE_RESP3		(MMCIF_TOP_ADDR + 0x20)	/* RSP[127:96] */
#define CE_RESP2		(MMCIF_TOP_ADDR + 0x24)	/* RSP[95:64]  */
#define CE_RESP1		(MMCIF_TOP_ADDR + 0x28)	/* RSP[63:32]  */
#define CE_RESP0		(MMCIF_TOP_ADDR + 0x2C)	/* RSP[31:0]   */
#define CE_RESP_CMD12	(MMCIF_TOP_ADDR + 0x30)	/* Auto CMD12 Response */
#define CE_DATA			(MMCIF_TOP_ADDR + 0x34)	/* Data  */
#define CE_BOOT			(MMCIF_TOP_ADDR + 0x3C)	/* Boot Operation Config */
#define CE_INT			(MMCIF_TOP_ADDR + 0x40)	/* Interrupt Flag   */
#define CE_INT_MASK		(MMCIF_TOP_ADDR + 0x44)	/* Interrupt Enable */
#define CE_HOST_STS1	(MMCIF_TOP_ADDR + 0x48)	/* Status 1  */
#define CE_HOST_STS2	(MMCIF_TOP_ADDR + 0x4C)	/* Status 2  */
#define CE_VERSION		(MMCIF_TOP_ADDR + 0x7C)	/* Version  */


/*--- CE_CMD_SET ---*/
#define CE_CMD_SET_BOOT			(0x40000000L)
#define CE_CMD_SET_CMD_MASK		(0x3F000000L)
#define CE_CMD_SET_RTYP_MASK	(0x00C00000L)
#define CE_CMD_SET_RBSY			(0x00200000L)
#define CE_CMD_SET_CCSEN		(0x00100000L)
#define CE_CMD_SET_WDAT			(0x00080000L)
#define CE_CMD_SET_DWEN			(0x00040000L)
#define CE_CMD_SET_CMLTE		(0x00020000L)
#define CE_CMD_SET_CMD12EN		(0x00010000L)
#define CE_CMD_SET_RIDXC_MASK	(0x0000C000L)
#define CE_CMD_SET_RCRC7C_MASK	(0x00003000L)
#define CE_CMD_SET_CRC16C		(0x00000400L)
#define CE_CMD_SET_BOOTACK		(0x00000200L)
#define CE_CMD_SET_CRCSTE		(0x00000100L)
#define CE_CMD_SET_TBIT			(0x00000080L)
#define CE_CMD_SET_OPDM			(0x00000040L)
#define CE_CMD_SET_CCSH			(0x00000020L)

#define CE_CMD_SET_DATW_MASK	(0x00000003L)
#define CE_CMD_SET_DATW_1BIT	(0x00000000L)
#define CE_CMD_SET_DATW_4BIT	(0x00000001L)
#define CE_CMD_SET_DATW_8BIT	(0x00000002L)

/*--- CE_INT ---*/
#define CE_INT_CCSDE			(0x20000000L)
#define CE_INT_CMD12DRE			(0x04000000L)
#define CE_INT_CMD12RBE			(0x02000000L)
#define CE_INT_CMD12CRE			(0x01000000L)
#define CE_INT_DTRANE			(0x00800000L)		/* Transfer end */
#define CE_INT_BUFRE			(0x00400000L)		/* Buff read end */
#define CE_INT_BUFWEN			(0x00200000L)		/* Buff write enable */
#define CE_INT_BUFREN			(0x00100000L)		/* Buff read enable */
#define CE_INT_CCSRCV			(0x00080000L)
#define CE_INT_RBSYE			(0x00020000L)		/* Resp Busy end */
#define CE_INT_CRSPE			(0x00010000L)		/* Cmd Resp end */
#define CE_INT_CMDVIO			(0x00008000L)
#define CE_INT_BUFVIO			(0x00004000L)
#define CE_INT_WDATERR			(0x00000800L)
#define CE_INT_RDATERR			(0x00000400L)
#define CE_INT_RIDXERR			(0x00000200L)
#define CE_INT_RSPERR			(0x00000100L)
#define CE_INT_CCSTO			(0x00000020L)
#define CE_INT_CRCSTO			(0x00000010L)
#define CE_INT_WDATTO			(0x00000008L)
#define CE_INT_RDATTO			(0x00000004L)
#define CE_INT_RBSYTO			(0x00000002L)
#define CE_INT_RSPTO			(0x00000001L)

#define CE_INT_CRSPE_CLEAR		(0xFFFEFFFFL)
#define CE_INT_ALL_ERROR		(0x0000FFFFL)

/*--- CE_HOST_STS1 ---*/
#define CE_HOST_STS1_CMDSEQ			(0x80000000L)
#define CE_HOST_STS1_CMDSIG			(0x40000000L)
#define CE_HOST_STS1_RSPIDX_MASK	(0x3F000000L)
#define CE_HOST_STS1_DATSIG_MASK	(0x00FF0000L)
#define CE_HOST_STS1_RCVBLK_MASK	(0x0000FFFFL)

/*--- CE_HOST_STS2 ---*/
#define CE_HOST_STS2_CRCSTE			(0x80000000L)
#define CE_HOST_STS2_CRC16E			(0x40000000L)
#define CE_HOST_STS2_AC12CRCE		(0x20000000L)
#define CE_HOST_STS2_RSPCRC7E		(0x10000000L)
#define CE_HOST_STS2_CRCSTEBE		(0x08000000L)
#define CE_HOST_STS2_RDATEBE		(0x04000000L)
#define CE_HOST_STS2_AC12REBE		(0x02000000L)
#define CE_HOST_STS2_RSPEBE			(0x01000000L)
#define CE_HOST_STS2_AC121DXE		(0x00800000L)
#define CE_HOST_STS2_RSPIDXE		(0x00400000L)
#define CE_HOST_STS2_BTACKPATE		(0x00200000L)
#define CE_HOST_STS2_BTACKEBE		(0x00100000L)
#define CE_HOST_STS2_CRCST_MASK		(0x00070000L)
#define CE_HOST_STS2_STCCSTO		(0x00008000L)
#define CE_HOST_STS2_STRDATTO		(0x00004000L)
#define CE_HOST_STS2_DATBSYTO		(0x00002000L)
#define CE_HOST_STS2_CRCSTTO		(0x00001000L)
#define CE_HOST_STS2_AC12BSYTO		(0x00000800L)
#define CE_HOST_STS2_RSPBSYTO		(0x00000400L)
#define CE_HOST_STS2_AC12RSPTO		(0x00000200L)
#define CE_HOST_STS2_STRSPTO		(0x00000100L)
#define CE_HOST_STS2_BTACKTO		(0x00000080L)
#define CE_HOST_STS2_1STBTDATTO		(0x00000040L)
#define CE_HOST_STS2_BTDATTO		(0x00000020L)

/*--- CE_VERSION ---*/
#define CE_VERSION_SWRST			(0x80000000L)

/*--- CE_CLK_CTRL ---*/
#define CE_CLK_CTRL_CLKEN			(0x01000000L)
#define CE_CLK_CTRL_CLKDIV_MASK		(0x000F0000L)
#define CE_CLK_CTRL_SRSPTO_MASK		(0x00003000L)
#define CE_CLK_CTRL_SRBSYTO_MASK	(0x00000F00L)
#define CE_CLK_CTRL_SRWDTO_MASK		(0x000000F0L)
#define CE_CLK_CTRL_SCCSTO_MASK		(0x0000000FL)
#define CE_CLOCK_ENABLE				CE_CLK_CTRL_CLKEN
#define CE_CLOCK_DISABLE			(0xFEFFFFFFL)

/*--- CE_BOOT ---*/
#define CE_BOOT_BTCLKDIV_MASK		(0xF0000000L)
#define CE_BOOT_SBTACKTO_MASK		(0x0F000000L)
#define CE_BOOT_S1STBTDATTO_MASK	(0x00F00000L)
#define CE_BOOT_SBTDATTO_MASK		(0x000F0000L)

/*--- CE_CMD_CTRL ---*/
#define CE_CMD_CTRL_BREAK			(0x00000001L)

/*--- CE_BUF_ACC ---*/
#define CE_BUF_ACC_SWAP_ENABLE		(0x00010000L)
#define CE_BUF_ACC_SWAP_DISABLE		(0x00000000L)

/*--- CE_CE_BLOCK_SET ---*/
#define CE_BLOCK_SET_SECTOR			(0x00000200L)


/*------------------------------------------------------*/
/* CPG Registers										*/
/*------------------------------------------------------*/
#define SMSTPCR3					(0xE615013CL)
#define SRCR3						(0xE61580B8L)

/*--- Registers Value ---*/
#define SMSTPCR3_MMC_ENABLE			(0xFFFF7FFFL)
#define SMSTPCR3_MMC_DISABLE		(0x00008000L)
#define SRCR3_MMC_ENABLE			(0xFFFF7FFFL)
#define SRCR3_MMC_DISABLE			(0x00008000L)

/*------------------------------------------------------*/
/* GPIO Registers										*/
/*------------------------------------------------------*/
#define GPIO_PORTR319_288DR			(0xE605600CL)
#define GPIO_PORTR319_288DSR		(0xE605610CL)
#define GPIO_PORTR319_288DCR		(0xE605620CL)

#define GPIO_PORT300CR				(0xE605212CL)
#define GPIO_PORT301CR				(0xE605212DL)
#define GPIO_PORT302CR				(0xE605212EL)
#define GPIO_PORT303CR				(0xE605212FL)
#define GPIO_PORT304CR				(0xE6052130L)
#define GPIO_PORT305CR				(0xE6052131L)
#define GPIO_PORT306CR				(0xE6052132L)
#define GPIO_PORT307CR				(0xE6052133L)
#define GPIO_PORT308CR				(0xE6052134L)
#define GPIO_PORT309CR				(0xE6052135L)
#define GPIO_PORT310CR				(0xE6052136L)

#define GPIO_MSEL4CR				(0xE6058024L)	

/*--- Registers Value ---*/

#define PULL_OFF_REG_11		(0x11)		/* Pull U/D off, select function 1 */
#define PULL_UP_REG_C1		(0xC1)		/* Pull UP, select function 1 */
#define PULL_UP_REG_E1		(0xE1)		/* Pull UP, select function 1, IE */
#define PULL_UP_REG_10		(0x10)		/* pull up to A0 or 11 -> 10 */


#define GPIO_MMC_RST_POP		(0x00040000L)	/* PORT306 : RST_n */

#define GPIO_MMC_RST_EXTERNAL	(0x00400000L)	/* PORT310 : RST_n */

/**************************************************************************
; Macro Definition
***************************************************************************/
/*--- 32bit registers ---*/
#define SET_REG32(r, v) (*(volatile unsigned long *)(r) = (v))
#define GET_REG32(r)    (*(volatile unsigned long *)(r))

/*--- 16bit registers ---*/
#define SET_REG16(r, v) (*(volatile unsigned short *)(r) = (v))
#define GET_REG16(r)    (*(volatile unsigned short *)(r))

/*--- 8bit registers ---*/
#define SET_REG8(r, v) (*(volatile unsigned char *)(r) = (v))
#define GET_REG8(r)    (*(volatile unsigned char *)(r))


/*****************************************************************************
; Data Definition
******************************************************************************/
#define EMMC_MAX_CMD_NUMBER			(38)		/* Number of CMD */
#define EMMC_MAX_CSD_LENGTH			(16)		/* CSD byte size */
#define EMMC_MAX_CID_LENGTH			(16)		/* CID byte size */
#define EMMC_MAX_EXT_CSD_LENGTH		(512)		/* EXT_CSD byte size */
#define EMMC_MAX_RESPONSE_LENGTH	(17)		/* CMD Response (R2) byte size */
#define EMMC_MAX_RESP_SHORT_LENGTH	(6)			/* CMD Response (R1,R3,R4,R5) byte size */


/* RCA */
#define EMMC_RCA					(0x0001L)

/* clock config */
#define EMMC_CLK_200KHZ				(0x00082440L)		/* 203kHz,  RspBusy=1291ms, Data=1291ms  */
#define EMMC_CLK_52MHZ				(0x00002CC0L)		/* 52.0MHz, RspBusy=1291ms, Data=1291ms */

/* wait count */
#define EMMC_RESP_TIMEOUT			(1)			/* retry count for respons complete interrupt */
#define EMMC_RESP_BUSY_TIMEOUT		(1500)		/* retry count for respons busy timeout */
#define EMMC_DATA_TIMEOUT			(1500)		/* retry count for transfer data */
#define EMMC_POWER_UP_WAIT			(90)		/* wait for power up setting */
#define CMD1_RETRY_COUNT			(5000)		/* retry count for CMD1 */
#define CMD13_RETRY_COUNT			(50)		/* retry count for response busy timeout */
#define ERROR_RESET_RETRY_COUNT		(50000)		/* retry count for error reset */
#define EMMC_TIMEOUT_COUNT			(202000)	/* retry count for software timer */
#define EMMC_INT_TIMEOUT_COUNT		(35000)		/* retry count for wait interrupt */


/* status flags */
#define EMMC_ACCESS_MODE_BYTE		(0)
#define EMMC_ACCESS_MODE_SECTOR		(1)

/* OCR */
#define EMMC_HOST_OCR_VALUE			(0xC0000080L)
#define EMMC_OCR_STATUS_BIT			(0x80000000L)	/* Card power up status bit */
#define EMMC_OCR_ACCESS_MODE_MASK	(0x60000000L)
#define EMMC_OCR_ACCESS_MODE_SECT	(0x40000000L)
#define EMMC_OCR_ACCESS_MODE_BYTE	(0x00000000L)

/* Card Status (R1) */
#define EMMC_R1_ERROR_MASK			(0xfd3fe080L)		/* Type 'E' bit and bit14(must be 0). ignore bit22 and bit23  */ 
#define EMMC_R1_STATE_MASK			(0x00001E00L)		/* [12:9] */
#define EMMC_R1_READY				(0x00000100L)		/* bit8 */
#define EMMC_R1_STATE_SHIFT			(9)

#define EMMC_R1_STATE_IDLE			(0)
#define EMMC_R1_STATE_READY			(1)
#define EMMC_R1_STATE_IDENT			(2)
#define EMMC_R1_STATE_STBY			(3)
#define EMMC_R1_STATE_TRAN			(4)
#define EMMC_R1_STATE_DATA			(5)
#define EMMC_R1_STATE_RCV			(6)
#define EMMC_R1_STATE_PRG			(7)
#define EMMC_R1_STATE_DIS			(8)
#define EMMC_R1_STATE_BTST			(9)


/* R4 */
#define EMMC_R4_RCA_MASK			(0xFFFF0000L)
#define EMMC_R4_STATUS				(0x00008000L)

/* CSD */
/* SPEC_VERS (CSD[125-122]) bit positiion */
#define EMMC_CSD_SPEC_VERS_BYTE		(15)
#define EMMC_CSD_SPEC_VERS_SHIFT	(2)
#define EMMC_CSD_SPEC_VERS_MASK		(0x0F)


/* EXT_CSD */
#define EMMC_EXT_CSD_S_CMD_SET				(504)
#define EMMC_EXT_CSD_INI_TIMEOUT_AP			(241)
#define EMMC_EXT_CSD_PWR_CL_DDR_52_360		(239)
#define EMMC_EXT_CSD_PWR_CL_DDR_52_195		(238)
#define EMMC_EXT_CSD_MIN_PERF_DDR_W_8_52	(235)
#define EMMC_EXT_CSD_MIN_PERF_DDR_R_8_52	(234)
#define EMMC_EXT_CSD_TRIM_MULT				(232)
#define EMMC_EXT_CSD_SEC_FEATURE_SUPPORT	(231)
#define EMMC_EXT_CSD_SEC_ERASE_MULT			(229)
#define EMMC_EXT_CSD_BOOT_INFO				(228)
#define EMMC_EXT_CSD_BOOT_SIZE_MULTI		(226)
#define EMMC_EXT_CSD_ACC_SIZE				(225)
#define EMMC_EXT_CSD_HC_ERASE_GRP_SIZE		(224)
#define EMMC_EXT_CSD_ERASE_TIMEOUT_MULT		(223)
#define EMMC_EXT_CSD_PEL_WR_SEC_C			(222)
#define EMMC_EXT_CSD_HC_WP_GRP_SIZE			(221)
#define EMMC_EXT_CSD_S_C_VCC				(220)
#define EMMC_EXT_CSD_S_C_VCCQ				(219)
#define EMMC_EXT_CSD_S_A_TIMEOUT			(217)
#define EMMC_EXT_CSD_SEC_COUNT				(212)
#define EMMC_EXT_CSD_MIN_PERF_W_8_52		(210)
#define EMMC_EXT_CSD_MIN_PERF_R_8_52		(209)
#define EMMC_EXT_CSD_MIN_PERF_W_8_26_4_52	(208)
#define EMMC_EXT_CSD_MIN_PERF_R_8_26_4_52	(207)
#define EMMC_EXT_CSD_MIN_PERF_W_4_26		(206)
#define EMMC_EXT_CSD_MIN_PERF_R_4_26		(205)
#define EMMC_EXT_CSD_PWR_CL_26_360			(203)
#define EMMC_EXT_CSD_PWR_CL_52_360			(202)
#define EMMC_EXT_CSD_PWR_CL_26_195			(201)
#define EMMC_EXT_CSD_PWR_CL_52_195			(200)
#define EMMC_EXT_CSD_CARD_TYPE				(196)
#define EMMC_EXT_CSD_CSD_STRUCTURE			(194)
#define EMMC_EXT_CSD_EXT_CSD_REV			(192)
#define EMMC_EXT_CSD_CMD_SET				(191)
#define EMMC_EXT_CSD_CMD_SET_REV			(189)
#define EMMC_EXT_CSD_POWER_CLASS			(187)
#define EMMC_EXT_CSD_HS_TIMING				(185)
#define EMMC_EXT_CSD_BUS_WIDTH				(183)
#define EMMC_EXT_CSD_ERASED_MEM_CONT		(181)
#define EMMC_EXT_CSD_PARTITION_CONFIG		(179)
#define EMMC_EXT_CSD_BOOT_CONFIG_PROT		(178)
#define EMMC_EXT_CSD_BOOT_BUS_WIDTH			(177)
#define EMMC_EXT_CSD_ERASE_GROUP_DEF		(175)
#define EMMC_EXT_CSD_BOOT_WP				(173)
#define EMMC_EXT_CSD_USER_WP				(171)
#define EMMC_EXT_CSD_FW_CONFIG				(169)
#define EMMC_EXT_CSD_RPMB_SIZE_MULT			(168)
#define EMMC_EXT_CSD_RST_n_FUNCTION			(162)
#define EMMC_EXT_CSD_PARTITIONING_SUPPORT	(160)
#define EMMC_EXT_CSD_MAX_ENH_SIZE_MULT		(159)
#define EMMC_EXT_CSD_PARTITIONS_ATTRIBUTE	(156)
#define EMMC_EXT_CSD_PARTITION_SETTING_COMPLETED	(155)
#define EMMC_EXT_CSD_GP_SIZE_MULT			(154)
#define EMMC_EXT_CSD_ENH_SIZE_MULT			(142)
#define EMMC_EXT_CSD_ENH_START_ADDR			(139)
#define EMMC_EXT_CSD_SEC_BAD_BLK_MGMNT		(134)

#define EMMC_EXT_CSD_CARD_TYPE_26MHZ			(0x01)
#define EMMC_EXT_CSD_CARD_TYPE_52MHZ			(0x02)
#define EMMC_EXT_CSD_CARD_TYPE_DDR_52MHZ_12V	(0x04)
#define EMMC_EXT_CSD_CARD_TYPE_DDR_52MHZ_18V	(0x08)
#define EMMC_EXT_CSD_CARD_TYPE_52MHZ_MASK		(0x0e)

/* SWITCH (CMD6) argument */
#define EMMC_SWITCH_HS_TIMING			(0x03b90100L)	/* HS_TIMING = 0x01 */
#define EMMC_SWITCH_BUS_WIDTH_8			(0x03B70200L)	/* BUS_WIDTH = 0x02 */
#define EMMC_SWITCH_BUS_WIDTH_4			(0x03B70100L)	/* BUS_WIDTH = 0x01 */
#define EMMC_SWITCH_BUS_WIDTH_1			(0x03B70000L)	/* BUS_WIDTH = 0x00 */

/* Bus width */
#define EMMC_BUSWIDTH_1BIT			CE_CMD_SET_DATW_1BIT
#define EMMC_BUSWIDTH_4BIT			CE_CMD_SET_DATW_4BIT
#define EMMC_BUSWIDTH_8BIT			CE_CMD_SET_DATW_8BIT

#define BOOT_CONFIG_PARTITION_CONFIG	((uchar)0x48)
#define BOOT_CONFIG_BOOT_BUS_WIDTH		((uchar)0x02)
#define BOOT_CONFIG_RST_n_FUNCTION		((uchar)0x01)


typedef enum {
	EMMC_CMD_TYPE_BC = 0,	/* CMD type (bc) */
	EMMC_CMD_TYPE_BCR,		/* CMD type (bcr) */
	EMMC_CMD_TYPE_AC,		/* CMD type (ac) */
	EMMC_CMD_TYPE_ADTC,		/* CMD type (adtc) */
	EMMC_CMD_TYPE_MAX		/* The maximum of defined value */
} EMMC_CMD_TYPE;

typedef enum {
	EMMC_CMD_RESP_NON = 0,	/* No response */
	EMMC_CMD_RESP_R1,		/* R1 */
	EMMC_CMD_RESP_R1B,		/* R1b */
	EMMC_CMD_RESP_R2,		/* R2 */
	EMMC_CMD_RESP_R3,		/* R3 */
	EMMC_CMD_RESP_R4,		/* R4 */
	EMMC_CMD_RESP_R5,		/* R5 */
	EMMC_CMD_RESP_MAX		/* The maximum of defined value */
} EMMC_CMD_RESP;

typedef enum {
	EMMC_DATA_DIR_NONE = 0,	/* n/a */
	EMMC_DATA_DIR_READ,		/* Read (Card --> Host) */
	EMMC_DATA_DIR_READBT,	/* Read (byte access) */
	EMMC_DATA_DIR_WRITE,	/* Write (Host --> Card) */
	EMMC_DATA_DIR_WRITEBT,	/* Write (byte access) */
	EMMC_DATA_DIR_MAX		/* The maximum of defined value */
} EMMC_DATA_DIR;

typedef struct {
	EMMC_CMD_TYPE type;		/* CMD type */
	EMMC_CMD_RESP resp;		/* CMD Response */
	EMMC_DATA_DIR dir;		/* Direction */
	ulong hw;				/* setting value */
} EMMC_CMD_SPEC;

typedef struct {
	ulong cmd;				/* CMD No. */
	EMMC_CMD_TYPE type;		/* type */
	EMMC_CMD_RESP rsp;		/* response type */
	EMMC_DATA_DIR dir;		/* direction */
	ulong hw;				/* H/W depend */
	ulong arg;				/* CMD argument */
	uchar *pBuff;			/* trans data buffer */
	uint64 req_length;		/* trans request byte size */
	uint64 remain_length;	/* remain byte size */
	ulong *pRsp;			/* response buffer */
	ulong rsp_length;		/* response byte size */
} EMMC_CMD_INFO;

typedef struct {
	/* eMMC device size */
	ulong sec_count;	/* sector count of user data partition */
	
	/* bus width */
	ulong bus_width;	/* current bus width */
	
	/* clock */
	ulong freq; /* request freq [kHz] */
	ulong current_freq; /* current MMC clock[kHz] */

	/* state flag */
	ulong access_mode; /* TRUE : sector access, FALSE : byte access */
	ulong during_transfer; /* TRUE : during transfer */
	ulong current_state; /* current card status */

	/* response data */
	ulong r1_card_status; /* card status for R1 */
	ulong r3_ocr; /* ocr for R3 */
	ulong r4_resp; /* R4 */
	ulong r5_resp; /* R5 */

	/* Card register buffer(4byte align) */
	uchar csd_buf[EMMC_MAX_CSD_LENGTH]; /* CSD buffer */
	uchar cid_buf[EMMC_MAX_CID_LENGTH]; /* CID buffer */
	uchar ext_csd_buf[EMMC_MAX_EXT_CSD_LENGTH]; /* EXT_CSD buffer */
	uchar response_buf[EMMC_MAX_RESPONSE_LENGTH]; /* response bufer */
} EMMC_DRV_INFO;


/*****************************************************************************
; Function definition
******************************************************************************/
extern RC emmc_Go_Idle_State(void);
extern RC emmc_Go_Transfer_State(void);
extern RC emmc_Set_Ext_Csd(ulong cmd_arg);
extern RC emmc_Issue_Cmd(void);
extern RC emmc_Exec_Cmd(void);
extern void emmc_Make_Cmd(ulong cmd, ulong cmd_arg, ulong *pBuff, uint64 length);
extern RC emmc_Trans_Data(ulong *pBuff, ulong count);
extern RC emmc_Trans_Byte(ulong *pBuff, ulong length);
extern void emmc_Error_Reset(void);
extern RC emmc_Wait_Int(ulong event, ulong loop);

extern RC emmc_Clock_Enable(void);
extern RC emmc_Wait_Ready_For_Data(void);
extern void emmc_Wait(ulong loop);
extern RC emmc_Send_Stop(void);
extern RC emmc_Check_Param_Multi(uchar* pBuff, ulong start_sector, ulong sector_count);
extern RC emmc_Check_Param_Single(uchar* pBuff, ulong start_sector);


#endif /* _EMMC_PRIVATE_H_ */
