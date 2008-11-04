#ifndef	USEREMUL_H
#define	USEREMUL_H

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
 *  $Id: useremul.h,v 1.1.2.1 2008-01-18 19:12:32 debug Exp $
 */

#include "misc.h"


void useremul_setup(struct cpu *, int, char **);
void useremul_syscall(struct cpu *cpu, uint32_t code);
void useremul_name_to_useremul(struct cpu *, char *name,
        int *arch, char **machine_name, char **cpu_name);
void useremul_list_emuls(void);
void useremul_init(void);


/*  FreeBSD syscall emulation:  */
void useremul_freebsd_setup(struct cpu *cpu, int argc, char **host_argv);
void useremul_freebsd(struct cpu *cpu, uint32_t code);


/*  NetBSD syscall emulation:  */
void useremul_netbsd_setup(struct cpu *cpu, int argc, char **host_argv);
void useremul_netbsd(struct cpu *cpu, uint32_t code);


/*  Common syscall implementations:  */
#define USEREMUL_SYSCALL0(name)  int32_t useremul_syscall_##name (struct cpu *cpu)
#define USEREMUL_SYSCALL1(name)  int32_t useremul_syscall_##name (struct cpu *cpu, uint64_t arg0)

/*  (Alphabetic order...)  */

USEREMUL_SYSCALL1(exit);
USEREMUL_SYSCALL0(sync);


#endif	/*  USEREMUL_H  */
