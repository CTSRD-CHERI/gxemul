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

#include "components/RootComponent.h"
#include "ComponentFactory.h"


RootComponent::RootComponent()
	: Component("root", "root")
	, m_time(0.0)
{
	SetVariableValue("name", "\"root\"");

	ResetState();

	AddVariable("time", &m_time);
}


void RootComponent::ResetState()
{
	m_time = 0.0;

	Component::ResetState();
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_RootComponent_CreateComponent()
{
	refcount_ptr<Component> component;

	component = ComponentFactory::CreateComponent("root");
	UnitTest::Assert("creating a root component with CreateComponent "
	    "should NOT be possible", component.IsNULL() == true);
}

static void Test_RootComponent_InitialStepAndTime()
{
	refcount_ptr<Component> component = new RootComponent;

	StateVariable* time = component->GetVariable("time");
	StateVariable* step = component->GetVariable("step");

	UnitTest::Assert("time should be 0.0", time->ToDouble(), 0.0);
	UnitTest::Assert("step should be 0", step->ToInteger(), 0);
}

UNITTESTS(RootComponent)
{
	UNITTEST(Test_RootComponent_CreateComponent);
	UNITTEST(Test_RootComponent_InitialStepAndTime);
}

#endif

