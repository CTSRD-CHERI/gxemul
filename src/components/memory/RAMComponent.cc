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

#include "components/RAMComponent.h"


RAMComponent::RAMComponent()
	: Component("ram")
{
}


refcount_ptr<Component> RAMComponent::Create()
{
	return new RAMComponent();
}


string RAMComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "A generic RAM component.";

	return Component::GetAttribute(attributeName);
}


AddressDataBus* RAMComponent::AsAddressDataBus()
{
	return this;
}


void RAMComponent::AddressSelect(uint64_t address)
{
}

void RAMComponent::ReadData(uint8_t& data)
{
}

void RAMComponent::ReadData(uint16_t& data)
{
}

void RAMComponent::ReadData(uint32_t& data)
{
}

void RAMComponent::ReadData(uint64_t& data)
{
}

void RAMComponent::WriteData(uint8_t& data)
{
}

void RAMComponent::WriteData(uint16_t& data)
{
}

void RAMComponent::WriteData(uint32_t& data)
{
}

void RAMComponent::WriteData(uint64_t& data)
{
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "ComponentFactory.h"

static void Test_RAMComponent_IsStable()
{
	UnitTest::Assert("the RAMComponent should be stable",
	    ComponentFactory::HasAttribute("ram", "stable"));
}

static void Test_RAMComponent_AddressDataBus()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");

	AddressDataBus* bus = ram->AsAddressDataBus();
	UnitTest::Assert("addressdatabus expected", bus != NULL);
}

static void Test_RAMComponent_InitiallyZero()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(0);

	// By default, RAM should be zero-filled:
	uint8_t data8 = 42;
	bus->ReadData(data8);
	UnitTest::Assert("memory should be zero filled", data8, 0);

	uint16_t data16 = 142;
	bus->ReadData(data16);
	UnitTest::Assert("memory should be zero filled (16)", data16, 0);

	uint32_t data32 = 342;
	bus->ReadData(data32);
	UnitTest::Assert("memory should be zero filled (32)", data32, 0);

	uint64_t data64 = 942;
	bus->ReadData(data64);
	UnitTest::Assert("memory should be zero filled (64)", data64, 0);

	bus->AddressSelect(0x10000);

	data8 = 43;
	bus->ReadData(data8);
	UnitTest::Assert("B: memory should be zero filled", data8, 0);

	data16 = 143;
	bus->ReadData(data16);
	UnitTest::Assert("B: memory should be zero filled (16)", data16, 0);

	data32 = 343;
	bus->ReadData(data32);
	UnitTest::Assert("B: memory should be zero filled (32)", data32, 0);

	data64 = 943;
	bus->ReadData(data64);
	UnitTest::Assert("B: memory should be zero filled (64)", data64, 0);
}

UNITTESTS(RAMComponent)
{
	UNITTEST(Test_RAMComponent_IsStable);
	UNITTEST(Test_RAMComponent_AddressDataBus);
	UNITTEST(Test_RAMComponent_InitiallyZero);

	// TODO: Write + readback tests etc.
}

#endif

