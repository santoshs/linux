/***********************************************************************************
 * Copyright 2009 © Discretix Technologies Ltd.
 * This software is protected by copyright, international treaties and
 * various patents. If the license governing the use of this Software
 * allows copy or redistribution of this  software then any copy or
 * reproduction of this Software must include this Copyright Notice
 * as well as any other notices provided under such license.
 ***********************************************************************************/

/************* Include Files ****************/

#include "DX_VOS_BaseTypes.h"
#include "PLAT_SystemDep.h"
#include "CRYS.h"
#include "CRYS_COMMON_math.h"
#include "CRYS_COMMON.h"
#include "LLF_PKI.h"
#include "LLF_PKI_error.h"
#include "LLF_PKI_HwDefs.h"
#include "DX_VOS_Mem.h"
//#include "CRYS_SELF_TEST_Local.h"



/************************ Defines **********************************/

/* canceling the lint warning:
   Info 717: do ... while(0) */
/*lint --e{717} */

/* canceling the lint warning:
   Use of goto is deprecated */
/*lint --e{801} */

/************************ Enums ************************************/

/************************ Typedefs *********************************/

/************************ Global Data ******************************/

/* importing the base address of the CRYPTO Cell . this value must be
   initialized in the CRYS_Init function */
extern DxUint32_t PLAT_CryptoCellBaseAddr;


#ifndef CRYS_PKA_SRAM_DATA_MEMORY_MODE
extern DxUint32_t LLF_SramStartAddr;
#else
extern const DxUint32_t LLF_SramStartAddr;
#endif

#ifdef LLF_PKI_PKA_DEBUG
extern DxUint32_t tempRes[LLF_PKI_PKA_MAX_REGISTER_SIZE_WORDS + 1];
#endif


/************* Exported functions prototypes *************************/


/*******************************************************************************************/
/**
 * @brief This function executes the RSA primitive: private key CRT exponent
 *
 *    Algorithm [PKCS #1 v2.1]:
 *
 *     1. If NonCRT exponent, then  M  =  C^D  mod N.
 *
 *     Where: M- message representative, C- ciphertext, D- priv.exponent, N- modulus,
 *            ^ - exponentiation symbol.
 *
 *     Note: PKA registers used: r0,r1,r2,r3,r4,r5  r30,r31.
 *
 * @param[in]     PubKey_ptr    - the private key database.
 * @param[in/out] PrivData_ptr  - the structure, containing DataIn and DataOut buffers.
 * @param[in] VirtualHwBaseAddr -  Virtual HW base address, passed by user.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in .
 */
CRYSError_t LLF_PKI_ExecPrivKeyExpNonCrt(
                                     CRYSRSAPrivKey_t    *PrivKey_ptr ,
                                     CRYS_RSAPrimeData_t *PrivData_ptr,
                                     DxUint32_t           VirtualHwBaseAddr )
{
   /* LOCAL DECLARATIONS */

  /* error identification */
   CRYSError_t Error;

   /* modulus and exponents sizes in bytes */
   DxUint32_t  modSizeBytes, eSizeInBytes, dSizeInBytes;

   /* PKA status */
   DxUint32_t  status;


   /* FUNCTION LOGIC */

   /* .................... initialize local variables ...................... */
   /* ---------------------------------------------------------------------- */

   /* initialize the error identifier to success */
   Error = CRYS_OK;

   /* modulus N size in bytes */
   modSizeBytes = (PrivKey_ptr->nSizeInBits + 7) / 8;

   /* priv. exponent size in bytes */
   dSizeInBytes = (PrivKey_ptr->PriveKeyDb.NonCrt.dSizeInBits + 7) / 8;

#ifdef LLF_PKI_PKA_DEBUG
   /* check that there is enough memory for PKA registers needed for
      exponentiation with Self Test (8) and without (7) */
 #ifndef CRYS_NO_RSA_SELF_TEST_SUPPORT
   if( LLF_PKI_PKA_MAX_REGS_MEM_SIZE_BYTES < 8*(modSizeBytes+4) )
 #else
   if( LLF_PKI_PKA_MAX_REGS_MEM_SIZE_BYTES < 7*(modSizeBytes+4) )
 #endif
      return LLF_PKI_PKA_NOT_ENOUGH_MEMORY_ERROR;
#endif

   /* -------------------------------------------------------------------- */
   /*      copy the N, Np DataIn and D into PKA registers                  */
   /* -------------------------------------------------------------------- */
   /* N => r0 */
   LLF_PKI_PKA_CopyDataIntoPkaReg( 0/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->n/*src_ptr*/,
                                   modSizeBytes , VirtualHwBaseAddr );
   /* NP => r1 */
   LLF_PKI_PKA_CopyDataIntoPkaReg( 1/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->LLF.NonCrt.NP/*src_ptr*/,
                                   4*LLF_PKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS , VirtualHwBaseAddr );
   /* DataIn => r2 */
   LLF_PKI_PKA_CopyDataIntoPkaReg( 2/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivData_ptr->DataIn/*src_ptr*/,
                                   modSizeBytes , VirtualHwBaseAddr );
   /* D => r3 */
   LLF_PKI_PKA_CopyDataIntoPkaReg( 3/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.NonCrt.d/*src_ptr*/,
                                   dSizeInBytes , VirtualHwBaseAddr );

  /* -------------------------------------------------------------------- */
  /* .. calculate the exponent: DataOut= Res = DataIn^D mod N;        ... */
  /* -------------------------------------------------------------------- */
  LLF_PKI_PKA_ModExp( 0/*LenID*/, 2/*OpA*/, 3/*OpB*/, 4/*Res*/, 0/*Tag*/, VirtualHwBaseAddr );


  /*--------------------------------------------------------------------------*/
  /* ................ START : Countermeasuers DFA test    ................... */
  /*--------------------------------------------------------------------------*/

#ifndef CRYS_NO_RSA_SELF_TEST_SUPPORT

   /* set public exponent into PKA reg. r3 for Counter measuers DFA, if necessary */
   if(PrivKey_ptr->PriveKeyDb.NonCrt.eSizeInBits != 0)
   {
       /* publ. exponent size in bytes */
       eSizeInBytes = (PrivKey_ptr->PriveKeyDb.NonCrt.eSizeInBits + 7) / 8;
	   /* copy publ. e=> r3 */
       LLF_PKI_PKA_CopyDataIntoPkaReg( 3/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.NonCrt.e/*src_ptr*/,
                                       eSizeInBytes , VirtualHwBaseAddr );
   }

	if( PrivKey_ptr->PriveKeyDb.NonCrt.eSizeInBits != 0 )
	{
		/*    Execue the inverse exponent operation Res1=>r3:               */
		/*          Res1 = Res^e mod N      (Note: D=r3 not needed)         */
		/*------------------------------------------------------------------*/

	    LLF_PKI_PKA_ModExp( 0/*LenID*/, 4/*OpA*/, 3/*OpB*/, 5/*Res*/, 0/*Tag*/, VirtualHwBaseAddr );

		/* if the result is not the same as the original DataIn, set the test as failure */
        LLF_PKI_PKA_Compare(1/*LenID*/, 2/*OpA*/, 5/*OpB*/, 0/*Tag*/, VirtualHwBaseAddr);

        /* read zero bit from the status; if Z != 1 then test fail */
        LLF_PKI_PKA_GET_StatusAluOutZero(VirtualHwBaseAddr, status);
        if( status != 1 )
        {
		  #ifndef CRYS_NO_FIPS_SUPPORT
			DX_GLOBAL_FIPS_MODE |= DX_CRYS_FIPS_MODE_ERROR_STATE;
		  #endif

			Error = LLF_PKI_COUNTERMEASURES_DFA_FAIL_ERROR;

			goto End;
        }

	} /* end if eSizeInBits!=0 */

#endif /*CRYS_NO_RSA_SELF_TEST_SUPPORT*/


   /*-----------------------------*/
   /*  Finish PKA and copy result */
   /*-----------------------------*/

    /* copy result into output buffer */
    LLF_PKI_PKA_CopyDataFromPkaReg( (DxUint8_t*)PrivData_ptr->DataOut, modSizeBytes, 4/*srcReg*/, VirtualHwBaseAddr );
End:

   /* clear used PKA registers (0-5 and 30,31) for security goals */
   LLF_PKI_PKA_ClearBlockOfRegs(  0/*FirstReg*/, 6/*Count*/, 1/*LenID*/, VirtualHwBaseAddr );
   LLF_PKI_PKA_ClearBlockOfRegs( 30/*FirstReg*/, 2/*Count*/, 1/*LenID*/, VirtualHwBaseAddr );

   /* Finish PKA operations (waiting PKI done and close PKA clocks) */
   LLF_PKI_PKA_FinishPKA( VirtualHwBaseAddr );

   return Error;

}/* END OF LLF_PKI_ExecPrivKeyExpNonCrt */


/*******************************************************************************************/
/**
 * @brief This function executes the RSA primitive: private key CRT exponent
 *
 *    Algorithm [PKCS #1 v2.1]:
 *
 *   CRT exponentiation algorithm:
 *        1. Mq  =  C^dQ mod Q;
 *        2. Mp  =  C ^dP mod P,
 *        3  h = (Mp-Mq)*qInv mod P;
 *        4. M = Mq + Q * h.
 *
 *     Where: M- message representative, C- ciphertext, D- priv.exponent, N- modulus,
 *            P,Q,dP,dQ, qInv - CRT private key parameters;
 *            ^ - exponentiation symbol.
 *
 *     Note: 9 PKA registers are used: r0-r6,  r30,r31.
 *
 * @param[in]     PubKey_ptr    - the private key database.
 * @param[in/out] PrivData_ptr  - the structure, containing DataIn and DataOut buffers.
 * @param[in] VirtualHwBaseAddr -  Virtual HW base address, passed by user.
 *
 * @return CRYSError_t - On success CRYS_OK is returned, on failure a
 *                        value MODULE_* as defined in .
 */
CRYSError_t LLF_PKI_ExecPrivKeyExpCrt(
                                     CRYSRSAPrivKey_t    *PrivKey_ptr ,
                                     CRYS_RSAPrimeData_t *PrivData_ptr,
                                     DxUint32_t           VirtualHwBaseAddr )
{
   /* LOCAL DECLARATIONS */

  /* error identification */
   CRYSError_t Error;

   /* modulus and exponents sizes in bytes */
   DxUint32_t  modSizeBytes, PQSizeInBytes;

   /* virtual registers pointers
      Note: don't change rQ = 6  */
   DxInt8_t  rN=0, rNP=1, rD=2, rT=3, rT1=4, rMq=5, rQ=6, rqInv=7;

   /* PKA tag - used for internal goals: if Tag == 1 and PKA debug flag is defined, then
      the PKA operation copies data in special buffer for debugging */
   DxUint8_t Tag = 1;


   /* FUNCTION LOGIC */

   /* ---------------------------------------------------------------------- */
   /* .................... initializations            ...................... */
   /* ---------------------------------------------------------------------- */

   /* error identifier */
   Error = CRYS_OK;

   /* modulus N size in bytes */
   modSizeBytes = (PrivKey_ptr->nSizeInBits + 7) / 8;

   /* check that there is enough memory for 9 PKA registers needed for
      CRT exponentiation */
#ifdef LLF_PKI_PKA_DEBUG
   if( LLF_PKI_PKA_MAX_REGS_MEM_SIZE_BYTES < 9*(modSizeBytes+4) )
      return LLF_PKI_PKA_NOT_ENOUGH_MEMORY_ERROR;
#endif

   /*  set Sizes table: 0- Nsize, 1- Nsize+1w (is done), 2- Psize  */
   LLF_PKI_PKA_SetRegSize( PrivKey_ptr->PriveKeyDb.Crt.PSizeInBits,    2/*EntryNum*/, VirtualHwBaseAddr );
   LLF_PKI_PKA_SetRegSize( PrivKey_ptr->PriveKeyDb.Crt.PSizeInBits+32, 3/*EntryNum*/, VirtualHwBaseAddr );

   /* P and Q size in bytes */
   PQSizeInBytes = (PrivKey_ptr->PriveKeyDb.Crt.PSizeInBits + 7) / 8;

   /*--------------------------------------------------------------*/
   /* PKA modular operations  according to modulus Q:              */
   /*--------------------------------------------------------------*/

   /* copy CRT parametersrs Q, dQ, QP into PKA registers */
   LLF_PKI_PKA_CopyDataIntoPkaReg( rN/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.Q/*src_ptr*/,
                                   PQSizeInBytes , VirtualHwBaseAddr );

   LLF_PKI_PKA_CopyDataIntoPkaReg( rD/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.dQ/*src_ptr*/,
                                   PQSizeInBytes , VirtualHwBaseAddr );

   LLF_PKI_PKA_CopyDataIntoPkaReg( rNP/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->LLF.Crt.QP/*src_ptr*/,
                                   4*LLF_PKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS , VirtualHwBaseAddr );
   /* copy DataIn into rT and rT! */
   LLF_PKI_PKA_CopyDataIntoPkaReg( rT/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivData_ptr->DataIn/*src_ptr*/,
                                   modSizeBytes , VirtualHwBaseAddr );
   LLF_PKI_PKA_Copy( 1/*LenID*/, rT1/*dest*/, rT/*src*/, 0/*Tag*/, VirtualHwBaseAddr );

   /*-----------------------------*/
   /*  Calculation of Mq          */
   /*-----------------------------*/
   /* reduction of the input data by modulus Q  rT = rT mod Q */
   LLF_PKI_PKA_Div( 0/*LenID*/, rT/*OpA*/, rN/*OpB=rN=Q*/, rQ/*Res not used*/, 1/*Tag*/, VirtualHwBaseAddr );

   /*  calculate of Mq = DataIn^dQ mod Q: Mq = rT^rD mod rN        */
   LLF_PKI_PKA_ModExp( 2/*LenID*/, rT/*OpA*/, rD/*OpB*/, rMq/*Res*/, Tag, VirtualHwBaseAddr );

   /*--------------------------------------------------------------*/
   /* PKA modular operations  according to modulus P:              */
   /*--------------------------------------------------------------*/

   /* copy prime factor P into rQ for swapping with rN */
   LLF_PKI_PKA_CopyDataIntoPkaReg( rQ/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.P/*src_ptr*/,
                                   PQSizeInBytes , VirtualHwBaseAddr );
   /* swap rQ <-> rN so that Q->rQ and P->rN */
   rQ = 0; rN = 6;
   /* set new value to N_NP_TO_T1 register according N->6, Np->1,T0->30,T1->31: 0x000FF826*/
   CRYS_PLAT_SYS_WriteRegister( VirtualHwBaseAddr + LLF_PKI_PKA_N_NP_T0_T1_REG_ADDR, 0x000FF826 );

   /* copy Barrett tag PP: PP=>NP */
   LLF_PKI_PKA_CopyDataIntoPkaReg( rNP/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->LLF.Crt.PP/*src_ptr*/,
                                   4*LLF_PKI_PKA_BARRETT_MOD_TAG_BUFF_SIZE_IN_WORDS , VirtualHwBaseAddr );

   /* copy priv. exponent factor dP: dP=>rD */
   LLF_PKI_PKA_CopyDataIntoPkaReg( rD/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.dP/*src_ptr*/,
                                   PQSizeInBytes , VirtualHwBaseAddr );

   /* copy qInv coefficient: qInv=>rqInv   */
   LLF_PKI_PKA_CopyDataIntoPkaReg( rqInv/*dstReg*/, 1/*LenID*/, (DxUint8_t*)PrivKey_ptr->PriveKeyDb.Crt.qInv/*src_ptr*/,
                                   PQSizeInBytes , VirtualHwBaseAddr );

   /*-----------------------------*/
   /*  Calculation of Mp          */
   /*-----------------------------*/
   /* reduction of input data by modulus P:  rT = rT1 mod P  */
   LLF_PKI_PKA_Div( 0/*LenID*/, rT1/*OpA and remainder*/, rN/*OpB*/, rT/*res not used*/, 0/*Tag*/, VirtualHwBaseAddr );

   /* calculate exponent Mp = DataIn^dP mod P , i.e: rT = rT1^rD mod rP  */
   LLF_PKI_PKA_ModExp( 2/*LenID*/, rT1/*OpA*/, rD/*exp*/, rT/*Res*/, Tag, VirtualHwBaseAddr );

   /*-------------------------------------------*/
   /* Calculation of  h = (Mp-Mq)*qInv mod P    */
   /*-------------------------------------------*/

   /* rT1 = Mq mod P - needed for right calculating in next operation if Mq>P */
   LLF_PKI_PKA_ModAddIm( 2/*LenID*/, rMq/*OpA*/, 0/*immed OpB*/, rT1/*Res*/, Tag, VirtualHwBaseAddr );

   /* rT = Mp - Mq mod P */
   LLF_PKI_PKA_ModSub( 2/*LenID*/, rT/*OpA*/, rT1/*OpB*/, rT/*Res*/, Tag, VirtualHwBaseAddr );

   /* rT1 = h = (Mp - Mq)*qInv mod P */
   LLF_PKI_PKA_ModMul( 2/*LenID*/, rT/*OpA*/, rqInv/*rqInv*/, rT1/*Res*/, Tag, VirtualHwBaseAddr );

   /*-----------------------------*/
   /*       M = Mq + Q*h;         */
   /*  OpSize according Nsize     */
   /*-----------------------------*/

   /* copy rT1 and Mq in other registers for clearing junk from registers high part  */
   LLF_PKI_PKA_Clr( 1/*LenID*/, 30/*dest*/, Tag, VirtualHwBaseAddr );
   LLF_PKI_PKA_Copy( 2/*LenID*/, rT/*dest*/, rT1/*src*/, 0/*Tag*/, VirtualHwBaseAddr );
   LLF_PKI_PKA_Clr( 1/*LenID*/, 30/*dest*/, Tag, VirtualHwBaseAddr );
   LLF_PKI_PKA_Copy( 2/*LenID*/, rT1/*dest*/, rMq/*src*/, 0/*Tag*/, VirtualHwBaseAddr );

   /* Q*h => rT = rQ*rT */
   LLF_PKI_PKA_LMul( 0/*LenID*/, rT/*OpA*/, rQ/*OpB*/, rT/*Res*/, Tag, VirtualHwBaseAddr );
   /* M = rT1 = rMq + rT */
   LLF_PKI_PKA_Add( 0/*LenID*/, rT1/*OpA*/, rT/*OpB*/, rT/*Res*/, Tag, VirtualHwBaseAddr );

   /*-----------------------------*/
   /*  Finish PKA and copy result */
   /*-----------------------------*/
   LLF_PKI_PKA_CopyDataFromPkaReg( (DxUint8_t*)PrivData_ptr->DataOut,  modSizeBytes,
                                   rT/*srcReg*/, VirtualHwBaseAddr );

 #ifndef LLF_PKI_PKA_DEBUG_
   /* clear used PKA registers (0-7 and 30,31) for security goals */
   LLF_PKI_PKA_ClearBlockOfRegs(  0/*FirstReg*/, 8/*Count*/, 1/*LenID*/, VirtualHwBaseAddr );
   LLF_PKI_PKA_ClearBlockOfRegs( 30/*FirstReg*/, 2/*Count*/, 1/*LenID*/, VirtualHwBaseAddr );
 #endif

   return Error;

}/* END OF LLF_PKI_ExecPrivKeyExpCrt */
