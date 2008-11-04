/*
 *  Copyright (C) 2006-2008  Anders Gavare.  All rights reserved.
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
 *  $Id: machine_sparc.c,v 1.6.2.1 2008-01-18 19:12:33 debug Exp $
 *
 *  COMMENT: SUN SPARC machines
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


MACHINE_SETUP(sparc)
{
	switch (machine->machine_subtype) {

	case MACHINE_SPARC_SS5:
		machine->machine_name = "SUN SPARCstation 5";

		break;

	case MACHINE_SPARC_SS20:
		machine->machine_name = "SUN SPARCstation 20";

		break;

	case MACHINE_SPARC_ULTRA1:
		machine->machine_name = "SUN Ultra1";

		break;

	case MACHINE_SPARC_ULTRA60:
		machine->machine_name = "SUN Ultra60";

		break;

	case MACHINE_SPARC_SUN4V:
		machine->machine_name = "SUN Generic sun4v";

		break;

	default:fatal("Unimplemented SPARC machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}

	if (!machine->prom_emulation)
		return;

}


MACHINE_DEFAULT_CPU(sparc)
{
	switch (machine->machine_subtype) {

	case MACHINE_SPARC_SS5:
		machine->cpu_name = strdup("MB86907");
		break;

	case MACHINE_SPARC_SS20:
		machine->cpu_name = strdup("TMS390Z50");
		break;

	case MACHINE_SPARC_ULTRA1:
		machine->cpu_name = strdup("UltraSPARC");
		break;

	case MACHINE_SPARC_ULTRA60:
		machine->cpu_name = strdup("UltraSPARC-II");
		break;

	case MACHINE_SPARC_SUN4V:
		machine->cpu_name = strdup("T1");
		break;

	default:fatal("Unimplemented SPARC machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}
}


MACHINE_DEFAULT_RAM(sparc)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(sparc)
{
	MR_DEFAULT(sparc, "SPARC", ARCH_SPARC, MACHINE_SPARC);

	machine_entry_add_alias(me, "sparc");

	machine_entry_add_subtype(me, "SUN SPARCstation 5", MACHINE_SPARC_SS5,
	    "ss5", "sparcstation5", NULL);

	machine_entry_add_subtype(me, "SUN SPARCstation 20", MACHINE_SPARC_SS20,
	    "ss20", "sparcstation20", NULL);

	machine_entry_add_subtype(me, "SUN Ultra1", MACHINE_SPARC_ULTRA1,
	    "ultra1", NULL);

	machine_entry_add_subtype(me, "SUN Ultra60", MACHINE_SPARC_ULTRA60,
	    "ultra60", NULL);

	machine_entry_add_subtype(me, "SUN Generic sun4v", MACHINE_SPARC_SUN4V,
	    "sun4v", NULL);

	me->set_default_ram = machine_default_ram_sparc;
}

