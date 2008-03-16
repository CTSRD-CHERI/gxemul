#!/bin/sh
#
#  $Id: test_netbsd_pmax_install.r4400.sh,v 1.1 2006/07/22 12:19:58 debug Exp $
#
#  Litet enkelt test för att mäta hur lång tid det tar att installera
#  en full NetBSD/pmax 3.0 för R4400, utan interaktion.
#  Starta med:
#
#	experiments/test_netbsd_pmax_install.r4400.sh
#

rm -f nbsd_pmax.img
dd if=/dev/zero of=nbsd_pmax.img bs=1024 count=1 seek=1900000
sync
sleep 2

time experiments/test_netbsd_pmax_install.r4400.expect 2> /tmp/gxemul_result

echo
echo
echo
cat /tmp/gxemul_result
