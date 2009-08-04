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
#include "SymbolRegistry.h"
#include "UnitTest.h"

class AddressDataBus;


#define N_DYNTRANS_IC_ARGS	3
/**
 * \brief A dyntrans instruction call.
 *
 * f points to a function to be executed.
 * arg[] contains arguments, such as pointers to registers.
 *
 * The type of arg is size_t, so that it can hold a host pointer; otherwise
 * the requirement is that arg can hold at least an uint32_t. (uint64_t is
 * not required.)
 */
struct DyntransIC
{
	void (*f)(CPUComponent*, DyntransIC*);
	size_t arg[N_DYNTRANS_IC_ARGS];
};


/*
 * Some helpers for implementing dyntrans instructions.
 */
#define DECLARE_DYNTRANS_INSTR(name) static void instr_##name(CPUComponent* cpubase, DyntransIC* ic);
#define DYNTRANS_INSTR(class,name) void class::instr_##name(CPUComponent* cpubase, DyntransIC* ic)
#define DYNTRANS_INSTR_HEAD(class)  class* cpu = (class*) cpubase;

#define REG32(arg)	(*((uint32_t*)arg))
#define REG64(arg)	(*((uint64_t*)arg))


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
	static refcount_ptr<Component> Create(const ComponentCreateArgs& args);

	/**
	 * \brief Get attribute information about the CPUComponent class.
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value.
	 */
	static string GetAttribute(const string& attributeName);

	/**
	 * \brief Gets a reference to the CPU's symbol registry.
	 *
	 * @return A reference to the symbol registry.
	 */
	SymbolRegistry& GetSymbolRegistry()
	{
		return m_symbolRegistry;
	}
	const SymbolRegistry& GetSymbolRegistry() const
	{
		return m_symbolRegistry;
	}

	virtual void ResetState();

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

	uint64_t Unassemble(int nRows, bool indicatePC, uint64_t vaddr, ostream& output);

	/**
	 * \brief Virtual to physical address translation (MMU).
	 *
	 * This function should be overridden in each CPU implementation.
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
					bool& writable) = 0;

	/**
	 * \brief Convert PC value to instuction address.
	 *
	 * Usually, this does not need to be overridden. However, some
	 * architectures use e.g. the lowest bit of the PC register to indicate
	 * a different encoding mode (MIPS16), but the instruction is still
	 * aligned as if the lowest bit was 0.
	 */
	virtual uint64_t PCtoInstructionAddress(uint64_t pc)
	{
		return pc;
	}

	// CPUComponent:
	void FunctionTraceCall();
	void FunctionTraceReturn();

	// Overridden by each CPU architecture:
	virtual int FunctionTraceArgumentCount() { return 0; }
	virtual int64_t FunctionTraceArgument(int n) { return 0; }
	virtual bool FunctionTraceReturnImpl(int64_t& retval) { return false; }

	virtual int GetDyntransICshift() const;
	virtual void (*GetDyntransToBeTranslated())(CPUComponent* cpu, DyntransIC* ic) const;

	int DyntransExecute(GXemul* gxemul, int nrOfCycles);
	void DyntransToBeTranslatedBegin(struct DyntransIC*);
	bool DyntransReadInstruction(uint16_t& iword);
	bool DyntransReadInstruction(uint32_t& iword);
	void DyntransToBeTranslatedDone(struct DyntransIC*);

	/**
	 * \brief Calculate m_pc based on m_nextIC and m_ICpage.
	 */
	void DyntransResyncPC();

	/**
	 * \brief Calculate m_nextIC and m_ICpage, based on m_pc.
	 *
	 * This function may return pointers to within an existing translation
	 * page (hopefully the most common case, since it is the fastest), or
	 * it may allocate a new empty page.
	 */
	void DyntransPCtoPointers();

private:
	void DyntransInit();

	bool LookupAddressDataBus(GXemul* gxemul = NULL);

protected:
	/*
	 * Generic dyntrans instruction implementations, that may be used by
	 * several different cpu architectures.
	 */
	DECLARE_DYNTRANS_INSTR(nop);
	DECLARE_DYNTRANS_INSTR(abort);
	DECLARE_DYNTRANS_INSTR(abort_in_delay_slot);
	DECLARE_DYNTRANS_INSTR(endOfPage);
	DECLARE_DYNTRANS_INSTR(endOfPage2);

	// Data movement.
	DECLARE_DYNTRANS_INSTR(set_u64_imms32);

	// Arithmetic.
	DECLARE_DYNTRANS_INSTR(add_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(add_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(add_u64_u64_imms32_truncS32);
	DECLARE_DYNTRANS_INSTR(add_u64_u64_imms32);
	DECLARE_DYNTRANS_INSTR(sub_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(sub_u32_u32_u32);

	// Logic.
	DECLARE_DYNTRANS_INSTR(and_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(or_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(or_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(xor_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(xor_u64_u64_immu32);
	
	// Shifts, rotates.
	DECLARE_DYNTRANS_INSTR(shift_left_u64_u64_imm5_truncS32);

protected:
	/*
	 * Variables common to all (or most) kinds of CPUs:
	 */

	// Framework frequency/runability:
	double			m_frequency;
	bool			m_paused;

	// Architecture fundamentals:
	string			m_cpuArchitecture;
	int			m_pageSize;

	// Program counter:
	uint64_t		m_pc;

	// Memory dump and disassembly:
	uint64_t		m_lastDumpAddr;
	uint64_t		m_lastUnassembleVaddr;
	bool			m_hasUsedUnassemble;

	// Endianness:
	bool			m_isBigEndian;

	// Function call trace:
	bool			m_showFunctionTraceCall;
	bool			m_showFunctionTraceReturn;
	int32_t			m_functionCallTraceDepth;
	int64_t			m_nrOfTracedFunctionCalls;

	// Delay slot related:
	bool			m_inDelaySlot;
	uint64_t		m_delaySlotTarget;


	/*
	 * Cached/volatile state:
	 */
	AddressDataBus *	m_addressDataBus;
	uint64_t		m_addressSelect;
	struct DyntransIC *	m_ICpage;
	struct DyntransIC *	m_nextIC;
	int			m_dyntransPageMask;
	int			m_dyntransICentriesPerPage;
	int			m_dyntransICshift;
	int			m_executedCycles;
	int			m_nrOfCyclesToExecute;
	bool			m_exceptionInDelaySlot;

private:
	SymbolRegistry		m_symbolRegistry;

	// DUMMY/TEST
	vector< struct DyntransIC > m_dummyICpage;
	uint32_t		m_dyntransTestVariable;
};


#endif	// CPUCOMPONENT_H
