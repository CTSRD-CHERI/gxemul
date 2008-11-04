#ifndef	CPU_M32R_H
#define	CPU_M32R_H

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
 *  $Id: cpu_m32r.h,v 1.1.2.1 2008-01-18 19:12:31 debug Exp $
 */

#include "misc.h"
#include "interrupt.h"


struct cpu_family;

/*  M32R CPU types:  */
struct m32r_cpu_type_def {
	char		*name;
};

#define	M32R_CPU_TYPE_DEFS				{	\
	{ "M32R" },						\
	{ NULL } }


#define	M32R_N_IC_ARGS			3
#define	M32R_INSTR_ALIGNMENT_SHIFT	2
#define	M32R_IC_ENTRIES_SHIFT		10
#define	M32R_IC_ENTRIES_PER_PAGE	(1 << M32R_IC_ENTRIES_SHIFT)
#define	M32R_PC_TO_IC_ENTRY(a)		(((a)>>M32R_INSTR_ALIGNMENT_SHIFT) \
					& (M32R_IC_ENTRIES_PER_PAGE-1))
#define	M32R_ADDR_TO_PAGENR(a)		((a) >> (M32R_IC_ENTRIES_SHIFT \
					+ M32R_INSTR_ALIGNMENT_SHIFT))

DYNTRANS_MISC_DECLARATIONS(m32r,M32R,uint32_t)

#define	M32R_MAX_VPH_TLB_ENTRIES		192


#define	N_M32R_GPRS		16

struct m32r_cpu {
	struct m32r_cpu_type_def cpu_type;

	/*  General-Purpose Registers:  */
	uint32_t		r[N_M32R_GPRS+1];

	/*  Current interrupt assertion:  */
	int			irq_asserted;


	/*
	 *  Instruction translation cache, internal TLB structure, and 32-bit
	 *  virtual -> physical -> host address translation arrays for both
	 *  normal access and for the special .usr access mode (available in
	 *  supervisor mode).
	 */
	DYNTRANS_ITC(m32r)
	VPH_TLBS(m32r,M32R)
	VPH32(m32r,M32R)
};


/*  cpu_m32r.c:  */
int m32r_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib);
int m32r_run_instr(struct cpu *cpu);
void m32r_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void m32r_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void m32r_invalidate_code_translation(struct cpu *cpu, uint64_t, int);
int m32r_memory_rw(struct cpu *cpu, struct memory *mem, uint64_t vaddr,
	unsigned char *data, size_t len, int writeflag, int cache_flags);
int m32r_cpu_family_init(struct cpu_family *);

/*  memory_m32r.c:  */
int m32r_translate_v2p(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_addr, int flags);


#endif	/*  CPU_M32R_H  */
