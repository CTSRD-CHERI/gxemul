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

#include <assert.h>

#include "actions/AddComponentAction.h"
#include "components/DummyComponent.h"
#include "GXemul.h"


AddComponentAction::AddComponentAction(
		GXemul& gxemul,
		refcount_ptr<Component> componentToAdd,
		refcount_ptr<Component> whereToAddIt)
	: Action("add component")
	, m_gxemul(gxemul)
	, m_componentToAdd(componentToAdd)
{
	m_whereToAddIt = whereToAddIt->GeneratePath();
}


AddComponentAction::~AddComponentAction()
{
}


void AddComponentAction::Execute()
{
	assert(m_componentToAdd->GetParent() == NULL);

	refcount_ptr<Component> whereToAdd =
	    m_gxemul.GetRootComponent()->LookupPath(m_whereToAddIt);

	whereToAdd->AddChild(m_componentToAdd);

	m_addedComponent = m_componentToAdd->GeneratePath();

	assert(m_componentToAdd->GetParent() != NULL);

	m_oldDirtyFlag = m_gxemul.GetDirtyFlag();
	m_gxemul.SetDirtyFlag(true);
}


void AddComponentAction::Undo()
{
	// Remove the child from its parent.
	refcount_ptr<Component> componentToRemove =
	    m_gxemul.GetRootComponent()->LookupPath(m_addedComponent);

	componentToRemove->GetParent()->RemoveChild(componentToRemove);
	assert(componentToRemove->GetParent() == NULL);

	m_componentToAdd = componentToRemove;

	m_gxemul.SetDirtyFlag(m_oldDirtyFlag);
}


/*****************************************************************************/


#ifndef WITHOUTUNITTESTS

#include "actions/ResetAction.h"

static void Test_AddComponentAction_WithUndoRedo()
{
	GXemul gxemulDummy(false);
	refcount_ptr<Component> dummyComponentA = new DummyComponent;
	refcount_ptr<Component> dummyComponentB = new DummyComponent;

	dummyComponentA->SetVariableValue("x", "123");
	dummyComponentB->SetVariableValue("x", "456");

	refcount_ptr<Action> actionA = new AddComponentAction(
	    gxemulDummy, dummyComponentA, gxemulDummy.GetRootComponent());
	refcount_ptr<Action> actionB = new AddComponentAction(
	    gxemulDummy, dummyComponentB, gxemulDummy.GetRootComponent());

	UnitTest::Assert("the root component should initially have no children",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 0);
	Checksum checksum0;
	gxemulDummy.GetRootComponent()->AddChecksum(checksum0);

	UnitTest::Assert("the model should initially not be dirty",
	    gxemulDummy.GetDirtyFlag() == false);

	ActionStack& actionStack = gxemulDummy.GetActionStack();

	actionStack.PushActionAndExecute(actionA);
	UnitTest::Assert("the root component should have child A",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 1);
	UnitTest::Assert("the model should now be dirty",
	    gxemulDummy.GetDirtyFlag() == true);

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
	UnitTest::Assert("the model should again be not dirty",
	    gxemulDummy.GetDirtyFlag() == false);

	Checksum checksum02;
	gxemulDummy.GetRootComponent()->AddChecksum(checksum02);
	UnitTest::Assert("checksums fail? 0 = 02", checksum0 == checksum02);

	actionStack.Redo();

	UnitTest::Assert("the model should again be dirty",
	    gxemulDummy.GetDirtyFlag() == true);

	actionStack.Redo();
	UnitTest::Assert("both children should be there again",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 2);

	Checksum checksumAB3;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumAB3);
	UnitTest::Assert("checksums fail? AB = AB3", checksumAB == checksumAB3);
}

static void Test_AddComponentAction_UndoAfterReset()
{
	GXemul gxemulDummy(false);
	refcount_ptr<Component> dummyComponentA = new DummyComponent;

	refcount_ptr<Action> actionA = new AddComponentAction(
	    gxemulDummy, dummyComponentA, gxemulDummy.GetRootComponent());

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

	// Reset
	refcount_ptr<Action> actionReset = new ResetAction(gxemulDummy);
	actionStack.PushActionAndExecute(actionReset);
	actionStack.Undo();

	UnitTest::Assert("the root component should have only child A",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 1);

	Checksum checksumA2;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumA2);
	UnitTest::Assert("checksums fail? A = A2", checksumA == checksumA2);

	actionStack.Undo();
	UnitTest::Assert("the root component should have no children",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 0);

	Checksum checksum02;
	gxemulDummy.GetRootComponent()->AddChecksum(checksum02);
	UnitTest::Assert("checksums fail? 0 = 02", checksum0 == checksum02);

	actionStack.Redo();
	UnitTest::Assert("the children should be there again",
	    gxemulDummy.GetRootComponent()->GetChildren().size(), 1);

	Checksum checksumA3;
	gxemulDummy.GetRootComponent()->AddChecksum(checksumA3);
	UnitTest::Assert("checksums fail? A = A3", checksumA == checksumA3);
}

UNITTESTS(AddComponentAction)
{
	UNITTEST(Test_AddComponentAction_WithUndoRedo);
	UNITTEST(Test_AddComponentAction_UndoAfterReset);
}

#endif
