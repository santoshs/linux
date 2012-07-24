/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


  /*
   *  Object %name    : FVOS.c
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief Secure Boot implemtation of hash queues
   *
   *  \version
   *  \author yevgenys
   */
#include "DX_VOS_Mem.h"

#include "FVOS_Types.h"
#include "FVOS_API.h"
#include "FVOS_Config.h"
#include "FVOS_HwDefs.h"
#include "gen.h"
#include "FVOS_Error.h"
#include "log_output.h"
//#include "log_output.h"

#ifdef DX_PPC_INTEGRATOR_DEBUG
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#endif

#ifndef DX_PPC_INTEGRATOR_DEBUG
#ifndef DX_USE_BOARD_DMA_AREA
//#include "rt_misc.h"
#endif

#endif
#if defined DX_ARM_INTEGRATOR_DEBUG | DX_PPC_INTEGRATOR_DEBUG
#include "time.h"
#endif


/*-------------------------
  GLOBAL variables
--------------------------*/

/* counter for the gpr value */
DxUint32_t          g_GPRPollValue;

/* file descriptor for virtual adress mapping in PPC  platform */
DxUint32_t          g_PPC_FileDescript;

/* diff erence between virtual and phisical base address for  ppc no OS platform , for all other platforms tis value has to be zero*/
DxInt32_t          g_FVOS_SharedAddrDiff;

/* virtual reg address for  ppc no OS platform */
DxUint32_t          g_FVOS_VirRegAddress;

/* diff between virtual reg address and phis reg address in  ppc no OS platform */
DxInt32_t          g_FVOS_DiffRegAddress;

/* address of the SEP shared area */
DxUint32_t          g_FVOS_SharedAreaAddress;

/* the start address of the area where the DMA tables for synchronic symmetric operations are created */
DxUint32_t          g_FVOS_DMATableAreaAddress;

/* number of DMA tables already created during current API */
DxUint32_t          g_FVOS_DMATablesCreated;

/* allocation offset */
DxUint32_t          g_FVOS_AllocOffset = 0;

/* static memory address */
DxUint32_t          g_FVOS_StaticMemoryAddr;

/* start address of memory region for allocations */
DxUint32_t          g_FVOS_AllocMemoryAddr;

/* start adress of the region for the system data (time, caller id etc) */
DxUint32_t          g_FVOS_SystemDataMemoryAreaAddress;

#ifdef DX_PPC_INTEGRATOR_DEBUG
DxUint32_t          g_FVOS_DMAUserDataOutAreaAddr;

DxUint32_t          g_FVOS_DMADataOutAreaAddr;

DxUint32_t          g_FVOS_DMADataSize;

DxUint32_t          g_FVOS_DMADataInAreaAddr;

DxUint32_t          g_FVOS_ResidentAddr;
#endif


#ifndef CRYS_DSM_FLAG
#ifndef DX_USE_BOARD_DMA_AREA
extern unsigned int  Image$$DATA_SECTION$$ZI$$Limit;
#ifndef DX_NO_WMDRM_SUPPORT
extern unsigned int  Image$$HEAP_FOR_DMA_SECTION$$Base;
extern unsigned int  Image$$STACK_FOR_DMA_SECTION$$Base;
#endif
#endif
#endif
#ifdef DX_USE_BOARD_DMA_AREA
DxUint32_t  outMemAddress;
DxUint32_t  outMemDataSize;
DxUint32_t  dmaOutputAddress;
DxUint8_t*  dmaInputDataArea_ptr;
DxUint8_t*  dmaOutputDataArea_ptr;
#endif

/*-------------------------
  DEFINES
--------------------------*/

#define DX_CSI                              0xFFFFFFFC

#ifdef DX_PPC_INTEGRATOR_DEBUG
#define SEP_REG_AREA_SIZE				0x10000
#define SEP_SHARED_AREA_SIZE			    0x10000000
#endif


/*-----------------------------
    prototypes
-------------------------------*/

/**
 * @brief     This function handles the send message to SEP command
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleMsgReadyCmd(void);

/**
 * @brief     This function handles the send reply message to SEP command
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReplyMsgReadyCmd(void);

/**
 * @brief     This function handles the allocate memory from dynamic pool command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleAllocMemoryCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles the build DMa tables for symmetric action (AES,DES,HASH)
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleBuildSymDmaTablesCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles the free DMA resources command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleFreeDMAResources(void);

/**
 * @brief     This function handles write data command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleWriteDataCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles read data command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReadDataCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles get static pool data command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleGetStaticPoolDataCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles convert static poll physical address to virtual command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleConverStaticPhysToVirtCmd(FVOS_Cntrl_info_t* cmdData_ptr);


/**
 * @brief     This function handles the get time command
 *
 * @param[out] timeAddress_ptr - address of the stored time value
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleGetTimeCmd(FVOS_Cntrl_info_t* cmdData_ptr);


/**
 * @brief     This function handles check configuration command
 *
 * @param[out] timeAddress_ptr - address of the stored time value
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleCheckConfigurationCmd(FVOS_Cntrl_info_t* cmdData_ptr);



/**
 * @brief     This function handles the realloc cache and resident command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReallocCacheResidentCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles the realloc command of the external cache
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReallocExtCacheCmd(FVOS_Cntrl_info_t* cmdData_ptr);

/**
 * @brief     This function handles theSEP Init command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleSEPInitCmd(FVOS_Cntrl_info_t* cmdData_ptr);


/**
 * @brief     This function constructs the lli table from the input of the array of physical addresses
 *            sizes for each physical buffer ,  and the number of physical buffer. Function uses the the VOS function for allocation of the
 *            memory needed for the
 *
 *
 * @param[in] inputPhysicalBuffersArray - input physical buffers and their size
 * @param[out] firstInputLLITable_ptr - holds pointer to the first lli table in the list
 * @param[out] numEntriesInTable_ptr - number of entries in the first lli table
 *
 * @return     DxError_t:
 *
 */
/*static FVOS_Error_t FVOS_PrepareInputDMATable(DxUint32_t   dataInAddr,
                                              DxUint32_t   dataInSize,
                                              DxUint32_t*  firstInputLLITable_ptr,
                                              DxUint32_t*  numEntriesInTable_ptr,
                                              DxUint32_t*  firstTableDataSize_ptr);

*/
/**
 * @brief     This function creates tables for symmetric engines (AES,DES). The size of data in each
 *            table is modulud the operationBlockSize
 *
 *
 * @param[in] inputPhysicalBuffersArray - physical memory buffers for input
 * @param[in] outputPhysicalBuffersArray - physical memory buffers for output
 * @param[in] operationBlockSize - block size of the operation (AES or DES)
 * @param[out] firstInputLLITable_ptr - pointer to the first table in the list of input tables
 * @param[out] firstOutputLLITable_ptr - pointer to the first table in the list of output tables
 * @param[out] numInTableEntries_ptr - number of entries in the first input table
 * @param[out] numOutTableEntries_ptr - number of entries in the first output table
 * @param[out] firstTableDataSize_ptr - the size of data in the first table (input or output, since they must be equal)
 *
 * @return     DxError_t:
 *
 */
/*static FVOS_Error_t FVOS_PrepareInputOutputDMATables(DxUint32_t   dataInAddr,
                                                     DxUint32_t   dataOutAddr,
                                                     DxUint32_t   dataInSize,
                                                     DxUint32_t   operationBlockSize,
                                                     DxUint32_t*  firstInputLLITable_ptr,
                                                     DxUint32_t*  firstOutputLLITable_ptr,
                                                     DxUint32_t*  numInTableEntries_ptr,
                                                     DxUint32_t*  numOutTableEntries_ptr,
                                                     DxUint32_t*  firstTableDataSize_ptr);


static FVOS_Error_t FVOS_StartRom(FVOS_Cntrl_info_t* cmdData_ptr);

#if defined DX_PPC_INTEGRATOR_DEBUG | DX_ARM_INTEGRATOR_DEBUG
static FVOS_Error_t FVOS_TST_Init();

static void FVOS_TST_SepRomLoader ( void );
static void FVOS_TST_OTPBurning(void);
 void FVOS_TST_OTPRead(void);
#endif*/
/**
 * @brief     This function should retrieve the physical address of the
 *            RAR shared area. Not relevant in the NoOs.
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 */
//FVOS_Error_t FVOS_HandleRarCmd(FVOS_Cntrl_info_t* cmdData_ptr);


/* function to be called as callback for NVS transactions */
//extern FVOS_Error_t NVSTreadParser(void);



/*--------------------------
    FUNCTIONS
----------------------------*/

/**
 * @brief     This functions initializes the FVOS - shared area address
 *
 * @param[in] driver_name_ptr - the sep driver name
 * @return     DxError_t_t:
 *
 */
/*FVOS_Error_t  FVOS_Init(const DxChar_t *driver_name_ptr)
{
  g_FVOS_DiffRegAddress = 0;

#ifdef DX_PPC_INTEGRATOR_DEBUG
    if ((g_PPC_FileDescript = open("/dev/mem", O_RDWR|O_SYNC))<0)
	{
		perror("open");
		exit(-1);
	}
*/
	/* map the workspace buffer */
/*	g_FVOS_VirRegAddress = (unsigned int)mmap(0, SEP_REG_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_PPC_FileDescript,                                              GEN_CC_BASE_ADDR );
    if (g_FVOS_VirRegAddress == 0 || g_FVOS_VirRegAddress == 0xFFFFFFFF )
    {
	perror("register mmap failed");
		exit(-1);
    }
	g_FVOS_DiffRegAddress = GEN_CC_BASE_ADDR - g_FVOS_VirRegAddress;
#endif

#if defined DX_PPC_INTEGRATOR_DEBUG | DX_ARM_INTEGRATOR_DEBUG
	FVOS_TST_Init();
#endif
  return FVOS_OK;
}*/

/**
 * @brief     This functionsconfigurates the FVOS - shared area address
 *
 * @param[in] sharedAreaAddr - address of the shared area
 * @return     DxError_t_t:
 *
 */
void  FVOS_Config(DxUint32_t sharedAreaAddr)
{

  /*-------------------------
      CODE
  ----------------------------*/
  /* set the shared area */
  g_FVOS_SharedAddrDiff = 0;

#ifdef DX_PPC_INTEGRATOR_DEBUG

  g_FVOS_SharedAreaAddress = (unsigned int)mmap(0, SEP_SHARED_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, g_PPC_FileDescript,                                               sharedAreaAddr);
  if (g_FVOS_SharedAreaAddress == 0 || g_FVOS_SharedAreaAddress == 0xFFFFFFFF )
  {
	perror("register mmap failed");
		exit(-1);
  }
  g_FVOS_SharedAddrDiff = sharedAreaAddr - g_FVOS_SharedAreaAddress;

#else
  g_FVOS_SharedAreaAddress = sharedAreaAddr;
  //PRINTF("g_FVOS_SharedAreaAddress = %x\n", g_FVOS_SharedAreaAddress);
  //PRINTF("g_FVOS_SharedAreaAddress = %X\n", g_FVOS_SharedAreaAddress);
#endif


  /* set the static memory address at the end of the shared message area */
  g_FVOS_StaticMemoryAddr = g_FVOS_SharedAreaAddress + FVOS_MESSAGE_MEMORY_SIZE_IN_BYTES;

  /* set the allocation memory address at the end of the static memory */
  g_FVOS_AllocMemoryAddr = g_FVOS_StaticMemoryAddr + FVOS_STATIC_MEMORY_SIZE_IN_BYTES;

  /* set the DMA tables start address at the end of the allocation memory area */
  g_FVOS_DMATableAreaAddress = g_FVOS_AllocMemoryAddr + FVOS_SYSTEM_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES;

  /* set the area for system data */
  g_FVOS_SystemDataMemoryAreaAddress = g_FVOS_DMATableAreaAddress + FVOS_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES;

#ifdef DX_PPC_INTEGRATOR_DEBUG
   g_FVOS_ResidentAddr = g_FVOS_SystemDataMemoryAreaAddress + FVOS_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES + 1024*140;
   g_FVOS_ResidentAddr = g_FVOS_ResidentAddr + (8192 - (g_FVOS_ResidentAddr%8192));

  g_FVOS_DMADataInAreaAddr = g_FVOS_SystemDataMemoryAreaAddress + FVOS_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES;
#endif

  g_FVOS_DMATablesCreated = 0;

#ifdef DX_USE_BOARD_DMA_AREA
  dmaInputDataArea_ptr = (DxUint8_t*)0xC2040000;
  dmaOutputDataArea_ptr = (DxUint8_t*)0xC2050000;
#endif

  return;
}


/**
 * @brief     This functions resets the FVOS registers (IMR). In NoOS (polling), we do nothing
 *
 * @return
 *
 */
/*void FVOS_Reset()
{
  return;
}*/


/**
 * @brief     This functions start the transaction to the SEP - this is the first function to be called during transaction to SEP
 *            In NoOS configuration no action needs to be performed
 *
 * @return     DxError_t_t:
 *
 */
/*FVOS_Error_t FVOS_StartSingletonTransaction(void)
{
    return FVOS_OK;
}*/

/**
 * @brief     This functions start the transaction to the SEP - this is the first function to be called during transaction to SEP
 *            In NoOS configuration no action needs to be performed
 *
 * @return     DxError_t_t:
 *
 */
//FVOS_Error_t FVOS_StartTransaction()
//{
  /* regVal */
  //DxUint32_t      regVal;

  /*-----------------------------
      CODE
  ---------------------------------*/

  //g_FVOS_AllocOffset = 0;

  //#ifndef DX_PPC_INTEGRATOR_DEBUG
  /* read the current value of the GRP SEP register */
  //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal);
  /*Check that GPR3 has legal value of working SEP*/
  //if ((regVal != 0x0) &&  /* regular work mode */
    //  (regVal != 0x4) &&  /* After cold boot */
      //(regVal != 0x8) &&  /* After Warm boot */
	//  (regVal != 0x10))   /* After Cold Warm boot */
  //{
  //		return FVOS_SEP_IS_DISABLED_ERR;
  //}
  //#endif

  //return FVOS_OK;
//}

/**
 * @brief     This function end the transaction to SEP - this is the last function in the transaction
 *            In NoOS configuration no action needs to be performed
 *
 * @return     DxError_t_t:
 *
 */
//FVOS_Error_t FVOS_EndTransaction(DxUint32_t mappedAddr)
//{
  /*-----------------------------
      CODE
  ---------------------------------*/

 // return FVOS_OK;
//}

/**
 * @brief     This function maps the shared message area to the application memory. It returns a pointer,
 *            trough which application can directly write message data into message shared area
 *
 * @param[out] mappedAddr_ptr - the mapped address will be returned into this pointer
 *
 * @return     DxError_t_t:
 *
 */
FVOS_Error_t FVOS_Mmap(DxUint32_t* mappedAddr_ptr)
{
  /*-----------------------------
      CODE
  ---------------------------------*/

  /* the message starts from the second word of the shared area, the first word is taken by the flows status word */
  *mappedAddr_ptr = g_FVOS_SharedAreaAddress ;
  PRINTF("mappedAddr_ptr = %X\n",mappedAddr_ptr);
  return FVOS_OK;
}


/**
 * @brief     This function performs a number of commands according to the CMD paramater:
 *            1) Signal to SEP that the message is ready
 *            2) Allocate memory continues locked memory
 *            3) Read from the allocated memory
 *            4) Write into allocated memory
 *
 * @param[in] commandId - the id of command to execute
 * @param[in] cmdData_ptr - pointer to the strucutre that contains the paramaters for command execution
 *
 * @return     DxError_t_t:
 *
 */
//FVOS_Error_t FVOS_Cntrl(FVOS_Cntrl_cmd_t commandId , FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /* error */
  //FVOS_Error_t    error;

  /*-----------------------------
      CODE
  ---------------------------------*/

  /*error = FVOS_OK;

  switch(commandId)
  {
    case FVOS_CNTRL_SEND_MSG_RDY_CMD:
    */
      /* handle the send message to SEP command */
    /*  error = FVOS_HandleMsgReadyCmd();

      break;

    case FVOS_CNTRL_SEND_RPLY_MSG_RDY_CMD:
     */
      /* handle the send reply message to SEP command */
      /*error = FVOS_HandleReplyMsgReadyCmd();

      break;

    case FVOS_CNTRL_ALLOC_MEM_CMD:
      */
      /* handle alloc memory from dynamic pool command */
      /*error = FVOS_HandleAllocMemoryCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_BUILD_SYM_DMA_TABLES_CMD:
    */
      /* handle the command */
      /*error = FVOS_HandleBuildSymDmaTablesCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_FREE_DMA_RESOURSES:

      error = FVOS_HandleFreeDMAResources();*/
      /* no need for any action in the NoOS  */
/*      break;

    case FVOS_CNTRL_WRITE_DATA_CMD:

      error = FVOS_HandleWriteDataCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_READ_DATA_CMD:

      error = FVOS_HandleReadDataCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_GET_STATIC_POOL_ADDR:

      error = FVOS_HandleGetStaticPoolDataCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_CONV_STATIC_PHYS_TO_VIRT:

      error = FVOS_HandleConverStaticPhysToVirtCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_SEP_API_MODE:
  */
      /* nothing needs to be done */
    /*  break;

    case FVOS_CNTRL_GET_TIME:

      error = FVOS_HandleGetTimeCmd(cmdData_ptr);

      break;

    case FVOS_CHECK_CONFIGURATION:

      error = FVOS_HandleCheckConfigurationCmd(cmdData_ptr);

      break;

    case FVOS_READ_PHYS_REGISTER:

      FVOS_ReadRegister(cmdData_ptr->data.regDataOp.regVirtAddr,cmdData_ptr->data.regDataOp.regData);

      break;

    case FVOS_WRITE_PHYS_REGISTER:

      FVOS_WriteRegister(cmdData_ptr->data.regDataOp.regVirtAddr,cmdData_ptr->data.regDataOp.regData);

      break;

    case FVOS_REALLOC_CACHE_RESIDENT:

      error = FVOS_HandleReallocCacheResidentCmd(cmdData_ptr);

      break;

    case FVOS_REALLOC_EXT_CACHE_RESIDENT:

      error = FVOS_HandleReallocExtCacheCmd(cmdData_ptr);

      break;

    case FVOS_CNTRL_SEP_INIT:

      error = FVOS_HandleSEPInitCmd(cmdData_ptr);

      break;

	case FVOS_CNTRL_RAR_PREPARE_MESSAGE:

	  error = FVOS_HandleRarCmd(cmdData_ptr);

      break;

	case FVOS_START_ROM:

	  error = FVOS_StartRom(cmdData_ptr);

      break;


    default:
      break;
  }

  return error;
}*/


/**
 * @brief     This function polls for the for the response from SEP
 *            Polling can be blocking or busy wait,and can be regulated by timeout
 * @param[in] Timeout - timeout for polling
 * @param[out] pollSource_ptr - the source of the poll interrupt (regular response, or MVS request from SEP
 * @return     DxError_t_t:
 *
 */
//FVOS_Error_t FVOS_Poll(DxUint32_t  timeout , DxUint32_t* pollSource_ptr)
//{
  /* error */
  //FVOS_Error_t    error;

  /* regVal */
  //DxUint32_t      regVal2,regVal3;

  /*-----------------------
      CODE
  -------------------------*/

  //error = FVOS_OK;

  /* update counter, minding the wrap around */
  //g_GPRPollValue = (g_GPRPollValue + 1) & 0x3FFFFFFF;

  /* wait for incoming message notification */
  //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR2_REG_ADDR , regVal2);
  //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal3);
  //while( (regVal3 ==0x0) && (regVal2 & 0x3FFFFFFF) != g_GPRPollValue )
  //{
    //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR2_REG_ADDR , regVal2);
    //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal3);
  //}

  //if(regVal3 !=0x0)
  //{
  //	return FVOS_CC_POLLING_STATUS_ERR;
  //}
  /* check if the callback bit is raised - if raised call the callback function */
  //if(regVal2 & (1<<30))
  //{
    //*pollSource_ptr = FVOS_SEP_PRINTF;
    //return error;
  //}
  //if(regVal2 >> 31)
  //{
    //*pollSource_ptr = FVOS_SEP_REQUEST;
  //}
  //else
  //{
    //*pollSource_ptr = FVOS_SEP_REPLY;
  //}

  //return error;
//}

/*-------------------------------------
    Private functions
---------------------------------------*/

/**
 * @brief     This function handles the send message to SEP command
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleMsgReadyCmd()
//{
  /* read the current value of the GRP SEP register */
  //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR2_REG_ADDR , g_GPRPollValue);

  /* raise interrupt to SEP */
  //FVOS_WriteRegister(GEN_HW_HOST_HOST_SEP_GPR0_REG_ADDR , 0x2);

  //return FVOS_OK;
//}

/**
 * @brief     This function handles the send reply message to SEP command
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReplyMsgReadyCmd()
//{
  /* update counter, minding the wrap around */
  //g_GPRPollValue = (g_GPRPollValue + 1) & 0x3FFFFFFF;

  /* raise interrupt to SEP */
  //FVOS_WriteRegister(GEN_HW_HOST_HOST_SEP_GPR2_REG_ADDR , g_GPRPollValue);

  //return FVOS_OK;
//}


/**
 * @brief     This function handles the allocate memory from dynamic pool command
 *
 * @param[in] allocSize - size to allocate
 * @param[out] fvosAddr_ptr - fvos address (virtual) of the allocated memory
 * @param[out] physAddr_ptr - physical address of the alloctaed memory
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleAllocMemoryCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{

  /* error */
  //FVOS_Error_t  error;

  /*-----------------------
      CODE
  --------------------------*/

  //error = FVOS_OK;

  /* check that there is a space left for allocation */
  //if( (g_FVOS_AllocOffset + cmdData_ptr->data.allocCmdData.allocSize) > FVOS_SYSTEM_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES)
  //{
    //error = FVOS_ALLOCATE_FAIL_ERR;

    //goto end_function;
  //}

  //cmdData_ptr->data.allocCmdData.virtAddr = g_FVOS_AllocMemoryAddr + g_FVOS_AllocOffset;
  //cmdData_ptr->data.allocCmdData.physAddr = cmdData_ptr->data.allocCmdData.virtAddr  + g_FVOS_SharedAddrDiff;

  //g_FVOS_AllocOffset += cmdData_ptr->data.allocCmdData.allocSize;


//end_function:

  //return error;
//}

/**
 * @brief     This function handles the allocate memory from dynamic pool command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleBuildSymDmaTablesCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /* error */
  //FVOS_Error_t  error = FVOS_OK;

  /* build mode */
  //DxUint32_t    buildMode = 3;

  /*--------------------------------
      CODE
  --------------------------------*/

  /* check if output table must be created */
  //if(cmdData_ptr->data.buildSymDMATables.dataOutAddr == DX_NULL)
  //{
    //buildMode  &= ~(0x2);
  //}

  /* check if output table should be CSI */
  //if(cmdData_ptr->data.buildSymDMATables.dataOutAddr >= DX_CSI)
  //{
    //buildMode  &= ~(0x2);
    //cmdData_ptr->data.buildSymDMATables.firstOutTableAddr = cmdData_ptr->data.buildSymDMATables.dataOutAddr;
    //cmdData_ptr->data.buildSymDMATables.firstOutTableNumEntries = 2;
  //}

  /* check if input table should be CSI */
  //if(cmdData_ptr->data.buildSymDMATables.dataInAddr >= DX_CSI)
  //{
    //buildMode  &= ~(0x1);
    //cmdData_ptr->data.buildSymDMATables.firstInTableAddr = cmdData_ptr->data.buildSymDMATables.dataInAddr;
    //cmdData_ptr->data.buildSymDMATables.firstInTableNumEntries = 2;
  //}

  //switch(buildMode)
  //{
    //case 0x1:
      /* only input table should be created */
      /*error = FVOS_PrepareInputDMATable(cmdData_ptr->data.buildSymDMATables.dataInAddr,
                                        cmdData_ptr->data.buildSymDMATables.dataInSize,
                                        &cmdData_ptr->data.buildSymDMATables.firstInTableAddr,
                                        &cmdData_ptr->data.buildSymDMATables.firstInTableNumEntries,
                                        &cmdData_ptr->data.buildSymDMATables.firstTableDataSize);

      break;

    case 0x2:*/
      /* only output table should be created */
      /*error = FVOS_PrepareInputDMATable(cmdData_ptr->data.buildSymDMATables.dataOutAddr,
                                        cmdData_ptr->data.buildSymDMATables.dataInSize,
                                        &cmdData_ptr->data.buildSymDMATables.firstOutTableAddr,
                                        &cmdData_ptr->data.buildSymDMATables.firstOutTableNumEntries,
                                        &cmdData_ptr->data.buildSymDMATables.firstTableDataSize);
      break;

    case 0x3:*/
      /* both tables should be created */
      /*error = FVOS_PrepareInputOutputDMATables(cmdData_ptr->data.buildSymDMATables.dataInAddr,
                                               cmdData_ptr->data.buildSymDMATables.dataOutAddr,
                                               cmdData_ptr->data.buildSymDMATables.dataInSize,
                                               cmdData_ptr->data.buildSymDMATables.blockSize,
                                               &cmdData_ptr->data.buildSymDMATables.firstInTableAddr,
                                               &cmdData_ptr->data.buildSymDMATables.firstOutTableAddr,
                                               &cmdData_ptr->data.buildSymDMATables.firstInTableNumEntries,
                                               &cmdData_ptr->data.buildSymDMATables.firstOutTableNumEntries,
                                               &cmdData_ptr->data.buildSymDMATables.firstTableDataSize);
      break;

    default:*/

      /* this is the case when CSI with hash - we must set the table size */
      //cmdData_ptr->data.buildSymDMATables.firstTableDataSize = cmdData_ptr->data.buildSymDMATables.dataInSize;

      /* this is no error since it is possible that both input and output are for CSI, or we are working in CSI for HASH */
     // break;
  //}

  //return error;
//}





/**
 * @brief     This function handles the free DMA resources command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleFreeDMAResources()
//{
  /*------------------------
      CODE
  --------------------------*/
//#ifdef DX_PPC_INTEGRATOR_DEBUG

  //if(g_FVOS_DMAUserDataOutAreaAddr != 0)
  //{
    /* copy data to the output memory */
   // DX_VOS_FastMemCpy((DxUint8_t*)g_FVOS_DMAUserDataOutAreaAddr , (DxUint8_t*)g_FVOS_DMADataOutAreaAddr , g_FVOS_DMADataSize);
  //}
//#else
//#ifdef DX_USE_BOARD_DMA_AREA
  //if(outMemAddress != 0)
  //{
    /* copy data to the output memory */
    //DX_VOS_FastMemCpy((DxUint8_t*)outMemAddress , (DxUint8_t*)dmaOutputAddress , outMemDataSize);
  //}

//#endif
//#endif


 // g_FVOS_DMATablesCreated = 0;



 // return FVOS_OK;
//}

/**
 * @brief     This function handles write data command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleWriteDataCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /*------------------
      CODE
  ---------------------*/
  //DX_VOS_FastMemCpy((DxUint8_t*)cmdData_ptr->data.writeCmdData.destFvosAddr , (DxUint8_t*)cmdData_ptr->data.writeCmdData.srcAppAddr , cmdData_ptr->data.writeCmdData.dataSize);

/*	if (cmdData_ptr->data.writeCmdData.bytesArrayFlag)
	{
		unsigned int i;
		unsigned int *p = (unsigned int *)cmdData_ptr->data.writeCmdData.destFvosAddr;
		for (i=0;i<(( cmdData_ptr->data.writeCmdData.dataSize + 3 ) / 4);i++)
		{
			p[i] = DX_GEN_CHANGE_WORD_ENDIANNESS(p[i]);
		}
	}

  return FVOS_OK;
}*/

/**
 * @brief     This function handles read data command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReadDataCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /*------------------------
      CODE
  ---------------------------*/

  /*if (cmdData_ptr->data.readCmdData.bytesArrayFlag)
	{
		unsigned int i;
		unsigned int *p = (unsigned int *)cmdData_ptr->data.readCmdData.srcFvosAddr;
		for (i=0;i<(( cmdData_ptr->data.readCmdData.dataSize + 3 ) / 4);i++)
		{
			p[i] = DX_GEN_CHANGE_WORD_ENDIANNESS(p[i]);
		}
	}

  DX_VOS_FastMemCpy((DxUint8_t*)cmdData_ptr->data.readCmdData.destAppAddr , (DxUint8_t*)cmdData_ptr->data.readCmdData.srcFvosAddr , cmdData_ptr->data.readCmdData.dataSize);

  return FVOS_OK;
}*/

/**
 * @brief     This function handles get static pool data command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleGetStaticPoolDataCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /*----------------------------
      CODE
  -------------------------------*/

  /* set the addresses of the static memory */
  /*cmdData_ptr->data.staticPoolInfo.physAddr = g_FVOS_StaticMemoryAddr + g_FVOS_SharedAddrDiff;
  cmdData_ptr->data.staticPoolInfo.fvosAddr = g_FVOS_StaticMemoryAddr;

  return FVOS_OK;
}*/

/**
 * @brief     This function handles convert static poll physical address to virtual command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleConverStaticPhysToVirtCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /* set the addresses of the static memory */
  //cmdData_ptr->data.convStatPhysToVirt.virtAddr = cmdData_ptr->data.convStatPhysToVirt.physAddr - g_FVOS_SharedAddrDiff;

  //return FVOS_OK;
//}



/**
 * @brief     This function returns address of the stored time value
 *
 * @param[out] timeAddress_ptr - address of the stored time value
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleGetTimeCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /* time value */
  //DxInt32_t    timeVal;

  /*-----------------
      CODE
  ---------------------*/

  //timeVal = 0;

//#if defined DX_ARM_INTEGRATOR_DEBUG | DX_PPC_INTEGRATOR_DEBUG
  /* get the value from the RT clock */
  //timeVal = time(DX_NULL);
//#endif

  //if(timeVal == -1)
  //{
    //return FVOS_SYS_CALL_FAIL_ERR;
  //}

  /* store the time at the beginning of the pool - first set the secure token */
  //*(DxUint32_t*)g_FVOS_SystemDataMemoryAreaAddress = FVOS_TIME_VAL_TOKEN;

  /* set the time */
  //*(DxUint32_t*)(g_FVOS_SystemDataMemoryAreaAddress + sizeof(DxUint32_t)) = (DxUint32_t)timeVal;

  //cmdData_ptr->data.getTime.timeAddress = g_FVOS_SystemDataMemoryAreaAddress + g_FVOS_SharedAddrDiff;

  //cmdData_ptr->data.getTime.timeValue = (DxUint32_t)timeVal;

  //return FVOS_OK;
//}

/**
 * @brief     This function checks the configuration
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleCheckConfigurationCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /*------------------
      CODE
  ----------------------*/

  /*if(cmdData_ptr->data.checkConfigData.hostSharedAreaSize < (FVOS_MESSAGE_MEMORY_SIZE_IN_BYTES + FVOS_STATIC_MEMORY_SIZE_IN_BYTES + FVOS_SYSTEM_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES + FVOS_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES + FVOS_SYSTEM_DATA_MEMORY_SIZE_IN_BYTES ))
  {
    return FVOS_SHARED_AREA_UNSUFFICENT_SIZE_ERR;
  }

  return FVOS_OK;
}*/

/**
 * @brief     This function handles the realloc cache and resident command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
/*static FVOS_Error_t FVOS_HandleReallocCacheResidentCmd(FVOS_Cntrl_info_t* cmdData_ptr)
{
  DxUint32_t regVal;*/
  /*-----------------------------
      CODE
  --------------------------------*/
//#ifdef DX_PPC_INTEGRATOR_DEBUG
  //DxUint32_t virAddres;

//#endif

  /*Check that the CC_INIT was not called yet*/
  /*FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR ,regVal );
  if(regVal != 0x2)
  {
      return FVOS_CC_INIT_SECOND_CALL_ERR;
  }

#ifdef DX_PPC_INTEGRATOR_DEBUG

virAddres = g_FVOS_ResidentAddr;

 DX_VOS_FastMemCpy((DxUint8_t*)virAddres , (DxUint8_t*)cmdData_ptr->data.reallocCachResident.residentAddr ,
                   cmdData_ptr->data.reallocCachResident.residentSize);
 cmdData_ptr->data.reallocCachResident.newResidentAddr = virAddres + g_FVOS_SharedAddrDiff;

 virAddres  = g_FVOS_ResidentAddr + (1024*8);

 DX_VOS_FastMemCpy((DxUint8_t*)virAddres , (DxUint8_t*)cmdData_ptr->data.reallocCachResident.cacheAddr ,
                   cmdData_ptr->data.reallocCachResident.cacheSize);

 cmdData_ptr->data.reallocCachResident.newCacheAddr = virAddres + g_FVOS_SharedAddrDiff;
 cmdData_ptr->data.reallocCachResident.newDCacheAddr = virAddres + g_FVOS_SharedAddrDiff + (520*1024) ;

#else
  cmdData_ptr->data.reallocCachResident.newCacheAddr = cmdData_ptr->data.reallocCachResident.cacheAddr;
  cmdData_ptr->data.reallocCachResident.newResidentAddr = cmdData_ptr->data.reallocCachResident.residentAddr;
  cmdData_ptr->data.reallocCachResident.newDCacheAddr = cmdData_ptr->data.reallocCachResident.dcacheAddr;

#endif

  cmdData_ptr->data.reallocCachResident.newBaseAddr = cmdData_ptr->data.reallocCachResident.baseAddr;
  cmdData_ptr->data.reallocCachResident.newSharedAreaAddr = g_FVOS_SharedAreaAddress + g_FVOS_SharedAddrDiff;
  return  FVOS_OK;
}*/

/**
 * @brief     This function handles the realloc command of the external cache
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleReallocExtCacheCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /*-----------------------------
      CODE
  --------------------------------*/

  /*cmdData_ptr->data.reallocExtCache.newCacheAddr = cmdData_ptr->data.reallocExtCache.cacheAddr;

  return  FVOS_OK;
}*/

/**
 * @brief     This function handles theSEP Init command
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_HandleSEPInitCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  /* error */
  //FVOS_Error_t  error;

  /* register value */
  //DxUint32_t    regVal;

  /* counter */
  //DxUint32_t    counter;

  /* pointer to message */
  //DxUint32_t*   message_ptr;

  /*-----------------------
      CODE
  -------------------------*/

  //error  = FVOS_OK;

  //message_ptr = (DxUint32_t*)cmdData_ptr->data.sepInit.messageAddr;

  /* set the base address of the SRAM  */

  //FVOS_WriteRegister(GEN_HW_SRAM_ADDR_REG_ADDR , cmdData_ptr->data.sepInit.SepSramAddr );

  //for( counter = 0 ; counter < cmdData_ptr->data.sepInit.messageSizeInWords ; counter++ )
  //{
    /* write data to SRAM */
    //FVOS_WriteRegister( GEN_HW_SRAM_DATA_REG_ADDR, message_ptr[counter] );

		/* wait for write complete */
    //FVOS_WAIT_SRAM_WRITE_COMPLETE();
  //}

  /* signal SEP */
  //FVOS_WriteRegister(GEN_HW_HOST_HOST_SEP_GPR0_REG_ADDR ,0x1 );

  /*do
  {
    FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal);
  }while(!(regVal & 0xFFFFFFFD));*/

  /* check the value */
//  if(regVal == 0x1)
  //{

    /* fatal error - read erro status from GPRO */
    //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR , error);
//	return error;
  //}
  /* Clear GPR3 */
  //FVOS_WriteRegister(GEN_HW_HOST_HOST_SEP_GPR0_REG_ADDR ,0x10 );

  /*do
  {
	  FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal);
  }while(regVal != 0x0);

#ifndef LITTLE__ENDIAN*/
//    FVOS_WriteRegister( GEN_HW_HOST_HOST_ENDIAN_REG_ADDR, 0x1/*BE*/);
//#endif
  //return 0x0;
//}





/**
 * @brief     This function constructs the lli table from the input of the array of physical addresses
 *            sizes for each physical buffer ,  and the number of physical buffer. Function uses the the VOS function for allocation of the
 *            memory needed for the
 *
 *
 * @param[in] inputPhysicalBuffersArray - input physical buffers and their size
 * @param[out] firstInputLLITable_ptr - holds pointer to the first lli table in the list
 * @param[out] numEntriesInTable_ptr - number of entries in the first lli table
 *
 * @return     DxError_t:
 *
 */
//static FVOS_Error_t FVOS_PrepareInputDMATable(DxUint32_t   dataInAddr,
                                              //DxUint32_t   dataInSize,
                                              //DxUint32_t*  firstInputLLITable_ptr,
                                              //DxUint32_t*  numEntriesInTable_ptr,
                                              //DxUint32_t*  firstTableDataSize_ptr)
//{
  /* lli entry ptr */
  //FVOS_LliEntry_t*    lliEntryPtr;

  /* error */
  //FVOS_Error_t        error;

  /* table size */
  //DxUint32_t          tableSize;


  /*------------------------
      CODE
  --------------------------*/
//#ifdef DX_PPC_INTEGRATOR_DEBUG
  /* copy the data to the board DMA area */
  //DX_VOS_FastMemCpy((DxUint8_t*)g_FVOS_DMADataInAreaAddr , (DxUint8_t*)dataInAddr, dataInSize);
  //dataInAddr = (DxUint8_t*)g_FVOS_DMADataInAreaAddr;
  //g_FVOS_DMAUserDataOutAreaAddr = 0;
//#else

//#ifdef DX_USE_BOARD_DMA_AREA
  /* copy the data to the board DMA area */
  //DX_VOS_FastMemCpy((DxUint8_t*)0xc2000000 , (DxUint8_t*)dataInAddr, dataInSize);
  //dataInAddr = 0xc2000000;
  //outMemAddress = 0;
//#endif
 //#endif

  /* init variables */
 // error = FVOS_OK;

  /* in NoOs each table has only 2 entries */
 // tableSize = sizeof(FVOS_LliEntry_t) * 2;

  //lliEntryPtr = (FVOS_LliEntry_t*)(g_FVOS_DMATableAreaAddress + (g_FVOS_DMATablesCreated * tableSize));

  /* set the address and size */

//#ifndef LITTLE__ENDIAN
  //lliEntryPtr->physicalAddress = DX_GEN_CHANGE_WORD_ENDIANNESS ((DxUint32_t)dataInAddr + g_FVOS_SharedAddrDiff);
  //lliEntryPtr->blockLength = DX_GEN_CHANGE_WORD_ENDIANNESS(dataInSize);
//#else
  //lliEntryPtr->physicalAddress = (DxUint32_t)dataInAddr + g_FVOS_SharedAddrDiff;
  //lliEntryPtr->blockLength = dataInSize;
//#endif

  //lliEntryPtr++;

  /* set the last info entry */
  //lliEntryPtr->blockLength = 0x0;
  //lliEntryPtr->physicalAddress = 0xffffffff;

  /* this is the first table - set the output paramater */
 // *firstInputLLITable_ptr = g_FVOS_DMATableAreaAddress + g_FVOS_SharedAddrDiff;

  /* num entries should include the last entry that points to the next table */
//  *numEntriesInTable_ptr = 2;

  //*firstTableDataSize_ptr = dataInSize;

  /* set number of already created tables */
  //g_FVOS_DMATablesCreated += 1;

  //return error;

//}


/**
 * @brief     This function creates tables for symmetric engines (AES,DES). The size of data in each
 *            table is modulud the operationBlockSize
 *
 *
 * @param[in] inputPhysicalBuffersArray - physical memory buffers for input
 * @param[in] outputPhysicalBuffersArray - physical memory buffers for output
 * @param[in] operationBlockSize - block size of the operation (AES or DES)
 * @param[out] firstInputLLITable_ptr - pointer to the first table in the list of input tables
 * @param[out] firstOutputLLITable_ptr - pointer to the first table in the list of output tables
 * @param[out] numEntriesInTable_ptr - number of entries in the first input/output tables
 *
 * @return     DxError_t:
 *
 */
/*static FVOS_Error_t FVOS_PrepareInputOutputDMATables(DxUint32_t   dataInAddr,
                                                     DxUint32_t   dataOutAddr,
                                                     DxUint32_t   dataInSize,
                                                     DxUint32_t   operationBlockSize,
                                                     DxUint32_t*  firstInputLLITable_ptr,
                                                     DxUint32_t*  firstOutputLLITable_ptr,
                                                     DxUint32_t*  numInTableEntries_ptr,
                                                     DxUint32_t*  numOutTableEntries_ptr,
                                                     DxUint32_t*  firstTableDataSize_ptr)
{*/
  /* pointer to allocated LLI input table */
  //FVOS_LliEntry_t*          inputLLITable_ptr;

  /* pointer to allocated LLI output table */
  //FVOS_LliEntry_t*          outputLLITable_ptr;

  /* table size */
  //DxUint32_t                tableSize;

  /* error */
  //FVOS_Error_t              error = FVOS_OK;

  /*-------------------------------
    CODE
  --------------------------------*/

//#ifdef DX_PPC_INTEGRATOR_DEBUG
  /* copy the data to the board DMA area */
  //DX_VOS_FastMemCpy((DxUint8_t*)g_FVOS_DMADataInAreaAddr , (DxUint8_t*)dataInAddr, dataInSize);
  //dataInAddr = (DxUint8_t*)g_FVOS_DMADataInAreaAddr;
  //g_FVOS_DMAUserDataOutAreaAddr = dataOutAddr;
  //g_FVOS_DMADataOutAreaAddr = dataInAddr + dataInSize;
  //dataOutAddr = dataInAddr + dataInSize;
  //g_FVOS_DMADataSize = dataInSize;

//#else
//#ifdef DX_USE_BOARD_DMA_AREA
  /* copy the data to the board DMA area */
  //DX_VOS_FastMemCpy((DxUint8_t*)0xc2000000 , (DxUint8_t*)dataInAddr, dataInSize);
  //dataInAddr = 0xc2000000;

  //outMemAddress = dataOutAddr;
  //outMemDataSize = dataInSize;
  //dmaOutputAddress = dataInAddr + dataInSize;

  //dataOutAddr = dataInAddr + dataInSize;
//#endif
//#endif

  /* in NoOs each table has only 2 entries */
  //tableSize = sizeof(FVOS_LliEntry_t) * 2;

  /* allocate input and output tables */
  /*inputLLITable_ptr = (FVOS_LliEntry_t*)(g_FVOS_DMATableAreaAddress + (g_FVOS_DMATablesCreated * tableSize));
  outputLLITable_ptr = (FVOS_LliEntry_t*)( ((DxUint32_t)inputLLITable_ptr) + tableSize);
  */
  /* set the fields for the output */
  //*firstInputLLITable_ptr = (DxUint32_t)inputLLITable_ptr;
  //*firstOutputLLITable_ptr = (DxUint32_t)outputLLITable_ptr;


  /* set the input table parameters */
/*#ifndef LITTLE__ENDIAN
  inputLLITable_ptr->physicalAddress = DX_GEN_CHANGE_WORD_ENDIANNESS ((DxUint32_t)dataInAddr + g_FVOS_SharedAddrDiff);
  inputLLITable_ptr->blockLength = DX_GEN_CHANGE_WORD_ENDIANNESS (dataInSize);
#else
  inputLLITable_ptr->physicalAddress = (DxUint32_t)dataInAddr + g_FVOS_SharedAddrDiff;
  inputLLITable_ptr->blockLength = dataInSize;
#endif
  inputLLITable_ptr++;
  */
  /* set the last info entry */
  //inputLLITable_ptr->blockLength = 0x0;
  //inputLLITable_ptr->physicalAddress = 0xffffffff;

  /* set the output table parameters */
 /*#ifndef LITTLE__ENDIAN
  outputLLITable_ptr->physicalAddress = DX_GEN_CHANGE_WORD_ENDIANNESS((DxUint32_t)dataOutAddr + g_FVOS_SharedAddrDiff);
  outputLLITable_ptr->blockLength = DX_GEN_CHANGE_WORD_ENDIANNESS(dataInSize);
#else
  outputLLITable_ptr->physicalAddress = (DxUint32_t)dataOutAddr + g_FVOS_SharedAddrDiff;
  outputLLITable_ptr->blockLength = DX_GEN_CHANGE_WORD_ENDIANNESS(dataInSize);
#endif
  outputLLITable_ptr++;
  */
  /* set the last info entry */
  //outputLLITable_ptr->blockLength = 0x0;
  //outputLLITable_ptr->physicalAddress = 0xffffffff;

  /* set the number of entries in the input/output table and the data size of the first table */
  /**numInTableEntries_ptr = 2;
  *numOutTableEntries_ptr = 2;
  *firstTableDataSize_ptr = dataInSize;

  */
  /* set the offset for the next table */
  /*g_FVOS_DMATablesCreated += 2;

  *firstInputLLITable_ptr += g_FVOS_SharedAddrDiff;
  *firstOutputLLITable_ptr += g_FVOS_SharedAddrDiff;

  return error;
}*/

//#ifndef DX_PPC_INTEGRATOR_DEBUG
//#if !defined(DX_USE_BOARD_DMA_AREA) && !defined (DSM_SIM)

/* scatter working stack initialization */
//__value_in_regs struct __initial_stackheap __user_initial_stackheap(

  //      unsigned R0, unsigned SP, unsigned R2, unsigned SL)

//{

  //  struct __initial_stackheap config;

    //R0 = R0;

    //R2 = R2;

    //SL = SL;

    //config.heap_base = ((unsigned int)&Image$$DATA_SECTION$$ZI$$Limit) +  (1024 * 16); // defined in heap.s

    //config.stack_base = SP;   // inherit sp from the execution environment

//#ifndef DX_NO_WMDRM_SUPPORT
	/*These 2 sections are allocated at the end of the end of the DMA access area.*/
	/*The heap pointer is at the begining of the area (right after the cache) and */
	/*the stack pointer is at the end of the memory */
  //  config.heap_base = ((unsigned int)&Image$$HEAP_FOR_DMA_SECTION$$Base);

    //config.stack_base = ((unsigned int)&Image$$STACK_FOR_DMA_SECTION$$Base);
//#endif
  //  return config;

//}

//#endif
//#endif//#ifdef DX_PPC_INTEGRATOR_DEBUG
/**
 * @brief     This function return the printf buffer used by the SEP,
 *
 *
 * @param[out] printfBuff_ptr - the mapped address will be returned into this pointer
 *
 * @return     DxError_t_t:
 *
 */
//FVOS_Error_t FVOS_GetPrintfBuffer(DxUint32_t* printfBuff_ptr)
//{
  /*-----------------------------
      CODE
  ---------------------------------*/
  /* The printf buffer is located at the end of the shared area for the message (last 256 bytes from 6K) */
  //*printfBuff_ptr = g_FVOS_SharedAreaAddress + 5888; /*5888 = 6K-256*/

  //return FVOS_OK;
//}


/**
 * @brief     This function should retrieve the physical address of the
 *            RAR shared area. Not relevant in NoOs.
 *
 * @param[in/out] cmd - command structure wuth input/output argument
 *
 * @return     DxError_t:
 */
//FVOS_Error_t FVOS_HandleRarCmd(FVOS_Cntrl_info_t* cmdData_ptr)
//{
  //DxUint32_t rarHandleOffset;
  /*-----------------------------
      CODE
  --------------------------------*/

//  cmdData_ptr->data.rar.handle = DX_NULL;

  //rarHandleOffset = (g_FVOS_SharedAreaAddress +
  //					 FVOS_MESSAGE_MEMORY_SIZE_IN_BYTES +
  //					 FVOS_STATIC_MEMORY_SIZE_IN_BYTES +
  //					 FVOS_SYSTEM_DATA_POOL_SHARED_AREA_SIZE_IN_BYTES +
  //				     FVOS_SYNCHRONIC_DMA_TABLES_AREA_SIZE_IN_BYTES +
  //				     FVOS_TIME_MEMORY_SIZE_IN_BYTES +
  //				     sizeof(DxUint32_t) /* RAR tagging */);

  //DX_GEN_WriteRegister( rarHandleOffset, cmdData_ptr->data.rar.handle );

  //return  FVOS_OK;
//}

//FVOS_Error_t FVOS_StartRom(FVOS_Cntrl_info_t* cmdData_ptr)
//{

  //FVOS_Error_t     Error = FVOS_OK;

  /* reg val */
  //DxUint32_t    regVal;

  /* warning */
  //DxUint32_t    warning;

  //DxUint32_t CCRes = DX_CC_START_IllaglResult_t;
  /*-----------------------------
    CODE
  ------------------------------*/

  //warning = FVOS_OK;
  /* wait in polling for message from SEP */
  //do// ROM_START
  //{
    //FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal);
  //}while(!regVal);

  //switch(regVal)
  //{
  //	case 0x1:
	    /* fatal error - read erro status from GPRO */
//	    FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR , Error);
//		CCRes = DX_CC_START_fatalError_t;
//		break;
  //	case 0x2:
	    /* Boot First Phase ended  */
//	    FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR , warning);
//		if(!warning)
//		{
//		    CCRes = DX_CC_START_ColdBootFirstPhaseComplete_t;
//		}
//		else
//		{
//		    CCRes = DX_CC_START_ColdBootFirstPhaseCompleteWithWarning_t;
//		}
//		Error = FVOS_OK;
//		break;
  //	case 0x4:
	    /* Cold boot ended successfully  */
//	    FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR , warning);
//		if(!warning)
//		{
//		    CCRes = DX_CC_START_ColdBootComplete_t;
//		}
//		else
//		{
//		    CCRes = DX_CC_START_ColdBootCompleteWithWarning_t;
//		}
//		Error = FVOS_OK;
//		break;
  //	case 0x8:
	    /* Warmboot ended successfully */
/*	    FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR , warning);
		if(!warning)
		{
		    CCRes = DX_CC_START_WarmBootComplete_t;
		}
		else
		{
		    CCRes = DX_CC_START_WarmBootCompleteWithWarning_t;
		}
		Error = FVOS_OK;*/
		/* Clear GPR3 */
/*		FVOS_WriteRegister(GEN_HW_HOST_HOST_SEP_GPR0_REG_ADDR ,0x10 );
		do
		{
			FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal);
		}while(regVal != 0x0);

		break;
	case 0x10:*/
	    /* ColdWarm boot ended successfully */
	    /*FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR0_REG_ADDR , warning);
		if(!warning)
		{
		    CCRes = DX_CC_START_ColdWarmBootComplete_t;
		}
		else
		{
		    CCRes = DX_CC_START_ColdWarmBootCompleteWithWarning_t;
		}
		Error = FVOS_OK;*/
		/* Clear GPR3 */
/*		FVOS_WriteRegister(GEN_HW_HOST_HOST_SEP_GPR0_REG_ADDR ,0x10 );
		do
		{

			FVOS_ReadRegister(GEN_HW_HOST_SEP_HOST_GPR3_REG_ADDR , regVal);
		}while(regVal != 0x0);
		break;
	case 0x20:*/
	    /* BM error  */
		/*CCRes = DX_CC_START_ICacheFailure_t;
		Error = FVOS_OK;
		break;
	case 0x40:*/
	    /* BM error  */
/*		CCRes = DX_CC_START_DCacheFailure_t;
		Error = FVOS_OK;
		break;
  }

	cmdData_ptr->data.romStartResult.warningVal = warning;
	cmdData_ptr->data.romStartResult.CCResult = CCRes;

  return Error;
}

#if defined DX_PPC_INTEGRATOR_DEBUG | (!DX_TST_OS)

#if defined(DX_ARM_INTEGRATOR_DEBUG) | DX_PPC_INTEGRATOR_DEBUG*/
  /* CRYS_PIC_ROM data */
//   static DxUint32_t CRYS_SEP_ROM[] = {
  //										#include "SEP_ROM_image.h"
   //									 };

   //static DxUint32_t OTP_VALUES[] = {
   //										#include "OTP_VALUES.h"
   //									 };

/**
 * @brief This function initializes the CRYS SW for the testing platform.
 *
 *  The function should be called before the CRYS_INIT function.
 *
 * @param[in] - HwBaseAddress the hardware base address.

 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in ...
 */
//static FVOS_Error_t FVOS_TST_Init()
//{
//#ifdef DX_ARM1176
  /* FUNCTION LOGIC */
  //FVOS_WriteRegister(0x10000020,0x0000a05f); // Unlock
  //FVOS_WriteRegister(0x10000014,0xbc5c); /* 5M*/
  //FVOS_WriteRegister(0x10000020,0x00000000); // lock
//#else
//#ifndef DX_PPC_INTEGRATOR_DEBUG
  //FVOS_WriteRegister(HOST_SC_LOC,0xA05F);
  //FVOS_WriteRegister(HOST_SC_OSC,CLK_10MHZ);
//#endif // DX_PPC_INTEGRATOR_DEBUG
//#endif /* DX_ARM1176 */

 // FVOS_TST_OTPBurning();
  //FVOS_TST_SepRomLoader();
  //FVOS_TST_OTPRead();

  //return;
//}



/*****************SepRomPlatform functions for all kind of boards***************************/

//#ifdef DX_PPC_INTEGRATOR_DEBUG
//static void FVOS_TST_SepRomLoader (void)
//{
  /* Index variables */
  //DxUint32_t i, k,j;
  //DxUint32_t  regVal;
  /* Loading ROM from SEP_ROM_image.h file */
  //k = sizeof(CRYS_SEP_ROM);

  //for(i = 0;i < 4; i++)
  //{
      /* write bank */

  //	  FVOS_WriteRegister(SEP_ROM_BANK_register ,i);
  /*
	  for(j = 0;j < CRYS_SEP_ROM_length / 4;j++)
	  {

	      FVOS_WriteRegister(CRYS_SEP_ROM_start_address + 4*j, CRYS_SEP_ROM[i * 0x1000 + j]);
		  k= k - 4;
		  if(k==0)
	{
          j= CRYS_SEP_ROM_length;
		i=4;
        }
	  }
  }*/

  /* reset the SEP*/
/*  FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x0);
  FVOS_WriteRegister(ENV_CC_RST_N_REG_address,0x1);
  FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x1);

  return;
}

#else
#ifndef DX_ARM1176
static void FVOS_TST_SepRomLoader (void)
{*/
  /* Index variables */
  //DxUint32_t i, k;
  /* Loading ROM from SEP_ROM_image.h file */
  //k = sizeof(CRYS_SEP_ROM);

  //FVOS_WriteRegister(CRYS_SEP_ROM_control_address ,0x80000000);

	/* Write ROM data  to RAM */
  //for(i = 0; i <  CRYS_SEP_ROM_length ; i++)
  //{
    //FVOS_WriteRegister(CRYS_SEP_ROM_start_address + 4*i, CRYS_SEP_ROM[i]);

    /*decrese the data size*/
    //k= k - 4;
    //if(k==0)
    //{
      //i= CRYS_SEP_ROM_length;
    //}
  //}
  //FVOS_WriteRegister(CRYS_SEP_ROM_control_address ,0x0);
  /* reset the SEP*/

  //FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x0);
  //FVOS_WriteRegister(ENV_CC_RST_N_REG_address,0x1);
  //FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x1);

  //return;
//}

//#else

//#ifndef DX_ARM1176_CC5

//static void FVOS_TST_SepRomLoader (void)
//{
  /* Index variables */
  //DxUint32_t i, k,j;
  //DxUint32_t  regVal;

  /* Loading ROM from SEP_ROM_image.h file */
  //k = sizeof(CRYS_SEP_ROM);

  /*for(i = 0;i < 2; i++)
  {*/
      /* write bank */
  /*	  FVOS_WriteRegister(SEP_ROM_BANK_register ,i);

	  for(j = 0;j < CRYS_SEP_ROM_length / 2;j++)
	  {
	      FVOS_WriteRegister(CRYS_SEP_ROM_start_address + 4*j, CRYS_SEP_ROM[i * 0x1000 + j]);
		  k= k - 4;
		  if(k==0)
	{
          j= CRYS_SEP_ROM_length;
		i=4;
        }
	  }
  }*/

  /* reset the SEP*/
  //FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x0);
  //FVOS_WriteRegister(ENV_CC_RST_N_REG_address,0x1);
  //FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x1);

  //return;
//}

//#else

/*static void FVOS_TST_SepRomLoader (void)
{*/
  /* Index variables */
  //DxUint32_t i, k,j;
  //DxUint32_t  regVal;

  /* Loading ROM from SEP_ROM_image.h file */
  //k = sizeof(CRYS_SEP_ROM);

  //for(i = 0;i < 4; i++)
  //{
      /* write bank */
  //	  FVOS_WriteRegister(SEP_ROM_BANK_register ,i);

  //	  for(j = 0;j < CRYS_SEP_ROM_length / 4;j++)
  //	  {
//	      FVOS_WriteRegister(CRYS_SEP_ROM_start_address + 4*j, CRYS_SEP_ROM[i * 0x1000 + j]);
  //		  k= k - 4;
  //		  if(k==0)
    //		{
      //    j= CRYS_SEP_ROM_length;
        //	i=4;
        //}
	 // }
  //}

  /* reset the SEP*/
  //FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x0);
  //FVOS_WriteRegister(ENV_CC_RST_N_REG_address,0x1);
  //FVOS_WriteRegister(ENV_CC_POR_N_ADDR_REG_address,0x1);

  //return;
//}

//#endif //DX_ARM1176_CC5

//#endif //DX_ARM1176
//#endif // DX_PPC_INTEGRATOR_DEBUG
//#endif // DX_PPC_INTEGRATOR_DEBUG
//#endif

//#if defined DX_ARM_INTEGRATOR_DEBUG | DX_PPC_INTEGRATOR_DEBUG
//static void FVOS_TST_OTPBurning(void)
//{

  /*  DxUint32_t i;

    for (i = 0; i <sizeof(OTP_VALUES)/4;i++)
    {
        FVOS_WriteRegister(DX_CC_BASE_ADDR + 4*i, OTP_VALUES[i]);
    }


  return;
}


#define FVOS_HW_READ_BLOCK_FROM_SRAM(Addr , ptr , SizeWords ) \
do \
{ \
   DxUint32_t ii; \
   volatile DxUint32_t dummy; \
   FVOS_WriteRegister( GEN_HW_SRAM_ADDR_REG_ADDR ,Addr ); \
   FVOS_ReadRegister( GEN_HW_SRAM_DATA_REG_ADDR, dummy ); \
   for( ii = 0 ; ii < SizeWords ; ii++ ) \
   { \
      do \
      { \
         FVOS_ReadRegister( GEN_HW_SRAM_DATA_READY_REG_ADDR, dummy ); \
      }while(!(dummy & 0x1)); \
      FVOS_ReadRegister( GEN_HW_SRAM_DATA_REG_ADDR, (ptr)[ii] ); \
   } \
   do \
   { \
      FVOS_ReadRegister( GEN_HW_SRAM_DATA_READY_REG_ADDR, dummy ); \
   }while(!(dummy & 0x1)); \
}while(0)

#define INTEG_SRAM_BASE_ADDR 0xF00
 void FVOS_TST_OTPRead(void)
{

    DxUint32_t buff[64];

	FVOS_HW_READ_BLOCK_FROM_SRAM(INTEG_SRAM_BASE_ADDR,buff,64);

  return;
}*/

//#endif
/*****************End of SepRomPlatform functions ***************************/
