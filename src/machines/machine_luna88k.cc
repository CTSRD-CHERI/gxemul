/*
 *  Copyright (C) 2008-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: LUNA88K machine
 *
 *  This is for experiments with OpenBSD/luna88k. See
 *  openbsd/sys/arch/luna88k/luna88k/locore.S for more information about
 *  how OpenBSD starts up on this platform.
 *
 *  RAMDISK kernel used for experiments:
 *
 *	ftp://ftp.se.openbsd.org/pub/OpenBSD/4.1/luna88k/bsd.rd
 *
 *  Launch with   gxemul -e luna-88k2 -Ttv 0x20000:0x20:bsd.rd
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(luna88k)
{
	switch (machine->machine_subtype) {

	case MACHINE_LUNA_88K:
		machine->machine_name = strdup("LUNA 88K");
		break;

	case MACHINE_LUNA_88K2:
		machine->machine_name = strdup("LUNA 88K2");
		break;

	default:fatal("Unimplemented LUNA88K machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}

	if (!machine->prom_emulation)
		return;

	/*  TODO  */
}


MACHINE_DEFAULT_CPU(luna88k)
{
	machine->cpu_name = strdup("88100");
}


MACHINE_DEFAULT_RAM(luna88k)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(luna88k)
{
	MR_DEFAULT(luna88k, "LUNA88K", ARCH_M88K, MACHINE_LUNA88K);

	machine_entry_add_alias(me, "luna88k");

	machine_entry_add_subtype(me, "LUNA-88K", MACHINE_LUNA_88K,
	    "luna-88k", NULL);

	machine_entry_add_subtype(me, "LUNA-88K2", MACHINE_LUNA_88K2,
	    "luna-88k2", NULL);

	me->set_default_ram = machine_default_ram_luna88k;
}

