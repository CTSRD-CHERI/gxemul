/*
 *  Copyright (C) 2008-2009  Anders Gavare.  All rights reserved.
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
#include <iomanip>

#include "AddressDataBus.h"
#include "components/CPUComponent.h"
#include "GXemul.h"


CPUComponent::CPUComponent(const string& className, const string& cpuArchitecture)
	: Component(className, "cpu")	// all cpus have "cpu" as their
					// visible class name, regardless of
					// their actual class name
	, m_frequency(33.0e6)
	, m_cpuArchitecture(cpuArchitecture)
	, m_pageSize(0)
	, m_pc(0)
	, m_lastDumpAddr(0)
	, m_lastUnassembleVaddr(0)
	, m_hasUsedUnassemble(false)
	, m_isBigEndian(true)
	, m_addressDataBus(NULL)
	, m_nextIC(NULL)
{
	AddVariable("architecture", &m_cpuArchitecture);
	AddVariable("pc", &m_pc);
	AddVariable("lastDumpAddr", &m_lastDumpAddr);
	AddVariable("lastUnassembleVaddr", &m_lastUnassembleVaddr);
	AddVariable("hasUsedUnassemble", &m_hasUsedUnassemble);
	AddVariable("frequency", &m_frequency);
	AddVariable("bigendian", &m_isBigEndian);
}


refcount_ptr<Component> CPUComponent::Create()
{
	return NULL;
}


double CPUComponent::GetCurrentFrequency() const
{
        return m_frequency;
}


CPUComponent * CPUComponent::AsCPUComponent()
{
        return this;
}


void CPUComponent::ResetState()
{
	m_hasUsedUnassemble = false;

	m_symbolRegistry.Clear();

	Component::ResetState();
}


void CPUComponent::DyntransInit()
{
	m_nextIC = NULL;
	m_ICpage = NULL;

	m_dyntransICshift = GetDyntransICshift();

	m_dyntransICentriesPerPage = m_pageSize >> m_dyntransICshift;
	m_dyntransPageMask = (m_pageSize - 1) - ((1 << m_dyntransICshift) - 1);
}


int CPUComponent::GetDyntransICshift() const
{
	std::cerr << "CPUComponent::GetDyntransICshift() must be overridden"
	    " by the specific CPU implementation.\n";
	throw std::exception();
}


void (*CPUComponent::GetDyntransToBeTranslated())(CPUComponent*, DyntransIC*) const
{
	std::cerr << "CPUComponent::GetDyntransToBeTranslated() must be overridden"
	    " by the specific CPU implementation.\n";
	throw std::exception();
}


/*
 * Dynamic translation core
 * ------------------------
 *
 * The core of GXemul's dynamic translation is a simple function call to
 * an entry in an array of pointers. Each call also moves the pointer of the
 * next function call to the next entry in the array. For most simple
 * instruction implementations, the instruction call pointer (m_nextIC) does
 * not have to be modified, because it is assumed that an instruction will
 * change the program counter to the next instruction.
 *
 * Before starting the main loop, the pc is used to look up the correct
 * m_nextIC value, by calling DyntransPCtoPointers().
 *
 * During the loop, the pc value is _not_ necessarily updated for each
 * instruction call. Instead, the low bits of the pc value should be considered
 * meaningless, and the offset of the m_nextIC pointer within the current
 * code page (pointed to by m_ICpage) defines the lowest pc bits.
 *
 * After completing the main loop, the pc value is resynched by calling
 * DyntransResyncPC().
 */
int CPUComponent::DyntransExecute(GXemul* gxemul, int nrOfCycles)
{
	if (gxemul->GetRunState() == GXemul::SingleStepping) {
		if (nrOfCycles != 1) {
			std::cerr << "Internal error: Single stepping,"
			    " but nrOfCycles = " << nrOfCycles << ".\n";
			throw std::exception();
		}

		stringstream disasm;
		Unassemble(1, false, m_pc, disasm);
		gxemul->GetUI()->ShowDebugMessage(this, disasm.str());
	}

	DyntransInit();

	DyntransPCtoPointers();

	struct DyntransIC *ic = m_nextIC;
	if (m_nextIC == NULL || m_ICpage == NULL) {
		std::cerr << "Internal error: m_nextIC or m_ICpage is NULL.\n";
		throw std::exception();
	}

	// TODO: Optimized calls if we're far enough away from a hazard,
	// so that we can run e.g. instruction combinations

	// TODO: Optimized loops of 120 IC calls...

	// TODO: Possibility to break out. These should not count as
	// executed cycles.

/*
 * The normal instruction execution core: Get the instruction call pointer
 * (and move the nextIC to the following instruction in advance), then
 * execute the instruction call by calling its f.
 */
#define IC	ic = m_nextIC ++; ic->f(this, ic);

	m_executedCycles = nrOfCycles;
	for (int i=0; i<nrOfCycles; i++) {
		IC
	}

	// TODO: ... then slowly execute the last few instructions.

	DyntransResyncPC();

	// If execution aborted, then reset the aborting instruction slot
	// to the to-be-translated function:
	if (m_nextIC->f == instr_abort)
		m_nextIC->f = GetDyntransToBeTranslated();

	return m_executedCycles;
}


void CPUComponent::DyntransPCtoPointers()
{
	// TODO. Page lookup.
	
	// TODO. If page lookup failed: allocate new page.

	// TODO: Move this to a helper function.
	// Fill the newly allocated page with suitable function pointers.
	if (m_dummyICpage.size() == 0) {
		m_dummyICpage.resize(m_dyntransICentriesPerPage + 2);

		// Fill the page with "to be translated" entries, which when
		// executed will read the instruction from memory, attempt to
		// translate it, and then execute it.
		void (*f)(CPUComponent*, DyntransIC*) = GetDyntransToBeTranslated();
		for (size_t i=0; i<m_dummyICpage.size(); ++i) {
			m_dummyICpage[i].f = f;
		}

		// ... and set the entries after the last instruction slot to
		// special "end of page" handlers.
		m_dummyICpage[m_dyntransICentriesPerPage + 0].f = CPUComponent::instr_endOfPage;
		m_dummyICpage[m_dyntransICentriesPerPage + 1].f = CPUComponent::instr_endOfPage2;
	}

	// TODO: m_dummyICpage, change to whatever was returned from the code above.
	m_ICpage = &m_dummyICpage[0];

	// Here, m_ICpage points to a valid page. Calculate m_nextIC from
	// the low bits of m_pc:
	int offsetWithinPage = (m_pc & m_dyntransPageMask) >> m_dyntransICshift;
	m_nextIC = m_ICpage + offsetWithinPage;
}


void CPUComponent::DyntransResyncPC()
{
	int offsetWithinICpage = (size_t)m_nextIC - (size_t)m_ICpage;
	int instructionIndex = offsetWithinICpage / sizeof(struct DyntransIC);

	// On a page with e.g. 1024 instruction slots, instructionIndex is usually
	// between 0 and 1023. This means that the PC points to within this
	// page.
	//
	// We synchronize the PC by clearing out the bits within the IC page,
	// and then adding the offset to the instruction.
	if (instructionIndex >= 0 && instructionIndex < m_dyntransICentriesPerPage) {
		m_pc &= ~m_dyntransPageMask;
		m_pc += (instructionIndex << m_dyntransICshift);
		return;
	}

	// However, the instruction index may point outside the IC page.
	// This happens when synching the PC just after the last instruction
	// on a page has been executed. This means that we set the PC to
	// the start of the next page.
	if (instructionIndex == m_dyntransICentriesPerPage) {
		m_pc &= ~m_dyntransPageMask;
		m_pc += (m_dyntransPageMask + (1 << m_dyntransICshift));
		return;
	}

	if (instructionIndex == m_dyntransICentriesPerPage + 1) {
		std::cerr << "TODO: DyntransResyncPC: Second end-of-page slot.\n";
		// This may happen for delay-slot architectures.
		throw std::exception();
	}

	// We can arrive here if m_nextIC pointed outside of the IC page.
	// This is ok if m_nextIC is a "break out" instruction call, for
	// early termination of the dyntrans core loop.

	// TODO: Check to make sure that it is the break out ic!
}


void CPUComponent::DyntransToBeTranslatedBegin(struct DyntransIC* ic)
{
	// Resynchronize the PC to the instruction currently being translated.
	// (m_nextIC should already have been increased, to point to the _next_
	// instruction slot.)
	m_nextIC = ic;
	DyntransResyncPC();

	// TODO: Check for m_pc breakpoints etc.

	// First, let's assume that the translation will fail.
	ic->f = NULL;
}


bool CPUComponent::DyntransReadInstruction(uint32_t& iword)
{
	// TODO: Fast lookup.

	AddressSelect(m_pc);
	bool readable = ReadData(iword, m_isBigEndian? BigEndian : LittleEndian);

	if (!readable) {
		UI* ui = GetUI();
		if (ui != NULL) {
			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction at 0x" << m_pc << " could not be read!";
			ui->ShowDebugMessage(this, ss.str());
		}
		return false;
	}

	return true;
}


void CPUComponent::DyntransToBeTranslatedDone(struct DyntransIC* ic)
{
	if (ic->f == NULL) {
		UI* ui = GetUI();
		if (ui != NULL) {
			bool isSingleStepping = GetRunningGXemulInstance()->GetRunState() == GXemul::SingleStepping;

			stringstream ss;
			ss.flags(std::ios::hex);
			ss << "instruction translation failed";

			// If we were single-stepping, then the instruction
			// disassembly has already been displayed. If we were
			// running in continuous mode, then we need to display
			// it now:
			if (!isSingleStepping) {
				ss << " at:\n";
				Unassemble(1, false, m_pc, ss);
			}

			ui->ShowDebugMessage(this, ss.str());
		}

		ic->f = instr_abort;
	}

	// Finally, execute the translated instruction.
	m_nextIC = ic + 1;
	ic->f(this, ic);
}


void CPUComponent::GetMethodNames(vector<string>& names) const
{
	// Add our method names...
	names.push_back("dump");
	names.push_back("registers");
	names.push_back("unassemble");

	// ... and make sure to call the base class implementation:
	Component::GetMethodNames(names);
}


bool CPUComponent::MethodMayBeReexecutedWithoutArgs(const string& methodName) const
{
	if (methodName == "dump")
		return true;

	if (methodName == "unassemble")
		return true;

	// ... and make sure to call the base class implementation:
	return Component::MethodMayBeReexecutedWithoutArgs(methodName);
}


void CPUComponent::ExecuteMethod(GXemul* gxemul, const string& methodName,
	const vector<string>& arguments)
{
	if (methodName == "dump") {
		uint64_t vaddr = m_lastDumpAddr;

		if (arguments.size() > 1) {
			gxemul->GetUI()->ShowDebugMessage("syntax: .dump [addr]\n");
			return;
		}

		if (arguments.size() == 1) {
			gxemul->GetUI()->ShowDebugMessage("TODO: parse address expression\n");
			gxemul->GetUI()->ShowDebugMessage("(for now, only hex immediate values are supported!)\n");

			stringstream ss;
			ss << arguments[0];
			ss.flags(std::ios::hex);
			ss >> vaddr;
		}

		const int nRows = 16;
		for (int i=0; i<nRows; i++) {
			const size_t len = 16;
			unsigned char data[len];
			bool readable[len];

			stringstream ss;
			ss.flags(std::ios::hex);

			if (vaddr > 0xffffffff)
				ss << std::setw(16);
			else
				ss << std::setw(8);

			ss << std::setfill('0') << vaddr;

			size_t k;
			for (k=0; k<len; ++k) {
				AddressSelect(vaddr + k);
				readable[k] = ReadData(data[k]);
			}

			ss << " ";

			for (k=0; k<len; ++k) {
				if ((k&3) == 0)
					ss << " ";

				ss << std::setw(2) << std::setfill('0');
				if (readable[k])
					ss << (int)data[k];
				else
					ss << "--";
			}

			ss << "  ";

			for (k=0; k<len; ++k) {
				char s[2];
				s[0] = data[k] >= 32 && data[k] < 127? data[k] : '.';
				s[1] = '\0';

				if (readable[k])
					ss << s;
				else
					ss << "-";
			}

			ss << "\n";

			gxemul->GetUI()->ShowDebugMessage(ss.str());

			vaddr += len;
		}

		m_lastDumpAddr = vaddr;

		return;
	}

	if (methodName == "registers") {
		ShowRegisters(gxemul, arguments);
		return;
	}

	if (methodName == "unassemble") {
		uint64_t vaddr = m_lastUnassembleVaddr;
		if (!m_hasUsedUnassemble)
			vaddr = m_pc;

		if (arguments.size() > 1) {
			gxemul->GetUI()->ShowDebugMessage("syntax: .unassemble [addr]\n");
			return;
		}

		if (arguments.size() == 1) {
			gxemul->GetUI()->ShowDebugMessage("TODO: parse address expression\n");
			gxemul->GetUI()->ShowDebugMessage("(for now, only hex immediate values are supported!)\n");

			stringstream ss;
			ss << arguments[0];
			ss.flags(std::ios::hex);
			ss >> vaddr;
		}

		const int nRows = 20;

		stringstream output;
		vaddr = Unassemble(nRows, true, vaddr, output);
		gxemul->GetUI()->ShowDebugMessage(output.str());

		m_hasUsedUnassemble = true;
		m_lastUnassembleVaddr = vaddr;
		return;
	}

	// Call base...
	Component::ExecuteMethod(gxemul, methodName, arguments);
}


uint64_t CPUComponent::Unassemble(int nRows, bool indicatePC, uint64_t vaddr, ostream& output)
{
	vector< vector<string> > outputRows;

	for (int i=0; i<nRows; i++) {
		outputRows.push_back(vector<string>());

		// TODO: GENERALIZE! Some archs will have longer
		// instructions, or unaligned, or over page boundaries!
		const size_t maxLen = sizeof(uint32_t);
		unsigned char instruction[maxLen];

		bool readOk = true;
		for (size_t k=0; k<maxLen; ++k) {
			AddressSelect(vaddr + k);
			readOk &= ReadData(instruction[k]);
		}

		string symbol = GetSymbolRegistry().LookupAddress(vaddr, false);
		if (symbol != "") {
			outputRows[outputRows.size()-1].push_back("<" + symbol + ">");
			outputRows.push_back(vector<string>());
		}

		stringstream ss;
		ss.flags(std::ios::hex | std::ios::showbase);
		ss << vaddr;

		if (indicatePC && m_pc == vaddr)
			ss << " <- ";
		else
			ss << "    ";

		outputRows[outputRows.size()-1].push_back(ss.str());

		if (!readOk) {
			stringstream ss2;
			ss2 << "\tmemory could not be read";
			if (m_addressDataBus == NULL)
				ss2 << "; no address/data bus connected to the CPU";
			ss2 << "\n";

			outputRows[outputRows.size()-1].push_back(ss2.str());
			break;
		} else {
			vector<string> result;

			size_t len = DisassembleInstruction(vaddr,
			    maxLen, instruction, result);
			vaddr += len;

			for (size_t j=0; j<result.size(); ++j)
				outputRows[outputRows.size()-1].push_back(result[j]);
		}
	}

	// Output the rows with equal-width columns:
	vector<size_t> columnWidths;
	size_t row;
	for (row=0; row<outputRows.size(); ++row) {
		size_t nColumns = outputRows[row].size();

		// Skip lines such as "<symbol>" on empty lines, when
		// calculating column width.
		if (nColumns <= 1)
			continue;

		if (columnWidths.size() < nColumns)
			columnWidths.resize(nColumns);

		for (size_t col=0; col<nColumns; ++col) {
			const string& s = outputRows[row][col];
			if (s.length() > columnWidths[col])
				columnWidths[col] = s.length();
		}
	}

	for (row=0; row<outputRows.size(); ++row) {
		const vector<string>& rowVector = outputRows[row];

		for (size_t i=0; i<rowVector.size(); ++i) {
			// Note: i>=2 because:
			// index 0 is the first column, no spaces before that one,
			// but also: index 1 because the spaces after the vaddr
			// is a special case ("<-" pc indicator).
			if (i >= 2)
				output << "   ";

			size_t len = rowVector[i].length();
			output << rowVector[i];
			
			int nspaces = columnWidths[i] - len;
			for (int j=0; j<nspaces; ++j)
				output << " ";
		}

		output << "\n";
	}

	return vaddr;
}


AddressDataBus * CPUComponent::AsAddressDataBus()
{
        return this;
}


void CPUComponent::FlushCachedStateForComponent()
{
	m_addressDataBus = NULL;
}


bool CPUComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	// If AddressDataBus lookup fails, then the CPU fails.
	if (!LookupAddressDataBus(gxemul)) {
		gxemul->GetUI()->ShowDebugMessage(this, "this CPU"
		    " has neither any child components nor any parent component"
		    " that can act as address/data bus, so there is no place"
		    " to read instructions from\n");
		return false;
	}

	return true;
}


string CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "Base-class for all processors.";

	return Component::GetAttribute(attributeName);
}


bool CPUComponent::LookupAddressDataBus(GXemul* gxemul)
{
	if (m_addressDataBus != NULL)
		return true;

	// Find a suitable address data bus.
	AddressDataBus *bus = NULL;

	// 1) A direct first-level decendant of the CPU is probably a
	//    cache. Use this if it exists.
	//    If there are multiple AddressDataBus capable children,
	//    print a debug warning, and just choose any of the children
	//    (the last one).
	Components& children = GetChildren();
	Component* choosenChild = NULL;
	bool multipleChildBussesFound = false;
	for (size_t i=0; i<children.size(); ++i) {
		AddressDataBus *childBus = children[i]->AsAddressDataBus();
		if (childBus != NULL) {
			if (bus != NULL)
				multipleChildBussesFound = true;
			bus = childBus;
			choosenChild = children[i];
		}
	}

	if (multipleChildBussesFound && gxemul != NULL)
		gxemul->GetUI()->ShowDebugMessage(this, "warning: this CPU has "
		    "multiple child components that can act as address/data busses; "
		    "using " + choosenChild->GenerateShortestPossiblePath() + "\n");

	// 2) If no cache exists, go to a parent bus (usually a mainbus).
	if (bus == NULL) {
		refcount_ptr<Component> component = GetParent();
		while (!component.IsNULL()) {
			bus = component->AsAddressDataBus();
			if (bus != NULL)
				break;
			component = component->GetParent();
		}
	}

	m_addressDataBus = bus;

	return m_addressDataBus != NULL;
}


void CPUComponent::ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const
{
	gxemul->GetUI()->ShowDebugMessage("The registers method has not yet "
	    "been implemented for this CPU type. TODO.\n");
}


void CPUComponent::AddressSelect(uint64_t address)
{
	m_addressSelect = address;
}


bool CPUComponent::ReadData(uint8_t& data)
{
	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->ReadData(data);
}


bool CPUComponent::ReadData(uint16_t& data, Endianness endianness)
{
	assert((m_addressSelect & 1) == 0);

	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->ReadData(data, endianness);
}


bool CPUComponent::ReadData(uint32_t& data, Endianness endianness)
{
	assert((m_addressSelect & 3) == 0);

	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->ReadData(data, endianness);
}


bool CPUComponent::ReadData(uint64_t& data, Endianness endianness)
{
	assert((m_addressSelect & 7) == 0);

	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->ReadData(data, endianness);
}


bool CPUComponent::WriteData(const uint8_t& data)
{
	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->WriteData(data);
}


bool CPUComponent::WriteData(const uint16_t& data, Endianness endianness)
{
	assert((m_addressSelect & 1) == 0);

	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->WriteData(data, endianness);
}


bool CPUComponent::WriteData(const uint32_t& data, Endianness endianness)
{
	assert((m_addressSelect & 3) == 0);

	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->WriteData(data, endianness);
}


bool CPUComponent::WriteData(const uint64_t& data, Endianness endianness)
{
	assert((m_addressSelect & 7) == 0);

	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(m_addressSelect, paddr, writable);

	m_addressDataBus->AddressSelect(paddr);
	return m_addressDataBus->WriteData(data, endianness);
}


/*****************************************************************************/


/*
 * A do-nothing instruction. (It still counts as a cylce, though.)
 */
DYNTRANS_INSTR(CPUComponent,nop)
{
}


/*
 * A break-out-of-dyntrans function. Setting ic->f to this function will
 * cause dyntrans execution to be aborted. The cycle counter will _not_
 * count this as executed cycles.
 */
DYNTRANS_INSTR(CPUComponent,abort)
{
	// Cycle reduction:
	-- cpubase->m_executedCycles;

	cpubase->m_nextIC = ic;
}


DYNTRANS_INSTR(CPUComponent,endOfPage)
{
	std::cerr << "TODO: endOfPage\n";
	throw std::exception();
}


DYNTRANS_INSTR(CPUComponent,endOfPage2)
{
	std::cerr << "TODO: endOfPage2\n";
	throw std::exception();
}


/*
 * arg 0: 64-bit register
 * arg 1: 32-bit signed immediate
 *
 * Sets the register at arg 0 to the immediate value in arg 1.
 */
DYNTRANS_INSTR(CPUComponent,set_u64_imms32)
{
	REG64(ic->arg[0]) = (int32_t) ic->arg[1];
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * Adds the unsigned immediate to arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,add_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) + (uint32_t)ic->arg[2];
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * Adds arg 1 and arg 2, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,add_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) + REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit signed immediate
 *
 * Adds the signed immediate to arg 1, and stores the result in arg 0, truncated
 * to a signed 32-bit value.
 */
DYNTRANS_INSTR(CPUComponent,add_u64_u64_imms32_truncS32)
{
	REG64(ic->arg[0]) = (int32_t) (REG64(ic->arg[1]) + (int32_t)ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit signed immediate
 *
 * Adds the signed immediate to arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,add_u64_u64_imms32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) + (int64_t)(int32_t)ic->arg[2];
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * Subtracts the unsigned immediate from arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,sub_u32_u32_immu32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) - (uint32_t)ic->arg[2];
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * Subtracts arg 2 from arg 1, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,sub_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) - REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * ANDs the 32-bit immediate into arg 1, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0xffffffff80001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0x0000000080001200 (note: the
 * upper bits are not sign-extended from bit 31).
 */
DYNTRANS_INSTR(CPUComponent,and_u64_u64_immu32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) & (uint32_t)ic->arg[2];
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * ORs arg 1 and arg 2 together, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,or_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) | REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * ORs the 32-bit immediate into arg 1, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0x0000000000001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0x0000000080001234 (note: the
 * upper bits are not sign extended from bit 31).
 */
DYNTRANS_INSTR(CPUComponent,or_u64_u64_immu32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) | (uint32_t)ic->arg[2];
}


/*
 * arg 0: 32-bit register
 * arg 1: 32-bit register
 * arg 2: 32-bit register
 *
 * XORs arg 1 and arg 2, and stores the result in arg 0.
 */
DYNTRANS_INSTR(CPUComponent,xor_u32_u32_u32)
{
	REG32(ic->arg[0]) = REG32(ic->arg[1]) ^ REG32(ic->arg[2]);
}


/*
 * arg 0: 64-bit register
 * arg 1: 64-bit register
 * arg 2: 32-bit unsigned immediate
 *
 * XORs the 32-bit immediate into arg 1, storing the result in arg 0.
 *
 * Note: No sign truncation is performed, i.e. if arg 1 is 0xffffffff80001234
 * and arg 2 is 0x80001200, then arg 0 becomes 0xffffffff00000034 (note: the
 * upper bits are not sign-extended from bit 31).
 */
DYNTRANS_INSTR(CPUComponent,xor_u64_u64_immu32)
{
	REG64(ic->arg[0]) = REG64(ic->arg[1]) ^ (uint32_t)ic->arg[2];
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_CPUComponent_IsStable()
{
	UnitTest::Assert("the CPUComponent should be stable",
	    ComponentFactory::HasAttribute("cpu", "stable"));
}

static void Test_CPUComponent_Create()
{
	// CPUComponent is abstract, and should not be possible to create.
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("cpu");
	UnitTest::Assert("component was created?", cpu.IsNULL());
}

static void Test_CPUComponent_PreRunCheck()
{
	GXemul gxemul;

	// Attempting to run a cpu with nothing connected to it should FAIL!
	gxemul.GetCommandInterpreter().RunCommand("add mips_cpu");
	UnitTest::Assert("preruncheck should fail",
	    gxemul.GetRootComponent()->PreRunCheck(&gxemul) == false);

	// Running a CPU with RAM should however succeed:
	gxemul.GetCommandInterpreter().RunCommand("add ram cpu0");
	UnitTest::Assert("preruncheck should succeed",
	    gxemul.GetRootComponent()->PreRunCheck(&gxemul) == true);
}

static void Test_CPUComponent_Methods_Reexecutableness()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("mips_cpu");

	UnitTest::Assert("dump method SHOULD be re-executable"
	    " without args", cpu->MethodMayBeReexecutedWithoutArgs("dump") == true);

	UnitTest::Assert("registers method should NOT be re-executable"
	    " without args", cpu->MethodMayBeReexecutedWithoutArgs("registers") == false);

	UnitTest::Assert("unassemble method SHOULD be re-executable"
	    " without args", cpu->MethodMayBeReexecutedWithoutArgs("unassemble") == true);

	UnitTest::Assert("nonexistant method should NOT be re-executable"
	    " without args", cpu->MethodMayBeReexecutedWithoutArgs("nonexistant") == false);
}

static void Test_CPUComponent_ResetSteps()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("mips_cpu");

	cpu->SetVariableValue("step", "42");
	UnitTest::Assert("steps should be 42", cpu->GetVariable("step")->ToInteger(), 42);

	cpu->Reset();
	UnitTest::Assert("steps should be 0", cpu->GetVariable("step")->ToInteger(), 0);
}

static void Test_CPUComponent_Dyntrans_PreReq()
{
	struct DyntransIC ic;

	UnitTest::Assert("nr of dyntrans args too few", N_DYNTRANS_IC_ARGS >= 3);
	UnitTest::Assert("size of dyntrans args to small", sizeof(ic.arg[0]) >= sizeof(uint32_t));
}

UNITTESTS(CPUComponent)
{
	UNITTEST(Test_CPUComponent_IsStable);
	UNITTEST(Test_CPUComponent_Create);
	UNITTEST(Test_CPUComponent_PreRunCheck);
	UNITTEST(Test_CPUComponent_Methods_Reexecutableness);
	UNITTEST(Test_CPUComponent_ResetSteps);

	// Dyntrans:
	UNITTEST(Test_CPUComponent_Dyntrans_PreReq);
}

#endif

