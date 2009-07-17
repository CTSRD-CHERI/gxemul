#ifndef RAMCOMPONENT_H
#define	RAMCOMPONENT_H

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

// COMPONENT(ram)


#include "AddressDataBus.h"
#include "MemoryMappedComponent.h"

#include "UnitTest.h"


/**
 * \brief A Random Access Memory Component.
 *
 * RAM is emulated by allocating large blocks of host memory (e.g. 4 MB
 * per block), and simply forwarding all read and write requests to those
 * memory blocks.
 *
 * The host memory blocks are not allocated until they are actually written to.
 * Reading from uninitialized/unwritten emulated memory returns zeros. This
 * allows e.g. an emulated machine to have, say, 1 TB RAM, even if the host
 * only has 1 GB, as long as the emulated guest OS does not touch all of the
 * emulated memory.
 *
 * In addition, host memory blocks are allocated as anonymous zero-filled
 * memory using mmap(), so the blocks do not necessariliy use up host RAM
 * unless they are touched.
 *
 * Note 1: This class does <i>not</i> handle unaligned access. It is up to the
 * caller to make sure that e.g. ReadData(uint64_t&, Endianness) is only
 * called when the selected address is 64-bit aligned.
 *
 * (The reason for this is that different emulated components want different
 * semantics for unaligned access. For example, an x86 processor will
 * transparently allow unaligned access, most RISC processors will cause
 * an unaligned address exception, and some old ARM processors may even simply
 * ignore the lowest bits of the address!)
 *
 * Note 2: The RAM component's size and base offset are defined by state
 * variables in the MemoryMappedComponent base class.
 */
class RAMComponent
	: public MemoryMappedComponent
	, public AddressDataBus
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a RAMComponent.
	 *
	 * @param visibleClassName The visible class name. Defaults to
	 *	"ram". Useful alternatives may be "rom" or
	 *	"vram" for video ram.
	 */
	RAMComponent(const string& visibleClassName = "ram");

	virtual ~RAMComponent();

	/**
	 * \brief Creates a RAMComponent.
	 */
	static refcount_ptr<Component> Create();

	/**
	 * \brief Get attribute information about the RAMComponent class.
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value.
	 */
	static string GetAttribute(const string& attributeName);

        virtual void GetMethodNames(vector<string>& names) const;

	virtual void ExecuteMethod(GXemul* gxemul,
		const string& methodName,
		const vector<string>& arguments);

	/**
	 * \brief Returns the component's AddressDataBus interface.
	 *
	 * @return	A pointer to an AddressDataBus.
	 */
	virtual AddressDataBus* AsAddressDataBus();

	/* Implementation of AddressDataBus: */
	virtual void AddressSelect(uint64_t address);
	virtual bool ReadData(uint8_t& data);
	virtual bool ReadData(uint16_t& data, Endianness endianness);
	virtual bool ReadData(uint32_t& data, Endianness endianness);
	virtual bool ReadData(uint64_t& data, Endianness endianness);
	virtual bool WriteData(const uint8_t& data);
	virtual bool WriteData(const uint16_t& data, Endianness endianness);
	virtual bool WriteData(const uint32_t& data, Endianness endianness);
	virtual bool WriteData(const uint64_t& data, Endianness endianness);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	void* AllocateBlock();

private:
	const size_t	m_blockSizeShift;// Host block size, in bit shift steps
	const size_t	m_blockSize;	 // Host block size, in bytes

	// State:
	typedef vector<void *> BlockNrToMemoryBlockVector;
	BlockNrToMemoryBlockVector	m_memoryBlocks;
	bool				m_writeProtected;
	uint64_t			m_lastDumpAddr;

	// Cached/runtime state:
	uint64_t	m_addressSelect;  // For AddressDataBus read/write
	void *		m_selectedHostMemoryBlock;
	size_t		m_selectedOffsetWithinBlock;
};


#endif	// RAMCOMPONENT_H
