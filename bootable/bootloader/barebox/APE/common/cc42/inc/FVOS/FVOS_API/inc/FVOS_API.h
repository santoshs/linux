/***********************************************************************************  
 * Copyright 2009 © Discretix Technologies Ltd. 
 * Copyright 2011 (C) Renesas Mobile Corp. All rights reserved.
 * This software is protected by copyright, international treaties and 
 * various patents. If the license governing the use of this Software 
 * allows copy or redistribution of this  software then any copy or 
 * reproduction of this Software must include this Copyright Notice 
 * as well as any other notices provided under such license. 
 ***********************************************************************************/
 
 
#ifndef _FVOS_API_H_
#define _FVOS_API_H_

#include "FVOS_Types.h"
/*
   *  Object %name    : FVOS_API.h
   *  State           :  %state%
   *  Creation date   :  Wed Nov 17 17:39:24 2004
   *  Last modified   :  %modify_time%
   */
  /** @file
   *  \brief FVOS h file (API and structures )
   *
   *  \version 
   *  \author yevgenys
   */


/*------------------------------
    DEFINES
--------------------------------*/


/* define for infinite timeout */
#define FVOS_TIMEOUT_INFINITE                   0xFFFFFFFF

/* source of the interrupt is SEP reply to HOST message */
#define FVOS_SEP_REPLY                          0x1

/* source interrupt is SEP request from HOST */
#define FVOS_SEP_REQUEST                        0x2

/* source interrupt is SEP request from HOST */
#define FVOS_SEP_PRINTF                         0x4

/*-------------------------------
  STRUCTURES
---------------------------------*/




typedef enum
{
  /* array si for input data */
  FVOS_InputArray = 0,
  
  /* array si for output data */
  FVOS_OutputArray = 1,
  
  /* last token */
  FVOS_InputArrayFlagLast = 0x7fffffff,
}FVOS_InputArrayFlag_t;

/* data structure for the FVOS_Cntrl command */
typedef struct _FVOS_Cntrl_data_t
{
  union
  {
    struct
    {
      /* size of the shared area */
      DxUint32_t    hostSharedAreaSize;
    }checkConfigData;
  
    struct
    {
      /* the address where the rime is stored */
      DxUint32_t    timeAddress;
      
      /* time value */
      DxUint32_t	timeValue;
    }getTime;
  
    /* allocate memory command */
    struct
    {
      /* address of the application pointer */
      DxUint32_t  appAddr;
      
      /* address of the allocated memory in the fvos */
      DxUint32_t  virtAddr;
      
      /* physical address */
      DxUint32_t  physAddr;
      
      /* size of the memory to allocate in bytes */
      DxUint32_t  allocSize;
    }allocCmdData;
    
    struct
    {
      /* address of the application data in pointer */
      DxUint32_t  srcAppAddr;
      
      /* address of the allocated memory in the fvos */
      DxUint32_t  destFvosAddr;
      
      /* size of data to write in bytes */
      DxUint32_t  dataSize;

  	  /* indicates that the data is a byte array and endianess should be swapped */
	  DxUint32_t  bytesArrayFlag;
    }writeCmdData;
    
    struct
    {
      /* address of the application to where to read */
      DxUint32_t  destAppAddr;
      
      /* address of the allocated memory in the fvos from where to read */
      DxUint32_t  srcFvosAddr;
      
      /* size of data to read in bytes */
      DxUint32_t  dataSize;

  	  /* indicates that the data is a byte array and endianess should be swapped */
	  DxUint32_t  bytesArrayFlag;
    }readCmdData;
    
    struct
    {
      /* address value of the data in */ 
      DxUint32_t  dataInAddr;
      
      /* size of data in */
      DxUint32_t  dataInSize;
      
      /* address of the data out */
      DxUint32_t  dataOutAddr;
      
      /* the size of the block of the operation - if needed, every table will be modulo this parameter */
      DxUint32_t  blockSize;
      
      /* the physical address of the first input DMA table */
      DxUint32_t  firstInTableAddr;
      
      /* number of entries in the first input DMA table */
      DxUint32_t  firstInTableNumEntries;
      
      /* the physical address of the first output DMA table */
      DxUint32_t  firstOutTableAddr;
      
      /* number of entries in the first output DMA table */
      DxUint32_t  firstOutTableNumEntries;
      
      /* data in the first input table */
      DxUint32_t  firstTableDataSize;
      
    }buildSymDMATables;
    
    struct
    {
      /* flow type */
      DxUint32_t              flowType;
      
      /* address value of the data in */ 
      DxUint32_t              buffersArrayAddr;
      
      /* size of data in */
      DxUint32_t              buffersArraySize;
      
      /* input or output flag */
      FVOS_InputArrayFlag_t   inputOutputFlag;
      
      /* the physical address of the first input DMA table */
      DxUint32_t              firstInTableAddr;
      
      /* number of entries in the first input DMA table */
      DxUint32_t              firstInTableNumEntries;
      
      /* data in the first input table */
      DxUint32_t              firstTableDataSize;
    }buildFlowDMATables;
    
    struct
    {
      /* flow id  */
      DxUint32_t  flowId;
      
      /* flag for input output */
      DxUint32_t  inputOutputFlag;
      
      /* address value of the data in */ 
      DxUint32_t  buffersArrayAddr;
      
      /* size of data in */
      DxUint32_t  buffersArraySize;
      
      /* address of the first table */
      DxUint32_t  firstTableAddr;
      
      /* number of entries in the first table */
      DxUint32_t  firstTableNumEntries;
      
      /* data size of the first table */
      DxUint32_t  firstTableDataSize;
      
    }addTables;
    
    struct
    {
      /* flow id  */
      DxUint32_t  flowId;
      
      /* message address */
      DxUint32_t  messageAddress;
      
      /* message size in words */
      DxUint32_t  messageSizeInWords;
      
    }addTablesMessage;
    
    struct
    {
      /* flow type */
      DxUint32_t  flowId;
      
    }setFlowId;
    
    struct
    {
      /* physical address */
      DxUint32_t  physAddr;
      
      /* fvos address */
      DxUint32_t  fvosAddr;
      
    }staticPoolInfo;
    
    struct
    {
      /* physical address */
      DxUint32_t  physAddr;
      
      /* virt address */
      DxUint32_t  virtAddr;
      
    }convStatPhysToVirt;
    
    struct
    {
      /* blocking mode */
      DxUint32_t  blockingMode;
    }setAPIMode;
    
    struct
    {
      /* flow id */
      DxUint32_t  flowId;
    }freeFlowResources;
    
    struct
    {
      /* register virtual address */
      DxUint32_t  regVirtAddr;
      
      /* register data (read/write) */
      DxUint32_t  regData;
      
    }regDataOp;
    
    struct 
    {
      /* address of the message */
      DxUint32_t  messageAddr;
      
      /* message size in words */
      DxUint32_t  messageSizeInWords;
      
      /* Base Address of the SEP SRAM */
      DxUint32_t  SepSramAddr;
    }sepInit;
    
    struct
    {
      /* base address */
      DxUint32_t  baseAddr;
    
      /* current cache address */
      DxUint32_t  cacheAddr;
      
      /* cache size */
      DxUint32_t  cacheSize;
      
      /* resident address */
      DxUint32_t  residentAddr;
      
      /* resident size */
      DxUint32_t  residentSize;

      /* current extended cache address */
      DxUint32_t  extCacheAddr;
      
      /* extended cache size */
      DxUint32_t  extCacheSize;

      /* resident address */
      DxUint32_t  dcacheAddr;
      
      /* dcache size */
      DxUint32_t  dcacheSize;
      
      /* new address of the resident */
      DxUint32_t  newResidentAddr;
      
      /* new address of the cache */
      DxUint32_t  newCacheAddr;
      
      /* new dcache address */
      DxUint32_t  newDCacheAddr;
      
      /* new address of the shared area */
      DxUint32_t  newSharedAreaAddr;
      
      /* new base address */
      DxUint32_t  newBaseAddr;
          
    }reallocCachResident;
    
    struct
    {
      /* current cache address */
      DxUint32_t  cacheAddr;
      
      /* cache size */
      DxUint32_t  cacheSize;

      /* new address of the external cache */
      DxUint32_t  newCacheAddr;

    }reallocExtCache;
    
    struct
    {
      /* pid */
      DxUint32_t  pid;
      
      /* virtual address of the caller id */
      DxUint32_t  callerIdAddress;
      
      /* caller id size in bytes */
      DxUint32_t  callerIdSizeInBytes;
      
    }setCallerId;
    
  	struct
  	{
  		/* type that specifies different kinds of RAR regions that could be set up */
  		DxUint32_t rarType;		

  		/* the size of the RAR buffer */
  		DxUint32_t rarBuffSize;

  		/* handle that can be used to refer to reserved block */
  		void* handle;

		/* buffer physical address */
  		DxUint32_t physical_address;
  		
  	}rar;
    
	struct
  	{
  		DxUint32_t CCResult;
		
		DxUint32_t  warningVal;
  		
  	}romStartResult;
  }data;
  
}FVOS_Cntrl_info_t;


typedef enum
{
  /* send message notification to sep */
  FVOS_CNTRL_SEND_MSG_RDY_CMD       = 1,            

  /* send reply(NVS) message notification to sep */
  FVOS_CNTRL_SEND_RPLY_MSG_RDY_CMD ,

  /* allocate memory command id */
  FVOS_CNTRL_ALLOC_MEM_CMD ,

  /* write data to memory command */
  FVOS_CNTRL_WRITE_DATA_CMD ,

  /* read data from memory command */
  FVOS_CNTRL_READ_DATA_CMD,
  
  /* build DMA LLI tables for symmetric APIs(HASH, AES, DES etc' ) command */
  FVOS_CNTRL_BUILD_SYM_DMA_TABLES_CMD,
  
  /* build DMA LLI table for FLOWs command */
  FVOS_CNTRL_BUILD_FLOW_DMA_TABLES_CMD,
  
  /* add flow dma tables */
  FVOS_CNTRL_ADD_FLOW_DMA_TABLES_CMD,
  
  /* add flow tables message */
  FVOS_CNTRL_ADD_FLOW_TABLE_MSG_CMD,
  
  /* set flow id for the newly created flow */
  FVOS_CNTRL_SET_FLOW_ID_IN_TABLES_CMD,
  
  /* frees all the resources of the flow */
  FVOS_CNTRL_FREE_FLOW_RESOURSES,
  
  /* free resourses used for DMA execution */
  FVOS_CNTRL_FREE_DMA_RESOURSES,
  
  /* get the static pool physical and virtual address */
  FVOS_CNTRL_GET_STATIC_POOL_ADDR,
  
  /* convert physical adress of the pool to virtual */
  FVOS_CNTRL_CONV_STATIC_PHYS_TO_VIRT,
  
  /* get current RT time */
  FVOS_CNTRL_GET_TIME,
  
  /* setting the blocking or non-blocking mode of the API's */
  FVOS_CNTRL_SEP_API_MODE,
  
  /* check configuration of the shared area */
  FVOS_CHECK_CONFIGURATION,
  
  /* SEP start operation */
  FVOS_CNTRL_SEP_START,
  
  /* SEP init operation */
  FVOS_CNTRL_SEP_INIT,
  
  /* read register */
  FVOS_READ_PHYS_REGISTER,
  
  /* read register */
  FVOS_WRITE_PHYS_REGISTER,
  
  /* command for reallocation of resident and cache */
  FVOS_REALLOC_CACHE_RESIDENT,
  
  /* command for reallocation of external cache */
  FVOS_REALLOC_EXT_CACHE_RESIDENT,
  
  /* command for preparing the RAR physical output address */
  FVOS_CNTRL_RAR_PREPARE_MESSAGE,
  
  /* sets the caller id data */
  FVOS_SET_CALLER_ID,
  
  /*start rom init*/
  FVOS_START_ROM,
  
  FVOS_Cntrl_cmdLast = 0x7FFFFFFF,

}FVOS_Cntrl_cmd_t;

/*------------------------------------------------
    FUNCTIONS
--------------------------------------------------*/

/**
 * @brief     This functions initializes the FVOS - shared area address
 *
 * @param[in] driver_name_ptr - the sep driver name
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t  FVOS_Init(const DxChar_t *driver_name_ptr);

/**
 * @brief     This functions terminate the FVOS
 *
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t  FVOS_Terminate(void);

/**
 * @brief     This functions configs the FVOS
 *
 * @param[in] sharedAreaAddr - address of the shared area
 * @return     DxError_t_t:  
 *                        
 */
void  FVOS_Config(DxUint32_t sharedAreaAddr);

/**
 * @brief     This functions resets the FVOS registers (IMR)
 *
 * @return      
 *                        
 */
void FVOS_Reset(void);


/**
 * @brief     This functions start the transaction to the SEP - this is the first function to be called during transaction to SEP
 * 
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_StartTransaction(void);


/**
 * @brief     This function end the transaction to SEP - this is the last function in the transaction
 * 
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_EndTransaction(DxUint32_t mappedAddr_ptr);

/**
 * @brief     This functions start the singleton transaction to SEP - once opened, no other singleton transaction
 *            can be opened to SeP until the reboot. Applicable only in OS Linux enviroment
 * 
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_StartSingletonTransaction(void);


/**
 * @brief     This function maps the shared message area to the application memory. It returns a pointer,
 *            trough which application can directly write message data into message shared area
 * 
 * @param[out] mappedAddr_ptr - the mapped address will be returned into this pointer
 *
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_Mmap(DxUint32_t* mappedAddr_ptr);
                                           

/**
 * @brief     This function performs a number of commands according to the CMD paramater:
 *            1) Signal to SEP that the message is ready
 *            2) Allocate memory continues locked memory
 *            3) Read from the allocated memory
 *            4) Write into allocated memory
 *            5) Create DMA LLI tables for symmetric, hash, or FLOW APIs
 *            6) Get adresses of the static pool
 * 
 * @param[in] commandId - the id of command to execute
 * @param[in] cmdData_ptr - pointer to the strucutre that contains the paramaters for command execution
 *
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_Cntrl(FVOS_Cntrl_cmd_t commandId , FVOS_Cntrl_info_t* cmdData_ptr);


/**
 * @brief     This function polls for the for the response from SEP
 *            Polling can be blocking or busy wait,and can be regulated by timeout
 * @param[in] Timeout - timeout for polling
 * @param[out] pollSource_ptr - the source of the poll interrupt (regular response, or MVS request from SEP
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_Poll(DxUint32_t  timeout , DxUint32_t* pollSource_ptr);

/**
 * @brief     This function check if the SEP interrupt bit is up (interrupt was rraised by SEP
 *
 * @return     DX_TRUE - interrput is raised:
 *             DX_FALSE - interrupt is not raised  
 *                        
 */
DxBool_t  FVOS_CheckSEPInterrupt(void);

/**
 * @brief     This function clears the interrupt register, signalling that the interrupt from SEP was handled
 *
 * @return      
 *                        
 */
void  FVOS_ClearSEPInterrupt(void);
 
/**
 * @brief     This function return the printf buffer used by the SEP,
 *            
 * 
 * @param[out] printfBuff_ptr - the mapped address will be returned into this pointer
 *
 * @return     DxError_t_t:  
 *                        
 */
FVOS_Error_t FVOS_GetPrintfBuffer(DxUint32_t* printfBuff_ptr);

#endif
