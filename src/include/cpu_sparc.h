#ifndef	CPU_SPARC_H
#define	CPU_SPARC_H

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
 *  SPARC CPU definitions.
 */

#include "misc.h"


struct cpu_family;


/*  SPARC CPU types:  */
struct sparc_cpu_type_def { 
	const char	*name;
	int		v;			/*  v8, v9 etc  */
	int		h;			/*  hypervisor? sun4v = 1  */
	int		bits;			/*  32 or 64  */
	int		nwindows;		/*  usually 8 or more  */
	int		icache_shift;
	int		ilinesize;
	int		iway;
	int		dcache_shift;
	int		dlinesize;
	int		dway;
	int		l2cache_shift;
	int		l2linesize;
	int		l2way;
};

/*  NOTE/TODO: Maybe some of the types listed below as v8 are in
    fact v7; I haven't had time to check. Also, the nwindows value is
    just bogus.  */
/*  See http://www.sparc.com/standards/v8v9-numbers.html for
    implementation numbers!  */
/*  Note/TODO: sun4v is listed as 10  */

#define SPARC_CPU_TYPE_DEFS	{					\
	{ "TMS390Z50",		8, 0, 32, 8, 14,5,2, 14,5,2,  0,0,0 },	\
	{ "MB86904",		8, 0, 32, 8, 14,5,2, 13,4,2,  0,0,0 },	\
	{ "MB86907",		8, 0, 32, 8, 14,5,2, 14,5,2, 19,5,1 },	\
	{ "UltraSPARC",		9, 0, 64, 8, 14,5,4, 14,5,4, 19,6,1 },	\
	{ "UltraSPARC-IIi",	9, 0, 64, 8, 15,5,2, 14,5,2, 21,6,1 },	\
	{ "UltraSPARC-II",	9, 0, 64, 8, 15,5,2, 14,5,2, 22,6,1 },	\
	{ "T1",			9, 1, 64, 8, 15,5,2, 14,5,2, 22,6,1 },	\
	{ NULL,			0, 0,  0, 0,  0,0,0,  0,0,0,  0,0,0 }	\
	}


#define	SPARC_N_IC_ARGS			3
#define	SPARC_INSTR_ALIGNMENT_SHIFT	2
#define	SPARC_IC_ENTRIES_SHIFT		10
#define	SPARC_IC_ENTRIES_PER_PAGE	(1 << SPARC_IC_ENTRIES_SHIFT)
#define	SPARC_PC_TO_IC_ENTRY(a)		(((a)>>SPARC_INSTR_ALIGNMENT_SHIFT) \
					& (SPARC_IC_ENTRIES_PER_PAGE-1))
#define	SPARC_ADDR_TO_PAGENR(a)		((a) >> (SPARC_IC_ENTRIES_SHIFT \
					+ SPARC_INSTR_ALIGNMENT_SHIFT))

#define	SPARC_L2N		17
#define	SPARC_L3N		18	/*  4KB pages on 32-bit sparc,  */
					/*  8KB pages on 64-bit?  TODO  */

DYNTRANS_MISC_DECLARATIONS(sparc,SPARC,uint64_t)
DYNTRANS_MISC64_DECLARATIONS(sparc,SPARC,uint8_t)

#define	SPARC_MAX_VPH_TLB_ENTRIES		128


#define	N_SPARC_REG		32
#define	N_SPARC_GLOBAL_REG	8
#define	N_SPARC_INOUT_REG	8
#define	N_SPARC_LOCAL_REG	8
#define	SPARC_REG_NAMES	{				\
	"g0","g1","g2","g3","g4","g5","g6","g7",	\
	"o0","o1","o2","o3","o4","o5","sp","o7",	\
	"l0","l1","l2","l3","l4","l5","l6","l7",	\
	"i0","i1","i2","i3","i4","i5","fp","i7" }

#define	SPARC_ZEROREG		0	/*  g0  */
#define	SPARC_REG_G0		0
#define	SPARC_REG_G1		1
#define	SPARC_REG_G2		2
#define	SPARC_REG_G3		3
#define	SPARC_REG_G4		4
#define	SPARC_REG_G5		5
#define	SPARC_REG_G6		6
#define	SPARC_REG_G7		7
#define	SPARC_REG_O0		8
#define	SPARC_REG_O1		9
#define	SPARC_REG_O2		10
#define	SPARC_REG_O3		11
#define	SPARC_REG_O4		12
#define	SPARC_REG_O5		13
#define	SPARC_REG_O6		14
#define	SPARC_REG_O7		15
#define	SPARC_REG_L0		16
#define	SPARC_REG_L1		17
#define	SPARC_REG_L2		18
#define	SPARC_REG_L3		19
#define	SPARC_REG_L4		20
#define	SPARC_REG_L5		21
#define	SPARC_REG_L6		22
#define	SPARC_REG_L7		23
#define	SPARC_REG_I0		24
#define	SPARC_REG_I1		25
#define	SPARC_REG_I2		26
#define	SPARC_REG_I3		27
#define	SPARC_REG_I4		28
#define	SPARC_REG_I5		29
#define	SPARC_REG_I6		30
#define	SPARC_REG_I7		31

/*  Privileged registers:  */
#define	N_SPARC_PREG		32
#define	SPARC_PREG_NAMES	{					\
	"tpc", "tnpc", "tstate", "tt", "tick", "tba", "pstate", "tl",	\
	"pil", "cwp", "cansave", "canrestore", "cleanwin", "otherwin",	\
	"wstate", "fq", "reserved16", "reserved17", "reserved18", \
	"reserved19", "reserved20", "reserved21", "reserved22", \
	"reserved23", "reserved24", "reserved25", "reserved26", \
	"reserved27", "reserved28", "reserved29", "reserved30", \
	"ver" }

#define	N_SPARC_BRANCH_TYPES	16
#define	SPARC_BRANCH_NAMES {						\
	"bn", "be",  "ble", "bl",  "bleu", "bcs", "bneg", "bvs",	\
	"ba", "bne", "bg",  "bge", "bgu",  "bcc", "bpos", "bvc"  }

#define	N_SPARC_REGBRANCH_TYPES	8
#define	SPARC_REGBRANCH_NAMES {						\
	"br?","brz","brlez","brlz","br??","brnz", "brgz", "brgez"  }

#define	N_ALU_INSTR_TYPES	64
#define	SPARC_ALU_NAMES {						\
	"add", "and", "or", "xor", "sub", "andn", "orn", "xnor",	\
	"addx", "[9]", "umul", "smul", "subx", "[13]", "udiv", "sdiv",	\
	"addcc","andcc","orcc","xorcc","subcc","andncc","orncc","xnorcc",\
	"addxcc","[25]","umulcc","smulcc","subxcc","[29]","udivcc","sdivcc",\
	"taddcc","tsubcc","taddcctv","tsubcctv","mulscc","sll","srl","sra",\
	"rd" /* membar/stbar on sparcv9 */,				\
	"rd" /* rd psr on pre-sparcv9 */, "rdpr","rd",	 		\
	"[44]","[45]","popc","movre",	 \
	"wr*","saved/restored","wrpr","[51]", "[52]","[53]","[54]","[55]",\
	"jmpl", "rett", "trap", "flush", "save", "restore", "[62]","[63]" }

#define	N_LOADSTORE_TYPES	64
#define	SPARC_LOADSTORE_NAMES {						\
	"lduw","ldub","lduh","ldd", "st","stb","sth","std",		\
	"ldsw","ldsb","ldsh","ldx", "[12]","ldstub","stx","swap",	\
	"lda","lduba","lduha","ldda", "sta","stba","stha","stda",	\
	"[24]","ldsba","ldsha","ldxa", "[28]","ldstuba","stxa","swapa",	 \
	"ldf","ldfsr","[34]","lddf", "stf","stfsr","stdfq","stdf",	\
	"[40]","[41]","[42]","[43]", "[44]","prefetch","[46]","[47]",	\
	"ldc","ldcsr","[50]","lddc", "stc","stcsr","scdfq","scdf",	\
	"[56]","[57]","[58]","[59]", "[60]","prefetcha","casxa","[63]" }


/*  Max number of Trap Levels, Global Levels, and Register Windows:  */
#define	MAXTL			6
#define	MAXGL			7
#define	N_REG_WINDOWS		8


struct sparc_cpu {
	struct sparc_cpu_type_def cpu_type;

	/*  Registers in the Current Window:  */
	uint64_t	r[N_SPARC_REG];

	uint64_t	r_inout[N_REG_WINDOWS][N_SPARC_INOUT_REG];
	uint64_t	r_local[N_REG_WINDOWS][N_SPARC_LOCAL_REG];

	uint64_t	r_global[MAXGL+1][N_SPARC_GLOBAL_REG];

	uint64_t	scratch;

	/*  Pre-SPARCv9 specific:  */
	uint32_t	psr;		/*  Processor State Register  */
	uint32_t	tbr;		/*  Trap base register  */
	uint32_t	wim;		/*  Window invalid mask  */

	/*  SPARCv9 etc.:  */
	uint64_t	pstate;		/*  Processor State Register  */
	uint64_t	y;		/*  Y-reg (only low 32-bits used)  */
	uint64_t	fprs;		/*  Floating Point Register Status  */
	uint64_t	tick;		/*  Tick Register  */
	uint64_t	tick_cmpr;	/*  Tick Compare Register (?)  */
	uint64_t	ver;		/*  Version register  */

	uint8_t		cwp;		/*  Current Window Pointer  */
	uint8_t		cansave;	/*  CANSAVE register  */
	uint8_t		canrestore;	/*  CANRESTORE register  */
	uint8_t		otherwin;	/*  OTHERWIN register  */
	uint8_t		cleanwin;	/*  CLEANWIN register  */

	uint8_t		wstate;		/*  Window state  */

	uint8_t		ccr;		/*  Condition Code Register  */
	uint8_t		asi;		/*  Address Space Identifier  */
	uint8_t		tl;		/*  Trap Level Register  */
	uint8_t		gl;		/*  Global Level Register  */
	uint8_t		pil;		/*  Processor Interrupt Level Reg.  */

	uint64_t	tpc[MAXTL];	/*  Trap Program Counter  */
	uint64_t	tnpc[MAXTL];	/*  Trap Next Program Counter  */
	uint64_t	tstate[MAXTL];	/*  Trap State  */
	uint32_t	ttype[MAXTL];	/*  Trap Type  */

	uint64_t	tba;		/*  Trap Base Address  */

	uint64_t	hpstate;	/*  Hyper-Privileged State Register  */
	uint64_t	htstate[MAXTL];	/*  Hyper-Privileged Trap State  */
	uint64_t	hintp;		/*  Hyper-Privileged InterruptPending */
	uint64_t	htba;		/*  Hyper-Privileged Trap Base Addr  */
	uint64_t	hver;		/*  Hyper-Privileged Version Reg.  */


	/*
	 *  Instruction translation cache and Virtual->Physical->Host
	 *  address translation:
	 */
	DYNTRANS_ITC(sparc)
	VPH_TLBS(sparc,SPARC)
	VPH32(sparc,SPARC)
	VPH64(sparc,SPARC)
};


/*  Processor State Register (PSTATE) bit definitions:  */
#define	SPARC_PSTATE_PID1	0x800
#define	SPARC_PSTATE_PID0	0x400
#define	SPARC_PSTATE_CLE	0x200	/*  Current Little Endian  */
#define	SPARC_PSTATE_TLE	0x100	/*  Trap Little Endian  */
#define	SPARC_PSTATE_MM_MASK	0x0c0	/*  Memory Model (TODO)  */
#define	SPARC_PSTATE_MM_SHIFT	    6
#define	SPARC_PSTATE_RED	0x020	/*  Reset/Error/Debug state  */
#define	SPARC_PSTATE_PEF	0x010	/*  Enable Floating-point  */
#define	SPARC_PSTATE_AM		0x008	/*  Address Mask  */
#define	SPARC_PSTATE_PRIV	0x004	/*  Privileged Mode  */
#define	SPARC_PSTATE_IE		0x002	/*  Interrupt Enable  */
#define	SPARC_PSTATE_AG		0x001	/*  Alternate Globals  */


/*  Hyper-Privileged State Register (HPSTATE) bit definitions:  */
#define	SPARC_HPSTATE_ID	0x800
#define	SPARC_HPSTATE_IBE	0x400	/*  Instruction Break Enable  */
#define	SPARC_HPSTATE_RED	0x020	/*  Reset/Error/Debug state  */
#define	SPARC_HPSTATE_HPRIV	0x004	/*  Hyper-Privileged mode  */
#define	SPARC_HPSTATE_TLZ	0x001	/*  Trap Level Zero trap enable  */


/*  Condition Code Register bit definitions:  */
#define	SPARC_CCR_XCC_MASK	0xf0
#define	SPARC_CCR_XCC_SHIFT	4
#define	SPARC_CCR_ICC_MASK	0x0f
#define	SPARC_CCR_N		8
#define	SPARC_CCR_Z		4
#define	SPARC_CCR_V		2
#define	SPARC_CCR_C		1


/*  CWP, CANSAVE, CANRESTORE, OTHERWIN, CLEANWIN bitmask:  */
#define	SPARC_CWP_MASK		0x1f


/*  Window State bit definitions:  */
#define	SPARC_WSTATE_OTHER_MASK		0x38
#define	SPARC_WSTATE_OTHER_SHIFT	3
#define	SPARC_WSTATE_NORMAL_MASK	0x07


/*  Tick Register bit definitions:  */
#define	SPARC_TICK_NPT		(1ULL << 63)	/*  Non-privileged trap  */


/*  Addess Space Identifier bit definitions:  */
#define	SPARC_ASI_RESTRICTED	0x80


/*  Trap Level Register bit definitions:  */
#define	SPARC_TL_MASK		0x07


/*  Processor Interrupt Level Register bit definitions:  */
#define	SPARC_PIL_MASK		0x0f


/*  Trap Type Register bit definitions:  */
#define	SPARC_TTYPE_MASK	0x1ff


/*  Trap Base Address bit definitions:  */
#define	SPARC_TBA_MASK		0xffffffffffff8000ULL


/*
 *  Full address for a trap is:
 *	TBA<bits 63..15> || X || TTYPE[TL] || 00000
 *
 *  where X is a bit which is true if TL>0 when the trap was taken.
 */


/*  Version Register bit definitions:  */
#define	SPARC_VER_MANUF_SHIFT	48
#define	SPARC_VER_IMPL_SHIFT	32
#define	SPARC_VER_MASK_SHIFT	24
#define	SPARC_VER_MAXTL_SHIFT	8
#define	SPARC_VER_MAXWIN_SHIFT	0


/*  cpu_sparc.c:  */
int sparc_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib);
int sparc_run_instr(struct cpu *cpu);
void sparc_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void sparc_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void sparc_invalidate_code_translation(struct cpu *cpu, uint64_t, int);
int sparc32_run_instr(struct cpu *cpu);
void sparc32_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void sparc32_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void sparc32_invalidate_code_translation(struct cpu *cpu, uint64_t, int);
void sparc_init_64bit_dummy_tables(struct cpu *cpu);
int sparc_memory_rw(struct cpu *cpu, struct memory *mem, uint64_t vaddr,
	unsigned char *data, size_t len, int writeflag, int cache_flags);
int sparc_cpu_family_init(struct cpu_family *);

/*  memory_sparc.c:  */
int sparc_translate_v2p(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_addr, int flags);


#endif	/*  CPU_SPARC_H  */
