/*
 *  Copyright (C) 2008  Anders Gavare.  All rights reserved.
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
 *  COMMENT: NCR 53C710 SCSI I/O Processor (SIOP)
 *
 *  Based on reverse-engineering OpenBSD's osiop.c, and a PDF manual:
 *  "Symbios SYM53C710 SCSI I/O Processor Technical Manual, version 3.1".
 *
 *  TODO: Many things. Especially the SCRIPTS instruction set and DMA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#include "osiopreg.h"

#define	DEV_OSIOP_LENGTH	OSIOP_NREGS
#define	OSIOP_CHIP_REVISION	2

#define	OSIOP_TICK_SHIFT	16

struct osiop_data {
	struct interrupt	irq;
	int			asserted;

	uint8_t			reg[OSIOP_NREGS];
};


/*
 *  osiop_update_sip_and_dip():
 *
 *  The SIP bit in ISTAT is basically a "summary" of whether there is
 *  a non-masked SCSI interrupt. Similarly for DIP, for DMA.
 */
void osiop_update_sip_and_dip(struct osiop_data *d)
{
	d->reg[OSIOP_ISTAT] &= ~(OSIOP_ISTAT_SIP | OSIOP_ISTAT_DIP);

	/*  Any enabled SCSI interrupts?  */
	if (d->reg[OSIOP_SSTAT0] & d->reg[OSIOP_SIEN])
		d->reg[OSIOP_ISTAT] |= OSIOP_ISTAT_SIP;

	/*  Or enabled DMA interrupts?  */
	if (d->reg[OSIOP_DSTAT] & d->reg[OSIOP_DIEN] & ~OSIOP_DIEN_RES)
		d->reg[OSIOP_ISTAT] |= OSIOP_ISTAT_DIP;
}


/*
 *  osiop_reassert_interrupts():
 *
 *  Recalculate interrupt assertions; if either of the SIP or DIP bits
 *  in the ISTAT register is set, then we should cause an interrupt.
 *
 *  (The d->asserted stuff is to make sure we don't call
 *  INTERRUPT_DEASSERT etc. too often when not necessary.)
 */
void osiop_reassert_interrupts(struct osiop_data *d)
{
	int assert = 0;

	osiop_update_sip_and_dip(d);

	if (d->reg[OSIOP_ISTAT] & (OSIOP_ISTAT_SIP | OSIOP_ISTAT_DIP))
		assert = 1;

	if (assert && !d->asserted)
		INTERRUPT_ASSERT(d->irq);
	if (!assert && d->asserted)
		INTERRUPT_DEASSERT(d->irq);

	d->asserted = assert;
}


DEVICE_TICK(osiop)
{
	struct osiop_data *d = extra;

	osiop_reassert_interrupts(d);
}


DEVICE_ACCESS(osiop)
{
	uint64_t idata = 0, odata = 0;
	uint8_t oldreg = 0;
	struct osiop_data *d = extra;
	int origofs = relative_addr;
	int non1lenOk = 0;

	/*  Make relative_addr suit addresses in osiopreg.h:  */
	if (cpu->byte_order == EMUL_BIG_ENDIAN) {
		relative_addr =
		    (relative_addr & ~3) |
		    (3 - (relative_addr & 3));
	}

	if (len == sizeof(uint32_t)) {
		/*
		 *  NOTE: These are stored in HOST byte order!
		 */
		switch (origofs) {
		case OSIOP_DSA:
		case OSIOP_TEMP:
		case OSIOP_DBC:
		case OSIOP_DNAD:
		case OSIOP_DSP:
		case OSIOP_DSPS:
		case OSIOP_SCRATCH:
		case OSIOP_ADDER:
			relative_addr = origofs;
			non1lenOk = 1;
			if (writeflag == MEM_WRITE) {
				*(uint32_t*) &d->reg[origofs] = idata;
			} else {
				odata = *(uint32_t*) &d->reg[origofs];
			}
			break;
		}
	} else {
		/*  Byte access:  */
		oldreg = d->reg[relative_addr];
		if (writeflag == MEM_WRITE) {
			idata = memory_readmax64(cpu, data, len);
			d->reg[relative_addr] = idata;
		} else {
			odata = oldreg;
		}
	}

	switch (relative_addr) {

	case OSIOP_SCNTL0:
	case OSIOP_SCNTL1:
		break;

	case OSIOP_SIEN:
		/*  Used by OpenBSD/mvme88k during probing:  */
		if (len == 4)
			non1lenOk = 1;
		if (writeflag == MEM_WRITE)
			osiop_reassert_interrupts(d);
		break;

	case OSIOP_SCID:
		if (idata != oldreg) {
			fatal("osiop TODO: attempt to change SCID?\n");
			exit(1);
		}
		break;

	case OSIOP_SBCL:
		break;

	case OSIOP_DSTAT:
		/*  Cleared when read. Most likely not writable.  */
		odata = d->reg[OSIOP_DSTAT];
		d->reg[OSIOP_DSTAT] = OSIOP_DSTAT_DFE;
		osiop_reassert_interrupts(d);
		break;

	case OSIOP_SSTAT0:
		/*  Cleared when read. Most likely not writable.  */
		odata = d->reg[OSIOP_SSTAT0];
		d->reg[OSIOP_SSTAT0] = 0;
		osiop_reassert_interrupts(d);
		break;

	case OSIOP_DSA:
		break;

	case OSIOP_CTEST0:
	case OSIOP_CTEST1:
	case OSIOP_CTEST2:
	case OSIOP_CTEST3:
	case OSIOP_CTEST4:
	case OSIOP_CTEST5:
	case OSIOP_CTEST6:
	case OSIOP_CTEST7:
		break;

	case OSIOP_TEMP:
		break;

	case OSIOP_ISTAT:
		if (writeflag == MEM_WRITE) {
			if ((idata & 0x3f) != 0x00) {
				fatal("osiop TODO: istat 0x%x\n", (int) idata);
				exit(1);
			}

			d->reg[relative_addr] = idata &
			    ~(OSIOP_ISTAT_ABRT | OSIOP_ISTAT_RST);
			osiop_reassert_interrupts(d);
		}
		break;

	case OSIOP_CTEST8:
		odata = (odata & 0xf) | (OSIOP_CHIP_REVISION << 4);
		break;

	case OSIOP_DSP:
		break;

	case OSIOP_DMODE:
		break;

	case OSIOP_DIEN:
		if (writeflag == MEM_WRITE)
			osiop_reassert_interrupts(d);
		break;

	case OSIOP_DWT:
		break;

	case OSIOP_DCNTL:
		if (writeflag == MEM_WRITE) {
			if (idata & OSIOP_DCNTL_SSM) {
				fatal("osiop TODO: SSM\n");
				exit(1);
			}
			if (idata & OSIOP_DCNTL_LLM) {
				fatal("osiop TODO: LLM\n");
				exit(1);
			}
			if (idata & OSIOP_DCNTL_STD) {
				fatal("osiop TODO: STD\n");
				exit(1);
			}
		}
		break;

	default:
		if (writeflag == MEM_READ) {
			fatal("[ osiop: read from 0x%02lx ]\n",
			    (long)relative_addr);
		} else {
			fatal("[ osiop: write to  0x%02lx: 0x%02x ]\n",
			    (long)relative_addr, (int)idata);
		}

		exit(1);
	}

	if (len != 1 && !non1lenOk) {
		fatal("[ osiop: TODO: len != 1, addr 0x%0x ]\n",
		    (int)relative_addr);
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(osiop)
{
	struct osiop_data *d;

	CHECK_ALLOCATION(d = malloc(sizeof(struct osiop_data)));
	memset(d, 0, sizeof(struct osiop_data));

	d->reg[OSIOP_SCID] = OSIOP_SCID_VALUE(7);

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	memory_device_register(devinit->machine->memory, "osiop",
	    devinit->addr, DEV_OSIOP_LENGTH,
	    dev_osiop_access, d, DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine,
	    dev_osiop_tick, d, OSIOP_TICK_SHIFT);

	return 1;
}

