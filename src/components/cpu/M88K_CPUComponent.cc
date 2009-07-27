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



M88K_CPUComponent::M88K_CPUComponent()
	: CPUComponent("m88k_cpu", "Motorola 88000")
	, m_m88k_type("88100")
{
	ResetState();

	AddVariable("model", &m_m88k_type);

	for (size_t i=0; i<N_M88K_REGS; i++) {
		stringstream ss;
		ss << "r" << i;
		AddVariable(ss.str(), &m_r[i]);
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

	for (size_t i=0; i<N_M88K_REGS; i++)
		m_r[i] = 0;

	m_pc = 0;

	CPUComponent::ResetState();
}


bool M88K_CPUComponent::PreRunCheckForComponent(GXemul* gxemul)
{
	if (m_r[0] != 0) {
		gxemul->GetUI()->ShowDebugMessage(this, "the r0 register "
		    "must contain the value 0.\n");
		return false;
	}

	return CPUComponent::PreRunCheckForComponent(gxemul);
}


void M88K_CPUComponent::ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const
{
	stringstream ss;
	ss.flags(std::ios::hex);
	ss << " pc = 0x" << std::setfill('0') << std::setw(8) << m_pc << "\n";
	// TODO: Symbol lookup

	for (size_t i=0; i<N_M88K_REGS; i++) {
		stringstream regname;
		regname << "r" << i;
		
		ss << std::setfill(' ');
		ss << std::setw(3) << regname.str() << " = 0x";
		ss << std::setfill('0') << std::setw(8) << m_r[i];
		if ((i&3) == 3)
			ss << "\n";
		else
			ss << "  ";
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

#if 0
int M88K_CPUComponent::Run(int nrOfCycles)
{
	int nrOfCyclesExecuted = 0;

	// Check for interrupts. TODO
	// (This may cause an exception, i.e. a change of PC and other state.)

	while (nrOfCycles-- > 0) {
		// Read an instruction from emulated memory and execute it:
		uint32_t iword;
		if (!ReadInstructionWord(iword, m_pc)) {
			std::cerr << "TODO: M88K: no instruction?\n";
			throw std::exception();
		}
		ExecuteInstruction(iword);

		++ nrOfCyclesExecuted;
	}

	return nrOfCyclesExecuted;
}
#endif


void M88K_CPUComponent::ExecuteInstruction(uint32_t iword)
{
	// TODO: switch/case for all instructions
	std::cerr.flags(std::ios::hex | std::ios::showbase);	
	std::cerr << "EXECUTE iword32 " << iword << " at pc " << m_pc << "\n";
	std::cerr << "TODO: M88K: unimplemented instruction\n";
	throw std::exception();
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
	uint32_t iword = *((uint32_t *) instruction);
	if (m_isBigEndian)
		iword = BE32_TO_HOST(iword);
	else
		iword = LE32_TO_HOST(iword);

	// ... and add it to the result:
	char tmp[9];
	snprintf(tmp, sizeof(tmp), "%08x", (int) iword);
	result.push_back(tmp);

	int hi6 = iword >> 26;

	switch (hi6) {


	default:
		{
			stringstream ss;
			ss << "unimplemented instruction: "; // << hi6_names[hi6];
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

