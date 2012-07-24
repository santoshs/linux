#  cc42.mk


#----------------------------------------------------------------
# target
#----------------------------------------------------------------
TARGET=cc42
ULIB = $(TARGET).a

#----------------------------------------------------------------
# include and symbol path
#----------------------------------------------------------------
INC0   = ../inc/GEN/inc
#INC1   = ../inc/INIT/inc
#INC2   = ../inc/NVS/inc
#INC3   = ../inc/NVS/NVS_Files
INC4   = ../inc/PLAT/inc
INC5   = ../inc/VOS6/BaseLayer/NoOs
INC6   = ../inc/FVOS/FVOS_API/inc
INC8   = ../inc/FVOS/FVOS_NoOS/inc
INC9   = ../inc/CRYS/CRYS/AES/inc
INC10  = ../inc/CRYS/CRYS/AESCCM/inc
INC11  = ../inc/CRYS/CRYS/AESGCM/inc
INC12  = ../inc/CRYS/CRYS/C2/inc
INC13  = ../inc/CRYS/CRYS/CCM/inc
INC14  = ../inc/CRYS/CRYS/COMMON/inc
INC15  = ../inc/CRYS/CRYS/CRYS_KMNG/inc
INC15  = ../inc/CRYS/CRYS/CRYS_SST/inc
INC16  = ../inc/CRYS/CRYS/DES/inc
INC17  = ../inc/CRYS/CRYS/ECC/ECCcommon/inc
INC18  = ../inc/CRYS/CRYS/HASH/inc
INC19  = ../inc/CRYS/CRYS/HMAC/inc
INC20  = ../inc/CRYS/CRYS/INIT/inc
INC21  = ../inc/CRYS/CRYS/KDF/inc
INC22  = ../inc/CRYS/CRYS/RC4/inc
INC23  = ../inc/CRYS/CRYS/RND/inc
INC24  = ../inc/CRYS/CRYS/RSA/inc
INC26  = ../inc/CRYS/LLF_AES/inc
INC27  = ../inc/CRYS/LLF_AESCCM/inc
INC28  = ../inc/CRYS/LLF_AESGCM/inc
INC29  = ../inc/CRYS/LLF_C2/inc
INC30  = ../inc/CRYS/LLF_COMMON/inc
INC31  = ../inc/CRYS/LLF_DES/inc
INC32  = ../inc/CRYS/LLF_HASH/inc
INC33  = ../inc/CRYS/LLF_PKI/inc
INC34  = ../inc/CRYS/LLF_PKI_EC/inc
INC35  = ../inc/CRYS/LLF_RC4/inc
INC36  = ../inc/CRYS/LLF_RND/inc
INC37  = ../inc/CRYS/LLFCD/DES/inc
INC38  = ../inc/CRYS/LLFCD/HASH/inc
INC39  = ../inc/CRYS/LLFCD/RC4/inc
INC41  = ../inc/VOS6/BaseLayer/Generic
INC42  = ../inc/VOS6/BaseLayer/NoOs/inc
INC43  = ../inc/VOS6/VOS_API
INC44  = ../inc/CRYS/CRYS_API/inc
INC45  = ../inc/CRYS/CRYS_API/tinc
INC46  = ../inc/SST/DX_SST6_API/inc
INC47  = ../inc/SST/SST6/inc
INC48  = ../inc/SST/SST6_AA/inc
INC49  = ../inc/SST/SST6_API/inc
INC50 =  ../inc/SST/SST6_DB/inc
INC51 =  ../inc/SST/SST6_DBG/inc
INC52 =  ../inc/SST/SST6_General/inc
INC53 = ../inc/SST/SST6_IX/inc
INC54 = ../inc/SST/SST6_VCRYS/inc
INC55 = ../inc/SST/SST6_VDB/inc
INC56 = ../inc/SST/SST6_VRPC/inc
INC57 = ../inc/KMNG_API/inc
#INC58 = ../src/APE/common/cc42/src/CRYS_ATP/HASH/tinc/
#INC59 = ../inc/CRYS_ATP/ATP_TST_MAIN/tinc
#INC60 = ../inc/CRYS_ATP/ATP_TST_UTIL/tinc
#INC61 = ../inc/MW_TST/ATP_TST/tinc
#INC62 = ../inc/MW_TST/UTIL/tinc
INC63 = ../../../inc 
INC64 = ../inc/CRYS/LLFCD/AES/inc
INC65 = ../inc/CRYS/CRYS/DH/inc
INC66 = ../inc/cc_hw_Interface/inc 
INC  = -I$(INC0)  -I$(INC4) -I$(INC5) -I$(INC6) -I$(INC8) -I$(INC9) -I$(INC10) \
       -I$(INC11) -I$(INC12) -I$(INC13) -I$(INC14) -I$(INC15) -I$(INC16) -I$(INC17) -I$(INC18) -I$(INC19) -I$(INC20) \
       -I$(INC21) -I$(INC22) -I$(INC23) -I$(INC24)  -I$(INC26) -I$(INC27) -I$(INC28) -I$(INC29) -I$(INC30) \
       -I$(INC31) -I$(INC32) -I$(INC33) -I$(INC34) -I$(INC35) -I$(INC36) -I$(INC37) -I$(INC38) -I$(INC39) \
       -I$(INC41) -I$(INC42) -I$(INC43) -I$(INC44) -I$(INC45) -I$(INC46) -I$(INC47) -I$(INC48) -I$(INC49) -I$(INC50) \
       -I$(INC51) -I$(INC52) -I$(INC53) -I$(INC54) -I$(INC55) -I$(INC56)  -I$(INC57)\
        -I$(INC63)-I$(INC64) -I$(INC65) -I$(INC66) 


#####CC#####
SRC1  = ../src/CRYS/CRYS/AES/src
SRC2  = ../src/CRYS/CRYS/AESCCM/src
SRC3  = ../src/CRYS/CRYS/AESGCM/src
SRC4  = ../src/CRYS/CRYS/C2/src
SRC5  = ../src/CRYS/CRYS/CCM/src
SRC6  = ../src/CRYS/CRYS/COMMON/src
#SRC7  = ../src/CRYS/CRYS/CRYS_KMNG/src
#SRC8  = ../src/CRYS/CRYS/CRYS_SST/src
SRC9  = ../src/CRYS/CRYS/DES/src
SRC10 = ../src/CRYS/CRYS/DH/src
SRC11 = ../src/CRYS/CRYS/ECC/EC_ELGAMAL/src
SRC12 = ../src/CRYS/CRYS/ECC/ECCcommon/src
#SRC13 = ../src/CRYS/CRYS/ECC/ECDH/src
#SRC14 = ../src/CRYS/CRYS/ECC/ECDSA/src
SRC15 = ../src/CRYS/CRYS/HASH/src
SRC16 = ../src/CRYS/CRYS/HMAC/src
#SRC17 = ../src/CRYS/CRYS/INIT/src
#SRC19 = ../src/CRYS/CRYS/KDF/src
SRC20 = ../src/CRYS/CRYS/RC4/src
SRC21 = ../src/CRYS/CRYS/RND/src
SRC22 = ../src/CRYS/CRYS/RSA/src
#SRC23 = ../src/CRYS/CRYS/SELF_TEST/src
SRC24 = ../src/CRYS/LLF_AES/src
SRC25 = ../src/CRYS/LLF_AESCCM/src
SRC26 = ../src/CRYS/LLF_AESGCM/src
SRC27 = ../src/CRYS/LLF_C2/src
SRC28 = ../src/CRYS/LLF_COMMON/src
SRC29 = ../src/CRYS/LLF_DES/src
SRC30 = ../src/CRYS/LLF_HASH/src
SRC31 = ../src/CRYS/LLF_PKI/src
SRC32 = ../src/CRYS/LLF_PKI_EC/src
SRC33 = ../src/CRYS/LLF_RC4/src
SRC34 = ../src/CRYS/LLF_RND/src
SRC35 = ../src/CRYS/LLFCD/AES/src
SRC36 = ../src/CRYS/LLFCD/DES/src
SRC37 = ../src/CRYS/LLFCD/HASH/src
SRC38 = ../src/CRYS/LLFCD/RC4/src
#SRC39 = ../src/CRYS_ATP/HASH/tsrc
SRC40 = ../src/FVOS/FVOS_NoOS/src
SRC41 = ../src/INIT/src
#SRC42 = ../src/NVS/NVS_Files/src
#SRC43 = ../src/NVS/NVS_RAM/src
#SRC44 = ../src/NVS/src
SRC45 = ../src/VOS6/BaseLayer/NoOs/src
SRC46 = ../src/PLAT/src
SRC47 = ../src/VOS6/IFLayer
SRC48 = ../src/VOS6/BaseLayer/Generic
SRC49 = ../src/cc_hw_Interface/src

VPATH = $(INC0):$(INC4):$(INC5):$(INC6):$(INC8):$(INC9):$(INC10):$(INC11):$(INC12):$(INC13)\
        :$(INC14):$(INC15):$(INC16):$(INC17):$(INC18):$(INC19):$(INC20):$(INC21):$(INC22):$(INC23):$(INC24)\
        :$(INC26):$(INC27):$(INC28):$(INC29):$(INC30):$(INC31):$(INC32):$(INC33):$(INC34):$(INC35):$(INC36):$(INC37)\
        :$(INC38):$(INC39):$(INC41):$(INC42):$(INC43):$(INC44):$(INC45):$(INC46):$(INC47):$(INC48):$(INC49)\
        :$(INC50):$(INC51):$(INC52):$(INC53):$(INC54):$(INC55):$(INC56):$(INC57)\
        :$(INC63):$(INC64):$(INC65):$(INC66)::$(SRC1):$(SRC2):$(SRC3):$(SRC4):$(SRC5):$(SRC6):$(SRC9)\
        :$(SRC10):$(SRC11):$(SRC12):$(SRC15)\
        :$(SRC16):$(SRC20):$(SRC21):$(SRC22):$(SRC24):$(SRC25):$(SRC26):$(SRC27):$(SRC28):$(SRC29)\
        :$(SRC30):$(SRC31):$(SRC32):$(SRC33):$(SRC34):$(SRC35):$(SRC36):$(SRC37):$(SRC38):$(SRC40):$(SRC41):$(SRC45)\
        :$(SRC46):$(SRC47):$(SRC48):$(SRC49)

#----------------------------------------------------------------
# option
#----------------------------------------------------------------
VIA_DIR = ../../../via
OPT     = $(VIA_DIR)/loader_opt.mk

include $(OPT)
#----------------------------------------------------------------
# object
#----------------------------------------------------------------
#####SECURITY#####
CCOBJ	=	        CRYS_COMMON.o \
			CRYS_COMMON_conv_endian.o \
			CRYS_COMMON_global_data.o \
			CRYS_COMMON_math.o \
                        CRYS_CCM.o \
			FVOS.o \
                        PLAT_SystemDep.o \
                        dx_vos_mem.o \
                        DX_VOS_BaseSem.o \
                        DX_VOS_BaseGenericMemMap.o \
                        DX_VOS_BaseThread.o \
                        DX_VOS_ThreadStorage.o \
                        DX_VOS_Stdio.o \
                        dx_vos_string.o \
                        DX_VOS_Utils.o\
                        CRYS_HASH.o \
                        LLF_HASH.o \
                        LLFCD_HASH.o \
                        LLF_AES.o\
                        LLFCD_AES.o \
                        CRYS_DES.o \
                        LLF_DES.o \
                        LLFCD_DES.o \
                        CRYS_HMAC.o\
                        CRYS_AES_CF.o \
                        LLF_AES_XTS.o\
                        LLF_AES_XCBC_CMAC.o\
                        LLF_COMMON.o\
                        cc_hw_interface.o\
                        CRYS_RC4.o \
                        LLF_RC4.o \
                        LLFCD_RC4.o \
                        CRYS_C2_Cipher.o \
                        LLF_C2_Cipher.o \
                        CRYS_RSA_BUILD.o\
                        CRYS_RSA_PRIM.o \
                        CRYS_RND.o \
                        LLF_PKI_RSA.o \
                        LLF_PKI_PKA.o \
                        LLF_PKI_Exp.o \
                        LLF_AES_Restrict.o\
                        LLF_RND.o \
                        LLF_RND_CF_port.o\
                        CRYS_DH.o\
                        CRYS_EC_ELGAMAL.o \
                         LLF_ECPKI_Domains.o \
                        LLF_ECPKI_GenKey.o\
                         LLF_ECPKI_ModularArithmetic.o\
                         LLF_ECPKI_ECArithmetic.o \
                         LLF_ECPKI_Export.o\
                         CRYS_ECPKI_BUILD.o
                         
                   
                                              


OBJS	=	$(CCOBJ) 
			

.PHONY : allclean clean all
all: $(ULIB)
#----------------------------------------------------------------
# dependence
#----------------------------------------------------------------
$(OBJS)                     : $(TARGET).mk $(OPT)

CRYS_COMMON.o               :DX_VOS_Mem.h DX_VOS_Sem.h CRYS_COMMON_error.h CRYS.h LLF_COMMON.h
CRYS_COMMON_conv_endian.o   :CRYS.h  CRYS_COMMON.h CRYS_COMMON_math.h CRYS_COMMON_error.h
CRYS_COMMON_global_data.o   :DX_VOS_Mem.h CRYS_COMMON_error.h CRYS.h
CRYS_COMMON_math.o          :CRYS.h CRYS_COMMON_math.h CRYS_COMMON_error.h
CRYS_CCM.o                  :DX_VOS_Errors.h CRYS.h CRYS_CCM.h CRYS_CCM_error.h PLAT_SystemDep.h
FVOS.o                         : DX_VOS_Mem.h FVOS_Types.h FVOS_API.h FVOS_Config.h FVOS_HwDefs.h gen.h FVOS_Error.h
PLAT_SystemDep.o               : PLAT_SystemDep.h CRYS_Defs.h DX_VOS_BaseTypes.h CRYS_test_flags.h
dx_vos_mem                     : DX_VOS_Mem.h DX_VOS_BaseMem.h 
DX_VOS_BaseSem.o               : DX_VOS_Sem.h
DX_VOS_BaseGenericMemMap.o     : DX_VOS_Memmap.h
DX_VOS_BaseThread.o            : DX_VOS_Thread.h
DX_VOS_ThreadStorage.o         : DX_VOS_ThreadStorage.h DX_VOS_Mem.h DX_VOS_Thread.h
DX_VOS_Stdio.o                 : DX_VOS_Stdio.h DX_VOS_Utils.h DX_VOS_String.h DX_VOS_Mem.h log_output.h DX_VOS_File.h \
                                 DX_VOS_BaseStdio.h
dx_vos_string.o                : DX_VOS_String.h DX_VOS_Mem.h
DX_VOS_Utils.o                 : DX_VOS_Utils.h DX_VOS_Mem.h DX_VOS_String.h
CRYS_HASH.o                     :DX_VOS_Mem.h CRYS_HASH_error.h CRYS.h CRYS_COMMON.h CRYS_HASH_Local.h CRYS_CCM.h\
                                 CRYS_COMMON_math.h LLF_HASH.h
LLF_HASH.o                      :DX_VOS_Mem.h DX_VOS_Sem.h DX_VOS_Memmap.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                 CRYS_COMMON_math.h LLF_HASH.h LLF_HASH_error.h LLF_HASH_HwDefs.h LLFCD_HASH.h \
                                 LLF_COMMON.h DX_VOS_BaseTypes.h CRYS_test_flags.h
LLFCD_HASH.o                   :DX_VOS_Mem.h PLAT_SystemDep.h CRYS.h LLF_HASH_HwDefs.h LLFCD_HASH.h \
                                DX_VOS_BaseTypes.h CRYS_test_flags.h
LLF_AES.o                      : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                 CRYS_COMMON_math.h CRYS_AES_error.h LLF_COMMON.h LLF_AES.h LLF_AES_error.h \
                                 LLF_AES_HwDefs.h DX_VOS_Sem.h
LLFCD_AES.o                    : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h\
                                  CRYS_COMMON_math.h LLF_AES.h LLF_AES_error.h LLF_AES_HwDefs.h LLFCD_AES.h
CRYS_DES.o                     : DX_VOS_Mem.h CRYS_CCM.h CRYS.h CRYS_COMMON.h CRYS_DES_error.h CRYS_DES_Local.h
LLF_DES.o                     : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                CRYS_COMMON_math.h LLF_COMMON.h LLF_DES.h LLF_DES_error.h LLF_DES_HwDefs.h LLFCD_DES.h \
                                LLF_COMMON.h
LLFCD_DES.o                    : DX_VOS_Mem.h PLAT_SystemDep.h CRYS.h LLF_DES_HwDefs.h LLFCD_DES.h
CRYS_HMAC.o                    : DX_VOS_Mem.h CRYS.h CRYS_HMAC_error.h CRYS_HMAC_Local.h CRYS_CCM.h CRYS_COMMON.h 
CRYS_AES_CF.o                  : DX_VOS_Mem.h CRYS.h CRYS_COMMON.h CRYS_COMMON_math.h CRYS_AES_Local.h CRYS_AES_error.h \
                                 CRYS_CCM.h
LLF_AES_XTS.o                  : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                 CRYS_COMMON_math.h CRYS_AES_error.h LLF_COMMON.h LLF_AES.h \
                                 LLF_AES_error.h LLF_AES_HwDefs.h LLFCD_AES.h
LLF_AES_XCBC_CMAC.o            : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                 CRYS_COMMON_math.h CRYS_AES_error.h LLF_COMMON.h LLF_AES.h \
                                 LLF_AES_error.h LLF_AES_HwDefs.h LLFCD_AES.h
LLF_COMMON.o                     :DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                  LLF_COMMON_error.h LLF_COMMON_HwDefs.h DX_VOS_BaseTypes.h CRYS_test_flags.h
cc_hw_interface.o              : DX_VOS_BaseTypes.h cc_hw_interface.h host_hw_defs.h LLF_COMMON_HwDefs.h FVOS_API.h \
                                 error.h
CRYS_RC4.o                     : DX_VOS_Mem.h CRYS.h CRYS_CCM.h CRYS_RC4_error.h CRYS_RC4_Local.h LLF_RC4.h
LLF_RC4.o                      : DX_VOS_Sem.h DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_BaseTypes.h PLAT_SystemDep.h LLF_COMMON.h \
                                 CRYS.h LLF_RC4.h LLF_RC4_error.h LLF_RC4_HwDefs.h LLFCD_RC4.h cc_hw_interface.h
LLFCD_RC4.o                    : DX_VOS_Mem.h PLAT_SystemDep.h CRYS.h LLF_RC4.h LLF_RC4_HwDefs.h LLFCD_RC4.h
CRYS_C2_Cipher.o              : DX_VOS_Mem.h CRYS_C2_error.h CRYS.h CRYS_CCM.h CRYS_C2_Local.h CRYS_C2.h LLF_C2.h
LLF_C2_Cipher.o               : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h LLF_C2.h DX_VOS_BaseTypes.h PLAT_SystemDep.h \
                                CRYS.h CRYS_COMMON_math.h LLF_C2_error.h LLF_C2_HwDefs.h cc_hw_interface.h 
CRYS_RSA_BUILD.o                : DX_VOS_Mem.h CRYS.h CRYS_COMMON.h CRYS_COMMON_math.h CRYS_RSA_error.h CRYS_RSA_Local.h \
                                  LLF_PKI_RSA.h
CRYS_RSA_PRIM.o                 : DX_VOS_Mem.h CRYS.h CRYS_COMMON.h CRYS_COMMON_math.h CRYS_RSA_error.h CRYS_RSA_Local.h \
                                  LLF_PKI_RSA.h PLAT_SystemDep.h
CRYS_RND.o                      : DX_VOS_Mem.h DX_VOS_Sem.h CRYS.h LLF_AES_Restrict.h CRYS_RND_error.h CRYS_RND_local.h \
                                  LLF_RND.h CRYS_COMMON.h CRYS_COMMON_math.h 
LLF_PKI_RSA.o                   :DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS_COMMON_math.h \
                                CRYS.h LLF_PKI.h LLF_PKI_error.h LLF_PKI_RSA.h DX_VOS_Sem.h
LLF_PKI_PKA.o                   : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_BaseTypes.h CRYS.h PLAT_SystemDep.h \
                                  CRYS_COMMON.h CRYS_COMMON_math.h LLF_PKI_HwDefs.h LLF_PKI.h LLF_PKI_error.h \
                                  LLF_PKI_RSA.h DX_VOS_Sem.h
LLF_PKI_Exp.o                   : DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h CRYS_COMMON_math.h CRYS_COMMON.h LLF_PKI.h \
                                  LLF_PKI_error.h LLF_PKI_HwDefs.h DX_VOS_Mem.h 
LLF_AES_Restrict.o              : DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_Sem.h DX_VOS_BaseTypes.h PLAT_SystemDep.h CRYS.h \
                                 CRYS_AES_error.h LLF_AES.h LLF_AES_error.h LLF_AES_HwDefs.h LLF_AES_Restrict.h
LLF_RND.o                       : DX_VOS_Sem.h DX_VOS_Mem.h DX_VOS_Memmap.h DX_VOS_BaseTypes.h PLAT_SystemDep.h \
                                  CRYS_COMMON_math.h CRYS.h CRYS_RND_local.h CRYS_RND_error.h LLF_RND.h LLF_RND_error.h \
                                  LLF_AES_Restrict.h LLF_AES_HwDefs.h LLF_RND_HwDefs.h
LLF_RND_CF_port.o               : DX_VOS_BaseTypes.h CRYS.h CRYS_RND.h
CRYS_DH.o                      : DX_VOS_Mem.h CRYS_DH_errors.h CRYS.h CRYS_CCM.h CRYS_DH_local.h PLAT_SystemDep.h \
                                 CRYS_COMMON.h CRYS_COMMON_math.h CRYS_RSA_BUILD.h
CRYS_EC_ELGAMAL.o              : DX_VOS_Mem.h CRYS.h CRYS_COMMON.h CRYS_CCM.h PLAT_SystemDep.h CRYS_COMMON_math.h \
                                 CRYS_ECPKI_error.h CRYS_ECPKI_Local.h LLF_ECPKI_Export.h
LLF_ECPKI_Domains.o            : DX_VOS_BaseTypes.h CRYS_HASH.h CRYS_ECPKI_Types.h
LLF_ECPKI_GenKey.o         : DX_VOS_Mem.h CRYS_RND.h CRYS_ECPKI_KG.h CRYS_ECPKI_error.h CRYS_ECPKI_Local.h \
                                 CRYS_COMMON.h CRYS_COMMON_math.h LLF_PKI.h LLF_ECPKI_ECArithmetic.h \
                                 LLF_ECPKI_ModularArithmetic.h LLF_ECPKI_EngineInfo.h
LLF_ECPKI_ModularArithmetic.o  : DX_VOS_BaseTypes.h DX_VOS_Mem.h CRYS_Defs.h CRYS_COMMON_math.h CRYS_RND.h LLF_PKI.h \
                                  LLF_PKI_error.h CRYS_ECPKI_Types.h LLF_ECPKI_error.h LLF_ECPKI.h \
                                  LLF_ECPKI_ModularArithmetic.h
LLF_ECPKI_ECArithmetic.o       : DX_VOS_BaseTypes.h DX_VOS_Mem.h CRYS_Defs.h PLAT_SystemDep.h CRYS_COMMON_math.h LLF_PKI.h \
                                 CRYS_ECPKI_Types.h LLF_ECPKI_error.h LLF_ECPKI_ModularArithmetic.h LLF_ECPKI_ECArithmetic.h
LLF_ECPKI_Export.o             : DX_VOS_Mem.h DX_VOS_Memmap.h CRYS_RND.h PLAT_SystemDep.h CRYS_COMMON.h CRYS_COMMON_math.h \
                                 CRYS_ECPKI_Types.h CRYS_ECPKI_KG.h CRYS_ECPKI_Local.h LLF_ECPKI_error.h LLF_ECPKI.h \
                                 LLF_ECPKI_Export.h LLF_ECPKI_ECArithmetic.h LLF_ECPKI_ModularArithmetic.h DX_VOS_Sem.h
CRYS_ECPKI_BUILD.o             : DX_VOS_Mem.h CRYS.h CRYS_COMMON.h CRYS_COMMON_math.h CRYS_ECPKI_error.h CRYS_ECPKI_Local.h

#---------------------------------------------------------------
#COMPLILE FLAGS
#---------------------------------------------------------------
CC_CCFLAGS = -DLITTLE__ENDIAN \
          -DCRYS_NO_OTF_SUPPORT \
          -DCRYS_NO_EXT_IF_MODE_SUPPORT \
          -DCRYS_NO_CMLA_SUPPORT \
          -DCRYS_NO_AESCCM_SUPPORT \
          -DCRYS_NO_OTF_MC_SUPPORT \
          -DCRYS_AES_CF_VERSION_USED \
          -DUSE_DX_FS_INIT \
          -DCRYS_PKA_SRAM_DATA_MEMORY_MODE \
          -DCRYS_NO_EXT_IF_MODE_SUPPORT \
          -DDX_USE_BOARD_DMA_AREA \
          -DCRYS_NO_CONTEXT_ENCRYPTION_PROTECTION \
          -DDX_NO_DEBUG_MEM \
          -DDX_NO_FILE_UTILS
#----------------------------------------------------------------
# LINK START
#----------------------------------------------------------------

include $(VIA_DIR)/loader_lib.mk
