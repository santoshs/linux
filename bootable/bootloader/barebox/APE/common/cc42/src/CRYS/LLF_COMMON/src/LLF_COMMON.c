/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

#include "DX_VOS_Mem.h"
#include "DX_VOS_Memmap.h"
#ifndef CRYS_NO_GLOBAL_DATA
#include "DX_VOS_Sem.h"
#else
#define DX_VOS_SemWait(semId,timeout)	CRYS_OK
#define DX_VOS_SemGive(semId)
#endif
#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "LLF_COMMON_error.h"
#include "LLF_COMMON_HwDefs.h"

/************************ Defines ******************************/
/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */


/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/* importing the base address of the CRYPTO Cell . this value must be initialized in the
   CRYS_Init function */

extern DxUint32_t PLAT_CryptoCellBaseAddr;

const DxUint32_t CRYS_COMMON_BusAlignerMask = 0x3;
#ifndef CRYS_SEP_SIDE_WORK_MODE
const DxUint32_t CRYS_COMMON_BusAlignerShift = 0x3;
#else
const DxUint32_t CRYS_COMMON_BusAlignerShift = 0x2;
#endif



/************************ Public Functions ******************************/

/**
 * @brief The low level LLF_COMMON_Init function initialisez the HW components common to al modules
 *
 *
 * @param[in] param1 -
 * @param[in] param2_ptr -
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */

//CRYSError_t  LLF_COMMON_Init(DxUint32_t   param1 , void* param2_ptr)
 //{
	/* LOCAL DECLERATIONS */

	/* The return error identifier */
//	CRYSError_t Error;

	/* the virtual address definition */
//	DxUint32_t VirtualHwBaseAddr;

	/* register value */
//	DxUint32_t regVal;

	/* FUNCTION LOGIC */

	/* ............... local initializations .............................. */
	/* -------------------------------------------------------------------- */

	/* initializing the Error to O.K */
//	Error = CRYS_OK;

  //  param1 = param1;
    //param2_ptr = param2_ptr;

	/* ............... mapping the physical memory to the virtual one ...... */
	/* --------------------------------------------------------------------- */

//	Error = DX_VOS_MemMap( PLAT_CryptoCellBaseAddr,        /* low address - in */
  //                        LLF_COMMON_HW_CRYPTO_ADDR_SPACE,   /* 16 LS bit space - in */
    //                      &VirtualHwBaseAddr );           /* The virtual address - out */

//	if( Error != CRYS_OK )
//	{
//	goto Return;
//	}

    /* read the flags */
  //  CRYS_PLAT_SYS_ReadRegister( VirtualHwBaseAddr + LLF_COMMON_HW_AHB_FLAGS_ADDR, regVal);

    /* write the DMA configuration */
    //CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_AHB_RD_WR_BURSTS_ADDR, regVal );

	/* .... un mappping the physical memory to the virtual one and releasing the semaphore ...... */
//	DX_VOS_MemUnMap( &VirtualHwBaseAddr,             /* virtual address - in */
  //                  LLF_COMMON_HW_CRYPTO_ADDR_SPACE ); /* 16 LS bit space - in */

//Return:

//	return Error;

 //}/* END OF LLF_COMMON_Bypass_Block */
