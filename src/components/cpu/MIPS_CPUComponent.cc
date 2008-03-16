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
 *
 *
 *  $Id: MIPS_CPUComponent.cc,v 1.2 2008/03/14 12:12:15 debug Exp $
 */

#include <assert.h>
#include "components/MIPS_CPUComponent.h"


MIPS_CPUComponent::MIPS_CPUComponent()
	: CPUComponent("mips_cpu")
{
	m_pc = 0xffffffffbfc00000ULL;
	m_pageSize = 4096;
}


refcount_ptr<Component> MIPS_CPUComponent::Create()
{
	return new MIPS_CPUComponent();
}


double MIPS_CPUComponent::GetCurrentFrequency() const
{
	return 100.0e6;
}


int MIPS_CPUComponent::Run(int nrOfCycles)
{
	int nrOfCyclesExecuted = 0;

	// Check for interrupts. TODO
	// This may cause an exception, i.e. a change of PC and other state.

	while (nrOfCycles-- > 0) {	
		// 1. Check if there is a native translation already for this
		//    address. If there is, then execute that block, and return.

		if (NativeTranslationExists()) {
			// TODO: Execute the native translation.
			assert(false);
			continue;
		}


		// 2. Read an instruction word from emulated memory.
		//
		//    a) Normally, the current PC page should be cached. If not,
		//       then look up the page. (This may cause exceptions, etc.,
		//       which need to be handled.)

		if (m_currentCodePage == NULL)
			LookupCurrentCodePage();

		//    b) Read the instruction word from the code page.

		int offsetInCodePage = m_pc & (m_pageSize - 1);
		bool mips16 = (offsetInCodePage & 1) != 0;
		uint32_t iword;
		if (mips16)
			iword = ((uint16_t *)m_currentCodePage)
			    [offsetInCodePage >> 1];
		else
			iword = m_currentCodePage[offsetInCodePage >> 2];

		//    c) Swap byte order within the instruction word, if the
		//       emulated CPU's byte order is not the same as the host's
		//       byte order:

		// TODO: LE32_TO_HOST or BE32_TO_HOST, depending on emulated cpu

		//    d) Update the emulated PC after the instruction has been
		//       read. Also, when going off the edge of a page, or when
		//       branching to a different page, the current code page
		//       should be reset to NULL.

		m_pc += mips16? sizeof(uint16_t) : sizeof(uint32_t);
		offsetInCodePage += mips16? sizeof(uint16_t) : sizeof(uint32_t);
		if (offsetInCodePage >= m_pageSize)
			m_currentCodePage = NULL;

		++ nrOfCyclesExecuted;


		// 3. Decode and execute the instruction.
		//
		//    Also, update statistics about nr of times this code has
		//    been executed. If it has reached a certain threshold,
		//    re-compile the code to native code. TODO

		if (mips16)
			ExecuteMIPS16Instruction(iword);
		else
			ExecuteInstruction(iword);
	}

	return nrOfCyclesExecuted;
}


void MIPS_CPUComponent::ExecuteMIPS16Instruction(uint16_t iword)
{
	// TODO: Mega switch/case.
}


void MIPS_CPUComponent::ExecuteInstruction(uint32_t iword)
{
	// TODO: Mega switch/case.
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

