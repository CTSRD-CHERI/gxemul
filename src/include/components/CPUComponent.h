#ifndef CPUCOMPONENT_H
#define	CPUCOMPONENT_H

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

// COMPONENT(cpu)


#include "AddressDataBus.h"
#include "Component.h"

#include "UnitTest.h"

class AddressDataBus;


/**
 * \brief A Component base-class for processors.
 */
class CPUComponent
	: public Component
	, public AddressDataBus
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a CPUComponent.
	 *
	 * @param className The class name for the component.
	 * @param cpuKind The CPU kind, e.g. "MIPS R4400" for a
	 *	MIPS R4400 processor.
	 */
	CPUComponent(const string& className, const string& cpuKind);

	/**
	 * \brief Creates a CPUComponent.
	 */
	static refcount_ptr<Component> Create();

	/**
	 * \brief Get attribute information about the CPUComponent class.
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value.
	 */
	static string GetAttribute(const string& attributeName);

	virtual double GetCurrentFrequency() const;

	virtual CPUComponent* AsCPUComponent();

        virtual void GetMethodNames(vector<string>& names) const;

	virtual bool MethodMayBeReexecutedWithoutArgs(const string& methodName) const;

	virtual void ExecuteMethod(GXemul* gxemul,
		const string& methodName,
		const vector<string>& arguments);

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

	/**
	 * \brief Disassembles an instruction into readable strings.
	 *
	 * @param vaddr The virtual address of the program counter.
	 * @param maxLen The number of bytes in the instruction buffer.
	 * @param instruction A pointer to a buffer containing the instruction.
	 * @param result A vector where the implementation will add:
	 *	<ol>
	 *		<li>machine code bytes in a standard notation
	 *		<li>instruction mnemonic
	 *		<li>instruction arguments
	 *		<li>instruction comments
	 *	</ol>
	 *	All of the fields above are optional, but they have to be
	 *	specified in the same order for a particular CPU implementation,
	 *	so that the fields of the vector can be listed in a tabular
	 *	format.
	 * @return The number of bytes that the instruction occupied.
	 */	
	virtual size_t DisassembleInstruction(uint64_t vaddr, size_t maxLen,
		unsigned char *instruction, vector<string>& result) = 0;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual void FlushCachedStateForComponent();
	virtual bool PreRunCheckForComponent(GXemul* gxemul);

	virtual void ShowRegisters(GXemul* gxemul, const vector<string>& arguments) const;

	// Used by all (or most) CPU implementations:
	bool ReadInstructionWord(uint16_t& iword, uint64_t vaddr);
	bool ReadInstructionWord(uint32_t& iword, uint64_t vaddr);

	/**
	 * \brief Virtual to physical address translation (MMU).
	 *
	 * This function should be overridden in each CPU implementation.
	 * The default implementation is just a dummy, which returns paddr =
	 * vaddr for all addresses, and everything is writable.
	 *
	 * @param vaddr The virtual address to translate.
	 * @param paddr The return value; physical address.
	 * @param writable This is set to true or false by the function,
	 *	depending on if the memory at the virtual address was
	 *	writable or not.
	 * @return True if the translation succeeded, false if there was a
	 *	translation error.
	 */
	virtual bool VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
					bool& writable);

private:
	bool LookupAddressDataBus(GXemul* gxemul = NULL);

protected:
	// Variables common to all (or most) kinds of CPUs:
	double			m_frequency;
	string			m_cpuArchitecture;
	int			m_pageSize;
	uint64_t		m_pc;
	uint64_t		m_lastDumpAddr;
	uint64_t		m_lastUnassembleVaddr;
	bool			m_hasUsedUnassemble;
	bool			m_isBigEndian;

	// Cached state:
	AddressDataBus *	m_addressDataBus;
	const uint8_t *		m_currentCodePage;

	// Other volatile state:
	uint64_t		m_addressSelect;
};


#endif	// CPUCOMPONENT_H
