/*
 * eos_common.h
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
#ifndef __H_EOS_COMMON_
#define __H_EOS_COMMON_

/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/

/***************************************************************/
/* PUBLIC CONSTANT DEFINITON                                   */
/***************************************************************/
/* REGESTER READ/WRITE MACRO */
#define _SET_MEM8( adr, dat )	(*(VU1 *)adr = (u1)dat)		/* byte write */
#define _SET_MEM16( adr, dat )	(*(VU2 *)adr = (u2)dat)		/* word write */
#define _SET_MEM32( adr, dat )	(*(VU4 *)adr = (u4)dat)		/* long-word write */
#define _GET_MEM8( adr )		(*(VU1 *)adr)				/* byte read */
#define _GET_MEM16( adr )		(*(VU2 *)adr)				/* word read */
#define _GET_MEM32( adr )		(*(VU4 *)adr)				/* long-word read */

#define U1S_EOS_MODE_NORMAL		(u1)( 0x00 )
#define U1S_EOS_MODE_PWR_DEBUG	(u1)( 0x01 )
#define U1S_EOS_MODE_DEBUG		(u1)( 0x02 )

/***************************************************************/
/* PUBLIC TYPEDEFE ENUM                                        */
/***************************************************************/

/***************************************************************/
/* PUBLIC TYPEDEFE                                             */
/***************************************************************/
typedef struct {
	u1  u1_lsi_int_ver;				
	u1  u1_lsi_dec_ver;				
	u1  u1_tp_int_ver;				
	u1  u1_tp_dec_ver;				
	u1  u1_tp_code_ver;				
} ST_EVM_VER;

/***************************************************************/
/* PUBLIC VARIABLE EXTERN                                      */
/***************************************************************/
/* EVM VERSION */
extern ST_EVM_VER stg_evm_ver;
extern u1 u1g_eos_mode;

/***************************************************************/
/* PUBLIC FUNCTION EXTERN                                      */
/***************************************************************/
extern void monprintf( const char *format,...);
extern void vog_ver_code_get( ST_EVM_VER* ptt_evm_ver );
extern u4 u4g_mystrtoul( const char * ,char ** ,int base );
extern u4 u4g_strtol(const char* s,char** endp,int base);

#endif  /* __H_EOS_COMMON_ */
