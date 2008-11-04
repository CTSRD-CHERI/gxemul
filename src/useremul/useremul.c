/*
 *  Copyright (C) 2004-2008  Anders Gavare.  All rights reserved.
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
 *  $Id: useremul.c,v 1.6.2.1 2008-01-18 19:12:34 debug Exp $
 *
 *  COMMENT: Userland (syscall) emulation framework
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "machine.h"
#include "misc.h"
#include "useremul.h"


struct syscall_emul {
	char		*name;
	int		arch;
	char		*cpu_name;
	void		(*f)(struct cpu *, uint32_t);
	void		(*setup)(struct cpu *, int, char **);

	struct syscall_emul *next;
};

static struct syscall_emul *first_syscall_emul;

/*  Max length of strings passed using syscall parameters:  */
#define	MAXLEN		8192


/*
 *  useremul_setup():
 *
 *  Set up an emulated environment suitable for running userland code. The
 *  program should already have been loaded into memory when this function
 *  is called.
 */
void useremul_setup(struct cpu *cpu, int argc, char **host_argv)
{
	struct syscall_emul *sep;

	sep = first_syscall_emul;

	while (sep != NULL) {
		if (strcasecmp(cpu->machine->userland_emul, sep->name) == 0) {
			sep->setup(cpu, argc, host_argv);
			return;
		}
		sep = sep->next;
	}

	fatal("useremul_setup(): internal error, unimplemented emulation?\n");
	exit(1);
}


/*
 *  useremul_syscall():
 *
 *  Handle userland syscalls.  This function is called whenever a userland
 *  process runs a 'syscall' instruction.  The code argument is the code
 *  embedded into the syscall instruction, if any.  (This 'code' value is not
 *  necessarily used by specific emulations.)
 */
void useremul_syscall(struct cpu *cpu, uint32_t code)
{
	if (cpu->useremul_syscall == NULL) {
		fatal("useremul_syscall(): cpu->useremul_syscall == NULL\n");
	} else
		cpu->useremul_syscall(cpu, code);
}


/*
 *  useremul_name_to_useremul():
 *
 *  Example:
 *     Input:  name = "netbsd/pmax"
 *     Output: sets *arch = ARCH_MIPS, *machine_name = "NetBSD/pmax",
 *             and *cpu_name = "R3000".
 */
void useremul_name_to_useremul(struct cpu *cpu, char *name, int *arch,
	char **machine_name, char **cpu_name)
{
	struct syscall_emul *sep;

	sep = first_syscall_emul;

	while (sep != NULL) {
		if (strcasecmp(name, sep->name) == 0) {
			if (cpu_family_ptr_by_number(sep->arch) == NULL) {
				printf("\nSupport for the CPU family needed"
				    " for '%s' userland emulation was not"
				    " enabled at configuration time.\n",
				    sep->name);
				exit(1);
			}

			if (cpu != NULL)
				cpu->useremul_syscall = sep->f;

			if (arch != NULL)
				*arch = sep->arch;

			if (machine_name != NULL)
				CHECK_ALLOCATION((*machine_name) =
				    strdup(sep->name));

			if (cpu_name != NULL)
				CHECK_ALLOCATION((*cpu_name) =
				    strdup(sep->cpu_name));

			return;
		}

		sep = sep->next;
	}

	fatal("Unknown userland emulation '%s'\n", name);
	exit(1);
}


/*
 *  add_useremul():
 *
 *  For internal use, from useremul_init() only. Adds an emulation mode.
 */
static void add_useremul(char *name, int arch, char *cpu_name,
	void (*f)(struct cpu *, uint32_t),
	void (*setup)(struct cpu *, int, char **))
{
	struct syscall_emul *sep;

	CHECK_ALLOCATION(sep = malloc(sizeof(struct syscall_emul)));
	memset(sep, 0, sizeof(sep));

	sep->name     = name;
	sep->arch     = arch;
	sep->cpu_name = cpu_name;
	sep->f        = f;
	sep->setup    = setup;

	sep->next = first_syscall_emul;
	first_syscall_emul = sep;
}


/*
 *  useremul_list_emuls():
 *
 *  List all available userland emulation modes.  (Actually, only those which
 *  have CPU support enabled.)
 */
void useremul_list_emuls(void)
{
	struct syscall_emul *sep;
	int iadd = DEBUG_INDENTATION * 2;

	sep = first_syscall_emul;

	if (sep == NULL)
		return;

	debug("The following userland-only (syscall) emulation modes are"
	    " available:\n\n");
	debug_indentation(iadd);

	while (sep != NULL) {
		if (cpu_family_ptr_by_number(sep->arch) != NULL) {
			debug("%s (default CPU \"%s\")\n",
			    sep->name, sep->cpu_name);
		}

		sep = sep->next;
	}

	debug_indentation(-iadd);
	debug("\n(Most of these modes are bogus.)\n\n");
}


/*
 *  useremul_init():
 *
 *  This function should be called before any other useremul_*() function
 *  is used.
 */
void useremul_init(void)
{
	/*  Note: These are in reverse alphabetic order:  */

	add_useremul("NetBSD/pmax", ARCH_MIPS, "R3000",
	    useremul_netbsd, useremul_netbsd_setup);

	add_useremul("FreeBSD/Alpha", ARCH_ALPHA, "21364",
	    useremul_freebsd, useremul_freebsd_setup);
}

