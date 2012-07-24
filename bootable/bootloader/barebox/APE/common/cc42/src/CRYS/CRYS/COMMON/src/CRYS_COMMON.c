/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/


/************* Include Files ****************/

#include "/home/buildsys/Android-security/git_EOS2_PF/BIZEN/src/APE/common/cc42/inc/VOS6/VOS_API/DX_VOS_Mem.h"
#ifndef CRYS_NO_GLOBAL_DATA
#include "/home/buildsys/Android-security/git_EOS2_PF/BIZEN/src/APE/common/cc42/inc/VOS6/VOS_API/DX_VOS_Sem.h"
#else
#define DX_VOS_SemCreate(semId, type, semName)	CRYS_OK
#define DX_VOS_SemDelete(semId)	CRYS_OK
#endif
#include "/home/buildsys/Android-security/git_EOS2_PF/BIZEN/src/APE/common/cc42/inc/CRYS/CRYS/COMMON/inc/CRYS_COMMON_error.h"
#include "/home/buildsys/Android-security/git_EOS2_PF/BIZEN/src/APE/common/cc42/inc/CRYS/CRYS_API/inc/CRYS.h"
#include "/home/buildsys/Android-security/git_EOS2_PF/BIZEN/src/APE/common/cc42/inc/CRYS/LLF_COMMON/inc/LLF_COMMON.h"

/************************ Defines **********************************/

/************************ Enums ************************************/

/************************ Typedefs *********************************/

/************************ Global Data ******************************/

/**
  \brief allocating the semaphores required to operate any hardware machine.
*/
#ifndef CRYS_NO_GLOBAL_DATA

/* RND semaphore */
DxVosSem SemRndId;

/* PKA semaphore defined if PKI uses PKA */
#if (LLF_PKI_ENGINE_TYPE == CRYS_DEFS_CC6_PKA_ENGINE_TYPE)
DxVosSem SemPkaId;
#endif

#if !CRYS_DEFS_ALL_ENGINES_SW_TYPE	/*Only if there is some HW Engine*/
	DxVosSem SemHwAccessId;    /* semaphore used for the access to the hardware machine */
#endif

#endif /* CRYS_NO_GLOBAL_DATA */

/************* Private function prototype *******************************/


/************************ Public Functions ******************************/

/***********************************************************************
 **
 * @brief This function initializes the CRYS COMMON module in cold init.
 *        it is called from the CRYS_INIT function.
 *
 *
 * @return CRYSError_t - On success CRYS_OK.
 */

/* CRYSError_t CRYS_COMMON_Init(DxUint32_t param )
 {*/
   /* FUNCTION DECLERATIONS */

//	 CRYSError_t Error2 = CRYS_OK;

//	 #if !CRYS_DEFS_ALL_ENGINES_SW_TYPE	|| !defined(CRYS_NO_GLOBAL_DATA)
//	 DxChar_t StringName[10];
//	 #endif

   /* FUNCTION LOGIC */
/*#ifndef CRYS_SEP_SIDE_WORK_MODE
   #if !CRYS_DEFS_ALL_ENGINES_SW_TYPE*/	/*Only if there is some HW Engine*/

   /* ............ creating the semaphores that the modules use .......... */
   /* -------------------------------------------------------------------- */

   /* creating the Hardware access semaphore (all modules besides PKA semaphore) */
/*   DX_VOS_MemSet(StringName,0,sizeof(StringName));
   DX_VOS_FastMemCpy(StringName,"HW",sizeof("HW"));
   Error2 = DX_VOS_SemCreate( &SemHwAccessId , DX_VOS_SEM_FREE , StringName );

   if( Error2 != CRYS_OK )

      return CRYS_COMMON_INIT_HW_SEM_CREATION_FAILURE;

   #endif
*/
   /* creating the RND access semaphore  */
  /* DX_VOS_MemSet(StringName,0,sizeof(StringName));
   DX_VOS_FastMemCpy(StringName,"RND",sizeof("RND"));
   Error2 = DX_VOS_SemCreate( &SemRndId , DX_VOS_SEM_FREE , StringName );

   if( Error2 != CRYS_OK )

      return CRYS_COMMON_INIT_HW_SEM_CREATION_FAILURE;
*/
   /* creating the PKA access semaphore  */
  /* #if (LLF_PKI_ENGINE_TYPE == CRYS_DEFS_CC6_PKA_ENGINE_TYPE)
   DX_VOS_MemSet(StringName,0,sizeof(StringName));
   DX_VOS_FastMemCpy(StringName,"PKA",sizeof("PKA"));
   Error2 = DX_VOS_SemCreate( &SemPkaId , DX_VOS_SEM_FREE , StringName );

   if( Error2 != CRYS_OK )

      return CRYS_COMMON_INIT_HW_SEM_CREATION_FAILURE;
   #endif
#endif //CRYS_SEP_SIDE_WORK_MODE
   Error2 = LLF_COMMON_Init(param ,DX_NULL);

   return Error2;

 }*//* END OF CRYS_COMMON_Init */
//#ifndef CRYS_SEP_SIDE_WORK_MODE
/* ------------------------------------------------------------
 **
 * @brief This function terminates the CRYS COMMON module .
 *        it is called from the CRYS_Terminate function.
 *
 *
 * @return CRYSError_t - On success CRYS_OK.
 */

/* CRYSError_t CRYS_COMMON_Terminate(void )
 {*/
   /* FUNCTION DECLERATIONS */

//	 CRYSError_t Error2 = CRYS_OK;
   /* FUNCTION LOGIC */
/*
   #if !CRYS_DEFS_ALL_ENGINES_SW_TYPE*/	/*Only if there is some HW Engine*/

   /* ............ creating the semaphores that the module uses .......... */
   /* -------------------------------------------------------------------- */

   /* creating the Hardware access semaphore */
/*   Error2 =  DX_VOS_SemDelete (SemHwAccessId);

   if( Error2 != CRYS_OK )

      return CRYS_COMMON_TERM_HW_SEM_DELETE_FAILURE;

   #endif

   Error2 = DX_VOS_SemDelete( SemRndId );

   if( Error2 != CRYS_OK )

      return CRYS_COMMON_TERM_HW_SEM_DELETE_FAILURE;


   return Error2;

 }*//* END OF CRYS_COMMON_Init */

//#endif
