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
 */

#include <assert.h>
#include "components/MIPS_CPUComponent.h"


MIPS_CPUComponent::MIPS_CPUComponent()
	: CPUComponent("mips_cpu")
{
	// MIPS CPUs are hardwired to start at 0xffffffffbfc00000:
	m_pc = (int32_t) 0xbfc00000;

	// Most MIPS CPUs use 4 KB native page size.
	// TODO: A few use 1 KB pages; this should be supported as well.
	m_pageSize = 4096;

	m_frequency = 100e6;
}


refcount_ptr<Component> MIPS_CPUComponent::Create()
{
	return new MIPS_CPUComponent();
}


int MIPS_CPUComponent::Run(int nrOfCycles)
{
	int nrOfCyclesExecuted = 0;

	// Check for interrupts. TODO
	// (This may cause an exception, i.e. a change of PC and other state.)

	while (nrOfCycles-- > 0) {
		bool mips16 = (m_pc & 1) != 0;
		bool instructionWasRead;

		// TODO: Virtual to physical memory translation.

		// Read an instruction from emulated memory and execute it:
		if (mips16) {
			uint16_t iword;
			instructionWasRead = ReadInstructionWord(
			    iword, m_pc & ~1);
			ExecuteMIPS16Instruction(iword);
		} else {
			uint32_t iword;
			instructionWasRead = ReadInstructionWord(iword, m_pc);
			ExecuteInstruction(iword);
		}

		if (!instructionWasRead) {
			std::cerr << "TODO: MIPS: no instruction\n";
			throw std::exception();
		}

		++ nrOfCyclesExecuted;
	}

	return nrOfCyclesExecuted;
}


void MIPS_CPUComponent::ExecuteMIPS16Instruction(uint16_t iword)
{
	// TODO: switch/case for all instructions
	std::cout << "EXECUTE iword16 " << iword << " at pc " << m_pc << "\n";
}


void MIPS_CPUComponent::ExecuteInstruction(uint32_t iword)
{
	// TODO: switch/case for all instructions
	std::cout << "EXECUTE iword32 " << iword << " at pc " << m_pc << "\n";
}


bool MIPS_CPUComponent::VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	bool& writable)
{
	// TODO. For now, just return the lowest 30 bits.

	paddr = vaddr & 0x3fffffff;
	writable = true;
	return true;
}


string MIPS_CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "MIPS processor.";

	return Component::GetAttribute(attributeName);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "ComponentFactory.h"

static void Test_MIPS_CPUComponent_IsStable()
{
	UnitTest::Assert("the MIPS_CPUComponent should be stable",
	    ComponentFactory::HasAttribute("mips_cpu", "stable"));
}

static void Test_MIPS_CPUComponent_Create()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("mips_cpu");
	UnitTest::Assert("component was not created?", !cpu.IsNULL());

	const StateVariable * p = cpu->GetVariable("pc");
	UnitTest::Assert("cpu has no pc state variable?", p != NULL);
	UnitTest::Assert("initial pc", p->ToString(), "18446744072631615488");
}

UNITTESTS(MIPS_CPUComponent)
{
	UNITTEST(Test_MIPS_CPUComponent_IsStable);
	UNITTEST(Test_MIPS_CPUComponent_Create);
}

#endif

