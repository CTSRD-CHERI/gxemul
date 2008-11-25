/*
 *  Copyright (C) 2005-2008  Anders Gavare.  All rights reserved.
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
 *  A Iyonix machine mode. Not yet working.
 */

#include <stdio.h>
#include <string.h>

#include "bus_isa.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(iyonix)
{
	machine->machine_name = "Iyonix";

	cpu->cd.arm.coproc[6] = arm_coproc_i80321_6;

	/*  0xa0000000 = physical ram, 0xc0000000 = uncached  */
	dev_ram_init(machine, 0xa0000000, 0x20000000, DEV_RAM_MIRROR, 0x0);
	dev_ram_init(machine, 0xc0000000, 0x20000000, DEV_RAM_MIRROR, 0x0);
	dev_ram_init(machine, 0xf0000000, 0x08000000, DEV_RAM_MIRROR, 0x0);

	device_add(machine, "ns16550 irq=0 addr=0xfe800000 in_use=0");

	bus_isa_init(machine, machine->path, 0, 0x90000000ULL, 0x98000000ULL);

	device_add(machine, "i80321 addr=0xffffe000");

	if (!machine->prom_emulation)
		return;

	arm_setup_initial_translation_table(cpu,
	    machine->physical_ram_in_mb * 1048576 - 65536);
	arm_translation_table_set_l1(cpu, 0xa0000000, 0xa0000000);
	arm_translation_table_set_l1(cpu, 0xc0000000, 0xa0000000);
	arm_translation_table_set_l1_b(cpu, 0xff000000, 0xff000000);
}


MACHINE_DEFAULT_CPU(iyonix)
{
	machine->cpu_name = strdup("80321_600_B0");
}


MACHINE_DEFAULT_RAM(iyonix)
{
	machine->physical_ram_in_mb = 32;
}


MACHINE_REGISTER(iyonix)
{
	MR_DEFAULT(iyonix, "Iyonix", ARCH_ARM, MACHINE_IYONIX);

	machine_entry_add_alias(me, "iyonix");

	me->set_default_ram = machine_default_ram_iyonix;
}

