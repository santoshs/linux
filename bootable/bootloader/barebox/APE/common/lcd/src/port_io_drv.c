/*
 * port_io_drv.c
 *
 * Copyright (C) 2012 Renesas Mobile Corp.
 * All rights reserved.
 *
 */
/***************************************************************/
/* INCLUDE FILE                                                */
/***************************************************************/
#include "eos_csw.h"					/* EOS CSW Header file */


#include "eos_stdio.h"					/* EOS-Standard Header */
#include "port_io_drv.h"				/* port io header file */

/***************************************************************/
/* STATIC TYPEDEF                                              */
/***************************************************************/

/***************************************************************/
/* STATIC VARIABLE                                             */
/***************************************************************/

/***************************************************************/
/* STATIC FUNCTION PROTOTYPE                                   */
/***************************************************************/

/***************************************************************/
/* PUBLIC VARIABLE PROTOTYPE                                   */
/***************************************************************/

/*****************************************************************************************/
/* ST_PORT_INFO stg_scan_port_tbl[]                                                      */
/*---------------------------------------------------------------------------------------*/
/* OUTLINE : GPIO PORT DATA TABLE for PORT COMMAND                                       */
/*---------------------------------------------------------------------------------------*/
/* table deteil :                                                                       */
/*  u2 u2_port_num        PORT num                                                       */
/*  u4 u4_portcr_addr     PORT CR Addr                                                   */
/*  u4 u4_portdr_addr     PORT Data reg Addr                                             */
/*  u1 u1_portdr_bit      PORT Reg Bit Num(LSB0-MSB31)                                   */
/*  u1 u1_port_io         PORT I/O (U1S_PORT_RW(0)=I/O U1S_PORT_R(1)=I U1S_PORT_W(2)=O ) */
/*---------------------------------------------------------------------------------------*/
/* Author: MMS                                                                           */
/* Date  : 2011.11.28 new                                                                */
/*****************************************************************************************/
static ST_PORT_INFO sts_scan_port_tbl[] = 
{
	{ 0  , 0xE6050000, 0xE6054000,  0, U1G_PORT_RW },	/* PORT0CR   ini:H'A0 */
	{ 1  , 0xE6050001, 0xE6054000,  1, U1G_PORT_RW },	/* PORT1CR   ini:H'A0 */
	{ 2  , 0xE6050002, 0xE6054000,  2, U1G_PORT_RW },	/* PORT2CR   ini:H'A0 */
	{ 3  , 0xE6050003, 0xE6054000,  3, U1G_PORT_RW },	/* PORT3CR   ini:H'A0 */
	{ 4  , 0xE6050004, 0xE6054000,  4, U1G_PORT_RW },	/* PORT4CR   ini:H'A0 */
	{ 5  , 0xE6050005, 0xE6054000,  5, U1G_PORT_RW },	/* PORT5CR   ini:H'A0 */
	{ 6  , 0xE6050006, 0xE6054000,  6, U1G_PORT_RW },	/* PORT6CR   ini:H'A0 */
	{ 7  , 0xE6050007, 0xE6054000,  7, U1G_PORT_RW },	/* PORT7CR   ini:H'A0 */
	{ 8  , 0xE6050008, 0xE6054000,  8, U1G_PORT_RW },	/* PORT8CR   ini:H'A0 */
	{ 9  , 0xE6050009, 0xE6054000,  9, U1G_PORT_RW },	/* PORT9CR   ini:H'A0 */
	{ 10 , 0xE605000A, 0xE6054000, 10, U1G_PORT_RW },	/* PORT10CR  ini:H'A0 */
	{ 11 , 0xE605000B, 0xE6054000, 11, U1G_PORT_RW },	/* PORT11CR  ini:H'A0 */
	{ 12 , 0xE605000C, 0xE6054000, 12, U1G_PORT_RW },	/* PORT12CR  ini:H'A0 */
	{ 13 , 0xE605000D, 0xE6054000, 13, U1G_PORT_RW },	/* PORT13CR  ini:H'A0 */
	{ 14 , 0xE605000E, 0xE6054000, 14, U1G_PORT_RW },	/* PORT14CR  ini:H'A0 */
	{ 15 , 0xE605000F, 0xE6054000, 15, U1G_PORT_RW },	/* PORT15CR  ini:H'A0 */
	{ 16 , 0xE6050010, 0xE6054000, 16, U1G_PORT_RW },	/* PORT16CR  ini:H'A0 */
	{ 17 , 0xE6050011, 0xE6054000, 17, U1G_PORT_RW },	/* PORT17CR  ini:H'A0 */
	{ 18 , 0xE6050012, 0xE6054000, 18, U1G_PORT_RW },	/* PORT18CR  ini:H'A0 */
	{ 19 , 0xE6050013, 0xE6054000, 19, U1G_PORT_RW },	/* PORT19CR  ini:H'A0 */
	{ 20 , 0xE6050014, 0xE6054000, 20, U1G_PORT_RW },	/* PORT20CR  ini:H'A0 */
	{ 21 , 0xE6050015, 0xE6054000, 21, U1G_PORT_RW },	/* PORT21CR  ini:H'A0 */
	{ 22 , 0xE6050016, 0xE6054000, 22, U1G_PORT_RW },	/* PORT22CR  ini:H'A0 */
	{ 23 , 0xE6050017, 0xE6054000, 23, U1G_PORT_RW },	/* PORT23CR  ini:H'A0 */
	{ 24 , 0xE6050018, 0xE6054000, 24, U1G_PORT_RW },	/* PORT24CR  ini:H'A0 */
	{ 25 , 0xE6050019, 0xE6054000, 25, U1G_PORT_RW },	/* PORT25CR  ini:H'A0 */
	{ 26 , 0xE605001A, 0xE6054000, 26, U1G_PORT_RW },	/* PORT26CR  ini:H'A0 */
	{ 27 , 0xE605001B, 0xE6054000, 27, U1G_PORT_RW },	/* PORT27CR  ini:H'A0 */
	{ 28 , 0xE605001C, 0xE6054000, 28, U1G_PORT_RW },	/* PORT28CR  ini:H'A0 */
	{ 29 , 0xE605001D, 0xE6054000, 29, U1G_PORT_RW },	/* PORT29CR  ini:H'A0 */
	{ 30 , 0xE605001E, 0xE6054000, 30, U1G_PORT_RW },	/* PORT30CR  ini:H'A0 */
	{ 31 , 0xE605001F, 0xE6054000, 31, U1G_PORT_RW },	/* PORT31CR  ini:H'A0 */
	
	{ 32 , 0xE6050020, 0xE6054004,  0, U1G_PORT_RW },	/* PORT32CR  ini:H'A0 */
	{ 33 , 0xE6050021, 0xE6054004,  1, U1G_PORT_RW },	/* PORT33CR  ini:H'A0 */
	{ 34 , 0xE6050022, 0xE6054004,  2, U1G_PORT_RW },	/* PORT34CR  ini:H'A0 */
	{ 35 , 0xE6050023, 0xE6054004,  3, U1G_PORT_RW },	/* PORT35CR  ini:H'A0 */
	{ 36 , 0xE6050024, 0xE6054004,  4, U1G_PORT_RW },	/* PORT36CR  ini:H'A0 */
	{ 37 , 0xE6050025, 0xE6054004,  5, U1G_PORT_RW },	/* PORT37CR  ini:H'A0 */
	{ 38 , 0xE6050026, 0xE6054004,  6, U1G_PORT_RW },	/* PORT38CR  ini:H'A0 */
	{ 39 , 0xE6050027, 0xE6054004,  7, U1G_PORT_RW },	/* PORT39CR  ini:H'A0 */
	{ 40 , 0xE6050028, 0xE6054004,  8, U1G_PORT_RW },	/* PORT40CR  ini:H'80 */
	{ 41 , 0xE6050029, 0xE6054004,  9, U1G_PORT_RW },	/* PORT41CR  ini:H'80 */
	{ 42 , 0xE605002A, 0xE6054004, 10, U1G_PORT_RW },	/* PORT42CR  ini:H'80 */
	{ 43 , 0xE605002B, 0xE6054004, 11, U1G_PORT_RW },	/* PORT43CR  ini:H'80 */
	{ 44 , 0xE605002C, 0xE6054004, 12, U1G_PORT_RW },	/* PORT44CR  ini:H'A0 */
	{ 45 , 0xE605002D, 0xE6054004, 13, U1G_PORT_RW },	/* PORT45CR  ini:H'A0 */
	{ 46 , 0xE605002E, 0xE6054004, 14, U1G_PORT_RW },	/* PORT46CR  ini:H'A0 */
	{ 47 , 0xE605002F, 0xE6054004, 15, U1G_PORT_RW },	/* PORT47CR  ini:H'A0 */
	{ 48 , 0xE6050030, 0xE6054004, 16, U1G_PORT_RW },	/* PORT48CR  ini:H'A0 */
	
	{ 64 , 0xE6050040, 0xE6054008,  0, U1G_PORT_RW },	/* PORT64CR  ini:H'11 */
	{ 65 , 0xE6050041, 0xE6054008,  1, U1G_PORT_RW },	/* PORT65CR  ini:H'11 */
	{ 66 , 0xE6050042, 0xE6054008,  2, U1G_PORT_RW },	/* PORT66CR  ini:H'A1 */
	{ 67 , 0xE6050043, 0xE6054008,  3, U1G_PORT_RW },	/* PORT67CR  ini:H'11 */
	{ 68 , 0xE6050044, 0xE6054008,  4, U1G_PORT_RW },	/* PORT68CR  ini:H'11 */
	{ 69 , 0xE6050045, 0xE6054008,  5, U1G_PORT_RW },	/* PORT69CR  ini:H'A1 */
	{ 70 , 0xE6050046, 0xE6054008,  6, U1G_PORT_RW },	/* PORT70CR  ini:H'A1 */
	{ 71 , 0xE6050047, 0xE6054008,  7, U1G_PORT_RW },	/* PORT71CR  ini:H'A1 */
	{ 72 , 0xE6050048, 0xE6054008,  8, U1G_PORT_RW },	/* PORT72CR  ini:H'A0 */
	{ 73 , 0xE6050049, 0xE6054008,  9, U1G_PORT_RW },	/* PORT73CR  ini:H'A0 */
	{ 74 , 0xE605004A, 0xE6054008, 10, U1G_PORT_RW },	/* PORT74CR  ini:H'A0 */
	{ 75 , 0xE605004B, 0xE6054008, 11, U1G_PORT_RW },	/* PORT75CR  ini:H'A0 */
	{ 76 , 0xE605004C, 0xE6054008, 12, U1G_PORT_RW },	/* PORT76CR  ini:H'A0 */
	{ 77 , 0xE605004D, 0xE6054008, 13, U1G_PORT_RW },	/* PORT77CR  ini:H'A0 */
	{ 78 , 0xE605004E, 0xE6054008, 14, U1G_PORT_RW },	/* PORT78CR  ini:H'A0 */
	{ 79 , 0xE605004F, 0xE6054008, 15, U1G_PORT_RW },	/* PORT79CR  ini:H'A0 */
	{ 80 , 0xE6050050, 0xE6054008, 16, U1G_PORT_RW },	/* PORT80CR  ini:H'80 */
	{ 81 , 0xE6050051, 0xE6054008, 17, U1G_PORT_RW },	/* PORT81CR  ini:H'80 */
	{ 82 , 0xE6050052, 0xE6054008, 18, U1G_PORT_RW },	/* PORT82CR  ini:H'80 */
	{ 83 , 0xE6050053, 0xE6054008, 19, U1G_PORT_RW },	/* PORT83CR  ini:H'80 */
	{ 84 , 0xE6050054, 0xE6054008, 20, U1G_PORT_RW },	/* PORT84CR  ini:H'80 */
	{ 85 , 0xE6050055, 0xE6054008, 21, U1G_PORT_RW },	/* PORT85CR  ini:H'80 */
	{ 86 , 0xE6050056, 0xE6054008, 22, U1G_PORT_RW },	/* PORT86CR  ini:H'80 */
	{ 87 , 0xE6050057, 0xE6054008, 23, U1G_PORT_RW },	/* PORT87CR  ini:H'80 */
	{ 88 , 0xE6050058, 0xE6054008, 24, U1G_PORT_RW },	/* PORT88CR  ini:H'80 */
	{ 89 , 0xE6050059, 0xE6054008, 25, U1G_PORT_RW },	/* PORT89CR  ini:H'80 */
	{ 90 , 0xE605005A, 0xE6054008, 26, U1G_PORT_RW },	/* PORT90CR  ini:H'80 */
	{ 91 , 0xE605005B, 0xE6054008, 27, U1G_PORT_RW },	/* PORT91CR  ini:H'80 */
	
	{ 96 , 0xE6050060, 0xE605400C,  0, U1G_PORT_RW },	/* PORT96CR  ini:H'A0 */
	{ 97 , 0xE6050061, 0xE605400C,  1, U1G_PORT_RW },	/* PORT97CR  ini:H'A0 */
	{ 98 , 0xE6050062, 0xE605400C,  2, U1G_PORT_RW },	/* PORT98CR  ini:H'A0 */
	{ 99 , 0xE6050063, 0xE605400C,  3, U1G_PORT_RW },	/* PORT99CR  ini:H'A0 */
	{ 100, 0xE6050064, 0xE605400C,  4, U1G_PORT_RW },	/* PORT100CR ini:H'A0 */
	{ 101, 0xE6050065, 0xE605400C,  5, U1G_PORT_RW },	/* PORT101CR ini:H'A0 */
	{ 102, 0xE6050066, 0xE605400C,  6, U1G_PORT_RW },	/* PORT102CR ini:H'A0 */
	{ 103, 0xE6050067, 0xE605400C,  7, U1G_PORT_RW },	/* PORT103CR ini:H'A0 */
	{ 104, 0xE6050068, 0xE605400C,  8, U1G_PORT_RW },	/* PORT104CR ini:H'A0 */
	{ 105, 0xE6050069, 0xE605400C,  9, U1G_PORT_RW },	/* PORT105CR ini:H'A0 */
	{ 106, 0xE605006A, 0xE605400C, 10, U1G_PORT_RW },	/* PORT106CR ini:H'A0 */
	{ 107, 0xE605006B, 0xE605400C, 11, U1G_PORT_RW },	/* PORT107CR ini:H'A0 */
	{ 108, 0xE605006C, 0xE605400C, 12, U1G_PORT_RW },	/* PORT108CR ini:H'A0 */
	{ 109, 0xE605006D, 0xE605400C, 13, U1G_PORT_RW },	/* PORT109CR ini:H'A0 */
	{ 110, 0xE605006E, 0xE605400C, 14, U1G_PORT_RW },	/* PORT110CR ini:H'A0 */
	
	{ 128, 0xE6051080, 0xE6055000,  0, U1G_PORT_RW },	/* PORT128CR ini:H'11 */
	{ 129, 0xE6051081, 0xE6055000,  1, U1G_PORT_RW },	/* PORT129CR ini:H'81 */
	{ 130, 0xE6051082, 0xE6055000,  2, U1G_PORT_RW },	/* PORT130CR ini:H'11 */
	{ 131, 0xE6051083, 0xE6055000,  3, U1G_PORT_RW },	/* PORT131CR ini:H'81 */
	{ 133, 0xE6051085, 0xE6055000,  4, U1G_PORT_RW },	/* PORT133CR ini:H'E1 */
	{ 134, 0xE6051086, 0xE6055000,  5, U1G_PORT_RW },	/* PORT134CR ini:H'A1 */
	{ 135, 0xE6051087, 0xE6055000,  6, U1G_PORT_RW },	/* PORT135CR ini:H'E1 */
	{ 136, 0xE6051088, 0xE6055000,  7, U1G_PORT_RW },	/* PORT136CR ini:H'A1 */
	{ 137, 0xE6051089, 0xE6055000,  8, U1G_PORT_RW },	/* PORT137CR ini:H'11 */
	{ 138, 0xE605108A, 0xE6055000,  9, U1G_PORT_RW },	/* PORT138CR ini:H'81 */
	{ 139, 0xE605108B, 0xE6055000, 10, U1G_PORT_RW },	/* PORT139CR ini:H'11 */
	{ 140, 0xE605108C, 0xE6055000, 11, U1G_PORT_RW },	/* PORT140CR ini:H'11 */
	{ 141, 0xE605108D, 0xE6055000, 12, U1G_PORT_RW },	/* PORT141CR ini:H'11 */
	{ 142, 0xE605108E, 0xE6055000, 13, U1G_PORT_RW },	/* PORT142CR ini:H'11 */
	{ 198, 0xE60520C6, 0xE6056000,  6, U1G_PORT_RW },	/* PORT198CR ini:H'80 */
	{ 199, 0xE60520C7, 0xE6056000,  7, U1G_PORT_RW },	/* PORT199CR ini:H'80 */
	{ 200, 0xE60520C8, 0xE6056000,  8, U1G_PORT_RW },	/* PORT200CR ini:H'80 */
	{ 201, 0xE60520C9, 0xE6056000,  9, U1G_PORT_RW },	/* PORT201CR ini:H'80 */
	{ 202, 0xE60520CA, 0xE6056000, 10, U1G_PORT_RW },	/* PORT202CR ini:H'A1 */
	{ 203, 0xE60520CB, 0xE6056000, 11, U1G_PORT_RW },	/* PORT203CR ini:H'80 */
	{ 204, 0xE60520CC, 0xE6056000, 12, U1G_PORT_RW },	/* PORT204CR ini:H'80 */
	{ 205, 0xE60520CD, 0xE6056000, 13, U1G_PORT_RW },	/* PORT205CR ini:H'80 */
	{ 206, 0xE60520CE, 0xE6056000, 14, U1G_PORT_RW },	/* PORT206CR ini:H'80 */
	{ 207, 0xE60520CF, 0xE6056000, 15, U1G_PORT_RW },	/* PORT207CR ini:H'80 */
	{ 208, 0xE60520D0, 0xE6056000, 16, U1G_PORT_RW },	/* PORT208CR ini:H'80 */
	{ 209, 0xE60520D1, 0xE6056000, 17, U1G_PORT_RW },	/* PORT209CR ini:H'80 */
	{ 210, 0xE60520D2, 0xE6056000, 18, U1G_PORT_RW },	/* PORT210CR ini:H'80 */
	{ 211, 0xE60520D3, 0xE6056000, 19, U1G_PORT_RW },	/* PORT211CR ini:H'80 */
	{ 212, 0xE60520D4, 0xE6056000, 20, U1G_PORT_RW },	/* PORT212CR ini:H'80 */
	{ 213, 0xE60520D5, 0xE6056000, 21, U1G_PORT_RW },	/* PORT213CR ini:H'80 */
	{ 214, 0xE60520D6, 0xE6056000, 22, U1G_PORT_RW },	/* PORT214CR ini:H'80 */
	{ 215, 0xE60520D7, 0xE6056000, 23, U1G_PORT_RW },	/* PORT215CR ini:H'80 */
	{ 216, 0xE60520D8, 0xE6056000, 24, U1G_PORT_RW },	/* PORT216CR ini:H'80 */
	{ 217, 0xE60520D9, 0xE6056000, 25, U1G_PORT_RW },	/* PORT217CR ini:H'80 */
	{ 218, 0xE60520DA, 0xE6056000, 26, U1G_PORT_RW },	/* PORT218CR ini:H'80 */
	{ 219, 0xE60520DB, 0xE6056000, 27, U1G_PORT_RW },	/* PORT219CR ini:H'80 */
	
	{ 224, 0xE60520E0, 0xE6056004,  0, U1G_PORT_RW },	/* PORT224CR ini:H'E0 H'E1 */
	{ 225, 0xE60520E1, 0xE6056004,  1, U1G_PORT_RW },	/* PORT225CR ini:H'E0 H'E1 */
	{ 226, 0xE60520E2, 0xE6056004,  2, U1G_PORT_RW },	/* PORT226CR ini:H'E0 H'E1 */
	{ 227, 0xE60520E3, 0xE6056004,  3, U1G_PORT_RW },	/* PORT227CR ini:H'E0 H'E1 */
	{ 228, 0xE60520E4, 0xE6056004,  4, U1G_PORT_RW },	/* PORT228CR ini:H'E0 H'E1 */
	{ 229, 0xE60520E5, 0xE6056004,  5, U1G_PORT_RW },	/* PORT229CR ini:H'E0 H'E1 */
	{ 230, 0xE60520E6, 0xE6056004,  6, U1G_PORT_RW },	/* PORT230CR ini:H'E0 H'E1 */
	{ 231, 0xE60520E7, 0xE6056004,  7, U1G_PORT_RW },	/* PORT231CR ini:H'E0 H'E1 */
	{ 232, 0xE60520E8, 0xE6056004,  8, U1G_PORT_RW },	/* PORT232CR ini:H'A0 H'A1 */
	{ 233, 0xE60520E9, 0xE6056004,  9, U1G_PORT_RW },	/* PORT233CR ini:H'A0 H'A1 */
	{ 234, 0xE60520EA, 0xE6056004, 10, U1G_PORT_RW },	/* PORT234CR ini:H'A0 H'A1 */
	{ 235, 0xE60520EB, 0xE6056004, 11, U1G_PORT_RW },	/* PORT235CR ini:H'A0 H'A1 */
	{ 236, 0xE60520EC, 0xE6056004, 12, U1G_PORT_RW },	/* PORT236CR ini:H'A0 H'A1 */
	{ 237, 0xE60520ED, 0xE6056004, 13, U1G_PORT_RW },	/* PORT237CR ini:H'A0 H'A1 */
	{ 238, 0xE60520EE, 0xE6056004, 14, U1G_PORT_RW },	/* PORT238CR ini:H'A0 H'A1 */
	{ 239, 0xE60520EF, 0xE6056004, 15, U1G_PORT_RW },	/* PORT239CR ini:H'A0 H'A1 */
	{ 240, 0xE60520F0, 0xE6056004, 16, U1G_PORT_RW },	/* PORT240CR ini:H'A0 H'A1 */
	{ 241, 0xE60520F1, 0xE6056004, 17, U1G_PORT_RW },	/* PORT241CR ini:H'A0 H'A1 */
	{ 242, 0xE60520F2, 0xE6056004, 18, U1G_PORT_RW },	/* PORT242CR ini:H'A0 H'A1 */
	{ 243, 0xE60520F3, 0xE6056004, 19, U1G_PORT_RW },	/* PORT243CR ini:H'A0 H'A1 */
	{ 244, 0xE60520F4, 0xE6056004, 20, U1G_PORT_RW },	/* PORT244CR ini:H'A0 H'A1 */
	{ 245, 0xE60520F5, 0xE6056004, 21, U1G_PORT_RW },	/* PORT245CR ini:H'A0 H'A1 */
	{ 246, 0xE60520F6, 0xE6056004, 22, U1G_PORT_RW },	/* PORT246CR ini:H'A0 H'A1 */
	{ 247, 0xE60520F7, 0xE6056004, 23, U1G_PORT_RW },	/* PORT247CR ini:H'A0 H'A1 */
	{ 248, 0xE60520F8, 0xE6056004, 24, U1G_PORT_RW },	/* PORT248CR ini:H'10 H'11 */
	{ 249, 0xE60520F9, 0xE6056004, 25, U1G_PORT_RW },	/* PORT249CR ini:H'E0 H'E1 */
	{ 250, 0xE60520FA, 0xE6056004, 26, U1G_PORT_RW },	/* PORT250CR ini:H'E0 H'E1 */
	{ 251, 0xE60520FB, 0xE6056004, 27, U1G_PORT_RW },	/* PORT251CR ini:H'E0 H'E1 */
	{ 252, 0xE60520FC, 0xE6056004, 28, U1G_PORT_RW },	/* PORT252CR ini:H'E0 H'E1 */
	{ 253, 0xE60520FD, 0xE6056004, 29, U1G_PORT_RW },	/* PORT253CR ini:H'E0 H'E1 */
	{ 254, 0xE60520FE, 0xE6056004, 30, U1G_PORT_RW },	/* PORT254CR ini:H'E0 H'E1 */
	{ 255, 0xE60520FF, 0xE6056004, 31, U1G_PORT_RW },	/* PORT255CR ini:H'E0 H'E1 */
	
	{ 256, 0xE6052100, 0xE6056008,  0, U1G_PORT_RW },	/* PORT256CR ini:H'E0 H'E1 */
	{ 257, 0xE6052101, 0xE6056008,  1, U1G_PORT_RW },	/* PORT257CR ini:H'E0 H'E1 */
	{ 258, 0xE6052102, 0xE6056008,  2, U1G_PORT_RW },	/* PORT258CR ini:H'A0 H'A1 */
	{ 259, 0xE6052103, 0xE6056008,  3, U1G_PORT_RW },	/* PORT259CR ini:H'A0 H'A2 */
	{ 260, 0xE6052104, 0xE6056008,  4, U1G_PORT_RW },	/* PORT260CR ini:H'A0 */
	{ 261, 0xE6052105, 0xE6056008,  5, U1G_PORT_RW },	/* PORT261CR ini:H'80 */
	{ 262, 0xE6052106, 0xE6056008,  6, U1G_PORT_RW },	/* PORT262CR ini:H'A0 */
	{ 263, 0xE6052107, 0xE6056008,  7, U1G_PORT_RW },	/* PORT263CR ini:H'80 */
	{ 264, 0xE6052108, 0xE6056008,  8, U1G_PORT_RW },	/* PORT264CR ini:H'80 */
	{ 265, 0xE6052109, 0xE6056008,  9, U1G_PORT_RW },	/* PORT265CR ini:H'80 */
	{ 266, 0xE605210A, 0xE6056008, 10, U1G_PORT_RW },	/* PORT266CR ini:H'80 */
	{ 267, 0xE605210B, 0xE6056008, 11, U1G_PORT_RW },	/* PORT267CR ini:H'80 */
	{ 268, 0xE605210C, 0xE6056008, 12, U1G_PORT_RW },	/* PORT268CR ini:H'A0 */
	{ 269, 0xE605210D, 0xE6056008, 13, U1G_PORT_RW },	/* PORT269CR ini:H'80 */
	{ 270, 0xE605210E, 0xE6056008, 14, U1G_PORT_RW },	/* PORT270CR ini:H'80 */
	{ 271, 0xE605210F, 0xE6056008, 15, U1G_PORT_RW },	/* PORT271CR ini:H'A0 */
	{ 272, 0xE6052110, 0xE6056008, 16, U1G_PORT_RW },	/* PORT272CR ini:H'80 */
	{ 273, 0xE6052111, 0xE6056008, 17, U1G_PORT_RW },	/* PORT273CR ini:H'80 */
	{ 274, 0xE6052112, 0xE6056008, 18, U1G_PORT_RW },	/* PORT274CR ini:H'80 */
	{ 275, 0xE6052113, 0xE6056008, 19, U1G_PORT_RW },	/* PORT275CR ini:H'80 */
	{ 276, 0xE6052114, 0xE6056008, 20, U1G_PORT_RW },	/* PORT276CR ini:H'80 */
	{ 277, 0xE6052115, 0xE6056008, 21, U1G_PORT_RW },	/* PORT277CR ini:H'80 */
	
	{ 288, 0xE6052120, 0xE605600C,  0, U1G_PORT_RW },	/* PORT288CR ini:H'11 */
	{ 289, 0xE6052121, 0xE605600C,  1, U1G_PORT_RW },	/* PORT289CR ini:H'A1 */
	{ 290, 0xE6052122, 0xE605600C,  2, U1G_PORT_RW },	/* PORT290CR ini:H'A1 */
	{ 291, 0xE6052123, 0xE605600C,  3, U1G_PORT_RW },	/* PORT291CR ini:H'A1 */
	{ 292, 0xE6052124, 0xE605600C,  4, U1G_PORT_RW },	/* PORT292CR ini:H'A1 */
	{ 293, 0xE6052125, 0xE605600C,  5, U1G_PORT_RW },	/* PORT293CR ini:H'A1 */
	{ 294, 0xE6052126, 0xE605600C,  6, U1G_PORT_RW },	/* PORT294CR ini:H'11 */
	{ 295, 0xE6052127, 0xE605600C,  7, U1G_PORT_RW },	/* PORT295CR ini:H'A1 */
	{ 296, 0xE6052128, 0xE605600C,  8, U1G_PORT_RW },	/* PORT296CR ini:H'A1 */
	{ 297, 0xE6052129, 0xE605600C,  9, U1G_PORT_RW },	/* PORT297CR ini:H'A1 */
	{ 298, 0xE605212A, 0xE605600C, 10, U1G_PORT_RW },	/* PORT298CR ini:H'A1 */
	{ 299, 0xE605212B, 0xE605600C, 11, U1G_PORT_RW },	/* PORT299CR ini:H'A1 */
	{ 300, 0xE605212C, 0xE605600C, 12, U1G_PORT_RW },	/* PORT300CR ini:H'E1 */
	{ 301, 0xE605212D, 0xE605600C, 13, U1G_PORT_RW },	/* PORT301CR ini:H'E1 */
	{ 302, 0xE605212E, 0xE605600C, 14, U1G_PORT_RW },	/* PORT302CR ini:H'E1 */
	{ 303, 0xE605212F, 0xE605600C, 15, U1G_PORT_RW },	/* PORT303CR ini:H'E1 */
	{ 304, 0xE6052130, 0xE605600C, 16, U1G_PORT_RW },	/* PORT304CR ini:H'E1 */
	{ 305, 0xE6052131, 0xE605600C, 17, U1G_PORT_RW },	/* PORT305CR ini:H'E1 */
	{ 306, 0xE6052132, 0xE605600C, 18, U1G_PORT_RW },	/* PORT306CR ini:H'E1 */
	{ 307, 0xE6052133, 0xE605600C, 19, U1G_PORT_RW },	/* PORT307CR ini:H'E1 */
	{ 308, 0xE6052134, 0xE605600C, 20, U1G_PORT_RW },	/* PORT308CR ini:H'E1 */
	{ 309, 0xE6052135, 0xE605600C, 21, U1G_PORT_RW },	/* PORT309CR ini:H'11 */
	{ 310, 0xE6052136, 0xE605600C, 22, U1G_PORT_RW },	/* PORT310CR ini:H'11 */
	{ 311, 0xE6052137, 0xE605600C, 23, U1G_PORT_RW },	/* PORT311CR ini:H'A1 */
	{ 312, 0xE6052138, 0xE605600C, 24, U1G_PORT_RW },	/* PORT312CR ini:H'A1 */
	
	{ 320, 0xE6052140, 0xE6056010,  0, U1G_PORT_RW },	/* PORT320CR ini:H'A1 */
	{ 321, 0xE6052141, 0xE6056010,  1, U1G_PORT_RW },	/* PORT321CR ini:H'A1 */
	{ 322, 0xE6052142, 0xE6056010,  2, U1G_PORT_RW },	/* PORT322CR ini:H'A1 */
	{ 323, 0xE6052143, 0xE6056010,  3, U1G_PORT_RW },	/* PORT323CR ini:H'A1 */
	{ 324, 0xE6052144, 0xE6056010,  4, U1G_PORT_RW },	/* PORT324CR ini:H'A1 */
	{ 325, 0xE6052145, 0xE6056010,  5, U1G_PORT_RW },	/* PORT325CR ini:H'A1 */
	{ 326, 0xE6052146, 0xE6056010,  6, U1G_PORT_RW },	/* PORT326CR ini:H'11 */
	{ 327, 0xE6052147, 0xE6056010,  7, U1G_PORT_RW },	/* PORT327CR ini:H'E1 */
    { U2G_PORT_END , 0x00000000, 0x00000000, 0 , U1G_PORT_RW },
};

/***************************************************************/
/* PUBLIC FUNCTION PROTOTYPE                                   */
/***************************************************************/
void vog_port_st_get( u2 u2t_port_num, u1* ptt_get_data );
void vog_port_st_set( u2 u2t_port_num, u1  u1t_set_data );
void vog_port_get( u2 u2t_port_num, u1* ptt_get_data );
void vog_port_set( u2 u2t_port_num, u1  u1t_set_data );
void vog_port_on ( u2 u2t_port_num );
void vog_port_off( u2 u2t_port_num );
u1 u1g_port_info_check( u2 u2t_port_num, ST_PORT_INFO* ptt_port_info );
u1 u1g_port_search( u2 u2t_port_num );

/*************************************************************************************************/
/* FUNCTION : u1g_port_search( u2 u2t_port_num )                                                 */
/* ARGUMENT : u2 u2t_port_num                                                                    */
/* RETURN   : u1(found:0, not found:0xff)                                                        */
/* OUTLINE  : port status get                                                                    */
/* NOTE     : 2011/11/28                                                                         */
/*************************************************************************************************/
u1 u1g_port_serach( u2 u2t_port_num )
{

	u2 u2t_cnt = U2G_ZERO;
	u1 u1t_ret = 0xFF;

	/* Search port num by port table */
	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			/* read port control register */
			u1t_ret = U1G_ZERO;
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}

	return u1t_ret;
}

/*************************************************************************************************/
/* FUNCTION : vog_port_st_get( u2 u2t_port_num, u1* ptt_get_data )                               */
/* ARGUMENT : u2 u2t_port_num, u1* ptt_get_data(found:port_status, not found:0xff)               */
/* RETURN   : void                                                                               */
/* OUTLINE  : port status get                                                                    */
/*************************************************************************************************/
void vog_port_st_get( u2 u2t_port_num, u1* ptt_get_data )
{

	u2 u2t_cnt = U2G_ZERO;
	*ptt_get_data = 0xFF;

	/* Search port num by port table */
	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			/* read port control register */
			*ptt_get_data = *(VU1 *)sts_scan_port_tbl[u2t_cnt].u4_portcr_addr;
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}
}

/*************************************************************************************************/
/* FUNCTION : vog_port_get( u2 u2t_port_num, u1* ptt_get_data )                                  */
/* ARGUMENT : u2 u2t_port_num, u1* ptt_get_data                                                  */
/* RETURN   : void                                                                               */
/* OUTLINE  : port data get                                                                      */
/*************************************************************************************************/
void vog_port_get( u2 u2t_port_num, u1* ptt_get_data )
{

	u2 u2t_cnt = U2G_ZERO;
	u4 u4t_port_data = U4G_ZERO;

	/* Search port num by port table */
	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			u4t_port_data = *(VU4 *)sts_scan_port_tbl[u2t_cnt].u4_portdr_addr;
			/* PORT data get  */
			*ptt_get_data = (u1)( ( u4t_port_data >> sts_scan_port_tbl[u2t_cnt].u1_portdr_bit ) & U4G_ON );
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}
}

/*************************************************************************************************/
/* FUNCTION : vog_port_st_set( u2 u2t_port_num, u1 u1t_set_data )                                */
/* ARGUMENT : u2 u2t_port_num, u1 u1t_set_data                                                   */
/* RETURN   : void                                                                               */
/* OUTLINE  : port status set                                                                    */
/*************************************************************************************************/
void vog_port_st_set( u2 u2t_port_num, u1 u1t_set_data )
{

	u2 u2t_cnt = U2G_ZERO;
	u1 u1t_port_st = U1G_ZERO;

	/* Search port num by port table */
	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			/* Read Setting status */
			u1t_port_st = *(VU1 *)sts_scan_port_tbl[u2t_cnt].u4_portcr_addr;
			if( u1t_port_st != u1t_set_data ){
				*(VU1 *)sts_scan_port_tbl[u2t_cnt].u4_portcr_addr = u1t_set_data;
			}
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}
}

/*************************************************************************************************/
/* FUNCTION : vog_port_st_set( u2 u2t_port_num, u1  u1t_set_data )                               */
/* ARGUMENT : u2 u2t_port_num, u1  u1t_set_data                                                  */
/* RETURN   : void                                                                               */
/* OUTLINE  : port data set                                                                      */
/*************************************************************************************************/
void vog_port_set( u2 u2t_port_num, u1  u1t_set_data )
{

	u2 u2t_cnt = U2G_ZERO;

	/* Search port num by port table */
	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			if( sts_scan_port_tbl[u2t_cnt].u1_port_io != U1G_PORT_R )
			{
				switch( u1t_set_data )
				{
					case U1G_PORT_OFF:
						/* SET PORT 0(LOW) */
						*(VU4 *)sts_scan_port_tbl[u2t_cnt].u4_portdr_addr &= ~( U4G_ON << sts_scan_port_tbl[u2t_cnt].u1_portdr_bit);
						break;
					case U1G_PORT_ON:
						/* SET PORT 1(HIGH) */
						*(VU4 *)sts_scan_port_tbl[u2t_cnt].u4_portdr_addr |= ( U4G_ON << sts_scan_port_tbl[u2t_cnt].u1_portdr_bit);
						break;
					default:
						break;
				}
			}
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}
}

/*************************************************************************************************/
/* Function name   : vog_port_on( u2 u2t_port_num )                                              */
/* Input     : u2 u2t_port_num                                                                   */
/* Return   : void                                                                               */
/* Processing : PORT DATA SET 1                                                                  */
/*************************************************************************************************/
void vog_port_on( u2 u2t_port_num )
{

	u2 u2t_cnt = U2G_ZERO;

	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			if( sts_scan_port_tbl[u2t_cnt].u1_port_io != U1G_PORT_R )
			{
				*(VU4 *)sts_scan_port_tbl[u2t_cnt].u4_portdr_addr |= ( U4G_ON << sts_scan_port_tbl[u2t_cnt].u1_portdr_bit);
			}
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}
}

/*************************************************************************************************/
/* Function name   : vog_port_off( u2 u2t_port_num )                                                    */
/* Input     : u2 u2t_port_num                                                                    */
/* Return   : void                                                                               */
/* Processing : PORT DATA SET 0                                                                    */
/*************************************************************************************************/
void vog_port_off( u2 u2t_port_num )
{

	u2 u2t_cnt = U2G_ZERO;

	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			if( sts_scan_port_tbl[u2t_cnt].u1_port_io != U1G_PORT_R )
			{
				*(VU4 *)sts_scan_port_tbl[u2t_cnt].u4_portdr_addr &= ~( U4G_ON << sts_scan_port_tbl[u2t_cnt].u1_portdr_bit);
			}
			u2t_cnt = U2G_PORT_END;
		}
		else
		{
			u2t_cnt++;
		}
	}
}

/*************************************************************************************************/
/* FUNCTION : u1g_port_info_check( u2t_port_num, ST_PORT_INFO* ptt_port_info )                   */
/* ARGUMENT : u2t_port_num, ST_PORT_INFO* ptt_port_info                                          */
/* RETURN   : u1( port found:0 port not found:0xFF )                                             */
/* OUTLINE  : port info check                                                                    */
/*************************************************************************************************/
u1 u1g_port_info_check( u2 u2t_port_num, ST_PORT_INFO* ptt_port_info )
{

	u1 u1t_ret = 0xFF;
	u2 u2t_cnt = U2G_ZERO;

	while( U2G_PORT_END != u2t_cnt )
	{
		if( u2t_port_num == sts_scan_port_tbl[u2t_cnt].u2_port_num )
		{
			*ptt_port_info = sts_scan_port_tbl[u2t_cnt];
			u2t_cnt = U2G_PORT_END;
			u1t_ret = U1G_ZERO;
		}
		else
		{
			u2t_cnt++;
		}
	}
	return u1t_ret;
}

