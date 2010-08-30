/*
 *  Copyright (C) 2008-2010  Anders Gavare.  All rights reserved.
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

#include "components/MemoryMappedComponent.h"


MemoryMappedComponent::MemoryMappedComponent(const string& className,
		const string& visibleClassName)
	: Component(className, visibleClassName)
	, m_memoryMappedBase(0)
	, m_memoryMappedSize(0)
	, m_memoryMappedAddrMul(1)
{
	AddVariable("memoryMappedBase", &m_memoryMappedBase);
	AddVariable("memoryMappedSize", &m_memoryMappedSize);
	AddVariable("memoryMappedAddrMul", &m_memoryMappedAddrMul);
}

string MemoryMappedComponent::GenerateDetails() const
{
	stringstream ss;
	ss << Component::GenerateDetails();

	const StateVariable* memoryMappedBase = GetVariable("memoryMappedBase");
	const StateVariable* memoryMappedSize = GetVariable("memoryMappedSize");
	const StateVariable* memoryMappedAddrMul =
	    GetVariable("memoryMappedAddrMul");
	if (memoryMappedBase != NULL && memoryMappedSize != NULL) {
		if (!ss.str().empty())
			ss << ", ";

		uint64_t nBytes = memoryMappedSize->ToInteger();
		if (nBytes >= (1 << 30))
			ss << (nBytes >> 30) << " GB";
		else if (nBytes >= (1 << 20))
			ss << (nBytes >> 20) << " MB";
		else if (nBytes >= (1 << 10))
			ss << (nBytes >> 10) << " KB";
		else if (nBytes != 1)
			ss << nBytes << " bytes";
		else
			ss << nBytes << " byte";

		ss << " at offset ";
		ss.flags(std::ios::hex | std::ios::showbase);
		ss << memoryMappedBase->ToInteger();

		if (memoryMappedAddrMul != NULL &&
		    memoryMappedAddrMul->ToInteger() != 1)
			ss << ", addrmul " << memoryMappedAddrMul->ToInteger();
	}

	return ss.str();
}

