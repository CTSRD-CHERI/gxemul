#!/bin/sh
#
#  $Id: test_openbsd_landisk_install.sh,v 1.1 2007/05/26 09:04:03 debug Exp $
#
#  Litet enkelt test för att mäta hur lång tid det tar att installera
#  OpenBSD/landisk 4.1, utan interaktion.
#
#  Starta med:
#
#	experiments/test_openbsd_landisk_install.sh
#

rm -f obsd_landisk.img
dd if=/dev/zero of=obsd_landisk.img bs=1024 count=1 seek=1900000
sync
sleep 2

time experiments/test_openbsd_landisk_install.expect 2> /tmp/gxemul_result

echo
echo
echo
echo
cat /tmp/gxemul_result
