#ifndef TRIGGERS_H
#define	TRIGGERS_H

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

#include "misc.h"

#include "Trigger.h"
#include "UnitTest.h"


/**
 * \brief A collection of Trigger objects.
 */
class Triggers
	: public UnitTestable
{
public:
	/**
	 * \brief Constructs a Triggers collection.
	 */
	Triggers();

	/**
	 * \brief Adds a Trigger to the collection.
	 *
	 * @param trigger  The Trigger to add.
	 */
	void Add(refcount_ptr<Trigger> trigger);

	/**
	 * \brief Removes all triggers for a specific property name.
	 *
	 * This function should be used when the property itself is removed,
	 * or is not reachable any longer.
	 *
	 * @param propertyName  The property name.
	 */
	void Remove(const string& propertyName);

	/**
	 * \brief Triggers triggers for a specific property name.
	 *
	 * This function should be called whenever the specified property name
	 * is written to. If the property was written to but the value not
	 * changed (i.e. being the same as before the write), then the
	 * changed argument should be false.
	 *
	 * Note: The return value is mostly used by unit tests.
	 *
	 * @param propertyName  The property name.
	 * @param changed  True if the property's value was changed into a
	 *	new value, false otherwise (i.e. if it was written to,
	 *	but with the same value as before).
	 * @return True if any trigger was triggered, false if no trigger
	 *	was added for the specified property name.
	 */
	bool PropertyWrittenTo(const string& propertyName, bool changed) const;

	/**
	 * \brief Gets the number of triggers in the collection.
	 *
	 * This function is mostly used during debugging and unit testing.
	 */
	int GetNumberOfTriggers() const;

	/**
	 * \brief Gets the number of triggers in the collection for a
	 *	specific property name,
	 *
	 * This function is mostly used during debugging and unit testing.
	 *
	 * @param propertyName  The property name.
	 */
	int GetNumberOfTriggers(const string& propertyName) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);

private:
	typedef multimap<string,refcount_ptr<Trigger> >	TriggerMultiMap;
	TriggerMultiMap	m_triggers;
};


#endif	// TRIGGERS_H
