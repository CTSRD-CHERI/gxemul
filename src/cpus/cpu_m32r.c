/*
 *  Copyright (C) 2007-2008  Anders Gavare.  All rights reserved.
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
 *  $Id: cpu_m32r.c,v 1.1.2.1 2008-01-18 19:12:25 debug Exp $
 *
 *  Mitsubishi/Renesas M32R CPU emulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cpu.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "settings.h"
#include "symbol.h"


#define DYNTRANS_32
#include "tmp_m32r_head.c"


void m32r_pc_to_pointers(struct cpu *);

void m32r_irq_interrupt_assert(struct interrupt *interrupt);
void m32r_irq_interrupt_deassert(struct interrupt *interrupt);


/*
 *  m32r_cpu_new():
 *
 *  Create a new M32R cpu object by filling the CPU struct.
 *  Return 1 on success, 0 if cpu_type_name isn't a valid M32R processor.
 */
int m32r_cpu_new(struct cpu *cpu, struct memory *mem,
	struct machine *machine, int cpu_id, char *cpu_type_name)
{
	int i, found;
	struct m32r_cpu_type_def cpu_type_defs[] = M32R_CPU_TYPE_DEFS;

	/*  Scan the list for this cpu type:  */
	i = 0; found = -1;
	while (i >= 0 && cpu_type_defs[i].name != NULL) {
		if (strcasecmp(cpu_type_defs[i].name, cpu_type_name) == 0) {
			found = i;
			break;
		}
		i++;
	}
	if (found == -1)
		return 0;

	cpu->run_instr = m32r_run_instr;
	cpu->memory_rw = m32r_memory_rw;
	cpu->update_translation_table = m32r_update_translation_table;
	cpu->invalidate_translation_caches =
	    m32r_invalidate_translation_caches;
	cpu->invalidate_code_translation = m32r_invalidate_code_translation;
	cpu->translate_v2p = m32r_translate_v2p;

	cpu->cd.m32r.cpu_type = cpu_type_defs[found];
	cpu->name            = cpu->cd.m32r.cpu_type.name;
	cpu->is_32bit        = 1;
	cpu->byte_order      = EMUL_BIG_ENDIAN;

	/*  Only show name and caches etc for CPU nr 0:  */
	if (cpu_id == 0) {
		debug("%s", cpu->name);
	}


	/*
	 *  Add register names as settings:
	 */

	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);

	/*  TODO  */

	/*  Register the CPU interrupt pin:  */
	{
		struct interrupt template;
		char name[50];
		snprintf(name, sizeof(name), "%s", cpu->path);

		memset(&template, 0, sizeof(template));
		template.line = 0;
		template.name = name;
		template.extra = cpu;
		template.interrupt_assert = m32r_irq_interrupt_assert;
		template.interrupt_deassert = m32r_irq_interrupt_deassert;
		interrupt_handler_register(&template);
	}

	return 1;
}


/*
 *  m32r_cpu_dumpinfo():
 */
void m32r_cpu_dumpinfo(struct cpu *cpu)
{
	/*  struct m32r_cpu_type_def *ct = &cpu->cd.m32r.cpu_type;  */

	debug(", %s-endian",
	    cpu->byte_order == EMUL_BIG_ENDIAN? "Big" : "Little");

	debug("\n");
}


/*
 *  m32r_cpu_list_available_types():
 *
 *  Print a list of available M32R CPU types.
 */
void m32r_cpu_list_available_types(void)
{
	int i, j;
	struct m32r_cpu_type_def tdefs[] = M32R_CPU_TYPE_DEFS;

	i = 0;
	while (tdefs[i].name != NULL) {
		debug("%s", tdefs[i].name);
		for (j=13 - strlen(tdefs[i].name); j>0; j--)
			debug(" ");
		i++;
		if ((i % 5) == 0 || tdefs[i].name == NULL)
			debug("\n");
	}
}


/*
 *  m32r_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *  
 *  gprs: set to non-zero to dump GPRs and some special-purpose registers.
 *  coprocs: set bit 0..3 to dump registers in coproc 0..3.
 */
void m32r_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	char *symbol;
	uint64_t offset;
	int i, x = cpu->cpu_id;

	if (gprs) {
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);
		debug("cpu%i:  pc  = 0x%08"PRIx32, x, (uint32_t)cpu->pc);
		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		for (i=0; i<N_M32R_GPRS; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			if (i == 0)
				debug("                  ");
			else
				debug("  r%-2i = 0x%08"PRIx32,
				    i, cpu->cd.m32r.r[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}
}


/*
 *  m32r_cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void m32r_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
}


/*
 *  m32r_irq_interrupt_assert():
 *  m32r_irq_interrupt_deassert():
 */
void m32r_irq_interrupt_assert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.m32r.irq_asserted = 1;
}
void m32r_irq_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.m32r.irq_asserted = 0;
}


/*
 *  m32r_cpu_disassemble_instr():
 *
 *  Convert an instruction word into human readable format, for instruction
 *  tracing.
 *              
 *  If running is 1, cpu->pc should be the address of the instruction.
 *
 *  If running is 0, things that depend on the runtime environment (eg.
 *  register contents) will not be shown, and dumpaddr will be used instead of
 *  cpu->pc for relative addresses.
 */                     
int m32r_cpu_disassemble_instr(struct cpu *cpu, unsigned char *ib,
        int running, uint64_t dumpaddr)
{
	uint32_t iw;
	char *symbol;
	uint64_t offset;

	if (running)
		dumpaddr = cpu->pc;

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset == 0)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i:\t", cpu->cpu_id);

	debug("%08"PRIx32": ", (uint32_t) dumpaddr);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iw = ib[0] + (ib[1]<<8) + (ib[2]<<16) + (ib[3]<<24);
	else
		iw = ib[3] + (ib[2]<<8) + (ib[1]<<16) + (ib[0]<<24);

	debug("%08"PRIx32"\t", (uint32_t) iw);

	switch (iw) {

	default:
		debug("UNIMPLEMENTED iw=0x%08x\n", iw);
	}

	return sizeof(uint32_t);
}


#include "tmp_m32r_tail.c"


