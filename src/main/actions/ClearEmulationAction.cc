/*
 *  Copyright (C) 2008-2009  Anders Gavare.  All rights reserved.
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

#include <assert.h>

#include "actions/ClearEmulationAction.h"
#include "components/DummyComponent.h"
#include "GXemul.h"


ClearEmulationAction::ClearEmulationAction(GXemul& gxemul)
	: Action("clear emulation", false)
	, m_gxemul(gxemul)
{
}


ClearEmulationAction::~ClearEmulationAction()
{
}


void ClearEmulationAction::Execute()
{
	m_gxemul.ClearEmulation();
}


void ClearEmulationAction::Undo()
{
	assert(false);
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

static void Test_ClearEmulationAction_TestAction()
{
	GXemul gxemul;

	refcount_ptr<Component> dummyComponentA = new DummyComponent;
	refcount_ptr<Component> dummyComponentB = new DummyComponent;
	dummyComponentA->SetVariableValue("x", "123");
	dummyComponentB->SetVariableValue("x", "456");

	gxemul.GetRootComponent()->AddChild(dummyComponentA);
	gxemul.GetRootComponent()->AddChild(dummyComponentB);
	UnitTest::Assert("the root component should have children",
	    gxemul.GetRootComponent()->GetChildren().size(), 2);

	UnitTest::Assert("the gxemul instance should have no filename",
	    gxemul.GetEmulationFilename(), "");
	gxemul.SetEmulationFilename("hello.gxemul");
	UnitTest::Assert("the root component should have a filename",
	    gxemul.GetEmulationFilename(), "hello.gxemul");

	ActionStack& actionStack = gxemul.GetActionStack();

	refcount_ptr<Action> clearAction = new ClearEmulationAction(gxemul);
	actionStack.PushActionAndExecute(clearAction);

	UnitTest::Assert("the root component should have no children",
	    gxemul.GetRootComponent()->GetChildren().size(), 0);
	UnitTest::Assert("gxemul instance should have no filename after clear",
	    gxemul.GetEmulationFilename(), "");
	UnitTest::Assert("undo should not be possible",
	    actionStack.IsUndoPossible() == false);
}

UNITTESTS(ClearEmulationAction)
{
	UNITTEST(Test_ClearEmulationAction_TestAction);
}

#endif
