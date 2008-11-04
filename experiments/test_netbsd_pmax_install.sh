#!/bin/sh
#
#  $Id: test_netbsd_pmax_install.sh,v 1.2 2006-07-08 22:38:42 debug Exp $
#
#  Litet enkelt test för att mäta hur lång tid det tar att installera
#  en full NetBSD/pmax 3.0 för R3000, utan interaktion.
#  Starta med:
#
#	experiments/test_netbsd_pmax_install.sh
#

rm -f nbsd_pmax.img
dd if=/dev/zero of=nbsd_pmax.img bs=1024 count=1 seek=1900000
sync
sleep 2

time experiments/test_netbsd_pmax_install.expect 2> /tmp/gxemul_result

echo
echo
echo
cat /tmp/gxemul_result
