#!/bin/bash
#
# ############################################################################
# #                               Renesas                                    #
# ############################################################################
# #                                                                          #
# ############################# COPYRIGHT INFORMATION ########################
# # This program contains proprietary information that is a trade secret of  #
# # Renesas and also is protected as an unpublished work under               #
# # applicable Copyright laws. Recipient is to retain this program in        #
# # confidence and is not permitted to use or make copies thereof other than #
# # as permitted in a written agreement with Renesas.                        #
# #                                                                          #
# # All rights reserved. Company confidential.                               #
# ############################################################################
#
# This script asserts the sanity of this particular SW component.
set -ue

#CBRANCH=$(git rev-parse --abbrev-ref --symbolic $(git symbolic-ref HEAD))
#GIT_DIR=$(git rev-parse --git-dir 2>/dev/null)
if [ "$#" -gt 0 ]; then args=("$@"); else args=("test"); fi
make -f emu_makefile --directory=../build ${args[*]}

