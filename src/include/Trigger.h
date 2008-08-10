#ifndef TRIGGER_H
#define	TRIGGER_H

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

#include "UnitTest.h"


/**
 * \brief A trigger is a callback to a function, whenever a named
 * property changes.
 *
 * Usually, the named property is a StateVariable, but it can also be
 * other names which have been agreed upon.
 */
class Trigger
	: public ReferenceCountable
	, public UnitTestable
{
public:
	/**
	 * \brief A type defining a callback function for triggers.
	 *
	 * @param propertyName  This is the name of the property which
	 *	was written to.
	 * @param propertyChanged  True if the property was changed,
	 *	false if it was not changed (i.e. it was written to,
	 *	but the value was the same as before).
	 */
	typedef void (CallbackFunction)(string propertyName,
		bool propertyChanged);

public:
	/**
	 * \brief Constructs a Trigger.
	 *
	 * @param propertyName	The name of a property (usually a
	 *	StateVariable) to listen to.
	 * @param callbackFunction  A function which will be called
	 *	whenever the property is written to.
	 * @param changesOnly  True if the trigger should be triggered
	 *	only when the property <i>changes</i> (i.e. to something
	 *	else than it was), false if all writes to the property
	 *	should trigger the trigger.
	 */
	Trigger(const string& propertyName,
		CallbackFunction* callbackFunction,
		bool changesOnly);

	virtual ~Trigger();

	/**
	 * \brief Gets the name of the property that the trigger triggers on.
	 */
	const string& GetPropertyName() const;

	/**
	 * \brief Executes the trigger's callback function.
	 *
	 * @param propertyChanged  True if the property was changed,
	 *	false if it was not changed (i.e. it was written to,
	 *	but the value was the same as before).
	 */
	void ExecuteCallbackFunction(bool propertyChanged) const;


	/********************************************************************/

	static void RunUnitTests(int& nSucceeded, int& nFailures);


private:
	string			m_propertyName;
	CallbackFunction*	m_callbackFunction;
	bool			m_changesOnly;
};


#endif	// TRIGGER_H
