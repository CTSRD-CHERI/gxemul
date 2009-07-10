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

#include "components/RAMComponent.h"

#include <assert.h>
#include <sys/mman.h>


RAMComponent::RAMComponent(const string& visibleClassName)
	: MemoryMappedComponent("ram", visibleClassName)
	, m_blockSizeShift(22)		// 22 = 4 MB per block
	, m_blockSize(1 << m_blockSizeShift)
	, m_writeProtected(false)
	, m_addressSelect(0)
	, m_selectedHostMemoryBlock(NULL)
	, m_selectedOffsetWithinBlock(0)
{
	AddVariable("writeProtect", &m_writeProtected);
}


RAMComponent::~RAMComponent()
{
	for (size_t i=0; i<m_memoryBlocks.size(); ++i) {
		if (m_memoryBlocks[i] != NULL)
			munmap(m_memoryBlocks[i], m_blockSize);
	}
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

	return MemoryMappedComponent::GetAttribute(attributeName);
}


AddressDataBus* RAMComponent::AsAddressDataBus()
{
	return this;
}


void RAMComponent::AddressSelect(uint64_t address)
{
	m_addressSelect = address;

	uint64_t blockNr = address >> m_blockSizeShift;

	if (blockNr+1 > m_memoryBlocks.size())
		m_selectedHostMemoryBlock = NULL;
	else
		m_selectedHostMemoryBlock = m_memoryBlocks[blockNr];

	m_selectedOffsetWithinBlock = address & (m_blockSize-1);
}


void* RAMComponent::AllocateBlock()
{
	void * p = mmap(NULL, m_blockSize, PROT_WRITE | PROT_READ | PROT_EXEC,
	    MAP_ANON | MAP_PRIVATE, -1, 0);

	if (p == MAP_FAILED || p == NULL) {
		std::cerr << "RAMComponent::AllocateBlock: Could not allocate "
		    << m_blockSize << " bytes. Aborting.\n";
		throw std::exception();
	}

	uint64_t blockNr = m_addressSelect >> m_blockSizeShift;

	if (blockNr+1 > m_memoryBlocks.size())
		m_memoryBlocks.resize(blockNr + 1);

	m_memoryBlocks[blockNr] = p;

	return p;
}


bool RAMComponent::ReadData(uint8_t& data)
{
	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint8_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock]);

	return true;
}


bool RAMComponent::ReadData(uint16_t& data, Endianness endianness)
{
	assert((m_addressSelect & 1) == 0);

	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint16_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock >> 1]);

	if (endianness == BigEndian)
		data = BE16_TO_HOST(data);
	else
		data = LE16_TO_HOST(data);

	return true;
}


bool RAMComponent::ReadData(uint32_t& data, Endianness endianness)
{
	assert((m_addressSelect & 3) == 0);

	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint32_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock >> 2]);

	if (endianness == BigEndian)
		data = BE32_TO_HOST(data);
	else
		data = LE32_TO_HOST(data);

	return true;
}


bool RAMComponent::ReadData(uint64_t& data, Endianness endianness)
{
	assert((m_addressSelect & 7) == 0);

	if (m_selectedHostMemoryBlock == NULL)
		data = 0;
	else
		data = (((uint64_t*)m_selectedHostMemoryBlock)
		    [m_selectedOffsetWithinBlock >> 3]);

	if (endianness == BigEndian)
		data = BE64_TO_HOST(data);
	else
		data = LE64_TO_HOST(data);

	return true;
}


bool RAMComponent::WriteData(const uint8_t& data)
{
	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	(((uint8_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock]) = data;

	return true;
}


bool RAMComponent::WriteData(const uint16_t& data, Endianness endianness)
{
	assert((m_addressSelect & 1) == 0);

	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	uint16_t d;
	if (endianness == BigEndian)
		d = BE16_TO_HOST(data);
	else
		d = LE16_TO_HOST(data);

	(((uint16_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock >> 1]) = d;

	return true;
}


bool RAMComponent::WriteData(const uint32_t& data, Endianness endianness)
{
	assert((m_addressSelect & 3) == 0);

	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	uint32_t d;
	if (endianness == BigEndian)
		d = BE32_TO_HOST(data);
	else
		d = LE32_TO_HOST(data);

	(((uint32_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock >> 2]) = d;

	return true;
}


bool RAMComponent::WriteData(const uint64_t& data, Endianness endianness)
{
	assert((m_addressSelect & 7) == 0);

	if (m_writeProtected)
		return false;

	if (m_selectedHostMemoryBlock == NULL)
		m_selectedHostMemoryBlock = AllocateBlock();

	uint64_t d;
	if (endianness == BigEndian)
		d = BE64_TO_HOST(data);
	else
		d = LE64_TO_HOST(data);

	(((uint64_t*)m_selectedHostMemoryBlock)
	    [m_selectedOffsetWithinBlock >> 3]) = d;

	return true;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

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
	UnitTest::Assert("The RAMComponent should implement the "
	    "AddressDataBus interface", bus != NULL);
}

static void Test_RAMComponent_InitiallyZero()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(0);

	// By default, RAM should be zero-filled:
	uint8_t data8 = 42;
	bus->ReadData(data8);
	UnitTest::Assert("A: memory should be zero filled (8)", data8, 0);

	uint16_t data16 = 142;
	bus->ReadData(data16, BigEndian);
	UnitTest::Assert("A: memory should be zero filled (16)", data16, 0);

	uint32_t data32 = 342;
	bus->ReadData(data32, BigEndian);
	UnitTest::Assert("A: memory should be zero filled (32)", data32, 0);

	uint64_t data64 = 942;
	bus->ReadData(data64, BigEndian);
	UnitTest::Assert("A: memory should be zero filled (64)", data64, 0);

	bus->AddressSelect(0x10000);

	data8 = 43;
	bus->ReadData(data8);
	UnitTest::Assert("B: memory should be zero filled (8)", data8, 0);

	data16 = 143;
	bus->ReadData(data16, BigEndian);
	UnitTest::Assert("B: memory should be zero filled (16)", data16, 0);

	data32 = 343;
	bus->ReadData(data32, BigEndian);
	UnitTest::Assert("B: memory should be zero filled (32)", data32, 0);

	data64 = 943;
	bus->ReadData(data64, BigEndian);
	UnitTest::Assert("B: memory should be zero filled (64)", data64, 0);
}

static void Test_RAMComponent_WriteThenRead()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(256);

	uint64_t data64 = ((uint64_t)0x1234567 << 32) | 0x89abcdef;
	bus->WriteData(data64, BigEndian);

	uint64_t data64_b = 0;
	bus->ReadData(data64_b, BigEndian);

	UnitTest::Assert("memory should be same when read", data64_b, data64);

	uint32_t data32_b = 0;
	bus->ReadData(data32_b, BigEndian);

	UnitTest::Assert("32-bit read should give top 32 bits, "
	    "in big endian mode", data32_b, data64 >> 32);

	uint16_t data16_b = 0;
	bus->ReadData(data16_b, BigEndian);

	UnitTest::Assert("16-bit read should give top 16 bits, "
	    "in big endian mode", data16_b, data64 >> 48);

	uint8_t data8_b = 0;
	bus->ReadData(data8_b);

	UnitTest::Assert("8-bit read should give top 8 bits, "
	    "in big endian mode", data8_b, data64 >> 56);
}

static void Test_RAMComponent_WriteThenRead_ReverseEndianness()
{
	refcount_ptr<Component> ram = ComponentFactory::CreateComponent("ram");
	AddressDataBus* bus = ram->AsAddressDataBus();

	bus->AddressSelect(256);

	uint64_t data64 = ((uint64_t)0x1234567 << 32) | 0x89abcdef;
	bus->WriteData(data64, BigEndian);

	bus->AddressSelect(256 + 4);

	uint32_t data32_b = 0;
	bus->ReadData(data32_b, LittleEndian);

	UnitTest::Assert("32-bit read", data32_b, 0xefcdab89);

	uint16_t data16_b = 0;
	bus->ReadData(data16_b, LittleEndian);

	UnitTest::Assert("16-bit read", data16_b, 0xab89);

	uint8_t data8_b = 0;
	bus->ReadData(data8_b);

	UnitTest::Assert("8-bit read", data8_b, 0x89);
}

UNITTESTS(RAMComponent)
{
	UNITTEST(Test_RAMComponent_IsStable);
	UNITTEST(Test_RAMComponent_AddressDataBus);
	UNITTEST(Test_RAMComponent_InitiallyZero);
	UNITTEST(Test_RAMComponent_WriteThenRead);
	UNITTEST(Test_RAMComponent_WriteThenRead_ReverseEndianness);

	// TODO: Write protect test

	// TODO: Serialization, deserialization
}

#endif

