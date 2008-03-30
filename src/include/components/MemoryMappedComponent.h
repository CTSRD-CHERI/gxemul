#ifndef MEMORYMAPPEDCOMPONENT_H
#define	MEMORYMAPPEDCOMPONENT_H

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

// COMPONENT(memorymapped)


#include "Component.h"

#include "UnitTest.h"


/**
 * \brief A base-class for memory-mapped Components.
 */
class MemoryMappedComponent
	: public Component
{
public:
	/**
	 * \brief Constructs a MemoryMappedComponent.
	 */
	MemoryMappedComponent(const string& className);

	/**
	 * \brief Creates a MemoryMappedComponent.
	 */
	static refcount_ptr<Component> Create();

	/**
	 * \brief Get attribute information about the MemoryMappedComponent
	 * class.
	 *
	 * @param attributeName The attribute name.
	 * @return A string representing the attribute value.
	 */
	static string GetAttribute(const string& attributeName);

protected:
	// Variables common to all memory mapped components:
	uint64_t	m_memoryMappedBase;
	uint64_t	m_memoryMappedSize;
	uint64_t	m_memoryMappedAddrMul;
};


#endif	// MEMORYMAPPEDCOMPONENT_H
