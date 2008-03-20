#ifndef RAMCOMPONENT_H
#define	RAMCOMPONENT_H

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

// COMPONENT(ram)


#include "AddressDataBus.h"
#include "Component.h"

#include "UnitTest.h"


/**
 * \brief A Random Access Memory Component.
 *
 * RAM is emulated by allocating large blocks of host memory (e.g. 1 MB
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
 * caller to make sure that e.g. ReadData(uint64_t&) is only called when
 * the selected address is 64-bit aligned.
 *
 * (The reason for this is that different emulated components want different
 * semantics for unaligned access. For example, an x86 processor will
 * transparently allow unaligned access, most RISC processors will cause
 * an unaligned address exception, and some old ARM processors may even simply
 * ignore the lowest bits of the address!)
 *
 * Note 2: The RAM component does not have any maximum size associated with it.
 * The intended usage scenario is to map the RAM component into another
 * bus' address range. E.g. for most machines, RAM is located at memory offset
 * 0 of the mainbus, and is of a certain size. It is up to that bus to forward
 * only those read/write requests that are within the RAM component's
 * range.
 */
class RAMComponent
	: public Component
	, public AddressDataBus
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a RAMComponent.
	 */
	RAMComponent();

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

	/**
	 * \brief Returns the component's AddressDataBus interface.
	 *
	 * @return	A pointer to an AddressDataBus.
	 */
	virtual AddressDataBus* AsAddressDataBus();

	/* Implementation of AddressDataBus: */
	virtual void AddressSelect(uint64_t address);
	virtual void ReadData(uint8_t& data);
	virtual void ReadData(uint16_t& data);
	virtual void ReadData(uint32_t& data);
	virtual void ReadData(uint64_t& data);
	virtual void WriteData(uint8_t& data);
	virtual void WriteData(uint16_t& data);
	virtual void WriteData(uint32_t& data);
	virtual void WriteData(uint64_t& data);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	void* AllocateBlock();

private:
	const size_t	m_blockSizeShift;// Host block size, in bit shift steps
	const size_t	m_blockSize;	 // Host block size, in bytes

	typedef vector<void *> BlockNrToMemoryBlockVector;
	BlockNrToMemoryBlockVector m_memoryBlocks;

	uint64_t	m_addressSelect;  // For AddressDataBus read/write
	void *		m_selectedHostMemoryBlock;
	size_t		m_selectedOffsetWithinBlock;
};


#endif	// RAMCOMPONENT_H
