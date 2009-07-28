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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iomanip>

#include "GXemul.h"
#include "components/M88K_CPUComponent.h"

static const char* opcode_names[] = M88K_OPCODE_NAMES;
static const char* opcode_names_3c[] = M88K_3C_OPCODE_NAMES;
static const char* opcode_names_3d[] = M88K_3D_OPCODE_NAMES;
static m88k_cpu_type_def cpu_type_defs[] = M88K_CPU_TYPE_DEFS;

static const char *memop[4] = { ".d", "", ".h", ".b" };

static const char *m88k_cr_names[] = M88K_CR_NAMES;
//static const char *m88k_cr_197_names[] = M88K_CR_NAMES_197;

static const char *m88k_cr_name(int i)
{
	const char **cr_names = m88k_cr_names;

	// TODO: Is this really MVME197 specific? Or 88110?
	//if (cpu->machine->machine_subtype == MACHINE_MVME88K_197)
	//	cr_names = m88k_cr_197_names;

	return cr_names[i];
}


M88K_CPUComponent::M88K_CPUComponent()
	: CPUComponent("m88k_cpu", "Motorola 88000")
	, m_m88k_type("88100")
{
	m_frequency = 50e6;	// 50 MHz

	// Find (and cache) the cpu type in m_type:
	memset((void*) &m_type, 0, sizeof(m_type));
	for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
		if (m_m88k_type == cpu_type_defs[j].name) {
			m_type = cpu_type_defs[j];
			break;
		}
	}

	if (m_type.name == NULL) {
		std::cerr << "Internal error: Unimplemented M88K type?\n";
		throw std::exception();
	}

	ResetState();

	AddVariable("model", &m_m88k_type);

	for (size_t i=0; i<N_M88K_REGS; i++) {
		stringstream ss;
		ss << "r" << i;
		AddVariable(ss.str(), &m_r[i]);
	}

	for (size_t i=0; i<N_M88K_CONTROL_REGS; i++) {
		stringstream ss;
		ss << "cr" << i;
		AddVariable(ss.str(), &m_cr[i]);
	}

	for (size_t i=0; i<N_M88K_FPU_CONTROL_REGS; i++) {
		stringstream ss;
		ss << "fcr" << i;
		AddVariable(ss.str(), &m_fcr[i]);
	}
}


refcount_ptr<Component> M88K_CPUComponent::Create()
{
	return new M88K_CPUComponent();
}


void M88K_CPUComponent::ResetState()
{
	m_pageSize = 4096;

	// r0 .. r31 and the extra "r32/r0" zero register:
	for (size_t i=0; i<N_M88K_REGS+1; i++)
		m_r[i] = 0;

	for (size_t i=0; i<N_M88K_CONTROL_REGS; i++)
		m_cr[i] = 0;

	for (size_t i=0; i<N_M88K_FPU_CONTROL_REGS; i++)
		m_fcr[i] = 0;

	m_pc = 0;

	// Set the Processor ID:
	m_cr[M88K_CR_PID] = m_type.pid | M88K_PID_MC;

	// Start in supervisor mode, with interrupts disabled.
	m_cr[M88K_CR_PSR] = M88K_PSR_MODE | M88K_PSR_IND;
	if (!m_isBigEndian)
		m_cr[M88K_CR_PSR] |= M88K_PSR_BO;

	CPUComponent::ResetState();
}


bool M88K_CPUComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	if (m_r[0] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "the r0 register "
		    "must contain the value 0.\n");
		return false;
	}

	if (m_r[N_M88K_REGS] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "internal error: the "
		    "register following r31 must mimic the r0 register.\nIf"
		    " you encounter this message, please write a bug report!\n");
		return false;
	}

	return CPUComponent::PreRunCheckForComponent(gxemul);
}


void M88K_CPUComponent::ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const
{
	bool done = false;

	stringstream ss;
	ss.flags(std::ios::hex);

	if (arguments.size() == 0 ||
	    find(arguments.begin(), arguments.end(), "r") != arguments.end()) {
		ss << "   pc = 0x" << std::setfill('0') << std::setw(8) << m_pc << "\n";
		// TODO: Symbol lookup

		for (size_t i=0; i<N_M88K_REGS; i++) {
			stringstream regname;
			regname << "r" << i;
		
			ss << std::setfill(' ');
			ss << std::setw(5) << regname.str() << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_r[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (find(arguments.begin(), arguments.end(), "cr") != arguments.end()) {
		for (size_t i=0; i<N_M88K_CONTROL_REGS; i++) {
			stringstream regname;
			regname << "cr" << i;
		
			ss << std::setfill(' ');
			ss << std::setw(5) << regname.str() << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_cr[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (find(arguments.begin(), arguments.end(), "crn") != arguments.end()) {
		for (size_t i=0; i<N_M88K_CONTROL_REGS; i++) {
			ss << std::setfill(' ');
			ss << std::setw(5) << m88k_cr_name(i) << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_cr[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (find(arguments.begin(), arguments.end(), "fcr") != arguments.end()) {
		for (size_t i=0; i<N_M88K_FPU_CONTROL_REGS; i++) {
			stringstream regname;
			regname << "fcr" << i;
		
			ss << std::setfill(' ');
			ss << std::setw(5) << regname.str() << " = 0x";
			ss << std::setfill('0') << std::setw(8) << m_fcr[i];
			if ((i&3) == 3)
				ss << "\n";
			else
				ss << "  ";
		}

		done = true;
	}

	if (!done) {
		ss << "M88K usage: .registers [r] [cr] [crn] [fcr]\n"
		    "r   = pc and general purpose registers  (default)\n"
		    "cr  = control registers\n"
		    "crn = control registers with symbolic names instead of crX\n"
		    "fcr = floating point control registers\n";
	}

	gxemul->GetUI()->ShowDebugMessage(ss.str());
}


int M88K_CPUComponent::Execute(GXemul* gxemul, int nrOfCycles)
{
	if (gxemul->GetRunState() == GXemul::SingleStepping) {
		stringstream disasm;
		Unassemble(1, false, m_pc, disasm);
		gxemul->GetUI()->ShowDebugMessage(this, disasm.str());
	}

	// TODO: Replace this bogus stuff with actual instruction execution.
	m_r[1] += nrOfCycles * 42;
	m_pc += nrOfCycles * sizeof(uint32_t);

	return nrOfCycles;
}


bool M88K_CPUComponent::VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	bool& writable)
{
	// TODO. For now, just return paddr = vaddr.

	paddr = vaddr & 0xffffffff;
	writable = true;
	return true;
}


size_t M88K_CPUComponent::DisassembleInstruction(uint64_t vaddr, size_t maxLen,
	unsigned char *instruction, vector<string>& result)
{
	const size_t instrSize = sizeof(uint32_t);

	if (maxLen < instrSize) {
		assert(false);
		return 0;
	}

	// Read the instruction word:
	uint32_t iw = *((uint32_t *) instruction);
	if (m_isBigEndian)
		iw = BE32_TO_HOST(iw);
	else
		iw = LE32_TO_HOST(iw);

	// ... and add it to the result:
	char tmp[9];
	snprintf(tmp, sizeof(tmp), "%08x", (int) iw);
	result.push_back(tmp);

	uint32_t op26   = (iw >> 26) & 0x3f;
	uint32_t op11   = (iw >> 11) & 0x1f;
	uint32_t op10   = (iw >> 10) & 0x3f;
	uint32_t d      = (iw >> 21) & 0x1f;
	uint32_t s1     = (iw >> 16) & 0x1f;
	uint32_t s2     =  iw        & 0x1f;
	uint32_t op3d   = (iw >>  8) & 0xff;
	uint32_t imm16  = iw & 0xffff;
	uint32_t w5     = (iw >>  5) & 0x1f;
	uint32_t cr6    = (iw >>  5) & 0x3f;
	int32_t  d16    = ((int16_t) (iw & 0xffff)) * 4;
	int32_t  d26    = ((int32_t)((iw & 0x03ffffff) << 6)) >> 4;

	switch (op26) {

	case 0x00:	/*  xmem.bu  */
	case 0x01:	/*  xmem     */
	case 0x02:	/*  ld.hu    */
	case 0x03:	/*  ld.bu    */
	case 0x04:	/*  ld.d     */
	case 0x05:	/*  ld       */
	case 0x06:	/*  ld.h     */
	case 0x07:	/*  ld.b     */
	case 0x08:	/*  st.d     */
	case 0x09:	/*  st       */
	case 0x0a:	/*  st.h     */
	case 0x0b:	/*  st.b     */
	case 0x10:	/*  and     */
	case 0x11:	/*  and.u   */
	case 0x12:	/*  mask    */
	case 0x13:	/*  mask.u  */
	case 0x14:	/*  xor     */
	case 0x15:	/*  xor.u   */
	case 0x16:	/*  or      */
	case 0x17:	/*  or.u    */
	case 0x18:	/*  addu    */
	case 0x19:	/*  subu    */
	case 0x1a:	/*  divu    */
	case 0x1b:	/*  mulu    */
	case 0x1c:	/*  add    */
	case 0x1d:	/*  sub    */
	case 0x1e:	/*  div    */
	case 0x1f:	/*  cmp    */
		if (iw == 0x00000000) {
			result.push_back("-");
		} else {
			// Two registers (d, s1) and an immediate.
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss << "r" << d << ",r" << s1;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << "," << imm16;
			result.push_back(ss.str());
		}
		break;

	case 0x20:
		if ((iw & 0x001ff81f) == 0x00004000) {
			result.push_back("ldcr");
			stringstream ss;
			ss << "r" << d << ",cr" << cr6;
			result.push_back(ss.str());

			stringstream comment;
			comment << "; cr" << cr6 << " = " << m88k_cr_name(cr6);
			result.push_back(comment.str());
		} else if ((iw & 0x001ff81f) == 0x00004800) {
			result.push_back("fldcr");
			stringstream ss;
			ss << "r" << d << ",fcr" << cr6;
			result.push_back(ss.str());
		} else if ((iw & 0x03e0f800) == 0x00008000) {
			result.push_back("stcr");
			stringstream ss;
			ss << "r" << s1 << ",cr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");

			stringstream comment;
			comment << "; cr" << cr6 << " = " << m88k_cr_name(cr6);
			result.push_back(comment.str());
		} else if ((iw & 0x03e0f800) == 0x00008800) {
			result.push_back("fstcr");
			stringstream ss;
			ss << "r" << s1 << ",fcr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");
		} else if ((iw & 0x0000f800) == 0x0000c000) {
			result.push_back("xcr");
			stringstream ss;
			ss << "r" << d << ",r" << s1 << ",cr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");

			stringstream comment;
			comment << "; cr" << cr6 << " = " << m88k_cr_name(cr6);
			result.push_back(comment.str());
		} else if ((iw & 0x0000f800) == 0x0000c800) {
			result.push_back("fxcr");
			stringstream ss;
			ss << "r" << d << ",r" << s1 << ",fcr" << cr6;
			result.push_back(ss.str());
			if (s1 != s2)
				result.push_back("; Weird encoding: s1 != s2");
		} else {
			result.push_back("unimpl_0x20_variant");
		}
		break;

	case 0x21:
		switch (op11) {
		case 0x00:	/*  fmul  */
		case 0x05:	/*  fadd  */
		case 0x06:	/*  fsub  */
		case 0x07:	/*  fcmp  */
		case 0x0e:	/*  fdiv  */
			{
				stringstream ss;
				switch (op11) {
				case 0x00: ss << "fmul"; break;
				case 0x05: ss << "fadd"; break;
				case 0x06: ss << "fsub"; break;
				case 0x07: ss << "fcmp"; break;
				case 0x0e: ss << "fdiv"; break;
				}
				ss << "." <<
				    (((iw >> 5) & 1)? "d" : "s") <<
				    (((iw >> 9) & 1)? "d" : "s") <<
				    (((iw >> 7) & 1)? "d" : "s");
				result.push_back(ss.str());

				stringstream ss2;
				ss2 << "r" << d << ",r" << s1 << ",r" << s2;
				result.push_back(ss2.str());
			}
			break;
		case 0x04:	/*  flt  */
			{
				stringstream ss;
				switch (op11) {
				case 0x04: ss << "flt"; break;
				}
				ss << "." << (((iw >> 5) & 1)? "d" : "s") << "s";
				result.push_back(ss.str());

				stringstream ss2;
				ss2 << "r" << d << ",r" << s2;
				result.push_back(ss2.str());
			}
			break;
		case 0x09:	/*  int  */
		case 0x0a:	/*  nint  */
		case 0x0b:	/*  trnc  */
			{
				stringstream ss;
				switch (op11) {
				case 0x09: ss << "int"; break;
				case 0x0a: ss << "nint"; break;
				case 0x0b: ss << "trnc"; break;
				}
				ss << ".s" << (((iw >> 7) & 1)? "d" : "s");
				result.push_back(ss.str());

				stringstream ss2;
				ss2 << "r" << d << ",r" << s2;
				result.push_back(ss2.str());
			}
			break;
		default:{
				stringstream ss;
				ss << "unimpl_0x21, op11=" << op11;
				result.push_back(ss.str());
			}
		}
		break;

	case 0x30:	/*  br  */
	case 0x31:	/*  br.n  */
	case 0x32:	/*  bsr  */
	case 0x33:	/*  bsr.n  */
		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << ((uint32_t) (vaddr + d26));
			result.push_back(ss.str());
		}
		break;

	case 0x34:	/*  bb0    */
	case 0x35:	/*  bb0.n  */
	case 0x36:	/*  bb1    */
	case 0x37:	/*  bb1.n  */
	case 0x3a:	/*  bcnd    */
	case 0x3b:	/*  bcnd.n  */
		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			if (op26 == 0x3a || op26 == 0x3b) {
				/*  Attempt to decode bcnd condition:  */
				switch (d) {
				case 0x1: ss << "gt0"; break;
				case 0x2: ss << "eq0"; break;
				case 0x3: ss << "ge0"; break;
				case 0x7: ss << "not_maxneg"; break;
				case 0x8: ss << "maxneg"; break;
				case 0xc: ss << "lt0"; break;
				case 0xd: ss << "ne0"; break;
				case 0xe: ss << "le0"; break;
				default:  ss << "unk_" << d;
				}
			} else {
				ss << d;
			}

			ss << ",r" << s1 << ",";

			ss.flags(std::ios::hex | std::ios::showbase);
			ss << ((uint32_t) (vaddr + d16));
			result.push_back(ss.str());
		}

		break;

	case 0x3c:
		if ((iw & 0x0000f000)==0x1000 || (iw & 0x0000f000)==0x2000) {
			/*  Load/store:  */
			stringstream ss;
			ss << ((iw & 0x0000f000) == 0x1000? "ld" : "st");

			switch (iw & 0x00000c00) {
			case 0x000: ss << ".d"; break;
			case 0x400: break;
			case 0x800: ss << ".x"; break;
			default: ss << ".UNIMPLEMENTED";
			}

			if (iw & 0x100)
				ss << ".usr";
			if (iw & 0x80)
				ss << ".wt";

			result.push_back(ss.str());

			stringstream ss2;
			ss2 << "r" << d << ",r" << s1;
			if (iw & 0x200)
				ss2 << "[r" << s2 << "]";
			else
				ss2 << ",r" << s2;

			result.push_back(ss2.str());
		} else switch (op10) {
		case 0x20:	/*  clr  */
		case 0x22:	/*  set  */
		case 0x24:	/*  ext  */
		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
		case 0x2a:	/*  rot  */
			/*  Two-register plus bit position/length:  */
			{
				result.push_back(opcode_names_3c[op10]);

				stringstream ss;
				ss << "r" << d << ",r" << s1 << ",";

				/*  Don't include w5 for the rot instruction:  */
				if (op10 != 0x2a)
					ss << w5;

				/*  Note: o5 = s2:  */
				ss << "<" << s2 << ">";

				result.push_back(ss.str());
			}
			break;
		case 0x34:	/*  tb0  */
		case 0x36:	/*  tb1  */
			/*  Two-register plus 9-bit immediate:  */
			{
				result.push_back(opcode_names_3c[op10]);

				stringstream ss;
				ss << "r" << d << ",r" << s1 << ",";
				ss.flags(std::ios::hex | std::ios::showbase);
				ss << (iw & 0x1ff);
				result.push_back(ss.str());
			}
			break;
		default:{
				stringstream ss;
				ss << "unimpl_" << opcode_names_3c[op10];
				result.push_back(ss.str());
			}
		}
		break;

	case 0x3d:
		if ((iw & 0xf000) <= 0x3fff) {
			/*  Load, Store, xmem, and lda:  */
			stringstream op;
			
			switch (iw & 0xf000) {
			case 0x2000: op << "st"; break;
			case 0x3000: op << "lda"; break;
			default:     if ((iw & 0xf800) >= 0x0800)
					op << "ld";
				     else
					op << "xmem";
			}
			
			if ((iw & 0xf000) >= 0x1000) {
				/*  ld, st, lda  */
				op << memop[(iw >> 10) & 3];
			} else if ((iw & 0xf800) == 0x0000) {
				/*  xmem  */
				if (!(iw & 0x400))
					op << ".bu";
			} else {
				/*  ld  */
				if ((iw & 0xf00) < 0xc00)
					op << ".hu";
				else
					op << ".bu";
			}
			
			if (iw & 0x100)
				op << ".usr";
			if (iw & 0x80)
				op << ".wt";

			result.push_back(op.str());

			stringstream ss;
			ss << "r" << d << ",r" << s1;
			if (iw & 0x200)
				ss << "[r" << s2 << "]";
			else
				ss << ",r" << s2;

			result.push_back(ss.str());
		} else switch (op3d) {
		case 0x40:	/*  and  */
		case 0x44:	/*  and.c  */
		case 0x50:	/*  xor  */
		case 0x54:	/*  xor.c  */
		case 0x58:	/*  or  */
		case 0x5c:	/*  or.c  */
		case 0x60:	/*  addu  */
		case 0x61:	/*  addu.co  */
		case 0x62:	/*  addu.ci  */
		case 0x63:	/*  addu.cio  */
		case 0x64:	/*  subu  */
		case 0x65:	/*  subu.co  */
		case 0x66:	/*  subu.ci  */
		case 0x67:	/*  subu.cio  */
		case 0x68:	/*  divu  */
		case 0x69:	/*  divu.d  */
		case 0x6c:	/*  mul  */
		case 0x6d:	/*  mulu.d  */
		case 0x6e:	/*  muls  */
		case 0x70:	/*  add  */
		case 0x71:	/*  add.co  */
		case 0x72:	/*  add.ci  */
		case 0x73:	/*  add.cio  */
		case 0x74:	/*  sub  */
		case 0x75:	/*  sub.co  */
		case 0x76:	/*  sub.ci  */
		case 0x77:	/*  sub.cio  */
		case 0x78:	/*  div  */
		case 0x7c:	/*  cmp  */
		case 0x80:	/*  clr  */
		case 0x88:	/*  set  */
		case 0x90:	/*  ext  */
		case 0x98:	/*  extu  */
		case 0xa0:	/*  mak  */
		case 0xa8:	/*  rot  */
			/*  Three-register opcodes:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "r" << d << ",r" << s1 << ",r" << s2;
				result.push_back(ss.str());
			}
			break;
		case 0xc0:	/*  jmp  */
		case 0xc4:	/*  jmp.n  */
		case 0xc8:	/*  jsr  */
		case 0xcc:	/*  jsr.n  */
			/*  One-register jump opcodes:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "(r" << s2 << ")";
				result.push_back(ss.str());
			}
			break;
		case 0xe8:	/*  ff1  */
		case 0xec:	/*  ff0  */
			/*  Two-register opcodes d,s2:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "r" << d << ",r" << s2;
				result.push_back(ss.str());
			}
			break;
		case 0xf8:	/*  tbnd  */
			/*  Two-register opcodes s1,s2:  */
			{
				result.push_back(opcode_names_3d[op3d]);

				stringstream ss;
				ss << "r" << s1 << ",r" << s2;
				result.push_back(ss.str());
			}
			break;
		case 0xfc:
			switch (iw & 0xff) {
			case 0x00:
				result.push_back("rte");
				break;
			case 0x01:
			case 0x02:
			case 0x03:
				{
					stringstream ss;
					ss << "illop" << (iw & 0xff);
					result.push_back(ss.str());
				}
				break;
			case (M88K_PROM_INSTR & 0xff):
				result.push_back("gxemul_prom_call");
				break;
			default:{
					stringstream ss;
					ss << "unimpl_3d_0xfc_" << (iw & 0xff);
					result.push_back(ss.str());
				}
			}
			break;
		default:{
				stringstream ss;
				ss << "unimpl_" << opcode_names_3d[op3d];
				result.push_back(ss.str());
			}
		}
		break;

	case 0x3e:	/*  tbnd  */
		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss << "r" << s1;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << "," << imm16;
			result.push_back(ss.str());
		}
		break;

	default:
		{
			stringstream ss;
			ss << "unimpl_" << opcode_names[op26];
			result.push_back(ss.str());
		}
		break;
	}

	return instrSize;
}


string M88K_CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "Motorola 88000 processor.";

	return Component::GetAttribute(attributeName);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_M88K_CPUComponent_IsStable()
{
	UnitTest::Assert("the M88K_CPUComponent should be stable",
	    ComponentFactory::HasAttribute("m88k_cpu", "stable"));
}

static void Test_M88K_CPUComponent_Create()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");
	UnitTest::Assert("component was not created?", !cpu.IsNULL());

	const StateVariable * p = cpu->GetVariable("pc");
	UnitTest::Assert("cpu has no pc state variable?", p != NULL);
	UnitTest::Assert("initial pc", p->ToString(), "0");
}

static void Test_M88K_CPUComponent_IsCPU()
{
	refcount_ptr<Component> m88k_cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");

	CPUComponent* cpu = m88k_cpu->AsCPUComponent();
	UnitTest::Assert("m88k_cpu is not a CPUComponent?", cpu != NULL);
}

static void Test_M88K_CPUComponent_DefaultModel()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");

	// Suitable default models would be 88100 and 88110 (the only two
	// implementations there were of the 88K architecture). However,
	// right now (2009-07-27), 88110 emulation isn't implemented yet.
	UnitTest::Assert("wrong default model",
	    cpu->GetVariable("model")->ToString(), "88100");
}

static void Test_M88K_CPUComponent_Disassembly_Basic()
{
	refcount_ptr<Component> m88k_cpu =
	    ComponentFactory::CreateComponent("m88k_cpu");
	CPUComponent* cpu = m88k_cpu->AsCPUComponent();

	vector<string> result;
	size_t len;
	unsigned char instruction[sizeof(uint32_t)];
	// This assumes that the default endianness is BigEndian...
	instruction[0] = 0x63;
	instruction[1] = 0xdf;
	instruction[2] = 0x00;
	instruction[3] = 0x10;

	len = cpu->DisassembleInstruction(0x12345678, sizeof(uint32_t),
	    instruction, result);

	UnitTest::Assert("disassembled instruction was wrong length?", len, 4);
	UnitTest::Assert("disassembly result incomplete?", result.size(), 3);
	UnitTest::Assert("disassembly result[0]", result[0], "63df0010");
	UnitTest::Assert("disassembly result[1]", result[1], "addu");
	UnitTest::Assert("disassembly result[2]", result[2], "r30,r31,0x10");
}

UNITTESTS(M88K_CPUComponent)
{
	UNITTEST(Test_M88K_CPUComponent_IsStable);
	UNITTEST(Test_M88K_CPUComponent_Create);
	UNITTEST(Test_M88K_CPUComponent_IsCPU);
	UNITTEST(Test_M88K_CPUComponent_DefaultModel);

	// Disassembly:
	UNITTEST(Test_M88K_CPUComponent_Disassembly_Basic);
}

#endif

