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
 *  $Id: AddComponentAction.cc,v 1.2 2008/03/12 11:45:41 debug Exp $
 */

#include "actions/AddComponentAction.h"
#include "components/DummyComponent.h"
#include "GXemul.h"


AddComponentAction::AddComponentAction(
		refcount_ptr<Component> componentToAdd,
		refcount_ptr<Component> whereToAddIt)
	: Action("add component")
	, m_componentToAdd(componentToAdd)
	, m_whereToAddIt(whereToAddIt)
{
}


AddComponentAction::~AddComponentAction()
{
}


void AddComponentAction::Execute()
{
	m_whereToAddIt->AddChild(m_componentToAdd);
}


void AddComponentAction::Undo()
{
	m_whereToAddIt->RemoveChild(m_componentToAdd);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

static void Test_AddComponentAction_WithUndoRedo()
{
	GXemul gxemulDummy(false);
	refcount_ptr<Component> dummyComponentA = new DummyComponent;
	refcount_ptr<Component> dummyComponentB = new DummyComponent;

	dummyComponentA->SetVariableValue("x", "123");
	dummyComponentB->SetVariableValue("x", "456");

	refcount_ptr<Action> actionA = new AddComponentAction(
	    dummyComponentA, gxemulDummy.GetRootComponent());
	refcount_ptr<Action> actionB = new AddComponentAction(
	    dummyComponentB, gxemulDummy.GetRootComponent());

	UnitTest::Assert("the root component should initially have no children",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 0);
	Checksum checksum0;
	gxemulDummy.GetRootComponent()->AddChecksum(checksum0);

	ActionStack& actionStack = gxemulDummy.GetActionStack();

	actionStack.PushActionAndExecute(actionA);
	UnitTest::Assert("the root component should have child A",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 1);

	Checksum checksumA;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumA);

	actionStack.PushActionAndExecute(actionB);
	UnitTest::Assert("the root component should have children A and B",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 2);

	Checksum checksumAB;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumAB);

	actionStack.Undo();
	UnitTest::Assert("the root component should have only child A",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 1);

	Checksum checksumA2;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumA2);
	UnitTest::Assert("checksums fail? A = A2", checksumA == checksumA2);

	actionStack.Redo();
	UnitTest::Assert("the root component should have child A and B again",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 2);

	Checksum checksumAB2;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumAB2);
	UnitTest::Assert("checksums fail? AB = AB2", checksumAB == checksumAB2);

	actionStack.Undo();
	actionStack.Undo();
	UnitTest::Assert("the root component should have no children",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 0);

	Checksum checksum02;
	gxemulDummy.GetRootComponent()->AddChecksum(checksum02);
	UnitTest::Assert("checksums fail? 0 = 02", checksum0 == checksum02);

	actionStack.Redo();
	actionStack.Redo();
	UnitTest::Assert("both children should be there again",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 2);

	Checksum checksumAB3;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumAB3);
	UnitTest::Assert("checksums fail? AB = AB3", checksumAB == checksumAB3);
}

UNITTESTS(AddComponentAction)
{
	UNITTEST(Test_AddComponentAction_WithUndoRedo);
}

#endif
