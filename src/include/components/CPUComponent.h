#ifndef CPUCOMPONENT_H
#define	CPUCOMPONENT_H

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

// Note: Not a COMPONENT; this is used as a base-class only.


#include "Component.h"

#include "UnitTest.h"

class AddressDataBus;


/**
 * \brief A Component base-class for processors.
 */
class CPUComponent
	: public Component
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a CPUComponent.
	 */
	CPUComponent(const string& className);

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


	/**
	 * \brief Returns the current frequency (in Hz) that the component
	 *      runs at.
	 *
	 * @return      The component's frequency in Hz.
	 */
	virtual double GetCurrentFrequency() const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

protected:
	virtual void FlushCachedStateForComponent();

	// Used by all (or most) CPU implementations:
	bool ReadInstructionWord(uint16_t& iword, uint64_t addr);
	bool ReadInstructionWord(uint32_t& iword, uint64_t addr);

private:
	void LookupAddressDataBus();

protected:
	// Variables common to all (or most) kinds of CPUs:
	double			m_frequency;

	uint64_t		m_pc;
	enum Endianness		m_endianness;

	AddressDataBus *	m_addressDataBus;
	const uint8_t *		m_currentCodePage;
	int			m_pageSize;
};


#endif	// CPUCOMPONENT_H
