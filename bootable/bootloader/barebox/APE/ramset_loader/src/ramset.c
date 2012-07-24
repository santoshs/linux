/* ************************************************************************* **
**                               Renesas                                     **
** ************************************************************************* */

/* *************************** COPYRIGHT INFORMATION *********************** **
** This program contains proprietary information that is a trade secret of   **
** Renesas and also is protected as an unpublished work under                **
** applicable Copyright laws. Recipient is to retain this program in         **
** confidence and is not permitted to use or make copies thereof other than  **
** as permitted in a written agreement with Renesas.                         **
**                                                                           **
** Copyright (C) 2012 Renesas Mobile Corp.	                                 **
** All rights reserved.                                                      **
** ************************************************************************* */

/* ****************************** DESCRIPTION ****************************** **
**                                                                           **
** ************************************************************************* */

/* *********************** HEADER (INCLUDE) SECTION ************************ */
#include "ramset_ram.h"

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
/* CCCR */
#define CCCR_U2_ES200                           0x00003E10   /** CCCR U2ES2.00 */
#define CCCR_U2_ES201                           0x00003E11   /** CCCR U2ES2.01 */
#define CCCR_U2_ES202                           0x00003E12   /** CCCR U2ES2.02 */
#define CCCR_BIT15_TO_0_MASK                    0x0000FFFF   /** CCCR bit15-0 mask value */

/* 2nd image */
#define LOAD_2ND_IMAGE_ADDRESS                  0x0000A000   /** 2nd image address */
#define LOAD_2ND_IMAGE_SIZE                     0x00040000   /** Size of 2nd image(256KB). */

/* SYSC */
// #define SYSC_BASE                               0xE6180000
// #define RESCNT                                  *((volatile unsigned long *)(SYSC_BASE+0x801C))
#define MD7                                     0x04000000

/* ************************** FUNCTION PROTOTYPES ************************** */
/** Extern Global variable of Public ROM Side. */
extern rom_rpc_cb_func g_load_image_callback_es200;
extern rom_rpc_cb_func g_load_image_callback_es202;

/** Extern function of RAM Side. */
extern EMMC_ERROR_CODE emmc_read_sector_es200 (uint32 *buff_address_virtual, uint32 sector_number, uint32 count,
                                               HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode);
extern EMMC_ERROR_CODE emmc_read_sector_es202 (uint32 *buff_address_virtual, uint32 sector_number, uint32 count,
                                               HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode);

extern EMMC_ERROR_CODE emmc_select_partition_es200(EMMC_PARTITION_ID id);
extern EMMC_ERROR_CODE emmc_select_partition_es202(EMMC_PARTITION_ID id);

extern void emmc_write_error_info_es200(uint16 func_no, EMMC_ERROR_CODE error_code);
extern void emmc_write_error_info_es202(uint16 func_no, EMMC_ERROR_CODE error_code);

extern void emmc_write_error_info_func_no_es200 (uint16 func_no);
extern void emmc_write_error_info_func_no_es202 (uint16 func_no);

extern BOOL is_timeout_es200( CMT1_CHANNEL ch );
extern BOOL is_timeout_es202( CMT1_CHANNEL ch );

extern st_mmc_base mmc_drv_obj_es200;
extern st_mmc_base mmc_drv_obj_es202;

extern void * rom_mmu_p_to_v_es200( void * p_addr );
extern void * rom_mmu_p_to_v_es202( void * p_addr );

extern void timer_start_es200( CMT1_CHANNEL ch, unsigned long timeout_ms );
extern void timer_start_es202( CMT1_CHANNEL ch, unsigned long timeout_ms );

extern void timer_stop_es200( CMT1_CHANNEL ch );
extern void timer_stop_es202( CMT1_CHANNEL ch );

/** Typedef. For function pointer of Public ROM. */
typedef EMMC_ERROR_CODE (*ramset_emmc_read_sector)(uint32 *buff_address_virtual, uint32 sector_number, uint32 count,
                                                   HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode);
typedef EMMC_ERROR_CODE (*ramset_emmc_select_partition)(EMMC_PARTITION_ID id);
typedef void (*ramset_emmc_write_error_info)(uint16 func_no, EMMC_ERROR_CODE error_code);
typedef void (*ramset_emmc_write_error_info_func_no)(uint16 func_no);
typedef BOOL (*ramset_is_timeout)( CMT1_CHANNEL ch );
st_mmc_base g_ramset_mmc_drv_obj;
typedef void* (*ramset_rom_mmu_p_to_v)( void * p_addr );
typedef void (*ramset_timer_start)( CMT1_CHANNEL ch, unsigned long timeout_ms );
typedef void (*ramset_timer_stop)( CMT1_CHANNEL ch );

/** PROTOTYPES. For function pointer of Public ROM. */
ramset_emmc_read_sector g_ramset_emmc_read_sector;
ramset_emmc_select_partition g_ramset_emmc_select_partition;
ramset_emmc_write_error_info g_ramset_emmc_write_error_info;
ramset_emmc_write_error_info_func_no g_ramset_emmc_write_error_info_func_no;
ramset_is_timeout g_ramset_is_timeout;
ramset_rom_mmu_p_to_v g_ramset_rom_mmu_p_to_v;
ramset_timer_start g_ramset_timer_start;
ramset_timer_stop g_ramset_timer_stop;

/** 2nd load image function */
static uint32 ramset_rpc_emmc_loadimage (uint32 interface, uint32 size_of_image
                                       , uint32 address_to_load_from, uint32 address_to_load_to
                                       , uint32 configuration_info_p, uint32 feature_flags, uint32 timeout);
static EMMC_ERROR_CODE ramset_emmc_exec_load_image(uint32 interface, uint32 size_of_image, uint32 address_to_load_from,
                                                   uint32 address_to_load_to, uint32 configuration_info_p, uint32 feature_flags, uint32 timeout);

/* ***************************** CODE SECTION ****************************** */


cb_pbrom_uart_printf g_pbrom_uart_printf = 0;

/* ****************************************************************************
** Function name      : ramset_main
** Description        : This function RAMSet.
** Parameters         : None
** Return value       : None
** Compile directives : None
** ***************************************************************************/

uint32_t ramset_main(cb_register_func  ramset_cb_register_func, cb_pbrom_uart_printf ramset_cb_pbrom_uart_printf)
{
    g_pbrom_uart_printf = ramset_cb_pbrom_uart_printf;

    /* Null check */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("[RAMSET]ramset_main()");
        set_ramset_callback(ramset_cb_register_func);
    }
    /* The register initialization is written here. */
    #ifdef __INTEGRITY_CHECK_ENABLE__
    SBSC_Init();
    #endif /* __INTEGRITY_CHECK_ENABLE__ */
    SYSC_Soft_Power_On_Reset();

    return 0;
}

/* ****************************************************************************
** Function name      : set_ramset_callback
** Description        : This function set callback function.
** Parameters         : None
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void set_ramset_callback( cb_register_func ramset_cb_register_func )
{
    uint32 rescnt_val;
    uint32 es_number;
	
	g_pbrom_uart_printf("[RAMSET]set_ramset_callback()");
	
	 /* Read ROM Debug mode pin. */
    rescnt_val = *RESCNT;
    rescnt_val &= MD7;

    /* Read ES number. */
    es_number  = *CHIP_VER_REG_CCCR;
    es_number &= CCCR_BIT15_TO_0_MASK;

    if ( (rescnt_val != MD7) &&                  /* Only Mask ROM     */
         ((es_number == CCCR_U2_ES200) ||        /* MaskROM of ES2.00 or ES2.01 or ES2.02 */
          (es_number == CCCR_U2_ES201) ||
          (es_number == CCCR_U2_ES202)) )
    {
        /* Set load 2nd image patch point function. */
        ramset_cb_register_func(RAMSET_LOAD_IMAGE_ID        , ramset_Load_Image);
    }
    else
    {
        g_pbrom_uart_printf("Don't set patch function.");
    }
	
}

/* ****************************************************************************
** Function name      : ramset_Load_Image
** Description        : This function is patch_point function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void ramset_Load_Image(va_list ap)
{
    st_load_image_param_t*     load_image_p;

    load_image_p = va_arg(ap, st_load_image_param_t*);

    /* Check Null */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("ramset_Load_Image() Call");
        g_pbrom_uart_printf(" interface           : %d",     load_image_p->interface );
        g_pbrom_uart_printf(" size_of_image       : 0x%x",   load_image_p->size_of_image );
        g_pbrom_uart_printf(" address_to_load_from: 0x%08x", load_image_p->address_to_load_from );
        g_pbrom_uart_printf(" address_to_load_to  : 0x%08x", load_image_p->address_to_load_to );
        g_pbrom_uart_printf(" configuration_info_p: 0x%08x", load_image_p->configuration_info_p );
        g_pbrom_uart_printf(" feature_flags       : 0x%x",   load_image_p->feature_flags );
        g_pbrom_uart_printf(" timeout             : 0x%x",   load_image_p->timeout );
    }

    /* Set 2nd image load function. And set pointer. */
    set_func_pointer();
}

/* ****************************************************************************
** Function name      : set_func_pointer
** Description        : Set function pointer(ES2 or ES2.02)
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void set_func_pointer(void)
{
    uint32 es_number;

    g_pbrom_uart_printf("set_func_pointer call");

    /* Read ES number. */
    es_number  = *CHIP_VER_REG_CCCR;
    es_number &= CCCR_BIT15_TO_0_MASK;

    /* Set function pointer. */
    if( (es_number == CCCR_U2_ES200) || (es_number == CCCR_U2_ES201) )      /* MaskROM of ES2.00 or ES2.01 */
    {
        g_load_image_callback_es200            = ramset_rpc_emmc_loadimage;

        g_ramset_emmc_read_sector              = emmc_read_sector_es200;
        g_ramset_emmc_select_partition         = emmc_select_partition_es200;
        g_ramset_emmc_write_error_info         = emmc_write_error_info_es200;
        g_ramset_emmc_write_error_info_func_no = emmc_write_error_info_func_no_es200;
        g_ramset_is_timeout                    = is_timeout_es200;
        g_ramset_mmc_drv_obj                   = mmc_drv_obj_es200;
        g_ramset_rom_mmu_p_to_v                = rom_mmu_p_to_v_es200;
        g_ramset_timer_start                   = timer_start_es200;
        g_ramset_timer_stop                    = timer_stop_es200;
    }
    else if( es_number == CCCR_U2_ES202)                                    /* MaskROM of ES2.02 */
    {
        g_load_image_callback_es202            = ramset_rpc_emmc_loadimage;

        g_ramset_emmc_read_sector              = emmc_read_sector_es202;
        g_ramset_emmc_select_partition         = emmc_select_partition_es202;
        g_ramset_emmc_write_error_info         = emmc_write_error_info_es202;
        g_ramset_emmc_write_error_info_func_no = emmc_write_error_info_func_no_es202;
        g_ramset_is_timeout                    = is_timeout_es202;
        g_ramset_mmc_drv_obj                   = mmc_drv_obj_es202;
        g_ramset_rom_mmu_p_to_v                = rom_mmu_p_to_v_es202;
        g_ramset_timer_start                   = timer_start_es202;
        g_ramset_timer_stop                    = timer_stop_es202;
    }
    else
    {
        /* Nothing to do. */
        g_pbrom_uart_printf("Fail set patch function pointer.");
    }
}

/* ****************************************************************************
** Function name      : ramset_rpc_emmc_loadimage
** Description        : This function is 2nd_image load function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
static uint32 ramset_rpc_emmc_loadimage (
    uint32 interface,
    uint32 size_of_image,
    uint32 address_to_load_from,
    uint32 address_to_load_to,
    uint32 configuration_info_p,
    uint32 feature_flags,
    uint32 timeout
    )
{
    EMMC_ERROR_CODE result;

    g_pbrom_uart_printf("ramset_rpc_emmc_loadimage() call");

    /* state check */
    if (g_ramset_mmc_drv_obj.mount != TRUE)
    {
        g_ramset_emmc_write_error_info(EMMC_FUNCNO_LOADIMAGE_RPC, EMMC_ERR_STATE);
        g_pbrom_uart_printf(" Err:state");
        return PUB_ROM_RET_FAIL;
    }

    /* parameter check */
    if ( ((address_to_load_from % EMMC_BLOCK_LENGTH) != 0)            /* multiple of block length */
        || ((address_to_load_to & EMMC_4B_BOUNDARY_CHECK_MASK) != 0)  /* 4bytes boundary */
        || (size_of_image == 0)
        || ((size_of_image % EMMC_BLOCK_LENGTH) != 0)                 /* multiple of block length */
        || (timeout == 0)
        )
    {
        g_ramset_emmc_write_error_info(EMMC_FUNCNO_LOADIMAGE_RPC, EMMC_ERR_PARAM);
        g_pbrom_uart_printf(" Err:param");
        return PUB_ROM_RET_FAIL;
    }

    /* flag clear */
    g_ramset_mmc_drv_obj.force_terminate = FALSE;
    g_ramset_mmc_drv_obj.state_machine_blocking = FALSE;

    /* Start timeout timer */
    g_ramset_timer_start(CMT1_CHANNEL0, 1000);

    /* load image */
    result = ramset_emmc_exec_load_image(interface, size_of_image, address_to_load_from, address_to_load_to, configuration_info_p, feature_flags, timeout);

    /* timeout check */
    if(g_ramset_is_timeout(CMT1_CHANNEL0) == FALSE)
    {
        /* nothing to do. */
    }
    else
    {
        g_ramset_mmc_drv_obj.force_terminate = TRUE;
        g_ramset_mmc_drv_obj.state_machine_blocking = FALSE;
        g_ramset_emmc_write_error_info(EMMC_FUNCNO_NONE, EMMC_ERR_FORCE_TERMINATE);
    }
    g_ramset_timer_stop(CMT1_CHANNEL0);

    /* error ? */
    if (result != EMMC_SUCCESS)
    {
        g_ramset_emmc_write_error_info_func_no(EMMC_FUNCNO_LOADIMAGE_RPC);
        g_pbrom_uart_printf(" Err:timer");
        return PUB_ROM_RET_FAIL;    /* loading SW image failed */
    }
    else
    {
        g_pbrom_uart_printf("ramset_rpc_emmc_loadimage() End");
        return PUB_ROM_RET_OK;      /* loading SW image succeeded */
    }
}

/* ****************************************************************************
** Function name      : ramset_emmc_exec_load_image
** Description        : This function is 2nd_image load function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
static EMMC_ERROR_CODE ramset_emmc_exec_load_image(
    uint32 interface,
    uint32 size_of_image,
    uint32 address_to_load_from,
    uint32 address_to_load_to,
    uint32 configuration_info_p,
    uint32 feature_flags,
    uint32 timeout
    )
{
    EMMC_ERROR_CODE result;
    HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode;
    uint32 *dest_address_v = (uint32 *)g_ramset_rom_mmu_p_to_v((void *)address_to_load_to);

    g_pbrom_uart_printf("ramset_emmc_exec_load_image() Start");

    /* UNREFERENCED PARAMETER */
    if ((configuration_info_p == 0) || (timeout == 0))
    {
        /* This parameter is unused. It is not an error. */
    }

    /* DMA? */
    if ((feature_flags & LOADIMAGE_FLAGS_DMA_ENABLE) != 0)
    {
        transfer_mode = HAL_MEMCARD_DMA;
    }
    else
    {
        transfer_mode = HAL_MEMCARD_NOT_DMA;
    }

    /* Select_partition */
    if (g_ramset_mmc_drv_obj.image_num==1)
    {
        result = g_ramset_emmc_select_partition(PARTITION_ID_BOOT_1);    /* loaded 1st image #1 */
    }
    else
    {
        result = g_ramset_emmc_select_partition(PARTITION_ID_BOOT_2);    /* loaded 1st image #2 */
    }

    if (result != EMMC_SUCCESS)
    {
        g_pbrom_uart_printf(" Err:select partition");
        return result;
    }

    /* load image 
     * emmc_read_sector(pBuf, address[sector], size[sector])
     */
    /* USER or BOOT partiotion */
    result = g_ramset_emmc_read_sector (dest_address_v, LOAD_2ND_IMAGE_ADDRESS>>EMMC_SECTOR_SIZE_SHIFT, LOAD_2ND_IMAGE_SIZE>>EMMC_SECTOR_SIZE_SHIFT, transfer_mode);

    g_pbrom_uart_printf("ramset_emmc_exec_load_image() End");
    return result;
}

#if 0
/* ****************************************************************************
** Function name      : ramset_eMMC_Before_Loder
** Description        : This function is patch_point function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void ramset_eMMC_Before_Loder(va_list ap)
{
    /* Null check */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("ramset_eMMC_Before_Loder() Call");
    }
}

/* ****************************************************************************
** Function name      : ramset_Usb_Before_Loder
** Description        : This function is patch_point function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void ramset_Usb_Before_Loder(va_list ap)
{
    /* Null check */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("ramset_Usb_Before_Loder() Call");
    }
}

/* ****************************************************************************
** Function name      : ramset_Soft_Reset
** Description        : This function is patch_point function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void ramset_Soft_Reset(va_list ap)
{
    /* Null check */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("ramset_Soft_Reset() Call");
    }
}

/* ****************************************************************************
** Function name      : ramset_eMMC_After_Loder
** Description        : This function is patch_point function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void ramset_eMMC_After_Loder(va_list ap)
{
    /* Null check */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("ramset_eMMC_After_Loder() Call");
    }
}

/* ****************************************************************************
** Function name      : ramset_Usb_After_Loder
** Description        : This function is patch_point function.
** Parameters         : va_list ap
** Return value       : None
** Compile directives : None
** ***************************************************************************/
void ramset_Usb_After_Loder(va_list ap)
{
     /* Check null */
    if( g_pbrom_uart_printf != NULL)
    {
        g_pbrom_uart_printf("ramset_Usb_After_Loder() Call");
    }
}
#endif

/* ********************************** END ********************************** */


