#ifndef _CC_HW_INTERFACE_H_
#define _CC_HW_INTERFACE_H_

/*------------------------------
    DEFINES
--------------------------------*/
#define SEP_DRIVER_CREATE_SYM_DMA_TABLE_CMD 	_IOW('s' , 0x05 , struct ioctl_params_dma)
#define SEP_DRIVER_FREE_SYM_DMA_TABLE_CMD   	_IO('s' , 0x07)
#define SEP_HW_INIT    				_IO('s' , 70)	
#define SEP_FLUSH_DEVICE_MEMORY    		_IO('s' , 72)	

#define LLF_COMMON_HW_TMP_SRAM_SRC_LLI_TAB_ADDR 0x0100      /* SRAM offset */
#define LLF_COMMON_HW_TMP_SRAM_DST_LLI_TAB_ADDR	0x0200      /* SRAM offset */
	
#define	MAX_PAGES_FOR_LLI_WORK_PLAN 256
#define	DST_LLI	1001
#define	SRC_LLI	1002

#ifndef u32
	typedef unsigned int u32 ;
#endif

//u32 	g_VirtualHwBaseAddr;

/*-------------------------------
  STRUCTURES
---------------------------------*/
struct LLI_workPlan{
	/* bus/dma/physical address */
	u32 dmaAddress_LLI_WORD0 ;
	/* Bit# 30 denotes last entry. Bit# 31 First entry */
	u32 size_LLI_WORD1 ;
}__attribute__((aligned(32),packed)) ;

struct Multiple_LLIs {
	u32 nValidLLIs;
	struct LLI_workPlan LLI_workPlans[MAX_PAGES_FOR_LLI_WORK_PLAN];
}__attribute__((aligned(32),packed)) ;

struct ioctl_params_dma{
	void	*userSpaceVirtualAddress ; //_IN_
	int		memSize ; // _IN_

	// As IN param -- caller sets MAX number of pages that can be created.
	// AS OUT, driver sets the actual number of pages that were created
	struct	Multiple_LLIs Multiple_LLIs;  //_IN_OUT_	
	int		DstRsrcLLI ;  // DMA from device or to device ? _IN_
}__attribute__((aligned(32),packed)) ;

/* create sym dma lli tables */
#define SEP_IOCCREATESYMDMATABLE      \
      _IOW(SEP_IOC_MAGIC_NUMBER, 5, struct ioctl_params_dma)

         
      
/*------------------------------------------------
    FUNCTIONS
--------------------------------------------------*/
/**
 * @brief       This function initializes the cc Driver
 * 
 * @param[in] driver_name_ptr - the cc driver name
 * @return     DxError_t:  
 *                        
 */
DxError_t CCDriver_Init(const DxChar_t *driver_name_ptr);
 
/**
 * @brief       This function builds DMA
 * 
 * @param[in] driver_name_ptr - the cc driver name
 * @return     DxError_t:  
 *                        
 */   
DxError_t CC_Hw_InterFace_Map_And_Start_DMA(
	 void	*Data_ptr,  //_IN_
	 int	DataSize,   //_IN_
         DxUint32_t  VirtualHwBaseAddr,
	 int	DstRsrcLLI); //_IN_
void   LLF_Alignment_Pointer(
                                   DxUint8_t        *DataIn_ptr,
                                   DxUint8_t         *DataOut_ptr,
                                   DxUint32_t         DataInSize);
   
/**
 * @brief       This function initializes the cc Driver
 * 
 * @param[in] driver_name_ptr - the cc driver name
 * @return     DxError_t:  
 *                        
 */
void CC_Hw_InterFace_unMap(void);
//void mdelay(volatile int );
   
void debugReadSRAM(
			       u32 *buffer,
	  int nWords,
   unsigned int sramOffset
			      );   
   
   
void chip_reset(void) ;
   
#endif  /* _CC_HW_INTERFACE_H_ */




