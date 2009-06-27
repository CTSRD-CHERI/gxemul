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
 *  SPARC instructions.
 *
 *  Individual functions should keep track of cpu->n_translated_instrs.
 *  (If no instruction was executed, then it should be decreased. If, say, 4
 *  instructions were combined into one function and executed, then it should
 *  be increased by 3.)
 */


/*
 *  invalid:  For catching bugs.
 */
X(invalid)
{
	fatal("FATAL ERROR: An internal error occured in the SPARC"
	    " dyntrans code. Please contact the author with detailed"
	    " repro steps on how to trigger this bug.\n");
	exit(1);
}


/*
 *  nop:  Do nothing.
 */
X(nop)
{
}


/*****************************************************************************/


/*
 *  call
 *
 *  arg[0] = int32_t displacement compared to the current instruction
 *  arg[1] = int32_t displacement of current instruction compared to
 *           start of the page
 */
X(call)
{
	MODE_uint_t old_pc = cpu->pc;
	old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	old_pc += (int32_t)ic->arg[1];
	cpu->cd.sparc.r[SPARC_REG_O7] = old_pc;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = old_pc + (int32_t)ic->arg[0];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(call_trace)
{
	MODE_uint_t old_pc = cpu->pc;
	old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	old_pc += (int32_t)ic->arg[1];
	cpu->cd.sparc.r[SPARC_REG_O7] = old_pc;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = old_pc + (int32_t)ic->arg[0];
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bl
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(bl)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = (cpu->cd.sparc.ccr & SPARC_CCR_N) ? 1 : 0;
	int v = (cpu->cd.sparc.ccr & SPARC_CCR_V) ? 1 : 0;
	int cond = n ^ v;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bl_xcc)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_N)? 1:0;
	int v = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_V)? 1:0;
	int cond = n ^ v;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  ble
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(ble)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = (cpu->cd.sparc.ccr & SPARC_CCR_N) ? 1 : 0;
	int v = (cpu->cd.sparc.ccr & SPARC_CCR_V) ? 1 : 0;
	int z = (cpu->cd.sparc.ccr & SPARC_CCR_Z) ? 1 : 0;
	int cond = (n ^ v) || z;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(ble_xcc)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_N)? 1:0;
	int v = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_V)? 1:0;
	int z = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_Z)? 1:0;
	int cond = (n ^ v) || z;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bne
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(bne)
{
	MODE_uint_t old_pc = cpu->pc;
	int cond = (cpu->cd.sparc.ccr & SPARC_CCR_Z) ? 0 : 1;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bne_a)
{
	MODE_uint_t old_pc = cpu->pc;
	int cond = (cpu->cd.sparc.ccr & SPARC_CCR_Z) ? 0 : 1;
	cpu->delay_slot = TO_BE_DELAYED;
	if (!cond) {
		/*  Nullify the delay slot:  */
		cpu->cd.sparc.next_ic ++;
		return;
	}
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
		    << SPARC_INSTR_ALIGNMENT_SHIFT);
		cpu->pc = old_pc + (int32_t)ic->arg[0];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bg
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(bg)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = (cpu->cd.sparc.ccr & SPARC_CCR_N) ? 1 : 0;
	int v = (cpu->cd.sparc.ccr & SPARC_CCR_V) ? 1 : 0;
	int z = (cpu->cd.sparc.ccr & SPARC_CCR_Z) ? 1 : 0;
	int cond = !(z | (n ^ v));
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bg_xcc)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_N)? 1:0;
	int v = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_V)? 1:0;
	int z = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_Z)? 1:0;
	int cond = !(z | (n ^ v));
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bge
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(bge)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = (cpu->cd.sparc.ccr & SPARC_CCR_N) ? 1 : 0;
	int v = (cpu->cd.sparc.ccr & SPARC_CCR_V) ? 1 : 0;
	int cond = !(n ^ v);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bge_xcc)
{
	MODE_uint_t old_pc = cpu->pc;
	int n = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_N)? 1:0;
	int v = ((cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_V)? 1:0;
	int cond = !(n ^ v);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  be
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(be)
{
	MODE_uint_t old_pc = cpu->pc;
	int cond = cpu->cd.sparc.ccr & SPARC_CCR_Z;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(be_xcc)
{
	MODE_uint_t old_pc = cpu->pc;
	int cond = (cpu->cd.sparc.ccr >> SPARC_CCR_XCC_SHIFT) & SPARC_CCR_Z;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  ba
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 */
X(ba)
{
	MODE_uint_t old_pc = cpu->pc;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
		    << SPARC_INSTR_ALIGNMENT_SHIFT);
		cpu->pc = old_pc + (int32_t)ic->arg[0];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  brnz
 *
 *  arg[0] = int32_t displacement compared to the start of the current page
 *  arg[1] = ptr to rs1
 */
X(brnz)
{
	MODE_uint_t old_pc = cpu->pc;
	int cond = reg(ic->arg[1]) != 0;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			old_pc &= ~((SPARC_IC_ENTRIES_PER_PAGE - 1)
			    << SPARC_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[0];
			quick_pc_to_pointers(cpu);
		}
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  Save:
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 *  arg[2] = ptr to rd (_after_ the register window change)
 */
X(save_v9_imm)
{
	MODE_uint_t rs = reg(ic->arg[0]) + (int32_t)ic->arg[1];
	int cwp = cpu->cd.sparc.cwp;

	if (cpu->cd.sparc.cansave == 0) {
		fatal("save_v9_imm: spill trap. TODO\n");
		exit(1);
	}

	if (cpu->cd.sparc.cleanwin - cpu->cd.sparc.canrestore == 0) {
		fatal("save_v9_imm: clean_window trap. TODO\n");
		exit(1);
	}

	/*  Save away old in registers:  */
	memcpy(&cpu->cd.sparc.r_inout[cwp][0], &cpu->cd.sparc.r[SPARC_REG_I0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_I0]) * N_SPARC_INOUT_REG);

	/*  Save away old local registers:  */
	memcpy(&cpu->cd.sparc.r_local[cwp][0], &cpu->cd.sparc.r[SPARC_REG_L0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_L0]) * N_SPARC_INOUT_REG);

	cpu->cd.sparc.cwp = (cwp + 1) % cpu->cd.sparc.cpu_type.nwindows;
	cpu->cd.sparc.cansave --;
	cpu->cd.sparc.canrestore ++;	/*  TODO: modulo here too?  */
	cwp = cpu->cd.sparc.cwp;

	/*  The out registers become the new in registers:  */
	memcpy(&cpu->cd.sparc.r[SPARC_REG_I0], &cpu->cd.sparc.r[SPARC_REG_O0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_O0]) * N_SPARC_INOUT_REG);

	/*  Read new local registers:  */
	memcpy(&cpu->cd.sparc.r[SPARC_REG_L0], &cpu->cd.sparc.r_local[cwp][0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_L0]) * N_SPARC_INOUT_REG);

	reg(ic->arg[2]) = rs;
}


/*
 *  Restore:
 */
X(restore)
{
	int cwp = cpu->cd.sparc.cwp;

	if (cpu->cd.sparc.canrestore == 0) {
		fatal("restore: spill trap. TODO\n");
		exit(1);
	}

	cpu->cd.sparc.cwp = cwp - 1;
	if (cwp == 0)
		cpu->cd.sparc.cwp = cpu->cd.sparc.cpu_type.nwindows - 1;
	cpu->cd.sparc.cansave ++;
	cpu->cd.sparc.canrestore --;
	cwp = cpu->cd.sparc.cwp;

	/*  The in registers become the new out registers:  */
	memcpy(&cpu->cd.sparc.r[SPARC_REG_O0], &cpu->cd.sparc.r[SPARC_REG_I0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_O0]) * N_SPARC_INOUT_REG);

	/*  Read back the local registers:  */
	memcpy(&cpu->cd.sparc.r[SPARC_REG_L0], &cpu->cd.sparc.r_local[cwp][0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_L0]) * N_SPARC_INOUT_REG);

	/*  Read back the in registers:  */
	memcpy(&cpu->cd.sparc.r[SPARC_REG_I0], &cpu->cd.sparc.r_inout[cwp][0],
	    sizeof(cpu->cd.sparc.r[SPARC_REG_I0]) * N_SPARC_INOUT_REG);
}


/*
 *  Jump and link
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 *  arg[2] = ptr to rd
 */
X(jmpl_imm)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);
	reg(ic->arg[2]) = cpu->pc;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + (int32_t)ic->arg[1];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jmpl_imm_no_rd)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + (int32_t)ic->arg[1];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jmpl_reg)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);
	reg(ic->arg[2]) = cpu->pc;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + reg(ic->arg[1]);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jmpl_reg_no_rd)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + reg(ic->arg[1]);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


X(jmpl_imm_trace)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);
	reg(ic->arg[2]) = cpu->pc;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + (int32_t)ic->arg[1];
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jmpl_reg_trace)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);
	reg(ic->arg[2]) = cpu->pc;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + reg(ic->arg[1]);
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(retl_trace)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + (int32_t)ic->arg[1];
		quick_pc_to_pointers(cpu);
		cpu_functioncall_trace_return(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  Return
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 */
X(return_imm)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + (int32_t)ic->arg[1];
		quick_pc_to_pointers(cpu);
		instr(restore)(cpu, ic);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(return_reg)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + reg(ic->arg[1]);
		quick_pc_to_pointers(cpu);
		instr(restore)(cpu, ic);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(return_imm_trace)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + (int32_t)ic->arg[1];
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
		instr(restore)(cpu, ic);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(return_reg_trace)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = reg(ic->arg[0]) + reg(ic->arg[1]);
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
		instr(restore)(cpu, ic);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  set:  Set a register to a value (e.g. sethi).
 *
 *  arg[0] = ptr to rd
 *  arg[1] = value (uint32_t)
 */
X(set)
{
	reg(ic->arg[0]) = (uint32_t)ic->arg[1];
}


/*
 *  Computational/arithmetic instructions:
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 *  arg[2] = ptr to rd
 */
X(add)      { reg(ic->arg[2]) = reg(ic->arg[0]) + reg(ic->arg[1]); }
X(add_imm)  { reg(ic->arg[2]) = reg(ic->arg[0]) + (int32_t)ic->arg[1]; }
X(and)      { reg(ic->arg[2]) = reg(ic->arg[0]) & reg(ic->arg[1]); }
X(and_imm)  { reg(ic->arg[2]) = reg(ic->arg[0]) & (int32_t)ic->arg[1]; }
X(andn)     { reg(ic->arg[2]) = reg(ic->arg[0]) & ~reg(ic->arg[1]); }
X(andn_imm) { reg(ic->arg[2]) = reg(ic->arg[0]) & ~(int32_t)ic->arg[1]; }
X(or)       { reg(ic->arg[2]) = reg(ic->arg[0]) | reg(ic->arg[1]); }
X(or_imm)   { reg(ic->arg[2]) = reg(ic->arg[0]) | (int32_t)ic->arg[1]; }
X(xor)      { reg(ic->arg[2]) = reg(ic->arg[0]) ^ reg(ic->arg[1]); }
X(xor_imm)  { reg(ic->arg[2]) = reg(ic->arg[0]) ^ (int32_t)ic->arg[1]; }
X(sub)      { reg(ic->arg[2]) = reg(ic->arg[0]) - reg(ic->arg[1]); }
X(sub_imm)  { reg(ic->arg[2]) = reg(ic->arg[0]) - (int32_t)ic->arg[1]; }

X(sll)      { reg(ic->arg[2]) = (uint32_t)reg(ic->arg[0]) <<
		(reg(ic->arg[1]) & 31); }
X(sllx)     { reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0]) <<
		(reg(ic->arg[1]) & 63); }
X(sll_imm)  { reg(ic->arg[2]) = (uint32_t)reg(ic->arg[0]) << ic->arg[1]; }
X(sllx_imm) { reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0]) << ic->arg[1]; }

X(srl)      { reg(ic->arg[2]) = (uint32_t)reg(ic->arg[0]) >>
		(reg(ic->arg[1]) & 31); }
X(srlx)     { reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0]) >>
		(reg(ic->arg[1]) & 63); }
X(srl_imm)  { reg(ic->arg[2]) = (uint32_t)reg(ic->arg[0]) >> ic->arg[1]; }
X(srlx_imm) { reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0]) >> ic->arg[1]; }

X(sra)      { reg(ic->arg[2]) = (int32_t)reg(ic->arg[0]) >>
		(reg(ic->arg[1]) & 31); }
X(srax)     { reg(ic->arg[2]) = (int64_t)reg(ic->arg[0]) >>
		(reg(ic->arg[1]) & 63); }
X(sra_imm)  { reg(ic->arg[2]) = (int32_t)reg(ic->arg[0]) >> ic->arg[1]; }
X(srax_imm) { reg(ic->arg[2]) = (int64_t)reg(ic->arg[0]) >> ic->arg[1]; }

X(udiv)
{
	uint64_t z = (cpu->cd.sparc.y << 32) | (uint32_t)reg(ic->arg[0]);
	z /= (uint32_t)reg(ic->arg[1]);
	if (z > 0xffffffff)
		z = 0xffffffff;
	reg(ic->arg[2]) = z;
}
X(udiv_imm)
{
	uint64_t z = (cpu->cd.sparc.y << 32) | (uint32_t)reg(ic->arg[0]);
	z /= (uint32_t)ic->arg[1];
	if (z > 0xffffffff)
		z = 0xffffffff;
	reg(ic->arg[2]) = z;
}


/*
 *  Add with ccr update:
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 *  arg[2] = ptr to rd
 */
int32_t sparc_addcc32(struct cpu *cpu, int32_t rs1, int32_t rs2);
#ifdef MODE32
int32_t sparc_addcc32(struct cpu *cpu, int32_t rs1, int32_t rs2)
#else
int64_t sparc_addcc64(struct cpu *cpu, int64_t rs1, int64_t rs2)
#endif
{
	int cc = 0, sign1 = 0, sign2 = 0, signd = 0, mask = SPARC_CCR_ICC_MASK;
	MODE_int_t rd = rs1 + rs2;
	if (rd == 0)
		cc = SPARC_CCR_Z;
	else if (rd < 0)
		cc = SPARC_CCR_N, signd = 1;
	if (rs1 < 0)
		sign1 = 1;
	if (rs2 < 0)
		sign2 = 1;
	if (sign1 == sign2 && sign1 != signd)
		cc |= SPARC_CCR_V;
	/*  TODO: SPARC_CCR_C  */
#ifndef MODE32
	mask <<= SPARC_CCR_XCC_SHIFT;
	cc <<= SPARC_CCR_XCC_SHIFT;
#endif
	cpu->cd.sparc.ccr &= ~mask;
	cpu->cd.sparc.ccr |= cc;
	return rd;
}
X(addcc)
{
	/*  Like add, but updates the ccr, and does both 32-bit and
	    64-bit comparison at the same time.  */
	MODE_int_t rs1 = reg(ic->arg[0]), rs2 = reg(ic->arg[1]), rd;
	rd = sparc_addcc32(cpu, rs1, rs2);
#ifndef MODE32
	rd = sparc_addcc64(cpu, rs1, rs2);
#endif
	reg(ic->arg[2]) = rd;
}
X(addcc_imm)
{
	MODE_int_t rs1 = reg(ic->arg[0]), rs2 = (int32_t)ic->arg[1], rd;
	rd = sparc_addcc32(cpu, rs1, rs2);
#ifndef MODE32
	rd = sparc_addcc64(cpu, rs1, rs2);
#endif
	reg(ic->arg[2]) = rd;
}


/*
 *  And with ccr update:
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 *  arg[2] = ptr to rd
 */
int32_t sparc_andcc32(struct cpu *cpu, int32_t rs1, int32_t rs2);
#ifdef MODE32
int32_t sparc_andcc32(struct cpu *cpu, int32_t rs1, int32_t rs2)
#else
int64_t sparc_andcc64(struct cpu *cpu, int64_t rs1, int64_t rs2)
#endif
{
	int cc = 0, mask = SPARC_CCR_ICC_MASK;
	MODE_int_t rd = rs1 & rs2;
	if (rd == 0)
		cc = SPARC_CCR_Z;
	else if (rd < 0)
		cc = SPARC_CCR_N;
	/*  Note: SPARC_CCR_C and SPARC_CCR_V are always zero.  */
#ifndef MODE32
	mask <<= SPARC_CCR_XCC_SHIFT;
	cc <<= SPARC_CCR_XCC_SHIFT;
#endif
	cpu->cd.sparc.ccr &= ~mask;
	cpu->cd.sparc.ccr |= cc;
	return rd;
}
X(andcc)
{
	/*  Like and, but updates the ccr, and does both 32-bit and
	    64-bit comparison at the same time.  */
	MODE_int_t rs1 = reg(ic->arg[0]), rs2 = reg(ic->arg[1]), rd;
	rd = sparc_andcc32(cpu, rs1, rs2);
#ifndef MODE32
	rd = sparc_andcc64(cpu, rs1, rs2);
#endif
	reg(ic->arg[2]) = rd;
}
X(andcc_imm)
{
	MODE_int_t rs1 = reg(ic->arg[0]), rs2 = (int32_t)ic->arg[1], rd;
	rd = sparc_andcc32(cpu, rs1, rs2);
#ifndef MODE32
	rd = sparc_andcc64(cpu, rs1, rs2);
#endif
	reg(ic->arg[2]) = rd;
}


/*
 *  Subtract with ccr update:
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 *  arg[2] = ptr to rd
 */
int32_t sparc_subcc32(struct cpu *cpu, int32_t rs1, int32_t rs2);
#ifdef MODE32
int32_t sparc_subcc32(struct cpu *cpu, int32_t rs1, int32_t rs2)
#else
int64_t sparc_subcc64(struct cpu *cpu, int64_t rs1, int64_t rs2)
#endif
{
	int cc = 0, sign1 = 0, sign2 = 0, signd = 0, mask = SPARC_CCR_ICC_MASK;
	MODE_int_t rd = rs1 - rs2;
	if (rd == 0)
		cc = SPARC_CCR_Z;
	else if (rd < 0)
		cc = SPARC_CCR_N, signd = 1;
	if (rs1 < 0)
		sign1 = 1;
	if (rs2 < 0)
		sign2 = 1;
	if (sign1 != sign2 && sign1 != signd)
		cc |= SPARC_CCR_V;
	/*  TODO: SPARC_CCR_C  */
#ifndef MODE32
	mask <<= SPARC_CCR_XCC_SHIFT;
	cc <<= SPARC_CCR_XCC_SHIFT;
#endif
	cpu->cd.sparc.ccr &= ~mask;
	cpu->cd.sparc.ccr |= cc;
	return rd;
}
X(subcc)
{
	/*  Like sub, but updates the ccr, and does both 32-bit and
	    64-bit comparison at the same time.  */
	MODE_int_t rs1 = reg(ic->arg[0]), rs2 = reg(ic->arg[1]), rd;
	rd = sparc_subcc32(cpu, rs1, rs2);
#ifndef MODE32
	rd = sparc_subcc64(cpu, rs1, rs2);
#endif
	reg(ic->arg[2]) = rd;
}
X(subcc_imm)
{
	MODE_int_t rs1 = reg(ic->arg[0]), rs2 = (int32_t)ic->arg[1], rd;
	rd = sparc_subcc32(cpu, rs1, rs2);
#ifndef MODE32
	rd = sparc_subcc64(cpu, rs1, rs2);
#endif
	reg(ic->arg[2]) = rd;
}


#include "tmp_sparc_loadstore.cc"


/*
 *  flushw:  Flush Register Windows
 */
X(flushw)
{
	/*  flushw acts as a nop, if cansave = nwindows - 2:  */
	if (cpu->cd.sparc.cansave == cpu->cd.sparc.cpu_type.nwindows - 2)
		return;

	/*  TODO  */
	fatal("flushw: TODO: cansave = %i\n", cpu->cd.sparc.cansave);
	exit(1);
}


/*
 *  rd:  Read special register
 *
 *  arg[2] = ptr to rd
 */
X(rd_psr)
{
	reg(ic->arg[2]) = cpu->cd.sparc.psr;
}


/*
 *  rdpr:  Read privileged register
 *
 *  arg[2] = ptr to rd
 */
X(rdpr_tba)
{
	reg(ic->arg[2]) = cpu->cd.sparc.tba;
}
X(rdpr_ver)
{
	reg(ic->arg[2]) = cpu->cd.sparc.ver;
}


/*
 *  wrpr:  Write to privileged register
 *
 *  arg[0] = ptr to rs1
 *  arg[1] = ptr to rs2 or an immediate value (int32_t)
 */
X(wrpr_tick)
{
	cpu->cd.sparc.tick = (uint32_t) (reg(ic->arg[0]) ^ reg(ic->arg[1]));
}
X(wrpr_tick_imm)
{
	cpu->cd.sparc.tick = (uint32_t) (reg(ic->arg[0]) ^ (int32_t)ic->arg[1]);
}
X(wrpr_pil)
{
	cpu->cd.sparc.pil = (reg(ic->arg[0]) ^ reg(ic->arg[1]))
	    & SPARC_PIL_MASK;
}
X(wrpr_pil_imm)
{
	cpu->cd.sparc.pil = (reg(ic->arg[0]) ^ (int32_t)ic->arg[1])
	    & SPARC_PIL_MASK;
}
X(wrpr_pstate)
{
	sparc_update_pstate(cpu, reg(ic->arg[0]) ^ reg(ic->arg[1]));
}
X(wrpr_pstate_imm)
{
	sparc_update_pstate(cpu, reg(ic->arg[0]) ^ (int32_t)ic->arg[1]);
}
X(wrpr_cleanwin)
{
	cpu->cd.sparc.cleanwin = (uint32_t) (reg(ic->arg[0]) ^ reg(ic->arg[1]));
}
X(wrpr_cleanwin_imm)
{
	cpu->cd.sparc.cleanwin =
	    (uint32_t) (reg(ic->arg[0]) ^ (int32_t)ic->arg[1]);
}


/*****************************************************************************/


X(end_of_page)
{
	/*  Update the PC:  (offset 0, but on the next page)  */
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1) <<
	    SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (SPARC_IC_ENTRIES_PER_PAGE <<
	    SPARC_INSTR_ALIGNMENT_SHIFT);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);

	/*  end_of_page doesn't count as an executed instruction:  */
	cpu->n_translated_instrs --;
}


X(end_of_page2)
{
	/*  Synchronize PC on the _second_ instruction on the next page:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);
	cpu->pc &= ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);

	/*  This doesn't count as an executed instruction.  */
	cpu->n_translated_instrs --;

	quick_pc_to_pointers(cpu);

	if (cpu->delay_slot == NOT_DELAYED)
		return;

	fatal("end_of_page2: fatal error, we're in a delay slot\n");
	exit(1);
}


/*****************************************************************************/


/*
 *  sparc_instr_to_be_translated():
 *
 *  Translate an instruction word into a sparc_instr_call. ic is filled in with
 *  valid data for the translated instruction, or a "nothing" instruction if
 *  there was a translation failure. The newly translated instruction is then
 *  executed.
 */
X(to_be_translated)
{
	MODE_uint_t addr;
	int low_pc, in_crosspage_delayslot = 0;
	uint32_t iword;
	unsigned char *page;
	unsigned char ib[4];
	int main_opcode, op2, rd, rs1, rs2, btype, asi, cc, p, use_imm, x64 = 0;
	int store, signedness, size;
	int32_t tmpi32, siconst;
	/* void (*samepage_function)(struct cpu *, struct sparc_instr_call *);*/

	/*  Figure out the (virtual) address of the instruction:  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.sparc.cur_ic_page)
	    / sizeof(struct sparc_instr_call);

	/*  Special case for branch with delayslot on the next page:  */
	if (cpu->delay_slot == TO_BE_DELAYED && low_pc == 0) {
		/*  fatal("[ delay-slot translation across page "
		    "boundary ]\n");  */
		in_crosspage_delayslot = 1;
	}

	addr = cpu->pc & ~((SPARC_IC_ENTRIES_PER_PAGE-1)
	    << SPARC_INSTR_ALIGNMENT_SHIFT);
	addr += (low_pc << SPARC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc = addr;
	addr &= ~((1 << SPARC_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Read the instruction word from memory:  */
#ifdef MODE32
	page = cpu->cd.sparc.host_load[addr >> 12];
#else
	{
		const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
		const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
		const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
		uint32_t x1 = (addr >> (64-DYNTRANS_L1N)) & mask1;
		uint32_t x2 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
		uint32_t x3 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N-
		    DYNTRANS_L3N)) & mask3;
		struct DYNTRANS_L2_64_TABLE *l2 = cpu->cd.sparc.l1_64[x1];
		struct DYNTRANS_L3_64_TABLE *l3 = l2->l3[x2];
		page = l3->host_load[x3];
	}
#endif

	if (page != NULL) {
		/*  fatal("TRANSLATION HIT!\n");  */
		memcpy(ib, page + (addr & 0xffc), sizeof(ib));
	} else {
		/*  fatal("TRANSLATION MISS!\n");  */
		if (!cpu->memory_rw(cpu, cpu->mem, addr, ib,
		    sizeof(ib), MEM_READ, CACHE_INSTRUCTION)) {
			fatal("to_be_translated(): "
			    "read failed: TODO\n");
			goto bad;
		}
	}

	/*  SPARC instruction words are always big-endian. Convert
	    to host order:  */
	{
		uint32_t *p = (uint32_t *) ib;
		iword = *p;
		iword = BE32_TO_HOST(iword);
	}

#define DYNTRANS_TO_BE_TRANSLATED_HEAD
#include "cpu_dyntrans.cc"
#undef  DYNTRANS_TO_BE_TRANSLATED_HEAD


	/*
	 *  Translate the instruction:
	 */

	main_opcode = iword >> 30;
	rd = (iword >> 25) & 31;
	btype = rd & (N_SPARC_BRANCH_TYPES - 1);
	rs1 = (iword >> 14) & 31;
	use_imm = (iword >> 13) & 1;
	asi = (iword >> 5) & 0xff;
	rs2 = iword & 31;
	siconst = (int16_t)((iword & 0x1fff) << 3) >> 3;
	op2 = (main_opcode == 0)? ((iword >> 22) & 7) : ((iword >> 19) & 0x3f);
	cc = (iword >> 20) & 3;
	p = (iword >> 19) & 1;

	switch (main_opcode) {

	case 0:	switch (op2) {

		case 1:	/*  branch (icc or xcc)  */
			tmpi32 = (iword << 13);
			tmpi32 >>= 11;
			ic->arg[0] = (int32_t)tmpi32 + (addr & 0xffc);
			/*  rd contains the annul bit concatenated with 4 bits
			    of condition code. cc=0 for icc, 2 for xcc:  */
			/*  TODO: samepage  */
			switch (rd + (cc << 5)) {
			case 0x01:	ic->f = instr(be);  break;
			case 0x02:	ic->f = instr(ble); break;
			case 0x03:	ic->f = instr(bl);  break;
			case 0x08:	ic->f = instr(ba);  break;
			case 0x09:	ic->f = instr(bne); break;
			case 0x0a:	ic->f = instr(bg);  break;
			case 0x0b:	ic->f = instr(bge); break;
			case 0x19:	ic->f = instr(bne_a);  break;
			case 0x41:	ic->f = instr(be_xcc); break;
			case 0x42:	ic->f = instr(ble_xcc);break;
			case 0x43:	ic->f = instr(bl_xcc); break;
			case 0x48:	ic->f = instr(ba);     break;
			case 0x4a:	ic->f = instr(bg_xcc); break;
			case 0x4b:	ic->f = instr(bge_xcc);break;
			default:fatal("Unimplemented branch, 0x%x\n",
				    rd + (cc<<5));
				goto bad;
			}
			break;

		case 2:	/*  branch (32-bit integer comparison)  */
			tmpi32 = (iword << 10);
			tmpi32 >>= 8;
			ic->arg[0] = (int32_t)tmpi32 + (addr & 0xffc);
			/*  rd contains the annul bit concatenated with 4 bits
			    of condition code:  */
			/*  TODO: samepage  */
			switch (rd) {
			case 0x01:	ic->f = instr(be);  break;
			case 0x03:	ic->f = instr(bl);  break;
			case 0x08:	ic->f = instr(ba);  break;
			case 0x09:	ic->f = instr(bne); break;
			case 0x0b:	ic->f = instr(bge); break;
			default:fatal("Unimplemented branch rd=%i\n", rd);
				goto bad;
			}
			break;

		case 3:	/*  branch on register, 64-bit integer comparison  */
			tmpi32 = ((iword & 0x300000) >> 6) | (iword & 0x3fff);
			tmpi32 <<= 16;
			tmpi32 >>= 14;
			ic->arg[0] = (int32_t)tmpi32 + (addr & 0xffc);
			ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs1];
			/*  TODO: samepage  */
			switch (btype) {
			case 0x05:	ic->f = instr(brnz); break;
			default:fatal("Unimplemented branch 0x%x\n", rd);
				goto bad;
			}
			break;

		case 4:	/*  sethi  */
			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rd];
			ic->arg[1] = (iword & 0x3fffff) << 10;
			ic->f = instr(set);
			if (rd == SPARC_ZEROREG)
				ic->f = instr(nop);
			break;

		default:fatal("TODO: unimplemented op2=%i for main "
			    "opcode %i\n", op2, main_opcode);
			goto bad;
		}
		break;

	case 1:	/*  call and link  */
		tmpi32 = (iword << 2);
		ic->arg[0] = (int32_t)tmpi32;
		ic->arg[1] = addr & 0xffc;
		if (cpu->machine->show_trace_tree)
			ic->f = instr(call_trace);
		else
			ic->f = instr(call);
		/*  TODO: samepage  */
		break;

	case 2:	switch (op2) {

		case 0:	/*  add  */
		case 1:	/*  and  */
		case 2:	/*  or  */
		case 3:	/*  xor  */
		case 4:	/*  sub  */
		case 5:	/*  andn  */
		case 14:/*  udiv  */
		case 16:/*  addcc  */
		case 17:/*  andcc  */
		case 20:/*  subcc (cmp)  */
		case 37:/*  sll  */
		case 38:/*  srl  */
		case 39:/*  sra  */
		case 60:/*  save  */
			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rs1];
			ic->f = NULL;
			if (use_imm) {
				ic->arg[1] = siconst;
				switch (op2) {
				case 0:	ic->f = instr(add_imm); break;
				case 1:	ic->f = instr(and_imm); break;
				case 2:	ic->f = instr(or_imm); break;
				case 3:	ic->f = instr(xor_imm); break;
				case 4:	ic->f = instr(sub_imm); break;
				case 5:	ic->f = instr(andn_imm); break;
				case 14:ic->f = instr(udiv_imm); break;
				case 16:ic->f = instr(addcc_imm); break;
				case 17:ic->f = instr(andcc_imm); break;
				case 20:ic->f = instr(subcc_imm); break;
				case 37:if (siconst & 0x1000) {
						ic->f = instr(sllx_imm);
						ic->arg[1] &= 63;
						x64 = 1;
					} else {
						ic->f = instr(sll_imm);
						ic->arg[1] &= 31;
					}
					break;
				case 38:if (siconst & 0x1000) {
						ic->f = instr(srlx_imm);
						ic->arg[1] &= 63;
						x64 = 1;
					} else {
						ic->f = instr(srl_imm);
						ic->arg[1] &= 31;
					}
					break;
				case 39:if (siconst & 0x1000) {
						ic->f = instr(srax_imm);
						ic->arg[1] &= 63;
						x64 = 1;
					} else {
						ic->f = instr(sra_imm);
						ic->arg[1] &= 31;
					}
					break;
				case 60:switch (cpu->cd.sparc.cpu_type.v) {
					case 9:	ic->f = instr(save_v9_imm);
						break;
					default:fatal("only for v9 so far\n");
						goto bad;
					}
				}
			} else {
				ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs2];
				switch (op2) {
				case 0: ic->f = instr(add); break;
				case 1: ic->f = instr(and); break;
				case 2: ic->f = instr(or); break;
				case 3: ic->f = instr(xor); break;
				case 4: ic->f = instr(sub); break;
				case 5: ic->f = instr(andn); break;
				case 14:ic->f = instr(udiv); break;
				case 16:ic->f = instr(addcc); break;
				case 17:ic->f = instr(andcc); break;
				case 20:ic->f = instr(subcc); break;
				case 37:if (siconst & 0x1000) {
						ic->f = instr(sllx);
						x64 = 1;
					} else
						ic->f = instr(sll);
					break;
				case 38:if (siconst & 0x1000) {
						ic->f = instr(srlx);
						x64 = 1;
					} else
						ic->f = instr(srl);
					break;
				case 39:if (siconst & 0x1000) {
						ic->f = instr(srax);
						x64 = 1;
					} else
						ic->f = instr(sra);
					break;
				}
			}
			if (ic->f == NULL) {
				fatal("TODO: Unimplemented instruction "
				    "(possibly missed use_imm impl.)\n");
				goto bad;
			}
			ic->arg[2] = (size_t)&cpu->cd.sparc.r[rd];
			if (rd == SPARC_ZEROREG) {
				/*
				 *  Some opcodes should write to the scratch
				 *  register instead of becoming NOPs, when
				 *  rd is the zero register.
				 *
				 *  Any opcode which updates the condition
				 *  codes, or anything which changes register
				 *  windows.
				 */
				switch (op2) {
				case 16:/*  addcc  */
				case 17:/*  andcc  */
				case 20:/*  subcc  */
				case 60:/*  save  */
					ic->arg[2] = (size_t)
					    &cpu->cd.sparc.scratch;
					break;
				default:ic->f = instr(nop);
				}
			}
			break;

		case 41:/*  rd %psr,%gpr on pre-sparcv9  */
			if (cpu->is_32bit) {
				ic->f = instr(rd_psr);
				ic->arg[2] = (size_t)&cpu->cd.sparc.r[rd];
				if (rd == SPARC_ZEROREG)
					ic->f = instr(nop);
			} else {
				fatal("opcode 2,41 not yet implemented"
				    " for 64-bit cpus\n");
				goto bad;
			}
			break;

		case 42:/*  rdpr on sparcv9  */
			if (cpu->is_32bit) {
				fatal("opcode 2,42 not yet implemented"
				    " for 32-bit cpus\n");
				goto bad;
			}
			ic->arg[2] = (size_t)&cpu->cd.sparc.r[rd];
			if (rd == SPARC_ZEROREG)
				ic->f = instr(nop);
			switch (rs1) {
			case  5:  ic->f = instr(rdpr_tba); break;
			case 31:  ic->f = instr(rdpr_ver); break;
			default:fatal("Unimplemented rs1=%i\n", rs1);
				goto bad;
			}
			break;

		case 43:if (iword == 0x81580000) {
				ic->f = instr(flushw);
			} else {
				fatal("Unimplemented iword=0x%08"PRIx32"\n",
				    iword);
				goto bad;
			}
			break;

		case 48:/*  wr  (Note: works as xor)  */
			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rs1];
			if (use_imm) {
				ic->arg[1] = siconst;
				ic->f = instr(xor_imm);
			} else {
				ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs2];
				ic->f = instr(xor);
			}
			ic->arg[2] = (size_t) NULL;
			switch (rd) {
			case 0:	ic->arg[2] = (size_t)&cpu->cd.sparc.y;
				break;
			case 6:	ic->arg[2] = (size_t)&cpu->cd.sparc.fprs;
				break;
			case 0x17:
				ic->arg[2] = (size_t)&cpu->cd.sparc.tick_cmpr;
				break;
			}
			if (ic->arg[2] == (size_t)NULL) {
				fatal("TODO: Unimplemented wr instruction, "
				    "rd = 0x%02x\n", rd);
				goto bad;
			}
			break;

		case 50:/*  wrpr  (Note: works as xor)  */
			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rs1];
			ic->f = NULL;
			if (use_imm) {
				ic->arg[1] = siconst;
				switch (rd) {
				case 4:	ic->f = instr(wrpr_tick_imm); break;
				case 6: ic->f = instr(wrpr_pstate_imm); break;
				case 8: ic->f = instr(wrpr_pil_imm); break;
				case 12:ic->f = instr(wrpr_cleanwin_imm);break;
				}
			} else {
				ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs2];
				switch (rd) {
				case 4:	ic->f = instr(wrpr_tick); break;
				case 6:	ic->f = instr(wrpr_pstate); break;
				case 8:	ic->f = instr(wrpr_pil); break;
				case 12:ic->f = instr(wrpr_cleanwin); break;
				}
			}
			if (ic->f == NULL) {
				fatal("TODO: Unimplemented wrpr instruction,"
				    " rd = 0x%02x\n", rd);
				goto bad;
			}
			break;

		case 56:/*  jump and link  */
			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rs1];
			ic->arg[2] = (size_t)&cpu->cd.sparc.r[rd];
			if (rd == SPARC_ZEROREG)
				ic->arg[2] = (size_t)&cpu->cd.sparc.scratch;

			if (use_imm) {
				ic->arg[1] = siconst;
				if (rd == SPARC_ZEROREG)
					ic->f = instr(jmpl_imm_no_rd);
				else
					ic->f = instr(jmpl_imm);
			} else {
				ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs2];
				if (rd == SPARC_ZEROREG)
					ic->f = instr(jmpl_reg_no_rd);
				else
					ic->f = instr(jmpl_reg);
			}

			/*  special trace case:  */
			if (cpu->machine->show_trace_tree) {
				if (iword == 0x81c3e008)
					ic->f = instr(retl_trace);
				else {
					if (use_imm)
						ic->f = instr(jmpl_imm_trace);
					else
						ic->f = instr(jmpl_reg_trace);
				}
			}
			break;

		case 57:/*  return  */
			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rs1];

			if (use_imm) {
				ic->arg[1] = siconst;
				ic->f = instr(return_imm);
			} else {
				ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs2];
				ic->f = instr(return_reg);
			}

			/*  special trace case:  */
			if (cpu->machine->show_trace_tree) {
				if (use_imm)
					ic->f = instr(return_imm_trace);
				else
					ic->f = instr(return_reg_trace);
			}
			break;

		default:fatal("TODO: unimplemented op2=%i for main "
			    "opcode %i\n", op2, main_opcode);
			goto bad;
		}
		break;

	case 3:	switch (op2) {

		case  0:/*  lduw  */
		case  1:/*  ldub  */
		case  2:/*  lduh  */
		case  4:/*  st(w) */
		case  5:/*  stb   */
		case  6:/*  sth   */
		case  8:/*  ldsw  */
		case  9:/*  ldsb  */
		case 10:/*  ldsh  */
		case 11:/*  ldx  */
		case 14:/*  stx   */
			store = 1; signedness = 0; size = 3;
			switch (op2) {
			case  0: /*  lduw  */	store=0; size=2; break;
			case  1: /*  ldub  */	store=0; size=0; break;
			case  2: /*  lduh  */	store=0; size=1; break;
			case  4: /*  st  */	size = 2; break;
			case  5: /*  stb  */	size = 0; break;
			case  6: /*  sth  */	size = 1; break;
			case  8: /*  ldsw  */	store=0; size=2; signedness=1;
						break;
			case  9: /*  ldsb  */	store=0; size=0; signedness=1;
						break;
			case 10: /*  ldsh  */	store=0; size=1; signedness=1;
						break;
			case 11: /*  ldx  */	store=0; break;
			case 14: /*  stx  */	break;
			}
			ic->f =
#ifdef MODE32
			    sparc32_loadstore
#else
			    sparc_loadstore
#endif
			    [ use_imm*16 + store*8 + size*2 + signedness ];

			ic->arg[0] = (size_t)&cpu->cd.sparc.r[rd];
			ic->arg[1] = (size_t)&cpu->cd.sparc.r[rs1];
			if (use_imm)
				ic->arg[2] = siconst;
			else
				ic->arg[2] = (size_t)&cpu->cd.sparc.r[rs2];

			if (!store && rd == SPARC_ZEROREG)
				ic->arg[0] = (size_t)&cpu->cd.sparc.scratch;

			break;

		default:fatal("TODO: unimplemented op2=%i for main "
			    "opcode %i\n", op2, main_opcode);
			goto bad;
		}
		break;

	}


	if (x64 && cpu->is_32bit) {
		fatal("TODO: 64-bit instr on 32-bit cpu\n");
		goto bad;
	}


#define	DYNTRANS_TO_BE_TRANSLATED_TAIL
#include "cpu_dyntrans.cc"
#undef	DYNTRANS_TO_BE_TRANSLATED_TAIL
}

