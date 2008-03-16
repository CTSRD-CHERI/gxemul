#ifndef MIPS_CPUCOMPONENT_H
#define	MIPS_CPUCOMPONENT_H

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
 *
 *
 *  $Id: MIPS_CPUComponent.h,v 1.2 2008/03/14 12:12:16 debug Exp $
 */

// COMPONENT(mips_cpu)


#include "CPUComponent.h"


/**
 * \brief A Component representing a MIPS processor.
 */
class MIPS_CPUComponent
	: public CPUComponent
{
public:
	/**
	 * \brief Constructs a MIPS_CPUComponent.
	 */
	MIPS_CPUComponent();

	/**
	 * \brief Creates a MIPS_CPUComponent.
	 */
	static refcount_ptr<Component> Create();

	/**
	 * \brief Get attribute information about the MIPS_CPUComponent class.
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value.
	 */
	static string GetAttribute(const string& attributeName);

	/**
	 * \brief Runs the component for a number of cycles.
	 *
	 * @param nrOfCycles    The number of cycles to run.
	 * @return      The number of cycles actually executed.
	 */
	virtual int Run(int nrOfCycles);

	/**
	 * \brief Returns the current frequency (in Hz) that the component
	 *	runs at.
	 *
	 * @return	The component's frequency in Hz.
	 */
	virtual double GetCurrentFrequency() const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	void ExecuteMIPS16Instruction(uint16_t iword);
	void ExecuteInstruction(uint32_t iword);
};


#endif	// MIPS_CPUCOMPONENT_H
