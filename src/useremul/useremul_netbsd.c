/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: NetBSD userland (syscall) emulation implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "useremul.h"

#include "errno_netbsd.h"
#include "syscall_netbsd.h"


/*
 *  useremul_netbsd_setup():
 *
 *  Set up an emulated userland environment suitable for running NetBSD
 *  binaries.
 */
void useremul_netbsd_setup(struct cpu *cpu, int argc, char **host_argv)
{
	uint64_t stack_top = 0x7fff0000;
	uint64_t stacksize = 8 * 1048576;
	uint64_t stack_margin = 16384;
	uint64_t cur_argv;
	int i, i2;
	int envc = 1;

	switch (cpu->machine->arch) {
	case ARCH_MIPS:
		/*  See netbsd/sys/src/arch/mips/mips_machdep.c:setregs()  */
		cpu->cd.mips.gpr[MIPS_GPR_A0] = stack_top - stack_margin;
		cpu->cd.mips.gpr[MIPS_GPR_T9] = cpu->pc;

		/*  The userland stack:  */
		cpu->cd.mips.gpr[MIPS_GPR_SP] = stack_top - stack_margin;
		add_symbol_name(&cpu->machine->symbol_context,
		    stack_top - stacksize, stacksize, "userstack", 0, 0);

		/*  Stack contents:  (TODO: is this correct?)  */
		store_32bit_word(cpu, stack_top - stack_margin, argc);

		cur_argv = stack_top - stack_margin + 128 + (argc + envc)
		    * sizeof(uint32_t);
		for (i=0; i<argc; i++) {
			debug("adding argv[%i]: '%s'\n", i, host_argv[i]);

			store_32bit_word(cpu, stack_top - stack_margin +
			    4 + i*sizeof(uint32_t), cur_argv);
			store_string(cpu, cur_argv, host_argv[i]);
			cur_argv += strlen(host_argv[i]) + 1;
		}

		/*  Store a NULL value between the args and the environment
		    strings:  */
		store_32bit_word(cpu, stack_top - stack_margin +
		    4 + i*sizeof(uint32_t), 0);  i++;

		/*  TODO: get environment strings from somewhere  */

		/*  Store all environment strings:  */
		for (i2 = 0; i2 < envc; i2 ++) {
			store_32bit_word(cpu, stack_top - stack_margin + 4
			    + (i+i2)*sizeof(uint32_t), cur_argv);
			store_string(cpu, cur_argv, "DISPLAY=localhost:0.0");
			cur_argv += strlen("DISPLAY=localhost:0.0") + 1;
		}
		break;

	default:
		fatal("useremul_netbsd_setup(): unimplemented arch\n");
		exit(1);
	}
}


/*
 *  useremul_netbsd():
 *
 *  NetBSD syscall emulation.
 */
void useremul_netbsd(struct cpu *cpu, uint32_t code)
{
	int syscall_nr = -1, error_flag = 0, result_high_set = 0;
	uint64_t arg0=0,arg1=0,arg2=0,arg3=0,stack0=0,stack1=0,stack2=0;
	int64_t result = 0;


	/*
	 *  Retrieve syscall arguments (in registers and/or the stack):
	 */

	switch (cpu->machine->arch) {

	case ARCH_MIPS:
		syscall_nr = cpu->cd.mips.gpr[MIPS_GPR_V0];
		if (syscall_nr == NETBSD_SYS___syscall) {
			syscall_nr = cpu->cd.mips.gpr[MIPS_GPR_A0] +
			    (cpu->cd.mips.gpr[MIPS_GPR_A1] << 32);
			arg0 = cpu->cd.mips.gpr[MIPS_GPR_A2];
			arg1 = cpu->cd.mips.gpr[MIPS_GPR_A3];

			/*  TODO:  stack arguments? Are these correct?  */
			arg2 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 8);
			arg3 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 16);
			stack0 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 24);
			stack1 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 32);
			stack2 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 40);
		} else {
			arg0 = cpu->cd.mips.gpr[MIPS_GPR_A0];
			arg1 = cpu->cd.mips.gpr[MIPS_GPR_A1];
			arg2 = cpu->cd.mips.gpr[MIPS_GPR_A2];
			arg3 = cpu->cd.mips.gpr[MIPS_GPR_A3];

			/*  TODO:  stack arguments? Are these correct?  */
			stack0 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 4);
			stack1 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 8);
			stack2 = load_32bit_word(cpu,
			    cpu->cd.mips.gpr[MIPS_GPR_SP] + 12);
		}
		break;
	}


	/*
	 *  Handle the syscall:
	 */

	switch (syscall_nr) {

	case NETBSD_SYS_exit:
		useremul_syscall_exit(cpu, arg0);
		break;

	case NETBSD_SYS_sync:
		useremul_syscall_sync(cpu);
		break;

	default:
		fatal("[ UNIMPLEMENTED NetBSD syscall nr %i ]\n", syscall_nr);
		error_flag = 1;  result = NETBSD_ENOSYS;

		/*  For now, let's abort execution:  */
		cpu->running = 0;
	}


	/*
	 *  Return:
	 */

	switch (cpu->machine->arch) {

	case ARCH_MIPS:
		/*
		 *  NetBSD/mips return values:
		 *
		 *  a3 is 0 if the syscall was ok, otherwise 1.
		 *  v0 (and sometimes v1) contain the result value.
		 */
		cpu->cd.mips.gpr[MIPS_GPR_A3] = error_flag;
		if (error_flag)
			cpu->cd.mips.gpr[MIPS_GPR_V0] = (int32_t)result;

		if (result_high_set)
			cpu->cd.mips.gpr[MIPS_GPR_V1] = (int32_t)(result >> 32);
		break;
	}
}

