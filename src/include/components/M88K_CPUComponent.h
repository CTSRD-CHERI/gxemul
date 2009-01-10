#ifndef M88K_CPUCOMPONENT_H
#define	M88K_CPUCOMPONENT_H

/*
 *  Copyright (C) 2009  Anders Gavare.  All rights reserved.
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

// COMPONENT(m88k_cpu)


#include "CPUComponent.h"

#include "cpu_m88k.h"


/***********************************************************************/

/**
 * \brief A Component representing a Motorola 88000 processor.
 */
class M88K_CPUComponent
	: public CPUComponent
{
public:
	/**
	 * \brief Constructs a M88K_CPUComponent.
	 */
	M88K_CPUComponent();

	/**
	 * \brief Creates a M88K_CPUComponent.
	 */
	static refcount_ptr<Component> Create();

	static string GetAttribute(const string& attributeName);

	virtual void ResetState();

	virtual int Run(int nrOfCycles);

	virtual size_t DisassembleInstruction(uint64_t vaddr, size_t maxlen,
		unsigned char *instruction, vector<string>& result);


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual bool VirtualToPhysical(uint64_t vaddr, uint64_t& paddr,
	    bool& writable);

private:
	void ExecuteInstruction(uint32_t iword);

private:
	// State:
	string			m_m88k_type;	// E.g. "88100"

	/*
	 *  General-Purpose Registers:
	 *
	 *  32 (N_M88K_REGS) registers, plus one which is always zero. (This
	 *  is to support st.d with d = r31. ld.d with d=r31 is converted to
	 *  just ld. TODO)
	 */
	uint32_t		m_r[N_M88K_REGS+1];

	/*  Destination scratch register for non-nop instructions with d=r0:  */
	uint32_t		m_zero_scratch;

	/*  Control Registers:  */
	uint32_t		m_cr[N_M88K_CONTROL_REGS];

	/*  Floating Point Control registers:  */
	uint32_t		m_fcr[N_M88K_FPU_CONTROL_REGS];


};


#endif	// M88K_CPUCOMPONENT_H

