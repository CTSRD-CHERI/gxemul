#ifndef M88K_CPUCOMPONENT_H
#define	M88K_CPUCOMPONENT_H

/*
 *  Copyright (C) 2009  Anders Gavare.  All rights reserved.
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
 */

// COMPONENT(m88k_cpu)


#include "CPUComponent.h"

#include "thirdparty/m88k_psl.h"


/*  M88K CPU types:  */
struct m88k_cpu_type_def {
	const char	*name;
	int		type;
	uint32_t	pid;
};

#define M88K_PID(arn,vn) ((arn << M88K_ARN_SHIFT) | (vn << M88K_VN_SHIFT))

#define	M88K_CPU_TYPE_DEFS				{	\
	{ "88100", 88100, M88K_PID(M88K_ARN_88100,3) },		\
	{ "88110", 88110, M88K_PID(M88K_ARN_88110,0) },		\
	{ NULL,        0, 0			     }		\
	}

/*
 * Opcode names:
 */
#define	M88K_OPCODE_NAMES {						\
	"xmem.bu",  "xmem",     "ld.hu",    "ld.bu",			\
	"ld.d",     "ld",       "ld.h",     "ld.b",			\
	"st.d",     "st",       "st.h",     "st.b",			\
	"opcode0c", "opcode0d", "opcode0e", "opcode0f",			\
	"and",      "and.u",    "mask",     "mask.u",			\
	"xor",      "xor.u",    "or",       "or.u",			\
	"addu",     "subu",     "divu",     "mulu",			\
	"add",      "sub",      "div",      "cmp",			\
	"opcode20", "opcode21", "opcode22", "opcode23",			\
	"opcode24", "opcode25", "opcode26", "opcode27",			\
	"opcode28", "opcode29", "opcode2a", "opcode2b",			\
	"opcode2c", "opcode2d", "opcode2e", "opcode2f",			\
	"opcode30", "opcode31", "opcode32", "opcode33",			\
	"opcode34", "opcode35", "opcode36", "opcode37",			\
	"opcode38", "opcode39", "opcode3a", "opcode3b",			\
	"opcode3c", "opcode3d", "tbnd",     "opcode3f" }

/*  Control register names:  */
#define	N_M88K_CONTROL_REGS	64
#define	M88K_CR_NAMES	{						\
	"PID",  "PSR",  "EPSR", "SSBR",		/*   0 ..  3  */	\
	"SXIP", "SNIP", "SFIP", "VBR",		/*   4 ..  7  */	\
	"DMT0", "DMD0", "DMA0", "DMT1",		/*   8 .. 11  */	\
	"DMD1", "DMA1", "DMT2", "DMD2",		/*  12 .. 15  */	\
	"DMA2", "SR0",  "SR1",  "SR2",		/*  16 .. 19  */	\
	"SR3",  "CR21", "CR22", "CR23",		/*  20 .. 23  */	\
	"CR24", "CR25", "CR26", "CR27",		/*  24 .. 27  */	\
	"CR28", "CR29", "CR30", "CR31",		/*  28 .. 31  */	\
	"CR32", "CR33", "CR34", "CR35",		/*  32 .. 35  */	\
	"CR36", "CR37", "CR38", "CR39",		/*  36 .. 39  */	\
	"CR40", "CR41", "CR42", "CR43",		/*  40 .. 43  */	\
	"CR44", "CR45", "CR46", "CR47",		/*  44 .. 47  */	\
	"CR48", "CR49", "CR50", "CR51",		/*  48 .. 51  */	\
	"CR52", "CR53", "CR54", "CR55",		/*  52 .. 55  */	\
	"CR56", "CR57", "CR58", "CR59",		/*  56 .. 59  */	\
	"CR60", "CR61", "CR62", "CR63"		/*  60 .. 63  */	}

#define M88K_CR_PID	0
#define M88K_CR_PSR     1 
#define M88K_CR_EPSR    2 
#define M88K_CR_SSBR    3 
#define M88K_CR_SXIP    4 
#define M88K_CR_SNIP    5 
#define M88K_CR_SFIP    6 
#define M88K_CR_VBR     7 
#define M88K_CR_DMT0    8 
#define M88K_CR_DMD0    9 
#define M88K_CR_DMA0    10
#define M88K_CR_DMT1    11
#define M88K_CR_DMD1    12
#define M88K_CR_DMA1    13
#define M88K_CR_DMT2    14
#define M88K_CR_DMD2    15
#define M88K_CR_DMA2    16
#define M88K_CR_SR0     17
#define M88K_CR_SR1     18
#define M88K_CR_SR2     19
#define M88K_CR_SR3     20

/*  MVME197 extended control registers:  */
#define	M88K_CR_NAMES_197	{					\
	"PID",  "PSR",  "EPSR", "SSBR",		/*   0 ..  3  */	\
	"EXIP", "ENIP", "SFIP", "VBR",		/*   4 ..  7  */	\
	"DMT0", "DMD0", "DMA0", "DMT1",		/*   8 .. 11  */	\
	"DMD1", "DMA1", "DMT2", "DMD2",		/*  12 .. 15  */	\
	"SRX", "SR0",  "SR1",  "SR2",		/*  16 .. 19  */	\
	"SR3",  "CR21", "CR22", "CR23",		/*  20 .. 23  */	\
	"CR24", "ICMD", "ICTL", "ISAR",		/*  24 .. 27  */	\
	"ISAP", "IUAP", "IIR",  "IBP",		/*  28 .. 31  */	\
	"IPPU", "IPPL", "ISR",  "ILAR",		/*  32 .. 35  */	\
	"IPAR", "CR37", "CR38", "CR39",		/*  36 .. 39  */	\
	"DCMD", "DCTL", "DSAR", "DSAP",		/*  40 .. 43  */	\
	"DUAP", "DIR",  "DBP",  "DPPU",		/*  44 .. 47  */	\
	"DPPL", "DSR",  "DLAR", "DPAR",		/*  48 .. 51  */	\
	"CR52", "CR53", "CR54", "CR55",		/*  52 .. 55  */	\
	"CR56", "CR57", "CR58", "CR59",		/*  56 .. 59  */	\
	"CR60", "CR61", "CR62", "CR63"		/*  60 .. 63  */	}

#define M88K_CR_EXIP    4
#define M88K_CR_ENIP    5
#define M88K_CR_SRX     16
#define M88K_CR_ICMD    25
#define M88K_CR_ICTL    26
#define M88K_CR_ISAR    27
#define M88K_CR_ISAP    28
#define M88K_CR_IUAP    29
#define M88K_CR_IIR     30
#define M88K_CR_IBP     31
#define M88K_CR_IPPU    32
#define M88K_CR_IPPL    33
#define M88K_CR_ISR     34
#define M88K_CR_ILAR    35
#define M88K_CR_IPAR    36
#define M88K_CR_DCMD    40
#define M88K_CR_DCTL    41
#define M88K_CR_DSAR    42
#define M88K_CR_DSAP    43
#define M88K_CR_DUAP    44
#define M88K_CR_DIR     45
#define M88K_CR_DBP     46
#define M88K_CR_DPPU    47
#define M88K_CR_DPPL    48
#define M88K_CR_DSR     49
#define M88K_CR_DLAR    50
#define M88K_CR_DPAR    51


#define	N_M88K_FPU_CONTROL_REGS		64

#define M88K_FPCR_FPECR		0
#define	M88K_FPECR_FDVZ			(1 << 3)
#define	M88K_FPECR_FUNIMP		(1 << 6)
/*  ... TODO: more  */


#define	M88K_N_IC_ARGS			3
#define	M88K_INSTR_ALIGNMENT_SHIFT	2
#define	M88K_IC_ENTRIES_SHIFT		10
#define	M88K_IC_ENTRIES_PER_PAGE	(1 << M88K_IC_ENTRIES_SHIFT)
#define	M88K_PC_TO_IC_ENTRY(a)		(((a)>>M88K_INSTR_ALIGNMENT_SHIFT) \
					& (M88K_IC_ENTRIES_PER_PAGE-1))
#define	M88K_ADDR_TO_PAGENR(a)		((a) >> (M88K_IC_ENTRIES_SHIFT \
					+ M88K_INSTR_ALIGNMENT_SHIFT))

#define	M88K_MAX_VPH_TLB_ENTRIES		128


#define	N_M88K_REGS		32

/*  Register r0 is always zero, r1 is the return address on function calls.  */
#define	M88K_ZERO_REG		0
#define	M88K_RETURN_REG		1

#define	M88K_CMP_HS	0x00000800
#define	M88K_CMP_LO	0x00000400
#define	M88K_CMP_LS	0x00000200
#define	M88K_CMP_HI	0x00000100
#define	M88K_CMP_GE	0x00000080
#define	M88K_CMP_LT	0x00000040
#define	M88K_CMP_LE	0x00000020
#define	M88K_CMP_GT	0x00000010
#define	M88K_CMP_NE	0x00000008
#define	M88K_CMP_EQ	0x00000004

/*  Exception numbers:  */
#define	M88K_EXCEPTION_RESET				0
#define	M88K_EXCEPTION_INTERRUPT			1
#define	M88K_EXCEPTION_INSTRUCTION_ACCESS		2
#define	M88K_EXCEPTION_DATA_ACCESS			3
#define	M88K_EXCEPTION_MISALIGNED_ACCESS		4
#define	M88K_EXCEPTION_UNIMPLEMENTED_OPCODE		5
#define	M88K_EXCEPTION_PRIVILEGE_VIOLATION		6
#define	M88K_EXCEPTION_BOUNDS_CHECK_VIOLATION		7
#define	M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE		8
#define	M88K_EXCEPTION_INTEGER_OVERFLOW			9
#define	M88K_EXCEPTION_ERROR				10
#define	M88K_EXCEPTION_SFU1_PRECISE			114
#define	M88K_EXCEPTION_SFU1_IMPRECISE			115
#define	M88K_EXCEPTION_USER_TRAPS_START			128

/*  A reserved/unimplemented instruction, used for PROM calls:  */
#define	M88K_PROM_INSTR		0xf400fc92


/*
 *  M88200/88204 CMMU:
 */

#define	MAX_M8820X_CMMUS		8
#define	M8820X_LENGTH			0x1000
#define	N_M88200_BATC_REGS		10
#define	N_M88200_PATC_ENTRIES		56
#define	M8820X_PATC_SUPERVISOR_BIT	0x00000001

struct m8820x_cmmu {
	uint32_t	reg[M8820X_LENGTH / sizeof(uint32_t)];
	uint32_t	batc[N_M88200_BATC_REGS];
	uint32_t	patc_v_and_control[N_M88200_PATC_ENTRIES];
	uint32_t	patc_p_and_supervisorbit[N_M88200_PATC_ENTRIES];
	int		patc_update_index;
};



/***********************************************************************/

/**
 * \brief A Component representing a Motorola 88000 processor.
 */
class M88K_CPUComponent
	: public CPUComponent
{
public:
	/**
	 * \brief Constructs a M88K_CPUComponent.
	 */
	M88K_CPUComponent();

	/**
	 * \brief Creates a M88K_CPUComponent.
	 */
	static refcount_ptr<Component> Create();

	static string GetAttribute(const string& attributeName);

	virtual void ResetState();

	virtual int Execute(GXemul* gxemul, int nrOfCycles);

	virtual bool PreRunCheckForComponent(GXemul* gxemul);

	virtual size_t DisassembleInstruction(uint64_t vaddr, size_t maxlen,
		unsigned char *instruction, vector<string>& result);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual bool VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	    bool& writable);

	virtual void ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const;

private:
	/*
	 * State:
	 */
	string			m_m88k_type;	// E.g. "88100"

	// General-Purpose Registers:
	//
	// 32 (N_M88K_REGS) registers, plus one which is always zero. (This
	// is to support st.d with d = r31. ld.d with d=r31 is converted to
	// just ld.
	uint32_t		m_r[N_M88K_REGS+1];

	// Destination scratch register for non-nop instructions with d=r0.
	// (Not serialized.)
	uint32_t		m_zero_scratch;

	// Control Registers:
	uint32_t		m_cr[N_M88K_CONTROL_REGS];

	// Floating Point Control registers:
	uint32_t		m_fcr[N_M88K_FPU_CONTROL_REGS];

	/*
	 * Cached other state:
	 */
	m88k_cpu_type_def	m_type;	// based on m_m88k_type
};


#endif	// M88K_CPUCOMPONENT_H

