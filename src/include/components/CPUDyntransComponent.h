#ifndef CPUDYNTRANSCOMPONENT_H
#define	CPUDYNTRANSCOMPONENT_H

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

// Note: Not included in the component registry.


#include "CPUComponent.h"
#include "UnitTest.h"


class CPUDyntransComponent;


#define N_DYNTRANS_IC_ARGS	3
/**
 * \brief A dyntrans instruction call.
 *
 * f points to a function to be executed.
 * arg[] contains arguments, such as pointers to registers, or immediate values.
 */
struct DyntransIC
{
	void (*f)(CPUDyntransComponent*, DyntransIC*);

	union {
		void* p;
		uint32_t u32;
	} arg[N_DYNTRANS_IC_ARGS];
};

/*
 * A dyntrans page contains DyntransIC calls for each instruction slot, followed
 * by some special entries, which handle execution going over the end of a page
 * (by changing the PC to the start of the next virtual page).
 */
#define	DYNTRANS_PAGE_NSPECIALENTRIES	2


/*
 * Some helpers for implementing dyntrans instructions.
 */
#define DECLARE_DYNTRANS_INSTR(name) static void instr_##name(CPUDyntransComponent* cpubase, DyntransIC* ic);
#define DYNTRANS_INSTR(class,name) void class::instr_##name(CPUDyntransComponent* cpubase, DyntransIC* ic)
#define DYNTRANS_INSTR_HEAD(class)  class* cpu = (class*) cpubase;

#define REG32(arg)	(*((uint32_t*)((arg).p)))
#define REG64(arg)	(*((uint64_t*)((arg).p)))

#define DYNTRANS_SYNCH_PC	cpu->m_nextIC = ic; cpu->DyntransResyncPC()


/**
 * \brief A base-class for processors Component implementations that
 *	use dynamic translation.
 */
class CPUDyntransComponent
	: public CPUComponent
{
public:
	/**
	 * \brief Constructs a CPUDyntransComponent.
	 *
	 * @param className The class name for the component.
	 * @param cpuKind The CPU kind, e.g. "MIPS R4400" for a
	 *	MIPS R4400 processor.
	 */
	CPUDyntransComponent(const string& className, const string& cpuKind);

	virtual int Execute(GXemul* gxemul, int nrOfCycles);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	// Implemented by specific CPU families:
	virtual int GetDyntransICshift() const = 0;
	virtual void (*GetDyntransToBeTranslated())(CPUDyntransComponent* cpu, DyntransIC* ic) const = 0;

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
	struct DyntransIC* DyntransGetICPage(uint64_t addr);
	void DyntransClearICPage(struct DyntransIC* icpage);

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

	// Branches.
	DECLARE_DYNTRANS_INSTR(branch_samepage);

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
	DECLARE_DYNTRANS_INSTR(and_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(and_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(or_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(or_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(or_u64_u64_immu32);
	DECLARE_DYNTRANS_INSTR(xor_u32_u32_immu32);
	DECLARE_DYNTRANS_INSTR(xor_u32_u32_u32);
	DECLARE_DYNTRANS_INSTR(xor_u64_u64_immu32);
	
	// Shifts, rotates.
	DECLARE_DYNTRANS_INSTR(shift_left_u64_u64_imm5_truncS32);

protected:
	/*
	 * Volatile state:
	 */
	struct DyntransIC *	m_ICpage;
	struct DyntransIC *	m_nextIC;
	int			m_dyntransPageMask;
	int			m_dyntransICentriesPerPage;
	int			m_dyntransICshift;
	int			m_executedCycles;
	int			m_nrOfCyclesToExecute;
};


#endif	// CPUDYNTRANSCOMPONENT_H
