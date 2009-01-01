#!/bin/sh
#
#  A small test to measure how long time it takes (real time) to install
#  OpenBSD/mvme88k 4.4, without interaction.
#
#  Start with:
#
#	experiments/test_openbsd_mvme88k_install.sh
#

rm -f obsd_mvme88k.img
dd if=/dev/zero of=obsd_mvme88k.img bs=1024 count=1 seek=1900000
sync
sleep 2

time experiments/test_openbsd_mvme88k_install.expect 2> /tmp/gxemul_result

echo
echo
echo
echo
cat /tmp/gxemul_result
