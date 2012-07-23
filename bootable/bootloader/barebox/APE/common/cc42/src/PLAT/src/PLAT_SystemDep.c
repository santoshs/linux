/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/



/************* Include Files ****************/

#include "PLAT_SystemDep.h"
/*#include <stdio.h>
#include <stdarg.h>*/ /* WIP_NoOS_R_Loader */
#include "CRYS_Defs.h"

/* canceling the lint warning: */
/*lint --e{718} */
/*lint --e{746} */

/************************ Defines ******************************/


/************************ Enums ******************************/


/************************ Typedefs ******************************/


/************************ Global Data ******************************/

/* declaring the base address of the CRYPTO Cell . this value must be initialized in the
   CRYS_Init function */
#if !CRYS_DEFS_ALL_ENGINES_SW_TYPE	/*Only if there is some HW Engine*/
#ifndef CRYS_NO_GLOBAL_DATA
	DxUint32_t PLAT_CryptoCellBaseAddr;
#else
const	DxUint32_t PLAT_CryptoCellBaseAddr = CRYS_COMMON_TST_CRYPT_LITE_BOARD_BASE_ADDR;
#endif
#endif

/************* Private function prototype ****************/


/************************ Public Functions ******************************/

/**
 * @brief The folowing function tap the SEP watchdog .
 *
 *
 */
void PLAT_WatchDogReset( void )
{
	return;

}/* END OF PLAT_LOG_DEV */
/**
 * @brief The folowing function handels the printings logging of the develop group .
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] format,... - the printf format.
 *
 */
 /* ********************* Provate Functions *******************************/
#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED

void PLAT_LOG_DEV( DxUint8_t level , const char *format,... )
{
  /* LOCAL DECLERATIONS */

  va_list ap;

  /* FUNCTION LOGIC */

  /* if the printig level is enabled then print it */
  if( level < CRYS_LOG_DEV_MAX_LEVEL_ENABLED )
  {
    va_start(ap, format);
    vprintf( format , ap );
    va_end(ap);
  }

  return;

}/* END OF PLAT_LOG_DEV */

#endif

/**
 * @brief The folowing function handels the printings logging of the testing group .
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] format,... - the printf format.
 *
 */
 /* ********************* Provate Functions *******************************/

#if CRYS_LOG_TST_MAX_LEVEL_ENABLED

void PLAT_LOG_TST( DxUint8_t level , const char *format,... )
{
  /* LOCAL DECLERATIONS */

  va_list ap;

  /* FUNCTION LOGIC */

  /* if the printig level is enabled then print it */
  if( level < CRYS_LOG_TST_MAX_LEVEL_ENABLED )
  {
    va_start(ap, format);
    vprintf( format , ap );
    va_end(ap);
  }

  return;

}/* END OF PLAT_LOG_TST */

#endif

/**
 * @brief The folowing function prints a buffer to the Maple format on Little endian.
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 *
 */
 /* ********************* Provate Functions *******************************/
#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED

void PLAT_LOG_DEV_DisplayDataMapleLittleEndian(DxUint8_t level ,char  *Label,DxUint8_t *Buffer,DxUint32_t Size )

{
  /* LOCAL DECLERATIONS */

  /* loop variable */
  DxUint32_t  i;

  /* FUNCTION LOGIC */

  /* print the label */
  PLAT_LOG_DEV_PRINT(( level , "\n Maple LittleEndian %s -[Size %ld ] Addr : %p\n",Label,Size,Buffer));

  /* print the maplel line */
  PLAT_LOG_DEV_PRINT((level , "%s := convert(`",Label));

  for( i=Size ; i>0 ;i--)

     PLAT_LOG_DEV_PRINT((level , "%02x",Buffer[i-1]));

  PLAT_LOG_DEV_PRINT((level , "`,decimal,hex);"));

  PLAT_LOG_DEV_PRINT((level , "\n"));

  return;

}/* END OF PLAT_LOG_DEV_DisplayDataMapleLittleEndian */

/**
 * @brief The folowing function prints a buffer to the Maple format on Big endian.
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 *
 */
 /* ********************* Provate Functions *******************************/
void PLAT_LOG_DEV_DisplayDataMapleBigEndian(DxUint8_t level , char  *Label,DxUint8_t *Buffer,DxUint32_t Size )

{
  /* LOCAL DECLERATIONS */

  /* loop variable */
  DxUint32_t  i;

  /* FUNCTION LOGIC */

  /* print the label */
  PLAT_LOG_DEV_PRINT(( level , "\n Maple BigEndian %s -[Size %ld ] Addr : %p\n",Label,Size,Buffer));

  /* print the maplel line */
  PLAT_LOG_DEV_PRINT((level , "%s := convert(`",Label));

  for( i=0 ; i<Size ;i++)

     PLAT_LOG_DEV_PRINT((level , "%02x",Buffer[i]));

  PLAT_LOG_DEV_PRINT((level ,"`,decimal,hex);"));

  PLAT_LOG_DEV_PRINT((level ,"\n"));

  return;

}/* END OF PLAT_LOG_DEV_DisplayDataMapleBigEndian */

#endif /* CRYS_LOG_DEV_MAX_LEVEL_ENABLED */

/**
 * @brief The folowing function prints a buffer.
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_DEV_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 *
 */
 /* ********************* Provate Functions *******************************/
#if CRYS_LOG_DEV_MAX_LEVEL_ENABLED
void PLAT_LOG_DEV_DisplayBuffer(DxUint8_t level , char  *Label,DxUint8_t *Buffer,DxUint32_t Size )

{
  /* LOCAL DECLERATIONS */

  /* loop variable */
  DxUint32_t  i;

  /* FUNCTION LOGIC */

  /* print the label */
  PLAT_LOG_DEV_PRINT(( level , "\n  %s -[Size %ld ] Addr : %p\n",Label,Size,Buffer));

  for( i=0 ; i<Size ;i++)
  {
     if ( !(i % 16) )

        PLAT_LOG_DEV_PRINT((level, "\n" ));

     PLAT_LOG_DEV_PRINT((level , "0x%02x,",Buffer[i]));

  }

  PLAT_LOG_DEV_PRINT((level ,"\n"));

  return;

}/* END OF PLAT_LOG_DEV_DisplayBuffer */

#endif /* CRYS_LOG_DEV_MAX_LEVEL_ENABLED */

/**
 * @brief The folowing function prints a buffer to the Maple format on Little endian.
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 *
 */
 /* ********************* Provate Functions *******************************/
#if CRYS_LOG_TST_MAX_LEVEL_ENABLED

void PLAT_LOG_TST_DisplayDataMapleLittleEndian(DxUint8_t level ,char  *Label,DxUint8_t *Buffer,DxUint32_t Size )

{
  /* LOCAL DECLERATIONS */

  /* loop variable */
  DxUint32_t  i;

  /* FUNCTION LOGIC */

  /* print the label */
  PLAT_LOG_TST_PRINT(( level , "\n Maple LittleEndian %s -[Size %ld ] Addr : %p\n",Label,Size,Buffer));

  /* print the maplel line */
  PLAT_LOG_TST_PRINT((level , "%s := convert(`",Label));

  for( i=Size ; i>0 ;i--)

     PLAT_LOG_TST_PRINT((level , "%02x",Buffer[i-1]));

  PLAT_LOG_TST_PRINT((level , "`,decimal,hex);"));

  PLAT_LOG_TST_PRINT((level , "\n"));

  return;

}/* END OF PLAT_LOG_TST_DisplayDataMapleLittleEndian */

/**
 * @brief The folowing function prints a buffer to the Maple format on Big endian.
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_LOG_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 *
 */
 /* ********************* Provate Functions *******************************/
void PLAT_LOG_TST_DisplayDataMapleBigEndian(DxUint8_t level , char  *Label,DxUint8_t *Buffer,DxUint32_t Size )

{
  /* LOCAL DECLERATIONS */

  /* loop variable */
  DxUint32_t  i;

  /* FUNCTION LOGIC */

  /* print the label */
  PLAT_LOG_TST_PRINT(( level , "\n Maple BigEndian %s -[Size %ld ] Addr : %p\n",Label,Size,Buffer));

  /* print the maplel line */
  PLAT_LOG_TST_PRINT((level , "%s := convert(`",Label));

  for( i=0 ; i<Size ;i++)

     PLAT_LOG_TST_PRINT((level , "%02x",Buffer[i]));

  PLAT_LOG_TST_PRINT((level ,"`,decimal,hex);"));

  PLAT_LOG_TST_PRINT((level ,"\n"));

  return;

}/* END OF PLAT_LOG_TST_DisplayDataMapleBigEndian */

#endif /* CRYS_LOG_TST_MAX_LEVEL_ENABLED */

/**
 * @brief The folowing function prints a buffer.
 *
 *
 * @param[in] level - the printing level if it is below the CRYS_TST_TST_MAX_LEVEL_ENABLED
 *                    flag setting it will not be printed.
 *
 * @param[in] Label - a lable string.
 * @param[in] Buffer - the buffer to be printed.
 * @param[in] Size  - the buffer size in bytes.
 *
 */
 /* ********************* Provate Functions *******************************/
#if CRYS_LOG_TST_MAX_LEVEL_ENABLED
void PLAT_LOG_TST_DisplayBuffer(DxUint8_t level , char  *Label,DxUint8_t *Buffer,DxUint32_t Size )

{
  /* LOCAL DECLERATIONS */

  /* loop variable */
  DxUint32_t  i;

  /* FUNCTION LOGIC */

  /* print the label */
  PLAT_LOG_TST_PRINT(( level , "\n%s BufferSize(Byte) %ld // BufferAddr %p\n",Label,Size,Buffer));

  for( i=0 ; i<Size ;i++)
  {
     if ( !(i % 16) )

        PLAT_LOG_TST_PRINT((level, "\n" ));

     PLAT_LOG_TST_PRINT((level , "0x%02x,",Buffer[i]));

  }

  PLAT_LOG_TST_PRINT((level ,"\n\n"));

  return;

}/* END OF PLAT_LOG_TST_DisplayBuffer */

#endif /* CRYS_LOG_TST_MAX_LEVEL_ENABLED */
