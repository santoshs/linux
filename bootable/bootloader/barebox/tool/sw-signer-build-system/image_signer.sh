#!/bin/bash
#
# ############################################################################
# #    Copyright © Renesas Mobile Corporation 2010 . All rights reserved     #
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
# General script for SW signing. See README.txt for further information.
# 

set -ue

#
# Set environment variables according to command line arguments
#

USAGE="Usage: $0 [-product <name>] [-server <address:port,address:port,...>] [-productpath] 
[-op <variant>] [-imeisvn <value> ] [-product_id <id> ] [-retry <value>] [-offline] [-ut] [-nosign] "


while [ $# -gt 0 ]
do
    case "$1" in
        -product )
            export PRODUCT=$2
            shift
            ;;
        -productpath )
            export SIGN_PRODUCT_PATH=$2
            shift
            ;;
        -op )
            export OPERATOR_VARIANT=$2
            shift
            ;;
        -imeisvn )
            export IMEI_SVN=$2
            shift
            ;;
        -product_id )
            export PRODUCT_ID=$2
            shift
            ;;
        -server )
            export SIGN_SERVER=$2
            shift
            ;;
        -retry )
            export SIGN_SERVER_RETRY=$2
            shift
            ;;
        -offline )
            export OFFLINE_MODE=TRUE
            ;;
        -ut )
            export UNIT_TEST=TRUE
            ;;
        -nosign )
            export SIGNING_SUPPORT=FALSE
            ;;    
        -help | -* ) echo $USAGE >&2
            exit 1
            ;;
        *)  break	# terminate while loop
            ;;
    esac
    shift
done


#
# Set timestamp at top level script to make sure all runs get the same value.
#
export TIMESTAMP=`date -u +%Y%m%d%H%MZ`

./bin/core.sh

echo
echo "Completed"
