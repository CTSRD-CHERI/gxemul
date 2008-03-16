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
 *  $Id: CPUComponent.cc,v 1.1 2008/03/14 12:12:32 debug Exp $
 */

#include <assert.h>
#include "components/CPUComponent.h"


CPUComponent::CPUComponent(const string& className)
	: Component(className)
	, m_pc(0)
	, m_currentCodePage(NULL)
	, m_pageSize(0)
{
	AddVariableUInt64("pc", &m_pc);
}


refcount_ptr<Component> CPUComponent::Create()
{
	return NULL;
}


string CPUComponent::GetAttribute(const string& attributeName)
{
	if (attributeName == "stable")
		return "yes";

	if (attributeName == "description")
		return "Base-class for all processors.";

	return Component::GetAttribute(attributeName);
}


bool CPUComponent::NativeTranslationExists()
{
	// TODO
	return false;
}


void CPUComponent::LookupCurrentCodePage()
{
	// TODO
	std::cerr << "CPUComponent::Run(): Page lookup.\n";
	assert(false);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "ComponentFactory.h"

static void Test_CPUComponent_IsStable()
{
	UnitTest::Assert("the CPUComponent should be stable",
	    ComponentFactory::HasAttribute("cpu", "stable"));
}

static void Test_CPUComponent_Create()
{
	refcount_ptr<Component> cpu =
	    ComponentFactory::CreateComponent("cpu");
	UnitTest::Assert("component was created?", cpu.IsNULL());
}

UNITTESTS(CPUComponent)
{
	UNITTEST(Test_CPUComponent_IsStable);
	UNITTEST(Test_CPUComponent_Create);
}

#endif

