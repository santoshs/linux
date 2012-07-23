#include "DX_VOS_BaseTypes.h"
#include "cc_hw_interface.h"
#include "host_hw_defs.h"
#include "LLF_COMMON_HwDefs.h"
#include "FVOS_API.h"
#include "error.h"
#include "log_output.h"
#include "DX_VOS_Stdio.h"
//#include <stdio.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>

/*------------------------------
    INTERNAL FUNCTIONS
--------------------------------*/
static DxError_t CC_Hw_InterFace_Map_DMA (
			void	*DataIn_ptr, //_IN_
			int	DataInSize, //_IN_
			int	DstRsrcLLI, //_IN_
			struct	Multiple_LLIs *pMultiple_LLIs); //_IN_OUT_

static DxError_t HostToSramCopy(
    struct LLI_workPlan *pWorkPlan,
    int nPlans,
    unsigned int sramOffset,
    DxUint32_t VirtualHwBaseAddr) ;

 DxUint32_t makeAligned(DxUint32_t DataIn_ptr);

void   LLF_Alignment_Pointer(
                                   DxUint8_t        *DataIn_ptr,
                                   DxUint8_t         *DataOut_ptr,
                                   DxUint32_t         DataInSize);

    

/*------------------------------
    DEFINES
--------------------------------*/
#ifndef DxUint32_t
#define	DxUint32_t  u32
#endif

#ifdef	CRYS_PLAT_SYS_ReadRegister
#undef	CRYS_PLAT_SYS_ReadRegister
#endif
#define CRYS_PLAT_SYS_ReadRegister(addr,val)  ( (val) = (*((volatile DxUint32_t*)(addr))) )

#ifdef	CRYS_PLAT_SYS_WriteRegister
#undef	CRYS_PLAT_SYS_WriteRegister
#endif
#define CRYS_PLAT_SYS_WriteRegister(addr,val)  ( (*((volatile DxUint32_t*)(addr))) = (DxUint32_t)(val) )

/*--------------------------------------
   GLOBAL variables
---------------------------------------*/
/* the opened file descriptor for the device driver */
int     g_FD=0;
//u32 	g_VirtualHwBaseAddr;

/*-------------------------------
  	STRUCTURES
---------------------------------*/
static struct Multiple_LLIs g_Multiple_LLIs ={
	.nValidLLIs = MAX_PAGES_FOR_LLI_WORK_PLAN,
};
/*------------------------------------------------
    FUNCTIONS
--------------------------------------------------*/
/**
 * @brief       This function initializes the cc Driver
 * 
 * @param[in] driver_name_ptr - the driver name string who represent the driver file descriptor
 * @return     DxError_t:  
 *                        
 */
//DxError_t CCDriver_Init(const DxChar_t *driver_name_ptr)
//{
  /*----------------------
      CODE
  -----------------------*/
/* Cancelling lint warning - Ignoring return value of function */
/*lint -save -e{534} */
  //return FVOS_Init(driver_name_ptr);
/*lint -restore */

//}

/**
 * @brief       This function builds DMA
 * 
 * @param[in] driver_name_ptr - the driver name string who represent the driver file descriptor
 * @return     DxError_t:  
 *                        
 */
static DxError_t CC_Hw_InterFace_Map_DMA (
			void	*DataIn_ptr, //_IN_
			int	DataInSize, //_IN_
			int	DstRsrcLLI, //_IN_
			struct	Multiple_LLIs *pMultiple_LLIs) //_IN_OUT_
{
/*----------------------
      CODE
  -----------------------*/
	DxError_t error ;
	int driverError=0, i; 
	struct ioctl_params_dma ioctl_params_dma;

	error = DX_OK;	
       PRINTF("Entered CC_Hw_InterFace_Map_DMA \n");
	//
	// assumed opened already
	//
/*	if(g_FD < 0)
	{
		error = -1 ;
		g_FD = 0;
		goto end_function;
	}*/
		
	/* set parameters */
	ioctl_params_dma.Multiple_LLIs.nValidLLIs = pMultiple_LLIs->nValidLLIs ;
	ioctl_params_dma.userSpaceVirtualAddress = DataIn_ptr ;
	ioctl_params_dma.memSize = DataInSize ;
	ioctl_params_dma.DstRsrcLLI = DstRsrcLLI ;
	
	/* send write request */
	//driverError = ioctl(g_FD , SEP_DRIVER_CREATE_SYM_DMA_TABLE_CMD , &(ioctl_params_dma));
	//if ( driverError < 0 )  {
	//	error = 13 ;
	//	goto end_function ;
	//}

//	for (i=0 ; i< ioctl_params_dma.Multiple_LLIs.nValidLLIs; ++i )	{
        for (i=0 ; i<(DataInSize/256); i++ )  {
		
 /*pMultiple_LLIs->LLI_workPlans[i].dmaAddress_LLI_WORD0 = ioctl_params_dma.Multiple_LLIs.LLI_workPlans[i].dmaAddress_LLI_WORD0 ;*/
  // pMultiple_LLIs->LLI_workPlans[i].dmaAddress_LLI_WORD0 = DataIn_ptr + (DxUint32_t)i*512;
     pMultiple_LLIs->LLI_workPlans[i].dmaAddress_LLI_WORD0 = DataIn_ptr + (DxUint32_t)i*256;
 //   pMultiple_LLIs->LLI_workPlans[i].size_LLI_WORD1 = ioctl_params_dma.Multiple_LLIs.LLI_workPlans[i].size_LLI_WORD1 ;
//  pMultiple_LLIs->LLI_workPlans[i].size_LLI_WORD1 = 512;
     pMultiple_LLIs->LLI_workPlans[i].size_LLI_WORD1 = 256;
  DX_VOS_Printf("Work plan details --> DMA address is 0x%08X and size is 0x%08X \n",pMultiple_LLIs->LLI_workPlans[i].dmaAddress_LLI_WORD0, pMultiple_LLIs->LLI_workPlans[i].size_LLI_WORD1);

  }
		
#ifdef  DEBUG_CC_HW_IFACE		
		printf("From IOCTL return Work plan details --> DMA address is 0x%08X and size is 0x%08X \n",
		       	pMultiple_LLIs->LLI_workPlans[i].dmaAddress_LLI_WORD0, 
	  		pMultiple_LLIs->LLI_workPlans[i].size_LLI_WORD1 ) ;
#endif
		
	
	
//	pMultiple_LLIs->nValidLLIs = ioctl_params_dma.Multiple_LLIs.nValidLLIs;
       pMultiple_LLIs->nValidLLIs =(DataInSize/256);
      PRINTF("Ended CC_Hw_InterFace_Map_DMA\n");

end_function :
	return error; 
}

/**
 * @brief       This function initializes the cc Driver
 * 
 * @param[in] driver_name_ptr - the driver name string who represent the driver file descriptor
 * @return     DxError_t:  
 *                        
 */

/*void debugReadSRAM(
	u32 *buffer,
    	int nWords,
    	unsigned int sramOffset
)
{
	
	LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(g_VirtualHwBaseAddr);
		
	CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_ADDR_REG_ADDR, 
				     sramOffset ); 	
	
	CRYS_PLAT_SYS_ReadRegister( g_VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR, (*buffer) ) ; // dummy read
	
	LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(g_VirtualHwBaseAddr);
	
	while(nWords--)
		CRYS_PLAT_SYS_ReadRegister( g_VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR, (*buffer++) ) ;
		
}*/

/**
 * @brief       This function initializes the cc Driver
 * 
 * @param[in] driver_name_ptr - the driver name string who represent the driver file descriptor
 * @return     DxError_t:  
 *                        
 */
static DxError_t HostToSramCopy(
	struct LLI_workPlan *pWorkPlan,
	int nPlans,
	unsigned int sramOffset,
        DxUint32_t  VirtualHwBaseAddr)
{
/*----------------------
      CODE
  -----------------------*/
	unsigned int buffer[512];   // tbd 512 -> macro please related to NUM_OF_MAX_PLANS
	int j, i, nWords;	// in DX arch/terminology word is 32-bits	
        PRINTF("Entered HostToSramCopy \n");
	for ( i=0,j=0; i<nPlans; ++i,j+=2,++pWorkPlan)	{
		buffer[j]=pWorkPlan->dmaAddress_LLI_WORD0;
		buffer[j+1]=pWorkPlan->size_LLI_WORD1 ;

#ifdef  DEBUG_CC_HW_IFACE		
		printf("\nWork plan details before SRAM copy --> DMA address is 0x%08X and size is 0x%08X \n",
		       pWorkPlan->dmaAddress_LLI_WORD0, pWorkPlan->size_LLI_WORD1 ) ;
#endif	
	}
	
	nWords = nPlans*2 ;
		
	// assert sram offset is 32 bit aligned tbd-sasi

	/* wait when SRAM is ready */
 //	LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(g_VirtualHwBaseAddr);
      PRINTF("Started LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT\n");
       LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);
      PRINTF("Ended LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT\n");

	/* write LLI start address into SRAM address register  */
	/*CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_ADDR_REG_ADDR, 
		sramOffset );*/
          CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_ADDR_REG_ADDR,
                sramOffset ); 

	
	for(i = 0; i < nWords; ++i )
	{      

#ifdef  DEBUG_CC_HW_IFACE		
		if(!(i%2))
			printf("\nWork plan details --> DMA address is 0x%08X and size is 0x%08X \n",
		       		buffer[i], buffer[i+1] ) ;
#endif
		
            /*		CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR, 
			buffer[i]);		*/
                CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_COMMON_HW_SRAM_DATA_REG_ADDR,
                        buffer[i]);
		
		/* wait when SRAM is ready (after a word write) */
		//LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(g_VirtualHwBaseAddr);
              PRINTF("Started LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT\n");
               LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT(VirtualHwBaseAddr);
             PRINTF("Ended LLF_COMMON_HW_WAIT_ON_SRAM_DATA_READY_BIT\n");

	} 
        PRINTF("Ended HostToSramCopy\n");
	return -21 ;
}

/**
 * @brief       This function builds DMA
 * 
 * @param[in] driver_name_ptr - the driver name string who represent the driver file descriptor
 * @return     DxError_t:  
 *                        
 */
DxError_t CC_Hw_InterFace_Map_And_Start_DMA(
	 void	*Data_ptr,  //_IN_
	 int	DataSize,   //_IN_
         DxUint32_t  VirtualHwBaseAddr,  //_IN_
	 int	DstRsrcLLI) //_IN_
{
/*----------------------
      CODE
  -----------------------*/
	unsigned int sramOffset ; 
	DxError_t error ;
	
	error = DX_OK;

        PRINTF("Entered CC_Hw_InterFace_Map_And_Start_DMA \n");	
	do{
		
		g_Multiple_LLIs.nValidLLIs = MAX_PAGES_FOR_LLI_WORK_PLAN ;

		error = CC_Hw_InterFace_Map_DMA(
				Data_ptr,
				DataSize,
				DstRsrcLLI,
				&g_Multiple_LLIs);
		
		if(error != DX_OK)
  		{
    			break ;
  		}

		// set SRAM offset
		if ( DstRsrcLLI == SRC_LLI )	
			sramOffset = LLF_COMMON_HW_TMP_SRAM_SRC_LLI_TAB_ADDR ;
		else if ( DstRsrcLLI == DST_LLI ) 	
			sramOffset = LLF_COMMON_HW_TMP_SRAM_DST_LLI_TAB_ADDR ;
		else
			sramOffset = -1 ;  // error case

		// set first , last entry bits in LLI and copy to SRAM
		{
			int i=0, nWorkPlans = g_Multiple_LLIs.nValidLLIs ;
			volatile struct LLI_workPlan *pWorkPlan = g_Multiple_LLIs.LLI_workPlans;

 		
		       DX_VOS_Printf("nWorkPlans = %d\n",nWorkPlans);	
			// set FIRST entry bit and fall through the list end
			for (	pWorkPlan[0].size_LLI_WORD1 |= (1UL << LLF_COMMON_HW_LLI_WORD1_FIRST_LLI_WORD_POS); 
					i < nWorkPlans; ++i )	 ;
			
			// go back to last entry if needed
			if ( i ) --i;
			
			// set LAST entry bit
			pWorkPlan[i].size_LLI_WORD1 |= (1UL << LLF_COMMON_HW_LLI_WORD1_LAST_LLI_WORD_POS) ;
			
			// copy above LLI table to SRAM
			// set FIRST entry bit and fall through the list end
#ifdef  DEBUG_CC_HW_IFACE			
			for (	i=0; i < nWorkPlans; ++i )	 {
				printf("\n From IOCTL return stage 2 Work plan details --> DMA address is 0x%08X and size is 0x%08X \n",
					pWorkPlan[i].dmaAddress_LLI_WORD0, 
					pWorkPlan[i].size_LLI_WORD1	) ;
			}				
#endif			
			HostToSramCopy( pWorkPlan, nWorkPlans, 	sramOffset,VirtualHwBaseAddr );
                        PRINTF("Ended HostToSramCopy \n");

		}
		// SRAM copy end

#if 0		
		//
		// debug only - 
		//  
		// test direct LLI_workPlan
		//
		
		{
			struct LLI_workPlan *pWorkPlan = g_Multiple_LLIs.LLI_workPlans;
			
			if(DstRsrcLLI == DST_LLI)
			{
				/* initialize the destination LLI address */
				CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + HW_DST_LLI_WORD0_REG_ADDR, \
						(pWorkPlan[0].dmaAddress_LLI_WORD0 /*& (0xfffff000)*/) );

				/* initialize the destination size and set the first & last LLI word */
				CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + HW_DST_LLI_WORD1_REG_ADDR, \
						 pWorkPlan[0].size_LLI_WORD1  /* 0x1000 */ |
								(1UL << 30) | 
								(1UL << 31) ); 
			}
			else
			{
				/* initialize the source LLI address */ 
				CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + HW_SRC_LLI_WORD0_REG_ADDR, \
						(pWorkPlan[0].dmaAddress_LLI_WORD0  /*& (0xfffff000)*/) );

				/* initialize the source size and set the first & last LLI word - this triggers the operation */
				CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + HW_SRC_LLI_WORD1_REG_ADDR, \
						pWorkPlan[0].size_LLI_WORD1 /*0x1000*/ |
								(1UL << 30) | 
								(1UL << 31) );			
			}
			
			
		}
		
		break ;
		// debug end
		// test end
		//
		//
#endif
				
		
		
		// trigger both the src and dst DMA by writing to source
		if ( DstRsrcLLI == SRC_LLI )
	             /* CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + HW_SRC_LLI_SRAM_ADDR_REG_ADDR, 
				sramOffset );*/
                  CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + HW_SRC_LLI_SRAM_ADDR_REG_ADDR,
                                sramOffset ); 
		else
		/*	CRYS_PLAT_SYS_WriteRegister( g_VirtualHwBaseAddr + HW_DST_LLI_SRAM_ADDR_REG_ADDR, 
				sramOffset );*/
                CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr+ HW_DST_LLI_SRAM_ADDR_REG_ADDR,
                                sramOffset ); 
			
				    

	} while(0);

end_function:
	return error ; 
}

/*void mdelay(volatile int delay)
{
        volatile ulong i = delay*300*1000;

        // Wait for delay amount of milli seconds.
        while(i--);
}
*/

 DxUint32_t makeAligned(DxUint32_t DataIn_ptr){

  DxUint32_t align_ptr,offset,Dataptr;
  offset = DataIn_ptr & 0x0000FFFF;
  DX_VOS_Printf("offset = %08x\n",offset);
  Dataptr = DataIn_ptr - offset;
  if(offset)
  {
   align_ptr = Dataptr + 0x00010000;
   DX_VOS_Printf("align_ptr = %08x\n",align_ptr);
  }
 else
  align_ptr = Dataptr;

  return align_ptr;

}



void LLF_Alignment_Pointer(DxUint8_t       *DataIn_ptr,
                           DxUint8_t       *DataOut_ptr,
                           DxUint32_t       DataInSize)

{
  DxUint8_t *DataOut_ptr1,*DataIn_ptr1;
  int i;

  DataOut_ptr1 = makeAligned(DataOut_ptr);
  DataIn_ptr1 = makeAligned(DataIn_ptr);
  for(i=0;i<DataInSize;i++){
 *(DataIn_ptr1 +i) = *(DataIn_ptr +i);
  DX_VOS_Printf("value is %x,",*(DataIn_ptr1 +i));
 }
 DataIn_ptr = DataIn_ptr1;
 DataOut_ptr = DataOut_ptr1;
 DX_VOS_Printf("DataIn_ptr = %08x\n",DataIn_ptr);
 DX_VOS_Printf("DataOut_ptr = %08x\n",DataOut_ptr);

 return;

}


/**
 * @brief       This function initializes the cc Driver
 * 
 * @param[in] driver_name_ptr - the driver name string who represent the driver file descriptor
 * @return     DxError_t:  
 *                        
 */
/*void CC_Hw_InterFace_unMap(void)
{*/
/*----------------------
      CODE
  -----------------------*/
//	DxInt32_t		result = 0;
	//result = ioctl(g_FD , SEP_DRIVER_FREE_SYM_DMA_TABLE_CMD );
//}*/


//void chip_reset(void)
//{
//	CRYS_PLAT_SYS_WriteRegister((g_VirtualHwBaseAddr+0xBEC), 1);
//}

/*void flush_IO_pages(void)
{
//	ioctl(g_FD , SEP_FLUSH_DEVICE_MEMORY );	
}
*/
		
