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
 *  SPARC CPU emulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "settings.h"
#include "symbol.h"


#define	DYNTRANS_DUALMODE_32
#define	DYNTRANS_DELAYSLOT
#include "tmp_sparc_head.cc"


static char *sparc_regnames[N_SPARC_REG] = SPARC_REG_NAMES;
static char *sparc_pregnames[N_SPARC_PREG] = SPARC_PREG_NAMES;
static char *sparc_regbranch_names[N_SPARC_REGBRANCH_TYPES] =
	SPARC_REGBRANCH_NAMES;
static char *sparc_branch_names[N_SPARC_BRANCH_TYPES] = SPARC_BRANCH_NAMES;
static char *sparc_alu_names[N_ALU_INSTR_TYPES] = SPARC_ALU_NAMES;
static char *sparc_loadstore_names[N_LOADSTORE_TYPES] = SPARC_LOADSTORE_NAMES;


/*
 *  sparc_cpu_new():
 *
 *  Create a new SPARC cpu object.
 *
 *  Returns 1 on success, 0 if there was no matching SPARC processor with
 *  this cpu_type_name.
 */
int sparc_cpu_new(struct cpu *cpu, struct memory *mem, struct machine *machine,
	int cpu_id, char *cpu_type_name)
{
	int any_cache = 0;
	int i = 0;
	struct sparc_cpu_type_def cpu_type_defs[] = SPARC_CPU_TYPE_DEFS;

	/*  Scan the cpu_type_defs list for this cpu type:  */
	while (cpu_type_defs[i].name != NULL) {
		if (strcasecmp(cpu_type_defs[i].name, cpu_type_name) == 0) {
			break;
		}
		i++;
	}
	if (cpu_type_defs[i].name == NULL)
		return 0;

	cpu->memory_rw = sparc_memory_rw;

	cpu->cd.sparc.cpu_type = cpu_type_defs[i];
	cpu->name              = cpu->cd.sparc.cpu_type.name;
	cpu->byte_order        = EMUL_BIG_ENDIAN;
	cpu->is_32bit = (cpu->cd.sparc.cpu_type.bits == 32)? 1 : 0;

	cpu->instruction_has_delayslot = sparc_cpu_instruction_has_delayslot;

	/*  TODO: Separate this into 64-bit vs 32-bit?  */
	cpu->translate_v2p = sparc_translate_v2p;

	if (cpu->is_32bit) {
		cpu->run_instr = sparc32_run_instr;
		cpu->update_translation_table =
		    sparc32_update_translation_table;
		cpu->invalidate_translation_caches =
		    sparc32_invalidate_translation_caches;
		cpu->invalidate_code_translation =
		    sparc32_invalidate_code_translation;
	} else {
		cpu->run_instr = sparc_run_instr;
		cpu->update_translation_table = sparc_update_translation_table;
		cpu->invalidate_translation_caches =
		    sparc_invalidate_translation_caches;
		cpu->invalidate_code_translation =
		    sparc_invalidate_code_translation;
	}

	/*  Only show name and caches etc for CPU nr 0 (in SMP machines):  */
	if (cpu_id == 0) {
		debug("%s", cpu->name);

		if (cpu->cd.sparc.cpu_type.icache_shift != 0)
			any_cache = 1;
		if (cpu->cd.sparc.cpu_type.dcache_shift != 0)
			any_cache = 1;
		if (cpu->cd.sparc.cpu_type.l2cache_shift != 0)
			any_cache = 1;

		if (any_cache) {
			debug(" (I+D = %i+%i KB", (int)
			    (1 << (cpu->cd.sparc.cpu_type.icache_shift-10)),
			    (int)(1<<(cpu->cd.sparc.cpu_type.dcache_shift-10)));
			if (cpu->cd.sparc.cpu_type.l2cache_shift != 0) {
				debug(", L2 = %i KB",
				    (int)(1 << (cpu->cd.sparc.cpu_type.
				    l2cache_shift-10)));
			}
			debug(")");
		}
	}

	/*  After a reset, the Tick register is not readable by user code:  */
	cpu->cd.sparc.tick |= SPARC_TICK_NPT;

	/*  Insert number of Windows and Trap levels into the version reg.:  */
	cpu->cd.sparc.ver |= N_REG_WINDOWS | (MAXTL << SPARC_VER_MAXTL_SHIFT);

	/*  Misc. initial settings suitable for userland emulation:  */
	cpu->cd.sparc.cansave = cpu->cd.sparc.cpu_type.nwindows - 2;
	cpu->cd.sparc.canrestore = 0;
	cpu->cd.sparc.cleanwin = 1;
	cpu->cd.sparc.otherwin = 0;

	if (cpu->cd.sparc.cansave + cpu->cd.sparc.canrestore
	    + cpu->cd.sparc.otherwin != cpu->cd.sparc.cpu_type.nwindows - 2) {
		fatal("Fatal internal error: inconsistent windowing "
		    "parameters!\n");
		exit(1);
	}

	if (cpu->cd.sparc.cpu_type.nwindows > N_REG_WINDOWS) {
		fatal("Fatal internal error: nwindows = %1 is more than %i\n",
		    cpu->cd.sparc.cpu_type.nwindows, N_REG_WINDOWS);
		exit(1);
	}

	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);
	CPU_SETTINGS_ADD_REGISTER64("y", cpu->cd.sparc.y);
	CPU_SETTINGS_ADD_REGISTER64("pstate", cpu->cd.sparc.pstate);
	for (i=0; i<N_SPARC_REG; i++)
		CPU_SETTINGS_ADD_REGISTER64(sparc_regnames[i],
		    cpu->cd.sparc.r[i]);
	/*  TODO: Handler for writes to the zero register!  */

	return 1;
}


/*
 *  sparc_cpu_list_available_types():
 *
 *  Print a list of available SPARC CPU types.
 */
void sparc_cpu_list_available_types(void)
{
	int i, j;
	struct sparc_cpu_type_def tdefs[] = SPARC_CPU_TYPE_DEFS;

	i = 0;
	while (tdefs[i].name != NULL) {
		debug("%s", tdefs[i].name);
		for (j=16 - strlen(tdefs[i].name); j>0; j--)
			debug(" ");
		i++;
		if ((i % 4) == 0 || tdefs[i].name == NULL)
			debug("\n");
	}
}


/*
 *  sparc_cpu_dumpinfo():
 */
void sparc_cpu_dumpinfo(struct cpu *cpu)
{
	debug(", %i-bit\n", cpu->cd.sparc.cpu_type.bits);
}


/*
 *  sparc_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *
 *  gprs: set to non-zero to dump GPRs and some special-purpose registers.
 *  coprocs: set bit 0..3 to dump registers in coproc 0..3.
 */
void sparc_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	char *symbol;
	uint64_t offset;
	int i, x = cpu->cpu_id;
	int bits32 = cpu->is_32bit;

	if (gprs) {
		/*  Special registers (pc, ...) first:  */
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);

		debug("cpu%i: pc = 0x", x);
		if (bits32)
			debug("%08"PRIx32, (uint32_t) cpu->pc);
		else
			debug("%016"PRIx64, (uint64_t) cpu->pc);
		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		debug("cpu%i: y  = 0x%08"PRIx32"   ",
		    x, (uint32_t)cpu->cd.sparc.y);
		debug("icc = ");
		debug(cpu->cd.sparc.ccr & SPARC_CCR_N? "N" : "n");
		debug(cpu->cd.sparc.ccr & SPARC_CCR_Z? "Z" : "z");
		debug(cpu->cd.sparc.ccr & SPARC_CCR_V? "V" : "v");
		debug(cpu->cd.sparc.ccr & SPARC_CCR_C? "C" : "c");
		if (!bits32) {
			debug("  xcc = ");
			debug((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT)
			    & SPARC_CCR_N? "N" : "n");
			debug((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT)
			    & SPARC_CCR_Z? "Z" : "z");
			debug((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT)
			    & SPARC_CCR_V? "V" : "v");
			debug((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT)
			    & SPARC_CCR_C? "C" : "c");
		}
		debug("\n");

		if (bits32)
			debug("cpu%i: psr = 0x%08"PRIx32"\n",
			    x, (uint32_t) cpu->cd.sparc.psr);
		else
			debug("cpu%i: pstate = 0x%016"PRIx64"\n",
			    x, (uint64_t) cpu->cd.sparc.pstate);

		if (bits32) {
			for (i=0; i<N_SPARC_REG; i++) {
				if ((i & 3) == 0)
					debug("cpu%i: ", x);
				/*  Skip the zero register:  */
				if (i == SPARC_ZEROREG) {
					debug("               ");
					continue;
				}
				debug("%s=", sparc_regnames[i]);
				debug("0x%08"PRIx32,
				    (uint32_t) cpu->cd.sparc.r[i]);
				if ((i & 3) < 3)
					debug("  ");
				else
					debug("\n");
			}
		} else {
			for (i=0; i<N_SPARC_REG; i++) {
				int r = ((i >> 1) & 15) | ((i&1) << 4);
				if ((i & 1) == 0)
					debug("cpu%i: ", x);

				/*  Skip the zero register:  */
				if (i == SPARC_ZEROREG) {
					debug("                         ");
					continue;
				}

				debug("%s = ", sparc_regnames[r]);
				debug("0x%016"PRIx64, (uint64_t)
				    cpu->cd.sparc.r[r]);

				if ((i & 1) < 1)
					debug("  ");
				else
					debug("\n");
			}
		}
	}

	if (coprocs & 1) {
		int sum;

		debug("cpu%i: cwp        = 0x%02x\n", x, cpu->cd.sparc.cwp);
		debug("cpu%i: cansave    = 0x%02x\n", x, cpu->cd.sparc.cansave);
		debug("cpu%i: canrestore = 0x%02x\n", x,
		    cpu->cd.sparc.canrestore);
		debug("cpu%i: otherwin   = 0x%02x\n", x,
		    cpu->cd.sparc.otherwin);
		debug("cpu%i: cleanwin   = 0x%02x\n", x,
		    cpu->cd.sparc.cleanwin);

		sum = cpu->cd.sparc.cansave + cpu->cd.sparc.canrestore +
		    cpu->cd.sparc.otherwin;
		debug("cpu%i: cansave + canrestore + otherwin = %i + %i + %i"
		    " = %i", x, cpu->cd.sparc.cansave, cpu->cd.sparc.canrestore,
		    cpu->cd.sparc.otherwin, sum);
		if (sum == cpu->cd.sparc.cpu_type.nwindows - 2)
			debug("  (consistent)\n");
		else
			debug("  (INCONSISTENT!)\n");

		debug("cpu%i: wstate: other = %i, normal = %i\n",
		    x, (cpu->cd.sparc.wstate & SPARC_WSTATE_OTHER_MASK)
		    >> SPARC_WSTATE_OTHER_SHIFT, cpu->cd.sparc.wstate &
		    SPARC_WSTATE_NORMAL_MASK);

		debug("cpu%i: asi = 0x%02x\n", x, cpu->cd.sparc.asi);
		debug("cpu%i: tl  = 0x%02x\n", x, cpu->cd.sparc.tl);
		debug("cpu%i: pil = 0x%02x\n", x, cpu->cd.sparc.pil);

		for (i=0; i<MAXTL; i++) {
			debug("cpu%i: tpc[%i]    = 0x", x, i);
			if (bits32)
				debug("%08"PRIx32"\n",
				    (uint32_t) cpu->cd.sparc.tpc[i]);
			else
				debug("%016"PRIx64"\n",
				    (uint64_t) cpu->cd.sparc.tpc[i]);

			debug("cpu%i: tnpc[%i]   = 0x", x, i);
			if (bits32)
				debug("%08"PRIx32"\n",
				    (uint32_t) cpu->cd.sparc.tnpc[i]);
			else
				debug("%016"PRIx64"\n",
				    (uint64_t) cpu->cd.sparc.tnpc[i]);

			debug("cpu%i: tstate[%i] = 0x", x, i);
			if (bits32)
				debug("%08"PRIx32"\n",
				    (uint32_t) cpu->cd.sparc.tstate[i]);
			else
				debug("%016"PRIx64"\n",
				    (uint64_t) cpu->cd.sparc.tstate[i]);

			debug("cpu%i: ttype[%i]  = 0x"PRIx32"\n",
			    x, i, cpu->cd.sparc.ttype[i]);
		}

		debug("cpu%i: tba = 0x", x);
		if (bits32)
			debug("%08"PRIx32"\n", (uint32_t) cpu->cd.sparc.tba);
		else
			debug("%016"PRIx64"\n", (uint64_t) cpu->cd.sparc.tba);
	}
}


/*
 *  sparc_cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void sparc_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
}


/*
 *  sparc_cpu_instruction_has_delayslot():
 *
 *  Return 1 if an opcode is a branch, 0 otherwise.
 */
int sparc_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib)
{
	uint32_t iword = *((uint32_t *)&ib[0]);
	int hi2, op2;

	iword = BE32_TO_HOST(iword);

	hi2 = iword >> 30;
	op2 = (hi2 == 0)? ((iword >> 22) & 7) : ((iword >> 19) & 0x3f);

	switch (hi2) {
	case 0:	/*  conditional branch  */
		switch (op2) {
		case 1:
		case 2:
		case 3:	return 1;
		}
		break;
	case 1:	/*  call  */
		return 1;
	case 2:	/*  misc alu instructions  */
		switch (op2) {
		case 56:/*  jump and link  */
			return 1;
		case 57:/*  return  */
			return 1;
		}
		break;
	}

	return 0;
}


/*
 *  sparc_cpu_disassemble_instr():
 *
 *  Convert an instruction word into human readable format, for instruction
 *  tracing.
 *
 *  If running is 1, cpu->pc should be the address of the instruction.
 *
 *  If running is 0, things that depend on the runtime environment (eg.
 *  register contents) will not be shown, and addr will be used instead of
 *  cpu->pc for relative addresses.
 */
int sparc_cpu_disassemble_instr(struct cpu *cpu, unsigned char *instr,
	int running, uint64_t dumpaddr)
{
	uint64_t offset, tmp;
	uint32_t iword;
	int hi2, op2, rd, rs1, rs2, siconst, btype, tmps, no_rd = 0;
	int asi, no_rs1 = 0, no_rs2 = 0, jmpl = 0, shift_x = 0, cc, p;
	char *symbol, *mnem, *rd_name, *rs_name;

	if (running)
		dumpaddr = cpu->pc;

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset==0)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i: ", cpu->cpu_id);

	if (cpu->is_32bit)
		debug("%08"PRIx32, (uint32_t) dumpaddr);
	else
		debug("%016"PRIx64, (uint64_t) dumpaddr);

	iword = *(uint32_t *)&instr[0];
	iword = BE32_TO_HOST(iword);

	debug(": %08"PRIx32, iword);

	if (running && cpu->delay_slot)
		debug(" (d)");

	debug("\t");


	/*
	 *  Decode the instruction:
	 *
	 *  http://www.cs.unm.edu/~maccabe/classes/341/labman/node9.html is a
	 *  good quick description of SPARC instruction encoding.
	 */

	hi2 = iword >> 30;
	rd = (iword >> 25) & 31;
	btype = rd & (N_SPARC_BRANCH_TYPES - 1);
	rs1 = (iword >> 14) & 31;
	asi = (iword >> 5) & 0xff;
	rs2 = iword & 31;
	siconst = (int16_t)((iword & 0x1fff) << 3) >> 3;
	op2 = (hi2 == 0)? ((iword >> 22) & 7) : ((iword >> 19) & 0x3f);
	cc = (iword >> 20) & 3;
	p = (iword >> 19) & 1;

	switch (hi2) {

	case 0:	switch (op2) {

		case 0:	debug("illtrap\t0x%x", iword & 0x3fffff);
			break;

		case 1:
		case 2:
		case 3:	if (op2 == 3)
				debug("%s", sparc_regbranch_names[btype & 7]);
			else
				debug("%s", sparc_branch_names[btype]);
			if (rd & 16)
				debug(",a");
			tmps = iword;
			switch (op2) {
			case 1:	tmps <<= 13;
				tmps >>= 11;
				if (!p)
					debug(",pn");
				debug("\t%%%s,", cc==0 ? "icc" :
				    (cc==2 ? "xcc" : "UNKNOWN"));
				break;
			case 2:	tmps <<= 10;
				tmps >>= 8;
				debug("\t");
				break;
			case 3:	if (btype & 8)
					debug("(INVALID)");
				if (!p)
					debug(",pn");
				debug("\t%%%s,", sparc_regnames[rs1]);
				tmps = ((iword & 0x300000) >> 6)
				    | (iword & 0x3fff);
				tmps <<= 16;
				tmps >>= 14;
				break;
			}
			tmp = (int64_t)(int32_t)tmps;
			tmp += dumpaddr;
			debug("0x%"PRIx64, (uint64_t) tmp);
			symbol = get_symbol_name(&cpu->machine->
			    symbol_context, tmp, &offset);
			if (symbol != NULL)
				debug(" \t<%s>", symbol);
			break;

		case 4:	if (rd == 0) {
				debug("nop");
				break;
			}
			debug("sethi\t%%hi(0x%x),", (iword & 0x3fffff) << 10);
			debug("%%%s", sparc_regnames[rd]);
			break;

		default:debug("UNIMPLEMENTED hi2=%i, op2=0x%x", hi2, op2);
		}
		break;

	case 1:	tmp = (int32_t)iword << 2;
		tmp += dumpaddr;
		debug("call\t0x%"PRIx64, (uint64_t) tmp);
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    tmp, &offset);
		if (symbol != NULL)
			debug(" \t<%s>", symbol);
		break;

	case 2:	mnem = sparc_alu_names[op2];
		rs_name = sparc_regnames[rs1];
		rd_name = sparc_regnames[rd];
		switch (op2) {
		case 0:	/*  add  */
			if (rd == rs1 && (iword & 0x3fff) == 0x2001) {
				mnem = "inc";
				no_rs1 = no_rs2 = 1;
			}
			break;
		case 2:	/*  or  */
			if (rs1 == 0) {
				mnem = "mov";
				no_rs1 = 1;
			}
			break;
		case 4:	/*  sub  */
			if (rd == rs1 && (iword & 0x3fff) == 0x2001) {
				mnem = "dec";
				no_rs1 = no_rs2 = 1;
			}
			break;
		case 20:/*  subcc  */
			if (rd == 0) {
				mnem = "cmp";
				no_rd = 1;
			}
			break;
		case 37:/*  sll  */
		case 38:/*  srl  */
		case 39:/*  sra  */
			if (siconst & 0x1000) {
				siconst &= 0x3f;
				shift_x = 1;
			} else
				siconst &= 0x1f;
			break;
		case 40:/*  rd on pre-sparcv9, membar etc on sparcv9  */
			no_rs2 = 1;
			rs_name = "UNIMPLEMENTED";
			switch (rs1) {
			case 0:	rs_name = "y"; break;
			case 2:	rs_name = "ccr"; break;
			case 3:	rs_name = "asi"; break;
			case 4:	rs_name = "tick"; break;
			case 5:	rs_name = "pc"; break;
			case 6:	rs_name = "fprs"; break;
			case 15:/*  membar etc.  */
				if ((iword >> 13) & 1) {
					no_rd = 1;
					mnem = "membar";
					rs_name = "#TODO";
				}
				break;
			case 23:rs_name = "tick_cmpr"; break;	/*  v9 ?  */
			}
			break;
		case 41:rs_name = "psr";
			no_rs2 = 1;
			break;
		case 42:/*  TODO: something with wim only, on sparc v8?  */
			rs_name = sparc_pregnames[rs1];
			no_rs2 = 1;
			break;
		case 43:/*  ?  */
			/*  TODO: pre-sparcv9: rd, rs_name = "tbr";  */
			if (iword == 0x81580000) {
				mnem = "flushw";
				no_rs1 = no_rs2 = no_rd = 1;
			}
			break;
		case 48:/*  wr* (SPARCv8)  */
			mnem = "wr";
			if (rs1 == SPARC_ZEROREG)
				no_rs1 = 1;
			switch (rd) {
			case 0:	rd_name = "y"; break;
			case 2:	rd_name = "ccr"; break;
			case 3:	rd_name = "asi"; break;
			case 6:	rd_name = "fprs"; break;
			case 23:rd_name = "tick_cmpr"; break;	/*  v9 ?  */
			default:rd_name = "UNIMPLEMENTED";
			}
			break;
		case 49:/*  ?  */
			if (iword == 0x83880000) {
				mnem = "restored";
				no_rs1 = no_rs2 = no_rd = 1;
			}
			break;
		case 50:/*  wrpr  */
			rd_name = sparc_pregnames[rd];
			if (rs1 == SPARC_ZEROREG)
				no_rs1 = 1;
			break;
		case 56:/*  jmpl  */
			jmpl = 1;
			if (iword == 0x81c7e008) {
				mnem = "ret";
				no_rs1 = no_rs2 = no_rd = 1;
			}
			if (iword == 0x81c3e008) {
				mnem = "retl";
				no_rs1 = no_rs2 = no_rd = 1;
			}
			break;
		case 61:/*  restore  */
			if (iword == 0x81e80000)
				no_rs1 = no_rs2 = no_rd = 1;
			break;
		case 62:if (iword == 0x83f00000) {
				mnem = "retry";
				no_rs1 = no_rs2 = no_rd = 1;
			}
			break;
		}
		debug("%s", mnem);
		if (shift_x)
			debug("x");
		debug("\t");
		if (!no_rs1)
			debug("%%%s", rs_name);
		if (!no_rs1 && !no_rs2) {
			if (jmpl)
				debug("+");
			else
				debug(",");
		}
		if (!no_rs2) {
			if ((iword >> 13) & 1) {
				if (siconst >= -9 && siconst <= 9)
					debug("%i", siconst);
				else if (siconst < 0 && (op2 == 0 ||
				    op2 == 4 || op2 == 20 || op2 == 60))
					debug("-0x%x", -siconst);
				else
					debug("0x%x", siconst);
			} else {
				debug("%%%s", sparc_regnames[rs2]);
			}
		}
		if ((!no_rs1 || !no_rs2) && !no_rd)
			debug(",");
		if (!no_rd)
			debug("%%%s", rd_name);
		break;

	case 3:	mnem = sparc_loadstore_names[op2];
		switch (op2) {
		case 0:	/*  'lduw' was called only 'ld' in pre-v9  */
			if (cpu->cd.sparc.cpu_type.v < 9)
				mnem = "ld";
			break;
		}
		debug("%s\t", mnem);
		if (op2 & 4)
			debug("%%%s,", sparc_regnames[rd]);
		debug("[%%%s", sparc_regnames[rs1]);
		if ((iword >> 13) & 1) {
			if (siconst > 0)
				debug("+");
			if (siconst != 0)
				debug("%i", siconst);
		} else {
			if (rs2 != 0)
				debug("+%%%s", sparc_regnames[rs2]);
		}
		debug("]");
		if ((op2 & 0x30) == 0x10)
			debug("(%i)", asi);
		if (!(op2 & 4))
			debug(",%%%s", sparc_regnames[rd]);
		break;
	}

	debug("\n");
	return sizeof(iword);
}


/*
 *  sparc_update_pstate():
 *
 *  Update the pstate register (64-bit sparcs).
 */
static void sparc_update_pstate(struct cpu *cpu, uint64_t new_pstate)
{
	/*  uint64_t old_pstate = cpu->cd.sparc.pstate;  */

	/*  TODO: Check individual bits.  */

	cpu->cd.sparc.pstate = new_pstate;
}


#include "tmp_sparc_tail.cc"

