#
# Copyright (C) 2012 Renesas Electronics Corporation
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
#

#
# 256MiB at the bottom of the System RAM will be reserved:
#
#   64MiB@0x40000000 ... reserved for Modem-CPU workspace
#   64MiB@0x44000000 ... Reserved
#   64MiB@0x48000000 ... reserved for RT-CPU and production test
#   64MiB@0x4C000000 ... Reserved
#  768MiB@0x50000000 ... Linux kernel
#
# U-Boot is relocated to / works at TEXT_BASE, with 16MiB offset from
# the bottom of the Linux kernel workspace, that is 0x53000000.
TEXT_BASE = 0x53000000
