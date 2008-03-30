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
#include "AddressDataBus.h"
#include "components/CPUComponent.h"


CPUComponent::CPUComponent(const string& className)
	: Component(className)
	, m_frequency(33.0e6)
	, m_pc(0)
	, m_endianness(BigEndian)
	, m_addressDataBus(NULL)
	, m_currentCodePage(NULL)
	, m_pageSize(0)
{
	AddVariableUInt64("pc", &m_pc);
	AddVariableDouble("frequency", &m_frequency);
	// TODO: Endianness as a variable?
}


refcount_ptr<Component> CPUComponent::Create()
{
	return NULL;
}


double CPUComponent::GetCurrentFrequency() const
{
        return m_frequency;
}


void CPUComponent::FlushCachedStateForComponent()
{
	m_addressDataBus = NULL;
	m_currentCodePage = NULL;
}


string CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "Base-class for all processors.";

	return Component::GetAttribute(attributeName);
}


void CPUComponent::LookupAddressDataBus()
{
	if (m_addressDataBus != NULL)
		return;

	// Find a suitable address data bus.
	AddressDataBus *bus = NULL;

	// 1) A direct first-level decendant of the CPU is probably a
	//    cache. Use this if it exists.
	Components& children = GetChildren();
	for (size_t i=0; i<children.size(); ++i) {
		bus = children[i]->AsAddressDataBus();
		if (bus != NULL)
			break;
	}

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

	if (m_addressDataBus == NULL) {
		std::cerr << "CPUComponent::LookupAddressDataBus: "
		    "No AddressDataBus to read from?\n";
		throw std::exception();
	}
}


bool CPUComponent::ReadInstructionWord(uint16_t& iword, uint64_t addr)
{
	LookupAddressDataBus();

	if (m_currentCodePage == NULL) {
		// First attempt: Try to look up the code page in memory,
		// so that we can read directly from it.

		// TODO
		// m_addressDataBus->LookupPage(addr); something
	}

	int offsetInCodePage = addr & (m_pageSize - 1);

	if (m_currentCodePage == NULL) {
		// If the lookup failed, read using the AddressDataBus
		// interface manually.
		m_addressDataBus->AddressSelect(addr);
		bool success = m_addressDataBus->ReadData(iword, m_endianness);
		
		if (!success)
			return false;
	} else {
		// Read directly from the page:
		uint16_t itmp = ((uint16_t *)m_currentCodePage)
		    [offsetInCodePage >> 1];

		if (m_endianness == BigEndian)
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


bool CPUComponent::ReadInstructionWord(uint32_t& iword, uint64_t addr)
{
	LookupAddressDataBus();

	if (m_currentCodePage == NULL) {
		// First attempt: Try to look up the code page in memory,
		// so that we can read directly from it.

		// TODO
		// m_addressDataBus->LookupPage(addr); something
	}

	int offsetInCodePage = addr & (m_pageSize - 1);

	if (m_currentCodePage == NULL) {
		// If the lookup failed, read using the AddressDataBus
		// interface manually.
		m_addressDataBus->AddressSelect(addr);
		bool success = m_addressDataBus->ReadData(iword, m_endianness);
		
		if (!success)
			return false;
	} else {
		// Read directly from the page:
		uint32_t itmp = ((uint32_t *)m_currentCodePage)
		    [offsetInCodePage >> 2];

		if (m_endianness == BigEndian)
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


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "ComponentFactory.h"

static void Test_CPUComponent_IsStable()
{
	UnitTest::Assert("the CPUComponent should be stable",
	    ComponentFactory::HasAttribute("cpu", "stable"));
}

static void Test_CPUComponent_Create()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("cpu");
	UnitTest::Assert("component was created?", cpu.IsNULL());
}

UNITTESTS(CPUComponent)
{
	UNITTEST(Test_CPUComponent_IsStable);
	UNITTEST(Test_CPUComponent_Create);
}

#endif

