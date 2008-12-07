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
 *  COMMENT: Dreamcast G2 bus
 *
 *  Register offsets are from KOS, NetBSD sources, etc.
 *
 *  TODO:
 *	Figure out what all these registers do!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#ifdef UNSTABLE_DEVEL
#define debug fatal
#endif

#define	NREGS		(0x100/sizeof(uint32_t))

struct dreamcast_g2_data {
	uint32_t	dma_reg[NREGS];
	uint32_t	extdma_reg[NREGS];
	uint32_t	unknown_reg[NREGS];
};

/*  dma_reg[] offsets:  */
#define	PVR_STATE		0x00
#define	PVR_LEN			0x04
#define	PVR_DST			0x08
#define	PVR_LMMODE0		0x84
#define	PVR_LMMODE1		0x88

/*
 *  External DMA:  4 channels
 *
 *  Note: Addresses and sizes must be 32-byte aligned.
 *  DIR is 0 for CPU to External device, 1 for External to CPU.
 *  MODE should be 5 for transfers to/from the SPU.
 */
#define	EXTDMA_CTRL_EXT_ADDR	0x00	/*  EXTDMA_CTRL_* are repeated  */
#define	EXTDMA_CTRL_SH4_ADDR	0x04	/*  4 times (once for each channel)  */
#define	EXTDMA_CTRL_SIZE	0x08
#define	EXTDMA_CTRL_DIR		0x0c
#define	EXTDMA_CTRL_MODE	0x10
#define	EXTDMA_CTRL_CTRL1	0x14
#define	EXTDMA_CTRL_CTRL2	0x18
#define	EXTDMA_CTRL_UNKNOWN	0x1c

#define	EXTDMA_WAITSTATE	0x90
#define	EXTDMA_MAGIC		0xbc
#define	EXTDMA_MAGIC_VALUE	    0x4659404f

#define	EXTDMA_STAT_EXT_ADDR	0xc0	/*  EXTDMA_STAT_* are repeated 4  */
#define	EXTDMA_STAT_SH4_ADDR	0xc4	/*  times too  */
#define	EXTDMA_STAT_SIZE	0xc8
#define	EXTDMA_STAT_STATUS	0xcc


DEVICE_ACCESS(dreamcast_g2)
{
	struct dreamcast_g2_data *d = extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Default read:  */
	if (writeflag == MEM_READ)
		odata = d->dma_reg[relative_addr / sizeof(uint32_t)];

	switch (relative_addr) {

	case PVR_LMMODE0:	/*  0x84  */
	case PVR_LMMODE1:	/*  0x88  */
		if (writeflag == MEM_WRITE) {
			if (idata == 0) {
				/*  Done by IP.BIN during startup...  */
			} else {
				fatal("[ dreamcast_g2: UNIMPLEMENTED write "
				    "0x84/0x88: TODO ]\n");
				exit(1);
			}
		} else {
			fatal("[ dreamcast_g2: read from 0x84/0x88: TODO ]\n");
			exit(1);
		}
		break;

	case 0x8c:
		if (writeflag == MEM_WRITE) {
			fatal("[ dreamcast_g2: write to 0x8c: TODO ]\n");
			exit(1);
		} else {
			/*  0x20 means G2 DMA in progress?  */
			/*  0x11 = mask which has to do with AICA  */
			odata = 0x11 * (random() & 1);
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ dreamcast_g2: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ dreamcast_g2: write to addr 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)idata);
		}
/*		exit(1); */
	}

	/*  Default write:  */
	if (writeflag == MEM_WRITE)
		d->dma_reg[relative_addr / sizeof(uint32_t)] = idata;

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(dreamcast_g2_extdma)
{
	struct dreamcast_g2_data *d = extra;
	uint64_t idata = 0, odata = 0;
	int reg = relative_addr, channel = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Default read:  */
	if (writeflag == MEM_READ)
		odata = d->extdma_reg[relative_addr / sizeof(uint32_t)];

	if (reg < 0x7f) {
		channel = (reg >> 5) & 3;
		reg &= 0x1f;
	}

	if (reg >= 0xc0 && reg < 0xff) {
		channel = (reg >> 4) & 3;
		reg = 0xc0 + (reg & 0xf);
	}

	switch (reg) {

	case EXTDMA_WAITSTATE:
		break;

	case 0x94:
		/*  Written to by boot stage 1 in IP.BIN?  */
		if (writeflag == MEM_WRITE) {
			if (idata != 0x271) {
				fatal("Unimplemented write to extdma 0x94\n");
				exit(1);
			}
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ dreamcast_g2_extdma: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ dreamcast_g2_extdma: write to addr 0x%x: "
			    "0x%x ]\n", (int)relative_addr, (int)idata);
		}
/*		exit(1); */
	}

	/*  Default write:  */
	if (writeflag == MEM_WRITE)
		d->extdma_reg[relative_addr / sizeof(uint32_t)] = idata;

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(dreamcast_g2_unknown)
{
	struct dreamcast_g2_data *d = extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Default read:  */
	if (writeflag == MEM_READ)
		odata = d->unknown_reg[relative_addr / sizeof(uint32_t)];

	switch (relative_addr) {

	case 0x80:
		if (writeflag != MEM_WRITE || idata != 0x400) {
			fatal("[ dreamcast_g2_unknown: unimplemented 0x80 ]\n");
			exit(1);
		}
		break;

	case 0x90:
	case 0x94:
/*		if (writeflag != MEM_WRITE || idata != 0x222) {
			fatal("[ dreamcast_g2_unknown: unimplemented 0x90 ]\n");
			exit(1);
		}
*/		break;

	case 0xa0:
	case 0xa4:
		if (writeflag != MEM_WRITE || idata != 0x2001) {
			fatal("[ dreamcast_g2_unknown: unimplemented 0xa0 ]\n");
			exit(1);
		}
		break;

	case 0xe4:
		/*  Writing 0x1fffff resets a disabled GD-ROM drive?  */
		if (writeflag != MEM_WRITE || idata != 0x1fffff) {
			fatal("[ dreamcast_g2_unknown: unimplemented 0xe4 ]\n");
			exit(1);
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ dreamcast_g2_unknown: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ dreamcast_g2_unknown: write to addr 0x%x: "
			    "0x%x ]\n", (int)relative_addr, (int)idata);
		}
/*		exit(1); */
	}

	/*  Default write:  */
	if (writeflag == MEM_WRITE)
		d->unknown_reg[relative_addr / sizeof(uint32_t)] = idata;

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(dreamcast_g2)
{
	struct machine *machine = devinit->machine;
	struct dreamcast_g2_data *d;

	CHECK_ALLOCATION(d = malloc(sizeof(struct dreamcast_g2_data)));
	memset(d, 0, sizeof(struct dreamcast_g2_data));

	memory_device_register(machine->memory, devinit->name,
	    0x005f6800, 0x100, dev_dreamcast_g2_access, d, DM_DEFAULT, NULL);

	memory_device_register(machine->memory, devinit->name, 0x005f7400,
	    0x100, dev_dreamcast_g2_unknown_access, d, DM_DEFAULT, NULL);

	memory_device_register(machine->memory, devinit->name, 0x005f7800,
	    0x100, dev_dreamcast_g2_extdma_access, d, DM_DEFAULT, NULL);

	return 1;
}

