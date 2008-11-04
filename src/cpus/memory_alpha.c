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
 *  $Id: memory_alpha.c,v 1.8.2.1 2008-01-18 19:12:27 debug Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "alpha_rpb.h"


/*
 *  alpha_translate_v2p():
 */
int alpha_translate_v2p(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_paddr, int flags)
{
	uint64_t base = cpu->cd.alpha.pcb.apcb_ptbr << ALPHA_PAGESHIFT;
	uint64_t addr, pte1, pte2, pte3;
	int i1, i2, i3;
	unsigned char *pt_entry_ptr;

	/*  Kernel direct-mapped space:  */
	if ((vaddr & ~0x1ffffffffffULL) == 0xfffffc0000000000ULL) {
		*return_paddr = vaddr & 0x000003ffffffffffULL;
		return 2;
	}

	i1 = (vaddr >> 33) & 0x3ff;
	i2 = (vaddr >> 23) & 0x3ff;
	i3 = (vaddr >> 13) & 0x3ff;

	debug("base = 0x%016"PRIx64"\n", base);
	debug("i1=0x%x i2=0x%x i3=0x%x\n", i1, i2, i3);

	addr = base + i1 * sizeof(uint64_t);

	pt_entry_ptr = memory_paddr_to_hostaddr(cpu->mem, addr, 0);
	if (pt_entry_ptr == NULL)
		goto not_found;

	pte1 = *(uint64_t *)(pt_entry_ptr);
	pte1 = LE64_TO_HOST(pte1);

	debug("pte1 = 0x%016"PRIx64"\n", pte1);

	addr = ((pte1 >> 32) << ALPHA_PAGESHIFT) + (i2 * sizeof(uint64_t));

	pt_entry_ptr = memory_paddr_to_hostaddr(cpu->mem, addr, 0);
	if (pt_entry_ptr == NULL)
		goto not_found;

	pte2 = *(uint64_t *)(pt_entry_ptr);
	pte2 = LE64_TO_HOST(pte2);

	debug("pte2 = 0x%016"PRIx64"\n", pte2);

	addr = ((pte2 >> 32) << ALPHA_PAGESHIFT) + (i3 * sizeof(uint64_t));

	pt_entry_ptr = memory_paddr_to_hostaddr(cpu->mem, addr, 0);
	if (pt_entry_ptr == NULL)
		goto not_found;

	pte3 = *(uint64_t *)(pt_entry_ptr);
	pte3 = LE64_TO_HOST(pte3);

	debug("pte3 = 0x%016"PRIx64"\n", pte3);

not_found:
	/*  No match.  */
	debug("[ alpha_translate_v2p: 0x%016"PRIx64" wasn't found ]\n", vaddr);

#if 1
	/*  UGLY hack for now:  */
	/*  TODO: Real virtual memory support.  */
	*return_paddr = vaddr & 0x000003ffffffffffULL;

	if ((vaddr & ~0x7fff) == 0x0000000010000000ULL)
		*return_paddr = (vaddr & 0x7fff) + HWRPB_PADDR;

	if ((vaddr & ~0xffffff) == 0xfffffe0000000000ULL)
		*return_paddr = 0x7efa000 + (vaddr & 0xffffff);

	/*  At 0x20000000, NetBSD stores 8KB temp prom data  */
	if ((vaddr & ~0x1fff) == 0x0000000020000000ULL)
		*return_paddr = (vaddr & 0x1fff) + PROM_ARGSPACE_PADDR;

	return 2;
#else
	return 0;
#endif
}

