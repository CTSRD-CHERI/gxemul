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

#include "Triggers.h"


Triggers::Triggers()
{
}


void Triggers::Add(refcount_ptr<Trigger> trigger)
{
	m_triggers.insert(pair<string,refcount_ptr<Trigger> >(
	    trigger->GetPropertyName(), trigger));
}


void Triggers::Remove(const string& propertyName)
{
	m_triggers.erase(propertyName);
}


bool Triggers::PropertyWrittenTo(const string& propertyName, bool changed) const
{
	TriggerMultiMap::const_iterator it = m_triggers.find(propertyName);
	int n = 0;

	while (it != m_triggers.end() && it->first == propertyName) {
		it->second->ExecuteCallbackFunction(changed);

		++ n;
		++ it;
	}
	
	return n>0;
}


int Triggers::GetNumberOfTriggers() const
{
	return m_triggers.size();
}


int Triggers::GetNumberOfTriggers(const string& propertyName) const
{
	return m_triggers.count(propertyName);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static int myCallbackTestVariable;

static void myCallbackFunc(string a, bool b)
{
	myCallbackTestVariable ++;
}

static void Test_Triggers_Add()
{
	Triggers triggers;
	
	UnitTest::Assert("count should be 0",
	    triggers.GetNumberOfTriggers(), 0);

	refcount_ptr<Trigger> myTrigger = new Trigger("hello.something",
	    myCallbackFunc, false);
	myCallbackTestVariable = 42;

	triggers.Add(myTrigger);

	UnitTest::Assert("count should be 1",
	    triggers.GetNumberOfTriggers(), 1);

	UnitTest::Assert("count for the specific property name should be 1",
	    triggers.GetNumberOfTriggers("hello.something"), 1);
	UnitTest::Assert("count for the second property name should be 0",
	    triggers.GetNumberOfTriggers("hello.something2"), 0);

	UnitTest::Assert("the trigger should not have been executed",
	    myCallbackTestVariable, 42);

	refcount_ptr<Trigger> myTrigger2 = new Trigger("hello.something2",
	    myCallbackFunc, false);

	triggers.Add(myTrigger2);

	UnitTest::Assert("count should be 2",
	    triggers.GetNumberOfTriggers(), 2);

	UnitTest::Assert("the trigger should still not have been executed",
	    myCallbackTestVariable, 42);

	UnitTest::Assert("count for the property name should still be 1",
	    triggers.GetNumberOfTriggers("hello.something"), 1);
	UnitTest::Assert("count for the second property name should be 1",
	    triggers.GetNumberOfTriggers("hello.something2"), 1);
}

static void Test_Triggers_Remove()
{
	Triggers triggers;
	
	myCallbackTestVariable = 42;

	refcount_ptr<Trigger> myTrigger1 = new Trigger("a",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger2 = new Trigger("b",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger3 = new Trigger("c",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger4 = new Trigger("a",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger5 = new Trigger("b",
	    myCallbackFunc, false);

	triggers.Add(myTrigger1);
	triggers.Add(myTrigger2);
	triggers.Add(myTrigger3);
	triggers.Add(myTrigger4);
	triggers.Add(myTrigger5);

	UnitTest::Assert("count should be 5",
	    triggers.GetNumberOfTriggers(), 5);

	UnitTest::Assert("count for \"a\" mismatch",
	    triggers.GetNumberOfTriggers("a"), 2);
	UnitTest::Assert("count for \"b\" mismatch",
	    triggers.GetNumberOfTriggers("b"), 2);
	UnitTest::Assert("count for \"c\" mismatch",
	    triggers.GetNumberOfTriggers("c"), 1);

	triggers.Remove("b");

	UnitTest::Assert("count should be 3",
	    triggers.GetNumberOfTriggers(), 3);

	UnitTest::Assert("count for \"a\" mismatch",
	    triggers.GetNumberOfTriggers("a"), 2);
	UnitTest::Assert("count for \"b\" mismatch",
	    triggers.GetNumberOfTriggers("b"), 0);
	UnitTest::Assert("count for \"c\" mismatch",
	    triggers.GetNumberOfTriggers("c"), 1);

	triggers.Remove("c");

	UnitTest::Assert("count should be 2",
	    triggers.GetNumberOfTriggers(), 2);

	UnitTest::Assert("count for \"a\" mismatch",
	    triggers.GetNumberOfTriggers("a"), 2);
	UnitTest::Assert("count for \"b\" mismatch",
	    triggers.GetNumberOfTriggers("b"), 0);
	UnitTest::Assert("count for \"c\" mismatch",
	    triggers.GetNumberOfTriggers("c"), 0);

	triggers.Remove("xxx");

	UnitTest::Assert("count should still be 2",
	    triggers.GetNumberOfTriggers(), 2);

	UnitTest::Assert("count for \"a\" mismatch",
	    triggers.GetNumberOfTriggers("a"), 2);
	UnitTest::Assert("count for \"b\" mismatch",
	    triggers.GetNumberOfTriggers("b"), 0);
	UnitTest::Assert("count for \"c\" mismatch",
	    triggers.GetNumberOfTriggers("c"), 0);

	triggers.Remove("a");

	UnitTest::Assert("count should be 0",
	    triggers.GetNumberOfTriggers(), 0);

	UnitTest::Assert("count for \"a\" mismatch",
	    triggers.GetNumberOfTriggers("a"), 0);
	UnitTest::Assert("count for \"b\" mismatch",
	    triggers.GetNumberOfTriggers("b"), 0);
	UnitTest::Assert("count for \"c\" mismatch",
	    triggers.GetNumberOfTriggers("c"), 0);
}

static void Test_Triggers_PropertyWrittenTo()
{
	Triggers triggers;
	
	myCallbackTestVariable = 123;

	refcount_ptr<Trigger> myTrigger1 = new Trigger("a",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger2 = new Trigger("b",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger3 = new Trigger("c",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger4 = new Trigger("a",
	    myCallbackFunc, false);
	refcount_ptr<Trigger> myTrigger5 = new Trigger("b",
	    myCallbackFunc, false);

	triggers.Add(myTrigger1);
	triggers.Add(myTrigger2);
	triggers.Add(myTrigger3);
	triggers.Add(myTrigger4);
	triggers.Add(myTrigger5);

	triggers.PropertyWrittenTo("b", true);
	UnitTest::Assert("wrong number of triggers executed",
	    myCallbackTestVariable, 123 + 2);

	triggers.PropertyWrittenTo("a", true);
	UnitTest::Assert("wrong number of triggers executed",
	    myCallbackTestVariable, 123 + 2 + 2);

	triggers.PropertyWrittenTo("xxx", true);
	UnitTest::Assert("wrong number of triggers executed",
	    myCallbackTestVariable, 123 + 2 + 2);

	triggers.PropertyWrittenTo("c", true);
	UnitTest::Assert("wrong number of triggers executed",
	    myCallbackTestVariable, 123 + 2 + 2 + 1);
}

UNITTESTS(Triggers)
{
	UNITTEST(Test_Triggers_Add);
	UNITTEST(Test_Triggers_Remove);
	UNITTEST(Test_Triggers_PropertyWrittenTo);
}

#endif
