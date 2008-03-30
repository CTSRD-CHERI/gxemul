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

#include "components/MainbusComponent.h"


MainbusComponent::MainbusComponent()
	: Component("mainbus")
	, m_memoryMap(NULL)
	, m_currentAddressDataBus(NULL)
{
}


MainbusComponent::~MainbusComponent()
{
	if (m_memoryMap != NULL) {
		delete m_memoryMap;
		m_memoryMap = NULL;
	}
}


refcount_ptr<Component> MainbusComponent::Create()
{
	return new MainbusComponent();
}


string MainbusComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "A generic main bus.";

	return Component::GetAttribute(attributeName);
}


void MainbusComponent::FlushCachedStateForComponent()
{
	if (m_memoryMap != NULL) {
		delete m_memoryMap;
		m_memoryMap = NULL;
	}

	m_currentAddressDataBus = NULL;
}


void MainbusComponent::MakeSureMemoryMapExists()
{
	if (m_memoryMap != NULL)
		return;

	m_memoryMap = new MemoryMap;

	// Build a memory map of all immediate children, who implement the
	// AddressDataBus interface:
	Components children = GetChildren();
	for (size_t i=0; i<children.size(); ++i) {
		AddressDataBus* bus = children[i]->AsAddressDataBus();
		if (bus != NULL) {
			MemoryMapEntry mmEntry;
			mmEntry.addressDataBus = bus;
			mmEntry.addrMul = 1;
			mmEntry.base = 0;

			const StateVariable* varBase =
			    children[i]->GetVariable("memoryMappedBase");
			const StateVariable* varSize =
			    children[i]->GetVariable("memoryMappedSize");
			const StateVariable* varAddrMul =
			    children[i]->GetVariable("memoryMappedAddrMul");

			if (varBase != NULL && !varBase->ToString().empty()) {
				stringstream tmpss;
				tmpss << varBase->ToString();
				tmpss >> mmEntry.base;
			}
			if (varSize != NULL && !varSize->ToString().empty()) {
				stringstream tmpss;
				tmpss << varSize->ToString();
				tmpss >> mmEntry.size;
			}
			if (varAddrMul != NULL && !varAddrMul->ToString()
			    .empty()) {
				stringstream tmpss;
				tmpss << varAddrMul->ToString();
				tmpss >> mmEntry.addrMul;
			}

			if (varSize == NULL || varBase == NULL) {
				std::cerr << "No base or size? TODO.\n";
				throw std::exception();
			}

			m_memoryMap->push_back(mmEntry);

			// TODO: Check for overlaps!
		}
	}
}


AddressDataBus* MainbusComponent::AsAddressDataBus()
{
	return this;
}


void MainbusComponent::AddressSelect(uint64_t address)
{
	MakeSureMemoryMapExists();

	m_currentAddressDataBus = NULL;

	for (size_t i=0; i<m_memoryMap->size(); ++i) {
		MemoryMapEntry& mmEntry = (*m_memoryMap)[i];
		if (address >= mmEntry.base &&
		    address < mmEntry.base + mmEntry.size) {
			m_currentAddressDataBus = mmEntry.addressDataBus;
			m_currentAddressDataBus->AddressSelect(
			    (address - mmEntry.base) / mmEntry.addrMul);
			break;
		}
	}
}


bool MainbusComponent::ReadData(uint8_t& data)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data);
	else
		return false;
}


bool MainbusComponent::ReadData(uint16_t& data, Endianness endianness)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::ReadData(uint32_t& data, Endianness endianness)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::ReadData(uint64_t& data, Endianness endianness)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->ReadData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint8_t& data)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint16_t& data, Endianness endianness)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint32_t& data, Endianness endianness)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


bool MainbusComponent::WriteData(const uint64_t& data, Endianness endianness)
{
	MakeSureMemoryMapExists();

	if (m_currentAddressDataBus != NULL)
		return m_currentAddressDataBus->WriteData(data, endianness);
	else
		return false;
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "ComponentFactory.h"

static void Test_MainbusComponent_IsStable()
{
	UnitTest::Assert("the MainbusComponent should be stable",
	    ComponentFactory::HasAttribute("mainbus", "stable"));
}

static void Test_MainbusComponent_AddressDataBus()
{
	refcount_ptr<Component> mainbus =
	    ComponentFactory::CreateComponent("mainbus");

	AddressDataBus* bus = mainbus->AsAddressDataBus();
	UnitTest::Assert("The MainbusComponent should implement the "
	    "AddressDataBus interface", bus != NULL);
}

UNITTESTS(MainbusComponent)
{
	UNITTEST(Test_MainbusComponent_IsStable);
	UNITTEST(Test_MainbusComponent_AddressDataBus);
}

#endif

