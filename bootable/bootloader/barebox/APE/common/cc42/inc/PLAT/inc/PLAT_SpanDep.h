/**************************************************************************
 *   Copyright 2009 © Discretix Technologies Ltd. This software is         *
 *   protected by copyright, international treaties and various patents.   *
 *   Any copy or reproduction of this Software as permitted below, must    *
 *   include this Copyright Notice as well as any other notices provided   *
 *   under such license.                                                   *
 *                                                                         *
 *   This program shall be governed by, and may be used and redistributed  *
 *   under the terms and conditions of the GNU Lesser General Public       *
 *   License, version 2.1, as published by the Free Software Foundation.   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY liability and WARRANTY; without even the implied      *
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      *
 *   See the GNU General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, please write to the          *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 
#ifndef PLAT_SPAN_DEP_H
#define PLAT_SPAN_DEP_H

/*
 * Includes
 */


  /*
   *  Object %name    : %
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:09:43 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief A brief description of this module
   *
   *  \version PLAT_SpanDep.h#1:incl:1
   *  \author adams
   */

/************************ Defines ******************************/

/* .................... basic Macros for accessing hardware registers ...................... */
/* ----------------------------------------------------------------------------------------- */

#define SAFE_BASE_ADDR   (0x4CF00000)
#define WR_MSB_OFFSET    (0x00000000)
#define WR_LSB_OFFSET    (0x00000008)
#define RD_MSB_OFFSET    (0x0000000c)
#define RD_LSB_OFFSET    (0x00000008)


#if CRYS_TST_HARDWARE_REG_PRINT_MODE

	#define CRYS_PLAT_SYS_WriteRegister(Address , Value) \
	do \
	{ \
		DxUint32_t addr_msb , addr_lsb , value_msb , value_lsb , gen_addr; \
		addr_msb  = (((DxUint32_t)(Address)) >> 12) & 0x000FFFF0 ; \
	    addr_lsb  = (((DxUint32_t)(Address)) << 4) & 0x000FFFF0; \
	    value_msb = ((Value) & 0xFFFF0000) >> 16   ; \
	    value_lsb = ((Value) & 0x0000FFFF); \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = (value_msb); \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + WR_LSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = (value_lsb); \
		printf( "master_write(32'h%08lX , 32'h%08lX); \n",(Address),(Value)); \
	}while(0)


	/*----------------------------------------------READ-----------------------------------------------------------*/


	#define CRYS_PLAT_SYS_ReadRegister(Address, Value) \
	do \
	{ \
		DxUint32_t addr_msb , addr_lsb , read_msb , read_lsb , gen_addr; \
		addr_msb  = (((DxUint32_t)(Address)) >> 12) & 0x000FFFF0 ; \
	    addr_lsb  = (((DxUint32_t)(Address)) << 4) & 0x000FFFF0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = 0x0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + RD_LSB_OFFSET); \
	    read_lsb  = *((volatile DxUint32_t*)gen_addr); \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = 0x0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + RD_MSB_OFFSET); \
	    read_msb  = *((volatile DxUint32_t*)gen_addr); \
	    Value = (read_msb & 0xFFFF0000); \
	    Value |= ((read_lsb >> 16) & 0xFFFF); \
	    printf( "master_read_expect(32'h%08lX , 32'h%08lX); \n",(Address),(DxUint32_t)(Value)); \
	}while(0)

	/*************************************************************************/
	/*************************************************************************/
	/*************************************************************************/
	/*THIS DEBUG IS ONLY FOR THE CF3 LLFCD C implementation (till we can use */
	/* the assembler implementation 										 */
	#define CRYS_PLAT_SYS_WriteRegister_NO_PRINT(Address,Value) \
	do \
	{ \
		DxUint32_t addr_msb , addr_lsb , value_msb , value_lsb , gen_addr; \
		addr_msb  = (((DxUint32_t)(Address)) >> 12) & 0x000FFFF0 ; \
	    addr_lsb  = (((DxUint32_t)(Address)) << 4) & 0x000FFFF0; \
	    value_msb = ((Value) & 0xFFFF0000) >> 16   ; \
	    value_lsb = ((Value) & 0x0000FFFF); \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = (value_msb); \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + WR_LSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = (value_lsb); \
	}while(0)

	#define CRYS_PLAT_SYS_ReadRegister_NO_PRINT(Address,Value) \
	do \
	{ \
		DxUint32_t addr_msb , addr_lsb , read_msb , read_lsb , gen_addr; \
		addr_msb  = (((DxUint32_t)(Address)) >> 12) & 0x000FFFF0 ; \
	    addr_lsb  = (((DxUint32_t)(Address)) << 4) & 0x000FFFF0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = 0x0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + RD_LSB_OFFSET); \
	    read_lsb  = *((volatile DxUint32_t*)gen_addr); \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = 0x0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + RD_MSB_OFFSET); \
	    read_msb  = *((volatile DxUint32_t*)gen_addr); \
	    Value = (read_msb & 0xFFFF0000); \
	    Value |= ((read_lsb >> 16) & 0xFFFF); \
	}while(0)

	/*************************************************************************/
	/*************************************************************************/
	/*************************************************************************/
#else /* CRYS_TST_HARDWARE_REG_PRINT_MODE */

	#define CRYS_PLAT_SYS_WriteRegister(Address,Value) \
	do \
	{ \
		DxUint32_t addr_msb , addr_lsb , value_msb , value_lsb , gen_addr; \
		addr_msb  = (((DxUint32_t)(Address)) >> 12) & 0x000FFFF0 ; \
	    addr_lsb  = (((DxUint32_t)(Address)) << 4) & 0x000FFFF0; \
	    value_msb = ((Value) & 0xFFFF0000) >> 16   ; \
	    value_lsb = ((Value) & 0x0000FFFF); \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = (value_msb); \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + WR_LSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = (value_lsb); \
	}while(0)

	#define CRYS_PLAT_SYS_ReadRegister(Address,Value) \
	do \
	{ \
		DxUint32_t addr_msb , addr_lsb , read_msb , read_lsb , gen_addr; \
		addr_msb  = (((DxUint32_t)(Address)) >> 12) & 0x000FFFF0 ; \
	    addr_lsb  = (((DxUint32_t)(Address)) << 4) & 0x000FFFF0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = 0x0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + RD_LSB_OFFSET); \
	    read_lsb  = *((volatile DxUint32_t*)gen_addr); \
	    gen_addr = (SAFE_BASE_ADDR + addr_msb + WR_MSB_OFFSET); \
	    *((volatile DxUint32_t*)gen_addr) = 0x0; \
	    gen_addr = (SAFE_BASE_ADDR + addr_lsb + RD_MSB_OFFSET); \
	    read_msb  = *((volatile DxUint32_t*)gen_addr); \
	    Value = (read_msb & 0xFFFF0000); \
	    Value |= ((read_lsb >> 16) & 0xFFFF); \
	}while(0)


	#define CRYS_PLAT_SYS_ReadRegister_NO_PRINT CRYS_PLAT_SYS_ReadRegister
	#define CRYS_PLAT_SYS_WriteRegister_NO_PRINT CRYS_PLAT_SYS_WriteRegister

#endif /* CRYS_TST_HARDWARE_REG_PRINT_MODE */



#endif /*PLAT_SPAN_DEP_H*/


