#ifndef MAINBUSCOMPONENT_H
#define	MAINBUSCOMPONENT_H

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

// COMPONENT(mainbus)


#include "AddressDataBus.h"
#include "Component.h"

#include "UnitTest.h"


/**
 * \brief Main bus Component.
 *
 * An AddressDataBus Component which forwards reads and writes to other
 * AddressDataBus components, e.g. the RAMComponent.
 */
class MainbusComponent
	: public Component
	, public AddressDataBus
	, public UnitTestable
{
public:
	/**
	 * \brief Constructs a MainbusComponent.
	 */
	MainbusComponent();

	virtual ~MainbusComponent();

	/**
	 * \brief Creates a MainbusComponent.
	 */
	static refcount_ptr<Component> Create();

	/**
	 * \brief Get attribute information about the MainbusComponent class.
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
};


#endif	// MAINBUSCOMPONENT_H
