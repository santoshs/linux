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
    SBSC_Init();
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
    g_pbrom_uart_printf("[RAMSET]set_ramset_callback()");
}




