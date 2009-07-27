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
#include <iomanip>

#include "GXemul.h"
#include "components/M88K_CPUComponent.h"

static const char* opcode_names[] = M88K_OPCODE_NAMES;
static m88k_cpu_type_def cpu_type_defs[] = M88K_CPU_TYPE_DEFS;


M88K_CPUComponent::M88K_CPUComponent()
	: CPUComponent("m88k_cpu", "Motorola 88000")
	, m_m88k_type("88100")
{
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
	m_frequency = 50e6;	// 50 MHz

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
		ss << "M88K usage: .registers [r] [cr] [fcr]\n"
		    "r   = pc and general purpose registers  (default)\n"
		    "cr  = control registers\n"
		    "fcr = floating point control registers\n";
	}

	gxemul->GetUI()->ShowDebugMessage(ss.str());
}


int M88K_CPUComponent::Execute(GXemul* gxemul, int nrOfCycles)
{
	stringstream disasm;
	Unassemble(1, false, m_pc, disasm);
	gxemul->GetUI()->ShowDebugMessage(this, disasm.str());

	// TODO: Replace this bogus stuff with actual instruction execution.
	m_r[1] += nrOfCycles * 42;

	m_pc += sizeof(uint32_t);

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
//	uint32_t op11   = (iw >> 11) & 0x1f;
//	uint32_t op10   = (iw >> 10) & 0x3f;
	uint32_t d      = (iw >> 21) & 0x1f;
	uint32_t s1     = (iw >> 16) & 0x1f;
//	uint32_t s2     =  iw        & 0x1f;
	uint32_t imm16  = iw & 0xffff;
//	int32_t  simm16 = (int16_t) (iw & 0xffff);
//	uint32_t w5     = (iw >>  5) & 0x1f;
//	uint32_t cr6    = (iw >>  5) & 0x3f;
//	int32_t  d16    = ((int16_t) (iw & 0xffff)) * 4;
//	int32_t  d26    = ((int32_t)((iw & 0x03ffffff) << 6)) >> 4;

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
			break;
		}

		{
			result.push_back(opcode_names[op26]);

			stringstream ss;
			ss << "r" << d << ",r" << s1;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << "," << imm16;
			result.push_back(ss.str());
		}

		break;
#if 0
	case 0x20:
		if ((iw & 0x001ff81f) == 0x00004000) {
			debug("ldcr\tr%i,%s\n", d,
			    m88k_cr_name(cpu, cr6));
		} else if ((iw & 0x001ff81f) == 0x00004800) {
			debug("fldcr\tr%i,%s\n", d,
			    m88k_fcr_name(cpu, cr6));
		} else if ((iw & 0x03e0f800) == 0x00008000) {
			debug("stcr\tr%i,%s", s1,
			    m88k_cr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else if ((iw & 0x03e0f800) == 0x00008800) {
			debug("fstcr\tr%i,%s", s1,
			    m88k_fcr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else if ((iw & 0x0000f800) == 0x0000c000) {
			debug("xcr\tr%i,r%i,%s", d, s1,
			    m88k_cr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else if ((iw & 0x0000f800) == 0x0000c800) {
			debug("fxcr\tr%i,r%i,%s", d, s1,
			    m88k_fcr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else {
			debug("UNIMPLEMENTED 0x20\n");
		}
		break;

	case 0x21:
		switch (op11) {
		case 0x00:	/*  fmul  */
		case 0x05:	/*  fadd  */
		case 0x06:	/*  fsub  */
		case 0x07:	/*  fcmp  */
		case 0x0e:	/*  fdiv  */
			switch (op11) {
			case 0x00: mnem = "fmul"; break;
			case 0x05: mnem = "fadd"; break;
			case 0x06: mnem = "fsub"; break;
			case 0x07: mnem = "fcmp"; break;
			case 0x0e: mnem = "fdiv"; break;
			}
			debug("%s.%c%c%c r%i,r%i,r%i\n",
			    mnem,
			    ((iw >> 5) & 1)? 'd' : 's',
			    ((iw >> 9) & 1)? 'd' : 's',
			    ((iw >> 7) & 1)? 'd' : 's',
			    d, s1, s2);
			break;
		case 0x04:	/*  flt  */
			switch (op11) {
			case 0x04: mnem = "flt"; break;
			}
			debug("%s.%cs\tr%i,r%i\n",
			    mnem,
			    ((iw >> 5) & 1)? 'd' : 's',
			    d, s2);
			break;
		case 0x09:	/*  int  */
		case 0x0a:	/*  nint  */
		case 0x0b:	/*  trnc  */
			switch (op11) {
			case 0x09: mnem = "int"; break;
			case 0x0a: mnem = "nint"; break;
			case 0x0b: mnem = "trnc"; break;
			}
			debug("%s.s%c r%i,r%i\n",
			    mnem,
			    ((iw >> 7) & 1)? 'd' : 's',
			    d, s2);
			break;
		default:debug("UNIMPLEMENTED 0x21, op11=0x%02x\n", op11);
		}
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
		debug("b%sr%s\t",
		    op26 >= 0x32? "s" : "",
		    op26 & 1? ".n" : "");
		debug("0x%08"PRIx32, (uint32_t) (dumpaddr + d26));
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    dumpaddr + d26, &offset);
		if (symbol != NULL && supervisor)
			debug("\t; <%s>", symbol);
		debug("\n");
		break;

	case 0x34:	/*  bb0    */
	case 0x35:	/*  bb0.n  */
	case 0x36:	/*  bb1    */
	case 0x37:	/*  bb1.n  */
	case 0x3a:	/*  bcnd    */
	case 0x3b:	/*  bcnd.n  */
		switch (op26) {
		case 0x34:
		case 0x35: mnem = "bb0"; break;
		case 0x36:
		case 0x37: mnem = "bb1"; break;
		case 0x3a:
		case 0x3b: mnem = "bcnd"; break;
		}
		debug("%s%s\t", mnem, op26 & 1? ".n" : "");
		if (op26 == 0x3a || op26 == 0x3b) {
			/*  Attempt to decode bcnd condition:  */
			switch (d) {
			case 0x1: debug("gt0"); break;
			case 0x2: debug("eq0"); break;
			case 0x3: debug("ge0"); break;
			case 0x7: debug("not_maxneg"); break;
			case 0x8: debug("maxneg"); break;
			case 0xc: debug("lt0"); break;
			case 0xd: debug("ne0"); break;
			case 0xe: debug("le0"); break;
			default:  debug("unimplemented_%i", d);
			}
		} else {
			debug("%i", d);
		}
		debug(",r%i,0x%08"PRIx32, s1, (uint32_t) (dumpaddr + d16));
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    dumpaddr + d16, &offset);
		if (symbol != NULL && supervisor)
			debug("\t; <%s>", symbol);
		debug("\n");
		break;

	case 0x3c:
		if ((iw & 0x0000f000)==0x1000 || (iw & 0x0000f000)==0x2000) {
			int scale = 0;

			/*  Load/store:  */
			debug("%s", (iw & 0x0000f000) == 0x1000? "ld" : "st");
			switch (iw & 0x00000c00) {
			case 0x000: scale = 8; debug(".d"); break;
			case 0x400: scale = 4; break;
			case 0x800: debug(".x"); break;
			default: debug(".UNIMPLEMENTED");
			}
			if (iw & 0x100)
				debug(".usr");
			if (iw & 0x80)
				debug(".wt");
			debug("\tr%i,r%i", d, s1);
			if (iw & 0x200)
				debug("[r%i]", s2);
			else
				debug(",r%i", s2);

			if (running && scale >= 1) {
				uint32_t tmpaddr = cpu->cd.m88k.r[s1];
				if (iw & 0x200)
					tmpaddr += scale * cpu->cd.m88k.r[s2];
				else
					tmpaddr += cpu->cd.m88k.r[s2];
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tmpaddr, &offset);
				if (symbol != NULL && supervisor)
					debug("\t; [<%s>]", symbol);
				else
					debug("\t; [0x%08"PRIx32"]", tmpaddr);
			}

			debug("\n");
		} else switch (op10) {
		case 0x20:	/*  clr  */
		case 0x22:	/*  set  */
		case 0x24:	/*  ext  */
		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
		case 0x2a:	/*  rot  */
			switch (op10) {
			case 0x20: mnem = "clr"; break;
			case 0x22: mnem = "set"; break;
			case 0x24: mnem = "ext"; break;
			case 0x26: mnem = "extu"; break;
			case 0x28: mnem = "mak"; break;
			case 0x2a: mnem = "rot"; break;
			}
			debug("%s\tr%i,r%i,", mnem, d, s1);
			/*  Don't include w5 for the rot instruction:  */
			if (op10 != 0x2a)
				debug("%i", w5);
			/*  Note: o5 = s2:  */
			debug("<%i>\n", s2);
			break;
		case 0x34:	/*  tb0  */
		case 0x36:	/*  tb1  */
			switch (op10) {
			case 0x34: mnem = "tb0"; break;
			case 0x36: mnem = "tb1"; break;
			}
			debug("%s\t%i,r%i,0x%x\n", mnem, d, s1, iw & 0x1ff);
			break;
		default:debug("UNIMPLEMENTED 0x3c, op10=0x%02x\n", op10);
		}
		break;

	case 0x3d:
		if ((iw & 0xf000) <= 0x3fff) {
			int scale = 0;

			/*  Load, Store, xmem, and lda:  */
			switch (iw & 0xf000) {
			case 0x2000: debug("st"); break;
			case 0x3000: debug("lda"); break;
			default:     if ((iw & 0xf800) >= 0x0800)
					  debug("ld");
				     else
					  debug("xmem");
			}
			if ((iw & 0xf000) >= 0x1000) {
				/*  ld, st, lda  */
				scale = 1 << (3 - ((iw >> 10) & 3));
				debug("%s", memop[(iw >> 10) & 3]);
			} else if ((iw & 0xf800) == 0x0000) {
				/*  xmem  */
				if (iw & 0x400)
					scale = 4;
				else
					debug(".bu"), scale = 1;
			} else {
				/*  ld  */
				if ((iw & 0xf00) < 0xc00)
					debug(".hu"), scale = 2;
				else
					debug(".bu"), scale = 1;
			}
			if (iw & 0x100)
				debug(".usr");
			if (iw & 0x80)
				debug(".wt");
			debug("\tr%i,r%i", d, s1);
			if (iw & 0x200)
				debug("[r%i]", s2);
			else
				debug(",r%i", s2);

			if (running && scale >= 1) {
				uint32_t tmpaddr = cpu->cd.m88k.r[s1];
				if (iw & 0x200)
					tmpaddr += scale * cpu->cd.m88k.r[s2];
				else
					tmpaddr += cpu->cd.m88k.r[s2];
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tmpaddr, &offset);
				if (symbol != NULL && supervisor)
					debug("\t; [<%s>]", symbol);
				else
					debug("\t; [0x%08"PRIx32"]", tmpaddr);
			}

			debug("\n");
		} else switch ((iw >> 8) & 0xff) {
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
			switch ((iw >> 8) & 0xff) {
			case 0x40: mnem = "and"; break;
			case 0x44: mnem = "and.c"; break;
			case 0x50: mnem = "xor"; break;
			case 0x54: mnem = "xor.c"; break;
			case 0x58: mnem = "or"; break;
			case 0x5c: mnem = "or.c"; break;
			case 0x60: mnem = "addu"; break;
			case 0x61: mnem = "addu.co"; break;
			case 0x62: mnem = "addu.ci"; break;
			case 0x63: mnem = "addu.cio"; break;
			case 0x64: mnem = "subu"; break;
			case 0x65: mnem = "subu.co"; break;
			case 0x66: mnem = "subu.ci"; break;
			case 0x67: mnem = "subu.cio"; break;
			case 0x68: mnem = "divu"; break;
			case 0x69: mnem = "divu.d"; break;
			case 0x6c: mnem = "mul"; break;
			case 0x6d: mnem = "mulu.d"; break;
			case 0x6e: mnem = "muls"; break;
			case 0x70: mnem = "add"; break;
			case 0x71: mnem = "add.co"; break;
			case 0x72: mnem = "add.ci"; break;
			case 0x73: mnem = "add.cio"; break;
			case 0x74: mnem = "sub"; break;
			case 0x75: mnem = "sub.co"; break;
			case 0x76: mnem = "sub.ci"; break;
			case 0x77: mnem = "sub.cio"; break;
			case 0x78: mnem = "div"; break;
			case 0x7c: mnem = "cmp"; break;
			case 0x80: mnem = "clr"; break;
			case 0x88: mnem = "set"; break;
			case 0x90: mnem = "ext"; break;
			case 0x98: mnem = "extu"; break;
			case 0xa0: mnem = "mak"; break;
			case 0xa8: mnem = "rot"; break;
			}
			debug("%s\tr%i,r%i,r%i\n", mnem, d, s1, s2);
			break;
		case 0xc0:	/*  jmp  */
		case 0xc4:	/*  jmp.n  */
		case 0xc8:	/*  jsr  */
		case 0xcc:	/*  jsr.n  */
			debug("%s%s\t(r%i)",
			    op11 & 1? "jsr" : "jmp",
			    iw & 0x400? ".n" : "",
			    s2);
			if (running) {
				uint32_t tmpaddr = cpu->cd.m88k.r[s2];
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tmpaddr, &offset);
				debug("\t\t; ");
				if (symbol != NULL && supervisor)
					debug("<%s>", symbol);
				else
					debug("0x%08"PRIx32, tmpaddr);
			}
			debug("\n");
			break;
		case 0xe8:	/*  ff1  */
		case 0xec:	/*  ff0  */
			debug("%s\tr%i,r%i\n",
			    ((iw >> 8) & 0xff) == 0xe8 ? "ff1" : "ff0", d, s2);
			break;
		case 0xf8:	/*  tbnd  */
			debug("tbnd\tr%i,r%i\n", s1, s2);
			break;
		case 0xfc:
			switch (iw & 0xff) {
			case 0x00:
				debug("rte\n");
				break;
			case 0x01:
			case 0x02:
			case 0x03:
				debug("illop%i\n", iw & 0xff);
				break;
			case (M88K_PROM_INSTR & 0xff):
				debug("gxemul_prom_call\n");
				break;
			default:debug("UNIMPLEMENTED 0x3d,0xfc: 0x%02x\n",
				    iw & 0xff);
			}
			break;
		default:debug("UNIMPLEMENTED 0x3d, opbyte = 0x%02x\n",
			    (iw >> 8) & 0xff);
		}
		break;
#endif

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

UNITTESTS(M88K_CPUComponent)
{
	UNITTEST(Test_M88K_CPUComponent_IsStable);
	UNITTEST(Test_M88K_CPUComponent_Create);
	UNITTEST(Test_M88K_CPUComponent_IsCPU);
}

#endif

