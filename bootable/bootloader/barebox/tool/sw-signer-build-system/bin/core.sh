#!/bin/bash
#
# ############################################################################
# #    Copyright © Renesas Mobile Corporation 2012 . All rights reserved     #
# ############################################################################
# #                                                                          #
# # This material, including documentation and any related source code and   #
# # information, is protected by copyright controlled by Renesas. All rights #
# # are reserved. Copying, including reproducing, storing, adapting,         #
# # translating and modifying, including decompiling or reverse engineering, #
# # any or all of this material requires the prior written consent           #
# # of Renesas. This material also contains confidential information,        #
# # which may not be disclosed to others without the prior written consent   #
# # of Renesas.                                                              #
# #                                                                          #
# ############################################################################
#
# Main script for SW signing.
# 

set -ue

###############################################################################
#
# Attach UTC time stamp to image name
#
###############################################################################

TIMESTAMP=${TIMESTAMP:-`date -u +%Y%m%d%H%MZ`}

###############################################################################
#
# General settings
#
###############################################################################

# Symlink to product data 
test -h ${PRODUCT} && rm -f ${PRODUCT}
ln -s ${SIGN_PRODUCT_PATH} ${PRODUCT}

#
# Read environment parameters from product and default configuration 
#

source ./bin/defaults.source

#
# INI files
#

EMMC_1STIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.img1.ini
EMMC_2NDIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.img2.ini
EMMC_3RDIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.img3.ini

FLASHING_1STIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.flash1.ini
FLASHING_2NDIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.flash2.ini
FLASHING_3RDIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.flash3.ini

FINAL_1STIMAGE_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.final1.ini

SECU_INI=${SIGN_INI_FILES}/${TARGET_PLATFORM}.${FORM_FACTOR}.secu.ini

#
# EMMC images
#
#EMMC_1STIMAGE=${BOOT_SOURCE}_1stimg.bin
#EMMC_2NDIMAGE=${BOOT_SOURCE}_2ndimg.bin
EMMC_1STIMAGE=r_loader_1st.img
EMMC_2NDIMAGE=r_loader_2nd.img
EMMC_3RDIMAGE=${BOOT_SOURCE}_3rdimg.bin

#
# Flashing images
#
#FLASH_1STIMAGE=${BOOT_SOURCE}_1stimg_${UPDATE_METHOD}.bin
#FLASH_2NDIMAGE=${BOOT_SOURCE}_2ndimg_${UPDATE_METHOD}.bin
FLASH_1STIMAGE=flashwriter_1st.img
FLASH_2NDIMAGE=flashwriter_2nd.img
FLASH_3RDIMAGE=${BOOT_SOURCE}_3rdimg_${UPDATE_METHOD}.bin

#
# Final images
#
FINAL_1STIMAGE=${BOOT_SOURCE}_1stimg_${UPDATE_METHOD}_${TIMESTAMP}.bin
FINAL_2NDIMAGE=${BOOT_SOURCE}_2ndimg_${UPDATE_METHOD}_${TIMESTAMP}.bin
FINAL_3RDIMAGE=${BOOT_SOURCE}_3rdimg_${UPDATE_METHOD}_${TIMESTAMP}.bin



#
# Environment configuration
#
echo "SW image signer settings:"
echo "  HW engine: $TARGET_PLATFORM"
echo "  HW type: $FORM_FACTOR"
echo "  Boot mode: $BOOT_SOURCE"


###############################################################################
#
# Create and/or clean output directories
#
###############################################################################

test -d ${SIGN_OUTPUT_FILES} || mkdir -p ${SIGN_OUTPUT_FILES}
rm -rf ${SIGN_OUTPUT_FILES}/*

# Intermediate components
tempdir=${SIGN_OUTPUT_FILES}/components
test -d ${tempdir} || mkdir -p ${tempdir}
rm -f ${tempdir}/*

# Output images
resultdir=${SIGN_OUTPUT_FILES}/images
test -d ${resultdir} || mkdir -p ${resultdir}
rm -f ${resultdir}/*

if [ "${TARGET_PLATFORM}" == 'eos2' ]; then
# External certificates
certdir=${SIGN_OUTPUT_FILES}/certs
test -d ${certdir} || mkdir -p ${certdir}
rm -f ${certdir}/*

# Security filesystem components
securodir=${tempdir}/securityro
test -d ${securodir} || mkdir -p ${securodir}
rm -f ${securodir}/*

securwdir=${tempdir}/securityrw
test -d ${securwdir} || mkdir -p ${securwdir}
rm -f ${securwdir}/*
fi

###############################################################################
#
# Debug and logging
#
###############################################################################

DEBUGDIR="$(mktemp -d ${SIGN_OUTPUT_FILES}/siss_XXXXXXXX)"
trap 'status=$?; exit ${status}' EXIT

SCRIPTLOG=${DEBUGDIR}/build.sh.log
TODEVNULL="&> ${SCRIPTLOG}"

dosrun=`uname -a | grep -i cygwin || true`

if [ -z "${dosrun}" ]; then
  exeext=""
  SISS_DEBUG="--debug_dir ${DEBUGDIR}"
  _pathconv=echo
else
  exeext=".exe"
  SISS_DEBUG="--debug_dir `cygpath -w ${DEBUGDIR}`"
  _pathconv="cygpath -a -w"
  _pathconv=echo
fi

###############################################################################
#
# Tools
#
###############################################################################

SIGNER=${SIGN_TOOL_PATH:-}/siss${exeext}
FLASH_CREATOR=${SIGN_TOOL_PATH:-}/flic.pl
MAKE_EXT4FS=${SIGN_TOOL_PATH:-}/make_ext4fs

###############################################################################
#
# Whitelist bit infromation
#
###############################################################################

#
# Whitelist bits in *.ini files
#
# cellular image:     36
# minihost image:     35
#

#
# Used by function build_absref_ini
#
wlbit_ramset=32  
wlbit_mlo=33  
wlbit_algo=34  

#
# Whitelist bit used by __build.sh further down
#
wlbit_plmn=40

###############################################################################
#
# Helper functions
#
###############################################################################


function pathconv {
    ${_pathconv} $1
}

function elfcheck {
    if [ ! -r $1 ]; then
        echo "ERROR: $1 does not exist" 1>&2
        exit -1
    fi
    if [ ! -z `perl -e '$f=shift;open F, "<$f"; read F,$m,4; ($a)=unpack("V",$m);if (0x464c457f != $a){print "NOELF";};' $1` ]; then
        echo "ERROR: $1 is not an ELF file" 1>&2
        exit -1
    fi
}

function elf_entry_point {
    elfcheck $1
    perl -e '$f=shift;open F, "<$f"; read F,$m,(16+2+2+4); read F,$m,4; ($a)=unpack("V",$m); printf("0x%08X", $a);' $1
}

function elf_start_address {
    elfcheck $1
    perl -e '$f=shift;open F, "<$f"; read F,$m,(16+2+2+4+4); read F,$m,4; ($p)=unpack("V",$m);seek F,$p+8,0;read F,$a,4;($v)=unpack("V",$a); printf("0x%08X",$v);' $1
}

function elf_to_binary {
    elfcheck $1
    perl -e '$f=shift;open F, "<$f"; read F,$m,(16+2+2+4+4); read F,$m,4; ($p)=unpack("V",$m);seek F,$p+4,0;read F,$l,(4+4+4+4);($o,$f1,$f2,$s)=unpack("VVVV",$l); seek F,$o,0;read F,$b,$s; print $b;' $1 > $2
}

function build_absref_ini {
    eval wlb=\$wlbit_$1
    echo "[swsign]"
    echo "common_flags = 0"
    echo "operator_variant = 0x0"
    echo "whitelist_bit = $wlb"
    echo "count = 1"
    echo
    echo "[swsign1]"
    echo "filepath = `pathconv $2` "
    echo "flags = 0x0"
    echo "entry = `elf_entry_point   $3`"
    echo "start = `elf_start_address $3`"
    echo "check_interval = 0x0"   
    echo "data32_object_id = 0x20464552"
}


function _run_signer {
    echo -n $1
    [ "${debug:-}" != "" ] && echo ": $3 $2"
    ( $3 $2 > ${SCRIPTLOG} &&                                                           \
      echo "done")   || (echo                           &&                              \
                         echo "ERROR:"                  &&                              \
                         echo "$3 $2"                   &&                              \
                         cat ${SCRIPTLOG}               &&                              \
                         echo                           &&                              \
                         echo "Logs stored to directory ${SIGN_OUTPUT_FILES}/${DEBUGDIR##*/}" &&    \
                         exit -1)
}
                             

function run_signer {
    _run_signer "$1" "$2" "${SIGNER}"
}

function get_buildid {
    perl -e '$$f=shift; open F, $$f; read F, $$raw, -s $$f; '   \
         -e '$$h=join("", unpack("H*", $$raw ));'               \
         -e '$$pat=shift; $$pat .= "00000000"; '                \
         -e '($$buildid) = ($$h =~ /$$pat(.{16})/); '           \
         -e 'print "$$buildid\n";'                              \
         ${1} ${2}
}

function extract_issw {

    perl -e '$f=shift;open F,"<$f";for($i=0;$i<10;$i++){'               \
         -e ' read F,$t,0x20;($o,$s,$d0,$d1,$d3,$n)=unpack("V6",$t);'   \
         -e ' if(0x57535349==$n){'                                      \
         -e '  seek F,$o,0;read F,$b,$s; print $b; exit;'               \
         -e ' }'                                                        \
         -e '}' $1
}

function get_ini_wlb {
    perl -e 'while(<>){if(($wlb)= ($_=~/whitelist_bit\s*=\s*([0-9]+)/)){printf("$wlb");}};' ${1}
}

###############################################################################
#
# Offline mode
#
###############################################################################

if [ -z "${OFFLINE_MODE:-}" ]; then
    SISS_SERVER="--server ${SIGN_SERVER}"
    SISS_SERVER_RETRY="--server_retry ${SIGN_SERVER_RETRY}"
    SISS_PROD_ID="--product_id ${PRODUCT_ID}"
    SISS_OFFLINE=
    PREFIX=
else
    echo -n "Offline mode..."
    SISS_SERVER=
    SISS_SERVER_RETRY=
    SISS_PROD_ID=

    OFFLINE_INI_TEMPLATE=./offline/ini/offline.ini.sample
    OFFLINE_INI=${tempdir}/offline.ini
    SISS_OFFLINE="--ini_offline ${OFFLINE_INI}"
    PREFIX="offline."

    if [ "${OFFLINE_RKH:-}" == "231c9059" ]; then
        echo -n "RKH accepted..."
    else
        echo -n "RKH rejected"
        exit -1
    fi

    sed -e "s#OFFLINE_ISSW_TEMPLATE#${OFFLINE_ISSW}#"                                    \
        -e "s#OFFLINE_KEY_TEMPLATE#${OFFLINE_KEY_DIRECTORY}#" ${OFFLINE_INI_TEMPLATE} >  \
        ${OFFLINE_INI}

    perl -e '$$rkh=shift;for($$x=0;$$x<4;$$x++){ ' \
         -e '  printf("rkh%d=0x%02X\n", $$x, 0xFF & ((hex $$rkh) >> (8*(3-$$x))));' \
         -e '}' ${OFFLINE_RKH} >>         ${OFFLINE_INI}

    cp ${OFFLINE_ISSW} ${tempdir}/ISSW.${OFFLINE_RKH}

    echo "done"

fi


if [ "${SIGNING_SUPPORT}" == 'TRUE' ]; then
###############################################################################
#
# Check compatibility of ISSW and SA
#
###############################################################################

if [ -z "${OFFLINE_MODE:-}" ]; then
    ISSW_SECRELEASE=${SECRELEASE_DIR}/ISSW.secsigned
else
    ISSW_SECRELEASE=${OFFLINE_ISSW}
fi
SA_SECRELEASE=${SECRELEASE_DIR}/SA.secsigned

echo

if [ ! -r "${ISSW_SECRELEASE}" ] ; then
    echo "ERROR: Can't read file ${ISSW_SECRELEASE}"
    exit -1
fi

if [ ! -r "${SA_SECRELEASE}" ] ; then
    echo "ERROR: Can't read file ${SA_SECRELEASE}"
    exit -1
fi

issw_buildid=`get_buildid ${ISSW_SECRELEASE} 57535349`
sa_buildid=`get_buildid ${SA_SECRELEASE}  53412020`

if [ -z "${issw_buildid:-}" ] ; then
    echo "ERROR: Can't find ISSW build id?";
    exit -1;
fi

if [ "${issw_buildid}" == "${sa_buildid}" ]; then
    echo
else
    echo "ERROR: ${ISSW_SECRELEASE} and ${SA_SECRELEASE} are not from the same release"
    exit -1;
fi

fi

###############################################################################
###############################################################################
###############################################################################
#
# Version information
#
###############################################################################

echo -n "Signing environment: "
if [ -r build.id ]; then
    cat build.id;
else
    echo "R&D"
fi

echo -n "SW signer tool: "
${SIGNER} -version
echo

###############################################################################
#
# RAMSET
#
###############################################################################

RAMSET_INI=${tempdir}/normalboot.generated.ramset.ini
RAMSET_RAW=${tempdir}/normalboot.ramset.raw

if [ "${UPDATE_METHOD}" == 'flashing' ]; then
FLASHING_RAMSET_INI=${tempdir}/flashing.generated.ramset.ini
FLASHING_RAMSET_RAW=${tempdir}/flashing.ramset.raw
fi

#
echo -n "Prepare RAMSET file(s)..."
#
build_absref_ini ramset ${RAMSET_RAW}          ${RAMSET}          > ${RAMSET_INI}
elf_to_binary ${RAMSET}     ${RAMSET_RAW}

if [ "${UPDATE_METHOD}" == 'flashing' ]; then
    build_absref_ini ramset ${FLASHING_RAMSET_RAW} ${FLASHING_RAMSET} > ${FLASHING_RAMSET_INI}
    elf_to_binary ${FLASHING_RAMSET} ${FLASHING_RAMSET_RAW}
fi

echo "done"

###############################################################################
#
# LOADER
#
###############################################################################

LOADER_INI=${tempdir}/normalboot.generated.loader.ini
LOADER_RAW=${tempdir}/normalboot.loader.raw

#
echo -n "Prepare LOADER file(s)..."
#
build_absref_ini mlo ${LOADER_RAW}          ${LOADER}          > ${LOADER_INI}
elf_to_binary ${LOADER}     ${LOADER_RAW}

echo "done"


if [ "${UPDATE_METHOD}" == 'flashing' ]; then
###############################################################################
#
# ALGO
#
###############################################################################

FLASHING_ALGO_INI=${tempdir}/flashing.generated.algo.ini
FLASHING_ALGO_RAW=${tempdir}/flashing.algo.raw

#
echo -n "Prepare ALGO file(s)..."
#
build_absref_ini algo ${FLASHING_ALGO_RAW}          ${FLASHING_ALGO}          > ${FLASHING_ALGO_INI}
elf_to_binary ${FLASHING_ALGO}     ${FLASHING_ALGO_RAW}

echo "done"
fi

if [ "${SIGNING_SUPPORT}" == 'TRUE' ]; then
###############################################################################
#
# Modem SW
#
###############################################################################

CELLULAR_INI=${SIGN_INI_FILES}/cv.ini

fi

if [ "${TARGET_PLATFORM}" != 'eos2' ]; then
###############################################################################
#
# Minihost SW
#
###############################################################################

MH_INI=${SIGN_INI_FILES}/mh.ini

fi

if [ "${SIGNING_SUPPORT}" == 'TRUE' ]; then
###############################################################################
#
# PLMN whitelist
#
###############################################################################

PLMN_INI=${SIGN_INI_FILES}/plmn.ini

fi

###############################################################################
#
# IMEISV
#
###############################################################################

echo "$${IMEI_SVN:-01}" | perl -ane 'printf("%s%c", substr($$F[0],0,2),0);last;' > ${tempdir}/SVN


if [ "${SIGNING_SUPPORT}" == 'TRUE' ]; then
###############################################################################
#
# Show Watchdog configuration
#
###############################################################################
#
echo -n "Watchdog configuration list for ${PRODUCT}: "
#
wl_bits=""
 
wl_bits="`get_ini_wlb ${RAMSET_INI}` `get_ini_wlb ${LOADER_INI}` `get_ini_wlb ${CELLULAR_INI}` `get_ini_wlb ${PLMN_INI}`"
if [ "${UPDATE_METHOD}" == 'flashing' ]; then
wl_bits="${wl_bits} `get_ini_wlb ${FLASHING_ALGO_INI}`"
fi
if [ "${TARGET_PLATFORM}" != 'eos2' ]; then
wl_bits="${wl_bits} `get_ini_wlb ${MH_INI}`"
fi 

echo ${wl_bits} | perl -ane 'printf("%s\n", join(",", sort @F));';
fi

if [ "${SIGNING_SUPPORT}" == 'TRUE' ]; then
###############################################################################
#
# Create certificates for product
#
###############################################################################

run_signer "Create certificates for ${PRODUCT}..." \
           " ${SISS_DEBUG} \
             ${SISS_OFFLINE} \
             ${SISS_SERVER} \
             ${SISS_SERVER_RETRY} \
             ${SISS_PROD_ID} \
             --operator_variant ${OPERATOR_VARIANT} \
             --production_mode ${PRODUCTION_MODE} \
             --prodconf ${PRODUCT_CONFIG_ID_SECURE_EMMC} ${PRODUCT_CONFIG_ID_PUBLIC_EMMC} \
             --paramfile ${SIGN_INI_FILES}/${PREFIX}${PRODUCT}.signer.cfg"
fi

###############################################################################
#
# EMMC 1st image 
#
###############################################################################

#
echo -n "Create EMMC 1st image..."
#

${FLASH_CREATOR} --align 512 \
                --optional ${SIGN_INI_FILES}/TC,0x800 \
                --offset TC,0x9800 \
                --sub ${EMMC_1STIMAGE_INI} \
                 > ${tempdir}/${EMMC_1STIMAGE}

if [ -z "${OFFLINE_MODE:-}" ] ; then
    echo -n "offline ISSW..."
    for i1 in `pathconv ${tempdir}/${EMMC_1STIMAGE} `*; do
        extract_issw ${i1} > ${tempdir}/${BOOT_SOURCE}.ISSW.online.signed
    done
fi

echo "done"

###############################################################################
#
# EMMC 2nd image 
#
###############################################################################

#
echo -n "Create EMMC 2nd image..."
#

${FLASH_CREATOR} --align 512 \
                 --sub ${EMMC_2NDIMAGE_INI} \
                 > ${tempdir}/${EMMC_2NDIMAGE}


echo "done"

if [ "${TARGET_PLATFORM}" != 'eos2' ]; then
###############################################################################
#
# EMMC 3rd image 
#
###############################################################################

#
echo -n "Create EMMC 3rd image..."
#

if [ -z "${OFFLINE_MODE:-}" ]; then
    echo -n ""
else
    echo -n "PKC..."
    mv ${tempdir}/pkc.bin ${tempdir}/pkc.bin.${OFFLINE_RKH}
fi

#
echo -n "SECU..."
#

${FLASH_CREATOR} --align 512 \
                 --sub ${SECU_INI} \
                 > ${tempdir}/SECU


${FLASH_CREATOR} --align 512 \
                 --sub ${EMMC_3RDIMAGE_INI} \
                 > ${tempdir}/${EMMC_3RDIMAGE}


echo "done"
fi

if [ "${UPDATE_METHOD}" == 'flashing' ]; then

if [ "${SIGNING_SUPPORT}" == 'TRUE' ]; then
###############################################################################
#
# Create certificates for product - FLASH part
#
###############################################################################

run_signer "Create flash certificates for ${PRODUCT}..." \
           " ${SISS_DEBUG} \
             ${SISS_OFFLINE} \
             ${SISS_SERVER} \
             ${SISS_SERVER_RETRY} \
             ${SISS_PROD_ID} \
             --operator_variant ${OPERATOR_VARIANT} \
             --production_mode ${PRODUCTION_MODE} \
             --prodconf ${PRODUCT_CONFIG_ID_SECURE_FLASHING} ${PRODUCT_CONFIG_ID_PUBLIC_FLASHING} \
             --paramfile ${SIGN_INI_FILES}/${PRODUCT}.signer.flash.cfg"
fi


###############################################################################
#
# USB boot 1st image 
#
###############################################################################
#
echo -n "Create USB boot 1st image..."
#

${FLASH_CREATOR} --align 512 \
                --optional ${SIGN_INI_FILES}/TC,0x800 \
                --offset TC,0x9800 \
                --sub ${FLASHING_1STIMAGE_INI} \
                 > ${tempdir}/${FLASH_1STIMAGE}

if [ -z "${OFFLINE_MODE:-}" ] ; then
    echo -n "offline flashing ISSW..."
    for i1 in `pathconv ${tempdir}/${FLASH_1STIMAGE} `*; do
        extract_issw ${i1} > ${tempdir}/${BOOT_SOURCE}.ISSW.flashing.online.signed
    done
fi

echo "done"

###############################################################################
#
# USB boot 2nd image 
#
###############################################################################
#
echo -n "Create USB boot 2nd image..."
#

${FLASH_CREATOR} --align 512 \
                --sub ${FLASHING_2NDIMAGE_INI} \
                 > ${tempdir}/${FLASH_2NDIMAGE}

echo "done"

if [ "${TARGET_PLATFORM}" != 'eos2' ]; then
###############################################################################
#
# USB boot 3rd image 
#
###############################################################################
#
echo -n "Create USB boot 3rd image..."
#

${FLASH_CREATOR} --align 512 \
                --sub ${FLASHING_3RDIMAGE_INI} \
                 > ${tempdir}/${FLASH_3RDIMAGE}

echo "done"

###############################################################################
#
# Create final images 
#
###############################################################################
#
echo -n "Flashing 1st image..."
#

${FLASH_CREATOR} --align 512 \
                --sub ${FINAL_1STIMAGE_INI} \
                 > ${resultdir}/${FINAL_1STIMAGE}

echo "done"

cp  ${tempdir}/${FLASH_2NDIMAGE} ${resultdir}/${FINAL_2NDIMAGE}
cp  ${tempdir}/${FLASH_3RDIMAGE} ${resultdir}/${FINAL_3RDIMAGE}

else
    cp  ${tempdir}/${EMMC_1STIMAGE} ${resultdir}/${EMMC_1STIMAGE}
    cp  ${tempdir}/${EMMC_2NDIMAGE} ${resultdir}/${EMMC_2NDIMAGE}
    cp  ${tempdir}/${FLASH_1STIMAGE} ${resultdir}/${FLASH_1STIMAGE}
    cp  ${tempdir}/${FLASH_2NDIMAGE} ${resultdir}/${FLASH_2NDIMAGE}
fi
else
    cp  ${tempdir}/${EMMC_1STIMAGE} ${resultdir}/${FINAL_1STIMAGE}
    cp  ${tempdir}/${EMMC_2NDIMAGE} ${resultdir}/${FINAL_2NDIMAGE}
    if [ "${TARGET_PLATFORM}" != 'eos2' ]; then
        cp  ${tempdir}/${EMMC_3RDIMAGE} ${resultdir}/${FINAL_3RDIMAGE}
    fi
fi

# if [ "${TARGET_PLATFORM}" == 'eos2' ]; then
# ###############################################################################
# #
# # Create security file system 
# #
# ###############################################################################
# #
# echo "Create security file system:"
# #

# echo "   securityro.img..."
# ${MAKE_EXT4FS} -l 6M ${resultdir}/securityro.img ${tempdir}/securityro

# echo "  securityrw.img..."
# ${MAKE_EXT4FS} -l 6M ${resultdir}/securityrw.img ${tempdir}/securityrw
# fi

###############################################################################
#
# Clean up
#
###############################################################################


[ "${debug:-}" == "" ] && rm -Rf ${DEBUGDIR}

exit 0

#
# EOF
#
