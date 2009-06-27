/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *   
 *
 *  COMMENT: BeBox
 *
 *  Experimental machine for running NetBSD/bebox (see
 *  http://www.netbsd.org/ports/bebox/ for more info.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_isa.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(bebox)
{
	char tmpstr[300];

	machine->machine_name = strdup("BeBox");
	if (machine->emulated_hz == 0)
		machine->emulated_hz = 33000000;

	snprintf(tmpstr, sizeof(tmpstr), "bebox irq=%s.cpu[%i]",
	    machine->path, machine->bootstrap_cpu);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "eagle irq=%s.cpu[%i]",
	    machine->path, machine->bootstrap_cpu);
	device_add(machine, tmpstr);

	if (!machine->prom_emulation)
		return;

	/*  According to the docs, and also used by NetBSD:  */
	store_32bit_word(cpu, 0x3010, machine->physical_ram_in_mb * 1048576);

	/*  Used by Linux:  */
	store_32bit_word(cpu, 0x32f8, machine->physical_ram_in_mb * 1048576);

	/*  TODO: List of stuff, see http://www.beatjapan.org/
	    mirror/www.be.com/aboutbe/benewsletter/
	    Issue27.html#Cookbook  for the details.  */
	store_32bit_word(cpu, 0x301c, 0);

	/*  NetBSD/bebox: r3 = startkernel, r4 = endkernel,
	    r5 = args, r6 = ptr to bootinfo?  */
	cpu->cd.ppc.gpr[3] = 0x3100;
	cpu->cd.ppc.gpr[4] = 0x400000;
	cpu->cd.ppc.gpr[5] = 0x2000;
	store_string(cpu, cpu->cd.ppc.gpr[5], "-a");
	cpu->cd.ppc.gpr[6] = machine->physical_ram_in_mb * 1048576 - 0x100;

	/*  See NetBSD's bebox/include/bootinfo.h for details  */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 0, 12);  /*  next  */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 4, 0);  /*  mem  */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 8,
	    machine->physical_ram_in_mb * 1048576);

	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 12, 20);  /* next */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 16, 1); /* console */
	store_buf(cpu, cpu->cd.ppc.gpr[6] + 20,
	    machine->x11_md.in_use? "vga" : "com", 4);
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 24, 0x3f8);/* addr */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 28, 9600);/* speed */

	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 32, 0);  /*  next  */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 36, 2);  /*  clock */
	store_32bit_word(cpu, cpu->cd.ppc.gpr[6] + 40,
	    (machine->emulated_hz / 4));
}


MACHINE_DEFAULT_CPU(bebox)
{
	/*  For NetBSD/bebox. Dual 133 MHz 603e CPUs, for example.  */
	machine->cpu_name = strdup("PPC603e");
}


MACHINE_DEFAULT_RAM(bebox)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(bebox)
{
	MR_DEFAULT(bebox, "BeBox", ARCH_PPC, MACHINE_BEBOX);

	machine_entry_add_alias(me, "bebox");

	me->set_default_ram = machine_default_ram_bebox;
}

