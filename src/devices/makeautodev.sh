#!/bin/sh
###############################################################################
#
#  Copyright (C) 2005-2008  Anders Gavare.  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. The name of the author may not be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
#  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGE.


printf "Generating autodev.c... "

rm -f autodev.c

printf "/*\n *  DO NOT EDIT. AUTOMATICALLY CREATED\n */\n\n" >> autodev.c

cat autodev_head.c >> autodev.c

printf "5"
rm -f .index
for a in *.c; do
	B=`grep COMMENT $a`
	if [ z"$B" != z ]; then
		printf "$a " >> .index
		echo "$B"|cut -d : -f 2- >> .index
	fi
done

printf "4"
for a in dev_*.c; do
	B=`grep DEVINIT $a`
	if [ z"$B" != z ]; then
		C=`grep DEVINIT $a | cut -d \( -f 2|cut -d \) -f 1`
		for B in $C; do
			printf "int devinit_$B(struct devinit *);\n" >> autodev.c
		done
	fi
done

printf "3"
for a in bus_pci.c; do
	B=`grep PCIINIT $a`
	if [ z"$B" != z ]; then
		C=`grep PCIINIT $a | cut -d \( -f 2|cut -d \) -f 1`
		for B in $C; do
			printf "void pciinit_$B(struct machine *, " >> autodev.c
			printf "struct memory *, struct pci_device *);\n" >> autodev.c
		done
	fi
done

cat autodev_middle.c >> autodev.c

printf "2"
for a in dev_*.c; do
	B=`grep DEVINIT $a`
	if [ z"$B" != z ]; then
		C=`grep DEVINIT $a | cut -d \( -f 2|cut -d \) -f 1`
		for B in $C; do
			printf "\tdevice_register(\""$B"\"," >> autodev.c
			printf " devinit_$B);\n" >> autodev.c
		done
	fi
done

printf "1"
for a in bus_pci.c; do
	B=`grep PCIINIT $a`
	if [ z"$B" != z ]; then
		C=`grep PCIINIT $a | cut -d \( -f 2|cut -d \) -f 1`
		for B in $C; do
			printf "\tpci_register(\""$B"\"," >> autodev.c
			printf " pciinit_$B);\n" >> autodev.c
		done
	fi
done

cat autodev_tail.c >> autodev.c

printf " done\n"
