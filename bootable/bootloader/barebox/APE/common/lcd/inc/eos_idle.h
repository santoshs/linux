/*
 * eos_idle.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_EOS_IDLE_
#define __H_EOS_IDLE_

/**************************************************/
/* INCLUDE FILE                                   */
/**************************************************/
#include "eos_system.h"

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
#define kArgMax			10		
#define kCmdMax			24
#define U2G_KIN_DEV0_BUFF_MAX	(u2)( 4096 )/* kInDev0BuffMax	4096 */
#define U2G_KOFFSET_MAX			(u2)( U2G_KIN_DEV0_BUFF_MAX - 2048 )/*	kOffsetMax		(U2G_KIN_DEV0_BUFF_MAX - 2048)*/
#define U1G_KXON				(u1)( 0x11 )	
#define U1G_KXOFF				(u1)( 0x13 )   
#define kESC			0x1B
#define kCTRLC			0x03
#define kBS				0x08
#define kCmdLineBuffMax	150		
#define U1G_CMD_LINE_MAX		(u1)( 5 )	

#if defined(CW_EXPERIMENT)
	#define	kCR		'\n'
#else
	#define	kCR		0x0D
#endif

#define FLAG_ARROW		0x5b
#define UP_ARROW		0x41
#define DOWN_ARROW		0x42
#define RIGHT_ARROW		0x43
#define LEFT_ARROW		0x44

#define ARROW_READY		0x00
#define ARROW_ESC		0x01
#define ARROW_SELECT	0x02
#define UP_ARROW_READY	0x00
#define UP_ARROW_SELECT 0x01

#if 0
#define kArgMax			10		
#define kCmdMax			24
#define kOffsetMax		(kInDev0BuffMax - 2048)
#define kXon			0x11	
#define kXoff			0x13	
#define kESC			0x1B
#define kCTRLC			0x03
#define kBS				0x08
#define kCmdLineBuffMax	150		
#define kCmdLineMax		5		
#define kInDev0BuffMax	4096

#if defined(CW_EXPERIMENT)
	#define	kCR		'\n'
#else
	#define	kCR		0x0D
#endif

#define FLAG_ARROW		0x5b
#define UP_ARROW		0x41
#define DOWN_ARROW		0x42
#define RIGHT_ARROW		0x43
#define LEFT_ARROW		0x44

#define ARROW_READY		0x00
#define ARROW_ESC		0x01
#define ARROW_SELECT	0x02
#define UP_ARROW_READY	0x00
#define UP_ARROW_SELECT 0x01
#endif

/***************************************************************/
/* PUBLIC TYPEDEFE ENUM                                        */
/***************************************************************/

/***************************************************************/
/* PUBLIC TYPEDEFE                                             */
/***************************************************************/
typedef enum
{
	kNORM,
	kGO
}EN_MODE;


typedef	struct CmdLineType
{
	u1 u1_buff[kCmdLineBuffMax];			
	u1 u1_valid;							
}ST_CMD_LINE_TYPE;
/* CmdLineType; */

typedef struct ConsoleType
{
	u1*	              pt_prompt;				
	ST_CMD_LINE_TYPE  st_cmd_line[U1G_CMD_LINE_MAX];	
	ST_CMD_LINE_TYPE* pt_base;					
	u1                u1_echo;					
	u1                u1_mode;					
                                                
}ST_CONSOLE_TYPE;

typedef struct CmdTblType {
	u1   u1_cmd[8];							
	void (*Function)();						
	u1   u1_attribute;						
}ST_CMD_TBL_TYPE;
/* CmdTblType; */

typedef struct CtrlTblType {
	u1   u1_ctrl[4];
	void (*Function)();
	u1   u1_attribute;
}ST_CTRL_TBL_TYPE;

typedef struct PastCmdTable {
	u1 u1_cmd_length;			
	u1 u1_pre_cmd[20];			
	u1 u1_valid;				
}ST_PAST_CMD_TBL;

typedef struct BuffType {					
	u1 u1_data;							
	u1 u1_valid;							
}ST_BUFF_TYPE;
/* BuffType; */

typedef struct InDev0BuffType {
	ST_BUFF_TYPE  st_buff[U2G_KIN_DEV0_BUFF_MAX];	
	ST_BUFF_TYPE* pt_eff_buff_write;			
	ST_BUFF_TYPE* pt_eff_buff_read;			
	u1 u1_xoff;							
	u1 u1_stop;						
} ST_IN_DEV0_BUFF_TYPE;




#if 0
typedef enum
{
	kNORM,
	kGO
}EN_MODE;

/* CmdLine Type data */
typedef	struct CmdLineType
{
	char	Buff[kCmdLineBuffMax];			
	char	Valid;							
}ST_CMD_LINE_TYPE;

typedef struct ConsoleType
{
	char	*Prompt;						
	struct	CmdLineType	CmdLine[kCmdLineMax];
											
	struct	CmdLineType	*base;				
	char	echo;							
	char	Mode;							
}ST_CONSOLE_TYPE;

typedef struct CmdTblType {
	char	Cmd[8];							
	void	(*Function)();					
	char	Attribute;						
}ST_CMD_TBL_TYPE;

typedef struct CtrlTblType {
	char	Ctrl[4];
	void	(*Function)();
	char	Attribute;
} CtrlTblType;
#endif


#if 0
typedef struct PastCmdTable {
	int 			nCmdLength;				
	unsigned char 	ucPreCmd[20];			
	int				fValid;					
} PAST_CMD_TBL;
#endif

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/
extern ST_CONSOLE_TYPE stg_console; 				

extern u1 u1g_pre_cmd[30][50];
/* Input TP command total(MAX30) */
extern u1 u1g_cmd_cnt;

extern u1 u1g_cmd_max;

extern u1 u1g_cmd_pt;

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/

extern void GpioPortScan(int ,char** );
extern void vog_gpio_port_io( int argc ,char** argv );
extern void i2c_command( int argc, char** argv );

extern void reg_io( int argc, char** argv );

extern void vog_mode_change( int argc, char** argv );
extern void vog_shipment_test(int,char**);
extern void vog_help(int,char**);
extern void vog_version_code( int argc, char **argv );



extern void vog_up_arrow( ST_CONSOLE_TYPE* ptt_console );
extern void vog_lo_arrow( ST_CONSOLE_TYPE* ptt_console );
extern void vog_le_arrow( ST_CONSOLE_TYPE* ptt_console );

extern u1 u1g_monwrite( u1 u1t_file_no, u1* ptt_buff, u1 u1t_write_num );
extern u1 u1g_mongets( u1* ptt_buff );

#endif  /* __H_EOS_IDLE_ */
