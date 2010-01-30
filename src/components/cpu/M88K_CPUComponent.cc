/*
 *  Copyright (C) 2009-2010  Anders Gavare.  All rights reserved.
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

#include "ComponentFactory.h"
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
	: CPUDyntransComponent("m88k_cpu", "Motorola 88000")
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

	AddVariable("inDelaySlot", &m_inDelaySlot);
	AddVariable("delaySlotTarget", &m_delaySlotTarget);
}


refcount_ptr<Component> M88K_CPUComponent::Create(const ComponentCreateArgs& args)
{
	// Defaults:
	ComponentCreationSettings settings;
	settings["model"] = "88100";

	if (!ComponentFactory::GetCreationArgOverrides(settings, args))
		return NULL;

	refcount_ptr<Component> cpu = new M88K_CPUComponent();
	if (!cpu->SetVariableValue("model", "\"" + settings["model"] + "\""))
		return NULL;

	return cpu;
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

	CPUDyntransComponent::ResetState();
}


bool M88K_CPUComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	if (m_r[M88K_ZERO_REG] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "the r0 register "
		    "must contain the value 0.\n");
		return false;
	}

	if (m_pc > (uint64_t)0xffffffff) {
		gxemul->GetUI()->ShowDebugMessage(this, "the pc register "
		    "must be a 32-bit value.\n");
		return false;
	}

	if (m_pc & 0x2) {
		gxemul->GetUI()->ShowDebugMessage(this, "the pc register must have"
		    " its lower two bits clear!\n");
		return false;
	}

	if (m_r[N_M88K_REGS] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "internal error: the "
		    "register following r31 must mimic the r0 register.\nIf"
		    " you encounter this message, please write a bug report!\n");
		return false;
	}

	return CPUDyntransComponent::PreRunCheckForComponent(gxemul);
}


bool M88K_CPUComponent::CheckVariableWrite(StateVariable& var, const string& oldValue)
{
	UI* ui = GetUI();

	if (m_r[M88K_ZERO_REG] != 0) {
		if (ui != NULL) {
			ui->ShowDebugMessage(this, "the zero register (r0) "
			    "must contain the value 0.\n");
		}
		return false;
	}

	if (m_m88k_type != m_type.name) {
		bool found = false;
		for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
			if (m_m88k_type == cpu_type_defs[j].name) {
				m_type = cpu_type_defs[j];
				found = true;
				break;
			}
		}

		if (!found) {
			if (ui != NULL) {
				stringstream ss;
				ss << "Unknown model \"" + m_m88k_type + "\". Available types are:\n";
				for (size_t j=0; cpu_type_defs[j].name != NULL; j++) {
					if ((j % 6) != 0)
						ss << "\t";
					ss << cpu_type_defs[j].name;
					if ((j % 6) == 5)
						ss << "\n";
				}
				ui->ShowDebugMessage(this, ss.str());
			}
			return false;
		}
	}

	return CPUDyntransComponent::CheckVariableWrite(var, oldValue);
}


void M88K_CPUComponent::ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const
{
	bool done = false;

	stringstream ss;
	ss.flags(std::ios::hex);

	if (arguments.size() == 0 ||
	    find(arguments.begin(), arguments.end(), "r") != arguments.end()) {
		ss << "   pc = 0x" << std::setfill('0') << std::setw(8) << m_pc;

		string symbol = GetSymbolRegistry().LookupAddress(m_pc, true);
		if (symbol != "")
			ss << "  <" << symbol << ">";
		ss << "\n";

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


int M88K_CPUComponent::GetDyntransICshift() const
{
	// 4 bytes per instruction, i.e. shift is 2 bits.
	return M88K_INSTR_ALIGNMENT_SHIFT;
}


void (*M88K_CPUComponent::GetDyntransToBeTranslated())(CPUDyntransComponent*, DyntransIC*) const
{
	return instr_ToBeTranslated;
}


bool M88K_CPUComponent::VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	bool& writable)
{
	// TODO. For now, just return paddr = vaddr.

	paddr = vaddr & 0xffffffff;
	writable = true;
	return true;
}


void M88K_CPUComponent::Exception(int vector, int is_trap)
{
	std::cerr << "TODO: M88K exception\n";
	throw std::exception();
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
	{
		stringstream ss;
		ss.flags(std::ios::hex);
		ss << std::setfill('0') << std::setw(8) << (uint32_t) iw;
		if (m_pc == vaddr && m_inDelaySlot)
			ss << " (delayslot)";
		result.push_back(ss.str());
	}

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

			string symbol = GetSymbolRegistry().LookupAddress(
			    (uint32_t) (vaddr + d26), true);
			if (symbol != "")
				result.push_back("; <" + symbol + ">");
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

			string symbol = GetSymbolRegistry().LookupAddress(
			    (uint32_t) (vaddr + d16), true);
			if (symbol != "")
				result.push_back("; <" + symbol + ">");
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


/*
 *  cmp_imm:  Compare S1 with immediate value.
 *  cmp:      Compare S1 with S2.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or imm
 */
void M88K_CPUComponent::m88k_cmp(struct DyntransIC *ic, uint32_t y)
{
	uint32_t x = REG32(ic->arg[1]);
	uint32_t r;

	if (x == y) {
		r = M88K_CMP_HS | M88K_CMP_LS | M88K_CMP_GE
		  | M88K_CMP_LE | M88K_CMP_EQ;
	} else {
		if (x > y)
			r = M88K_CMP_NE | M88K_CMP_HS | M88K_CMP_HI;
		else
			r = M88K_CMP_NE | M88K_CMP_LO | M88K_CMP_LS;
		if ((int32_t)x > (int32_t)y)
			r |= M88K_CMP_GE | M88K_CMP_GT;
		else
			r |= M88K_CMP_LT | M88K_CMP_LE;
	}

	REG32(ic->arg[0]) = r;
}


DYNTRANS_INSTR(M88K_CPUComponent,cmp)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_cmp(ic, REG32(ic->arg[2]));
}


DYNTRANS_INSTR(M88K_CPUComponent,cmp_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_cmp(ic, ic->arg[2].u32);
}


/*
 *  mak:      Make bit field, W<O> taken from register s2.
 *  mak_imm:  Make bit field, immediate W<O>.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or immediate.
 */
void M88K_CPUComponent::m88k_mak(struct DyntransIC *ic, int w, int o)
{
	uint32_t x = REG32(ic->arg[1]);
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}

	REG32(ic->arg[0]) = x << o;
}


DYNTRANS_INSTR(M88K_CPUComponent,mak)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_mak(ic, (REG32(ic->arg[2]) >> 5) & 0x1f, REG32(ic->arg[2]) & 0x1f);
}


DYNTRANS_INSTR(M88K_CPUComponent,mak_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)
	cpu->m88k_mak(ic, ic->arg[2].u32 >> 5, ic->arg[2].u32 & 0x1f);
}


/*
 *  mulu_imm:  d = s1 * immediate
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = immediate.
 */
DYNTRANS_INSTR(M88K_CPUComponent,mulu_imm)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	if (cpu->m_cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		DYNTRANS_SYNCH_PC;
		cpu->m_fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		cpu->Exception(M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else {
		REG32(ic->arg[0]) = REG32(ic->arg[1]) * ic->arg[2].u32;
	}
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = cpu->m_pc + ic->arg[2].u32;

	cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[1].u32);
	cpu->DyntransPCtoPointers();
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr_samepage)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->m_r[M88K_RETURN_REG] = (cpu->m_pc & ~((M88K_IC_ENTRIES_PER_PAGE-1)
	    << M88K_INSTR_ALIGNMENT_SHIFT)) + ic->arg[2].u32;
	cpu->m_nextIC = (struct DyntransIC *) ic->arg[0].p;
}


DYNTRANS_INSTR(M88K_CPUComponent,bsr_functioncalltrace)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->m_pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->m_r[M88K_RETURN_REG] = cpu->m_pc + ic->arg[2].u32;

	cpu->m_pc = (uint32_t) (cpu->m_pc + ic->arg[1].u32);
	cpu->FunctionTraceCall();
	cpu->DyntransPCtoPointers();
}


DYNTRANS_INSTR(M88K_CPUComponent,jmp_n)
{
	std::cerr << "jmp_n when not single stepping: TODO\n";
	throw std::exception();
}


DYNTRANS_INSTR(M88K_CPUComponent,jmp_n_functioncalltrace)
{
	std::cerr << "jmp_n with function call trace when not single stepping: TODO\n";
	throw std::exception();
}


// Note: This IC function is used both when function call trace is enabled
// and disabled. (Ok, since it is only used when singlestepping.)
DYNTRANS_INSTR(M88K_CPUComponent,jmp_n_functioncalltrace_singlestep)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	if (cpu->m_showFunctionTraceCall && ic->arg[2].p == &cpu->m_r[M88K_RETURN_REG])
		cpu->FunctionTraceReturn();

	// Prepare for the delayed branch.
	cpu->m_inDelaySlot = true;
	cpu->m_exceptionInDelaySlot = false;

	cpu->m_delaySlotTarget = REG32(ic->arg[2]);

	// m_nextIC already points to the next instruction
}


/*
 *  ldcr:   Load value from a control register, store in register d.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = 6-bit control register number
 */
DYNTRANS_INSTR(M88K_CPUComponent,ldcr)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	if (cpu->m_cr[M88K_CR_PSR] & M88K_PSR_MODE) {
		int cr = ic->arg[1].u32;
		REG32(ic->arg[0]) = cpu->m_cr[cr];
	} else {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
	}
}


/*
 *  st:   Store word.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = uint16_t offset
 */
DYNTRANS_INSTR(M88K_CPUComponent,st)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	uint32_t data = REG32(ic->arg[0]);
	uint32_t addr = REG32(ic->arg[1]) + ic->arg[2].u32;

	if (addr & 3) {
		DYNTRANS_SYNCH_PC;
		cpu->Exception(M88K_EXCEPTION_MISALIGNED_ACCESS, 0);
		return;
	}

	cpu->AddressSelect(addr);

	if (!cpu->WriteData(data, cpu->m_isBigEndian? BigEndian : LittleEndian)) {
		// TODO: bus error exception?
//		DYNTRANS_SYNCH_PC;
//		cpu->Exception(M88K_EXCEPTION_MISALIGNED_ACCESS, 0);
	}
}


/*****************************************************************************/


void M88K_CPUComponent::Translate(uint32_t iw, struct DyntransIC* ic)
{
	bool singleInstructionLeft = (m_executedCycles == m_nrOfCyclesToExecute - 1);
	UI* ui = GetUI();	// for debug messages

	uint32_t op26   = (iw >> 26) & 0x3f;
//	uint32_t op11   = (iw >> 11) & 0x1f;
	uint32_t op10   = (iw >> 10) & 0x3f;
	uint32_t d      = (iw >> 21) & 0x1f;
	uint32_t s1     = (iw >> 16) & 0x1f;
	uint32_t s2     =  iw        & 0x1f;
//	uint32_t op3d   = (iw >>  8) & 0xff;
	uint32_t imm16  = iw & 0xffff;
//	uint32_t w5     = (iw >>  5) & 0x1f;
	uint32_t cr6    = (iw >>  5) & 0x3f;
//	int32_t  d16    = ((int16_t) (iw & 0xffff)) * 4;
	int32_t  d26    = ((int32_t)((iw & 0x03ffffff) << 6)) >> 4;

	switch (op26) {

//	case 0x02:	/*  ld.hu  */
//	case 0x03:	/*  ld.bu  */
//	case 0x04:	/*  ld.d   */
//	case 0x05:	/*  ld     */
//	case 0x06:	/*  ld.h   */
//	case 0x07:	/*  ld.b   */
//	case 0x08:	/*  st.d   */
	case 0x09:	/*  st     */
//	case 0x0a:	/*  st.h   */
//	case 0x0b:	/*  st.b   */
		{
			int store = 0, opsize = 0; // signedness = 0

			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = imm16;

			switch (op26) {
//			case 0x02: opsize = 1; break;
//			case 0x03: opsize = 0; break;
//			case 0x04: opsize = 3; break;
//			case 0x05: opsize = 2; break;
//			case 0x06: opsize = 1; signedness = 1; break;
//			case 0x07: opsize = 0; signedness = 1; break;
//			case 0x08: store = 1; opsize = 3; break;
			case 0x09: ic->f = instr_st; store = 1; opsize = 2; break;
//			case 0x0a: store = 1; opsize = 1; break;
//			case 0x0b: store = 1; opsize = 0; break;
			}

			if (opsize == 3 && d == 31) {
				// m88k load/store of register pair r31/r0 is not yet implemented
				ic->f = NULL;
				break;
			}

			// ic->f = m88k_loadstore[ opsize
			//     + (store? M88K_LOADSTORE_STORE : 0)
			//     + (signedness? M88K_LOADSTORE_SIGNEDNESS:0)
			//     + (cpu->byte_order == EMUL_BIG_ENDIAN?
			//        M88K_LOADSTORE_ENDIANNESS : 0) ];
		}
		break;

	case 0x10:	/*  and    immu32  */
	case 0x11:	/*  and.u  immu32  */
	case 0x12:	/*  mask   immu32  */
	case 0x13:	/*  mask.u immu32  */
	case 0x14:	/*  xor    immu32  */
	case 0x15:	/*  xor.u  immu32  */
	case 0x16:	/*  or     immu32  */
	case 0x17:	/*  or.u   immu32  */
	case 0x18:	/*  addu   immu32  */
	case 0x19:	/*  subu   immu32  */
	case 0x1b:	/*  mulu   immu32  */
	case 0x1f:	/*  cmp    immu32  */
		{
			int shift = 0;
			switch (op26) {
			case 0x10: ic->f = instr_and_u32_u32_immu32; break; // Note (see below): and only ands upper or lower part!
			case 0x11: ic->f = instr_and_u32_u32_immu32; shift = 16; break;
			case 0x12: ic->f = instr_and_u32_u32_immu32; break; // Note: mask is implemented using and
			case 0x13: ic->f = instr_and_u32_u32_immu32; shift = 16; break;
			case 0x14: ic->f = instr_xor_u32_u32_immu32; break;
			case 0x15: ic->f = instr_xor_u32_u32_immu32; shift = 16; break;
			case 0x16: ic->f = instr_or_u32_u32_immu32; break;
			case 0x17: ic->f = instr_or_u32_u32_immu32; shift = 16; break;
			case 0x18: ic->f = instr_add_u32_u32_immu32; break;
			case 0x19: ic->f = instr_sub_u32_u32_immu32; break;
	//		case 0x1a: ic->f = instr(divu_imm); break;
			case 0x1b: ic->f = instr_mulu_imm; break;
	//		case 0x1c: ic->f = instr(add_imm); break;
	//		case 0x1d: ic->f = instr(sub_imm); break;
	//		case 0x1e: ic->f = instr(div_imm); break;
			case 0x1f: ic->f = instr_cmp_imm; break;
			}

			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = imm16 << shift;

			// The 'and' instruction only ands bits in the upper or
			// lower parts of the word; the 'mask' instruction works
			// on the whole register.
			if (op26 == 0x10)
				ic->arg[2].u32 |= 0xffff0000;
			if (op26 == 0x11)
				ic->arg[2].u32 |= 0x0000ffff;

			if (d == M88K_ZERO_REG)
				ic->f = instr_nop;
		}
		break;

	case 0x20:
		if ((iw & 0x001ff81f) == 0x00004000) {
			ic->f = instr_ldcr;
			ic->arg[0].p = &m_r[d];
			ic->arg[1].u32 = cr6;
			if (d == M88K_ZERO_REG)
				ic->arg[0].p = &m_zero_scratch;
		}
//		} else if ((iword & 0x001ff81f) == 0x00004800) {
//			ic->f = instr(fldcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
//			ic->arg[1] = cr6;
//			if (d == M88K_ZERO_REG)
//				ic->arg[0] = (size_t)
//				    &cpu->cd.m88k.zero_scratch;
//		} else if ((iword & 0x03e0f800) == 0x00008000) {
//			ic->f = instr(stcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];
//			ic->arg[1] = cr6;
//			if (s1 != s2)
//				goto bad;
//		} else if ((iword & 0x03e0f800) == 0x00008800) {
//			ic->f = instr(fstcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];
//			ic->arg[1] = cr6;
//			if (s1 != s2)
//				goto bad;
//		} else if ((iword & 0x0000f800) == 0x0000c000) {
//			ic->f = instr(xcr);
//			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
//			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
//			ic->arg[2] = cr6;
//			if (s1 != s2)
//				goto bad;
//		} else
//			goto bad;
		break;


	case 0x30:	/*  br     */
//	case 0x31:	/*  br.n   */
	case 0x32:	/*  bsr    */
//	case 0x33:	/*  bsr.n  */
		{
			void (*samepage_function)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

			switch (op26) {
			case 0x30:
				ic->f = NULL; // instr(br);
				samepage_function = instr_branch_samepage;
				break;
	//		case 0x31:
	//			ic->f = instr(br_n);
	//			if (cpu->translation_readahead > 2)
	//				cpu->translation_readahead = 2;
	//			break;
			case 0x32:
				ic->f = instr_bsr;
				samepage_function = instr_bsr_samepage;
				break;
	//		case 0x33:
	//			ic->f = instr(bsr_n);
	//			break;
			}

			int offset = (m_pc & 0xffc) + d26;

			/*  Prepare both samepage and offset style args.
			    (Only one will be used in the actual instruction.)  */
			ic->arg[0].p = ( m_firstIConPage + (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
			ic->arg[1].u32 = offset;

			/*  Return offset for bsr (stored in m_r[M88K_RETURN_REG]):  */
			ic->arg[2].u32 = (m_pc & 0xffc) + 4;

			if (offset >= 0 && offset <= 0xffc &&
			    samepage_function != NULL)
				ic->f = samepage_function;

			if (m_showFunctionTraceCall) {
				if (op26 == 0x32)
					ic->f = instr_bsr_functioncalltrace;
//				if (op26 == 0x33)
//					ic->f = instr(bsr_n_trace);
			}
		}
		break;

	case 0x3c:
		switch (op10) {

//		case 0x20:	/*  clr  */
//		case 0x22:	/*  set  */
//		case 0x24:	/*  ext  */
//		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].u32 = iw & 0x3ff;

			switch (op10) {
//			case 0x20: ic->f = instr(mask_imm);
//				   {
//					int w = ic->arg[2] >> 5;
//					int o = ic->arg[2] & 0x1f;
//					uint32_t x = w == 0? 0xffffffff
//					    : ((uint32_t)1 << w) - 1;
//					x <<= o;
//					ic->arg[2] = ~x;
//				   }
//				   break;
//			case 0x22: ic->f = instr(or_imm);
//				   {
//					int w = ic->arg[2] >> 5;
//					int o = ic->arg[2] & 0x1f;
//					uint32_t x = w == 0? 0xffffffff
//					    : ((uint32_t)1 << w) - 1;
//					x <<= o;
//					ic->arg[2] = x;
//				   }
//				   break;
//			case 0x24: ic->f = instr(ext_imm); break;
//			case 0x26: ic->f = instr(extu_imm); break;
			case 0x28: ic->f = instr_mak_imm; break;
			}

			if (d == M88K_ZERO_REG)
				ic->f = instr_nop;
			break;

//		case 0x34:	/*  tb0  */
//		case 0x36:	/*  tb1  */
//			ic->arg[0] = 1 << d;
//			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
//			ic->arg[2] = iword & 0x1ff;
//			switch (op10) {
//			case 0x34: ic->f = instr(tb0); break;
//			case 0x36: ic->f = instr(tb1); break;
//			}
//			break;
		}
		break;

	case 0x3d:
		if ((iw & 0xf000) <= 0x3fff ) {
			// Load, Store, xmem, and lda:
			// TODO
		} else switch ((iw >> 8) & 0xff) {
//		case 0x40:	/*  and    */
//		case 0x44:	/*  and.c  */
		case 0x50:	/*  xor    */
//		case 0x54:	/*  xor.c  */
		case 0x58:	/*  or     */
//		case 0x5c:	/*  or.c   */
		case 0x60:	/*  addu   */
//		case 0x61:	/*  addu.co  */
//		case 0x62:	/*  addu.ci  */
		case 0x64:	/*  subu   */
//		case 0x65:	/*  subu.co  */
//		case 0x66:	/*  subu.ci  */
//		case 0x68:	/*  divu   */
//		case 0x6c:	/*  mul    */
//		case 0x70:	/*  add    */
//		case 0x78:	/*  div    */
		case 0x7c:	/*  cmp    */
//		case 0x80:	/*  clr    */
//		case 0x88:	/*  set    */
//		case 0x90:	/*  ext    */
//		case 0x98:	/*  extu   */
		case 0xa0:	/*  mak    */
//		case 0xa8:	/*  rot    */
			ic->arg[0].p = &m_r[d];
			ic->arg[1].p = &m_r[s1];
			ic->arg[2].p = &m_r[s2];

			switch ((iw >> 8) & 0xff) {
//			case 0x40: ic->f = instr(and);   break;
//			case 0x44: ic->f = instr(and_c); break;
			case 0x50: ic->f = instr_xor_u32_u32_u32; break;
//			case 0x54: ic->f = instr(xor_c); break;
			case 0x58: ic->f = instr_or_u32_u32_u32; break;
//			case 0x5c: ic->f = instr(or_c);  break;
			case 0x60: ic->f = instr_add_u32_u32_u32; break;
//			case 0x61: ic->f = instr(addu_co); break;
//			case 0x62: ic->f = instr(addu_ci); break;
			case 0x64: ic->f = instr_sub_u32_u32_u32; break;
//			case 0x65: ic->f = instr(subu_co); break;
//			case 0x66: ic->f = instr(subu_ci); break;
//			case 0x68: ic->f = instr(divu);  break;
//			case 0x6c: ic->f = instr(mul);   break;
//			case 0x70: ic->f = instr(add);   break;
//			case 0x78: ic->f = instr(div);   break;
			case 0x7c: ic->f = instr_cmp; break;
//			case 0x80: ic->f = instr(clr);   break;
//			case 0x88: ic->f = instr(set);   break;
//			case 0x90: ic->f = instr(ext);   break;
//			case 0x98: ic->f = instr(extu);  break;
			case 0xa0: ic->f = instr_mak; break;
//			case 0xa8: ic->f = instr(rot);   break;
			}

			/*
			 * Handle the case when the destination register is r0:
			 *
			 * If there is NO SIDE-EFFECT! (i.e. no carry out),
			 * then replace the instruction with a nop. If there is
			 * a side-effect, we still have to run the instruction,
			 * so replace the destination register with a scratch
			 * register.
			 */
			if (d == M88K_ZERO_REG) {
				int opc = (iw >> 8) & 0xff;
				if (opc != 0x61 && opc != 0x63 &&
				    opc != 0x65 && opc != 0x67 &&
				    opc != 0x71 && opc != 0x73 &&
				    opc != 0x75 && opc != 0x77)
					ic->f = instr_nop;
				else
					ic->arg[0].p = &m_zero_scratch;
			}
			break;
//		case 0xc0:	/*  jmp    */
		case 0xc4:	/*  jmp.n  */
//		case 0xc8:	/*  jsr    */
//		case 0xcc:	/*  jsr.n  */
			{
				void (*f_ss)(CPUDyntransComponent*, struct DyntransIC*) = NULL;

				switch ((iw >> 8) & 0xff) {
	//			case 0xc0: ic->f = instr(jmp); break;
				case 0xc4: ic->f = instr_jmp_n; f_ss = instr_jmp_n_functioncalltrace_singlestep; break;
	//			case 0xc8: ic->f = instr(jsr); break;
	//			case 0xcc: ic->f = instr(jsr_n); break;
				}

				ic->arg[1].u32 = (m_pc & 0xffc) + 4;
				ic->arg[2].p = &m_r[s2];

				if (((iw >> 8) & 0x04) == 0x04)
					ic->arg[1].u32 = (m_pc & 0xffc) + 8;

				if (m_showFunctionTraceCall && s2 == M88K_RETURN_REG) {
	//				if (ic->f == instr(jmp)) {
	//					ic->f = instr(jmp_trace);
	//					f_ss = NULL; //instr(jmp_trace);
	//				}
					if (ic->f == instr_jmp_n) {
						ic->f = instr_jmp_n_functioncalltrace;
						f_ss = instr_jmp_n_functioncalltrace_singlestep;
					}
				}

	//			if (m_showFunctionTraceCall) {
	//				if (ic->f == instr(jsr))
	//					ic->f = instr(jsr_trace);
	//					TODO f_ss
	//				if (ic->f == instr(jsr_n))
	//					ic->f = instr(jsr_n_trace);
	//					TODO f_ss
	//			}

				if (singleInstructionLeft)
					ic->f = f_ss;
			}
			break;
//		case 0xe8:	/*  ff1  */
//		case 0xec:      /*  ff0  */
			// TODO
//		case 0xfc:
			// TODO
		}
		break;

	default:
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "unimplemented opcode 0x" << op26;
			ui->ShowDebugMessage(this, ss.str());
		}
	}
}


DYNTRANS_INSTR(M88K_CPUComponent,ToBeTranslated)
{
	DYNTRANS_INSTR_HEAD(M88K_CPUComponent)

	cpu->DyntransToBeTranslatedBegin(ic);

	uint32_t iword;
	if (cpu->DyntransReadInstruction(iword))
		cpu->Translate(iword, ic);

	cpu->DyntransToBeTranslatedDone(ic);
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

static void Test_M88K_CPUComponent_Execute_Basic()
{
	GXemul gxemul;
	gxemul.GetCommandInterpreter().RunCommand("add testm88k");

	refcount_ptr<Component> cpu = gxemul.GetRootComponent()->LookupPath("root.machine0.mainbus0.cpu0");
	UnitTest::Assert("huh? no cpu?", !cpu.IsNULL());

	AddressDataBus* bus = cpu->AsAddressDataBus();
	UnitTest::Assert("cpu should be addressable", bus != NULL);

	// Place a hardcoded instruction in memory, and try to execute it.
	// addu r30, r31, 0x10
	uint32_t data32 = 0x63df0010;
	bus->AddressSelect(48);
	bus->WriteData(data32, BigEndian);

	bus->AddressSelect(52);
	bus->WriteData(data32, BigEndian);

	cpu->SetVariableValue("pc", "48");
	cpu->SetVariableValue("r30", "1234");
	cpu->SetVariableValue("r31", "5678");

	gxemul.SetRunState(GXemul::Running);
	gxemul.Execute(1);

	UnitTest::Assert("pc should have increased", cpu->GetVariable("pc")->ToInteger(), 52);
	UnitTest::Assert("r30 should have been modified", cpu->GetVariable("r30")->ToInteger(), 5678 + 0x10);
	UnitTest::Assert("r31 should not have been modified", cpu->GetVariable("r31")->ToInteger(), 5678);

	cpu->SetVariableValue("r31", "1111");

	gxemul.SetRunState(GXemul::SingleStepping);
	gxemul.Execute(1);

	UnitTest::Assert("pc should have increased again", cpu->GetVariable("pc")->ToInteger(), 56);
	UnitTest::Assert("r30 should have been modified again", cpu->GetVariable("r30")->ToInteger(), 1111 + 0x10);
}

UNITTESTS(M88K_CPUComponent)
{
	UNITTEST(Test_M88K_CPUComponent_IsStable);
	UNITTEST(Test_M88K_CPUComponent_Create);
	UNITTEST(Test_M88K_CPUComponent_IsCPU);
	UNITTEST(Test_M88K_CPUComponent_DefaultModel);

	// Disassembly:
	UNITTEST(Test_M88K_CPUComponent_Disassembly_Basic);

	// Dyntrans execution:
	UNITTEST(Test_M88K_CPUComponent_Execute_Basic);
}

#endif

