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
 *  $Id: dev_pcc2.c,v 1.6.2.1 2008-01-18 19:12:29 debug Exp $
 *
 *  COMMENT: Peripheral Channel Controller (PCC2) bus (used in MVME machines)
 *
 *  See "Single Board Computers Programmer's Reference Guide (Part 2 of 2)",
 *  "VMESBCA2/PG1" (vmesbcp2.pdf) for more details.
 *
 *  Note: This is somewhat MVME187-specific, at the moment.
 *
 *
 *  TODO: Lots of stuff.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#include "mvme187.h"
#include "mvme_pcctworeg.h"

#define	INTERRUPT_LEVEL_MASK	0x07


/*  #define	debug fatal  */

struct pcc2_data {
	struct interrupt	cpu_irq;

	uint8_t			pcctwo_reg[PCC2_SIZE];

	uint8_t			cur_int_vec;
};


static void reassert_interrupts(struct pcc2_data *d)
{
	/*  Block interrupts at the mask level or lower:  */
	if ((d->pcctwo_reg[PCCTWO_IPL] & INTERRUPT_LEVEL_MASK) <=
	    (d->pcctwo_reg[PCCTWO_MASK] & INTERRUPT_LEVEL_MASK))
		INTERRUPT_DEASSERT(d->cpu_irq);
	else
		INTERRUPT_ASSERT(d->cpu_irq);
}


DEVICE_ACCESS(pcc2)
{
	uint64_t idata = 0, odata = 0;
	struct pcc2_data *d = extra;

	/*  0xfff42000..0xfff42fff, but only 0x40 unique registers:  */
	relative_addr %= PCC2_SIZE;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (writeflag == MEM_READ)
		odata = d->pcctwo_reg[relative_addr];

	switch (relative_addr) {

	case PCCTWO_CHIPID:
	case PCCTWO_CHIPREV:
		if (writeflag == MEM_WRITE) {
			fatal("TODO: write to PCCTWO_CHIPID or CHIPREV?\n");
			exit(1);
		}
		break;

	case PCCTWO_VECBASE:
		if (writeflag == MEM_WRITE) {
			d->pcctwo_reg[PCCTWO_VECBASE] = idata & 0xf0;
			if (idata & ~0xf0)
				fatal("[ pcc2: HUH? write to PCCTWO_VECBASE"
				    " with value 0x%02x. ]\n", (int) idata);
		}
		break;

	case PCCTWO_IPL:
		if (writeflag == MEM_WRITE) {
			fatal("[ pcc2: HUH? Write attempt to PCCTWO_IPL. ]\n");
		}
		break;

	case PCCTWO_MASK:
		if (writeflag == MEM_WRITE) {
			d->pcctwo_reg[relative_addr] = idata;
			reassert_interrupts(d);
		}
		break;

	default:
		debug("[ pcc2: unimplemented %s offset 0x%x",
		    writeflag == MEM_WRITE? "write to" : "read from",
		    (int) relative_addr);
		if (writeflag == MEM_WRITE)
			debug(": 0x%x", (int)idata);
		debug(" ]\n");
		/*  exit(1);  */
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(mvme187_iack)
{
	uint64_t odata = 0;
	struct pcc2_data *d = extra;

	if (writeflag == MEM_WRITE) {
		fatal("[ pcc2: write to mvme187_iack? ]\n");
	} else {
		odata = d->cur_int_vec;
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVINIT(pcc2)
{
	struct pcc2_data *d;

	CHECK_ALLOCATION(d = malloc(sizeof(struct pcc2_data)));
	memset(d, 0, sizeof(struct pcc2_data));

	/*  Initial values, according to the manual:  */
	d->pcctwo_reg[PCCTWO_CHIPID] = PCC2_ID;
	d->pcctwo_reg[PCCTWO_CHIPREV] = 0x00;
	d->pcctwo_reg[PCCTWO_VECBASE] = 0x0f;

	/*  Connect to the CPU's interrupt pin:  */
	INTERRUPT_CONNECT(devinit->interrupt_path, d->cpu_irq);

	memory_device_register(devinit->machine->memory, "pcc2",
	    devinit->addr, 4096, dev_pcc2_access, (void *)d,
	    DM_DEFAULT, NULL);

	memory_device_register(devinit->machine->memory, "mvme187_iack",
	    M187_IACK, 32, dev_mvme187_iack_access, (void *)d,
	    DM_DEFAULT, NULL);

	return 1;
}

