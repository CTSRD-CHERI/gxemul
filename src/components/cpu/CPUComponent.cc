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
	, m_currentCodePage(NULL)
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
		for (int i=0; i<nRows; i++) {
			// TODO: GENERALIZE! Some archs will have longer
			// instructions, or unaligned, or over page boundaries!
			const size_t maxLen = sizeof(uint32_t);
			unsigned char instruction[maxLen];
			vector<string> result;

			bool readOk = true;			
			for (size_t k=0; k<maxLen; ++k) {
				AddressSelect(vaddr + k);
				readOk &= ReadData(instruction[k]);
			}
			
			stringstream ss;
			ss.flags(std::ios::hex | std::ios::showbase);
			ss << vaddr;

			if (!readOk) {
				ss << "\tmemory could not be read";
				if (m_addressDataBus == NULL)
					ss << "; no address/data bus connected to the CPU?";
				ss << "\n";
				gxemul->GetUI()->ShowDebugMessage(ss.str());
				break;
			} else {
				size_t len = DisassembleInstruction(vaddr,
				    maxLen, instruction, result);
				vaddr += len;

				// TODO: Rewrite this to gather all nRows of
				// output, and then output with equal-width
				// columns!
				for (size_t j=0; j<result.size(); ++j)
					ss << "\t" << result[j];
				ss << "\n";
				gxemul->GetUI()->ShowDebugMessage(ss.str());
			}
		}

		m_hasUsedUnassemble = true;
		m_lastUnassembleVaddr = vaddr;
		return;
	}

	// Call base...
	Component::ExecuteMethod(gxemul, methodName, arguments);
}


AddressDataBus * CPUComponent::AsAddressDataBus()
{
        return this;
}


void CPUComponent::FlushCachedStateForComponent()
{
	m_addressDataBus = NULL;
	m_currentCodePage = NULL;
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


bool CPUComponent::ReadInstructionWord(uint16_t& iword, uint64_t vaddr)
{
	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(vaddr, paddr, writable);

	if (m_currentCodePage == NULL) {
		// First attempt: Try to look up the code page in memory,
		// so that we can read directly from it.

		// TODO
		// m_addressDataBus->LookupPage(addr); something
	}

	int offsetInCodePage = vaddr & (m_pageSize - 1);

	if (m_currentCodePage == NULL) {
		// If the lookup failed, read using the AddressDataBus
		// interface manually.
		m_addressDataBus->AddressSelect(paddr);
		bool success = m_addressDataBus->ReadData(iword,
		    m_isBigEndian? BigEndian : LittleEndian);
		
		if (!success)
			return false;
	} else {
		// Read directly from the page:
		uint16_t itmp = ((uint16_t *)m_currentCodePage)
		    [offsetInCodePage >> 1];

		if (m_isBigEndian)
			iword = BE16_TO_HOST(itmp);
		else
			iword = LE16_TO_HOST(itmp);
	}

	// Update the emulated PC after the instruction has been
	// read. Also, when going off the edge of a page, or when
	// branching to a different page, the current code page
	// should be reset to NULL.
	m_pc += sizeof(iword);
	offsetInCodePage += sizeof(iword);
	if (offsetInCodePage >= m_pageSize)
		m_currentCodePage = NULL;

	return true;
}


bool CPUComponent::ReadInstructionWord(uint32_t& iword, uint64_t vaddr)
{
	if (!LookupAddressDataBus())
		return false;

	uint64_t paddr;
	bool writable;
	VirtualToPhysical(vaddr, paddr, writable);

	if (m_currentCodePage == NULL) {
		// First attempt: Try to look up the code page in memory,
		// so that we can read directly from it.

		// TODO
		// m_addressDataBus->LookupPage(addr); something
	}

	int offsetInCodePage = vaddr & (m_pageSize - 1);

	if (m_currentCodePage == NULL) {
		// If the lookup failed, read using the AddressDataBus
		// interface manually.
		m_addressDataBus->AddressSelect(paddr);
		bool success = m_addressDataBus->ReadData(iword,
		    m_isBigEndian? BigEndian : LittleEndian);
		
		if (!success)
			return false;
	} else {
		// Read directly from the page:
		uint32_t itmp = ((uint32_t *)m_currentCodePage)
		    [offsetInCodePage >> 2];

		if (m_isBigEndian)
			iword = BE32_TO_HOST(itmp);
		else
			iword = LE32_TO_HOST(itmp);
	}

	// Update the emulated PC after the instruction has been
	// read. Also, when going off the edge of a page, or when
	// branching to a different page, the current code page
	// should be reset to NULL.
	m_pc += sizeof(iword);
	offsetInCodePage += sizeof(iword);
	if (offsetInCodePage >= m_pageSize)
		m_currentCodePage = NULL;
	
	return true;
}


bool CPUComponent::VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
   bool& writable)
{
	// Dummy implementation, return physical address = virtual address.
	paddr = vaddr;
	writable = true;
	return true;
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

UNITTESTS(CPUComponent)
{
	UNITTEST(Test_CPUComponent_IsStable);
	UNITTEST(Test_CPUComponent_Create);
	UNITTEST(Test_CPUComponent_PreRunCheck);
	UNITTEST(Test_CPUComponent_Methods_Reexecutableness);
}

#endif

